/****************************************************************************************************

		PPoint.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PPoint.h"

#include "MCCountedPtrHelper.h"
#include "IShTokenStream.h"
#include "Vector2.h"
#include "PWall.h"
#include "PRoom.h"
#include "PPlan.h"
#include "PLevel.h"

#include "Utils.h"
#include "MiscComUtilsImpl.h"
#include "BuildingDef.h"
#include "I3dShFacetMesh.h"

VPoint::VPoint(BuildingPrimData* data, Level* inLevel, const TVector2& pos, Wall* onWall)
: CommonPoint(pos,data)
{
	fLevel = inLevel;
	
	fFlags = 0;
	fIndex = 0;

	fPointDomain = eInsideWallDomain;


	if(onWall)
		fWallArray.AddElem(onWall);

	fLevel->LevelPlan().AddPointReference(this);

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 12);
	SetName(fData->fDictionary, objectName);
}

VPoint::VPoint(BuildingPrimData* data, Level* inLevel, const TVector2& pos, const TMCCountedPtrArray<Wall>& onWalls)
: CommonPoint(pos,data)
{
	fLevel = inLevel;
	
	fPointDomain = eInsideWallDomain;

	fWallArray = onWalls;
	
	fLevel->LevelPlan().AddPointReference(this);

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 12);
	SetName(fData->fDictionary, objectName);
}

VPoint::~VPoint()
{
	// Deleting the last reference to the point, there shouldn't be any
	// Wall using this point anymore
	MY_ASSERT(fWallArray.GetElemCount()==0);

	// Clean just in case
//	DeletePoint();
}

void VPoint::CreatePoint(VPoint **point, BuildingPrimData* data, Level* inLevel, const TVector2& pos, Wall* onWall)
{
	TMCCountedCreateHelper<VPoint> result(point);

	result = new VPoint(data,inLevel,pos,onWall);
	ThrowIfNoMem(result);
}

void VPoint::CreatePoint(VPoint **point, BuildingPrimData* data, Level* inLevel, const TVector2& pos, const TMCCountedPtrArray<Wall>& onWalls)
{
	TMCCountedCreateHelper<VPoint> result(point);

	result = new VPoint(data,inLevel,pos,onWalls);
	ThrowIfNoMem(result);
}

void VPoint::DeletePoint()
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=wallCount-1 ; iWall>=0 ; iWall-- )
	{
		fWallArray[iWall]->DeleteWall();
	// The wall remove itself from the points 	fWallArray.RemoveElem(iWall,1);
	}

	// Must also supress reference in the Plan
	TMCCountedPtr<Level> levelPtr = fLevel;
	fLevel = NULL;
	levelPtr->LevelPlan().RemovePointReference(this);
}

void VPoint::SetPositionCheckWalls( const TVector2& newPos )
{
	TVector2 pos = newPos;
	// Check that we're not making a wall with a length == 0
	const int32 wallCount = fWallArray.GetElemCount();
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		Wall* wall = fWallArray[iWall];
		const real32 halfThick = wall->GetThickness();

		VPoint* otherPoint = wall->GetOtherPoint(this);
		const TVector2& otherPos = otherPoint->Position();
		if( (newPos-otherPos).GetSquaredNorm()<halfThick*halfThick )
		{
			// We're to near
			TVector2 dir = fPosition - otherPoint->Position();
			dir.Normalize();
			pos = otherPos + halfThick*dir;
		}
	}

	fPosition = pos;
}

TVector3 VPoint::Get3DPos() const
{
	return TVector3(fPosition.x,fPosition.y,fLevel->GetDistanceToGround());
}

const int32	VPoint::GetLevelIndex()const 
{
	return fLevel->GetLevelIndex();
}


void VPoint::RemoveWallReference( Wall* wall )
{
	// Find the wall and remove it
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=wallCount-1 ; iWall>=0 ; iWall-- )
	{
		if( wall == fWallArray[iWall] )
		{
			fWallArray.RemoveElem(iWall,1);

// Not need now. See later if there're more stuff in the deletePoint method
//			if(wallCount==1) // There's no more wall using this point, remove it
//				this->DeletePoint();
			ClearFlag(eWallOrdered);
		}
		else
		{
			// Invalidate the tessellation on all the walls arround
			fWallArray[iWall]->ClearFlag(eWallTessellated);
		}
	}
}

void VPoint::OrderWalls()
{
	if( Flag(eWallOrdered) )
		return; // Already done
	
	SetFlag(eWallOrdered);

	const int32 wallCount = fWallArray.GetElemCount();

	if( wallCount<3 ) return;// Nothing to do

	// Organize the walls in a counter-clockwise circle around the point
	for( int32 iWall=0 ; iWall<wallCount-1 ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		TVector2 dir = wall->GetOtherPoint(this)->Position();
		dir -= fPosition; // direction of this wall
		dir.Normalize();
		int32 iWanted = iWall+1;
		real32 smallestAngle = 360;
		for( int32 iSearch=iWall+1 ; iSearch<wallCount ; iSearch++ )
		{
			Wall* otherWall = fWallArray[iSearch];
			TVector2 otherDir = otherWall->GetOtherPoint(this)->Position();
			otherDir -= fPosition; // direction of the other wall
			otherDir.Normalize();

			// Now get the angle between dir and otherDir
			const real32 angle= GetPositiveAngle(dir, otherDir);
			if(angle<smallestAngle)
			{
				smallestAngle = angle;
				iWanted = iSearch;
			}
		}

		Wall* tmp = fWallArray[iWall+1];
		fWallArray.SetElem( iWall+1, fWallArray[iWanted] );
		fWallArray.SetElem( iWanted, tmp );
	}
}

int32 VPoint::GetWallIndex(const Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(wall == fWallArray[iWall])
			return iWall;
	}

	return -1;
}

Wall* VPoint::GetWall(VPoint* otherPoint)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(fWallArray[iWall]->GetOtherPoint(this) == otherPoint)
			return fWallArray[iWall];
	}

	return NULL;
}

int32 VPoint::GetWallIndex(Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(fWallArray[iWall] == wall)
			return iWall;
	}

	return -1;
}

Wall* VPoint::GetLeftWall(Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(wall == fWallArray[iWall])
			return fWallArray[(iWall+1)%wallCount];
	}

	return NULL;
}

Wall* VPoint::GetRightWall(Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(wall == fWallArray[iWall])
			return fWallArray[iWall==0?wallCount-1:iWall-1];
	}

	return NULL;
}

void VPoint::SetLevel(Level* level) {fLevel = level;}
void VPoint::SetData(BuildingPrimData* data) {fData = data;}

bool VPoint::GetLeftPos(TVector2& pos, Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	// Security (might be useless)
	if(GetWallIndex(wall)<0) return false;

	const int32 wallCount = fWallArray.GetElemCount();
	if(wallCount==1)
	{	// Special case: this point is an extremity
		pos = fPosition;
		const real32 wallThick2 = wall->GetThickness()*.5;
		TVector2 dir = wall->GetCurvedDirection(this);
		TVector2 vtxDir(-dir.y,dir.x);

		vtxDir*=wallThick2;
		pos.x+=vtxDir.x;
		pos.y+=vtxDir.y;
	}
	else
	{
		// Intersection with the wall beside
		Wall* nextWall = GetLeftWall(wall);
		GetIntersection(pos, wall, nextWall);
	}

	return true;
}

void VPoint::GetLeftPos(TVector3& pos, Wall* wall)
{
	TVector2 pos2d;
	if( !GetLeftPos(pos2d, wall) )
		return ;

	pos.SetFromXY(pos2d, fLevel->GetDistanceToGround());

	// See if we're in a room or outside
	Room* room=NULL;
	if(wall->GetPointIndex(this) == 0)
		room = wall->GetLeftRoom();
	else
		room = wall->GetRightRoom();

	if(room)
		pos.z+=room->GetFloorThickness();
}

bool VPoint::GetRightPos(TVector2& pos, Wall* wall)
{
	if( !Flag(eWallOrdered) )
		OrderWalls();

	if(GetWallIndex(wall)<0) return false;

	const int32 wallCount = fWallArray.GetElemCount();
	if(wallCount==1)
	{	// Special case: this point is an extremity
		// Init the position
		pos = fPosition;

		const real32 wallThick2 = wall->GetThickness()*.5;
		const TVector2 O = Position();
		TVector2 dir = wall->GetOtherPoint(this)->Position();
		dir -= O; // direction of this wall
		dir.Normalize();
		TVector2 vtxDir(dir.y,-dir.x);

		vtxDir*=wallThick2;
		pos.x+=vtxDir.x;
		pos.y+=vtxDir.y;
	}
	else
	{
		// Init the position
		Wall* prevWall = GetRightWall(wall);
		GetIntersection(pos, prevWall, wall);
	}

	return true;
}

void VPoint::GetRightPos(TVector3& pos, Wall* wall)
{
	TVector2 pos2d;
	if( !GetRightPos(pos2d, wall) )
		return;

	pos.SetFromXY(pos2d, fLevel->GetDistanceToGround());

	// See if we're in a room or outside
	Room* room=NULL;
	if(wall->GetPointIndex(this) == 1)
		room = wall->GetLeftRoom();
	else
		room = wall->GetRightRoom();

	if(room)
		pos.z+=room->GetFloorThickness();
}

void VPoint::GetIntersection(TVector2& intersection, Wall* wall1, Wall* wall2)
{
	intersection = fPosition;

	const real32 wall1HalfThick = wall1->GetThickness()*.5;
	const real32 wall2HalfThick = wall2->GetThickness()*.5;

	const TVector2 O = Position();
	TVector2 dir1 = wall1->GetCurvedDirection(this);
	TVector2 dir2 = wall2->GetCurvedDirection(this);

	if(wall1==wall2 || RealAbs(dir1^dir2)<kRealEpsilon)
	{	// an extremity wall or the 2 walls are aligned
		TVector2 vtxDir(-dir1.y,dir1.x);

		vtxDir*=wall1HalfThick;
		intersection.x+=vtxDir.x;
		intersection.y+=vtxDir.y;
	}
	else
	{	// Compute the intersection
		TVector2 O1(O.x-wall1HalfThick*dir1.y,
					O.y+wall1HalfThick*dir1.x);
		TVector2 O2(O.x+wall2HalfThick*dir2.y,
					O.y-wall2HalfThick*dir2.x);
		TVector2 OO1 = O1-O;
		TVector2 OO2 = O2-O;
		const real32 K1 = O1*OO1;
		const real32 K2 = O2*OO2;
		const real32 denom = OO1.y*OO2.x-OO2.y*OO1.x;

		if(denom)
		{
			intersection.x = (OO1.y*K2-OO2.y*K1)/denom;
			intersection.y = (OO2.x*K1-OO1.x*K2)/denom;
		}
		else if(OO1.GetNorm()>kRealEpsilon)
		{
			intersection = O1;
		}
		else if(OO2.GetNorm()>kRealEpsilon)
		{
			intersection = O2;
		}
		// else intersection == position
	}
}

boolean VPoint::IsInRoomPath() const
{
	const int32 wallCount = fWallArray.GetElemCount();

	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		Wall* wall = fWallArray[iWall];
		Room* room0 = wall->GetRoom(0);
		if(room0!=NULL)
		{
			Room* room1 = wall->GetRoom(1);
			if(room1!=room0)
				return true;
			else 
				return false; // Same room on both sides: inside the room, not in the path
		}
	}

	return false;
}

boolean VPoint::CheckConsistency(const boolean canDelete)
{
	boolean modif = false;

	VPoint* onPoint = fLevel->LevelPlan().PointOnPoint( this );
	if(onPoint)
	{
		// This point is on another one: merge them
		this->Merge( onPoint, canDelete );
		modif = true;
	}
	else
	{
		Wall* onWall = fLevel->LevelPlan().PosOnWall( fPosition );
		if(onWall)
		{
			// This point is on a wall: split the wall
			onWall->Split(this);
			modif = true;
		}
		else
		{
#ifdef USE_POINT_IN
			Room* inRoom = fLevel->LevelPlan().PosInRoom( fPosition );
			if(inRoom)
			{
				if( inRoom->GetPathPointIndex(this) == -1 &&
					inRoom->GetInPointIndex(this) == -1 )
				{
					// Add the point in
					if(!IsInRoomPath())
					{
						// If the point is already register as a point inside another room, we need to remove if first
						const int32 wallCount = fWallArray.GetElemCount();

						for(int32 iWall=0 ; iWall<wallCount ; iWall++)
						{
							Wall* wall = fWallArray[iWall];
							Room* room0 = wall->GetRoom(0);
							if(room0!=NULL)
							{
								MY_ASSERT( wall->GetRoom(1)==room0 );
								wall->RemoveRoomReference(room0);
								room0->ReplacePointReferences(this,NULL);
							}
						}

						// Warning: this will add also the points on the border (might not be what we want)
						inRoom->AddPointInReference(this);
						for(int32 i=0 ; i<wallCount ; i++)
						{
							Wall* wall = fWallArray[i];
							wall->SetRoom(0,inRoom);
							wall->SetRoom(1,inRoom);
						}
						InvalidateTessellation();
						modif = true;
					}
				}
			}
			else
			{	// If the point is outside any room, check that there's no more memory of a room
				if(!IsInRoomPath())
				{
					// If the point is already register as a point inside another room, we need to remove if first
					const int32 wallCount = fWallArray.GetElemCount();

					for(int32 iWall=0 ; iWall<wallCount ; iWall++)
					{
						Wall* wall = fWallArray[iWall];
						Room* room0 = wall->GetRoom(0);
						if(room0!=NULL)
						{
							MY_ASSERT( wall->GetRoom(1)==room0 );
							wall->RemoveRoomReference(room0);
							room0->ReplacePointReferences(this,NULL);
						}
					}
				}
			}
#endif
		}
	}

	return modif;
}

// Merge the passed point into this one
void VPoint::Merge(VPoint* mergedPoint, const boolean canDelete)
{
	MY_ASSERT(mergedPoint);
//	MY_ASSERT(Flag(ePointToKeep));

	// The room shapes and numbers can be completly modify, so just remove them and
	// rebuild them

	const int32 otherWallCount = mergedPoint->GetWallCount();
	if(!otherWallCount)
	{	// This point is not used, just remove it from the database
		if(canDelete)
			mergedPoint->DeletePoint();
		else
			mergedPoint->SetFlag(ePointToDelete);

		return;
	}

	{
		// Mark the walls
		for( int32 iWall=0 ; iWall<otherWallCount ; iWall++ )
		{
			Wall* wall = mergedPoint->GetWall(iWall);
			Room* room0 = wall->GetRoom(0);
			Room* room1 = wall->GetRoom(1);
			if(room0!=room1) // pointInRoom case does not destroy the room
			{
				if(room0) wall->SetFlag(eWallRebuildRoom0);
				if(room1) wall->SetFlag(eWallRebuildRoom1);
			}
		}
	}

	const int32 wallCount = fWallArray.GetElemCount();
	{	// Mark the walls
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];
			Room* room0 = wall->GetRoom(0);
			Room* room1 = wall->GetRoom(1);
			if(room0!=room1) // pointInRoom case does not destroy the room
			{
				if(room0) wall->SetFlag(eWallRebuildRoom0);
				if(room1) wall->SetFlag(eWallRebuildRoom1);
			}
		}
	}

	// Then merge
	{
		Wall* commonWall = GetWall(mergedPoint);
		if(commonWall)
		{
			// Clean up the referenc in the rooms before deleting the wall so we won't destroy the room
			Room* room0 = commonWall->GetRoom(0);
			if(room0)
			{
				room0->ReplacePointReferences(mergedPoint,NULL);
				commonWall->SetRoom(0,NULL);
			}
			Room* room1 = commonWall->GetRoom(1);
			if(room1)
			{
				room1->ReplacePointReferences(mergedPoint,NULL);
				commonWall->SetRoom(1,NULL);
			}
			commonWall->DeleteWall();
		}

		// Replace the references in the walls
		while(mergedPoint->GetWallCount())
		{
			Wall* wall = mergedPoint->GetWall((int32)0);

			wall->ReplacePointReference(mergedPoint,this);
		}

		// Delete
		if(canDelete)
			mergedPoint->DeletePoint();
		else
			mergedPoint->SetFlag(ePointToDelete);

		ClearFlag(eWallOrdered);
	}
}

void VPoint::SetSelection( const boolean select )
{
	if( select != Selected() )
	{
		if(select)
		{
			SetFlag(eIsSelected);
			
			// Select Walls and Rooms using this point if possible
			const int32 wallCount = fWallArray.GetElemCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = fWallArray[iWall];
				if( wall->GetOtherPoint(this)->Selected() )
					wall->SetFlag(eIsSelected);
				Room* room0 = wall->GetRoom(0);
				if(room0) room0->SelectIfPossible();
				Room* room1 = wall->GetRoom(1);
				if(room1) room1->SelectIfPossible();
			}

			// Check if the level is selected
			fLevel->SelectIfPossible();
		}
		else		
		{
			ClearFlag(eIsSelected);
			fLevel->ClearFlag(eIsSelected);

			// Deselect Walls and Rooms using this point
			const int32 wallCount = fWallArray.GetElemCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = fWallArray[iWall];
				wall->ClearFlag(eIsSelected);
				Room* room0 = wall->GetRoom(0);
				if(room0) room0->ClearFlag(eIsSelected);
				Room* room1 = wall->GetRoom(1);
				if(room1) room1->ClearFlag(eIsSelected);
			}
		}

		fData->InvalidateStatus();
	}
}

void VPoint::InvalidateTessellation(const boolean extraInvalidation)
{
	ClearFlag(ePointTessellated);

	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		wall->InvalidateTessellation();

		VPoint* otherPoint = wall->GetOtherPoint(this);

		// Add also the left and the right walls ( need to recompute their intersection )
		const int32 otherWalls = otherPoint->GetWallCount();
		for( int32 other=0 ; other<otherWalls ; other++ )
		{
			otherPoint->GetWall(other)->InvalidateTessellation();
		}

		if( wall->GetRoom(0) )
			wall->GetRoom(0)->InvalidateTessellation();

		if( wall->GetRoom(1) )
			wall->GetRoom(1)->InvalidateTessellation();
	}

	if(extraInvalidation)
		ClearFlag(eWallOrdered);
}

void AddVertex(TMCClassArray<Vertex>& onArray, int32 index,
			   const TVector3& pos3D,
			   const TVector3& normal,
			   const TVector2& uv )
{
	onArray[index].SetPosition(pos3D);
	onArray[index].SetNormal(normal);
	onArray[index].SetUV(uv);
}

void VPoint::BuildColumn( real32 zBottom, real32 zTop, const TMCClassArray<TVector2>& posArround )
{
	const int32 prevVtxCount = fVertexArray.GetElemCount();
	const int32 posCount = posArround.GetElemCount();
	const int32 prevTglCount = fTriangleArray.GetElemCount();
	
	// 4*: each vertice as 2 normals
	fVertexArray.AddElemCount(4*posCount);
	fTriangleArray.AddElemCount(2*posCount);

	// Vertices arround
	{
		for(int32 iPos=0 ; iPos<posCount ; iPos++)
		{
			const int32 curVtxIndex = prevVtxCount+4*iPos;

			const TVector2& curPos2D = posArround[iPos];
			const TVector2& nextPos2D = posArround[(iPos+1)%posCount];

			// compute the normal
			TVector2 dir = nextPos2D-curPos2D;
			real32 length = dir.Normalize(dir);
			TVector3 normal(dir.y, -dir.x, 0 );

			// Add the 4 vertices
			int32 curIndex = curVtxIndex;
			AddVertex(fVertexArray, curIndex, TVector3(curPos2D.x, curPos2D.y, zBottom ), 
							normal, fData->UVData().ComputeUV(TVector2(-length,zBottom)));

			curIndex++;;
			AddVertex(fVertexArray, curIndex, TVector3(curPos2D.x, curPos2D.y, zTop ), 
							normal, fData->UVData().ComputeUV(TVector2(-length,zTop)));
			
			curIndex++;;
			AddVertex(fVertexArray, curIndex, TVector3(nextPos2D.x, nextPos2D.y, zBottom ), 
							normal, fData->UVData().ComputeUV(TVector2(0,zBottom)));

			curIndex++;;
			AddVertex(fVertexArray, curIndex, TVector3(nextPos2D.x, nextPos2D.y, zTop ), 
							normal, fData->UVData().ComputeUV(TVector2(0,zTop)));

			// Build the 2 triangles
			curIndex = prevTglCount + 2*iPos;
			fTriangleArray[curIndex].pt1 =  curVtxIndex;
			fTriangleArray[curIndex].pt2 =  curVtxIndex+1;
			fTriangleArray[curIndex].pt3 =  curVtxIndex+2;
			curIndex++;
			fTriangleArray[curIndex].pt1 =  curVtxIndex+3;
			fTriangleArray[curIndex].pt2 =  curVtxIndex+2;
			fTriangleArray[curIndex].pt3 =  curVtxIndex+1;
		}
	}
}

void VPoint::BuildPolygon( const TVector3& normal, real32 z, const TMCClassArray<TVector2>& posArround )
{
	const int32 startIndex = fVertexArray.GetElemCount();
	const int32 posCount = posArround.GetElemCount();
	
	fVertexArray.AddElemCount(posCount+1);

	// Center
	fVertexArray[startIndex].SetPosition(TVector3(fPosition.x,fPosition.y,z));
	fVertexArray[startIndex].SetNormal(normal);
	fVertexArray[startIndex].SetUV(fData->UVData().ComputeUV(fPosition));

	// Vertices arround
	{
		for(int32 iPos=0 ; iPos<posCount ; iPos++)
		{
			const int32 curIndex = startIndex+iPos+1;
			TVector3 pose3D(posArround[iPos].x, posArround[iPos].y, z );
			fVertexArray[curIndex].SetPosition(pose3D);
			fVertexArray[curIndex].SetNormal(normal);
			fVertexArray[curIndex].SetUV(fData->UVData().ComputeUV(posArround[iPos])); // , eWallBetween));
		}
	}

	// Triangles fan
	{
		const int32 prevTglCount = fTriangleArray.GetElemCount();
		fTriangleArray.AddElemCount(posCount);
		for(int32 iPos=0 ; iPos<posCount ; iPos++)
		{
			const int32 curIndex = prevTglCount + iPos;
			fTriangleArray[curIndex].pt1 =  startIndex;
			fTriangleArray[curIndex].pt2 =  startIndex+iPos+1;
			if(iPos+1<posCount)
				fTriangleArray[curIndex].pt3 =  startIndex+iPos+2;
			else
				fTriangleArray[curIndex].pt3 =  startIndex+1;
		}
	}
}

void VPoint::ValidateTessellation()
{
	if( Flag(ePointTessellated) ) 
		return; // Tessellation is valid

	// Clear previous data
	fTriangleArray.ArrayFree();
	fVertexArray.ArrayFree();

	// 3D tessellation

	// TODO: get the wall domain
	fPointDomain = eInsideWallDomain;

	// Mesh only in wallCount >= 3
	const int32 wallCount = GetWallCount();
	if(wallCount>=3)
	{
		// Get the highest one
		// Get the pos arround the point
		bool drawColumn = false;
		real32 wallHeight = GetWall((int32)0)->GetWallHeight();
		TMCClassArray<TVector2> posArround(wallCount);
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* curWall =  GetWall(iWall);
			if( curWall->GetWallHeight()!=wallHeight )
			{
				drawColumn = true;
				if(curWall->GetWallHeight()>wallHeight)
					wallHeight = curWall->GetWallHeight();
			}
		
			GetLeftPos(posArround[iWall], curWall);
		}

		const real32 zBottom = fLevel->GetDistanceToGround();
		const real32 zTop = zBottom + wallHeight;

		// Build a column
		if(drawColumn)
		{
			BuildColumn( zBottom, zTop, posArround );
		}

		// Build the facet on the top 
		BuildPolygon( TVector3::kUnitZ, zTop, posArround );

		// Build the facet on the bottom
		BuildPolygon( -TVector3::kUnitZ, zBottom, posArround );
	}

	SetFlag(ePointTessellated);
}

void VPoint::SetExtendedSelection()
{
	if(Selected())
	{
		const int32 wallCount = fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];

			wall->SetFlag(eWallExtendedSelection);
			VPoint* otherPoint = wall->GetOtherPoint(this);
			if(!otherPoint->Selected())
			{
				// Add also the left and the right walls ( need to recompute their intersection )
				const int32 otherWalls = otherPoint->GetWallCount();
				for( int32 other=0 ; other<otherWalls ; other++ )
				{
					otherPoint->GetWall(other)->SetFlag(eWallExtendedSelection);
				}
			}

			if( wall->GetRoom(0) )
				wall->GetRoom(0)->SetFlag(eRoomExtendedSelection);

			if( wall->GetRoom(1) )
				wall->GetRoom(1)->SetFlag(eRoomExtendedSelection);

			// If a point is moved, the tessellation need to be invaidated in the levels under and over
//			if(fLevel->GetLevelOver())
//				fLevel->GetLevelOver()->LevelPlan().AddWallsAndRoomsToExtendedSelection();
//			if(fLevel->GetLevelUnder())
//				fLevel->GetLevelUnder()->LevelPlan().AddWallsAndRoomsToExtendedSelection();
		}
	}
	else
	{
		const int32 wallCount = fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];

			if(!wall->GetOtherPoint(this)->Selected())
				wall->ClearFlag(eWallExtendedSelection);

			Room* room0 = wall->GetRoom(0);
			if( room0 )
			{
				boolean reset = true;
				const int32 pathCount = room0->GetPathPointCount();
				for(int32 iPt=0 ; iPt<pathCount ; iPt++ )
				{
					if(room0->GetPathPoint(iPt)->Selected())
					{
						reset=false;
						break;
					}
				}
				if(reset)
					room0->ClearFlag(eRoomExtendedSelection);
			}

			Room* room1 = wall->GetRoom(1);
			if( room1 )
			{
				boolean reset = true;
				const int32 pathCount = room1->GetPathPointCount();
				for(int32 iPt=0 ; iPt<pathCount ; iPt++ )
				{
					if(room1->GetPathPoint(iPt)->Selected())
					{
						reset=false;
						break;
					}
				}
				if(reset)
					room1->ClearFlag(eRoomExtendedSelection);
			}

			// If there's no more selection in this level, we might be able to remove the
			// extended selection from the levels over and under
//			Level* over = fLevel->GetLevelOver();
//			Level* under = fLevel->GetLevelUnder();
//			if(over || under)
//			{
//				if(!fLevel->LevelPlan().HasPointSelection())
//				{
//					if(over)
//						over->LevelPlan().RemoveWallsAndRoomsFromExtendedSelection();
//					if(under)
//						under->LevelPlan().RemoveWallsAndRoomsFromExtendedSelection();
//				}
//			}
		}
	}
}

void VPoint::DeselectIfPossible()
{	// Deselect if nothing using this point is selected
	if(Selected())
	{
		const int32 wallCount = fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];

			if(wall->Selected())
				return;

			if( wall->GetRoom(0) && wall->GetRoom(0)->Selected() )
				return;

			if( wall->GetRoom(1) && wall->GetRoom(1)->Selected() )
				return;
		}

		ClearFlag(eIsSelected);
	}
}

boolean VPoint::SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode) 
{
	if( Hidden() )
		return false;

	boolean outside=false;

	const TVector3 pos = Get3DPos();

	const int32 planeCount = rayPlanes.GetElemCount();
	for( int32 iPlane=0; iPlane<planeCount; iPlane++ )
	{
		const Plane& rayPlane = rayPlanes[iPlane];
		if( ((pos-rayPlane.fPoint)*rayPlane.fNormal) < kRealZero)
		{
			outside=true;
			break;
		}
	}

	if(outside) // Outside the selection area: restaure the selection as it was before
	{
		// Restore the selection on the point
		RestoreSelection();

		// Restore the selection on the walls and rooms
		const int32 wallCount = fWallArray.GetElemCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];
			wall->RestoreSelection();

// They take care of themselves
//			const int32 objCount = wall->GetObjectCount();
//			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
//				wall->GetObject(iObj)->RestoreSelection();

			Room* room0 = wall->GetRoom(0);
			if(room0)
			{
				room0->RestoreIfPossible();
// They take care of themselves
//				const int32 objCount = room0->GetObjectCount();
//				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
//					room0->GetObject(iObj)->RestoreSelection();
			}
			Room* room1 = wall->GetRoom(1);
			if(room1)
			{
				room1->RestoreIfPossible();
// They take care of themselves
//				const int32 objCount = room1->GetObjectCount();
//				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
//					room1->GetObject(iObj)->RestoreSelection();
			}
		}
	}
	else // Inside the area: select or deselect
	{
		SetSelection(true);

	/*	if( marqueeMode == kMarqueeSelect )
		{
			if (fSelectionFlags&kMarqueeVertex)
				Select(false, false);
			return Select(true);
		}
		else if (marqueeMode == kMarqueeSwap) 
		{
			if (fSelectionFlags&kMarqueeVertex)
				return Select(false);
			else
				return Select(true);
		}
		else if (marqueeMode == kMarqueeDeselect)
		{
			if (fSelectionFlags&kMarqueeVertex)
				Select(true, false);
			return Select(false);
		}*/
	}

	return true;
}

void VPoint::InvertSelection() 
{
	if( Hidden() )
		return;

	if(Flag(eWasSelected))
	{	// Deselect this point if possible:
		// check that all the wall arroud were selected
		boolean canDeselect = true;
		const int32 wallCount = fWallArray.GetElemCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];
			if( wall->Flag(eWasSelected) )
			{
				wall->ClearFlag(eIsSelected);
			}
			else
			{
				canDeselect = false;
				wall->SetFlag(eIsSelected);
			}

			{
				const int32 objCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					wall->GetObject(iObj)->InvertSelection();
			}

			Room* room0 = wall->GetRoom(0);
			if(room0)
			{
				if(room0->Flag(eWasSelected))
					room0->ClearFlag(eIsSelected);

				const int32 objCount = room0->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					room0->GetObject(iObj)->InvertSelection();
			}
			Room* room1 = wall->GetRoom(1);
			if(room1)
			{
				if(room1->Flag(eWasSelected))
					room1->ClearFlag(eIsSelected);

				const int32 objCount = room1->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					room1->GetObject(iObj)->InvertSelection();
			}
		}

		if(canDeselect)
			ClearFlag(eIsSelected);
	}
	else // Select this point
	{
		SetFlag(eIsSelected);

		const int32 wallCount = fWallArray.GetElemCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];
			const boolean wasSelected = wall->Flag(eWasSelected);
			if( wall->Flag(eWasSelected) )
				wall->ClearFlag(eIsSelected);
			else
				wall->SetFlag(eIsSelected);

			{
				const int32 objCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					wall->GetObject(iObj)->InvertSelection();
			}

			Room* room0 = wall->GetRoom(0);
			if(room0)
			{
				if(!room0->Flag(eWasSelected))
					room0->SelectIfPossible();

				const int32 objCount = room0->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					room0->GetObject(iObj)->InvertSelection();
			}
			Room* room1 = wall->GetRoom(1);
			if(room1)
			{
				if(!room1->Flag(eWasSelected))
					room1->SelectIfPossible();

				const int32 objCount = room1->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
					room1->GetObject(iObj)->InvertSelection();
			}
		}
	}
}

void VPoint::HidePoint()
{
	// Hide the point only if there's no visible wall or room using this point
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		if(!wall->Hidden())
			return;
		if(wall->GetRoom(0))
			if(!wall->GetRoom(0)->Hidden()) return;
		if(wall->GetRoom(1))
			if(!wall->GetRoom(1)->Hidden()) return;
	}
	SetFlag(eIsHidden);
	ClearFlag(eIsSelected);
}

void VPoint::SetWallFlag(const int32 flag)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->SetFlag(flag);
	}
}

void VPoint::ClearWallFlag(const int32 flag)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->ClearFlag(flag);
	}
}

void VPoint::GetSurroundingPoints(TMCClassArray<TVector2>& points)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		if(wall->Hidden())
			continue;

		points.AddElem(wall->GetOtherPoint(this)->Position());
	}
}

void VPoint::GetSurroundingPoints(TMCCountedPtrArray<CommonPoint>& pointsArround)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		if(wall->Hidden())
			continue;

		VPoint* otherPoint = wall->GetOtherPoint(this);

		if(!otherPoint->Selected())
			pointsArround.AddElem(otherPoint);
	}
}

bool VPoint::GetPointFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags)
{
	TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = vertices.GetElemCount();
	if(!vertexCount)
		return false;

	TMCClassArray<Triangle>& facets = Triangles();
	const int32 facetCount = facets.GetElemCount();
	if(!facetCount)
		return false;

	FacetMesh::Create(outMesh);
	TMCCountedPtr<FacetMesh> facetMesh;
	facetMesh = *outMesh;

	const boolean needcolor = FLAG(meshFlags,eShellMesh)?false:true;

	TMCArray<TVector3>& meshVertices = facetMesh->fVertices;
	TMCArray<TVector2>& meshUVs = facetMesh->fuv;
	TMCArray<TVector3>& meshNormals = facetMesh->fNormals;
	TMCArray<Triangle>& meshFacets = facetMesh->fFacets;
	TMCArray<uint32>& meshUVSpaceIDs = facetMesh->fUVSpaceID;
	TMCArray<TMCColorRGBA8>& meshColors = facetMesh->fPolygonColors;
	//TMCArray<TMCColorRGBA8>& backColors = facetMesh->fPolygonBackColors;

	meshVertices.SetElemCount(vertexCount); 
	meshUVs.SetElemCount(vertexCount); 
	meshNormals.SetElemCount(vertexCount); 
	meshFacets.SetElemCount(facetCount);   
	meshUVSpaceIDs.SetElemCount(facetCount);   
	if(needcolor)
		meshColors.SetElemCount(facetCount);

	const TMCColorRGBA8 color = (Selected()?fData->fSelCol:
							(Targeted()?fData->fTarCol:
							Flag(eSnapedPosition)?fData->fSnaCol:
							Flag(eWallHelper)?fData->fHelCol:fData->fDefCol));

	const boolean faceted = FLAG(meshFlags, eFaceted);

	// Fill the vertices positions
	for( int32 iVertex=0 ; iVertex<vertexCount ; iVertex++ )
	{
		const int32 index = iVertex;
		meshVertices[index] = vertices[iVertex].Position();
		meshUVs[index] = vertices[iVertex].UV();
		meshNormals[index] = vertices[iVertex].Normal();
	}

	// Fill the facets
	for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
	{
		meshFacets[iFacet] = facets[iFacet];
		meshUVSpaceIDs[iFacet] = fPointDomain;

		if(needcolor)
		{
			if(faceted)
				meshColors[iFacet] = GetFacetedColor(meshNormals[meshFacets[iFacet].pt1], color);
			else
				meshColors[iFacet] = color;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////
//
//
//
void VPoint::Clone(VPoint** newPoint, Level* inLevel)
{
	VPoint::CreatePoint(newPoint,inLevel->GetPrimitiveData(),inLevel,fPosition,NULL);

	TMCCountedPtr<VPoint> pointPtr;	
	pointPtr = *newPoint;	
	pointPtr->SetFlags(fFlags);
	pointPtr->SetNamePtr(fName);

	pointPtr->ClearFlag(eWallOrdered);
	pointPtr->ClearFlag(eIsTargeted);
	pointPtr->ClearFlag(ePointHelper);
}

/****************************************************************************************************

		PRoom.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PRoom.h"

#include "MCCountedPtrHelper.h"
#include "I3dShFacetMesh.h"
#include "IShTokenStream.h"

#include "PWall.h"
#include "PPoint.h"
#include "PPlan.h"
#include "PLevel.h"

#include "Utils.h"
#include "PTessellator.h"
#include "MiscComUtilsImpl.h"
#include "BuildingDef.h"

Room::Room(BuildingPrimData* data, Level* inLevel, 
		TMCCountedPtrArray<VPoint>& path, const boolean turnLeft)
{
	fLevel = inLevel;

	fLevel->LevelPlan().AddRoomReference(this);

	fData = data;

	fPath = path;

	fOffset = 0;
	fTgleOffset = 0;

	fFlags = 0;
	if(turnLeft)	SetFlag(eTurnLeft);
	else			ClearFlag(eTurnLeft);

	fFloorThickness = kDefaultThickness;
	fCeilingThickness = kDefaultThickness;

	fFloorDomain = eInsideFloorDomain;
	fCeilingDomain = eInsideCeilingDomain;

	// Must register the new room in its walls
	RegisterInWalls();

	// Invalidate the tessellation
	InvalidateTessellation(true);

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 9);
	SetName(fData->fDictionary, objectName);
}

Room::~Room()
{
}

void Room::CreateRoom(Room **room, BuildingPrimData* data, Level* inLevel, 
					  TMCCountedPtrArray<VPoint>& path, const boolean turnLeft)
{
	TMCCountedCreateHelper<Room> result(room);

	result = new Room(data, inLevel, path, turnLeft);
	ThrowIfNoMem(result);
}

void Room::DeleteRoom()
{
	UnregisterInWalls();

	fPath.ArrayFree();
#ifdef USE_POINT_IN
	fPointInArray.ArrayFree(); // poit that define walls inside the room
#endif

	// Delete the objects
	//const int32 objCount = fSubObjects.GetElemCount();

	//for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
	//{
	//	RoomSubObject* object = fSubObjects[iObj];
	//	object->DeleteObj();

	//}
	fSubObjects.ArrayFree();

	TMCCountedPtr<Level> levelPtr = fLevel;
	fLevel = NULL;
	levelPtr->LevelPlan().RemoveRoomReference(this);
}

void Room::UnregisterInWalls()
{
	const int32 pointCount = fPath.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		Wall* wall = fPath[iPoint]->GetWall(fPath[(iPoint+1)%pointCount]);
		if(wall)
			wall->RemoveRoomReference(this);
		else
			MCNotify("No Wall");
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for(int32 iIn=0 ; iIn<inCount ; iIn++)
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			if(wall)
				wall->RemoveRoomReference(this);
			else
				MCNotify("No Wall");
		}
	}
#endif
}


void Room::RegisterInWalls()
{
	const int32 pointCount = fPath.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		Wall* wall = fPath[iPoint]->GetWall(fPath[(iPoint+1)%pointCount]);

		if(!MCVerify (wall) )
		{	// Error: no wall between these 2 points
			// We build a new one to avoid the crash.
			VPoint* point1 = fPath[iPoint];
			VPoint* point2 = fPath[(iPoint+1)%pointCount];

			wall = fLevel->MakeWall( point1, point2, eBasic, false );
		}

		if( wall->GetPointIndex(fPath[iPoint])==0 )
			wall->SetRoom((Flag(eTurnLeft)?0:1), this);
		else
			wall->SetRoom((Flag(eTurnLeft)?1:0), this);

	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for(int32 iIn=0 ; iIn<inCount ; iIn++)
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			wall->SetRoom(0,this);
			wall->SetRoom(1,this);
		}
	}
#endif
}

void Room::ReplacePointReferences(VPoint* oldPt,VPoint* newPt)
{
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		if( fPath[iPoint]==oldPt )
		{
			if(newPt)
				fPath.SetElem(iPoint, newPt);
			else
				fPath.RemoveElem(iPoint,1);
		return;
		}
	}			
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		if( fPointInArray[iIn] == oldPt )
		{
			if(newPt)
				fPointInArray.SetElem(iIn, newPt);
			else
				fPointInArray.RemoveElem(iIn,1);
			return;
		}
	}
#endif
}

void Room::SetSelection(const boolean select)
{
	if( select != Selected() )
	{
		if(select)
		{
			SetFlag(eIsSelected);

			// Select the points making the room
			const int32 pointCount = fPath.GetElemCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				fPath[iPoint]->SetFlag(eIsSelected);
			}
#ifdef USE_POINT_IN
			const int32 inCount = fPointInArray.GetElemCount();
			for( int32 iIn=0 ; iIn<inCount ; iIn++ )
			{
				fPointInArray[iIn]->SetFlag(eIsSelected);
			}
#endif
			// Check if the level is selected
			fLevel->SelectIfPossible();
		}
		else		
		{
			ClearFlag(eIsSelected);
			fLevel->ClearFlag(eIsSelected);

			// Deselect the points that aren't used anymore
			const int32 pointCount = fPath.GetElemCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				fPath[iPoint]->DeselectIfPossible();
			}
#ifdef USE_POINT_IN
			const int32 inCount = fPointInArray.GetElemCount();
			for( int32 iIn=0 ; iIn<inCount ; iIn++ )
			{
				fPointInArray[iIn]->DeselectIfPossible();
			}
#endif
		}

		fData->InvalidateStatus();
	}
}

// Select the room, its objects, its walls, and the wall objects
void Room::SetCompleteSelection()
{
	SetFlag(eIsSelected);

	{
		const int32 objCount = fSubObjects.GetElemCount();

		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			fSubObjects[iObj]->SetFlag(eIsSelected);
		}
	}

	// Select the points making the room
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPath[iPoint]->SetFlag(eIsSelected);
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		VPoint* curPoint = fPointInArray[iIn];

		curPoint->SetFlag(eIsSelected);
		curPoint->SetExtendedSelection();
		const int32 wallCount = curPoint->GetWallCount();
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = curPoint->GetWall(iWall);
			wall->SetFlag(eIsSelected);

			const int32 objCount = wall->GetObjectCount();
			for(int32 iObj=0 ; iObj<objCount ; iObj++)
			{
				wall->GetObject(iObj)->SetFlag(eIsSelected);
			}
		}
	}
#endif

	// Select the wall and objects
	for(int32 iWall=0 ; iWall<pointCount ; iWall++)
	{
		Wall* wall = GetPathWall(iWall);
		wall->SetFlag(eIsSelected);

		const int32 objCount = wall->GetObjectCount();
		for(int32 iObj=0 ; iObj<objCount ; iObj++)
		{
			wall->GetObject(iObj)->SetFlag(eIsSelected);
		}
	}

	// Check if the level is selected
	fLevel->SelectIfPossible();
}

void Room::SelectIfPossible()
{
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		if( !fPath[iPoint]->Selected() ) return;
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		if( !fPointInArray[iIn]->Selected() ) return;
	}
#endif

	SetFlag(eIsSelected);
}

void Room::RestoreIfPossible()
{
	const boolean wasSelected =Flag(eWasSelected);

	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		if( wasSelected && !fPath[iPoint]->Selected() )
			return;
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		if( wasSelected && !fPath[iPoint]->Selected() )
			return;
	}
#endif

	RestoreSelection();
}

void Room::ShowRoom()
{
	if(!Hidden())
		return;

	ClearFlag(eIsHidden);
	// Clear objects flag
	const int32 objectCount = GetObjectCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		GetObject(iObj)->ClearFlag(eIsHidden);
	}
	// Show the points, even if the wall aren't visible
	ClearPointFlag(eIsHidden);
}

void Room::HideRoom()
{
	if(Hidden())
		return;

	SetFlag(eIsHidden);
	ClearFlag(eIsSelected);
	// Clear objects flag
	const int32 objectCount = GetObjectCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		GetObject(iObj)->SetFlag(eIsHidden);
		GetObject(iObj)->ClearFlag(eIsSelected);
	}
	// Don't touch the point, the walls could be still visible
}

void Room::SetPointFlag(const int32 flag)
{
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPath[iPoint]->SetFlag(flag);
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		fPointInArray[iIn]->SetFlag(flag);
	}
#endif
}

void Room::ClearPointFlag(const int32 flag)
{
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPath[iPoint]->ClearFlag(flag);
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		fPointInArray[iIn]->ClearFlag(flag);
	}
#endif
}

void Room::SetWallFlag(const int32 flag)
{
	const int32 pointCount = fPath.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		Wall* wall = fPath[iPoint]->GetWall(fPath[(iPoint+1)%pointCount]);
		if(wall) wall->SetFlag(flag);
		else MCNotify("No Wall");
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for(int32 iIn=0 ; iIn<inCount ; iIn++)
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			if(wall) wall->SetFlag(flag);
			else MCNotify("No Wall");
		}
	}
#endif
}

Wall* Room::GetPathWall(const int32 index) const
{
	const int32 pointCount = fPath.GetElemCount();
	return fPath[index]->GetWall(fPath[(index+1)%pointCount]);
}

void Room::ClearWallFlag(const int32 flag)
{
	const int32 pointCount = fPath.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		Wall* wall = fPath[iPoint]->GetWall(fPath[(iPoint+1)%pointCount]);
		if(wall) wall->ClearFlag(flag);
		else MCNotify("No Wall");
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for(int32 iIn=0 ; iIn<inCount ; iIn++)
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			if(wall) wall->ClearFlag(flag);
			else MCNotify("No Wall");
		}
	}
#endif
}

void Room::AddObjectReference(RoomSubObject* obj)
{
	fSubObjects.AddElem(obj);
	ClearFlag(eRoomTessellated);
}

void Room::RemoveObjectReference(RoomSubObject* obj)
{
	// Find the obj and remove it from the array
	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		if(obj == fSubObjects[iObj])
		{
			if(iObj == objCount-1) // Last elem, just remove it
				fSubObjects.RemoveElem(iObj,1);
			else
			{
				fSubObjects.SetElem( iObj, fSubObjects[objCount-1] );
				fSubObjects.RemoveElem(objCount-1,1);
			}
			ClearFlag(eRoomTessellated); // do not need to invalidate the basic tessellation

			break;
		}
	}
}

void Room::InvalidateTessellation(const boolean invalidateArround)
{
	if(invalidateArround)
	{
		const int32 pointCount = fPath.GetElemCount();
		for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
		{
			fPath[iPoint]->InvalidateTessellation();
		}
	}
	else
	{
		ClearFlag(eRoomTessellated);
		ClearFlag(eRoomBasicTessellated);
		ClearFlag(eRoomObjPositionned);
		// Invalidate the objects 3D data
		const int32 objectCount = fSubObjects.GetElemCount();
		for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
		{	// Note: we shouldn't need to invalidate the 2D center pos here, but it fix a bug when scaling an object (they somtimes went crazy)
			fSubObjects[iObj]->Invalidate();
			fSubObjects[iObj]->InvalidateTessellationOver();
		}
	}
}

void Room::SetLevel(Level* level) {fLevel = level;}
void Room::SetData(BuildingPrimData* data) {fData = data;}

BuildingPrim* Room::GetBuildingPrim() const 
{
	return fLevel->GetPrimitiveNoAddRef();
}

int32 Room::GetPathPointIndex(VPoint* point) const
{
	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPt=0 ; iPt<pathCount ; iPt++ )
	{
		if(point==fPath[iPt])
			return iPt;
	}
	return -1;
}

int32 Room::GetInPointIndex(VPoint* point) const
{
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		if(point==fPointInArray[iIn])
			return iIn;
	}
#endif
	return -1;
}

void Room::SetWallsShadingDomain(const int32 domain)
{
	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPt=0 ; iPt<pathCount ; iPt++ )
	{
		Wall* wall = fPath[iPt]->GetWall(fPath[(iPt+1)%pathCount]);
		if(wall)
		{
			if( wall->GetLeftRoom() == this )
				wall->SetLeftDomain(domain);
			else
				wall->SetRightDomain(domain);
		}
		else
			MCNotify("No Wall");
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			if(wall)
			{
				wall->SetLeftDomain(domain);
				wall->SetRightDomain(domain);
			}
			else
				MCNotify("No Wall");
		}
	}
#endif
}

int32 Room::GetWallsDomain() const
{
	int32 domain = kNoDomains;

	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPt=0 ; iPt<pathCount ; iPt++ )
	{
		Wall* wall = fPath[iPt]->GetWall(fPath[(iPt+1)%pathCount]);
		if(wall)
		{
			const int32 wallDomain = (wall->GetLeftRoom()==this?wall->GetLeftDomain():wall->GetRightDomain());
			if(domain == kNoDomains)
				domain = wallDomain;
			else if(domain != wallDomain)
				return kMultipleDomains;
		}
		else
			MCNotify("No Wall");
	}
#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		const int32 wallInCount = fPointInArray[iIn]->GetWallCount();
		for(int32 i=0 ; i<wallInCount ; i++)
		{
			Wall* wall = fPointInArray[iIn]->GetWall(i);
			if(wall)
			{
				const int32 wallDomain0 = wall->GetLeftDomain();
				if(domain == kNoDomains)
					domain = wallDomain0;
				else if(domain != wallDomain0)
					return kMultipleDomains;
				const int32 wallDomain1 = wall->GetRightDomain();
				if(domain == kNoDomains)
					domain = wallDomain1;
				else if(domain != wallDomain1)
					return kMultipleDomains;
			}
			else
				MCNotify("No Wall");
		}
	}
#endif

	return domain;
}

int32 AdjustVtxIndex(int32 index,const int32 pathCount,const int32 ceilingOffset,const int32 outOffset)
{
	index-=ceilingOffset;
	if(index<pathCount)
		return index+outOffset;
	else
	{	// a small adjustment here: we added vertices 5 by 5 for the inside, 
		// but only 1 by 1 for the outside
		index-=pathCount;
		index/=5;
		return index+pathCount+outOffset;
	}
}

inline void Swap(uint32& value0, uint32& value1)
{
	const uint32 tmp = value0;
	value0 = value1;
	value1 = tmp;
}

void Room::ValidateTessellation()
{
	if( Flag(eRoomTessellated) ) 
		return; // Tessellation is valid

	SetFlag(eRoomTessellated);

	// 1. Clear previous data
	fTriangleArray.ArrayFree();
	fVertexArray.ArrayFree();

	// 2. Build the flat tesselation

	// Build the contour
	FlatPolygon contour;
	contour.mIsHole = false;

	// points arround
	const int32 wallCount = fPath.GetElemCount();
	contour.SetElemSpace(4); // preallocate some space
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		const int32 nextIndex = (iWall+1)%wallCount;
		Wall* curWall = GetPathWall(iWall);
		Wall* nextWall = GetPathWall(nextIndex);

		int32 curRoomCount = curWall->GetRoomCount();
		int32 nextRoomCount = nextWall->GetRoomCount();

		bool twoRooms = (curRoomCount==2);

		// Check the wall orientation
		const bool flip = (curWall->GetPointIndex( fPath[iWall] ) == 1 );

		// In some cases, need to add the last point too, to join the parts
		bool addAllPos = false;
		if( curRoomCount!=nextRoomCount )
		{	
			if(curWall->GetThickness()>0)
				addAllPos = true;
		}
		else if(curRoomCount==1) // mean that both has 1 room
		{

			if( !flip && fPath[nextIndex]->GetWallCount()>2 )
				addAllPos = true;

			if( flip && fPath[iWall]->GetWallCount()>2 )
				addAllPos = true;
		}

		if(!twoRooms)
		{
			const bool left = (curWall->GetLeftRoom() == this );
			// Copy the points
			const TMCClassArray<TVector2>& pos = (left?curWall->GetRightPosProjection():curWall->GetLeftPosProjection());

			const int32 posCount = pos.GetElemCount();
			if(flip)
			{
				int32 lastPoint = addAllPos?-1:0;
				for(int32 iPos=posCount-1 ; iPos>lastPoint ; iPos-- ) // Do not add the last point, we use the 1st point of the next wall
				{
					contour.AddElem(pos[iPos]);
				}
			}
			else
			{
				int32 lastPoint = addAllPos?posCount:posCount-1;
				for(int32 iPos=0 ; iPos<lastPoint ; iPos++ ) // Do not add the last point, we use the 1st point of the next wall
				{
					contour.AddElem(pos[iPos]);
				}
			}
		}
		else
		{
			// Copy the points
			const TMCPtrArray<ConstrPoint>&	constrPoint = curWall->GetWallConstructionPoints();
			const int32 posCount = constrPoint.GetElemCount();
			if(flip)
			{
				int32 lastPoint = addAllPos?-1:0;
				for(int32 iPos=posCount-1 ; iPos>lastPoint ; iPos-- ) // Do not add the last point, we use the 1st point of the next wall
				{
					contour.AddElem(constrPoint[iPos]->Position());
				}
			}
			else
			{
				int32 lastPoint = addAllPos?posCount:posCount-1;
				for(int32 iPos=0 ; iPos<lastPoint ; iPos++ ) // Do not add the last point, we use the 1st point of the next wall
				{
					contour.AddElem(constrPoint[iPos]->Position());
				}
			}
		}
	}

	// With the contour, build the flat tessellation 
	// (without any hole)
	BuildFlatTessellation(contour);

	// Four horizontal parts in the tessellation:
	// - the floor
	// - under the floor if its the 1st floor or therer's nothing under
	// - the ceiling if the ceiling is On
	// - over the ceiling if it's the last floor or there's nothing over

	TessellatFloor(contour);

	fOffset = fVertexArray.GetElemCount();
	fTgleOffset = fTriangleArray.GetElemCount();

	TessellatCeiling(contour);
		
	// Then use the roofs to cut the upper part of the room
	fLevel->LevelPlan().CutRoomWithRoofs(this);
}

int32 OffsetIndex(int32 index, int32 tglCount, boolean thick)
{
	if(thick)
		return tglCount + 2*index;
	else
		return tglCount + index;
}

void Room::TessellatCeiling(const FlatPolygon& contour)
{
	if(NoCeiling())
		return;

	BooleanPolygon booleanPoly;

	// Add the contour
	booleanPoly.AddPolygon( contour );

	// Add the holes
	const int32 objectCount = fSubObjects.GetElemCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		const TMCCountedPtrArray<OutlinePoint>& outline = fSubObjects[iObj]->GetOutline();
		const int32 holePointCount = outline.GetElemCount();
		FlatPolygon hole;
		hole.SetElemCount(holePointCount);
		for( int32 iHolePt=0 ; iHolePt<holePointCount ; iHolePt++ )
		{
			hole[iHolePt] = outline[iHolePt]->Position();
		}
		hole.mIsHole = true;
		booleanPoly.AddPolygon( hole );
	}
	
	// Get the polygons and the tesselation
	FlatMesh	flatTessellation;
	PolygonSet	polygonSetResult;
	booleanPoly.GetPolygons(polygonSetResult, true, flatTessellation);

	// Build the 3D facet mesh
	const real32 ceilingThickness = GetCeilingThickness();
	const real32 altitude = fLevel->GetDistanceToGround();
	const real32 zOut = fLevel->GetDistanceToGround() + GetRoomHeight();
	const real32 zIn = zOut - ceilingThickness;
	const boolean hasCeilingThickness = ceilingThickness>kRealEpsilon;

	const int32 prevTglCount = fTriangleArray.GetElemCount();
	const int32 prevVtxCount = fVertexArray.GetElemCount();

	const int32 vtxCount = flatTessellation.mVertices.GetElemCount();
	const int32 tglCount = flatTessellation.mTriangles.GetElemCount();
	
	// Preallocate some space
	const float factor = hasCeilingThickness?2:1;
	fTriangleArray.SetElemSpace( prevTglCount + factor*tglCount );
	fVertexArray.SetElemSpace( prevVtxCount + factor*tglCount  );

	// Add the vtx
	for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		const TVector2& pos2D = flatTessellation.mVertices[iVtx];
		TVector3 pos3D(pos2D.x, pos2D.y, zOut);
		
		Vertex newVertex0( pos3D, TVector3::kUnitZ, ComputeUV(pos2D, false)); // , eFloor) );

		fVertexArray.AddElem(newVertex0);
		
		if(hasCeilingThickness)
		{	// This part is inside the room
			pos3D.z = zIn;		
	
			Vertex newVertex1( pos3D, -TVector3::kUnitZ, ComputeUV(pos2D, true)); // , eCeiling) );

			fVertexArray.AddElem(newVertex1);
		}
	}

	// Add the triangles
	for(int32 iTgl=0 ; iTgl<tglCount ; iTgl++)
	{
		// Offset the indexes
		const Triangle& tgl = flatTessellation.mTriangles[iTgl];

		Triangle newTgl;
		// Flip the points to get the normal up
		newTgl.pt1 = OffsetIndex(tgl.pt3, prevVtxCount, hasCeilingThickness);
		newTgl.pt2 = OffsetIndex(tgl.pt2, prevVtxCount, hasCeilingThickness);
		newTgl.pt3 = OffsetIndex(tgl.pt1, prevVtxCount, hasCeilingThickness);

		fTriangleArray.AddElem(newTgl);

		if(hasCeilingThickness)
		{	// This part is inside the room
			Triangle otherTgl;
			// Flip again the points to get the normal down
			otherTgl.pt1 = newTgl.pt3 + 1;
			otherTgl.pt2 = newTgl.pt2 + 1;
			otherTgl.pt3 = newTgl.pt1 + 1;
			fTriangleArray.AddElem(otherTgl);
		}
	}

	if(hasCeilingThickness)
	{	// add the vertical parts between the 2 layers
		
		if( mUnfoldUVData.mPrevRoom )
			mUnfoldUVData.mCeilingBetweenOffset = mUnfoldUVData.mPrevRoom->UnfoldUVData().mCeilingBetweenOffset;
		else
			mUnfoldUVData.mCeilingBetweenOffset = 0;

		const int32 polygonCount = polygonSetResult.GetElemCount();
		for(int32 iPolygon=0 ; iPolygon<polygonCount ; iPolygon++)
		{
			AddVerticalPolygonToTessellation( polygonSetResult[iPolygon], zIn, zOut, -0.01f, 0, true );
		}
	}
}

void Room::AddVerticalPolygonToTessellation(const FlatPolygon& polygon, float zMin, float zMax, 
											float minOffset, float maxOffset, bool ceiling  )
{
	const int32 vtxCount = polygon.GetElemCount();

	if(vtxCount<3)
		return;

	real32& uvOffset = ceiling?mUnfoldUVData.mCeilingBetweenOffset:mUnfoldUVData.mFloorBetweenOffset;

	const float thickness = zMax-zMin;

	int32 indexForTgl = fVertexArray.GetElemCount();

	TVector2 prevVtx = polygon[vtxCount-3];
	TVector2 cur0Vtx = polygon[vtxCount-2];
	TVector2 cur1Vtx = polygon[vtxCount-1];

	// Smoothed normal, to slightly offset the points
	TVector2 prevDir2D = prevVtx-cur0Vtx;
	TVector2 prevNormal2D;
	prevNormal2D.SetValues(prevDir2D.y, -prevDir2D.x); // It looks like the polygon are always oriented in the same way => the normal is always this one
	prevNormal2D.Normalize();

	TVector2 curDir2D = cur0Vtx-cur1Vtx;
	TVector2 curNormal2D;
	curNormal2D.SetValues(curDir2D.y, -curDir2D.x);
	curNormal2D.Normalize();

	const bool hasMinOffset = (minOffset!=0);
	const bool hasMaxOffset = (maxOffset!=0);

	real32 prevLength = 0;

	for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		const TVector2& nextVtx = polygon[iVtx];

		// Add 4 vertices to build 2 triangles

		// Compute the common normal
		const TVector2 nextDir2D = cur1Vtx-nextVtx;
		real32 curLength = nextDir2D.GetMagnitude();
		TVector2 nextNormal2D;
		nextNormal2D.SetValues(nextDir2D.y, -nextDir2D.x);
		nextNormal2D /= curLength; // .Normalize();

		// TODO : smooth

		TVector3 normal3D(curNormal2D.x, curNormal2D.y, 0);
	
		TVector2 cur0UV;
		TVector2 cur1UV;
		if( fData->UVData().mMethod == eProportional )
		{
			cur0UV = fData->UVData().ComputeUV(cur0Vtx); // , eLevelBetween);		
			cur1UV = fData->UVData().ComputeUV(cur1Vtx); // , eLevelBetween);
		}
		else
		{
			cur0UV = fData->UVData().ComputeUV( TVector2(
					-(prevLength + uvOffset), 
					zMin ));
			cur1UV = fData->UVData().ComputeUV( TVector2(
					-(prevLength + curLength + uvOffset), 
					zMin ));

			uvOffset += prevLength;
			prevLength = curLength;
		}

		TVector3 cur0Pos3D(cur0Vtx.x, cur0Vtx.y, zMin);
		TVector3 cur1Pos3D(cur1Vtx.x, cur1Vtx.y, zMin);

		Vertex newVertex0( cur0Pos3D, normal3D, cur0UV );
		Vertex newVertex1( cur1Pos3D, normal3D, cur1UV );

		if(hasMinOffset)
		{	// Move the 2 points along the smoothed normal
			const TVector2 cur0Smooth = 0.5*(prevNormal2D+curNormal2D);
			newVertex0.XValue() += minOffset*cur0Smooth[0];
			newVertex0.YValue() += minOffset*cur0Smooth[1];
		
			const TVector2 cur1Smooth = 0.5*(curNormal2D+nextNormal2D);
			newVertex1.XValue() += minOffset*cur1Smooth[0];
			newVertex1.YValue() += minOffset*cur1Smooth[1];
		}

		fVertexArray.AddElem(newVertex0);
		fVertexArray.AddElem(newVertex1);

		// Offset pos
		cur0Pos3D.z = zMax;
		cur1Pos3D.z = zMax;
	
		// Offset UV
		if( fData->UVData().mMethod == eProportional )
		{
			cur0UV.x += thickness/fData->UVData().mDefaultUVLength;
			cur1UV.x += thickness/fData->UVData().mDefaultUVLength;
		}
		else
		{
			cur0UV.y = zMax/fData->UVData().mDefaultUVLength;
			cur1UV.y = zMax/fData->UVData().mDefaultUVLength;
		}

		Vertex newVertex2( cur0Pos3D, normal3D, cur0UV );
		Vertex newVertex3( cur1Pos3D, normal3D, cur1UV );

		if(hasMaxOffset)
		{	// Move the 2 points along the smoothed normal
			const TVector2 cur0Smooth = 0.5*(prevNormal2D+curNormal2D);
			newVertex2.XValue() += maxOffset*cur0Smooth[0];
			newVertex2.YValue() += maxOffset*cur0Smooth[1];
		
			const TVector2 cur1Smooth = 0.5*(curNormal2D+nextNormal2D);
			newVertex3.XValue() += maxOffset*cur1Smooth[0];
			newVertex3.YValue() += maxOffset*cur1Smooth[1];
		}

		fVertexArray.AddElem(newVertex2);
		fVertexArray.AddElem(newVertex3);

		// Create the triangles
		int32 index0 = indexForTgl++;
		int32 index1 = indexForTgl++;
		int32 index2 = indexForTgl++;
		int32 index3 = indexForTgl++;

		Triangle newTgl0(index0, index1, index3);
		fTriangleArray.AddElem(newTgl0);
		Triangle newTgl1(index3, index2, index0);
		fTriangleArray.AddElem(newTgl1);

		// Move to the next vertex
		prevVtx = cur0Vtx;
		cur0Vtx = cur1Vtx;
		cur1Vtx = nextVtx;

		prevNormal2D = curNormal2D;
		curNormal2D = nextNormal2D;
	}
}

TVector2 Room::ComputeUV(const TVector2& pos, bool ceiling)
{
	switch( fData->UVData().mMethod )
	{
	default:
	case eProportional: return fData->UVData().ComputeUV( pos );
	case eUnfold:
		{
			TBBox3D levelBBox;
			fLevel->GetBoundingBox( levelBBox, true, false );
			TVector3 bboxCenter;
			levelBBox.GetCenter( bboxCenter );
			if( ceiling )
			{
				return fData->UVData().ComputeUV( TVector2(
					pos.x - bboxCenter.x + 1.5*levelBBox.GetWidth(), 
					pos.y - bboxCenter.y + 0.5*levelBBox.GetHeight() + mUnfoldUVData.mOffset ) );
			}
			else
			{
				return fData->UVData().ComputeUV( TVector2(
					pos.x - bboxCenter.x + 0.5*levelBBox.GetWidth(), 
					pos.y - bboxCenter.y + 0.5*levelBBox.GetHeight() + mUnfoldUVData.mOffset ) ) ;
			}
		}
	}
}

void Room::TessellatFloor(const FlatPolygon& contour)
{
	BooleanPolygon booleanPoly;

	// Add the contour
	booleanPoly.AddPolygon( contour );

	// Add the holes
	// For the floor, look in the level under
	Level* levelUnder = fLevel->GetLevelUnder();
	if(levelUnder)
	{	// Parse the objects of the level under and get the ones
		// that make a hole in this one
		TMCCountedPtrArray<RoomSubObject> objectsUnder;
		levelUnder->LevelPlan().GetRoomObjectList(objectsUnder);
		int32 underCount = objectsUnder.GetElemCount();
		for(int32 iUnd=0 ; iUnd<underCount ; iUnd++)
		{
			RoomSubObject* objUnder = objectsUnder[iUnd];
			const TVector2& objCenter = objUnder->GetPolylineCenter();
			if(PointIn(objCenter, false))
			{	// This object is in this room: make a hole for it
				const TMCCountedPtrArray<OutlinePoint>& outline = objUnder->GetOutline();
				const int32 holePointCount = outline.GetElemCount();
				FlatPolygon hole;
				hole.SetElemCount(holePointCount);
				for( int32 iHolePt=0 ; iHolePt<holePointCount ; iHolePt++ )
				{
					hole[iHolePt] = outline[iHolePt]->Position();
				}
				hole.mIsHole = true;
				booleanPoly.AddPolygon( hole );
			}
		}
	}
	
	// Get the polygons and the tesselation
	FlatMesh	flatTessellation;
	PolygonSet	polygonSetResult;
	booleanPoly.GetPolygons(polygonSetResult, true, flatTessellation);

	// Build the 3D facet mesh
	const real32 floorThickness = GetFloorThickness();
	const real32 zOut = fLevel->GetDistanceToGround();
	const real32 zIn = zOut + floorThickness;
	const boolean hasFloorThickness = floorThickness>kRealEpsilon;

	const int32 prevTglCount = fTriangleArray.GetElemCount();
	const int32 prevVtxCount = fVertexArray.GetElemCount();

	const int32 vtxCount = flatTessellation.mVertices.GetElemCount();
	const int32 tglCount = flatTessellation.mTriangles.GetElemCount();
	
	// Preallocate some space
	const float factor = floorThickness?2:1;
	fTriangleArray.SetElemSpace( prevTglCount + factor*tglCount );
	fVertexArray.SetElemSpace( prevVtxCount + factor*tglCount  );

	// Add the vtx
	for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		const TVector2& pos2D = flatTessellation.mVertices[iVtx];
		TVector3 pos3D(pos2D.x, pos2D.y, zOut);
		
		Vertex newVertex0( pos3D, -TVector3::kUnitZ, ComputeUV(pos2D, true)); // , eCeiling) );

		fVertexArray.AddElem(newVertex0);
		
		if(hasFloorThickness)
		{	// Triangle inside the room
			pos3D.z = zIn;		
	
			Vertex newVertex1( pos3D, TVector3::kUnitZ, ComputeUV(pos2D, false)); // , eFloor) );

			fVertexArray.AddElem(newVertex1);
		}
	}

	// Add the triangles
	for(int32 iTgl=0 ; iTgl<tglCount ; iTgl++)
	{
		// Offset the indexes
		const Triangle& tgl = flatTessellation.mTriangles[iTgl];

		Triangle newTgl;
		newTgl.pt1 = OffsetIndex(tgl.pt1, prevVtxCount, hasFloorThickness);
		newTgl.pt2 = OffsetIndex(tgl.pt2, prevVtxCount, hasFloorThickness);
		newTgl.pt3 = OffsetIndex(tgl.pt3, prevVtxCount, hasFloorThickness);

		fTriangleArray.AddElem(newTgl);

		if(hasFloorThickness)
		{	// Triangle inside the room
			Triangle otherTgl;
			// Flip the points to get the normal up
			otherTgl.pt1 = newTgl.pt3 + 1;
			otherTgl.pt2 = newTgl.pt2 + 1;
			otherTgl.pt3 = newTgl.pt1 + 1;
			fTriangleArray.AddElem(otherTgl);
		}
	}

	if(hasFloorThickness)
	{	// add the vertical parts between the 2 layers

		if( mUnfoldUVData.mPrevRoom )
			mUnfoldUVData.mFloorBetweenOffset = mUnfoldUVData.mPrevRoom->UnfoldUVData().mFloorBetweenOffset;
		else
			mUnfoldUVData.mFloorBetweenOffset = 0;

		const int32 polygonCount = polygonSetResult.GetElemCount();
		for(int32 iPolygon=0 ; iPolygon<polygonCount ; iPolygon++)
		{
			AddVerticalPolygonToTessellation( polygonSetResult[iPolygon], zOut, zIn, 0, -0.01f, false);
		}
	}
}


bool Room::GetRoomFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags)
{
	const boolean is2DMesh = FLAG(meshFlags,e2DMesh);
	const boolean noTop = FLAG(meshFlags,eNoTop)||is2DMesh; // don't show the ceiling when in NoTop mode

	FacetMesh::Create(outMesh);

	TMCCountedPtr<FacetMesh> facetMesh;
	facetMesh = *outMesh;

	TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = noTop?fOffset:vertices.GetElemCount();

	TMCClassArray<Triangle>& facets = Triangles();
	const int32 facetCount = noTop?fTgleOffset:facets.GetElemCount();

	if(facetCount==0)
	{
		return false;
	}

	const boolean needcolor = FLAG(meshFlags,eShellMesh)?false:true;

	TMCArray<TVector3>& meshVertices = facetMesh->fVertices;
	TMCArray<TVector2>& meshUVs = facetMesh->fuv;
	TMCArray<TVector3>& meshNormals = facetMesh->fNormals;
	TMCArray<Triangle>& meshFacets = facetMesh->fFacets;
	TMCArray<uint32>& meshUVSpaceIDs = facetMesh->fUVSpaceID;
	TMCArray<TMCColorRGBA8>& meshColors = facetMesh->fPolygonColors;

	meshVertices.SetElemCount(vertexCount); 
	meshUVs.SetElemCount(vertexCount); 
	meshNormals.SetElemCount(vertexCount); 
	meshFacets.SetElemCount(facetCount);   
	meshUVSpaceIDs.SetElemCount(facetCount);   
	if(needcolor)
		meshColors.SetElemCount(facetCount);

//	facetMesh->fPolygonBackColors.SetElemCount(facetCount);

	const TMCColorRGBA8 color = (Selected()?fData->fSelCol:(Targeted()?fData->fTarCol:is2DMesh?fData->fFloCol:fData->fDefCol));

	const boolean faceted = FLAG(meshFlags, eFaceted);

	// Fill the vertices positions
	for( int32 iVertex=0 ; iVertex<vertexCount ; iVertex++ )
	{
		meshVertices[iVertex] = vertices[iVertex].Position();
		meshUVs[iVertex] = vertices[iVertex].UV();
		meshNormals[iVertex] = vertices[iVertex].Normal();
	}

	// Fill the facets
	for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
	{
		meshFacets[iFacet] = facets[iFacet];
		meshUVSpaceIDs[iFacet] = iFacet<fTgleOffset?fFloorDomain:fCeilingDomain;
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

void Room::BuildFlatTessellation(const FlatPolygon& contour)
{
	BooleanPolygon booleanPoly;

	// Add the contour
	booleanPoly.AddPolygon( contour );

	fFlatMesh.mVertices.ArrayFree();
	fFlatMesh.mTriangles.ArrayFree();
	
	// Get the polygons and the tesselation
	FlatMesh	flatTessellation;
	PolygonSet	polygonSetResult;
	booleanPoly.GetPolygons(polygonSetResult, true, fFlatMesh);
}

boolean Room::PointIn(const TVector2& point, boolean exact)
{
	if(PointIsInTriangleArray(point,FlatTriangles(),FlatVertices()))
		return true;

	return false;
}

boolean AllPointsIn(Room* room, const TVector2& offset,const TMCCountedPtrArray<OutlinePoint>& objPath)
{
	const int32 objPathCount = objPath.GetElemCount();
	for(int32 iPt=0 ; iPt<objPathCount ; iPt++)
	{
		TVector2 newPoint = objPath[iPt]->Position()+offset;
		if(!room->PointIn(newPoint, true)) // true: use the exact tessellation
			return false;
	}
	return true;
}

void Room::SetFloorThickness(const real32 t)
{
	if(t==fFloorThickness)
		return;
	if(t==fData->GetDefaultFloorThickness() )
	{
		if(fFloorThickness==kDefaultThickness)
			return;

		fFloorThickness=kDefaultThickness;
	}
	else
	{
		// Keep the floor under the ceiling
		if(t<GetRoomHeight() - GetCeilingThickness())
		{
			fFloorThickness=t;
		}
		else
			return;
	}
	
	// Invalidation
	InvalidateTessellation(true);
	// Replace the objects of the walls if needed
	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pathCount ; iPoint++ )
	{
		Wall* wall = GetPathWall(iPoint);
		wall->CheckWallObjects();
	}
#ifdef USE_POINT_IN
	// Check also the ones inside
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		VPoint* curPoint = fPointInArray[iIn];
		const int32 wallCount = curPoint->GetWallCount();
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = curPoint->GetWall(iWall);
			wall->CheckWallObjects();
		}
	}	
#endif
}

void Room::SetCeilingThickness(const real32 t)
{
	if(t==fCeilingThickness)
		return;
	if(t==fData->GetDefaultCeilingThickness() )
	{
		if(fCeilingThickness==kDefaultThickness)
			return;

		fCeilingThickness=kDefaultThickness;
	}
	else
	{
		// Keep the floor under the ceiling
		if(t<GetRoomHeight() - GetFloorThickness())
		{
			fCeilingThickness=t;
		}
	}
	
	// Invalidation
	InvalidateTessellation(true);
	// Replace the objects of the walls if needed
	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pathCount ; iPoint++ )
	{
		Wall* wall = GetPathWall(iPoint);
		wall->CheckWallObjects();
	}
#ifdef USE_POINT_IN
	// Check also the ones inside
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		VPoint* curPoint = fPointInArray[iIn];
		const int32 wallCount = curPoint->GetWallCount();
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = curPoint->GetWall(iWall);
			wall->CheckWallObjects();
		}
	}	
#endif
}

// Parse the wall arround and use the highest one
real32 Room::GetRoomHeight() const
{
	real32 roomHeight = 0;

	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pathCount ; iPoint++ )
	{
		const Wall* wall = GetPathWall(iPoint);
		MY_ASSERT(wall);
		const real32 wallHeight = wall->GetWallHeight();
		if(wallHeight>roomHeight)
			roomHeight = wallHeight;
	}
	return roomHeight; 
}


boolean Room::OffsetThisObject(RoomSubObject* object, const TVector2& offset)
{	// Move this obj with caution: check that we stay in the room
	// and that we don't overlap other objects. Return false if we couldn't
	// find any space for it.

	// Check the corners of our object to know if they're in the room

	const TMCCountedPtrArray<OutlinePoint>&	objPath = object->GetOutline();

	const int32 objPathCount = objPath.GetElemCount();
	MY_ASSERT(objPathCount);

	// 1: Check the n corners to know if they're in the room
	TMCArray<TVector2> corrections;
	for(int32 i=0 ; i<objPathCount ; i++)
	{
		const TVector2 newPoint = objPath[i]->Position()+offset;
		if(!PointIn(newPoint, true)) // true: use the exact tessellation
		{	// The point is going to be outside => modify the offset so it doesn't happen
			// Find the nearest pos inside the room to bring back the point to it 
			TVector2 bestPoint;
			GetBestPointIn(bestPoint,newPoint);
			corrections.AddElem( (bestPoint-newPoint) );
		}
	}

	// 2: TO DO: if the correction is too big, just let it go

	// Now move the points: 
	// if only one out, just move them all
	// if more, try first them all, then their sum, then by dichotomie x*sum
	const int32 outCount = corrections.GetElemCount();
	boolean OK=true;
	TVector2 offsetSolution = offset;
	for( int32 iOut=0 ; iOut<outCount ; iOut++ )
	{
		const TVector2 newOffset = offset+corrections[iOut];
		if(AllPointsIn(this,newOffset, objPath))
		{
			OK=true;
			offsetSolution = newOffset;
			break;
		}
		else
		{
			OK=false;
		}
	}

	if(!OK)
	{	// This is not fantastic but it works most of the time
		const real32 security = 1.01f;
		offsetSolution = offset+security*corrections[0];
		int32 index=0;
		while(!AllPointsIn(this,offsetSolution, objPath))
		{
			for(int32 i=0 ; i<objPathCount ; i++)
			{
				const TVector2 newPoint = objPath[(index+i)%objPathCount]->Position()+offsetSolution;
				if(!PointIn(newPoint, true)) // true: use the exact tessellation
				{	// The point is going to be outside => modify the offset so it doesn't happen
					// Find the nearest pos inside the room to bring back the point to it 
					TVector2 bestPoint;
					GetBestPointIn(bestPoint,newPoint);
					offsetSolution+=security*(bestPoint-newPoint);
				}
			}
			index++;

			if(index>100)
				return false;
		}
	}

	// 2: Offset first the object now to be able to use the collide method
	object->OffsetPolyline(offsetSolution, false);

	// 3: Check the position relative to other object: intersection and pointIn
	// Warning: this code doesn't work after the object has been rotated.
	// Must be modified

	const real32 securityMargin = .0001f;
	const TVector2 security(securityMargin,securityMargin);

	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		RoomSubObject* otherObject = fSubObjects[iObj];

			if(object!=otherObject)
			{
				int32 securityCount = 0;
				TVector2 problem;
				ECollisionType type = eUnknownCollision;
				while(type!=eNoCollision)
				{
					type = object->Collide(otherObject,problem);
					// If there's a conflict with another object, we need to adjust the offset
					if(type==ePointHereInOtherObject)
					{	// Bring back this point outside
						// Find a x offset value and a y offset value and use the smallest one
						const TVector2& center = object->GetPolylineCenter();
						TVector2 minCorner;
						otherObject->MinCorner(minCorner);
						minCorner-=security;
						TVector2 maxCorner;
						otherObject->MaxCorner(maxCorner);
						maxCorner+=security;

						real32 xOffset=0;
						if(problem.x>=center.x)
						{	// Negative offset
							xOffset = minCorner.x-problem.x;
						}
						else
						{	// Positive offset
							xOffset = maxCorner.x-problem.x;
						}

						real32 yOffset=0;
						if(problem.y>=center.y)
						{	// Negative offset
							yOffset = minCorner.y-problem.y;
						}
						else
						{	// Positive offset
							yOffset = maxCorner.y-problem.y;
						}

						TVector2 offset=TVector2::kZero;
						if(RealAbs(xOffset)<RealAbs(yOffset))
							offset.x=xOffset;
						else					
							offset.y=yOffset;

						object->OffsetPolyline(offset, false);
					}
					else if(type==ePointOtherInHereObject)
					{
						const TVector2& otherCenter = otherObject->GetPolylineCenter();
						TVector2 minCorner;
						object->MinCorner(minCorner);
						minCorner-=security;
						TVector2 maxCorner;
						object->MaxCorner(maxCorner);
						maxCorner+=security;

						real32 xOffset=0;
						if(problem.x<=otherCenter.x)
						{	// Negative offset
							xOffset = problem.x-maxCorner.x;
						}
						else
						{	// Positive offset
							xOffset = problem.x-minCorner.x;
						}

						real32 yOffset=0;
						if(problem.y<=otherCenter.y)
						{	// Negative offset
							yOffset = problem.y-maxCorner.y;
						}
						else
						{	// Positive offset
							yOffset = problem.y-minCorner.y;
						}

						TVector2 offset=TVector2::kZero;
						if(RealAbs(xOffset)<RealAbs(yOffset))
							offset.x=xOffset;
						else					
							offset.y=yOffset;

						object->OffsetPolyline(offset, false);
					}
					else if(type==eSideIntersection)
					{
						const TVector2& center = object->GetPolylineCenter();
						TVector2 minCorner;
						object->MinCorner(minCorner);
						minCorner-=security;
						TVector2 maxCorner;
						object->MaxCorner(maxCorner);
						maxCorner+=security;

						const TVector2& otherCenter = otherObject->GetPolylineCenter();
						TVector2 otherMinCorner;
						object->MinCorner(otherMinCorner);
						otherMinCorner-=security;
						TVector2 otherMaxCorner;
						object->MaxCorner(otherMaxCorner);
						otherMaxCorner+=security;

						real32 correctionX = 0;
						if(center.x>=otherCenter.x)
						{
							correctionX = MC_Max( problem.x-minCorner.x,  problem.x-otherMinCorner.x );
						}
						else
						{
							correctionX = MC_Min( problem.x-maxCorner.x,  problem.x-otherMaxCorner.x );
						}

						real32 correctionY = 0;
						if(center.y>=otherCenter.y)
						{
							correctionY = MC_Max( problem.y-minCorner.y,  problem.y-otherMinCorner.y );
						}
						else
						{
							correctionY = MC_Min( problem.y-maxCorner.y,  problem.y-otherMaxCorner.y );
						}
						
						TVector2 offset=TVector2::kZero;
						if(RealAbs(correctionX)>RealAbs(correctionY))
							offset.x=correctionX;
						else					
							offset.y=correctionY;

						object->OffsetPolyline(offset, false);
					}

					if( securityCount++ > 20 )
					{
						MCNotify("Collision detection");
						type=eNoCollision; // Get out of the loop
					}
				} 
			}
/*		TVector2 collision;
		if( object!=otherObject && object->Collide(otherObject,collision)!=eNoCollision )
		{
			// TO DO
		}*/
	}
	
	// 3: Recheck everything and return false if not enough space

	// 4: Finaly offset the polyline
//	object->OffsetPolyline(offsetSolution, false);
	return true;
}

boolean Room::ScaleThisObject(RoomSubObject* object, const TVector2& scale)
{
	const TMCCountedPtrArray<OutlinePoint>&	polyline = object->GetOutline();
	const TVector2& center = object->GetPolylineCenter();

	const int32 polyCount = polyline.GetElemCount();
	MY_ASSERT(polyCount);

	// 1: Check the 4 corners to know if they're in the room
	TVector2 newScale = scale;

	{
		for(int32 iPt=0 ; iPt<polyCount ; iPt++)
		{
			const TVector2 scaledPoint = ScalePoint(polyline[iPt]->Position(),newScale,center);
			if(!PointIn(scaledPoint, true)) // true: use the exact tessellation
			{	// The point is going to be outside => modify the scaling so it doesn't happen
				// Find the nearest pos inside the room to bring back the point to it 
				TVector2 bestPoint;
				GetBestPointIn(bestPoint,scaledPoint);
				newScale.x=(bestPoint.x-center.x)/(polyline[iPt]->Position().x-center.x);
				newScale.y=(bestPoint.y-center.y)/(polyline[iPt]->Position().y-center.y);
			}
		}
	}


	// 2: Scale it now so we can use the Collide method
	object->ScalePolyline(newScale, false);

	// 3: object size control: 
	// Warning: this code doesn't work after the object has been rotated.
	// Must be modified
	const real32 securityMargin = .0001f;
	const TVector2 security(securityMargin,securityMargin);
	{	// Check that were not in another object
		const int32 objCount = fSubObjects.GetElemCount();

		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			RoomSubObject* otherObject = fSubObjects[iObj];
			
			if(object!=otherObject)
			{
				TVector2 collision;
				ECollisionType type = eUnknownCollision;
				int32 securityCount=0;
				while(type!=eNoCollision)
				{
					type = object->Collide(otherObject,collision);
					// If there's a conflict with another object, we need to adjust the scaling
					if(type==ePointHereInOtherObject)
					{	// Bring back this point outside
						// Find a x scaling value and a y scaling value and use the smallest one
						TVector2 minCorner;
						otherObject->MinCorner(minCorner);
						minCorner-=security;
						TVector2 otherCorner;
						otherObject->MaxCorner(otherCorner);
						otherCorner+=security;
						real32 xScale=0;
						real32 yScale=0;
						if(collision.x>=center.x)
						{
							otherCorner.x=minCorner.x;
							if(otherCorner.x>=center.x)
								xScale = 1-(collision.x-otherCorner.x)/(collision.x-center.x);
						}
						else if(otherCorner.x<=center.x)
								xScale = 1-(collision.x-otherCorner.x)/(collision.x-center.x);

						if(collision.y>=center.y)
						{
							otherCorner.y=minCorner.y;
							if(otherCorner.y>=center.y)
								yScale = 1-(collision.y-otherCorner.y)/(collision.y-center.y);
						}
						else if(otherCorner.y<=center.y)
								yScale = 1-(collision.y-otherCorner.y)/(collision.y-center.y);

						TVector2 rescale=TVector2::kOnes;
						if(xScale>yScale)		rescale.x=xScale;
						else if(yScale>xScale)	rescale.y=yScale;
						else {rescale.x=.99f;rescale.y=.99f;} // A security
						object->ScalePolyline(rescale, false);
					}
					else if(type==ePointOtherInHereObject)
					{
						TVector2 minCorner;
						object->MinCorner(minCorner);
						minCorner-=security;
						TVector2 corner;
						object->MaxCorner(corner);
						corner+=security;
						real32 xScale=0;
						real32 yScale=0;
						if(collision.x<=center.x)
						{
							corner.x=minCorner.x;
							if(corner.x<=center.x)
								xScale = 1-(corner.x-collision.x)/(corner.x-center.x);
						}
						else if(corner.x>=center.x)
								xScale = 1-(corner.x-collision.x)/(corner.x-center.x);

						if(collision.y<=center.y)
						{
							corner.y=minCorner.y;
							if(corner.y<=center.y)
								yScale = 1-(corner.y-collision.y)/(corner.y-center.y);
						}
						else if(corner.y>=center.y)
								yScale = 1-(corner.y-collision.y)/(corner.y-center.y);

						TVector2 rescale=TVector2::kOnes;
						if(xScale>yScale)		rescale.x=xScale;
						else if(yScale>xScale)	rescale.y=yScale;
						else {rescale.x=.99f;rescale.y=.99f;} // A security
						object->ScalePolyline(rescale, false);
					}
					else if(type==eSideIntersection)
					{
						TVector2 minCorner;
						object->MinCorner(minCorner);
						minCorner-=security;
						TVector2 corner;
						object->MaxCorner(corner);
						corner+=security;
						real32 correctionX = securityMargin;
						if(collision.x<=center.x)
						{
							corner.x=minCorner.x;
							correctionX=-securityMargin;
						}

						real32 correctionY = securityMargin;
						if(collision.y<=center.y)
						{
							corner.y=minCorner.y;
							correctionY=-securityMargin;
						}
						
						// One of these 2 value is 1:
						real32 xScale = 1-(corner.x-collision.x+correctionX)/(corner.x-center.x);
						real32 yScale = 1-(corner.y-collision.y+correctionY)/(corner.y-center.y);
						if(xScale>=kNearlyOne && yScale>=kNearlyOne) {xScale=.99f;yScale=.99f;} // A security
						TVector2 rescale(xScale,yScale);
						object->ScalePolyline(rescale, false);

					}

					if( securityCount++ > 20 )
					{
						MCNotify("Collision detection");
						type=eNoCollision; // Get out of the loop
					}
				} 
			}
		}
	}
	return true;
}

void Room::GetBestPointIn(TVector2& bestPoint, const TVector2& pos)
{
	// Get the actual positions of the edges points
	const TMCClassArray<TVector2>& positions = FlatVertices();

	const int32 posCount = positions.GetElemCount();
	
	real32 alpha=0;
	real32 minAlpha=kBigRealValue;
	bestPoint=TVector2::kZero;
	for( int32 iPos=0 ; iPos<posCount ; iPos++ )
	{
		// Compute the distance from the point to the wall segment
		const TVector2 p0 = positions[iPos];
		const TVector2 p1 = positions[(iPos+1)%posCount];
		const TVector2 p0p1 = p1-p0;
		const TVector2 posp0 = p0-pos;
		const TVector2 posp1 = p1-pos;

		TVector2 point=TVector2::kZero;
		if( p0p1*posp0>=0 )
		{
			alpha = posp0.GetSquaredNorm();
			point = p0;
		}
		else if( p0p1*posp1<=0 )
		{
			alpha = posp1.GetSquaredNorm();
			point = p1;
		}
		else
		{	// Distance Point-Line
			const real32 value = RealAbs(p0p1^posp0);
			alpha = value*value/(p0p1.GetSquaredNorm());
			Project(pos,p0,p1,point);
		}
		if( alpha<minAlpha )
		{
			minAlpha = alpha;
			bestPoint = point;
		}
	}
}

void Room::InsertPointBetween(VPoint* insert, VPoint* point0, VPoint* point1)
{
	const int32 pointCount = fPath.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* curPoint = fPath[iPoint];
		if( curPoint==point0 )
		{	// Check if the other is the next or the previous one
			const int32 next = (iPoint+1)%pointCount;
			const int32 prev = iPoint>0?iPoint-1:pointCount-1;
			if(fPath[next]==point1)
				fPath.InsertElem(next,insert);
			else if(fPath[prev]==point1)
				fPath.InsertElem(iPoint,insert);
			else
				MCNotify("Point not found");
			
			return;
		}
		else if( curPoint==point1 )
		{	// Check if the other is the next or the previous one
			const int32 next = (iPoint+1)%pointCount;
			const int32 prev = iPoint>0?iPoint-1:pointCount-1;
			if(fPath[next]==point0)
				fPath.InsertElem(next,insert);
			else if(fPath[prev]==point0)
				fPath.InsertElem(iPoint,insert);
			else
				MCNotify("Point not found");
			
			return;
		}
	}

#ifdef USE_POINT_IN
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		VPoint* curPoint = fPointInArray[iIn];
		if( curPoint==point0 )
		{	// Check if the other is the next or the previous one
			const int32 next = (iIn+1)%inCount;
			const int32 prev = iIn>0?iIn-1:inCount-1;
			if(fPath[next]==point1)
				fPath.InsertElem(next,insert);
			else if(fPath[prev]==point1)
				fPath.InsertElem(iPoint,insert);
			else
				MCNotify("Point not found");
		}
		else if( curPoint==point1 )
		{	// Check if the other is the next or the previous one
			const int32 next = (iIn+1)%inCount;
			const int32 prev = iIn>0?iIn-1:inCount-1;
			if(fPath[next]==point0)
				fPath.InsertElem(next,insert);
			else if(fPath[prev]==point0)
				fPath.InsertElem(iPoint,insert);
			else
				MCNotify("Point not found");
		}
	}
#endif
	
	MCNotify("Point not found");
}

/*
void Room::ValidateObjPositions()
{
	if( Flag(eRoomObjPositionned) ) 
		return; // Tessellation is valid

	SetFlag(eRoomObjPositionned);

	// Set the nearest point on the objects of this room
	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		RoomSubObject* obj = fSubObjects[iObj];
		TVector2 center,pointPos;
		obj->GetPolylineCenter(center);
		obj->SetNearestPoint(FindNearestPoint(center, pointPos));
		obj->SetNearestPointOffset((center-pointPos));
	}
}

Point* Room::FindNearestPoint(const TVector2& pos, TVector2& pointPos) const
{
	real32 minDist = kBigRealValue;
	Point* nearestPoint=NULL;
	const int32 pathCount = fPath.GetElemCount();
	for( int32 iPt=0 ; iPt<pathCount ; iPt++ )
	{
		const TVector2& ptPos = fPath[iPt]->Position().CastToXY();
		const real32 dist = (pos-ptPos).GetSquaredNorm();
		if(dist<minDist)
		{
			minDist = dist;
			nearestPoint = fPath[iPt];
			pointPos = ptPos;
		}
	}

	// Parse also the points inside the room	
	const int32 inCount = fPointInArray.GetElemCount();
	for( int32 iIn=0 ; iIn<inCount ; iIn++ )
	{
		const TVector2& ptPos = fPath[iIn]->Position().CastToXY();
		const real32 dist = (pos-ptPos).GetSquaredNorm();
		if(dist<minDist)
		{
			minDist = dist;
			nearestPoint = fPath[iIn];
			pointPos = ptPos;
		}
	}

	return nearestPoint;
}
*/
// Offset the object when they are selected
void Room::OffsetAttachedObjects(const TVector2& offset)
{
//	ValidateObjPositions();
	
	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		RoomSubObject* obj = fSubObjects[iObj];
		if(obj->Selected())
		{
			obj->OffsetPolyline(offset,false);
		}
	}
}

void Room::CutGeometry(const TMCArray<TriangleVertices>& roofTriangles, const real32 roofMin, const real32 roofMax)
{
	// Use the passed in triangles to remove all the upper part of the wall

	const TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = vertices.GetElemCount();
	TMCClassArray<Vertex>  newVertices;

	const TMCClassArray<Triangle>& triangles = Triangles();
	const int32 triangleCount = triangles.GetElemCount();
	TMCClassArray<Triangle> newTriangles;
	newTriangles.SetElemSpace(triangleCount); // Minimum space used (about)

	// Cut the triangles
	for(int32 iTgle=fTgleOffset ; iTgle<triangleCount ; iTgle++)
	{
		CutTriangle(triangles[iTgle], vertices, fVertexArray.GetElemCount(),
			newTriangles, newVertices, roofTriangles,roofMin,roofMax);
	}

	// Replace the arrays
	fVertexArray.Append(newVertices); // Add the new vertices (we keep the old one because they're used elsewhere)
	fTriangleArray.RemoveElem(fTgleOffset, triangleCount-fTgleOffset); // keep the floor triangles
	fTriangleArray.Append(newTriangles); // Add the new ceiling triangles
}


///////////////////////////////////////////////////////////////////////////
//
//

MCCOMErr Room::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Room');
	if (result) return result;

	// Floor
	result=stream->PutKeyword('FloT');
	if (result) return result;
	result=stream->PutQuickFix(fFloorThickness);
	if (result) return result;

	// Ceiling
	result=stream->PutKeyword('CeiT');
	if (result) return result;
	result=stream->PutQuickFix(fCeilingThickness);
	if (result) return result;

	// Domain
	stream->PutInt32Attribute('FDom', fFloorDomain);
	stream->PutInt32Attribute('CDom', fCeilingDomain);

	// Path
	{
		const int32 pathCount = fPath.GetElemCount();

		stream->PutInt32Attribute('PatC', pathCount);

		TMCArray<int32> indices(pathCount,false);

		// Point indices
		for(int32 iPt=0 ; iPt<pathCount ; iPt++)
		{
			indices[iPt] = fPath[iPt]->GetIndex();
			if(indices[iPt]<0)
			{
				MCNotify("Database corrupted!");
				indices[iPt]=0;
			}
		}
		stream->PutInt32ArrayAttribute('Path', pathCount,(int32*)indices.BaseAddress());
	}

#ifdef USE_POINT_IN
	// Points In
	{
		const int32 inCount = fPointInArray.GetElemCount();

		result=stream->PutKeyword('PtIC');
		if (result) return result;
		result=stream->PutLong(inCount);
		if (result) return result;

		result=stream->PutKeyword('PtIn');
		if (result) return result;

		TMCArray<int32> indices(inCount,false);

		// Point indices
		for(int32 iPt=0 ; iPt<inCount ; iPt++)
		{
			indices[iPt] = fPointInArray[iPt]->GetIndex();
			if(indices[iPt]<0)
			{
				MCNotify("Database corrupted!");
				indices[iPt]=0;
			}
		}
		result=stream->PutLongArray(inCount,(int32*)indices.BaseAddress());
	}
#endif

	// Objects
	const int32 objCount = fSubObjects.GetElemCount();
	for(int32 iObj=0 ; iObj<objCount ; iObj++)
	{
		fSubObjects[iObj]->Write(stream);
	}

	// Common
	result=CommonBase::Write(stream);
	if (result) return result;

	result=stream->PutEnd();
	return result;
}

MCCOMErr Room::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	int32 pathCount=0;
	int32 pointInCount=0;

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
			case 'FloT':
			{
				result = stream->GetQuickFix(&fFloorThickness);
				if (result) return result;
			} break;
			case 'CeiT':
			{
				result = stream->GetQuickFix(&fCeilingThickness);
				if (result) return result;
			} break;
			case 'FDom':
			{
				fFloorDomain = stream->GetInt32Token();
			} break;
			case 'CDom':
			{
				fCeilingDomain = stream->GetInt32Token();
			} break;
			case 'PatC':
			{
				pathCount = stream->GetInt32Token();
			} break;
			case 'Path':
			{
				int32* indices = new int32[pathCount];
				stream->GetInt32ArrayToken(pathCount, indices);

				fPath.SetElemCount(pathCount);
				for(int32 iPt=0 ; iPt<pathCount ; iPt++)
				{
					fPath.SetElem(iPt, fLevel->GetPoint(indices[iPt]));
				}

				delete[] indices;
			} break;
#ifdef USE_POINT_IN
			case 'PtIC':
			{
				result = stream->GetLong(&pointInCount);
				if (result) return result;
			} break;
			case 'PtIn':
			{
				TMCArray<int32> indices(pointInCount,false);

				result = stream->GetLongArray(pointInCount,(int32*)indices.BaseAddress());
				if (result) return result;

				fPointInArray.SetElemCount(pointInCount);
				for(int32 iPt=0 ; iPt<pointInCount ; iPt++)
				{
					fPointInArray.SetElem(iPt, fLevel->GetPoint(indices[iPt]));
				}
			} break;
#endif
			case 'RObj':
			{
				RoomSubObject* newObject = NULL;
				RoomSubObject::CreateRoomSubObject(&newObject, this, GetBuildingPrim());
				newObject->Read(stream);
			} break;
			default:
				CommonBase::Read(stream,keyword,fData);
				break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	// Register the room in the walls
	RegisterInWalls();

	// Invalidate tessellation
	ClearFlag(eRoomTessellated);
	ClearFlag(eRoomBasicTessellated);
	ClearFlag(eIsTargeted);

	return result;
}

///////////////////////////////////////////////////////////////////////////
//
//

void Room::Clone(Room** newRoom, Level* inLevel,const TMCCountedPtrArray<VPoint>& newPoints, const ECloneChildrenMode cloneMode)
{
	// Path
	const int32 pathCount = fPath.GetElemCount();
	TMCCountedPtrArray<VPoint> clonePath;
	clonePath.SetElemCount(pathCount);
	for(int32 iPt=0 ; iPt<pathCount ; iPt++)
	{
		clonePath.SetElem(iPt, newPoints[fPath[iPt]->GetIndex()]);
	}

	Room::CreateRoom(newRoom, inLevel->GetPrimitiveData(), inLevel,
		clonePath,Flag(eTurnLeft));

	Room* roomPtr = *newRoom;

#ifdef USE_POINT_IN
	// Add the point inside this room 
	const int32 inCount = fPointInArray.GetElemCount();
	for(int32 iIn=0 ; iIn<inCount ; iIn++)
	{
		roomPtr->AddPointInReference(newPoints[fPointInArray[iIn]->GetIndex()]);
	}
#endif

	// Clone the flags
	roomPtr->SetFlags(fFlags);

	roomPtr->SetNamePtr(fName);

	roomPtr->fFloorThickness = fFloorThickness;	// don't use the method, they start computing stuffs roomPtr->SetFloorThickness(fFloorThickness);
	roomPtr->fCeilingThickness = fCeilingThickness;// don't use the method, they start computing stuffs roomPtr->SetCeilingThickness(fCeilingThickness);
	
	

	roomPtr->SetFloorDomain(fFloorDomain);
	roomPtr->SetCeilingDomain(fCeilingDomain);

	// Clone Room Objects
	const int32 objCount = fSubObjects.GetElemCount();
	for(int32 iObj=0 ; iObj<objCount ; iObj++)
	{
		TMCCountedPtr<RoomSubObject> newObj;
		fSubObjects[iObj]->Clone(&newObj, roomPtr, cloneMode);
	}

	// Invalidate the tessellation
	roomPtr->ClearFlag(eRoomTessellated);
	roomPtr->ClearFlag(eRoomBasicTessellated);	

	roomPtr->ClearFlag(eIsTargeted);
}

/****************************************************************************************************

		PWall.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PWall.h"

#include "MCCountedPtrHelper.h"
#include "IShTokenStream.h"

#include "PRoom.h"
#include "PPoint.h"
#include "PPlan.h"
#include "BuildingPrim.h"
#include "PVertex.h"
#include "PFacet.h"
#include "Utils.h"
#include "PTessellator.h"
#include "MiscComUtilsImpl.h"
#include "I3dShFacetMesh.h"

void Wall::Init()
{
	fThickness = kDefaultThickness;
	fHeight = kDefaultLevelHeight;

	// Flat wall
	SetArcSegmentCount(1);
	SetArcOffset(0);
	
	fFlags = 0;
	
	fLeftDomain = eOutsideWallDomain;
	fRightDomain = eOutsideWallDomain;
	fBetweenDomain = eOutsideWallDomain;

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 8);
	SetName(fData->fDictionary, objectName);
}

Wall::Wall(BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2)
{
	fPoint0 = point1;
	fPoint1 = point2;

	if(fPoint0)fPoint0->AddWallReference(this);
	if(fPoint1)fPoint1->AddWallReference(this);

	fLevel = inLevel;

	fData = data;

	Init();

	fLevel->LevelPlan().AddWallReference(this);
}

Wall::~Wall()
{
	mConstrPoints.DeleteAndRemoveAll();
}

void Wall::CreateWall(Wall **wall, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2)
{
	TMCCountedCreateHelper<Wall> result(wall);

	result = new Wall(data,inLevel,point1,point2);
	ThrowIfNoMem(result);
}

void Wall::DeleteWall()
{
	if(fRoom0)
		fRoom0->DeleteRoom();
	if(fRoom1)
		fRoom1->DeleteRoom();

	// Remove the references to the wall from the 2 points
	fPoint0->RemoveWallReference(this);
	fPoint1->RemoveWallReference(this);

	fPoint0=NULL;
	fPoint1=NULL;

	// Delete the objects
	//const int32 objCount = fSubObjects.GetElemCount();

	//for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
	//{
	//	WallSubObject* object = fSubObjects[iObj];
	//	object->DeleteObj();

	//}
	fSubObjects.ArrayFree();

	// Must also supress reference in the Plan
	TMCCountedPtr<Level> levelPtr = fLevel;
	fLevel = NULL;
	levelPtr->LevelPlan().RemoveWallReference(this);
}

void Wall::ReplacePointReference(VPoint* oldPt,VPoint* newPt)
{
	if(oldPt==fPoint0)
	{
		// Check if we don't have a double wall
		const int32 point1WallCount = fPoint1->GetWallCount();
		for( int32 iWall=0 ; iWall<point1WallCount ; iWall++ )
		{
			if(fPoint1->GetWall(iWall)->GetOtherPoint(fPoint1) == newPt)
			{
				if(fRoom0) fRoom0->ReplacePointReferences(oldPt, newPt);
				if(fRoom1) fRoom1->ReplacePointReferences(oldPt, newPt);
				// A wall already exist between this 2 points
				Wall* wallToKeep = fPoint1->GetWall(iWall);
				if(wallToKeep->GetPoint(1)==fPoint1)
				{	// the 2 walls have the same orientation
					if(wallToKeep->GetRoom(0)==NULL)
						wallToKeep->SetRoom(0,fRoom0);
					if(wallToKeep->GetRoom(1)==NULL)
						wallToKeep->SetRoom(1,fRoom1);
				}
				else
				{
					if(wallToKeep->GetRoom(0)==NULL)
						wallToKeep->SetRoom(0,fRoom1);
					if(wallToKeep->GetRoom(1)==NULL)
						wallToKeep->SetRoom(1,fRoom0);
				}
				oldPt->RemoveWallReference(this);
				fRoom0 = NULL;
				fRoom1 = NULL;
				DeleteWall();
				return;
			}
		}

		fPoint0=newPt;
	}
	else if(oldPt==fPoint1)
	{
		// Check if we don't have a double wall
		const int32 point0WallCount = fPoint0->GetWallCount();
		for( int32 iWall=0 ; iWall<point0WallCount ; iWall++ )
		{
			if(fPoint0->GetWall(iWall)->GetOtherPoint(fPoint0) == newPt)
			{
				// A wall already exist between this 2 points
				if(fRoom0) fRoom0->ReplacePointReferences(oldPt, newPt);
				if(fRoom1) fRoom1->ReplacePointReferences(oldPt, newPt);
				// A wall already exist between this 2 points
				Wall* wallToKeep = fPoint0->GetWall(iWall);
				if(wallToKeep->GetPoint(0)==fPoint0)
				{	// the 2 walls have the same orientation
					if(wallToKeep->GetRoom(0)==NULL)
						wallToKeep->SetRoom(0,fRoom0);
					if(wallToKeep->GetRoom(1)==NULL)
						wallToKeep->SetRoom(1,fRoom1);
				}
				else
				{
					if(wallToKeep->GetRoom(0)==NULL)
						wallToKeep->SetRoom(0,fRoom1);
					if(wallToKeep->GetRoom(1)==NULL)
						wallToKeep->SetRoom(1,fRoom0);
				}
				oldPt->RemoveWallReference(this);
				fRoom0 = NULL;
				fRoom1 = NULL;
				DeleteWall();
				return;
			}
		}
		
		fPoint1=newPt;
	}
	else return;

	// Keep the data base clean: replace also the references in the rooms
	if(fRoom0) fRoom0->ReplacePointReferences(oldPt, newPt);
	if(fRoom1) fRoom1->ReplacePointReferences(oldPt, newPt);

	oldPt->RemoveWallReference(this);
	newPt->AddWallReference(this);
}

boolean Wall::Split(VPoint* splitPoint)
{
	// Check that the split position is between the 2 points
	const TVector2& splitPos = splitPoint->Position();
	const TVector2& pos0 = fPoint0->Position();
	const TVector2& pos1 = fPoint1->Position();
	
	MY_ASSERT(pos0!=pos1);

	const TVector2 axis = pos1-pos0;
	if((axis*(splitPos-pos0))*(axis*(splitPos-pos1))>0)
		return false;

	// If the actual pos is nearly one of the extremity, don't create a new point
	if(splitPos.IsEqual(pos0,kRealEpsilon) || splitPos.IsEqual(pos1,kRealEpsilon))
		return false;

	// If the split point already has walls, need to build the rooms arround again
	const int32 otherWallCount = splitPoint->GetWallCount();
	if(otherWallCount)
	{
		// Mark the walls
		for(int32 iWall=0 ; iWall<otherWallCount ; iWall++)
		{

			TMCCountedPtr<Wall> wall;
			wall = splitPoint->GetWall(iWall);
			if(wall->GetRoom(0)) wall->SetFlag(eWallRebuildRoom0);
			if(wall->GetRoom(1)) wall->SetFlag(eWallRebuildRoom1);
		}

		if(fRoom0)
			SetFlag(eWallRebuildRoom0);

		if(fRoom1)
			SetFlag(eWallRebuildRoom1);
	}

	TMCCountedPtr<Wall> newWall;
	TMCCountedPtr<VPoint> prevPoint0 = fPoint0;
	TMCCountedPtr<VPoint> prevPoint1 = fPoint1;
	const int32 flags = fPoint1->GetFlags();
	boolean flip = false;
	// Check if a wall doesn't already exist
	Wall* wallTo0 = splitPoint->GetWall(fPoint0);
	Wall* wallTo1 = splitPoint->GetWall(fPoint1);
	if(wallTo0&&wallTo1)
	{
		// 2 walls already exist, this need to be destroyed
		// and its previous room rebuild
		{
			// Insert the new point in the rooms and the rooms in the new wall
			if(fRoom0)
			{
				fRoom0->InsertPointBetween(splitPoint,prevPoint0,prevPoint1);

				if(wallTo0->GetPoint(0) == fPoint0)
				{
					wallTo0->SetFlag(eWallRebuildRoom0);
					if(!wallTo0->GetRoom(0))
						wallTo0->SetRoom(0,fRoom0);
					else
						MCNotify("An error might occured later ?");
				}
				else				
				{
					wallTo0->SetFlag(eWallRebuildRoom1);
					if(!wallTo0->GetRoom(1))
						wallTo0->SetRoom(1,fRoom0);
					else
						MCNotify("An error might occured later ?");
				}

				if(wallTo1->GetPoint(1) == fPoint1)
				{
					wallTo1->SetFlag(eWallRebuildRoom0);
					if(!wallTo1->GetRoom(0))
						wallTo1->SetRoom(0,fRoom0);
					else
						MCNotify("An error might occured later ?");
				}
				else				
				{
					wallTo1->SetFlag(eWallRebuildRoom1);
					if(!wallTo1->GetRoom(1))
						wallTo1->SetRoom(1,fRoom0);
					else
						MCNotify("An error might occured later ?");
				}
			}
			if(fRoom1)
			{
				fRoom1->InsertPointBetween(splitPoint,prevPoint0,prevPoint1);
			
				if(wallTo0->GetPoint(0) == fPoint0)
				{
					wallTo0->SetFlag(eWallRebuildRoom1);
					if(!wallTo0->GetRoom(1))
						wallTo0->SetRoom(1,fRoom1);
					else
						MCNotify("An error might occured later ?");
				}
				else				
				{
					wallTo0->SetFlag(eWallRebuildRoom0);
					if(!wallTo0->GetRoom(0))
						wallTo0->SetRoom(0,fRoom1);
					else
						MCNotify("An error might occured later ?");
				}

				if(wallTo1->GetPoint(1) == fPoint1)
				{
					wallTo1->SetFlag(eWallRebuildRoom1);
					if(!wallTo1->GetRoom(1))
						wallTo1->SetRoom(1,fRoom1);
					else
						MCNotify("An error might occured later ?");
				}
				else				
				{
					wallTo1->SetFlag(eWallRebuildRoom0);
					if(!wallTo1->GetRoom(0))
						wallTo1->SetRoom(0,fRoom1);
					else
						MCNotify("An error might occured later ?");
				}
			}
		}
		fRoom0 = NULL; // Set the room to NULL to avoid deleting them when deleting the wall
		fRoom1 = NULL;
		DeleteWall();

		return false;// Return false: the wall wasn't splitted (but destroyed)
	}
	else if(wallTo0)
	{
		newWall = wallTo0;
		if(newWall->GetPoint(1) == fPoint0)
			flip = true;
		splitPoint->AddWallReference(this);
		fPoint0->RemoveWallReference(this);
		fPoint0 = splitPoint;

	}
	else if(wallTo1)
	{
		newWall = wallTo1;
		if(newWall->GetPoint(0) == fPoint1)
			flip = true;
		splitPoint->AddWallReference(this);
		fPoint1->RemoveWallReference(this);
		fPoint1 = splitPoint;
	
	}
	else
	{
		// Split the wall
		fPoint1 = splitPoint;
		splitPoint->AddWallReference(this);
		prevPoint1->RemoveWallReference(this);

		MY_ASSERT(fPoint0->Position()!=fPoint1->Position());
		MY_ASSERT(splitPoint->Position()!=prevPoint1->Position());

		Clone( &newWall, fLevel,splitPoint,prevPoint1, eNoChild );
		newWall->fSubObjects.ArrayFree(); // remove them, will move the one on it after
		// Wall::CreateWall(&newWall,fData,fLevel,splitPoint,prevPoint1);
	}

	//ClearFlag(eIsTargeted);
	//newWall->SetFlags(fFlags); // so the eWallRebuildRoom0 and eWallRebuildRoom1 are passed
	//newWall->SetThickness(fThickness);
	//newWall->SetWallHeight(fHeight);
	//newWall->SetArcOffset(GetArcOffset());
	//newWall->SetArcSegmentCount(GetArcSegmentCount());

	splitPoint->SetFlags( flags );
	splitPoint->ClearFlag(ePointToDelete);
	splitPoint->SetSelection(true);

	// Invalidations before rebuilding the rooms
	InvalidateTessellation();
	fData->InvalidateStatus();

	{
		// If the wall has objects, we need to see in which of the 2 parts they are now
		const int32 objCount = fSubObjects.GetElemCount();

		const real32 firstWallLength = (splitPos-pos0).GetNorm();
		for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
		{
			WallSubObject* object = fSubObjects[iObj];

			TVector2 center = object->GetPolylineCenter();
			if(center.x>firstWallLength)
			{	// Move this object in the 2nd wall
				object->SetOnWall(newWall);
				center.x-=firstWallLength;
				object->SetPolylineCenter( center, false );
			}
		}
	}

	{
		// Insert the new point in the rooms and the rooms in the new wall
		if(fRoom0)
		{
			newWall->SetRoom(flip?1:0,fRoom0);
			fRoom0->InsertPointBetween(splitPoint,prevPoint0,prevPoint1);
		}
		if(fRoom1)
		{
			newWall->SetRoom(flip?0:1,fRoom1);
			if(fRoom1!=fRoom0)
				fRoom1->InsertPointBetween(splitPoint,prevPoint0,prevPoint1);
		}
	}

	return true;
}

VPoint* Wall::Split(const TVector2& splitPos)
{
	// Check that the split position is between the 2 points
	const TVector2& pos0 = fPoint0->Position();
	const TVector2& pos1 = fPoint1->Position();

	MY_ASSERT(pos0!=pos1);

	const TVector2 axis = pos1-pos0;
	if((axis*(splitPos-pos0))*(axis*(splitPos-pos1))>0)
		return NULL;

	// Project the point on the axis to be sure to keep the wall straight 
	// (might not be necessary)
	TVector2 actualPos;
	Project(splitPos, pos0, pos1, actualPos	);

	// If the actual pos is nearly one of the extremity, don't create a new point
	if(actualPos.IsEqual(pos0,kRealEpsilon) ||actualPos.IsEqual(pos1,kRealEpsilon))
		return NULL;

	// Create the new point
	TMCCountedPtr<VPoint> newPoint;
	VPoint::CreatePoint(&newPoint,fData,fLevel,actualPos,this);
	// Communicate its neighboor flags
	newPoint->SetFlags(fPoint1->GetFlags());
	newPoint->ClearFlag(ePointToDelete);
	newPoint->ClearFlag(eWallOrdered);

	// Split the wall
	TMCCountedPtr<VPoint> prevExtremity = fPoint1;
	fPoint1 = newPoint;

	MY_ASSERT(fPoint0->Position()!=fPoint1->Position());

	prevExtremity->RemoveWallReference(this);

	MY_ASSERT(newPoint->Position()!=prevExtremity->Position());

	TMCCountedPtr<Wall> newWall;
	Clone( &newWall, fLevel,newPoint,prevExtremity, eNoChild );
	newWall->fSubObjects.ArrayFree(); // remove them, will move the one on it after
	//Wall::CreateWall(&newWall,fData,fLevel,newPoint,prevExtremity);

	//newWall->SetFlags(fFlags); // so the eWallRoom0Deleted and eWallRoom1Deleted are passed
	//newWall->ClearFlag(eIsTargeted);
	//newWall->SetThickness(fThickness);
	//newWall->SetWallHeight(fHeight);
	//newWall->SetArcOffset(GetArcOffset());
	//newWall->SetArcSegmentCount(GetArcSegmentCount());

	// Insert the new point in the rooms and the rooms in the new wall
	if(fRoom0)
	{
		newWall->SetRoom(0,fRoom0);
		fRoom0->InsertPointBetween(newPoint,fPoint0,prevExtremity);
	}
	if(fRoom1)
	{
		newWall->SetRoom(1,fRoom1);
		if(fRoom1!=fRoom0)
		{
			fRoom1->InsertPointBetween(newPoint,fPoint0,prevExtremity);
		}
	}

	newPoint->SetSelection(true);

	// Invalidations
	InvalidateTessellation();
	fData->InvalidateStatus();

	{
		// If the wall has objects, we need to see in which of the 2 parts they are now
		const int32 objCount = fSubObjects.GetElemCount();

		const real32 firstWallLength = (actualPos-pos0).GetNorm();
		for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
		{
			WallSubObject* object = fSubObjects[iObj];

			TVector2 center = object->GetPolylineCenter();
			if(center.x>firstWallLength)
			{	
				// Move this object in the 2nd wall
				object->SetOnWall(newWall);
				center.x-=firstWallLength;
				object->SetPolylineCenter( center, false );
			}
		}
	}

	return newPoint;
}

void Wall::SetSelection(const boolean select)
{
	if( select != Selected() )
	{
		if(select)
		{
			SetFlag(eIsSelected);
			// Check if the level is selected
			fLevel->SelectIfPossible();

			// Select the points making the wall
			fPoint0->SetSelection(true);
			fPoint1->SetSelection(true);
		}
		else		
		{
			ClearFlag(eIsSelected);
			fLevel->ClearFlag(eIsSelected);

			// Deselect the objects
			const int32 objectCount = fSubObjects.GetElemCount();
			for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
			{
				fSubObjects[iObj]->ClearFlag(eIsSelected);
			}

			// Deselect the points that aren't used anymore
			fPoint0->DeselectIfPossible();
			fPoint1->DeselectIfPossible();
		}

		fData->InvalidateStatus();
	}
}

// Select the walls arround, and ask to select the wall arround
void Wall::SetCompleteSelection()
{
	SetFlag(eIsSelected);

	fPoint0->SetFlag(eIsSelected);
	fPoint1->SetFlag(eIsSelected);

	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		fSubObjects[iObj]->SetFlag(eIsSelected);
	}


	if(fRoom0)
	{
		fRoom0->SetFlag(eIsSelected);
		const int32 objCount = fRoom0->GetObjectCount();

		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			fRoom0->GetObject(iObj)->SetFlag(eIsSelected);
		}
	}
	if(fRoom1)
	{
		fRoom1->SetFlag(eIsSelected);
		const int32 objCount = fRoom1->GetObjectCount();

		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			fRoom1->GetObject(iObj)->SetFlag(eIsSelected);
		}
	}

	{
		const int32 wallCount0 = fPoint0->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount0 ; iWall++)
		{
			Wall* wall = fPoint0->GetWall(iWall);
			if(!wall->Selected())
			{
				wall->SetCompleteSelection();
			}
		}
	}
	{
		const int32 wallCount1 = fPoint1->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount1 ; iWall++)
		{
			Wall* wall = fPoint1->GetWall(iWall);
			if(!wall->Selected())
			{
				wall->SetCompleteSelection();
			}
		}
	}
}

real32 Wall::GetStraightLength() const
{
	return (fPoint0->Position()-fPoint1->Position()).GetNorm();
}

real32 Wall::GetWallTotalHeight() const
{
	return GetWallHeight();
}

real32 Wall::GetWallHeight() const
{
	return(fHeight==kDefaultLevelHeight?fLevel->GetLevelHeight():fHeight);
}

TVector2 Wall::GetMidPos()
{
	return .5*(fPoint0->Position() + fPoint1->Position());
}

TVector2 Wall::GetStraightDirection(VPoint* fromPoint)
{
	TVector2 dir;
	dir = fPoint1->GetPosition() - fPoint0->GetPosition();
	if(fromPoint != fPoint0)
	{
		dir *= -1;
	}

	dir.Normalize();
	return dir;
}

void Wall::ValidateArc()
{
	if(fPoint0)
		mCircleArc.SetFrom(fPoint0->Position());
	if(fPoint1)
		mCircleArc.SetTo(fPoint1->Position());

	// ThickArc settings
	mCircleArc.SetLeftThickness( 0.5*GetThickness() );
	mCircleArc.SetRightThickness( 0.5*GetThickness() );
}

TVector2 Wall::GetCurvedDirection(VPoint* fromPoint)
{
	// Use the circle arc to get the direction
	
	ValidateArc();
	const TMCClassArray<TVector2>& arcPos = mCircleArc.GetArc();
	const int32 arcCount = arcPos.GetElemCount();
	TVector2 dir;
	if(arcCount>2)
	{
		if(fromPoint == fPoint0)
		{
			dir = arcPos[1] - arcPos[0];
		}
		else
		{
			dir = arcPos[arcCount-2] - arcPos[arcCount-1];
		}
	}
	else
	{
		dir = GetOtherPoint(fromPoint)->Position();
		dir -= fromPoint->Position();
	}

	dir.Normalize();
	return dir;
}

void Wall::GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K)
{
	if( !Flag(eWallBaseIsValid) ) 
	{
		// Optimisation needed: cache this base
		const real32 z = fLevel->GetDistanceToGround();
		fO.SetFromXY(GetPoint(0)->Position(),z);
		TVector3 pos1;pos1.SetFromXY(GetPoint(1)->Position(),z);
		const TVector3 wallAxe = pos1-fO;
		const real32 length = wallAxe.GetNorm();
		fI =  wallAxe/length;
		fJ = TVector3::kUnitZ;
		fK = fI^fJ;
	
		SetFlag(eWallBaseIsValid);
	}

	O = fO;
	I = fI;
	J = fJ;
	K = fK;
}

real32 PosMovingToLeft(const TVector2& point,
					   const TMCClassArray<TVector2>& minZones,
					   const TMCClassArray<TVector2>& maxZones)
{
	const int32 zoneCount = minZones.GetElemCount();
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const TVector2& min = minZones[iZone];
		const TVector2& max = maxZones[iZone];
		if( point.x>min.x && point.x<max.x &&
			point.y>min.y && point.y<max.y)
		{	// we're in a zone, move it to its left
			const TVector2 newPoint( min.x, point.y );
			return PosMovingToLeft(newPoint,minZones,maxZones);
		}
	}

	// nothing's bothering us, return the x value
	return point.x;
}

real32 PosMovingToRight(const TVector2& point,
					   const TMCClassArray<TVector2>& minZones,
					   const TMCClassArray<TVector2>& maxZones)
{
	const int32 zoneCount = minZones.GetElemCount();
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const TVector2& min = minZones[iZone];
		const TVector2& max = maxZones[iZone];
		if( point.x>min.x && point.x<max.x &&
			point.y>min.y && point.y<max.y)
		{	// we're in a zone, move it to its right
			const TVector2 newPoint( max.x, point.y );
			return PosMovingToRight(newPoint,minZones,maxZones);
		}
	}

	// nothing's bothering us, return the x value
	return point.x;
}

real32 PosMovingToDown(const TVector2& point,
					   const TMCClassArray<TVector2>& minZones,
					   const TMCClassArray<TVector2>& maxZones)
{
	const int32 zoneCount = minZones.GetElemCount();
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const TVector2& min = minZones[iZone];
		const TVector2& max = maxZones[iZone];
		if( point.x>min.x && point.x<max.x &&
			point.y>min.y && point.y<max.y)
		{	// we're in a zone, move it to its right
			const TVector2 newPoint( point.x, min.y );
			return PosMovingToDown(newPoint,minZones,maxZones);
		}
	}

	// nothing's bothering us, return the x value
	return point.y;
}

real32 PosMovingToUp(const TVector2& point,
					   const TMCClassArray<TVector2>& minZones,
					   const TMCClassArray<TVector2>& maxZones)
{
	const int32 zoneCount = minZones.GetElemCount();
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const TVector2& min = minZones[iZone];
		const TVector2& max = maxZones[iZone];
		if( point.x>min.x && point.x<max.x &&
			point.y>min.y && point.y<max.y)
		{	// we're in a zone, move it to its right
			const TVector2 newPoint( point.x, max.y );
			return PosMovingToUp(newPoint,minZones,maxZones);
		}
	}

	// nothing's bothering us, return the x value
	return point.y;
}

// A method that does it best to put the objects inside the wall.
// If an object can't fit, it's removed
boolean Wall::CheckWallObjects()
{
	boolean deleteSome = false;

	const int32 objCount = fSubObjects.GetElemCount();

	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		WallSubObject* object = fSubObjects[iObj];
		if( !object->CheckObjectPosition() )
			deleteSome = true;

	}

	return deleteSome;
}

const TMCPtrArray<ConstrPoint>& Wall::GetWallConstructionPoints()
{
	if( !Flag(eWallTessellated) ) 
	{
		ValidateTessellation();
	}

	return mConstrPoints; 
}

const TMCClassArray<TVector2>& Wall::GetLeftPosProjection()
{
	if( !Flag(eWallTessellated) ) 
	{
		ValidateTessellation();
	}

	return mLeftPos;
}

const TMCClassArray<TVector2>& Wall::GetRightPosProjection()
{
	if( !Flag(eWallTessellated) ) 
	{
		ValidateTessellation();
	}

	return mRightPos;
}

boolean Wall::OffsetThisObject(WallSubObject* object, const TVector2& offset)
{	// Move this wall with caution: check that we stay in the wall
	// and that we don't overlap other windows. Return false if we couldn't
	// find any space for it.

	// We first define an autorized area for the center. This area is represented
	// by one positive rectangular zone minus several negative rectangular zone
	// Each zone is define by 2 corners: min and max

	// 1: object size
	const real32 securityMargin = 0.0001f;
	const real32 halfWidth = .5*object->Get2DWidth()+securityMargin;
	const real32 halfHeight = .5*object->Get2DHeight()+securityMargin;
	const TVector2 halfDiag(halfWidth,halfHeight);

	// 2: positive zone
	const TMCClassArray<TVector2>& leftPos = GetLeftPosProjection();
	const int32 leftPosCount = leftPos.GetElemCount();

	const TMCClassArray<TVector2>& rightPos = GetRightPosProjection();
	const int32 rightPosCount = rightPos.GetElemCount();

	TVector2 minPositive(-kBigRealValue,-kBigRealValue);
	TVector2 maxPositive(kBigRealValue,kBigRealValue);
	//TMCClassArray<Vertex>& vertices = Vertices();
	ValidateTessellation();
	TVector3 O,I,J,K;
	GetBase(O, I, J, K);

	// Min
	const real32 z = fLevel->GetDistanceToGround();
	TVector3 leftMinCorner(leftPos[0].x, leftPos[0].y, z);
	TVector2 proj = ProjectIn(leftMinCorner,O,I,J);
	if(proj.x>minPositive.x) minPositive.x=proj.x;
	if(proj.y>minPositive.y) minPositive.y=proj.y;
	TVector3 rightMinCorner(rightPos[0].x, rightPos[0].y, z);
	proj = ProjectIn(rightMinCorner,O,I,J);
	if(proj.x>minPositive.x) minPositive.x=proj.x;
	if(proj.y>minPositive.y) minPositive.y=proj.y;
	minPositive+=halfDiag;

	// Max
	const real32 wallTotalHeight = GetWallTotalHeight();
	TVector3 leftMaxCorner(leftPos[leftPosCount-1].x, leftPos[leftPosCount-1].y, z+wallTotalHeight);
	proj = ProjectIn(leftMaxCorner,O,I,J);
	if(proj.x<maxPositive.x) maxPositive.x=proj.x;
	if(proj.y<maxPositive.y) maxPositive.y=proj.y;
	TVector3 rightMaxCorner(rightPos[rightPosCount-1].x, rightPos[rightPosCount-1].y, z+wallTotalHeight);
	proj = ProjectIn(rightMaxCorner,O,I,J);
	if(proj.x<maxPositive.x) maxPositive.x=proj.x;
	if(proj.y<maxPositive.y) maxPositive.y=proj.y;
	maxPositive-=halfDiag;

	if(maxPositive.x<minPositive.x)
		return false; // Object is too wide for this wall

	if(maxPositive.y<minPositive.y)
		return false; // Object is too high for this wall

	// 3: negative zones
	TMCClassArray<TVector2> minNegatives;
	TMCClassArray<TVector2> maxNegatives;

	const int32 objCount = fSubObjects.GetElemCount();
	const int32 negZoneCount = objCount-1;
	minNegatives.SetElemCount(negZoneCount);
	maxNegatives.SetElemCount(negZoneCount);

	int32 index = 0;
	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		WallSubObject* obj = fSubObjects[iObj];
		MY_ASSERT(obj->GetOutline().GetElemCount());
		if( obj != object )
		{
			const TBBox2D& bbox = obj->GetHoleBBox();
			minNegatives[index] = bbox.fMin-halfDiag;
			maxNegatives[index] = bbox.fMax+halfDiag;
			index++;
		}
	}

	// 4: find a space for our center
	// First put it in the wall if it's out
	const TVector2& center = object->GetPolylineCenter();
	TVector2 newCenter = center+offset;
	if(newCenter.x<minPositive.x)	newCenter.x=minPositive.x;
	if(newCenter.y<minPositive.y)	newCenter.y=minPositive.y;
	if(newCenter.x>maxPositive.x)	newCenter.x=maxPositive.x;
	if(newCenter.y>maxPositive.y)	newCenter.y=maxPositive.y;
	// Then get it out of the negatives zones if its in
	for(int32 iNeg=0 ; iNeg<negZoneCount ; iNeg++)
	{
		TVector2& min = minNegatives[iNeg];
		TVector2& max = maxNegatives[iNeg];
		if( newCenter.x>min.x && newCenter.x<max.x &&
			newCenter.y>min.y && newCenter.y<max.y)
		{	// we're in a negative zone, find the shortest hor or ver way out
			real32 leftX = PosMovingToLeft(newCenter, minNegatives, maxNegatives);
			real32 rightX = PosMovingToRight(newCenter, minNegatives, maxNegatives);
			real32 downY = PosMovingToDown(newCenter, minNegatives, maxNegatives);
			real32 upY = PosMovingToUp(newCenter, minNegatives, maxNegatives);

			// retain the shortest one
			const int32 useLeft = 0X00000001;
			const int32 useRight = 0X00000002;
			const int32 useDown = 0X00000004;
			const int32 useUp = 0X00000008;
			int32 use = useLeft+useRight+useDown+useUp;
			if(leftX<minPositive.x) use&=~useLeft;
			if(rightX>maxPositive.x) use&=~useRight;
			if(downY<minPositive.y) use&=~useDown;
			if(upY>maxPositive.y) use&=~useUp;

			if(use==0) return false; // we could look for some space somewhere else
									// (check the diagonals for example)
			const real32 distL = use&useLeft?newCenter.x-leftX:kBigRealValue;
			const real32 distR = use&useRight?rightX-newCenter.x:kBigRealValue;
			const real32 distD = use&useDown?newCenter.y-downY:kBigRealValue;
			const real32 distU = use&useUp?upY-newCenter.y:kBigRealValue;

			real32 minLR = kBigRealValue;
			if(distL<distR)
			{
				minLR = distL;
				use&=~useRight;
			}
			else
			{
				minLR = distR;
				use&=~useLeft;
			}

			real32 minDU = kBigRealValue;
			if(distD<distU)
			{
				minDU = distD;
				use&=~useUp;
			}
			else
			{
				minDU = distU;
				use&=~useDown;
			}

			real32 minLRDU=kBigRealValue;
			if(minLR<minDU)
			{
				minLRDU = minLR;
				use&=~useUp;
				use&=~useDown;
			}
			else
			{
				minLRDU = minDU;
				use&=~useRight;
				use&=~useLeft;
			}

			if(use==0) return false; // Shouldn't occurred here

			newCenter.x -= use&useLeft?minLRDU:0;
			newCenter.x += use&useRight?minLRDU:0;
			newCenter.y -= use&useDown?minLRDU:0;
			newCenter.y += use&useUp?minLRDU:0;

			object->SetPolylineCenter(newCenter, false);
			return true;
		}
	}

	object->SetPolylineCenter(newCenter, false);
	return true;
}

boolean Wall::ScaleThisObject(WallSubObject* object, const TVector2& scale)
{	// Scale this object with caution: check that we stay in the wall
	// and that we don't overlap other objects. Return false if we couldn't
	// find any space for it.

	// We first define an autorized area for the center. This area is represented
	// by one positive rectangular zone minus several negative rectangular zone
	// Each zone is define by 2 corners: min and max

	// 1: object size
	const real32 securityMargin = .0001f;
	const TVector2 security(securityMargin,securityMargin);

	// 2: positive zone
	const TMCClassArray<TVector2>& leftPos = GetLeftPosProjection();
	const int32 leftPosCount = leftPos.GetElemCount();

	const TMCClassArray<TVector2>& rightPos = GetRightPosProjection();
	const int32 rightPosCount = rightPos.GetElemCount();

	TVector2 minPositive(-kBigRealValue,-kBigRealValue);
	TVector2 maxPositive(kBigRealValue,kBigRealValue);
//	TMCClassArray<Vertex>& vertices = Vertices();
	ValidateTessellation();
	TVector3 O,I,J,K;
	GetBase(O, I, J, K);

	// Min
	const real32 z = fLevel->GetDistanceToGround();
	TVector3 leftMinCorner(leftPos[0].x, leftPos[0].y, z);
	TVector2 proj = ProjectIn(leftMinCorner,O,I,J);
	if(proj.x>minPositive.x) minPositive.x=proj.x;
	if(proj.y>minPositive.y) minPositive.y=proj.y;
	TVector3 rightMinCorner(rightPos[0].x, rightPos[0].y, z);
	proj = ProjectIn(rightMinCorner,O,I,J);
	if(proj.x>minPositive.x) minPositive.x=proj.x;
	if(proj.y>minPositive.y) minPositive.y=proj.y;
	minPositive+=security;

	// Max
	const real32 wallTotalHeight = GetWallTotalHeight();
	TVector3 leftMaxCorner(leftPos[leftPosCount-1].x, leftPos[leftPosCount-1].y, z+wallTotalHeight);
	proj = ProjectIn(leftMaxCorner,O,I,J);
	if(proj.x<maxPositive.x) maxPositive.x=proj.x;
	if(proj.y<maxPositive.y) maxPositive.y=proj.y;
	TVector3 rightMaxCorner(rightPos[rightPosCount-1].x, rightPos[rightPosCount-1].y, z+wallTotalHeight);
	proj = ProjectIn(rightMaxCorner,O,I,J);
	if(proj.x<maxPositive.x) maxPositive.x=proj.x;
	if(proj.y<maxPositive.y) maxPositive.y=proj.y;
	maxPositive-=security;

	if(maxPositive.x<minPositive.x)
		return false; // Object is too wide for this wall

	if(maxPositive.y<minPositive.y)
		return false; // Object is too high for this wall

	// 3: negative zones
	TMCClassArray<TVector2> minNegatives;
	TMCClassArray<TVector2> maxNegatives;

	const int32 objCount = fSubObjects.GetElemCount();
	const int32 negZoneCount = objCount-1;
	minNegatives.SetElemCount(negZoneCount);
	maxNegatives.SetElemCount(negZoneCount);

	int32 index = 0;
	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	{
		WallSubObject* obj = fSubObjects[iObj];
		MY_ASSERT(obj->GetOutline().GetElemCount());
		if( obj != object )
		{
			const TBBox2D& bbox = obj->GetHoleBBox();
			minNegatives[index] = bbox.fMin-security;
			maxNegatives[index] = bbox.fMax+security;
			index++;
		}
	}

	// 4: Check that the center of scaling is in a positive zone
	const TVector2& center = object->GetPolylineCenter();
	if(center.x<minPositive.x)	return false;
	if(center.y<minPositive.y)	return false;
	if(center.x>maxPositive.x)	return false;
	if(center.y>maxPositive.y)	return false;
	// Then get it out of the negatives zones if its in
	for(int32 iNeg=0 ; iNeg<negZoneCount ; iNeg++)
	{
		TVector2& min = minNegatives[iNeg];
		TVector2& max = maxNegatives[iNeg];
		if( center.x>min.x && center.x<max.x &&
			center.y>min.y && center.y<max.y)
		{	// we're in a negative zone
			return false;
		}
	}

	// 5: Check the points of the polyline
	// If the scaling is too big to be countain in the wall, we first try to move the center
	// of scaling, then we limit the scaling value
	TVector2 newScale = scale;
	TVector2 offsetCenter = TVector2::kZero;
	const TMCCountedPtrArray<OutlinePoint>& polyline = object->GetOutline();
	const int32 polyCount = polyline.GetElemCount();
	MY_ASSERT(polyCount);

	const real32 midX = .5*(minPositive.x+maxPositive.x);
	const real32 midY = .5*(minPositive.y+maxPositive.y);

	{	// Within the wall area
		for(int32 iPt=0 ; iPt<polyCount ; iPt++)
		{
			const TVector2& P = polyline[iPt]->Position();
			const TVector2 scaledPoint = ScalePoint(P+offsetCenter,newScale,center+offsetCenter);

			const real32 minXOver = minPositive.x-scaledPoint.x;
			if(minXOver>0)
			{	// We need to move the center if possible, otherwise lower the scaling value
				if(center.x + minXOver <= midX)
				{	// There's enougth space to move
					offsetCenter.x = minXOver;
				}
				else
				{	// We'll need to reduce the scaling to fit
					offsetCenter.x = midX-center.x;
					newScale.x = (midX-minPositive.x)/(midX-(P.x+offsetCenter.x)+securityMargin);
				}
				// For the fix center mode :
				//newScale.x=(minPositive.x-center.x)/(polyline[iPt].x-center.x);
			}
			else
			{
				const real32 maxXOver = scaledPoint.x-maxPositive.x;
				if(maxXOver>0)
				{
					if(center.x - maxXOver >= midX)
					{	// There's enougth space to move
						offsetCenter.x = -maxXOver;
					}
					else
					{	// We'll need to reduce the scaling to fit
						offsetCenter.x = midX-center.x;
						newScale.x = (maxPositive.x-midX)/((P.x+offsetCenter.x)-midX+securityMargin);
					}
					// For the fix center mode :
					//newScale.x=(maxPositive.x-center.x)/(polyline[iPt].x-center.x);
				}
			}

			const real32 minYOver = minPositive.y-scaledPoint.y;
			if(minYOver>0)
			{	// We need to move the center if possible, otherwise lower the scaling value
				if(center.y + minYOver <= midY)
				{	// There's enougth space to move
					offsetCenter.y = minYOver;
				}
				else
				{	// We'll need to reduce the scaling to fit
					offsetCenter.y = midY-center.y;
					newScale.y = (midY-minPositive.y)/(midY-(P.y+offsetCenter.y)+securityMargin);
				}
				// For the fix center mode :
				//newScale.y=(minPositive.y-center.y)/(polyline[iPt].y-center.y);
			}
			else
			{
				const real32 maxYOver = scaledPoint.y-maxPositive.y;
				if(maxYOver>0)
				{
					if(center.y - maxYOver >= midY)
					{	// There's enougth space to move
						offsetCenter.y = -maxYOver;
					}
					else
					{	// We'll need to reduce the scaling to fit
						offsetCenter.y = midY-center.y;
						newScale.y = (maxPositive.y-midY)/((P.y+offsetCenter.y)-midY+securityMargin);
					}
					// For the fix center mode :
					//newScale.y=(maxPositive.y-center.y)/(polyline[iPt].y-center.y);
				}
			}
		}
	}

	// Scale it now so we can use the Collide method
	object->OffsetPolyline(offsetCenter, false);
	object->ScalePolyline(newScale, false);

	{	// Check that were not in another object
		const int32 objCount = fSubObjects.GetElemCount();

		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			WallSubObject* otherObject = fSubObjects[iObj];
			
			if(object!=otherObject)
			{
				int32 securityCount=0;
				TVector2 collision;
				ECollisionType type = eUnknownCollision;
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

int32 Wall::GetSelectedRoomCount()
{
	int32 c=0; 
	if(fRoom0&&fRoom0->Selected()) 
		c++; 
	if(fRoom1&&fRoom1->Selected()) 
		c++; 
	return c;
}

void Wall::RemoveObjectReference(WallSubObject* obj)
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
			ClearFlag(eWallTessellated);

			break;
		}
	}
}

// Use the hitPoint to make a snapping
TVector3 Wall::GetSnappedPosOnWall(const TVector3& hitPoint,
									const EObjectType& forObject )
{
	// Get the wall base
	TVector3 O,I,J,K;
	GetBase(O, I, J, K);

	// Project the hitPoint on the wall to get it's 2D coordinates
	const TVector2 hitInWall = ProjectIn(hitPoint,O,I,J);

	TVector2 posInWall = hitInWall;
	switch(forObject)
	{
	default:
	case eWindow:
	case eNarrowWindow:
	case ePanoramicWindow:
		{
			if(hitInWall.x<0)
				posInWall.x = .5*GetStraightLength();
			else
				posInWall.x = Snap( hitInWall.x, 12, GetStraightLength() );

			posInWall.y = fData->GetDefaultWindowAltitude();
		} break;
	case eDoor:
	case eDoubleDoor:
	case eArrowDoor:
	case e2Circle16Door:
	case e4Circle16LDoor:
	case e4Circle16RDoor:
		{
			if(hitInWall.x<0)
				posInWall.x = .5*GetStraightLength();
			else
				posInWall.x = Snap( hitInWall.x, 12, GetStraightLength() );

			real32 maxThickness = 0;
			if(fRoom0 && fRoom0->GetFloorThickness()>maxThickness)
				maxThickness=fRoom0->GetFloorThickness();
			if(fRoom1 && fRoom1->GetFloorThickness()>maxThickness)
				maxThickness=fRoom1->GetFloorThickness();
			posInWall.y = maxThickness + .5*fData->GetDefaultDoorHeight();
		} break;
	}

	return (O+posInWall.x*I+posInWall.y*J);
}
/*
TVector3 Wall::GetPossibleObjectPos( const EObjectType& forObject )
{
	TVector3 pos=TVector3::kZero;

	TVector2 posInWall = TVector2::kZero;
	switch(forObject)
	{
	case eWindow:
		{
			posInWall.x = .5*GetStraightLength();
			posInWall.y = .5*GetWallHeight();
		} break;
	case eDoor:
		{
			posInWall.x = .5*GetStraightLength();
			const TMCClassArray<TVector2>& defaultDoor = fData->GetDefaultDoor();
			real32 maxThickness = 0;
			if(fRoom0 && fRoom0->GetFloorThickness()>maxThickness)
				maxThickness=fRoom0->GetFloorThickness();
			if(fRoom1 && fRoom1->GetFloorThickness()>maxThickness)
				maxThickness=fRoom1->GetFloorThickness();
			posInWall.y = maxThickness +
						.5*(defaultDoor[2].y-defaultDoor[0].y);
		} break;
	}

	TVector3 O,I,J,K;
	GetBase(O, I, J, K);

	pos = O+posInWall.x*I+posInWall.y*J;

	return pos;
}
*/

TVector3 Wall::GetMiddle()
{
	return(.5*(fPoint0->Get3DPos()+fPoint1->Get3DPos()));
}

TVector2 Wall::GetMiddle2D()
{
	return(.5*(fPoint0->Position()+fPoint1->Position()));
}

bool Wall::GetWallFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags)
{
	FacetMesh::Create(outMesh);
	TMCCountedPtr<FacetMesh> facetMesh;
	facetMesh = *outMesh;

	TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = vertices.GetElemCount();

	TMCClassArray<Triangle>& facets = Triangles();
	const int32 facetCount = facets.GetElemCount();

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
	ValidateDomains();
	for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
	{
		meshFacets[iFacet] = facets[iFacet];
		meshUVSpaceIDs[iFacet] = fTriangleDomain[iFacet];

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

void Wall::BuildRectangleUsingPos(	const int32 index1,
									const int32 index2,
									const int32 index3,
									const int32 index4,
									const TVector3& normal)
{
	const int32 indexes[4] = {index1,index2,index3,index4};
	const int32 startPos = fVertexArray.GetElemCount();
	for(int32 iVtx=0 ; iVtx<4 ; iVtx++)
	{
		const TVector3 pos = fVertexArray[indexes[iVtx]].Position();
		const TVector2 uv = fVertexArray[indexes[iVtx]].UV();
		Vertex vtx(pos,normal,uv);
		fVertexArray.AddElem(vtx);
	}

	Triangle triangle1(startPos,startPos+1,startPos+3);
	fTriangleArray.AddElem( triangle1 );
	fTriangleDomain.AddElem(fBetweenDomain);
	Triangle triangle2(startPos,startPos+3,startPos+2);
	fTriangleArray.AddElem( triangle2 );
	fTriangleDomain.AddElem(fBetweenDomain);
}

void Wall::BuildRectangleUsingPos(	const int32 index1,
									const int32 index2,
									const TVector3& pos1,
									const TVector3& pos2,
									const TVector3& normal)
{
	// Add 2 points using the index, 2 others using the pos
	const int32 startPos = fVertexArray.GetElemCount();
	{
		const TVector3 pos = fVertexArray[index1].Position();
		const TVector2 uv = fVertexArray[index1].UV();
		Vertex vtx(pos,normal,uv);
		fVertexArray.AddElem(vtx);
	}
	{
		const TVector3 pos = fVertexArray[index2].Position();
		const TVector2 uv = fVertexArray[index2].UV();
		Vertex vtx(pos,normal,uv);
		fVertexArray.AddElem(vtx);
	}
	{
		Vertex vtx(pos1,normal,TVector2(1,1));
		fVertexArray.AddElem(vtx);
	}
	{
		Vertex vtx(pos2,normal,TVector2(1,1));
		fVertexArray.AddElem(vtx);
	}

	Triangle triangle1(startPos,startPos+1,startPos+3);
	fTriangleArray.AddElem( triangle1 );
	fTriangleDomain.AddElem(fBetweenDomain);
	Triangle triangle2(startPos,startPos+3,startPos+2);
	fTriangleArray.AddElem( triangle2 );
	fTriangleDomain.AddElem(fBetweenDomain);
}

void Wall::ValidateTessellation2()
{
	// Clear previous data
	fTriangleArray.ArrayFree();
	fTriangleDomain.ArrayFree();
	fVertexArray.ArrayFree();

	// 1. Build the arc

	ValidateArc();
	const int32 segmentCount = GetArcSegmentCount();

	mSmooth = segmentCount>1;

	// Set the limits (intersections with neighbors)
	if(fPoint0->GetWallCount()>1)
	{
		TVector2 leftStartLimit;
		fPoint0->GetLeftPos(leftStartLimit, this);
		mCircleArc.SetLeftStartLimit( leftStartLimit );

		TVector2 rightStartLimit;
		fPoint0->GetRightPos(rightStartLimit, this);
		mCircleArc.SetRightStartLimit( rightStartLimit );

		mCircleArc.SetHasStartLimit( true );
	}
	else
	{
		mCircleArc.SetHasStartLimit( false );
	}

	if(fPoint1->GetWallCount()>1)
	{

		TVector2 leftEndLimit;
		fPoint1->GetLeftPos(leftEndLimit, this);
		mCircleArc.SetLeftEndLimit( leftEndLimit );

		TVector2 rightEndLimit;
		fPoint1->GetRightPos(rightEndLimit, this);
		mCircleArc.SetRightEndLimit( rightEndLimit ) ;
	
		mCircleArc.SetHasEndLimit( true );
	}
	else
	{
		mCircleArc.SetHasEndLimit( false );
	}

	// 2. Build the flat tesselation
	const real32 wallTotalHeight = GetWallTotalHeight();
	const real32 wallStraightLength = GetStraightLength();

	if(wallTotalHeight<=kRealEpsilon)
		return;

	if(wallStraightLength<=kRealEpsilon)
		return;

	BooleanPolygon booleanPoly;
	// Add the holes for the SubObjects
	const int32 objectCount = fSubObjects.GetElemCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		const TMCCountedPtrArray<OutlinePoint>& outline = fSubObjects[iObj]->GetOutline();
		const int32 holePointCount = outline.GetElemCount();
		FlatPolygon polygon;
		polygon.SetElemCount(holePointCount);
		for( int32 iHolePt=0 ; iHolePt<holePointCount ; iHolePt++ )
		{
			polygon[iHolePt] = outline[iHolePt]->Position();
		}
		polygon.mIsHole = true;
		booleanPoly.AddPolygon( polygon );
	}
	// Add custom shapes for specific walls
	AddExtraPolygones( booleanPoly );
	
	// n rectangular contours, to do 1 by 1
	TMCClassArray<FlatMesh> flatTessellation;
	TMCClassArray<PolygonSet> polygonSetResult;

	polygonSetResult.SetElemCount(segmentCount);
	flatTessellation.SetElemCount(segmentCount);

	FlatPolygon polygon;
	polygon.mIsHole = false;
	polygon.SetElemCount(4);
	// The 2 first point
	polygon[0].x = 0;
	polygon[0].y = wallTotalHeight;
	polygon[1].x = 0;
	polygon[1].y = 0;
	polygon[2].y = 0;
	polygon[3].y = wallTotalHeight;
	const real32 lengthStep = wallStraightLength/segmentCount;
	for(int32 iContour=0 ; iContour<segmentCount ; iContour++)
	{
		const int32 polyIndex0 = 2*(iContour+1);
		const int32 polyIndex1 = polyIndex0 + 1;
		const real32 curLength = (iContour+1)*lengthStep;
		polygon[2].x = curLength;
		polygon[3].x = curLength;

		// Add the contour
		booleanPoly.AddPolygon( polygon );

		// Get the polygons and the tesselation
		booleanPoly.GetPolygons(polygonSetResult[iContour], true, flatTessellation[iContour]);

		// Remove the contour
		booleanPoly.CleanUpContours();

		// Move to the next point
		polygon[0].x = curLength;
		polygon[1].x = curLength;
	}

	// 3. Build the mesh in 3D

	// Compute these values only once
	const real32 leftHeight = GetLeftHeight();
	const real32 rightHeight = GetRightHeight();

	// Add the 2 sides of the wall
	const int32 flatMeshCount = flatTessellation.GetElemCount();
	for(int32 iFlatMesh=0 ; iFlatMesh<flatMeshCount ; iFlatMesh++)
	{
		AddFlatMeshToTessellation( flatTessellation[iFlatMesh], iFlatMesh, leftHeight, rightHeight );
	}

	// Special case for walls without thickness
	const boolean thickWall = (GetThickness()>kRealEpsilon);

	if(thickWall)
	{	// Add the parts in between
		
		if( mUnfoldUVData.mPrevWall )
			mUnfoldUVData.mPolygonBetweenOffset = mUnfoldUVData.mPrevWall->UnfoldUVData().mPolygonBetweenOffset;
		else
			mUnfoldUVData.mPolygonBetweenOffset = 0;

		const int32 polySetCount = polygonSetResult.GetElemCount();
		for(int32 iPolySet=0 ; iPolySet<polySetCount ; iPolySet++)
		{
			AddPolySetToTessellation( polygonSetResult[iPolySet], iPolySet, leftHeight, rightHeight );
		}
	}

	SetFlag(eWallTessellated);

	// Then use the roofs to cut the upper part of the wall
	fLevel->LevelPlan().CutWallWithRoofs(this);

	// Invalidate and fill the bbox
	const int32 vtxCount = fVertexArray.GetElemCount();
	if(vtxCount)
	{
		fWallBBox.SetMin(fVertexArray[0].Position());
		fWallBBox.SetMax(fVertexArray[0].Position());
		for(int32 iVtx=1 ; iVtx<vtxCount ; iVtx++)
		{
			fWallBBox.AddPoint(fVertexArray[iVtx].Position());
		}
	}
	else
		fWallBBox.Init();
}

TVector2 Wall::ComputeUV(const TVector2& pos, bool left)
{
	switch( fData->UVData().mMethod )
	{
	default:
	case eProportional: return fData->UVData().ComputeUV( pos );
	case eUnfold:
		{
			if( left )
			{
				return fData->UVData().ComputeUV( TVector2(
					pos.x + mUnfoldUVData.mOffset, 
					pos.y + fLevel->GetDistanceToGround() ) );
			}
			else
			{
				return fData->UVData().ComputeUV( TVector2(
					pos.x + mUnfoldUVData.mOffset + GetStraightLength(), 
					pos.y + fLevel->GetDistanceToGround() ) ) ;
			}
		}
	}
}

void Wall::AddFlatMeshToTessellation( const FlatMesh& flatMesh, int32 segmentID, real32 leftHeight, real32 rightHeight  )
{
	ValidateArc();

	// x gives the absissa
	// y gives the altitude
	const real32 wallStraightLength = GetStraightLength();
	const real32 z = fLevel->GetDistanceToGround();

	const int32 vtxCount = flatMesh.mVertices.GetElemCount();
	const int32 prevVtxCount = fVertexArray.GetElemCount();

	// Special case for walls without thickness
	const boolean thickWall = (GetThickness()>kRealEpsilon);

	bool drawLeft = true;
	bool drawRight = true;

	if(thickWall)
	{
		fVertexArray.SetElemCount(prevVtxCount + 2 * vtxCount);
	}
	else
	{
		fVertexArray.SetElemCount(prevVtxCount + vtxCount);
		if(fRoom0) drawLeft = false;
		else drawRight = false;
	}

	real32 minLeftHeight = 0;
	if(fRoom0) // left room
		minLeftHeight = fRoom0->GetFloorThickness();

	real32 minRightHeight = 0;
	if(fRoom1) // left room
		minRightHeight = fRoom1->GetFloorThickness();

	// Fill in the vertices
	int32 curIndex = prevVtxCount;
	for( int32 iFlatVtx=0 ; iFlatVtx<vtxCount ; iFlatVtx++ )
	{
		const real32 curX = flatMesh.mVertices[iFlatVtx].x;
		const real32 curY = flatMesh.mVertices[iFlatVtx].y;

		real32 percent = curX/wallStraightLength;
		real32 leftAbscissa = 0;
		real32 rightAbscissa = 0;
		TVector2 normal2D;
		TVector2 direction2D;
		TVector2 position2D;
		TVector2 leftPos2D;
		TVector2 rightPos2D;
		mCircleArc.GetPosAroundArc(percent, 
							segmentID,
							leftAbscissa,
							rightAbscissa,
							position2D,
							leftPos2D, 
							rightPos2D,
							normal2D, 
							direction2D,
							true);		

		TVector2 flatXY(curX, curY);
		
		TVector3 normal3D;
		if(mSmooth)
		{	// Smooth: build the circle normal
			const TVector2& center = mCircleArc.GetCenter();
			TVector2 dir = position2D - center;
			dir.Normalize();
			normal3D.SetValues( dir.x, dir.y, 0);
		}
		else
		{	// Sharp: use normal
			normal3D.SetValues( normal2D.x, normal2D.y, 0);
		}

		// Left
		if(drawLeft)
		{
			TVector3 leftPosition3D(leftPos2D.x, leftPos2D.y, z+MC_Clamp( curY, minLeftHeight, leftHeight ) );
			fVertexArray[curIndex].SetPosition(leftPosition3D);
			fVertexArray[curIndex].SetNormal(-normal3D);
			fVertexArray[curIndex].SetUV(ComputeUV(TVector2(leftAbscissa, curY), true));
			curIndex++;
		}
		
		if(drawRight)
		{	// Right
			TVector3 rightPosition3D(rightPos2D.x, rightPos2D.y, z+MC_Clamp( curY, minRightHeight, rightHeight ) );
			fVertexArray[curIndex].SetPosition(rightPosition3D);
			fVertexArray[curIndex].SetNormal(normal3D);
			fVertexArray[curIndex].SetUV(ComputeUV(TVector2(rightAbscissa, curY), false));
			curIndex++;
		}
	}

	// fill in the triangles
	const int32 tglCount = flatMesh.mTriangles.GetElemCount();
	const int32 prevTglCount = fTriangleArray.GetElemCount();

	if(thickWall)
	{
		fTriangleArray.SetElemCount(prevTglCount + 2*tglCount);
		fTriangleDomain.SetElemCount(prevTglCount + 2*tglCount);
	}
	else
	{
		fTriangleArray.SetElemCount(prevTglCount + tglCount);
		fTriangleDomain.SetElemCount(prevTglCount + tglCount);
	}
	curIndex = prevTglCount;
	for( int32 iTgl=0 ; iTgl<tglCount ; iTgl++ )
	{
		if(thickWall)
		{	
			// Left
			fTriangleArray[curIndex].pt1 = 2*flatMesh.mTriangles[iTgl].pt1 + prevVtxCount;
			fTriangleArray[curIndex].pt2 = 2*flatMesh.mTriangles[iTgl].pt2 + prevVtxCount;
			fTriangleArray[curIndex].pt3 = 2*flatMesh.mTriangles[iTgl].pt3 + prevVtxCount;
			fTriangleDomain[curIndex] = fLeftDomain;
			curIndex++;
			// Right
			fTriangleArray[curIndex].pt3 = 2*flatMesh.mTriangles[iTgl].pt1+1 + prevVtxCount;
			fTriangleArray[curIndex].pt2 = 2*flatMesh.mTriangles[iTgl].pt2+1 + prevVtxCount;
			fTriangleArray[curIndex].pt1 = 2*flatMesh.mTriangles[iTgl].pt3+1 + prevVtxCount;
			fTriangleDomain[curIndex] = fRightDomain;
			curIndex++;
		}
		else
		{
			fTriangleArray[curIndex].pt1 = flatMesh.mTriangles[iTgl].pt1 + prevVtxCount;
			fTriangleArray[curIndex].pt2 = flatMesh.mTriangles[iTgl].pt2 + prevVtxCount;
			fTriangleArray[curIndex].pt3 = flatMesh.mTriangles[iTgl].pt3 + prevVtxCount;
			fTriangleDomain[curIndex] = fLeftDomain;
			curIndex++;
		}
	}
}

void Wall::AddPolySetToTessellation( const PolygonSet& polyset, int32 segmentID, real32 leftHeight, real32 rightHeight  )
{
	const int32 polygonCount = polyset.GetElemCount();
	for(int32 iPolygon=0 ; iPolygon<polygonCount ; iPolygon++)
	{
		AddPolygonToTessellation( polyset[iPolygon], segmentID, leftHeight, rightHeight  );
	}
}

void Wall::AddPolygonToTessellation( const FlatPolygon& polygon, int32 segmentID, real32 leftHeight, real32 rightHeight  )
{
	// x gives the absissa
	// y gives the altitude
	const real32 wallStraightLength = GetStraightLength();
	const real32 z = fLevel->GetDistanceToGround();
	const real32 wallThickness = GetThickness();

	const int32 vtxCount = polygon.GetElemCount();
	// polygon.mIsHole determine the orientation of the normal

	if(vtxCount<3)
		return;

	ValidateArc();

	real32 minLeftHeight = 0;
	if(fRoom0) // left room
		minLeftHeight = fRoom0->GetFloorThickness();

	real32 minRightHeight = 0;
	if(fRoom1) // left room
		minRightHeight = fRoom1->GetFloorThickness();

	// Prepare the points in 3D
	TMCClassArray<TVector3> leftPos3D;
	leftPos3D.SetElemCount(vtxCount);
	TMCClassArray<TVector3> rightPos3D;
	rightPos3D.SetElemCount(vtxCount);
	TMCClassArray<TVector3> tangent3D;
	tangent3D.SetElemCount(vtxCount);
	TMCClassArray<TVector2> leftUVs;
	leftUVs.SetElemCount(vtxCount);
	TMCClassArray<TVector2> rightUVs;
	rightUVs.SetElemCount(vtxCount);
	{
		TVector2 prevVtx = polygon[0];
		real32 polygonLength = 0;

		for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
		{
			const TVector2& curVtx = polygon[iVtx];

			real32 percent = curVtx.x/wallStraightLength;
			real32 leftAbscissa = 0;
			real32 rightAbscissa = 0;
			TVector2 normal2D;
			TVector2 direction2D;
			TVector2 position2D;
			TVector2 leftPos2D;
			TVector2 rightPos2D;
			mCircleArc.GetPosAroundArc( percent, 
										segmentID,
										leftAbscissa,
										rightAbscissa,
										position2D,
										leftPos2D, 
										rightPos2D,
										normal2D, 
										direction2D,
										true);		

			tangent3D[iVtx].SetValues(-normal2D.y, normal2D.x, 0);

			// Left
			const real32 leftZOffset =  z+MC_Clamp( curVtx.y, minLeftHeight, leftHeight ) ;
			leftPos3D[iVtx].SetValues(leftPos2D.x, leftPos2D.y, leftZOffset );
			// Right
			const real32 rightZOffset =  z+MC_Clamp( curVtx.y, minRightHeight, rightHeight ) ;
			rightPos3D[iVtx].SetValues(rightPos2D.x, rightPos2D.y, rightZOffset );
			
			// UVs in between
			if( fData->UVData().mMethod == eProportional )
			{	// I think these offsets were done to get some continuity between the levels
				leftUVs[iVtx].SetValues(leftAbscissa/*curVtx.x + leftZOffset*/, z + minLeftHeight );
				rightUVs[iVtx].SetValues(rightAbscissa/*curVtx.y + rightZOffset*/, z + minRightHeight + wallThickness );
			}
			else
			{
				polygonLength+= (prevVtx - curVtx).GetNorm();
				leftUVs[iVtx].SetValues(-(polygonLength + mUnfoldUVData.mPolygonBetweenOffset), z );
				rightUVs[iVtx].SetValues(-(polygonLength + mUnfoldUVData.mPolygonBetweenOffset), z + wallThickness );

				prevVtx = curVtx;
			}
		}

		mUnfoldUVData.mPolygonBetweenOffset += polygonLength;
	}

	// Compute the normals
	TMCClassArray<TVector3> normal3D;
	normal3D.SetElemCount(vtxCount);
	{
		TVector2 prevPos = polygon[vtxCount-1];
		for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
		{
			TVector2 curPos = polygon[iVtx];
			
			// Direction
			TVector2 flatDir = prevPos-curPos;
			flatDir.Normalize();

			// Normal
			float flatYNormal = polygon.mIsHole?flatDir.x:-flatDir.x;

			// Convert to 3D space
			float zNormal = flatYNormal;

			float coeff = sqrt(1 - zNormal*zNormal);

			if (prevPos.y<curPos.y)
			{	// Flip the orientation
				coeff *= -1;
			}

			normal3D[iVtx].x = tangent3D[iVtx].x * coeff;
			normal3D[iVtx].y = tangent3D[iVtx].y * coeff;
			normal3D[iVtx].z = zNormal;

			prevPos = curPos;
		}
	}

	// Build the triangles
	const int32 prevVtxCount = fVertexArray.GetElemCount();
	fVertexArray.SetElemCount(prevVtxCount + 4 * vtxCount);

	const int32 prevTglCount = fTriangleArray.GetElemCount();
	fTriangleArray.SetElemCount(prevTglCount + 2*vtxCount);
	fTriangleDomain.SetElemCount(prevTglCount + 2*vtxCount);

	int32 prevFace = vtxCount-1;
	for(int32 iFace=0 ; iFace<vtxCount ; iFace++)
	{
		// Build a rectangle: 4 vertices
		const int32 curVtxIndex = prevVtxCount + 4*iFace;
		const int32 curTglIndex = prevTglCount + 2*iFace;
		
		// Point 1
		fVertexArray[curVtxIndex].SetPosition(leftPos3D[prevFace]);
		fVertexArray[curVtxIndex].SetNormal(normal3D[iFace]);
		fVertexArray[curVtxIndex].SetUV(fData->UVData().ComputeUV(leftUVs[prevFace]));

		// Point 2
		fVertexArray[curVtxIndex+1].SetPosition(rightPos3D[prevFace]);
		fVertexArray[curVtxIndex+1].SetNormal(normal3D[iFace]);
		fVertexArray[curVtxIndex+1].SetUV(fData->UVData().ComputeUV(rightUVs[prevFace]));

		// Point 3
		fVertexArray[curVtxIndex+2].SetPosition(leftPos3D[iFace]);
		fVertexArray[curVtxIndex+2].SetNormal(normal3D[iFace]);
		fVertexArray[curVtxIndex+2].SetUV(fData->UVData().ComputeUV(leftUVs[iFace]));

		// Point 4
		fVertexArray[curVtxIndex+3].SetPosition(rightPos3D[iFace]);
		fVertexArray[curVtxIndex+3].SetNormal(normal3D[iFace]);
		fVertexArray[curVtxIndex+3].SetUV(fData->UVData().ComputeUV(rightUVs[iFace]));

		// Triangle 1
		fTriangleArray[curTglIndex].pt1 = curVtxIndex;
		fTriangleArray[curTglIndex].pt2 = curVtxIndex+1;
		fTriangleArray[curTglIndex].pt3 = curVtxIndex+2;
		fTriangleDomain[curTglIndex] = fBetweenDomain;

		// Triangle 2
		fTriangleArray[curTglIndex+1].pt1 = curVtxIndex+3;
		fTriangleArray[curTglIndex+1].pt2 = curVtxIndex+2;
		fTriangleArray[curTglIndex+1].pt3 = curVtxIndex+1;
		fTriangleDomain[curTglIndex+1] = fBetweenDomain;

		// Move to the next point
		prevFace = iFace;
	}
}

void Wall::ValidateTessellation()
{
	if( Flag(eWallTessellated) ) 
		return; // Tessellation is valid

	// 3D tessellation
	ValidateTessellation2();

	// Construction points
	{
		// Clean up
		mConstrPoints.DeleteAndRemoveAll();

		const TMCClassArray<TVector2>& arcPos = mCircleArc.GetArc();
		const int32 arcCount = arcPos.GetElemCount();

		mConstrPoints.SetElemCount(arcCount);
		mConstrPoints.SetElem( 0, new PointConstrPoint(fPoint0) );
		for(int32 iPt=1 ; iPt<arcCount-1 ; iPt++ )
		{
			mConstrPoints.SetElem( iPt, new WallConstrPoint(this, arcPos[iPt]));
		}

		// Last one is the last point
		mConstrPoints.SetElem( arcCount-1, new PointConstrPoint(fPoint1) );
	}

	// Projection position (left and right sides of the wall)
	{
		const int32 pointCount = mCircleArc.GetPointCount();

		mLeftPos.SetElemCount(pointCount);
		mRightPos.SetElemCount(pointCount);

		// Fill in the vertices
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			real32 percent = (real32)iPoint/(real32)(pointCount-1);
			real32 leftAbscissa = 0;
			real32 rightAbscissa = 0;
			TVector2 normal2D;
			TVector2 direction2D;
			TVector2 position2D;
			mCircleArc.GetPosAroundArc(percent, 
								iPoint-1, // segmentID,
								leftAbscissa,
								rightAbscissa,
								position2D,
								mLeftPos[iPoint], 
								mRightPos[iPoint],
								normal2D, 
								direction2D,
								true);	
		}
	}

	// MY_ASSERT(CheckVertexPos(fVertexArray));
}

void Wall::ComputeArcSegmentCount()
{
	real32 offset = GetArcOffset();
	if(offset == 0 )
		SetArcSegmentCount(1);
	else
		SetArcSegmentCount(1 + 20 * RealAbs( offset )/GetStraightLength());
}

TVector2 Wall::GetMidPointPos()
{
	TVector2 dir = GetStraightDirection(fPoint0);
	TVector2 normal(dir.y,-dir.x);

	return fPoint0->Position() + (0.5*GetStraightLength()*dir) + GetActualArcOffset()*normal;
}

void Wall::GetWallData(	const TVector2& flatPos,
						bool clamp,
						TVector2& position2D,
						TVector2& leftPos2D,
						TVector2& rightPos2D,
						TVector2& normal2D,
						TVector2& direction2D )
{
	// Project the flatPos on the wall to get the staight length from
	// the 1st point

	TVector2 projectedPos;
	Project(flatPos, fPoint0->Position(), fPoint1->Position(), projectedPos	);

	TVector2 posDir = projectedPos-fPoint0->Position();
	real32 curX = posDir.GetMagnitude();
	if(posDir*(fPoint1->Position()-fPoint0->Position()) < 0)
		curX*=-1;

	const real32 wallStraightLength = GetStraightLength();

	ValidateArc();

	const real32 percent = curX/wallStraightLength;
	real32 leftAbscissa = 0;
	real32 rightAbscissa = 0;
	mCircleArc.GetPosAroundArc(percent, 
						kUnusedIndex,
						leftAbscissa,
						rightAbscissa,
						position2D,
						leftPos2D, 
						rightPos2D,
						normal2D, 
						direction2D,
						clamp);
}



void Wall::SetWallHeight(const real32 h)
{
	if(h==fHeight)
		return;
	if(h==fData->GetDefaultWallHeight() )
	{
		if(fHeight==kDefaultLevelHeight)
			return;

		fHeight=kDefaultLevelHeight;
	}
	else
		fHeight=h;
	
	// Invalidation
	InvalidateTessellation(true);
}

void Wall::SetThickness(const real32 t)
{
	if(t==fThickness)
		return;
	if(t==fData->GetDefaultWallThickness() )
	{
		if(fThickness==kDefaultThickness)
			return;

		fThickness=kDefaultThickness;
	}
	else
		fThickness=t;

	// Invalidation
	InvalidateTessellation(true);
}

real32 Wall::GetLeftHeight() const
{
	const real32 wallTotalHeight = GetWallTotalHeight();
	if(fRoom0) // left room
	{
		if(!fRoom0->NoCeiling())
		{
			const real32 roomHeight = fRoom0->GetRoomHeight();
			if(wallTotalHeight==roomHeight)
				return wallTotalHeight-fRoom0->GetCeilingThickness();
			else
				return wallTotalHeight;
		}
	}

	return wallTotalHeight;
}

real32 Wall::GetRightHeight() const
{
	const real32 wallTotalHeight = GetWallTotalHeight();
	if(fRoom1) // right room
	{
		if(fRoom1->NoCeiling())
			return wallTotalHeight;
		else
		{
			const real32 roomHeight = fRoom1->GetRoomHeight();
			if(wallTotalHeight==roomHeight)
				return wallTotalHeight-fRoom1->GetCeilingThickness();
			else
				return wallTotalHeight;
		}
	}

	return wallTotalHeight;
}

void Wall::CheckRoomObjConflict(RoomSubObject* obj)
{
	// 1 fast check: check the distance to the center
	const real32 thickness = GetThickness();

	const TVector2& center = obj->GetPolylineCenter();
	const real32 halfWidth = .5*(obj->Get2DWidth() + thickness);
	const real32 halfHeight = .5*(obj->Get2DHeight() + thickness);
	const real32 sqrDist = halfWidth*halfWidth + halfHeight*halfHeight;

	const TVector2& pos0 = fPoint0->Position();
	const TVector2& pos1 = fPoint1->Position();
	const real32 dist = PointSegmentSqrDistance(center,pos0,pos1);

	if(dist<=sqrDist)
	{
		// 2 If near enough, check intersection
		const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
		const int32 ptCount = outline.GetElemCount();
		MY_ASSERT(ptCount);
		// Build an outside outline
		const TBBox2D& bbox = obj->GetHoleBBox();
		const real32 xMax = MC_Max(bbox.fMin.x, bbox.fMax.x);
		const real32 yMax = MC_Max(bbox.fMin.y, bbox.fMax.y);
		TMCClassArray<TVector2> largeOutline(ptCount);
		{
			for(int32 iPt=0 ; iPt<ptCount ; iPt++)
			{
				TVector2& newPos = largeOutline[iPt];
				newPos = outline[iPt]->Position();
				if(newPos.x == xMax) newPos.x+=thickness;
				else newPos.x-=thickness;
				if(newPos.y == yMax) newPos.y+=thickness;
				else newPos.y-=thickness;
			}
		}

		TVector2 intersections[2];
		int32 interCount=0;
		for(int32 iPt=0 ; iPt<ptCount ; iPt++)
		{
			if( GetSegmentsIntersection( intersections[interCount], pos0, pos1, // first segment
				largeOutline[iPt], largeOutline[(iPt+1)%ptCount] ) == eIntersection) // second segment
			{
				interCount++;
				if(interCount==2)
				{
					if(intersections[0] == intersections[1])
						interCount--; // we're going through a corner
					else
						break; // we've got 2 intersections, that's enough
				}
			}
		}

		if(interCount==1)
		{	// 1 intersection, move the point out
			const TVector2 pos(intersections[0].x, intersections[0].y );
			if( obj->PointIn(pos0) && fPoint0->Selected())// If in, should always be selected, but check it just in case
				fPoint0->SetPosition(pos);
			else if(fPoint1->Selected())// If in, should always be selected, but check it just in case
				fPoint1->SetPosition(pos);
		}
		else if(interCount==2)
		{	// 2 intersections
			if(fPoint0->Selected() && fPoint1->Selected())
			{
				// Find the nearest possible corner=>min(max on each side)
				const TVector2 inter = intersections[1]-intersections[0];
				real32 maxLeft=0;
				real32 maxRight=0;
				TVector2 leftCorner=TVector2::kZero;
				TVector2 rightCorner=TVector2::kZero;
				for(int32 i=0 ; i<ptCount ; i++)
				{
					if( ((largeOutline[i]-intersections[0])^inter)>0 )
					{	// one side
						const real32 dist = PointLineSqrDistance(largeOutline[i],intersections[0],intersections[1]);
						if(dist>maxLeft)
						{
							maxLeft=dist;
							leftCorner = largeOutline[i];
						}
					}
					else
					{	// the other side
						const real32 dist = PointLineSqrDistance(largeOutline[i],intersections[0],intersections[1]);
						if(dist>maxRight)
						{
							maxRight=dist;
							rightCorner = largeOutline[i];
						}
					}
				}

				TVector2 corner;
				if(maxLeft<maxRight)
					corner=leftCorner;
				else
					corner=rightCorner;

				// Then move the points
				TVector2 offset;
				Project(corner,intersections[0],intersections[1], offset);
				offset-=corner;

				fPoint0->OffsetPosition(offset);
				fPoint1->OffsetPosition(offset);

			}
			// It would be even better to send it in the best corner
			else if(fPoint0->Selected())
			{	// we can move only one point: move it to the nearest position to the other
				if((pos0-intersections[0]).GetSquaredNorm() < (pos0-intersections[1]).GetSquaredNorm())
				{
					const TVector2 pos(intersections[0].x, intersections[0].y );
					fPoint0->SetPosition(pos);
				}
				else
				{
					const TVector2 pos(intersections[1].x, intersections[1].y );
					fPoint0->SetPosition(pos);
				}
			}
			else if(fPoint1->Selected())
			{	// we can move only one point: move it to the nearest position to the other
				if((pos1-intersections[0]).GetSquaredNorm() < (pos1-intersections[1]).GetSquaredNorm())
				{
					const TVector2 pos(intersections[0].x, intersections[0].y );
					fPoint1->SetPosition(pos);
				}
				else
				{
					const TVector2 pos(intersections[1].x, intersections[1].y );
					fPoint1->SetPosition(pos);
				}
			}
		}

	}
}

void Wall::ShowWall()
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
	// Show the points, even if the room aren't visible
	fPoint0->ClearFlag(eIsHidden);
	fPoint1->ClearFlag(eIsHidden);
}

void Wall::HideWall()
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
	// Don't touch the point, the rooms could be still visible
}

void Wall::InvalidateTessellation(const boolean invalidateAround)
{
	if(invalidateAround)
	{
		// Invalidate points
		fPoint0->InvalidateTessellation(false);
		fPoint1->InvalidateTessellation(false);
	}
	else
	{
		fPoint0->ClearFlag(ePointTessellated);
		fPoint1->ClearFlag(ePointTessellated);
		ClearFlag(eWallTessellated);
		ClearFlag(eWallBaseIsValid);
		// Invalidate the objects 3D data
		const int32 objectCount = fSubObjects.GetElemCount();
		for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
		{	// Note: we shouldn't need to invalidate the 2D center pos here, but it fix a bug when scaling an object (they somtimes went crazy)
			fSubObjects[iObj]->Invalidate();
		}
	}
}

boolean Wall::CheckConsistency(const boolean canDelete)
{
	if( fLevel )
		return fLevel->LevelPlan().CheckWallConsistency( this, 0, canDelete );

	return false;
}

void Wall::SetLevel(Level* level) {fLevel = level;}
void Wall::SetData(BuildingPrimData* data) {fData = data;}

BuildingPrim* Wall::GetBuildingPrim() const 
{
	return fLevel->GetPrimitiveNoAddRef();
}

boolean PosUnderGeom(const TVector3& pos, const TMCArray<TriangleVertices>& cutterTriangles)
{
	const int32 tglCount = cutterTriangles.GetElemCount();

	TVector3 hitPosition;
	real64 alpha=0;
	for(int32 iTgle=0 ; iTgle<tglCount ; iTgle++)
	{		
		const TriangleVertices& vtx = cutterTriangles[iTgle];

		if( BasicTrianglePick( vtx.fVertices[0], vtx.fVertices[1], vtx.fVertices[2], alpha, pos, TVector3::kUnitZ, hitPosition ) ) 
			return true;
	}

	return false;
}

void Wall::CutGeometry(const TMCArray<TriangleVertices>& roofTriangles, const real32 roofMin, const real32 roofMax)
{
	// Use the passed in triangles to remove all the upper part of the wall

	if(ExtraHeight())
	{	// We want a wall that reach the roof: if the wall or part of it is under the roof, 
		// increase the height of the wall before cutting it
		const TVector3& pos0 = fPoint0->Get3DPos();
		const TVector3& pos1 = fPoint1->Get3DPos();
		// We don't use the middle point, it's too often on the limit
		const TVector3 dir = pos1-pos0;
		const TVector3 ptOne = pos0+.14f*dir;
		const TVector3 ptTwo = pos0+.16f*dir;
		const TVector3 ptThree = pos0+.84f*dir;
		const TVector3 ptFour = pos0+.86f*dir;
		boolean firstSide=false;
		boolean secondSide=false;
		if( PosUnderGeom(ptOne, roofTriangles) || PosUnderGeom(ptTwo, roofTriangles))
			firstSide=true;
		if( PosUnderGeom(ptThree, roofTriangles) || PosUnderGeom(ptFour, roofTriangles))
			secondSide=true;

		if(firstSide&&secondSide)
		{
			// Find all the vertices of the upper part of the wall and send them higher
		
			const real32 altitude = fLevel->GetDistanceToGround();

			const real32 leftHeight = GetLeftHeight() + altitude;
			const real32 rightHeight = GetRightHeight() + altitude;
			const real32 heightLimit = MC_Min(leftHeight, rightHeight );

			TMCClassArray<Vertex>& vertices = Vertices();
			const int32 vertexCount = vertices.GetElemCount();
			for( int32 iVertex=0 ; iVertex<vertexCount ; iVertex++ )
			{
				real32& zValue = vertices[iVertex].ZValue();
				if(zValue>=heightLimit)
				{
					// Need also to modify the UV value so the PCutter can compute the right new UVs
					vertices[iVertex].SetUV( vertices[iVertex].UV() + TVector2( 0, (roofMax-zValue)/fData->UVData().mDefaultUVLength) );

					// Set the new Z value
					zValue = roofMax;
				}
			}
		}
	}

	const TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = vertices.GetElemCount();
	TMCClassArray<Vertex>  newVertices;

	const TMCClassArray<Triangle>& triangles = Triangles();
	const int32 triangleCount = triangles.GetElemCount();
	TMCClassArray<Triangle> newTriangles;
	newTriangles.SetElemSpace(triangleCount); // Minimum space used (about)
	TMCArray<int32> newDomains;
	newDomains.SetElemSpace(triangleCount); // Minimum space used (about)

	// Cut the triangles
	int32 prevCount = 0;
	for(int32 iTgle=0 ; iTgle<triangleCount ; iTgle++)
	{
		const int32 domainID = fTriangleDomain[iTgle];
		CutTriangle(triangles[iTgle], vertices, fVertexArray.GetElemCount(),
			newTriangles, newVertices, roofTriangles, roofMin, roofMax);

		const int32 currentCount = newTriangles.GetElemCount();

		const int32 diff = currentCount - prevCount;
		TMCArray<int32> domain(diff,false);
		for(int32 iElem=0 ; iElem<diff ; iElem++)
			domain[iElem] = domainID;
		newDomains.Append( domain );

		prevCount = currentCount;
	}

	// Replace the arrays
	fVertexArray.Append(newVertices); // Add the new vertices (we keep the old one because they're used elsewhere)
	fTriangleArray = newTriangles; // Replace the triangles
	fTriangleDomain = newDomains; // Replace the domains
	MY_ASSERT(CheckTriangles(fTriangleArray,fVertexArray));
}

///////////////////////////////////////////////////////////////////////////
//
//

MCCOMErr Wall::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Wall');
	if (result) return result;

	// Thickness
	result=stream->PutKeyword('Thic');
	if (result) return result;
	result=stream->PutQuickFix(fThickness);
	if (result) return result;

	// Height
	result=stream->PutKeyword('Heig');
	if (result) return result;
	result=stream->PutQuickFix(fHeight);
	if (result) return result;

	// Arc offset and segment count
	result=stream->PutKeyword('ArcO');
	if (result) return result;
	result=stream->PutQuickFix(GetArcOffset());
	if (result) return result;
	stream->PutInt32Attribute('ArcS', GetArcSegmentCount());

	// Domains
	stream->PutInt32Attribute('LDom', fLeftDomain);
	stream->PutInt32Attribute('RDom', fRightDomain);
	stream->PutInt32Attribute('BDom', fBetweenDomain);

	// Common
	result=CommonBase::Write(stream);
	if (result) return result;

	//	Point indices
	int32 indices[2];
	indices[0]=fPoint0->GetIndex();
	if(indices[0]<0)
	{
		MCNotify("Database corrupted!");
		indices[0]=0;
	}
	indices[1]=fPoint1->GetIndex();
	if(indices[1]<0)
	{
		MCNotify("Database corrupted!");
		indices[1]=0;
	}
	stream->PutInt32ArrayAttribute('Inde', 2,indices);

	// Objects
	const int32 objCount = fSubObjects.GetElemCount();
	for(int32 iObj=0 ; iObj<objCount ; iObj++)
	{
		fSubObjects[iObj]->Write(stream);
	}

	result=stream->PutEnd();
	return result;
}

MCCOMErr Wall::Read(IShTokenStream* stream)
{ 
	int8 token[256];

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
			case 'Thic':
			{
				result = stream->GetQuickFix(&fThickness);
				if (result) return result;
			} break;
			case 'Heig':
			{
				result = stream->GetQuickFix(&fHeight);
				if (result) return result;
			} break;
			case 'ArcO':
			{
				real32 arcOffset = 0;
				result = stream->GetQuickFix(&arcOffset); 
				if (result) return result;
				SetArcOffset( arcOffset );
			} break;
			case 'ArcS':
			{
				int32 arcSegments = stream->GetInt32Token();
				SetArcSegmentCount( arcSegments );
			} break;
			case 'LDom':
			{
				fLeftDomain = stream->GetInt32Token();
			} break;
			case 'RDom':
			{
				fRightDomain = stream->GetInt32Token();
			} break;
			case 'BDom':
			{
				fBetweenDomain = stream->GetInt32Token();
			} break;
			case 'Inde':
			{
				int32 indices[2];
				stream->GetInt32ArrayToken(2, indices);

				fPoint0 = fLevel->GetPoint(indices[0]);
				fPoint1 = fLevel->GetPoint(indices[1]);

				if(!fPoint0)
				{	// Error, but avoid crash
					fPoint0 = fLevel->GetPoint(0);
				}
				if(!fPoint1)
				{	// Error, but avoid crash
					fPoint1 = fLevel->GetPoint(1);
				}

				if(fPoint0)
					fPoint0->AddWallReference(this);

				if(fPoint1)
					fPoint1->AddWallReference(this);
			} break;
			case 'WObj':
			{
				WallSubObject* newObject = NULL;
				WallSubObject::CreateWallSubObject(&newObject, this, GetBuildingPrim());
				newObject->Read(stream);
			} break;

			default:
				CommonBase::Read(stream,keyword,fData);
				break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	// Invalidate tessellation
	ClearFlag(eWallTessellated);
	ClearFlag(eWallBaseIsValid);
	ClearFlag(eIsTargeted);

	return result;
}

/////////////////////////////////////////////////////////////////////////////

void Wall::Clone(Wall** newWall, Level* inLevel, VPoint* point0, VPoint* point1, const ECloneChildrenMode cloneMode)
{
	Wall::CreateWall(newWall, inLevel->GetPrimitiveData(), inLevel,
		point0, point1);

	(*newWall)->CopyFrom( this, cloneMode );
}

void Wall::CopyFrom( Wall* otherWall, const ECloneChildrenMode cloneMode )
{
	// Copy the thickness and height and flags
	SetThickness(otherWall->fThickness);
	SetWallHeight(otherWall->fHeight);
	SetArcOffset(otherWall->GetArcOffset());
	SetArcSegmentCount(otherWall->GetArcSegmentCount());
	SetFlags(otherWall->fFlags);
	SetNamePtr(otherWall->fName);

	SetLeftDomain(otherWall->fLeftDomain);
	SetRightDomain(otherWall->fRightDomain);
	SetBetweenDomain(otherWall->fBetweenDomain);

	// Clone Wall Objects
	const int32 objCount = otherWall->fSubObjects.GetElemCount();
	for(int32 iObj=0 ; iObj<objCount ; iObj++)
	{
		TMCCountedPtr<WallSubObject> newObj;
		otherWall->fSubObjects[iObj]->Clone(&newObj, this, cloneMode);
	}

	// Invalidate the tessellation
	ClearFlag(eWallTessellated);
	ClearFlag(eWallBaseIsValid);

	ClearFlag(eIsTargeted);
	ClearFlag(eWallHelper);
}

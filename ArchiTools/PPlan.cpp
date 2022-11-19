/****************************************************************************************************

		PPlan.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PPlan.h"

#include "MCCountedPtrHelper.h"
#include "IShTokenStream.h"
#include "Vector2.h"

#include "PWall.h"
#include "PWallWithCrenel.h"
#include "PRoom.h"
#include "PPoint.h"
#include "PVertex.h"
#include "PLevel.h"

Plan::Plan(Level* onLevel)
{
	fLevel = onLevel;
}

Plan::~Plan()
{
	DeletePlan();
}

void Plan::DeletePlan()
{
	fLevel = NULL;
}

void Plan::RemovePointReference(VPoint* point)
{
	// Find the point and remove it from the array
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		if(point == fPointArray[iPoint])
		{
			if(iPoint == pointCount-1) // Last elem, just remove it
				fPointArray.RemoveElem(iPoint,1);
			else
			{
				fPointArray.SetElem( iPoint, fPointArray[pointCount-1] );
				fPointArray.RemoveElem(pointCount-1,1);
			}

			break;
		}
	}

//	RemoveLevelIfEmpty();
}
/*
void Plan::RemovePointFromRoofs(Point* point)
{
	// Find if the point is used in one or several roof
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		int32 replacedCount=0;

		Roof* roof = fRoofArray[iRoof];

		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
		{
			ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

			if(zoneSection.fLevelPoint == point)
			{
				int32 prev = iPt-1;
				if(prev<0) prev = zoneSectionCount-1;
				zoneSection.fLevelPoint = roof->GetRoofZoneSection(prev).fLevelPoint;
				replacedCount++;
				roof->ClearFlag(eRoofTessellated);
			}
		}

		if(replacedCount == zoneSectionCount)
		{	// All the roof zoneSection reference the same point that going to be removed => set NULL
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				roof->GetRoofZoneSection(iPt).fLevelPoint = NULL;
			}
		}
	}
}
*/
/*
void Plan::ReplacePointReferenceInRoofs(Point* replacedPoint, Point* replaceBy)
{
	// Find if the point is used in one or several roof
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		int32 replacedCount=0;

		Roof* roof = fRoofArray[iRoof];

		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
		{
			ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

			if(zoneSection.fLevelPoint == replacedPoint)
			{
				zoneSection.fLevelPoint = replaceBy;
				roof->ClearFlag(eRoofTessellated);
			}
		}
	}
}
*/
void Plan::RemoveWallReference(Wall* wall)
{
	// Find the wall and remove it from the array
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		if(wall == fWallArray[iWall])
		{
			if(iWall == wallCount-1) // Last elem, just remove it
				fWallArray.RemoveElem(iWall,1);
			else
			{
				fWallArray.SetElem( iWall, fWallArray[wallCount-1] );
				fWallArray.RemoveElem(wallCount-1,1);
			}

			break;
		}
	}

//	RemoveLevelIfEmpty();
}
void Plan::RemoveRoomReference(Room* room)
{
	// Find the room and remove it from the array
	const int32 roomCount = fRoomArray.GetElemCount();

	boolean found =false;

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		if(room == fRoomArray[iRoom])
		{
			if(iRoom == roomCount-1) // Last elem, just remove it
			{
				fRoomArray.RemoveElem(iRoom,1);
				found = true;
			}
			else
			{
				fRoomArray.SetElem( iRoom, fRoomArray[roomCount-1] );
				fRoomArray.RemoveElem(roomCount-1,1);
				found = true;
			}

			break;
		}
	}

	MY_ASSERT(found);

	MY_ASSERT(!HasRoomReference(room));
//	RemoveLevelIfEmpty();
}

boolean Plan::HasRoomReference(Room* room)
{
	// Search in rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		if(room == fRoomArray[iRoom])
		{
			MCNotify("Room ref in room array");
			return true;
		}
	}

	// Search in walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if(wall->GetRoom(0) == room)
		{
			MCNotify("Room ref in wall array");
			return true;
		}
		if(wall->GetRoom(1) == room)
		{
			MCNotify("Room ref in wall array");
			return true;
		}
	}

	return false;
}

void Plan::RemoveRoofReference(Roof* roof)
{
	// Find the wall and remove it from the array
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		if(roof == fRoofArray[iRoof])
		{
			if(iRoof == roofCount-1) // Last elem, just remove it
				fRoofArray.RemoveElem(iRoof,1);
			else
			{
				fRoofArray.SetElem( iRoof, fRoofArray[roofCount-1] );
				fRoofArray.RemoveElem(roofCount-1,1);
			}

			break;
		}
	}

//	RemoveLevelIfEmpty();
}

void Plan::RemoveLevelIfEmpty()
{
	if( fRoofArray.GetElemCount() == 0 &&
		fRoomArray.GetElemCount() == 0 &&
		fWallArray.GetElemCount() == 0 &&
		fPointArray.GetElemCount() == 0 &&
		fLevel->GetDistanceToGround() != 0 ) // always keep 1 level
	{
		fLevel->DeleteLevel();
	}
}

Level* Plan::GetLevelOver()
{
	return fLevel->GetLevelOver();
}

Level* Plan::GetLevelUnder()
{
	return fLevel->GetLevelUnder();
}

const real32 Plan::GetDistanceToGround()const
{
	return fLevel->GetDistanceToGround();
}

const real32 Plan::GetLevelHeight()const
{
	return fLevel->GetLevelHeight();
}

const int32 Plan::GetLevelIndex()const
{
	return fLevel->GetLevelIndex();
}

void Plan::MergePlan(Plan* plan)
{
	// Replace the level and data pointer in this objs
	BuildingPrimData* data = fLevel->GetPrimitiveData();

	{
		const int32 appRoofCount = plan->fRoofArray.GetElemCount();
		for(int32 iRoof=0 ; iRoof<appRoofCount ; iRoof++)
		{
			plan->fRoofArray[iRoof]->SetLevel(fLevel);
			plan->fRoofArray[iRoof]->SetData(data);
		}
		fRoofArray.Append(plan->fRoofArray);
	}

	{
		const int32 appRoomCount = plan->fRoomArray.GetElemCount();
		for(int32 iRoom=0 ; iRoom<appRoomCount ; iRoom++)
		{
			plan->fRoomArray[iRoom]->SetLevel(fLevel);
			plan->fRoomArray[iRoom]->SetData(data);
		}
		fRoomArray.Append(plan->fRoomArray);
	}

	{
		const int32 appWallCount = plan->fWallArray.GetElemCount();
		for(int32 iWall=0 ; iWall<appWallCount ; iWall++)
		{
			plan->fWallArray[iWall]->SetLevel(fLevel);
			plan->fWallArray[iWall]->SetData(data);
		}
		fWallArray.Append(plan->fWallArray);
	}

	{
		const int32 appPointCount = plan->fPointArray.GetElemCount();
		for(int32 iPt=0 ; iPt<appPointCount ; iPt++)
		{
			plan->fPointArray[iPt]->SetLevel(fLevel);
			plan->fPointArray[iPt]->SetData(data);
		}
		fPointArray.Append(plan->fPointArray);
	}
}

void Plan::Split()
{
	// Split the selected walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if(wall->Selected())
		{
			wall->Split( wall->GetMidPos() );
		}
		
		// Check if 2 successives points in a hole are selected
		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			SubObject* obj = wall->GetObject( iObj );
			if(obj->Hidden())
				obj->Split();
		}
	}

	// Split the objects in the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		// Check if 2 successives points in a hole are selected
		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			SubObject* obj = room->GetObject( iObj );
			if(obj->Hidden())
				obj->Split();
		}
	}

	// Split the selected profile 'segments' 
	// Split the selected section 'segments'
	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		Roof* roof = fRoofArray[iRoof];
			
		// Check first the sections
		int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		if(zoneSectionCount)
		{
			const ZoneSection& lastSection = roof->GetRoofZoneSection(zoneSectionCount-1);
			int32 prevIndex = zoneSectionCount-1;
			boolean prevSelected = lastSection.fZonePoint->Selected() || lastSection.fSpinePoint->Selected();
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				const boolean curSelected = zoneSection.fZonePoint->Selected() || zoneSection.fSpinePoint->Selected();
				if(prevSelected && curSelected)
				{	// Add a new section between
					const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);

					const TVector2 newSpinePos = .5*( prevSection.fSpinePoint->Position() + zoneSection.fSpinePoint->Position());
					const TVector2 newZonePos = .5*( prevSection.fZonePoint->Position() + zoneSection.fZonePoint->Position());

					ZoneSection newSection;
					RoofPoint::CreateRoofPoint(&(newSection.fZonePoint),roof->GetData(),newZonePos,roof, false);
					RoofPoint::CreateRoofPoint(&(newSection.fSpinePoint),roof->GetData(),newSpinePos,roof, true);

					roof->InsertRoofZoneSection(iPt, newSection);

					iPt++;
					zoneSectionCount++;
				}

				prevIndex = iPt;
				prevSelected = curSelected;
			}
		}

		// Check the profiles
		int32 botCount = roof->GetBotProfilePointCount();
		if(botCount)
		{
			int32 prevIndex = botCount-1;
			boolean prevSelected = roof->GetBotProfilePoint(prevIndex)->Selected();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				const boolean curSelected = roof->GetBotProfilePoint(iBot)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetBotProfilePoint(prevIndex)->Position() +
												roof->GetBotProfilePoint(iBot)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,false,true);
					roof->GetBotProfile().InsertElem(iBot,newPoint);

					iBot++;
					botCount++;
				}

				prevSelected = curSelected;
				prevIndex = iBot;
			}
		}
		int32 topCount = roof->GetTopProfilePointCount();
		if(topCount)
		{
			int32 prevIndex = topCount-1;
			boolean prevSelected = roof->GetTopProfilePoint(prevIndex)->Selected();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				const boolean curSelected = roof->GetTopProfilePoint(iTop)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetTopProfilePoint(prevIndex)->Position() +
												roof->GetTopProfilePoint(iTop)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,true,true);
					roof->GetTopProfile().InsertElem(iTop,newPoint);

					iTop++;
					topCount++;
				}

				prevSelected = curSelected;
				prevIndex = iTop;
			}
		}
		int32 bInCount = roof->GetBotInsidePointCount();
		if(bInCount)
		{
			int32 prevIndex = bInCount-1;
			boolean prevSelected = roof->GetBotInsidePoint(prevIndex)->Selected();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				const boolean curSelected = roof->GetBotInsidePoint(iBIn)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetBotInsidePoint(prevIndex)->Position() +
												roof->GetBotInsidePoint(iBIn)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,false, true);
					roof->GetBotInside().InsertElem(iBIn,newPoint);

					iBIn++;
					bInCount++;
				}

				prevSelected = curSelected;
				prevIndex = iBIn;
			}
		}
		int32 tInCount = roof->GetTopInsidePointCount();
		if(tInCount)
		{
			int32 prevIndex = tInCount-1;
			boolean prevSelected = roof->GetTopInsidePoint(prevIndex)->Selected();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				const boolean curSelected = roof->GetTopInsidePoint(iTIn)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetTopInsidePoint(prevIndex)->Position() +
												roof->GetTopInsidePoint(iTIn)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,true,false);
					roof->GetTopInside().InsertElem(iTIn,newPoint);

					iTIn++;
					tInCount++;
				}

				prevSelected = curSelected;
				prevIndex = iTIn;
			}
		}
	}
}

void Plan::Merge()
{
	// Merge the selected points
	const int32 pointCount = fPointArray.GetElemCount();

	ClearWallFlag(eWallRebuildRoom0);
	ClearWallFlag(eWallRebuildRoom1);

	ClearPointFlag(ePointToDelete);

	{
		TMCCountedPtr<VPoint> firstPoint;
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point = fPointArray[iPoint];

			if(point->Selected())
			{
				if(firstPoint)
					firstPoint->Merge(point,false);
				else
					firstPoint = point;
			}
		}
	}
	// Now clean up the points
	for(int32 iPt=pointCount-1 ; iPt>=0 ; iPt--)
	{
		VPoint* point = fPointArray[iPt];
		if(point->Selected())
		{
			if(point->Flag(ePointToDelete) || point->GetWallCount()==0 )
			{
				point->DeletePoint();
			}
		}
	}

	RebuildInvalidRooms();

#if 0 // TO DO: merge profile points
	// Split the selected profile 'segments' 
	// Split the selected section 'segments'
	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		Roof* roof = fRoofArray[iRoof];
			
		// Check first the sections
		int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		if(zoneSectionCount)
		{
			const ZoneSection& lastSection = roof->GetRoofZoneSection(zoneSectionCount-1);
			int32 prevIndex = zoneSectionCount-1;
			boolean prevSelected = lastSection.fZonePoint->Selected() || lastSection.fSpinePoint->Selected();
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				const boolean curSelected = zoneSection.fZonePoint->Selected() || zoneSection.fSpinePoint->Selected();
				if(prevSelected && curSelected)
				{	// Add a new section between
					const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);

					const TVector2 newSpinePos = .5*( prevSection.fSpinePoint->Position() + zoneSection.fSpinePoint->Position());
					const TVector2 newZonePos = .5*( prevSection.fZonePoint->Position() + zoneSection.fZonePoint->Position());

					ZoneSection newSection;
					RoofPoint::CreateRoofPoint(&(newSection.fZonePoint),roof->GetData(),newZonePos,roof, false);
					RoofPoint::CreateRoofPoint(&(newSection.fSpinePoint),roof->GetData(),newSpinePos,roof, true);

					roof->InsertRoofZoneSection(iPt, newSection);

					iPt++;
					zoneSectionCount++;
				}

				prevIndex = iPt;
				prevSelected = curSelected;
			}
		}

		// Check the profiles
		int32 botCount = roof->GetBotProfilePointCount();
		if(botCount)
		{
			int32 prevIndex = botCount-1;
			boolean prevSelected = roof->GetBotProfilePoint(prevIndex)->Selected();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				const boolean curSelected = roof->GetBotProfilePoint(iBot)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetBotProfilePoint(prevIndex)->Position() +
												roof->GetBotProfilePoint(iBot)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,false,true);
					roof->GetBotProfile().InsertElem(iBot,newPoint);

					iBot++;
					botCount++;
				}

				prevSelected = curSelected;
				prevIndex = iBot;
			}
		}
		int32 topCount = roof->GetTopProfilePointCount();
		if(topCount)
		{
			int32 prevIndex = botCount-1;
			boolean prevSelected = roof->GetTopProfilePoint(prevIndex)->Selected();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				const boolean curSelected = roof->GetTopProfilePoint(iTop)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetTopProfilePoint(prevIndex)->Position() +
												roof->GetTopProfilePoint(iTop)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,true,true);
					roof->GetTopProfile().InsertElem(iTop,newPoint);

					iTop++;
					topCount++;
				}

				prevSelected = curSelected;
				prevIndex = iTop;
			}
		}
		int32 bInCount = roof->GetBotInsidePointCount();
		if(bInCount)
		{
			int32 prevIndex = bInCount-1;
			boolean prevSelected = roof->GetBotInsidePoint(prevIndex)->Selected();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				const boolean curSelected = roof->GetBotInsidePoint(iBIn)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetBotInsidePoint(prevIndex)->Position() +
												roof->GetBotInsidePoint(iBIn)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,false, true);
					roof->GetBotInside().InsertElem(iBIn,newPoint);

					iBIn++;
					bInCount++;
				}

				prevSelected = curSelected;
				prevIndex = iBIn;
			}
		}
		int32 tInCount = roof->GetTopInsidePointCount();
		if(tInCount)
		{
			int32 prevIndex = tInCount-1;
			boolean prevSelected = roof->GetTopInsidePoint(prevIndex)->Selected();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				const boolean curSelected = roof->GetTopInsidePoint(iTIn)->Selected();
				if(prevSelected && curSelected)
				{	// Add a new profile point between
					const TVector2 newPos = .5*(roof->GetTopInsidePoint(prevIndex)->Position() +
												roof->GetTopInsidePoint(iTIn)->Position() );
		
					TMCCountedPtr<ProfilePoint> newPoint;
					ProfilePoint::CreateProfilePoint(&newPoint, roof->GetData(), newPos,roof,true,false);
					roof->GetTopInside().InsertElem(iTIn,newPoint);

					iTIn++;
					tInCount++;
				}

				prevSelected = curSelected;
				prevIndex = iTIn;
			}
		}
	}
#endif
}

void Plan::Smooth(const boolean smooth)
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		// Check if one of the wall object is selected
		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			WallSubObject* obj = wall->GetObject( iObj );
			if( obj->Hidden() )
			{	// smooth the selected polyline points
				obj->SmoothPolylinePoints(smooth);
			}
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			RoomSubObject* obj = room->GetObject( iObj );
			if( obj->Hidden() )
			{	// smooth the selected polyline points
				obj->SmoothPolylinePoints(smooth);
			}
		}
	}
}

Room* Plan::PosInRoom(const TVector2& pos)
{
	// Reference it also in a room if the point is in one
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		if( fRoomArray[iRoom]->PointIn(pos, false) )
		{
			return fRoomArray[iRoom];
		}
	}
	
	return NULL;
}

Wall* Plan::PosOnWall(const TVector2& pos)
{
	// Reference it also in a room if the point is in one
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		const TVector2& pos0 = wall->GetPoint(0)->Position();
		const TVector2& pos1 = wall->GetPoint(1)->Position();
		const TVector2 wallVec = pos1-pos0;

		if(RealAbs( wallVec^(pos-pos0) ) < kRealEpsilon)
		{
			if(wallVec*(pos-pos0)>0 && wallVec*(pos1-pos)>0)
				return wall;
		}
	}
	
	return NULL;
}

VPoint* Plan::PointOnPoint( VPoint* point )
{
	const TVector2& pos = point->Position();

	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPt=0 ; iPt<pointCount ; iPt++ )
	{
		VPoint* otherPoint = fPointArray[iPt];
		
		if( otherPoint->Flag(ePointHelper) ) // do not check this one
			continue;

		if( otherPoint!=point && pos.IsEqual(otherPoint->Position(),kRealEpsilon) )
		{
			return otherPoint;
		}
	}
	
	return NULL;
}

void Plan::AddPointInRoom(Room* room)
{
#ifdef USE_POINT_IN
	const int32 pointCount = fPointArray.GetElemCount();

	const TMCCountedPtrArray<Point>& path = room->GetPath();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		Point* point = fPointArray[iPoint];
		// Check if this point is not allready part of another room. If so, we can't add it in the room
		const boolean canAdd = !point->IsInRoomPath();

		if( canAdd && room->PointIn(point->Position(), false) )
		{
			// Check that it's not a point of the path
			if(path.FindElem(point)==kUnusedIndex)
			{
				room->AddPointInReference(point);
				point->InvalidateTessellation();
				// Register the room in the walls
				const int32 wallCount = point->GetWallCount();
				for(int32 iWall=0 ; iWall<wallCount ; iWall++)
				{
					Wall* wall = point->GetWall(iWall);
					// Maybe if room inside have an inner room ?
					if(wall->GetRoom(0)==NULL)
						wall->SetRoom(0,room);
					if(wall->GetRoom(1)==NULL)
						wall->SetRoom(1,room);
				}
			}
		}
	}
#endif
}

VPoint* Plan::GetPointHelper()
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		const int32 flags = fPointArray[iPoint]->GetFlags();
		if( fPointArray[iPoint]->Flag(ePointHelper) )
			return fPointArray[iPoint];
	}

	return NULL;
}

void Plan::GetSelectedRoomsPerimeter(TMCCountedPtrArray<VPoint>& perimeter)
{
	const int32 wallCount = fWallArray.GetElemCount();

	Wall* firstWall = NULL;

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if(wall->GetSelectedRoomCount() == 1 )
		{
			firstWall = wall;
			break;
		}
	}

	perimeter.ArrayFree();
	if(!firstWall) return;

	VPoint* firstPoint = firstWall->GetPoint(0);
	perimeter.AddElem(firstPoint);
	VPoint* curPoint = firstWall->GetPoint(1);
	Wall* curWall = firstWall;
	while(curPoint != firstPoint)
	{
		perimeter.AddElem(curPoint);

		const int32 wallCount = curPoint->GetWallCount();
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = curPoint->GetWall(iWall);
			if(wall!=curWall && wall->GetSelectedRoomCount()==1)
			{
				curWall = wall;
				curPoint = wall->GetOtherPoint(curPoint);
				break;
			}
		}
	}
}

void Plan::GetRoomsPerimeter(TMCCountedPtrArray<VPoint>& perimeter)
{
	const int32 wallCount = fWallArray.GetElemCount();

	Wall* firstWall = NULL;

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if(wall->GetRoomCount() == 1 )
		{
			firstWall = wall;
			break;
		}
	}

	perimeter.ArrayFree();
	if(!firstWall) return;

	VPoint* firstPoint = firstWall->GetPoint(0);
	perimeter.AddElem(firstPoint);
	VPoint* curPoint = firstWall->GetPoint(1);
	Wall* curWall = firstWall;
	while(curPoint != firstPoint)
	{
		perimeter.AddElem(curPoint);

		const int32 wallCount = curPoint->GetWallCount();
		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = curPoint->GetWall(iWall);
			if(wall!=curWall && wall->GetRoomCount()==1)
			{
				curWall = wall;
				curPoint = wall->GetOtherPoint(curPoint);
				break;
			}
		}
	}
}

inline void IncludePos(const TVector2& pos, TVector2& min, TVector2& max)
{
	if(pos.x<min.x) min.x=pos.x;
	if(pos.x>max.x) max.x=pos.x;
	if(pos.y<min.y) min.y=pos.y;
	if(pos.y>max.y) max.y=pos.y;
}

boolean Plan::GetWallBBox( TBBox3D& bbox, const boolean exact, const boolean onSelection )
{
	boolean hasPoints = false;
	if(exact)
	{
		const real32 altitude = fLevel->GetDistanceToGround();;
		const int32 wallCount = fWallArray.GetElemCount();
	
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];

			const boolean add = !onSelection||wall->Selected();

			if(add)
			{
				hasPoints = true;

				if(bbox.Valid())	bbox+=wall->GetWallBBox();
				else				bbox=wall->GetWallBBox();
			}
		}
	}
	else
	{
		const real32 distanceToGround = GetDistanceToGround();
		const real32 distPlusHeight = distanceToGround+GetLevelHeight();
		const int32 pointCount = fPointArray.GetElemCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			if(!onSelection || fPointArray[iPoint]->Selected())
			{
				hasPoints = true;
				const TVector2& pos = fPointArray[iPoint]->Position();

				bbox.AddPoint(TVector3(pos.x,pos.y,distanceToGround));
				bbox.AddPoint(TVector3(pos.x,pos.y,distPlusHeight));
			}
		}
	}

	return hasPoints;
}
	
void Plan::GetRoomObjectList( TMCCountedPtrArray<RoomSubObject>& objects)
{
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		const int32 roomObjCount = room->GetObjectCount();

		for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
		{
			objects.AddElem(room->GetObject(iObj));
		}
	}
}

void Plan::GetSelectedObjects(TMCCountedPtrArray<WallSubObject>& wallObj, TMCCountedPtrArray<RoomSubObject>& roomObj)
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		const int32 wallObjCount = wall->GetObjectCount();

		for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
		{
			if(wall->GetObject(iObj)->Selected())
			wallObj.AddElem(wall->GetObject(iObj));
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Hidden())
			continue;

		const int32 roomObjCount = room->GetObjectCount();

		for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
		{
			if(room->GetObject(iObj)->Selected())
			roomObj.AddElem(room->GetObject(iObj));
		}
	}
}

Room* Plan::GetNearestRoom(const TVector2& pos)
{
	real32 minAlpha=kBigRealValue;
	real32 alpha=0;

	Room* bestRoom=NULL;

	const int32 wallCount = fWallArray.GetElemCount();

	// find the nearest wall with 1 room to find which room is the nearest
	TVector2 bestPos;
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		// We need wall with only 1 room
		Room* room0 = wall->GetRoom(0);
		Room* room1 = wall->GetRoom(1);
		if((!room0 && !room1) || (room0 && room1))
			continue;
	
		// Compute the distance from the point to the wall segment
		const real32 alpha = PointSegmentSqrDistance(pos,wall->GetPoint(0)->Position(),
			wall->GetPoint(1)->Position());
		if( alpha<minAlpha )
		{
			minAlpha = alpha;
			bestRoom = room1?room1:room0;
		}
	}

	return bestRoom;
}

void Plan::InvalidateObjectSelection()
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			if( wall->GetObject( iObj )->Selected() )
			{
				wall->InvalidateTessellation();
				break;
			}
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		const int32 objCount = room->GetObjectCount();	
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			if( room->GetObject( iObj )->Selected() )
			{
				room->InvalidateTessellation();
				break;
			}
		}
	}
}

void Plan::SetSelectionHelper(const boolean set)
{
/*	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->Flag(eWallExtendedSelection) ) // use the extended selection: all the moving walls need to be set
		{
			if(set)	wall->SetFlag(eWallHelper);
			else	wall->ClearFlag(eWallHelper);
		}
	}

	// If a room is selected, set its wall as helper too
 Should be done by the extended selection
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if( room->Flag(eRoomExtendedSelection) )
		{
			if(set)	room->SetWallFlag(eWallHelper);
			else	room->ClearWallFlag(eWallHelper);
		}
	}*/

	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if (point->Selected())
		{
			if(set)
			{
				point->SetFlag(ePointHelper);
				point->SetWallFlag(eWallHelper);
			}
			else
			{
				point->ClearFlag(ePointHelper);
				point->ClearWallFlag(eWallHelper);
			}
		}
	}
}

void Plan::CheckSelectionConsistency()
{
#if 1
	ClearWallFlag(eWallRebuildRoom0);
	ClearWallFlag(eWallRebuildRoom1);

	ClearPointFlag(ePointToDelete);

	int32 pointCount = fPointArray.GetElemCount();

	{	// Merge and mark the points
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point = fPointArray[iPoint];

			if (point->Selected() && !point->Flag(ePointToDelete))
			{
				point->CheckConsistency(false);
			}
		}
	}

	int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<(int32)fWallArray.GetElemCount() ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->Flag(eWallExtendedSelection) )//Selected() ) // In case a room is selected without its walls
		{
			if( CheckWallConsistency( wall, 0, false ) )
			{
				const int32 objCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj< objCount ; iObj++ )
				{
					wall->GetObject( iObj )->CheckObjectPosition();
				}
			}
		}
	}

	{ // Delete the points
		for( int32 iPoint=pointCount-1 ; iPoint>=0 ; iPoint-- )
		{
			VPoint* point = fPointArray[iPoint];

			if(point->Flag(ePointToDelete))
			{
				MY_ASSERT(point->GetWallCount()==0);
				point->DeletePoint();
			}
		}
	}


	RebuildInvalidRooms();
#else
	ClearWallFlag(eWallRebuildRoom0);
	ClearWallFlag(eWallRebuildRoom1);

	int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		Point* point = fPointArray[iPoint];

		if (point->Selected())
		{
			if( point->CheckConsistency() )
			{
				// Topology change
				pointCount = fPointArray.GetElemCount();
			}
		}
	}

	int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->Flag(eWallExtendedSelection) )//Selected() )
		{
			if( wall->CheckConsistency() )
			{
				iWall = 0;
				wallCount = fWallArray.GetElemCount();
			}

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				wall->GetObject( iObj )->CheckObjectPosition();
			}
		}
	}

	RebuildInvalidRooms();
#endif
}

void Plan::InvalidateTessellation()
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->InvalidateTessellation();
	}

	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->InvalidateTessellation();
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->InvalidateTessellation();
	}

	// Invalidate the roof tesselation
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->ClearFlag(eRoofTessellated);
	}
}

void Plan::InvalidateSelection()
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if (point->Selected())
			point->InvalidateTessellation();
	}

	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Selected())
			wall->InvalidateTessellation();
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Selected())
			room->InvalidateTessellation();
	}

	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		if (roof->Selected())
			roof->ClearFlag(eRoofTessellated);
	}
}

void Plan::InvalidateExtendedSelection()
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Flag(eWallExtendedSelection))
			wall->InvalidateTessellation();
		else
		{	// Check if one of the wall object is selected
			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				WallSubObject* obj = wall->GetObject( iObj );
				if( obj->Selected() ||
					(obj->Hidden() && obj->HasSelectedHolePoint()) )// Check if one of the hole points is selected
				{
					wall->InvalidateTessellation();
					break;
				}
			}
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Flag(eRoomExtendedSelection))
			room->InvalidateTessellation();
		else
		{	// Check if one of the wall object is selected
			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				RoomSubObject* obj = room->GetObject( iObj );
				if( obj->Selected() ||
					(obj->Hidden() && obj->HasSelectedHolePoint()) )// Check if one of the hole points is selected
				{
					room->InvalidateTessellation();
					break;
				}
			}
		}
	}

	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		if (roof->Flag(eRoofExtendedSelection))
			roof->ClearFlag(eRoofTessellated);
	}
}

void Plan::InvalidateAll()
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		point->InvalidateTessellation();
	}

	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		wall->InvalidateTessellation();
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		room->InvalidateTessellation();
	}

	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		roof->ClearFlag(eRoofTessellated);
	}
}


boolean Plan::HasPointSelection() const
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		if( fPointArray[iPoint]->Selected() )
			return true;
	}

	return false;
}

boolean Plan::CheckRoofPointSelection()
{
	const int32 roofCount = fRoofArray.GetElemCount();

	boolean hasSelection = false;

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		if(roof->Hidden())
			continue;
	
		if(roof->Selected())
		{
			hasSelection = true;
			roof->SetFlag(eRoofExtendedSelection);
			continue;
		}
	
		boolean next = false;

		// zone
		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
		for(int32 iPt=0 ; iPt<zoneSectionCount && !next; iPt++)
		{
			const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

			if( zoneSection.fZonePoint->Selected() )
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}

			if( zoneSection.fSpinePoint->Selected() )
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}
		}

		// Profiles
		const int32 botCount = roof->GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount && !next ; iBot++)
		{
			if( roof->GetBotProfilePoint(iBot)->Selected())
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}
		}

		const int32 topCount = roof->GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount && !next ; iTop++)
		{
			if( roof->GetTopProfilePoint(iTop)->Selected())
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}
		}

		const int32 bInCount = roof->GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount && !next ; iBIn++)
		{
			if( roof->GetBotInsidePoint(iBIn)->Selected())
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}
		}

		const int32 tInCount = roof->GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount && !next ; iTIn++)
		{
			if( roof->GetTopInsidePoint(iTIn)->Selected())
			{
				hasSelection = true;
				roof->SetFlag(eRoofExtendedSelection);
				next = true;
			}
		}

		if(!next)
			roof->ClearFlag(eRoofExtendedSelection);
	}

	return hasSelection;
}

void Plan::AddWallsAndRoomsToExtendedSelection()
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		fWallArray[iWall]->SetFlag(eWallExtendedSelection);

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		fRoomArray[iRoom]->SetFlag(eRoomExtendedSelection);
}

void Plan::RestoreWallsAndRoomsExtendedSelection()
{
	// Clear the flag
	const int32 wallCount = fWallArray.GetElemCount();
	{
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			fWallArray[iWall]->ClearFlag(eWallExtendedSelection);
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	{
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			fRoomArray[iRoom]->ClearFlag(eRoomExtendedSelection);
	}

	// Then restore the one we need to keep

	// Arround the selected points
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];
		if (point->Selected())
		{
			point->SetExtendedSelection();
		}
	}

	// If an object is selected, add the wall or room in
	{
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = fWallArray[iWall];

			if (wall->Hidden())
				continue;

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				if( wall->GetObject( iObj )->Selected() )
				{
					wall->SetFlag(eWallExtendedSelection);
					continue;
				}
			}
		}
	}

	{
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = fRoomArray[iRoom];

			if (room->Hidden())
				continue;

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				if( room->GetObject( iObj )->Selected() )
				{
					room->SetFlag(eRoomExtendedSelection);
					continue;
				}
			}
		}
	}
}

// Check the topology of the extended selection:
// Verify that no wall are going through room objects
// Verify that the room objects are still in their room
// Maybe later: snap walls, points, ...
void Plan::CheckExtendedSelection()
{
	// Make a list of the objects here
	TMCCountedPtrArray<RoomSubObject> objectsHere;
	GetRoomObjectList(objectsHere);
	const int32 hereCount = objectsHere.GetElemCount();

	TMCCountedPtrArray<RoomSubObject> objectsUnder;
	Level* levelUnder = fLevel->GetLevelUnder();
	if(levelUnder)
		levelUnder->LevelPlan().GetRoomObjectList(objectsUnder);
	const int32 underCount = objectsUnder.GetElemCount();

	const int32 wallCount = fWallArray.GetElemCount();
/* This does not work wery well
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Flag(eWallExtendedSelection))
		{	// Check that this wall is not going through one of the objects
			for(int32 iObj=0 ; iObj<hereCount ; iObj++)
			{
				RoomSubObject* obj = objectsHere[iObj];
				if(!obj->Selected()) // selected obj already made their checking when moved
					wall->CheckRoomObjConflict(obj);
			}
			for(int32 iUn=0 ; iUn<underCount ; iUn++)
			{
				RoomSubObject* obj = objectsUnder[iUn];
				if(!obj->Selected()) // selected obj already made their checking when moved
					wall->CheckRoomObjConflict(obj);
			}
		}
	}
*/
	// Then check the object to know if they're still in the same room
	{
		for(int32 iObj=0 ; iObj<hereCount ; iObj++)
		{
			RoomSubObject* obj = objectsHere[iObj];
			if(!obj->Selected()) // selected obj already made their checking when moved
			{
				const TVector2& center = obj->GetPolylineCenter();
				Room* inRoom = PosInRoom(center);
				if(inRoom&&inRoom!=obj->GetInRoom())
				{
					obj->SetInRoom(inRoom);
				}
			}
		}
	}
}

void Plan::SetWallOffset(real32 offset, boolean computeSegCount)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		if(wall->Selected())
		{
			wall->SetArcOffset(offset);
			if(computeSegCount)
			{
				wall->ComputeArcSegmentCount();
			}
		}
	}
}

void Plan::OffsetSelection(const TVector2& offset)
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if (point->Selected())
		{
			point->OffsetPosition(offset);
		}
	}

	// Offset the Roofs points
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->OffsetSelection(offset);
	}

	// Now offset also the objects in the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->OffsetAttachedObjects(offset);
	}
}

void Plan::ScaleSelection(const TVector2& scale, const TVector2& center, const EOptionMode mode)
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if (point->Selected())
		{
			point->SetPosition(ScalePoint(point->Position(),scale,center));
		}
	}

	// Scale the Roofs points
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->ScaleSelection(scale,center,mode);
	}

	// Now offset also the objects in the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		const int32 roomObjCount = room->GetObjectCount();

		for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
		{
			RoomSubObject* obj = room->GetObject(iObj);
			if(obj->Selected())
			{
				obj->SetPolylineCenter(ScalePoint(obj->GetPolylineCenter(),scale,center),true);
			}
		}
	}
}

void OffsetSelectedPolyline( SubObject* obj, const TVector2& offset )
{
	if( obj->Hidden() )
	{	// offset the selected polyline points
		obj->OffsetPolylinePoints(offset);
	}
	else if( obj->Selected() )
	{
		obj->OffsetPolyline(offset, false); // No checking anymore, can go anywhere true);
	}
}

void Plan::OffsetObj(const TVector2& offset)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			OffsetSelectedPolyline( wall->GetObject( iObj ), offset );
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Hidden())
			continue;

		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			OffsetSelectedPolyline( room->GetObject( iObj ), offset );
		}
	}
}

void ScaleSelectedPolyline( SubObject* obj, const TVector2& scale )
{
	if( obj->Hidden() )
	{	// scale the selected polyline points
		obj->ScalePolylinePoints(scale, false);
	}
	else if( obj->Selected() )
	{
		obj->ScalePolyline(scale, false); // No checking, polyline can go anywhere true);
	}
}

void Plan::ScaleObj(const TVector2& scale)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			ScaleSelectedPolyline(wall->GetObject( iObj ),scale);
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Hidden())
			continue;

		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			ScaleSelectedPolyline(room->GetObject( iObj ),scale);
		}
	}
}

void RotateSelectedPolyline( SubObject* obj, const TVector2& cosSin )
{
	if( obj->Hidden() )
	{	// scale the selected polyline points
		obj->RotatePolylinePoints(cosSin, false);
	}
	else if( obj->Selected() )
	{
		obj->RotatePolyline(cosSin,true);
	}
}

void Plan::RotateObj(const TVector2& cosSin)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if (wall->Hidden())
			continue;

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			RotateSelectedPolyline(wall->GetObject( iObj ),cosSin);
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Hidden())
			continue;

		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			RotateSelectedPolyline( room->GetObject( iObj ), cosSin);
		}
	}
}

void Plan::RotateSelection(const TVector2& cosSin, const TVector2& center)
{
	const int32 pointCount = fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if (point->Selected())
		{
			point->SetPosition(RotatePoint(point->Position(),cosSin,center));
		}
	}

	// Rotate the Roofs points
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->RotateSelection(cosSin,center);
	}

	// Now rotate also the center of the objects in the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		const int32 roomObjCount = room->GetObjectCount();

		for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
		{
			RoomSubObject* obj = room->GetObject(iObj);
			if(obj->Selected())
			{
				obj->SetPolylineCenter(RotatePoint(obj->GetPolylineCenter(),cosSin,center),true);
			}
		}
	}
}

boolean Plan::SnapPosWithAxis(TVector2& pos, const TVector2& axis, const TVector2& preferedProjDir)
{
	const int32 pointCount = fPointArray.GetElemCount();

	const real32 limit = .5f;
	real32 bestValue = limit;
	TVector2 bestPos = TVector2::kZero;
	VPoint* bestPoint = NULL;

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if(!point->Flag(ePointHelper)) // do not check this one, it's the one being check
		{
			const TVector2& pointPos = point->Position();
			const real32 value = RealAbs((pointPos-pos)*axis);
			if( value<bestValue ||
				(value==bestValue && (pos-pointPos).GetSquaredNorm()<(pos-bestPos).GetSquaredNorm()) )
			{	// Snap the pos
				bestValue=value;
				bestPos = pointPos;
				bestPoint = point;
			}
		}
	}
	if(bestValue<limit && bestPoint)
	{
		// Project the pos on the line define by bestPos and the normal to axis
		TVector2 normal(-axis.y, axis.x);
		if(preferedProjDir!=TVector2::kZero && RealAbs(preferedProjDir*axis)>kRealEpsilon)
		{
			IntersectLineLine(	bestPos, // first point of the line
								bestPos+normal, // second point of the line
								pos, // point on the 2nd line
								preferedProjDir, // dir of the 2nd line
								pos	);
		}
		else
		{
			Project(pos, bestPos, bestPos+normal, pos);
		}
				
		bestPoint->SetFlag(eSnapedPosition);
		
		return true; 
	}

	return false;
}

void  Plan::InitMarqueeSelection()
{
	BuildingPrimData* data = fLevel->GetPrimitiveData();
	const boolean onSubObjectPoints = data->GetHoleEditEnable();

	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		wall->InitMarqueeSelection();

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			wall->GetObject(iObj)->InitMarqueeSelection(onSubObjectPoints);
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		room->InitMarqueeSelection();
	
		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			room->GetObject(iObj)->InitMarqueeSelection(onSubObjectPoints);
		}
	}

	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->InitMarqueeSelection();
	}

	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];
		roof->InitMarqueeSelection();
	
		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
		{
			const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

			zoneSection.fZonePoint->InitMarqueeSelection();
			zoneSection.fSpinePoint->InitMarqueeSelection();
		}
	//	const int32 objCount = roof->GetObjectCount();
	//	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	//	{
	//		roof->GetObject(iObj)->InitMarqueeSelection();
	//	}
	}

}

void Plan::SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode)
{
	BuildingPrimData* data = fLevel->GetPrimitiveData();
	const boolean onSubObjectPoints = data->GetHoleEditEnable();

	// Select/Deselect the points
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->SetMarquee(rayPlanes, marqueeMode);
	}

	// Select/Deselect the objects
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			if(onSubObjectPoints)
				wall->GetObject(iObj)->SetMarqueeOnPoints(rayPlanes, marqueeMode);
			else
				wall->GetObject(iObj)->SetMarqueeOnObject(rayPlanes, marqueeMode);
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj<objCount ; iObj++ )
		{
			if(onSubObjectPoints)
				room->GetObject(iObj)->SetMarqueeOnPoints(rayPlanes, marqueeMode);
			else
				room->GetObject(iObj)->SetMarqueeOnObject(rayPlanes, marqueeMode);
		}
	}

	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];
	
		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
		{
			const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

			zoneSection.fZonePoint->SetMarquee(rayPlanes, marqueeMode);
			zoneSection.fSpinePoint->SetMarquee(rayPlanes, marqueeMode);
		}
	//	const int32 objCount = roof->GetObjectCount();
	//	for( int32 iObj=0 ; iObj<objCount ; iObj++ )
	//	{
	//		roof->GetObject(iObj)->SetMarquee(rayPlanes, marqueeMode);
	//	}
	}
}

void Plan::ShowRooms()
{
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->ShowRoom();
	}
}

void Plan::HideRooms()
{
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->HideRoom();
	}
}

void Plan::ShowWalls()
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->ShowWall();
	}
}

void Plan::HideWalls()
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->HideWall();
	}
}

void Plan::HidePoints()
{
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->SetFlag(eIsHidden);
		fPointArray[iPoint]->ClearFlag(eIsSelected);
	}
}

void Plan::ShowRoofs()
{
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->ShowRoof();
	}
}

void Plan::HideRoofs()
{
	const int32 roofCount = fRoofArray.GetElemCount();

	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		fRoofArray[iRoof]->HideRoof();
	}
}

void ShowPts(SubObject* obj)
{
	obj->SetFlag(eIsHidden);
	obj->ClearFlag(eIsSelected);
	obj->ClearOutlineFlag(eIsHidden);
}

// Slightly different from the other show/hide:
// it's a switch between 'object handle' and 'object points'
void Plan::ShowHolePoints()
{
	// Walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		const int32	objCount = wall->GetObjectCount();
		for(int32 iObj=0 ; iObj<objCount ; iObj++)
		{
			ShowPts( wall->GetObject( iObj ) );
		}
	}

	// Rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		const int32	objCount = room->GetObjectCount();
		for(int32 iObj=0 ; iObj<objCount ; iObj++)
		{
			ShowPts( room->GetObject( iObj ) );
		}
	}
}

void HidePts(SubObject* obj)
{
	obj->ClearFlag(eIsHidden);
	obj->SetOutlineFlag(eIsHidden);
	obj->ClearOutlineFlag(eIsSelected);
}

// Slightly different from the other show/hide:
// it's a switch between 'object handle' and 'object points'
void Plan::HideHolePoints()
{
	// Walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		const int32	objCount = wall->GetObjectCount();
		for(int32 iObj=0 ; iObj<objCount ; iObj++)
		{
			HidePts( wall->GetObject( iObj ) );
		}
	}

	// Rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		const int32	objCount = room->GetObjectCount();
		for(int32 iObj=0 ; iObj<objCount ; iObj++)
		{
			HidePts( room->GetObject( iObj ) );
		}
	}
}


void Plan::SetShadingDomain(const int32 domainID, const int32 selectionSubPart)
{
	// On the walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if(wall->Selected())
		{
			if(selectionSubPart == 0) wall->SetLeftDomain(domainID);
			else if(selectionSubPart == 1) wall->SetRightDomain(domainID);
			else if(selectionSubPart == 2) wall->SetBetweenDomain(domainID);
		}
	}

	// On the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		if(room->Selected())
		{
			if(selectionSubPart == 0)
				room->SetFloorDomain(domainID);
			else if(selectionSubPart == 1)
				room->SetCeilingDomain(domainID);
			else if(selectionSubPart == 2)
			{	// Modify the shading on the walls arround
				room->SetWallsShadingDomain(domainID);
			}
		}
	}

	// On the roofs
	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];
		if(roof->Selected())
		{
			if(selectionSubPart == 0)
				roof->SetOutTopDomain(domainID);
			else if(selectionSubPart == 1)
				roof->SetOutMidDomain(domainID);
			else if(selectionSubPart == 2)
				roof->SetOutBotDomain(domainID);
			else if(selectionSubPart == 3)
				roof->SetInsideDomain(domainID);
		}
	}
}

void Plan::DelShadingDomain(const int32 domainID, const int32 replaceByID)
{
	// On the walls
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->GetLeftDomain()==domainID )
		{
			if(replaceByID==kNoDomains) wall->SetLeftDomain( (wall->GetLeftRoom()?eInsideWallDomain:eOutsideWallDomain) );
			else wall->SetLeftDomain(replaceByID);
		}
		if( wall->GetRightDomain()==domainID )
		{
			if(replaceByID==kNoDomains) wall->SetRightDomain( (wall->GetRightRoom()?eInsideWallDomain:eOutsideWallDomain) );
			else wall->SetRightDomain(replaceByID);
		}
		if( wall->GetBetweenDomain()==domainID )
		{
			if(replaceByID==kNoDomains) wall->SetBetweenDomain( (wall->GetRoomCount()?eInsideWallDomain:eOutsideWallDomain) );
			else wall->SetRightDomain(replaceByID);
		}
	}

	// On the rooms
	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
		if( room->GetFloorDomain()==domainID )
		{
			if(replaceByID==kNoDomains) room->SetFloorDomain(eInsideFloorDomain);
			else room->SetFloorDomain(replaceByID);
		}
		if( room->GetCeilingDomain()==domainID )
		{
			if(replaceByID==kNoDomains) room->SetCeilingDomain(eInsideCeilingDomain);
			else room->SetCeilingDomain(replaceByID);
		}
	}

	// On the roofs
	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];
		if( roof->GetOutTopDomain()==domainID )
		{
			if(replaceByID==kNoDomains) roof->SetOutTopDomain(eOutsideTopRoofDomain);
			else roof->SetOutTopDomain(replaceByID);
		}
		if( roof->GetOutMidDomain()==domainID )
		{
			if(replaceByID==kNoDomains) roof->SetOutMidDomain(eOutsideMidRoofDomain);
			else roof->SetOutMidDomain(replaceByID);
		}
		if( roof->GetOutBotDomain()==domainID )
		{
			if(replaceByID==kNoDomains) roof->SetOutBotDomain(eOutsideBotRoofDomain);
			else roof->SetOutBotDomain(replaceByID);
		}
		if( roof->GetInsideDomain()==domainID )
		{
			if(replaceByID==kNoDomains) roof->SetInsideDomain(eInsideRoofDomain);
			else roof->SetInsideDomain(replaceByID);
		}
	}
}

void Plan::SetPointFlag(const int32 flag)
{
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->SetFlag(flag);
	}
}

void Plan::ClearPointFlag(const int32 flag)
{
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		fPointArray[iPoint]->ClearFlag(flag);
	}
}

void Plan::SetWallFlag(const int32 flag)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->SetFlag(flag);
	}
}

void Plan::ClearWallFlag(const int32 flag)
{
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		fWallArray[iWall]->ClearFlag(flag);
	}
}

void Plan::SetRoomFlag(const int32 flag)
{
	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->SetFlag(flag);
	}
}

void Plan::ClearRoomFlag(const int32 flag)
{
	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		fRoomArray[iRoom]->ClearFlag(flag);
	}
}

boolean Plan::CheckWallConsistency( Wall* onWall, const int32 startIndex,const boolean canDelete )
{
	// Check if there're intersections with other walls. If so,
	// split the wall in as many pieces as necessary.
	const int32 wallCount = fWallArray.GetElemCount();
	VPoint* point0 = onWall->GetPoint(0);
	VPoint* point1 = onWall->GetPoint(1);
	const TVector2& p0 = point0->Position();
	const TVector2& p1 = point1->Position();

	MY_ASSERT(!point0->Flag(ePointToDelete));
	MY_ASSERT(!point1->Flag(ePointToDelete));

	for( int32 iWall=startIndex ; iWall<wallCount ; iWall++ )
	{
		Wall* otherWall = fWallArray[iWall];

		if(otherWall == onWall)
			continue;

		if(otherWall->Flag(eWallHelper))
			continue;

		VPoint* point2 = otherWall->GetPoint(0);
		VPoint* point3 = otherWall->GetPoint(1);
		const TVector2& p2 = point2->Position();
		const TVector2& p3 = point3->Position();

		MY_ASSERT(!point2->Flag(ePointToDelete));	
		MY_ASSERT(!point3->Flag(ePointToDelete));

		if( (point2==point0 || point2==point1) && (point3==point0||point3==point1) )
			continue;
		else if( point2==point0 || point2==point1)
		{
			const TVector2 vect0 = p3-p0;
			const TVector2 vect1 = p3-p1;
			if( RealAbs(vect0^vect1) < kRealEpsilon )
			{
				// The 3 points are aligned, check if p3 is in between
				if(vect0*vect1<0)
				{
					if( onWall->Split(point3) )
					{
						// Continue checking the remaining pieces
						CheckWallConsistency( onWall, iWall, canDelete );
						return true;
					}
					else
						return false;
				}
			}
		}
		else if(point3==point0 || point3==point1)
		{
			const TVector2 vect0 = p2-p0;
			const TVector2 vect1 = p2-p1;
			if( RealAbs(vect0^vect1) < kRealEpsilon )
			{
				// The 3 points are aligned, check if p2 is in between
				if(vect0*vect1<0)
				{
					if( onWall->Split(point2) )
					{
						// Continue checking the remaining pieces
						CheckWallConsistency( onWall, iWall, canDelete );
						return true;
					}
					else
						return false;
				}
			}
		}
		else
		{

			TVector2 intersection;
			EIntersectType type = GetSegmentsIntersection(intersection, p0, p1, p2, p3);
			if( type == eIntersection )
			{	// Split the wall and check the 2 resulting walls for other intersections
				VPoint* newPoint1 = onWall->Split(intersection);
				if(newPoint1)
				{
					Wall* newWall = newPoint1->GetWall(1);
					if(newWall==onWall)
						newWall = newPoint1->GetWall((int32)0);

					// Split also the intersected wall
					otherWall->Split(newPoint1);

					CheckWallConsistency( newWall, iWall, canDelete );
				}
				else
				{
					boolean splitted = false;
					if(p0.IsEqual(intersection,kRealEpsilon))
						splitted = onWall->Split(point0);
					else
						splitted = onWall->Split(point1);

					if(!splitted)
					{	// Then 2 points are at the same position => merge them
						VPoint* firstPoint=NULL;
						VPoint* secondPoint=NULL;
						if(p0.IsEqual(intersection,kRealEpsilon))
							firstPoint = point0;
						else
							firstPoint = point1;

						if(p2.IsEqual(intersection,kRealEpsilon))
							secondPoint = point2;
						else
							secondPoint = point3;

						firstPoint->Merge(secondPoint, canDelete);
					}
				}
				// Continue checking the remaining pieces
				CheckWallConsistency( onWall, iWall, canDelete );

				return true;
			}
		}
	}

	return false;
}

void Plan::RebuildInvalidRooms()
{
	const int32 wallCount = fWallArray.GetElemCount();

	ClearWallFlag(eWallRoom0Done);
	ClearWallFlag(eWallRoom1Done);

	// Keep track of the room objects to replace them later in the most appropriate room
	TMCCountedPtrArray<RoomSubObject> rescueObjects;

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];
		if(	wall->Flag(eWallRebuildRoom0) )
		{
			wall->ClearFlag(eWallRebuildRoom0);
			Room* room0 = wall->GetRoom(0);
			boolean canRebuild = false;
			real32 floorThickness = 0, ceilingThickness = 0;
			int32 flags=0, floorDomain=0, ceilingDomain=0;
			boolean noCeiling = false;
			if(room0)
			{
				// Keep some data from the previous room
				floorThickness = room0->GetFloorThickness();
				ceilingThickness = room0->GetCeilingThickness();
				noCeiling = room0->NoCeiling();
				floorDomain = room0->GetFloorDomain();
				ceilingDomain = room0->GetCeilingDomain();
				flags = room0->GetFlags();

				const int32	objCount = room0->GetObjectCount();
				for(int32 iObj=0 ; iObj<objCount ; iObj++)
				{
					rescueObjects.AddElem(room0->GetObject(iObj));
				}
				// Delete the old room
				room0->DeleteRoom();

				canRebuild = true;

				// Build the new one
				fLevel->BuildPossibleRooms(wall,true,false);
			}
			else
			{	// The room has been possibly deleted when rebuilding a room elsewhere
				// Check if there's a new room on the other side of the wall, if so, use its data
				Room* room1 = wall->GetRoom(1);
				if(room1 && wall->Flag(eWallRoom1Done))
				{
					floorThickness = room1->GetFloorThickness();
					ceilingThickness = room1->GetCeilingThickness();
					noCeiling = room1->NoCeiling();
					floorDomain = room1->GetFloorDomain();
					ceilingDomain = room1->GetCeilingDomain();
					flags = room1->GetFlags();
					canRebuild = true;

					// Build the new one
					fLevel->BuildPossibleRooms(wall,true,false);
				}
			}

			if(canRebuild)
			{
				room0 = wall->GetRoom(0);
				if(room0)
				{	
					// Set the saved data
					room0->SetFloorThickness(floorThickness);
					room0->SetCeilingThickness(ceilingThickness);
					room0->NoCeiling( noCeiling );
					room0->SetFloorDomain(floorDomain);
					room0->SetCeilingDomain(ceilingDomain);
					room0->SetFlags(flags);
					room0->SetFlag(eTurnLeft); // Because we just overrided it with fFlags
					room0->ClearFlag(eRoomTessellated);
					room0->ClearFlag(eRoomBasicTessellated);
					room0->ClearFlag(eIsTargeted);

					// Clear the flags
					const int32 wCount = room0->GetPathPointCount();
					for(int32 iW=0 ; iW<wCount ; iW++)
					{
						Wall* roomWall = room0->GetPathWall(iW);
						const int32 roomIndex = roomWall->GetRoomIndex(room0);
						if(roomIndex==0)
						{
							roomWall->SetFlag(eWallRoom0Done);
							roomWall->ClearFlag(eWallRebuildRoom0);
						}
						else
						{
							roomWall->SetFlag(eWallRoom1Done);
							roomWall->ClearFlag(eWallRebuildRoom1);
						}
					}
				}
			}
		}
		if(	wall->Flag(eWallRebuildRoom1) )
		{
			wall->ClearFlag(eWallRebuildRoom1);
			Room* room1 = wall->GetRoom(1);
			boolean canRebuild = false;
			real32 floorThickness = 0, ceilingThickness = 0;
			boolean noCeiling = false;
			int32 flags=0, floorDomain=0, ceilingDomain=0;
			if(room1)
			{
				// Keep some data from the previous room
				floorThickness = room1->GetFloorThickness();
				ceilingThickness = room1->GetCeilingThickness();
				noCeiling = room1->NoCeiling();
				floorDomain = room1->GetFloorDomain();
				ceilingDomain = room1->GetCeilingDomain();
				flags = room1->GetFlags();
		
				const int32	objCount = room1->GetObjectCount();
				for(int32 iObj=0 ; iObj<objCount ; iObj++)
				{
					rescueObjects.AddElem(room1->GetObject(iObj));
				}
				// Delete the old room
				room1->DeleteRoom();

				canRebuild = true;

				// Build the new one
				fLevel->BuildPossibleRooms(wall,false,true);
			}
			else
			{	// The room has been possibly deleted when rebuilding a room elsewhere
				// Check if there's a new room on the other side of the wall, if so, use its data
				Room* room0 = wall->GetRoom(0);
				if(room0 && wall->Flag(eWallRoom0Done))
				{
					floorThickness = room0->GetFloorThickness();
					ceilingThickness = room0->GetCeilingThickness();
					noCeiling = room0->NoCeiling();
					floorDomain = room0->GetFloorDomain();
					ceilingDomain = room0->GetCeilingDomain();
					flags = room0->GetFlags();
					canRebuild = true;

					// Build the new one
					fLevel->BuildPossibleRooms(wall,false,true);
				}
			}

			if(canRebuild)
			{
				room1 = wall->GetRoom(1);
				if(room1)
				{	
					// Set the saved data
					room1->SetFloorThickness(floorThickness);
					room1->SetCeilingThickness(ceilingThickness);
					room1->NoCeiling(noCeiling);
					room1->SetFloorDomain(floorDomain);
					room1->SetCeilingDomain(ceilingDomain);
					room1->SetFlags(flags);
					room1->ClearFlag(eTurnLeft); // Because we just overrided it with fFlags
					room1->ClearFlag(eRoomTessellated);
					room1->ClearFlag(eRoomBasicTessellated);
					room1->ClearFlag(eIsTargeted);

					// Clear the flags
					const int32 wCount = room1->GetPathPointCount();
					for(int32 iW=0 ; iW<wCount ; iW++)
					{
						Wall* roomWall = room1->GetPathWall(iW);
						const int32 roomIndex = roomWall->GetRoomIndex(room1);
						if(roomIndex==0)
						{
							roomWall->SetFlag(eWallRoom0Done);
							roomWall->ClearFlag(eWallRebuildRoom0);
						}
						else
						{
							MY_ASSERT(roomIndex==1);
							roomWall->SetFlag(eWallRoom1Done);
							roomWall->ClearFlag(eWallRebuildRoom1);
						}
					}
				}
			}
		}
	}

	// Now that all the rooms have been rebuild, replace the objects
	const int32 objectCount = rescueObjects.GetElemCount();
	for(int32 iObj=0 ; iObj<objectCount ; iObj++)
	{
		RoomSubObject* obj = rescueObjects[iObj];
		const TVector2& center = obj->GetPolylineCenter();
		Room* inRoom = PosInRoom(center);
		if(inRoom)
		{
			obj->SetInRoom(inRoom);
		}
	}
}

// Deleting a room or a wall does not necessarily delete the vertices under.
void Plan::DeleteSelection()
{
	BuildingPrimData* data = fLevel->GetPrimitiveData();
	const boolean onSubObjectPoints = data->GetHoleEditEnable();

	SetPointFlag(ePointToDelete);

	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=roofCount-1 ; iRoof>=0 ; iRoof--)
	{
		Roof* roof = fRoofArray[iRoof];
		if(roof->Selected())
		{
			roof->DeleteRoof();
		}
		else
		{	// Delete the selected points
			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			boolean roofExist = true;

			for(int32 iPt=zoneSectionCount-1 ; iPt>=0&&roofExist ; iPt--)
			{
				ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				if( zoneSection.fZonePoint->Selected() ||
					zoneSection.fSpinePoint->Selected() )
				{
					roofExist = roof->RemoveRoofZoneSection(iPt);
				}
			}

			if(!roofExist)
				continue; // the roof has been deleted

			const int32 botCount = roof->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				ProfilePoint* point = roof->GetBotProfilePoint(iBot);
				if( point->Selected())
					point->DeleteProfilePoint();
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				ProfilePoint* point = roof->GetTopProfilePoint(iTop);
				if( point->Selected())
					point->DeleteProfilePoint();
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
				if( point->Selected())
					point->DeleteProfilePoint();
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
				if( point->Selected())
					point->DeleteProfilePoint();
			}
		}
	//	else
	//	{
	//		const int32 objCount = roof->GetObjectCount();	
	//		for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
	//		{
	//			RoofSubObject* obj = roof->GetObject( iObj );
	//			if( obj->Selected() )
	//			{
	//				obj->SetInRoof(NULL);
	//			}
	//		}
	//	}
	}

	const int32 roomCount = fRoomArray.GetElemCount();
	for(int32 iRoom=roomCount-1 ; iRoom>=0 ; iRoom--)
	{
		Room* room = fRoomArray[iRoom];
		if(room->Selected())
		{
			room->ClearPointFlag(ePointToDelete);
			room->DeleteRoom();
		}
		else
		{
			const int32 objCount = room->GetObjectCount();	
			for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
			{
				TMCCountedPtr<RoomSubObject> obj;
				obj = room->GetObject( iObj );
				if( onSubObjectPoints )		obj->DeleteSelectedPoints();
				else if( obj->Selected() )	obj->SetInRoom(NULL);
			}
		}
	}

	const int32 wallCount = fWallArray.GetElemCount();
	for(int32 iWall=wallCount-1 ; iWall>=0 ; iWall--)
	{
		Wall* wall = fWallArray[iWall];;
		if(wall->Selected())
		{
			wall->GetPoint(0)->ClearFlag(ePointToDelete);
			wall->GetPoint(1)->ClearFlag(ePointToDelete);
			wall->DeleteWall();
		}
		else
		{
			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=objCount-1 ; iObj>=0 ; iObj-- )
			{
				TMCCountedPtr<WallSubObject> obj;
				obj = wall->GetObject( iObj );
				if( onSubObjectPoints )		obj->DeleteSelectedPoints();
				else if( obj->Selected() )	obj->SetOnWall(NULL);
			}
		}
	}

	const int32 pointCount = fPointArray.GetElemCount();
	for(int32 iPt=pointCount-1 ; iPt>=0 ; iPt--)
	{
		VPoint* point = fPointArray[iPt];
		if(point->Selected())
		{
			if(point->Flag(ePointToDelete) || point->GetWallCount()==0 )
			{
				point->DeletePoint();
			}
		}
	}

	ClearPointFlag(ePointToDelete);
}

void Plan::InvertSelection()
{
	InitMarqueeSelection();
	const int32 pointCount = fPointArray.GetElemCount();
	for(int32 iPt=0 ; iPt<pointCount ; iPt++)
	{
		VPoint* point = fPointArray[iPt];
		point->InvertSelection();
	}

	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		Roof* roof = fRoofArray[iRoof];
		if(roof->Selected())
		{
			roof->SetSelection(false);
		}
		else
		{
			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				zoneSection.fZonePoint->SetSelection( !zoneSection.fZonePoint->Selected() );
				zoneSection.fSpinePoint->SetSelection( !zoneSection.fSpinePoint->Selected() );
			}

			const int32 botCount = roof->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				ProfilePoint* point = roof->GetBotProfilePoint(iBot);
				point->SetSelection( !point->Selected() );
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				ProfilePoint* point = roof->GetTopProfilePoint(iTop);
				point->SetSelection( !point->Selected() );
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
				point->SetSelection( !point->Selected() );
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
				point->SetSelection( !point->Selected() );
			}
		}
	}

	// Invert the selection on the level
	if(fLevel->Selected())
		fLevel->ClearFlag(eIsSelected);
	else
		fLevel->SelectIfPossible();
}

void Plan::SelectByName(const TMCString& name, const boolean select)
{
	// Points
	const int32 pointCount = fPointArray.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point = fPointArray[iPoint];

		if( point->GetName() == name)
		{
			if(select)
			{
				point->SetFlag(eIsSelected); // select only the point
			}
			else
				point->SetSelection(false); // deselect also the walls and rooms
		}
	}

	// Wall and wall objects
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->GetName() == name)
		{
			if(select)	
			{
				wall->SetFlag(eIsSelected); // select only the wall
				wall->GetPoint(0)->SetFlag(eIsSelected);
				wall->GetPoint(1)->SetFlag(eIsSelected);
			}
			else	
				wall->SetSelection(false); // deselect also the rooms
		}

		const int32 objCount = wall->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			SubObject* obj = wall->GetObject( iObj );

			if( obj->GetName() == name)
			{
				obj->SetSelection(select);
			}
		}
	}

	// Room and room objects
	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
	
		if( room->GetName() == name)
		{
			room->SetSelection(select);
		}

		const int32 objCount = room->GetObjectCount();
		for( int32 iObj=0 ; iObj< objCount ; iObj++ )
		{
			SubObject* obj = room->GetObject( iObj );

			if( obj->GetName() == name)
			{
				obj->SetSelection(select);
			}
		}
	}

	// Roof and roof points (zone and profile)
	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		{
			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);
				if( zoneSection.fZonePoint->GetName() == name )
				{
					zoneSection.fZonePoint->SetSelection(select);
				}
				if( zoneSection.fSpinePoint->GetName() == name )
				{
					zoneSection.fSpinePoint->SetSelection(select);
				}
			}
		}

		{
			const int32 botProfileCount = roof->GetBotProfilePointCount();
			for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
			{
				CommonPoint* point = roof->GetBotProfilePoint(iPt);
				if( point->GetName() == name )
				{
					point->SetSelection(select);
				}
			}
		}

		{
			const int32 topProfileCount = roof->GetTopProfilePointCount();
			for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
			{
				CommonPoint* point = roof->GetTopProfilePoint(iPt);
				if( point->GetName() == name )
				{
					point->SetSelection(select);
				}
			}
		}

		{
			const int32 botInsideCount = roof->GetBotInsidePointCount();
			for(int32 iPt=0 ; iPt<botInsideCount ; iPt++)
			{
				CommonPoint* point = roof->GetBotInsidePoint(iPt);
				if( point->GetName() == name )
				{
					point->SetSelection(select);
				}
			}
		}

		{
			const int32 topInsideCount = roof->GetTopInsidePointCount();
			for(int32 iPt=0 ; iPt<topInsideCount ; iPt++)
			{
				CommonPoint* point = roof->GetTopInsidePoint(iPt);
				if( point->GetName() == name )
				{
					point->SetSelection(select);
				}
			}
		}

		if( roof->GetName() == name)
		{
			roof->SetSelection(select);
		}
	}
}

void Plan::SelectByDomain(const int32 domain, const boolean select)
{
	// Wall
	const int32 wallCount = fWallArray.GetElemCount();
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		if( wall->GetLeftDomain() == domain ||
			wall->GetRightDomain() == domain ||
			wall->GetBetweenDomain() == domain )
		{
			wall->SetSelection(select);
		}
	}

	// Room
	const int32 roomCount = fRoomArray.GetElemCount();
	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];
	
		if( room->GetFloorDomain() == domain ||
			room->GetCeilingDomain() == domain)
		{
			room->SetSelection(select);
		}
	}

	// Roof
	const int32 roofCount = fRoofArray.GetElemCount();
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		if( roof->GetOutTopDomain() == domain ||
			roof->GetOutMidDomain() == domain ||
			roof->GetOutBotDomain() == domain ||
			roof->GetInsideDomain() == domain )
		{
			roof->SetSelection(select);
		}
	}
}

void Plan::FlipSelection( const TVector2& center, const int32 axis )
{
	const boolean flipX = (axis==eXAxis || axis==eZAxis);
	const boolean flipY = (axis==eYAxis || axis==eZAxis);

	const boolean flipOrder = (axis!=eZAxis);

	const real32 xc2 = 2*center.x;
	const real32 yc2 = 2*center.y;

	const int32 pointCount = fPointArray.GetElemCount();
	for(int32 iPt=0 ; iPt<pointCount ; iPt++)
	{
		VPoint* point = fPointArray[iPt];
		if(point->Selected())
		{
			const TVector2& pos = point->Position();

			point->ClearFlag(eWallOrdered);
			
			point->SetPosition(TVector2((flipX?xc2-pos.x:pos.x),
										(flipY?yc2-pos.y:pos.y) ));
		}
	}
	{
		const int32 roomCount = fRoomArray.GetElemCount();
		for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
		{
			Room* room = fRoomArray[iRoom];
		
			// Need also to move the room objects
			const int32 roomObjCount = room->GetObjectCount();

			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				SubObject* subObject = room->GetObject(iObj);
				if(subObject->Selected())
				{
					const TVector2& pos = subObject->GetPolylineCenter();
					subObject->SetPolylineCenter(TVector2((flipX?xc2-pos.x:pos.x),
												(flipY?yc2-pos.y:pos.y)),false);
				}
			}
		}
	}

	if(flipOrder)
	{
		const int32 roomCount = fRoomArray.GetElemCount();
		for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
		{
			Room* room = fRoomArray[iRoom];
			if(room->Selected())
			{
				if( room->Flag(eTurnLeft) )
					room->ClearFlag(eTurnLeft);
				else
					room->SetFlag(eTurnLeft);
			}
		}
		const int32 wallCount = fWallArray.GetElemCount();
		for(int32 iWall=wallCount-1 ; iWall>=0 ; iWall--)
		{
			Wall* wall = fWallArray[iWall];;
			if(wall->Selected())
			{
				Room* room0 = wall->GetRoom(0);
				Room* room1 = wall->GetRoom(1);
				wall->SetRoom(0,room1);
				wall->SetRoom(1,room0);
			}
		}
	}

	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		Roof* roof = fRoofArray[iRoof];
		const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

		for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
		{
			ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);
			
			if(zoneSection.fZonePoint->Selected())
			{
				const TVector2& pos = zoneSection.fZonePoint->Position();
				
				zoneSection.fZonePoint->SetPosition(TVector2((flipX?xc2-pos.x:pos.x),
															(flipY?yc2-pos.y:pos.y) ));
			}

			if(zoneSection.fSpinePoint->Selected())
			{
				const TVector2& pos = zoneSection.fSpinePoint->Position();
				
				zoneSection.fSpinePoint->SetPosition(TVector2((flipX?xc2-pos.x:pos.x),
															(flipY?yc2-pos.y:pos.y) ));
			}
		}
	}
}

// Return false if there's no closed path from point1 to point2
boolean Plan::BuildPath(	Wall* onWall, 
							TMCCountedPtrArray<VPoint>& path,
							const boolean turnLeft )
{
	path.SetElemCount(0);

	if(onWall->GetRoom(0) && turnLeft)
		return false; // room already exist on left

	if(onWall->GetRoom(1) && !turnLeft)
		return false; // room already exist on right

	VPoint* startPoint = onWall->GetPoint(0);
	if(startPoint->GetWallCount()==1)
		return false;

	VPoint* currentPoint = onWall->GetPoint(1);
	path.AddElem(startPoint);

	VPoint* lastElem = NULL;//startPoint;
	VPoint* beforeLastElem = NULL;

	
	Wall* currentWall = onWall;
	TVector2 currentDir = currentWall->GetStraightDirection(currentPoint);

	real32 angle = 0;
	boolean justFlipped = false;
	do
	{
		Wall* nextWall = turnLeft?currentPoint->GetRightWall(currentWall)
							:currentPoint->GetLeftWall(currentWall);

		if(nextWall!=currentWall) // Otherwise it's an extremity, don't add the point to the path
		{	
			justFlipped = false;

			TVector2 nextDir = nextWall->GetStraightDirection(currentPoint);

			if(currentPoint == lastElem)
			{	// Don't add it again, it's already in (we were on an extremity just before)	

				if(turnLeft)angle += GetPositiveAngle(nextDir, currentDir);
				else		angle += GetPositiveAngle(currentDir, nextDir);
			}
			else if(currentPoint == beforeLastElem)
			{	// we went back on our step, remove the last point of the array, it's a dead branch
				const int32 pathCount = path.GetElemCount();
				if(pathCount>0)
					path.RemoveElem(pathCount-1, 1);
				lastElem = beforeLastElem;
				if(pathCount>2)
					beforeLastElem = path[pathCount-3];
				else
					beforeLastElem = NULL;

				angle-=360;

				// Add the missing angle value from the wall we just removed
				if(turnLeft)angle += GetPositiveAngle(nextDir, currentDir);
				else		angle += GetPositiveAngle(currentDir, nextDir);
			}
			else
			{	// Add the point to the path
				beforeLastElem = lastElem;
				lastElem = currentPoint;
				path.AddElem(currentPoint);

				if(turnLeft)angle += GetPositiveAngle(nextDir, currentDir);
				else		angle += GetPositiveAngle(currentDir, nextDir);
			}

			currentDir = -nextDir;
		}
		else
		{	// Same wall: just flip the direction
			if(justFlipped)
				return false;
			currentDir*=-1;
			justFlipped = true;
		}

		currentWall = nextWall;
		currentPoint = currentWall->GetOtherPoint(currentPoint);
	}
	while(currentPoint!=startPoint);

	// Now verify that the path is an inside path, not an outside one
	const int32 pointCount = path.GetElemCount();
	
	// Angle/180 should be equal to pointCount-2 if OK, pointCount+2 if wrong
	if(pointCount>2 && pointCount>angle/180)
		return true;
	else
		return false;
}

boolean Plan::BuildRoomFromSelection()
{
	boolean couldBuild = false;
	TMCCountedPtrArray<VPoint> path;

	const int32 wallCount = fWallArray.GetElemCount();
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		Wall* wall = fWallArray[iWall];

		if( wall->Selected() )
		{
			if( BuildPath( wall, path, true ) )
			{
				// Build a room with this path
				/*Room* room = */fLevel->MakeRoom(path,true);
				path.SetElemCount(0);
				couldBuild = true;
			}
			if( BuildPath( wall, path, false ) )
			{
				// Build a room with this path
				/*Room* room = */fLevel->MakeRoom(path,false);
				path.SetElemCount(0);
				couldBuild = true;
			}
		}
	}

	return couldBuild;
}

void Plan::CutWallWithRoofs(Wall* wall)
{
	const int32 roofCount = fRoofArray.GetElemCount();
	if(!roofCount)
		return;

	// Note: we could cache that to improve performances
	TMCArray<TriangleVertices> roofTriangles;
	real32 roofMin=kBigRealValue, roofMax=-kBigRealValue;
	const real32 levelAltitude = fLevel->GetDistanceToGround();

	// Get the triangle we're going to use for the boolean operation
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		const real32 min = levelAltitude + roof->GetRoofMin();
		if(min<roofMin)
			roofMin = min;

		const real32 max = levelAltitude + roof->GetRoofMax();
		if(max>roofMax)
			roofMax = max;

		TMCArray<TriangleVertices> triangles;
		roof->GetInsideTriangleVertices(triangles);

		roofTriangles.Append( triangles );
	}

	// Cut the wall geometry
	wall->CutGeometry(roofTriangles,roofMin,roofMax);
}

void Plan::CutRoomWithRoofs(Room* room)
{
	const int32 roofCount = fRoofArray.GetElemCount();
	if(!roofCount)
		return;

	// Note: we could cache that to improve performances
	TMCArray<TriangleVertices> roofTriangles;
	real32 roofMin=kBigRealValue, roofMax=-kBigRealValue;
	const real32 levelAltitude = fLevel->GetDistanceToGround();

	// Get the triangle we're going to use for the boolean operation
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		const real32 min = levelAltitude + roof->GetRoofMin();
		if(min<roofMin)
			roofMin = min;

		const real32 max = levelAltitude + roof->GetRoofMax();
		if(max>roofMax)
			roofMax = max;

		TMCArray<TriangleVertices> triangles;
		roof->GetInsideTriangleVertices(triangles);

		roofTriangles.Append( triangles );
	}
	
	// Cut the ceiling
	room->CutGeometry(roofTriangles, roofMin, roofMax);
}

void Plan::CutGeometryWithRoofs()
{
	const int32 roofCount = fRoofArray.GetElemCount();
	if(!roofCount)
		return;
	const int32 wallCount = fWallArray.GetElemCount();
	if(!wallCount)
		return;

	TMCArray<TriangleVertices> roofTriangles;
	real32 roofMin=kBigRealValue, roofMax=-kBigRealValue;
	const real32 levelHeight = fLevel->GetLevelHeight();

	// Get the triangle we're going to use for the boolean operation
	for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
	{
		Roof* roof = fRoofArray[iRoof];

		const real32 min = levelHeight + roof->GetRoofMin();
		if(min<roofMin)
			roofMin = min;

		const real32 max = levelHeight + roof->GetRoofMax();
		if(max>roofMax)
			roofMax = max;

		TMCArray<TriangleVertices> triangles;
		roof->GetInsideTriangleVertices(triangles);

		roofTriangles.Append( triangles );
	}

	// Cut the wall geometries
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		Wall* wall = fWallArray[iWall];
	
		wall->CutGeometry(roofTriangles,roofMin,roofMax);
	}

	// Cut also the ceilings
	const int32 roomCount = fRoomArray.GetElemCount();
	for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
	{
		Room* room = fRoomArray[iRoom];
	
		room->CutGeometry(roofTriangles,roofMin,roofMax);
	}
}
/*
void Plan::DecalChildren( const int32 afterIndex )
{
	const int32 wallCount = fWallArray.GetElemCount();

	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = fWallArray[iWall];

		const int32 wallObjCount = wall->GetObjectCount();

		for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
		{
			SubObject* subObject = wall->GetObject(iObj);
			if(subObject->GetChildIndex()>afterIndex)
			{
				subObject->SetChildIndex(subObject->GetChildIndex()-1);
			}
		}
	}

	const int32 roomCount = fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room = fRoomArray[iRoom];

		if (room->Hidden())
			continue;

		const int32 roomObjCount = room->GetObjectCount();

		for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
		{
			SubObject* subObject = room->GetObject(iObj);
			if(subObject->GetChildIndex()>afterIndex)
			{
				subObject->SetChildIndex(subObject->GetChildIndex()-1);
			}
		}
	}
}
*/
/////////////////////////////////////////////////////////////////////////////////////////
//
//
MCCOMErr Plan::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Plan');
	if (result) return result;

	// Index and write the points
	const int32 pointCount = fPointArray.GetElemCount();
	for(int32 iPt=0 ; iPt<pointCount ; iPt++)
	{
		VPoint* point = fPointArray[iPt];
		point->SetIndex(iPt);
		result = point->Write(stream);
		if (result) return result;
	}

	// Write the walls
	const int32 wallCount = fWallArray.GetElemCount();
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		result = fWallArray[iWall]->Write(stream);
		if (result) return result;
	}

	// Write the rooms
	const int32 roomCount = fRoomArray.GetElemCount();
	for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
	{
		result = fRoomArray[iRoom]->Write(stream);
		if (result) return result;
	}

	// Write the roofs
	const int32 roofCount = fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		result = fRoofArray[iRoof]->Write(stream);
		if (result) return result;
	}

	result=stream->PutEnd();
	return result;
}

MCCOMErr Plan::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	BuildingPrimData* data = fLevel->GetPrimitiveData();

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'Poin':
			{
				VPoint* newPoint = NULL;
				VPoint::CreatePoint(&newPoint, data, fLevel, TVector2::kZero, NULL);
				result = newPoint->Read(stream);
				if (result) return result;
 
				newPoint->ClearFlag(ePointToDelete);// Some point are saved with the ePointToDelete flag (error somewhere). Clean it here.
				// Clear other flags that are not necessary in undo/redo to prevent other errors
				newPoint->ClearFlag(ePointTessellated);
				newPoint->ClearFlag(ePointHelper);
			} break;
		case 'Wall':
			{
				Wall* newWall = NULL;
				Wall::CreateWall(&newWall, data, fLevel, NULL, NULL);
				result = newWall->Read(stream);
				if (result) return result;

				// Clear flags that are not necessary in undo/redo to prevent other errors
				newWall->ClearFlag(eWallTessellated);
				newWall->ClearFlag(eWallHelper);
			} break;
		case 'WCre':
			{
				Wall* newWall = NULL;
				WallWithCrenel::CreateWall(&newWall, data, fLevel, NULL, NULL);
				result = newWall->Read(stream);
				if (result) return result;

				// Clear flags that are not necessary in undo/redo to prevent other errors
				newWall->ClearFlag(eWallTessellated);
				newWall->ClearFlag(eWallHelper);
			} break;
		case 'Room':
			{
				TMCCountedPtrArray<VPoint> emptyPath;
				Room* newRoom = NULL;
				Room::CreateRoom(&newRoom, data, fLevel, emptyPath,true);
				result = newRoom->Read(stream);
				if (result) return result;

				// Clear flags that are not necessary in undo/redo to prevent other errors
				newRoom->ClearFlag(eRoomTessellated);
			} break;
		case 'Roof':
			{
				Roof* newRoof = NULL;
				Roof::CreateRoof(&newRoof, data, fLevel);
				result = newRoof->Read(stream);
				if (result) return result;
			} break;

		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

/****************************************************************************************************

		PLevel.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PLevel.h"

#include "MCCountedPtrHelper.h"
#include "I3dShFacetMesh.h"
#include "IShTokenStream.h"
#include "BuildingPrim.h"
#include "ArchiTools.h"
#include "PWallWithCrenel.h"

#include "Utils.h"
#include "MiscComUtilsImpl.h"
//Alan
#include "PPlan.h"

Level::Level(BuildingPrim* prim, Level* levelUnder, Level* levelOver)
: fLevelPlan(this)
{
	fLevelUnder = levelUnder;
	fLevelOver = levelOver;

	if(fLevelUnder)
		fLevelUnder->SetLevelOver(this);
	if(fLevelOver)
		fLevelOver->SetLevelUnder(this);

//	fFloorThickness = 0;
//	fFloorNumber = 0;
	fDistanceToGround = 0;
	fLevelHeight = kDefaultLevelHeight;
	fLevelIndex = 0;

	fFlags = 0;

	fBuildingPrimitive = prim;
	fData = &(prim->GetData());
}

Level::~Level()
{
}

void Level::CreateLevel(Level **newLevel, BuildingPrim* prim, Level* levelUnder, Level* levelOver)
{
	TMCCountedCreateHelper<Level> result(newLevel);

	result = new Level(prim,levelUnder,levelOver);
	ThrowIfNoMem(result);
}

void Level::DeleteLevel()
{
	// Link the levels under and over
	if(fLevelUnder)
		fLevelUnder->SetLevelOver(fLevelOver);
	if(fLevelOver)
		fLevelOver->SetLevelUnder(fLevelUnder);

	fLevelOver=NULL;
	fLevelUnder=NULL;

	fLevelPlan.DeletePlan();

	fBuildingPrimitive->RemoveLevelFromArray(fLevelIndex,1);
	fBuildingPrimitive = NULL; // It's not counted anymore, safer to set it to NULL
}

void Level::ReleaseReference()
{
	fBuildingPrimitive=NULL;
}

void Level::InvalidateTessellation()
{
	fLevelPlan.InvalidateTessellation();

	// BBox too
	fBuildingPrimitive->InvalidateBBox();
}

void Level::SetDistanceToGround(const real32 dist)
{
	if(dist==0)
	{	// Special case for the ground level: reset all
		fDistanceToGround = 0;
		if(fLevelOver)
		{
			fLevelOver->SetDistanceToGround(this->GetLevelHeight());
		}
		if(fLevelUnder)
		{
			fLevelUnder->SetDistanceToGround(-fLevelUnder->GetLevelHeight());
		}
	}
	else
	{
		const real32 offset = dist-fDistanceToGround;
		if(offset!=0)
		{
			fDistanceToGround=dist;
			InvalidateTessellation();

			if(fDistanceToGround>=0 && fLevelOver)
			{
				fLevelOver->SetDistanceToGround(fDistanceToGround+GetLevelHeight());
			}
			else if(fDistanceToGround<0 && fLevelUnder)
			{
				fLevelUnder->SetDistanceToGround(fDistanceToGround-fLevelUnder->GetLevelHeight());
			}
		}
	}
}


void Level::SetLevelHeight(const real32 h)
{
	const real32 offset = h==kDefaultLevelHeight?
		fData->GetDefaultWallHeight()-GetLevelHeight():
		h-GetLevelHeight();

	if(h==fLevelHeight)
		return;
	if(h==fData->GetDefaultLevelHeight() )
	{
		if(fLevelHeight==kDefaultLevelHeight)
			return;

		fLevelHeight=kDefaultLevelHeight;
	}
	else
		fLevelHeight=h;
	
	// Invalidation
	InvalidateTessellation();
	if(fDistanceToGround>=0 && fLevelOver)
	{
		fLevelOver->SetDistanceToGround(offset+fLevelOver->GetDistanceToGround());
	}
	else if(fDistanceToGround<0)
	{
		SetDistanceToGround(GetDistanceToGround()-offset);
	}
}

void Level::SetDefaultName()
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	const int32 groundIndex = fBuildingPrimitive->GetGroundLevelIndex();

	TMCDynamicString name;
	if(fLevelIndex>=groundIndex)
	{
		TMCString15 level;
		gResourceUtilities->GetIndString( level, kModelerStrings, 11);
		TMCString15 number;
		number.FromInt32(fLevelIndex-groundIndex+1);
		name = level+number;
	}
	else
	{
		TMCString15 basement;
		gResourceUtilities->GetIndString( basement, kModelerStrings, 12);
		TMCString15 number;
		number.FromInt32(groundIndex-fLevelIndex);
		name = basement+number;
	}

	SetName( fData->fDictionary, name );
}

void Level::SetSelection(const boolean select)
{
	if( select && Selected() )
		return;

	{
		if(select)	SetFlag(eIsSelected);
		else		ClearFlag(eIsSelected);

		// Modify the points, walls and rooms selection status
		const int32 pointCount = fLevelPlan.fPointArray.GetElemCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point= fLevelPlan.fPointArray[iPoint];

			if (point->Hidden())
				continue;

			if(select)	point->SetFlag(eIsSelected);
			else		point->ClearFlag(eIsSelected);
		}

		const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall= fLevelPlan.fWallArray[iWall];

			if (wall->Hidden())
				continue;

			if(select)
			{
				wall->SetFlag(eIsSelected);

				// Select the objects
				const int32 objectCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
				{
					SubObject* obj = wall->GetObject(iObj);
					if(obj->Hidden()) // Edition mode
						obj->SetOutlineFlag(eIsSelected);
					else 	// Object handle is visible
						obj->SetFlag(eIsSelected);
				}
			}
			else		
			{
				wall->ClearFlag(eIsSelected);

				// Deselect the objects
				const int32 objectCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
				{
					SubObject* obj = wall->GetObject(iObj);
					if(obj->Hidden()) // Edition mode
						obj->ClearOutlineFlag(eIsSelected);
					else 	// Object handle is visible
						obj->ClearFlag(eIsSelected);
				}
			}
		}

		const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room= fLevelPlan.fRoomArray[iRoom];

			if (room->Hidden())
				continue;

			if(select)
			{
				room->SetFlag(eIsSelected);

				// Select the objects
				const int32 objectCount = room->GetObjectCount();
				for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
				{
					SubObject* obj = room->GetObject(iObj);
					if(obj->Hidden()) // Edition mode
						obj->SetOutlineFlag(eIsSelected);
					else 	// Object handle is visible
						obj->SetFlag(eIsSelected);
				}
			}
			else
			{
				room->ClearFlag(eIsSelected);

				// Deselect the objects
				const int32 objectCount = room->GetObjectCount();
				for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
				{
					SubObject* obj = room->GetObject(iObj);
					if(obj->Hidden()) // Edition mode
						obj->ClearOutlineFlag(eIsSelected);
					else 	// Object handle is visible
						obj->ClearFlag(eIsSelected);
				}
			}
		}

		const int32 roofCount = fLevelPlan.fRoofArray.GetElemCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof= fLevelPlan.fRoofArray[iRoof];

			if (roof->Hidden())
				continue;

			if(select)
			{
				roof->SetSelection(true);

				// Select the objects
			//	const int32 objectCount = roof->GetObjectCount();
			//	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
			//	{
			//	}
			}
			else
			{
				roof->SetSelection(false);

				// Deselect the objects
			//	const int32 objectCount = roof->GetObjectCount();
			//	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
			//	{
			//	}
			}
		}

		fData->InvalidateStatus();
	}
}

void Level::ShowHide(const EShowHideOption option)
{
	switch(option)
	{
	case eShowAll:
		{
			ClearFlag(eIsHidden);
			fLevelPlan.ShowRooms();
			fLevelPlan.ShowWalls();
			fLevelPlan.ShowRoofs();
		} break;
	case eShowAllWalls:
		{
			ClearFlag(eIsHidden);
			fLevelPlan.ShowWalls();
		} break;
	case eShowAllRooms:
		{
			ClearFlag(eIsHidden);
			fLevelPlan.ShowRooms();
		} break;
	case eShowAllRoofs:
		{
			ClearFlag(eIsHidden);
			fLevelPlan.ShowRoofs();
		} break;
	case eShowAllHolePoints:
		{	// Show the hole points, hide the handle
			fLevelPlan.ShowHolePoints();
		} break;
	case eHideAllHolePoints:
		{	// Show the hole handle, hide the points
			fLevelPlan.HideHolePoints();
		} break;
	case eHideAll:
		{
			SetFlag(eIsHidden);
			ClearFlag(eIsSelected);
			fLevelPlan.HidePoints();
			fLevelPlan.HideRooms();
			fLevelPlan.HideWalls();
			fLevelPlan.HideRoofs();
		} break;
	case eHideAllWalls:
		{
			fLevelPlan.HideWalls();
		} break;
	case eHideAllRooms:
		{
			fLevelPlan.HideRooms();
		} break;
	case eHideAllRoofs:
		{
			fLevelPlan.HideRoofs();
		} break;
	case eHideSelection:
		{
			const int32 roofCount = fLevelPlan.fRoofArray.GetElemCount();
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof= fLevelPlan.fRoofArray[iRoof];
				if(roof->Selected())
				{
					roof->HideRoof();
				}
			}

			const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room= fLevelPlan.fRoomArray[iRoom];
				if(room->Selected())
				{
					room->HideRoom();
				}
			}

			const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall= fLevelPlan.fWallArray[iWall];

				if (wall->Selected())
				{
					wall->HideWall();
				}
			}
			const int32 pointCount = fLevelPlan.fPointArray.GetElemCount();

			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point= fLevelPlan.fPointArray[iPoint];
				if (point->Selected())
				{
					point->HidePoint();
				}
			}

			fData->InvalidateStatus();
		} break;
	}
}

void Level::SelectIfPossible()
{
	if( !Selected() )
	{
		// Check if all the walls are selected
		const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall= fLevelPlan.fWallArray[iWall];

			if (wall->Hidden())
				continue;

			if( !wall->Selected())
				return;
		}

		const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room= fLevelPlan.fRoomArray[iRoom];

			if (room->Hidden())
				continue;

			if( !room->Selected())
				return;
		}

		// Everything is selected: select the level
		SetFlag(eIsSelected);
	}
}

void Level::GetLevelFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags)
{
	FacetMesh::Create(outMesh);
	TMCCountedPtr<FacetMesh> facetMesh;
	facetMesh = *outMesh;

	// Warning: order of the facet meshes here should be the same as in UpdateFacetMeshColors
	
	const int32 pointCount = fLevelPlan.fPointArray.GetElemCount();

	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		VPoint* point= fLevelPlan.fPointArray[iPoint];

		if (MCVerify(point) && !point->Hidden())
		{
			TMCCountedPtr<FacetMesh> mesh;

			if( point->GetPointFacetMesh(&mesh, lodindex, meshFlags) )
				facetMesh->Append(*mesh);
		}
	}

	// Get the mesh of the walls
	if(!FLAG(meshFlags,e2DMesh))
	{
		const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall= fLevelPlan.fWallArray[iWall];

			if (MCVerify(wall))
			{
				if(!wall->Hidden())
				{
					TMCCountedPtr<FacetMesh> mesh;
					if( wall->GetWallFacetMesh(&mesh, lodindex, meshFlags) )
					{
						facetMesh->Append(*mesh);
					}
				}
			}
		}
	}

	// Get the mesh of the floor
	const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();

	for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
	{
		Room* room= fLevelPlan.fRoomArray[iRoom];

		if (MCVerify(room) && !room->Hidden())
		{
			TMCCountedPtr<FacetMesh> mesh;

			if( room->GetRoomFacetMesh(&mesh, lodindex, meshFlags) )
			{
				facetMesh->Append(*mesh);
			}
		}
	}

	// Get the mesh of the roofs
	if(!FLAG(meshFlags,e2DMesh)&&!FLAG(meshFlags, eNoTop)) // don't show the roof when in NoTop mode
	{
		const int32 roofCount = fLevelPlan.fRoofArray.GetElemCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = fLevelPlan.fRoofArray[iRoof];

			if (MCVerify(roof) && !roof->Hidden())
			{
				TMCCountedPtr<FacetMesh> mesh;

				if( roof->GetRoofFacetMesh(&mesh, lodindex, meshFlags) )
				{
					facetMesh->Append(*mesh);
				}
			}
		}
	}
}

VPoint* Level::MakePoint( const TVector2& pos )
{
	if( !CanCreatePoint(GetPointCount()) )
		return NULL; // DEMO VERSION: can create only a limited number of points

	TMCCountedPtr<VPoint> newPoint;
	try
	{
		VPoint::CreatePoint(&newPoint, fData, this, pos, NULL);

		fData->InvalidateStatus();
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return newPoint;
}

Wall* Level::MakeWall( VPoint* point1, VPoint* point2, EWallType type, const boolean tryBuildRoom )
{
	try
	{
		if(!point1 || !point2)
			return NULL;

		// Check first if there's not already a Wall between these 2 points
		if( point1->GetWall(point2) )
			return point1->GetWall(point2);

		Wall* newWall = NULL;

		// Then create the wall
		switch( type )
		{
		case eBasic:
			Wall::CreateWall(&newWall, fData,this, point1, point2);
			break;
		case eWithCrenel1:
			WallWithCrenel::CreateWall(&newWall, fData,this, point1, point2);
			static_cast<WallWithCrenel*>(*&newWall)->SetCrenelShape( eRectangular );
			break;
		case eWithCrenel2:
			WallWithCrenel::CreateWall(&newWall, fData,this, point1, point2);
			static_cast<WallWithCrenel*>(*&newWall)->SetCrenelShape( eSmallTop );
			break;
		case eWithCrenel3:
			WallWithCrenel::CreateWall(&newWall, fData,this, point1, point2);
			static_cast<WallWithCrenel*>(*&newWall)->SetCrenelShape( eDoubleTop );
			break;
		case eWithCrenel4:
			WallWithCrenel::CreateWall(&newWall, fData,this, point1, point2);
			static_cast<WallWithCrenel*>(*&newWall)->SetCrenelShape( eSlopeSides );
			break;
		case eWithCrenel5:
			WallWithCrenel::CreateWall(&newWall, fData,this, point1, point2);
			static_cast<WallWithCrenel*>(*&newWall)->SetCrenelShape( eSlopeTop );
			break;
		}

		if(tryBuildRoom && newWall)
		{
			// A wall between 2 existing points was created, see if we didn't create a 
			// new room at the same time.
			BuildPossibleRooms(newWall);
		}

		fData->InvalidateStatus();
	
		return newWall;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("Level::MakeWall"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("Level::MakeWall"));
	}

	return NULL;
}

Wall* Level::MakeWall( const TVector2& pos1, const TVector2& pos2 )
{
	try
	{
		VPoint* point0 = NULL;
		VPoint::CreatePoint(&point0, fData, this, pos1, NULL);
		VPoint* point1 = NULL;
		VPoint::CreatePoint(&point1, fData, this, pos2, NULL);

		Wall* newWall = NULL;
		Wall::CreateWall(&newWall, fData, this, point0, point1);

		fData->InvalidateStatus();
		return newWall;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("Level::MakeWall"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("Level::MakeWall"));
	}

	return NULL;
}

Room* Level::MakeRoom( TMCCountedPtrArray<VPoint>& path, const boolean turnleft )
{
	try
	{
		Room* newRoom = NULL;
		Room::CreateRoom(&newRoom, fData, this, path,turnleft);

		// Add the points that could be in
		fLevelPlan.AddPointInRoom(newRoom);

		fData->InvalidateStatus();
		return newRoom;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("Level::MakeRoom"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("Level::MakeRoom"));
	}

	return NULL;
}

Roof* Level::MakeRoof(const TMCCountedPtrArray<VPoint>& area, const ERoofType roofType)
{
	try
	{
		Roof* newRoof = NULL;
		Roof::CreateRoof(&newRoof, fData, this);

		// Check that the area is correctly oriented (trigo way)
		TMCClassArray<TVector2> polygon;
		int32 areaCount = area.GetElemCount();
		for(int32 iArea=0 ; iArea<areaCount ; iArea++)
		{
			polygon.AddElem( area[iArea]->Position() );
		}

		double areaValue = PolygonArea( polygon );
		if( areaValue>0 )
		{
			TMCCountedPtrArray<VPoint> flippedArea;
			for(int32 iArea=areaCount ; iArea>0 ; iArea--)
			{
				flippedArea.AddElem( area[iArea-1] );
			}
		
			newRoof->SetArea(flippedArea,roofType);
		}
		else
		{
			newRoof->SetArea(area,roofType);
		}

		fData->InvalidateStatus();
		return newRoof;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("Level::MakeRoof"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("Level::MakeRoof"));
	}

	return NULL;
}

void CompleteBBox(TBBox3D& bbox, SubObject* obj)
{
	if(!obj)
		return;

	if(obj->Hidden()) // Edition mode
	{
		const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
		TMCClassArray<TVector3> side0;
		TMCClassArray<TVector3> side1;
		TMCClassArray<TVector3> normals;
		obj->Get3DOutlines(side0,side1,normals);
		const int32 ptCount = outline.GetElemCount();
		for(int32 iPt=0 ; iPt<ptCount ; iPt++)
		{
			if(outline[iPt]->Selected())
			{
				if(bbox.Valid()) bbox.AddPoint(side0[iPt]);
				else bbox.fMin=bbox.fMax=side0[iPt];

				bbox.AddPoint(side1[iPt]);
			}
		}
	}
	else if(obj->Selected())
	{
		TVector3 center;
		obj->GetCenter(center);
		if(bbox.Valid()) bbox.AddPoint(center);
		else bbox.fMin=bbox.fMax=center;
	}
}

void Level::GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection)
{
	//TVector2 minValue(kBigRealValue,kBigRealValue);
	//TVector2 maxValue(-kBigRealValue,-kBigRealValue);

	boolean hasPoints=false;
	{
		TBBox3D wallBbox(TVector3(kBigRealValue,kBigRealValue,kBigRealValue),TVector3(-kBigRealValue,-kBigRealValue,-kBigRealValue));
		if( fLevelPlan.GetWallBBox(wallBbox, exact, onSelection) )
		{
			bbox = wallBbox;
			hasPoints = true;
		}
	}

	{
		// We need to add the single points (to fix a problem with the software renderer)
		const int32 pointCount = fLevelPlan.fPointArray.GetElemCount();
		TMCCountedPtrArray<VPoint> newPoints(pointCount);
		for(int32 iPt=0 ; iPt<pointCount ; iPt++)
		{
			if(fLevelPlan.GetPoint(iPt)->GetWallCount() == 0)
			{
				if(bbox.Valid())
					bbox.AddPoint(fLevelPlan.GetPoint(iPt)->Get3DPos());
				else
					bbox.fMin = bbox.fMax = fLevelPlan.GetPoint(iPt)->Get3DPos();
			}
		}
	}

	// Add the roofs bbox
	const int32 roofCount = fLevelPlan.fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		TBBox3D roofBbox(TVector3(kBigRealValue,kBigRealValue,kBigRealValue),TVector3(-kBigRealValue,-kBigRealValue,-kBigRealValue));
		fLevelPlan.fRoofArray[iRoof]->GetBoundingBox(roofBbox,exact,onSelection);

		if(roofBbox.Valid())
		{
			if(bbox.Valid())
				bbox+=roofBbox;
			else
				bbox=roofBbox;
		}
	}
	
	// When working on the selection, we need to check the objects if the wall is not selected
	if(onSelection)
	{
		const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall= fLevelPlan.fWallArray[iWall];

			if (wall->Hidden())
				continue;

			if (!wall->Selected())
			{
				const int32 objCount = wall->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
				{
					CompleteBBox( bbox, wall->GetObject(iObj) );
				}
			}
		}

		const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room= fLevelPlan.fRoomArray[iRoom];

			if (room->Hidden())
				continue;

			if (!room->Selected())
			{
				const int32 objCount = room->GetObjectCount();
				for( int32 iObj=0 ; iObj<objCount ; iObj++ )
				{
					CompleteBBox( bbox, room->GetObject(iObj) );
				}
			}
		}
	}
}

void Level::BuildPossibleRooms(Wall* onWall, const boolean onLeft, const boolean onRight)
{
	if(onWall->GetRoom(0) && onWall->GetRoom(1))
		return; // This wall already has rooms. 

	if(onLeft)
	{
		TMCCountedPtrArray<VPoint> path1;
		if( fLevelPlan.BuildPath( onWall, path1, true ) )
		{
			// Build a room with this path
			/*Room* room0 = */MakeRoom(path1,true);
		}
	}

	if(onRight)
	{
		TMCCountedPtrArray<VPoint> path2;
		if( fLevelPlan.BuildPath( onWall, path2, false ) )
		{
			// Build a room with this path
			/*Room* room1 = */MakeRoom(path2,false);
		}
	}
}

void Level::GetInstances( TMCCountedPtrArray<I3DShInstance>& instances )
{
	if(fBuildingPrimitive)
		fBuildingPrimitive->GetInstances(instances);
}

void Level::Clone(Level** newLevel, BuildingPrim* inPrimitive, const ECloneChildrenMode cloneMode)
{
	Level::CreateLevel( newLevel, inPrimitive, NULL, NULL );
	
	TMCCountedPtr<Level> levelPtr;
	levelPtr = *newLevel;

	levelPtr->SetLevelHeight( fLevelHeight );
//	levelPtr->SetLevelHeight( fDistanceToGround ); // need to clone this value, it's used in other methods during cloning
	levelPtr->SetFlags(fFlags);
	levelPtr->SetNamePtr(fName);

	// Clone the level index, it can be usefull for the copy/paste action
	levelPtr->SetLevelIndex( fLevelIndex );

	// Clone the points
	const int32 pointCount = fLevelPlan.fPointArray.GetElemCount();
	TMCCountedPtrArray<VPoint> newPoints(pointCount);
	for(int32 iPt=0 ; iPt<pointCount ; iPt++)
	{
		// Note: this clone is not complete, we still need to add the walls		
		fLevelPlan.fPointArray[iPt]->Clone(newPoints.Pointer(iPt),levelPtr);
		// Set the same index on both so we can find them later when
		// cloning the walls and rooms
		newPoints[iPt]->SetIndex(iPt);
		fLevelPlan.fPointArray[iPt]->SetIndex(iPt);
	}

	// Clone the walls
	const int32 wallCount = fLevelPlan.fWallArray.GetElemCount();
	for(int32 iWall=0 ; iWall<wallCount ; iWall++)
	{
		Wall* curWall = fLevelPlan.fWallArray[iWall];
		
		const int32 ip0 = curWall->GetPoint(0)->GetIndex();
		const int32 ip1 = curWall->GetPoint(1)->GetIndex();

		TMCCountedPtr<Wall> newWall;
		curWall->Clone(&newWall,levelPtr,newPoints[ip0],newPoints[ip1],cloneMode);

	}

	// Clone the rooms
	const int32 roomCount = fLevelPlan.fRoomArray.GetElemCount();
	for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
	{
		TMCCountedPtr<Room> newRoom;
		fLevelPlan.fRoomArray[iRoom]->Clone(&newRoom,levelPtr,newPoints,cloneMode);
	}

	// Clone the roofs
	const int32 roofCount = fLevelPlan.fRoofArray.GetElemCount();
	for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
	{
		TMCCountedPtr<Roof> newRoof;
		fLevelPlan.fRoofArray[iRoof]->Clone(&newRoof,levelPtr);
	}

	// Clear flag
	levelPtr->ClearFlag(eIsTargeted);
}

//////////////////////////////////////////////////////////////////////////
//
//

MCCOMErr Level::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Leve');
	if (result) return result;

	// Level Height
	result=stream->PutKeyword('LevH');
	if (result) return result;
	result=stream->PutQuickFix(fLevelHeight);
	if (result) return result;

	// Level plan
	result=fLevelPlan.Write(stream);
	if (result) return result;

	// Common
	result=CommonBase::Write(stream);
	if (result) return result;

	result=stream->PutEnd();
	return result;
}

MCCOMErr Level::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'LevH': 
			{
				result=stream->GetQuickFix(&fLevelHeight);				
				if (result) return result;
			} break;
		case 'Plan':
			{
				result=fLevelPlan.Read(stream);
				if (result) return result;
			} break;

		default:
			CommonBase::Read(stream,keyword,fData);
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	ClearFlag(eIsTargeted);
	ClearFlag(eWasSelected);
	ClearFlag(eSnapedPosition);
	
	return result;
}

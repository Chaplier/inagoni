/****************************************************************************************************

		MCreateActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/25/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MCreateActions.h"
#include "MBuildingAction.h"
#include "PBuildingVisitor.h"



#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "Geometry.h"


BuildWallAction::BuildWallAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const TMCPoint&		mousePos,
									EWallType			type)
									:
									fPicked(modeler,pane,mousePos, ePickWall+ePick2D),ModelerAction(modeler),
									BuildingRecorder()
{
	fPanePart = pane;
	mWallType = type;
	fRefreshGeometry = true;
	fMousePos = mousePos;
	SetInLevel(); // determine in which level we're working
}

void BuildWallAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const TMCPoint&		mousePos,
								EWallType			type)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new BuildWallAction(modeler, pane, mousePos, type) );
}

void BuildWallAction::CloseWall(Wall* prevWall)
{
	bool isValid = prevWall->GetPoint(0) && prevWall->GetPoint(1);

	fBuildingModeler->SetPointHelper(NULL,true);
	fBuildingModeler->SetWallHelper(NULL,true);
	
	if( isValid )
	{	// If the wall is valid
		Level* currentLevel = fBuildingPrimitive->GetLevel(fInLevel);

		currentLevel->LevelPlan().RebuildInvalidRooms();
		currentLevel->BuildPossibleRooms(prevWall);

		if( mWallType!=eBasic )
		{	// No ceiling on the Room
			Room* room0 = prevWall->GetRoom( 0 );
			if( room0 ) room0->NoCeiling( true );
			
			Room* room1 = prevWall->GetRoom( 1 );
			if( room1 ) room1->NoCeiling( true );
		}
	}
}

MCCOMErr BuildWallAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		VPoint* prevPoint = fBuildingModeler->GetPointHelper();

		boolean canClose = false;
		VPoint* newPoint = fBuildingModeler->SnapPointPos(	fPicked,
															fPanePart,
															fMousePos,
															prevPoint,
															fInLevel,
															true,
															canClose);

		if(!newPoint)
			return ModelerAction::Do();

		Level* currentLevel = fBuildingPrimitive->GetLevel(fInLevel);

		// Select only the new point to display its properties
		fBuildingPrimitive->SetSelection(false);
		newPoint->SetSelection(true);
			
		// Check if we already have a starting point
		boolean canBuildWall=true;
		if(!prevPoint)
		{	// it's the first point we're building	
			canClose = false;

			// If the new point is already part of the geometry, build a new
			// one that we can move
			if(newPoint->GetWallCount())
			{
				// Double the new one with a small offset to directly show the wall to the user.
				// A second click will validate the 2nd point position
				TVector2 pickedPos=newPoint->Position();

				pickedPos.x+=fBuildingPrimitive->Data().GetDefaultWallThickness();
				prevPoint = newPoint;

				newPoint = fBuildingPrimitive->MakePoint(pickedPos,fInLevel);
			}
			else
			{
				// The new point is a new free point
				// Wait for the user to click again to fix it somewhere
				fBuildingModeler->SetPointHelper(newPoint,true);
				fBuildingModeler->SetWallHelper(NULL,false);

				canBuildWall=false;
			}
		}

		Wall* prevWall = fBuildingModeler->GetWallHelper();
		if(!prevWall && prevPoint)
		{
			Wall* newWall = fBuildingPrimitive->MakeWall(prevPoint,newPoint,mWallType,fInLevel);

			// Mark the newly made wall, to be sure to try to build the room on both sides
			newWall->SetFlag(eWallRebuildRoom0);
			newWall->SetFlag(eWallRebuildRoom1);

			prevPoint->SetSelection(false);
			fBuildingModeler->SetPointHelper(newPoint,false);
			fBuildingModeler->SetWallHelper(newWall,false);
			currentLevel->LevelPlan().RebuildInvalidRooms();
		}
		else
		{
			if(canBuildWall)
			{
				// Mark the wall helper, to be sure to try to build the room on both sides

#ifndef USE_POINT_IN// Does not work when adding a point inside a room
				prevWall->SetFlag(eWallRebuildRoom0);
				prevWall->SetFlag(eWallRebuildRoom1);
#endif

				if(canClose)
				{
					// Special case when we clicked on a previous point to stop the construction:
					// We created a wall with 2 points at the same place that we need to delete
					VPoint* before = prevWall->GetOtherPoint(prevPoint);
					if(before==newPoint)
					{
						prevPoint->DeletePoint(); // this will delete the wall at the same time
						fBuildingModeler->SetPointHelper(NULL,false);
						fBuildingModeler->SetWallHelper(NULL,false);
						currentLevel->LevelPlan().RebuildInvalidRooms();
					}
					else
					{
						CloseWall(prevWall);
					}
				}
				else
				{
					Wall* newWall = fBuildingPrimitive->MakeWall(prevPoint,newPoint,mWallType,fInLevel);

					fBuildingModeler->SnapCommonPoint(newPoint, fBuildingModeler->GetDirections(), gActionManager->IsCommandDown(),  gActionManager->IsShiftDown());

					prevPoint->SetSelection(false);
					if( fBuildingModeler->SetPointHelper(newPoint,true) )
					{	// The point helper was merged (in an existing point, wall or room) => stop the wall creation
						CloseWall(prevWall);
					}
					else
					{	// Normal case: continue to build the wall
						fBuildingModeler->SetWallHelper(newWall,true);

						currentLevel->LevelPlan().RebuildInvalidRooms();
					}
				}
			}
		}

		// Selection was modified
		fBuildingPrimitive->BuildExtendedSelection();

		// Clear this flag (was used for the point and wall color)
		fBuildingPrimitive->ClearPointFlag(eSnapedPosition,fInLevel);
		fBuildingPrimitive->ClearWallFlag(eSnapedPosition,fInLevel);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildWallAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildWallAction::Do"));
	}

	return ModelerAction::Do();
}

void BuildWallAction::SetInLevel()
{
	fInLevel=0;

	VPoint* prevPoint = fBuildingModeler->GetPointHelper();
	if(prevPoint)
	{	// The building process already started, use the current level
		fInLevel = prevPoint->GetLevelIndex();
		return;
	}

	switch(fPicked.GetPickedType())
	{
	case eWallPicked:
	case eEdgePicked:
		{	// We'll use the wall to start the new one
			Wall* wall = static_cast<Wall*>(fPicked.PickedObject());
			fInLevel = wall->GetPoint(0)->GetLevelIndex();
		} break;
	case ePointPicked:
		{	// Use the picked point
			VPoint* pickedPoint = static_cast<VPoint*>(fPicked.PickedObject());
			fInLevel = pickedPoint->GetLevelIndex();
		} break;
	default:
		{	// Get the current altitude 
			if(fPanePart->UsePlanMesh())
			{
				fInLevel = fBuildingModeler->Get2DActiveLevel();
				if(fInLevel==kAllLevels)
					fInLevel = fBuildingModeler->Get3DActiveLevel();
			}
			else
			{
				fInLevel = fBuildingModeler->Get3DActiveLevel();
				if(fInLevel==kAllLevels)
					fInLevel = fBuildingModeler->Get2DActiveLevel();
			}
			if(fInLevel==kAllLevels)
				fInLevel=0;
		} break;
	}
}

bool IsWallTool( int32 currentTool )
{
	return (	currentTool==kBuildWallTool ||
				currentTool==kBuildWallWithCrenel1Tool ||
				currentTool==kBuildWallWithCrenel2Tool ||
				currentTool==kBuildWallWithCrenel3Tool ||
				currentTool==kBuildWallWithCrenel4Tool ||
				currentTool==kBuildWallWithCrenel5Tool );
}

MCCOMErr BuildWallAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	VPoint* prevPointHelper = fBuildingPrimitive->GetPointHelper(fInLevel);
	
	int32 currentTool=0;
	gMenuUtilities->GetCurrentGlobalTool(currentTool); 
	
	if( IsWallTool( currentTool ) && prevPointHelper)
	{
		fBuildingModeler->SetPointHelper(prevPointHelper,false);
		if(prevPointHelper->GetWallCount()==1)
		{
			fBuildingModeler->SetWallHelper(prevPointHelper->GetWall((int32)0),false);
		}
	}
	else
	{
		if(prevPointHelper)
		{
			prevPointHelper->ClearFlag(ePointHelper);
			if(prevPointHelper->GetWallCount()==1)
			{
				prevPointHelper->GetWall((int32)0)->ClearFlag(eWallHelper);
			}
		}

		fBuildingModeler->SetPointHelper(NULL,false);
		fBuildingModeler->SetWallHelper(NULL,false);
	}

	return ModelerAction::Undo();
}

MCCOMErr BuildWallAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	VPoint* prevPointHelper = fBuildingPrimitive->GetPointHelper(fInLevel);
	
	int32 currentTool=0;
	gMenuUtilities->GetCurrentGlobalTool(currentTool); 
	
	if( IsWallTool( currentTool ) && prevPointHelper)
	{
		fBuildingModeler->SetPointHelper(prevPointHelper,false);
		if(prevPointHelper->GetWallCount()==1)
		{
			fBuildingModeler->SetWallHelper(prevPointHelper->GetWall((int32)0),false);
		}
	}
	else
	{
		if(prevPointHelper)
		{
			prevPointHelper->ClearFlag(ePointHelper);
			if(prevPointHelper->GetWallCount()==1)
			{
				prevPointHelper->GetWall((int32)0)->ClearFlag(eWallHelper);
			}
		}

		fBuildingModeler->SetPointHelper(NULL,false);
		fBuildingModeler->SetWallHelper(NULL,false);
	}

	return ModelerAction::Redo();
}

MCCOMErr BuildWallAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 7);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

CreateRoomAction::CreateRoomAction( BuildingModeler*	modeler )
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fCouldBuild = true;
	fRefreshGeometry = true;
}

void CreateRoomAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new CreateRoomAction(modeler) );
}

MCCOMErr CreateRoomAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fCouldBuild = fBuildingPrimitive->BuildRoomFromSelection(kAllLevels);
		fBuildingPrimitive->BuildExtendedSelection();
		fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("CreateRoomAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("CreateRoomAction::Do"));
	}

	return ModelerAction::Do();
}

MCCOMErr CreateRoomAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr CreateRoomAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr CreateRoomAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 10);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

CreateRoofAction::CreateRoofAction( BuildingModeler*	modeler, const int32 actionNumber )
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fActionNumber = actionNumber;
	fCouldBuild = true;
	fRefreshGeometry = true;
}

void CreateRoofAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler, 
								const int32 actionNumber)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new CreateRoofAction(modeler,actionNumber) );
}

MCCOMErr CreateRoofAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		ERoofType roofType = eLevelShapeRoof;
		switch(fActionNumber)
		{
		case kCreateRoof1MenuAction: roofType=eLevelShapeRoof; break;
		case kCreateRoof2MenuAction: roofType=eLevelShapeClosedSpineRoof; break;
		case kCreateRoof3MenuAction: roofType=eRectangleRoof; break;
		case kCreateRoof4MenuAction: roofType=eRectangleRoofFlatExtremsX; break;
		case kCreateRoof5MenuAction: roofType=eRectangleRoofFlatExtremsY; break;
		case kCreateRoof6MenuAction: roofType=eHalfRectangleRoof1; break;
		case kCreateRoof7MenuAction: roofType=eHalfRectangleRoof2; break;
		case kCreateRoof8MenuAction: roofType=eHalfRectangleRoof3; break;
		case kCreateRoof9MenuAction: roofType=eHalfRectangleRoof4; break;
		case kCreateRoof10MenuAction: roofType=eBorderRoof; break;
		}

		Roof* roof = fBuildingPrimitive->BuildRoof(roofType);
		if(roof)
		{
			// Select only the new roof to display its properties
			fBuildingPrimitive->SetSelection(false);
			roof->SetSelection(true);
			fBuildingPrimitive->BuildExtendedSelection();
			fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
		//	fBuildingPrimitive->GetLevel(fBuildingPrimitive->GetLevelCount()-1)->InvalidateTessellation();
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("CreateRoofAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("CreateRoofAction::Do"));
	}

	return ModelerAction::Do();
}

MCCOMErr CreateRoofAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr CreateRoofAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr CreateRoofAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 18);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

RoofProfileAction::RoofProfileAction(	BuildingModeler*	modeler, 
										const ERoofProfileID profile,
										const boolean		onTop,
										const boolean		inside)
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fProfile = profile;
	fRefreshGeometry = true;
	fOnTop = onTop;
	fInside = inside;
}

void RoofProfileAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler, 
								const ERoofProfileID profile,
								const boolean		onTop,
								const boolean		inside)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new RoofProfileAction(modeler,profile,onTop,inside) );
}

MCCOMErr RoofProfileAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fBuildingPrimitive->SetRoofProfile(fProfile,fOnTop,fInside, kAllLevels);
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr RoofProfileAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr RoofProfileAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr RoofProfileAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 18);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

SplitAction::SplitAction(	BuildingModeler*	modeler )
							:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
}

void SplitAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SplitAction(modeler) );
}

MCCOMErr SplitAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fBuildingPrimitive->Split(kAllLevels);

		fBuildingPrimitive->BuildExtendedSelection();
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr SplitAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr SplitAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr SplitAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 37);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

MergeAction::MergeAction(	BuildingModeler*	modeler, const boolean inOne )
							:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fMergeInOne = inOne;
}

void MergeAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler,
							const boolean inOne )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new MergeAction(modeler, inOne) );
}

MCCOMErr MergeAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		if(fMergeInOne)
			fBuildingPrimitive->Merge(kAllLevels);
		else
			fBuildingPrimitive->CheckSelectionConsistency( kAllLevels );

		fBuildingPrimitive->BuildExtendedSelection();
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr MergeAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr MergeAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr MergeAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 41);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

DeleteAction::DeleteAction(	BuildingModeler*	modeler )
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fRefreshGeometry = true;
	fDeleteSelection = true;
}

void DeleteAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new DeleteAction(modeler) );
}

DeleteAction::DeleteAction(	BuildingModeler*	modeler,  
							const TMCPoint&		mousePos, 
							BuildingPanePart*	pane )
									:
									ModelerAction(modeler),
									BuildingRecorder(),
									fPicked(modeler,pane,mousePos,pane->GetPickingFilter())
{
	fRefreshGeometry = true;
	fDeleteSelection = false;
}

void DeleteAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler,  
								const TMCPoint& mousePos, 
								BuildingPanePart* pane)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new DeleteAction(modeler,mousePos,pane) );
}

MCCOMErr DeleteAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		if(fDeleteSelection)
		{
			fBuildingPrimitive->DeleteSelection(kAllLevels);
		}
		else
		{
			switch(fPicked.GetPickedType())
			{
			case eBuildingRoof:
				{
					Roof* roof = static_cast<Roof*>(fPicked.PickedObject());
					roof->DeleteRoof();
				} break;
			case eRoofPointPicked:
				{
					RoofPoint* point = static_cast<RoofPoint*>(fPicked.PickedObject());
					point->DeleteRoofPoint();
				} break;
			case eProfilePointPicked:
				{
					ProfilePoint* point = static_cast<ProfilePoint*>(fPicked.PickedObject());
					point->DeleteProfilePoint();
				} break;
			case eWallHolePointPicked:
			case eRoomHolePointPicked:
				{
					/*OutlinePoint* point =*/ static_cast<OutlinePoint*>(fPicked.PickedObject());
// TO DO					point->DeleteOutlinePoint();
				} break;
			case eWallPicked:
			case eEdgePicked:
				{
					Wall* wall = static_cast<Wall*>(fPicked.PickedObject());
					wall->InvalidateTessellation(true);
					wall->DeleteWall();
				} break;
			case ePointPicked:
				{
					VPoint* pickedPoint = static_cast<VPoint*>(fPicked.PickedObject());
					pickedPoint->DeletePoint();
				} break;
			case eRoomFloorPicked:
			case eRoomCeilingPicked:
				{
					Room* room = static_cast<Room*>(fPicked.PickedObject());
					room->InvalidateTessellation(true);
					room->DeleteRoom();
				} break;
			case eRoomObjectPicked:
				{
					TMCCountedPtr<RoomSubObject> obj;
					obj = static_cast<RoomSubObject*>(fPicked.PickedObject());
					if( obj )
						obj->SetInRoom(NULL);
				} break;
			case eWallObjectPicked:
				{
					TMCCountedPtr<WallSubObject> obj;
					obj = static_cast<WallSubObject*>(fPicked.PickedObject());
					if( obj )
						obj->SetOnWall(NULL);
				} break;
			}

			// Invalidation: if a roof is deleted, the walls under will also be
			fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

			// Then rebuild the ExtendedSelection
			fBuildingPrimitive->BuildExtendedSelection();
		}
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr DeleteAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr DeleteAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr DeleteAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 8);

	return MC_S_OK;
}


///////////////////////////////////////////////////////////////////////////
//
//

DeleteLevelAction::DeleteLevelAction(BuildingModeler*	modeler,
									 const int32 level)
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fRefreshGeometry = true;
	fLevel = level;
}

void DeleteLevelAction::Create(	IShAction**		outAction,
							BuildingModeler*	modeler,
							const int32			level)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new DeleteLevelAction(modeler, level) );
}

MCCOMErr DeleteLevelAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fBuildingPrimitive->GetLevel(fLevel)->DeleteLevel();
		fBuildingPrimitive->SetAllLevelDistanceToGround();
		const int32 newLevelCount = fBuildingPrimitive->GetLevelCount();
	
		if(fLevel>0)
			fBuildingPrimitive->InvalidateAll(fLevel-1);
		if(fLevel<newLevelCount)
			fBuildingPrimitive->InvalidateAll(fLevel);
	
		// Choose a new active level if necessary
		if(fBuildingModeler->Get2DActiveLevel()>=newLevelCount)
			fBuildingPrimitive->ActiveLevel(newLevelCount-1);
	
		fBuildingPrimitive->BuildExtendedSelection();
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr DeleteLevelAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr DeleteLevelAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr DeleteLevelAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 8);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

ReplaceWallAction::ReplaceWallAction(	BuildingModeler*	modeler, const int32 actionNumber )
							:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fActionNumber = actionNumber;
}

void ReplaceWallAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler,
							const int32 actionNumber )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new ReplaceWallAction(modeler, actionNumber) );
}

MCCOMErr ReplaceWallAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		switch( fActionNumber )
		{
		case kReplaceBySimpleWall:
			{
				ReplaceBySimpleWallVisitor visitor;
				visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
			} break;
		case kReplaceByWallWithCrenel:
			{
				ReplaceByWallWithCrenelVisitor visitor;
				visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
			} break;
		}
	
		// Update the selection status
		fBuildingPrimitive->BuildExtendedSelection();

	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr ReplaceWallAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr ReplaceWallAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

MCCOMErr ReplaceWallAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 64);

	return MC_S_OK;
}

#endif // !NETWORK_RENDERING_VERSION

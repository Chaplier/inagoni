/****************************************************************************************************

		MInsertionAction.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/18/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MInsertionAction.h"

#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"

#include "MPositionActions.h"
#include "MBuildingAction.h"

InsertObjectAction::InsertObjectAction(	BuildingModeler*	modeler,
												BuildingPanePart*	pane,
												const Picking&		picked,
												const TMCPoint&		mousePos,
												const boolean		shortClick,
												const int32			actionID)
									:
									fPicked(picked),ModelerAction(modeler),fMousePos(mousePos),
									BuildingRecorder()
{
	fPanePart = pane;
	fCouldInsert=true;
	fRefreshGeometry = true;
	fShortClick = shortClick;
	

	switch(actionID)
	{
	case kInsertWindowMenuAction:
	case kInsertWindowTool:			fObjectKind = eWindow; break;
	case kInsertNarrowWindowTool:	fObjectKind = eNarrowWindow; break;
	case kInsertPanoWindowTool:		fObjectKind = ePanoramicWindow; break;
	case kInsertArrowWindowTool:	fObjectKind = eArrowWindow; break;
	case kInsert2CircleWindowTool:	fObjectKind = e2Circle16Window; break;
	case kInsertCircleWindowTool:	fObjectKind = eCircle16Window; break;
	case kInsert4CircleLWindowTool:	fObjectKind = e4Circle16LWindow; break;
	case kInsert4CircleRWindowTool:	fObjectKind = e4Circle16RWindow; break;
	case kInsertDoorMenuAction:
	case kInsertDoorTool:			fObjectKind = eDoor; break;
	case kInsertDoubleDoorTool:		fObjectKind = eDoubleDoor; break;
	case kInsertArrowDoorTool:		fObjectKind = eArrowDoor; break;
	case kInsert2CircleDoorTool:	fObjectKind = e2Circle16Door; break;
	case kInsert4CircleLDoorTool:	fObjectKind = e4Circle16LDoor; break;
	case kInsert4CircleRDoorTool:	fObjectKind = e4Circle16RDoor; break;
	case kInsertSquareStairwayTool: fObjectKind = eSquareStairway; break;
	case kInsertLargeStairwayTool:	fObjectKind = eLargeStairway; break;
	case kInsertWideStairwayTool:	fObjectKind = eWideStairway; break;
	case kInsertCircleStairwayTool: fObjectKind = eCircle16Stairway; break;
	}
}

void InsertObjectAction::Create(	IShAction**			outAction,
										BuildingModeler*	modeler,
										BuildingPanePart*	pane,
										const Picking&		picked,
										const TMCPoint&		mousePos,
										const boolean		shortClick,
										const int32			objectKind)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new InsertObjectAction(modeler, pane, picked, mousePos,shortClick,objectKind) );
}

MCCOMErr InsertObjectAction::Do()
{
	SaveBuilding(fBuildingPrimitive);

	fPicked.GetHitPosition(fHitPos);

	SubObject* newObj=NULL;

	int32 levelIndex = kAllLevels;

	switch(fPicked.GetPickedType())
	{
	case eWallPicked:
		{
			Wall* onWall = static_cast<Wall*>(fPicked.PickedObject());
			// First insertion of the object, place it at a reasonable height
			TVector3 posProposition = onWall->GetSnappedPosOnWall(fHitPos, fObjectKind);
//			posProposition.x = fHitPos.x;
//			posProposition.y = fHitPos.y;
			newObj = fBuildingPrimitive->MakeWallSubObject(onWall,fObjectKind);
			int32 tryCount=0;
			while( !newObj->SetCenter(posProposition,true) ) // true: check if there's enough space
			{ // Couldn't find enough space for the object: no more checking
				tryCount++;
				if(tryCount>10)
				{
					newObj->SetCenter(posProposition,false);
				}
				// scale it
				const TVector2 scale(.9f,.9f);
				newObj->ScalePolyline(scale,false);
			}
			levelIndex = onWall->GetLevel()->GetLevelIndex();
		} break;
	case eRoomFloorPicked:
	case eRoomCeilingPicked:
		{
			Room* inRoom = static_cast<Room*>(fPicked.PickedObject());
			newObj = fBuildingPrimitive->MakeRoomSubObject(inRoom,fObjectKind);
			int32 tryCount=0;
			while( !newObj->SetCenter(fHitPos,true) ) // true: check if there's enough space
			{ // Couldn't find enough space for the object: no more checking
				tryCount++;
				if(tryCount>10)
				{
					newObj->SetCenter(fHitPos,false);
				}
				// scale it
				const TVector2 scale(.9f,.9f);
				newObj->ScalePolyline(scale,false);
			}
			levelIndex = inRoom->GetLevel()->GetLevelIndex();
		} break;
	default:
		return ModelerAction::Do();
	}

	fCouldInsert = true;

	// Select only the new object to display its properties
	fBuildingPrimitive->SetSelection(false);
	newObj->SetSelection(true);

	fBuildingPrimitive->BuildExtendedSelection();
	fBuildingPrimitive->InvalidateExtendedSelection(levelIndex);
//	fBuildingPrimitive->InvalidateObjectSelection(levelIndex);

	// Set the correct flag if we're in Hole Edition mode
	if( fBuildingPrimitive->Data().GetHoleEditEnable() )
	{
		newObj->SetFlag(eIsHidden);
		newObj->SetSelection(false);
	}

	if( !fShortClick )
	{	// Start moving it
		fPicked.SetPickedObject(newObj);
		fPicked.SetPickedType(eWallObjectPicked);

		TMCCountedPtr<IShMouseAction> mouseAction;
		MoveObjectMouseAction::Create( &mouseAction, fBuildingModeler, fPanePart, fPicked, fMousePos );
		if(mouseAction)
		{
			TMCCountedPtr<IMFPart> part;
			fPanePart->Get3DEditorHostPanePart()->QueryInterface(IID_IMFPart, (void**) &part);

			gActionManager->PostMouseAction(mouseAction, kDefaultActionNumber, fBuildingModeler->GetContext(), part, fMousePos);
		}
	}

	return ModelerAction::Do();
}

MCCOMErr InsertObjectAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);
				
	return ModelerAction::Undo();
}

MCCOMErr InsertObjectAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);

	return ModelerAction::Redo();
}

MCCOMErr InsertObjectAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 6);

	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Level Insertion Action
//
InsertLevelAction::InsertLevelAction(	BuildingModeler*	modeler,
										const Picking&		picked, 
										const int32			type)
									:
									fPicked(picked),ModelerAction(modeler),
									BuildingRecorder()
{
	fRefreshGeometry = true;

	fInsertType = type;
}

void InsertLevelAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const Picking&		picked,
								const int32			type)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new InsertLevelAction(modeler, picked, type) );
}

MCCOMErr InsertLevelAction::Do()
{
	SaveBuilding(fBuildingPrimitive);

	// On or under which level do we add the new level?
	Level* pickedLevel = static_cast<Level*>(fPicked.PickedObject());
	const int32 levelIndex = pickedLevel->GetLevelIndex();
	const boolean over = fPicked.GetPickedType()==eLevelUpPicked;
	fBuildingPrimitive->InsertNewLevel(levelIndex, over, fInsertType);

	return ModelerAction::Do();
}

MCCOMErr InsertLevelAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);
				
	return ModelerAction::Undo();
}

MCCOMErr InsertLevelAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);

	return ModelerAction::Redo();
}

MCCOMErr InsertLevelAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 9);

	return MC_S_OK;
}

#endif // !NETWORK_RENDERING_VERSION


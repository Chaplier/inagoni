/****************************************************************************************************

		MSelectionActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/4/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MSelectionActions.h"

#include "MCPoint.h"
#include "BuildingModeler.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#endif

SelectionRecorder::SelectionRecorder()
{
}

inline void RecordSelection(CommonBase* obj, LevelSelection& selection, int32& index)
{
	if(obj->Selected())	selection.SetSelected(index);
	else				selection.SetNotSelected(index);
	if(obj->Hidden())	selection.SetHidden(index++);
	else				selection.SetNotHidden(index++);
}

void SelectionRecorder::SaveSelection(BuildingPrim* primitive, TMCClassArray<LevelSelection>& selection)
{
	gMenuUtilities->InvalidateMenus();
	
	const int32 levelCount = primitive->GetLevelCount();

	selection.SetElemCount(levelCount);

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

/* Maybe later, be sure that everything works
		if (level->Hidden())
		{
			selection[iLevel].fAllHidden = true;
			continue;
		}
		if(level->Selected())
		{
			selection[iLevel].fAllSelected = true;
			continue;
		}
*/
		const int32 pointCount = level->GetPointCount();
		const int32 wallCount = level->GetWallCount();
		const int32 roomCount = level->GetRoomCount();
		const int32 roofCount = level->GetRoofCount();
		selection[iLevel].fSelectionFlag.SetElemCount(pointCount+wallCount+roomCount+roofCount);

		int32 index = 0;
		for( int32 iPt=0 ; iPt<pointCount ; iPt++ )
		{
			RecordSelection(level->GetPoint(iPt), selection[iLevel], index);
		}
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);
			RecordSelection(wall, selection[iLevel], index);

			// Record also the selection on the objects
			const int32 objCount = wall->GetObjectCount();
			selection[iLevel].fSelectionFlag.AddElemCount(objCount);
			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
			{
				SubObject* obj = wall->GetObject(iObj);
				RecordSelection(obj, selection[iLevel], index);
				const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
				const int32 ptCount = outline.GetElemCount();
				selection[iLevel].fSelectionFlag.AddElemCount(ptCount);
				for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					RecordSelection(dynamic_cast<CommonBase*>(outline[iPt]), selection[iLevel], index);
			}
		}

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);
			RecordSelection(room, selection[iLevel], index);

			// Record also the selection on the objects
			const int32 objCount = room->GetObjectCount();
			selection[iLevel].fSelectionFlag.AddElemCount(objCount);
			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
			{
				SubObject* obj = room->GetObject(iObj);
				RecordSelection(obj, selection[iLevel], index);
				const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
				const int32 ptCount = outline.GetElemCount();
				selection[iLevel].fSelectionFlag.AddElemCount(ptCount);
				for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					RecordSelection(dynamic_cast<CommonBase*>(outline[iPt]), selection[iLevel], index);
			}
		}

		// Save the selection on the roof and the roof points
		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);
			RecordSelection(roof, selection[iLevel], index);

			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
			selection[iLevel].fSelectionFlag.AddElemCount(2*zoneSectionCount);
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zonePoint = roof->GetRoofZoneSection(iPt);

				RecordSelection(zonePoint.fZonePoint, selection[iLevel], index);
				RecordSelection(zonePoint.fSpinePoint, selection[iLevel], index);
			}

			// Save the profile selection
			const int32 botCount = roof->GetBotProfilePointCount();
			const int32 topCount = roof->GetTopProfilePointCount();
			const int32 bInCount = roof->GetBotInsidePointCount();
			const int32 tInCount = roof->GetTopInsidePointCount();		
			selection[iLevel].fSelectionFlag.AddElemCount(botCount+topCount+bInCount+tInCount);
	
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
				RecordSelection(roof->GetBotProfilePoint(iBot), selection[iLevel], index);

			for(int32 iTop=0 ; iTop<topCount ; iTop++)
				RecordSelection(roof->GetTopProfilePoint(iTop), selection[iLevel], index);

			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
				RecordSelection(roof->GetBotInsidePoint(iBIn), selection[iLevel], index);

			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
				RecordSelection(roof->GetTopInsidePoint(iTIn), selection[iLevel], index);

			// Record also the selection on the objects
//			const int32 objCount = roof->GetObjectCount();
//			selection[iLevel].fSelectionFlag.AddElemCount(objCount);
//			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
//			{
//				RoofSubObject* obj = roof->GetObject(iObj);
//				if(obj->Selected()) selection[iLevel].SetSelected(index);
//				else			selection[iLevel].SetNotSelected(index);
//				if(obj->Hidden()) selection[iLevel].SetHidden(index++);
//				else			selection[iLevel].SetNotHidden(index++);
//			}
		}
	}
}

inline void RestoreSelection(CommonBase* obj, LevelSelection& selection, int32& index)
{
	obj->SetSelection(selection.Selected(index));
/*	Can't do only that: need to take care of the ExtendedSelection
	if(selection.Selected(index))	obj->SetFlag(eIsSelected);
	else							obj->ClearFlag(eIsSelected);*/
	if(selection.Hidden(index++))	obj->SetFlag(eIsHidden);
	else							obj->ClearFlag(eIsHidden);

}

void SelectionRecorder::SwapSelection( BuildingPrim* primitive )
{
	gMenuUtilities->InvalidateMenus();
	
	TMCClassArray<LevelSelection> currentSelection;
	SaveSelection(primitive, currentSelection);

	const int32 levelCount = fRecordedSelection.GetElemCount();
	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );
/* Maybe later, be sure that everything works
		if( fRecordedSelection[iLevel].fAllSelected )
		{
			level->ShowHide(eShowAll);
			level->SetSelection(true);
			continue;
		}
		else
		{
			level->ClearFlag(eIsSelected);
		}
		if( fRecordedSelection[iLevel].fAllHidden )
		{
			level->ShowHide(eHideAll);
			continue;
		}
		else
		{
			level->ClearFlag(eIsHidden);
		}
*/
		const int32 pointCount = level->GetPointCount();
		const int32 wallCount = level->GetWallCount();
		const int32 roomCount = level->GetRoomCount();
		const int32 roofCount = level->GetRoofCount();

		int32 index = 0;
		for( int32 iPt=0 ; iPt<pointCount ; iPt++ )
		{
			RestoreSelection(level->GetPoint(iPt),fRecordedSelection[iLevel],index);
		}
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);
			RestoreSelection(wall,fRecordedSelection[iLevel],index);

			// Restore also the selection on the objects
			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
			{
				SubObject* obj = wall->GetObject(iObj);
				RestoreSelection(obj, fRecordedSelection[iLevel], index);
				const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
				const int32 ptCount = outline.GetElemCount();
				for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					RestoreSelection(dynamic_cast<CommonBase*>(outline[iPt]), fRecordedSelection[iLevel], index);
			}
		}

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);
			RestoreSelection(room,fRecordedSelection[iLevel],index);

			// Restore also the selection on the objects
			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
			{
				SubObject* obj = room->GetObject(iObj);
				RestoreSelection(obj, fRecordedSelection[iLevel], index);
				const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
				const int32 ptCount = outline.GetElemCount();
				for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					RestoreSelection(dynamic_cast<CommonBase*>(outline[iPt]), fRecordedSelection[iLevel], index);
			}
		}

		// Swap the selection on the roof and the roof points
		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);
			RestoreSelection(roof,fRecordedSelection[iLevel],index);

			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zonePoint = roof->GetRoofZoneSection(iPt);

				RestoreSelection(zonePoint.fZonePoint,fRecordedSelection[iLevel],index);
				RestoreSelection(zonePoint.fSpinePoint,fRecordedSelection[iLevel],index);
			}

			// Restore the profile selection
			const int32 botCount = roof->GetBotProfilePointCount();
			const int32 topCount = roof->GetTopProfilePointCount();
			const int32 bInCount = roof->GetBotInsidePointCount();
			const int32 tInCount = roof->GetTopInsidePointCount();		
	
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
				RestoreSelection(roof->GetBotProfilePoint(iBot), fRecordedSelection[iLevel], index);

			for(int32 iTop=0 ; iTop<topCount ; iTop++)
				RestoreSelection(roof->GetTopProfilePoint(iTop), fRecordedSelection[iLevel], index);

			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
				RestoreSelection(roof->GetBotInsidePoint(iBIn), fRecordedSelection[iLevel], index);

			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
				RestoreSelection(roof->GetTopInsidePoint(iTIn), fRecordedSelection[iLevel], index);

			// Restore also the selection on the objects
//			const int32 objCount = roof->GetObjectCount();
//			for( int32 iObj=0 ; iObj<objCount ; iObj++ )
//			{
//				RoofSubObject* obj = roof->GetObject(iObj);
//				if(fRecordedSelection[iLevel].Selected(index))obj->SetFlag(eIsSelected);
//				else				obj->ClearFlag(eIsSelected);
//				if(fRecordedSelection[iLevel].Hidden(index++))obj->SetFlag(eIsHidden);
//				else				obj->ClearFlag(eIsHidden);
//			}
		}
	}

	primitive->BuildExtendedSelection();

	fRecordedSelection = currentSelection;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectionAction::SelectionAction(	BuildingModeler*	modeler,
									const Picking&		picked,
									const TMCPlatformEvent& event,
									const TMCPoint&		mousePos)
									:SelectionRecorder(),
									fPicked(picked),ModelerAction(modeler)
{
	fShift = event.fModifiers.ShiftKeyDown();
	fDoubleClick =	event.fClickCount == 2;
	fSelectionHasChanged = true;
}

void SelectionAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const Picking&		picked,
								const TMCPlatformEvent& event,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectionAction(modeler, picked,event, mousePos) );
}

MCCOMErr SelectionAction::Do()
{
	try
	{
		SaveSelection(fBuildingPrimitive);

		CommonBase* obj = fPicked.PickedObject();
		if(obj)
		{

			const boolean wasSelected = obj->Selected();
			fSelectionHasChanged = !wasSelected||fShift;
			
			if(fDoubleClick)
			{
				switch(fPicked.GetPickedType())
				{
				case eRoomFloorPicked:
				case eRoomCeilingPicked:
					{	// If the object is a Room, select all it countains
						Room* room = static_cast<Room*>(obj);
						room->SetCompleteSelection();
						fSelectionHasChanged = true;
					} break;
				case eWallPicked:
					{	// TO DO: If the object is a Wall, select all the touching walls
						Wall* wall = static_cast<Wall*>(obj);
						wall->SetCompleteSelection();
						fSelectionHasChanged = true;
					} break;
				case eBuildingRoof:
					{
						Roof* roof = static_cast<Roof*>(obj);
						roof->GetLevel()->SetSelection(true);
						fSelectionHasChanged = true;
					} break;
				}
			}
			if(fShift)
			{	// Extend the selection
				obj->SetSelection(!wasSelected);
				fBuildingPrimitive->InvalidateStatus();
			}
			else if (!wasSelected)
			{	// Deselect all before selecting
				fBuildingPrimitive->SetSelection(false);
				obj->SetSelection(true);
			}
		}
		else
		{
			if( !fShift )
			{
				// Deselect all
				fBuildingPrimitive->SetSelection(false);
				fSelectionHasChanged = true;
			}
		}

		if(fSelectionHasChanged)
		{
			fBuildingPrimitive->BuildExtendedSelection();
			fBuildingPrimitive->InvalidateStatus();
			gMenuUtilities->InvalidateMenus();
		}

	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr SelectionAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectionAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectionAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 2);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
MarqueeMouseAction::MarqueeMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const TMCPoint&		mousePos)
									:
									ModelerMouseAction(modeler,pane),
									SelectionRecorder()
{
	fFirstPoint=mousePos;
	fInLevel = pane->GetInLevel();
}

void MarqueeMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MarqueeMouseAction(modeler, pane, mousePos);
}

void MarqueeMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SaveSelection(fBuildingPrimitive);
			fBuildingPrimitive->InitMarqueeSelection(fInLevel);
			fBuildingModeler->BeginImmediateUpdate();	
			CreateFramePart();
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TMCArray<Plane> rayPlanes;
				TVector3 direction;
				TVector3 origin;
				TMCPoint midPoint((cur.x+fFirstPoint.x)/2, (cur.y+fFirstPoint.y)/2);
				fPanePart->Get3DEditorHostPanePart()->PixelRay(midPoint, origin, direction);
				const TMCPoint minPt(MC_Min(first.x,cur.x), MC_Min(first.y,cur.y) );
				const TMCPoint maxPt(MC_Max(first.x,cur.x), MC_Max(first.y,cur.y) );
				MakeRayPlanes(rayPlanes, fPanePart->Get3DEditorHostPanePart(), minPt, maxPt, origin, direction);
				const int32 mode = 0; //later
				fBuildingPrimitive->SetMarquee(rayPlanes, mode, fInLevel);
				{
					fPanePart->Get3DEditorHostPanePart()->DoFinalBlit(false);
					fBuildingModeler->InvalidateMeshesAttributes(true);
					fBuildingModeler->PostImmediateUpdate(false,true);
					fPanePart->Get3DEditorHostPanePart()->DoFinalBlit(true);
				}

				// Track the rectangle
				fBuildingModeler->EndImmediateUpdate();
				TMCRect rect(minPt,maxPt);
				fFramePart->SetBoundsInParent(rect);
				fPanePart->GetThisPartNoAddRef()->ProcessUpdatesPart();
				fBuildingModeler->BeginImmediateUpdate();
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	

			if(fFramePart)
			{
				fFramePart->RemoveFromParent();
				fFramePart = NULL;
			}
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr MarqueeMouseAction::Do()
{
	return ModelerMouseAction::Do();
}

MCCOMErr MarqueeMouseAction::Undo()
{
	SwapSelection(fBuildingPrimitive);
				
	return ModelerMouseAction::Undo();
}

MCCOMErr MarqueeMouseAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerMouseAction::Redo();
}

MCCOMErr MarqueeMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 2);

	return MC_S_OK;
}

void MarqueeMouseAction::CreateFramePart()
{
	TMCString31 partName("TMFFramePart");
	gPartUtilities->CreatePartByName(&fFramePart,partName,1,0);
	ThrowIfNil(fFramePart);

	fPanePart->GetThisPartNoAddRef()->AddChildIMFPart(fFramePart);
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ShowActiveLevel::ShowActiveLevel( BuildingModeler*	modeler,
							    const int32			showLevel)
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fRefreshGeometry = true;
	fShowLevel = showLevel;
}

void ShowActiveLevel::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
							    const int32			showLevel)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new ShowActiveLevel(modeler, showLevel) );
}

MCCOMErr ShowActiveLevel::Do()
{
	SaveSelection(fBuildingPrimitive);

	ShowLevel();

	return ModelerAction::Do();
}

MCCOMErr ShowActiveLevel::Undo()
{
	SwapSelection(fBuildingPrimitive);

	ShowLevel();

	return ModelerAction::Do();
}

MCCOMErr ShowActiveLevel::Redo()
{
	SwapSelection(fBuildingPrimitive);

	ShowLevel();

	return ModelerAction::Do();
}

void ShowActiveLevel::ShowLevel()
{
	const boolean prevShownAll = fBuildingPrimitive->ShowAll();
	const int32 prevLevelShown = fBuildingPrimitive->ActiveLevel();
	
	if(fShowLevel==kAllLevels)
		fBuildingPrimitive->ShowAll(true);
	else
	{
		fBuildingPrimitive->ShowAll(false);
		fBuildingPrimitive->ActiveLevel(fShowLevel);
	}
	if(prevShownAll)
		fShowLevel = kAllLevels;
	else
		fShowLevel = prevLevelShown;
}

MCCOMErr ShowActiveLevel::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 36);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
HoleEditionOnOff::HoleEditionOnOff( BuildingModeler*	modeler )
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fRefreshGeometry = true;
}

void HoleEditionOnOff::Create(	IShAction**			outAction,
								BuildingModeler*	modeler )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new HoleEditionOnOff(modeler) );
}

MCCOMErr HoleEditionOnOff::Do()
{
	SaveSelection(fBuildingPrimitive);

	SwitchMode();

	return ModelerAction::Do();
}

MCCOMErr HoleEditionOnOff::Undo()
{
	SwapSelection(fBuildingPrimitive);

	SwitchMode();

	return ModelerAction::Do();
}

MCCOMErr HoleEditionOnOff::Redo()
{
	SwapSelection(fBuildingPrimitive);

	SwitchMode();

	return ModelerAction::Do();
}

void HoleEditionOnOff::SwitchMode()
{
	// Need to switch the mode in the data
	// and to change the Hidden and Select flags on some objects

	const boolean isEditionOn = !fBuildingPrimitive->GetData().GetHoleEditEnable();
	fBuildingPrimitive->GetData().SetHoleEditEnable(isEditionOn);

	if(isEditionOn)
	{	// Hide the object handle, show the points		
		fBuildingPrimitive->ShowHide(eShowAllHolePoints);
	}
	else
	{	// Hide the object points, show the handle
		fBuildingPrimitive->ShowHide(eHideAllHolePoints);
	}
}

MCCOMErr HoleEditionOnOff::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 54);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SmoothSelectionAction::SmoothSelectionAction( BuildingModeler*	modeler, const boolean smooth )
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fRefreshGeometry = true;
	fSmooth = smooth;
}

void SmoothSelectionAction::Create(	IShAction**			outAction,
									BuildingModeler*	modeler, 
									const boolean		smooth )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SmoothSelectionAction(modeler, smooth) );
}

MCCOMErr SmoothSelectionAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->Smooth(fSmooth, kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr SmoothSelectionAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SmoothSelectionAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SmoothSelectionAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 54);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ShowHideAction::ShowHideAction( BuildingModeler*	modeler,
							    const EShowHideOption option)
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fRefreshGeometry = true;
	fOption = option;
}

void ShowHideAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
							    const EShowHideOption option)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new ShowHideAction(modeler, option) );
}

MCCOMErr ShowHideAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->ShowHide(fOption);

	return ModelerAction::Do();
}

MCCOMErr ShowHideAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr ShowHideAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr ShowHideAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 16);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
InvertSelectionAction::InvertSelectionAction( BuildingModeler*	modeler)
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
}

void InvertSelectionAction::Create(	IShAction**			outAction,
									BuildingModeler*	modeler )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new InvertSelectionAction(modeler) );
}

MCCOMErr InvertSelectionAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->InvertSelection(kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr InvertSelectionAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr InvertSelectionAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr InvertSelectionAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 21);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectAllAction::SelectAllAction( BuildingModeler*	modeler)
								:
						SelectionRecorder(),
						ModelerAction(modeler)
{
}

void SelectAllAction::Create(	IShAction**			outAction,
									BuildingModeler*	modeler )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectAllAction(modeler) );
}

MCCOMErr SelectAllAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->SetSelection(true);
	
	fBuildingPrimitive->BuildExtendedSelection();

	return ModelerAction::Do();
}

MCCOMErr SelectAllAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectAllAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectByNameAction::SelectByNameAction( BuildingModeler*	modeler, 
										const TMCString&	nameStr, 
										const boolean		select)
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fSelect = select;
	fName = nameStr;
}

void SelectByNameAction::Create(	IShAction**			outAction,
									BuildingModeler*	modeler, 
									const TMCString&	nameStr, 
									const boolean		select )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectByNameAction(modeler, nameStr, select) );
}

MCCOMErr SelectByNameAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->SelectByName(fName, fSelect, kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr SelectByNameAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectByNameAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectByNameAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, fSelect?43:44);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectByDomainAction::SelectByDomainAction( BuildingModeler*	modeler, 
											const int32			domain, 
											const boolean		select)
								:
								SelectionRecorder(),
								ModelerAction(modeler)
{
	fSelect = select;
	fDomain = domain;
}

void SelectByDomainAction::Create(	IShAction**			outAction,
									BuildingModeler*	modeler, 
									const int32			domain, 
									const boolean		select )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectByDomainAction(modeler, domain, select) );
}

MCCOMErr SelectByDomainAction::Do()
{
	SaveSelection(fBuildingPrimitive);

	fBuildingPrimitive->SelectByDomain(fDomain, fSelect, kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr SelectByDomainAction::Undo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectByDomainAction::Redo()
{
	SwapSelection(fBuildingPrimitive);

	return ModelerAction::Do();
}

MCCOMErr SelectByDomainAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, fSelect?45:46);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
LevelName::LevelName(BuildingModeler*	modeler,
					 const int32		level,
					const TMCString&	name) :
						ModelerAction(modeler)
{
	fLevel = level;
	fName = name;
}

void LevelName::Create(IShAction**			outAction,
					   BuildingModeler*	modeler,
						const int32			level,
						const TMCString&	name)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new LevelName(modeler,level,name) );
}

MCCOMErr LevelName::Do()
{
	SetName();
				
	return ModelerAction::Do();
}

MCCOMErr LevelName::Undo()
{
	SetName();

	return ModelerAction::Undo();
}

MCCOMErr LevelName::Redo()
{
	SetName();

	return ModelerAction::Redo();
}

void LevelName::SetName()
{
	Level* level = fBuildingPrimitive->GetLevel( fBuildingPrimitive->ActiveLevel() );

	if(MCVerify(level))
	{
		TMCDynamicString name = level->GetName();
		level->SetName(fBuildingPrimitive->GetData().fDictionary , fName);
		fName = name;
	}

	// Invalidation
	fBuildingPrimitive->InvalidateStatus();
}

MCCOMErr LevelName::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 42);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectionFlagAction::SelectionFlagAction(BuildingModeler*	modeler,
								const boolean		flagValue,
								const EFlagType		flagType) :
									ModelerAction(modeler)
{
	fFlagValue = flagValue;
	fFlagType = flagType;
	fRefreshGeometry = true;
}

void SelectionFlagAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const boolean		flagValue,
								const EFlagType		flagType)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectionFlagAction(modeler,flagValue,flagType) );
}

MCCOMErr SelectionFlagAction::Do()
{
	SetFlag();
				
	return ModelerAction::Do();
}

MCCOMErr SelectionFlagAction::Undo()
{
	// Use the stored undo data to restore the thickness
	const int32 levelCount = fBuildingPrimitive->GetLevelCount();
	int32 index = 0;
	const int32 prevCount = fPreviousFlags.GetElemCount();
	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fBuildingPrimitive->GetLevel(iLevel);

		if (level->Hidden())
			continue;

		if( fFlagType==eNoCeilingFlag ||
			fFlagType==eAutoFlipObjFlag )
		{
			const int32 roomCount = level->GetRoomCount();

			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom(iRoom);

				if( room->Selected() && fFlagType==eNoCeilingFlag )
				{
					room->NoCeiling(fPreviousFlags[index++]);

					if(index>=prevCount)
						return ValidUndo();
				}

				if( fFlagType==eAutoFlipObjFlag )
				{
					const int32 objCount = room->GetObjectCount();
					for( int32 iObj=0 ; iObj< objCount ; iObj++ )
					{
						SubObject* obj = room->GetObject( iObj );

						if( obj->Selected() )
						{
							obj->AutoFlip(fPreviousFlags[index++]);

							if(index>=prevCount)
								return ValidUndo();
						}
					}
				}
			}
		}

		if( fFlagType==eWallExtraHeightFlag ||
			fFlagType==eAutoFlipObjFlag )
		{
			const int32 wallCount = level->GetWallCount();

			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = level->GetWall(iWall);

				if( wall->Selected() && fFlagType==eWallExtraHeightFlag)
				{
					wall->ExtraHeight(fPreviousFlags[index++]);

					if(index>=prevCount)
						return ValidUndo();
				}

				if( fFlagType==eAutoFlipObjFlag )
				{
					const int32 objCount = wall->GetObjectCount();
					for( int32 iObj=0 ; iObj< objCount ; iObj++ )
					{
						SubObject* obj = wall->GetObject( iObj );

						if( obj->Selected() )
						{
							obj->AutoFlip(fPreviousFlags[index++]);

							if(index>=prevCount)
								return ValidUndo();
						}
					}
				}
			}
		}
	}

	// Invalidation
	return ValidUndo();
}

MCCOMErr SelectionFlagAction::ValidUndo()
{
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Undo();
}

MCCOMErr SelectionFlagAction::Redo()
{
	SetFlag();

	return ModelerAction::Redo();
}

void SelectionFlagAction::SetFlag()
{
	switch(fFlagType)
	{
	case eNoCeilingFlag:
		fBuildingPrimitive->SetNoCeiling(fFlagValue, fPreviousFlags, kAllLevels);
		break;
	case eWallExtraHeightFlag:
		fBuildingPrimitive->SetWallExtraHeight(fFlagValue, fPreviousFlags, kAllLevels);
		break;
	case eAutoFlipObjFlag:
		fBuildingPrimitive->SetAutoFlipObjects(fFlagValue, fPreviousFlags, kAllLevels);
		break;
	}

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
}

MCCOMErr SelectionFlagAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 40);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
SelectionNameAction::SelectionNameAction(BuildingModeler*	modeler,
										const TMCString&	name) :
									ModelerAction(modeler)
{
	fName = name;
}

void SelectionNameAction::Create(IShAction**		outAction,
								BuildingModeler*	modeler,
								const TMCString&	name)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new SelectionNameAction(modeler,name) );
}

MCCOMErr SelectionNameAction::Do()
{
	SetName();
				
	return ModelerAction::Do();
}

void SetCommonObjectName(CommonBase* common,
						 const TMCString& name,
						 NameChainedList& dictionnary)
{
	if (common->Hidden())
		return;

	if( common->Selected() )
	{
		common->SetName(dictionnary, name);
	}
}

MCCOMErr SelectionNameAction::Undo()
{
	// Use the stored undo data to restore the names
	const int32 levelCount = fBuildingPrimitive->GetLevelCount();
	int32 index = 0;
	const int32 prevCount = fPreviousNames.GetElemCount();
	NameChainedList& dictionary = fBuildingPrimitive->GetData().fDictionary;

	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fBuildingPrimitive->GetLevel(iLevel);

		if (level->Hidden())
			continue;

		boolean nameSet=false;

		// Wall and wall objects
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( wall->Selected() )
			{
				nameSet = true;
				wall->SetName(dictionary, fPreviousNames[index++]);

				if(index>=prevCount)
				{
					fBuildingPrimitive->InvalidateStatus();
					return ModelerAction::Undo();
				}
			}

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				SubObject* obj = wall->GetObject( iObj );

				if( obj->Selected() )
				{
					nameSet = true;
					obj->SetName(dictionary, fPreviousNames[index++]);

					if(index>=prevCount)
					{
						fBuildingPrimitive->InvalidateStatus();
						return ModelerAction::Undo();
					}
				}
			}
		}

		// Room and room objects
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);
		
			if( room->Selected() )
			{
				nameSet = true;
				room->SetName(dictionary, fPreviousNames[index++]);

				if(index>=prevCount)
				{
					fBuildingPrimitive->InvalidateStatus();
					return ModelerAction::Undo();
				}
			}

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				SubObject* obj = room->GetObject( iObj );

				if( obj->Selected() )
				{
					nameSet = true;
					obj->SetName(dictionary, fPreviousNames[index++]);

					if(index>=prevCount)
					{
						fBuildingPrimitive->InvalidateStatus();
						return ModelerAction::Undo();
					}
				}
			}
		}

		// Roof and roof points (zone and profile)
		const int32 roofCount = level->GetRoofCount();
		{
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);
				if( roof->Selected() )
				{
					nameSet = true;
					roof->SetName(dictionary, fPreviousNames[index++]);

					if(index>=prevCount)
					{
						fBuildingPrimitive->InvalidateStatus();
						return ModelerAction::Undo();
					}
				}
			}
		}

		if(!nameSet)
		{
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);
				{
					const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
					for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
					{
						const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);
						if( zoneSection.fZonePoint->Selected() )
						{
							zoneSection.fZonePoint->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
						if( zoneSection.fSpinePoint->Selected() )
						{
							zoneSection.fSpinePoint->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
					}
				}

				{
					const int32 botProfileCount = roof->GetBotProfilePointCount();
					for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
					{
						CommonPoint* point = roof->GetBotProfilePoint(iPt);
						if( point->Selected() )
						{
							point->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
					}
				}

				{
					const int32 topProfileCount = roof->GetTopProfilePointCount();
					for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
					{
						CommonPoint* point = roof->GetTopProfilePoint(iPt);
						if( point->Selected() )
						{
							point->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
					}
				}

				{
					const int32 botInsideCount = roof->GetBotInsidePointCount();
					for(int32 iPt=0 ; iPt<botInsideCount ; iPt++)
					{
						CommonPoint* point = roof->GetBotInsidePoint(iPt);
						if( point->Selected() )
						{
							point->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
					}
				}

				{
					const int32 topInsideCount = roof->GetTopInsidePointCount();
					for(int32 iPt=0 ; iPt<topInsideCount ; iPt++)
					{
						CommonPoint* point = roof->GetTopInsidePoint(iPt);
						if( point->Selected() )
						{
							point->SetName(dictionary, fPreviousNames[index++]);

							if(index>=prevCount)
							{
								fBuildingPrimitive->InvalidateStatus();
								return ModelerAction::Undo();
							}
						}
					}
				}
			}

			// Points
			const int32 pointCount = level->GetPointCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point = level->GetPoint(iPoint);

				if( point->Selected() )
				{
					point->SetName(dictionary, fPreviousNames[index++]);

					if(index>=prevCount)
					{
						fBuildingPrimitive->InvalidateStatus();
						return ModelerAction::Undo();
					}
				}
			}
		}
	}

	// Invalidation
	fBuildingPrimitive->InvalidateStatus();
				
	return ModelerAction::Undo();
}

MCCOMErr SelectionNameAction::Redo()
{
	SetName();

	return ModelerAction::Redo();
}

void SelectionNameAction::SetName()
{
	fBuildingPrimitive->SetSelectionName(fName, fPreviousNames, kAllLevels);

	// Invalidation
	fBuildingPrimitive->InvalidateStatus();
}

MCCOMErr SelectionNameAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 42);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

#endif // !NETWORK_RENDERING_VERSION


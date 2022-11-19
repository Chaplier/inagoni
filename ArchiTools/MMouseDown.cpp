/****************************************************************************************************

		MMouseDown.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/2/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MMouseDown.h"

#include "BuildingModeler.h"
#include "MBuildingPanePart.h"
#include "MPicking.h"
#include "MEditActions.h"
#include "MSelectionActions.h"
#include "MPositionActions.h"
#include "MInsertionAction.h"
#include "MCreateActions.h"
#include "MChildrenActions.h"
#include "MDefaultSettingActions.h"

#include "I3DEditorHostPartDefs.h"
#include "COMUtilities.h"
#include "MiscComUtilsImpl.h"
#include "IShUtilities.h"
#include "MFPublicDefs.h"
#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#endif

boolean MouseDown::SelectTool(	BuildingModeler* modeler,  
								const TMCPoint& mousePos, 
								const TMCPlatformEvent& event, 
								BuildingPanePart* pane,
								const int32 currentTool )
{
	boolean result = true;
	try
	{
		const boolean shift =		event.fModifiers.ShiftKeyDown();
		const boolean option =		event.fModifiers.OptionKeyDown();
		const boolean command =		event.fModifiers.CommandKeyDown();
		const boolean doubleClick =	event.fClickCount == 2;
		const boolean shortClick =	gActionManager->IsShortStaticClick();

		const boolean extendedSelection = shift;


		BuildingPrim* building = modeler->GetBuildingNoAddRef();
		PrimitiveStatus* status = building->GetStatus();

		TMCCountedPtr<IShMouseAction> mouseAction;
		TMCCountedPtr<IShAction> action;

		Picking pick(modeler,pane,mousePos, pane->GetPickingFilter());

		CommonBase* obj = pick.PickedObject();
		if(obj)
		{
			SelectionAction::Create( &action, modeler, pick, event, mousePos );

			if( !shortClick )
			{
				switch(currentTool)
				{
				case kMoveToolID:
					{
						//	drag selection
						switch( pick.GetPickedType() )
						{
						case eWallObjectPicked:
						case eRoomObjectPicked:
							MoveObjectMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eWallHandlePointPicked:
							MoveWallHandleMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eProfilePointPicked:
							MoveProfileMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eWallHolePointPicked:
						case eRoomHolePointPicked:
							MoveHolePointMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						default:
							MoveMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						}
						
					}break;
				case kScaleToolID:	
					{
						// Scale the selection
						switch( pick.GetPickedType() )
						{
						case eWallObjectPicked:
						case eRoomObjectPicked:
							ScaleObjectMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eProfilePointPicked:
							ScaleProfileMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eWallHolePointPicked:
						case eRoomHolePointPicked:
							ScaleHolePointMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						default:
							ScaleMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						}
					} break;
				case kRotateToolID:	
					{
						// Rotate the selection
						switch( pick.GetPickedType() )
						{
						case eWallObjectPicked:
						case eRoomObjectPicked:
							RotateObjectMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eProfilePointPicked:
							RotateProfileMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						case eWallHolePointPicked:
						case eRoomHolePointPicked:
							RotateHolePointMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						default:
							RotateMouseAction::Create( &mouseAction, modeler, pane, pick, mousePos );
							break;
						}				
					} break;
				}
			}
		}
		else
		{	// Nothing clicked
			if( !shift )
			{
				// Deselect all
				if(status->fHasSelection)
					SelectionAction::Create( &action, modeler, pick, event, mousePos );
			}
			if( !shortClick )
			{
				// Start a marquee selection
				MarqueeMouseAction::Create( &mouseAction, modeler, pane, mousePos);
			}
		}

		TMCCountedPtr<IMFPart> part;
		pane->Get3DEditorHostPanePart()->QueryInterface(IID_IMFPart, (void**) &part);

		if(action)
			gActionManager->PostAction(action, kDefaultActionNumber, modeler->GetContext());
		if(mouseAction)
			gActionManager->PostMouseAction(mouseAction, kDefaultActionNumber, modeler->GetContext(), part, mousePos);

	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return result;
}

boolean MouseDown::BuildWallTool(	BuildingModeler* modeler,  
									const TMCPoint& mousePos, 
									BuildingPanePart* pane,
									const int32 currentTool )
{
	// Build a wall if a point was already selected or created using
	// this tool. Otherwise create the first point of a wall polyline.
	TMCCountedPtr<IShAction> action;

	EWallType wallType = eBasic;
	switch( currentTool )
	{
	case kBuildWallTool: wallType = eBasic; break;
	case kBuildWallWithCrenel1Tool: wallType = eWithCrenel1; break;
	case kBuildWallWithCrenel2Tool: wallType = eWithCrenel2; break;
	case kBuildWallWithCrenel3Tool: wallType = eWithCrenel3; break;
	case kBuildWallWithCrenel4Tool: wallType = eWithCrenel4; break;
	case kBuildWallWithCrenel5Tool: wallType = eWithCrenel5; break;
	}

	BuildWallAction::Create( &action, modeler, pane, mousePos, wallType );

	if(action)
	{
		gActionManager->PostAction(action, kDefaultActionNumber, modeler->GetContext());
		return true;
	}

	return true;
}

boolean MouseDown::InsertObjectTool(	BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										BuildingPanePart* pane,
										const int32 currentTool )
{
	TMCCountedPtr<IShAction> action;

	Picking pick(modeler,pane,mousePos, pane->GetPickingFilter());

	const boolean shortClick =	gActionManager->IsShortStaticClick();

	switch( pick.GetPickedType() )
	{
	case eWallPicked:
		{
			if( IsWindowToolID(currentTool) ||
				IsDoorToolID(currentTool) )
			{
				InsertObjectAction::Create( &action, modeler, pane, pick, mousePos, shortClick, currentTool );
			}
		} break;
	case eRoomFloorPicked:
	case eRoomCeilingPicked:
		{
			if(IsStairwayToolID(currentTool))
		{
				InsertObjectAction::Create( &action, modeler, pane, pick, mousePos, shortClick, currentTool );
			}
		} break;
	}

	if(action)
	{
		gActionManager->PostAction(action, kDefaultActionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::InsertObject(	BuildingModeler* modeler,  
									const int32 currentTool)
{
	TMCCountedPtr<IShAction> action;

	BuildingPrim* building = modeler->GetBuildingNoAddRef();

	Picking pick;
	switch(currentTool)
	{
	case kInsertWindowMenuAction:
	case kInsertDoorMenuAction:
		{
			Wall* pickedWall = building->GetFirstSelectedWall(kAllLevels);
			if(!pickedWall)
			{
				CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
				TMCDynamicString message;
				gResourceUtilities->GetIndString( message, kAlertStrings, 1);
				gPartUtilities->Alert(message);
				return false;
			}
			pick.SetPickedObject(pickedWall);
			pick.SetPickedType(eWallPicked);
			pick.SetHitPosition( pickedWall->GetSnappedPosOnWall(pickedWall->GetMiddle(), currentTool==kInsertWindowMenuAction?eWindow:eDoor) );
		} break;
	case kInsertStairwayMenuAction:
		{
			Room* pickedRoom = building->GetFirstSelectedRoom(kAllLevels);
			if(!pickedRoom)
			{
				CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
				TMCDynamicString message;
				gResourceUtilities->GetIndString( message, kAlertStrings, 2);
				gPartUtilities->Alert(message);
				return false;
			}
			pick.SetPickedObject(pickedRoom);
			pick.SetPickedType(eRoomFloorPicked);
		}break;
	}
	
	TMCPoint point(0,0);
	InsertObjectAction::Create( &action, modeler, NULL, pick, point, true, currentTool );


	if(action)
	{
		gActionManager->PostAction(action, currentTool, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MouseDown::InsertLevelTool(	BuildingModeler* modeler,  
									const TMCPoint& mousePos, 
									BuildingPanePart* pane,
									const int32 currentTool )
{
	TMCCountedPtr<IShAction> action;

	Picking pick(modeler,pane,mousePos, ePickLevel);

	const boolean shortClick =	gActionManager->IsShortStaticClick();

	if( pick.GetPickedType() == eLevelUpPicked || 
		pick.GetPickedType() == eLevelDownPicked )
	{
		int32 type = eNewDuplicateUnder;
		switch(currentTool)
		{
		case kInsertShellLevelTool:type = eNewDuplicateShellUnder;break;
		case kInsertDuplicateLevelTool:type = eNewDuplicateUnder;break;
		case kInsertEmptyLevelTool:type = eNewEmptyLevel;break;
		}
		InsertLevelAction::Create( &action, modeler, pick, type );
		if(action)
		{
			gActionManager->PostAction(action, kDefaultActionNumber, modeler->GetContext());
			return true;
		}
	}


	return false;
}

boolean MouseDown::DeleteTool(	BuildingModeler* modeler,  
								const TMCPoint& mousePos, 
								BuildingPanePart* pane,
								const int32 currentTool )
{
	TMCCountedPtr<IShAction> action;

	const boolean shortClick =	gActionManager->IsShortStaticClick();

	DeleteAction::Create( &action, modeler, mousePos, pane );
	if(action)
	{
		gActionManager->PostAction(action, kDeleteAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::InsertLevel(BuildingModeler* modeler,  
								const int32 currentTool)
{
	TMCCountedPtr<IShAction> action;

	BuildingPrim* building = modeler->GetBuildingNoAddRef();

	const int32 groundIndex = building->GetGroundLevelIndex();

	int32 duplicateMode = eNewDuplicateShellUnder;

	Picking pick;
	switch(currentTool)
	{
	case kLevelTopMenuAction:
		{
			pick.SetPickedObject(building->GetLevel(building->GetLevelCount()-1));
			pick.SetPickedType(eLevelUpPicked);
		} break;
	case kLevelBottomMenuAction:
		{
			pick.SetPickedObject(building->GetLevel(0));
			pick.SetPickedType(eLevelDownPicked);
		} break;
	case kLevelOnActiveMenuAction:
		{
			const int32 levelIndex = modeler->Get2DActiveLevel();
			// If the level index is under the ground, we add the new level
			// under, otherwise we add it over
			if(levelIndex<0)
				return false;

			pick.SetPickedObject(building->GetLevel(levelIndex));
			pick.SetPickedType(levelIndex<groundIndex?eLevelDownPicked:eLevelUpPicked);

			duplicateMode = eNewDuplicateUnder;
		} break;
	}
	InsertLevelAction::Create( &action, modeler, pick, duplicateMode );

	if(action)
	{
		gActionManager->PostAction(action, currentTool, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::MovePoint(	BuildingModeler* modeler,  
								const TVector2& offset,
								CommonPoint* point)
{
	TMCCountedPtr<IShAction> action;

	if(offset.x==0 && offset.y==0)
		return false;

	MoveAction::Create( &action, modeler, offset, point);

	if(action)
	{
		gActionManager->PostAction(action, kOffsetAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::MoveObject(	BuildingModeler* modeler,  
								const TVector2& offset )
{
	TMCCountedPtr<IShAction> action;

	if(offset.x==0 && offset.y==0)
		return false;

	MoveObjectAction::Create( &action, modeler, offset);

	if(action)
	{
		gActionManager->PostAction(action, kOffsetObjectAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::ScaleSelection(	BuildingModeler* modeler,  
									const TVector2& scale )
{
	TMCCountedPtr<IShAction> action;

//	if(scale.x==0 || scale.y==0)
//		return false;

	if(scale.x==1 && scale.y==1)
		return false;

	ScaleAction::Create( &action, modeler, scale);

	if(action)
	{
		gActionManager->PostAction(action, kScaleAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::ScaleObject(BuildingModeler* modeler,  
								const TVector2& scale )
{
	TMCCountedPtr<IShAction> action;

	if(scale.x==0 || scale.y==0)
		return false;

	if(scale.x==1 && scale.y==1)
		return false;

	ScaleObjectAction::Create( &action, modeler, scale);

	if(action)
	{
		gActionManager->PostAction(action, kOffsetObjectAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::RotateSelection(BuildingModeler* modeler,  
									const TVector2& cosSin )
{
	TMCCountedPtr<IShAction> action;

	if(RealAbs(cosSin.x)>1)
		return false;

	if(RealAbs(cosSin.y)>1)
		return false;

	RotateAction::Create( &action, modeler, cosSin);

	if(action)
	{
		gActionManager->PostAction(action, kRotateAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::DeleteSelection( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	PrimitiveStatus* status = modeler->GetBuildingNoAddRef()->GetStatus();

	if(!status->fHasSelection)
		return false;

	DeleteAction::Create( &action, modeler);

	if(action)
	{
		gActionManager->PostAction(action, kDeleteAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::DeleteLevel( BuildingModeler* modeler, const int32 level )
{
	const int32 levelCount = modeler->GetBuildingNoAddRef()->GetLevelCount();
	if(level<0 || level>=levelCount || levelCount==1)
		return false;

	TMCCountedPtr<IShAction> action;

	DeleteLevelAction::Create( &action, modeler, level);

	if(action)
	{
		gActionManager->PostAction(action, kDeleteAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::CreateRoom( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	PrimitiveStatus* status = modeler->GetBuildingNoAddRef()->GetStatus();

	if(!status)
		return false;

	if(!status->fSelectedWallGlobalCount)
	{
		CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
		TMCDynamicString message;
		gResourceUtilities->GetIndString( message, kAlertStrings, 1);
		gPartUtilities->Alert(message);
		return false;
	}

	CreateRoomAction::Create( &action, modeler);

	if(action)
	{
		gActionManager->PostAction(action, kCreateRoomMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::CreateRoof( BuildingModeler* modeler, const int32 actionNumber )
{
	TMCCountedPtr<IShAction> action;

	CreateRoofAction::Create( &action, modeler, actionNumber);

	if(action)
	{
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::Split( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	SplitAction::Create( &action, modeler );

	if(action)
	{
		gActionManager->PostAction(action, kSplitMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::Merge( BuildingModeler* modeler, const boolean inOne )
{
	TMCCountedPtr<IShAction> action;

	MergeAction::Create( &action, modeler, inOne );

	if(action)
	{
		gActionManager->PostAction(action, kMergeMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::ReplaceWall( BuildingModeler* modeler, const int32 actionNumber )
{
	TMCCountedPtr<IShAction> action;

	ReplaceWallAction::Create( &action, modeler, actionNumber );

	if(action)
	{
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetLevelHeight(	BuildingModeler* modeler, 
									const TMCArray<int32>& levels, 
									const real32 height)
{
	TMCCountedPtr<IShAction> action;
	LevelHeightAction::Create( &action, modeler, levels, height);

	if(action)
	{
		gActionManager->PostAction(action, kLevelHeightMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetDimension(BuildingModeler*	modeler, 
								const real32		dimension,
								const EDimensionType	type)
{
	TMCCountedPtr<IShAction> action;
	DimensionAction::Create( &action, modeler, dimension, type);

	if(action)
	{
		int32 actionNumber = kFloorThicknessMenuAction;
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetUInt32Value(BuildingModeler*	modeler, 
								const uint32		value,
								const EDimensionType	type)
{
	TMCCountedPtr<IShAction> action;
	UInt32Action::Create( &action, modeler, value, type);

	if(action)
	{
		int32 actionNumber = kFloorThicknessMenuAction; // ?what the use of this number ?
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetSelectionFlag(BuildingModeler*	modeler, 
										const boolean		flagValue,
										const EFlagType		flagType)
{
	TMCCountedPtr<IShAction> action;
	SelectionFlagAction::Create( &action, modeler, flagValue, flagType);

	if(action)
	{
		int32 actionNumber = kDrawCeilingMenuAction;
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}
	
boolean MenuAction::SmoothSelection(BuildingModeler*	modeler,
									const boolean		value)
{
	TMCCountedPtr<IShAction> action;
	SmoothSelectionAction::Create( &action, modeler, value);

	if(action)
	{
		gActionManager->PostAction(action, kSmoothSelection, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetSelectionName(BuildingModeler*	modeler, 
									const TMCString&	name)
{
	TMCCountedPtr<IShAction> action;
	SelectionNameAction::Create( &action, modeler, name);

	if(action)
	{
		gActionManager->PostAction(action, kNameMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetRoofProfile(	BuildingModeler*	modeler, 
									const ERoofProfileID profile,
									const boolean		onTop,
									const boolean		inside)
{
	TMCCountedPtr<IShAction> action;
	RoofProfileAction::Create( &action, modeler, profile, onTop,inside);

	if(action)
	{
		gActionManager->PostAction(action, kRoofProfileMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetObjectInstance(BuildingModeler* modeler,
									TMCString& objectName,
									const boolean isObject ) // can be an object or a group
{
	TMCCountedPtr<IShAction> action;
	AttachObjectAction::Create( &action, modeler, objectName, isObject );

	if(action)
	{
		gActionManager->PostAction(action, kAttachObjectMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetObjectPlacementType(BuildingModeler* modeler,
										EPlacementType placement )
{
	TMCCountedPtr<IShAction> action;
	PlaceObjectChildAction::Create( &action, modeler, placement );

	if(action)
	{
		gActionManager->PostAction(action, kPlaceObjectChildMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetObjectPlacement(BuildingModeler* modeler,
										const real32 value,
										const int32 id)

{
	TMCCountedPtr<IShAction> action;
	CustomPlaceObjectChildAction::Create( &action, modeler, value, id );

	if(action)
	{
		gActionManager->PostAction(action, kPlaceObjectChildMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetLevelName(	BuildingModeler* modeler,
									const int32 level,
									const TMCString& name)
{
	TMCCountedPtr<IShAction> action;
	LevelName::Create( &action, modeler, level, name);

	if(action)
	{
		gActionManager->PostAction(action, kNameMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::ShowActiveLevel(BuildingModeler* modeler,
									const int32 show)
{
	TMCCountedPtr<IShAction> action;
	ShowActiveLevel::Create( &action, modeler, show);

	if(action)
	{
		gActionManager->PostAction(action, kShowActiveLevel, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::HoleEditionOnOff(BuildingModeler* modeler)
{
	TMCCountedPtr<IShAction> action;
	HoleEditionOnOff::Create( &action, modeler );

	if(action)
	{
		gActionManager->PostAction(action, kHolesEditionOnOff, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::ShowHide(BuildingModeler* modeler,  
								const int32 currentTool)
{
	TMCCountedPtr<IShAction> action;

	EShowHideOption option=eUnknownShowHideOption;
	switch(currentTool)
	{
		case kHideSelectionMenuAction:	option=eHideSelection; break;
		case kHideAllMenuAction:		option=eHideAll; break;
		case kHideWallsMenuAction:		option=eHideAllWalls; break;
		case kHideRoomsMenuAction:		option=eHideAllRooms; break;
		case kHideRoofsMenuAction:		option=eHideAllRoofs; break;
		case kShowAllMenuAction:		option=eShowAll; break;
		case kShowWallsMenuAction:		option=eShowAllWalls; break;
		case kShowRoomsMenuAction:		option=eShowAllRooms; break;
		case kShowRoofsMenuAction:		option=eShowAllRoofs; break;
		default: return false;
	}

	ShowHideAction::Create( &action, modeler, option );

	if(action)
	{
		gActionManager->PostAction(action, currentTool, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::InvertSelection( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	InvertSelectionAction::Create( &action, modeler );

	if(action)
	{
		gActionManager->PostAction(action, kInvertSelectionMenuAction, modeler->GetContext());
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////
//
// Name list filling function
//
#include "IMFTextPopupPart.h"
void FillInNamesPopup(IMFPart*	inPart, BuildingModeler* modeler, const int32 listID)
{
	TMCCountedPtr<IMFPart> popupPart;
	inPart->FindChildPartByID( &popupPart, listID );
	if(!popupPart)
		return;

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCCountedPtr<IMFTextPopupPart> menuPopup;
	popupPart->QueryInterface( IID_IMFTextPopupPart, (void **)&menuPopup );	

	// Clean the current popup menu
	menuPopup->RemoveAll();

	// Fill the menu with existing names
	BuildingPrim* building = modeler->GetBuildingNoAddRef();
	const NameChainedList& dictionary =  building->Data().fDictionary;

	const int32 nameCount = dictionary.NameCount();

	for(int32 iName=0 ; iName<nameCount ; iName++)
	{
		const TMCString& name = dictionary.GetNameStr(iName);
		menuPopup->AppendMenuItem(name);
//		menuPopup->SetItemEnabled(iName, true); // it's a custom menu: enable manualy the items
	}
}
////////////////////////////////////////////////////////////////////////////
//
// Domain list filling function
//
void FillInDomainsPopup(IMFPart*	inPart, BuildingModeler* modeler, const int32 listID)
{
	TMCCountedPtr<IMFPart> popupPart;
	inPart->FindChildPartByID( &popupPart, listID );
	if(!popupPart)
		return;

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCCountedPtr<IMFTextPopupPart> menuPopup;
	popupPart->QueryInterface( IID_IMFTextPopupPart, (void **)&menuPopup );	

	// Clean the current popup menu
	menuPopup->RemoveAll();

	// Fill the menu with existing names
	BuildingPrim* building = modeler->GetBuildingNoAddRef();

	const int32 domainCount = building->GetUVSpaceCount();

	for(int32 iDomain=0 ; iDomain<domainCount ; iDomain++)
	{
		UVSpaceInfo& domain = building->GetUVSpace(iDomain);
		menuPopup->AppendMenuItem(domain.fName);
	}
}

boolean MenuAction::SelectDeselectBy( BuildingModeler* modeler, const int32 actionNumber )
{
	TMCCountedPtr<IShAction> action;

	boolean select = false;

	switch(actionNumber)
	{
	case kSelectByName: select=true;
	case kDeselectByName:
		{
			// Ask for the name to select/deselect
			TMCDynamicString nameStr;

			// Ask for the object
			TMCCountedPtr<IMFDialogPart> theDialog;
			if( OpenDialog(&theDialog, kAskName) )
			{
				// Build the dialog popup menu
				TMCCountedPtr<IMFPart> theDialogPart;
				theDialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
				FillInNamesPopup(theDialogPart, modeler, eNameList);

				if( theDialog->Go() )
				{
					// User hit OK: selection is set to a new shading domain
					if( !GetDialogString(theDialog, nameStr, eNameList) )
					{
						// Couldn't get the string: return
						theDialog->Finished();
						break;
					}
				}
				else
				{
					// User hit cancel
					theDialog->Finished();
					break;
				}
				
				theDialog->Finished();
			}
			else
			{
				// Couldn't open the dialog
				break;
			}

			SelectByNameAction::Create( &action, modeler, nameStr, select );
	
		} break;
	case kSelectByShadingDomain: select=true;
	case kDeselectByShadingDomain:
		{
			// Ask for the domain to select/deselect
			int32 domain=0;

			// Ask for the object
			TMCCountedPtr<IMFDialogPart> theDialog;
			if( OpenDialog(&theDialog, kAskDomain) )
			{
				// Build the dialog popup menu
				TMCCountedPtr<IMFPart> theDialogPart;
				theDialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
				FillInDomainsPopup(theDialogPart, modeler, eDomainList);

				if( theDialog->Go() )
				{
					// User hit OK: selection is set to a new shading domain
					if( !GetDialogValue(theDialog, domain, eDomainList) )
					{
						// Couldn't get the string: return
						theDialog->Finished();
						break;
					}
				}
				else
				{
					// User hit cancel
					theDialog->Finished();
					break;
				}
				
				theDialog->Finished();
			}
			else
			{
				// Couldn't open the dialog
				break;
			}

			SelectByDomainAction::Create( &action, modeler, domain, select );
		} break;
	}

	if(action)
	{
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SelectAll( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	SelectAllAction::Create( &action, modeler );

	if(action)
	{
		gActionManager->PostAction(action, kaSelectAll, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::FlipSelection( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	FlipAction::Create( &action, modeler );

	if(action)
	{
		gActionManager->PostAction(action, cSymetry, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetDefaultSetting(	BuildingModeler* modeler,
											const real32 value,
											const int32 id)
{
	TMCCountedPtr<IShAction> action;

	DefaultSettingAction::Create( &action, modeler, value, id );

	if(action)
	{
		gActionManager->PostAction(action, kDefaultActionNumber, modeler->GetContext());
		return true;
	}

	return false;
}


boolean MenuAction::CopyCut( BuildingModeler* modeler, 
							 const int32 menuAction )
{
	TMCCountedPtr<IShAction> action;

	BuildingPrim* building = modeler->GetBuildingNoAddRef();
	PrimitiveStatus* status = building->GetStatus();

	if(!status)
		return false;

	if(!status->fHasSelection)
		return false;

	CutCopyAction::Create( &action, modeler, menuAction );


	if(action)
	{
		gActionManager->PostAction(action, menuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::Paste( BuildingModeler* modeler )
{
	TMCCountedPtr<IShAction> action;

	PasteAction::Create( &action, modeler );


	if(action)
	{
		gActionManager->PostAction(action, kaPaste, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::Duplicate( BuildingModeler* modeler, const int32 actionNumber )
{
	TMCCountedPtr<IShAction> action;

	int32 levelOffset = 0;
	if(actionNumber==kDuplicateOverAction)
		levelOffset = 1;
	else if(actionNumber==kDuplicateUnderAction)
		levelOffset = -1;

	DuplicateAction::Create( &action, modeler, (actionNumber==cSymDuplicate), levelOffset );

	if(action)
	{
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::CutAndPaste( BuildingModeler* modeler, const int32 actionNumber )
{
	TMCCountedPtr<IShAction> action;

	int32 levelOffset = 0;
	if(actionNumber==kMoveOverMenuAction)
		levelOffset = 1;
	else if(actionNumber==kMoveUnderMenuAction)
		levelOffset = -1;

	CutAndPasteAction::Create( &action, modeler, levelOffset );

	if(action)
	{
		gActionManager->PostAction(action, actionNumber, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::SetShadingDomain( BuildingModeler* modeler, 
							 const int32 menuAction )
{
	BuildingPrim* building = modeler->GetBuildingNoAddRef();
	PrimitiveStatus* status = building->GetStatus();

	if(!status)
		return false;

	if(!status->fHasSelection)
		return false;

	int32 shadingDomainID = kNoDomains;
	int32 menuID = -1;
	TMCString255 domainName;

	if(menuAction >= kCreateShadingDomain0)
	{
		menuID = menuAction - kCreateShadingDomain0;

		// Ask for a new shading domain
		TMCCountedPtr<IMFDialogPart> theDialog;
		if( OpenDialog(&theDialog, kAskDomainName) )
		{
			// Propose a default name for the domain: "Texture n"
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
			gResourceUtilities->GetIndString(domainName, kModelerStrings, 26 );
	//		TMCCountedPtr<I3DExGeometricPrimitive> primitive;
	//		building->QueryInterface(IID_I3DExGeometricPrimitive, (void**) &primitive);
	//		ThrowIfNil(primitive);
			TMCString31 num;
			num.FromInt32(building->GetUVSpaceCount());
			domainName+=num;

			SetDialogString(theDialog, domainName, eName);

			if( theDialog->Go() )
			{
				// User hit OK: selection is set to a new shading domain
				if( !GetDialogString(theDialog, domainName, eName) )
				{
					// Couldn't get the string: return
					theDialog->Finished();
					return false;
				}
			}
			else
			{
				// User hit cancel
				theDialog->Finished();
				return false;
			}
			
			theDialog->Finished();
		}
		else
		{
			// Couldn't open the dialog
			return false;
		}
	}
	else
	{
		shadingDomainID = menuAction - kShadingDomainID2;
		menuID = 2;
		if(shadingDomainID<0)
		{
			shadingDomainID = menuAction - kShadingDomainID1;
			menuID = 1;
			if(shadingDomainID<0)
			{
				shadingDomainID = menuAction - kShadingDomainID0;
				menuID = 0;
			}
		}
	}

	return MenuAction::ShadingDomain(modeler, menuAction, shadingDomainID, menuID, domainName);
}

boolean MenuAction::ShadingDomain(	BuildingModeler* modeler, 
									const int32 menuAction,
									const int32 domainID,
									const int32 subPartID,
									const TMCString& domainName )
{
	TMCCountedPtr<IShAction> action;

	ShadingDomainAction::Create( &action, modeler, domainID, subPartID, domainName );

	if(action)
	{
		gActionManager->PostAction(action, menuAction, modeler->GetContext());
		return true;
	}

	return false;
}

boolean MenuAction::DelShadingDomain(	BuildingModeler* modeler, 
										const int32 domainID  )
{
	if(domainID<0)
		return false;

	TMCCountedPtr<IShAction> action;

	DelShadingDomainAction::Create( &action, modeler, domainID );

	if(action)
	{
		gActionManager->PostAction(action, kDeleteShadingDomain, modeler->GetContext());
		return true;
	}

	return false;
}

#endif // !NETWORK_RENDERING_VERSION

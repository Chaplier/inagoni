/****************************************************************************************************

		MDefaultSettingActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/15/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MDefaultSettingActions.h"
#include "MiscComUtilsImpl.h"

//////////////////////////////////////////////////////////////////////
//
DefaultSettingAction::DefaultSettingAction(	BuildingModeler*	modeler,
											const real32		value,
											const int32			id )
									:ModelerAction(modeler)
{
	fValue = value;
	fID = id;

	fRefreshGeometry=true;
}

void DefaultSettingAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const real32		value,
								const int32			id )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new DefaultSettingAction(modeler, value, id) );
}

MCCOMErr DefaultSettingAction::Do()
{
	BuildingPrimData& data = fBuildingPrimitive->GetData();
	
	real32 prevValue=0;
	switch(fID)
	{
	case eDLevelHeight:
		{
			prevValue = data.GetDefaultLevelHeight();
			data.SetDefaultLevelHeight(fValue);
		}break;
	case eDRoofMin:
		{
			prevValue = data.GetDefaultRoofMin();
			data.SetDefaultRoofMin(fValue);
		}break;
	case eDRoofMax:
		{
			prevValue = data.GetDefaultRoofMax();
			data.SetDefaultRoofMax(fValue);
		}break;
	case eDWallThickness:
		{
			prevValue = data.GetDefaultWallThickness();
			data.SetDefaultWallThickness(fValue);
		}break;
	case eDFloorThickness:
		{
			prevValue = data.GetDefaultFloorThickness();
			data.SetDefaultFloorThickness(fValue);
		}break;
	case eDCeilingThickness:
		{
			prevValue = data.GetDefaultCeilingThickness();
			data.SetDefaultCeilingThickness(fValue);
		}break;
	case eDWindowHeight:
		{
			prevValue = data.GetDefaultWindowHeight();
			data.SetDefaultWindowHeight(fValue);
		}break;
	case eDWindowLength:
		{
			prevValue = data.GetDefaultWindowLength();
			data.SetDefaultWindowLength(fValue);
		}break;
	case eDWindowAltitude:
		{
			prevValue = data.GetDefaultWindowAltitude();
			data.SetDefaultWindowAltitude(fValue);
		}break;
	case eDDoorHeight:
		{
			prevValue = data.GetDefaultDoorHeight();
			data.SetDefaultDoorHeight(fValue);
		}break;
	case eDDoorLength:
		{
			prevValue = data.GetDefaultDoorLength();
			data.SetDefaultDoorLength(fValue);
		}break;
	case eDStairwayWidth:
		{
			prevValue = data.GetDefaultStairwayWidth();
			data.SetDefaultStairwayWidth(fValue);
		}break;
	case eDStairwayLength:
		{
			prevValue = data.GetDefaultStairwayLength();
			data.SetDefaultStairwayLength(fValue);
		}break;
	}

	fValue = prevValue;

	fBuildingPrimitive->InvalidateAll(kAllLevels);
	if(fID==eDLevelHeight)
		fBuildingPrimitive->SetAllLevelDistanceToGround();

	return ModelerAction::Do();
}

MCCOMErr DefaultSettingAction::Undo()
{
	return DefaultSettingAction::Do();
}

MCCOMErr DefaultSettingAction::Redo()
{
	return DefaultSettingAction::Do();
}

MCCOMErr DefaultSettingAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 17);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

#endif // !NETWORK_RENDERING_VERSION


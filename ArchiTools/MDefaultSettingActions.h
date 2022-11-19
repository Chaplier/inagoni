/****************************************************************************************************

		MDefaultSettingActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/15/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MDefaultSettingActions__
#define __MDefaultSettingActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "IShUtilities.h"
struct IShAction;

class DefaultSettingAction : public ModelerAction
{
protected:
	DefaultSettingAction(BuildingModeler*	modeler,
						const real32		value,
						const int32			id );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const real32		value,
						const int32			id );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	real32	fValue;
	int32	fID;
};

#endif

#endif // !NETWORK_RENDERING_VERSION

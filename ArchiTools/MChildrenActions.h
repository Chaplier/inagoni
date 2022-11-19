/****************************************************************************************************

		MChildrenActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/9/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MChildrenActions__
#define __MChildrenActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "IShUtilities.h"
class BuildingModeler;
struct I3DShObject;
struct IShAction;
class SubObject;

// Create instances of the I3DShObject on the selected SubObjects
class AttachObjectAction :	public ModelerAction//,
						//	public BuildingRecorder
{
protected:

	AttachObjectAction( BuildingModeler*	modeler,
						TMCString&			sceneObject,
						const boolean		isObject); // can be an object or a group

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						TMCString&			sceneObject,
						const boolean		isObject); // can be an object or a group

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TMCDynamicString	fObjectName;
	boolean				fIsObject;// can be an object or a group
	TMCClassArray<TMCDynamicString>	fPrevObjects; // Undo data
};

///////////////////////////////////////////////////////////////////////////////////
//
//

class PlaceObjectChildAction :	public ModelerAction,
								public BuildingRecorder
{
protected:

	PlaceObjectChildAction( BuildingModeler*	modeler,
							const EPlacementType placement );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const EPlacementType placement );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	EPlacementType fPlacement;
};

///////////////////////////////////////////////////////////////////////////////////
//
//

class CustomPlaceObjectChildAction :	public ModelerAction,
										public BuildingRecorder
{
protected:

	CustomPlaceObjectChildAction( BuildingModeler*	modeler,
								const real32		value,
								const int32			id );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const real32		value,
						const int32			id );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	real32 fValue;
	int32 fID;
};



#endif

#endif // !NETWORK_RENDERING_VERSION

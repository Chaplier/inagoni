/****************************************************************************************************

		MInsertionAction.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/18/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MInsertionAction__
#define __MInsertionAction__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "MPicking.h"
class BuildingModeler;
class BuildingPanePart;
class TMCPoint;
struct IShAction;

//////////////////////////////////////////////////////////////////////
//
// Object Insertion Action
//
class InsertObjectAction :	public ModelerAction,
								public BuildingRecorder
{
protected:

	InsertObjectAction(	BuildingModeler*	modeler,
							BuildingPanePart*	pane,
							const Picking&		picked,
							const TMCPoint&		mousePos,
							const boolean		shortClick,
							const int32			objectKind);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos,
						const boolean		shortClick,
						const int32			objectKind);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fCouldInsert; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	TMCCountedPtr<BuildingPanePart> fPanePart;
	Picking		fPicked;
	TVector3	fHitPos;
	TMCPoint	fMousePos;
	EObjectType	fObjectKind; // Window, door, ...
	boolean		fShortClick;
	boolean		fCouldInsert;
};

//////////////////////////////////////////////////////////////////////
//
// Level Insertion Action
//
class InsertLevelAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	InsertLevelAction(	BuildingModeler*	modeler,
						const Picking&		picked,
						const int32			type);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const Picking&		picked,
						const int32			type);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	Picking		fPicked;
	int32		fInsertType;
};


#endif

#endif // !NETWORK_RENDERING_VERSION

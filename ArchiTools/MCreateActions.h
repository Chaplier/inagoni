/****************************************************************************************************

		MCreateActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/25/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MCreateActions__
#define __MCreateActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "IShUtilities.h"
class BuildingModeler;
class BuildingPanePart;
#include "MPicking.h"
struct IShAction;

//////////////////////////////////////////////////////////////////////
//
// Create Action
//
class BuildWallAction :	public ModelerAction,
						public BuildingRecorder
{
protected:

	BuildWallAction(	BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const TMCPoint&		mousePos,
						EWallType			type );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const TMCPoint&		mousePos,
						EWallType			type);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
//	boolean		MCCOMAPI CanUndo()	{ return fBuiltWall; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	void SetInLevel();
	void CloseWall(Wall*);

	TMCCountedPtr<BuildingPanePart> fPanePart;
	Picking		fPicked;
	TMCPoint	fMousePos;
	int32		fInLevel;
	EWallType	mWallType;
};


//////////////////////////////////////////////////////////////////////
//
// Create room Action
//
class CreateRoomAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	CreateRoomAction( BuildingModeler*	modeler );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fCouldBuild; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	boolean						fCouldBuild;
};

//////////////////////////////////////////////////////////////////////
//
// Create roof Action
//
class CreateRoofAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	CreateRoofAction( BuildingModeler*	modeler, const int32 actionNumber );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const int32 actionNumber );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fCouldBuild; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	boolean						fCouldBuild;
	int32						fActionNumber;
};

//////////////////////////////////////////////////////////////////////

class RoofProfileAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	RoofProfileAction(	BuildingModeler*	modeler, 
						const ERoofProfileID profile,
						const boolean		onTop,
						const boolean		inside);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const ERoofProfileID profile,
						const boolean		onTop,
						const boolean		inside);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	void SetHeight();

	ERoofProfileID				fProfile;
	boolean						fOnTop;
	boolean						fInside;
};


//////////////////////////////////////////////////////////////////////
//
// Split Action: split Wall, ZoneSection or Profile
//
class SplitAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	SplitAction( BuildingModeler*	modeler );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler );

	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
};

//////////////////////////////////////////////////////////////////////
//
// Merge Action: merge common point in their middle
//
class MergeAction :	public ModelerAction,
					public BuildingRecorder
{
protected:

	MergeAction( BuildingModeler*	modeler, const boolean inOne );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const boolean inOne );

	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	boolean fMergeInOne;
};

//////////////////////////////////////////////////////////////////////
//
// Replace a wall by another
//
class ReplaceWallAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	ReplaceWallAction( BuildingModeler*	modeler, const int32 actionNumber );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const int32 actionNumber );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	int32						fActionNumber;
};


//////////////////////////////////////////////////////////////////////
//
// Delete Action
//
class DeleteAction :	public ModelerAction,
						public BuildingRecorder
{
protected:

	DeleteAction( BuildingModeler*	modeler );
	DeleteAction( BuildingModeler*	modeler,  
				const TMCPoint&		mousePos, 
				BuildingPanePart*	pane );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler);

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,  
						const TMCPoint&		mousePos, 
						BuildingPanePart*	pane);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	boolean fDeleteSelection;
	Picking	fPicked;
};

//////////////////////////////////////////////////////////////////////
//
// Delete Level Action
//
class DeleteLevelAction :	public ModelerAction,
						public BuildingRecorder
{
protected:

	DeleteLevelAction( BuildingModeler*	modeler, const int32 level );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const int32			level);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	int32 fLevel;
};


#endif

#endif // !NETWORK_RENDERING_VERSION


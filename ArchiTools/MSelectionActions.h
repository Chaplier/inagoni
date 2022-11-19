/****************************************************************************************************

		MSelectionActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/4/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MSelectionActions__
#define __MSelectionActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "MPicking.h"
#include "IMFPart.h"

//////////////////////////////////////////////////////////////////////
//
static const uint8 kSelectedFlag = 1;
static const uint8 kHiddenFlag = 2;
struct LevelSelection
{
	LevelSelection():fAllSelected(false),fAllHidden(false){}
	boolean fAllSelected; // if they are all selected, they are all visible
	boolean fAllHidden;	// if they are all hidden, they are all not selected
	TMCArray<uint8> fSelectionFlag;

	inline void			SetSelected(const int32 index)	{fSelectionFlag[index]|=kSelectedFlag;}
	inline void			SetHidden(const int32 index)	{fSelectionFlag[index]|=kHiddenFlag;}
	inline void			SetNotSelected(const int32 index){fSelectionFlag[index]&=~kSelectedFlag;}
	inline void			SetNotHidden(const int32 index) {fSelectionFlag[index]&=~kHiddenFlag;}
	inline boolean		Selected(const int32 index)		{return ((fSelectionFlag[index]&kSelectedFlag) != 0 );}
	inline boolean		Hidden(const int32 index)		{return ((fSelectionFlag[index]&kHiddenFlag) != 0 );}
};

class SelectionRecorder
{
public:
	SelectionRecorder();

	void SaveSelection( BuildingPrim* primitive ){SaveSelection(primitive, fRecordedSelection );}
	void SwapSelection( BuildingPrim* primitive );

protected:

	void SaveSelection( BuildingPrim* primitive, TMCClassArray<LevelSelection>& saveSelection );

	TMCClassArray<LevelSelection> fRecordedSelection;
};
//////////////////////////////////////////////////////////////////////

#include "BuildingModeler.h"
#include "MPicking.h"
struct IShAction;
#include "MCPoint.h"
//////////////////////////////////////////////////////////////////////
//
// Selection Action
//
class SelectionAction : public ModelerAction,
						public SelectionRecorder
{
protected:
	SelectionAction(	BuildingModeler*	modeler,
						const Picking&		picked,
						const TMCPlatformEvent& event,
						const TMCPoint&		mousePos);
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const Picking&		picked,
						const TMCPlatformEvent& event,
						const TMCPoint&		mousePos );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fSelectionHasChanged; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	Picking fPicked;
	boolean fShift;
	boolean fDoubleClick;
	boolean fSelectionHasChanged;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
class MarqueeMouseAction :	public ModelerMouseAction,
							public SelectionRecorder
{
protected:

	MarqueeMouseAction(	BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	void		CreateFramePart();

	boolean fHasMoved;
	TMCPoint fFirstPoint;
	int32	fInLevel;

	// The rectangle
	TMCCountedPtr<IMFPart>	fFramePart;
};

//////////////////////////////////////////////////////////////////////
//
//	change one level name
//
class LevelName : public ModelerAction
{
protected:
	LevelName(	BuildingModeler*	modeler,
				const int32 level,
				const TMCString& name );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const int32 level,
						const TMCString& name  );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	void	SetName();

	int32 fLevel;
	TMCDynamicString fName;
};

//////////////////////////////////////////////////////////////////////
//
// ShowActiveLevel Action: to show only the active level in the 3D view, or all the building
// (since it modifies the selection, it's a selection action)
//
class ShowActiveLevel : public ModelerAction,
						public SelectionRecorder
{
protected:
	ShowActiveLevel(	BuildingModeler*	modeler,
						const int32			show );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const int32			show );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	void	ShowLevel();

	int32 fShowLevel;
};

//////////////////////////////////////////////////////////////////////
//
// ShowActiveLevel Action: to show only the active level in the 3D view, or all the building
// (since it modifies the selection, it's a selection action)
//
class HoleEditionOnOff :	public ModelerAction,
							public SelectionRecorder
{
protected:
	HoleEditionOnOff(	BuildingModeler*	modeler  );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	void SwitchMode();
};

//////////////////////////////////////////////////////////////////////
//
// SmoothSelectionAction : smooth or sharpen the edges attached to the 
// curently selected hole points
//
class SmoothSelectionAction :	public ModelerAction,
								public SelectionRecorder
{
protected:
	SmoothSelectionAction(	BuildingModeler*	modeler, const boolean smooth  );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const boolean		smooth );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	boolean fSmooth;
};

//////////////////////////////////////////////////////////////////////
//
// Show/Hide Action (since it modifies the selection, it's a selection action)
//
class ShowHideAction : public ModelerAction,
						public SelectionRecorder
{
protected:
	ShowHideAction(	BuildingModeler*	modeler,
					const EShowHideOption option);
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const EShowHideOption option );

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	EShowHideOption fOption;
};

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Invert Selection
//
class InvertSelectionAction :	public ModelerAction,
								public SelectionRecorder
{
protected:
	InvertSelectionAction(	BuildingModeler*	modeler );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler);

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
};

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Select All
//
class SelectAllAction :	public ModelerAction,
						public SelectionRecorder
{
protected:
	SelectAllAction(	BuildingModeler*	modeler );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler);

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
};

//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//
// Select by Name
//
class SelectByNameAction :	public ModelerAction,
							public SelectionRecorder
{
protected:
	SelectByNameAction(	BuildingModeler* modeler, 
		const TMCString& nameStr, const boolean select );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const TMCString&	nameStr, 
						const boolean		select);

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TMCDynamicString	fName;
	boolean				fSelect;
};

//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Select by Domain
//
class SelectByDomainAction :	public ModelerAction,
								public SelectionRecorder
{
protected:
	SelectByDomainAction(	BuildingModeler* modeler, 
		const int32 domain, const boolean select );
public:
	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const int32			domain, 
						const boolean		select);

	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	int32	fDomain;
	boolean	fSelect;
};

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

class SelectionFlagAction :	public ModelerAction
{
protected:

	SelectionFlagAction(BuildingModeler*		modeler,
					const boolean		flagValue,
					const EFlagType		flagType);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const boolean		flagValue,
						const EFlagType		flagType);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	void SetFlag(); // for Do and Redo
	MCCOMErr ValidUndo();

	boolean						fFlagValue;
	EFlagType					fFlagType;
	TMCArray<boolean>			fPreviousFlags; // Undo data
};

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

class SelectionNameAction :	public ModelerAction
{
protected:

	SelectionNameAction(BuildingModeler*		modeler,
					const TMCString&			name);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TMCString&	name);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	void SetName(); // for Do and Redo

	TMCDynamicString			fName;
	TMCClassArray<TMCDynamicString>	fPreviousNames; // Undo data
};

#endif

#endif // !NETWORK_RENDERING_VERSION

/****************************************************************************************************

		MBuildingAction.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingAction__
#define __MBuildingAction__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"
#include "BuildingModeler.h"
#include "IShUtilities.h"

// Base for the modeler action.
// Do, Undo and Redo take care of the invalidations

//////////////////////////////////////////////////////////////////////
//
class ModelerAction : public TBasicAction 
{
protected:
	ModelerAction(BuildingModeler* modeler);
	virtual ~ModelerAction();

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	// IShAction methods
	MCCOMErr MCCOMAPI Do();
	MCCOMErr MCCOMAPI Undo();
	MCCOMErr MCCOMAPI Redo();
	boolean  MCCOMAPI WillCauseChange()	{ return true; }
	boolean  MCCOMAPI CanUndo()	{ return true; }
//	MCCOMErr MCCOMAPI GetName(TMCString& name);

protected:

	TMCCountedPtr<BuildingModeler>	fBuildingModeler;
	TMCCountedPtr<BuildingPrim>	fBuildingPrimitive;

	boolean	fRefreshGeometry; // Set this flag to true when the action modify the geometry
	boolean	fInvalidateStatus;
private:

};
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
class ModelerMouseAction : public TBasicMouseAction
{

protected:

	ModelerMouseAction(BuildingModeler* modeler, BuildingPanePart* pane);
	virtual ~ModelerMouseAction();

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();
	boolean		MCCOMAPI WillCauseChange()	{ return true; }
	boolean		MCCOMAPI CanUndo()	{ return true; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	TMCCountedPtr<BuildingModeler>	fBuildingModeler;
	TMCCountedPtr<BuildingPrim>		fBuildingPrimitive;
	TMCCountedPtr<BuildingPanePart> fPanePart;

	boolean	fRefreshGeometry; // Set this flag to true when the action modify the geometry
	boolean	fInvalidateStatus;
};
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
class BuildingRecorder
{
public:
	BuildingRecorder( );

	void SaveBuilding(BuildingPrim* primitive );
	void SwapBuilding(BuildingPrim* primitive );

protected:

	void WriteInBuffer(BuildingPrim* primitive, IShTokenStream** buffer );

	TMCCountedPtr<IShTokenStream> mSavedBuilding;
};
//////////////////////////////////////////////////////////////////////

#endif

#endif // !NETWORK_RENDERING_VERSION


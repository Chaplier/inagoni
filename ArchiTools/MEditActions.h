/****************************************************************************************************

		MEditActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	6/3/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MEditActions__
#define __MEditActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCPtrArray.h"
#include "MCCountedPtrHelper.h"
#include "BasicMCFCOMImplementations.h"
#include "IMFClipboard.h"
#include "IShUtilities.h"
#include "MBuildingAction.h"

class BuildingPrim;
class Level;

struct ClipboardData
{
	ClipboardData(){fLevelOffset=0;}
	ClipboardData(const int32 levelOffset){fLevelOffset=levelOffset;}

	// General data
	TMCCountedPtrArray<Level>			fLevels;
	// Objects data
	TMCCountedPtrArray<WallSubObject>	fWallObjects; // for the independant wall objects
	TMCCountedPtrArray<RoomSubObject>	fRoomObjects; // for the independant room objects
	// Profile data
	TMCCountedPtrArray<ProfilePoint> fBotProfilePoints;
	TMCCountedPtrArray<ProfilePoint> fTopProfilePoints;
	TMCCountedPtrArray<ProfilePoint> fBotInsidePoints;
	TMCCountedPtrArray<ProfilePoint> fTopInsidePoints;

	// Offset: for duplicate over or under
	int32	fLevelOffset;
};

class BuildingClipping :	public TBasicUnknown, 
							public IMFClipping
{

public:
	static void	Create(BuildingClipping** outClipping, const ClipboardData& clipData);

	virtual		MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	virtual		uint32		MCCOMAPI AddRef() { return TBasicUnknown::AddRef(); }
	STANDARD_RELEASE;

	// IMFClipping
	virtual		IDType	MCCOMAPI GetType(void) const { return fType; }
	virtual		void*	MCCOMAPI GetData(void) const { return (void*)&fData; } 
	virtual		uint32	MCCOMAPI GetSize(void) const { return 0; }
	virtual		IDType	MCCOMAPI GetFlattenedType(void) const { return kMFClipping_Empty; }
	virtual		boolean	MCCOMAPI Flatten(IMFClipping** outFlatClipping, uint32 inFlags) { return false; }

protected:

	BuildingClipping();
	BuildingClipping(IDType inDataType, const ClipboardData& clipData);

private:
	IDType			fType;
	ClipboardData	fData;
};


//////////////////////////////////////////////////////////////////////
//
// Cut Copy Action
//
class CutCopyAction :	public ModelerAction,
						public BuildingRecorder 
{
protected:
	CutCopyAction(BuildingModeler *modeler, const int32 actionNumber);
	virtual ~CutCopyAction();

public:
	static void	Create(IShAction** action, BuildingModeler *modeler, const int32 actionNumber);

	MCCOMErr MCCOMAPI Do();
	MCCOMErr MCCOMAPI Undo();
	MCCOMErr MCCOMAPI Redo();

protected:
	int32	fActionNumber;
};



//////////////////////////////////////////////////////////////////////
//
// Paste Action
//
class PasteAction :	public ModelerAction,
					public BuildingRecorder
{
protected:

	PasteAction(BuildingModeler*	modeler);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
};

//////////////////////////////////////////////////////////////////////
//
// Duplicate/DuplicateWithSymetrie
//
class DuplicateAction :	public ModelerAction,
						public BuildingRecorder
{
protected:

	DuplicateAction(BuildingModeler*	modeler, const boolean sym, const int32 levelOffset);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const boolean		sym, 
						const int32			levelOffset);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
	boolean						fSymetrie;
	int32						fLevelOffset;
};

//////////////////////////////////////////////////////////////////////
//
// Detach, Move Over and Move Under
//
class CutAndPasteAction :	public ModelerAction,
						public BuildingRecorder
{
protected:

	CutAndPasteAction(BuildingModeler*	modeler, const int32 levelOffset);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const int32			levelOffset);

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
	int32						fLevelOffset;
};

//////////////////////////////////////////////////////////////////////
//
// ShadingDomainAction
//
// Use kNoDomains and a name to create a new shading domain
//
class ShadingDomainAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	ShadingDomainAction(BuildingModeler*	modeler, 
						const int32 domain,
						const int32		menuID,
						const TMCString& name);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const int32			domain, 
						const int32			menuID,
						const TMCString&	name );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
	int32						fShadingDomain;
	int32						fMenuID; // To know on which part of the selection the domain is set
	TMCDynamicString			fDomainName;
};

class DelShadingDomainAction :	public ModelerAction,
							public BuildingRecorder
{
protected:

	DelShadingDomainAction(	BuildingModeler*	modeler, 
							const int32 domain);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler, 
						const int32			domain );

	// TBasicMouseAction methods
	MCCOMErr	MCCOMAPI Do();
	MCCOMErr	MCCOMAPI Undo();
	MCCOMErr	MCCOMAPI Redo();

protected:
	int32						fShadingDomain;
};

#endif

#endif // !NETWORK_RENDERING_VERSION


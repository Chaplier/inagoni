/****************************************************************************************************

		MBuildingDragAndDrop.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingDragAndDrop__
#define __MBuildingDragAndDrop__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"
#include "BuildingModeler.h"

extern const MCGUID CLSID_BuildingDropArea;
extern const MCGUID CLSID_BuildingDropCandidate;

class BuildingDropArea : public TBasicDropArea
{
public:
	BuildingDropArea();
	virtual ~BuildingDropArea();
	
	MCCOMErr MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	STANDARD_RELEASE;

	virtual void 		MCCOMAPI ReceiveDrop(
									IMFDropCandidate*	dropCandidate,
									IDType&				acceptedType,
									MFDragDropType		moveOrCopy,
									const TMCPoint&		mousePos);

	void						 SetModeler(BuildingModeler* modeler) {fBuildingModeler=modeler;}
	void						 SetPaneExt(BuildingPanePart* panePart) {fPanePart=panePart;}

protected:

	TMCCountedPtr<BuildingModeler>	fBuildingModeler;
	TMCCountedPtr<BuildingPanePart>	fPanePart;
};


class BuildingDropCandidate : public TBasicDropCandidate
{
public:

	BuildingDropCandidate();
	virtual ~BuildingDropCandidate();
	
	MCCOMErr MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	STANDARD_RELEASE;

	virtual MCCOMErr 	MCCOMAPI GetData	(	IDType 			inFlavor, 
												MFDragDropType 	inMoveOrCopy,
												void**			outData);
	
	
	void						 Initialize(int32 objectID){fObjectID=objectID;}

protected:

	int32 fObjectID;
};
                    
#endif

#endif // !NETWORK_RENDERING_VERSION

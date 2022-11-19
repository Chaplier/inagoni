/****************************************************************************************************

		MBuildingDragAndDrop.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingDragAndDrop.h"

#include "IShUtilities.h"
#include "COMUtilities.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "COMSafeUtilities.h"
#endif
#include "MMouseDown.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
const MCGUID CLSID_BuildingDropArea(R_CLSID_BuildingDropArea);
const MCGUID CLSID_BuildingDropCandidate(R_CLSID_BuildingDropCandidate);
#else
const MCGUID CLSID_BuildingDropArea={R_CLSID_BuildingDropArea};
const MCGUID CLSID_BuildingDropCandidate={R_CLSID_BuildingDropCandidate};
#endif


BuildingDropArea::BuildingDropArea()
{
}

BuildingDropArea::~BuildingDropArea()
{
}
	
MCCOMErr BuildingDropArea::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BuildingDropArea))
	{
		TMCCountedGetHelper<BuildingDropArea> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return TBasicDropArea::QueryInterface(riid, ppvObj);
}

void BuildingDropArea::ReceiveDrop(	IMFDropCandidate*	dropCandidate,
									IDType&				acceptedType,
									MFDragDropType		moveOrCopy,
									const TMCPoint&		mousePos)
{
	int32 dropID=0;

	dropCandidate->GetData(/*kDragFlavor_NewObject*/0,moveOrCopy,(void**)&dropID);

	gMenuUtilities->SetCurrentGlobalTool(dropID,true); 

	if( IsWindowToolID(dropID) || 
		IsDoorToolID(dropID) ||
		IsStairwayToolID(dropID) )
	{
		MouseDown::InsertObjectTool(fBuildingModeler, mousePos, fPanePart, dropID);
	}
	else if(IsLevelToolID(dropID))
	{
		MouseDown::InsertLevelTool(fBuildingModeler, mousePos, fPanePart, dropID);
	}
}


////////////////////////////////////////////////////////////////////////////////////


BuildingDropCandidate::BuildingDropCandidate()
{
//	fFlavors.AddElem(kDragFlavor_NewObject);

	fObjectID=0;
}

BuildingDropCandidate::~BuildingDropCandidate()
{
}
	
MCCOMErr BuildingDropCandidate::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BuildingDropCandidate))
	{
		TMCCountedGetHelper<BuildingDropCandidate> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return TBasicDropCandidate::QueryInterface(riid, ppvObj);
}

MCCOMErr BuildingDropCandidate::GetData( IDType 			inFlavor, 
									MFDragDropType 	inMoveOrCopy,
									void**			outData)
{
//	if (inFlavor == kDragFlavor_NewObject)
	{
		*(int32*)outData = fObjectID;
	}

	return MC_S_OK;
}
	
#endif // !NETWORK_RENDERING_VERSION

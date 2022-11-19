/****************************************************************************************************

		Building.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#include "ArchiTools.h"

#include "BuildingPrim.h"
#include "Copyright.h"
#include "MiscComUtilsImpl.h"

#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION
#include "BuildingModeler.h"
#include "MBuildingPanePart.h"
#include "MBuildingDragAndDrop.h"
#include "AssembleRoom.h"
#endif // !NETWORK_RENDERING_VERSION

#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#endif

#if (VERSIONNUMBER >= 0x040000)
extern boolean		gIsRenderingNode;	
#endif

// Serial Number global constantes
#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;

const int32 gMaxLevelCount = 2;
const int32 gMaxPointCount = 20;

void HandleException(TMCException* exception, const TMCString& methodName)
{
	static ExceptionLog gExceptionHandler(TMCDynamicString("ArchiTools"));
	
	if(exception)
	{
		gExceptionHandler.HandleException(*exception, methodName);
	}
	else
	{
		gExceptionHandler.HandleException(methodName);
	}
}

boolean CanCreateLevel(int32 curLevelCount)
{
	if(IsSerialValid())
		return true;
	else if(curLevelCount<gMaxLevelCount)
		return true;

	// Tell the user it's a demo version
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString alertMessage;
	gResourceUtilities->GetIndString(alertMessage, kAlertStrings, 7);
	gPartUtilities->Alert(alertMessage);

	return false;
}
boolean CanCreatePoint(int32 curPointCount)
{
	if(IsSerialValid())
		return true;
	else if(curPointCount<gMaxPointCount)
		return true;

	// Tell the user it's a demo version
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString alertMessage;
	gResourceUtilities->GetIndString(alertMessage, kAlertStrings, 7);
	gPartUtilities->Alert(alertMessage);

	return false;
}

#if NETWORK_RENDERING_VERSION
boolean IsSerialValid(){return true;}
#else
boolean IsSerialValid(){return gSerialNumberValid;}
#endif

boolean CheckSerial()
{
	if(!gSerialNumberValid)
	{
		const TMCString15 productInfo1("BMC3");
		const TMCString31 fileName1("BuildingData.dta");
		const int32 modulo1 = 11;
		const int32 resID = 250;
		const IDType familyID = kRID_GeometricPrimitiveFamilyID;
		const IDType classID = 'BuiP';

		TMCClassArray<KeyData> keyDatas;

		KeyData key1(	productInfo1,
						fileName1,
						modulo1,
						resID,
						familyID,
						classID);

		keyDatas.AddElem(key1);

		SerialNumber serial(keyDatas);
		serial.AddBannedLicense("73V3","C3M9","Y30M","M0BA");
		gSerialNumberValid = serial.CheckSerial();

		if(!gSerialNumberValid)
		{
			// Tell the user it's a demo version
			TMCDynamicString alertMessage;
			gResourceUtilities->GetIndString(alertMessage, kAlertStrings, 7);
			gPartUtilities->Alert(alertMessage);
		}
	}

	return gSerialNumberValid;
}

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	// Serial number
#if !NETWORK_RENDERING_VERSION // There's no serial number for the rendernode version
	CheckSerial();
#endif
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;

#if (VERSIONNUMBER >= 0x040000)
	// Network rendering version aren't allowed to work within Carrara. And vice-et-versa
#if !NETWORK_RENDERING_VERSION
	if( gIsRenderingNode )
	{
		// Alert the user
		CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
		TMCDynamicString message;
		gResourceUtilities->GetIndString( message, kAlertStrings, 6);
		gPartUtilities->Alert(message);

		return NULL;		
	}
#else // !NETWORK_RENDERING_VERSION
	// RENDER NODE version
	if( !gIsRenderingNode )
	{
		// Alert the user
		CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
		TMCDynamicString message;
		gResourceUtilities->GetIndString( message, kAlertStrings, 5);
		gPartUtilities->Alert(message);

		return NULL;		
	}
#endif // !NETWORK_RENDERING_VERSION
#endif // (VERSIONNUMBER >= 0x040000)

	if (classId == CLSID_BuildingPrim) res = new BuildingPrim;
	else if (classId == CLSID_HousePrim) res = new BuildingPrim;
#if !NETWORK_RENDERING_VERSION
	else if (classId == CLSID_BuildingModeler) res = new BuildingModeler;
	else if (classId == CLSID_BuildingPanePart) res = new BuildingPanePart;
	else if (classId == CLSID_BuildingDropArea) res = new BuildingDropArea;
	else if (classId == CLSID_BuildingDropCandidate) res = new BuildingDropCandidate;
	else if (classId == CLSID_AssembleRoomPart) res = new AssembleRoomPart;
#endif // !NETWORK_RENDERING_VERSION

	return res;
}


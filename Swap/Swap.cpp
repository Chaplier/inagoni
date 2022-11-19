/****************************************************************************************************

		Swap.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#include "Swap.h"


#include "SwapCommand.h"
#include "SwapPart.h"

// Serial number
#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	if(!gSerialNumberValid)
	{
		const TMCString15 productInfo1("SWC3");
		const int32 modulo1 = 57;
		const TMCString31 fileName1("SwapData.dta");
		const int32 resID = 280;
		const IDType familyID = kRID_SceneCommandFamilyID;
		const IDType classID = 'SWAC';

		TMCClassArray<KeyData> keyDatas;

		KeyData key1(	productInfo1,
						fileName1,
						modulo1,
						resID,
						familyID,
						classID);

		keyDatas.AddElem(key1);

		const TMCString15 productInfo2("ADVP"); // Advance Pack serial number
		const TMCString31 fileName2("InagoniData.dta"); // Advance Pack file
		const int32 modulo2 = 59; // Advance Pack modulo

		KeyData key2(	productInfo2,
						fileName2,
						modulo2,
						resID,
						familyID,
						classID);

		keyDatas.AddElem(key2);

		SerialNumber serial(keyDatas);
		gSerialNumberValid = serial.CheckSerial();
	}
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;

	if(!gSerialNumberValid)
		return res; 

	if (classId == CLSID_SwapCommand) res = new SwapCommand;
	else if (classId == CLSID_SwapPart) res = new SwapPart;

	return res;
}


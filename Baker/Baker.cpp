/****************************************************************************************************

		Baker.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/30/2004

****************************************************************************************************/

#include "Baker.h"

#include "BakingCommand.h"
#include "BakingCommandPrefs.h"
#include "FinalNormalMapShader.h"
#include "BakerPart.h"
#include "BakerShaderPart.h"

// Serial number
#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;

boolean IsSerialValid(){return gSerialNumberValid;}

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	if(!gSerialNumberValid)
	{
		const TMCString15 productInfo1("BAC3");
		const TMCString31 fileName1("BakerData.dta");
		const int32 modulo1 = 53;
		const int32 resID = 850;
		const IDType familyID = kRID_SceneCommandFamilyID;
		const IDType classID = 'Baki';

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

//	The check is done later
//	if(!gSerialNumberValid)
//		return res; 

	if (classId == CLSID_BakingCommand) res = new BakingCommand;
	else if (classId == CLSID_BakingCommandPrefs) res = new BakingCommandPrefs;
	else if (classId == CLSID_FinalNormalMapShader) res = new FinalNormalMapShader;
	else if (classId == CLSID_BakerPart) res = new BakerPart;
	else if (classId == CLSID_BakerShaderPart) res = new BakerShaderPart;

	return res;
}
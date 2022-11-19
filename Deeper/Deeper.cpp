/****************************************************************************************************

		NormalMap.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "Deeper.h"

#include "IShComponent.h"
#include "BasicShader.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "NormalMapShader.h"
#include "NormalMapRenderer.h"

// Serial number
#include "Common/SerialNumber.h"

static boolean gSerialNumberValid = false;

boolean IsSerialValid() {return gSerialNumberValid;}

boolean CheckSerial()
{
	if(!gSerialNumberValid)
	{
		const TMCString15 productInfo1("NOC3");
		const int32 modulo1 = 19;
		const TMCString31 fileName1("NormalMapData.dta");
		const int32 resID = 250;
		const IDType familyID = kRID_FinalRendererFamilyID;
		const IDType classID = 'NMRn';

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
	return gSerialNumberValid;
}

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	// CheckSerial can't be called here because of a bug with shaders on Mac
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_NormalMapShader)
	{	// Can't check the serial number here: check in the shader itself
		res = new NormalMapShader;
	}
	else if (classId == CLSID_NormalMapRenderer)
	{
		if(CheckSerial())
			res = new NormalMapRenderer;

	}

	return res;
}

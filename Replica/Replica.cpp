/****************************************************************************************************

		Instanciator.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "Replica.h"


#include "ReplicateCommand.h"
#include "TypePart.h"
#include "DisturbPart.h"
#include "ArrayModifier.h"

// Serial number
#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;

boolean IsSerialValid()
{
	return gSerialNumberValid;
}

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	if(!gSerialNumberValid)
	{
		const TMCString15 productInfo1("BAC3");
		const int32 modulo1 = 23;
		const TMCString31 fileName1("InstanciatorData.dta");
		const int32 resID = 750;
		const IDType familyID = kRID_SceneCommandFamilyID;
		const IDType classID = 'RepC';

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

		SerialNumber  serial(keyDatas);
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

	if (classId == CLSID_ReplicateCommand) res = new ReplicateCommand;
	else if (classId == CLSID_ReplicateData) res = new ReplicateSaveData;
	else if (classId == CLSID_TypePart) res = new TypePart;
	else if (classId == CLSID_DisturbPart) res = new DisturbPart;
	else if (classId == CLSID_ArrayModifier) res = new ArrayModifier;

	return res;
}


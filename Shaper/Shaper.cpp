/****************************************************************************************************

		Shaper.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "Shaper.h"

#include "FreeFormModifierNxN.h"
#include "FreeFormModifier2x2.h"
#include "FreeFormModifier3x3.h"
#include "FreeFormModifier4x4.h"
#include "FreeFormPartExt.h"

#include "MiscComUtilsImpl.h"
#include "IShPartUtilities.h"

// Serial number
#include "Common/SerialNumber.h"

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here

	if(!SerialNumber::IsSerialValid())
	{
		const TMCString15 productInfo1("SHAP");
		const int32 modulo1 = 77;
		const TMCString31 fileName1("ShaperData.dta");

		const int32 resID = 800; // dialog resource ID
		const IDType familyID = kRID_ModifierFamilyID;
		const IDType classID = 'FFDf';

		TMCClassArray<KeyData> keyDatas;

		KeyData key1(	productInfo1,
						fileName1,
						modulo1,
						resID,
						familyID,
						classID);
//SerialNumber::CreateKeyList( key1, 1000 );

		keyDatas.AddElem(key1);

		SerialNumber serial(keyDatas);
		
		if(!serial.CheckSerial())
		{
			// Tell the user it's a demo version
			CWhileInCompResFile myRes(kRID_ModifierFamilyID, 'FFDf');
			TMCDynamicString alertMessage;
			gResourceUtilities->GetIndString(alertMessage, kStrings, 2);
			gPartUtilities->Alert(alertMessage);
		}
	}
}

void Extension3DCleanup()
{
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	if (classId == CLSID_FreeFormModifierNxN) return new FreeFormModifierNxN;
	if (classId == CLSID_FreeFormModifier2x2) return new FreeFormModifier2x2;
	if (classId == CLSID_FreeFormModifier3x3) return new FreeFormModifier3x3;
	if (classId == CLSID_FreeFormModifier4x4) return new FreeFormModifier4x4;

	if (classId == CLSID_FreeFormPartExt) return new FreeFormPartExt;

	return NULL;
}


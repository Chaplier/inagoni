/****************************************************************************************************

		ExtraShaders.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "Veloute.h"

#include "IShComponent.h"
#include "BasicShader.h"

#include "TilingShader1.h"
#include "TilingShader2.h"
#include "TilingShader3.h"
#include "TilingShader4.h"
#include "RoofShader1.h"
#include "BumpNoise2DShader.h"
#include "BumpNoise3DShader.h"
#include "FilterShader.h"
#include "PerturbationShader.h"
#include "Gradient2DShader.h"
#include "Gradient3DShader.h"
#include "PartExtension.h"
#include "COMUtilities.h"
#include "IShSMP.h"


#include "GridShader1.h"
#include "GridShader2.h"
#include "GridShader3.h"
#include "GridShader4.h"
#include "WeaveShader1.h"
#include "RandomLines2DShader.h"
#include "RandomLines3DShader.h"



#include "MiscComUtilsImpl.h"
#include "IShPartUtilities.h"

#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;

boolean gSerialWasAsked = false;

boolean gLocked = false;     

IShAtomicCounter* gCounter = NULL;
const int32 gShadeLimit = 800000;

boolean IsLocked()
{
	// If there's a valid serial number, no lock
	if(gSerialNumberValid)
		return false;

	// If we didn't ask the number to the user, don't lock yet the rendering
	if(!gSerialWasAsked)
		return false;

	// Else allow "gShadeLimit" pixel renders for the demo version
	if (gLocked)
		return true;

	if(!gCounter)
	{
		// Create an atomic counter and authorised gShadeLimit renders
		gShellSMPUtilities->CreateAtomicCounter(&gCounter);
		gCounter->SetValue(0);
	}

	gCounter->Increment();
	gLocked = (gCounter->GetValue() > gShadeLimit);
	return false;
}

boolean CheckSerial()
{
	if(!gSerialNumberValid)
	{
		if(gSerialWasAsked)
			return false; // we already ask: the user is in Demo mode

		CWhileInCompResFile myRes(kRID_ShaderFamilyID, 'Til1');


		const TMCString15 productInfo1("V2C4");
		const int32 modulo1 = 73;

		//const TMCString15 productInfo1("VEC3");
		//const int32 modulo1 = 31;
		const TMCString31 fileName1("ExtraData.dta");
		const int32 resID = 250;
		const IDType familyID = kRID_ShaderFamilyID;
		const IDType classID = 'Til1';

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

		if(!gSerialNumberValid)
		{
			// Tell the user it's a demo version
			TMCDynamicString alertMessage;
			gResourceUtilities->GetIndString(alertMessage, 200, 49);
			gPartUtilities->Alert(alertMessage);
		}

		gSerialWasAsked = true;
	}

	return gSerialNumberValid;
}

void Extension3DInit(IMCUnknown* utilities)
{
	// Perform your dll initialization here
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_TilingShader1) res = new TilingShader1;
	else if (classId == CLSID_GlobalTilingShader1) res = new GlobalTilingShader1;
	
	else if (classId == CLSID_TilingShader2) res = new TilingShader2;
	else if (classId == CLSID_GlobalTilingShader2) res = new GlobalTilingShader2;
	
	else if (classId == CLSID_TilingShader3) res = new TilingShader3;
	else if (classId == CLSID_GlobalTilingShader3) res = new GlobalTilingShader3;
	
	else if (classId == CLSID_TilingShader4) res = new TilingShader4;
	else if (classId == CLSID_GlobalTilingShader4) res = new GlobalTilingShader4;
	
	else if (classId == CLSID_RoofShader1) res = new RoofShader1;
	else if (classId == CLSID_GlobalRoofShader1) res = new GlobalRoofShader1;
	
	else if (classId == CLSID_BumpNoise2DShader) res = new BumpNoise2DShader;
	else if (classId == CLSID_BumpNoise3DShader) res = new BumpNoise3DShader;
	else if (classId == CLSID_FilterShader) res = new FilterShader;
	else if (classId == CLSID_PerturbationShader) res = new PerturbationShader;
	else if (classId == CLSID_Gradient2DShader) res = new Gradient2DShader;
	else if (classId == CLSID_Gradient3DShader) res = new Gradient3DShader;

	// Part Extension
	else if (classId == CLSID_TParamChooserPart1) res = new TParamChooserPart1;
	else if (classId == CLSID_TParamChooserPart2) res = new TParamChooserPart2;
	else if (classId == CLSID_TParamChooserPart3) res = new TParamChooserPart3;
	
	else if (classId == CLSID_GridShader1) res = new GridShader1;
	else if (classId == CLSID_GlobalGridShader1) res = new GlobalGridShader1;

	else if (classId == CLSID_GridShader2) res = new GridShader2;
	else if (classId == CLSID_GlobalGridShader2) res = new GlobalGridShader2;

	else if (classId == CLSID_GridShader3) res = new GridShader3;
	else if (classId == CLSID_GlobalGridShader3) res = new GlobalGridShader3;

	else if (classId == CLSID_GridShader4) res = new GridShader4;
	else if (classId == CLSID_GlobalGridShader4) res = new GlobalGridShader4;

	else if (classId == CLSID_WeaveShader1) res = new WeaveShader1;
	else if (classId == CLSID_GlobalWeaveShader1) res = new GlobalWeaveShader1;

	else if (classId == CLSID_RandomLines2DShader) res = new RandomLines2DShader;
	else if (classId == CLSID_RandomLines3DShader) res = new RandomLines3DShader;

	// Part Extension
	else if (classId == CLSID_TParamChooserPart4) res = new TParamChooserPart4;

	return res;
}


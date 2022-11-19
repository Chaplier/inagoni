/****************************************************************************************************

		Gas.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/20/2004

****************************************************************************************************/

#include "Fire.h"
#include "Smog.h"
#include "Clouds.h"
#include "RisingSmoke.h"
#include "Modifier.h"
#include "GasCommonPart.h"
#include "PopupParts.h"
//#include "AssembleRoomPart.h"

// Serial number
#include "Common/SerialNumber.h"

boolean gSerialNumberValid = false;
boolean gSerialWasAsked = false;
boolean gLocked = false;     

#include "IShSMP.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#endif

TMCCountedPtr<IShAtomicCounter> gCounter;
const int32 gRayLimit = 5000000;

boolean IsDemoVersion()
{
	if(gSerialNumberValid)
		return false;

	if(!gSerialWasAsked)
		return false;

	return true;
}

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
	gLocked = (gCounter->GetValue() > gRayLimit);
	return false;
}

boolean CheckSerial()
{
	// Check the serial number
	if(!gSerialNumberValid)
	{
		if(gSerialWasAsked)
			return false; // we already ask: the user is in Demo mode

		const TMCString15 productInfo1("GAC3");
		const int32 modulo1 = 29;
		const TMCString31 fileName1("GasData.dta");
		const int32 resID = 350;
		const IDType familyID = kRID_GeometricPrimitiveFamilyID;
		const IDType classID = 'Fire';

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
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');
			TMCDynamicString alertMessage;
			gResourceUtilities->GetIndString(alertMessage, kStrings, 11);
			gPartUtilities->Alert(alertMessage);
		}

		gSerialWasAsked = true;
	}

	return gSerialNumberValid;
}

// Gradient presets
GradientPreset* gGradientPresets = NULL;

void Extension3DInit(IMCUnknown* utilities)
{
	CheckSerial();

	// Gradient presets
	if(!gGradientPresets)
	{
		gGradientPresets = new GradientPreset;
	}
}

void Extension3DCleanup()
{
	if(gGradientPresets)
	{
		delete gGradientPresets;
		gGradientPresets = NULL;
	}
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;

	if (classId == CLSID_Fire)				res = new Fire;
	else if (classId == CLSID_Smog)			res = new Smog;
	else if (classId == CLSID_RisingSmoke)	res = new RisingSmoke;
	else if (classId == CLSID_Clouds)		res = new Clouds;

	else if (classId == CLSID_Taper)		res = new TaperModifier;
	else if (classId == CLSID_Wave)			res = new WaveModifier;
	else if (classId == CLSID_Bulge)		res = new BulgeModifier;
	else if (classId == CLSID_Punch)		res = new PunchModifier;
	else if (classId == CLSID_Twist)		res = new TwistModifier;
	else if (classId == CLSID_Bend)			res = new BendModifier;
	else if (classId == CLSID_Shift)		res = new ShiftModifier;
	else if (classId == CLSID_Noise)		res = new NoiseModifier;
	else if (classId == CLSID_Stretch)		res = new StretchModifier;
	else if (classId == CLSID_Scale)		res = new ScaleModifier;

//	else if (classId == CLSID_AssembleRoomPart) res = new AssembleRoomPart;
	else if (classId == CLSID_GasCommonPart) res = new GasCommonPart;
	else if (classId == CLSID_ShaderPopupPart) res = new ShaderPopupPart;
	else if (classId == CLSID_ShaderChooserPart) res = new ShaderChooserPart;
	else if (classId == CLSID_MeshPopupPart) res = new MeshPopupPart;
	else if (classId == CLSID_MeshChooserPart) res = new MeshChooserPart;

	return res;
}


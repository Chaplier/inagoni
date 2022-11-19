/****************************************************************************************************

		BakerShaderPart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/21/2005

****************************************************************************************************/

#include "BakerShaderPart.h"

#include "BakerDef.h"
#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "IMFTextPopupPart.h"
#include "MCClassArray.h"
#include "ShaderTypes.h"
#include "MiscComUtilsImpl.h"
#include "InterfaceIDs.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "ISceneSelection.h"
#include "I3DShTreeElement.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_BakerShaderPart(R_CLSID_BakerShaderPart);
#else
const MCGUID CLSID_BakerShaderPart = {R_CLSID_BakerShaderPart};
#endif

static const int32 kCellHeight = 25;

// BakerShaderPart class

BakerShaderPart::BakerShaderPart()
{
}

void BakerShaderPart::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr BakerShaderPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BakerShaderPart))
	{
		TMCCountedGetHelper<BakerShaderPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

boolean	BakerShaderPart::Receive(int32 message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

	switch (message)
	{
	case 16: // I don't know this message, but it's a way to prepare the part
		{
			UpdateDisplay(true, true);
		} break;
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();

					int32 size = 0;

					switch (partID)
					{
					case 'XNeg':
						{
							boolean on = false;
							part->GetValue(&on, kBooleanValueType);
							SetFlag( on, eNegX );
						} break;
					case 'YNeg':
						{
							boolean on = false;
							part->GetValue(&on, kBooleanValueType);
							SetFlag( on, eNegY );
						} break;
					case 'ZNeg':
						{
							boolean on = false;
							part->GetValue(&on, kBooleanValueType);
							SetFlag( on, eNegZ );
						} break;
					case 'Rela':
						{
							boolean on = false;
							part->GetValue(&on, kBooleanValueType);
							SetFlag( on, eIsRelative );
						} break;
					case 'XCol':
					case 'YCol':
					case 'ZCol':
						{	// color
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);
							SetColorOnAxis(partID, option);
							UpdateDisplay();
						} break;
					case 'Pres':
						{	// Presets
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);
							IMFPart* typePart = GetTypePart();
							switch(option)
							{
							case 'Typ1':
								{	// Preset 1: Baker map
									typePart->SetValue((void**)&kBakerPreset, kInt32ValueType, false, false);
								} break;
							case 'Typ2':
								{	// Preset 2: ZBrush map
									typePart->SetValue((void**)&kZBrush1Preset, kInt32ValueType, false, false);
								} break;
							case 'Typ3':
								{	// Preset 3: other ZBrush map
									typePart->SetValue((void**)&kZBrush2Preset, kInt32ValueType, false, false);
								} break;
							case 'Typ4':
								{	// Preset 4: Baker relative map
									typePart->SetValue((void**)&kBakerRelativePreset, kInt32ValueType, false, false);
								} break;
							}
							UpdateDisplay();
						} break;
					default: break;
					}
				}
			}
		}
	}

	return handledMessage;
}

void BakerShaderPart::SetFlag(const boolean on, const ENormalFlags flag)
{
	IMFPart* typePart = GetTypePart();
	if(MCVerify(typePart))
	{
		int32 curFlags=0;
		typePart->GetValue(&curFlags, kInt32ValueType);
		if(on)	curFlags|=flag;
		else	curFlags&=~flag;
		typePart->SetValue((void**)&curFlags, kInt32ValueType,false, false);
	}
}

void SetColor(int32& curFlags, const ENormalFlags set)
{
	switch(set)
	{
		case eXRed:
		{
			if(curFlags&eYRed)// Need to remove the conflicts
			{
				curFlags&=~eYRed;
				if(curFlags&eXGreen) curFlags|=eYGreen;
				else curFlags|=eYBlue;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eXGreen;
			curFlags&=~eXBlue;
		} break;
		case eXGreen:
		{
			if(curFlags&eYGreen)// Need to remove the conflicts
			{
				curFlags&=~eYGreen;
				if(curFlags&eXRed) curFlags|=eYRed;
				else curFlags|=eYBlue;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eXRed;
			curFlags&=~eXBlue;
		} break;
		case eXBlue:
		{
			if(curFlags&eYBlue)// Need to remove the conflicts
			{
				curFlags&=~eYBlue;
				if(curFlags&eXRed) curFlags|=eYRed;
				else curFlags|=eYGreen;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eXRed;
			curFlags&=~eXGreen;
		} break;
		case eYRed:
		{
			if(curFlags&eXRed)// Need to remove the conflicts
			{
				curFlags&=~eXRed;
				if(curFlags&eYGreen) curFlags|=eXGreen;
				else curFlags|=eXBlue;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eYGreen;
			curFlags&=~eYBlue;
		} break;
		case eYGreen:
		{
			if(curFlags&eXGreen)// Need to remove the conflicts
			{
				curFlags&=~eXGreen;
				if(curFlags&eYRed) curFlags|=eXRed;
				else curFlags|=eXBlue;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eYRed;
			curFlags&=~eYBlue;
		} break;
		case eYBlue:
		{
			if(curFlags&eXBlue)// Need to remove the conflicts
			{
				curFlags&=~eXBlue;
				if(curFlags&eYRed) curFlags|=eXRed;
				else curFlags|=eXGreen;
			}
			// Set the new one
			curFlags|=set;
			// Clean the previous color
			curFlags&=~eYRed;
			curFlags&=~eYGreen;
		} break;
	}

}

void BakerShaderPart::SetColorOnAxis(const int32 onAxis, const int32 colorID)
{
	// Need to modify the other axis color in function
	IMFPart* typePart = GetTypePart();
	if(MCVerify(typePart))
	{
		int32 curFlags=0;
		typePart->GetValue(&curFlags, kInt32ValueType);

		switch(onAxis)
		{
		case 'XCol':
			{
				if(colorID == kRedID)			SetColor(curFlags, eXRed);
				else if(colorID == kGreenID)	SetColor(curFlags, eXGreen);
				else							SetColor(curFlags, eXBlue);
			} break;
		case 'YCol':
			{
				if(colorID == kRedID)			SetColor(curFlags, eYRed);
				else if(colorID == kGreenID)	SetColor(curFlags, eYGreen);
				else							SetColor(curFlags, eYBlue);
			} break;
		case 'ZCol':
			{
				const int32 curZColor = GetZColor(curFlags);
				if(colorID == kRedID)
				{
					if(curZColor == kGreenID)
					{
						if(curFlags&eXRed)
							SetColor(curFlags, eXGreen);
						else if(curFlags&eYRed)
							SetColor(curFlags, eYGreen);
					}
					else if(curZColor == kBlueID)
					{
						if(curFlags&eXRed)
							SetColor(curFlags, eXBlue);
						else if(curFlags&eYRed)
							SetColor(curFlags, eYBlue);
					}
				}
				else if(colorID == kGreenID)
				{
					if(curZColor == kRedID)
					{
						if(curFlags&eXGreen)
							SetColor(curFlags, eXRed);
						else if(curFlags&eYGreen)
							SetColor(curFlags, eYRed);
					}
					else if(curZColor == kBlueID)
					{
						if(curFlags&eXGreen)
							SetColor(curFlags, eXBlue);
						else if(curFlags&eYGreen)
							SetColor(curFlags, eYBlue);
					}
				}
				else
				{
					if(curZColor == kRedID)
					{
						if(curFlags&eXBlue)
							SetColor(curFlags, eXRed);
						else if(curFlags&eYBlue)
							SetColor(curFlags, eYRed);
					}
					else if(curZColor == kGreenID)
					{
						if(curFlags&eXBlue)
							SetColor(curFlags, eXGreen);
						else if(curFlags&eYBlue)
							SetColor(curFlags, eYGreen);
					}
				}
			} break;
		}

		typePart->SetValue((void**)&curFlags, kInt32ValueType, false, false);
	}
}

void BakerShaderPart::UpdateDisplay(const boolean invalidate, const boolean notify)
{
	IMFPart* typePart = GetTypePart();
	if(MCVerify(typePart))
	{
		IMFPart* thisPart = GetThisPartNoAddRef();

		TMCCountedPtr<IMFPart> preset;
		thisPart->FindChildPartByID(&preset,'Pres');
		const int32 hide = 'Hide';
		preset->SetValue((void**)&hide, kInt32ValueType, invalidate, notify);

		int32 flags=0;
		typePart->GetValue(&flags, kInt32ValueType);

		TMCCountedPtr<IMFPart> XNegPart;
		thisPart->FindChildPartByID(&XNegPart,'XNeg');
		TMCCountedPtr<IMFPart> YNegPart;
		thisPart->FindChildPartByID(&YNegPart,'YNeg');
		TMCCountedPtr<IMFPart> ZNegPart;
		thisPart->FindChildPartByID(&ZNegPart,'ZNeg');
		TMCCountedPtr<IMFPart> relativePart;
		thisPart->FindChildPartByID(&relativePart,'Rela');

		const boolean trueValue = true;
		const boolean falseValue = false;

		if(flags&eNegX) XNegPart->SetValue((void**)&trueValue, kBooleanValueType, invalidate, notify);
		else  XNegPart->SetValue((void**)&falseValue, kBooleanValueType, invalidate, notify);
	
		if(flags&eNegY) YNegPart->SetValue((void**)&trueValue, kBooleanValueType, invalidate, notify);
		else  YNegPart->SetValue((void**)&falseValue, kBooleanValueType, invalidate, notify);
	
		if(flags&eNegZ) ZNegPart->SetValue((void**)&trueValue, kBooleanValueType, invalidate, notify);
		else  ZNegPart->SetValue((void**)&falseValue, kBooleanValueType, invalidate, notify);

		if(flags&eIsRelative) relativePart->SetValue((void**)&trueValue, kBooleanValueType, invalidate, notify);
		else  relativePart->SetValue((void**)&falseValue, kBooleanValueType, invalidate, notify);
	
		// X axis color
		TMCCountedPtr<IMFPart> XColorPart;
		fThisPart->FindChildPartByID(&XColorPart,'XCol');
		if(flags&eXRed)			XColorPart->SetValue((void**)&kRedID, kInt32ValueType, invalidate, notify);
		else if(flags&eXGreen)	XColorPart->SetValue((void**)&kGreenID, kInt32ValueType, invalidate, notify);
		else					XColorPart->SetValue((void**)&kBlueID, kInt32ValueType, invalidate, notify);
	
		// Y axis color
		TMCCountedPtr<IMFPart> YColorPart;
		fThisPart->FindChildPartByID(&YColorPart,'YCol');
		if(flags&eYRed)			YColorPart->SetValue((void**)&kRedID, kInt32ValueType, invalidate, notify);
		else if(flags&eYGreen)	YColorPart->SetValue((void**)&kGreenID, kInt32ValueType, invalidate, notify);
		else					YColorPart->SetValue((void**)&kBlueID, kInt32ValueType, invalidate, notify);

		// Z axis color
		TMCCountedPtr<IMFPart> ZColorPart;
		fThisPart->FindChildPartByID(&ZColorPart,'ZCol');
		const int32 ZColor = GetZColor(flags);
		ZColorPart->SetValue((void**)&ZColor, kInt32ValueType, invalidate, notify);
	}
}
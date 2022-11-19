/****************************************************************************************************

		Smog.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#include "Smog.h"

#include "copyright.h"
#include "MiscComUtilsImpl.h"
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Smog(R_CLSID_Smog);
#else
const MCGUID CLSID_Smog={R_CLSID_Smog};
#endif

Smog::Smog()
{
	fPMap.fCustomNoiseIndex = 4; // noise
	GetNoiseName(fPMap.fCustomNoiseIndex, fPMap.fMasterShaderName);
}

void Smog::CustomPreProcess()
{
}

// Return a positive value
real32 Smog::GetLocalDensity(const TVector3& point)
{
	TVector3 deformedPoint = point;
	ApplyModifiers(deformedPoint);

	const real32 rampOff = RampOff(deformedPoint, fPMap.fRampOffFlag, fPMap.fIntensityRamps, true);
	if(rampOff<kRealEpsilon)
		return 0;

	// First scale the point
	// Use transform vector: the translation is done after
	TVector3 scaledPoint =  fTransform.TransformVector(deformedPoint);

	// Swirl
	const int32 swirlCount = fSwirlArray.GetElemCount();
	for(int32 iSwirl=swirlCount-1 ; iSwirl>=0 ; iSwirl--)
	{
		fSwirlArray[iSwirl].SwirlPoint(scaledPoint, (.5*point.z+.5)/*cos(point.z*PI)*/);
	}

	// Offset
	scaledPoint.x -= fPMap.fXOffset;
	scaledPoint.y -= fPMap.fYOffset;
	scaledPoint.z -= fPMap.fZOffset;

	return (GetGasValue(scaledPoint)*rampOff);
}

MCCOMErr Smog::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID = part->GetIMFPartID();

					switch (partID)
					{
					case 'SmoP':
						{	// Smog preset: set the color gradient
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);

							const int32 currentKeyCount = fPMap.fColorGradient.GetColorKeyCount();
							switch(popupItem)
							{
							case 'Opt0': { gGradientPresets->GetElem(GradientPreset::eGSmog1, fPMap.fColorGradient );} break;
							case 'Opt1': { gGradientPresets->GetElem(GradientPreset::eGSmog2, fPMap.fColorGradient );} break;
							case 'Opt2': { gGradientPresets->GetElem(GradientPreset::eGSmog3, fPMap.fColorGradient );} break;
							case 'Opt3': { gGradientPresets->GetElem(GradientPreset::eGSmog4, fPMap.fColorGradient );} break;
							case 'Opt4': { gGradientPresets->GetElem(GradientPreset::eGSmog5, fPMap.fColorGradient );} break;
							case 'Opt5': { gGradientPresets->GetElem(GradientPreset::eGWhite, fPMap.fColorGradient );} break;
							}

							// Unselect the button
							popupItem = 'NoPt';
							part->SetValue(&popupItem, kInt32ValueType, false, false);
						} break;
					}
				}
			}
		}
	}

	return GasBase::SimpleHandleEvent( message, source, data);
}

/****************************************************************************************************

		Fire.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#include "Fire.h"

#include "copyright.h"
#include "Utils.h"
#include "MiscComUtilsImpl.h"
#include "IMFPart.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Fire(R_CLSID_Fire);
#else
const MCGUID CLSID_Fire={R_CLSID_Fire};
#endif

Fire::Fire()
{
	fPMap.fCustomNoiseIndex = 3; // Ridged
	GetNoiseName(fPMap.fCustomNoiseIndex, fPMap.fMasterShaderName);

	fRisingAcc = 1;

	// Fire Colors
	gGradientPresets->GetElem(GradientPreset::eGFire1, fPMap.fColorGradient );
	
	// Default param
	fPMap.fQuality = .4f;
	fPMap.fSelfIntensity = 1.5f;
}

void Fire::CustomPreProcess()
{
}


// Return a positive value
real32 Fire::GetLocalDensity(const TVector3& point)
{
	TVector3 deformedPoint = point;
	ApplyModifiers(deformedPoint);

	// First scale the point
//	TVector3 scaledPoint =  point % fScaling;
	// Use transform vector: the translation is done after
	TVector3 scaledPoint =  fTransform.TransformVector(deformedPoint);

	// Swirl expansion
//	const real32 halfHeight = 10000/(fPMap.fGlobalScale*fPMap.fZScale); // we change our space from -1/1 to a scaled one
//	const real32 dist = (halfHeight + scaledPoint.z)/(2*halfHeight);
//	const real32 expandFactor = Expand(scaledPoint, dist);

	// Swirl
	const int32 swirlCount = fSwirlArray.GetElemCount();
	for(int32 iSwirl=swirlCount-1 ; iSwirl>=0 ; iSwirl--)
	{
		fSwirlArray[iSwirl].SwirlPoint(scaledPoint, (.5*point.z+.5)/*cos(point.z*PI)*/);
	}

	// RampOff after the swirling to keep the full deformation
	TVector3 unScaledPoint = scaledPoint;
	// Unexpand
//	unScaledPoint /= expandFactor;
	const real32 rampOff = RampOff(fInvertTransform.TransformVector(unScaledPoint), fPMap.fRampOffFlag, fPMap.fIntensityRamps, true);
	if(rampOff<kRealEpsilon)
		return 0;

	// height acceleration:
	const real32 zDist = 1 + point.z;
	const real32 factor = RealExp(-.1 * fRisingAcc * zDist); // Exp(0)=1, Exp(1)=2.7 Exp(2)=7
	scaledPoint.z *= (factor*factor);

	// Offset
	const TVector3 offset(fPMap.fXOffset,fPMap.fYOffset,fPMap.fZOffset);
//	const real32 turbScale = (1 + .5*fNoise.GetValueLinear(point - offset));
//	scaledPoint *= turbScale;
	scaledPoint -= offset;

	real32 value = GetGasValue(scaledPoint);

	// zDist goes from 0 to 2
//	const real32 coef = (1-Bias(1-.99*value,.5*zDist));
	value = MC_Clamp( (real32)(value - .5*zDist), kRealZero, kRealOne );
//	value *= coef;
// Need to be clamped	value = .85*value + .8*(.5 - 2*zDist);

	return (value*rampOff);
}
/*
real32 Fire::Expand(TVector3& point, const real32 dist) const
{ // Does not follow the same law than the ramp off: a sqrt is added to slow down the motion
	const real32 radius = fRisingDia + dist*(1-fRisingDia);	

	const real32 scaling = RealSqrt(1/radius);//RealSqrt(fPMap.fRisingDia/radius);// RealSqrt to slow down the expansion

	point*=scaling; 

	return scaling;
}
*/
MCCOMErr Fire::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
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
					case 'FirP':
						{	// Fire preset: set the color gradient
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);

							const int32 currentKeyCount = fPMap.fColorGradient.GetColorKeyCount();
							switch(popupItem)
							{
							case 'Opt0': { gGradientPresets->GetElem(GradientPreset::eGFire1, fPMap.fColorGradient );} break;
							case 'Opt1': { gGradientPresets->GetElem(GradientPreset::eGFire2, fPMap.fColorGradient );} break;
							case 'Opt2': { gGradientPresets->GetElem(GradientPreset::eGFire3, fPMap.fColorGradient );} break;
							case 'Opt3': { gGradientPresets->GetElem(GradientPreset::eGFire4, fPMap.fColorGradient );} break;
							case 'Opt4': { gGradientPresets->GetElem(GradientPreset::eGFire5, fPMap.fColorGradient );} break;
							case 'Opt5': { gGradientPresets->GetElem(GradientPreset::eGFire6, fPMap.fColorGradient );} break;
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


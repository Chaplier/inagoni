/****************************************************************************************************

		Clouds.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/15/2004

****************************************************************************************************/

#include "Clouds.h"

#include "copyright.h"
#include "IMFResponder.h"
#include "MiscComUtilsImpl.h"

#include "I3DShObject.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Clouds(R_CLSID_Clouds);
#else
const MCGUID CLSID_Clouds={R_CLSID_Clouds};
#endif

const TVector3 kOffSet((real32)SQRT2,(real32)EXP,(real32)PI2);

Clouds::Clouds()
{
	fPMap.fCustomNoiseIndex = 1; // Noise
	GetNoiseName(fPMap.fCustomNoiseIndex, fPMap.fMasterShaderName);

	mCloudsPMap.fSphereCount = 1;
	mCloudsPMap.fSphereSeed = 131313;
	mCloudsPMap.fMixType = eOverlayMix;
	mCloudsPMap.fBlenderDist = .7f;
	mCloudsPMap.fSphereCoeff = .5f;
	mCloudsPMap.fCoreDensity = 1;

	// Cloud Colors
	gGradientPresets->GetElem(GradientPreset::eGWhite, fPMap.fColorGradient);

	fR2 = 0;
	fNoiseCoeff = 0;
	fMulti1 = 0;
	fMulti2 = 0;
	fMulti3 = 0;
	fOverlay1 = 0;
	fOverlay2 = 0;

	fSphereNoiseInvalid = true;

	// Default settings to have something nice
	SetDensityPreset('Opt1'); // Sphere
	fPMap.fSelfIntensity = 1.5f;
	fPMap.fSelfShadowType = eDistantSelfShadow;
//	fPMap.fShadowIntensity = .5f;
}

void Clouds::CustomPreProcess()
{	
	// Perlin noise
	if(fSphereNoiseInvalid)
	{
		fSphereNoise.SetSeed(mCloudsPMap.fSphereSeed);
		fSphereNoise.InitGradientTab();

		fSphereNoiseInvalid = false;
	}

	fNoiseCoeff = 1-mCloudsPMap.fSphereCoeff;
	fMulti1 = 1 + mCloudsPMap.fSphereCoeff*(-3+2*mCloudsPMap.fSphereCoeff);
	fMulti2 = mCloudsPMap.fSphereCoeff*(-1+2*mCloudsPMap.fSphereCoeff);
	fMulti3 = mCloudsPMap.fSphereCoeff*(4-4*mCloudsPMap.fSphereCoeff);
	fOverlay1 = 1 + .5*(mCloudsPMap.fSphereCoeff-fNoiseCoeff);
	fOverlay2 = 1 - .5*(mCloudsPMap.fSphereCoeff-fNoiseCoeff);

	// Create the spheres
	fSphereArray.SetElemCount(mCloudsPMap.fSphereCount);

	// Init their values
	// Medium radius approximation
	real32 maxRadius = 1;
	for(int32 i=1 ; i<mCloudsPMap.fSphereCount ; i++)
	{
		const real32 val = (i+1);
		maxRadius -= 1.0/(val*val);
	}

	// Get the space we have
	const real32 maxOffset = 1-maxRadius;

	// blending size has the same size on all spheres, whatever their radius is
	const real32 blendingSize = mCloudsPMap.fBlenderDist*maxRadius;
	fR2 = blendingSize*blendingSize;

	for(int32 iSphere=0 ; iSphere<mCloudsPMap.fSphereCount ; iSphere++)
	{
		// Get a random direction
		TVector3 direction;
		direction.x = fSphereNoise.GetValueLinear((iSphere+.5f)*PI*TVector3::kOnes);
		direction.y = fSphereNoise.GetValueLinear((iSphere+.5f)*TWO_PI*TVector3::kOnes);
		direction.z = fSphereNoise.GetValueLinear((iSphere+.5f)*PI2*TVector3::kOnes);
		direction.Normalize();

		// A random percent
		real32 coeff = PI * (.5f + .5f*fSphereNoise.GetValueLinear((iSphere+.5f)*EXP*TVector3::kOnes) );
		coeff -= floor(coeff);

		fSphereArray[iSphere].fCenter = maxOffset * coeff * direction;

		// Get a random radius between blendingSize and maxRadius
		real32 coeff2 = EXP * (.5f + .5f*fSphereNoise.GetValueLinear((iSphere-.5f)*PI*TVector3::kOnes) );
		coeff2 -= floor(coeff2);
		fSphereArray[iSphere].fRadius = coeff2*(1-mCloudsPMap.fBlenderDist)*maxRadius;
		fSphereArray[iSphere].fBlendDist = blendingSize;
//		fSphereArray[iSphere].fWeight = 1;//mCloudsPMap.fCoreDensity;
	}
}

MCCOMErr Clouds::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (source && source->GetInstanceID() == 'SpSe')
	{
		mCloudsPMap.fSphereSeed = MCRandom();
//		fSphereNoise.SetSeed(mCloudsPMap.fSphereSeed);
//		fSphereNoise.InitGradientTab();
		fSphereNoiseInvalid = true;
//		fInvalid = true;
		return MC_S_OK;
	}
	else
		return GasBase::SimpleHandleEvent( message, source, data);
}

// Return a positive value
real32 Clouds::GetLocalDensity(const TVector3& point)
{
	real32 value = 0;

	TVector3 deformedPoint = point;
	ApplyModifiers(deformedPoint);

	const int32 swirlCount = fSwirlArray.GetElemCount();
	real32 rampOff = 0;
	TVector3 scaledPoint = TVector3::kZero;
	if(swirlCount)
	{
		// First scale the point
		scaledPoint =  fTransform.TransformVector(deformedPoint);

		// Swirl
		for(int32 iSwirl=swirlCount-1 ; iSwirl>=0 ; iSwirl--)
		{
			fSwirlArray[iSwirl].SwirlPoint(scaledPoint, (.5*point.z+.5)/*cos(point.z*PI)*/);
		}
	
		rampOff = RampOff(fInvertTransform.TransformVector(scaledPoint), fPMap.fRampOffFlag, fPMap.fIntensityRamps, true);
		if(rampOff<kRealEpsilon)
			return 0;
	}
	else
	{
		// Do the rampof first, we may avoid a transform for nothing
		rampOff = RampOff(deformedPoint, fPMap.fRampOffFlag, fPMap.fIntensityRamps, true);
		if(rampOff<kRealEpsilon)
			return 0;
		// Then scale the point
		scaledPoint =  fTransform.TransformVector(deformedPoint);
	}

	// Offset
	scaledPoint.x -= fPMap.fXOffset;
	scaledPoint.y -= fPMap.fYOffset;
	scaledPoint.z -= fPMap.fZOffset;

	switch(mCloudsPMap.fMixType)
	{
	case eSumMix:
		{
			const real32 sphereDensity = rampOff*SpheresDensity(deformedPoint);

			const real32 noiseDensity = GetGasValue(scaledPoint);
	
			const real32 blendedValue = mCloudsPMap.fSphereCoeff*sphereDensity + fNoiseCoeff*noiseDensity;

			return (blendedValue);
		} break;
	case eProductMix:
		{
			const real32 sphereDensity = rampOff*SpheresDensity(deformedPoint);
			if(sphereDensity<=kRealEpsilon)
				return 0;
	
			const real32 noiseDensity = GetGasValue(scaledPoint);
	
			if(noiseDensity<=kRealEpsilon)
				return 0;
	
			// do a ponderated product
			const real32 poundedProd = MC_Clamp( noiseDensity*fMulti1 + sphereDensity*fMulti2 + noiseDensity*sphereDensity*fMulti3, kRealZero, kRealOne );
			return (poundedProd);
		} break;
	case eOverlayMix:
		{
			// 0 where no sphere, 1 inside
			const real32 sphereDensity = rampOff*SpheresDensity(deformedPoint);
			if(sphereDensity<=kRealEpsilon)
				return 0;
	
			const real32 noiseDensity = GetGasValue(scaledPoint);
	
			const real32 mixedValue = MC_Clamp( sphereDensity * ( fOverlay1*sphereDensity + fOverlay2*noiseDensity ), kRealZero, kRealOne );
			return (mixedValue);
		} break;
	}

	return 0;
}

// Use the list of sphere to get the sphere density
real32 Clouds::SpheresDensity(const TVector3& pos)
{
	if(UseMesh())
		return 1;

	// Work in negatif so we can use the multiplication
	// 1 outside, 0 in a sphere, 1-Fdensity between
	real32 value=1;

	// Perturb the point to simulate a sphere deformation
	// Transform the point to authorized animation
	const real32 perturbation = .5 * fSphereNoise.GetValueSmooth(pos+fTransform.fTranslation); // a value between-1 and 1
	const int32 sphereCount = fSphereArray.GetElemCount();

	for(int32 iSphere=0 ; iSphere<sphereCount ; iSphere++)
	{
		const TVector3& sphereCenter = fSphereArray[iSphere].fCenter;
		const real32 sphereRadius = fSphereArray[iSphere].fRadius;
		const real32 blendDist = fSphereArray[iSphere].fBlendDist;
		const TVector3 dir = sphereCenter-pos;
		const real32 distToCenter = perturbation + dir.GetMagnitude();
		if(distToCenter<sphereRadius)
		{
			return 1; // the weight is moved out of this method
		//	value *= (1-fSphereArray[iSphere].fWeight);
		}
		else if(distToCenter<sphereRadius+blendDist)
		{
			real32 r2 = distToCenter-sphereRadius;
			r2 *= r2;
			value *= (1-DensityBlender(r2, fR2));
//			value *= (fSphereArray[iSphere].fWeight*(1-DensityBlender(r2, fR2)));
		}

		if(value<kRealEpsilon)
			return 1;
	}

	return mCloudsPMap.fCoreDensity*(1-value);
}


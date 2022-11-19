/****************************************************************************************************

		RisingSmoke.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#include "RisingSmoke.h"

#include "copyright.h"
#include "Utils.h"
#include "MiscComUtilsImpl.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_RisingSmoke(R_CLSID_RisingSmoke);
#else
const MCGUID CLSID_RisingSmoke={R_CLSID_RisingSmoke};
#endif


RisingSmoke::RisingSmoke()
{
	fRisingDia = 0.2f;
	fRisingRamp = 0.5f;

	fPMap.fCustomNoiseIndex = 2; // turbulence
	GetNoiseName(fPMap.fCustomNoiseIndex, fPMap.fMasterShaderName);

	fHalfHeight = 0;

	SetDensityPreset('Opt5'); // height
}

void RisingSmoke::CustomPreProcess()
{
	const real32 globalScaleFactor = 100/fPMap.fGlobalScale;
	fHalfHeight = globalScaleFactor * 100/fPMap.fZScale; // we change our space from -1/1 to a scaled one

	const real32 totalHeight = 2*fHalfHeight;

	// Swirl X
	{
		// Put n swirl between 0 and 2*halfHeight
		// For animation, we need to take into account fZOffset

		const real32 radius = globalScaleFactor * 100/fPMap.fXScale; // radius and swirlCount are kind of linked
		const real32 extraHeight = totalHeight + 2*radius;

		const real32 elemSize = 1.75*radius;

		const int32 swirlCount = extraHeight/elemSize;
		const int32 halfCount = (swirlCount+1)/2;
		fSwirlArray.SetElemCount(swirlCount);
		for(int32 iSwirl=0 ; iSwirl<swirlCount ; iSwirl++)
		{
			real32 dist = iSwirl*elemSize + fPMap.fZOffset;

			const int32 modulo = (int32)(dist/extraHeight);

			dist -= modulo*extraHeight;
			
			TVector3 swirlCenter(0,0,dist);
			fSwirlArray[iSwirl].SetPosition(swirlCenter);
			fSwirlArray[iSwirl].SetAxis(iSwirl&0x00000001?eX:eMinusX);
			fSwirlArray[iSwirl].SetRadius(radius);
		}
	}
	// Swirl Y
	{
		// Put n swirl between 0 and 2*halfHeight
		// For animation, we need to take into account fZOffset

		const real32 radius = globalScaleFactor * 100/fPMap.fYScale; // radius and swirlCount are kind of linked
		const real32 extraHeight = totalHeight + 2*radius;

		const real32 elemSize = 1.75*radius;

		const int32 swirlCount = extraHeight/elemSize;
		const int32 halfCount = (swirlCount+1)/2;
		fSwirlArray.AddElemCount(swirlCount);
		const int32 currentCount = fSwirlArray.GetElemCount();
		int32 iSwirl=0;
		for(int32 index=currentCount ; index<currentCount+swirlCount ; index++, iSwirl++)
		{
			real32 dist = (iSwirl)*elemSize + fPMap.fZOffset;

			const int32 modulo = (int32)(dist/extraHeight);

			dist -= modulo*extraHeight;
			
			TVector3 swirlCenter(0,0,dist);
			fSwirlArray[iSwirl].SetPosition(swirlCenter);
			fSwirlArray[iSwirl].SetAxis(iSwirl&0x00000001?eMinusY:eY);
			fSwirlArray[iSwirl].SetRadius(radius);
		}
	}
}


real32 RisingSmoke::GetLocalDensity(const TVector3& point)
{
	TVector3 deformedPoint = point;
	ApplyModifiers(deformedPoint);

	// Scale the point before getting the noise value
	// The point is originaly in a -1,1 space

	// First scale the point
//	TVector3 scaledPoint =  point % fScaling;
	// Use transform vector: the translation is done after
	TVector3 scaledPoint =  fTransform.TransformVector(deformedPoint);

	// Then add the stretchs and expand
	// For rising smoke
	const real32 dist = (fHalfHeight + scaledPoint.z)/(2*fHalfHeight);
	Stretch(scaledPoint, dist, fHalfHeight);
	const real32 expandFactor = Expand(scaledPoint, dist);

	// Swirl
	const int32 swirlCount = fSwirlArray.GetElemCount();
	for(int32 iSwirl=swirlCount-1 ; iSwirl>=0 ; iSwirl--)
	{
		fSwirlArray[iSwirl].SwirlPoint(scaledPoint, (.5*deformedPoint.z+.5));//cos(point.z*PI)
	}

	// RampOff after the swirling to keep the full deformation
	TVector3 unScaledPoint = scaledPoint;
	// Unexpand
	unScaledPoint /= expandFactor;
	// Unstretch
	unScaledPoint.z /= dist;
	unScaledPoint.z -= fHalfHeight;
	// Unscale
	unScaledPoint = fInvertTransform.TransformVector(unScaledPoint);
	real32 rampOff = RampOff(unScaledPoint, fPMap.fRampOffFlag, fPMap.fIntensityRamps, true);
	if(rampOff<kRealEpsilon)
		return 0;

	{	// to simulate rising smoke: narrow diameter a the base,
		// getting wider with height
		const real32 zDist = .5+.5*deformedPoint.z;
		// diameter is 1 at the top, fPMap.fRisingDiaRamp at the bottom
		const real32 radius = fRisingDia + zDist*zDist*(1-fRisingDia);	
		const real32 distSqr = deformedPoint.x*deformedPoint.x + deformedPoint.y*deformedPoint.y;
		const real32 dist = RealSqrt(distSqr);
	
		if(distSqr<=radius)
		{	// Ramp off
			const real32 rampLimit = radius*(1-fRisingRamp);
			if(distSqr>rampLimit)
			{
				rampOff*=(1-SmoothStep(rampLimit,radius,distSqr));
			}
		}
		else
		{	// out
			return 0;
		}
	}

	// Offset
	scaledPoint.x -= fPMap.fXOffset;
	scaledPoint.y -= fPMap.fYOffset;
	scaledPoint.z -= fPMap.fZOffset;

	return (GetGasValue(scaledPoint)*rampOff);
}

// Strech the height: a lot near the base, less at the to, to sinulate the
// slowing down off the smoke rising
void RisingSmoke::Stretch(TVector3& point, const real32 dist, const real32 halfHeight) const
{
	point.z += halfHeight;
	point.z *= dist;
//	point.z -= halfHeight*dist;
}

// Widden the diameter: do that in the rampOff
// Expand will be used for the global scaling with altitude
// Scaling is 1 at the top
real32 RisingSmoke::Expand(TVector3& point, const real32 dist) const
{ // Does not follow the same law than the ramp off: a sqrt is added to slow down the motion

	// to simulate rising smoke: narrow diameter a the base,
	// getting wider with height
	// diameter is 1 at the top, fPMap.fRisingDiaRamp at the bottom
	const real32 radius = fRisingDia + /*dist*/dist*(1-fRisingDia);	

	const real32 scaling = RealSqrt(1/radius);//RealSqrt(fPMap.fRisingDia/radius);// RealSqrt to slow down the expansion

	point*=scaling; 

	return scaling;
}


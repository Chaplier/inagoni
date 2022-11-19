/****************************************************************************************************

		Clouds.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/15/2004

****************************************************************************************************/

#ifndef __Clouds__
#define __Clouds__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GasBase.h"

extern const MCGUID CLSID_Clouds;

enum EMixType
{
	eNoMix = 0,
	eSumMix = 'Opt2',
	eProductMix = 'Opt3',
	eOverlayMix = 'Opt1',
};

struct Sphere
{
	TVector3 fCenter;
	real32	fRadius;
	real32	fBlendDist;
//	real32	fWeight; // core density
};

struct CloudsPMap
{
	int32 fSphereCount;
	int32 fSphereSeed; // to randomize their position and there radius
	int32 fMixType;
	real32 fBlenderDist; // dist after radius where the blending happens
	real32 fSphereCoeff; // proportion of sphere vs noise
	real32 fCoreDensity;
};

class Clouds : public GasBase
{
public:
	Clouds();

	virtual MCCOMErr MCCOMAPI SimpleHandleEvent			(MessageID message, IMFResponder* source, void* data);
	virtual int32		MCCOMAPI GetParamsBufferSize() const 
	{
		return (sizeof(GasBasePMap) + sizeof(CloudsPMap));
	}

protected:

	virtual void	CustomPreProcess();
	virtual real32	GetLocalDensity(const TVector3& point);

	real32 SpheresDensity(const TVector3& pos);

	// Complete the pmap
	CloudsPMap mCloudsPMap;
	// End pmap

	TNoiseBase	fSphereNoise;
	TMCClassArray<Sphere> fSphereArray;

	boolean fSphereNoiseInvalid;

	// Cached values
	real32 fR2;
	real32 fNoiseCoeff;
	real32 fMulti1;
	real32 fMulti2;
	real32 fMulti3;
	real32 fOverlay1;
	real32 fOverlay2;
};

#endif

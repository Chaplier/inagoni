/****************************************************************************************************

		RisingSmoke.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#ifndef __RisingSmoke__
#define __RisingSmoke__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GasBase.h"

extern const MCGUID CLSID_RisingSmoke;

class RisingSmoke : public GasBase
{
public:
	RisingSmoke();

	virtual int32		MCCOMAPI GetParamsBufferSize() const 
	{
		return (sizeof(GasBasePMap) + 2*sizeof(real32));
	}

protected:
	virtual void	CustomPreProcess();
	virtual real32	GetLocalDensity(const TVector3& point);

	real32 Expand(TVector3& point, const real32 dist) const;
	void Stretch(TVector3& point, const real32 dist, const real32 halfHeight) const;

	// Complete the pmap
	real32 fRisingDia; // bottom diameter in %
	real32 fRisingRamp; // ramp in %
	// End pmap

	real32 fHalfHeight;
};


#endif

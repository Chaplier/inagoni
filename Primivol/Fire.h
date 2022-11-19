/****************************************************************************************************

		Fire.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#ifndef __Fire__
#define __Fire__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GasBase.h"

extern const MCGUID CLSID_Fire;

class Fire : public GasBase
{
public:
	Fire();

	virtual MCCOMErr MCCOMAPI SimpleHandleEvent			(MessageID message, IMFResponder* source, void* data);

	virtual int32		MCCOMAPI GetParamsBufferSize() const 
	{
		return (sizeof(GasBasePMap) + sizeof(real32));
	}

protected:
	virtual void	CustomPreProcess();
	virtual real32	GetLocalDensity(const TVector3& point);

//	real32 Expand(TVector3& point, const real32 dist) const;

	// Complete the pmap
	real32	fRisingAcc; // acceleration control
	// End pmap
};


#endif

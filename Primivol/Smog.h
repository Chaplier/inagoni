/****************************************************************************************************

		Smog.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#ifndef __Smog__
#define __Smog__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GasBase.h"

extern const MCGUID CLSID_Smog;

class Smog : public GasBase
{
public:
	Smog();

	virtual MCCOMErr MCCOMAPI SimpleHandleEvent			(MessageID message, IMFResponder* source, void* data);
	
	virtual int32		MCCOMAPI GetParamsBufferSize() const 
	{
		return (sizeof(GasBasePMap));
	}

protected:
	virtual void	CustomPreProcess();
	virtual real32	GetLocalDensity(const TVector3& point);
};


#endif

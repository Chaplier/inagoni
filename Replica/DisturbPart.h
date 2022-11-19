/****************************************************************************************************

		DisturbPart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#ifndef __DisturbPart__
#define __DisturbPart__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"
#include "I3DShScene.h"

extern const MCGUID CLSID_DisturbPart;

class DisturbPart : public TBasicPart
{
public:
	DisturbPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	void	EnableShadingOptions(const boolean enable);
	void	EnableOption(const boolean enable, const int32 partID);

	TMCCountedPtr<I3DShScene> fScene;
};










#endif

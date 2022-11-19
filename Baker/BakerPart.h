/****************************************************************************************************

		BakerPart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/4/2004

****************************************************************************************************/

#ifndef __BakerPart__
#define __BakerPart__

#if CP_PRAGMA_ONCE
#pragma once
#endif


#include "BasicMCFCOMImplementations.h"

extern const MCGUID CLSID_BakerPart;

class BakerPart : public TBasicPart
{
public:
	BakerPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
//	virtual void	MCCOMAPI FinishCreateFromResource();
	virtual void	MCCOMAPI BaseWindowBecameVisible(boolean inShown);

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:

	void	DisplayOption( const int32 option );

	bool mIsPopuReady;
};










#endif

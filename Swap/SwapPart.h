/****************************************************************************************************

		SwapPart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#ifndef __SwapPart__
#define __SwapPart__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"
#include "MCClassArray.h"
#include "copyright.h"

extern const MCGUID CLSID_SwapPart;

class SwapPart : public TBasicPart
{
public:
	SwapPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:

	void	SetNameOn(const int32 pmapID, const TMCString& name);

	void	BuildObjectPopupMenu();
	void	DisplayOptionPanel(const int32 option);

#if (VERSIONNUMBER >= 0x050000)
	TMCClassArray<TMCDynamicString> fTreeID; // store the tree name to help the popup<->tree relationship
#else
	TMCArray<int32> fTreeID; // store the tree permanet ID to have a popup<->tree relationship
#endif
};










#endif

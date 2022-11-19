/****************************************************************************************************

		BakerShaderPart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/21/2005

****************************************************************************************************/

#ifndef __BakerShaderPart__
#define __BakerShaderPart__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"

#include "FinalNormalMapShader.h"

extern const MCGUID CLSID_BakerShaderPart;

static const int32 kTypePartID = 'Type';

class BakerShaderPart : public TBasicPart
{
public:
	BakerShaderPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	IMFPart* GetTypePart(){if(!fTypePart)GetThisPartNoAddRef()->FindChildPartByID(&fTypePart, kTypePartID);return fTypePart;}

	void SetFlag(const boolean on, const ENormalFlags flag);
	void UpdateDisplay(const boolean invalidate=false, const boolean notify=false);
	void SetColorOnAxis(const int32 onAxis, const int32 colorID);

	TMCCountedPtr<IMFPart> fTypePart;
};










#endif

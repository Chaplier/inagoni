/****************************************************************************************************

		GasCommonPart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/11/2004

****************************************************************************************************/

#ifndef __GasCommonPart__
#define __GasCommonPart__

#include "BasicMCFCOMImplementations.h"

#include "I3DShScene.h"
#include "MCClassArray.h"

extern const MCGUID CLSID_GasCommonPart;

// This part take care of the params common to all the gas primitives
class GasCommonPart : public TBasicPart
{
public:
	GasCommonPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	void EnableNoisePart(const boolean enable);
	void UpdateIntensityRampCheckBoxes(const int32 flag);
	void UpdateColorRampCheckBoxes(const int32 flag);

	TMCCountedPtr<I3DShScene>		fScene;
};












#endif

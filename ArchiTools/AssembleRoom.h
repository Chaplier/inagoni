/****************************************************************************************************

		AssembleRoom.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	7/22/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __AssembleRoom__
#define __AssembleRoom__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicMCFCOMImplementations.h"

#include "BuildingPrim.h"

extern const MCGUID CLSID_AssembleRoomPart;

class AssembleRoomPart : public TBasicPart
{
public:
	AssembleRoomPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual void		MCCOMAPI SelfActivate		(boolean beActive);
	virtual void		MCCOMAPI SetShown			(boolean inShown);
	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	BuildingPrim*	GetPrimitive();

	TMCCountedPtr<BuildingPrim> fBuildingPrimitive;
};

#endif

#endif // !NETWORK_RENDERING_VERSION

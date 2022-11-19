/****************************************************************************************************

		FreeFormPartExt.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#pragma once

#include "BasicMCFCOMImplementations.h"
#include "I3DShScene.h"

#include <set>

class FreeFormModifierBase;

extern const MCGUID CLSID_FreeFormPartExt;

class FreeFormPartExt : public TBasicPart
{
public:
	FreeFormPartExt();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

	void DisplayValue(const TVector3& pos);

	void PATCH_StoreExtraData( const std::set<int32>& selectedHandles );
	void PATCH_ResStoreExtraData();
protected:
	//void AttachModifier();
	bool GetComponent(IShParameterComponent** component);

	void SetChildPartValue( uint32 inPartID, float value );
	float GetChildPartValue( uint32 inPartID );

	bool GetModifier(FreeFormModifierBase** modifier);
	
	std::set<int32> mStoredSelectedHandles;
};

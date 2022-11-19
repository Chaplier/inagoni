/****************************************************************************************************

		FreeFormModifier2x2.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormModifier2x2.h"

#include "Copyright.h"
#include "Shaper.h"
#include "IShUtilities.h"
#include "IShComponent.h"
#include "MFPartTypes.h"

const MCGUID CLSID_FreeFormModifier2x2(R_CLSID_FreeFormModifier2x2);

FreeFormModifier2x2PMap::FreeFormModifier2x2PMap()
{
	Clear();
}

void FreeFormModifier2x2PMap::Clear()
{
	memset(this, 0, sizeof (*this));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////:

FreeFormModifier2x2::FreeFormModifier2x2()
{
}

MCCOMErr FreeFormModifier2x2::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormModifier2x2))
	{
		TMCCountedGetHelper<FreeFormModifier2x2> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return FreeFormModifierBase::QueryInterface(riid, ppvObj);
}

int16 FreeFormModifier2x2::GetResID()
{
	return 820;		// This is the view ID in the resource file
}

void FreeFormModifier2x2::Clone(IExDataExchanger** res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	FreeFormModifier2x2* clone = new FreeFormModifier2x2();
	result = (IExDataExchanger*)clone;

	if (clone)
	{
	    clone->mPMap=mPMap; // copy the FreeFormModifierData
		CopyComponentExtraData( result );
	}
	clone->SetControllingUnknown(pUnkOuter);
} 

//------------------------------------------------------
MCCOMErr FreeFormModifier2x2::CopyComponentExtraData (IExDataExchanger* dest)
{
	TMCCountedPtr<FreeFormModifier2x2> destModifier;
	dest->QueryInterface(CLSID_FreeFormModifier2x2, (void**)&destModifier);
	if (destModifier)
	{
		CopyComponentExtraDataBase(destModifier);
	}
	return MC_S_OK;
}

const TVector3& FreeFormModifier2x2::GetOffset( size_t x, size_t y, size_t z ) const
{
	size_t index = ConvertIndex( x, y, z );
	return mPMap.mOffset[index];
}

void FreeFormModifier2x2::SetOffset( size_t x, size_t y, size_t z , const TVector3& value )
{
	size_t index = ConvertIndex( x, y, z );
	mPMap.mOffset[index] = value;

	InvalidateComponent();
}



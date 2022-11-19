/****************************************************************************************************

		FreeFormModifier3x3.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormModifier3x3.h"

#include "Copyright.h"
#include "Shaper.h"
#include "IShUtilities.h"
#include "IShComponent.h"
#include "MFPartTypes.h"

const MCGUID CLSID_FreeFormModifier3x3(R_CLSID_FreeFormModifier3x3);

FreeFormModifier3x3PMap::FreeFormModifier3x3PMap()
{
	Clear();
}

void FreeFormModifier3x3PMap::Clear()
{
	memset(this, 0, sizeof (*this));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////:

FreeFormModifier3x3::FreeFormModifier3x3()
{
}

MCCOMErr FreeFormModifier3x3::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormModifier3x3))
	{
		TMCCountedGetHelper<FreeFormModifier3x3> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return FreeFormModifierBase::QueryInterface(riid, ppvObj);
}

int16 FreeFormModifier3x3::GetResID()
{
	return 820;		// This is the view ID in the resource file
}

void FreeFormModifier3x3::Clone(IExDataExchanger** res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	FreeFormModifier3x3* clone = new FreeFormModifier3x3();
	result = (IExDataExchanger*)clone;

	if (clone)
	{
	    clone->mPMap=mPMap; // copy the FreeFormModifierData
		CopyComponentExtraData( result );
	}
	clone->SetControllingUnknown(pUnkOuter);
} 

//------------------------------------------------------
MCCOMErr FreeFormModifier3x3::CopyComponentExtraData (IExDataExchanger* dest)
{
	TMCCountedPtr<FreeFormModifier3x3> destModifier;
	dest->QueryInterface(CLSID_FreeFormModifier3x3, (void**)&destModifier);
	if (destModifier)
	{
		CopyComponentExtraDataBase(destModifier);
	}
	return MC_S_OK;
}

const TVector3& FreeFormModifier3x3::GetOffset( size_t x, size_t y, size_t z ) const
{
	size_t index = ConvertIndex( x, y, z );
	return mPMap.mOffset[index];
}

void FreeFormModifier3x3::SetOffset( size_t x, size_t y, size_t z , const TVector3& value )
{
	size_t index = ConvertIndex( x, y, z );
	mPMap.mOffset[index] = value;

	InvalidateComponent();
}



/****************************************************************************************************

		FreeFormModifier4x4.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormModifier4x4.h"

#include "Copyright.h"
#include "Shaper.h"
#include "IShUtilities.h"
#include "IShComponent.h"
#include "MFPartTypes.h"

const MCGUID CLSID_FreeFormModifier4x4(R_CLSID_FreeFormModifier4x4);

FreeFormModifier4x4PMap::FreeFormModifier4x4PMap()
{
	Clear();
}

void FreeFormModifier4x4PMap::Clear()
{
	memset(this, 0, sizeof (*this));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////:

FreeFormModifier4x4::FreeFormModifier4x4()
{
}

MCCOMErr FreeFormModifier4x4::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormModifier4x4))
	{
		TMCCountedGetHelper<FreeFormModifier4x4> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return FreeFormModifierBase::QueryInterface(riid, ppvObj);
}

int16 FreeFormModifier4x4::GetResID()
{
	return 820;		// This is the view ID in the resource file
}

void FreeFormModifier4x4::Clone(IExDataExchanger** res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	FreeFormModifier4x4* clone = new FreeFormModifier4x4();
	result = (IExDataExchanger*)clone;

	if (clone)
	{
	    clone->mPMap=mPMap; // copy the FreeFormModifierData
		CopyComponentExtraData( result );
	}
	clone->SetControllingUnknown(pUnkOuter);
} 

//------------------------------------------------------
MCCOMErr FreeFormModifier4x4::CopyComponentExtraData (IExDataExchanger* dest)
{
	TMCCountedPtr<FreeFormModifier4x4> destModifier;
	dest->QueryInterface(CLSID_FreeFormModifier4x4, (void**)&destModifier);
	if (destModifier)
	{
		CopyComponentExtraDataBase(destModifier);
	}
	return MC_S_OK;
}

const TVector3& FreeFormModifier4x4::GetOffset( size_t x, size_t y, size_t z ) const
{
	size_t index = ConvertIndex( x, y, z );
	return mPMap.mOffset[index];
}

void FreeFormModifier4x4::SetOffset( size_t x, size_t y, size_t z , const TVector3& value )
{
	size_t index = ConvertIndex( x, y, z );
	mPMap.mOffset[index] = value;

	InvalidateComponent();
}



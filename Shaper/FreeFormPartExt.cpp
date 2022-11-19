/****************************************************************************************************

		FreeFormPartExt.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormPartExt.h"

#include "Shaper.h"
#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "MCClassArray.h"
#include "MiscComUtilsImpl.h"
#include "InterfaceIDs.h"
#include "IShComponent.h"
#include "ComMessages.h"
#include "IMFParameterPart.h"

#include "FreeFormModifierNxN.h"

const MCGUID CLSID_FreeFormPartExt(R_CLSID_FreeFormPartExt);

// FreeFormPartExt class
FreeFormPartExt::FreeFormPartExt()
{
}

void FreeFormPartExt::PATCH_StoreExtraData( const std::set<int32>& selectedHandles )
{
	mStoredSelectedHandles = selectedHandles;
}

void FreeFormPartExt::PATCH_ResStoreExtraData()
{
	TMCCountedPtr<FreeFormModifierBase> modifier;
	if( GetModifier(&modifier) )
	{
		modifier->PATCH_RetoreExtraData( mStoredSelectedHandles );
	}
}

void FreeFormPartExt::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr FreeFormPartExt::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormPartExt))
	{
		TMCCountedGetHelper<FreeFormPartExt> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void FreeFormPartExt::FinishCreateFromResource()
{
	TBasicPart::FinishCreateFromResource();
}

bool FreeFormPartExt::GetComponent(IShParameterComponent** component)
{
	// Look like the component part is the parent of our part extension
	IMFPart* part = this->GetThisPartNoAddRef();
	if( part )
	{
		TMCCountedPtr<IMFPart> parentPart;
		parentPart = part->GetPartParent();
		if( parentPart )
		{
			TMCCountedPtr<IMFParameterComponentPart> compPart;
			parentPart->QueryInterface(IID_IMFParameterComponentPart, (void**) &compPart);
			if( compPart )
			{
				compPart->GetParameterComponent( component );
				if( component )
					return true;
			}
		}
	}

	return false;
}

bool FreeFormPartExt::GetModifier(FreeFormModifierBase** modifier)
{
	TMCCountedPtr<IShParameterComponent> component;
	if( GetComponent(&component) )
		component->QueryInterface( CLSID_FreeFormModifierBase, (void**)modifier );

	return (*modifier!=NULL);
}

void FreeFormPartExt::SetChildPartValue( uint32 inPartID, float value )
{
	IMFPart* part = this->GetThisPartNoAddRef();
	if( !part )
		return;

	TMCCountedPtr<IMFPart> posPart;
	part->FindChildPartByID( &posPart, inPartID );
	if( posPart )
		posPart->SetValue( &value, kReal32ValueType, false, false );
}

float FreeFormPartExt::GetChildPartValue( uint32 inPartID )
{
	float result =0;

	IMFPart* part = this->GetThisPartNoAddRef();
	if( !part )
		return result;

	TMCCountedPtr<IMFPart> posPart;
	part->FindChildPartByID( &posPart, inPartID );
	if( posPart )
		posPart->GetValue( &result, kReal32ValueType );
	
	return result;
}

void FreeFormPartExt::DisplayValue(const TVector3& pos)
{
	SetChildPartValue( 'xPos', pos.x );
	SetChildPartValue( 'yPos', pos.y );
	SetChildPartValue( 'zPos', pos.z );
}

boolean	FreeFormPartExt::Receive(int32 message, IMFResponder* source, void* data)
{
	
	boolean handledMessage= false;

	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			PATCH_ResStoreExtraData();

			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();

					switch (partID)
					{
					case 'xPos':
					case 'yPos':
					case 'zPos':
						{
							TMCCountedPtr<FreeFormModifierBase> modifier;
							if( GetModifier(&modifier) )
							{
								real32 value=0;
								part->GetValue( &value,kReal32ValueType );
								const TVector3 prevPos = modifier->GetSelectionPos( );
								TVector3 newPos = prevPos;
								// Something's wrong: if we take only this part value, it appears that the component doesn't have the right position
								/*if( partID=='xPos')*/ newPos.x = GetChildPartValue('xPos');
								/*if( partID=='yPos')*/ newPos.y = GetChildPartValue('yPos');
								/*if( partID=='zPos')*/ newPos.z = GetChildPartValue('zPos');
								modifier->OffsetSelectionPos( newPos-prevPos );
							}
						} break;
					default: break;
					}
				}
			}
		}
	}

	return handledMessage;
}


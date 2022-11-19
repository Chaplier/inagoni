/****************************************************************************************************

		PartExtension.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	2/9/2004

****************************************************************************************************/

#include "PartExtension.h"

#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"


#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_TParamChooserPart4(R_CLSID_TParamChooserPart4);
#else
const MCGUID CLSID_TParamChooserPart4={R_CLSID_TParamChooserPart4};
#endif

////////////////////////////////////////////////////////////////////////
MCCOMErr TParamChooserPart4::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_TParamChooserPart4))
	{
		TMCCountedGetHelper<TParamChooserPart4> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TParamChooserPart::QueryInterface(riid, ppvObj);
}

void TParamChooserPart4::SetOption( const int32 selection )
{
	IMFPart* shapePart = GetThisPartNoAddRef()->FindChildPartByID('Shap');
	if(!shapePart) return; // Avoid crash

	switch(selection)
	{
	case 'Opt1':
	case 'Opt2':
	case 'Opt3':
	case 'Opt4':
		{
			shapePart->SetShown(false);
		} break;
	case 'Opt5':
	case 'Opt6':
	case 'Opt7':
	case 'Opt8':
		{
			shapePart->SetShown(true);
		} break;
	}
}


#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_TParamChooserPart1(R_CLSID_TParamChooserPart1);
const MCGUID CLSID_TParamChooserPart2(R_CLSID_TParamChooserPart2);
const MCGUID CLSID_TParamChooserPart3(R_CLSID_TParamChooserPart3);
#else
const MCGUID CLSID_TParamChooserPart1={R_CLSID_TParamChooserPart1};
const MCGUID CLSID_TParamChooserPart2={R_CLSID_TParamChooserPart2};
const MCGUID CLSID_TParamChooserPart3={R_CLSID_TParamChooserPart3};
#endif

TParamChooserPart::TParamChooserPart() { }
TParamChooserPart::~TParamChooserPart() { }

void TParamChooserPart::SelfPrepareToDestroy()
{
	fTypePart = NULL;

	TBasicPart::SelfPrepareToDestroy();
}

void TParamChooserPart::FinishCreateFromResource()
{
	IMFPart* thisPart = this->GetThisPartNoAddRef();

	if(thisPart)
	{
		fTypePart = thisPart->FindChildPartByID('Type');
	}

	fCurrentOption = 0;
}
/* Bug in Carrara: the part is shown with the default component values from time to time
void TParamChooserPart::SetShown(boolean inShown)
{
	if(!inShown) return;

	IMFPart* thisPart = this->GetThisPartNoAddRef();

	if(thisPart)
	{
		IMFPart* typePart = thisPart->FindChildPartByID('Type');

		int32 selection=0;
		typePart->GetValue((void**)&selection, kInt32ValueType);

		SetOption( selection );
	}
}
*/

void TParamChooserPart::SelfDraw(IMCGraphicContext* graphicContext, const TMCRect& inZone)
{
	// This is done in SelfDraw because it didn't work in others methods ( bug in Carrara )
	if(fTypePart)
	{
		int32 selection=0;
		fTypePart->GetValue((void**)&selection, kInt32ValueType);

		if(selection!=fCurrentOption)
			SetOption( selection );
	}

	TBasicPart::SelfDraw(graphicContext, inZone);
}

boolean TParamChooserPart::Receive(int32 message, IMFResponder* source, void* data)
{

	if (message == EMFPartMessage::kMsg_PartValueChanged)
	{
		if (MCVerify(source))
		{
			TMCCountedPtr<IMFPart> part; // = NULL;
			source->QueryInterface(IID_IMFPart, (void **)&part);
			if (MCVerify(part))
			{
				const IDType partID= part->GetIMFPartID();
				switch(partID)
				{
				case 'Type':
					{
						// Depending on the selected item, hide or show the children
						int32 selection=0;
						part->GetValue((void**)&selection, kInt32ValueType);

						if(selection!=fCurrentOption)
						{
							IMFPart* thisPart = this->GetThisPartNoAddRef();

							if(thisPart)
							{
								SetOption( selection );
							}
						}
						return false; // Let the message countinue so the shader can be updated
					}
				default:
					{
						return false; // Let the others messages go were they want to go
					}
				}
			}
		}
	}

	return false; // Let the others messages go were they want to go	
}

////////////////////////////////////////////////////////////////////////
MCCOMErr TParamChooserPart1::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_TParamChooserPart1))
	{
		TMCCountedGetHelper<TParamChooserPart1> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TParamChooserPart::QueryInterface(riid, ppvObj);
}

void TParamChooserPart1::SetOption( const int32 selection )
{
	IMFPart* sizePart = GetThisPartNoAddRef()->FindChildPartByID('Size');
	if(!sizePart) return; // Avoid crash

	switch(selection)
	{
	case 'Op13':
	case 'Op14':
	case 'Op15':
	case 'Op16':
	case 'Op17':
	case 'Op18':
	case 'Op19':
	case 'Op20':
	case 'Opt4': sizePart->SetShown(false); break;
	default:
		{
			sizePart->SetShown(true);
		} break;
	}
}

////////////////////////////////////////////////////////////////////////
MCCOMErr TParamChooserPart2::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_TParamChooserPart2))
	{
		TMCCountedGetHelper<TParamChooserPart2> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TParamChooserPart::QueryInterface(riid, ppvObj);
}

void TParamChooserPart2::SetOption( const int32 selection )
{
	IMFPart* sizePart = GetThisPartNoAddRef()->FindChildPartByID('Seco');
	if(!sizePart) return; // Avoid crash
	IMFPart* shiftPart = GetThisPartNoAddRef()->FindChildPartByID('Shif');
	if(!shiftPart) return; // Avoid crash
	IMFPart* slopePart = GetThisPartNoAddRef()->FindChildPartByID('Slop');
	if(!slopePart) return; // Avoid crash
	IMFPart* flatPart = GetThisPartNoAddRef()->FindChildPartByID('GapI');
	if(!flatPart) return; // Avoid crash
	IMFPart* gapPart = GetThisPartNoAddRef()->FindChildPartByID('GapS');
	if(!gapPart) return; // Avoid crash
	IMFPart* thickPart = GetThisPartNoAddRef()->FindChildPartByID('Thic');
	if(!thickPart) return; // Avoid crash
	IMFPart* seedPart = GetThisPartNoAddRef()->FindChildPartByID('SEED');
	if(!seedPart) return; // Avoid crash

	seedPart->SetShown(false);

	switch(selection)
	{
	case 'Opt7':
		{
			sizePart->SetShown(true);
			shiftPart->SetShown(true);
			slopePart->SetShown(false);
			gapPart->SetShown(false);
			flatPart->SetShown(false);
			thickPart->SetShown(false);
		} break;
	case 'Opt8':
	case 'Opt9':
		{
			sizePart->SetShown(false);
			shiftPart->SetShown(false);
			slopePart->SetShown(true);
			gapPart->SetShown(false);
			flatPart->SetShown(false);
			thickPart->SetShown(false);
		} break;
	case 'Op10':
		{
			sizePart->SetShown(false);
			shiftPart->SetShown(false);
			slopePart->SetShown(false);
			gapPart->SetShown(true);
			flatPart->SetShown(true);
			thickPart->SetShown(false);
		} break;
	case 'Op11':
		{
			sizePart->SetShown(false);
			shiftPart->SetShown(false);
			slopePart->SetShown(false);
			gapPart->SetShown(false);
			flatPart->SetShown(false);
			thickPart->SetShown(true);
		} break;
	case 'Op12':
		{
			seedPart->SetShown(true);
			sizePart->SetShown(false);
			shiftPart->SetShown(false);
			slopePart->SetShown(false);
			gapPart->SetShown(false);
			flatPart->SetShown(false);
			thickPart->SetShown(false);
		} break;
	}
}

////////////////////////////////////////////////////////////////////////
MCCOMErr TParamChooserPart3::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_TParamChooserPart3))
	{
		TMCCountedGetHelper<TParamChooserPart3> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TParamChooserPart::QueryInterface(riid, ppvObj);
}

void TParamChooserPart3::SetOption( const int32 selection )
{
	IMFPart* gapPart = GetThisPartNoAddRef()->FindChildPartByID('GapS');
	if(!gapPart) return; // Avoid crash
	IMFPart* amplitudePart = GetThisPartNoAddRef()->FindChildPartByID('Ampl');
	if(!amplitudePart) return; // Avoid crash
	IMFPart* periodPart = GetThisPartNoAddRef()->FindChildPartByID('Peri');
	if(!periodPart) return; // Avoid crash
	IMFPart* shiftPart = GetThisPartNoAddRef()->FindChildPartByID('Shif');
	if(!shiftPart) return; // Avoid crash
	IMFPart* intensityPart = GetThisPartNoAddRef()->FindChildPartByID('Inte');
	if(!intensityPart) return; // Avoid crash
	IMFPart* txt2Part = GetThisPartNoAddRef()->FindChildPartByID('Txt2');
	if(!txt2Part) return; // Avoid crash
	IMFPart* txt3Part = GetThisPartNoAddRef()->FindChildPartByID('Txt3');
	if(!txt3Part) return; // Avoid crash
	IMFPart* txt4Part = GetThisPartNoAddRef()->FindChildPartByID('Txt4');
	if(!txt4Part) return; // Avoid crash
	IMFPart* proportionPart = GetThisPartNoAddRef()->FindChildPartByID('Prop');
	if(!proportionPart) return; // Avoid crash

	switch(selection)
	{
	case 'Opt1':
		{
			gapPart->SetShown(true);
			periodPart->SetShown(false);
			amplitudePart->SetShown(false);
			shiftPart->SetShown(false);
			intensityPart->SetShown(false);
			txt2Part->SetShown(false);
			txt3Part->SetShown(false);
			txt4Part->SetShown(false);
			proportionPart->SetShown(false);
		} break;
	case 'Opt2':
		{
			gapPart->SetShown(true);
			periodPart->SetShown(false);
			amplitudePart->SetShown(false);
			shiftPart->SetShown(false);
			intensityPart->SetShown(false);
			txt2Part->SetShown(false);
			txt3Part->SetShown(false);
			txt4Part->SetShown(false);
			proportionPart->SetShown(false);
		} break;
	case 'Opt3':
		{
			gapPart->SetShown(false);
			periodPart->SetShown(true);
			amplitudePart->SetShown(true);
			shiftPart->SetShown(true);
			intensityPart->SetShown(true);
			txt2Part->SetShown(true);
			txt3Part->SetShown(true);
			txt4Part->SetShown(true);
			proportionPart->SetShown(false);
		} break;
	case 'Opt4':
	case 'Opt5':
		{
			gapPart->SetShown(false);
			periodPart->SetShown(false);
			amplitudePart->SetShown(true);
			shiftPart->SetShown(false);
			intensityPart->SetShown(true);
			txt2Part->SetShown(false);
			txt3Part->SetShown(false);
			txt4Part->SetShown(true);
			proportionPart->SetShown(true);
		} break;
	case 'Opt6':
		{
			gapPart->SetShown(false);
			periodPart->SetShown(false);
			amplitudePart->SetShown(false);
			shiftPart->SetShown(false);
			intensityPart->SetShown(false);
			txt2Part->SetShown(false);
			txt3Part->SetShown(false);
			txt4Part->SetShown(false);
			proportionPart->SetShown(false);
		} break;
	}
}



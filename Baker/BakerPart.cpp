/****************************************************************************************************

		BakerPart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/4/2004

****************************************************************************************************/

#include "BakerPart.h"

#include "BakerDef.h"
#include "Baker.h"
#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "IMFTextPopupPart.h"
#include "MCClassArray.h"
#include "ShaderTypes.h"
#include "MiscComUtilsImpl.h"
#include "InterfaceIDs.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "ISceneSelection.h"
#include "I3DShTreeElement.h"

#include "IShPartUtilities.h"
const MCGUID CLSID_BakerPart(R_CLSID_BakerPart);

static const int32 kDiffuseMap = 'shad';
static const int32 kLightMap = 'ligh';
static const int32 kNormalMap = 'norm';

static const int32 kCellHeight = 25;


// Helper function
void BuildShadingDomainPopupMenu(IMFTextPopupPart* shadingDomainPopup, 
								 TMCClassArray<UVSpaceInfo>& shadingDomains)
{
	ThrowIfNil(shadingDomainPopup);
	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'Baki');

	// The popup menu will look like that:
	// All Domains Merged
	// All Domains Separated
	// _____________________
	// Domain 0
	// Domain 1
	// ...

	// We can display the diffent domains only if there's one tree selected

	// Clean the current popup menu
	shadingDomainPopup->RemoveAll();

	// Add the 2 first option (merge and separated)
	TMCDynamicString str;
	gResourceUtilities->GetIndString(str, kOtherNames, 4 ); // "Merge"
	shadingDomainPopup->AppendMenuItem(str);
	gResourceUtilities->GetIndString(str, kOtherNames, 5 ); // "Split"
	shadingDomainPopup->AppendMenuItem(str);
//	shadingDomainPopup->SetItemEnabled(shadingDomainPopup->GetMenuItemsCount()-1, true); // it's a custom menu: enable manualy the items
//	int32 actionID = kCreateShadingDomain0;
//	shadingDomainPopup->SetItemActionNumber(shadingDomainPopup->GetMenuItemsCount()-1, actionID);

	const int32 domainCount = shadingDomains.GetElemCount();
	if(domainCount>1)
	{
		// Add a separator
		shadingDomainPopup->AppendSeparator();

		// Fill the menu with the shading domains
		for (int32 domainID = 0; domainID<domainCount; domainID++)
		{
			shadingDomainPopup->AppendMenuItem(shadingDomains[domainID].fName);
//			shadingDomainPopup->SetItemEnabled(domainID, true); // it's a custom menu: enable manualy the items
//			shadingDomainPopup->SetItemActionNumber(domainID, IDRoot+domainID);
		}
	}
}

// BakerPart class

BakerPart::BakerPart() : mIsPopuReady(false)
{
}

void BakerPart::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr BakerPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BakerPart))
	{
		TMCCountedGetHelper<BakerPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void BakerPart::BaseWindowBecameVisible(boolean inShown)
{
	TBasicPart::BaseWindowBecameVisible(inShown);

	// Default option setting (from the preferences)
	int32 option = kDiffuseMap;
	IMFPart* thispart = GetThisPartNoAddRef();
	if( thispart )
	{
		TMCCountedPtr<IMFPart> part;
		thispart->FindChildPartByID( &part, 'MapT' );
		if( part )
			part->GetValue(&option, kInt32ValueType);
	}

	DisplayOption(option);
}

boolean	BakerPart::Receive(int32 message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();

					int32 size = 0;

					switch (partID)
					{
					case 'MapT':
						{
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);
							DisplayOption(option);
						} break;
					case 'Doma':
						{
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);
						} break;
					case 'Opt1': size = 64; break;
					case 'Opt2': size = 128; break;
					case 'Opt3': size = 256; break;
					case 'Opt4': size = 512; break;
					case 'Opt5': size = 768; break;
					case 'Opt6': size = 1024; break;
					case 'Opt7': size = 2048; break;
					case 'Opt8': size = 4096; break;
					default: break;
					}

					if(size>0)
					{
						if(!IsSerialValid())
						{
							if(size>128)
							{	// demo version: size is limited to 128
								CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'Baki');
								TMCDynamicString message;
								gResourceUtilities->GetIndString( message, kOtherNames, 9);
								gPartUtilities->Alert(message);
								size = 128;
							}
						}

						IMFPart* thisPart = GetThisPartNoAddRef();
						if(MCVerify(thisPart))
						{
							TMCCountedPtr<IMFPart> widthPart;
							thisPart->FindChildPartByID(&widthPart,'MapW');
							TMCCountedPtr<IMFPart> heightPart;
							thisPart->FindChildPartByID(&heightPart,'MapH');

							if(MCVerify(widthPart) && MCVerify(heightPart))
							{
								widthPart->SetValue(&size, kInt32ValueType, true, true);
								heightPart->SetValue(&size, kInt32ValueType, true, true);
							}
						}
					}
				}
			}
		}
	}

	return handledMessage;
}

void ShowPart(IMFPart* part, const boolean show, TMCPoint& posInParent, const int32 cellHeight)
{
	part->SetShown(show);
	if(show)
	{
		part->SetPositionInParent(posInParent,true,true);
		posInParent.y += cellHeight;
	}
}

void BakerPart::DisplayOption( const int32 option )
{
	// Get all the parts
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCCountedPtr<IMFPart> diffPart;
	thisPart->FindChildPartByID(&diffPart,'DifC');
	TMCCountedPtr<IMFPart> highlightPart;
	thisPart->FindChildPartByID(&highlightPart,'HigC');
	TMCCountedPtr<IMFPart> shininessPart;
	thisPart->FindChildPartByID(&shininessPart,'ShiC');
	TMCCountedPtr<IMFPart> bumpPart;
	thisPart->FindChildPartByID(&bumpPart,'BumC');
	TMCCountedPtr<IMFPart> reflexionPart;
	thisPart->FindChildPartByID(&reflexionPart,'RflC');
	TMCCountedPtr<IMFPart> transparencyPart;
	thisPart->FindChildPartByID(&transparencyPart,'TraC');
	TMCCountedPtr<IMFPart> refractionPart;
	thisPart->FindChildPartByID(&refractionPart,'RfrC');
	TMCCountedPtr<IMFPart> glowPart;
	thisPart->FindChildPartByID(&glowPart,'GloC');
	TMCCountedPtr<IMFPart> spacePart;
	thisPart->FindChildPartByID(&spacePart,'SpaP');
	TMCCountedPtr<IMFPart> normalType;
	thisPart->FindChildPartByID(&normalType,'NorT');

	TMCCountedPtr<IMFPart> cameraPart;
	thisPart->FindChildPartByID(&cameraPart,'Came'); // 'UCam'

	TMCCountedPtr<IMFPart> diffuseHelp;
	thisPart->FindChildPartByID(&diffuseHelp,'DifH');
	TMCCountedPtr<IMFPart> lightHelp;
	thisPart->FindChildPartByID(&lightHelp,'LigH');
	TMCCountedPtr<IMFPart> normalHelp;
	thisPart->FindChildPartByID(&normalHelp,'NorH');

	diffuseHelp->SetShown(false);
	lightHelp->SetShown(false);
	normalHelp->SetShown(false);

	// Show/Hide depending on the kind of map
	switch(option)
	{
	case kDiffuseMap:
		{
			TMCPoint posInParent(0,0);

			ShowPart(diffPart, true, posInParent, kCellHeight);
			ShowPart(highlightPart, true, posInParent, kCellHeight);
			ShowPart(shininessPart, true, posInParent, kCellHeight);
			ShowPart(bumpPart, true, posInParent, kCellHeight);
			ShowPart(reflexionPart, true, posInParent, kCellHeight);
			ShowPart(transparencyPart, true, posInParent, kCellHeight);
			ShowPart(refractionPart, true, posInParent, kCellHeight);
			ShowPart(glowPart, true, posInParent, kCellHeight);
			ShowPart(cameraPart, false, posInParent, kCellHeight);
			ShowPart(spacePart, false, posInParent, 60);
			ShowPart(normalType, false, posInParent, 35);

			diffuseHelp->SetShown(true);
		} break;
	case kLightMap:
		{
			TMCPoint posInParent(0,0);

			ShowPart(diffPart, false, posInParent, kCellHeight);
			ShowPart(highlightPart, false, posInParent, kCellHeight);
			ShowPart(shininessPart, false, posInParent, kCellHeight);
			ShowPart(bumpPart, true, posInParent, kCellHeight);
			ShowPart(reflexionPart, false, posInParent, kCellHeight);
			ShowPart(transparencyPart, false, posInParent, kCellHeight);
			ShowPart(refractionPart, false, posInParent, kCellHeight);
			ShowPart(glowPart, false, posInParent, kCellHeight);
			ShowPart(cameraPart, true, posInParent, kCellHeight);
			ShowPart(spacePart, false, posInParent, 60);
			ShowPart(normalType, false, posInParent, 50);

			lightHelp->SetShown(true);
		} break;
	case kNormalMap:
		{
			TMCPoint posInParent(0,0);

			ShowPart(diffPart, false, posInParent, kCellHeight);
			ShowPart(highlightPart, false, posInParent, kCellHeight);
			ShowPart(shininessPart, false, posInParent, kCellHeight);
			ShowPart(bumpPart, true, posInParent, kCellHeight);
			ShowPart(reflexionPart, false, posInParent, kCellHeight);
			ShowPart(transparencyPart, false, posInParent, kCellHeight);
			ShowPart(refractionPart, false, posInParent, kCellHeight);
			ShowPart(glowPart, false, posInParent, kCellHeight);
			ShowPart(cameraPart, false, posInParent, kCellHeight);
			ShowPart(spacePart, true, posInParent, 60);
			ShowPart(normalType, true, posInParent, 50);

			normalHelp->SetShown(true);

			// Select the Bump channel, it's usualy wanted when baking normal maps
			TMCCountedPtr<IMFPart> check;
			bumpPart->FindChildPartByID(&check, 'Bump');
			MCAssert(check);

			boolean on = true;
			check->SetValue(&on, kBooleanValueType, true, true);

		} break;
	}

	// Popup menu with the domains
	if(!mIsPopuReady)
	{
		ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
		if (currentDoc)
		{
			TMCCountedPtr<ISceneSelection>	fSelection;
			currentDoc->GetSceneSelection(&fSelection);

			// The domain iDs are displayed only if 1 tree is selected. If there're
			// more, just show th Merge and Separate option
			TMCClassArray<UVSpaceInfo> shadingDomains;
			if(!fSelection->IsMultipleSelection() && !fSelection->IsEmpty())
			{
				// Get the domains IDs on this object
				if(fSelection->GetObjectCount()!=1)
					return; // Something's wrong
				ISelectableObject* leafObject = fSelection->GetSelectableObjectByIndex(0);

				if (MCVerify(leafObject))
				{
					TMCCountedPtr<I3DShTreeElement> tree;
					leafObject->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
					
					if (tree)
					{
						TMCCountedPtr<I3DShInstance> instance;
						tree->QueryInterface(IID_I3DShInstance, (void**) &instance); 

						if(MCVerify(instance))
						{
							const uint32 domainCount = instance->GetUVSpaceCount();
							if(domainCount>1)
							{
								shadingDomains.SetElemCount(domainCount);
								for(uint32 iDomain=0 ; iDomain<domainCount ; iDomain++)
								{
									instance->GetUVSpace(iDomain, &shadingDomains[iDomain]);
								}
							}
						}
					}
				}
			}

			TMCCountedPtr<IMFPart> popup;
			thisPart->FindChildPartByID(&popup, 'Doma');
			MCAssert(popup);

			TMCCountedPtr<IMFTextPopupPart> shadingDomainPopup;
			popup->QueryInterface( IID_IMFTextPopupPart, (void **)&shadingDomainPopup );

			BuildShadingDomainPopupMenu(shadingDomainPopup, shadingDomains);
			mIsPopuReady = true;
		}
	}
}


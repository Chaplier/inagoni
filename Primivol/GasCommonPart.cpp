/****************************************************************************************************

		GasCommonPart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/11/2004

****************************************************************************************************/

#include "GasCommonPart.h"

#include "GasDef.h"
#include "GasBase.h"
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
#include "ComMessages.h"
#include "IComponentChooser.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_GasCommonPart(R_CLSID_GasCommonPart);
#else
const MCGUID CLSID_GasCommonPart = {R_CLSID_GasCommonPart};
#endif

static const int32 kMsg_BecameFirstResponder = 'BFRe';


// GasCommonPart class

GasCommonPart::GasCommonPart()
{
}

void GasCommonPart::SelfPrepareToDestroy()
{
	fScene = NULL;

	TBasicPart::SelfPrepareToDestroy();
}

MCErr GasCommonPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_GasCommonPart))
	{
		TMCCountedGetHelper<GasCommonPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void GasCommonPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	TBasicPart::FinishCreateFromResource();
}

void GasCommonPart::EnableNoisePart(const boolean enable)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return;
	TMCCountedPtr<IMFPart> optionPart;
	thisPart->FindChildPartByID(&optionPart, 'Opti');
	if(!optionPart)
		return;

	optionPart->Enable(enable);
}

void SetBooleanValue(IMFPart* thisPart, const int32 partID, const boolean value)
{
	TMCCountedPtr<IMFPart> part;
	thisPart->FindChildPartByID(&part, partID);
	if(!part)
		return;
	part->SetValue(&value, kBooleanValueType,false,false);
}

void GasCommonPart::UpdateIntensityRampCheckBoxes(const int32 rampFlag)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return;

	SetBooleanValue(thisPart, 'DPHU', (rampFlag&ePosHalfSphereUp) != 0 );
	SetBooleanValue(thisPart, 'DPHD', (rampFlag&ePosHalfSphereDown ) != 0 );
	SetBooleanValue(thisPart, 'DNHU', (rampFlag&eNegHalfSphereUp ) != 0 );
	SetBooleanValue(thisPart, 'DNHD', (rampFlag&eNegHalfSphereDown ) != 0 );

	SetBooleanValue(thisPart, 'DPXC', (rampFlag&ePosXCylinder ) != 0 );
	SetBooleanValue(thisPart, 'DPYC', (rampFlag&ePosYCylinder ) != 0 );
	SetBooleanValue(thisPart, 'DPZC', (rampFlag&ePosZCylinder ) != 0 );
	SetBooleanValue(thisPart, 'DNXC', (rampFlag&eNegXCylinder ) != 0 );
	SetBooleanValue(thisPart, 'DNYC', (rampFlag&eNegYCylinder ) != 0 );
	SetBooleanValue(thisPart, 'DNZC', (rampFlag&eNegZCylinder ) != 0 );

	SetBooleanValue(thisPart, 'DXPo', (rampFlag&ePosXDist ) != 0 );
	SetBooleanValue(thisPart, 'DYPo', (rampFlag&ePosYDist ) != 0 );
	SetBooleanValue(thisPart, 'DZPo', (rampFlag&ePosZDist ) != 0 );
	SetBooleanValue(thisPart, 'DXNe', (rampFlag&eNegXDist ) != 0 );
	SetBooleanValue(thisPart, 'DYNe', (rampFlag&eNegYDist ) != 0 );
	SetBooleanValue(thisPart, 'DZNe', (rampFlag&eNegZDist ) != 0 );
}

void GasCommonPart::UpdateColorRampCheckBoxes(const int32 rampFlag)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return;

	SetBooleanValue(thisPart, 'FPHU', (rampFlag&ePosHalfSphereUp) != 0 );
	SetBooleanValue(thisPart, 'FPHD', (rampFlag&ePosHalfSphereDown ) != 0 );
	SetBooleanValue(thisPart, 'FNHU', (rampFlag&eNegHalfSphereUp ) != 0 );
	SetBooleanValue(thisPart, 'FNHD', (rampFlag&eNegHalfSphereDown ) != 0 );

	SetBooleanValue(thisPart, 'FPXC', (rampFlag&ePosXCylinder ) != 0 );
	SetBooleanValue(thisPart, 'FPYC', (rampFlag&ePosYCylinder ) != 0 );
	SetBooleanValue(thisPart, 'FPZC', (rampFlag&ePosZCylinder ) != 0 );
	SetBooleanValue(thisPart, 'FNXC', (rampFlag&eNegXCylinder ) != 0 );
	SetBooleanValue(thisPart, 'FNYC', (rampFlag&eNegYCylinder ) != 0 );
	SetBooleanValue(thisPart, 'FNZC', (rampFlag&eNegZCylinder ) != 0 );

	SetBooleanValue(thisPart, 'FXPo', (rampFlag&ePosXDist ) != 0 );
	SetBooleanValue(thisPart, 'FYPo', (rampFlag&ePosYDist ) != 0 );
	SetBooleanValue(thisPart, 'FZPo', (rampFlag&ePosZDist ) != 0 );
	SetBooleanValue(thisPart, 'FXNe', (rampFlag&eNegXDist ) != 0 );
	SetBooleanValue(thisPart, 'FYNe', (rampFlag&eNegYDist ) != 0 );
	SetBooleanValue(thisPart, 'FZNe', (rampFlag&eNegZDist ) != 0 );
}

boolean	GasCommonPart::Receive(int32 message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;
	
	switch (message)
	{
/*	case EMFPartMessage::kMsg_TabPageChanged:
		{	// Record the tab
			if(source)
			{
				TMCCountedPtr<IMFTabPart> tab;
				source->QueryInterface(IID_IMFTabPart, (void**) &tab);
				if(!tab)
					break;
				IDType pageID = tab->GetCurrentPage();

				IMFPart* thisPart = GetThisPartNoAddRef();
				if(!thisPart)
					break;
				TMCCountedPtr<IMFPart> pageIDPart;
				thisPart->FindChildPartByID(&pageIDPart, 'Page');
				if(!pageIDPart)
					break;
				pageIDPart->SetValue(&pageID, kInt32ValueType,false,false);


			}
		} break;*/
/*	case 16: // it's kMsg_LocatePartsByID from autoupdate
		{	
				IMFPart* thisPart = GetThisPartNoAddRef();
				if(!thisPart)
					break;
				TMCCountedPtr<IMFPart> tabPart;
				thisPart->FindChildPartByID(&tabPart, 'Tabs');
				if(!tabPart)
					break;
				TMCCountedPtr<IMFTabPart> tab;
				tabPart->QueryInterface(IID_IMFTabPart, (void**) &tab);
				if(!tab)
					break;
				IDType pageID = tab->GetCurrentPage();

				TMCCountedPtr<IMFPart> pageIDPart;
				thisPart->FindChildPartByID(&pageIDPart, 'Page');
				if(!pageIDPart)
					break;
				IDType pageIDRecorded;
				pageIDPart->GetValue(&pageIDRecorded, kInt32ValueType);

				if(pageID!=pageIDRecorded && pageIDRecorded>0)
				{
					tab->SetCurrentPage(pageIDRecorded);
				}
		} break;*/
	case EMFPartMessage::kMsg_OutOfScopePartValueChanged:
		{	// Return true to fix the non selection bug in C4
			return true;
		} break;
	case EMFPartMessage::kMsg_StartFastDraw:
		{
			// A hack to have the part properly initialized when switching from the Assemble room
			IMFPart* thisPart = GetThisPartNoAddRef();
			if(!thisPart)
				break;

			{	// update the checkboxes
				TMCCountedPtr<IMFPart> flagPart;
				thisPart->FindChildPartByID(&flagPart, 'Ramp');
				if(flagPart) // the small part doesn't have it
				{
					int32 flagValue = 0;
					flagPart->GetValue(&flagValue, kInt32ValueType);
					UpdateIntensityRampCheckBoxes(flagValue);
				}
			}
			{	// update the checkboxes
				TMCCountedPtr<IMFPart> flagPart;
				thisPart->FindChildPartByID(&flagPart, 'ColR');
				if(flagPart) // the small part doesn't have it
				{
					int32 flagValue = 0;
					flagPart->GetValue(&flagValue, kInt32ValueType);
					UpdateColorRampCheckBoxes(flagValue);
				}
			}

			// Hack not needed anymore, it was for the assemble room
	// 		int32 data = EMFPartMessage::kMsg_StartFastDraw;
	//		BroadcastUp(EMFPartMessage::kMsg_PartValueChanged, &data);
		} break;
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();

					int32 densityRamp = -1;

					switch (partID)
					{
					case 'DePr': // Density preset
						{	// update the checkboxes
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							UpdateIntensityRampCheckBoxes(GetDensityFlag(popupItem));
						} break;

					case 'CoPr': // Color preset
						{	// update the checkboxes
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							UpdateColorRampCheckBoxes(GetDensityFlag(popupItem));
						} break;
				
					case 'ShPo':
						{	// Shader popup: get its name or the index after (Noise, Turbulence, Plasma)
							TMCCountedPtr<IMFTextPopupPart> popup;
							part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
							ThrowIfNil(popup);
							if( popup->GetMenuItemsCount()<2 )
								break; // message from an empty menu, we don't care

							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
						
							// Find the shader name part to store or erase it
							IMFPart* thisPart = GetThisPartNoAddRef();
							if(!thisPart)
								break;
							TMCCountedPtr<IMFPart> namePart;
							thisPart->FindChildPartByID(&namePart, 'ShaN');

							TMCCountedPtr<IMFPart> customIndexPart;
							thisPart->FindChildPartByID(&customIndexPart, 'CusI');

							TMCDynamicString name;

							const int32 masterCount = (int32)fScene->GetMasterShadersCount();
							if(popupItem>=masterCount)
							{	// We use a build in noise, erase the currently stored name
								int32 index = popupItem-masterCount;
								GetNoiseName(index, name);
								namePart->SetValue(&name, kStringValueType,false,false);
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
								EnableNoisePart(true);
							}
							else
							{	// Get the name of the shader to store it
								TMCCountedPtr<I3DShMasterShader> masterShader;
								fScene->GetMasterShaderByIndex(&masterShader, popupItem);

								masterShader->GetName(name);
						
								namePart->SetValue(&name, kStringValueType,false,false);
								int32 index = 0;
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
								EnableNoisePart(false);
							}

							{	// Clean the popup menu so just the arrows will appear
								popup->RemoveAll();
								TMCString15 empty;
								popup->AppendMenuItem(empty);
							}

							thisPart->Invalidate(); // To redraw the text part near
							handledMessage = false; // Very important, otherwise the modification is not taken immediatly into account
						} break;
					default: break;
					}
				}
			}
		} break;
	case kMsg_MCC_NewComponentAdded:
	case kMsg_MCC_ComponentDeleted:
	case kMsg_MCC_ComponentModified:
	// Don't send this message anymore, otherwise the component 
	// animation is broken (only 2 keyframes are possible)
	// case kMsg_MCC_ComponentTracking:
/*	case kMsg_SCC_ComponentModified:
	case kMsg_SCC_ComponentTracking:
	case kMsg_SCC_NewComponentChosen:
	case kMsg_CUIP_SubpartChanged:
	case kMsg_CUIP_SubpartTracking:
	case kMsg_CUIP_DialogSubpartChanged:*/ //used when our component is edited in a dialog
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID = part->GetIMFPartID();
					MCAssert(partID == 'MoCo' );
				}
				// Check that it comes from our multiChooser part
				TMCCountedPtr<IMultipleComponentChooser> multiChooser;
				source->QueryInterface(IID_IMultipleComponentChooser, (void **)&multiChooser);
				if (multiChooser)
				{
				//	TMCCountedPtrArray<IShParameterComponent> componentList;
				//	multiChooser->GetComponentList(&componentList);
					MCAssert(multiChooser->GetFamilyID() == 'data' );

					// Don't forget to delete it at the reception
					MultiCompData* comData = new MultiCompData(message, multiChooser);

					BroadcastUp(EMFPartMessage::kMsg_PartValueChanged, comData);
				}
			//	BroadcastUp(kMsg_MCC_ComponentModified, nil);
				return kMFReceiveResult_Stop;
			}
		} break;
	}

	return handledMessage;
}


/****************************************************************************************************

		PopupParts.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/06/2006

****************************************************************************************************/

#include "PopupParts.h"

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
#include "I3DShObject.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_ShaderPopupPart(R_CLSID_ShaderPopupPart);
const MCGUID CLSID_ShaderChooserPart(R_CLSID_ShaderChooserPart);

const MCGUID CLSID_MeshPopupPart(R_CLSID_MeshPopupPart);
const MCGUID CLSID_MeshChooserPart(R_CLSID_MeshChooserPart);
#else
const MCGUID CLSID_ShaderPopupPart = {R_CLSID_ShaderPopupPart};
const MCGUID CLSID_ShaderChooserPart = {R_CLSID_ShaderChooserPart};

const MCGUID CLSID_MeshPopupPart = {R_CLSID_MeshPopupPart};
const MCGUID CLSID_MeshChooserPart = {R_CLSID_MeshChooserPart};
#endif

// Utils

void GetShaderNames( I3DShScene* inScene, TMCClassArray<TMCDynamicString>& shaderNames )
{
	const int32 shaderCount = inScene->GetMasterShadersCount();
	shaderNames.SetElemCount(shaderCount);

	for(int32 iShader=0 ; iShader<shaderCount ; iShader++)
	{
		TMCCountedPtr<I3DShMasterShader> masterShader;
		inScene->GetMasterShaderByIndex(&masterShader, iShader);

		TMCDynamicString name;
		masterShader->GetName(name);

		shaderNames[iShader] = name;
	}
}

void GetMeshNames( I3DShScene* scene, TMCClassArray<TMCDynamicString>& meshNames )
{
	const int32 treeCount = scene->GetTreesCount();
	for (int32 iTree = 0; iTree<treeCount; iTree++)
	{
		I3DShTreeElement* tree = scene->GetTreeByIndex(iTree);

		TMCCountedPtr<I3DShInstance> instance;
		tree->QueryInterface(IID_I3DShInstance, (void **)&instance);

		if(instance && instance->GetInstanceKind() == I3DShInstance::kPrimitiveInstance)
		{
#if (VERSIONNUMBER < 0x050000)
			TMCCountedPtr<FacetMesh> facetmesh;
			instance->GetFMesh(0, &facetmesh);
#else // From Carrara 5
			FacetMesh* facetmesh = instance->GetRenderingFacetMesh();
#endif

			if(facetmesh && facetmesh->fFacets.GetElemCount())
			{
				// Check that instance has a geometry
				TMCCountedPtr<I3DShObject> object;
				instance->Get3DObject(&object);

				if(object)
				{
					TMCDynamicString name;
					tree->GetName(name);

					meshNames.AddElem(name);
				}
			}
		}
	}
}

void BuildCustomPopupMenu(IMFPart* thisPart,
						   const int32 partID, 
						const TMCClassArray<TMCDynamicString>& itemNames,
						const int32 extraNameCount)
{
	const int32 nameCount = itemNames.GetElemCount();
	if(nameCount<1)
		return;

	TMCCountedPtr<IMFPart> part;
	thisPart->FindChildPartByID(&part, partID);
	ThrowIfNil(part);

	TMCCountedPtr<IMFTextPopupPart> popup;
	part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
	ThrowIfNil(popup);
	

	// The popup menu will look like that:
	// Item 0
	// Item 1
	// ...
	// _____________________
	// Extra Name (None or Noise)

	// Clean the current popup menu
	popup->RemoveAll();

	const int32 lastName = nameCount-extraNameCount;
	if(nameCount>0)
	{
		// Fill the menu with the first names ( the shading domains, ... )
		for (int32 iName = 0; iName<lastName; iName++)
		{
			popup->AppendMenuItem(itemNames[iName]);
		}

		// Add a separator
		popup->AppendSeparator();
	}

	// Add the last extra names (None, Noise, Turbulence, Plasma)
	{
		for (int32 iName = lastName; iName<nameCount; iName++)
		{
			popup->AppendMenuItem(itemNames[iName]);
		}
	}

//	popup->SetSelectedItem(selectItem,true);

}

//////////////////////////////////////////////////////////////////
//
// ShaderPopupPart
//

ShaderPopupPart::ShaderPopupPart()
{
}

void ShaderPopupPart::SelfPrepareToDestroy()
{
	fScene = NULL;
	fType = 0;

	TBasicPart::SelfPrepareToDestroy();
}

MCErr ShaderPopupPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ShaderPopupPart))
	{
		TMCCountedGetHelper<ShaderPopupPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void ShaderPopupPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	TBasicPart::FinishCreateFromResource();
}

// Read the token to know in which tab is this menu. 
// If fType==0, it's the firts tab menu, with 4 extras items in the menu
// If fType==1, it's the 3rd tab menu, with only 1 extra item
boolean	ShaderPopupPart::ReadAttribute(int32 inKeyword, TMCiostream& inStream)
{
	switch (inKeyword)
	{
	case 'Type':
		{
			inStream >> fType;
			return true;
		}
	default:
		return false;
	}
}

void ShaderPopupPart::BuildPopup()
{
	// Fill in the popup menus
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');

	TMCClassArray<TMCDynamicString> shaderNames;
	GetShaderNames( fScene, shaderNames );

	// Last name: Noise, Plasma and Turbulence
	const int32 shaderCount = shaderNames.GetElemCount();
	
	int32 extraNames = 0;

	if(fType==0)
	{
		extraNames = 4;
		shaderNames.AddElemCount(extraNames);
		for(int32 iName=0 ; iName<extraNames ; iName++)
		{
			TMCDynamicString str;
			GetNoiseName(iName+1, str);
			shaderNames[shaderCount+iName] = str;
		}
	}
	else if(fType==1)
	{
		extraNames = 1;
		shaderNames.AddElemCount(extraNames);
		TMCDynamicString none;
		gResourceUtilities->GetIndString(none, kStrings, 8 );
		shaderNames[shaderCount] = none;
	}

	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	BuildCustomPopupMenu( thisPart, thisPart->GetIMFPartID(), shaderNames, extraNames );
}

TMFEventResult ShaderPopupPart::SelfMouseDown(const TMCPoint& inWhere, const TMCPlatformEvent& inEvent)
{
	if (fScene)
	{
		BuildPopup();
	}
	return TBasicPart::SelfMouseDown(inWhere, inEvent);
}

//////////////////////////////////////////////////////////////////
//
// ShaderChooserPart (this part countains a shaderPopupPart)
//

ShaderChooserPart::ShaderChooserPart()
{
	fParam1ID = 'PaID'; // default Pmap index parameter name. Change it if several popup are used
	fParam2ID = 'ShNa'; // default Pmap name parameter name. Change it if several popup are used
}

void ShaderChooserPart::SelfPrepareToDestroy()
{
	fScene = NULL;
	TBasicPart::SelfPrepareToDestroy();
}

MCErr ShaderChooserPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ShaderChooserPart))
	{
		TMCCountedGetHelper<ShaderChooserPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void ShaderChooserPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	// Find the sub part that will be the one with the paramID
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(thisPart)
	{
		TMCCountedPtr<IMFPart> paramPart;
		thisPart->FindChildPartByID(&paramPart, 'PaID');

		if(paramPart)
		{
			paramPart->SetPartID(fParam1ID);
		}

		TMCCountedPtr<IMFPart> namePart;
		thisPart->FindChildPartByID(&namePart, 'ShNa');

		if(namePart)
		{
			namePart->SetPartID(fParam2ID);
		}
	}

	TBasicPart::FinishCreateFromResource();
}

boolean	ShaderChooserPart::ReadAttribute(int32 inKeyword, TMCiostream& inStream)
{
	switch (inKeyword)
	{
	case 'PaID':
		{
			inStream >> fParam1ID;
			return true;
		}
	case 'ShNa':
		{
			inStream >> fParam2ID;
			return true;
		}
	default:
		return false;
	}
}

boolean	ShaderChooserPart::Receive(int32 message, IMFResponder* source, void* data)
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

					int32 densityRamp = -1;

					switch (partID)
					{
					case 'ShaP':
						{	// Shader popup: get its name or the index after (Noise, Turbulence, Plasma)
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							TMCDynamicString name;
							part->GetValue(&name, kStringValueType);
						
							// Find the shader name part to store or erase it
							IMFPart* thisPart = GetThisPartNoAddRef();
							if(!thisPart)
								break;
							TMCCountedPtr<IMFPart> namePart;
							thisPart->FindChildPartByID(&namePart, fParam2ID);

							TMCCountedPtr<IMFPart> customIndexPart;
							thisPart->FindChildPartByID(&customIndexPart, fParam1ID);

			//				TMCDynamicString name;

							const int32 masterCount = (int32)fScene->GetMasterShadersCount();
							if(popupItem>=masterCount)
							{	// We use a build in noise, erase the currently stored name
								int32 index = popupItem-masterCount;
			//					GetNoiseName(index, name);
								namePart->SetValue(&name, kStringValueType,false,false);
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
							}
							else
							{	// Get the name of the shader to store it
			//					TMCCountedPtr<I3DShMasterShader> masterShader;
			//					fScene->GetMasterShaderByIndex(&masterShader, popupItem);

			//					masterShader->GetName(name);
						
								namePart->SetValue(&name, kStringValueType,false,false);
								int32 index = 0;
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
							}

							{	// Clean the popup menu so just the arrows will appear
								TMCCountedPtr<IMFTextPopupPart> popup;
								part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
								ThrowIfNil(popup);
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
	}

	return handledMessage;
}

//////////////////////////////////////////////////////////////////
//
// MeshPopupPart
//

MeshPopupPart::MeshPopupPart()
{
}

void MeshPopupPart::SelfPrepareToDestroy()
{
	fScene = NULL;
	fType = 0;

	TBasicPart::SelfPrepareToDestroy();
}

MCErr MeshPopupPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_MeshPopupPart))
	{
		TMCCountedGetHelper<MeshPopupPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void MeshPopupPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	TBasicPart::FinishCreateFromResource();
}

void MeshPopupPart::BuildPopup()
{
	// Fill in the popup menus
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');

	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCClassArray<TMCDynamicString> meshNames;
	GetMeshNames( fScene, meshNames );

	// Last name: Noise, Plasma and Turbulence
	const int32 meshCount = meshNames.GetElemCount();
	
	const int32 extraNames = 1;
	meshNames.AddElemCount(extraNames);
	TMCDynamicString none;
	gResourceUtilities->GetIndString(none, kStrings, 8 );
	meshNames[meshCount] = none;

	BuildCustomPopupMenu( thisPart, thisPart->GetIMFPartID(), meshNames, extraNames );
}

TMFEventResult MeshPopupPart::SelfMouseDown(const TMCPoint& inWhere, const TMCPlatformEvent& inEvent)
{
	if (fScene)
	{
		BuildPopup();
	}
	return TBasicPart::SelfMouseDown(inWhere, inEvent);
}

//////////////////////////////////////////////////////////////////
//
// MeshChooserPart (this part countains a MeshPopupPart)
//

MeshChooserPart::MeshChooserPart()
{
	fParam1ID = 'MeID'; // default Pmap index parameter name. Change it if several popup are used
	fParam2ID = 'MeNa'; // default Pmap name parameter name. Change it if several popup are used
}

void MeshChooserPart::SelfPrepareToDestroy()
{
	fScene = NULL;
	TBasicPart::SelfPrepareToDestroy();
}

MCErr MeshChooserPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ShaderChooserPart))
	{
		TMCCountedGetHelper<MeshChooserPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void MeshChooserPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	// Find the sub part that will be the one with the paramID
	// Use this if more than one of this popup is used for 1 pmap => we need new 4charIDs for the other parameters
/*	IMFPart* thisPart = GetThisPartNoAddRef();
	if(thisPart)
	{
		TMCCountedPtr<IMFPart> paramPart;
		thisPart->FindChildPartByID(&paramPart, 'PaID');

		if(paramPart)
		{
			paramPart->SetPartID(fParam1ID);
		}

		TMCCountedPtr<IMFPart> namePart;
		thisPart->FindChildPartByID(&namePart, 'ShNa');

		if(namePart)
		{
			namePart->SetPartID(fParam2ID);
		}
	}*/

	TBasicPart::FinishCreateFromResource();
}
/*
boolean	MeshChooserPart::ReadAttribute(int32 inKeyword, TMCiostream& inStream)
{
	switch (inKeyword)
	{
	case 'PaID':
		{
			inStream >> fParam1ID;
			return true;
		}
	case 'ShNa':
		{
			inStream >> fParam2ID;
			return true;
		}
	default:
		return false;
	}
}
*/
boolean	MeshChooserPart::Receive(int32 message, IMFResponder* source, void* data)
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

					int32 densityRamp = -1;

					switch (partID)
					{
					case 'MesP':
						{	// Mesh popup: get its name or the index after (None)
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							TMCDynamicString name;
							part->GetValue(&name, kStringValueType);
						
							// Find the shader name part to store or erase it
							IMFPart* thisPart = GetThisPartNoAddRef();
							if(!thisPart)
								break;
							TMCCountedPtr<IMFPart> namePart;
							thisPart->FindChildPartByID(&namePart, fParam2ID);

							TMCCountedPtr<IMFPart> customIndexPart;
							thisPart->FindChildPartByID(&customIndexPart, fParam1ID);


							TMCCountedPtr<IMFTextPopupPart> popup;
							part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
							ThrowIfNil(popup);
							const uint32 itemCount = popup->GetMenuItemsCount();
	
							if(popupItem>=(int32)(itemCount-1))
							{	// Last item : We don't use a build in noise, erase the currently stored name
								int32 index = 1;
								namePart->SetValue(&name, kStringValueType,false,false);
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
							}
							else
							{	// Get the name of the tree element to store it						
								namePart->SetValue(&name, kStringValueType,false,false);
								int32 index = 0;
								customIndexPart->SetValue(&index, kInt32ValueType,false,false);
							}

							{	// Clean the popup menu so just the arrows will appear
								TMCCountedPtr<IMFTextPopupPart> popup;
								part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
								ThrowIfNil(popup);
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
	}

	return handledMessage;
}


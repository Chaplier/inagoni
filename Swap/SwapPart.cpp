/****************************************************************************************************

		SwapPart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#include "SwapPart.h"

#include "SwapDef.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "IMFTextPopupPart.h"
#include "MiscComUtilsImpl.h"
#include "InterfaceIDs.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "I3DShTreeElement.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "ISceneSelection.h"
#include "I3DShCamera.h"
#include "I3DShShader.h"
#include "I3DShLightsource.h"
#include "PublicUtilities.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_SwapPart(R_CLSID_SwapPart);
#else
const MCGUID CLSID_SwapPart = {R_CLSID_SwapPart};
#endif

// SwapPart class

SwapPart::SwapPart()
{
}

void SwapPart::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr SwapPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_SwapPart))
	{
		TMCCountedGetHelper<SwapPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void SwapPart::FinishCreateFromResource()
{
	try
	{
		// Build the Tree popup menu for the surface type
		BuildObjectPopupMenu();

		// display the right option panel
		ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
		if(!currentDoc)
			return;
		TMCCountedPtr<ISceneSelection>	selection;
		currentDoc->GetSceneSelection(&selection);

		TTreeSelectionIterator iter(selection);
		I3DShTreeElement* firstTree=iter.First();
		if(firstTree)
		{
			TMCCountedPtr<I3DShInstance> instance;
			firstTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(instance)
			{
				if( instance->GetInstanceKind() == I3DShInstance::kCameraInstance )
				{
					DisplayOptionPanel('Opt4');
				}
				else if( instance->GetInstanceKind() == I3DShInstance::kLightInstance )
				{
					DisplayOptionPanel('Opt3');
				}
				else
					DisplayOptionPanel('Opt1');
			}
			else
			{	// It's a group
				DisplayOptionPanel('Opt1');
			}
		}
		else
		{	// It shouldn't happen
			DisplayOptionPanel('Opt1');
		}
	}
	catch(TMCException&)
	{
		// Couldn't replace
	}

	TBasicPart::FinishCreateFromResource();
}

void SwapPart::SetNameOn(const int32 pmapID, const TMCString& name)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCCountedPtr<IMFPart> namePart;
	thisPart->FindChildPartByID(&namePart, pmapID);
	
	if(namePart)
		namePart->SetValue(&name, kStringValueType, true, true); // true for the initialisation
}

boolean	SwapPart::Receive(int32 message, IMFResponder* source, void* data)
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

					switch (partID)
					{
					case 'Obje':
						{	// Get the object name and record it
							TMCDynamicString name;
							part->GetValue(&name, kStringValueType);
							SetNameOn('ObjN', name);
						} break;
					case 'Shad':
						{	// Get the object name and record it
							TMCDynamicString name;
							part->GetValue(&name, kStringValueType);
							SetNameOn('ShaN', name);
						} break;
					case 'Tree':
						{	// Get the tree index
							TMCDynamicString title, explanation;
							{
								CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'SWAC');

								gResourceUtilities->GetIndString(title, kStrings, 5);
								gResourceUtilities->GetIndString(explanation, kStrings, 6);
							}

							// get the active scene
							TMCPtr<ISceneDocument>	sceneDoc;
							sceneDoc = gShell3DUtilities->GetLastActiveSceneDoc();
							MCVerify(sceneDoc);
							TMCPtr<I3DShScene> scene;
							scene = sceneDoc->GetScene();
							MCVerify(scene);
							
							TMCCountedPtr<I3DShTreeElement> tree;
							TTreeIdPath targetObject;
							scene->GetTreeByIDPath(&tree, targetObject);

							if (gShell3DUtilities->ChooseTreeDialog(&tree, title, explanation, nil, tree))
							{
								targetObject = TTreeIdPath::Root() + scene->GetTreePermanentID(tree);

								// Set the name, so the user can see it
								TMCDynamicString name;
								tree->GetName(name);

								part->SetValue(&name, kStringValueType, true, true);
	
								uint32 treeIndex = scene->GetTreeIndex(tree);
								
								IMFPart* thisPart = GetThisPartNoAddRef();
								if(!thisPart)
									break ;

								TMCCountedPtr<IMFPart> treeIDPart;
								thisPart->FindChildPartByID(&treeIDPart, 'TreI');
								if(treeIDPart)
									treeIDPart->SetValue(&treeIndex, kInt32ValueType, true, true); // true for the initialisation
							}
						} break;
					case 'Type':
						{	// display the right option panel
							int32 option;
							part->GetValue(&option, kInt32ValueType);
							DisplayOptionPanel(option);
						} break;
					default: break;
					}
				}
			}
		}
	}

	return handledMessage;
}

void SwapPart::BuildObjectPopupMenu()
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'SWAC');

	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();

	if(!currentDoc)
		return;

	// Get the scene
	TMCCountedPtr<I3DShScene> scene;
	currentDoc->GetScene(&scene);

	// Object Popup menu
	{
		TMCCountedPtr<IMFPart> popup;
		thisPart->FindChildPartByID(&popup, 'Obje');
		ThrowIfNil(popup);

		TMCCountedPtr<IMFTextPopupPart> objectPopup;
		popup->QueryInterface( IID_IMFTextPopupPart, (void **)&objectPopup );
		ThrowIfNil(objectPopup);

		// Clean the current popup menu
		objectPopup->RemoveAll();

		const int32 objectCount = scene->Get3DObjectsCount();

		// Fill the menu with the object of the scene
		for (int32 iObject = 0; iObject<objectCount; iObject++)
		{
			TMCCountedPtr<I3DShObject> object;
			scene->Get3DObjectByIndex(&object, iObject);

			TMCDynamicString name;
			object->GetName(name);

			objectPopup->AppendMenuItem(name);

	//		if(iObject==0)
	//			SetNameOn('ObjN', name);
		}

		if(objectCount)
			objectPopup->SetSelectedItem(0, true);
	}

	// Shader Popup menu
	{
		TMCCountedPtr<IMFPart> popup;
		thisPart->FindChildPartByID(&popup, 'Shad');
		ThrowIfNil(popup);

		TMCCountedPtr<IMFTextPopupPart> shaderPopup;
		popup->QueryInterface( IID_IMFTextPopupPart, (void **)&shaderPopup );
		ThrowIfNil(shaderPopup);

		// Clean the current popup menu
		shaderPopup->RemoveAll();

		const int32 shaderCount = scene->GetMasterShadersCount();

		// Fill the menu with the object of the scene
		for (int32 iShader = 0; iShader<shaderCount; iShader++)
		{
			TMCCountedPtr<I3DShMasterShader> masterShader;
			scene->GetMasterShaderByIndex(&masterShader, iShader);

			TMCDynamicString name;
			masterShader->GetName(name);

			shaderPopup->AppendMenuItem(name);

	//		if(iShader==0)
	//			SetNameOn('ShaN', name);
		}

		if(shaderCount)
			shaderPopup->SetSelectedItem(0, true);
	}

	// Tree Popup menu
	{
#if (VERSIONNUMBER >= 0x050000)
#else
		TMCCountedPtr<IMFPart> popup;
		thisPart->FindChildPartByID(&popup, 'Tree');
		ThrowIfNil(popup);

		TMCCountedPtr<IMFTextPopupPart> treePopup;
		popup->QueryInterface( IID_IMFTextPopupPart, (void **)&treePopup );
		ThrowIfNil(treePopup);

		// Clean the current popup menu
		treePopup->RemoveAll();

		const int32 treeCount = scene->GetTreesCount();
		fTreeID.SetElemCount(treeCount);

		// Fill the menu with the instances of the scene
		for (int32 iTree = 0; iTree<treeCount; iTree++)
		{
			I3DShTreeElement* tree = scene->GetTreeByIndex(iTree);

			TMCDynamicString name;
			tree->GetName(name);

			treePopup->AppendMenuItem(name);

			fTreeID[iTree] = tree->GetTreePermanentID();

	//		if(iObject==0)
	//			SetNameOn('ObjN', name);
		}

		if(treeCount)
			treePopup->SetSelectedItem(0, true);
#endif
	}
}

void SwapPart::DisplayOptionPanel(const int32 option)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCCountedPtr<IMFPart> objectPart;
	thisPart->FindChildPartByID(&objectPart, 'Obje');
	ThrowIfNil(objectPart);

	TMCCountedPtr<IMFPart> objectFitInPart;
	thisPart->FindChildPartByID(&objectFitInPart, 'OFit');
	ThrowIfNil(objectFitInPart);

	TMCCountedPtr<IMFPart> shaderPart;
	thisPart->FindChildPartByID(&shaderPart, 'Shad');
	ThrowIfNil(shaderPart);

	TMCCountedPtr<IMFPart> treePart;
	thisPart->FindChildPartByID(&treePart, 'Tree');
	ThrowIfNil(treePart);

	TMCCountedPtr<IMFPart> treeFitInPart;
	thisPart->FindChildPartByID(&treeFitInPart, 'TFit');
	ThrowIfNil(treeFitInPart);

	TMCCountedPtr<IMFPart> cameraPart;
	thisPart->FindChildPartByID(&cameraPart, 'CNod');
	ThrowIfNil(cameraPart);

	TMCCountedPtr<IMFPart> lightPart;
	thisPart->FindChildPartByID(&lightPart, 'LNod');
	ThrowIfNil(lightPart);

	objectPart->SetShown(false, false);
	objectFitInPart->SetShown(false, false);
	shaderPart->SetShown(false, false);
	treePart->SetShown(false, false);
	treeFitInPart->SetShown(false, false);
	cameraPart->SetShown(false, false);
	lightPart->SetShown(false, false);

	switch(option)
	{
	case 'Opt0':
		{
			treePart->SetShown(true, false);
			treeFitInPart->SetShown(true, false);
		} break;
	case 'Opt1':
		{
			objectPart->SetShown(true, false);
			objectFitInPart->SetShown(true, false);
		} break;
	case 'Opt2':
		{
			shaderPart->SetShown(true, false);
		} break;
	case 'Opt3':
		{
			cameraPart->SetShown(true, false);
		} break;
	case 'Opt4':
		{
			lightPart->SetShown(true, false);
		} break;
	}
}
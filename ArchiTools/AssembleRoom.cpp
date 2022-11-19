/****************************************************************************************************

		AssembleRoom.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	7/22/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "AssembleRoom.h"

#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"

#include "ISceneDocument.h"
#include "ISceneSelection.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShObject.h"
#include "IShComponent.h"

//#include "IComponentChooser.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
const MCGUID CLSID_AssembleRoomPart(R_CLSID_AssembleRoomPart);
#else
const MCGUID CLSID_AssembleRoomPart = {R_CLSID_AssembleRoomPart};
#endif

AssembleRoomPart::AssembleRoomPart()
{
}

void AssembleRoomPart::SelfPrepareToDestroy()
{
	fBuildingPrimitive = NULL;

	TBasicPart::SelfPrepareToDestroy();
}

MCErr AssembleRoomPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_AssembleRoomPart))
	{
		TMCCountedGetHelper<AssembleRoomPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void AssembleRoomPart::FinishCreateFromResource()
{
	BuildingPrim* buildingPrimitive = GetPrimitive();

	if(buildingPrimitive)
	{
		// Initialize the interface
		const int32 option = buildingPrimitive->AssembleRoomShowAll()?'Opt0':'Opt1';

		TMCCountedPtr<IMFPart> switcher;
		GetThisPartNoAddRef()->FindChildPartByID(&switcher, eShowActiveLevel);
		if(MCVerify(switcher) )
			switcher->SetValue(&option, kInt32ValueType, false, false);
// Test
		fBuildingPrimitive = NULL;
	}

	TBasicPart::FinishCreateFromResource();
}

boolean	AssembleRoomPart::Receive(int32 message, IMFResponder* source, void* data)
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

					BuildingPrim* buildingPrimitive = GetPrimitive();

					if(MCVerify(buildingPrimitive))
					{
						switch (partID)
						{
						case eShowActiveLevel:
							{	// Show All/ Show One switch
								int32 option=0;
								part->GetValue(&option,kInt32ValueType);
								if(option == 'Opt0')
								{	// Show All
									buildingPrimitive->AssembleRoomShowAll(true);
								}
								else if(option == 'Opt1')
								{	// Show One
									buildingPrimitive->AssembleRoomShowAll(false);
								}
								buildingPrimitive->ExtensionDataChanged();
								handledMessage = true;
							} break;
						case eShowLevelUp:
							{	// Show upper level (if possible)
								buildingPrimitive->AssembleRoomShowLevel(fBuildingPrimitive->AssembleRoomShowLevel()+1);
								buildingPrimitive->ExtensionDataChanged();
								handledMessage = true;
							} break;
						case eShowLevelDown:
							{	// Show under level (if possible)
								buildingPrimitive->AssembleRoomShowLevel(fBuildingPrimitive->AssembleRoomShowLevel()-1);
								buildingPrimitive->ExtensionDataChanged();
								handledMessage = true;
							} break;
						}
					}
// Test
		fBuildingPrimitive = NULL;
				}
			}
		}
	}

	return handledMessage;
}

void AssembleRoomPart::SelfActivate(boolean beActive)
{
	if(!beActive)
		fBuildingPrimitive = NULL;
}
void AssembleRoomPart::SetShown(boolean inShown)
{
	if(!inShown)
		fBuildingPrimitive = NULL;
}

BuildingPrim* AssembleRoomPart::GetPrimitive()
{
	if(!fBuildingPrimitive)
	{
		ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
		if(!currentDoc)
			return NULL;

		TMCCountedPtr<ISceneSelection> selection;
		currentDoc->GetSceneSelection(&selection);
		if(!selection)
			return NULL;
		
		TMCCountedPtr<I3DShObject> object;
#if (VERSIONNUMBER >= 0x040000)
		TMCCountedPtrArray<I3DShObject> objects;
		GetSelectedObjects(selection, IID_I3DShObject, objects);
		if(!objects.GetElemCount())
		{
			TMCCountedPtrArray<I3DShTreeElement> selectedTrees;
			GetSelectedObjects(selection, IID_I3DShTreeElement, selectedTrees);
			if(!selectedTrees.GetElemCount())
				return NULL;
		
			TMCCountedPtr<I3DShInstance> instance;
			selectedTrees[0]->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(!instance)
				return NULL;

			instance->Get3DObject(&object);
		}
		else
		{
			object = objects[0];
		}
#else
		GetSelectedObject(selection, IID_I3DShObject, (void **)&object);
		if(!object)
		{
			TMCCountedPtr<I3DShTreeElement> selectedTree;
			GetSelectedObject(selection, IID_I3DShTreeElement, (void **)&selectedTree);
			if(!selectedTree)
				return NULL;
		
			TMCCountedPtr<I3DShInstance> instance;
			selectedTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(!instance)
				return NULL;

			instance->Get3DObject(&object);
		}
#endif

		if(!object)
			return NULL;	

		TMCCountedPtr<I3DShPrimitive> primitive;
		object->QueryInterface(IID_I3DShPrimitive, (void**)&primitive);
		if(!primitive)
			return NULL;
	
		TMCCountedPtr<I3DShExternalPrimitive> externalPrimitive;
		primitive->QueryInterface(IID_I3DShExternalPrimitive, (void**) &externalPrimitive);
		if(!externalPrimitive)
			return NULL;
	
		TMCCountedPtr<IShComponent> component;
		externalPrimitive->GetPrimitiveComponent(&component);
		if(!component)
			return NULL;
	
		component->QueryInterface(CLSID_BuildingPrim, (void**) &fBuildingPrimitive);
	}

	return fBuildingPrimitive;
}

#endif // !NETWORK_RENDERING_VERSION

/****************************************************************************************************

		MImportCurve.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/23/2006

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MImportCurve.h"


#include "MMouseDown.h"
#include "BuildingModeler.h"

boolean MenuAction::ImportCurve( BuildingModeler* modeler )
{
	// Ask for the curve to import
	TMCCountedPtr<IMFDialogPart> theDialog;
	if( OpenDialog(&theDialog, kAskCurve) )
	{
		// Build the dialog popup menu
		TMCCountedPtr<IMFPart> theDialogPart;
		theDialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
//		FillInNamesPopup(theDialogPart, modeler, eNameList);

		if( theDialog->Go() )
		{
			// User hit OK: selection is set to a new shading domain
/*			if( !GetDialogString(theDialog, nameStr, eNameList) )
			{
				// Couldn't get the string: return
				theDialog->Finished();
				return false;
			}*/
		}
		else
		{
			// User hit cancel
			theDialog->Finished();
			return false;
		}
		
		theDialog->Finished();
	}
	else
	{
		// Couldn't open the dialog
		return false;
	}

//	ImportCurveAction::Create( &action, modeler, nameStr );
	return true;
}



////////////////////////////////////////////////////////////////////////////
//
// TO DO
//
#if 0
void FillInChildrenPopup(IMFPart*	inPart, 
						 const TMCString& objName, 
						 BuildingModeler* modeler, 
						 const int32 listID )
{
	TMCCountedPtr<IMFPart> popupPart = NULL;
	inPart->FindChildPartByID( &popupPart, listID );
	if(!popupPart)
		return;

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCCountedPtr<IMFTextPopupPart> menuPopup;
	popupPart->QueryInterface( IID_IMFTextPopupPart, (void **)&menuPopup );	

	// Clean the current popup menu
	menuPopup->RemoveAll();

	// Fill the menu with existing objects
	I3DShScene* scene = modeler->GetScene();
	const int32 objectCount = scene->Get3DObjectsCount();
	TMCCountedPtr<I3DShObject> sceneObject;
	int32 itemAdded=0;
	int32 currentItem=-1; // No child item
	for(int32 iObject=0 ; iObject<objectCount ; iObject++)
	{
		scene->Get3DObjectByIndex(&sceneObject, iObject);

		// Don't add any kind of object (no building for example)
		TMCCountedPtr<I3DShExternalPrimitive> extShPrimitive;
		sceneObject->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extShPrimitive);
		if(extShPrimitive)
		{
			TMCCountedPtr<IShComponent>	shComponent;
			extShPrimitive->GetPrimitiveComponent(&shComponent);
			if(!shComponent)
				continue;

			TMCCountedPtr<I3DExGeometricPrimitive> geomPrimitive;
			shComponent->QueryInterface(IID_I3DExGeometricPrimitive, (void**) &geomPrimitive);
			if(!geomPrimitive)
				continue;
			TMCCountedPtr<BuildingPrim> building;
			if( geomPrimitive->QueryInterface(CLSID_BuildingPrim, (void**)&building)==MC_S_OK )
				continue;
			if( geomPrimitive->QueryInterface(CLSID_HousePrim, (void**)&building)==MC_S_OK )
				continue;
		}
		else
		{	// Check if it's a scene instance
			TMCCountedPtr<I3DShMasterGroup> masterGroup;
			sceneObject->QueryInterface(IID_I3DShMasterGroup, (void**) &masterGroup);
			if(!masterGroup)
				continue;
		}

		TMCDynamicString name;
		sceneObject->GetName(name);
		menuPopup->AppendMenuItem(name);
		menuPopup->SetItemEnabled(itemAdded, true); // it's a custom menu: enable manualy the items
		menuPopup->SetItemActionNumber(itemAdded, kChildAction+iObject);

		if(objName==name)
			currentItem = itemAdded;

		itemAdded++;
	}

	// Add the groups
/* Later: still need to do the mechanism inside the subObjects
	menuPopup->AppendSeparator();
	const int32 treeCount = scene->GetTreesCount();
	boolean added = false;
	for(int32 iTree=0 ; iTree<treeCount ; iTree++ )
	{
		I3DShTreeElement* tree = scene->GetTreeByIndex(iTree);
		if(!tree)
			continue;

		// See if its a group
		TMCCountedPtr<I3DShGroup> group;
		tree->QueryInterface(IID_I3DShGroup, (void**)&group);
		if(group)
		{
			if(group == modeler->GetUniverse())
				continue;
			TMCDynamicString name;
			tree->GetName(name);
			menuPopup->AppendMenuItem(name);
			menuPopup->SetItemEnabled(objectAdded+groupAdded, true); // it's a custom menu: enable manualy the items
			menuPopup->SetItemActionNumber(objectAdded+groupAdded, kChildAction+objectAdded+groupAdded);
		
			if(objName==name)
				currentItem = objectAdded+groupAdded + 1; // + 1 for the separator

			groupAdded++;
			added = true;
		}
	
	}
		
	// Add an empty one (No children)
	int32 separCount = 1;
	if(added)
	{
		separCount = 2;*/
		menuPopup->AppendSeparator();
//	}
	TMCString15 noChild;
	gResourceUtilities->GetIndString( noChild, kModelerStrings, 13);
	menuPopup->AppendMenuItem(noChild);
	menuPopup->SetItemEnabled(menuPopup->GetMenuItemsCount()-1, true); // it's a custom menu: enable manualy the items
	menuPopup->SetItemActionNumber(menuPopup->GetMenuItemsCount()-1, kChildAction+menuPopup->GetMenuItemsCount()-1);
	
	if(objName == kMultiName)
	{	// Add a Multi Children one
		TMCString31 multiChild;
		gResourceUtilities->GetIndString( multiChild, kModelerStrings, 39);
		menuPopup->AppendMenuItem(multiChild);
		menuPopup->SetSelectedItem(menuPopup->GetMenuItemsCount()-1, false);
	}
	else
	{
		// Select the right one
		if(currentItem>=0)
			menuPopup->SetSelectedItem(currentItem, false);
		else
			menuPopup->SetSelectedItem(menuPopup->GetMenuItemsCount()-1, false);
	}
}
#endif // 0


#endif // !NETWORK_RENDERING_VERSION

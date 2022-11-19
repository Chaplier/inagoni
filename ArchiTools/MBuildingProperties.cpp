/****************************************************************************************************

		MBuildingProperties.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingProperties.h"

#include "COMUtilities.h"
#include "COM3DUtilities.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "I3DShModule.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "3DViewPublicDefs.h"
#include "HierarchyPublicDefs.h"
#include "MMouseDown.h"
#include "IMFTextPopupPart.h"
#include "I3DShObject.h"
#include "I3DShMasterGroup.h"
#include "IShComponent.h"
#include "IMFTabPart.h"
#include "IMFTextPopupPart.h"
#include "IMFListPart.h"
#include "I3DEditorHostPartDefs.h"
#include "IMFCollapsiblePart.h"
#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#include "BasicModule.h"
#include "BasicPropertiesClient.h"

#endif
////////////////////////////////////////////////////////////////////////////
//
// Shading domains
//
void BuildShadingDomainPopupMenu(IMFTextPopupPart* shadingDomainPopup, 
								 BuildingPrim* buildingPrimitive,
								 const int32 IDRoot)
{
	ThrowIfNil(shadingDomainPopup);

	// Clean the current popup menu
	shadingDomainPopup->RemoveAll();

	// Fill the menu with existing shading domains

	const int32 itemCount=buildingPrimitive->GetUVSpaceCount();
	MY_ASSERT(itemCount);

	for (int32 domainID = 0; domainID<itemCount; domainID++)
	{
		UVSpaceInfo& domainInfo = buildingPrimitive->GetUVSpace(domainID);
		shadingDomainPopup->AppendMenuItem(domainInfo.fName);
		shadingDomainPopup->SetItemEnabled(domainID, true); // it's a custom menu: enable manualy the items
		shadingDomainPopup->SetItemActionNumber(domainID, IDRoot+domainID);
	}

	// Add a separator and the Create item
	shadingDomainPopup->AppendSeparator();
	TMCDynamicString str;
	gResourceUtilities->GetIndString(str, kModelerStrings, 23 ); // "Create new shading domain..."
	shadingDomainPopup->AppendMenuItem(str);
	shadingDomainPopup->SetItemEnabled(shadingDomainPopup->GetMenuItemsCount()-1, true); // it's a custom menu: enable manualy the items
	int32 actionID = kCreateShadingDomain0;
	switch(IDRoot)
	{
	case kShadingDomainID0: actionID = kCreateShadingDomain0; break;
	case kShadingDomainID1: actionID = kCreateShadingDomain1; break;
	case kShadingDomainID2: actionID = kCreateShadingDomain2; break;
	case kShadingDomainID3: actionID = kCreateShadingDomain3; break;
	}
	shadingDomainPopup->SetItemActionNumber(shadingDomainPopup->GetMenuItemsCount()-1, actionID);
}

void DisplayShadingDomainPopupMenu(IMFTextPopupPart* shadingDomainPopup, 
								   const int32 shadingDomain,
								   const int32 totalCount)
{
	TMCDynamicString str;

	const int32 currentItemCount = (int32)shadingDomainPopup->GetMenuItemsCount();
	// If the selection contain several shading domains, show it in a gray menu item
	if(shadingDomain == -1)
	{
		// Add it only if it's missing
		if(currentItemCount<=totalCount+2)
		{
			gResourceUtilities->GetIndString(str, kModelerStrings, 22 );
			shadingDomainPopup->InsertMenuItem(str, 0);
		// Custom menu items are disable by default fShadingDomainPopup->SetItemEnabled(0, false);
		}
		shadingDomainPopup->SetSelectedItem(0, false);
	}
	else
	{
		// Remove the Multiple Domain if still here
		if(currentItemCount>totalCount+2)
		{
			shadingDomainPopup->DeleteMenuItem(0);
		// Custom menu items are disable by default fShadingDomainPopup->SetItemEnabled(0, false);
		}
		// Select the current selection shading domain
		shadingDomainPopup->SetSelectedItem(shadingDomain, false);
	}
}

void InitDomainList(BuildingPrim* buildingPrimitive, IMFPart* part, TMCArray<int32>& domainList)
{
	TMCCountedPtr<IMFListPart> listPart;
	part->QueryInterface(IID_IMFListPart, (void **)&listPart);
	ThrowIfNil(listPart);

	listPart->DeleteCells(0, kLIST_DELETE_ALL, true);
	domainList.ArrayFree();

	const int32 totalDomainCount = buildingPrimitive->GetUVSpaceCount();
	const int32 extraDomainCount = totalDomainCount-kBasicDomainsCount;
	listPart->AddCells( 0, extraDomainCount, true);
	int32 index=0;
	for( int32 iDomain=kBasicDomainsCount ; iDomain<totalDomainCount ; iDomain++, index++ )
	{
		listPart->SelectCell( index, true, false);
		UVSpaceInfo& uvSpaceInfo = buildingPrimitive->GetUVSpace(iDomain);
		part->SetValue( (void*)&uvSpaceInfo.fName, kStringValueType, true, false);
		domainList.AddElem(uvSpaceInfo.fID);
	}
}

////////////////////////////////////////////////////////////////////////////
//
// Children
//
void FillInChildrenPopup(IMFPart*	inPart, 
						 const TMCString& objName, 
						 BuildingModeler* modeler, 
						 const int32 listID )
{
	TMCCountedPtr<IMFPart> popupPart;
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
	menuPopup->SetItemActionNumber(itemAdded, kChildActionLimit);
	
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

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

void MCCOMAPI BuildingPropertiesClient::Create(IPropertiesModule*	propertiesModule,
							BuildingModeler* inModeler,BuildingPropertiesClient** client)
{
	TMCCountedCreateHelper<BuildingPropertiesClient> result(client);
	result = new BuildingPropertiesClient(propertiesModule,inModeler);
}

BuildingPropertiesClient::BuildingPropertiesClient(	IPropertiesModule* 	inPropertiesModule, 
													BuildingModeler* 	inModeler)
: TBasicPropertiesClient(inPropertiesModule)
{
	fBuildingModeler = inModeler;

	fDomainPopupValid = false;

	fSelectedCell = -1;

	fObjectCount = 0;
	fGroupCount = 0;

	gShell3DUtilities->GetImmediateUpdateChannel(&fImmediateUpdateChannel);

	gPartUtilities->CreatePartByResource(&fExtraPart, (EMFResources)'Node', kExtraPart);
	ThrowIfNil(fExtraPart);		
}

BuildingPropertiesClient::~BuildingPropertiesClient()
{
	fImmediateUpdateChannel->UnregisterListener(this);

	if(fExtraPart)
	{
		// We created this part, we're in charge of destroying it ( was detached from its
		// parents in the properties module ).
		fExtraPart->CallPrepareToDestroy();
		fExtraPart.Release();
	}
}

void BuildingPropertiesClient::DataChanged(IChangeChannel* 	channel, 
									IDType 					changeKind, 
									IMCUnknown* 			changedData)
{
	TMCCountedPtr<I3DShModule> shModule;
	fPropertiesModule->QueryInterface(IID_I3DShModule, (void **)&shModule);
	if (!shModule->GetIsHydrated())
		return;
	
	if (channel)
	{
		if (channel == fImmediateUpdateChannel)
		{
			TMCCountedPtr<IMFPart> hostPart;
			fPropertiesModule->GetHostPart(&hostPart);
			hostPart->ProcessUpdatesPart();
		
			// Invalidate the toolpart to have it updated when inserting a primitive
		//	fToolTabPart->GetThisPart()->Invalidate();
		}
	}
}

void BuildingPropertiesClient::GetExtraPart(IMFPart** outPart)
{
	TMCCountedGetHelper<IMFPart> helper(outPart);
	helper = fExtraPart;
}

void MCCOMAPI BuildingPropertiesClient::GetSelection(ISceneSelection**	outSelection)
{
	fBuildingModeler->GetSelection(outSelection);
}

void MCCOMAPI BuildingPropertiesClient::GetSelectionChannel(IChangeChannel** outChannel)
{
	fBuildingModeler->GetSelectionChannel(outChannel);
}

#if (VERSIONNUMBER >= 0x040000)
ResourceID MCCOMAPI	BuildingPropertiesClient::GetPropResID()
#else
ResourceID MCCOMAPI	BuildingPropertiesClient::GetPropResID(ISceneSelection* inSelection)
#endif
{
	return kBuildingPropertiesResID;
}

void MCCOMAPI BuildingPropertiesClient::ExtraTabsLoaded(IMFPart* inHostPart)
{
//	FindToolParts(fExtraPart);
}

void MCCOMAPI BuildingPropertiesClient::UnLoadProperties(IMFSplitBarPart*	inPropertiesSplitBar)
{
	if(fImmediateUpdateChannel)
		fImmediateUpdateChannel->UnregisterListener(this);

	TBasicPropertiesClient::UnLoadProperties(inPropertiesSplitBar);
}

void MCCOMAPI BuildingPropertiesClient::PreLoadProperties(IMFSplitBarPart*	inPropertiesSplitBar)
{
	TBasicPropertiesClient::PreLoadProperties(inPropertiesSplitBar);

	if(fImmediateUpdateChannel)
		fImmediateUpdateChannel->RegisterListener(this);	
}

#if (VERSIONNUMBER >= 0x040000)
void MCCOMAPI BuildingPropertiesClient::LoadPageData(	IMFPart* 			inTopPart)
#else
void MCCOMAPI BuildingPropertiesClient::LoadPageData(	IMFPart* 			inTopPart,
														ISceneSelection*	inSelection)
#endif
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

#if (VERSIONNUMBER >= 0x040000)
	fPropertiesModule->SetCurrentSelectionType(GetPropResID());//'none');//
#else
	fPropertiesModule->SetCurrentSelectionType(GetPropResID(inSelection));

	if (inSelection)
#endif
	{
		// Load the Modelisation properties

		// Get the selection status
		BuildingPrim* buildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();
		PrimitiveStatus* status = buildingPrimitive->GetStatus();

		// Current tool infos
		TMCCountedPtr<IMFPart> tabPart;
		fExtraPart->FindChildPartByID(&tabPart, kToolTabPartID);
		ThrowIfNil(tabPart);
		LoadCurrentToolInfos(tabPart, status);
		// Dimension
		TMCCountedPtr<IMFPart> dimensionPart;
		inTopPart->FindChildPartByID(&dimensionPart, 'Stac'/*'Posi'*/);
		MY_ASSERT(dimensionPart);
		LoadDimensionProperties(dimensionPart,status);
		// Shading
		TMCCountedPtr<IMFPart> shadingPart;
		inTopPart->FindChildPartByID(&shadingPart, 'Shad');
		MY_ASSERT(shadingPart);
		LoadShadingProperties(shadingPart,status);
		// Level
		TMCCountedPtr<IMFPart> levelPart;
		inTopPart->FindChildPartByID(&levelPart, 'Leve');
		MY_ASSERT(levelPart);
		LoadLevelProperties(levelPart,status);

		// Load the Default Parameters properties
		const BuildingPrimData& data = buildingPrimitive->Data();

		//Dimensions
		TMCCountedPtr<IMFPart> defDimPart;
		inTopPart->FindChildPartByID(&defDimPart, 'DDim');
		MY_ASSERT(defDimPart);
		// Level
		LoadDefaultLevelProperties(defDimPart,data);
		// Roof
		LoadDefaultRoofProperties(defDimPart,data);
		// Wall
		LoadDefaultWallProperties(defDimPart,data);
		// Room
		LoadDefaultRoomProperties(defDimPart,data);
		// Window
		LoadDefaultWindowProperties(defDimPart,data);
		// Door
		LoadDefaultDoorProperties(defDimPart,data);
		// Stairway
		LoadDefaultStairwayProperties(defDimPart,data);

		// Working Box
		TMCCountedPtr<IMFPart> defWBPart;
		inTopPart->FindChildPartByID(&defWBPart, 'DWor');
		MY_ASSERT(defWBPart);
		LoadDefaultWorkingBoxProperties(defWBPart,data);

		// Load the global properties

		TMCCountedPtr<IMFPart> domainList;
		inTopPart->FindChildPartByID(&domainList, 'ShDm');
		MY_ASSERT(domainList);
		LoadDomainList(domainList, buildingPrimitive);
		TMCCountedPtr<IMFPart> backdropPanel;
		inTopPart->FindChildPartByID(&backdropPanel, 'Back');
		MY_ASSERT(backdropPanel);
		LoadBackdropData(backdropPanel, buildingPrimitive);
	}
}

void BuildingPropertiesClient::LoadCurrentToolInfos(IMFPart* tabPart, PrimitiveStatus* status)
{
	TMCCountedPtr<IMFTabPart> toolTab;
	tabPart->QueryInterface(IID_IMFTabPart, (void**) &toolTab);
	ThrowIfNil(toolTab);

	int32 currentTool=0;
	gMenuUtilities->GetCurrentGlobalTool(currentTool); 

	switch (currentTool)
	{
	case kMoveToolID:
		{
			toolTab->SetCurrentPage(kMoveTab); 
			// Display the current selection center
			TVector3 center;
			status->fSelectionBBox.GetCenter(center);
			TMCCountedPtr<IMFPart> moveX;
			fExtraPart->FindChildPartByID(&moveX, eMoveX);
			MY_ASSERT(moveX);
			moveX->SetValue(&center.x, kReal32ValueType, true, true);
			TMCCountedPtr<IMFPart> moveY;
			fExtraPart->FindChildPartByID(&moveY, eMoveY);
			MY_ASSERT(moveY);
			moveY->SetValue(&center.y, kReal32ValueType, true, true);	

			fExtraPart->ProcessUpdatesPart(); // The marquee selection changes are not displayed without this
		}break;
	case kScaleToolID:	
		{
			toolTab->SetCurrentPage(kScaleTab); 
			// Display 100% for the scaling
			const real32 hundred=100;
			TMCCountedPtr<IMFPart> scaleX;
			fExtraPart->FindChildPartByID(&scaleX, eScaleX);
			MY_ASSERT(scaleX);
			scaleX->SetValue(&hundred, kReal32ValueType, true, true);
			TMCCountedPtr<IMFPart> scaleY;
			fExtraPart->FindChildPartByID(&scaleY, eScaleY);
			MY_ASSERT(scaleY);
			scaleY->SetValue(&hundred, kReal32ValueType, true, true);
			TMCCountedPtr<IMFPart> globalScale;
			fExtraPart->FindChildPartByID(&globalScale, eGlobalScale);
			MY_ASSERT(globalScale);
			globalScale->SetValue(&hundred, kReal32ValueType, true, true);
			// Display the current selection size
			TVector3 delta;
			status->fSelectionBBox.GetDelta(delta);
			TMCCountedPtr<IMFPart> sizeX;
			fExtraPart->FindChildPartByID(&sizeX, eSizeX);
			MY_ASSERT(sizeX);
			sizeX->SetValue(&delta.x, kReal32ValueType, true, true);
			TMCCountedPtr<IMFPart> sizeY;
			fExtraPart->FindChildPartByID(&sizeY, eSizeY);
			MY_ASSERT(sizeY);
			sizeY->SetValue(&delta.y, kReal32ValueType, true, true);	

			fExtraPart->ProcessUpdatesPart(); // The marquee selection changes are not displayed without this
		}break;
	case kRotateToolID:		
		toolTab->SetCurrentPage(kRotateTab); 
		break;

	case kCameraTrackXYToolID:
	case kCameraTrackXZToolID:
	case kCameraTrackYZToolID:
	case kCameraDollyToolID:
	case kCameraPanToolID:
	case kCameraBankToolID:
	case k2DPanToolID:
	case k2DZoomToolID:
	case k2DCameraToolID:
		toolTab->SetCurrentPage(kCameraTab);
		break;

	default: 
		toolTab->SetCurrentPage(kDefaultTab);
		break;
	}

	fPropertiesModule->RepositionExtraPart();
}

void GetPart(IMFPart* inPart, IMFPart** outPart, uint32 inPartID, uint32 inPartRes)
{
	inPart->FindChildPartByID(outPart, inPartID);
	if(!*outPart)
	{	// The part is missing : build one and add it to the container
		CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
		gPartUtilities->CreatePartByResource(outPart, (EMFResources)'Node', inPartRes);
		MY_ASSERT(*outPart);

		inPart->AddChildIMFPart(*outPart);
	}
	MY_ASSERT(outPart);
}

void BuildingPropertiesClient::LoadDimensionProperties(IMFPart* inPart, PrimitiveStatus* status)
{
	// Get all the sub part and hide them

	// Walls
	TMCCountedPtr<IMFPart> wallDim;
	inPart->FindChildPartByID(&wallDim, eWallData);
	TMCCountedPtr<IMFPart> wallGablePart;
	GetPart(inPart, &wallGablePart, eWallGable, 3010);
	TMCCountedPtr<IMFPart> crenelDataPart;
	GetPart(inPart, &crenelDataPart, eCrenelData, 3011);

	TMCCountedPtr<IMFPart> roofDim;
	inPart->FindChildPartByID(&roofDim, eRoofData);
	MY_ASSERT(roofDim);
	TMCCountedPtr<IMFPart> roomDim;
	inPart->FindChildPartByID(&roomDim, eRoomData);
	MY_ASSERT(roomDim);
	TMCCountedPtr<IMFPart> onePosPart;
	inPart->FindChildPartByID(&onePosPart, e1Pos);
	MY_ASSERT(onePosPart);
	TMCCountedPtr<IMFPart> twoPosPart;
	inPart->FindChildPartByID(&twoPosPart, e2Pos);
	MY_ASSERT(twoPosPart);
	TMCCountedPtr<IMFPart> obJPosPart;
	inPart->FindChildPartByID(&obJPosPart, eObjectPos);
	MY_ASSERT(obJPosPart);
	TMCCountedPtr<IMFPart> wallObJPosPart;
	inPart->FindChildPartByID(&wallObJPosPart, eWallObjectPos);
	MY_ASSERT(wallObJPosPart);
	TMCCountedPtr<IMFPart> roomObJPosPart;
	inPart->FindChildPartByID(&roomObJPosPart, eRoomObjectPos);
	MY_ASSERT(roomObJPosPart);
	TMCCountedPtr<IMFPart> centerPosPart;
	inPart->FindChildPartByID(&centerPosPart, eCenterPos);
	MY_ASSERT(centerPosPart);
	TMCCountedPtr<IMFPart> selectionNamePart;
	inPart->FindChildPartByID(&selectionNamePart, eSelectionNamepart);
	MY_ASSERT(selectionNamePart);
	TMCCountedPtr<IMFPart> holePointPart;
	GetPart(inPart, &holePointPart, eHolePointpartID, eHolePointpartRes);

	wallDim->SetShown(false, false);
	crenelDataPart->SetShown(false, false);
	wallGablePart->SetShown(false, false);
	roofDim->SetShown(false, false);
	roomDim->SetShown(false, false);
	onePosPart->SetShown(false, false);
	twoPosPart->SetShown(false, false);
	obJPosPart->SetShown(false, false);
	wallObJPosPart->SetShown(false, false);
	roomObJPosPart->SetShown(false, false);
	centerPosPart->SetShown(false, false);
	selectionNamePart->SetShown(false, false);
	holePointPart->SetShown(false, false);

	VPoint* helper = fBuildingModeler->GetPointHelper();
	if(helper)
	{
		onePosPart->SetShown(true, false);
		onePosPart->Enable(false);
		DisplayPositionInPart(helper->Position(),onePosPart,eXPos1,eYPos1);
	}
	else if(status->fHasSelection)
	{		
		TMCCountedPtr<IMFPart> selectionNameText;
		inPart->FindChildPartByID(&selectionNameText, eSelectionName);
		MY_ASSERT(selectionNameText);

		const int32 roofCount = status->fSelectedRoofCount;
		const int32 roomCount = status->fSelectedRoomCount;
		const int32 wallCount = status->fSelectedWallGlobalCount;
		const int32 wallObjCount = status->fSelectedWallObjectCount;
		const int32 roomObjCount = status->fSelectedRoomObjectCount;
		const int32 holePoint = status->fSelectedWallHolePointCount + status->fSelectedRoomHolePointCount;

		if(holePoint>0)
		{	// Display the Smooth Checkbox
			holePointPart->SetShown(true, false);
			TMCCountedPtr<IMFPart> smoothPart;
			inPart->FindChildPartByID(&smoothPart, eSmoothBox);
			MY_ASSERT(smoothPart);
			boolean allSmooth = (status->fIsSmoothed>0?true:false);
			smoothPart->SetValue(&allSmooth, kBooleanValueType, true, true);
		}

		if( !wallObjCount && !roomObjCount )
		{
			if(roofCount && !wallCount && !roomCount && !status->fSelectedPointCount)
			{	// Roofs selected
				roofDim->SetShown(true, false);
	
				// Show the selection name
				selectionNamePart->SetShown(true, false);
				selectionNameText->SetValue(&status->fRoofSelectionName, kStringValueType, true, true);

				{
					// Display the roof max height
					TMCCountedPtr<IMFPart> roofHeightPart;
					roofDim->FindChildPartByID(&roofHeightPart, eRoofCustomHeight);
					MY_ASSERT(roofHeightPart);

					const real32 max = status->fRoofMax;
					roofHeightPart->SetValue(&max, kReal32ValueType, true, true);
				
					TMCCountedPtr<IMFPart> roofHeightSwitch;
					roofDim->FindChildPartByID(&roofHeightSwitch, eRoofAutoHeight);
					MY_ASSERT(roofHeightSwitch);

					int32 option = 'Opt1';
					if( max != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultRoofMax() )
						option = 'Opt2';
					roofHeightSwitch->SetValue(&option, kInt32ValueType, true, true);
				}

				{
					// Display the roof min altitude
					TMCCountedPtr<IMFPart> roofAltitudePart;
					roofDim->FindChildPartByID(&roofAltitudePart, eRoofCustomBase);
					MY_ASSERT(roofAltitudePart);

					const real32 altitude = status->fRoofMin;
					roofAltitudePart->SetValue(&altitude, kReal32ValueType, true, true);
				
					TMCCountedPtr<IMFPart> roofAltitudeSwitch;
					roofDim->FindChildPartByID(&roofAltitudeSwitch, eRoofAutoBase);
					MY_ASSERT(roofAltitudeSwitch);

					int32 option = 'Opt1';
					if( altitude != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultRoofMin() )
						option = 'Opt2';
					roofAltitudeSwitch->SetValue(&option, kInt32ValueType, true, true);
				}
			}
			else if(roomCount && !wallCount && !roofCount)
			{	// Rooms selected
				roomDim->SetShown(true, false);

				// Show the selection name
				selectionNamePart->SetShown(true, false);
				selectionNameText->SetValue(&status->fRoomSelectionName, kStringValueType, true, true);

				{
					// Display the floor thickness
					TMCCountedPtr<IMFPart> floorThicknessPart;
					roomDim->FindChildPartByID(&floorThicknessPart, eFloorCustomThickness);
					MY_ASSERT(floorThicknessPart);

					const real32 thickness = status->fFloorThickness;
					floorThicknessPart->SetValue(&thickness, kReal32ValueType, true, true);
				
					TMCCountedPtr<IMFPart> floorThicknessSwitch;
					roomDim->FindChildPartByID(&floorThicknessSwitch, eFloorAutoThickness);
					MY_ASSERT(floorThicknessSwitch);

					int32 option = 'Opt1';
					if( thickness != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultFloorThickness() )
						option = 'Opt2';
					floorThicknessSwitch->SetValue(&option, kInt32ValueType, true, true);
				}
				{
					// Display the ceiling thickness
					TMCCountedPtr<IMFPart> ceilingThicknessPart;
					roomDim->FindChildPartByID(&ceilingThicknessPart, eCeilingCustomThickness);
					MY_ASSERT(ceilingThicknessPart);

					const real32 thickness = status->fCeilingThickness;
					ceilingThicknessPart->SetValue(&thickness, kReal32ValueType, true, true);
				
					TMCCountedPtr<IMFPart> ceilingThicknessSwitch;
					roomDim->FindChildPartByID(&ceilingThicknessSwitch, eCeilingAutoThickness);
					MY_ASSERT(ceilingThicknessSwitch);

					int32 option = 'Opt1';
					if( thickness != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultCeilingThickness() )
						option = 'Opt2';
					ceilingThicknessSwitch->SetValue(&option, kInt32ValueType, true, true);
				}
				{
					// Display the Draw Ceiling option (if != on all the room, show the draw on)
					TMCCountedPtr<IMFPart> drawCeilingPart;
					roomDim->FindChildPartByID(&drawCeilingPart, eDrawCeiling);
					MY_ASSERT(drawCeilingPart);

					const boolean draw = !status->fNoCeiling;
					drawCeilingPart->SetValue(&draw, kBooleanValueType, true, true);
				}
			}
			else if(roofCount==0 && roomCount==0 && wallCount)
			{	// If multiple walls: display common wall data
				// The display of the position is done later

				wallDim->SetShown(true, false);

				// Show the selection name
				selectionNamePart->SetShown(true, false);
				selectionNameText->SetValue(&status->fWallSelectionName, kStringValueType, true, true);

				{
					// Display the wall height
					TMCCountedPtr<IMFPart> wallHeightPart;
					wallDim->FindChildPartByID(&wallHeightPart, eWallCustomHeight);
					MY_ASSERT(wallHeightPart);

					const real32 height = status->fWallHeight;
					wallHeightPart->SetValue(&height, kReal32ValueType, true, true);
					// if height=-1: different wall heights
				
					TMCCountedPtr<IMFPart> wallHeightSwitch;
					wallDim->FindChildPartByID(&wallHeightSwitch, eWallAutoHeight);
					MY_ASSERT(wallHeightSwitch);

					int32 option = 'Opt1';
					if( height != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultWallHeight() )
						option = 'Opt2';
					wallHeightSwitch->SetValue(&option, kInt32ValueType, true, true);
				}
				{
					// Display the wall thickness
					TMCCountedPtr<IMFPart> wallThicknessPart;
					wallDim->FindChildPartByID(&wallThicknessPart, eWallCustomThickness);
					MY_ASSERT(wallThicknessPart);

					const real32 thickness = status->fWallThickness;
					wallThicknessPart->SetValue(&thickness, kReal32ValueType, true, true);
					// if thickness=-1: different wall thickness
				
					TMCCountedPtr<IMFPart> wallThicknessSwitch;
					wallDim->FindChildPartByID(&wallThicknessSwitch, eWallAutoThickness);
					MY_ASSERT(wallThicknessSwitch);

					int32 option = 'Opt1';
					if( thickness != fBuildingModeler->GetBuildingNoAddRef()->GetData().GetDefaultWallThickness() )
						option = 'Opt2';
					wallThicknessSwitch->SetValue(&option, kInt32ValueType, true, true);
				}
				{
					// Display the wall arc data
					const real32 offset = status->fWallOffset;
					DisplayValueInPart(offset, wallDim, eWallArcOffset );
				
					const int32 segments = status->fWallSegments;
					DisplayValueInPart(segments, wallDim, eWallArcSegments );
				}

				if( status->fSelectedSimpleWallCount>0 )
				{
					// Display the wall gable
					wallGablePart->SetShown(true, false);

					TMCCountedPtr<IMFPart> wallOptionPart;
					wallGablePart->FindChildPartByID(&wallOptionPart, eWallOption);
					MY_ASSERT(wallOptionPart);
					
					int32 option = 'Opt3'; // Multi selection option
					switch( status->fWallExtraHeight )
					{
					case 0: option = 'Opt1'; break;
					case 1: option = 'Opt2'; break;
					}

					wallOptionPart->SetValue(&option, kInt32ValueType, true, true);
				}
				
				if( status->fSelectedWallWithCrenelCount>0 )
				{
					crenelDataPart->SetShown(true, false);
					
					DisplayValueInPart(status->mCrenel.mDepth, crenelDataPart, eWallCrenelHeight );
					DisplayValueInPart(status->mCrenel.mWidth, crenelDataPart, eWallCrenelWidth );
					DisplayValueInPart(status->mCrenel.mSpacing, crenelDataPart, eWallCrenelSpacing );	
					DisplayValueInPart(status->mCrenel.mOffset, crenelDataPart, eWallCrenelOffset );	
					
					const int32 crenelShape = status->mCrenel.mShape;
					DisplayValueInPart(crenelShape, crenelDataPart, eWallCrenelShape );	
				}

			}			

			// 1 or 2 points selected
			if(status->fSelectedCommonPointCount == 1)
			{	// One point selected
				onePosPart->SetShown(true, false);
				onePosPart->Enable(true);
				DisplayPositionInPart(status->fFirstSelectedPos,onePosPart,eXPos1,eYPos1);

				// Show the selection name
				selectionNamePart->SetShown(true, false);
				selectionNameText->SetValue(&status->fPointSelectionName, kStringValueType, true, true);
			}
			else if(status->fSelectedCommonPointCount == 2 &&
					(  status->fSelectedPointCount==2 
					|| status->fSelectedProfilePointCount==2 
					|| status->fSelectedRoofPointCount==2
					|| status->fSelectedWallHolePointCount==2
					|| status->fSelectedRoomHolePointCount==2) )
			{	// Two points of the same kind
				twoPosPart->SetShown(true, false);
				DisplayPositionInPart(status->fFirstSelectedPos,twoPosPart,eXPos1,eYPos1);
				DisplayPositionInPart(status->fSecondSelectedPos,twoPosPart,eXPos2,eYPos2);
			}
		}
		else if(!roofCount && !roomCount && !wallCount)
		{	
			if( ( status->fSelectedWallObjectCount && !status->fSelectedRoomObjectCount ) ||
				 ( !status->fSelectedWallObjectCount && status->fSelectedRoomObjectCount ) )
			{
				// Display the properties  of the room or the wall object

				// Show the selection name
				selectionNamePart->SetShown(true, false);
				selectionNameText->SetValue(&status->fObjectSelectionName, kStringValueType, true, true);

				// Obj pos
				if(status->fSelectedWallObjectCount==1 || status->fSelectedRoomObjectCount==1)
				{
					centerPosPart->SetShown(true, false);

					DisplayPositionInPart(status->f2DObjCenter,centerPosPart,eXCenter,eYCenter);
				}

				// Multi selection properties
				const TVector2 dim(status->f2DObjWidth, status->f2DObjHeight);
				if(status->fSelectedWallObjectCount)
				{	// Wall objects case
					wallObJPosPart->SetShown(true, false);
					DisplayPositionInPart(dim,wallObJPosPart,eXDim,eYDim);
				}
				else
				{	// Room object case
					roomObJPosPart->SetShown(true, false);
					DisplayPositionInPart(dim,roomObJPosPart,eXDim,eYDim);
				}
				obJPosPart->SetShown(true, false);

				FillInChildrenPopup(obJPosPart, status->fSceneObjectName, 
					fBuildingModeler, eChildrenList);

				TMCCountedPtr<IMFPart> placementPart;
				obJPosPart->FindChildPartByID(&placementPart, eChildPlacement);
				MY_ASSERT(placementPart);
				int32 currentOption = 'Opt1';
				switch(status->fObjPlacement)
				{
				case eBigger:currentOption='Opt1';break;
				case eSlightlyBigger:currentOption='Opt2';break;
				case eFitIn:currentOption='Opt3';break;
				case eSlightlySmaller:currentOption='Opt4';break;
				case eSmaller:currentOption='Opt5';break;
				case eCustom:currentOption='Opt6';break;
				case eFreePlacement:currentOption='Opt7';break;
				case eMultiPlacement:currentOption='Opt0';break; // hidden checkbox
				}
				placementPart->SetValue(&currentOption, kInt32ValueType, true, true);

				// Display the current values of the placement
				const TVector3& offset = status->fObjOffset;
				DisplayValueInPart(offset.x, obJPosPart, ePlacementOffsetX);
				DisplayValueInPart(offset.y, obJPosPart, ePlacementOffsetY);
				DisplayValueInPart(offset.z, obJPosPart, ePlacementOffsetZ);
				const TVector3& scale = status->fObjScale;
				DisplayValueInPart(scale.x, obJPosPart, ePlacementScaleX);
				DisplayValueInPart(scale.y, obJPosPart, ePlacementScaleY);
				DisplayValueInPart(scale.z, obJPosPart, ePlacementScaleZ);
				const TVector3& rotate = status->fObjRotate;
				DisplayValueInPart(rotate.x, obJPosPart, ePlacementRotateX);
				DisplayValueInPart(rotate.y, obJPosPart, ePlacementRotateY);
				DisplayValueInPart(rotate.z, obJPosPart, ePlacementRotateZ);
			
				// Display the Auto Flip option (if != on all the obj, show it checked)
				TMCCountedPtr<IMFPart> autoFlipPart;
				obJPosPart->FindChildPartByID(&autoFlipPart, eAutoFlip);
				MY_ASSERT(autoFlipPart);

				const boolean autoFlip = status->fAutoFlip;
				autoFlipPart->SetValue(&autoFlip, kBooleanValueType, true, true);

			}
		}
	}

	// To restack the stacking part (ugly)
	TMCCountedPtr<IMFPart> stack;
	inPart->FindChildPartByID(&stack, 'Stac');
	MY_ASSERT(stack);
	TMCRect boundsRect;
	stack->GetBoundsInParent(boundsRect);
	TMCPoint size;
	size.x = boundsRect.GetWidth();
	size.y = boundsRect.GetHeight();
	size.y+=1; // to be sure to invalidate
	stack->SetSize(size, false, true);	
	// With this the stacking part is restacked, but it's ugly.
/*	TMCCountedPtr<IMFPart> selection;
	inPart->FindChildPartByID(&selection, 'Posi');
	if(selection)
	{
		TMCCountedPtr<IMFCollapsiblePart> collapsiblePart;
		selection->QueryInterface(IID_IMFCollapsiblePart, (void **)&collapsiblePart);

		if(collapsiblePart)
		{
			collapsiblePart->SetCollapseStatus(kCollapsed, true);
			collapsiblePart->SetCollapseStatus(kExpanded, true);
		}
	}*/
	
}

void BuildingPropertiesClient::LoadShadingProperties(IMFPart* inPart, PrimitiveStatus* status)
{
	// Get all the sub part and hide them
	TMCCountedPtr<IMFPart> node0;
	inPart->FindChildPartByID(&node0, eNode0);
	MY_ASSERT(node0);
	TMCCountedPtr<IMFPart> popup0;
	node0->FindChildPartByID(&popup0, ePopup0);
	MY_ASSERT(popup0);
	TMCCountedPtr<IMFTextPopupPart> shadingDomainPopup0;
	popup0->QueryInterface( IID_IMFTextPopupPart, (void **)&shadingDomainPopup0 );
	
	TMCCountedPtr<IMFPart> node1;
	inPart->FindChildPartByID(&node1, eNode1);
	MY_ASSERT(node1);
	TMCCountedPtr<IMFPart> popup1;
	node1->FindChildPartByID(&popup1, ePopup1);
	MY_ASSERT(popup1);
	TMCCountedPtr<IMFTextPopupPart> shadingDomainPopup1;
	popup1->QueryInterface( IID_IMFTextPopupPart, (void **)&shadingDomainPopup1 );
	
	TMCCountedPtr<IMFPart> node2;
	inPart->FindChildPartByID(&node2, eNode2);
	MY_ASSERT(node2);
	TMCCountedPtr<IMFPart> popup2;
	node2->FindChildPartByID(&popup2, ePopup2);
	MY_ASSERT(popup2);
	TMCCountedPtr<IMFTextPopupPart> shadingDomainPopup2;
	popup2->QueryInterface( IID_IMFTextPopupPart, (void **)&shadingDomainPopup2 );	

	TMCCountedPtr<IMFPart> node3;
	inPart->FindChildPartByID(&node3, eNode3);
	MY_ASSERT(node3);
	TMCCountedPtr<IMFPart> popup3;
	node3->FindChildPartByID(&popup3, ePopup3);
	MY_ASSERT(popup3);
	TMCCountedPtr<IMFTextPopupPart> shadingDomainPopup3;
	popup3->QueryInterface( IID_IMFTextPopupPart, (void **)&shadingDomainPopup3 );	

	BuildingPrim* buildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();
	if(!fDomainPopupValid)
	{
		BuildShadingDomainPopupMenu(shadingDomainPopup0, buildingPrimitive, kShadingDomainID0);
		BuildShadingDomainPopupMenu(shadingDomainPopup1, buildingPrimitive, kShadingDomainID1);
		BuildShadingDomainPopupMenu(shadingDomainPopup2, buildingPrimitive, kShadingDomainID2);
		BuildShadingDomainPopupMenu(shadingDomainPopup3, buildingPrimitive, kShadingDomainID3);
		fDomainPopupValid = true;
	}

	node0->SetShown(false, false);
	node1->SetShown(false, false);
	node2->SetShown(false, false);
	node3->SetShown(false, false);

	if(status->fHasSelection)
	{		
		if( !status->fSelectedWallObjectCount &&
			!status->fSelectedRoomObjectCount )
		{
			// If there's only rooms, roofs or walls selected, display the domains of the selection

			const int32 roofCount = status->fSelectedRoofCount;
			const int32 roomCount = status->fSelectedRoomCount;
			const int32 wallCount = status->fSelectedWallGlobalCount;

			if(roofCount && roomCount==0 && wallCount==0)
			{	// Selection countains only roofs
				const int32 shadingDomainCount = buildingPrimitive->GetUVSpaceCount();
				DisplayShadingDomainPopupMenu(shadingDomainPopup0, status->fDomainRoofOutTop, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup1, status->fDomainRoofOutMid, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup2, status->fDomainRoofOutBot, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup3, status->fDomainRoofInside, shadingDomainCount);
				node0->SetShown(true, false);
				node1->SetShown(true, false);
				node2->SetShown(true, false);
				node3->SetShown(true, false);
	
				TMCString31 str;

				gResourceUtilities->GetIndString( str, kModelerStrings, 30);
				TMCCountedPtr<IMFPart> name0;
				node0->FindChildPartByID(&name0, eText0);
				MY_ASSERT(name0);
				name0->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 31);
				TMCCountedPtr<IMFPart> name1;
				node1->FindChildPartByID(&name1, eText1);
				MY_ASSERT(name1);
				name1->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 32);
				TMCCountedPtr<IMFPart> name2;
				node2->FindChildPartByID(&name2, eText2);
				MY_ASSERT(name2);
				name2->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 33);
				TMCCountedPtr<IMFPart> name3;
				node3->FindChildPartByID(&name3, eText3);
				MY_ASSERT(name3);
				name3->SetValue(&str, kStringValueType, true, true);
			}
			else if(roofCount==0 && roomCount && wallCount==0)
			{	// Selection countains only rooms
				const int32 shadingDomainCount = buildingPrimitive->GetUVSpaceCount();
				DisplayShadingDomainPopupMenu(shadingDomainPopup0, status->fDomainRoomFloor, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup1, status->fDomainRoomCeiling, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup2, status->fDomainRoomWalls, shadingDomainCount);
				node0->SetShown(true, false);
				node1->SetShown(true, false);
				node2->SetShown(true, false);
	
				TMCString31 str;

				gResourceUtilities->GetIndString( str, kModelerStrings, 27);
				TMCCountedPtr<IMFPart> name0;
				node0->FindChildPartByID(&name0, eText0);
				MY_ASSERT(name0);
				name0->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 28);
				TMCCountedPtr<IMFPart> name1;
				node1->FindChildPartByID(&name1, eText1);
				MY_ASSERT(name1);
				name1->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 29);
				TMCCountedPtr<IMFPart> name2;
				node2->FindChildPartByID(&name2, eText2);
				MY_ASSERT(name2);
				name2->SetValue(&str, kStringValueType, true, true);
			}
			else if(roofCount==0 && roomCount==0 && wallCount)
			{	// Selection countains only walls
				const int32 shadingDomainCount = buildingPrimitive->GetUVSpaceCount();
				DisplayShadingDomainPopupMenu(shadingDomainPopup0, status->fDomainWallLeft, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup1, status->fDomainWallRight, shadingDomainCount);
				DisplayShadingDomainPopupMenu(shadingDomainPopup2, status->fDomainWallBetween, shadingDomainCount);
				node0->SetShown(true, false);
				node1->SetShown(true, false);
				node2->SetShown(true, false);
	
				TMCString31 str;

				gResourceUtilities->GetIndString( str, kModelerStrings, 24);
				TMCCountedPtr<IMFPart> name0;
				node0->FindChildPartByID(&name0, eText0);
				MY_ASSERT(name0);
				name0->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 25);
				TMCCountedPtr<IMFPart> name1;
				node1->FindChildPartByID(&name1, eText1);
				MY_ASSERT(name1);
				name1->SetValue(&str, kStringValueType, true, true);

				gResourceUtilities->GetIndString( str, kModelerStrings, 33);
				TMCCountedPtr<IMFPart> name2;
				node2->FindChildPartByID(&name2, eText2);
				MY_ASSERT(name2);
				name2->SetValue(&str, kStringValueType, true, true);
			}
		}
	}

	// I got to do this to have an update
/*	TMCCountedPtr<IMFPart> stack;
	inPart->FindChildPartByID(&stack, 'Stac');
	MY_ASSERT(stack);
	TMCRect bounds;
	stack->GetBoundsInParent(bounds);
	bounds.bottom+=1;
	stack->SetBoundsInParent(bounds, true, true);
	fBuildingModeler->PostToSelectionChannel(0);*/
}

void BuildingPropertiesClient::LoadLevelProperties(IMFPart* inPart, PrimitiveStatus* status)
{
	// First display the active level
//	FillInActiveLevelPopup(inPart);
	int32 currentLevel = fBuildingModeler->Get2DActiveLevel();
	BuildingPrim* buildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();
	if(currentLevel<0)
	{
		currentLevel = buildingPrimitive->GetGroundLevelIndex();
		buildingPrimitive->ActiveLevel(currentLevel);
		fBuildingModeler->Invalidate2DCache();
	}

	// Then display the properties of the current level
	Level* level = buildingPrimitive->GetLevel(currentLevel);

	TMCCountedPtr<IMFPart> levelNamePart;
	inPart->FindChildPartByID(&levelNamePart, eLevelName);
	MY_ASSERT(levelNamePart);

	levelNamePart->SetValue(&level->GetName(), kStringValueType, true, true);

	TMCCountedPtr<IMFPart> levelHeightPart;
	inPart->FindChildPartByID(&levelHeightPart, eLevelCustomHeight);
	MY_ASSERT(levelHeightPart);

	const real32 height = level->GetLevelHeight();
	levelHeightPart->SetValue(&height, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> levelHeightSwitch;
	inPart->FindChildPartByID(&levelHeightSwitch, eLevelAutoHeight);
	MY_ASSERT(levelHeightSwitch);

	int32 option = 'Opt1';
	if( height != buildingPrimitive->GetData().GetDefaultLevelHeight() )
		option = 'Opt2';
	levelHeightSwitch->SetValue(&option, kInt32ValueType, true, true);

	// Active level display
	TMCCountedPtr<IMFPart> levelShownSwitch;
	inPart->FindChildPartByID(&levelShownSwitch, eDisplayActiveLevel);
	MY_ASSERT(levelShownSwitch);

	const int32 levelShown = buildingPrimitive->ShowAll()?'Opt1':'Opt2';
	levelShownSwitch->SetValue(&levelShown, kInt32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultLevelProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defLevelHeightPart;
	inPart->FindChildPartByID(&defLevelHeightPart, eDLevelHeight);
	MY_ASSERT(defLevelHeightPart);

	const real32 height = data.GetDefaultLevelHeight();
	defLevelHeightPart->SetValue(&height, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultRoofProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defRoofMinPart;
	inPart->FindChildPartByID(&defRoofMinPart, eDRoofMin);
	MY_ASSERT(defRoofMinPart);

	const real32 min = data.GetDefaultRoofMin();
	defRoofMinPart->SetValue(&min, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defRoofMaxPart;
	inPart->FindChildPartByID(&defRoofMaxPart, eDRoofMax);
	MY_ASSERT(defRoofMaxPart);

	const real32 max = data.GetDefaultRoofMax();
	defRoofMaxPart->SetValue(&max, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultWallProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defWallThickPart;
	inPart->FindChildPartByID(&defWallThickPart, eDWallThickness);
	MY_ASSERT(defWallThickPart);

	const real32 thick = data.GetDefaultWallThickness();
	defWallThickPart->SetValue(&thick, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultRoomProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defFloorThickPart;
	inPart->FindChildPartByID(&defFloorThickPart, eDFloorThickness);
	MY_ASSERT(defFloorThickPart);

	const real32 floorThick = data.GetDefaultFloorThickness();
	defFloorThickPart->SetValue(&floorThick, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defCeilingThickPart;
	inPart->FindChildPartByID(&defCeilingThickPart, eDCeilingThickness);
	MY_ASSERT(defCeilingThickPart);

	const real32 ceilingThick = data.GetDefaultCeilingThickness();
	defCeilingThickPart->SetValue(&ceilingThick, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultWindowProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defWindowHeightPart;
	inPart->FindChildPartByID(&defWindowHeightPart, eDWindowHeight);
	MY_ASSERT(defWindowHeightPart);

	const real32 height = data.GetDefaultWindowHeight();
	defWindowHeightPart->SetValue(&height, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defWindowLengthPart;
	inPart->FindChildPartByID(&defWindowLengthPart, eDWindowLength);
	MY_ASSERT(defWindowLengthPart);

	const real32 length = data.GetDefaultWindowLength();
	defWindowLengthPart->SetValue(&length, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defWindowAltitudePart;
	inPart->FindChildPartByID(&defWindowAltitudePart, eDWindowAltitude);
	MY_ASSERT(defWindowAltitudePart);

	const real32 altitude = data.GetDefaultWindowAltitude();
	defWindowAltitudePart->SetValue(&altitude, kReal32ValueType, true, true);

}

void BuildingPropertiesClient::LoadDefaultDoorProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defDoorHeightPart;
	inPart->FindChildPartByID(&defDoorHeightPart, eDDoorHeight);
	MY_ASSERT(defDoorHeightPart);

	const real32 height = data.GetDefaultDoorHeight();
	defDoorHeightPart->SetValue(&height, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defDoorLengthPart;
	inPart->FindChildPartByID(&defDoorLengthPart, eDDoorLength);
	MY_ASSERT(defDoorLengthPart);

	const real32 length = data.GetDefaultDoorLength();
	defDoorLengthPart->SetValue(&length, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultStairwayProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	TMCCountedPtr<IMFPart> defStairwayWidthPart;
	inPart->FindChildPartByID(&defStairwayWidthPart, eDStairwayWidth);
	MY_ASSERT(defStairwayWidthPart);

	const real32 width = data.GetDefaultStairwayWidth();
	defStairwayWidthPart->SetValue(&width, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defStairwayLengthPart;
	inPart->FindChildPartByID(&defStairwayLengthPart, eDStairwayLength);
	MY_ASSERT(defStairwayLengthPart);

	const real32 length = data.GetDefaultStairwayLength();
	defStairwayLengthPart->SetValue(&length, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::LoadDefaultWorkingBoxProperties(IMFPart* inPart, const BuildingPrimData& data)
{
	// Grid
	TMCCountedPtr<IMFPart> defGridSpacingPart;
	inPart->FindChildPartByID(&defGridSpacingPart, eDGridSpacing);
	MY_ASSERT(defGridSpacingPart);
	const real32 spac = data.GetDefaultGridSpacing();
	defGridSpacingPart->SetValue(&spac, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defWBSizeX;
	inPart->FindChildPartByID(&defWBSizeX, eDWBSizeX);
	MY_ASSERT(defWBSizeX);
	const real32 sizeX = data.GetDefaultWBSizeX();
	defWBSizeX->SetValue(&sizeX, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defWBSizeY;
	inPart->FindChildPartByID(&defWBSizeY, eDWBSizeY);
	MY_ASSERT(defWBSizeY);
	const real32 sizeY = data.GetDefaultWBSizeY();
	defWBSizeY->SetValue(&sizeY, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defWBSizeZ;
	inPart->FindChildPartByID(&defWBSizeZ, eDWBSizeZ);
	MY_ASSERT(defWBSizeZ);
	const real32 sizeZ = data.GetDefaultWBSizeZ();
	defWBSizeZ->SetValue(&sizeZ, kReal32ValueType, true, true);

	// Movement
	TMCCountedPtr<IMFPart> defSnapPrecision;
	inPart->FindChildPartByID(&defSnapPrecision, eDSnapPrecision);
	MY_ASSERT(defSnapPrecision);
	const real32 snapPrec = data.GetDefaultSnapPrecision();
	defSnapPrecision->SetValue(&snapPrec, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defConstrainAngle;
	inPart->FindChildPartByID(&defConstrainAngle, eDConstrainAngle);
	MY_ASSERT(defConstrainAngle);
	const real32 constAngle = data.GetDefaultConstrainAngle();
	defConstrainAngle->SetValue(&constAngle, kReal32ValueType, true, true);

	TMCCountedPtr<IMFPart> defSnap;
	inPart->FindChildPartByID(&defSnap, eDSnap);
	MY_ASSERT(defSnap);
	const boolean snap = data.GetDefaultSnap();
	defSnap->SetValue(&snap, kBooleanValueType, true, true);
}

void BuildingPropertiesClient::LoadDomainList(IMFPart* inPart, BuildingPrim* buildingPrimitive)
{
	// List part
	inPart->FindChildPartByID(&fDomainListPart, eUVDomainListPartID);
	ThrowIfNil(fDomainListPart);

	// Name Part
	inPart->FindChildPartByID(&fDomainNamePart, eNameDomainPartID);
	ThrowIfNil(fDomainNamePart);

	InitDomainList(buildingPrimitive, fDomainListPart, fDomainList);
	// Display the selected cell name
	TMCDynamicString string;
	fDomainListPart->GetValue( (void*)&string, kStringValueType );
	fDomainNamePart->SetValue( (void*)&string, kStringValueType, true, false );
	// remember its index
	TMCCountedPtr<IMFListPart> listPart;
	fDomainListPart->QueryInterface(IID_IMFListPart, (void **)&listPart);
	ThrowIfNil(listPart);
	listPart->GetSelectedCellRange(fSelectedCell, fSelectedCell);
	MY_ASSERT(fSelectedCell<(int32)fDomainList.GetElemCount());
}

void BuildingPropertiesClient::LoadBackdropData(IMFPart* inPart, BuildingPrim* buildingPrimitive)
{
	const BuildingPrimData& data = buildingPrimitive->Data();
	// Front Back
//	if(data.GetFBBackdrop().Length()) // If no name, need to set it empty
	{
		TMCCountedPtr<IMFPart> namePart;
		inPart->FindChildPartByID(&namePart, eImageFrontBackPartID);
		ThrowIfNil(namePart);
		namePart->SetValue(&data.GetFBBackdrop(), kStringValueType, true, false);

		TMCCountedPtr<IMFPart> enablePart;
		inPart->FindChildPartByID(&enablePart, eEnableFrontBackPartID);
		ThrowIfNil(enablePart);
		const boolean enable = data.GetFBEnable();
		enablePart->SetValue(&enable, kBooleanValueType, true, false);
	}
	// Left Right
//	if(data.GetLRBackdrop().Length()) // If no name, need to set it empty
	{
		TMCCountedPtr<IMFPart> namePart;
		inPart->FindChildPartByID(&namePart, eImageLeftRightPartID);
		ThrowIfNil(namePart);
		namePart->SetValue(&data.GetLRBackdrop(), kStringValueType, true, false);

		TMCCountedPtr<IMFPart> enablePart;
		inPart->FindChildPartByID(&enablePart, eEnableLeftRightPartID);
		ThrowIfNil(enablePart);
		const boolean enable = data.GetLREnable();
		enablePart->SetValue(&enable, kBooleanValueType, true, false);
	}
	// Top Bottom
//	if(data.GetTBBackdrop().Length()) // If no name, need to set it empty
	{
		TMCCountedPtr<IMFPart> namePart;
		inPart->FindChildPartByID(&namePart, eImageTopBottomPartID);
		ThrowIfNil(namePart);
		namePart->SetValue(&data.GetTBBackdrop(), kStringValueType, true, false);

		TMCCountedPtr<IMFPart> enablePart;
		inPart->FindChildPartByID(&enablePart, eEnableTopBottomPartID);
		ThrowIfNil(enablePart);
		const boolean enable = data.GetTBEnable();
		enablePart->SetValue(&enable, kBooleanValueType, true, false);
	}
}

void BuildingPropertiesClient::DisplayPositionInPart(const TVector2& position, 
													IMFPart* inPart,
													const int32 xId,
													const int32 yId )
{
	TMCCountedPtr<IMFPart> positionText;
	// Display X
	inPart->FindChildPartByID(&positionText, xId);
	ThrowIfNil(positionText);
	positionText->SetValue(&position.x, kReal32ValueType, true, true);
	// Display Y
	inPart->FindChildPartByID(&positionText, yId);
	ThrowIfNil(positionText);
	positionText->SetValue(&position.y, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::DisplayValueInPart(const real32 value, 
												IMFPart* inParentPart,
												const int32 partID )
{
	TMCCountedPtr<IMFPart> valuePart;
	inParentPart->FindChildPartByID(&valuePart, partID);
	ThrowIfNil(valuePart);
	if(value==kMultiVecField)
	{
		TMCString15 diff("diff");
		valuePart->SetValue(&diff, kStringValueType, true, true);
	}
	else
		valuePart->SetValue(&value, kReal32ValueType, true, true);
}

void BuildingPropertiesClient::DisplayValueInPart(const int32 value, 
												IMFPart* inParentPart,
												const int32 partID )
{
	TMCCountedPtr<IMFPart> valuePart;
	inParentPart->FindChildPartByID(&valuePart, partID);
	ThrowIfNil(valuePart);
	if(value==kMultipleValues)
	{
		TMCString15 diff("diff");
		valuePart->SetValue(&diff, kStringValueType, true, true);
	}
	else
		valuePart->SetValue(&value, kInt32ValueType, true, true);
}

/*
void BuildingPropertiesClient::FillInActiveLevelPopup(IMFPart*	inPart)
{
	TMCCountedPtr<IMFPart> popupPart;
	inPart->FindChildPartByID( &popupPart, eActiveLevel );
	if(!popupPart)
		return;

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCCountedPtr<IMFTextPopupPart> menuPopup;
	popupPart->QueryInterface( IID_IMFTextPopupPart, (void **)&menuPopup );	

	// Clean the current popup menu
	menuPopup->RemoveAll();

	// Fill the menu with existing levels
	BuildingPrim* buildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();

	const int32 groundIndex = buildingPrimitive->GetGroundLevelIndex();
	const int32 levelCount = buildingPrimitive->GetLevelCount();
	TMCString15 level;
	gResourceUtilities->GetIndString( level, kModelerStrings, 11);
	TMCString15 basement;
	gResourceUtilities->GetIndString( basement, kModelerStrings, 12);
	for(int32 iLevel=levelCount-1 ; iLevel>=0 ; iLevel--)
	{
		TMCDynamicString name;
		if(iLevel>=groundIndex)
		{
			TMCString15 number;
			number.FromInt32(iLevel-groundIndex+1);
			name = level+number;
		}
		else
		{
			TMCString15 number;
			number.FromInt32(groundIndex-iLevel);
			name = basement+number;
		}
		const int32 item = levelCount-1-iLevel;
		menuPopup->AppendMenuItem(name);
		menuPopup->SetItemEnabled(item, true); // it's a custom menu: enable manualy the items
		menuPopup->SetItemActionNumber(item, kActiveLevelAction+iLevel);
	}

	// Select the current active level
	const int32 currentLevel = fBuildingModeler->Get2DActiveLevel();
	if(currentLevel>=0)
		menuPopup->SetSelectedItem(levelCount-1-currentLevel, false);
	else
	{
		fBuildingModeler->SetActiveLevel(groundIndex);
		menuPopup->SetSelectedItem(groundIndex, false);
	}
}
*/
void BuildingPropertiesClient::EditSelectionPosition(IMFPart* part, int32 toolID,PrimitiveStatus* status)
{
	real32 value=0;
	part->GetValue( (void*)&value, kReal32ValueType);
	switch(toolID)
	{
		case eMoveX:
		{
			TVector3 bboxCenter;
			status->fSelectionBBox.GetCenter(bboxCenter);
			const real32 curX = bboxCenter.x;
			if(curX==value) return;
			const TVector2 offset(value-curX, kRealZero);
			MenuAction::MovePoint(fBuildingModeler, offset, NULL);
		} break;
		case eMoveY:
		{
			TVector3 bboxCenter;
			status->fSelectionBBox.GetCenter(bboxCenter);
			const real32 curY = bboxCenter.y;
			if(curY==value) return;
			const TVector2 offset(kRealZero, value-curY);
			MenuAction::MovePoint(fBuildingModeler, offset, NULL);
		} break;
		case eOffsetX:
		{
			if(value==0) return;
			const TVector2 offset(value, kRealZero);
			MenuAction::MovePoint(fBuildingModeler, offset, NULL);
			value = kRealZero;
		} break;
		case eOffsetY:
		{
			if(value==0) return;
			const TVector2 offset(kRealZero, value);
			MenuAction::MovePoint(fBuildingModeler, offset, NULL);
			value = kRealZero;
		} break;
		case eScaleX:
		{
			const TVector2 scale(value*0.01f, kRealOne);
			MenuAction::ScaleSelection(fBuildingModeler, scale);
			MenuAction::ScaleObject(fBuildingModeler, scale);
			value = 100;
		} break;
		case eScaleY:
		{
			const TVector2 scale(kRealOne, value*0.01f);
			MenuAction::ScaleSelection(fBuildingModeler, scale);
			MenuAction::ScaleObject(fBuildingModeler, scale);
			value = 100;
		} break;
		case eGlobalScale:
		{
			value*=0.01f;
			TVector2 scale(value, value);
			MenuAction::ScaleSelection(fBuildingModeler, scale);
			MenuAction::ScaleObject(fBuildingModeler, scale);
			value = 100;
		} break;
		case eSizeX:
		{
			TVector3 delta;
			status->fSelectionBBox.GetDelta(delta);
			const real32 curX = delta.x;
			if(RealAbs(curX)>kRealEpsilon)
			{
				MenuAction::ScaleSelection(fBuildingModeler, TVector2(value/curX, kRealOne));
			}
			else
			{
				MenuAction::ScaleSelection(fBuildingModeler, TVector2(0, kRealOne));
			}
		} break;
		case eSizeY:
		{
			TVector3 delta;
			status->fSelectionBBox.GetDelta(delta);
			const real32 curY = delta.y;
			if(RealAbs(curY)>kRealEpsilon)
			{
				MenuAction::ScaleSelection(fBuildingModeler, TVector2(kRealOne, value/curY));
			}
			else
			{
				MenuAction::ScaleSelection(fBuildingModeler, TVector2(kRealOne, 0));
			}
		} break;
		case eRotateAngle:
		{
			if(value==0) return;
			TVector2 cosSin;
			RealSinCos( value*kRealRadToDeg, cosSin.x, cosSin.y );
			MenuAction::RotateSelection(fBuildingModeler, cosSin );
			value = kRealZero;
		} break;
	}
	// Display default value after performing the action
	part->SetValue( (void*)&value, kReal32ValueType, true, false);
}

#if (VERSIONNUMBER >= 0x040000)
boolean MCCOMAPI BuildingPropertiesClient::HandlePageHit(
											IMFPart*			inTopPart,
											int32				inMessage,
											IMFResponder*		inSource,
											void*				inData)
#else
boolean MCCOMAPI BuildingPropertiesClient::HandlePageHit(
											IMFPart*			inTopPart,
											ISceneSelection*	inSelection,
											int32				inMessage,
											IMFResponder*		inSource,
											void*				inData)
#endif
{
	switch(inMessage)
	{
	case EMFPartMessage::kMsg_BeginPartTracking:
		{
		}
		break;
		
	case EMFPartMessage::kMsg_ContinuePartTracking:
		{
		}
		break;

	case EMFPartMessage::kMsg_OutOfScopePartValueChanged:
		{
			TMCCountedPtr<IMFPart> part;
			inSource->QueryInterface(IID_IMFPart, (void **)&part);
			ThrowIfNil(part);
			IDType id=part->GetIMFPartID();

			BuildingPrim* buildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();
			PrimitiveStatus* status = buildingPrimitive->GetStatus();
			
			int32 index = 0; // Use when move a point
			int32 axis = 0; // Use when move a point

			int32 objAxis=-1;
			int32 scale=0;

			switch(id)
			{
			case eXDim: scale=1;
			case eXCenter: objAxis=0; break;
			case eYDim: scale=1;
			case eYCenter: objAxis=1; break;
			case eXPos1:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TVector2 delta=TVector2::kZero;
					delta[0] = value-status->fFirstSelectedPos[0];

					boolean result = MenuAction::MovePoint(fBuildingModeler, delta, 
						buildingPrimitive->GetFirstSelectedPoint(kAllLevels));
				} break;
			case eYPos1:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TVector2 delta=TVector2::kZero;
					delta[1] = value-status->fFirstSelectedPos[1];

					boolean result = MenuAction::MovePoint(fBuildingModeler, delta,
						buildingPrimitive->GetFirstSelectedPoint(kAllLevels));
				} break;
			case eXPos2:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TVector2 delta=TVector2::kZero;
					delta[0] = value-status->fSecondSelectedPos[0];

					boolean result = MenuAction::MovePoint(fBuildingModeler, delta,
						buildingPrimitive->GetSecondSelectedPoint(kAllLevels));
				} break;
			case eYPos2:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TVector2 delta=TVector2::kZero;
					delta[1] = value-status->fSecondSelectedPos[1];

					boolean result = MenuAction::MovePoint(fBuildingModeler, delta,
						buildingPrimitive->GetSecondSelectedPoint(kAllLevels));
				} break;
			case eMoveX:
			case eMoveY:
			case eOffsetX:
			case eOffsetY:
			case eScaleX:
			case eScaleY:
			case eScaleZ:
			case eGlobalScale:
			case eSizeX:
			case eSizeY:
			case eSizeZ:
			case eRotateAngle:
				{
					EditSelectionPosition(part, id, status);
				} break;
			case eSmoothBox:
				{	// Smooth/unsmooth the selected points on the holes
					boolean value;
					part->GetValue(&value,kBooleanValueType);

					MenuAction::SmoothSelection(fBuildingModeler,value);
				} break;
			case eSelectionName:
				{
					TMCDynamicString name;
					part->GetValue(&name,kStringValueType);

					MenuAction::SetSelectionName(fBuildingModeler,name);
				} break;
			case eLevelName:
				{
					TMCDynamicString name;
					part->GetValue(&name,kStringValueType);

					MenuAction::SetLevelName(fBuildingModeler, buildingPrimitive->ActiveLevel(),name);
				} break;
			case eDisplayActiveLevel:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					// Since the selection can be modified, we need to post an action
					if(option == 'Opt1')
					{	// Show all
						MenuAction::ShowActiveLevel(fBuildingModeler,kAllLevels);
					}
					else
					{	// Show current level only
						MenuAction::ShowActiveLevel(fBuildingModeler,buildingPrimitive->ActiveLevel());
					}
				} break;
			case eDisplayLevelUp:
				{
					const int32 currentLevel = fBuildingModeler->Get2DActiveLevel();
					const int32 levelCount = fBuildingModeler->GetBuildingNoAddRef()->GetLevelCount();
				
					if(currentLevel<levelCount-1)
					{
						const int32 levelIndex = currentLevel+1;

						if(buildingPrimitive->ShowAll())
						{
							// No need to post an action, the database is not modifyed
							buildingPrimitive->ActiveLevel(levelIndex);
							fBuildingModeler->Invalidate2DCache();
							fBuildingModeler->InvalidatePropertiesModule(); // display the level height and name
						}
						else
						{
							// If the 3D view display only 1 level too, we need to post an action to remove a possible selection
							MenuAction::ShowActiveLevel(fBuildingModeler,levelIndex);
						}
		
						fBuildingModeler->ImmediateUpdate(false,false);
					}
				} break;	
			case eDisplayLevelDown:
				{
					const int32 currentLevel = fBuildingModeler->Get2DActiveLevel();
				
					if(currentLevel>0)
					{
						const int32 levelIndex = currentLevel-1;

						if(buildingPrimitive->ShowAll())
						{
							// No need to post an action, the database is not modifyed
							buildingPrimitive->ActiveLevel(levelIndex);
							fBuildingModeler->Invalidate2DCache();
							fBuildingModeler->InvalidatePropertiesModule(); // display the level height and name
						}
						else
						{
							// If the 3D view display only 1 level too, we need to post an action to remove a possible selection
							MenuAction::ShowActiveLevel(fBuildingModeler,levelIndex);
						}
		
						fBuildingModeler->ImmediateUpdate(false,false);
					}
				} break;	
				/*	popup menu answer
			case eActiveLevel:
				{
					int32 item=0;
					part->GetValue(&item,kInt32ValueType);

					// No need to post an action, the database is not modifyed
					const int32 levelCount = fBuildingModeler->GetBuildingNoAddRef()->GetLevelCount();
					const int32 levelIndex = levelCount-item-1;
					fBuildingModeler->SetActiveLevel(levelIndex);

					// If the 3D view display only 1 level too, we need to post an action to remove a possible selection
					if(buildingPrimitive->GetShownLevel()!=kAllLevels)
					{
						MenuAction::ShowActiveLevel(fBuildingModeler,levelIndex);
					}
	
					fBuildingModeler->ImmediateUpdate(false,false);
				} break;*/
			case eLevelCustomHeight:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TMCArray<int32> level;
					level.AddElem(fBuildingModeler->Get2DActiveLevel());
					MenuAction::SetLevelHeight(fBuildingModeler,level,value);
				} break;
			case eLevelAutoHeight:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == 'Opt1')
					{ // User switched back to auto mode : restore default value
						const real32 defaultHeight = buildingPrimitive->GetData().GetDefaultLevelHeight();
						TMCArray<int32> level;
						level.AddElem(fBuildingModeler->Get2DActiveLevel());
						MenuAction::SetLevelHeight(fBuildingModeler,level,defaultHeight);
					}
				} break;
			case eFloorCustomThickness:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value, eFloorThickness);
				} break;
			case eFloorAutoThickness:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == 'Opt1')
					{ // User switched back to auto mode : restore default value
						const real32 defaultThickness = buildingPrimitive->GetData().GetDefaultFloorThickness();
						MenuAction::SetDimension(fBuildingModeler,defaultThickness, eFloorThickness);
					}
				} break;
			case eCeilingCustomThickness:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value, eCeilingThickness);
				} break;
			case eDrawCeiling:
				{
					boolean value=0;
					part->GetValue(&value,kBooleanValueType);

					MenuAction::SetSelectionFlag(fBuildingModeler,!value, eNoCeilingFlag);
				} break;
			case eCeilingAutoThickness:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == 'Opt1')
					{ // User switched back to auto mode : restore default value
						const real32 defaultThickness = buildingPrimitive->GetData().GetDefaultFloorThickness();
						MenuAction::SetDimension(fBuildingModeler,defaultThickness, eCeilingThickness);
					}
				} break;
			case eWallCustomHeight:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eWallHeight);
				} break;
			case eWallAutoHeight:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == 'Opt1')
					{ // User switched back to auto mode : restore default value
						const real32 defaultHeight = buildingPrimitive->GetData().GetDefaultWallHeight();
						MenuAction::SetDimension(fBuildingModeler,defaultHeight,eWallHeight);
					}
				} break;
			case eWallCustomThickness:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eWallThickness);
				} break;
			case eWallAutoThickness:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == 'Opt1')
					{ // User switched back to auto mode : restore default value
						const real32 defaultThickness = buildingPrimitive->GetData().GetDefaultWallThickness();
						MenuAction::SetDimension(fBuildingModeler,defaultThickness,eWallThickness);
					}
				} break;
			case eWallOption:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					const boolean extraHeight = (option == 'Opt2');

					MenuAction::SetSelectionFlag(fBuildingModeler, extraHeight, eWallExtraHeightFlag);
				} break;
			case eWallArcOffset:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eWallOffset);
				} break;
			case eWallArcSegments:
				{
					int32 value=0;
					part->GetValue(&value,kInt32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eWallSegments);
				} break;
			case eWallCrenelWidth:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eCrenelWidth);
				} break;
			case eWallCrenelHeight:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eCrenelHeight);
				} break;
			case eWallCrenelSpacing:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eCrenelSpacing);
				} break;
			case eWallCrenelOffset:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eCrenelOffset);
				} break;
			case eWallCrenelShape:
				{
					int32 value=0;
					part->GetValue(&value,kInt32ValueType);

					MenuAction::SetUInt32Value(fBuildingModeler,(uint32)value,eCrenelShape);
				} break;
			case eRoofCustomHeight:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eRoofMax);
				} break;
			case eRoofAutoHeight:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == eOption1)
					{ // User switched back to auto mode : restore default value
						const real32 defaultHeight = buildingPrimitive->GetData().GetDefaultRoofMax();
						MenuAction::SetDimension(fBuildingModeler,defaultHeight,eRoofMax);
					}
				} break;
			case eRoofCustomBase:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					MenuAction::SetDimension(fBuildingModeler,value,eRoofMin);
				} break;
			case eRoofAutoBase:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					if(option == eOption1)
					{ // User switched back to auto mode : restore default value
						const real32 defaultBase = buildingPrimitive->GetData().GetDefaultRoofMin();
						MenuAction::SetDimension(fBuildingModeler,defaultBase,eRoofMin);
					}
				} break;
			case eRoofTopShape:
			case eRoofBottomShape:
			case eRoofTopInside:
			case eRoofBottomInside:
				{
					int32 option=0;
					part->GetValue(&option,kInt32ValueType);

					ERoofProfileID profile = eShape0;

					switch(option)
					{
					case eOption0: profile = eShape0; break;
					case eOption1: profile = eShape1; break;
					case eOption2: profile = eShape2; break;
					case eOption3: profile = eShape3; break;
					case eOption4: profile = eShape4; break;
					case eOption5: profile = eShape5; break;
					case eOption6: profile = eShape6; break;
					case eOption7: profile = eShape7; break;
					case eOption8: profile = eShape8; break;
					case eOption9: profile = eShape9; break;
					}

					const boolean top = (id==eRoofTopShape||id==eRoofTopInside)?true:false;
					const boolean in = (id==eRoofBottomInside||id==eRoofTopInside)?true:false;
					part->SetValue(&eNoOption,kInt32ValueType,false,false);
					
					MenuAction::SetRoofProfile(fBuildingModeler,profile,top,in);
				} break;

			case eDelLevelPartID:
				{
					// Remove the active level from the database
					MenuAction::DeleteLevel(fBuildingModeler, fBuildingModeler->Get2DActiveLevel());
				} break;

			case eChildrenList:
				{	// Object children
					int32 itemIndex=0;
					part->GetValue(&itemIndex,kInt32ValueType);

					TMCDynamicString objectName;
					part->GetValue(&objectName,kStringValueType);

					MenuAction::SetObjectInstance(fBuildingModeler,objectName, itemIndex<fObjectCount);
				} break;
			case eAutoFlip:
				{
					boolean value=0;
					part->GetValue(&value,kBooleanValueType);

					MenuAction::SetSelectionFlag(fBuildingModeler,value, eAutoFlipObjFlag);
				} break;
			case eChildPlacement:
				{	// Object children placement type
					int32 option;
					part->GetValue(&option,kInt32ValueType);
					EPlacementType placement = eFitIn;
					switch(option)
					{
					case 'Opt1':placement = eBigger; break;
					case 'Opt2':placement = eSlightlyBigger; break;
					case 'Opt3':placement = eFitIn;  break;
					case 'Opt4':placement = eSlightlySmaller;  break;
					case 'Opt5':placement = eSmaller;  break;
					case 'Opt6':placement = eCustom;  break;
					case 'Opt7':placement = eFreePlacement;  break;
					}

					MenuAction::SetObjectPlacementType(fBuildingModeler,placement);
				} break;
			case ePlacementOffsetX:
			case ePlacementOffsetY:
			case ePlacementOffsetZ:
			case ePlacementScaleX:
			case ePlacementScaleY:
			case ePlacementScaleZ:
			case ePlacementRotateX:
			case ePlacementRotateY:
			case ePlacementRotateZ:
				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					MenuAction::SetObjectPlacement(fBuildingModeler, value, id);
				} break;
			case eDLevelHeight:
			case eDRoofMin:
			case eDRoofMax:
			case eDWallThickness:
			case eDFloorThickness:
			case eDCeilingThickness:
			case eDWindowHeight:
			case eDWindowLength:
			case eDWindowAltitude:
			case eDDoorHeight:
			case eDDoorLength:
			case eDStairwayWidth:
			case eDStairwayLength:
				{	// Post an action, all the level using the default height are going to be modified
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					MenuAction::SetDefaultSetting(fBuildingModeler, value, id);
				}break;
			case eDGridSpacing:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultGridSpacing(value);
					fBuildingModeler->DefaultWorkingBox(); // Rebuild the working box
					fBuildingModeler->RegularUpdate();
				} break;
			case eDWBSizeX:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultWBSizeX(value);
					fBuildingModeler->DefaultWorkingBox(); // Rebuild the working box
					fBuildingModeler->RegularUpdate();
				} break;
			case eDWBSizeY:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultWBSizeY(value);
					fBuildingModeler->DefaultWorkingBox(); // Rebuild the working box
					fBuildingModeler->RegularUpdate();
				} break;
			case eDWBSizeZ:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultWBSizeZ(value);
					fBuildingModeler->DefaultWorkingBox(); // Rebuild the working box
					fBuildingModeler->RegularUpdate();
				} break;
			case eDSnapPrecision:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultSnapPrecision(value);
				} break;
			case eDConstrainAngle:
				{	// No action, just set the pmap value and redraw the working box
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);
					buildingPrimitive->GetData().SetDefaultConstrainAngle(value);
				} break;
			case eDSnap:
				{	// No action, just set the pmap value and redraw the working box
					boolean value=0;
					part->GetValue(&value,kBooleanValueType);
					buildingPrimitive->GetData().SetDefaultSnap(value);
				} break;
			case eDDimPrefs:
				{	// Set the current primitive global dimension as prefs
					fBuildingModeler->PrefsFromPrimitiveDimension();
				} break;
			case eDWBPrefs:
				{	// Set the current primitive global WB as prefs
					fBuildingModeler->PrefsFromPrimitiveWB();
				} break;

			// Global Tab
			case eUVDomainListPartID:
				{
					TMCCountedPtr<IMFListPart> listPart;
					part->QueryInterface(IID_IMFListPart, (void **)&listPart);
					ThrowIfNil(listPart);
					listPart->GetSelectedCellRange(fSelectedCell, fSelectedCell);
					// Not Supported by TMFListPart: part->GetValue(&cell,kInt32ValueType);

					if(fSelectedCell>=0 && fSelectedCell<(int32)fDomainList.GetElemCount())
					{
						// Display the selected cell name
						TMCDynamicString string;
						fDomainListPart->GetValue( (void*)&string, kStringValueType );
						fDomainNamePart->SetValue( (void*)&string, kStringValueType, true, false );
					}
				} break;

			case eNameDomainPartID:
				{
					if(fSelectedCell>=0)
					{
						// change the name of the domain and the name in the cell
						TMCDynamicString string;
						part->GetValue( (void*)&string, kStringValueType );
						
						const int32 domainCount = buildingPrimitive->GetUVSpaceCount();
						for( int32 i=kBasicDomainsCount ; i<domainCount ; i++ )
						{
							UVSpaceInfo& domainInfo = buildingPrimitive->GetUVSpace(i);
							if( domainInfo.fID == (uint32)fDomainList[fSelectedCell] )
							{
								domainInfo.fName = string;
								break;
							}
						}

						fDomainListPart->SetValue( (void*)&string, kStringValueType, true, false );

						// modify the shading domain popu menu
						fBuildingModeler->InvalidateDomainList();
						fBuildingModeler->PostToSelectionChannel(0);
					}
				} break;

			case eDelDomainPartID:
				{
					if(fSelectedCell>=0)
					{
						// Delete the currently selected shading domain
						MenuAction::DelShadingDomain(fBuildingModeler,fDomainList[fSelectedCell]);
					}
				} break;

			case eAddDomainPartID:
				{
					CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

					// Propose a default name for the texture: "Texture n"
					TMCCountedPtr<IMFListPart> listPart;
					fDomainListPart->QueryInterface(IID_IMFListPart, (void **)&listPart);
					ThrowIfNil(listPart);

					// Propose a default name for the domain: "Texture n"
					TMCString255 string;
					gResourceUtilities->GetIndString(string, kModelerStrings, 26 );
					TMCString31 num;
					num.FromInt32(buildingPrimitive->GetUVSpaceCount());
					string+=num;

					MenuAction::ShadingDomain(fBuildingModeler,kCreateShadingDomain0,kNoDomains,-1,string);
				} break;

				// Backdrop
				case eImageFrontBackPartID:
				{
					TMCDynamicString filePath;
					part->GetValue(&filePath,kStringValueType);
					fBuildingModeler->SetBackdropImageWithFile( filePath, kWBXDrawPlane );
				} break;
				case eEnableFrontBackPartID:
				{
					boolean value;
					part->GetValue(&value,kBooleanValueType);
					fBuildingModeler->ActivateBackdrop( value, kWBXDrawPlane );
				} break;
				case eImageLeftRightPartID:
				{
					TMCDynamicString filePath;
					part->GetValue(&filePath,kStringValueType);
					fBuildingModeler->SetBackdropImageWithFile( filePath, kWBYDrawPlane );
				} break;
				case eEnableLeftRightPartID:
				{
					boolean value;
					part->GetValue(&value,kBooleanValueType);
					fBuildingModeler->ActivateBackdrop( value, kWBYDrawPlane );
				} break;
				case eImageTopBottomPartID:
				{
					TMCDynamicString filePath;
					part->GetValue(&filePath,kStringValueType);
					fBuildingModeler->SetBackdropImageWithFile( filePath, kWBZDrawPlane );
				} break;
				case eEnableTopBottomPartID:
					{
					boolean value;
					part->GetValue(&value,kBooleanValueType);
					fBuildingModeler->ActivateBackdrop( value, kWBZDrawPlane );
				} break;
			}

			if(index>0)
			{	// There's a position to modify
				const TVector2& pos = (index==1?status->fFirstSelectedPos:status->fSecondSelectedPos);

				{
					real32 value=0;
					part->GetValue(&value,kReal32ValueType);

					TVector2 delta=TVector2::kZero;
					delta[axis] = value-pos[axis];

					boolean result = MenuAction::MovePoint(fBuildingModeler, delta, NULL);
				}
			}
			else if(objAxis>-1)
			{
				real32 value=0;
				part->GetValue(&value,kReal32ValueType);

				if(scale)
				{
					TVector2 scale=TVector2::kOnes;
					if(objAxis==0)
						scale[0] = value/status->f2DObjWidth;
					else
						scale[1] = value/status->f2DObjHeight;

					MenuAction::ScaleObject(fBuildingModeler,scale);
				}
				else
				{
					const TVector2 center = status->f2DObjCenter;

					TVector2 delta=TVector2::kZero;
					delta[objAxis] = value-center[objAxis];

					MenuAction::MoveObject(fBuildingModeler,delta);
				}
			}
		}
	}

	return true;
}

void BuildingPropertiesClient::GetExtraTabs(TMCClassArray<TPropertyTab>& extraTabs)
{
	extraTabs.SetElemSpace(3);

	TPropertyTab previewTab(kRID_ModuleFamilyID, k3DViewModuleClassID, kPreviewTabNodeID, k3DViewModulePublicStrings, k3DViewModulePreviewString);
	previewTab.fPage= gShell3DUtilities->GetScenePreviewPart();
	extraTabs.AddElem(previewTab);

	extraTabs.AddElem( TPropertyTab(kRID_ModuleFamilyID, kHierModuleID, 15005,	kHierarchyModulePublicStrings, kHierModuleInstancesString) );
	extraTabs.AddElem( TPropertyTab(kRID_ModuleFamilyID, kHierModuleID, kObjectsPartNodeID,	kHierarchyModulePublicStrings, kHierModuleObjectsString) );
}

void BuildingPropertiesClient::GetPreferedSplit(int32& outPreferedSize, boolean& outForTopPart)
{
	outPreferedSize = 300;
	outForTopPart	= false;

	if (MCVerify(fBuildingModeler))
	{
		ModelerPrefs* prefs= fBuildingModeler->GetModelerPrefs();
		if (MCVerify(prefs))
		{
			outPreferedSize= prefs->fPreferedSplitSize;
		}
	}
}

void BuildingPropertiesClient::StorePreferedSplit(int32 inPreferedSize)
{
	if (MCVerify(fBuildingModeler))
	{
		ModelerPrefs* prefs= fBuildingModeler->GetModelerPrefs();
		if (MCVerify(prefs))
		{
			prefs->fPreferedSplitSize= inPreferedSize;
		}
	}
}

#endif // !NETWORK_RENDERING_VERSION

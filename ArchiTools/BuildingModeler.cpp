/****************************************************************************************************

		BuildingModeler.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/23/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "BuildingModeler.h"

#include "MBuildingRenderable.h"
#include "Copyright.h"
#include "BuildingPrim.h"
#include "MBuildingProperties.h"
#include "MBuildingDragAndDrop.h"
#include "MMouseDown.h"
#include "MPicking.h"
#include "PBuildingVisitor.h"

#include "COMUtilities.h"
#include "COM3DUtilities.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "IChangeManagement.h"
#include "I3DShGroup.h"
#include "I3DEditorHostPart.h"
#include "I3DEditorHostPartDefs.h"
#include "IMFPart.h"
#include "IShComponent.h"
#include "I3DShModule.h"
#include "IMFResponder.h"
#include "IMFDocument.h"
#include "ISceneDocument.h"
#include "IPropertiesModule.h"
#include "IMFWindow.h"
#include "IMFMenu.h"
#include "MFPublicDefs.h"
#include "3DPublicDefs.h"
#include "IWorkingBox.h"
#include "ISceneSelection.h"
#include "Geometry.h"

#include "I3DShMasterGroup.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"
const MCGUID CLSID_BuildingModeler(R_CLSID_BuildingModeler);
#else
const MCGUID CLSID_BuildingModeler={R_CLSID_BuildingModeler};
#endif

BuildingModeler::BuildingModeler()
{
	fActionBeingProcessed=0;
	fImmediateUpdate=false;
	fSelfDraw=true;
//	fActiveLevel=kAllLevels;
}

BuildingModeler::~BuildingModeler()
{
	// These data couldn't be destroy earlier: they re needed for the undo
	if (fSelectionChangeChannel)
		fSelectionChangeChannel->UnregisterListener(this);
	fSelectionChangeChannel=NULL;

	if (fImmediateUpdateChannel)
		fImmediateUpdateChannel->UnregisterListener(this);
	fImmediateUpdateChannel=NULL;

	fShPrimitive=NULL;
}

MCCOMErr BuildingModeler::Initialize(IMCUnknown* inElement)
{
	// Initialize is going to be called twice : once with the Universe, once with the Master Object
	if(!inElement) return MC_E_INVALIDARG;

	TMCCountedPtr<I3DShGroup> universe;
	if(inElement->QueryInterface(IID_I3DShGroup, (void**) &universe) == MC_S_OK)
	{
		TBasicModule::Initialize(inElement);
		return Initialize1(universe);
	}

	if(!fUniverse) return MC_E_INVALIDARG;

	TMCCountedPtr<I3DShPrimitive> primitive;
	if(inElement->QueryInterface(IID_I3DShPrimitive, (void**)&primitive) == MC_S_OK)
	{
		MCCOMErr result;
		result = Initialize2(primitive);
		return result;
	}

	return MC_E_INVALIDARG;
}

MCCOMErr BuildingModeler::Initialize1(I3DShGroup* universe)
{
	// Initialization Call #1 : the Universe is passed
	if(!universe) return MC_E_INVALIDARG;
	fUniverse = universe;
	return MC_S_OK;
}

MCCOMErr BuildingModeler::Initialize2(IMCUnknown* inElement)
{
	// Initialization Call #2 : the primitive is passed
	if(inElement->QueryInterface(IID_I3DShPrimitive, (void**)&fShPrimitive)==MC_S_OK)
	{
		if(fBuildingPrimitive)
		{
			//	We are switching objects:
			// should we do something?
		}

		if (fShPrimitive)
		{
			TMCCountedPtr<I3DShExternalPrimitive> externalPrimitive;
			fShPrimitive->QueryInterface(IID_I3DShExternalPrimitive, (void**) &externalPrimitive);
			if (externalPrimitive)
			{
				TMCCountedPtr<IShComponent> component;
				externalPrimitive->GetPrimitiveComponent(&component);
				if (component)
					component->QueryInterface(CLSID_BuildingPrim, (void**) &fBuildingPrimitive);
			}
		}

		// Init the cache
		fBuildingCache.SetPrimitive(fBuildingPrimitive);
	}
	else
		return MC_E_INVALIDARG;

	// Selection
#if (VERSIONNUMBER >= 0x040000)
	// CreateSceneSelection is not available any more
#else
	gShell3DUtilities->CreateSceneSelection(&fSelection);
#endif
	gChangeManager->CreateChannel(&fSelectionChangeChannel, 0);

	// Renderables
	BuildingMeshRenderable::Create(&f2DFlatFacetRenderable,	this, I3DShRenderable::kType_Facet);
	BuildingMeshRenderable::Create(&f3DFlatFacetRenderable,	this, I3DShRenderable::kType_Facet);
	BuildingMeshRenderable::Create(&f2DFacetRenderable,		this, I3DShRenderable::kType_Facet);
	BuildingMeshRenderable::Create(&f3DFacetRenderable,		this, I3DShRenderable::kType_Facet);
	BuildingMeshRenderable::Create(&f2DSegmentRenderable,	this, I3DShRenderable::kType_Segment);
	BuildingMeshRenderable::Create(&f3DSegmentRenderable,	this, I3DShRenderable::kType_Segment);
	BuildingMeshRenderable::Create(&f2DPointRenderable,		this, I3DShRenderable::kType_Point);
	BuildingMeshRenderable::Create(&f3DPointRenderable,		this, I3DShRenderable::kType_Point);
	f2DFlatFacetRenderable->SetFlatFacets(true);
	f3DFlatFacetRenderable->SetFlatFacets(true);
	f2DFacetRenderable->Set2D(true);
	f2DFlatFacetRenderable->Set2D(true);
	f2DSegmentRenderable->Set2D(true);
	f2DPointRenderable->Set2D(true);

	// Extra renderables
	LevelGridRenderable::Create(&fLevelGridRenderable,this,0);

	// Get a few interfaces

	QueryInterface(IID_I3DShModule, (void**) &fModule); ThrowIfNil(fModule);

	fDoc=fModule->GetDocumentNoAddRef(); ThrowIfNil(fDoc);

	fDoc->QueryInterface(IID_IMFResponder, (void**) &fContext); ThrowIfNil(fContext);
	fDoc->QueryInterface(IID_ISceneDocument, (void**) &fSceneDoc); ThrowIfNil(fSceneDoc);

	fSceneDoc->GetScene(&fScene); ThrowIfNil(fScene);


	gShell3DUtilities->GetDocumentModule(&fHierarchyModule, fDoc, kHierModuleID);

	TMCCountedPtr<I3DShModule> module;
	gShell3DUtilities->GetDocumentModule(&module, fDoc, kPropertiesModuleID);	
	module->QueryInterface(IID_IPropertiesModule, (void **)&fPropertiesModule);

	if (MCVerify(fPropertiesModule))
	{
		BuildingPropertiesClient::Create(fPropertiesModule,this,&fClient);
		TMCCountedPtr<IPropertiesClient> propertiesClient;
		fClient->QueryInterface(IID_IPropertiesClient, (void **)&propertiesClient);
		fPropertiesModule->SetCurrentClient(propertiesClient);
	}

	// Register as a listener

	gShell3DUtilities->GetImmediateUpdateChannel(&fImmediateUpdateChannel);
	fImmediateUpdateChannel->RegisterListener(this);

	gChangeManager->CreateChannel(&fRegularUpdateChannel);
	fRegularUpdateChannel->RegisterListener(this);

	fScene->GetTreePropertyChangeChannel(&fTreePropertyChangeChannel); ThrowIfNil(fTreePropertyChangeChannel);
	fTreePropertyChangeChannel->RegisterListener(this);

	fScene->GetTimeChangeChannel(&fTimeChangeChannel); ThrowIfNil(fTimeChangeChannel);
	fTimeChangeChannel->RegisterListener(this);


	// Initialize the window
	if(fWindow==(IMFPart*)NULL)
		CreateWindows();	
	
	// Init the handle shape
	fBuildingCache.SetHandleShape((uint32)GetModelerPrefs()->fHandleShape);

	// Init the unit system
	fBuildingCache.SetUnitSystem((EUnits)GetModelerPrefs()->fUnitSystem);

	//
//	PromotePrimitiveToMasterGroup();

	return MC_S_OK;
}

MCCOMErr BuildingModeler::Destroy()
{
	fUniverse = NULL;
	fScene = NULL;
	fDoc = NULL;
	fContext = NULL;
	fSceneDoc = NULL;
	fModule = NULL; // Building modeler module
	fHierarchyModule = NULL;
	fPropertiesModule = NULL;

	f2DFlatFacetRenderable = NULL;
	f3DFlatFacetRenderable = NULL;
	f2DFacetRenderable = NULL;
	f3DFacetRenderable = NULL;
	f2DSegmentRenderable = NULL;
	f3DSegmentRenderable = NULL;
	f2DPointRenderable = NULL;
	f3DPointRenderable = NULL;
	fLevelGridRenderable = NULL;

	fWindow = NULL;
	fHostPart = NULL; // editorHost part, contains the panes
	f3DEditorHostPart = NULL;

	fBuildingPrimitive = NULL;

	// This need to be kept for the undo in another room
//	fShPrimitive=NULL;

	// This need to be kept for the undo in another room
//	if (fImmediateUpdateChannel)
//		fImmediateUpdateChannel->UnregisterListener(this);
//	fImmediateUpdateChannel=NULL;

	if (fRegularUpdateChannel)
		fRegularUpdateChannel->UnregisterListener(this);
	fRegularUpdateChannel=NULL;

	if (fTreePropertyChangeChannel)
		fTreePropertyChangeChannel->UnregisterListener(this);
	fTreePropertyChangeChannel=NULL;

	// This need to be kept for the undo in another room
//	if (fSelectionChangeChannel)
//		fSelectionChangeChannel->UnregisterListener(this);
//	fSelectionChangeChannel=NULL;

	if (fTimeChangeChannel)				
		fTimeChangeChannel->UnregisterListener(this);
	fTimeChangeChannel=NULL;

	fSelection=NULL;
	fClient=NULL;

	fPropertiesModule = NULL;
	fHierarchyModule = NULL;
	fWorkingBox=NULL;

	ResetPointHelper();
	fDirections.ArrayFree();

	DestroyWindows();

	fBuildingCache.Release();
	
	return TBasicModule::Destroy();
}

void BuildingModeler::CreateWindows()
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	fModule->CreateWindowByResource(&fWindow,kBuildingModelerWindow,true);
	ThrowIfNil(fWindow);

	// Put our window on top of the others
	TMCCountedPtr<IMFWindow> window;
	fWindow->QueryInterface(IID_IMFWindow, (void**) &window); ThrowIfNil(window);
	window->SelectWindow();

	fWindow->FindChildPartByID(&fHostPart, 'host'); ThrowIfNil(fHostPart);

	if (fHostPart)
	{
		fHostPart->QueryInterface(IID_I3DEditorHostPart, (void **)&f3DEditorHostPart);
		if (f3DEditorHostPart)
		{
			// Get the current working box, and set the scene magnitude acording to it.
			real32 sceneMagnitude = 36; // default magnitude: 1 unit = 3 feet = 3*12 inches
 			f3DEditorHostPart->InitEditorHost(sceneMagnitude);
			f3DEditorHostPart->GetWorkingBox(&fWorkingBox);
			ThrowIfNil(fWorkingBox);
			SetSceneMagnitude(sceneMagnitude);

			TMCCountedPtr<IMFResponder> actionResponder;
			fDoc->QueryInterface(IID_IMFResponder, (void**)&actionResponder);
			f3DEditorHostPart->SetActionResponder(actionResponder);

			f3DEditorHostPart->SetPaneExtension('BMPP');
		}	
	}

	// Store the panes
	fPanes.ArrayFree();
	fPaneExtensions.ArrayFree();

	for( int32 iPane = 0 ; iPane<4 ; iPane++ )
	{
		TMCCountedPtr<IMFPart> part;
		fWindow->FindChildPartByID(&part, kBasePanePartID + iPane);
		fPanes.AddElem(part);

		TMCCountedPtr<BuildingPanePart> panePart;
		GetPanePartExt(&panePart, part); ThrowIfNil(panePart);
		fPaneExtensions.AddElem(panePart);
		panePart->SetModeler(this);
	}

	// Working box from pref file
	DefaultWorkingBox();

	// Color from pref file
	SetUIColors();

	// Title
	SetWindowTitle();

	const BuildingPrimData& primData = fBuildingPrimitive->Data();

	// Pane config
	//ModelerPrefs* prefs = GetModelerPrefs();
	f3DEditorHostPart->SetPaneConfig(primData.GetPaneConfig());

	// Backdrops
	SetBackdropImageWithFile(primData.GetFBBackdrop(), kWBXDrawPlane);
	SetBackdropImageWithFile(primData.GetLRBackdrop(), kWBYDrawPlane);
	SetBackdropImageWithFile(primData.GetTBBackdrop(), kWBZDrawPlane);
	ActivateBackdrop(primData.GetFBEnable(), kWBXDrawPlane);
	ActivateBackdrop(primData.GetLREnable(), kWBYDrawPlane);
	ActivateBackdrop(primData.GetTBEnable(), kWBZDrawPlane);
}

void BuildingModeler::GetToolBarsInfo(TMCArray<TToolBarInfo>& toolbarList)
{
	TToolBarInfo toolBarInfo(kBuildingToolBar, 'TBAR', 'BuiM'); // 11jan2009: add 'BuiM' here, but I don't know what's needed
	toolbarList.AddElem( toolBarInfo );
}

void BuildingModeler::DestroyWindows()
{
	f3DEditorHostPart=NULL;

	fPanes.ArrayFree();
	fPaneExtensions.ArrayFree();
	
	if (fWindow)
	{
		TMCCountedPtr<IMFWindow> window;
		fWindow->QueryInterface(IID_IMFWindow, (void **)&window);
		if (window) window->TryToClose();
	}

	fHostPart=NULL;
	fWindow=NULL;
}

MCCOMErr BuildingModeler::AboutToCloseMainWindow()
{
	DestroyWindows();	// We need to loose our references, because the window is about to die
	return MC_S_OK;
}

MCCOMErr BuildingModeler::Hydrate()
{
	if (fWindow == (IMFPart*)NULL)
		CreateWindows();	// It may happen that our window was closed during a dehydratation

	if (MCVerify(fWindow))
	{
		TMCCountedPtr<IMFWindow> win;
		fWindow->QueryInterface(IID_IMFWindow, (void **)&win);
		win->Show(true, false);
	}

	if (fPropertiesModule) 
		fPropertiesModule->GetThisModule()->Hydrate();

	if (fHierarchyModule)
			fHierarchyModule->Hydrate();

	gMenuUtilities->SetCurrentGlobalTool(kMoveToolID, true);

	return MC_S_OK;
}

MCCOMErr BuildingModeler::Dehydrate()
{
	if (fPropertiesModule) 
		fPropertiesModule->GetThisModule()->Dehydrate();
	if (fHierarchyModule)
		fHierarchyModule->Dehydrate();

	if (fWindow)
	{	// Hide the window
		TMCCountedPtr<IMFWindow> win;
		fWindow->QueryInterface(IID_IMFWindow, (void **)&win); ThrowIfNil(win);
		win->Show(false, false);
	}

	return MC_S_OK;
}

MCCOMErr BuildingModeler::Activate()
{
	if (fWindow==(IMFPart*)NULL)
		CreateWindows();

	if (MCVerify(fWindow))
	{
		TMCCountedPtr<IMFWindow> win;
		fWindow->QueryInterface(IID_IMFWindow, (void **)&win);
		win->Show(true, true); // this call is needed otherwise our modeler's window is not bring over others.
		win->SelectWindow();
	}

	if (fPropertiesModule) 
	{
		I3DShModule* propertiesModule=fPropertiesModule->GetThisModule();
		//	Check that the module is hydrated, another module closure might have dehydrated the properties
		if( !propertiesModule->GetIsHydrated() )
			propertiesModule->Hydrate();

		TMCCountedPtr<IPropertiesClient> propertiesClient;
		fClient->QueryInterface(IID_IPropertiesClient, (void **)&propertiesClient);
		ThrowIfNil(propertiesClient);
		fPropertiesModule->SetCurrentClient(propertiesClient);

		propertiesModule->Activate();
	}

	if (fHierarchyModule)
	{
		//	Check that the module is hydrated, another module closure might have dehydrated the properties
		if( !fHierarchyModule->GetIsHydrated() )
			fHierarchyModule->Hydrate();

		fHierarchyModule->Activate();
	}

	return MC_S_OK;
}

MCCOMErr BuildingModeler::Deactivate()
{
	if (fPropertiesModule) 
	{
		fPropertiesModule->GetThisModule()->Deactivate();
		fPropertiesModule->SetCurrentClient(NULL);
	}

	if (fHierarchyModule)
		fHierarchyModule->Deactivate();

	return MC_S_OK;
}

void BuildingModeler::SaveViewSettings()
{
	BuildingPrimData& data = fBuildingPrimitive->GetData();

	// Record the view settings before destroying the window
	if(f3DEditorHostPart)
	{
		data.SetPaneConfig( f3DEditorHostPart->GetPaneConfig() );
	}
}


// Use this method to add some items into an already existing menu (like Edit)
MCCOMErr BuildingModeler::BuildMenuBar()
{
	int16			menu=0, item=0;
	TMCString255	itemName;
	f3DEditorHostPart->BuildMenuBar();

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	// Get Duplicate item ID
	gMenuUtilities->MenuActionToMenuItem(cSymDuplicate, menu, item);

	// Add Duplicate Under
	gResourceUtilities->GetIndString(itemName, kModelerStrings, 48);
	gMenuUtilities->AddMenuItem(menu, itemName, item, kDuplicateUnderAction);
	// Add Duplicate over
	gResourceUtilities->GetIndString(itemName, kModelerStrings, 47);
	gMenuUtilities->AddMenuItem(menu, itemName, item, kDuplicateOverAction);

	return MC_S_OK;
}


void BuildingModeler::SelfPrepareMenus()
{
	TBasicModule::SelfPrepareMenus();

	// Enable/Disable the tools
	gMenuUtilities->EnableGlobalTool(k2DZoomToolID,		true, false);
	gMenuUtilities->EnableGlobalTool(k2DPanToolID,		true, false);
	gMenuUtilities->EnableGlobalTool(kMoveToolID,		true, false);
	gMenuUtilities->EnableGlobalTool(kRotateToolID,		true, false);
	gMenuUtilities->EnableGlobalTool(kScaleToolID,		true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallWithCrenel1Tool,	true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallWithCrenel2Tool,	true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallWithCrenel3Tool,	true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallWithCrenel4Tool,	true, false);
	gMenuUtilities->EnableGlobalTool(kBuildWallWithCrenel5Tool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertDoubleDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertArrowDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert2CircleDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert4CircleLDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert4CircleRDoorTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertNarrowWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertPanoWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertArrowWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertCircleWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert2CircleWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert4CircleLWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsert4CircleRWindowTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertSquareStairwayTool,true, false);
	gMenuUtilities->EnableGlobalTool(kInsertLargeStairwayTool,true, false);
	gMenuUtilities->EnableGlobalTool(kInsertWideStairwayTool,true, false);
	gMenuUtilities->EnableGlobalTool(kInsertCircleStairwayTool,true, false);
	gMenuUtilities->EnableGlobalTool(kInsertShellLevelTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertDuplicateLevelTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kInsertEmptyLevelTool,	true, false);
	gMenuUtilities->EnableGlobalTool(kDeleteTool,		true, false);

	//-- File menu
	TMCCountedPtr<IMFMenu> fileMenu;
	gMenuUtilities->FindMenuByID(kMenuFile, &fileMenu);
	if(fileMenu)
		fileMenu->Enable(true);

	//-- Edit menu
	TMCCountedPtr<IMFMenu> editMenu;
	gMenuUtilities->FindMenuByID(kMenuEdit, &editMenu);
	if(editMenu)
		editMenu->Enable(true);
	gMenuUtilities->EnableMenuAction(kaCut,				true);
	gMenuUtilities->EnableMenuAction(kaCopy,			true);
	gMenuUtilities->EnableMenuAction(kaPaste,			true);
	gMenuUtilities->EnableMenuAction(kaClear,			true);
	gMenuUtilities->EnableMenuAction(kaDuplicate,		true);
	gMenuUtilities->EnableMenuAction(kDuplicateOverAction,	true);
	gMenuUtilities->EnableMenuAction(kDuplicateUnderAction,	true);
	gMenuUtilities->EnableMenuAction(cSymDuplicate,		true);
	gMenuUtilities->EnableMenuAction(kaSelectAll,		true);
	gMenuUtilities->EnableMenuAction(cSymetry,			true);

	//-- View Menu
	gMenuUtilities->EnableMenuAction(kaResetPan,		true);
	gMenuUtilities->EnableMenuAction(kHideSelectionMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kHideAllMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kHideRoomsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kHideRoofsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kHideWallsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kShowAllMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kShowRoomsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kShowWallsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kShowRoofsMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kInvertSelectionMenuAction,true);
	gMenuUtilities->EnableMenuAction(kSelectByName,				true);
	gMenuUtilities->EnableMenuAction(kSelectByShadingDomain,	true);
	gMenuUtilities->EnableMenuAction(kDeselectByName,			true);
	gMenuUtilities->EnableMenuAction(kDeselectByShadingDomain,	true);
		
	gMenuUtilities->EnableCheckMenu(kShowActiveLevel, true, !fBuildingPrimitive->ShowAll());
	gMenuUtilities->EnableCheckMenu(kHolesEditionOnOff,true,fBuildingPrimitive->GetData().GetHoleEditEnable());

	//-- Create Menu
	gMenuUtilities->EnableMenuAction(kInsertWindowMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kInsertDoorMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kInsertStairwayMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kLevelTopMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kLevelBottomMenuAction,true);
	gMenuUtilities->EnableMenuAction(kLevelOnActiveMenuAction,true);
	gMenuUtilities->EnableMenuAction(kCreateRoomMenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof1MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof2MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof3MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof4MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof5MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof6MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof7MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof8MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof9MenuAction,	true);
	gMenuUtilities->EnableMenuAction(kCreateRoof10MenuAction,	true);

	//-- Selection Menu
	PrimitiveStatus* status = fBuildingPrimitive->GetStatus();
	gMenuUtilities->EnableMenuAction(kLevelHeightMenuAction,	status->fPartialySelectedLevelCount);
	gMenuUtilities->EnableMenuAction(kWallThicknessMenuAction,	status->fSelectedWallGlobalCount);
	gMenuUtilities->EnableMenuAction(kWallHeightMenuAction,		status->fSelectedWallGlobalCount);
	gMenuUtilities->EnableMenuAction(kWallArcOffsetMenuAction,	status->fSelectedWallGlobalCount);
	gMenuUtilities->EnableMenuAction(kWallArcSegmentsMenuAction, status->fSelectedWallGlobalCount);
	gMenuUtilities->EnableMenuAction(kFloorThicknessMenuAction,	status->fSelectedRoomCount);
	gMenuUtilities->EnableMenuAction(kCeilingThicknessMenuAction,status->fSelectedRoomCount);
	gMenuUtilities->EnableMenuAction(kRoofHeightMenuAction,		status->fSelectedRoofCount);
	gMenuUtilities->EnableMenuAction(kRoofBaseMenuAction,		status->fSelectedRoofCount);
	gMenuUtilities->EnableMenuAction(kChildMenuAction,			(status->fSelectedWallObjectCount+status->fSelectedRoomObjectCount>0));
	gMenuUtilities->EnableMenuAction(kSplitMenuAction,			status->fCouldSplit);
	gMenuUtilities->EnableMenuAction(kMergeMenuAction,			status->fCouldSplit);
	gMenuUtilities->EnableMenuAction(kRebuildMenuAction,		status->fHasSelection);
	gMenuUtilities->EnableMenuAction(kDetachMenuAction,			status->fHasSelection);
	gMenuUtilities->EnableMenuAction(kMoveOverMenuAction,		status->fHasSelection);
	gMenuUtilities->EnableMenuAction(kMoveUnderMenuAction,		status->fHasSelection);
	gMenuUtilities->EnableMenuAction(kImportCurveMenuAction,	(status->fWallObjectCount+status->fRoomObjectCount>0));
	gMenuUtilities->EnableMenuAction(kReplaceBySimpleWall, status->fSelectedWallWithCrenelCount>0); // Can morph walls with crenels into simple walls
	gMenuUtilities->EnableMenuAction(kReplaceByWallWithCrenel,	status->fSelectedSimpleWallCount>0); // Can morph simple walls into walls with crenels

	// Window menu
	TMCCountedPtr<IMFMenu> windowMenu;
	gMenuUtilities->FindMenuByID(kMenuWindow, &windowMenu);
	if(windowMenu)
		windowMenu->Enable(true);

	// Ghost menu entries.
	gMenuUtilities->EnableMenuAction(kBuildWallTool,		true);
	gMenuUtilities->EnableMenuAction(kInsertDoorTool,		true);
	gMenuUtilities->EnableMenuAction(kInsertDoubleDoorTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertArrowDoorTool, true);
	gMenuUtilities->EnableMenuAction(kInsert2CircleDoorTool, true);
	gMenuUtilities->EnableMenuAction(kInsert4CircleLDoorTool, true);
	gMenuUtilities->EnableMenuAction(kInsert4CircleRDoorTool, true);
	gMenuUtilities->EnableMenuAction(kInsertWindowTool,		true);
	gMenuUtilities->EnableMenuAction(kInsertNarrowWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsertPanoWindowTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertArrowWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsertCircleWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsert2CircleWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsert4CircleLWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsert4CircleRWindowTool, true);
	gMenuUtilities->EnableMenuAction(kInsertSquareStairwayTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertLargeStairwayTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertWideStairwayTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertCircleStairwayTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertShellLevelTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertDuplicateLevelTool,	true);
	gMenuUtilities->EnableMenuAction(kInsertEmptyLevelTool,	true);
	gMenuUtilities->EnableMenuAction(kDeleteTool,			true);
}

boolean BuildingModeler::SelfMenuAction(ActionNumber actionNumber)
{
	boolean result=false; // <=> action not treated
	boolean update=false;
	boolean done=false; // <=> action not treated

	switch (actionNumber)
	{

	default:
		{
			if ( actionNumber>=kShadingDomainID0 && actionNumber<=kCreateShadingDomain3)
			{	// Modify the shading domain ID of the selection
				result = MenuAction::SetShadingDomain(this,actionNumber);
				done = true; // Don't ask several times for the same thing
			}

			break;
		}

	// Edit menu
	case kaConvertToOtherModeler:
		{	
			// Check if it's a demo version
			if( !IsSerialValid() )
			{
				CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
				TMCDynamicString alertMessage;
				gResourceUtilities->GetIndString(alertMessage, kAlertStrings, 8);
				gPartUtilities->Alert(alertMessage);

				done = true;
			}
			
			// Propose to unfold the UVs
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
			TMCDynamicString mainMessage;
			gResourceUtilities->GetIndString(mainMessage, kModelerStrings, 63);
			TMCDynamicString yes( "Yes" ); 
			TMCDynamicString no( "No" ); 
			TMCDynamicString empty; 
			if( gShellUtilities->DoAlert(kShNotificationAlert, mainMessage,empty,empty,empty,2,yes,no,empty) == 0 )
			{	// Do it
				fBuildingPrimitive->GetData().UVData().mMethod = eUnfold;
				PrepareUnfoldUV prepareVisitor;
				prepareVisitor.TraverseVisible( fBuildingPrimitive, kAllLevels );
				fBuildingPrimitive->InvalidateAll(kAllLevels);
				InvalidatePrimitive();
			}
				// if( gShellUtilities->Alert(kTwoButtons, message, doSkip) == 0 )
		} break;
	case kaCut:
	case kaCopy:
		{
			result = MenuAction::CopyCut(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case kaPaste:
		{
			result = MenuAction::Paste(this);
			done = true; // Don't ask several times for the same thing
		} break;
	case cSymDuplicate:
	case kaDuplicate:
	case kDuplicateOverAction:
	case kDuplicateUnderAction:
		{
			result = MenuAction::Duplicate(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case cSymetry: 
		{
			result = MenuAction::FlipSelection(this);
			done = true; // Don't ask several times for the same thing
		} break;
	case kaSelectAll: 
		{
			result = MenuAction::SelectAll(this);
			done = true; // Don't ask several times for the same thing
		} break;
	case kaClear: 
		{	// Delete the selection
			result = MenuAction::DeleteSelection(this);
			done = true; // Don't ask several times for the same thing
		} break;

	// Insert menu
	case kInsertWindowMenuAction:
	case kInsertDoorMenuAction:
	case kInsertStairwayMenuAction:
		{
			result = MenuAction::InsertObject(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;

	case kLevelTopMenuAction:
	case kLevelBottomMenuAction:
	case kLevelOnActiveMenuAction:
		{
			result = MenuAction::InsertLevel(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;

	case kCreateRoomMenuAction:
		{
			result = MenuAction::CreateRoom(this);
			done = true; // Don't ask several times for the same thing
		} break;

	case kCreateRoof1MenuAction:
	case kCreateRoof2MenuAction:
	case kCreateRoof3MenuAction:
	case kCreateRoof4MenuAction:
	case kCreateRoof5MenuAction:
	case kCreateRoof6MenuAction:
	case kCreateRoof7MenuAction:
	case kCreateRoof8MenuAction:
	case kCreateRoof9MenuAction:
	case kCreateRoof10MenuAction:
		{
			result = MenuAction::CreateRoof(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;

	case kHideSelectionMenuAction:
	case kHideAllMenuAction:
	case kHideWallsMenuAction:
	case kHideRoomsMenuAction:
	case kHideRoofsMenuAction:
	case kShowAllMenuAction:
	case kShowWallsMenuAction:
	case kShowRoomsMenuAction:
	case kShowRoofsMenuAction:
		{
			result = MenuAction::ShowHide(this,actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case kInvertSelectionMenuAction:
		{
			result = MenuAction::InvertSelection(this);
			done = true; // Don't ask several times for the same thing
		} break;
	case kSelectByName:
	case kSelectByShadingDomain:
	case kDeselectByName:
	case kDeselectByShadingDomain:
		{
			result = MenuAction::SelectDeselectBy(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case kReplaceBySimpleWall:
	case kReplaceByWallWithCrenel:
		{
			result = MenuAction::ReplaceWall(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case kShowActiveLevel:
		{
			result = MenuAction::ShowActiveLevel(this, fBuildingPrimitive->ShowAll()?
									fBuildingPrimitive->ActiveLevel():kAllLevels);
			done = true; // Don't ask several times for the same thing
		} break;
	case kHolesEditionOnOff:
		{	// An action is needed: it changes the Hidden flag of the object and the points, so it has no impact on the
			// Undo/Redo.
			result = MenuAction::HoleEditionOnOff(this);
//			boolean prev = fBuildingPrimitive->GetData().GetHoleEditEnable();
//			fBuildingPrimitive->GetData().SetHoleEditEnable(!prev);
//			update = true;
			done = true;
		} break;

	// Selection menu
	case kSplitMenuAction:
		{
			result = MenuAction::Split(this); 
			done = true; // Don't ask several times for the same thing
		} break;
	case kMergeMenuAction:
		{
			result = MenuAction::Merge(this, true); // true: merge everything in one point 
			done = true; // Don't ask several times for the same thing
		} break;
	case kRebuildMenuAction:
		{
			result = MenuAction::Merge(this, false); // false: merge everything in the same level (reorganize the building)
			done = true; // Don't ask several times for the same thing
		} break;
	case kDetachMenuAction: // <=> ctrlX + ctrlV
	case kMoveOverMenuAction:
	case kMoveUnderMenuAction:
		{
			result = MenuAction::CutAndPaste(this, actionNumber);
			done = true; // Don't ask several times for the same thing
		} break;
	case kChildMenuAction:
		{
			TMCDynamicString objectName;

			// Ask for the object
			int32 objectCount = 0;
			int32 groupCount = 0;
			int32 index = 0;
			TMCCountedPtr<IMFDialogPart> theDialog;
			if( OpenDialog(&theDialog, kAskObject) )
			{
				// Build the dialog popup menu
				TMCCountedPtr<IMFPart> theDialogPart;
				theDialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
				FillInChildrenPopup(theDialogPart, 	
					fBuildingPrimitive->GetStatus()->fSceneObjectName, 
					this, eMenuChildrenList);

				if( theDialog->Go() )
				{
					// User hit OK: selection is set to a new shading domain
					if( !GetDialogString(theDialog, objectName, eMenuChildrenList) ||
						!GetDialogValue(theDialog, index, eMenuChildrenList))
					{
						// Couldn't get the string: return
						theDialog->Finished();
						break;
					}
				}
				else
				{
					// User hit cancel
					theDialog->Finished();
					break;
				}
				
				theDialog->Finished();
			}
			else
			{
				// Couldn't open the dialog
				break;
			}
	
			// Set it on the selection
			result = MenuAction::SetObjectInstance(this,objectName,index<objectCount);
			done = true; // Don't ask several times for the same thing
		} break;
	case kLevelHeightMenuAction:
	case kWallThicknessMenuAction:
	case kWallHeightMenuAction:
	case kWallArcOffsetMenuAction:
	case kWallArcSegmentsMenuAction:
	case kFloorThicknessMenuAction:
	case kCeilingThicknessMenuAction:
	case kRoofHeightMenuAction:
	case kRoofBaseMenuAction:
		{
			// Ask the dimension
			real32 newDimension = 0;
			// Get the current value
			BuildingPrimData& data = fBuildingPrimitive->GetData();
			switch (actionNumber)
			{
			case kLevelHeightMenuAction:		newDimension=data.GetDefaultLevelHeight(); break;
			case kWallThicknessMenuAction:		newDimension=data.GetDefaultWallThickness(); break;
			case kWallHeightMenuAction:			newDimension=data.GetDefaultWallHeight(); break;
			case kWallArcOffsetMenuAction:		newDimension=0; break;
			case kWallArcSegmentsMenuAction:	newDimension=1; break;
			case kFloorThicknessMenuAction:		newDimension=data.GetDefaultFloorThickness(); break;
			case kCeilingThicknessMenuAction:	newDimension=data.GetDefaultCeilingThickness(); break;
			case kRoofHeightMenuAction:			newDimension=data.GetDefaultRoofMax(); break;
			case kRoofBaseMenuAction:			newDimension=data.GetDefaultRoofMin(); break;
			}
			TMCCountedPtr<IMFDialogPart> theDialog;
			if( OpenDialog(&theDialog, kAskValue) )
			{

				SetDialogValue(theDialog, newDimension, eValue);

				if( theDialog->Go() )
				{
					// User hit OK: selection is set to a new shading domain
					if( !GetDialogValue(theDialog, newDimension, eValue) )
					{
						// Couldn't get the value: return
						theDialog->Finished();
						break;
					}
					else
					{
						theDialog->Finished();

						// Set it
						switch (actionNumber)
						{
						case kLevelHeightMenuAction:
							{
								TMCArray<int32> selectedLevels;
								const int32 levelCount = fBuildingPrimitive->GetLevelCount();
								for(int32 iLevel=0 ; iLevel<levelCount ; iLevel++)
								{
									if(fBuildingPrimitive->GetLevel(iLevel)->LevelPlan().HasPointSelection())
										selectedLevels.AddElem(iLevel);
								}

								result = MenuAction::SetLevelHeight(this,selectedLevels,newDimension);
							}break;
						case kWallThicknessMenuAction:		result = MenuAction::SetDimension(this,newDimension, eWallThickness); break;
						case kWallHeightMenuAction:			result = MenuAction::SetDimension(this,newDimension, eWallHeight); break;
						case kWallArcOffsetMenuAction:		result = MenuAction::SetDimension(this,newDimension, eWallOffset); break;
						case kWallArcSegmentsMenuAction:	result = MenuAction::SetDimension(this,newDimension, eWallSegments); break;
						case kFloorThicknessMenuAction:		result = MenuAction::SetDimension(this,newDimension, eFloorThickness); break;
						case kCeilingThicknessMenuAction:	result = MenuAction::SetDimension(this,newDimension, eCeilingThickness); break;
						case kRoofHeightMenuAction:			result = MenuAction::SetDimension(this,newDimension, eRoofMax); break;
						case kRoofBaseMenuAction:			result = MenuAction::SetDimension(this,newDimension, eRoofMin); break;
						}
					}
				}
				else
				{
					// User hit cancel
					theDialog->Finished();
					break;
				}
			}
			done = true; // Don't ask several times for the same thing
		} break;
	case kImportCurveMenuAction: 
		{
			// Open the dialog, post the action if the user click OK
			result = MenuAction::ImportCurve(this);
			done = true; // Don't ask several times for the same thing
		} break;
	}

	if(update)
		gChangeManager->PostChange(fRegularUpdateChannel, 0, NULL);

	return done;
}

boolean BuildingModeler::SelfToolAction(int32 inOldTool, int32 inNewTool)
{
	// See which tool was chosen and do appropriate preparation.
	if(inOldTool == inNewTool)
		return false;

	if( ResetPointHelper() )
		ImmediateUpdate(true,false);

	// Tell the property tray to display the right tool properties
	InvalidatePropertiesModule();

	return false;
}

// I3DExMiniModeler methods
MCCOMErr BuildingModeler::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_I3DExMiniModeler))
	{
		TMCCountedGetHelper<I3DExMiniModeler> result(ppvObj);
		result = static_cast<I3DExMiniModeler*>(this);
		return MC_S_OK;
	}
	return TBasicModule::QueryInterface(riid, ppvObj);
}

// Initialisation common to the miniModeler and the normal modeler
MCCOMErr BuildingModeler::BasicInitialize(IMCUnknown* inElement, IMFDocument* inDocument)
{
	if(inElement->QueryInterface(IID_I3DShPrimitive, (void**)&fShPrimitive)==MC_S_OK)
	{
		if(fBuildingPrimitive)
		{
			//	We are switching objects:
			// should we do something?
		}

		if (fShPrimitive)
		{
			TMCCountedPtr<I3DShExternalPrimitive> externalPrimitive;
			fShPrimitive->QueryInterface(IID_I3DShExternalPrimitive, (void**) &externalPrimitive);
			if (externalPrimitive)
			{
				TMCCountedPtr<IShComponent> component;
				externalPrimitive->GetPrimitiveComponent(&component);
				if (component)
					component->QueryInterface(CLSID_BuildingPrim, (void**) &fBuildingPrimitive);
			}
		}

		// Init the cache
		fBuildingCache.SetPrimitive(fBuildingPrimitive);
	}
	else
		return MC_E_INVALIDARG;


	fDoc = inDocument;
	ThrowIfNil(fDoc);

	fDoc->QueryInterface(IID_IMFResponder, (void**) &fContext);

	fDoc->QueryInterface(IID_ISceneDocument, (void**) &fSceneDoc);
	ThrowIfNil(fSceneDoc);

	fSceneDoc->GetScene(&fScene);
	ThrowIfNil(fScene);

	gShell3DUtilities->GetImmediateUpdateChannel(&fImmediateUpdateChannel);
	fImmediateUpdateChannel->RegisterListener(this);

	return MC_S_OK;
}

MCCOMErr BuildingModeler::CreateMiniModelerUI(IMFPart** UIPart, 
											  IMCUnknown* inElement, 
											  IMFDocument* inDocument)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	BasicInitialize(inElement, inDocument);

	TMCCountedGetHelper<IMFPart> result(UIPart);

	gPartUtilities->CreatePartByResource(&fAssembleRoomPart, (EMFResources)'Node', kAssembleRoomResID);
	result = fAssembleRoomPart;
	return MC_S_OK;
//	return MC_S_FALSE;
}

MCCOMErr BuildingModeler::CleanUp()
{
	Destroy();

	if(fAssembleRoomPart)
	{
		// We created this part, we're in charge of destroying it.
		fAssembleRoomPart->CallPrepareToDestroy();
		fAssembleRoomPart.Release();
	}
	return MC_S_OK;
}

boolean BuildingModeler::CanModify(	IMFResponder*		inSource,
									int32				inMessage)
{
	// This is currently not used. The modifications are handle by the part
	// extension. But only because we don't post any action (the building is 
	// not modified when the Assemble Room part is used)
	return false;
}

boolean BuildingModeler::HandleUIPartHit(	IMFPart*			inTopPart,
											int32				inMessage,
											IMFResponder*		inSource,
											void*				inData)
{
	// This is currently not used. The modifications are handle by the part
	// extension. But only because we don't post any action (the building is 
	// not modified when the Assemble Room part is used)
	return false;
}

boolean BuildingModeler::ResetPointHelper()
{
	if(fPointHelper)
	{
		fPointHelper->DeletePoint(); // this will also delete the wall
	
		SetPointHelper(NULL,false); // false: no check, the point and wall were deleted
		SetWallHelper(NULL,false); // false: no check, the point and wall were deleted

		fPointHelper = NULL;
		fWallHelper = NULL;
		return true;
	}

	return false;
}

boolean BuildingModeler::KeyDownFromPane(BuildingPanePart* pane,const TMCPlatformEvent& inEvent)
{
	boolean handled=false;
	boolean update=false;
	switch( inEvent.fChar )
	{
	case kchFwdDelete:
	case kchBackspace:
		{	// Cancel construction if we're doing some
			if(!fPointHelper)
			{
		// kaClear action we'll take care of it
				// Delete the current selection
		//		handled = MenuAction::DeleteSelection(this);
		//		handled = true; // Be sure not to delete stuff in the Scene
			}
		}
	case kchEscape:
		{	// Cancel construction if we're doing some
			handled = ResetPointHelper();
			if(handled) update=true;
		} break;
	case '+':
	case '-':
		{
		} break;
	}

	if(update)
		ImmediateUpdate(true,false);

	return handled;
}

boolean PosIsValid(const TVector2& pos,
				   const TMCArray<TVector2>& pointToAvoid,
				   const TMCArray<real32>& distToAvoid)
{
	const int32 count = pointToAvoid.GetElemCount();
	for(int32 i=0 ; i<count ; i++)
	{
		const real32 dist = distToAvoid[i];
		if((pointToAvoid[i]-pos).GetSquaredNorm()<dist*dist)
			return false;
	}

	return true;
}

class Constraint
{
public:
	Constraint();

	inline int32 AddConstraint(	const TVector2& csrtPoint,
							const TVector2& csrtDir );

	inline void ApplyCounstraint(TVector2& position);

	TVector2 fCstrPoint0;
	TVector2 fCstrDir0;
	TVector2 fCstrPoint1;
	TVector2 fCstrDir1;
	int32 fConstrCount;

};
Constraint::Constraint()
{
	fCstrPoint0 = TVector2::kZero;
	fCstrDir0 = TVector2::kZero;
	fCstrPoint1 = TVector2::kZero;
	fCstrDir1 = TVector2::kZero;
	fConstrCount = 0;
}
inline int32 Constraint::AddConstraint(	const TVector2& csrtPoint,
									const TVector2& csrtDir )
{
	if(fConstrCount>=2)
		return fConstrCount;

	if(fConstrCount)
	{
		if( RealAbs(fCstrDir0^csrtDir)<kRealEpsilon)
			return fConstrCount; // We already have a similar constraint

		fCstrPoint1 = csrtPoint;
		fCstrDir1 = csrtDir;
	}
	else
	{
		fCstrPoint0 = csrtPoint;
		fCstrDir0 = csrtDir;
	}

	return ++fConstrCount;
}
inline void Constraint::ApplyCounstraint(TVector2& position)
{
	if(fConstrCount==2)
	{	// intersection of the 2 constraints
		TVector2 result;
		if( IntersectLineLine2( fCstrPoint0, fCstrDir0, fCstrPoint1, fCstrDir1, result ) )
		{
			position = result;
		}
		else
		{
			MY_ASSERT(NULL);
		}
	}
	else if(fConstrCount==1)
	{	// proj on the constraint
		Project( position, fCstrPoint0, fCstrPoint0+fCstrDir0, position);
	}
}

VPoint* BuildingModeler::SnapPointPos(Picking& picked,
									 BuildingPanePart* pane,
									 const TMCPoint& inWhere,
									 VPoint* prevPoint,
									 const int32 inLevel,
									 const boolean needNewPoint,
									 boolean& pickExistingPoint)
{
	const real32 precision = GetSnapPrecision();
	const real32 sqrdPrecision = precision*precision;

	// This flag is used here: clear it before
	fBuildingPrimitive->ClearPointFlag(eSnapedPosition,inLevel);
	fBuildingPrimitive->ClearWallFlag(eSnapedPosition,inLevel);

	// If a point is not passed, we have to return one, otherwise we just set
	// the position on the point
	TMCCountedPtr<VPoint> newPoint;
	if(!needNewPoint)
		newPoint = prevPoint;

	pickExistingPoint = false;

	// Warning: walls can't be too small. If the passed point is not NULL
	// and have walls, check that these wall won't get shorter than their thickness
	TMCArray<TVector2> pointToAvoid;
	TMCArray<real32> distToAvoid;
	if(prevPoint)
	{
		const int32 wallCount = prevPoint->GetWallCount();
		
		pointToAvoid.SetElemCount(wallCount);
		distToAvoid.SetElemCount(wallCount);

		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			Wall* wall = prevPoint->GetWall(iWall);
			VPoint* otherPoint = wall->GetOtherPoint(prevPoint);
			pointToAvoid[iWall] = otherPoint->Position();
			distToAvoid[iWall] = wall->GetThickness();
		}
	}

	switch(picked.GetPickedType())
	{
	case eWallPicked:
	case eEdgePicked:
		{	// If we're near from an extremity, snap the wall on it,
			// otherwise split the wall to create the new point
			pickExistingPoint = true;
			if(newPoint) newPoint->SetFlag(eSnapedPosition);

			TVector3 wallHit;
			picked.GetHitPosition(wallHit);
			TVector2 wallHitProj = wallHit.CastToXY();
			Wall* wall = static_cast<Wall*>(picked.PickedObject());
			const TVector2& p0 = wall->GetPoint(0)->Position();
			const TVector2& p1 = wall->GetPoint(1)->Position();
			if( (wallHitProj-p0).GetSquaredNorm()<sqrdPrecision )
			{
				if(PosIsValid(p0,pointToAvoid,distToAvoid))
				{
					if(newPoint) newPoint->SetPosition(p0);
					else newPoint = wall->GetPoint(0);
				}
			}
			else if( (wallHitProj-p1).GetSquaredNorm()<sqrdPrecision )
			{
				if(PosIsValid(p1,pointToAvoid,distToAvoid))
				{
					if(newPoint) newPoint->SetPosition(p1);
					else newPoint = wall->GetPoint(1);
				}
			}
			else
			{
				if(PosIsValid(wallHitProj,pointToAvoid,distToAvoid))
				{	// Check if we're in a near perpendicular or other specific pos (1/2 wall, 1/3, 1/4)
					const int32 wallCount = pointToAvoid.GetElemCount();

					boolean posSet = false;
					for(int32 iWall=0 ; iWall<wallCount && !posSet ; iWall++)
					{
						// Perpendicular
						TVector2 proj;
						Project( pointToAvoid[iWall], p0, p1, proj);
						if((wallHitProj-proj).GetSquaredNorm()<sqrdPrecision)
						{
							prevPoint->GetWall(iWall)->SetFlag(eSnapedPosition);							
							wallHitProj = proj;
							posSet=true; break;
						}

						// Vertical
						if(RealAbs(pointToAvoid[iWall].x-wallHitProj.x)<precision)
						{
							TVector2 intersect;
							if( IntersectLineLine(	p0, p1, pointToAvoid[iWall], TVector2::kUnitY, intersect ) )
							{
								prevPoint->GetWall(iWall)->SetFlag(eSnapedPosition);							
								wallHitProj = intersect;
								posSet=true; break;
							}
						}

						// Horizontal
						if(RealAbs(pointToAvoid[iWall].y-wallHitProj.y)<precision)
						{
							TVector2 intersect;
							if( IntersectLineLine(	p0, p1, pointToAvoid[iWall], TVector2::kUnitX, intersect ) )
							{
								prevPoint->GetWall(iWall)->SetFlag(eSnapedPosition);							
								wallHitProj = intersect;
								posSet=true; break;
							}
						}
					}

					if(!posSet)
					{
						wallHitProj = (wall->GetSnappedPosOnWall(wallHit, eUnknownObjectType)).CastToXY();
					}

					if(newPoint) newPoint->SetPosition(wallHitProj);
					else
					{
						if(prevPoint)
						{
							wall->Split(prevPoint);
							newPoint = fBuildingPrimitive->MakePoint(wallHitProj,inLevel);
						}
						else
						{
							newPoint = wall->Split(wallHitProj);
						}
					}
				}
				else
					break;
			}
		} return newPoint; // Snap to wall
	case ePointPicked:
		{	// Use the picked point
			VPoint* pt = static_cast<VPoint*>(picked.PickedObject());
			if(pt==NULL)
			{
				// Something is wrong
				return NULL;
			}

			if(newPoint)
			{
				newPoint->SetPosition(pt->Position());
			}
			else
			{
				if(prevPoint && prevPoint->GetWallCount()==0)
				{
					const TVector2& pos = pt->Position();

					newPoint = fBuildingPrimitive->MakePoint(pos,inLevel);
				}
				else
				{
					newPoint = pt;
				}
			}

			pickExistingPoint = true;
			if(newPoint)
			{
				newPoint->SetFlag(eSnapedPosition);
			}
		} return newPoint; // Snap to point
	}

	{	// Create a new pos or point
		TVector3 origin, direction, pickPos;
		TVector3 planePoint(0,0,fBuildingPrimitive->GetAltitude(inLevel));
		pane->Get3DEditorHostPanePart()->PixelRay(inWhere,origin,direction);
		if(IntersectLinePlane2(origin, direction, TVector3::kUnitZ,
											planePoint, pickPos	) )
		{
			TVector2 pickedPos = pickPos.CastToXY();

			if(newPoint) newPoint->SetPosition(pickedPos);
			else newPoint = fBuildingPrimitive->MakePoint(pickedPos,inLevel);
		}
		else
			return newPoint;
	}

	// The point wasn't snap to a wall or another point => see if there's some particularity
//	SnapPoint(newPoint);
	return newPoint;
}

void BuildingModeler::GetGridConstraint( Constraint& cstr, const TVector2& newPos)
{
	if(!SnapToGrid())
		return;

	const real32 precision = GetSnapPrecision();

	const real32 gridSpacing = GetGridSpacingPref();
	// Get the pos in a 0,gridSpacing space
	{
		const int32 xRoot = floor(newPos.x/gridSpacing);
		const real32 xStep =  xRoot*gridSpacing;
		const real32 xGap =  newPos.x - xStep;
		if(xGap<precision)
		{
			const TVector2 cstrPt(xStep,newPos.y);
			cstr.AddConstraint(cstrPt,TVector2::kUnitY);
		}
		else if(gridSpacing-xGap<precision)
		{
			const TVector2 cstrPt(xStep+gridSpacing,newPos.y);
			cstr.AddConstraint(cstrPt,TVector2::kUnitY);
		}
	}
	{
		const int32 yRoot = floor(newPos.y/gridSpacing);
		const real32 yStep = yRoot*gridSpacing;
		const real32 yGap = newPos.y - yStep;
		if(yGap<precision)
		{
			const TVector2 cstrPt(newPos.x,yStep);
			cstr.AddConstraint(cstrPt,TVector2::kUnitX);
		}
		else if(gridSpacing-yGap<precision)
		{
			const TVector2 cstrPt(newPos.x,yStep+gridSpacing);
			cstr.AddConstraint(cstrPt,TVector2::kUnitX);
		}
	}
}

void BuildingModeler::GetDirConstraint(Constraint& cstr, 
									   const TMCClassArray<Line2D>& directions,
									   const TVector2& pos)
{
	const real32 distMin=.1f; // tmp

	const real32 precision = GetSnapPrecision();

	const int32 dirCount = directions.GetElemCount();

	{
		for(int32 iDir=0 ; iDir<dirCount ; iDir++)
		{
			const Line2D& dir = directions[iDir];

			if(dir.fDir.GetSquaredNorm()>kRealEpsilon)
			{
				const real32 dist = PointLineDistance(pos,dir.fPoint,dir.fDir);
				if(dist<precision)
				{
					cstr.AddConstraint(dir.fPoint,dir.fDir);
				
					if(cstr.fConstrCount>=2)
						return; // We've got 2 constraint, stop looking for other
				}
			}
		}
	}

	
	{ // X and Y directions
		for(int32 iDir=0 ; iDir<dirCount ; iDir++)
		{
			const Line2D& dir = directions[iDir];

			const TVector2& otherPos = dir.fPoint;

			const real32 diffX = otherPos.x - pos.x;
			const real32 diffY = otherPos.y - pos.y;

			// edge nearly horizontal
			if(RealAbs(diffX) < precision && RealAbs(diffY)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitY);
			
				if(cstr.fConstrCount>=2)
					return; // We've got 2 constraint, stop looking for other
			}
			// edge nearly horizontal
			if(RealAbs(diffY) < precision && RealAbs(diffX)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitX);
			
				if(cstr.fConstrCount>=2)
					return; // We've got 2 constraint, stop looking for other
			}
		}
	}
	
/*	const int32 pointCount = points.GetElemCount();

	TVector2 curDir = pos-prevPos;
	const boolean hasRefDir = curDir.Normalize();

	if(hasRefDir)
	{ // directions arround
		for(int32 iPt=0 ; iPt<pointCount ; iPt++)
		{
			const TVector2& otherPos = points[iPt];

			TVector2 prevDir = otherPos-prevPos;
			if(prevDir.Normalize())
			{
				const real32 dist = PointLineDistance(pos,otherPos,prevDir);
				if(dist<precision)
				{
					cstr.AddConstraint(otherPos,prevDir);
				
					if(cstr.fConstrCount>=2)
						return; // We've got 2 constraint, stop looking for other
				}
			}
		}
	}
	
	{ // X and Y directions
		for(int32 iPt=0 ; iPt<pointCount ; iPt++)
		{
			const TVector2& otherPos = points[iPt];

			const real32 diffX = otherPos.x - pos.x;
			const real32 diffY = otherPos.y - pos.y;

			// edge nearly horizontal
			if(RealAbs(diffX) < precision && RealAbs(diffY)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitY);
			
				if(cstr.fConstrCount>=2)
					return; // We've got 2 constraint, stop looking for other
			}
			// edge nearly horizontal
			if(RealAbs(diffY) < precision && RealAbs(diffX)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitX);
			
				if(cstr.fConstrCount>=2)
					return; // We've got 2 constraint, stop looking for other
			}
		}
	}*/
}

// Prepare some data relative to the surrounding of the point
void BuildingModeler::PreparePointConstraints(CommonPoint* point, TMCClassArray<Line2D>& directions)
{
	// Erase older data
	fDirections.ArrayFree();

	if(!point)
		return;

	const TVector2& curPos = point->Position();

	// Get the non-selected points arround
	TMCCountedPtrArray<CommonPoint> pointsArround;
	point->GetSurroundingPoints(pointsArround);

	// Use these points to build prefered directions
	const int32 arroundCount = pointsArround.GetElemCount();

	for(int32 iPt=0 ; iPt<arroundCount ; iPt++)
	{
		CommonPoint* ptArround = pointsArround[iPt];
		const TVector2& ptPos = ptArround->Position();
	//	if(curPos!=ptPos)
		{
			Line2D& line = directions.AddElem();
			line.fPoint = curPos;
			line.fDir = curPos-ptPos;
			line.fDir.Normalize();
		}

		// We then could check arround the surrounding points to get a second level of constraints
		TMCCountedPtrArray<CommonPoint> pointsArroundBis;
		ptArround->GetSurroundingPoints(pointsArroundBis);
		const int32 countBis = pointsArroundBis.GetElemCount();

		for(int32 iPtAr=0 ; iPtAr<countBis ; iPtAr++)
		{
			CommonPoint* ptArroundBis = pointsArroundBis[iPtAr];

			if(ptArroundBis==point)
				continue;

			const TVector2& ptArPos = ptArroundBis->Position();
			if(ptPos!=ptArPos)
			{
				TVector2 vec = ptPos-ptArPos;
				vec.Normalize();

				// In the dir
				Line2D& align = directions.AddElem();
				align.fPoint = ptPos;
				align.fDir = vec;
				align.fDir.Normalize();

				// In the normal dir
				Line2D& normal = directions.AddElem();
				normal.fPoint = ptPos;
				normal.fDir.x = -vec.y;
				normal.fDir.y = vec.x;
			}
		}
	}

}

void BuildingModeler::SnapCommonPoint(CommonPoint* point, 
									  const TMCClassArray<Line2D>& directions, 
									  const boolean ctrl, const boolean shift)
{
	if(shift && !ctrl)
		return;

	const TVector2& pos = point->Position();

	// a point can be under 2 constraint at the same time.
	Constraint cstr;

	if(ctrl)
	{	// Best direction
		GetDirConstraint(cstr,directions,pos);
	}
	if(!shift)
	{	// Snap to grid
		GetGridConstraint(cstr,pos);
	}

	if(cstr.fConstrCount)
	{
		TVector2 newPos = pos;
		cstr.ApplyCounstraint(newPos);
		point->SetFlag(eSnapedPosition);

		point->SetPosition(newPos);
	}
}

void BuildingModeler::SnapPoint(VPoint* newPoint)
{
	const TVector2& pos = newPoint->Position();


#if 1
	// a point can be under 2 constraint at the same time.
	Constraint cstr;

	// 1: check if the walls arround are aligned with the X or the Y axis,
	// or with the diagonal
	{
		const real32 precision = GetSnapPrecision();
		const int32 wallCount = newPoint->GetWallCount();
	
		for(int32 iWall=0 ; iWall<wallCount && cstr.fConstrCount<2 ; iWall++)
		{
			Wall* wall = newPoint->GetWall(iWall);

			VPoint* otherPoint = wall->GetOtherPoint(newPoint);
			const TVector2& otherPos = otherPoint->Position();
			const real32 distMin = wall->GetThickness();
			real32 diffX = otherPos.x - pos.x;
			real32 diffY = otherPos.y - pos.y;

			// Wall nearly horizontal
			if(RealAbs(diffX) < precision && RealAbs(diffY)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitY);
				newPoint->SetFlag(eSnapedPosition);
				wall->SetFlag(eSnapedPosition);
			
				if(cstr.fConstrCount>=2) break; // We've got 2 constraint, stop looking for other
			}
			// Wall nearly horizontal
			if(RealAbs(diffY) < precision && RealAbs(diffX)>distMin)
			{
				cstr.AddConstraint(otherPos,TVector2::kUnitX);
				newPoint->SetFlag(eSnapedPosition);
				wall->SetFlag(eSnapedPosition);
			
				if(cstr.fConstrCount>=2) break; // We've got 2 constraint, stop looking for other
			}
		}
	}

	// 6: Snap to grip
	GetGridConstraint(cstr,pos);

	TVector2 newPos = pos;
	cstr.ApplyCounstraint(newPos);

#else
	// 1: X and Y directions
	{
		TVector2 preferedProjAxis = TVector2::kZero;

		for(int32 iWall=0 ; iWall<wallCount ; iWall++)
		{
			const real32 distMin = distToAvoid[iWall];
			real32 diffX = pointToAvoid[iWall].x - newPos.x;
			real32 diffY = pointToAvoid[iWall].y - newPos.y;
			// Define a prefered direction of projection so a point can 
			// respect 2 constrains at the same time
			Wall* wall = newPoint->GetWall(iWall);

			// Horizontal
			if(RealAbs(diffX) < precision && RealAbs(diffY)>distMin)
			{
				preferedProjAxis = TVector2::kUnitY;
				newPos.x = pointToAvoid[iWall].x;
				diffX = 0;
				newPoint->SetFlag(eSnapedPosition);
				wall->SetFlag(eSnapedPosition);
			}

			// Vertical
			if(RealAbs(diffY) < precision && RealAbs(diffX)>distMin)
			{
				preferedProjAxis = TVector2::kUnitX;
				newPos.y = pointToAvoid[iWall].y;
				diffY=0;
				newPoint->SetFlag(eSnapedPosition);
				wall->SetFlag(eSnapedPosition);
			}

			// Diagonal
			if(RealAbs(RealAbs(diffX)-RealAbs(diffY)) < precision)
			{
				preferedProjAxis = (diffX*diffY>0?	TVector2((real32)INV_SQRT2,(real32)INV_SQRT2):
													TVector2((real32)INV_SQRT2,(real32)(-INV_SQRT2)));
				Project(newPos, pointToAvoid[iWall], pointToAvoid[iWall]+preferedProjAxis, newPos);
				newPoint->SetFlag(eSnapedPosition);
				wall->SetFlag(eSnapedPosition);
			}
	
			// 4: Align with points in this level
			TVector2 axis(diffX,diffY);
			if( axis.Normalize() )
			{
				if(preferedProjAxis!=TVector2::kZero)
					axis=preferedProjAxis;
				if( fBuildingPrimitive->SnapPosWithAxis(newPos,axis,preferedProjAxis,inLevel) )
				{	// Already a good snaping position, stop looking for one
					newPoint->SetPosition(newPos);
					newPoint->SetFlag(eSnapedPosition);
					return newPoint;
				}
			}
		}

		// 4: Find points in the X and Y directions
//		if(RealAbs(axis.x)>kRealEpsilon && RealAbs(axis.y)>kRealEpsilon )
		{	// try also with the X and Y direction
			if( !preferedProjAxis.IsEqual(TVector2::kUnitY,kRealEpsilon) &&
				fBuildingPrimitive->SnapPosWithAxis(newPos,TVector2::kUnitX,preferedProjAxis,inLevel) )
			{	// Already a good snaping position, stop looking for one
				newPoint->SetPosition(newPos);
				newPoint->SetFlag(eSnapedPosition);
				return newPoint;
			}
			if( !preferedProjAxis.IsEqual(TVector2::kUnitX,kRealEpsilon) &&
				fBuildingPrimitive->SnapPosWithAxis(newPos,TVector2::kUnitY,preferedProjAxis,inLevel) )
			{	// Already a good snaping position, stop looking for one
				newPoint->SetPosition(newPos);
				newPoint->SetFlag(eSnapedPosition);
				return newPoint;
			}
		}
	}

	// 2: Perpendicularity to prev or next walls (if their direction is different from X, Y or diagonal)
	// 3: maybe later: some paralellism or perpendicularity with the last wall target
	// 5: Snap to points over and under

	// 6: Snap to grip
	const real32 gridSpacing = GetGridSpacingPref();
	// Get the pos in a 0,gridSpacing space
	{
		const int32 xRoot = floor(newPos.x/gridSpacing);
		const real32 xStep =  xRoot*gridSpacing;
		const real32 xGap =  newPos.x - xStep;
		if(xGap<precision) newPos.x = xStep;
		else if(gridSpacing-xGap<precision) newPos.x = xStep+gridSpacing;
	}
	{
		const int32 yRoot = floor(newPos.y/gridSpacing);
		const real32 yStep = yRoot*gridSpacing;
		const real32 yGap = newPos.y - yStep;
		if(yGap<precision) newPos.y = yStep;
		else if(gridSpacing-yGap<precision) newPos.y = yStep+gridSpacing;
	}
#endif
	newPoint->SetPositionCheckWalls(newPos);
}

boolean BuildingModeler::MouseMovedFromPane(BuildingPanePart* pane, const TMCPoint& inWhere, const TMCModifiers& modifier)
{
	try
	{
		int32 currentTool=0;
		gMenuUtilities->GetCurrentGlobalTool(currentTool); 

		switch (currentTool)
		{
		case kBuildWallTool:
		case kBuildWallWithCrenel1Tool:
		case kBuildWallWithCrenel2Tool:
		case kBuildWallWithCrenel3Tool:
		case kBuildWallWithCrenel4Tool:
		case kBuildWallWithCrenel5Tool:
			{ // Update the display of the created wall
				if(fPointHelper)
				{
					Picking picked(this, pane,inWhere, ePickWall+ePick2D);

					boolean onPoint=false;
					fPointHelper = SnapPointPos(picked,pane,inWhere,fPointHelper,fPointHelper->GetLevelIndex(),false,onPoint);
					if(!onPoint)
						SnapCommonPoint(fPointHelper, fDirections, gActionManager->IsCommandDown(), gActionManager->IsShiftDown());

					if(fWallHelper)
					{
						VPoint* otherPoint = fWallHelper->GetOtherPoint(fPointHelper);
						otherPoint->InvalidateTessellation();
						otherPoint->ClearFlag(eWallOrdered); // We don't know on which side we'll be
					}
					fBuildingPrimitive->InvalidateBBox();

					// Invalidate the properties to display the position of the point
	#ifdef WIN
					InvalidatePropertiesModule();
	#else
	#if (TARGET_ARCH==x86_64)
					// Do nothing: it's too slow
	#else
					InvalidatePropertiesModule();
	#endif
	#endif
				
					ImmediateUpdate(true, false);
				}

			} break;
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingModeler::MouseMovedFromPane"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingModeler::MouseMovedFromPane"));
	}

	return true;
}

boolean BuildingModeler::MouseDownFromPane(BuildingPanePart* pane, const TMCPoint& inWhere, const TMCPlatformEvent& inEvent)
{
	try
	{
		int32 currentTool=0;
		gMenuUtilities->GetCurrentGlobalTool(currentTool); 

		switch (currentTool)
		{
		case kMoveToolID:
		case kScaleToolID:	
		case kRotateToolID:	
			{
				// Check if it's a long static click before doing anything in order to have the
				// same reactivity as in the 3DView. So the Tracking won't start before a laps 
				// of time or a move of a few pixels has been done.
				gActionManager->IsLongStaticClick();

				return MouseDown::SelectTool(this, inWhere, inEvent, pane, currentTool);
			} break;
		case kInsertWindowTool:
		case kInsertNarrowWindowTool:
		case kInsertPanoWindowTool:
		case kInsertArrowWindowTool:
		case kInsertCircleWindowTool:
		case kInsert2CircleWindowTool:
		case kInsert4CircleLWindowTool:
		case kInsert4CircleRWindowTool:
		case kInsertDoorTool:
		case kInsertDoubleDoorTool:
		case kInsertArrowDoorTool:
		case kInsert2CircleDoorTool:
		case kInsert4CircleLDoorTool:
		case kInsert4CircleRDoorTool:
		case kInsertSquareStairwayTool:
		case kInsertLargeStairwayTool:
		case kInsertWideStairwayTool:
		case kInsertCircleStairwayTool:
			{
				return MouseDown::InsertObjectTool(this, inWhere, pane, currentTool);
			} break;
		case kBuildWallTool:
		case kBuildWallWithCrenel1Tool:
		case kBuildWallWithCrenel2Tool:
		case kBuildWallWithCrenel3Tool:
		case kBuildWallWithCrenel4Tool:
		case kBuildWallWithCrenel5Tool:
			{
				return MouseDown::BuildWallTool(this, inWhere, pane, currentTool);
			} break;
		case kInsertShellLevelTool:
		case kInsertDuplicateLevelTool:
		case kInsertEmptyLevelTool:
			{
				return MouseDown::InsertLevelTool(this, inWhere, pane, currentTool);
			} break;
		case kDeleteTool:
			{
				return MouseDown::DeleteTool(this, inWhere, pane, currentTool);
			} break;
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingModeler::MouseDownFromPane"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingModeler::MouseDownFromPane"));
	}

	return false;
}

void BuildingModeler::DataChanged(IChangeChannel* channel, IDType changeKind, IMCUnknown* changedData)
{
	try
	{

		TBasicModule::DataChanged(channel, changeKind, changedData);

		const boolean isFaceless = IsFaceless();
		if(isFaceless)
		{	// Is there anything to do when no pane are visible ?
			return;
		}

		if(channel == fRegularUpdateChannel)
		{
			// Invalidate the renderable and the render
			InvalidateMeshes();
			for( int32 iPane = 0 ; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++)
			{
				fPaneExtensions[iPane]->InvalidateRender();
				fPanes[iPane]->Invalidate();
			}
		}
		else if(channel == fImmediateUpdateChannel)
		{
			switch(changeKind)
			{
				// Action processing handling
				// Avoid cache invalidation when doing a simple camera dolly
				case kChange_ActionBegin: fActionBeingProcessed++; break;
				case kChange_ActionEnd: fActionBeingProcessed--; break;
				// Other messages
				default:
				{
					boolean shouldUpdate = false;
					int32 updateOnlyPaneID = -1;
					if( changedData == NULL )
						shouldUpdate = true;
					else
					{	// See what kind of data we've got
						TMCCountedPtr<I3DEditorHostPanePart>	changedPanePart;
						changedData->QueryInterface(IID_I3DEditorHostPanePart, (void**) &changedPanePart);
						if (changedPanePart)
						{
							// See if it's one of our pane part
							for( int32 iPane=0 ; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++ )
							{
								if (fPaneExtensions[iPane]->Get3DEditorHostPanePart() == changedPanePart)
								{
									shouldUpdate = true;
									updateOnlyPaneID = iPane;
									break;
								}
							}
						}
						if( !shouldUpdate )
						{
							// Check if it's the primitive that's changed
							TMCCountedPtr<I3DShPrimitive>	changedPrimitive;
							changedData->QueryInterface(IID_I3DShPrimitive, (void**) &changedPrimitive);
							if(fShPrimitive == changedPrimitive)
								shouldUpdate = true;
						}
					}

					if( shouldUpdate)
					{
						// Update the panes
						switch(changeKind)
						{
							case kChange_BeginImmediateUpdate:
							{
								fImmediateUpdate=true;
								for( int32 iPane=0; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++ )
								{
									fPaneExtensions[iPane]->Get3DEditorHostPanePart()->SetImmediateMode(true);
								}
								break;
							}
							case kChange_ImmediateUpdate:
							{
								if(updateOnlyPaneID>=0)
								{
									fPaneExtensions[updateOnlyPaneID]->InvalidateRender((fActionBeingProcessed==0));
									if(fSelfDraw)
										fPaneExtensions[updateOnlyPaneID]->InternalSelfDraw(NULL,NULL);
								}
								else
								{
									for( int32 iPane=0; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++ )
									{
										fPaneExtensions[iPane]->InvalidateRender((fActionBeingProcessed==0));
										if(fSelfDraw)
											fPaneExtensions[iPane]->InternalSelfDraw(NULL,NULL);
									}
								}
								break;
							}
							case kChange_EndImmediateUpdate:
							{
								fImmediateUpdate=false;
								for( int32 iPane=0; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++ )
								{
									fPaneExtensions[iPane]->Get3DEditorHostPanePart()->SetImmediateMode(false);
								}
								break;
							}	
						}
					}
				}
			}
		}
		else if (channel==fTimeChangeChannel && 
				(changeKind==kChange_CurrentTime || changeKind==kChange_LastTimeUpdate))
		{	// Time changed
		}
		else if (channel == fPreferencesChangeChannel)
		{
			// Invalidate panes in case their grid settings changed

			// Update the shape of the handles
			fBuildingCache.SetHandleShape((uint32)GetModelerPrefs()->fHandleShape);

			// Update the unit system
			fBuildingCache.SetUnitSystem((EUnits)GetModelerPrefs()->fUnitSystem);

			// Reset the Working box
			DefaultWorkingBox();

			// Reset the colors
			SetUIColors();
	
			for( int32 iPane=0; iPane<(int32)fPaneExtensions.GetElemCount() ; iPane++ )
			{
				fPaneExtensions[iPane]->InvalidateRender();
			}
			fWorkingBox->InvalidateRenderables();
		}
		else if (channel == fTreePropertyChangeChannel)
		{
			if (changeKind == kChange_MasterObjectName)
			{
				// Update window title
				TMCCountedPtr<I3DShObject>	shObject;
				fShPrimitive->QueryInterface(IID_I3DShObject, (void**) &shObject);
				if (shObject == static_cast<I3DShObject*>(changedData))
				{
					// Update window title
					SetWindowTitle();
				}
			}
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingModeler::DataChanged"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingModeler::DataChanged"));
	}
}

void BuildingModeler::InvalidateDomainList()
{
	if(fClient)
		fClient->InvalidateDomainList();
}

void BuildingModeler::BeginImmediateUpdate()
{
	MY_ASSERT(!fImmediateUpdate);
	gChangeManager->PostChange(fImmediateUpdateChannel, kChange_BeginImmediateUpdate, fShPrimitive);
}

void BuildingModeler::PostImmediateUpdate(const boolean invalidateGeometry, const int32 selfDraw)
{
	MY_ASSERT(fImmediateUpdate);

	fSelfDraw = selfDraw;

	if (invalidateGeometry)
		InvalidateMeshes();

	gChangeManager->PostChange(fImmediateUpdateChannel, kChange_ImmediateUpdate, fShPrimitive);
}

void BuildingModeler::EndImmediateUpdate()
{
	MY_ASSERT(fImmediateUpdate);

	fSelfDraw = true; // default value for the camera tools

	gChangeManager->PostChange(fImmediateUpdateChannel, kChange_EndImmediateUpdate, fShPrimitive);
}

void BuildingModeler::ImmediateUpdate(const boolean invalidateGeometry, const int32 selfDraw)
{
	BeginImmediateUpdate();
	PostImmediateUpdate(invalidateGeometry, selfDraw);
	EndImmediateUpdate();
}

void BuildingModeler::RegularUpdate()
{
	gChangeManager->PostChange(fRegularUpdateChannel, 0, NULL);
}

void BuildingModeler::GetPanePartExt(BuildingPanePart** panePartExt, IMFPart* part)
{
	TMCCountedPtr<I3DEditorHostPanePart> editorHostPanePart;
	part->QueryInterface(IID_I3DEditorHostPanePart, (void**)&editorHostPanePart);
	ThrowIfNil(editorHostPanePart);

	TMCCountedPtr<IEx3DEditorHostPanePart> exEditorHostPanePart;
	editorHostPanePart->GetPaneExtension(&exEditorHostPanePart);
	ThrowIfNil(exEditorHostPanePart);

	exEditorHostPanePart->QueryInterface(CLSID_BuildingPanePart, (void**)panePartExt);
	ThrowIfNil(panePartExt);
}

void  BuildingModeler::SetWindowTitle()
{
	TMCDynamicString docName;
	fDoc->GetTitle(docName);

	TMCCountedPtr<I3DShObject>	object;
	fShPrimitive->QueryInterface(IID_I3DShObject, (void**) &object); ThrowIfNil(object);
	TMCDynamicString objectName;
	object->GetName(objectName);

	TMCString31 link;
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( link, kModelerStrings, 1);

	objectName+=link;
	objectName+=docName;

	TMCCountedPtr<IMFWindow> window;
	fWindow->QueryInterface(IID_IMFWindow, (void**) &window); ThrowIfNil(window);
	window->SetTitle(objectName);
}

ModelerPrefs* BuildingModeler::GetModelerPrefs()
{
	if(fModule) return static_cast<ModelerPrefs*>( fModule->GetPrefs() );
	else return NULL;
}

// TMCColorRGBA8 from TMCColorRGB
inline TMCColorRGBA8 ConvertRGBColor(const TMCColorRGB& color)
{
	return TMCColorRGBA8(color.red*255,color.green*255,color.blue*255,255);
}

void BuildingModeler::SetUIColors()
{
	ModelerPrefs* prefs = GetModelerPrefs();
	BuildingPrimData& data = fBuildingPrimitive->GetData();

	const TMCColorRGBA8 prevDefCol = data.fDefCol;
	const TMCColorRGBA8 prevSelCol = data.fSelCol;
	const TMCColorRGBA8 prevFloCol = data.fFloCol;

	data.fDefCol = ConvertRGBColor(prefs->fDefaultColor);
	data.fObjCol = ConvertRGBColor(prefs->fObjectColor);
	data.fSelCol = ConvertRGBColor(prefs->fSelectionColor);
	data.fTarCol = ConvertRGBColor(prefs->fTargetColor);
	data.fFreCol = ConvertRGBColor(prefs->fFreezeColor);
	data.fHelCol = ConvertRGBColor(prefs->fHelperColor);
	data.fSnaCol = ConvertRGBColor(prefs->fSnapColor);
	data.fBRoCol = ConvertRGBColor(prefs->fBotRoofColor);
	data.fTRoCol = ConvertRGBColor(prefs->fTopRoofColor);
	data.fFloCol = ConvertRGBColor(prefs->f2DFloorColor);

	if( data.fDefCol!=prevDefCol || data.fSelCol!=prevSelCol || data.fFloCol != prevFloCol )
	{	// Invalidate the facet mesh
		if(!IsFaceless())
			ImmediateUpdate( true, false );
	}
}

void BuildingModeler::NewPrimitiveFromPrefs()
{
	// Use the prefs to init the primitive settings

	MY_ASSERT(fBuildingPrimitive);

	BuildingPrimData& primData = fBuildingPrimitive->GetData();

	if(primData.GetIsNew())
	{
		primData.SetIsNew(false);

		ModelerPrefs* prefs = GetModelerPrefs();

		// Set Working Box settings
		primData.SetDefaultGridSpacing( prefs->fGridSpacing );
		primData.SetDefaultWBSizeX( prefs->fWorkingBoxSizeX );
		primData.SetDefaultWBSizeY( prefs->fWorkingBoxSizeY );
		primData.SetDefaultWBSizeZ( prefs->fWorkingBoxSizeZ );
		primData.SetDefaultSnapPrecision( prefs->fSnapPrecision );
		primData.SetDefaultConstrainAngle( prefs->fRotationConstraint );
		primData.SetDefaultSnap( prefs->fSnapToGrid );

		// Set dimension settings
		primData.SetDefaultLevelHeight( prefs->fLevelHeight, false );
		primData.SetDefaultRoofMin( prefs->fRoofMin );
		primData.SetDefaultRoofMax( prefs->fRoofMax );
		primData.SetDefaultWallThickness( prefs->fWallThickness );
		primData.SetDefaultFloorThickness( prefs->fFloorThickness, false );
		primData.SetDefaultCeilingThickness( prefs->fCeilingThickness, false );
		primData.SetDefaultWindowHeight( prefs->fWindowHeight, false );
		primData.SetDefaultWindowLength( prefs->fWindowLength );
		primData.SetDefaultWindowAltitude( prefs->fWindowAltitude, false );
		primData.SetDefaultDoorHeight( prefs->fDoorHeight, false );
		primData.SetDefaultDoorLength( prefs->fDoorLength );
		primData.SetDefaultStairwayWidth( prefs->fStairwayWidth );
		primData.SetDefaultStairwayLength( prefs->fStairwayLength );
	}
}

void BuildingModeler::PrefsFromPrimitiveDimension()
{
	const BuildingPrimData& primData = fBuildingPrimitive->Data();

	ModelerPrefs* prefs = GetModelerPrefs();

	// Set dimension settings
	prefs->fLevelHeight =		primData.GetDefaultLevelHeight();
	prefs->fRoofMin =			primData.GetDefaultRoofMin();
	prefs->fRoofMax =			primData.GetDefaultRoofMax();
	prefs->fWallThickness =		primData.GetDefaultWallThickness();
	prefs->fFloorThickness =	primData.GetDefaultFloorThickness();
	prefs->fCeilingThickness =	primData.GetDefaultCeilingThickness();
	prefs->fWindowHeight =		primData.GetDefaultWindowHeight();
	prefs->fWindowLength =		primData.GetDefaultWindowLength();
	prefs->fWindowAltitude =	primData.GetDefaultWindowAltitude();
	prefs->fDoorHeight =		primData.GetDefaultDoorHeight();
	prefs->fDoorLength =		primData.GetDefaultDoorLength();
	prefs->fStairwayWidth =		primData.GetDefaultStairwayWidth();
	prefs->fStairwayLength =	primData.GetDefaultStairwayLength();
}

void BuildingModeler::PrefsFromPrimitiveWB()
{
	const BuildingPrimData& primData = fBuildingPrimitive->Data();

	ModelerPrefs* prefs = GetModelerPrefs();

	// Set WB settings
	prefs->fGridSpacing =		primData.GetDefaultGridSpacing();
	prefs->fWorkingBoxSizeX =	primData.GetDefaultWBSizeX();
	prefs->fWorkingBoxSizeY =	primData.GetDefaultWBSizeY();
	prefs->fWorkingBoxSizeZ =	primData.GetDefaultWBSizeZ();
	prefs->fSnapPrecision =		primData.GetDefaultSnapPrecision();
	prefs->fRotationConstraint = primData.GetDefaultConstrainAngle();
	prefs->fSnapToGrid =		primData.GetDefaultSnap();
}

void BuildingModeler::DefaultWorkingBox()
{
	NewPrimitiveFromPrefs();

	// Use the primitive to build a working box

	BuildingPrimData& primData = fBuildingPrimitive->GetData();

	const real32 sizeX = primData.GetDefaultWBSizeX();
	const real32 sizeY = primData.GetDefaultWBSizeY();
	const real32 sizeZ = primData.GetDefaultWBSizeZ();
	const real32 spacing = primData.GetDefaultGridSpacing();

	const real32 halfX = (real32)spacing*(int32)((.5*sizeX)/(spacing));
	const real32 halfY = (real32)spacing*(int32)((.5*sizeY)/(spacing));
	const real32 halfZ = (real32)spacing*(int32)((.5*sizeZ)/(spacing));

	// Set the line Spacing
	const TVector2 space2(spacing,spacing);
	TMCCountedPtr<IWorkingBoxPlane> workingBoxPlane;
	for(int32 iPlane=0; iPlane<3; ++iPlane)
	{
		fWorkingBox->GetPlane(iPlane,&workingBoxPlane);
		ThrowIfNil(workingBoxPlane);
		workingBoxPlane->SetDelta(space2);
	}

	fWorkingBox->SetCurrentPlane(0);

	// bbox centered in 0,0,halfSize.
	TBBox3D		bbox;
/*	bbox.SetMin( - TVector3(sizeX-halfX, sizeY-halfY, 0));
	bbox.SetMax( TVector3(halfX, halfY, sizeZ));*/
	bbox.SetMin( - TVector3(sizeX*.5, sizeY*.5, sizeZ*.5));
	bbox.SetMax( TVector3(sizeX*.5, sizeY*.5, sizeZ*.5));
	fWorkingBox->SetBoundingBox(bbox);

	// working box centered in epsX,epsY,halfSize.
	// the working box is slightly excentered to be able to use the snap to grid option
#if (VERSIONNUMBER >= 0x060000)
	TTransform3D origin;
	origin.fTranslation.SetValues( sizeX*.5 - halfX, sizeY*.5 - halfY, sizeZ*.5 );
	fWorkingBox->SetTransform(origin);
#elif (VERSIONNUMBER >= 0x040000)
	TTreeTransform origin;
	origin.SetOffset( TVector3( sizeX*.5 - halfX, sizeY*.5 - halfY, sizeZ*.5 ) );
	fWorkingBox->SetTreeTransform(origin);
#else
	TTreeTransform origin;
	origin.SetOffset( TVector3( sizeX*.5 - halfX, sizeY*.5 - halfY, sizeZ*.5 ) );
	fWorkingBox->SetGlobalTreeTransform(origin);
#endif

	fWorkingBox->InvalidateRenderables();

	// Update the panes
	f3DEditorHostPart->UpdateWorkingBoxImages();
	// Set the new scene magnitude.
	// The spacing gives a good idea of what the scene magnitude should be
	real32 sceneMagnitude = 0.5*spacing; // default magnitude: 1 unit = 3 feet = 3*12 inches
	SetSceneMagnitude(sceneMagnitude);
}

void BuildingModeler::SetSceneMagnitude(real32 sceneMagnitude)
{
	f3DEditorHostPart->SetGeneralSize(sceneMagnitude);
	fBuildingPrimitive->GetData().SetGeneralSize(sceneMagnitude);

	// Increase the movement speed of the camera
	real32 inMovSpeed = 0;
	real32 inRotSpeed = 0;
	f3DEditorHostPart->GetDefaultSpeeds(inMovSpeed, inRotSpeed);
	f3DEditorHostPart->SetDefaultSpeeds(sceneMagnitude, inRotSpeed);
}

boolean BuildingModeler::GetToolDropCandidate(	IMFDropCandidate**	outDropCandidate,
												int32 inToolID,
												IMFToolbarPart* inToolbarPart)
{
	ReleaseIfNotNull<IMFDropCandidate>(outDropCandidate);

	if(	IsWindowToolID(inToolID) ||
		IsDoorToolID(inToolID) ||
		IsStairwayToolID(inToolID) ||
		IsLevelToolID(inToolID) )
	{
		gDragAndDropUtilities->CreateDropCandidate('BMDC', outDropCandidate);
		ThrowIfNil(*outDropCandidate);

		TMCCountedPtr<BuildingDropCandidate> drop;
		(*outDropCandidate)->QueryInterface(CLSID_BuildingDropCandidate, (void**)&drop);
		ThrowIfNil(drop);

		drop->Initialize(inToolID);
	}

	return true;
}

const Quotation& BuildingModeler::GetQuotation() const 
{
	return fBuildingCache.GetQuotation();
}

void BuildingModeler::GetCurrentFacetMesh(FacetMesh** outMesh, const boolean withNormals, const boolean mesh2D)
{
	TMCCountedGetHelper<FacetMesh> result(outMesh);
	result= (withNormals) ? fBuildingCache.GetFacetMesh(mesh2D,mesh2D?fBuildingPrimitive->ActiveLevel():kAllLevels)
		: fBuildingCache.GetFlatFacetMesh(mesh2D,mesh2D?fBuildingPrimitive->ActiveLevel():kAllLevels);
}

void BuildingModeler::GetBuildingPrimitive(BuildingPrim** building)
{
	TMCCountedGetHelper<BuildingPrim> result(building);
	result = fBuildingPrimitive;
}

void BuildingModeler::GetSelection(ISceneSelection** outSelection)
{
	TMCCountedGetHelper<ISceneSelection> result(outSelection);
	result = fSelection;
}

void BuildingModeler::GetSelectionChannel(IChangeChannel** outChannel)
{
	TMCCountedGetHelper<IChangeChannel> result(outChannel);
	result = fSelectionChangeChannel;
}

void BuildingModeler::PostToSelectionChannel(IDType changeKind)
{
	gChangeManager->PostChange(fSelectionChangeChannel, changeKind, fSelection);
}

void SetRenderableTransform(TRenderableAndTfm& renderable)
{
/*	for( int32 i=0; i<3; ++i )
	{
		for( int32 j=0; j<3; ++j )
		{
			renderable.fT.fRotationAndScale[i][j]=(i==j)?kRealOne:kRealZero;
			renderable.fModelFromWorldTfm.fRotationAndScale[i][j]=(i==j)?kRealOne:kRealZero;
		}
		renderable.fT.fTranslation[i]=kRealZero;
		renderable.fModelFromWorldTfm.fTranslation[i]=kRealZero;
	}*/
}

boolean BuildingModeler::Get2DFlatFacetRenderable(TRenderableAndTfm& facetsRenderable)
{
	facetsRenderable.fInstance=nil;
	facetsRenderable.fRenderable = f2DFlatFacetRenderable;
	SetRenderableTransform(facetsRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get3DFlatFacetRenderable(TRenderableAndTfm& facetsRenderable)
{
	facetsRenderable.fInstance=nil;
	facetsRenderable.fRenderable = f3DFlatFacetRenderable;
	SetRenderableTransform(facetsRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get2DFacetRenderable(TRenderableAndTfm& facetsRenderable)
{
	facetsRenderable.fInstance=nil;
	facetsRenderable.fRenderable = f2DFacetRenderable;
	SetRenderableTransform(facetsRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get3DFacetRenderable(TRenderableAndTfm& facetsRenderable)
{
	facetsRenderable.fInstance=nil;
	facetsRenderable.fRenderable = f3DFacetRenderable;
	SetRenderableTransform(facetsRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get2DSegmentRenderable(TRenderableAndTfm& edgesRenderable)
{
	edgesRenderable.fInstance=nil;
	edgesRenderable.fRenderable = f2DSegmentRenderable;
	SetRenderableTransform(edgesRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get3DSegmentRenderable(TRenderableAndTfm& edgesRenderable)
{
	edgesRenderable.fInstance=nil;
	edgesRenderable.fRenderable = f3DSegmentRenderable;
	SetRenderableTransform(edgesRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get2DPointRenderable(TRenderableAndTfm& pointRenderable)
{
	pointRenderable.fInstance=nil;
	pointRenderable.fRenderable = f2DPointRenderable;
	SetRenderableTransform(pointRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

boolean BuildingModeler::Get3DPointRenderable(TRenderableAndTfm& pointRenderable)
{
	pointRenderable.fInstance=nil;
	pointRenderable.fRenderable = f3DPointRenderable;
	SetRenderableTransform(pointRenderable);

	return true; //fBuildingPrim->HasRenderablePolygons();
}

void BuildingModeler::MakeGridLevelRenderable( TRenderableAndTfm& gridRenderable, 
			const real32 altitude)
{
	fLevelGridRenderable->SetAltitude(altitude);

	gridRenderable.fInstance=nil;
	gridRenderable.fRenderable = fLevelGridRenderable;
	SetRenderableTransform(gridRenderable);
}

const TSegmentMesh* BuildingModeler::GetLevelGridSegmentMesh(const real32 altitude)
{	// Draw a grid the size of the bbox at this altitude
	fExtraMesh.fVertex.ArrayFree();
	fExtraMesh.fSegmentColors.ArrayFree();
	fExtraMesh.fSegmentIndices.ArrayFree();

	real32 xMin=-5, yMin=-5, xMax=5, yMax=5;
	TBBox3D bbox;
	fBuildingPrimitive->GetBoundingBox(bbox);
	if(bbox.Valid())
	{
		xMin = bbox.fMin.x;
		yMin = bbox.fMin.y;
		xMax = bbox.fMax.x;
		yMax = bbox.fMax.y;
	}

	TVector3 pos(xMin,yMin,altitude);
	fExtraMesh.fVertex.AddElem(pos);
	pos.x = xMax;
	fExtraMesh.fVertex.AddElem(pos);
	pos.y = yMax;
	fExtraMesh.fVertex.AddElem(pos);
	pos.x = xMin;
	fExtraMesh.fVertex.AddElem(pos);
			
	TIndex2 index2;
	for(int32 iLine=0 ; iLine<4 ; iLine++)
	{
		index2[0]=iLine;
		index2[1]=(iLine+1)%4;
		fExtraMesh.fSegmentColors.AddElem(GetModelerPrefs()->fTargetColor);
		fExtraMesh.fSegmentIndices.AddElem(index2);
	}

	return &fExtraMesh;
}

void BuildingModeler::SetBackdropImageWithFile( const TMCDynamicString& filePath, const uint16 planeID )
{
	TMCCountedPtr<IWorkingBoxPlane> workingPlane;
	fWorkingBox->GetPlane(planeID, &workingPlane);
	ThrowIfNil(workingPlane);

	TMCDynamicString pathName = filePath;
	workingPlane->SetPlaneImageWithPathName( pathName );

	// Record the backdrop data
	BuildingPrimData& primData = fBuildingPrimitive->GetData();
	switch(planeID)
	{
	case kWBXDrawPlane: primData.SetFBBackdrop(filePath); break;
	case kWBYDrawPlane: primData.SetLRBackdrop(filePath); break;
	case kWBZDrawPlane: primData.SetTBBackdrop(filePath); break;
	}
	ActivateBackdrop((filePath.Length()>0), planeID);

	// Update the panes
	f3DEditorHostPart->UpdateWorkingBoxImages();

	// Invalidate the properties so the Active check bax can be checked
	InvalidatePropertiesModule();

	if(!IsFaceless())
		ImmediateUpdate( false, false );
}

void BuildingModeler::ActivateBackdrop( const boolean active, const uint16 planeID )
{
	TMCCountedPtr<IWorkingBoxPlane> workingPlane;
	fWorkingBox->GetPlane(planeID, &workingPlane);
	ThrowIfNil(workingPlane);

	workingPlane->ActivatePlaneImage( active );

	// Record the backdrop data
	BuildingPrimData& primData = fBuildingPrimitive->GetData();
	switch(planeID)
	{
	case kWBXDrawPlane: primData.SetFBEnable(active); break;
	case kWBYDrawPlane: primData.SetLREnable(active); break;
	case kWBZDrawPlane: primData.SetTBEnable(active); break;
	}

	// Update the panes
	f3DEditorHostPart->UpdateWorkingBoxImages();

	if(!IsFaceless())
		ImmediateUpdate( false, false );
}

/*
void BuildingModeler::PromotePrimitiveToMasterGroup()
{
	// Should we check first if we're not already a master group ?

	TMCCountedPtr<I3DShTreeElement> stackTop;
	fSceneDoc->GetJumpInStackTop(&stackTop);

	// if the stackTop is a group then the master group contains this group
	I3DShMasterGroup* currentMasterGroup = stackTop->GetMasterGroup();
	ThrowIfNil(currentMasterGroup);

	// if the stackTop is a sceneInstance, then the top scene is the scene it points on
	{
		TMCCountedPtr<I3DShInstance> sceneInstance;
		stackTop->QueryInterface(IID_I3DShInstance,(void**)&sceneInstance);

		if (sceneInstance && sceneInstance->GetInstanceKind()==I3DShInstance::kSceneInstance)
		{
			TMCCountedPtr<I3DShObject> object;
			sceneInstance->Get3DObject(&object);
			ThrowIfNil(object);

			object->QueryInterface(IID_I3DShMasterGroup,(void**)&currentMasterGroup);
			ThrowIfNil(currentMasterGroup);
		}
	}

	TMCCountedPtr<I3DShGroup> rootGroup;
	TMCCountedPtr<I3DShTreeElement> rootTree;
	currentMasterGroup->GetTreeRoot(&rootGroup);
	ThrowIfNil(rootGroup);
	rootGroup->QueryInterface(IID_I3DShTreeElement, (void **)&rootTree);
	
	TMCCountedPtr<I3DShMasterGroup> newMasterGroup;
	gComponentUtilities->CoCreateInstance(CLSID_MasterGroup, NULL, 1, IID_I3DShMasterGroup, (void **)&newMasterGroup);
	ThrowIfNil(newMasterGroup);
	
	TMCCountedPtr<I3DShInstance> newSceneInstance;
	gComponentUtilities->CoCreateInstance(CLSID_StandardSceneInstance, NULL, 1, IID_I3DShInstance, (void **)&newSceneInstance);
	ThrowIfNil(newSceneInstance);
	
	TMCCountedPtr<I3DShObject> newMasterGroupObject;
	newMasterGroup->QueryInterface(IID_I3DShObject, (void **)&newMasterGroupObject);
	ThrowIfNil(newMasterGroupObject);
	newSceneInstance->Set3DObject(newMasterGroupObject);

	// Get the first tree elem of the primitive
	TMCCountedPtr<I3DShObject> primitiveObject;
	fShPrimitive->QueryInterface(IID_I3DShObject,(void**)&primitiveObject);
	ThrowIfNil(primitiveObject);
	TMCCountedPtrArray<I3DShInstance> instances;
	primitiveObject->GetInstanceArray(instances);
	TMCCountedPtr<I3DShTreeElement> treeElem;
	instances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElem);
	TMCDynamicString name;
	treeElem->GetName(name);
	newMasterGroupObject->SetName(name);

	fScene->Insert3DObject(newMasterGroupObject);

	RelinkTreeElementInfo info;
	TMCCountedPtr<I3DShTreeElement> oldFather;
	treeElem->Unlink(&oldFather, &info);

	TMCCountedPtr<I3DShGroup> group;
	treeElem->QueryInterface(IID_I3DShGroup, (void **)&group);

	treeElem->SetMasterGroup(newMasterGroup);

	TMCCountedPtr<I3DShTreeElement> newSceneInstanceTree;
	newSceneInstance->QueryInterface(IID_I3DShTreeElement, (void **)&newSceneInstanceTree);
	ThrowIfNil(newSceneInstanceTree);

	// the transform of the scene instance
	TTreeTransform treeTransform;

	if (group)
	{
		TMCCountedPtr<I3DShTreeElement> groupTree;
		group->QueryInterface(IID_I3DShTreeElement, (void **)&groupTree);

		groupTree->GetLocalTreeTransform(treeTransform);

		TTreeTransform identityTransform;
		groupTree->SetLocalTreeTransform(identityTransform);
	}
	else
	{
		// the inserted tree is not a group, so create one and put it in
		gComponentUtilities->CoCreateInstance(CLSID_StandardGroup, NULL, 1, IID_I3DShGroup, (void **)&group);
		ThrowIfNil(group);

		TTransform3D transform;
		treeElem->GetLocalTreeTransform(treeTransform);

		TTreeTransform identityTransform;
		treeElem->SetLocalTreeTransform(identityTransform);
		
		TMCCountedPtr<I3DShTreeElement> groupTree;
		group->QueryInterface(IID_I3DShTreeElement, (void **)&groupTree);
		groupTree->SetMasterGroup(newMasterGroup);
		groupTree->InsertLast(treeElem);

		newSceneInstanceTree->SetGlobalTransform3D(transform);
	}
	newSceneInstanceTree->SetLocalTreeTransform(treeTransform);
	newSceneInstanceTree->SetName(name);

	// we call SetScene before SetTreeRoot to make sure that this scene is recognized
	// as a Master Group and not an independant scene
	
	newMasterGroup->SetScene(fScene);
	newMasterGroup->SetTreeRoot(group);
	
	rootTree->InsertLast(newSceneInstanceTree);
	
	TMCCountedPtr<ISelectableObject> newSceneInstanceSelectable;
	newSceneInstance->QueryInterface(IID_ISelectableObject, (void **)&newSceneInstanceSelectable);
	ThrowIfNil(newSceneInstanceSelectable);

	TTreeSelectionIterator selectionIter(fSelection);
	I3DShTreeElement* tree;

	for (tree = selectionIter.First(); selectionIter.More(); tree = selectionIter.Next())
	{
		tree->PostMoveChange();
	}

	newSceneInstanceSelectable->AddToSelection(fSelection, false);

	selectionIter = TTreeSelectionIterator(fSelection);
	for (tree = selectionIter.First(); selectionIter.More(); tree = selectionIter.Next())
	{
		tree->PostMoveChange();
	}
	
//	gChangeManager->PostChange(fSceneSelectionChannel, 0, fSelection);
//	
//	gChangeManager->PostChange(fTreeHierarchyChannel, 0, newSceneInstance);
}*/

#endif // !NETWORK_RENDERING_VERSION

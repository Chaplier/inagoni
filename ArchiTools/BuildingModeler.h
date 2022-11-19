/****************************************************************************************************

		BuildingModeler.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/23/2004

****************************************************************************************************/
#ifndef __BuildingModeler__
#define __BuildingModeler__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "BasicModule.h"

#include "MCCountedPtr.h"
#include "MBuildingPanePart.h"
#include "MBuildingCache.h"
#include "MBuildingRenderable.h"
#include "IMFDragDrop.h"
struct IPropertiesModule;
struct I3DEditorHostPart;
#include "I3dShObject.h"
#include "I3DShModule.h"
#include "IWorkingBox.h"

#if (VERSIONNUMBER > 0x040000)
#include "I3dExMiniModeler.h"
#endif

#include "I3DShFacetMesh.h"
#include "BasicModule.h"
#include "BasicPropertiesClient.h"

class BuildingPrim;
class BuildingPropertiesClient;
class BuildingPanePart;
class Picking;
class Constraint;

extern const MCGUID CLSID_BuildingModeler;

// Global preferences to the modeler
struct ModelerPrefs // ( <=> kBuildingPrefs )
{
	ModelerPrefs() {fWorkingBoxSizeX=fWorkingBoxSizeY=fWorkingBoxSizeZ=1;fSnapPrecision=.5;fGridSpacing=2;fTextSize=16;}

	// WB ( use the one in the primitive, these one are just for the default value at the creation time)
	real32			fWorkingBoxSizeX;
	real32			fWorkingBoxSizeY;
	real32			fWorkingBoxSizeZ;
	real32			fGridSpacing;
	real32			fSnapPrecision;
	real32			fRotationConstraint;
	// UI
	int32			fPreferedSplitSize; // split the properties panel
	uint32			fHandleShape; // Shape of the points
	int32			fUnitSystem; // inches, mm, ...

	TMCColorRGBA	fDefaultColor;
	TMCColorRGBA	fObjectColor;
	TMCColorRGBA	fSelectionColor;
	TMCColorRGBA	fTargetColor;
	TMCColorRGBA	fFreezeColor;
	TMCColorRGBA	fHelperColor;
	TMCColorRGBA	fSnapColor;
	TMCColorRGBA	fBotRoofColor;
	TMCColorRGBA	fTopRoofColor;
	TMCColorRGBA	f2DFloorColor;
	TMCColorRGBA	fTextColor;
	int32			fTextSize;

	// Snap ( use the one in the primitive, these one are just for the default value at the creation time)
	boolean			fSnapToGrid;
	// Quotation
	boolean			fShowDimensions;	

	// Dimensions ( use the one in the primitive, these one are just for the default value at the creation time)
	real32			fLevelHeight;
	real32			fRoofMin;
	real32			fRoofMax;
	real32			fWallThickness;
	real32			fFloorThickness;
	real32			fCeilingThickness;
	real32			fWindowHeight;
	real32			fWindowLength;
	real32			fWindowAltitude;
	real32			fDoorHeight;
	real32			fDoorLength;
	real32			fStairwayWidth;
	real32			fStairwayLength;
};

// Local preferences to the modeler. Can it works ?
struct ModelerPMap
{
	ModelerPMap(){fDefaultParam=0;}

	int32 fDefaultParam;
};

class BuildingModeler : public TBasicModule
						, public I3DExMiniModeler

{
public :  
	BuildingModeler();
	virtual ~BuildingModeler();

	// IMCUnknown methods	
	STANDARD_RELEASE;

	MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	uint32		MCCOMAPI AddRef() { return TBasicModule::AddRef(); }

	// IExDataExchanger methods :
	void*		MCCOMAPI GetExtensionDataBuffer(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(ModelerPMap);}

	// I3DExModule methods
	virtual int32	MCCOMAPI GetPrefsBufferSize() const {return sizeof(ModelerPrefs);}
	MCCOMErr 	MCCOMAPI Initialize(IMCUnknown* inElement);
	MCCOMErr 	MCCOMAPI Destroy();
	MCCOMErr 	MCCOMAPI Hydrate();
	MCCOMErr 	MCCOMAPI Dehydrate();
	MCCOMErr	MCCOMAPI Activate();
	MCCOMErr	MCCOMAPI Deactivate();

	MCCOMErr	MCCOMAPI BuildMenuBar();
	void		MCCOMAPI SelfPrepareMenus();
	boolean		MCCOMAPI SelfMenuAction(ActionNumber actionNumber);
	boolean		MCCOMAPI SelfToolAction(int32 inOldTool, int32 inNewTool);

	boolean 	MCCOMAPI GetToolDropCandidate(	IMFDropCandidate**	outDropCandidate,
												int32 inToolID,
												IMFToolbarPart* inToolbarPart);

	boolean		MCCOMAPI Receive(int32 message, IMFResponder* source, void* data){return false;}

	void 		MCCOMAPI DataChanged(IChangeChannel*	channel, 
									 IDType 			changeKind, 
									 IMCUnknown* 		changedData);
	MCCOMErr	MCCOMAPI AboutToCloseMainWindow();

	virtual void MCCOMAPI GetToolBarsInfo(TMCArray<TToolBarInfo>& toolbarList);

	// I3DExMiniModeler methods
	MCCOMErr	MCCOMAPI CreateMiniModelerUI(IMFPart** UIPart, IMCUnknown* inElement, IMFDocument* inDocument);
	MCCOMErr	MCCOMAPI CleanUp();
	boolean		MCCOMAPI CanModify(	IMFResponder*		inSource,
											int32				inMessage);
	boolean		MCCOMAPI HandleUIPartHit(	IMFPart*			inTopPart,
													int32				inMessage,
													IMFResponder*		inSource,
													void*				inData);
	// Initialization common to the 2 modelers
	MCCOMErr				BasicInitialize(IMCUnknown* inElement, IMFDocument* inDocument);

	// Initialisation, Destruction
	MCCOMErr 				Initialize1(I3DShGroup* universe);
	MCCOMErr 				Initialize2(IMCUnknown* inElement);
	void					CreateWindows();
	void					DestroyWindows();

	boolean					IsFaceless(){ return (fWindow == (IMFPart*)NULL) || (!fModule->GetIsHydrated());}

	I3DShPrimitive*			GetPrimitiveNoAddRef() { return fShPrimitive; }
	BuildingPrim*			GetBuildingNoAddRef() { return fBuildingPrimitive; }
	void					GetBuildingPrimitive(BuildingPrim** building);
	IWorkingBox*			GetWorkingBoxNoAddRef() { return fWorkingBox; }
	ModelerPrefs*			GetModelerPrefs();
	real32					GetGridSpacingPref() const {return fBuildingPrimitive->Data().GetDefaultGridSpacing();}
	real32					GetSnapPrecision() const {return fBuildingPrimitive->Data().GetDefaultSnapPrecision();}
	boolean					SnapToGrid() const {return fBuildingPrimitive->Data().GetDefaultSnap();}
	boolean					ShowDimensions() {return GetModelerPrefs()->fShowDimensions;}
	real32					GetRotationConstraint() const {return fBuildingPrimitive->Data().GetDefaultConstrainAngle();}
	IMFResponder*			GetContext() {return fContext;}

	// Cache access
	void					GetCurrentFacetMesh(FacetMesh** outMesh, const boolean withNormals, const boolean mesh2D);
	const TSegmentMesh*		GetCurrentSegmentMesh(const int32 mesh2D)	{ return fBuildingCache.GetSegmentMesh(mesh2D,mesh2D?fBuildingPrimitive->ActiveLevel():kAllLevels); }
	const TPointMesh*		GetCurrentPointMesh(const int32 mesh2D)	{ return fBuildingCache.GetPointMesh(mesh2D,mesh2D?fBuildingPrimitive->ActiveLevel():kAllLevels); }
	void					GetCurrentBoundingBox(TBBox3D& bbox)	{ fBuildingCache.GetBoundingBox(bbox); }
	// Other meshes 
	const TSegmentMesh*		GetLevelGridSegmentMesh(const real32 altitude);
	// Renderable access
	boolean					Get2DFlatFacetRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get3DFlatFacetRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get2DFacetRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get3DFacetRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get2DSegmentRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get3DSegmentRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get2DPointRenderable(TRenderableAndTfm& facetsRenderable);
	boolean					Get3DPointRenderable(TRenderableAndTfm& facetsRenderable);
	// Extra renderable access
	void					MakeGridLevelRenderable( TRenderableAndTfm& gridRenderable, const real32 altitude);
	const Quotation&		GetQuotation() const;

	boolean					GetImmediateUpdate() {return fImmediateUpdate;}
	// Invalidate the cached facet mesh
	void					InvalidateMeshes() {fBuildingCache.InvalidateCache(); }
	// Does not invalidate the cached facet mesh but only some of it's attributes
	void					InvalidateMeshesAttributes( boolean updateProperties ) { fBuildingCache.InvalidateModelerData(); 
															if(updateProperties) PostToSelectionChannel(0); }
	void					InvalidatePrimitive() { fShPrimitive->ChangedData(); }

	void					InvalidateDomainList();

	void					BeginImmediateUpdate();
	void					PostImmediateUpdate(const boolean invalidateGeometry, const int32 selfDraw);
	void					EndImmediateUpdate();
	void					ImmediateUpdate(const boolean invalidateGeometry, const int32 selfDraw);
	void					RegularUpdate();

	// Tool actions
	boolean					KeyDownFromPane(BuildingPanePart* pane,const TMCPlatformEvent& inEvent);
	boolean					MouseMovedFromPane(BuildingPanePart* pane, const TMCPoint& inWhere, const TMCModifiers& modifier);
	boolean					MouseDownFromPane(BuildingPanePart* pane, const TMCPoint& inWhere, const TMCPlatformEvent& inEvent);

	// Data access
	inline const int32		Get2DActiveLevel() const {return fBuildingPrimitive->ActiveLevel();}
	inline const int32		Get3DActiveLevel() const ;
	inline void				Invalidate2DCache() { fBuildingCache.Invalidate2DData(); }

	inline I3DShScene*		GetScene() const {return fScene;}

	inline VPoint*			GetPointHelper() const {return fPointHelper;}
	inline boolean			SetPointHelper(VPoint* point, const boolean checkPrevious);
	inline Wall*			GetWallHelper() const {return fWallHelper;}
	inline void				SetWallHelper(Wall* wall, const boolean checkPrevious);

	void					GetSelection(ISceneSelection** outSelection);
	void					GetSelectionChannel(IChangeChannel** outChannel);

	inline void				InvalidatePropertiesModule(){PostToSelectionChannel(0);}
	void					PostToSelectionChannel(IDType changeKind);

	// Snap util
	VPoint*					SnapPointPos(Picking& picked,
									 BuildingPanePart* pane, 
									 const TMCPoint& inWhere,
									 VPoint* forPoint,
									 const int32 inLevel,
									 const boolean buildNewPoint,
									 boolean& pickExistingPoint);
	void					SnapPoint(VPoint* point);
	void					SnapCommonPoint(CommonPoint* point, const TMCClassArray<Line2D>& directions, const boolean ctrl, const boolean shift);
	void					PreparePointConstraints(CommonPoint* point, TMCClassArray<Line2D>& directions);
	const TMCClassArray<Line2D>& GetDirections(){ return fDirections; }

	uint16					GetCurrentPlane(){ return fWorkingBox->GetCurrentPlane(); }

	void					SaveViewSettings();

	void					SetBackdropImageWithFile( const TMCDynamicString& filePath, const uint16 planeID );
	void					ActivateBackdrop( const boolean active, const uint16 planeID );

	void					DefaultWorkingBox(); // Use the primitive data to built it
	void					PrefsFromPrimitiveDimension();// copy the primitive dimensions into the prefs
	void					PrefsFromPrimitiveWB();// copy the primitive WB into the prefs

	I3DShGroup*				GetUniverse(){return fUniverse;}

protected:
	void				SetSceneMagnitude(real32 sceneMagnitude);
	void				GetPanePartExt(BuildingPanePart** panePart, IMFPart* part);
	void				SetWindowTitle();
	void				NewPrimitiveFromPrefs();// copy the pref into the new primitive
	void				SetUIColors();

	void				GetGridConstraint( Constraint& cstr, const TVector2& newPos);
	void				GetDirConstraint(Constraint& cstr, 
									   const TMCClassArray<Line2D>& directions,
									   const TVector2& pos);
//	void				PromotePrimitiveToMasterGroup();

	boolean				ResetPointHelper();

private:

	TMCCountedPtr<I3DShGroup>			fUniverse;
	TMCCountedPtr<I3DShScene>			fScene;
	TMCCountedPtr<IMFDocument>			fDoc;
	TMCCountedPtr<IMFResponder>			fContext;
	TMCCountedPtr<ISceneDocument>		fSceneDoc;
	TMCCountedPtr<I3DShModule>			fModule; // Building modeler module
	TMCCountedPtr<I3DShModule>			fHierarchyModule;
	TMCCountedPtr<IPropertiesModule>	fPropertiesModule;
	TMCCountedPtr<ISceneSelection>		fSelection;
	TMCCountedPtr<IChangeChannel>		fSelectionChangeChannel;
	TMCCountedPtr<BuildingPropertiesClient> fClient;

	TMCCountedPtr<IChangeChannel>		fImmediateUpdateChannel;
	TMCCountedPtr<IChangeChannel>		fRegularUpdateChannel;
	TMCCountedPtr<IChangeChannel>		fTreePropertyChangeChannel;
	TMCCountedPtr<IChangeChannel>		fTimeChangeChannel;

	TMCCountedPtr<IMFPart>				fAssembleRoomPart;
	TMCCountedPtr<IMFPart>				fWindow;
	TMCCountedPtr<IMFPart>				fHostPart; // editorHost part, contains the panes
	TMCCountedPtr<I3DEditorHostPart>	f3DEditorHostPart;
	TMCCountedPtrArray<IMFPart>			fPanes;
	TMCCountedPtrArray<BuildingPanePart>	fPaneExtensions;
	TMCCountedPtr<IWorkingBox>			fWorkingBox;

	TMCCountedPtr<I3DShPrimitive>		fShPrimitive;	// shell primitive
	TMCCountedPtr<BuildingPrim>			fBuildingPrimitive;

	// Modeler pmap
	ModelerPMap		fPMap;

	// Some data
	int32			fActionBeingProcessed; // tells us when camera is moving
	boolean			fImmediateUpdate; // true in immediate update mode
	boolean			fSelfDraw; // So we know what to redraw when in immediate update mode	
	// Renderable
	TMCCountedPtr<BuildingMeshRenderable>	f2DFlatFacetRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f3DFlatFacetRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f2DFacetRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f3DFacetRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f2DSegmentRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f3DSegmentRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f2DPointRenderable;
	TMCCountedPtr<BuildingMeshRenderable>	f3DPointRenderable;
	// Extra renderables
	TMCCountedPtr<LevelGridRenderable>		fLevelGridRenderable;
	TSegmentMesh							fExtraMesh;

	// Cache
	BuildingCache	fBuildingCache;

	// Current level: can be kAllLevels
//	int32			fActiveLevel;

	// Point used to build a wall
	TMCCountedPtr<VPoint>	fPointHelper;
	TMCCountedPtr<Wall>		fWallHelper;
	TMCClassArray<Line2D>	fDirections; // Snapping data cached here
};

inline const int32 BuildingModeler::Get3DActiveLevel() const
{
	if(fPointHelper) // we're currently building stuff in this level, do everything in
		return fPointHelper->GetLevelIndex();
	else if(fBuildingPrimitive->ShowAll())
		return kAllLevels;
	else
		return fBuildingPrimitive->ActiveLevel();
}

// return true if the previous point helper was merged
inline boolean BuildingModeler::SetPointHelper(VPoint* point, const boolean checkPrevious)
{
	boolean merged = false;

	// would this be helpfull ?
	Level* level = fBuildingPrimitive->GetLevel( fBuildingPrimitive->ActiveLevel() );
	if(level)
	{
		level->ClearFlag(ePointHelper);
	}

	if(point)
		point->SetFlag(ePointHelper);
	// Remove the previous wall
	if(fPointHelper)
	{
		fPointHelper->ClearFlag(ePointHelper);
		if(checkPrevious/* && AutoMerge()*/) // can't block this one, its essencial to the room building
			merged = fPointHelper->CheckConsistency(true);
	}
	// Set the new one
	fPointHelper=point;

	// Init the snapping data
	PreparePointConstraints( point, fDirections );

	return merged;
}

inline void BuildingModeler::SetWallHelper(Wall* wall, const boolean checkPrevious)
{
	// would this be helpfull ?
	Level* level = fBuildingPrimitive->GetLevel( fBuildingPrimitive->ActiveLevel() );
	if(level)
	{
		level->ClearFlag(eWallHelper);
	}

	// Set the flag on the new wall at the begining so it won't be used for the
	// consistency check
	if(wall)
		wall->SetFlag(eWallHelper);
	// Remove the previous wall
	if(fWallHelper)
	{
		fWallHelper->ClearFlag(eWallHelper);
		if(checkPrevious)
			fWallHelper->CheckConsistency(true);
	}
	// Set the new one
	fWallHelper=wall;
}

inline boolean IsWindowToolID(const int32 inToolID) 
{
	if(	inToolID == kInsertWindowTool ||
		inToolID == kInsertNarrowWindowTool ||
		inToolID == kInsertPanoWindowTool ||
		inToolID == kInsertArrowWindowTool ||
		inToolID == kInsertCircleWindowTool ||
		inToolID == kInsert2CircleWindowTool ||
		inToolID == kInsert4CircleLWindowTool ||
		inToolID == kInsert4CircleRWindowTool )
		return true;

	return false;

}

inline boolean IsDoorToolID(const int32 inToolID)
{
	if(	inToolID == kInsertDoorTool ||
		inToolID == kInsertDoubleDoorTool ||
		inToolID == kInsertArrowDoorTool ||
		inToolID == kInsert2CircleDoorTool ||
		inToolID == kInsert4CircleLDoorTool ||
		inToolID == kInsert4CircleRDoorTool )
		return true;

	return false;
}

inline boolean IsStairwayToolID(const int32 inToolID)
{
	if(	inToolID == kInsertSquareStairwayTool ||
		inToolID == kInsertLargeStairwayTool ||
		inToolID == kInsertWideStairwayTool ||
		inToolID == kInsertCircleStairwayTool )
		return true;

	return false;
}

inline boolean IsLevelToolID(const int32 inToolID)
{
	if(	inToolID == kInsertShellLevelTool ||
		inToolID == kInsertDuplicateLevelTool ||
		inToolID == kInsertEmptyLevelTool )
		return true;

	return false;
}

#endif

#endif // !NETWORK_RENDERING_VERSION

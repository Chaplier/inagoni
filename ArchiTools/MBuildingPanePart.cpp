/****************************************************************************************************

		MBuildingPanePart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/23/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingPanePart.h"

#include "MBuildingDragAndDrop.h"
#include "MPicking.h"

#include "IShUtilities.h"
#include "COMUtilities.h"
#include "COM3DUtilities.h"
#include "MiscComUtilsImpl.h"
#include "I3DExRendererBox.h"
#include "I3DShObject.h"
#include "I3DEditorHostPartDefs.h"
#include "I3DShShader.h"
#include "IWorkingBox.h"
#include "MFCursors.h"
#include "IMCGraphicContext.h"
#include "PQuotation.h"
//Alan
#include "MCPoint.h"
#include "MCRect.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
const MCGUID CLSID_BuildingPanePart(R_CLSID_BuildingPanePart);
#else
const MCGUID CLSID_BuildingPanePart={R_CLSID_BuildingPanePart};
#endif

BuildingPanePart::BuildingPanePart()
{
	fCursorID = kUnknownCursor;
	fUnderType = eNothingPicked;
}

BuildingPanePart::~BuildingPanePart()
{
}

MCCOMErr BuildingPanePart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BuildingPanePart))
	{
		TMCCountedGetHelper<BuildingPanePart> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	else if(MCIsEqualIID(riid,IID_IEx3DEditorHostPanePart))
	{
		TMCCountedGetHelper<IEx3DEditorHostPanePart> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void  BuildingPanePart::SetModeler (BuildingModeler* modeler)
{
	fBuildingModeler= modeler;
}
	
void BuildingPanePart::Set3DEditorHostPanePart(I3DEditorHostPanePart* editorHostPanePart)
{
	f3DEditorHostPanePart=editorHostPanePart;

	if( editorHostPanePart )
		editorHostPanePart->QueryInterface(IID_IMFPart, (void**)&fPart);
	else
		fPart = NULL;
}

MCCOMErr BuildingPanePart::InvalidateRender(boolean invalidate, boolean cameraChanged)
{
	f3DEditorHostPanePart->InvalidateRender();

	if(invalidate) 
		fPart->Invalidate();

	return MC_S_OK;
}

enum EDisplayMode
{
	eWireFrame,
	eFlat,
	eGouraud,
	ePlanWireFrame,	// Draw only the plan of the selected level
	ePlanFlat,		// Draw only the plan of the selected level
	ePlanGouraud	// Draw only the plan of the selected level
};

EDisplayMode GetDisplayMode(InteractiveSettings::ERenderMode renderMode,
											  const boolean isPlan )
{
	switch (renderMode)
	{								
	case InteractiveSettings::kRenderMode_BoundingBox:
	case InteractiveSettings::kRenderMode_Wireframe	:
	case InteractiveSettings::kRenderMode_LitWireframe:
		return isPlan?ePlanWireFrame:eWireFrame;
		
	case InteractiveSettings::kRenderMode_Gouraud	:
	case InteractiveSettings::kRenderMode_Phong		:
	case InteractiveSettings::kRenderMode_Textured	:
		return isPlan?ePlanGouraud:eGouraud;
		
	// kRenderMode_Flat/kRenderMode_Sketch/kRenderMode_None/kRenderMode_ShadowBuffer/kRenderMode_Max
	default: 
		return isPlan?ePlanFlat:eFlat;
	}	
}

#if (VERSIONNUMBER >= 0x050000)
MCCOMErr BuildingPanePart::SelfRenderDraw(TRenderable2DArray& renderables2D)
#else
MCCOMErr BuildingPanePart::SelfRenderDraw()
#endif
{
	try
	{
		// Get the renderables and give them to the interactive renderer

		TMCCountedPtr<I3DExRendererBox> interactiveRenderer;
		f3DEditorHostPanePart->GetInteractiveRenderer(&interactiveRenderer);

		const InteractiveSettings::ERenderMode renderingMode= (InteractiveSettings::ERenderMode)interactiveRenderer->GetRenderingMode();
		const EDisplayMode displayMode = GetDisplayMode(renderingMode, UsePlanMesh());

		TRenderableAndTfmArray renderables;

		fBuildingModeler->SaveViewSettings();

		//	Get the facets
		TRenderableAndTfm facetsRenderable;
		if ((displayMode == eFlat) && fBuildingModeler->Get3DFlatFacetRenderable(facetsRenderable))
			renderables.AddElem(facetsRenderable);
		else if ((displayMode == eGouraud ) && fBuildingModeler->Get3DFacetRenderable(facetsRenderable))
			renderables.AddElem(facetsRenderable);
		else if ((displayMode == eWireFrame) && fBuildingModeler->Get3DFacetRenderable(facetsRenderable)) // in wireframe mode too, gives the facet mesh: carrara will convert it into a wireframe mesh
			renderables.AddElem(facetsRenderable);
		if ((displayMode == ePlanFlat) && fBuildingModeler->Get2DFlatFacetRenderable(facetsRenderable))
			renderables.AddElem(facetsRenderable);
		else if ((displayMode == ePlanGouraud) && fBuildingModeler->Get2DFacetRenderable(facetsRenderable))
			renderables.AddElem(facetsRenderable);


		//	Get the edges
		TRenderableAndTfm edgesRenderable;
		const boolean drawPlan = (displayMode==ePlanFlat || displayMode==ePlanGouraud || displayMode==ePlanWireFrame);
		if ( drawPlan && fBuildingModeler->Get2DSegmentRenderable(edgesRenderable) )
			renderables.AddElem(edgesRenderable);
		else if ( !drawPlan && fBuildingModeler->Get3DSegmentRenderable(edgesRenderable) )
			renderables.AddElem(edgesRenderable);

		// Add the grid at the level
		TRenderableAndTfm gridRenderable;
		if(fUnderType==eLevelUpPicked || fUnderType==eLevelDownPicked)
		{
			// Get the altitude
			real32 altitude = (static_cast<Level*>(&*fObjectUnder))->GetDistanceToGround();
			if(fUnderType==eLevelUpPicked)
				altitude+=(static_cast<Level*>(&*fObjectUnder))->GetLevelHeight();
			fBuildingModeler->MakeGridLevelRenderable( gridRenderable,altitude);
			renderables.AddElem(gridRenderable);
		}

		//	Get the handles
		TRenderableAndTfm pointsRenderable;
		if ( drawPlan && fBuildingModeler->Get2DPointRenderable(pointsRenderable) )
			renderables.AddElem(pointsRenderable);
		else if ( !drawPlan && fBuildingModeler->Get3DPointRenderable(pointsRenderable) )
			renderables.AddElem(pointsRenderable);

		//	Bounding box type objects
		//	Some other tools in wireframe

		//  The working box
		IWorkingBox* workingBox = fBuildingModeler->GetWorkingBoxNoAddRef();
		workingBox->AddRenderables(renderables);

		// A light source

		// Give everything to the interactive renderer
		if (renderables.GetElemCount())
	#if (VERSIONNUMBER >= 0x050000)
			interactiveRenderer->DrawInstances(renderables, renderables2D);
	#else
			interactiveRenderer->DrawInstances(renderables);
	#endif

		return MC_S_OK;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPanePart::SelfRenderDraw"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPanePart::SelfRenderDraw"));
	}
	return MC_S_FALSE;
}

void BuildingPanePart::SelfDraw(IMCGraphicContext* 	graphicContext, const TMCRect& inZone)
{
	if (fBuildingModeler)
	{
		// Check if we're in immediate update mode
		if (fBuildingModeler->GetImmediateUpdate()) 
			return;

		// Check if we need the UVs
	//	fBuildingModeler->CheckRenderableUVNeeds();
	}

	InternalSelfDraw(graphicContext, (TMCRect*)&inZone);
}

void BuildingPanePart::DrawQuotation(IMCGraphicContext* graphicContext, TMCRect* inZone)
{
	if(graphicContext)
	{
		TMCRect inRect;
		if(!inZone)
		{
			// Visible rectangle of the part
			fPart->GetClipBoundsInWindow(inRect);
		}
		else
		{
			inRect = *inZone;
		}

		const Quotation& quotation = fBuildingModeler->GetQuotation();

		const int32 qCount = quotation.GetElemCount();

		if(!qCount)
			return;

		// Init the correspondance between sreen space and 2D plan space

		// Screen X and Y axis are not the same than the plan X and Y
		// On the screen, X is from the left to the right, Y from the top to the bottom
		// It's the oppositite in the plan

		// Top left
		TVector3 directionTL;
		TVector3 originTL;
		const TMCPoint TL = inRect.TopLeft();
		
		f3DEditorHostPanePart->PixelRay(TL,originTL, directionTL);

		// Bottom right
		TVector3 directionBR,originBR;
		const TMCPoint BR = inRect.BottomRight();
		f3DEditorHostPanePart->PixelRay(BR,originBR, directionBR);

		const real32 Xmin = originTL.x;
		const real32 Xmax = originBR.x;
		const real32 Ymin = originTL.y;
		const real32 Ymax = originBR.y;
		const real32 Xdist = Xmax-Xmin;
		const real32 Ydist = Ymax-Ymin;
		if(Xdist<=0) return;
		if(Ydist<=0) return;

		const int32 Xpoints = BR.y - TL.y;
		const int32 Ypoints = BR.x - TL.x;

		TMCColorRGBA colorRGBA = fBuildingModeler->GetModelerPrefs()->fTextColor;
		TMCRGBColor color;
		colorRGBA.ToTMCRGBColor(color);
		graphicContext->SetTextColor(color);

		const uint16 fontSize = fBuildingModeler->GetModelerPrefs()->fTextSize;
		TMCTextStyle style;
		style.SetLogicalSize(fontSize);
		graphicContext->SetTextStyle(style);

		const int32 halfW = 40;//40;
		const int32 halfL = 0.5*fontSize;//6;

		for(int32 iQ=0 ; iQ<qCount ; iQ++)
		{
			const Dimension& dimension = quotation[iQ];

			// We could test if it's outside the screen (but use a margin)

			// Find in which point is the center
			TMCPoint center;
			center.y = (int32)(TL.y + Xpoints*((dimension.fPosition.x - Xmin)/Xdist));
			center.x = (int32)(TL.x + Ypoints*((dimension.fPosition.y - Ymin)/Ydist));

			// Build a rectangle arround this point
			if(!dimension.fXOriented)
			{	// Horizontal text
				TMCPoint topLeft = center;
				topLeft.x-=halfW;
				topLeft.y-=halfL;
				TMCPoint bottomRight = center;
				bottomRight.x+=halfW;
				bottomRight.y+=halfL;
				TMCRect textBox(topLeft,bottomRight);
				TMCString15 str;
				str.FromReal32(dimension.fDimension, 2);
				graphicContext->DrawString( textBox, str, kJustCenter, kTMDoNotWrap);
			}
/* Does not work in Release
			*/
			else
			{	// Vertical text
				TMCPoint topLeft = center;
				topLeft.x-=halfL;
				topLeft.y-=halfW;
				TMCPoint bottomRight = center;
				bottomRight.x+=halfL;
				bottomRight.y+=halfW;
				TMCRect textBox(topLeft,bottomRight);
				TMCString15 str;
				str.FromReal32(dimension.fDimension, 2);
				graphicContext->DrawString( textBox, str, kJustCenter, (MCTextMode)(kTMVerticalOrientationDown+kTMDoNotWrap));
			}
		}
	}
}

TMFEventResult BuildingPanePart::SelfMouseDown( const TMCPoint& 		inWhere, 
												const TMCPlatformEvent& inEvent)
{	
	try
	{
		TMFEventResult result;
			
		if (fPart)
			result= fPart->SelfMouseDown(inWhere, inEvent);

		if (result.WasHandled())
		{
			return result;
		}
		else if (fBuildingModeler->MouseDownFromPane(this, inWhere, inEvent))
		{
			return TMFEventResult(true, true);
		}
		else return result;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPanePart::SelfMouseDown"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPanePart::SelfMouseDown"));
	}
			
	return TMFEventResult(true, true);
}

boolean	BuildingPanePart::SelfMouseMoved(const TMCPoint& inWhere, const TMCModifiers& modifier)
{
	fBuildingModeler->MouseMovedFromPane(this, inWhere, modifier);

	return true; // If we don't return true, our cursor management will never be called
}



boolean BuildingPanePart::SelfKeyDown(TMCPlatformEvent* event)
{
	return fBuildingModeler->KeyDownFromPane(this, *event);
}


void BuildingPanePart::SetShown(boolean inShown)
{
	TMCCountedPtr<I3DExRendererBox> interactiveRenderer;
	f3DEditorHostPanePart->GetInteractiveRenderer(&interactiveRenderer);

	if (inShown)
	{	
		if (interactiveRenderer)
			interactiveRenderer->Rehydrate();

		if (f3DEditorHostPanePart)
			f3DEditorHostPanePart->InvalidateRender();
	}
	else
	{	
		if (interactiveRenderer)
			interactiveRenderer->Dehydrate();
	}
}

void BuildingPanePart::BaseWindowBecameVisible(boolean inShown)
{
	BuildingPanePart::SetShown( inShown );
}

MCCOMErr BuildingPanePart::SelfRenderPickDraw(I3DExPickRenderer* pickRenderer)
{
	return MC_E_NOTIMPL;
}

MCCOMErr BuildingPanePart::GetLocationOfSelectionInGlobal(TVector3& outGlobalLocation)
{
	outGlobalLocation=TVector3::kZero;

	// Get the bounding box of the selection
	TBBox3D bbox;
	bbox.Init();

	// Ask the center of the bounding box to the primitive
	fBuildingModeler->GetBuildingNoAddRef()->GetBoundingBox(bbox, false, true); // true: on selection

	bbox.GetCenter(outGlobalLocation);

	return MC_S_OK;
}

void BuildingPanePart::GetCurrentScene(I3DShScene **scene, boolean createIfNone)
{
	if(createIfNone)
	{
		I3DShPrimitive* prim = fBuildingModeler->GetPrimitiveNoAddRef();

		TMCCountedPtr<I3DShObject> object;
		prim->QueryInterface(IID_I3DShObject,(void**)&object);
		ThrowIfNil(object);

		TMCCountedPtr<I3DShObject> clone;
		object->Clone(&clone,kNoAnim);

		// get the shader of the object
#if (VERSIONNUMBER >= 0x060000)
		TMCCountedPtrArray<I3DShMasterShader> masterShaderArray;

		TMCCountedPtrArray<I3DShInstance> instances;
		object->GetInstanceArray(instances);
		const int32 index = 0;
		if(index < (int32)instances.GetElemCount())
		{
			TMCCountedPtr<I3DShMasterShader> masterShader;
			instances[index]->GetShader(&masterShader);
			masterShaderArray.AddElem( masterShader );
			
			// Add the UVSpace shaders
			const int32 spaceCount = instances[index]->GetUVSpaceCount();
			for(int32 iSpace=0 ; iSpace<spaceCount ; iSpace++)
			{
				instances[index]->GetUVSpaceShader(iSpace, &masterShader);
				masterShaderArray.AddElem( masterShader );
			}
		}

		gShell3DUtilities->CreateScene(scene,clone,&masterShaderArray);
#else
		TMCCountedPtr<I3DShMasterShader> masterShader;

		TMCCountedPtrArray<I3DShInstance> instances;
		object->GetInstanceArray(instances);
//		const int32 index = fBuildingModeler->GetInstanceIndex();
		const int32 index = 0;
		if(index < (int32)instances.GetElemCount())
		{
			instances[index]->GetShader(&masterShader);
		}

		gShell3DUtilities->CreateScene(scene,clone,masterShader);
#endif
	}
}

void BuildingPanePart::GetCameraToolsStatus(TCameraToolsStatus& outStatus, boolean& outAllowSavePos)
{
	boolean isCanonical= false;
	if (MCVerify(f3DEditorHostPanePart))
	{
		I3DShCamera* camera= f3DEditorHostPanePart->GetCurrentCameraNoAddRef();
		isCanonical= camera && (camera->GetCanonicalCameraType() != kCanonicalCameraType_NotCanonical);
	}

	outStatus		= (isCanonical) ? k2DToolsEnabled : kAllEnabled;
	outAllowSavePos	= !isCanonical;
}

boolean BuildingPanePart::UsePlanMesh()
{
	if (MCVerify(f3DEditorHostPanePart))
	{
		I3DShCamera* camera= f3DEditorHostPanePart->GetCurrentCameraNoAddRef();
		if( camera )
		{
			switch( camera->GetCanonicalCameraType() )
			{
			case kCanonicalCameraType_Top:
			case kCanonicalCameraType_Bottom:
				return true;
			default:
				return false;
			}
		}
	}

	return false;
}

int32 BuildingPanePart::GetInLevel()
{
	return UsePlanMesh()?fBuildingModeler->Get2DActiveLevel():fBuildingModeler->Get3DActiveLevel();
}

int32 BuildingPanePart::GetPickingFilter()
{

	int32 pickingFilter = ePick2D;

	const boolean planMesh = UsePlanMesh();
	if( planMesh )	pickingFilter = ePick2D;
	else			pickingFilter = ePick3D;

	// Check the interactive renderer: if it's in wireframe mode, don't pick the polygons
	// (we want to be able to pick objects visible behind)
	TMCCountedPtr<I3DExRendererBox> interactiveRenderer;
	f3DEditorHostPanePart->GetInteractiveRenderer(&interactiveRenderer);

	const InteractiveSettings::ERenderMode renderingMode= (InteractiveSettings::ERenderMode)interactiveRenderer->GetRenderingMode();
	switch(renderingMode)
	{
	case InteractiveSettings::kRenderMode_None:
	case InteractiveSettings::kRenderMode_BoundingBox:
	case InteractiveSettings::kRenderMode_Wireframe:
	case InteractiveSettings::kRenderMode_LitWireframe:
		{
			// Do not pick polygons
			pickingFilter&=~ePickWall;
			pickingFilter&=~ePickRoom;
			pickingFilter&=~ePickRoof;
		} break;
	}

	return pickingFilter;
}

void BuildingPanePart::InternalSelfDraw(IMCGraphicContext* graphicContext, TMCRect* inZone)
{
#if (VERSIONNUMBER >= 0x060000)
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	f3DEditorHostPanePart->RenderDraw(graphicContext,inZone, currentDoc);	
#else
	f3DEditorHostPanePart->RenderDraw(graphicContext,inZone);	
#endif

	// Draw Quotation
	if(fBuildingModeler->ShowDimensions()&&UsePlanMesh())
		DrawQuotation(graphicContext,inZone);
}

void BuildingPanePart::SelfMouseEntering(const TMCPoint& inWhere, const TMCModifiers&)
{
	UpdateCursor(inWhere);
}

void BuildingPanePart::SelfMouseStillInside(const TMCPoint& inWhere, const TMCModifiers&)
{
	UpdateCursor(inWhere);
}

void BuildingPanePart::SelfMouseLeaving(const TMCModifiers&)
{
	fCursorID = kUnknownCursor;
	if( ResetUnderObjects() )
	{
		fBuildingModeler->InvalidateMeshesAttributes(false);
		fBuildingModeler->ImmediateUpdate(false,false);
	}
}

void BuildingPanePart::ZoomToSelection()
{
	I3DShCamera* camera = f3DEditorHostPanePart->GetCurrentCameraNoAddRef();

	TMCCountedPtr<I3DShTreeElement> cameraTreeElement;
	camera->QueryInterface(IID_I3DShTreeElement, (void**)&cameraTreeElement);
	TTreeTransform matCam, matCamInverse;
	cameraTreeElement->GetGlobalTreeTransform(matCam);
#if (VERSIONNUMBER >= 0x060000)
	TTransform3D inverseTrans;
	matCam.GetInverseTransform3D(inverseTrans);
	matCamInverse.SetFromTransform3D(inverseTrans);
#else
	matCamInverse = matCam.InverseWithoutXYZScaling();
#endif
	TMCRealRect scrZone;
	scrZone.SetInvalid();

	// Get the bounding box of the selection
	TBBox3D bbox, camBBox;
	fBuildingModeler->GetBuildingNoAddRef()->GetBoundingBox(bbox, false, true);

	camBBox = matCamInverse.TransformBBox(bbox);

	// Project the point and create a rect that countain them all
	for (int idx = 0; idx < 8; idx++)
	{
		TVector3 camPt = camBBox[idx];
		TVector2 uvPos;
		real32 dist;
		camera->Project3DTo2D(&camPt, &uvPos, &dist);
		scrZone.UnionPt(uvPos);
	}

	f3DEditorHostPanePart->ZoomToRect(scrZone);
}

void BuildingPanePart::SelfPrepareMenus()
{
	gMenuUtilities->EnableMenuAction(kaZoomToSel, true);
}

boolean BuildingPanePart::SelfMenuAction(ActionNumber actionNumber)
{
	switch(actionNumber)
	{
		case kaZoomToSel:
		{
			ZoomToSelection();
			return true; // Action was handled
		}
	}

	return false;// Action not handled: return false
}

void BuildingPanePart::SelfPrepareToDestroy()
{
	fPart = NULL;
	f3DEditorHostPanePart = NULL;
	fBuildingModeler = NULL;
	fObjectUnder = NULL;

	TBasicPart::SelfPrepareToDestroy();
}

ECanonicalCameraType BuildingPanePart::GetCameraType()
{
	I3DShCamera* camera=f3DEditorHostPanePart->GetCurrentCameraNoAddRef();
	return camera->GetCanonicalCameraType();
}

boolean BuildingPanePart::ResetUnderObjects()
{
	if(fObjectUnder)
	{
		fObjectUnder->ClearFlag(eIsTargeted);
		fObjectUnder = NULL;
		fUnderType = eNothingPicked;
		return true;
	}
	return false;
}

MFCursorID BuildingPanePart::GetCursorID(const TMCPoint& inWhere)
{
	int32 currentTool=0;
	gMenuUtilities->GetCurrentGlobalTool(currentTool); 

	MFCursorID cursorID = kCursor_Default;

	switch (currentTool) 
	{
		case kMoveToolID:
		case kRotateToolID:
		case kScaleToolID:
		case kDeleteTool:
			{
				//	do some picking to know what's under
				Picking pick(fBuildingModeler,this,inWhere, GetPickingFilter());

				if( fObjectUnder!=pick.PickedObject() )
				{
					ResetUnderObjects();
					fObjectUnder = pick.PickedObject();
					if(fObjectUnder)
						fObjectUnder->SetFlag(eIsTargeted);
					fBuildingModeler->InvalidateMeshesAttributes(false);
					fBuildingModeler->ImmediateUpdate(false,false);
				}

				if(currentTool==kRotateToolID) cursorID = kCursor_Rotate;
				else if(currentTool==kDeleteTool) cursorID = kDeleteCursor;
			} break;
		case k2DZoomToolID:	
			{
				if(gActionManager->IsOptionDown()) cursorID = kCursor_DeMagnify;
				else cursorID = kCursor_Magnify;
			} break;
		case k2DPanToolID:	cursorID = kCursor_OpenHand; break;

		case kBuildWallTool:
		case kBuildWallWithCrenel1Tool:
		case kBuildWallWithCrenel2Tool:
		case kBuildWallWithCrenel3Tool:
		case kBuildWallWithCrenel4Tool:
		case kBuildWallWithCrenel5Tool:
			{
				//	do some picking to know what's under
				Picking pick(fBuildingModeler,this,inWhere, ePickWall+ePick2D);

				if( fObjectUnder!=pick.PickedObject() )					
				{
					ResetUnderObjects();
					if( pick.GetPickedType() == eWallPicked || 
						pick.GetPickedType() == ePointPicked ) // highlight only the walls and the points
					{
						fObjectUnder = pick.PickedObject();
						if(fObjectUnder)
							fObjectUnder->SetFlag(eIsTargeted);
					}
			
					fBuildingModeler->InvalidateMeshesAttributes(false);
					fBuildingModeler->ImmediateUpdate(false,false);
				}
				if(fObjectUnder)
					cursorID = kAddCursor;
				else
					cursorID = kCantAddCursor;
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
			{
				//	do some picking to know what's under
				Picking pick(fBuildingModeler,this,inWhere, GetPickingFilter());

				if( fObjectUnder!=pick.PickedObject() )					
				{
					ResetUnderObjects();
					if(pick.GetPickedType() == eWallPicked ) // highlight only the walls
					{
						fObjectUnder = pick.PickedObject();
						if(fObjectUnder)
							fObjectUnder->SetFlag(eIsTargeted);
					}
					fBuildingModeler->InvalidateMeshesAttributes(false);
					fBuildingModeler->ImmediateUpdate(false,false);
				}
				cursorID = kMoveCursor;
			} break;

		case kInsertSquareStairwayTool:
		case kInsertLargeStairwayTool:
		case kInsertWideStairwayTool:
		case kInsertCircleStairwayTool:
			{
				//	do some picking to know what's under
				Picking pick(fBuildingModeler,this,inWhere, GetPickingFilter());

				if( fObjectUnder!=pick.PickedObject() )					
				{
					ResetUnderObjects();
					if( pick.GetPickedType() == eRoomFloorPicked ||
						pick.GetPickedType() == eRoomCeilingPicked) // highlight only the walls
					{
						fObjectUnder = pick.PickedObject();
						if(fObjectUnder)
							fObjectUnder->SetFlag(eIsTargeted);
					}
					fBuildingModeler->InvalidateMeshesAttributes(false);
					fBuildingModeler->ImmediateUpdate(false,false);
				}
				cursorID = kMoveCursor;
			} break;

		case kInsertShellLevelTool:
		case kInsertDuplicateLevelTool:
		case kInsertEmptyLevelTool:
			{
				//Pick the level
				Picking pick(fBuildingModeler,this,inWhere, ePickLevel);
				if( fObjectUnder!=pick.PickedObject() ||
					fUnderType != pick.GetPickedType() )					
				{
					ResetUnderObjects();
					fObjectUnder = pick.PickedObject();
					fUnderType = pick.GetPickedType(); // to know if it's the upper or the lower part of the level
				
					fBuildingModeler->InvalidateMeshesAttributes(false);
					fBuildingModeler->ImmediateUpdate(false,false);
				}

				cursorID = kSelectCursor;
			} break;
	
		default:
			cursorID = kUnknownCursor; break;// so the appli will take care of it
	}

	return cursorID;
}

void BuildingPanePart::UpdateCursor(const TMCPoint& inWhere)
{
	const MFCursorID newCursorID = GetCursorID( inWhere );

	if(newCursorID != fCursorID)
	{
		fCursorID = newCursorID;

		TMCCountedPtr<IMFPart>	part;
		f3DEditorHostPanePart->QueryInterface(IID_IMFPart, (void**) &part); if (part==(IMFPart*)NULL) return;
	
		if (newCursorID > 1500)
		{	// One of our custom cursors
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
			part->SetCursor(newCursorID);
		}
		else
		{	// One of the Carrara cursors
			part->SetCursor(newCursorID);
		}
	}
}

void BuildingPanePart::GetDropArea(	const TMCPoint&		inWhere,
								IMFDropCandidate&	inCandidate,
								IDType&				outAcceptedFlavor,
								IMFDropArea**		outDropArea )
{
	gDragAndDropUtilities->CreateDropArea('BMDA', fPart, outDropArea);
	ThrowIfNil(*outDropArea);

	TMCCountedPtr<BuildingDropArea> drop;

	(*outDropArea)->QueryInterface(CLSID_BuildingDropArea, (void**)&drop);
	ThrowIfNil(drop);

	drop->SetModeler(fBuildingModeler);
	drop->SetPaneExt(this);

	//
	UpdateCursor(inWhere);
}

#endif // !NETWORK_RENDERING_VERSION


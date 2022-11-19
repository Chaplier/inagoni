/****************************************************************************************************

		MBuildingPanePart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/23/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingPanePart__
#define __MBuildingPanePart__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "IMFPart.h"
#include "I3DShScene.h"
#include "I3DEditorHostPart.h"
#include "IEx3DEditorHostPart.h"
#include "BasicMCFCOMImplementations.h"

class BuildingModeler;
#include "BuildingModeler.h"
#include "ArchiTools.h"
//class Wall;
//class Room;
//class Point;
class CommonBase;

extern const MCGUID CLSID_BuildingPanePart;

const MFCursorID kUnknownCursor=0;

class BuildingPanePart :	public TBasicPart,
							public IEx3DEditorHostPanePart
{
public:

	BuildingPanePart();
	virtual ~BuildingPanePart();

	STANDARD_RELEASE;

	// IMCUnknown
	MCErr				MCCOMAPI QueryInterface	( const MCIID& riid, void** ppvObj );
	uint32				MCCOMAPI AddRef() {return TBasicPart::AddRef();}

	// IMFExPart
	virtual TMFEventResult MCCOMAPI SelfMouseDown		 (const TMCPoint& inWhere, const TMCPlatformEvent& inEvent);
	virtual void		MCCOMAPI SelfDraw			(IMCGraphicContext* graphicContext, const TMCRect& inZone);
	virtual boolean		MCCOMAPI SelfKeyDown		(TMCPlatformEvent* event);

	virtual boolean		MCCOMAPI SelfMouseMoved		(const TMCPoint& inWhere, const TMCModifiers& inModifiers);
	virtual void		MCCOMAPI SelfMouseEntering	(const TMCPoint& inWhere, const TMCModifiers& inModifiers);
	virtual void		MCCOMAPI SelfMouseStillInside(const TMCPoint& inWhere, const TMCModifiers& inModifiers);
	virtual void		MCCOMAPI SelfMouseLeaving	(const TMCModifiers& inModifiers);
	virtual void		MCCOMAPI SelfPrepareMenus	();
	virtual	boolean		MCCOMAPI SelfMenuAction		(ActionNumber actionNumber);
	virtual void		MCCOMAPI SelfPrepareToDestroy();
	boolean				MCCOMAPI GetIndirectToolAction(
							IShMouseAction**	outAction,
							ActionNumber&		outActionNumber,
							IMFResponder**		outContext,
							IMFPart**			outPart,
							const TMCPoint&		inWhere,
							int32 				inToolID,
							IMFToolbarPart*		inToolbarPart) {return false;}
	virtual boolean		MCCOMAPI Receive(int32 message, IMFResponder* source, void* data){return false;}

	virtual void		MCCOMAPI SetShown				(boolean inShown);
	virtual void		MCCOMAPI BaseWindowBecameVisible(boolean inShown);
	virtual MFCursorID	MCCOMAPI GetCursorID			(const TMCPoint& inWhere) const {return fCursorID;}

	virtual void		MCCOMAPI GetDropArea			( const TMCPoint&		inWhere,
														 IMFDropCandidate&	inCandidate,
														 IDType&				outAcceptedFlavor,
														 IMFDropArea**		outDropArea);

	//	IEx3DEditorHostPanePart
	virtual void		MCCOMAPI Set3DEditorHostPanePart(I3DEditorHostPanePart* editorHostPanePart);
	virtual MCCOMErr	MCCOMAPI InvalidateRender( boolean invalidate = true, boolean cameraChanged=false );
#if (VERSIONNUMBER >= 0x050000)
	virtual MCCOMErr	MCCOMAPI SelfRenderDraw	(TRenderable2DArray& renderables2D);
#else
	virtual MCCOMErr	MCCOMAPI SelfRenderDraw();
#endif
	virtual MCCOMErr	MCCOMAPI SelfRenderPickDraw(I3DExPickRenderer* pickRenderer);
	virtual MCCOMErr	MCCOMAPI GetLocationOfSelectionInGlobal(TVector3& outGlobalLocation);
	virtual boolean		MCCOMAPI GetHitLocationInGlobal(const TMCPoint& inWindowPoint, TVector3& outGlobalLocation) {return false;}
	virtual void	    MCCOMAPI GetCurrentScene(I3DShScene **scene, boolean createIfNone);
	virtual void		MCCOMAPI GetCameraToolsStatus	(TCameraToolsStatus& outStatus, boolean& outAllowSavePos);		
#if (VERSIONNUMBER >= 0x050000)
	virtual I3DEditorHostPanePart* MCCOMAPI  Get3DEditorHostPanePart() const { return f3DEditorHostPanePart; }
#else
	virtual I3DEditorHostPanePart* MCCOMAPI  Get3DEditorHostPanePart() { return f3DEditorHostPanePart; }
#endif
#if (VERSIONNUMBER >= 0x060000)
	virtual void		MCCOMAPI BeginAreaRender(){}
	virtual void		MCCOMAPI EndAreaRender(){}
#endif
#if (VERSIONNUMBER >= 0x070000)
	virtual void		MCCOMAPI	CameraMoved() {}
#endif

	IMFPart*	GetThisPartNoAddRef() {return fPart;}
	void		SetModeler( BuildingModeler* modeler);

	void		InternalSelfDraw(IMCGraphicContext* graphicContext, TMCRect* inZone);
		
	ECanonicalCameraType GetCameraType();

	boolean		UsePlanMesh();
	int32		GetPickingFilter();
	int32		GetInLevel();

protected:

	void		ZoomToSelection();
	void		DrawQuotation(IMCGraphicContext* graphicContext, TMCRect* inZone);

	// Cursor managment
	MFCursorID	GetCursorID(const TMCPoint& inWhere);
	void		UpdateCursor(const TMCPoint& inWhere);
	boolean		ResetUnderObjects();
private:

	TMCCountedPtr<IMFPart>					fPart;
	TMCCountedPtr<I3DEditorHostPanePart>	f3DEditorHostPanePart;
	TMCCountedPtr<BuildingModeler>			fBuildingModeler;

	MFCursorID	fCursorID;
	TMCCountedPtr<CommonBase>	fObjectUnder;
	EPickedType					fUnderType;
};

#endif

#endif // !NETWORK_RENDERING_VERSION

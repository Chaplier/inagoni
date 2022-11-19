/****************************************************************************************************

		MPositionActions.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/4/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MPositionActions__
#define __MPositionActions__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MBuildingAction.h"
#include "MPicking.h"

class BuildingPrim;
class BuildingModeler;
class BuildingPanePart;
struct IShAction;

//////////////////////////////////////////////////////////////////////
//
class PositionRecorder
{
public:
	PositionRecorder();

	void SavePosition( BuildingPrim* primitive );
	void SwapPosition( BuildingPrim* primitive );

protected:

	TMCClassArray<TVector2> fRecordedPointPos;
	// Some points have a object attached to them => record their position too
	TMCClassArray<TVector2> fRecordedAttachedObjPos;
};

//////////////////////////////////////////////////////////////////////

class MoveMouseAction : public ModelerMouseAction,
						public PositionRecorder//,
					//	public BuildingRecorder // Some points, walls can be merged at the end of the move
{
protected:

	MoveMouseAction(	BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	boolean fIsWallPoint;
	TVector3 fPlaneNormal;
	TVector3 fHitPos;
	TMCCountedPtr<CommonPoint>	fReferencePoint;
	TMCClassArray<Line2D>		fRefDirections;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
struct WallCurve
{
	WallCurve(){}
	WallCurve(const int32 inLevel, const int32 wallIndex, real32 offset, int32 segmentCount) 
		: fInLevel(inLevel), fOnWallIndex(wallIndex), fOffset(offset), fSegmentCount(segmentCount) {}

// Can't record a pointer	TMCCountedPtr<Wall> fOnWall;
	// This way of recording a wall is not very safe: depend on the order in the database
	int32 fOnWallIndex;
	int32 fInLevel;
	real32 fOffset;
	int32 fSegmentCount;
};

class WallCurveRecorder
{
public:
	WallCurveRecorder();

	void SaveWallCurve(BuildingPrim* primitive);
	void SwapWallCurve(BuildingPrim* primitive);
	void ResetWallCurve(BuildingPrim* primitive);

protected:
	TMCClassArray<WallCurve> fRecordedWallCurves;
};

//////////////////////////////////////////////////////////////////////

class MoveWallHandleMouseAction :	public ModelerMouseAction,
									public WallCurveRecorder
{
protected:

	MoveWallHandleMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	TVector3 fHitPos;
	boolean fHasMoved;

	TVector2 fFlatNormal;
	TVector2 fFlatMiddle;
};

//////////////////////////////////////////////////////////////////////

class MoveProfileMouseAction :	public ModelerMouseAction,
								public PositionRecorder
{
protected:

	MoveProfileMouseAction(	BuildingModeler*	modeler,
							BuildingPanePart*	pane,
							const Picking&		picked,
							const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	TVector3 fHitPos;
	TVector3 fO;
	TVector3 fI;
	TVector3 fJ;
};

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// If point is NULL, move the selection
class MoveAction :	public ModelerAction,
					public PositionRecorder
{
protected:

	MoveAction(	BuildingModeler*	modeler,
				const TVector2&		offset,
				CommonPoint*		point);		

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		offset,
						CommonPoint*		point);

	// TBasicMouseAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TVector2					fOffset;
	CommonPoint*				fPoint;
};

//////////////////////////////////////////////////////////////////////

class ScaleMouseAction : public ModelerMouseAction,
						public PositionRecorder//,
					//	public BuildingRecorder // Some points, walls can be merged at the end of the move
{
protected:

	ScaleMouseAction(	BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	TVector3 fPlaneNormal;
	TVector3 fHitPos;
	TVector2 fCenter; // scaling center
	TVector2 fFirstDiff;
	real32	fPrecision; // Used for the constraints
};

//////////////////////////////////////////////////////////////////////

class ScaleProfileMouseAction : public ModelerMouseAction,
								public PositionRecorder
{
protected:

	ScaleProfileMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	TVector3 fHitPos;
	TVector2 fCenter;
	TVector3 fO;
	TVector3 fI;
	TVector3 fJ;
	TVector2 fFirstDiff;
	real32	fPrecision; // Used for the constraints
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class RotateProfileMouseAction : public ModelerMouseAction,
								public PositionRecorder
{
protected:

	RotateProfileMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	TVector3 fHitPos;
	TVector2 fFirstPos;
	TVector3 fO;
	TVector3 fI;
	TVector3 fJ;
};

//////////////////////////////////////////////////////////////////////

class RotateMouseAction :	public ModelerMouseAction,
							public PositionRecorder//,
					//	public BuildingRecorder // Some points, walls can be merged at the end of the move
{
protected:

	RotateMouseAction(	BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	real32	fConstraintAngle;
	TVector3 fPlaneNormal;
	TVector3 fHitPos;
	TVector2 fCenter; // rotation center
};

//////////////////////////////////////////////////////////////////////
class FlipAction :	public ModelerAction,
					public PositionRecorder
{
protected:

	FlipAction(	BuildingModeler*	modeler );		

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler );

	// TBasicMouseAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();

protected:
};

//////////////////////////////////////////////////////////////////////
//
class ObjectPosRecorder
{
public:
	ObjectPosRecorder( const boolean recordCenter );

	void SavePosition(BuildingPrim* primitive);
	void SwapPosition(BuildingPrim* primitive);
	void ResetPosition(BuildingPrim* primitive);

protected:
	boolean fRecordCenter;	// We can record only the center of the polyline, or
							// all its points
	TMCClassArray<WallObjPos> fRecordedWallObjPos;
	TMCClassArray<RoomObjPos> fRecordedRoomObjPos;
};

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

class MoveObjectMouseAction :	public ModelerMouseAction,
								public ObjectPosRecorder
{
protected:

	MoveObjectMouseAction(	BuildingModeler*	modeler,
							BuildingPanePart*	pane,
							const Picking&		picked,
							const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	boolean fWallObjPicked;
	TVector3 fHitPos;

	// A base attached to the picked object
	TVector3 fObjO;
	TVector3 fObjI;
	TVector3 fObjJ;
	TVector3 fObjK;
};

//////////////////////////////////////////////////////////////////////

class MoveObjectAction :	public ModelerAction,
							public ObjectPosRecorder
{
protected:

	MoveObjectAction(	BuildingModeler*	modeler,
						const TVector2&		offset );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		offset );


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TVector2					fOffset;
};


//////////////////////////////////////////////////////////////////////

class ScaleObjectMouseAction :	public ModelerMouseAction,
								public ObjectPosRecorder
{
protected:

	ScaleObjectMouseAction(	BuildingModeler*	modeler,
							BuildingPanePart*	pane,
							const Picking&		picked,
							const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	boolean fWallObjPicked;
	TVector3 fHitPos;
	TVector2 fCenter; // Center of the selected object
	TVector2 fFirstDiff;

	// A base attached to the picked object
	TVector3 fObjO;
	TVector3 fObjI;
	TVector3 fObjJ;
	TVector3 fObjK;
};

//////////////////////////////////////////////////////////////////////

class ScaleObjectAction :	public ModelerAction,
							public ObjectPosRecorder
{
protected:

	ScaleObjectAction(	BuildingModeler*	modeler,
						const TVector2&		scale );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		scale );


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TVector2					fScale;
};


//////////////////////////////////////////////////////////////////////

class RotateObjectMouseAction :	public ModelerMouseAction,
								public ObjectPosRecorder
{
protected:

	RotateObjectMouseAction(BuildingModeler*	modeler,
							BuildingPanePart*	pane,
							const Picking&		picked,
							const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	Picking fPicked;
	boolean fHasMoved;
	real32 fConstraintAngle;
	TVector3 fHitPos;
	TVector2 fCenter; // Center of the selected object
};

//////////////////////////////////////////////////////////////////////

class HolePointMouseAction :	public ModelerMouseAction,
									public ObjectPosRecorder
{
protected:

	HolePointMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	// TBasicMouseAction methods
	void		MCCOMAPI Track(IMCGraphicContext* gc, int16 stage, TMCPoint& first, TMCPoint &prev, TMCPoint &cur,boolean moved, IShMouseAction**	nextAction);
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	boolean		MCCOMAPI CanUndo()	{ return fHasMoved; }

	// Redefine this in the derived classes
	virtual void TrackMove( const TVector3& newIntersection,
							const boolean option,
							const boolean shift,
							const boolean command ) = 0;
	// Add some extra preparation
	virtual void PrepareMove(OutlinePoint* point, SubObject* object){};

	MCCOMErr	MCCOMAPI GetName(TMCString& name)=0;

protected:
	Picking fPicked;
	boolean fHasMoved;
	TVector3 f3DHitPos;
	TVector2 f2DHitPos;
	TVector3 fO;
	TVector3 fI;
	TVector3 fJ;
	TVector3 fK;
	// For GetName
	int16 fStringID;
};

// Move hole points (windows, doors and stairways)
class MoveHolePointMouseAction :	public HolePointMouseAction
{
protected:

	MoveHolePointMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
			: HolePointMouseAction(modeler,pane,picked,mousePos){fStringID = 3;}

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	virtual void TrackMove( const TVector3& newIntersection,
							const boolean option,
							const boolean shift,
							const boolean command );

	MCCOMErr	MCCOMAPI GetName(TMCString& name);
};

// Scale hole points (windows, doors and stairways)
class ScaleHolePointMouseAction :	public HolePointMouseAction
{
protected:

	ScaleHolePointMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
			: HolePointMouseAction(modeler,pane,picked,mousePos){fStringID = 4;}

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	virtual void TrackMove( const TVector3& newIntersection,
							const boolean option,
							const boolean shift,
							const boolean command );

	virtual void PrepareMove(OutlinePoint* point, SubObject* object);

	MCCOMErr	MCCOMAPI GetName(TMCString& name);
protected:
	TVector2 fCenter; // center of the selection
	TVector2 fFirstDiff;
};

// Rotate hole point
class RotateHolePointMouseAction :	public HolePointMouseAction
{
protected:

	RotateHolePointMouseAction(	BuildingModeler*	modeler,
								BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
			: HolePointMouseAction(modeler,pane,picked,mousePos){fStringID = 5;}

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShMouseAction**	outAction,
						BuildingModeler*	modeler,
						BuildingPanePart*	pane,
						const Picking&		picked,
						const TMCPoint&		mousePos );

	virtual void TrackMove( const TVector3& newIntersection,
							const boolean option,
							const boolean shift,
							const boolean command );
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

	virtual void PrepareMove(OutlinePoint* point, SubObject* object);

protected:
	TVector2 fFirstDir;
	TVector2 fCenter; // rotation center
	real32 fConstraintAngle;
};


//////////////////////////////////////////////////////////////////////

class ScaleAction :	public ModelerAction,
					public PositionRecorder
{
protected:

	ScaleAction(	BuildingModeler*	modeler,
					const TVector2&		scale );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		scale );


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TVector2					fScale;
};

//////////////////////////////////////////////////////////////////////

class RotateAction :	public ModelerAction,
						public PositionRecorder
{
protected:

	RotateAction(	BuildingModeler*	modeler,
					const TVector2&		cosSin );

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		cosSin );


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	TVector2					fCosSin;
};

//////////////////////////////////////////////////////////////////////

class LevelHeightAction :	public ModelerAction
{
protected:

	LevelHeightAction(	BuildingModeler*	modeler,
						const TMCArray<int32>& level, 
						const real32 height);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const TMCArray<int32>& level, 
						const real32 height);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	void SetHeight();

	real32				fHeight;
	TMCArray<int32>		fLevels;
	TMCArray<int32>		fPrevHeight; // Undo data
};

class DimensionAction :	public ModelerAction,
						public BuildingRecorder // Some settings can modify the pos of the objects
{
protected:

	DimensionAction(BuildingModeler*		modeler,
					const real32			dimension,
					const EDimensionType	type);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const real32		dimension,
						const EDimensionType	type);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:
	real32						fDimension;
	EDimensionType				fDimensionType;
	TMCArray<real32>			fPreviousDimension; // Undo data (not used anymore)
};

// To change an enum value or an uint32/int32 data 
class UInt32Action :	public ModelerAction,
						public BuildingRecorder // Some settings can modify the pos of the objects
{
protected:

	UInt32Action(BuildingModeler*		modeler,
				const uint32			newValue,
				const EDimensionType	type);

public:

	// IMCUnknown methods
 	STANDARD_RELEASE;

	static void Create(	IShAction**			outAction,
						BuildingModeler*	modeler,
						const uint32		newValue,
						const EDimensionType	type);


	// TBasicAction methods
	MCErr		MCCOMAPI Do();
	MCErr		MCCOMAPI Undo();
	MCErr		MCCOMAPI Redo();
	MCCOMErr	MCCOMAPI GetName(TMCString& name);

protected:

	uint32						fNewvalue;
	EDimensionType				fDimensionType;
};


#endif

#endif // !NETWORK_RENDERING_VERSION


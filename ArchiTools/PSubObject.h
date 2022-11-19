/****************************************************************************************************

		PSubObject.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/31/2004

****************************************************************************************************/

#ifndef __PSubObject__
#define __PSubObject__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"
#include "PCommonBase.h"
#include "PWall.h"
#include "Utils.h"
#include "I3DShInstance.h"
#include "POutlinePoint.h"

class Wall;
struct I3DShObject;
class TTreeTransform;
struct IShTokenStream;
class BuildingPrim;

static const int32 kNoChild = -1;

void CopyPointArray(const TMCCountedPtrArray<OutlinePoint>& fromArray, 
					TMCCountedPtrArray<OutlinePoint>& toArray);

struct WallObjPos
{
	WallObjPos(){}
	WallObjPos(const int32 inLevel, const int32 wallIndex, TVector2& center);
	WallObjPos(const int32 inLevel, const int32 wallIndex, const TMCCountedPtrArray<OutlinePoint>& pos);

// Can't record a pointer	TMCCountedPtr<Wall> fOnWall;
	// This way of recording a wall is not very safe: depend on the order in the database
	int32 fOnWallIndex;
	int32 fInLevel;
	TMCCountedPtrArray<OutlinePoint> fPositions;
};

struct RoomObjPos
{
	RoomObjPos(){}
	RoomObjPos(const int32 inLevel, const int32 roomIndex, TVector2& center);
	RoomObjPos(const int32 inLevel, const int32 roomIndex, const TMCCountedPtrArray<OutlinePoint>& pos);
// Can't record a pointer	TMCCountedPtr<Room> fInRoom;
	// This way of recording a room is not very safe: depend on the order in the database
	int32 fInRoomIndex;
	int32 fInLevel;
	TMCCountedPtrArray<OutlinePoint> fPositions;
};

// the 16 last bits are for the common flags
enum EObjFlags
{
	eObjPosAreValid		= 0x00010000,
	eObjCenterIsValid	= 0x00020000, // polyline center valid
	eObjTransformIsValid= 0x00040000,
	eAutoFlipObj		= 0x00080000,
	eHoleBBoxIsValid	= 0x00100000,
	eSelBBoxIsValid		= 0x00200000, // selection bbox valid
};

enum ECollisionType
{
	eNoCollision,
	ePointHereInOtherObject,
	ePointOtherInHereObject,
	eSideIntersection,
	eUnknownCollision
};

class SubObject : public CommonBase
{
public:
//	virtual void	DeleteObj(){fHoleOutline.ArrayFree();}
	void			DeleteSelectedPoints();

	void			Copy(SubObject* copyFrom, Level* level, const ECloneChildrenMode cloneMode);

	virtual MCCOMErr	Write(IShTokenStream* stream);
	MCCOMErr		Read(IShTokenStream* stream, const int32 token, BuildingPrim* building); 

	void									SetOutline(const TMCCountedPtrArray<OutlinePoint>&);
	const TMCCountedPtrArray<OutlinePoint>&	GetOutline() const { return fHoleOutline; }
	int32									GetPosCount() const { return fHoleOutline.GetElemCount(); }

	// Build a box the size of the hole (return 8 pos)
	// 0,1,2,3 are at the bottom, 4,5,6,7 at the top
	// 0->1 : length, 1->2 : width, 2->3 : length, 3->0 : width
	void			GetBox( TMCClassArray<TVector3>& pos );
	void			Get3DOutlines(TMCClassArray<TVector3>& side0,
								  TMCClassArray<TVector3>& side1,
								  TMCClassArray<TVector3>& normals);
	void			Get2DNormals(TMCClassArray<TVector2>& normals);
	void			GetCenter( TVector3& pos );
	inline TVector2& GetPolylineCenter(){ValidateCenter();return fPolylineCenter;}
//	inline real32	Get2DWidth() const {return RealAbs(fHoleOutline[2].x-fHoleOutline[0].x);}
//	inline real32	Get2DHeight() const {return RealAbs(fHoleOutline[2].y-fHoleOutline[0].y);}
	inline real32	Get2DWidth() {return RealAbs(GetHoleBBox().GetWidth());}
	inline real32	Get2DHeight() {return RealAbs(GetHoleBBox().GetHeight());}
	virtual real32	GetThickness() const {return 0;}
	inline const TVector3&	GetOffset()const {return fOffset;}
	inline TVector3&	Offset() {return fOffset;}
	inline void		SetOffset(const TVector3& vec) {fOffset=vec;}
	inline const TVector3&	GetScale()const {return fScale;}
	inline TVector3&	Scale() {return fScale;}
	inline void		SetScale(const TVector3& vec) {fScale=vec;}
	inline const TVector3&	GetRotate()const {return fRotate;}
	inline TVector3&	Rotate() {return fRotate;}
	inline void		SetRotate(const TVector3& vec) {fRotate=vec;}
	inline EPlacementType	GetPlacement()const {return fPlacement;}
	inline void		SetPlacement(const EPlacementType p) {fPlacement=p;Invalidate();}
	boolean			SetPolylineCenter( const TVector2& center, const boolean checkFirst );
	// On the selected points
	virtual boolean OffsetPolylinePoints(const TVector2& offset);
	virtual boolean ScalePolylinePoints(const TVector2& scale, const boolean arroundObjCenter);
	virtual boolean RotatePolylinePoints(const TVector2& cosSin, const boolean arroundObjCenter);
	virtual boolean SmoothPolylinePoints(const boolean smooth);
	// On the whole polyline
	virtual boolean	OffsetPolyline(const TVector2& offset, const boolean checkFirst);
	virtual boolean	ScalePolyline(const TVector2& scale, const boolean checkFirst);
	virtual boolean	RotatePolyline(const TVector2& cosSin, const boolean checkFirst);
	virtual boolean	SetCenter( const TVector3& center, const boolean checkFirst )=0;
	const TBBox2D&	GetHoleBBox() { ValidateBBox(); return fHoleBBox; }
	const TBBox2D&	GetSelectionBBox() { ValidateSelectionBBox(); return fSelBBox; }
	void			Invalidate()
	{
		ClearFlag(eObjPosAreValid);
		ClearFlag(eObjCenterIsValid);
		ClearFlag(eObjTransformIsValid);
		ClearFlag(eHoleBBoxIsValid);
		ClearFlag(eSelBBoxIsValid);
	}
	virtual void	InvalidateTessellation()=0;
	void			InitMarqueeSelection(const boolean onPoints);
	boolean			SetMarqueeOnPoints(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode);
	boolean			SetMarqueeOnObject(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode);
	void			SetSelection(const boolean select){ select?SetFlag(eIsSelected):ClearFlag(eIsSelected);}
	void			InvertSelection();
	// Return true if at least one of the points is selected
	boolean			HasSelectedHolePoint();

	// Outline flags
	void			SetOutlineFlag(const int32 flag);
	void			ClearOutlineFlag(const int32 flag);

	void			MinCorner(TVector2& corner);
	void			MaxCorner(TVector2& corner);
	void			AutoFlip(const boolean autoFlip){autoFlip?SetFlag(eAutoFlipObj):ClearFlag(eAutoFlipObj);}
	boolean			AutoFlip()const{return Flag(eAutoFlipObj);}

	virtual void	GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K)
	{O=TVector3::kZero;I=TVector3::kUnitX;J=TVector3::kUnitY;K=TVector3::kUnitZ;}

	ECollisionType	Collide(SubObject* otherObject,TVector2& problem);
	boolean			PointIn(const TVector2& point);

	// Instance/Object access
	I3DShInstance*	SetSceneObject(I3DShObject* objectPrimitive);
	// need to give the primitive when the object is not attached to it yet
	I3DShInstance*	GetChildNoAddRef(BuildingPrim* inBuildingPrimitive=NULL); 
	const TTreeIdPath& GetChildTreePath() const { return fChildTreePath; }
//	int32			GetChildIndex() const { return fChildIndex; }
//	void			SetChildIndex(const int32 index) { fChildIndex = index; }
//	const TMCString& GetRescueName() const {return fRescueObject;}
//	void			SetRescueName(const TMCString& name){fRescueObject = name;}

	void			SetInstanceTransform();
	void			UpdateInstanceTransform();

	// Split successive polyline points that are selected
	void			Split();

protected:
	SubObject();
	virtual ~SubObject();

	virtual void	Validate()=0;
	virtual void	ValidateCenter();
	void			ValidateBBox();
	void			ValidateSelectionBBox();
	virtual void	ComputeTreeTransform(TTreeTransform& treeTransform, 
											const TBBox3D& originBBox)=0;
	void			Compute3DOutlines(const TVector3& thicknesss); // compute the fSide0, fSide1 and fNormalsArrays

	void	GetInstances(	TMCCountedPtrArray<I3DShInstance>& instances );

	BuildingPrim* GetParentBuilding() { return fBuilding; }

	// The SubObject will use the object it's on to get its orientation
	// and position in space

	// Hole description
//	TMCClassArray<TVector2>	fHoleOutline;
	TMCCountedPtrArray<OutlinePoint>	fHoleOutline;

	// Cached dat
	TVector2 fPolylineCenter; // center of all the polyline points
	TVector3 fOrigin;
	TVector3 fWidth;
	TVector3 fLength;
	TVector3 fHeight;
	TVector3 fCenter;
	TMCClassArray<TVector3> fSide0;
	TMCClassArray<TVector3> fSide1;
	TMCClassArray<TVector3> fNormals;

	// Instance in the scene
	TTreeIdPath				fChildTreePath;
//	int32					fChildIndex; // a child is reconized through it's index in the children list
//	TMCDynamicString		fRescueObject; // Undo data
	EPlacementType			fPlacement;
	// Custom placement of the external instance position in the hole.
	// These are relative to the original pos in the bbox.
	TVector3				fOffset;	// Wall: X=Hor, Y=Ver, Z=Norm
	TVector3				fScale;		// Room: X=X, Y=Y, Z=Z
	TVector3				fRotate;

	BuildingPrim*			fBuilding;
private:
	// Private so the derived class always ask it through accessors
	TBBox2D					fHoleBBox; // the 2D bbox of the outline
	TBBox2D					fSelBBox; // the 2D bbox of the selected points of the outline
};

class WallSubObject : public SubObject
{
public:
//	virtual void		DeleteObj();

	virtual MCCOMErr	Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	static void			CreateWallSubObject(WallSubObject **newObject, Wall* onWall, BuildingPrim* building);

	void				Clone(WallSubObject** newObject, Wall* onWall, const ECloneChildrenMode cloneMode);

	// Will modify the positions of the points in the fHolePolyline using the 
	// new center location. Note that the center will pe projected on the wall.
	// Return false if the operation can't be done ( for lack of space )
	boolean				SetCenter( const TVector3& center, const boolean checkFirst );
	virtual boolean		OffsetPolyline(const TVector2& offset, const boolean checkFirst);
	virtual boolean		ScalePolyline(const TVector2& scale, const boolean checkFirst);
	virtual real32		GetThickness() const;
	void				SetOnWall( Wall* onWall );
	Wall*				GetOnWall(){ return fOnWall; }
	virtual void		InvalidateTessellation();

	// Move and scale the object to fit it in the wall. Delete it and return false if couldn't
	boolean				CheckObjectPosition();

	void				GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K);

	inline const boolean	OnLeftSide() const {return fOnLeftSide;}
	inline void				SetOnLeftSide(const boolean onLeft) {fOnLeftSide=onLeft;}
protected:
	WallSubObject(Wall* onWall);
	virtual ~WallSubObject();

	// Build a box the size of the hole
	virtual void		Validate();
	virtual void		ComputeTreeTransform(TTreeTransform& treeTransform, 
											const TBBox3D& originBBox);

	TMCCountedPtr<Wall>	fOnWall;

	boolean				fOnLeftSide;// use the left side as a plane reference
									// <=> side number 1. True by default.
};

class RoomSubObject : public SubObject
{
public:
//	virtual void		DeleteObj();

	virtual MCCOMErr	Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	static void			CreateRoomSubObject(RoomSubObject **newObject, Room* inRoom, BuildingPrim* building);

	void				Clone(RoomSubObject** newObject, Room* inRoom, const ECloneChildrenMode cloneMode);

	boolean				SetCenter( const TVector3& center, const boolean checkFirst  );
	virtual boolean		OffsetPolyline(const TVector2& offset, const boolean checkFirst);
	virtual boolean		ScalePolyline(const TVector2& scale, const boolean checkFirst);
	virtual real32		GetThickness() const;
	void				SetInRoom( Room* inRoom );
	Room*				GetInRoom(){ return fInRoom; }
	virtual void		InvalidateTessellation();
	void				InvalidateTessellationOver();
	void				GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K);

	boolean				CheckObjectPosition();

protected:
	RoomSubObject(Room* inRoom);
	virtual ~RoomSubObject();

	// Build a box the size of the hole
	virtual void		Validate();
	virtual void		ComputeTreeTransform(TTreeTransform& treeTransform, 
											const TBBox3D& originBBox);

	TMCCountedPtr<Room>	fInRoom;
};



#endif

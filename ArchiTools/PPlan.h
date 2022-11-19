/****************************************************************************************************

		PPlan.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PPlan__
#define __PPlan__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"
#include "MCCountedPtr.h"
#include "MCCountedPtrArray.h"

#include "PPoint.h"
#include "PWall.h"
#include "PRoom.h"
#include "PRoof.h"
class Wall;
class Level;
class VPoint;
class Room;
class RoomSubObject;
struct IShTokenStream;

class Plan // Agreged with the Level: do not count this one : public TMCCountedObject
{
public:

	void				DeletePlan();

	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	void				AddPointReference(VPoint* point){fPointArray.AddElem(point);}
	void				AddWallReference(Wall* wall){fWallArray.AddElem(wall);}
	void				AddRoomReference(Room* room){fRoomArray.AddElem(room);}
	void				AddRoofReference(Roof* roof){fRoofArray.AddElem(roof);}

	void				RemovePointReference(VPoint* point);
	void				RemoveWallReference(Wall* wall);
	void				RemoveRoomReference(Room* room);
	void				RemoveRoofReference(Roof* roof);

	Room*				GetNearestRoom(const TVector2& pos);
	Room*				PosInRoom(const TVector2& pos);
	void				AddPointInRoom(Room* room);
	void				GetRoomsPerimeter(TMCCountedPtrArray<VPoint>& perimeter);
	void				GetSelectedRoomsPerimeter(TMCCountedPtrArray<VPoint>& perimeter);

	Wall*				PosOnWall(const TVector2& pos);
	VPoint*				PointOnPoint( VPoint* point );

	boolean				GetWallBBox( TBBox3D& bbox, const boolean exact, const boolean onSelection  );

	inline int32		GetRoofCount(){return fRoofArray.GetElemCount();}
	inline int32		GetRoomCount(){return fRoomArray.GetElemCount();}
	inline int32		GetWallCount(){return fWallArray.GetElemCount();}
	inline int32		GetPointCount(){return fPointArray.GetElemCount();}

	inline Roof*		GetRoof(const int32 i){return fRoofArray[i];}
	inline Room*		GetRoom(const int32 i){return fRoomArray[i];}
	inline Wall*		GetWall(const int32 i){return fWallArray[i];}
	inline VPoint*		GetPoint(const int32 i){return fPointArray[i];}

	VPoint*				GetPointHelper();

	void				GetRoomObjectList( TMCCountedPtrArray<RoomSubObject>& objects);
	void				GetSelectedObjects(TMCCountedPtrArray<WallSubObject>& wallObj, TMCCountedPtrArray<RoomSubObject>& roomObj);

	inline Level*		GetLevel(){return fLevel;}
	Level*				GetLevelOver();
	Level*				GetLevelUnder();
	const real32		GetDistanceToGround()const;
	const real32		GetLevelHeight()const;
	const int32			GetLevelIndex()const;

	void				MergePlan(Plan* plan);

	void				Split();
	void				Merge();

	// Smooth the selescted points on the holes
	void				Smooth(const boolean smooth);
	
//	Room*				PointIsInRoom(const TVector2& point);

	void				InvalidateObjectSelection();
	void				InvalidateSelection();
	void				InvalidateExtendedSelection();
	void				InvalidateAll();
	void				CheckExtendedSelection();
	void				InvalidateTessellation();
	void				AddWallsAndRoomsToExtendedSelection();
	void				RestoreWallsAndRoomsExtendedSelection();

	boolean				CheckWallConsistency( Wall* onWall, const int32 startIndex, const boolean canDelete );
	void				RebuildInvalidRooms();

	boolean				HasPointSelection() const ;
	boolean				CheckRoofPointSelection();

	void				DeleteSelection();
	void				InvertSelection();
	void				SelectByName(const TMCString& name, const boolean select);
	void				SelectByDomain(const int32 domain, const boolean select);
	void				FlipSelection(const TVector2& center, const int32 axis);

	void				OffsetSelection(const TVector2& offset);
	void				ScaleSelection(const TVector2& scale, const TVector2& center, const EOptionMode mode);
	void				RotateSelection(const TVector2& cosSin, const TVector2& center);

	void				OffsetObj(const TVector2& offset);
	void				ScaleObj(const TVector2& scale);
	void				RotateObj(const TVector2& cosSin);

	void				SetWallOffset(real32 offset, boolean computeSegCount);

	boolean				SnapPosWithAxis(TVector2& pos, const TVector2& axis, const TVector2& preferedProjDir);

	void				InitMarqueeSelection();
	void				SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode);

	void				ClearPointFlag(const int32 flag);
	void				ClearWallFlag(const int32 flag);
	void				ClearRoomFlag(const int32 flag);

	void				SetPointFlag(const int32 flag);
	void				SetWallFlag(const int32 flag);
	void				SetRoomFlag(const int32 flag);

	void				SetSelectionHelper(const boolean set);
	void				CheckSelectionConsistency();

	boolean				BuildPath( Wall* onWall, TMCCountedPtrArray<VPoint>& path, const boolean turnLeft );
	boolean				BuildRoomFromSelection();

	void				ShowRoofs();
	void				ShowRooms();
	void				ShowWalls();
	void				ShowHolePoints();
	void				HideRoofs();
	void				HideRooms();
	void				HideWalls();
	void				HidePoints();
	void				HideHolePoints();

	// Shading Domain
	void				SetShadingDomain(const int32 domainID, const int32 selectionSubPart);
	void				DelShadingDomain(const int32 domainID, const int32 replaceByID);

	// Cut with roofs
	void				CutWallWithRoofs(Wall* wall);
	void				CutRoomWithRoofs(Room* room);
	void				CutGeometryWithRoofs();

	// Object children instances 
//	void				DecalChildren( const int32 afterIndex );

protected:

	// Debug method
	boolean HasRoomReference(Room* room);

	friend class Level; // To agregated the Plan in the Level ( a Level always has a Plan )

	Plan(Level* onLevel);
	virtual ~Plan();

	void RemoveLevelIfEmpty();

	Level* fLevel;

	TMCCountedPtrArray<Roof>	fRoofArray;
	TMCCountedPtrArray<Room>	fRoomArray;
	TMCCountedPtrArray<Wall>	fWallArray;
	TMCCountedPtrArray<VPoint>	fPointArray;

// Maybe	TMCCountedPtrArray<SubObject>	fRoomObjectArray;
// Maybe	TMCCountedPtrArray<SubObject>	fWallObjectArray;
};









#endif

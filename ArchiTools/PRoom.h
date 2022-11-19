/****************************************************************************************************

		PRoom.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PRoom__
#define __PRoom__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"
#include "MCCountedPtr.h"

#include "BuildingPrimitiveData.h"
#include "ArchiTools.h"

#include "PPoint.h"
#include "PFacet.h"
#include "PVertex.h"
#include "PCommonBase.h"
#include "PCutter.h"
#include "I3dShFacetMesh.h"
class BuildingPrim;
class Level;
class VPoint;
class Facet;
class RoomSubObject;
struct IShTokenStream;

enum ERoomFlags
{
	eRoomTessellated = 0x00010000,
	eRoomExtendedSelection = 0x00020000,
	eRoomBasicTessellated = 0x00040000,
	eRoomObjPositionned = 0x00080000,
	eTurnLeft = 0x00100000,// To know the orientation of the path
	eNoCeiling = 0x00200000,
};

//#define USE_POINT_IN

class Room :	public CommonBase
{
public:

	static void			CreateRoom(Room **room, BuildingPrimData* data, Level* inLevel, 
									TMCCountedPtrArray<VPoint>& path, const boolean turnLeft);
	void				DeleteRoom();

	void				Clone(Room** newRoom, Level* inLevel,const TMCCountedPtrArray<VPoint>& newPoints, const ECloneChildrenMode cloneMode);

	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	void				RegisterInWalls();
	void				UnregisterInWalls();

	void				ReplacePointReferences(VPoint* oldPt,VPoint* newPt);

	void				SetSelection(const boolean select);
	void				SetCompleteSelection();
	void				SelectIfPossible();
	void				RestoreIfPossible();
	void				ShowRoom();
	void				HideRoom();

	void				SetPointFlag(const int32 flag);
	void				SetWallFlag(const int32 flag);
	void				ClearPointFlag(const int32 flag);
	void				ClearWallFlag(const int32 flag);

	bool				GetRoomFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlag);

	void				InvalidateTessellation(const boolean invalidateArround=false);

	TMCClassArray<Triangle>& Triangles(){ValidateTessellation(); return fTriangleArray;}
	TMCClassArray<Vertex>& Vertices(){ValidateTessellation(); return fVertexArray;}

	const TMCClassArray<Triangle>& FlatTriangles(){ValidateTessellation(); return fFlatMesh.mTriangles;}
	const TMCClassArray<TVector2>& FlatVertices(){ValidateTessellation(); return fFlatMesh.mVertices;}

	void				SetFloorThickness(const real32 t);
	void				SetCeilingThickness(const real32 t);
	inline real32		GetFloorThickness() const { return(fFloorThickness==kDefaultThickness?fData->GetDefaultFloorThickness():fFloorThickness);}
	inline real32		GetCeilingThickness() const { return(fCeilingThickness==kDefaultThickness?fData->GetDefaultCeilingThickness():fCeilingThickness);}
	inline real32		GetFloorToCeiling() const { return GetRoomHeight()-(GetFloorThickness()+GetCeilingThickness()); }
	real32				GetRoomHeight() const;
	inline real32		GetMidHeight() const { return (GetFloorThickness()+.5*GetFloorToCeiling()); }

	inline void			NoCeiling(const boolean no){no?SetFlag(eNoCeiling):ClearFlag(eNoCeiling);}
	inline boolean		NoCeiling()const{return Flag(eNoCeiling);}

	const inline int32	GetPathPointCount() const {return fPath.GetElemCount();}
	inline VPoint*		GetPathPoint( const int32 iPt ) {return fPath[iPt];}
	inline TMCCountedPtrArray<VPoint>&	GetPath() {return fPath;}
	Wall*				GetPathWall(const int32 index) const;

	boolean				OffsetThisObject(RoomSubObject* object, const TVector2& offset);
	boolean				ScaleThisObject(RoomSubObject* object, const TVector2& scale);

	// Call this when a point of the room is offset
	void				OffsetAttachedObjects(const TVector2& offset);

	// Points inside the room
	boolean				PointIn(const TVector2& point, const boolean exact);
#ifdef USE_POINT_IN
	void				AddPointInReference(Point* point){fPointInArray.AddElem(point);}
	inline TMCCountedPtrArray<VPoint>&	GetPointsIn() {return fPointInArray;}
#endif

	void				GetBestPointIn(TVector2& bestPoint, const TVector2& pos);

	void				InsertPointBetween(VPoint* insert, VPoint* point0, VPoint* point1);

	// SubObjects access
	void				AddObjectReference(RoomSubObject* obj);
	void				RemoveObjectReference(RoomSubObject* obj);
	const int32			GetObjectCount()const{return fSubObjects.GetElemCount();}
	RoomSubObject*		GetObject(const int32 i) const { return fSubObjects[i];}
//	void				ValidateObjPositions();

	Level*				GetLevel(){return fLevel;}
	void				SetLevel(Level* level);

	BuildingPrimData*	GetData() const {return fData;}
	void				SetData(BuildingPrimData* data);

	BuildingPrim*		GetBuildingPrim() const ;

	int32				GetPathPointIndex(VPoint* point) const;
	int32				GetInPointIndex(VPoint* point) const;

	int32				GetTgleOffset() const {return fTgleOffset;}
	// Domain
	void				SetFloorDomain(const int32 id){fFloorDomain=id;}
	void				SetCeilingDomain(const int32 id){fCeilingDomain=id;}
	void				SetWallsShadingDomain(const int32 domain);

	int32				GetFloorDomain() const {return fFloorDomain;}
	int32				GetCeilingDomain() const {return fCeilingDomain;}
	int32				GetWallsDomain() const ;

	// Cut the geometry
	void				CutGeometry(const TMCArray<TriangleVertices>& roofTriangles, const real32 roofMin, const real32 roofMax);
	
	// Data use to unfold
	struct UnfoldUV
	{
		UnfoldUV() : mOffset(0), mCeilingBetweenOffset(0), mFloorBetweenOffset(0), mPrevRoom(0) {}

		real32	mOffset; // Offset the level so the rooms UV are not over each others
		real32	mCeilingBetweenOffset;
		real32	mFloorBetweenOffset;
		Room*	mPrevRoom;
	};
	Room::UnfoldUV& UnfoldUVData() { return mUnfoldUVData; }


protected:

	Room(BuildingPrimData* data, Level* inLevel, 
		TMCCountedPtrArray<VPoint>& path, const boolean turnLeft);
	virtual ~Room();

	void ValidateTessellation();

	void AddFacet( const boolean ceiling, const int32 index1,const int32 index2,const int32 index3);

	// Flat tesselation
	void BuildFlatTessellation(const FlatPolygon& contour);
	// 3D tesselation
	void TessellatFloor(const FlatPolygon& contour);
	void TessellatCeiling(const FlatPolygon& contour);
	void AddVerticalPolygonToTessellation(const FlatPolygon& polygon, float zMin, float zMax, 
											float minOffset, float maxOffset, bool ceiling  );

	TVector2 ComputeUV(const TVector2& pos, bool ceiling);

	TMCCountedPtr<Level> fLevel;

	BuildingPrimData*	fData;

	TMCCountedPtrArray<VPoint>	fPath;
#ifdef USE_POINT_IN
	TMCCountedPtrArray<VPoint>	fPointInArray; // poit that define walls inside the room
#endif

	// The array of objects in the floor (stairways, chimey, ...)
	TMCCountedPtrArray<RoomSubObject> fSubObjects;

	real32				fFloorThickness;
	real32				fCeilingThickness;
	// Shading Domains
	int32				fFloorDomain;
	int32				fCeilingDomain;

	TMCClassArray<Triangle>	fTriangleArray;
	TMCClassArray<Vertex>	fVertexArray;
	int32					fOffset;	// The first vertices are for the floor.
										// The one after fOffset are for the Ceiling
	int32					fTgleOffset;// Before: floor, after:ceiling

	// A basic tessellation without the holes in it. Use the vertices of the path
	FlatMesh				fFlatMesh;
	
	// UV
	UnfoldUV	mUnfoldUVData;
};



#endif

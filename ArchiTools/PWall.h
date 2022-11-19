/****************************************************************************************************

		PWall.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PWall__
#define __PWall__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"
#include "MCCountedPtr.h"
#include "MCClassArray.h"
#include "Vector3.h"
#include "I3dShFacetMesh.h" // For the class Triangle
#include "PCircleArc.h"
#include "PBooleanPolygon.h"
#include "PRoom.h"

class VPoint;
class Level;
class Facet;
class Vertex;

#include "BuildingPrimitiveData.h"
#include "ArchiTools.h"
#include "PSubObject.h"
#include "PCommonBase.h"
#include "PCutter.h"
#include "PConstrPoint.h"

class WallSubObject;
struct FatRay;
struct IShTokenStream;

// the 16 last bits are for the common flags
enum EWallFlags
{
	eWallTessellated	= 0x00010000,
	eWallFree			= 0x00020000,	// A wall that does not delimited a room
	eWallExtendedSelection = 0x00040000,
	eWallHelper			= 0x00080000,
	eWallRebuildRoom0	= 0x00100000, // true when the left room need to be rebuild before being used again
	eWallRebuildRoom1	= 0x00200000, // true when the right room need to be rebuild before being used again
	eWallRoom0Done		= 0x00400000,
	eWallRoom1Done		= 0x00800000,
	eWallExtraHeight	= 0x01000000, // build gable (higher walls) when under roofs
	eWallSetDomains		= 0x02000000, // true when a shading domain has been specificaly set
	eWallBaseIsValid	= 0x04000000,
};

enum EPointPos
{
	eOutside,
	eInvalidInside,
	eInside,
	eUnknownPointPos = 0xEEEEEEEE
};

class PolygonSet : public TMCClassArray<FlatPolygon>
{
};

class WallWithCrenel;

class Wall :	public CommonBase
{
public:
	// To replace the dynamic cast
	virtual WallWithCrenel* GetWallWithCrenel() { return NULL; }

	static void			CreateWall(Wall **wall, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);
	void				DeleteWall();

	virtual void		Clone(Wall** newWall, Level* inLevel, VPoint* point0, VPoint* point1, const ECloneChildrenMode cloneMode);
	virtual void		CopyFrom( Wall* otherWall, const ECloneChildrenMode cloneMode );

	virtual MCCOMErr	Write(IShTokenStream* stream);
	virtual MCCOMErr	Read(IShTokenStream* stream); 

	virtual void		ExtraHeight(const boolean extra){extra?SetFlag(eWallExtraHeight):ClearFlag(eWallExtraHeight);}
	inline boolean		ExtraHeight()const{return Flag(eWallExtraHeight);}

	VPoint*				Split(const TVector2& splitPos);
	boolean				Split(VPoint* splitPoint);
	
	void				SetSelection(const boolean select);
	void				SetCompleteSelection();
	void				ShowWall();
	void				HideWall();


	bool				GetWallFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags);
	void				GetWallData(const TVector2& flatPos,
									bool clamp,
									TVector2& position2D,
									TVector2& leftPos2D,
									TVector2& rightPos2D,
									TVector2& normal2D,
									TVector2& direction2D );

	const TMCPtrArray<ConstrPoint>&		GetWallConstructionPoints();
	const TMCClassArray<TVector2>&		GetLeftPosProjection();
	const TMCClassArray<TVector2>&		GetRightPosProjection();

	virtual real32		GetWallTotalHeight() const;
	real32				GetWallHeight() const;
	inline real32		GetThickness() const { return(fThickness==kDefaultThickness?fData->GetDefaultWallThickness():fThickness);}
	real32				GetStraightLength() const;
	real32				GetLeftHeight() const;
	real32				GetRightHeight() const;

	// Use kDefaultLevelHeight to have the default level height
	void				SetWallHeight(const real32 h);
	// Use kDefaultThickness to have the default thickness
	void				SetThickness(const real32 t);

	real32				GetActualArcOffset() const { return GetArcSegmentCount()>1?mCircleArc.GetOffset():0; }
	real32				GetArcOffset() const { return mCircleArc.GetOffset(); }
	int32				GetArcSegmentCount() const { return mCircleArc.GetPointCount()-1; }
	const TMCClassArray<TVector2>& GetCircleArc()  { return mCircleArc.GetArc(); }
	void				SetArcOffset(real32 offset) { mCircleArc.SetOffset(offset); }
	void				SetArcSegmentCount(int32 seg) { mCircleArc.SetPointCount(seg+1); }
	void				ComputeArcSegmentCount();
	TVector2			GetMidPointPos();

	VPoint*				GetPoint(const int32 i) const {return (i==0?fPoint0:fPoint1);}
	VPoint*				GetOtherPoint(const VPoint* point) const {if(fPoint0==point) return fPoint1;else return fPoint0;}
	int32				GetPointIndex(VPoint* point) const {return(fPoint0==point?0:1);}

	TVector2			GetMidPos();
	TVector2			GetStraightDirection(VPoint* fromPoint);
	TVector2			GetCurvedDirection(VPoint* fromPoint);
	void				GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K);

	boolean				OffsetThisObject(WallSubObject* object, const TVector2& offset);
	boolean				ScaleThisObject(WallSubObject* object, const TVector2& scale);
	void				CheckRoomObjConflict(RoomSubObject* obj);
	boolean				CheckWallObjects();

	int32				GetRoomCount(){int32 c=0; if(fRoom0) c++; if(fRoom1) c++; return c;}
	int32				GetSelectedRoomCount();
	int32				GetRoomIndex(const Room* room) const {return (room==fRoom0?0:room==fRoom1?1:-1);}
	// Warning: can return NULL
	Room*				GetRoom(const int32 i) const {return (i==0?fRoom0:fRoom1);}
	Room*				GetOtherRoom(const Room* room) const {if(fRoom0==room) return fRoom1;else return fRoom0;}
	Room*				GetLeftRoom() const {return fRoom0;}
	Room*				GetRightRoom() const {return fRoom1;}
	void				SetRoom(const int32 index, Room* room){ index?fRoom1=room:fRoom0=room;}

	// SubObjects access
	void				AddObjectReference(WallSubObject* obj){fSubObjects.AddElem(obj);ClearFlag(eWallTessellated);}
	void				RemoveObjectReference(WallSubObject* obj);
	int32				GetObjectCount() const {return fSubObjects.GetElemCount();}
	WallSubObject*		GetObject(const int32 i) const { return fSubObjects[i];}

	const TBBox3D&		GetWallBBox(){ ValidateTessellation(); return fWallBBox; }

	// Return a location where an object could inserted in the wall
	TVector3			GetSnappedPosOnWall(const TVector3& hitPoint, const EObjectType& forObject );

	TVector2			GetMiddle2D();
	TVector3			GetMiddle();

	void				ReplacePointReference(VPoint* oldPt,VPoint* newPt);
	void				RemoveRoomReference(Room* room){MY_ASSERT((fRoom0==room)||(fRoom1==room));if(fRoom0==room)fRoom0=NULL;if(fRoom1==room)fRoom1=NULL;InvalidateTessellation();}

	TMCClassArray<Triangle>& Triangles(){ValidateTessellation(); return fTriangleArray;}
	TMCClassArray<Vertex>& Vertices(){ValidateTessellation(); return fVertexArray;}

	void				InvalidateTessellation(const boolean invalidateAround=false);

	boolean				CheckConsistency(const boolean canDelete);

	Level*				GetLevel() const { return fLevel; }
	void				SetLevel(Level* level);

	BuildingPrimData*	GetData() const {return fData;}
	void				SetData(BuildingPrimData* data);

	// Domains access

	inline void			SetLeftDomain(const int32 id){fLeftDomain=id;SetDomainFlag();}
	inline void			SetRightDomain(const int32 id){fRightDomain=id;SetDomainFlag();}
	inline void			SetBetweenDomain(const int32 id){fBetweenDomain=id;SetDomainFlag();}

	inline int32		GetLeftDomain() const {return fLeftDomain;}
	inline int32		GetRightDomain() const {return fRightDomain;}
	inline int32		GetBetweenDomain() const {return fBetweenDomain;}

	// Data use to unfold
	struct UnfoldUV
	{
		UnfoldUV() : mOffset(0), mPolygonBetweenOffset(0), mPrevWall(NULL) {}
		real32	mOffset; // Offset the wall so its UV are not over the previous wall
		real32	mPolygonBetweenOffset;
		Wall*	mPrevWall;
	};
	Wall::UnfoldUV& UnfoldUVData() { return mUnfoldUVData; }


	BuildingPrim* GetBuildingPrim() const ;

	// Cutting of the geometry
	void				CutGeometry(const TMCArray<TriangleVertices>& roofTriangles, const real32 roofMin, const real32 roofMax);

protected:

	void Init(); // Init the default values

	void ValidateArc();

	// Build the facet and vertex arrays
	void ValidateTessellation();
	void ValidateTessellation2();
	void AddFlatMeshToTessellation( const FlatMesh& flatMesh, int32 segmentID, real32 leftHeight, real32 rightHeight  );
	void AddPolySetToTessellation( const PolygonSet& polyset, int32 segmentID, real32 leftHeight, real32 rightHeight  );
	void AddPolygonToTessellation( const FlatPolygon& polygon, int32 segmentID, real32 leftHeight, real32 rightHeight  );

	void BuildRectangleUsingPos(const int32,const int32,const int32,const int32,const TVector3&);
	void BuildRectangleUsingPos(const int32,const int32,const TVector3&,const TVector3&,const TVector3&);

	// Custom tessellation steps
	virtual void AddExtraPolygones( BooleanPolygon& booleanPoly ) {}

	inline void ValidateDomains();
	inline void SetDomainFlag();

	TVector2 ComputeUV(const TVector2& pos, bool left);

	Wall(BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);
	virtual ~Wall();

	// Must always have 2 points
	TMCCountedPtr<VPoint>	fPoint0;
	TMCCountedPtr<VPoint>	fPoint1;

	TMCCountedPtr<Level>	fLevel;
	BuildingPrimData*		fData;

	ThickCircleArc	mCircleArc;
	bool			mSmooth;

	// The description of the wall in 2D
	TMCPtrArray<ConstrPoint>	mConstrPoints;
	TMCClassArray<TVector2>		mLeftPos;
	TMCClassArray<TVector2>		mRightPos;

	// To build the facets: these are the real positions
	TMCClassArray<Triangle>	fTriangleArray;
	TMCArray<int32>			fTriangleDomain; // store the shading domains
	TMCClassArray<Vertex>	fVertexArray;

	// Can separate 2 rooms, be in 1 room, or even be "free", without any room
	TMCCountedPtr<Room>	fRoom0;
	TMCCountedPtr<Room>	fRoom1;

	// The array of objects in the wall (windows, doors, ...)
	TMCCountedPtrArray<WallSubObject> fSubObjects;

	real32		fThickness;
	real32		fHeight; // it's possible to create walls shorter than the floor height

	TBBox3D		fWallBBox;

	// Shading domains IDs
	int32		fLeftDomain;
	int32		fRightDomain;
	int32		fBetweenDomain;

	// Attached base (use GetBase to access them)
	TVector3 fO;
	TVector3 fI;
	TVector3 fJ;
	TVector3 fK;

	// UV
	UnfoldUV	mUnfoldUVData;
};

inline void Wall::SetDomainFlag()
{
	if( (fRoom0&&fLeftDomain==eInsideWallDomain || !fRoom0&&fLeftDomain==eOutsideWallDomain) &&
		(fRoom1&&fRightDomain==eInsideWallDomain || !fRoom1&&fRightDomain==eOutsideWallDomain) && 
		(fRoom0&&fRoom1&&fBetweenDomain==eInsideWallDomain || ((!fRoom0||!fRoom1)&&fBetweenDomain==eOutsideWallDomain)) )
	{	// These are the default settings, clear the flag
		ClearFlag(eWallSetDomains);
	}
	else
	{	// Custom settings
		SetFlag(eWallSetDomains);
	}
}

inline void Wall::ValidateDomains()
{
	if(Flag(eWallSetDomains))
		return; // Custom shading

	if(fLeftDomain==eInsideWallDomain||fLeftDomain==eOutsideWallDomain) // Set the default domain only if no custom domain has been set
		fLeftDomain = (fRoom0?eInsideWallDomain:eOutsideWallDomain);
	if(fRightDomain==eInsideWallDomain||fRightDomain==eOutsideWallDomain) // Set the default domain only if no custom domain has been set
		fRightDomain = (fRoom1?eInsideWallDomain:eOutsideWallDomain);
	if(fBetweenDomain==eInsideWallDomain||fBetweenDomain==eOutsideWallDomain) // Set the default domain only if no custom domain has been set
		fBetweenDomain = (fRoom0&&fRoom1?eInsideWallDomain:eOutsideWallDomain);
}


#endif

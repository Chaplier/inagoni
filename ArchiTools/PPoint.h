/****************************************************************************************************

		PPoint.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PPoint__
#define __PPoint__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCAssert.h"
#include "MCCountedObject.h"
#include "Vector2.h"
#include "Vector3.h"
#include "MCCountedPtrArray.h"
#include "PWall.h"
#include "PCommonBase.h"
#include "Utils.h"
#include "I3DShFacetMesh.h"

class Level;
class Wall;
class Room;
class Vertex;
struct IShTokenStream;

enum EPointFlags
{
	eWallOrdered			= 0x00010000,
	ePointHelper			= 0x00020000,
	ePointToDelete			= 0x00040000,
	ePointTessellated		= 0x00080000,
};

//Alan

class VPoint :	public CommonPoint
{
public:

	static void			CreatePoint(VPoint **point, BuildingPrimData* data, Level* inLevel, const TVector2& pos, Wall* onWall);
	static void			CreatePoint(VPoint **point, BuildingPrimData* data, Level* inLevel, const TVector2& pos, const TMCCountedPtrArray<Wall>& onWalls);
	void				DeletePoint();

	void				Clone(VPoint** newPoint, Level* inLevel);

	void				SetPositionCheckWalls( const TVector2& newPos );
	TVector3			Get3DPos() const;
	const int32			GetLevelIndex()const;

	void				AddWallReference( Wall* wall ){fWallArray.AddElem(wall);ClearFlag(eWallOrdered);InvalidateTessellation();}
	void				RemoveWallReference( Wall* wall );

	// Merge the passed point into this one
	void				Merge(VPoint* mergedPoint, const boolean canDelete);

	void				SetSelection( const boolean select );
	void				DeselectIfPossible();
	void				SetExtendedSelection(); // to set the zone that will invalidate
	virtual void		InvalidateTessellation(const boolean extraInvalidation=false); // invalidate walls and rooms that depend of this point
	boolean				SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode);
	void				HidePoint();
	void				InvertSelection();

	// Return the first wall on the left or right if you're on the point and 
	// looking in the passed wall direction
	Wall* 				GetLeftWall(Wall* wall);
	Wall* 				GetRightWall(Wall* wall);
	// Return the wall between 2 points
	Wall*				GetWall(VPoint* otherPoint);
	Wall*				GetWall(const int32 i){ if(!Flag(eWallOrdered))OrderWalls();return fWallArray[i];}
	int32				GetWallCount(){ return fWallArray.GetElemCount();}
	int32				GetWallIndex(Wall* wall);

	Level*				GetLevel() const { return fLevel; }
	void				SetLevel(Level* level);

	BuildingPrimData*	GetData() const {return fData;}
	void				SetData(BuildingPrimData* data);

	// Compute the actual postion of the vertices, using the thickness
	// of the wall and the other walls around
	void				GetLeftPos(TVector3& pos, Wall* wall);
	bool				GetLeftPos(TVector2& pos, Wall* wall);
	void				GetRightPos(TVector3& pos, Wall* wall);
	bool				GetRightPos(TVector2& pos, Wall* wall);

//	const void			GetCornerVertex(Room* room, Vertex& vertexDown, Vertex& vertexUp);

	void				SetIndex(const int32 index){ fIndex = index; }
	int32				GetIndex(){ return fIndex; }

	boolean				IsInRoomPath() const;
	boolean				CheckConsistency(const boolean canDelete);

	void				GetSurroundingPoints(TMCClassArray<TVector2>& points);
	virtual void		GetSurroundingPoints(TMCCountedPtrArray<CommonPoint>& pointsArround);

	void				SetWallFlag(const int32 flag);
	void				ClearWallFlag(const int32 flag);

	// Tessellqtion
	bool				GetPointFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags);
	TMCClassArray<Triangle>& Triangles(){ValidateTessellation(); return fTriangleArray;}
	TMCClassArray<Vertex>& Vertices(){ValidateTessellation(); return fVertexArray;}


protected:

	VPoint(BuildingPrimData* data, Level* inLevel, const TVector2& pos, Wall* onWall);
	VPoint(BuildingPrimData* data, Level* inLevel, const TVector2& pos, const TMCCountedPtrArray<Wall>& onWalls);
	virtual ~VPoint();

	// Organize the walls in a counter-clockwise circle around the point
	void		OrderWalls();

	int32		GetWallIndex(const Wall* wall);

	void		GetIntersection(TVector2& intersection, Wall* wall1, Wall* wall2);

	void		ValidateTessellation();
	void		BuildPolygon( const TVector3& normal, real32 z, const TMCClassArray<TVector2>& posArround );
	void		BuildColumn( real32 zBottom, real32 zTop, const TMCClassArray<TVector2>& posArround );

	// Array of the walls using this point
	TMCCountedPtrArray<Wall>	fWallArray;

	TMCCountedPtr<Level>		fLevel;

	int32		fIndex; // used to build the edge mesh;

	TMCClassArray<Triangle>	fTriangleArray;
	TMCClassArray<Vertex>	fVertexArray;
	// Shading Domain
	int32					fPointDomain;
};



#endif

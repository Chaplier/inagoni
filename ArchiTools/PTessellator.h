/****************************************************************************************************

		PTessellator.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/5/2004

****************************************************************************************************/

#ifndef __PTessellator__
#define __PTessellator__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Utils.h"

struct FlatEdge;

struct FlatPoint
{
	TVector2	f2DPoint;
	int32		fIndex; // to build the facet mesh
	TMCArray<int32> fEdges;
	int32		fOnHole; // -1 : on container. Otherwise hole ID
};

struct PolygonIndexes
{
	TMCArray<int32> fPoints;
};

struct FlatEdge
{
	int32 fPoint1;
	int32 fPoint2;
	int32 fUsed;

	int32 OtherPoint(int32 p){return( p==fPoint1?fPoint2:fPoint1 );}
};

class PosArray : public TMCClassArray<TVector2> {};

// Build polygons using the squares to cut holes in the countainer
class FlatPolygonMaker
{
public:
	boolean			AddPointToCountainer(const TVector2& point, 
		const int32 index, const boolean withCaution=false, const boolean lastPoint=false);
	// IMPORTANT : add the points before adding the PolyHole (for fOnHole index)
	void			AddPoint(const TVector2& point, const int32 index);
	void			AddPolyHole(TMCArray<int32>& points);

	int32			MakePolygons(TMCClassArray<Triangle>& triangleArray);
	void			MakePolygons();
	inline int32	GetPolygonCount()const{return fPolygons.GetElemCount();}
	void			GetPolygon( const int32 index, TMCClassArray<FlatPoint>& polygon );

	inline int32	GetCountainerPointCount() const {return fCountainer.fPoints.GetElemCount();}

	// Return the list of the points. It may be different before and after the tessellation
	const TMCClassArray<FlatPoint>&		GetPointList() const { return fPointList; }
	const PolygonIndexes&					GetContainer() const { return fCountainer; }
	const TMCClassArray<PolygonIndexes>&	GetPolyHoles() const { return fPolyHoles; }

protected:

	void			MergeHoles();

	boolean			MakePolygon(FlatEdge* edge, const int32 point, PolygonIndexes& polygon);
	int32			GetNearestPoint(FlatPoint& thisPoint,
									FlatPoint& prevPoint, 
									FlatPoint& nextPoint, 
									TMCClassArray<FlatPoint>& inList,
									const bool clockWise,
									TMCClassArray<PosArray>& holes);
	int32			SegmentCutsHole(const TVector2& point1, const TVector2& point2,
									const TMCClassArray<PosArray>& holes, const int32 onHole);

	bool SegmentCutEdge( const TVector2& point1, const TVector2& point2);
	void TryToAddEdge(const int32 thisPointIndex, const bool clockwiseOrder, FlatPoint& thisPoint, FlatPoint& prevPoint, FlatPoint& nextPoint);
	void LinkCountainerWithHoles();

	PolygonIndexes fCountainer;
	TMCClassArray<PolygonIndexes> fPolyHoles;


	// Result
	TMCClassArray<PolygonIndexes> fPolygons;

private:
	void PrepareCachedData();
	// Cached data: the holes, but with vector2
	TMCClassArray<PosArray> fCachedHolePos;
	// Cached data: list of all the points and edges
	TMCClassArray<FlatPoint> fPointList;
	TMCClassArray<FlatEdge> fEdgeList;

};


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

namespace DelauneyCopy
{

class Vertex2DIndex
{
public:
	// no constructor taking a int32 enforce no implicit conversion
	inline Vertex2DIndex() : fValue(0) {}

	inline Vertex2DIndex(const Vertex2DIndex& index){fValue = index.fValue;}
	inline Vertex2DIndex& operator=(const Vertex2DIndex& index){fValue = index.fValue;return *this;}
	inline void operator++()								{fValue++;}
	inline boolean operator<(int32 count)					{return fValue<count;}
	inline boolean operator==(const Vertex2DIndex& index) const{return fValue == index.fValue;}
	inline boolean operator!=(const Vertex2DIndex& index) const{return fValue != index.fValue;}
	inline int32 GetValue() const{MY_ASSERT(fValue>=0);return fValue;}
	inline void SetValue(int32 value){MY_ASSERT(fValue>=0);fValue=value;}
private:
	int32 fValue;
};

class Vertex2D : public TVector2
{
public:
	enum eLocation
	{
		kInvalid=0,
		kInside=1,
		kOnEdge=2
	};

	Vertex2D(real x,real y,eLocation location);

	void SetInvalid() const { fLocation=kInvalid; }
	
	boolean IsInvalid() const   { return fLocation == kInvalid; }
	boolean IsOnEdge() const    { return fLocation == kOnEdge; }

	mutable eLocation	fLocation;
};

class Vertex2DList
{
public:
	Vertex2DList(const TMCArray<Vertex2D>& vertexArray);

	const Vertex2D& operator[](const Vertex2DIndex& vertexIndex) const { return fVertices2D[vertexIndex.GetValue()]; }

	Vertex2D& operator[](const int32 index){ return fVertices2D[index]; }

	// returns the number of vertices in the list
	int32 GetCount() const { return fVertices2D.GetElemCount(); }

private:
	TMCArray<Vertex2D>	fVertices2D;
};

class Triangle2D
{
public:
	Triangle2D(Vertex2DIndex vertex1,Vertex2DIndex vertex2,Vertex2DIndex vertex3);

	// returns true if the point is in the circumcircle of the triangle
	boolean InCircle(const TVector2& position,const Vertex2DList& vertices) const;

	// returns true if the edge (vertex1,vertex2) intersect the triangle
	boolean Intersect(Vertex2DIndex vertex1,Vertex2DIndex vertex2,const Vertex2DList& vertices) const;

	// returns true if the edge (vertex1,vertex2) is an edge of the triangle
	boolean ContainsEdge(Vertex2DIndex vertex1,Vertex2DIndex vertex2) const;

	Vertex2DIndex	fVertexIndex[3];
};

class Polygon2D : public TMCArray<Vertex2DIndex>
{
public:
	// returns the number of vertex in the polygon
	int32 GetVertexCount() const { return (int32)GetElemCount(); }

	Vertex2DIndex Vertex(int32 i) const { return (*this)[i]; }

	// try to merge one triangle with this polygon
	boolean Merge(const Triangle2D& triangle);

	// split the polygon by the segment [vertex1,vertex2] ( vertex1 and vertex2 must be vertices of the polygon )
	boolean Split(Vertex2DIndex vertex1,Vertex2DIndex vertex2,Polygon2D& polygon1,Polygon2D& polygon2) const;

protected:
	// return the index of the vertex of the triangle, non contained in commonPoints
	Vertex2DIndex NonCommonVertex(const Triangle2D& triangle,
								const TMCArray<int32>& commonPoints) const;
};

class Segment2D
{
public:
	Segment2D (Vertex2DIndex vertex1,Vertex2DIndex vertex2) : fVertex1(vertex1), fVertex2(vertex2)
	{
	}
	boolean IsInside(const TMCArray<Vertex2DIndex>& polygon,const Vertex2DList& vertices);

private:
	Vertex2DIndex fVertex1;
	Vertex2DIndex fVertex2;
};


// ************************************************************************
//						class Triangulator
//
// implement the classic constraint Delauney Triangulation for a set of
// points contained in a triangle ( the triangle vertices are part of the
// final triangulation
//
// all the vertices should be either inside or on the edges of the triangle
//
class Triangulator2D
{
public:
	// vertices is a set of points for which we want the triangulation
	// the three first points defines the enclosing triangle
	Triangulator2D(const TMCArray<Vertex2D>& vertices2D);

	virtual ~Triangulator2D();

	// Build Initial Triangulation
	void Triangulate();

	// add a constraint to the triangulation ( should be called only after Triangulate as been called)
	void AddConstraint(Vertex2DIndex vertexIndex1,Vertex2DIndex vertexIndex2);

	// returns a list of triangles corresponding to the triangulation
	const TMCPtrArray<const Triangle2D>* GetTriangulation();

	inline const Vertex2D& GetVertex(Vertex2DIndex vertexIndex)
	{
		return fVertices2D[vertexIndex];
	}

	// returns true if there was a constraint set between the two points
	boolean CheckContraint(Vertex2DIndex vertexIndex1,Vertex2DIndex vertexIndex2);

protected:
	// triangulate the polygon around the vertex, vertexIndex
	void TriangulateButterfly(const Polygon2D& polygon,Vertex2DIndex vertexIndex);

	// Merge the triangles into a polygon, returns false if unable to merge
	boolean MergeTriangles(Polygon2D& mergedPolygon,TMCPtrArray<Triangle2D> trianglesToMerge);

	// Triangulate a polygon and add the result to the current triangulation
	boolean Triangulate(const Polygon2D& polygon);

	// Remove duplicated points
//	void PruneDuplicatedPoints();

	// scale the vertices pos to place them in a 0,1 * 0,1 square
	void ScaleData();
private:
	Vertex2DList				fVertices2D; // vertices of the triangulation
	TMCPtrArray<Triangle2D>	fTriangles2D; // current triangulation

	TMCPtrArray< TMCArray<int32> >	fConstraints; // for each vertex, the list of vertex it is linked to by a constraint
};

// performs a basic test of triangulation
// boolean Test();

} // end of namespace DelauneyCopy






#endif

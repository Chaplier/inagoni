/****************************************************************************************************

		PCutter.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	6/29/2004

****************************************************************************************************/

#ifndef __PCutter__
#define __PCutter__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCAssert.h"
#include "Vector3.h"
#include "I3dShFacetMesh.h"
#include "PVertex.h"
#include "MCClassArray.h"

#include "Copyright.h"

// A triangle description in 3D space
struct TriangleVertices
{
#if (VERSIONNUMBER >= 0x060000)
	TVector3d fVertices[3];
#else
	TVector3 fVertices[3];
#endif
};

void CutTriangle(const Triangle& triangle, const TMCClassArray<Vertex>& vertices, 
				 const int32 offsetIndex,
				 TMCClassArray<Triangle>& newTriangles, 
				 TMCClassArray<Vertex>& newVertices,
				 const TMCArray<TriangleVertices>& cutterTriangles,
				 const real32 roofMin, const real32 roofMax);


//////////////////////////////////////////////////////////////////////
//
//
class CutterVertex
{
public:
	CutterVertex()
	{fCutterTgle=-1; fOnTriangle=false; fOnCutterTriangle=false;
	fOnEdge=-1; fOnCutterEdge=-1; fOnVertex=-1; fOnCutterVertex=-1; 
	fIsUsed = true; fReIndex=-1;}

	CutterVertex(	const int32			cutterTgle,
					TVector3&			position)
	{fPosition = position;
	fCutterTgle=cutterTgle; fOnTriangle=false; fOnCutterTriangle=false;
	fOnEdge=-1; fOnCutterEdge=-1; fOnVertex=-1; fOnCutterVertex=-1; 
	fIsUsed = true; fReIndex=-1;}

	virtual ~CutterVertex() {}

	// return the triangle whose intersection generated this vertex
	inline int32		GetTriangleIndex() const { return fCutterTgle; }
	inline void			SetTriangleIndex(int32 i) { fCutterTgle=i; }

	// Returns the position of the vertex in space
	inline const TVector3&	GetPosition(void) const { return fPosition; }
	inline void				SetPosition(const TVector3& pos) { fPosition = pos; }
	
	// Triangle to cut position
	void SetIsInTriangle(boolean onTgle)	{ fOnTriangle = onTgle; }	
	void SetIsOnEdge(int8 edgeId)			{ fOnEdge = edgeId; }
	void SetIsOnVertex(int8 vertexId)		{ fOnVertex = vertexId; }

	// Cutter triangle position
	void SetIsInCutterTriangle(boolean onTgle)	{ fOnCutterTriangle = onTgle; }	
	void SetIsOnCutterEdge(int8 edgeId)			{ fOnCutterEdge = edgeId; }
	void SetIsOnCutterVertex(int8 vertexId)		{ fOnCutterVertex = vertexId; }

	// Triangle to cut position
	boolean IsInTriangle() const	{ return fOnTriangle; }	
	int8 IsOnEdge() const			{ return fOnEdge; }
	int8 IsOnVertex() const			{ return fOnVertex; }

	// Cutter triangle position
	boolean IsInCutterTriangle() const	{ return fOnCutterTriangle; }	
	int8 IsOnCutterEdge() const			{ return fOnCutterEdge; }
	int8 IsOnCutterVertex() const		{ return fOnCutterVertex; }

	boolean				GetIsUsed() const {return fIsUsed;}
	void				SetIsUsed(const boolean b) {fIsUsed = b; if(!b)Reset();}

	int32				GetIndex() const {return fReIndex;}
	void				SetIndex(const int32 b) {fReIndex = b;}

protected:
	inline	void		Reset(){fOnTriangle=fOnCutterTriangle=false;
								fOnEdge=fOnCutterEdge=fOnVertex=fOnCutterVertex=-1;}

private:
	TVector3			fPosition;			// Vertex World Position
	int32				fCutterTgle;		// The triangle from the cutter geometry that made this vertex

	int32				fReIndex; // index after weld

	boolean				fOnTriangle;
	boolean				fOnCutterTriangle;
	int8				fOnEdge; // edge ID (0,1 or 2)
	int8				fOnCutterEdge; // edge ID (0,1 or 2)
	int8				fOnVertex; // vertex ID (0,1 or 2)
	int8				fOnCutterVertex; // vertex ID (0,1 or 2)

	boolean				fIsUsed; // set to false when the vertex is a duplicate of another one
};

//
//
class CutterSegment
{
public:
	CutterSegment(){fIsUsed=true;}
	virtual ~CutterSegment() {}

	void	Init(const int32 index1, const int32 index2);

	int32	GetVertexIndex(const int32 v)const{return fVertexIndex[v];}
	void	SetVertexIndex(const int32 v, const int32 index){fVertexIndex[v] = index;}

	boolean				GetIsUsed() const {return fIsUsed;}
	void				SetIsUsed(const boolean b) {fIsUsed = b;}
private:
	int32	fVertexIndex[2];

	boolean	fIsUsed;
};

class CutterPolyLine
{
public:
	CutterPolyLine() {}
	virtual ~CutterPolyLine() {}

	int32		AddIntersections( const int32 cutterTgle,
								  TVector3 intersections[],
								  int32 posOnTriangleToCut[],
								  int32 posOnCutter[]);

	void		WeldSegments();

	// Debug
	void		Check();

	int32		GetVerticesCount() const {return fCutterVertices.GetElemCount();}
	int32		GetSegmentsCount() const {return fCutterEdges.GetElemCount();}

	const TMCClassArray<CutterSegment>&	GetCutterSegments(void)	const { return fCutterEdges; }
	const TMCClassArray<CutterVertex>&	GetCutterVertices(void) const { return fCutterVertices; }
private:
	TMCClassArray<CutterVertex>		fCutterVertices;
	TMCClassArray<CutterSegment>	fCutterEdges;
};



#endif

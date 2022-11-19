/****************************************************************************************************

		PCutter.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	6/29/2004

****************************************************************************************************/

#include "PCutter.h"

#include "Geometry.h"

#include "PTessellator.h"

#include <iostream>
#include <fstream>

// can also see http://www.magic-software.com/

/*
// Return false if coplanar
boolean GetTriangleIntersection(const TVector3 triangle1[],
								const TVector3 triangle2[],
				 double source[3], double target[3] )
				 
{
  double dp1, dq1, dr1, dp2, dq2, dr2;
  double v1[3], v2[3], v[3];
  double N1[3], N2[3], N[3];
  double alpha;

  // Compute distance signs  of triangle1[0], triangle1[1] and triangle1[2] 
  // to the plane of triangle(triangle2[0],triangle2[1],triangle2[2])


  SUB(v1,triangle2[0],triangle2[2])
  SUB(v2,triangle2[1],triangle2[2])
  CROSS(N2,v1,v2)

  SUB(v1,triangle1[0],triangle2[2])
  dp1 = DOT(v1,N2);
  SUB(v1,triangle1[1],triangle2[2])
  dq1 = DOT(v1,N2);
  SUB(v1,triangle1[2],triangle2[2])
  dr1 = DOT(v1,N2);
  
  if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 

  // Compute distance signs  of triangle2[0], triangle2[1] and triangle2[2] 
  // to the plane of triangle(triangle1[0],triangle1[1],triangle1[2])

  
  SUB(v1,triangle1[1],triangle1[0])
  SUB(v2,triangle1[2],triangle1[0])
  CROSS(N1,v1,v2)

  SUB(v1,triangle2[0],triangle1[2])
  dp2 = DOT(v1,N1);
  SUB(v1,triangle2[1],triangle1[2])
  dq2 = DOT(v1,N1);
  SUB(v1,triangle2[2],triangle1[2])
  dr2 = DOT(v1,N1);
  
  if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

  // Permutation in a canonical form of T1's vertices


  if (dp1 > 0.0f) {
    if (dq1 > 0.0f) TRI_TRI_INTER_3D(triangle1[2],triangle1[0],triangle1[1],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
    else if (dr1 > 0.0f) TRI_TRI_INTER_3D(triangle1[1],triangle1[2],triangle1[0],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
	
    else TRI_TRI_INTER_3D(triangle1[0],triangle1[1],triangle1[2],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
  } else if (dp1 < 0.0f) {
    if (dq1 < 0.0f) TRI_TRI_INTER_3D(triangle1[2],triangle1[0],triangle1[1],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
    else if (dr1 < 0.0f) TRI_TRI_INTER_3D(triangle1[1],triangle1[2],triangle1[0],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
    else TRI_TRI_INTER_3D(triangle1[0],triangle1[1],triangle1[2],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
  } else {
    if (dq1 < 0.0f) {
      if (dr1 >= 0.0f) TRI_TRI_INTER_3D(triangle1[1],triangle1[2],triangle1[0],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(triangle1[0],triangle1[1],triangle1[2],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
    }
    else if (dq1 > 0.0f) {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(triangle1[0],triangle1[1],triangle1[2],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(triangle1[1],triangle1[2],triangle1[0],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
    }
    else  {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(triangle1[2],triangle1[0],triangle1[1],triangle2[0],triangle2[1],triangle2[2],dp2,dq2,dr2)
      else if (dr1 < 0.0f) TRI_TRI_INTER_3D(triangle1[2],triangle1[0],triangle1[1],triangle2[0],triangle2[2],triangle2[1],dp2,dr2,dq2)
      else {
	// triangles are co-planar

		  return false;
//	return coplanar_tri_tri3d(triangle1[0],triangle1[1],triangle1[2],triangle2[0],triangle2[1],triangle2[2],N1,N2);
      }
    }
  }
};
*/
enum vertexPos
{
	kUnknownPos,	// 0
	kNotOnVertex,	// 1
	kOnVertex0,		// 2
	kOnVertex1,		// 3
	kOnVertex2,		// 4
	kOnEdge0,		// 5
	kOnEdge1,		// 6
	kOnEdge2,		// 7
	kOnTriangle		// 8
};

enum EIntersectionResult
{
	eIntersectionsFound,
	eTrianglesCoplanars,
	eIntersectionError
};

#if (VERSIONNUMBER >= 0x060000)
int32 IsPointOnVertex(const TVector3d& pos, const TVector3d triangle[] )
#else
int32 IsPointOnVertex(const TVector3& pos, const TVector3 triangle[] )
#endif
{
	// See if it's on a vertex
	if( (triangle[0]-pos).GetMagnitude() < kRealEpsilon )
		return kOnVertex0;
	if( (triangle[1]-pos).GetMagnitude() < kRealEpsilon )
		return kOnVertex1;
	if( (triangle[2]-pos).GetMagnitude() < kRealEpsilon )
		return kOnVertex2;

	return kNotOnVertex;
}

#define EPSILON 0.0001f // 1E-4 because there're float/double convertion that brings arround 1E-5 error in release

#if (VERSIONNUMBER >= 0x060000)
EIntersectionResult GetTrianglesIntersection( const TVector3d triangleA[], 
											  const TVector3d triangleB[], 
											  TVector3 intersections[],
											  int32 posOnTriangleA[],
											  int32 posOnTriangleB[])
#else
EIntersectionResult GetTrianglesIntersection( const TVector3 triangleA[], 
											  const TVector3 triangleB[], 
											  TVector3 intersections[],
											  int32 posOnTriangleA[],
											  int32 posOnTriangleB[] )
#endif
{
	/* compute plane equation of triangle(V0,V1,V2) */
	TVector3 E1 = triangleB[1] - triangleB[0];
	TVector3 E2 = triangleB[2] - triangleB[0];
	TVector3 normalB = E1 ^ E2;
	if( normalB.Normalize() == 0 )
	{
		MCNotify( "Degenerate" );
		return eIntersectionError;
	}
	const real64 dB=-(normalB*triangleB[0]);

	/* plane equation 1: normalB.X+dB=0 */

	/* put triangleA into plane equation 1 to compute signed distances to the plane*/
	// Distance to plane
	real64 dB0 = normalB*triangleA[0]+dB;
	real64 dB1 = normalB*triangleA[1]+dB;
	real64 dB2 = normalB*triangleA[2]+dB;

	/* coplanarity robustness check */
	if(fabs(dB0)<EPSILON) 
		dB0=0.0f;
	if(fabs(dB1)<EPSILON) 
		dB1=0.0f;
	if(fabs(dB2)<EPSILON) 
		dB2=0.0f;

	/* compute plane of triangle (U0,U1,U2) */
	E1 = triangleA[1] - triangleA[0];
	E2 = triangleA[2] - triangleA[0];
	TVector3 normalA = E1 ^ E2;
	if( normalA.Normalize() == 0 )
	{
		MCNotify( "Degenerate" );
		return eIntersectionError;
	}
	const real64 dA=-(normalA*triangleA[0]);
	/* plane equation 2: normalA.X+==0 */

	/* put triangleB into plane equation 2 */
	// Distance to plane
	real64 dA0 = normalA*triangleB[0]+dA;
	real64 dA1 = normalA*triangleB[1]+dA;
	real64 dA2 = normalA*triangleB[2]+dA;

	if(fabs(dA0)<EPSILON) 
		dA0=0.0;
	if(fabs(dA1)<EPSILON) 
		dA1=0.0;
	if(fabs(dA2)<EPSILON) 
		dA2=0.0;

	// Coplanar triangles case
 	if( dA0==0 && dA1==0 && dA2==0 )
	{
		// The 2 triangles are coplanar
		MY_ASSERT(dB0==0 && dB1==0 && dB2==0);

		return eTrianglesCoplanars;
	}

	TVector3 a(0,0,0), b(0,0,0);
	int32 aOnEdgeIndex = 0;
	int32 bOnEdgeIndex = 0;
	// 2 points on the plane case
	if(dB0==0&&dB1==0)
	{
		a=triangleA[0];
		b=triangleA[1];
	}
	else if(dB0==0&&dB2==0)
	{
		a=triangleA[0];
		b=triangleA[2];
	}
	else if(dB1==0&&dB2==0)
	{
		a=triangleA[1];
		b=triangleA[2];
	}
	else // Normal case
	{
		TVector3 pA0(0,0,0), pA1(0,0,0), pA2(0,0,0);
		if(dB0*dB1>0)
		{
			MY_ASSERT(dB0*dB2<=0);

			// triangleA[2] is on the other side
			pA0 = triangleA[2]; pA1 = triangleA[0]; pA2 = triangleA[1];
			// a is on 2-0, b on 1-2
			aOnEdgeIndex = kOnEdge2; bOnEdgeIndex = kOnEdge1;
		}
		else if(dB0*dB2>0)
		{
			// triangleA[1] is on the other side
			pA0 = triangleA[1]; pA1 = triangleA[0]; pA2 = triangleA[2];
			// a is on 0-1, b on 1-2
			aOnEdgeIndex = kOnEdge0; bOnEdgeIndex = kOnEdge1;
		}
		else if(dB1*dB2>0)
		{
			// triangleA[0] is on the other side
			pA0 = triangleA[0]; pA1 = triangleA[1]; pA2 = triangleA[2];
			// a is on 0-1, b on 2-0
			aOnEdgeIndex = kOnEdge0; bOnEdgeIndex = kOnEdge2;
		}
		else if(dB0==0)
		{
			// triangleA[0] is coplanar with triangle B
			pA0 = triangleA[1]; pA1 = triangleA[0]; pA2 = triangleA[2];
			// a is triangleA[0], b on the other edge
			bOnEdgeIndex = kOnEdge1;
		}
		else if(dB1==0)
		{
			// triangleA[1] is coplanar with triangle B
			pA0 = triangleA[0]; pA1 = triangleA[1]; pA2 = triangleA[2];
			// a is triangleA[1], b on the other edge
			bOnEdgeIndex = kOnEdge2;
		}
		else if(dB2==0)
		{
			// triangleA[2] is coplanar with triangle B
			pA0 = triangleA[0]; pA1 = triangleA[2]; pA2 = triangleA[1];
			// a is triangleA[2], b on the other edge
			bOnEdgeIndex = kOnEdge0;
		}
		else
		{
			MCNotify("What's this case?");
		}

		boolean result =IntersectLinePlane( pA0, // first point of the line
											pA1, // second point of the line
											triangleB[0], // point on the plane
											normalB, // normal to the plane
											a	);
		MY_ASSERT(result);
		result =IntersectLinePlane( pA0, // first point of the line
									pA2, // second point of the line
									triangleB[0], // point on the plane
									normalB, // normal to the plane
									b	);
		MY_ASSERT(result);
	}

	TVector3 c(0,0,0), d(0,0,0);
	int32 cOnEdgeIndex = 0;
	int32 dOnEdgeIndex = 0;
	// 2 points on the plane case
	if(dA0==0&&dA1==0)
	{
		c=triangleB[0];
		d=triangleB[1];
	}
	else if(dA0==0&&dA2==0)
	{
		c=triangleB[0];
		d=triangleB[2];
	}
	else if(dA1==0&&dA2==0)
	{
		c=triangleB[1];
		d=triangleB[2];
	}
	else // Normal case
	{
		TVector3 pB0(0,0,0), pB1(0,0,0), pB2(0,0,0);
		if(dA0*dA1>0) 
		{
			MY_ASSERT(dA0*dA2<=0);

			// triangleB[2] is on the other side
			pB0 = triangleB[2]; pB1 = triangleB[0]; pB2 = triangleB[1];
			// c is on 2-0, d on 1-2
			cOnEdgeIndex = kOnEdge2; dOnEdgeIndex = kOnEdge1;
		}
		else if(dA0*dA2>0)
		{
			// triangleB[1] is on the other side
			pB0 = triangleB[1]; pB1 = triangleB[0]; pB2 = triangleB[2];
			// c is on 0-1, d on 1-2
			cOnEdgeIndex = kOnEdge0; dOnEdgeIndex = kOnEdge1;
		}
		else if(dA1*dA2>0)
		{
			// triangleB[0] is on the other side
			pB0 = triangleB[0]; pB1 = triangleB[1]; pB2 = triangleB[2];
			// c is on 0-1, d on 2-0
			cOnEdgeIndex = kOnEdge0; dOnEdgeIndex = kOnEdge2;
		}
		else if(dA0==0)
		{
			// triangleB[0] is coplanar with triangle A
			pB0 = triangleB[1]; pB1 = triangleB[0]; pB2 = triangleB[2];
			// c is triangleB[0], d on the other edge
			dOnEdgeIndex = kOnEdge1;
		}
		else if(dA1==0)
		{
			// triangleB[1] is coplanar with triangle A
			pB0 = triangleB[0]; pB1 = triangleB[1]; pB2 = triangleB[2];
			// c is triangleB[1], d on the other edge
			dOnEdgeIndex = kOnEdge2;
		}
		else if(dA2==0)
		{
			// triangleB[2] is coplanar with triangle A
			pB0 = triangleB[0]; pB1 = triangleB[2]; pB2 = triangleB[1];
			// c is triangleB[2], d on the other edge
			dOnEdgeIndex = kOnEdge0;
		}
		else
		{
			MCNotify("What's this case?");
		}

		boolean result =IntersectLinePlane( pB0, // first point of the line
									pB1, // second point of the line
									triangleA[0], // point on the plane
									normalA, // normal to the plane
									c	);
		MY_ASSERT(result);
		result =IntersectLinePlane( pB0, // first point of the line
									pB2, // second point of the line
									triangleA[0], // point on the plane
									normalA, // normal to the plane
									d	);
		MY_ASSERT(result);
	}


	// keep the 2 middle points: suppress ther max and the min
	/* compute and index to the largest component of D */
	/* compute direction of intersection line */
	TVector3 D = normalA ^ normalB;
	real64 max=fabs(D[0]);
	int32 index=0;
	real64 bb=fabs(D[1]);
	real64 cc=fabs(D[2]);
	if(bb>max) max=bb,index=1;
	if(cc>max) max=cc,index=2;

	if( a[index] > MC_Max(b[index], c[index], d[index]) )
	{
		// a is out
		if( b[index] <= MC_Min(c[index], d[index]) )
		{
			// b is out
			intersections[0] = c;
			intersections[1] = d;
			posOnTriangleA[0] = kUnknownPos; posOnTriangleB[0] = cOnEdgeIndex; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
		}
		else if( c[index] <= MC_Min(b[index], d[index]) )
		{
			// c is out
			intersections[0] = b;
			intersections[1] = d;
			posOnTriangleA[0] = bOnEdgeIndex; posOnTriangleB[0] = kUnknownPos; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
		}
		else
		{
			// d is out
			intersections[0] = b;
			intersections[1] = c;
			posOnTriangleA[0] = bOnEdgeIndex; posOnTriangleB[0] = kUnknownPos; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = cOnEdgeIndex; 
		}
	}
	else if( a[index] < MC_Min(b[index], c[index], d[index]) )
	{
		// a is out
		if( b[index] >= MC_Max(c[index], d[index]) )
		{
			// b is out
			intersections[0] = c;
			intersections[1] = d;
			posOnTriangleA[0] = kUnknownPos; posOnTriangleB[0] = cOnEdgeIndex; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
		}
		else if( c[index] >= MC_Max(b[index], d[index]) )
		{
			// c is out
			intersections[0] = b;
			intersections[1] = d;
			posOnTriangleA[0] = bOnEdgeIndex; posOnTriangleB[0] = kUnknownPos; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
		}
		else
		{
			// d is out
			intersections[0] = b;
			intersections[1] = c;
			posOnTriangleA[0] = bOnEdgeIndex; posOnTriangleB[0] = kUnknownPos; 
			posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = cOnEdgeIndex; 
		}
	}
	else
	{
		// a is OK, find the 2nd point
		intersections[0] = a;
		posOnTriangleA[0] = aOnEdgeIndex; posOnTriangleB[0] = kUnknownPos; 
		if( b[index] >= MC_Max(c[index], d[index]) )
		{
			// b is out
			if(c[index] < d[index])
			{
				intersections[1] = d;
				posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
			}
			else
			{
				intersections[1] = c;
				posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = cOnEdgeIndex; 
			}
		}
		else if( b[index] <= MC_Min(c[index], d[index]) )
		{
			// b is out
			if(c[index] > d[index])
			{
				intersections[1] = d;
				posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = dOnEdgeIndex; 
			}
			else
			{
				intersections[1] = c;
				posOnTriangleA[1] = kUnknownPos; posOnTriangleB[1] = cOnEdgeIndex; 
			}
		}
		else
		{
			// b is OK
			intersections[1] = b;
			posOnTriangleA[1] = bOnEdgeIndex; posOnTriangleB[1] = kUnknownPos; 
		}

	}

	// Determine their positions on the triangles:
	int32 posOnVtx = 0;

	for( int32 inter=0 ; inter<2 ; inter++ )
	{
		TVector3 point = intersections[inter];

		// On triangle A
		posOnVtx = IsPointOnVertex( point, triangleA );
		if(posOnVtx != kNotOnVertex)
			posOnTriangleA[inter] = posOnVtx;
		else if( posOnTriangleA[inter] == kUnknownPos )
		{	
			// see if it's on an edge
			if(aOnEdgeIndex == bOnEdgeIndex && aOnEdgeIndex!=kUnknownPos)
			{
				posOnTriangleA[inter] = aOnEdgeIndex;
			}
			else if( (point-a).GetMagnitude() < EPSILON/*kRealEpsilon*/ )
			{
				posOnTriangleA[inter] = aOnEdgeIndex; // On the edge where a is
			}
			else if( (point-b).GetMagnitude() < EPSILON/*kRealEpsilon*/ )	
			{
				posOnTriangleA[inter] = bOnEdgeIndex;// On the edge where b is
			}
			else 
			{ 
				MY_ASSERT(posOnTriangleB[inter]!= kOnTriangle);	
				posOnTriangleA[inter] = kOnTriangle; 
			} // Default case: on the polygon	
		}

		// On triangle B
		posOnVtx = IsPointOnVertex( point, triangleB );
		if(posOnVtx != kNotOnVertex)
			posOnTriangleB[inter] = posOnVtx;
		else if( posOnTriangleB[inter] == kUnknownPos )
		{	
			// see if it's on an edge
			if(cOnEdgeIndex == dOnEdgeIndex && cOnEdgeIndex!=kUnknownPos)	posOnTriangleB[inter] = cOnEdgeIndex;
			else if( (point-c).GetMagnitude() < EPSILON/*kRealEpsilon*/ )	posOnTriangleB[inter] = cOnEdgeIndex; // On the edge where a is
			else if( (point-d).GetMagnitude() < EPSILON/*kRealEpsilon*/ )	posOnTriangleB[inter] = dOnEdgeIndex;// On the edge where b is
			else { MY_ASSERT(posOnTriangleA[inter]!= kOnTriangle);			posOnTriangleB[inter] = kOnTriangle; } // Default case: on the polygon			
		}
	}

	return eIntersectionsFound;
}

void CutterSegment::Init(const int32 index1, const int32 index2)
{
	fVertexIndex[0] = index1;
	fVertexIndex[1] = index2;
}

int32 CutterPolyLine::AddIntersections( const int32 cutterTgle,
								  TVector3 intersection[],
								  int32 posOnTriangleToCut[],
								  int32 posOnCutter[])
{
	// First point
	const int32 index1 = fCutterVertices.GetElemCount();
	CutterVertex& v1 = fCutterVertices.AddElem();
	v1.SetTriangleIndex(cutterTgle);
	v1.SetPosition(intersection[0]);
	// Set the part on which the vertex are
	switch( posOnTriangleToCut[0] )
	{
		case kOnVertex0: v1.SetIsOnVertex(0); break;
		case kOnVertex1: v1.SetIsOnVertex(1); break;
		case kOnVertex2: v1.SetIsOnVertex(2); break;
		case kOnEdge0: v1.SetIsOnEdge(0); break;
		case kOnEdge1: v1.SetIsOnEdge(1); break;
		case kOnEdge2: v1.SetIsOnEdge(2); break;
		case kOnTriangle: v1.SetIsInTriangle(true); MY_ASSERT(posOnCutter[0]!=kOnTriangle); break;
		default: MCNotify("Vertex nowhere!!!"); break;
	}
	switch( posOnCutter[0] )
	{
		case kOnVertex0: v1.SetIsOnCutterVertex(0); break;
		case kOnVertex1: v1.SetIsOnCutterVertex(1); break;
		case kOnVertex2: v1.SetIsOnCutterVertex(2); break;
		case kOnEdge0: v1.SetIsOnCutterEdge(0); break;
		case kOnEdge1: v1.SetIsOnCutterEdge(1); break;
		case kOnEdge2: v1.SetIsOnCutterEdge(2); break;
		case kOnTriangle: v1.SetIsInCutterTriangle(true); break;
		default: MCNotify("Vertex nowhere!!!"); break;
	}

	// Second point
	const int32 index2 = fCutterVertices.GetElemCount();
	CutterVertex& v2 = fCutterVertices.AddElem();
	v2.SetTriangleIndex(cutterTgle);
	v2.SetPosition(intersection[1]);
	// Set the part on which the vertex are
	switch( posOnTriangleToCut[1] )
	{
		case kOnVertex0: v2.SetIsOnVertex(0); break;
		case kOnVertex1: v2.SetIsOnVertex(1); break;
		case kOnVertex2: v2.SetIsOnVertex(2); break;
		case kOnEdge0: v2.SetIsOnEdge(0); break;
		case kOnEdge1: v2.SetIsOnEdge(1); break;
		case kOnEdge2: v2.SetIsOnEdge(2); break;
		case kOnTriangle: v2.SetIsInTriangle(true); MY_ASSERT(posOnCutter[1]!=kOnTriangle); break;
		default: MCNotify("Vertex nowhere!!!"); break;
	}
	switch( posOnCutter[1] )
	{
		case kOnVertex0: v2.SetIsOnCutterVertex(0); break;
		case kOnVertex1: v2.SetIsOnCutterVertex(1); break;
		case kOnVertex2: v2.SetIsOnCutterVertex(2); break;
		case kOnEdge0: v2.SetIsOnCutterEdge(0); break;
		case kOnEdge1: v2.SetIsOnCutterEdge(1); break;
		case kOnEdge2: v2.SetIsOnCutterEdge(2); break;
		case kOnTriangle: v2.SetIsInCutterTriangle(true); break;
		default: MCNotify("Vertex nowhere!!!"); break;
	}

	// Segment
	const int32 segmentIndex = fCutterEdges.GetElemCount();
	CutterSegment& segment = fCutterEdges.AddElem();

	segment.Init(index1,index2);

	return segmentIndex;
}

void CutterPolyLine::Check()
{
	const int32 vtxCount = fCutterVertices.GetElemCount();

	int32 onEdgeCount = 0;
	boolean isOnVertex=false;

	for( int32 iVtx = 0 ; iVtx < vtxCount ; iVtx++ )
	{
		CutterVertex& vertex = fCutterVertices[iVtx];
		if( vertex.GetIsUsed() )
		{
			const int32 onEdge = vertex.IsOnEdge();
			if(onEdge>=0)
				onEdgeCount++;
			const int32 onVertex = vertex.IsOnVertex();
			if(onVertex>=0)
				isOnVertex = true;
		}
	}

	if(!isOnVertex)
	{
		if(onEdgeCount<2)
		{
			MCNotify("Missing points");
		}
	}
}
// Weld the segment on the cutter side ( there's nothing to weld on the triangle side, 
// we work with only one triangle )
void CutterPolyLine::WeldSegments()
{
#if 1
	// We can weld using the distance and a precision, we're not going to rebuild a perfect mesh

	const real64 precision = .001f;

	const int32 vtxCount = fCutterVertices.GetElemCount();

	TMCArray<int32> prevIndex;
	prevIndex.SetElemCount(vtxCount);

	int32 iVtx = 0;
	for( iVtx = 0 ; iVtx < vtxCount ; iVtx++ )
	{
		prevIndex[iVtx] = iVtx;
	}

	for( iVtx = 0 ; iVtx < vtxCount ; iVtx++ )
	{
		CutterVertex& vertex = fCutterVertices[iVtx];
		if( vertex.GetIsUsed() ) // this vertex was not set as useless yet
		{
			const TVector3 vertexPos = vertex.GetPosition();

			for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
			{
				CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
				if( otherVertex.GetIsUsed() &&
					otherVertex.GetPosition().IsEqual(vertexPos, precision) )
				{	// These 2 vertices are at the same place, keep the more useful one (on edges)
					if( otherVertex.IsOnVertex()>=0 )
					{
						vertex.SetIsOnEdge(otherVertex.IsOnVertex());
					}
					else if( otherVertex.IsOnEdge()>=0 )
					{
						vertex.SetIsOnEdge(otherVertex.IsOnEdge());
					}
					prevIndex[iOtherVtx] = iVtx;
					otherVertex.SetIsUsed(false);
				}
			}
		}
	}


	// Replace the vertices references in the edges
	const int32 edgeCount = fCutterEdges.GetElemCount();
	for ( int32 iEdge=0; iEdge<edgeCount ; iEdge++ )
	{
		CutterSegment& edge = fCutterEdges[iEdge];
		if ( edge.GetIsUsed() )
		{
			// Extract the 2 vertices of seg
			const int32 vertexIndex0 = prevIndex[fCutterEdges[iEdge].GetVertexIndex(0)];
			const int32 vertexIndex1 = prevIndex[fCutterEdges[iEdge].GetVertexIndex(1)];
			if ( vertexIndex0 != vertexIndex1 )
			{
				fCutterEdges[iEdge].SetVertexIndex(0,vertexIndex0);
				fCutterVertices[vertexIndex0].SetIsUsed(true); // to be safe

				fCutterEdges[iEdge].SetVertexIndex(1,vertexIndex1);
				fCutterVertices[vertexIndex1].SetIsUsed(true); // to be safe
			}
			else
				fCutterEdges[iEdge].SetIsUsed(false);
		}
	}
#else

// I don't think this will work for one triangle cut at different places
// we should store a unique index for the edges of the cutter and its triangles ?

	// Find all the duplicated points of the polyline
	const int32 vtxCount = fCutterVertices.GetElemCount();

	TMCArray<int32> prevIndex;
	prevIndex.SetElemCount(vtxCount);

	int32 iVtx = 0;
	for( iVtx = 0 ; iVtx < vtxCount ; iVtx++ )
	{
		prevIndex[iVtx] = iVtx;
	}

	for( iVtx = 0 ; iVtx < vtxCount ; iVtx++ )
	{
		CutterVertex& vertex = fCutterVertices[iVtx];
		if( vertex.GetIsUsed() ) // this vertex was not set as useless yet
		{
			// Most of the polyline vertices are on edges
			const int32 cutterEdge = vertex.IsOnCutterEdge();
			if(cutterEdge>=0)
			{	// Find the others polyline vertices on this edge and check if they've got the 
				// same other edge or polygon or vertex
				const boolean inTriangle = vertex.IsInTriangle();
				if(inTriangle)
				{
					for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
					{
						CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
						if( otherVertex.GetIsUsed() &&
							otherVertex.IsOnCutterEdge() == cutterEdge )
						{	// we found it
							prevIndex[iOtherVtx] = iVtx;
							otherVertex.SetIsUsed(false);
						}
					}
				}
				else
				{
					const int32 onEdge = vertex.IsOnEdge();
					if(onEdge>=0)
					{
						for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
						{
							CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
							if( otherVertex.GetIsUsed() &&
								otherVertex.IsOnCutterEdge() == cutterEdge)
							{	// we found it
								prevIndex[iOtherVtx] = iVtx;
								otherVertex.SetIsUsed(false);
							}
						}
					}
					else
					{
						const int32 onVertex = vertex.IsOnVertex();
						if(onVertex>=0)
						{
							for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
							{
								CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
								if( otherVertex.GetIsUsed() &&
									otherVertex.IsOnCutterEdge() == cutterEdge )
								{	// we found it
									prevIndex[iOtherVtx] = iVtx;
									otherVertex.SetIsUsed(false);
								}
							}
						}
					}
				}
			}
			else
			{	// rare: the polyline vertex can be on a cutter vertex
				const int32 cutterVertex = vertex.IsOnCutterVertex();
				if(cutterVertex>=0)
				{
					const boolean inTriangle = vertex.IsInTriangle();
					if(inTriangle)
					{
						for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
						{
							CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
							if( otherVertex.GetIsUsed() &&
								otherVertex.IsOnCutterVertex() == cutterVertex )
							{	// we found it
								prevIndex[iOtherVtx] = iVtx;
								otherVertex.SetIsUsed(false);
							}
						}
					}
					else
					{
						const int32 onEdge = vertex.IsOnEdge();
						if(onEdge>=0)
						{
							for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
							{
								CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
								if( otherVertex.GetIsUsed() &&
									otherVertex.IsOnCutterVertex() == cutterVertex)
								{	// we found it
									prevIndex[iOtherVtx] = iVtx;
									otherVertex.SetIsUsed(false);
								}
							}
						}
						else
						{
							const int32 onVertex = vertex.IsOnVertex();
							if(onVertex>=0)
							{
								for( int32 iOtherVtx=iVtx+1 ; iOtherVtx<vtxCount ; iOtherVtx++ )
								{
									CutterVertex& otherVertex = fCutterVertices[iOtherVtx];
									if( otherVertex.GetIsUsed() &&
										otherVertex.IsOnCutterVertex() == cutterVertex )
									{	// we found it
										prevIndex[iOtherVtx] = iVtx;
										otherVertex.SetIsUsed(false);
									}
								}
							}
						}
					}
				}
			}
			// Otherwise the cutter vertex is inside the triangle, but in that case we don't have anything to weld
		}
	}

	// Replace the vertices references in the edges
	const int32 edgeCount = fCutterEdges.GetElemCount();
	for ( int32 iEdge=0; iEdge<edgeCount ; iEdge++ )
	{
		CutterSegment& edge = fCutterEdges[iEdge];
		if ( edge.GetIsUsed() )
		{
			// Extract the 2 vertices of seg
			const int32 vertexIndex0 = prevIndex[fCutterEdges[iEdge].GetVertexIndex(0)];
			const int32 vertexIndex1 = prevIndex[fCutterEdges[iEdge].GetVertexIndex(1)];
			if ( vertexIndex0 != vertexIndex1 )
			{
				fCutterEdges[iEdge].SetVertexIndex(0,vertexIndex0);
				fCutterVertices[vertexIndex0].SetIsUsed(true); // to be safe

				fCutterEdges[iEdge].SetVertexIndex(1,vertexIndex1);
				fCutterVertices[vertexIndex1].SetIsUsed(true); // to be safe
			}
			else
				fCutterEdges[iEdge].SetIsUsed(false);
		}
	}
#endif
}

// Barycentric (or parametric) coordinates : 
// from http://softsurfer.com/Archive/algorithm_0104/algorithm_0104.htm
TVector2 GetUVFromXY(	const TVector2& UV0,
						const TVector2& UV1,
						const TVector2& UV2,
						const real32 denom,	// denom = vx*uy - vy*ux
						const TVector2& u,	// u = V1-V0
						const TVector2& v,	// v = V2-V0
						const TVector2& w)	// w = P -V0
{
	const real64 s = (w[1]*v[0] - w[0]*v[1])/denom;
	const real64 t = (w[0]*u[1] - w[1]*u[0])/denom;

	// Barycentric coordinates are then (1-s-t,s,t)
	return ( (1-s-t)*UV0 + s*UV1 + t*UV2 );
}

//
// project a vertex on the plane defined by (center,I,J)
//
inline DelauneyCopy::Vertex2D Project(const TVector3& position,
								  const TVector3& center,const TVector3& I,const TVector3& J,
								  DelauneyCopy::Vertex2D::eLocation location)
{
	TVector3 p = position;
	p-=center;
	return DelauneyCopy::Vertex2D( I*p, J*p,location);
}

boolean PosOverGeom(const TVector3& pos, const TMCArray<TriangleVertices>& cutterTriangles, const real32 roofMin, const real32 roofMax)
{
	if(pos.z<roofMin)
		return false;

	const int32 tglCount = cutterTriangles.GetElemCount();

	TVector3 hitPosition;
	real64 alpha=0;
	for(int32 iTgle=0 ; iTgle<tglCount ; iTgle++)
	{		
		const TriangleVertices& vtx = cutterTriangles[iTgle];

		if( BasicTrianglePick( vtx.fVertices[0], vtx.fVertices[1], vtx.fVertices[2], alpha, pos, -TVector3::kUnitZ, hitPosition ) ) 
			return true;
	}

	return false;
}

boolean Triangulate( const Vertex tglToCut[],
					 const Triangle& triangleToCutIndexes,
					 const CutterPolyLine& polyline,
					 const int32 offsetIndex,
				 TMCClassArray<Triangle>& newTriangles, // Add the triangles to this list
				 TMCClassArray<Vertex>& newVertices,	// Add the missing vertices here
				 const TVector3& triangleNormal,
				 const TMCArray<TriangleVertices>& cutterTriangles, 
				 const real32 roofMin, const real32 roofMax ) // To know which triangles we keep
{
	TVector3 triangleToCut[3];
	triangleToCut[0] = tglToCut[0].Position();
	triangleToCut[1] = tglToCut[1].Position();
	triangleToCut[2] = tglToCut[2].Position();

	// Get the plane countaining the triangle
	TVector3 I,J,K,center;
	center = TVector3::kZero; // Good enough for the use
	K = triangleNormal;
	I = triangleToCut[1] - triangleToCut[0];
	if( !I.Normalize() )
		return false;
	J = K^I; // To have an ortho base

	// For the computation of the UV
	const TVector2& UV0 = tglToCut[0].UV();
	const TVector2& UV1 = tglToCut[1].UV();
	const TVector2& UV2 = tglToCut[2].UV();
	const TVector2 V0 = ProjectIn(triangleToCut[0],triangleToCut[0],I,J);
	const TVector2 u = ProjectIn(triangleToCut[1],triangleToCut[0],I,J) - V0;
	const TVector2 v = ProjectIn(triangleToCut[2],triangleToCut[0],I,J) - V0;
	const real64 denom = v[0]*u[1] - v[1]*u[0]; // vx*uy - vy*ux
	{
		// we project all the vertices on the triangle
		TMCArray<DelauneyCopy::Vertex2D> vertices2D;

		const TMCClassArray<CutterVertex>& cutterVtx = polyline.GetCutterVertices();
		const int32 cutterVertexCount = cutterVtx.GetElemCount();

		TMCClassArray<TVector3> recordedPositions;
		recordedPositions.SetElemSpace(cutterVertexCount+3); // More than we need

		// Add first the 3 vertices of the triangle
		for(int32 iPt=0 ; iPt<3 ; iPt++)
		{
			const TVector3& pos = triangleToCut[iPt];
			recordedPositions.AddElem(pos);
			vertices2D.AddElem(Project(pos,center,I,J,DelauneyCopy::Vertex2D::kOnEdge));
		}

		// Then add the polyline vertices

		TMCArray<int32> recordedIndexes(cutterVertexCount,false);
		int32 index = 3; // Keep track of the indexes
		for (int32 vertexIndex=0;vertexIndex<cutterVertexCount;vertexIndex++)
		{
			const CutterVertex& polylineVertex = cutterVtx[ vertexIndex ];

			if(!polylineVertex.GetIsUsed())
				continue;

			const TVector3& position = polylineVertex.GetPosition();
			
			recordedIndexes[vertexIndex] = index++;
			recordedPositions.AddElem(position);

			if (polylineVertex.IsInTriangle())
			{
				vertices2D.AddElem(Project(position,center,I,J,DelauneyCopy::Vertex2D::kInside));
			}
			else if (polylineVertex.IsOnEdge()>=0)
			{
				vertices2D.AddElem(Project(position,center,I,J,DelauneyCopy::Vertex2D::kOnEdge));
			}
			else
			{
				if(MCVerify( polylineVertex.IsOnVertex()>=0 ))
				{	
					vertices2D.AddElem(Project(position,center,I,J,DelauneyCopy::Vertex2D::kOnEdge));
				}
			}
		}

		// now triangulate the triangle
		// build triangulation
		DelauneyCopy::Triangulator2D triangulator(vertices2D);
		 // builds Delauney Triangulation of the triangle
		triangulator.Triangulate();

		// add constraints using the edges of the polyline
		const TMCClassArray<CutterSegment>& cutterSgt = polyline.GetCutterSegments();

		const int32 cutterSgtCount = cutterSgt.GetElemCount();

		for (int32 sgtIndex=0;sgtIndex<cutterSgtCount;sgtIndex++)
		{
			const CutterSegment& polylineSegment = cutterSgt[ sgtIndex ];

			if ( !polylineSegment.GetIsUsed() )
				continue;

			const int32 vertexIndex1 = polylineSegment.GetVertexIndex(0);
			const int32 vertexIndex2 = polylineSegment.GetVertexIndex(1);

			if (vertexIndex1>=0 && vertexIndex2>=0)
			{
				const CutterVertex& vertex1 = cutterVtx[vertexIndex1];
				const CutterVertex& vertex2 = cutterVtx[vertexIndex2];
			
				if ( !vertex1.GetIsUsed() || !vertex2.GetIsUsed() )
					continue;
				
				DelauneyCopy::Vertex2DIndex v1,v2;

				v1.SetValue(recordedIndexes[vertexIndex1]);
				v2.SetValue(recordedIndexes[vertexIndex2]);


				triangulator.AddConstraint(v1,v2);
			}
		}

		// delete the triangle and build the new triangulation
		const TMCPtrArray<const DelauneyCopy::Triangle2D>& triangles = *triangulator.GetTriangulation();

		MY_ASSERT(&triangles != NULL);
		MY_ASSERT(triangles.GetElemCount() > 0);

		// add new triangles if possible
		const int32 subTriangleCount = triangles.GetElemCount();

		for (int32 triangleIndex=0;triangleIndex<subTriangleCount;triangleIndex++)
		{
			// The index 1, 2 and 3 are the one from the original triangle

			// the positions are stored in recordedPosition

			const DelauneyCopy::Triangle2D& subTgle = *triangles[triangleIndex];

			// Add the vertices to the list, then the triangles
			int32 vtx[3]; 
			TVector3 position[3];
			{	// Get the center of the triangle and check if we're over or under the roof
				TVector3 tglCenter = TVector3::kZero;
				for(int32 i=0 ; i<3 ; i++)
				{
					vtx[i] = subTgle.fVertexIndex[i].GetValue();
					position[i] = recordedPositions[vtx[i]];
					tglCenter+=position[i];
				}
				tglCenter/=3;
				if(PosOverGeom(tglCenter, cutterTriangles,roofMin,roofMax))
				{
					continue;
				}
			}
			{	// Add the trianfgle
				for(int32 i=0 ; i<3 ; i++)
				{
					if(vtx[i]<3)
					{	// Use the indexes we already know
						vtx[i] = triangleToCutIndexes[vtx[i]];
					}
					else
					{	// Add a new vertex
						const TVector3& pos = position[i];
						const TVector2 w = ProjectIn(pos,triangleToCut[0],I,J) - V0;
						const TVector2 UV = GetUVFromXY( UV0, UV1, UV2, denom, u, v, w);
						Vertex newVtx(pos,K,UV);
						vtx[i] = newVertices.GetElemCount() + offsetIndex;
						newVertices.AddElem(newVtx);
					}
				}
				Triangle newTgle(vtx[0], vtx[1], vtx[2]);
				newTriangles.AddElem(newTgle);
			}
		}
	}

	return true;
}

void CutTriangle(const Triangle& triangle, const TMCClassArray<Vertex>& vertices,
				 const int32 offsetIndex,
				 TMCClassArray<Triangle>& newTriangles, 
				 TMCClassArray<Vertex>& newVertices,
				 const TMCArray<TriangleVertices>& cutterTriangles,
				 const real32 roofMin, const real32 roofMax)
{
	const int32 cutterCount = cutterTriangles.GetElemCount();

	Vertex tglToCut[3];
	tglToCut[0] = vertices[triangle.pt1];
	tglToCut[1] = vertices[triangle.pt2];
	tglToCut[2] = vertices[triangle.pt3];

#if (VERSIONNUMBER >= 0x060000)
	TVector3d triangleToCut[3];
#else
	TVector3 triangleToCut[3];
#endif
	triangleToCut[0] = tglToCut[0].Position();
	triangleToCut[1] = tglToCut[1].Position();
	triangleToCut[2] = tglToCut[2].Position();

	TBBox3D tgleToCutBbox(triangleToCut[0],triangleToCut[0]);
	tgleToCutBbox.AddPoint(triangleToCut[1]);
	tgleToCutBbox.AddPoint(triangleToCut[2]);

	const TVector3& triangleNormal = vertices[triangle.pt1].Normal();

	boolean isCut = false;

	CutterPolyLine polyline;

	for(int32 iCut=0 ; iCut<cutterCount ; iCut++)
	{
		const TriangleVertices& cutterTriangle = cutterTriangles[iCut];

		// First test the bbox intersection
		TBBox3D tgleBbox(cutterTriangle.fVertices[0],cutterTriangle.fVertices[0]);
		tgleBbox.AddPoint(cutterTriangle.fVertices[1]);
		tgleBbox.AddPoint(cutterTriangle.fVertices[2]);

		if(BBoxIntersect(tgleBbox,tgleToCutBbox))
		{
			// Then precise intersection test
			if( IntersectTriangles(triangleToCut, cutterTriangle.fVertices) )
			{
				// Intersection exist, get it
		
				TVector3 intersection[2] = {TVector3::kZero,TVector3::kZero};
				int32 posOnTriangleToCut[2] = {0,0};
				int32 posOnCutter[2] = {0,0};


				EIntersectionResult result = GetTrianglesIntersection(triangleToCut, cutterTriangle.fVertices, 
					intersection, posOnTriangleToCut, posOnCutter);

				if( result == eIntersectionsFound )
				{
					// Add the intersection to the polyline

					const int32 index = polyline.AddIntersections(iCut, intersection,posOnTriangleToCut,posOnCutter);
				}
				else
					MCNotify("Are triangles coplanar ?");
			}
		}
	}



	if(polyline.GetSegmentsCount())
	{	// Do the cutting of the triangle
		polyline.WeldSegments();

		Triangulate(tglToCut, triangle, polyline, offsetIndex, newTriangles, newVertices, triangleNormal, cutterTriangles, roofMin, roofMax);
	}
	else
	{
#if (VERSIONNUMBER >= 0x060000)
		TVector3d tglCenter = TVector3d::kZero;
#else
		TVector3 tglCenter = TVector3::kZero;
#endif
		for(int32 i=0 ; i<3 ; i++)
			tglCenter+=triangleToCut[i];
		tglCenter/=3;
		if(!PosOverGeom(tglCenter, cutterTriangles, roofMin, roofMax))
		{
			if(tglCenter.z>roofMax)
			{	// Security: check another point, it can happens that the center is just at the limit of 2 triangles
				for(int32 i=0 ; i<3 ; i++)
				{
					if( tglCenter!=triangleToCut[i] )
						if( PosOverGeom(.5*(tglCenter+triangleToCut[i]), cutterTriangles, roofMin, roofMax) )
						{
							return;
						}
				}
			}
			// Keep the triangle as it was
			newTriangles.AddElem(triangle);
		}
	}
}


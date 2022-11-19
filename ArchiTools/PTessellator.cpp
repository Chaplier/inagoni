/****************************************************************************************************

		PTessellator.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/5/2004

****************************************************************************************************/

#include <iostream>
#include <fstream>

#include "PTessellator.h"



// Triangulate a PolygonIndexes
class FlatPolygonTessellator
{
public:
	FlatPolygonTessellator(TMCClassArray<FlatPoint>& polygon);

	void Tessellate(TMCClassArray<Triangle>& result);
protected:
	void Tessellate(const TMCClassArray<FlatPoint>& points, 
						TMCClassArray<Triangle>& result);

	boolean TessellateSpan(const TMCClassArray<FlatPoint>& points,
						TMCClassArray<Triangle>& result,
						const boolean safeMode);

	boolean TessellateSpanOneTriangle(const TMCClassArray<FlatPoint>& points,
						TMCClassArray<Triangle>& result,
						const boolean safeMode );

	void SubTessellate(const TMCClassArray<FlatPoint>& points, 
						const int32 i1, const int32 i2,
						TMCClassArray<Triangle>& result);

	boolean ValidSplit(const TMCClassArray<FlatPoint>& points, 
						const int32 i1, const int32 i2, 
						const boolean safeMode);

	TMCClassArray<FlatPoint> fPolygon;
};



boolean FlatPolygonMaker::AddPointToCountainer(const TVector2& point, 
											   const int32 index, 
											   const boolean withCaution,
											   const boolean lastPoint)
{
	const int32 pointIndex = fPointList.GetElemCount();
	
	if(withCaution)
	{
		// This won't check everything!
		// Check that we're not adding a point over another one
		if(pointIndex>0)
		{
			if(lastPoint)
			{	// Check if it's not the same as the 1st one
				if( fPointList[0].f2DPoint == point )
				{
					// Also check the previous one
					if(pointIndex>1)
					{
						if( fPointList[pointIndex-1].f2DPoint == point )
						{	// Remove it
							fPointList.RemoveElem(pointIndex-1, 1);
							fCountainer.fPoints.RemoveElem(pointIndex-1, 1);
						}
					}
					return false;
				}
			}

			if( fPointList[pointIndex-1].f2DPoint == point )
				return false;
		}
/*		if(pointIndex>1)
		{
			if( fPointList[pointIndex-2].f2DPoint == point )
			{	// Dead branch
				fPointList.RemoveElem(pointIndex-1, 1);
				fCountainer.fPoints.RemoveElem(pointIndex-1, 1);
				return false;
			}
		}*/
	}

	FlatPoint flatPoint;
	flatPoint.f2DPoint = point;
	flatPoint.fIndex =index;
	flatPoint.fOnHole = -1; // on comtainer
	fPointList.AddElem(flatPoint);

	fCountainer.fPoints.AddElem(pointIndex);

	return true;
}

// IMPORTANT : add the points before adding the PolyHole (for fOnHole index)
void FlatPolygonMaker::AddPoint(const TVector2& point, const int32 index)
{
	FlatPoint flatPoint;
	flatPoint.f2DPoint = point;
	flatPoint.fIndex =index;
	flatPoint.fOnHole = fPolyHoles.GetElemCount();
	fPointList.AddElem(flatPoint);
}

// IMPORTANT : add the points before adding the PolyHole (for fOnHole index)
void FlatPolygonMaker::AddPolyHole(TMCArray<int32>& points)
{
	PolygonIndexes polyHole;
	polyHole.fPoints = points;

	fPolyHoles.AddElem(polyHole);
}

boolean Get2Intersection(const PosArray& hole0, const PosArray& hole1,
						 int32& ptOut0A, int32& ptOut0B, int32& ptOut0C, int32& ptOut0D, TVector2& intersectionPt0,
						 int32& ptOut1A, int32& ptOut1B, int32& ptOut1C, int32& ptOut1D, TVector2& intersectionPt1)
{
	const int32 pointCount = hole0.GetElemCount();

	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		const int32 iNextPoint = (iPoint+1)%pointCount;

		const TVector2& point0 = hole0[iPoint];
		const TVector2& point1 = hole0[iNextPoint];

		if(SegmentIntersectPolygon(point0, point1, hole1, false, 
			ptOut0A, ptOut0B, intersectionPt0))
		{
			ptOut0C = iPoint;
			ptOut0D = iNextPoint;
			// We have a first intersection, find a second one
			for(int32 iPt=pointCount-1 ; iPt>=iPoint ; iPt--)
			{
				const int32 iPrevPt = (iPt-1+pointCount)%pointCount;

				const TVector2& pt0 = hole0[iPrevPt];
				const TVector2& pt1 = hole0[iPt];

				if(SegmentIntersectPolygon(pt0, pt1, hole1, true, 
					ptOut1A, ptOut1B, intersectionPt1))
				{
					ptOut1C = iPrevPt;
					ptOut1D = iPt;
					// Check that we didn't find twice the same point
					if( ptOut0A == ptOut1A &&
						ptOut0C == ptOut1C )
						return false;

					return true;
				}
			}

			return false; // couldn't find a second point
		}
	}

	return false; // No intersection
}

void FlatPolygonMaker::MergeHoles()
{
	TMCClassArray<PolygonIndexes> newPolyHoles;

	int32 holeCount = fPolyHoles.GetElemCount();
	newPolyHoles.SetElemSpace(holeCount);

	for( int32 iHole=0 ; iHole<holeCount ; iHole++ )
	{
		PosArray& hole = fCachedHolePos[iHole];

		// 
		for( int32 iOtherHole=iHole+1 ; iOtherHole<holeCount ; iOtherHole++ )
		{
			// First intersection
			int32 ptOut0A=0, ptOut0B=0, ptOut0C=0, ptOut0D=0;
			TVector2 intersectionPt0;
			// Second intersection
			int32 ptOut1A=0, ptOut1B=0, ptOut1C=0, ptOut1D=0;
			TVector2 intersectionPt1;

			if( Get2Intersection(hole, fCachedHolePos[iOtherHole],
				ptOut0A, ptOut0B, ptOut0C, ptOut0D, intersectionPt0,
				ptOut1A, ptOut1B, ptOut1C, ptOut1D, intersectionPt1) )
			{
				// We have 2 colliding holes, and we know the intersection points and
				// the index of the vertices points of the colliding segments.
				// We can now merge these 2 holes together

				// Make a new polygon starting from the intersection point
				// Add the 2 new points
		//		int32 index = Index in the facetMesh: ils n'existent pas encore !!
		//		AddPoint(const TVector2& point, const int32 index);

				PolygonIndexes newPolygon;

				// Remove the 2 old ones
			
				// Need to modified all the data:
				// fCachedHolePos
				// fPointList
				// fEdgeList
				// fPolyHoles

				MY_ASSERT(fCachedHolePos.GetElemCount() == fPolyHoles.GetElemCount());
				// Now holeCount is changed
				// holeCount--;
				// Our polygon is now much bigger, restart the intesection loop to check all
				// the remaining ones
				// iOtherHole=iHole;
			}
		}
	}	
}

void FlatPolygonMaker::PrepareCachedData()
{
	const int32 holeCount = fPolyHoles.GetElemCount();
	const int32 countainerCount = fCountainer.fPoints.GetElemCount();
	
	fEdgeList.SetElemSpace( countainerCount + 4*holeCount ); // An approximate value: *4 if the holes are squares
	// Build an array of the existing edges
	for( int32 i=0 ; i<countainerCount ; i++ )
	{
		FlatEdge newEdge;
		newEdge.fPoint1 = fCountainer.fPoints[i];
		newEdge.fPoint2 = fCountainer.fPoints[(i+1)%countainerCount];
		newEdge.fUsed = 1;
		const int32 index = fEdgeList.GetElemCount();
		fEdgeList.AddElem(newEdge);
		fPointList[newEdge.fPoint1].fEdges.AddElem(index);
		fPointList[newEdge.fPoint2].fEdges.AddElem(index);
	}

	fCachedHolePos.SetElemCount(holeCount);
	for( int32 iHole=0 ; iHole<holeCount ; iHole++ )
	{
		PolygonIndexes& hole = fPolyHoles[iHole];
		const int32 edgeCount = hole.fPoints.GetElemCount();
	
		// prepare data for intersection
		fCachedHolePos[iHole].SetElemCount(edgeCount);

		for( int32 iEdge=0 ; iEdge<edgeCount ; iEdge++)
		{
			fCachedHolePos[iHole].SetElem(iEdge, fPointList[hole.fPoints[iEdge]].f2DPoint);

			FlatEdge newEdge;
			newEdge.fPoint1 = hole.fPoints[iEdge];
			newEdge.fPoint2 = hole.fPoints[(iEdge+1)%edgeCount];
			newEdge.fUsed = 1;
			const int32 index = fEdgeList.GetElemCount();
			fEdgeList.AddElem(newEdge);
			fPointList[newEdge.fPoint1].fEdges.AddElem(index);
			fPointList[newEdge.fPoint2].fEdges.AddElem(index);
		}

	}
}

void FlatPolygonMaker::TryToAddEdge(const int32 thisPointIndex, const bool clockwiseOrder, FlatPoint& thisPoint, FlatPoint& prevPoint, FlatPoint& nextPoint)
{
	// Add an edge on this one

	FlatEdge newEdge;
	newEdge.fPoint1 = thisPointIndex;
	// Get the nearest point not hidden by the square
	newEdge.fPoint2 = GetNearestPoint(	thisPoint, 
										prevPoint, 
										nextPoint, 
										fPointList,
										clockwiseOrder,
										fCachedHolePos);
	if(newEdge.fPoint2>=0)
	{
		newEdge.fUsed = 0;
		const int32 index = fEdgeList.GetElemCount();
		fEdgeList.AddElem(newEdge);
		fPointList[newEdge.fPoint1].fEdges.AddElem(index);
		fPointList[newEdge.fPoint2].fEdges.AddElem(index);
	}
}

void FlatPolygonMaker::LinkCountainerWithHoles()
{
	const int32 countainerCount = fCountainer.fPoints.GetElemCount();

	for( int32 iArround=0 ; iArround<countainerCount ; iArround++ )
	{
		FlatPoint& thisPoint = fPointList[iArround];
		FlatPoint& prevPoint = fPointList[(countainerCount+iArround-1)%countainerCount];
		FlatPoint& nextPoint = fPointList[(iArround+1)%countainerCount];

		// true: not really conterclock wise, but working inside the countqiner, instead
		// of outside a hole => change this flag so the right verifications are done later.
		TryToAddEdge( iArround, true, thisPoint, prevPoint, nextPoint);
	}
}

void FlatPolygonMaker::MakePolygons()
{
	// Build the polygons making holes in the countainer
	fPolygons.ArrayFree();

	const int32 holeCount = fPolyHoles.GetElemCount();
	const int32 countainerCount = fCountainer.fPoints.GetElemCount();

	if(countainerCount<3)
		return;

	PrepareCachedData();

	// 1. Merge the holes that are inside each other
// We can't do this for now: there's a relationship
// between the tesselated mesh and the FacetMesh: we can't
// add new vertices during the tesselation
//	MergeHoles();

	// 2. Rebuild the outside edge
// We can't do this for now: there's a relationship
// between the tesselated mesh and the FacetMesh: we can't
// add new vertices during the tesselation

	// Add new edges: link the countainer points to the holes
	LinkCountainerWithHoles();

	// Add new edges: link the holes
	for( int32 jHole=0 ; jHole<holeCount ; jHole++ )
	{
		PolygonIndexes& hole = fPolyHoles[jHole];
		const int32 ptCount = hole.fPoints.GetElemCount();
		const int32 lastPt = ptCount-1;

		for( int32 iPt=0 ; iPt<ptCount ; iPt++)
		{
			const int32 thisPointIndex = hole.fPoints[iPt];
			FlatPoint& thisPoint = fPointList[thisPointIndex];
			if( thisPoint.fEdges.GetElemCount() == 2 )
			{	
				FlatPoint& prevPoint = fPointList[hole.fPoints[iPt-1<0?lastPt:iPt-1]];
				FlatPoint& nextPoint = fPointList[hole.fPoints[iPt+1>lastPt?0:iPt+1]];

				TryToAddEdge(thisPointIndex, false, thisPoint, prevPoint, nextPoint);

			}
		}
	}


	// Extract the new polygons
	const int32 edgeCount = fEdgeList.GetElemCount();

	for( int32 iEdge=0 ; iEdge<edgeCount ; iEdge++ )
	{
		FlatEdge* edge = &fEdgeList[iEdge];

		if(edge->fUsed<2)
		{
			PolygonIndexes polygon1;
			if(MakePolygon(edge, edge->fPoint2, polygon1))
			{
				fPolygons.AddElem(polygon1);
			}
			if(edge->fUsed<2)
			{
				PolygonIndexes polygon2;
				if(MakePolygon(edge, edge->fPoint1, polygon2))
				{
					fPolygons.AddElem(polygon2);
				}
			}
		}
	}

}

// Get the nearest point but using the 2 others to hide a part of the space
int32 FlatPolygonMaker::GetNearestPoint( FlatPoint& thisPoint,
						    FlatPoint& prevPoint, 
						    FlatPoint& nextPoint, 
						    TMCClassArray<FlatPoint>& inList,
							const bool clockWise,
							TMCClassArray<PosArray>& holes)
{
	const TVector2 thisPos = thisPoint.f2DPoint;
	const TVector2 limit1 = prevPoint.f2DPoint - thisPos;
	const TVector2 limit2 = thisPos - nextPoint.f2DPoint;
	const real sign = clockWise?-1:1;

	const bool isConvex = (sign*(limit1^limit2)>0)?true:false;

	const int32 pointCount = inList.GetElemCount();

	TMCArray<int32> forbidenList; // for the concave countainer

	real32 minDist = kBigRealValue;
	
	int32 nearestPoint = -1;
	for( int32 iPt=0 ; iPt<pointCount ; iPt++ )
	{
		FlatPoint& point = inList[iPt];
//		if( point.fIndex == prevPoint.fIndex || point.fIndex == nextPoint.fIndex)
//			continue;
		if( point.fOnHole == thisPoint.fOnHole )
			continue;

		if(kUnusedIndex == forbidenList.FindElem(point.fIndex))
		{	// Check this point
			TVector2 vect = point.f2DPoint - thisPos;
			boolean canCheck=false;
			if(isConvex)
				canCheck = ( sign*(vect^limit1)<0 || sign*(vect^limit2)<0 );
			else
				canCheck = ( sign*(vect^limit1)<0 && sign*(vect^limit2)<0 );

			if(canCheck)
			{	// Not in the shadow, get the distance
				const real32 dist = vect.GetMagnitudeSquared();
				if(dist<minDist)
				{
					// See if it's not in a concavity
/* no needs, we check edge intesections
					const int32 countainerCount = fCountainer.fPoints.GetElemCount();
					boolean OK=true;
					for( int32 iCount=1 ; iCount<countainerCount ; iCount++ )
					{
						const int32 i1 =fCountainer.fPoints[iCount-1];
						if(i1 == iPt) continue;
						const int32 i2 =fCountainer.fPoints[iCount];
						if(i2 == iPt) continue;
						TVector2 p1 = fPointList[i1].f2DPoint;
						TVector2 p2 = fPointList[i2].f2DPoint;
						if(Intersect(thisPos,point.f2DPoint,p1,p2))
						{
							forbidenList.AddElem(point.fIndex);
							// restart from the begining
							iPt = 0;
							OK=false;
						}
					}

					if(OK)*/
					{
						minDist = dist;
						// Need to check if this segment doesn't cut any hole
						if(!SegmentCutEdge(point.f2DPoint, thisPos))					
							nearestPoint = iPt;
					}
				}
			}
		}
	}

	return nearestPoint;
}

bool FlatPolygonMaker::SegmentCutEdge( const TVector2& point1, const TVector2& point2)
{
	const int32 edgeCount = fEdgeList.GetElemCount();

	for( int32 iEdge=0 ; iEdge<edgeCount ; iEdge++ )
	{
		FlatEdge& edge = fEdgeList[iEdge];
		const TVector2& ed1 = fPointList[edge.fPoint1].f2DPoint;
		const TVector2& ed2 = fPointList[edge.fPoint2].f2DPoint;
		// avoid the case where one of the points is a point of the polygon
		if( point1==ed1 )
			continue;
		if( point1==ed2 )
			continue;
		if( point2==ed1 )
			continue;
		if( point2==ed2 )
			continue;

		if( Intersect( point1, point2, ed1, ed2) )
			return true;
	}

	return false;
}

int32 FlatPolygonMaker::SegmentCutsHole(const TVector2& point1, const TVector2& point2,
										const TMCClassArray<PosArray>& holes, const int32 onHole)
{
	int32 holeID = -1;
	const int32 holeCount = holes.GetElemCount();
	for( int32 iHole=0 ; iHole<holeCount ; iHole++ )
	{
//	check also for this hole	if(onHole!=iHole)
			if(SegmentIntersectPolygonSegment(point1, point2, holes[iHole]))
				return iHole;
	}

	return holeID;
}

boolean FlatPolygonMaker::MakePolygon(FlatEdge* edge, const int32 point, PolygonIndexes& polygon)
{
	FlatEdge* curEdge = edge;
	int32 curPoint = point;
	TVector2 curPos = fPointList[curPoint].f2DPoint;
	TVector2 prevPos = fPointList[edge->OtherPoint(point)].f2DPoint;

	TMCPtrArray<FlatEdge> edges;

	real32 angle=0;

	int32 security=0;
	do
	{
		// Get the dir
		TVector2 dir = prevPos-curPos;
		dir.Normalize();

		real32 minAng=360;
		// find the next edge using the smallest angle
		const int32 edgeCount = fPointList[curPoint].fEdges.GetElemCount();
		FlatEdge* nextEdge=NULL;
		for( int32 iEdge=0 ; iEdge<edgeCount ; iEdge++ )
		{
			FlatEdge* otherEdge = &fEdgeList[fPointList[curPoint].fEdges[iEdge]];
			if( otherEdge != curEdge &&
				otherEdge->fUsed<2 )
			{
				// get the angle
				TVector2 nextDir = fPointList[otherEdge->OtherPoint(curPoint)].f2DPoint - curPos;
				nextDir.Normalize();
				const real32 ang = GetPositiveAngle(dir, nextDir);
				if(ang<minAng)
				{
					minAng = ang;
					nextEdge = otherEdge;
				}
			}
		}

		if(!nextEdge) // This shouldn't occur
			return false;

		// Get the next data
		curEdge = nextEdge;
		curPoint = nextEdge->OtherPoint(curPoint);
		prevPos = curPos;
		curPos = fPointList[curPoint].f2DPoint;

		angle+=minAng;
		edges.AddElem(curEdge);

		polygon.fPoints.AddElem(curPoint);

		// A security: shouldn't be used
		if(security++>100)
		{
			MCNotify("Tessellation problem");
			return false;
		}
	}
	while(curEdge!=edge);

	const int32 pointCount = polygon.fPoints.GetElemCount();
	
	// Angle/180 should be equal to pointCount-2 if OK, pointCount+2 if wrong
	if(pointCount>angle/180)
	{
		// Mark all the edges
		for( int32 iEdge=0 ; iEdge<pointCount ; iEdge++ )
		{
			edges[iEdge]->fUsed++;
		}
		return true;
	}
	return false;
}

void FlatPolygonMaker::GetPolygon( const int32 index, TMCClassArray<FlatPoint>& polygon )
{
	polygon.ArrayFree();

	PolygonIndexes& poly =  fPolygons[index];

	const int32 elemCount =poly.fPoints.GetElemCount();
	for( int32 iElem=0 ; iElem<elemCount ; iElem++ )
	{
		polygon.AddElem(fPointList[poly.fPoints[iElem]]);
	}
}

int32 FlatPolygonMaker::MakePolygons(TMCClassArray<Triangle>& triangleArray)
{
	int32 resultCount=0;

	MakePolygons();
	const int32 polyCount = GetPolygonCount();
	for( int32 iPoly=0 ; iPoly<polyCount ; iPoly++ )
	{
		TMCClassArray<FlatPoint> polygon;
		GetPolygon(iPoly, polygon);
		FlatPolygonTessellator tessellator(polygon);

		TMCClassArray<Triangle> result;
		tessellator.Tessellate(result);

		resultCount += result.GetElemCount();

		// Add the facet
		triangleArray.Append( result );
	}

	return resultCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////

FlatPolygonTessellator::FlatPolygonTessellator(TMCClassArray<FlatPoint>& polygon)
{
	fPolygon = polygon;
}

void FlatPolygonTessellator::Tessellate(TMCClassArray<Triangle>& result)
{
	result.ArrayFree();

	Tessellate(fPolygon, result);
}

void SquareTesselate(const FlatPoint& point0, 
					const FlatPoint& point1,
					const FlatPoint& point2,
					const FlatPoint& point3,
					TMCClassArray<Triangle>& result,
					const boolean firstTry )
{
	const TVector2& p0 = point0.f2DPoint;
	const TVector2& p1 = point1.f2DPoint;
	const TVector2& p2 = point2.f2DPoint;
	const TVector2& p3 = point3.f2DPoint;

	TVector2 axe = p2-p0; axe.Normalize();
	TVector2 p0p1 = p1-p0; p0p1.Normalize();
	TVector2 p0p3 = p3-p0; p0p3.Normalize();
	const real32 prod0 = (axe^p0p1);
	const real32 prod1 = (axe^p0p3);
	real32 side = prod0 * prod1;
	if( side > kRealEpsilon)
	{	// Didn't work: the 2 points are on the same side
		if(firstTry)
		{
			SquareTesselate(point1,point2,point3,point0,result, false);
		}
		else
		{	// Should be a flat polygon here
			MCNotify("Check why we couldn't tessellate properly this 4 points polygon");
			result.AddElem(Triangle(point0.fIndex,point1.fIndex,point3.fIndex));
			result.AddElem(Triangle(point1.fIndex,point2.fIndex,point3.fIndex));
		}
	}
	else if( side < -kRealEpsilon)
	{
		result.AddElem(Triangle(point0.fIndex,point1.fIndex,point2.fIndex));
		result.AddElem(Triangle(point0.fIndex,point2.fIndex,point3.fIndex));
	}
	else // 3 points are aligned
	{
		if(RealAbs(prod0)>RealAbs(prod1))//kRealEpsilon)
		{	// p0, p2, p3 are aligned
			// Check if p3 is beetween p2 and p0 (normal case)
			TVector2 p2p3 = p3-p2; p2p3.Normalize();
			if(p0p3*p2p3<=0)
			{
				result.AddElem(Triangle(point0.fIndex,point1.fIndex,point2.fIndex));
				result.AddElem(Triangle(point0.fIndex,point2.fIndex,point3.fIndex));
			}
			else
			{	// Weird polygon with a dead flat spike
				result.AddElem(Triangle(point0.fIndex,point1.fIndex,point3.fIndex));
				result.AddElem(Triangle(point1.fIndex,point2.fIndex,point3.fIndex));
			}
		}
		else if(RealAbs(prod1)>RealAbs(prod0))//kRealEpsilon)
		{	// p0,p1,p2 are aligned
			// Check if p1 is beetween p0 and p2 (normal case)
			TVector2 p2p1 = p1-p2; p2p1.Normalize();
			if(p0p1*p2p1<=0)
			{
				result.AddElem(Triangle(point0.fIndex,point1.fIndex,point3.fIndex));
				result.AddElem(Triangle(point1.fIndex,point2.fIndex,point3.fIndex));
			}
			else
			{	// Weird polygon with a dead flat spike
				result.AddElem(Triangle(point0.fIndex,point1.fIndex,point2.fIndex));
				result.AddElem(Triangle(point0.fIndex,point2.fIndex,point3.fIndex));
			}
		}
	}
}

void FlatPolygonTessellator::Tessellate(const TMCClassArray<FlatPoint>& points, 
										TMCClassArray<Triangle>& result)
{
	const int32 pointCount = points.GetElemCount();

	if( pointCount == 3 )
	{
		// Add the triangle only if it's not a flat one
		if( RealAbs( (points[0].f2DPoint-points[1].f2DPoint)
					^(points[0].f2DPoint-points[2].f2DPoint) )> kRealEpsilon )
			result.AddElem(Triangle(points[0].fIndex,points[1].fIndex,points[2].fIndex));
		return;
	}
	else if (pointCount == 4)
	{
		// 2 possibilities
		SquareTesselate(points[0],
						points[1],
						points[2],
						points[3],
						result, true);
		// else the square is flat
		return;
	}
	else if( TessellateSpan(points, result,true) )
		return;
	else if( TessellateSpanOneTriangle(points, result, true) )
		return;
	else if( TessellateSpan(points, result,false) )
	{
		MCNotify("Uses unsafe tessellation");
		return;
	}
	else if( TessellateSpanOneTriangle(points, result, false) )
	{
		MCNotify("Uses unsafe tessellation");
		return;
	}
	else
	{
		// Dumb tessellation
		MCNotify("Uses dumb tessellation");
		for (int32 i=2; i<pointCount; ++i) 
		{
			result.AddElem(Triangle(points[0].fIndex,points[i-1].fIndex,points[i-2].fIndex));
		}
	}
	// Remove flat triangles
}

boolean FlatPolygonTessellator::TessellateSpan(const TMCClassArray<FlatPoint>& points,
											   TMCClassArray<Triangle>& result, 
											   const boolean safeMode ) 
{
	int32 iMax=-1;
	real32 xMax=-kBigRealValue;
	real32 yMax=-kBigRealValue;

	const int32 pointCount = points.GetElemCount();
	{
		// Find an extrem
		for( int32 iPt=0; iPt<pointCount; iPt++ ) 
		{
			const FlatPoint& point=points[iPt];

			const real32 x = point.f2DPoint.x;
			if(x>xMax) 
			{
				xMax=x;
				yMax=point.f2DPoint.y;
				iMax=iPt;
			}
			else if( x == xMax )
			{
				const real32 y = point.f2DPoint.y;
				if(y>yMax) 
				{
					yMax=y;
					iMax=iPt;
				}
			}
		}
	}

	if (iMax == -1)
		return false;

	int32 span = pointCount/2;
	iMax=(iMax+pointCount/4)%pointCount;
	for(;;)
	{
		for( int32 iPt=0; iPt<pointCount; iPt++ )
		{
			int32 ia=(iMax+iPt)%pointCount;
			int32 ib=(ia+span)%pointCount;
			if (ia < ib && ValidSplit(points,ia,ib,safeMode)) 
			{
				SubTessellate(points,ia,ib,result);
				return true;
			}
		}
		if (span == 2)
			break;
		span/=2;
		if (span < 2)
			span=2;
	}
	return false;
}

// This method will try to cut out one triangle of the mesh
boolean FlatPolygonTessellator::TessellateSpanOneTriangle(const TMCClassArray<FlatPoint>& points,
											   TMCClassArray<Triangle>& result,
											   const boolean safeMode) 
{
	const int32 pointCount = points.GetElemCount();

	const int32 span = 2;
	for( int32 iPt=0; iPt<pointCount; iPt++ )
	{
		int32 ia=iPt;
		int32 ib=(ia+span)%pointCount;
		if(ia > ib)
		{
			int32 tmp=ia; ia=ib ; ib=tmp;
		}
		if (ValidSplit(points,ia,ib,safeMode)) 
		{
			SubTessellate(points,ia,ib,result);
			return true;
		}
	}

	return false;
}

boolean FlatPolygonTessellator::ValidSplit(const TMCClassArray<FlatPoint>& points,
										   const int32 i1, const int32 i2,
										   const boolean safeMode ) 
{
	const int32 pointCount=points.GetElemCount();

	// Some verifications
	int32 di=i2-i1+pointCount;
	if (di < 0) throw TMCException(-1);
	di=di%pointCount;
	if (di < 2 || di > pointCount-2) throw TMCException(-1);

	const TVector2& point1=points[i1].f2DPoint;
	const TVector2& point2=points[i2].f2DPoint;
	const TVector2 dir = point2-point1;
	if(!safeMode && dir.GetMagnitudeSquared()<kRealEpsilon ) // Fix for roof tessellation
	{	// if point1 and point2 are at the same position, it's still not sure it's a valid split
		return true; // we're not in safe mode anymore, considere these to be a valid split
	}
	TVector2 normal( -dir.y, dir.x );
	normal.Normalize();


	// Check all the intersections with the line point1,point2.
	// If an intersection is between point1 and point2, the split is invalid
	// If the path from 1 to 2 is on the sam side as the path from to 1, it's invalid too

	// From i1 to i2
	boolean LeftSide1=true;
	{
		TVector2 pos0 = point1;
		TVector2 pos1 = points[i1+1].f2DPoint;

		// check the 1st direction
		TVector2 orient = pos1-pos0;
		const float product = orient*normal;
		if(RealAbs( product )<kRealEpsilon)
		{	// point aligned, check if it's on our way
			if(orient*dir>0)
				return false; 
		}
		else if(product<0)
			LeftSide1 = false; // we start on the right side

		pos0 = pos1;

		for( int32 iPt=i1+1 ; iPt<i2-1 ; iPt++ )
		{
			pos1 = points[iPt+1].f2DPoint;

			TVector2 intersection;
			if( IntersectSegmentLine(pos0,pos1,point1,dir,intersection) )
			{	// There's an intersection, check where
				TVector2 vect = intersection - point1;
				if(vect.GetMagnitudeSquared()>kRealEpsilon ) // Fix for roof tessellation
				{
					if(vect*dir<0) // Switch side
						LeftSide1 = !LeftSide1;
					else
					{
						vect = intersection - point2;
						if(vect*dir<0) // forbidden area
							return false;
						// else we don't care
					}
				}
				else
					return false; // we may have switch side, but we don't know: let's stop here
			}
		
			pos0 = pos1;
		}
	}

	// From i2 to i1
	boolean LeftSide2=true;
	{
		TVector2 pos0 = point2;
		TVector2 pos1 = points[(i2+1)%pointCount].f2DPoint;

		// check the 1st direction
		TVector2 orient = pos1-pos0;
		const float product = orient*normal;
		if(RealAbs( product )<kRealEpsilon)
		{	// point aligned, check if it's on our way
			if(orient*dir<0)
				return false; 
		}
		else if(product<0)
			LeftSide2 = false; // we start on the right side

		pos0 = pos1;

		const int32 end = pointCount+i1-1;

		for( int32 iPt=i2+1 ; iPt<end ; iPt++ )
		{
			pos1 = points[(iPt+1)%pointCount].f2DPoint;

			TVector2 intersection;
			if( IntersectSegmentLine(pos0,pos1,point1,dir,intersection) )
			{	// There's an intersection, check where
				TVector2 vect = intersection - point2;
				if(vect.GetMagnitudeSquared()>kRealEpsilon ) // Fix for roof tessellation
				{
					if(vect*dir>0) // Switch side
						LeftSide1 = !LeftSide1;
					else
					{
						vect = intersection - point1;
						if(vect*dir>0) // forbidden area
							return false;
						// else we don't care
					}
				}
				else
					return false; // we may have switch side, but we don't know: let's stop here
			}
		
			pos0 = pos1;
		}
	}


	if(LeftSide1==LeftSide2)
		return false;

	return true;
}

void FlatPolygonTessellator::SubTessellate(const TMCClassArray<FlatPoint>& points,
							   const int32 i1, const int32 i2,
							   TMCClassArray<Triangle>& result ) 
{
	TMCClassArray<FlatPoint> subList;
	const int32 pointCount = points.GetElemCount();

	{
		subList.SetElemSpace(i2-i1+1);
		for( int32 iPt=i1; iPt<=i2; iPt++)
			subList.AddElem(points[iPt]);

		Tessellate( subList, result );
	}

	subList.ArrayFree();

	{
		const int32 end = i1+pointCount;
		subList.SetElemSpace(end-i2+1);
		for( int32 iPt=i2; iPt<=end; iPt++)
			subList.AddElem(points[iPt%pointCount]);
		Tessellate(subList, result);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Triangulation with constraints
//

namespace DelauneyCopy
{

Vertex2D::Vertex2D(const real32 x, const real32 y, const eLocation location) : TVector2(x,y)
{
	fLocation=location;
}

Triangle2D::Triangle2D(Vertex2DIndex vertex1,Vertex2DIndex vertex2,Vertex2DIndex vertex3)
{
	MY_ASSERT(vertex1 != vertex2 && vertex1 != vertex3 && vertex2 !=vertex3);

	fVertexIndex[0]=vertex1;
	fVertexIndex[1]=vertex2;
	fVertexIndex[2]=vertex3;
}

Vertex2DIndex Polygon2D::NonCommonVertex(const Triangle2D& triangle,
									 const TMCArray<int32>& commonPoints) const
{
	for (int32 i=0;i<3;i++)
	{
		const Vertex2DIndex& vertexIndex = triangle.fVertexIndex[i];

		if ( vertexIndex != Vertex(commonPoints[0]) &&
			 vertexIndex != Vertex(commonPoints[1]))
		{
			return vertexIndex;
		}
	}

	MCNotify("Shouldn't occured");

	return Vertex2DIndex();
}

// ***************************************************************************
//					Polygon2D::Merge
//
// Merge a triangle in the polygon if possible.
// Returns true if the merge was successful
//
boolean Polygon2D::Merge(const Triangle2D& triangle)
{
	if (GetVertexCount()==0)
	{
		// polygon is empty
		for (int32 i=0;i<3;i++)
		{
			AddElem(triangle.fVertexIndex[i]);
		}
		return true;
	}

	MY_ASSERT(GetVertexCount()>=3);

	//	find common points

	TMCArray<int32> commonPoints; // index of common points in polygon

	const int32 vertexCount = GetVertexCount();

	for (int32 j=0;j<vertexCount;j++)
	{
		for (int32 i=0;i<3;i++)
		{
			if ( triangle.fVertexIndex[i] == Vertex(j) )
			{
				commonPoints.AddElem(j);
			}
		}
	}
	MY_ASSERT(commonPoints.GetElemCount()<4); // duplicated points ?

	switch(commonPoints.GetElemCount())
	{
	case 2:
		{
			// find non common point
			
			// are the two points on a common edge of the polygon ?
			const int32 index1 = commonPoints[0];
			const int32 index2 = commonPoints[1];

			const int32 n = GetVertexCount();

			if ( index2 == ((index1+1)%n) )
			{
				InsertElem(index2,NonCommonVertex(triangle,commonPoints));
				return true;
			}
			if ( index2 == ((index1+n-1)%n) )
			{
				InsertElem(index1,NonCommonVertex(triangle,commonPoints));
				return true;
			}
			return false; // not on a common edge
		}
	case 3:
		{
			// the three points are common
			// are they next to each other in polygon
			const int32 index1 = commonPoints[0];
			const int32 index2 = commonPoints[1];
			const int32 index3 = commonPoints[1];

			const int32 n = GetVertexCount();

			if (index2 == ((index1+1)%n) && (index3 == ((index2+1)%n)))
			{
				RemoveElem(index2,1);
				return true;
			}
			if (index2 == ((index1+n-1)%n) && (index3 == ((index2+n-1)%n)))
			{
				RemoveElem(index2,1);
				return true;
			}
			return false;
			
		}
	default:
		{
			// 0 or 1 common point
			return false; // a common edge requires 2 common points
		}
	}
}

// ***************************************************************************
//
// Triangulator2D
//
Triangulator2D::Triangulator2D(const TMCArray<Vertex2D>& vertices): fVertices2D(vertices)
{
	fConstraints.SetElemCount(vertices.GetElemCount());
}

Triangulator2D::~Triangulator2D()
{
	const int32 triangleCount = fTriangles2D.GetElemCount();

	for (int32 triangleIndex=0;triangleIndex<triangleCount;triangleIndex++)
	{
		delete fTriangles2D[triangleIndex];
		fTriangles2D[triangleIndex]=NULL;
	}

	const int32 vertexCount = fConstraints.GetElemCount();

	for (int32 vertexIndex=0;vertexIndex<vertexCount;vertexIndex++)
	{
		delete fConstraints[vertexIndex];
		fConstraints[vertexIndex]=NULL;
	}
}

void Triangulator2D::TriangulateButterfly(const Polygon2D& polygon,Vertex2DIndex vertexIndex)
{
	// check if the vertex is on the edge
	boolean isOnEdge = GetVertex(vertexIndex).IsOnEdge();

	const int32 vertexCount = polygon.GetElemCount();

	for (int32 i=0;i<vertexCount;i++)
	{
	
		boolean addTriangle=true;

		if (isOnEdge)
		{
			// in this case we need to check for flat triangles
			const TVector2& a = GetVertex(vertexIndex);
			const TVector2& b = GetVertex(polygon[i]);
			const TVector2& c = GetVertex(polygon[(i+1)%vertexCount]);

			if ( fabs((b-a)^(c-a)) < kRealEpsilon)
			{
				addTriangle=false; // point is on edge
			}
		}
		if (addTriangle)
		{
			Triangle2D *triangle = new Triangle2D(vertexIndex,polygon[i],polygon[(i+1)%vertexCount]);
			fTriangles2D.AddElem(triangle);
		}
	}
}

// returns true if an intersection was found

inline boolean GetLineIntersection(const TVector2 &p0,
									const TVector2 &v0,
									const TVector2 &p1,
									const TVector2 &v1,
									TVector2 &intersection )
{
	TVector2 d = p1 - p0;
	real32 prod = (v0^v1);

	MY_ASSERT(prod!=0.0f); // v0 and v1 are colinear !

	if (prod != 0.0f)
	{
		real32 lambda = (d^v1) / prod;
		intersection = p0 + lambda*v0;
		return true;
	}

	return false; // flat triangle
}

// returns true if the point is in the circumcircle of the triangle
inline boolean Triangle2D::InCircle(const TVector2& position,const Vertex2DList& vertices) const
{
	const TVector2& a = vertices[fVertexIndex[0]];
	const TVector2& b = vertices[fVertexIndex[1]];
	const TVector2& c = vertices[fVertexIndex[2]];

	// Determine Circle center (geometric approch !!)
	TVector2 ab = (b-a);
	ab.Normalize();
	TVector2 ac = (c-a);
	ac.Normalize();

	TVector2 biseca(ab.y,-ab.x);
	TVector2 bisecb(ac.y,-ac.x);

	TVector2 CenterA = (b+a)*0.5f;
	TVector2 CenterB = (c+a)*0.5f;

	TVector2 circleCenter(0,0); // Circle center
	if( GetLineIntersection( CenterA, biseca, CenterB, bisecb, circleCenter ) )
	{
		real32 R = (a-circleCenter).GetMagnitudeSquared();
		real32 d = (position-circleCenter).GetMagnitudeSquared();

		if ( d < R )
			return true;
		else
			return false;
	}
	else
	{	// The triangle is a flat triangle
		// We shouldn't have vertices at the same position here
		MY_ASSERT(a!=b && a!=c && b!=c);

		// no circle
		return false;
	}
}

// delete the triangles pointed by the array
void DeleteTriangles(TMCPtrArray<Triangle2D>& triangles)
{
	const int32 triangleCount = triangles.GetElemCount();

	for (int32 triangleIndex=0;triangleIndex<triangleCount;triangleIndex++)
	{
		delete triangles[triangleIndex];
		triangles[triangleIndex]=NULL;
	}
	triangles.ArrayFree();
}

// copy the triangles pointed by the array
void CopyTriangles(TMCPtrArray<Triangle2D>& source,TMCPtrArray<Triangle2D>& destination)
{
	const int32 triangleCount = source.GetElemCount();

	for (int32 triangleIndex=0;triangleIndex<triangleCount;triangleIndex++)
	{
		if (MCVerify(source[triangleIndex]!=NULL))
		{
			destination.AddElem(source[triangleIndex]);
			source[triangleIndex]=NULL;
		}
	}
	source.ArrayFree();
}

boolean Triangulator2D::MergeTriangles(Polygon2D& mergedPolygon,TMCPtrArray<Triangle2D> trianglesToMerge)
{
		// check that there is a triangle that contain the point
		MY_ASSERT(trianglesToMerge.GetElemCount()!=0);
		if (trianglesToMerge.GetElemCount()==0)
		{
			return false;
		}

		// now merge all the triangles into one big polygon

		uint32 k=0;
		boolean couldMerge=true; // used to avoid infinite loop

		while (couldMerge && trianglesToMerge.GetElemCount() > 0)
		{
			// look for a triangle to merge with the polygon
			couldMerge=false;

			int32 triangleIndex=0;

			while (triangleIndex < (int32)trianglesToMerge.GetElemCount())
			{
				Triangle2D &triangle = *trianglesToMerge[triangleIndex];

				if ( mergedPolygon.Merge(triangle) )
				{
					trianglesToMerge.RemoveElem(triangleIndex,1);

					couldMerge=true;
					break;
				}
				else
				{
					triangleIndex++;
				}
			}
		}

		// there should be no triangle left !

		if (MCVerify(trianglesToMerge.GetElemCount()==0))
		{
			return true;
		}
		return false; // polys could not be merged (why ??)
}

void Triangulator2D::ScaleData()
{
	const int32 vtxCount = fVertices2D.GetCount();

	MY_ASSERT(vtxCount>=3);
	if(vtxCount<3)
		return;

	// get the bounding box of the triangles
	real32 minX=kBigRealValue;
	real32 minY=kBigRealValue;
	real32 maxX=-kBigRealValue;
	real32 maxY=-kBigRealValue;
	for( int32 i=0 ; i<vtxCount ; i++ )
	{
		Vertex2DIndex vertexIndex;
		vertexIndex.SetValue(i);
		const real32 x = fVertices2D[vertexIndex].x;
		const real32 y = fVertices2D[vertexIndex].y;
		minX = MC_Min(minX,x);
		minY = MC_Min(minY,y);
		maxX = MC_Max(maxX,x);
		maxY = MC_Max(maxY,y);
	}

	const real32 xScale=maxX-minX;
	MY_ASSERT(xScale>0);
	if(xScale<=0)
		return;

	const real32 yScale=maxY-minY;
	MY_ASSERT(yScale>0);
	if(yScale<=0)
		return;

	const TVector2 min( minX, minY );

	for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		fVertices2D[iVtx] -= min;
		fVertices2D[iVtx].x /= xScale;
		fVertices2D[iVtx].y /= yScale;
	}
}

// Build Initial Triangulation using Delauney algorithm :
//
// build the containing triangle
// for each point P
//		1 remove all triangles who circonscrit circle contains P
//		2 build a polygon by merging all these removed triangles
//		3 triangulate this triangle by linking P to all its points
//
void Triangulator2D::Triangulate()
{

	MY_ASSERT(fVertices2D.GetCount()>=3);

	// triangulate should only be called once !
	MY_ASSERT(fTriangles2D.GetElemCount()==0);

	// Prepare the points:

	// first remove duplicated points
// Vertices are now pruned outside	PruneDuplicatedPoints();
	// Scale them in a 0,1 * 0,1 square
 	ScaleData();

	// add the triangle in the trianglePool

	TMCPtrArray<Triangle2D> trianglesToMerge;
	
	Vertex2DIndex vertex1,vertex2,vertex3;

	vertex1.SetValue(0);
	vertex2.SetValue(1);
	vertex3.SetValue(2);

	fTriangles2D.AddElem(new Triangle2D(vertex1,vertex2,vertex3));

	const int32 vertexCount = fVertices2D.GetCount();

	Vertex2DIndex vertexIndex;
	// for each point check the triangle that need to be merged
	for (vertexIndex.SetValue(3);vertexIndex<vertexCount;++vertexIndex)
	{
		const Vertex2D& vertex = fVertices2D[vertexIndex];

		// 1 - get the triangles whose circum circle contains the point 

		int32 triangleIndex=0;

		while (triangleIndex < (int32)fTriangles2D.GetElemCount())
		{
			const Triangle2D* triangle = fTriangles2D[triangleIndex];

			MY_ASSERT(triangle!=NULL);

			if (triangle->InCircle(vertex,fVertices2D) )
			{
				trianglesToMerge.AddElem(triangle);
				fTriangles2D.RemoveElem(triangleIndex,1);
			}
			else
			{
				triangleIndex++;
			}
		}
		
		boolean pointAdded=false;

		if (trianglesToMerge.GetElemCount()>0)
		{
			// 2- merge the triangles

			Polygon2D mergedPolygon;

			if (MergeTriangles(mergedPolygon,trianglesToMerge))
			{

				// 3- Stellate this poly with respect to point

				TriangulateButterfly(mergedPolygon,vertexIndex);
				pointAdded=true;
			}
		}
		if (pointAdded)
		{
			DeleteTriangles(trianglesToMerge);
		}
		else
		{
			// something went wrong, let's kick this point out of the triangulation
			fVertices2D[vertexIndex].SetInvalid();

			CopyTriangles(trianglesToMerge,fTriangles2D);
		}
	}
}


// constructor of the vertex list
Vertex2DList::Vertex2DList(const TMCArray<Vertex2D>& vertexArray)
{
	fVertices2D = vertexArray;
}

boolean Triangle2D::ContainsEdge(Vertex2DIndex vertex1,Vertex2DIndex vertex2) const
{
	boolean found=false;

	for (int32 i=0;i<3;i++)
	{
		if (vertex1 == fVertexIndex[i])
		{
			found = true;
			break;
		}
	}
	if (found)
	{
		for (int32 i=0;i<3;i++)
		{
			if (vertex2 == fVertexIndex[i])
			{
				return true;
			}
		}
	}
	return false;
}


boolean Triangle2D::Intersect(Vertex2DIndex				vertex1,
							Vertex2DIndex				vertex2,
							const Vertex2DList&		vertices) const
{
	// look for a common point ( there should never be two )

	int32 commonPointIndex=-1;

	for (int32 i=0;i<3;i++)
	{
		if (vertex1 == fVertexIndex[i] || vertex2 == fVertexIndex[i])
		{
			commonPointIndex=i;
		}
	}

	// compute the equation of the line going through vertex1, vertex2

	const TVector2& a = vertices[vertex1];
	const TVector2& b = vertices[vertex2];

	TVector2 ab = b-a;
	TVector2 N(ab.y,-ab.x); // normal vector to the line
	N.Normalize();

	real64 D = N*a; // the line equation is  N*p-D = 0

	// go through all the edges and check those who can intersect the segment

	for (int32 edgeIndex=0;edgeIndex<3;edgeIndex++)
	{
		const TVector2& p0 = vertices[fVertexIndex[edgeIndex]];
		const TVector2& p1 = vertices[fVertexIndex[(edgeIndex+1)%3]];

		// don't consider edges that share a point
		if (commonPointIndex<0 || (commonPointIndex!=edgeIndex && commonPointIndex!=(edgeIndex+1)%3))
		{
			real64 test1 = (N*p0-D);
			real64 test2 = (N*p1-D);
			// check if point are on both side of the line
			if ( (N*p0-D) * (N*p1-D) <= 0.0f )
			{
				// the edge intersect the line
				TVector2 n(p1.y-p0.y , p0.x-p1.x);

				real64 d = n*p0; // line equation is n*p-d=0

				real64 test3 = (n*a-d);
				real64 test4 = (n*b-d);

				if ( (n*a-d) * (n*b-d) <= 0.0f )
				{
					return true; // the edge intersect the segment (vertex1,vertex2)
				} 
			}
		}
	}
	return false; // we don't intersect the triangle
}


// split the polygon by the segment [vertex1,vertex2] ( vertex1 and vertex2 must be vertices of the polygon )
boolean Polygon2D::Split(Vertex2DIndex vertex1,Vertex2DIndex vertex2,Polygon2D& polygon1,Polygon2D& polygon2) const
{
	
	const int32 vertexCount = GetVertexCount();

	// search the first vertex in the polygon

	int32 firstVertex=-1;

	int32 index=0;

	for (index=0;index<vertexCount;index++)
	{
		if (Vertex(index) == vertex1)
		{
			firstVertex=index;
			break;
		}
	}
	MY_ASSERT(firstVertex>=0);

	if (firstVertex<0) return false;
	// search for the second vertex in the polygon

	int32 secondVertex=-1;

	for (index=0;index<vertexCount;index++)
	{
		if (Vertex(index) == vertex2)
		{
			secondVertex=index;
			break;
		}
	}
	MY_ASSERT(secondVertex>=0);
		
	if (secondVertex<0) return false;

	// build first polygon
	polygon1.ArrayFree(); // just to be sure


	for (index=firstVertex;index != secondVertex; index= ((index+1)% vertexCount) )
	{
		polygon1.AddElem(Vertex(index));
	}
	polygon1.AddElem(vertex2);

	// build second polygon
	for (index=secondVertex;index!=firstVertex;index= ((index+1)% vertexCount))
	{
		polygon2.AddElem(Vertex(index));
	}
	polygon2.AddElem(vertex1);

	return true;
}

// returns true if there was a constraint set between the two points
boolean Triangulator2D::CheckContraint(Vertex2DIndex vertexIndex1,Vertex2DIndex vertexIndex2)
{
	int32 index1 = vertexIndex1.GetValue();
	int32 index2 = vertexIndex2.GetValue();

	TMCArray<int32>* array = fConstraints[index1];

	if (array)
	{
		for (int32 i=0;i<(int32)array->GetElemCount();i++)
		{
			if ( (*array)[i] == index2 )
			{
				return true;
			}
		}
	}
	return false;
}

// add a constraint to the triangulation ( should be called only after Triangulate as been called)
void Triangulator2D::AddConstraint(Vertex2DIndex vertexIndex1,Vertex2DIndex vertexIndex2)
{
	// check if a triangulation has been built
	MY_ASSERT(fTriangles2D.GetElemCount()>0);

	if (fVertices2D[vertexIndex1].IsInvalid() || 
		fVertices2D[vertexIndex2].IsInvalid())
	{
		// something went wrong in the triangulation, we cannot enforce this
		// constraint !!
		MCNotify("Can't enforce this constraint");
		return;
	}

	// add the constraint to the list of contraints
	int32 indices[2] = { vertexIndex1.GetValue(),vertexIndex2.GetValue() };

	for (int32 i=0;i<2;i++)
	{
		TMCArray<int32>*& array = fConstraints[indices[i]];

		if (!array)
		{
			array=new TMCArray<int32>;
		}
		array->AddElem(indices[(i+1)%2]);
	}

	// check if the edge is in the triangulation
	int32 triangleIndex=0;

	for (triangleIndex=0;triangleIndex<(int32)fTriangles2D.GetElemCount();triangleIndex++)
	{
		const Triangle2D* triangle = fTriangles2D[triangleIndex];

		if (triangle->ContainsEdge(vertexIndex1,vertexIndex2))
		{
			return; // the edge is already in the triangulation
		}
	}

	// now we need to find all the triangles that intersect that edge and merge
	// them into a big polygon.

	TMCPtrArray<Triangle2D> trianglesToMerge;

	triangleIndex=0;

	while (triangleIndex < (int32)fTriangles2D.GetElemCount())
	{
		const Triangle2D* triangle = fTriangles2D[triangleIndex];			

		if (triangle->Intersect(vertexIndex1,vertexIndex2,fVertices2D))
		{
			trianglesToMerge.AddElem(triangle);
			fTriangles2D.RemoveElem(triangleIndex,1);
		}
		else
		{
			triangleIndex++;
		}
	}
	Polygon2D mergedPolygon;

	boolean constraintAdded = false;

	if( trianglesToMerge.GetElemCount()==0 )
	{	// Something went wrong, we couldn't find any triangle.
		return;
	}

	if (MergeTriangles(mergedPolygon,trianglesToMerge))
	{
		// we need to split the polygon by the constraint
		// and triangulate it
		Polygon2D polygon1;
		Polygon2D polygon2;

		if (mergedPolygon.Split(vertexIndex1,vertexIndex2,polygon1,polygon2))
		{
			Triangulate(polygon1);
			Triangulate(polygon2);

			constraintAdded=true;
		}
	}

	if (constraintAdded)
	{
		// delete merged triangles
		DeleteTriangles(trianglesToMerge);
	}
	else
	{
		CopyTriangles(trianglesToMerge,fTriangles2D);
	}
}

// returns a list of triangles corresponding to the triangulation
const TMCPtrArray<const Triangle2D>* Triangulator2D::GetTriangulation()
{
	// check if a triangulation has been built
	MY_ASSERT(fTriangles2D.GetElemCount()>0);

	return (const TMCPtrArray<const Triangle2D>*)&fTriangles2D;
}


boolean Segment2D::IsInside(const TMCArray<Vertex2DIndex>& polygon,const Vertex2DList& vertices)
{
	// an segment is inside the polygon if :
	//     1 it does not cut any of the edges of the polygon
	//     2 its middle point is inside the polygon

	const int32 vertexCount = polygon.GetElemCount();

	const TVector2& p1 = vertices[fVertex1];
	const TVector2& p2 = vertices[fVertex2];

	// check if the middle point is inside the polygon
	{
		TVector2 p = (p1 + p2) * 0.5f;

		// intersect horizontal line containing the point and count intersections to the right of the point

		TMCArray<boolean> isIntersected; // for each vertex whether it is on the line or not
		
		isIntersected.SetElemCount(polygon.GetElemCount());

		int32 intersectionCount=0;

		// go through all the vertices and count the number of intersection
		
		for (int32 vertexIndex=0;vertexIndex<vertexCount;vertexIndex++)
		{
			const TVector2& a = vertices[polygon[vertexIndex]];

			if ( fabs(a.y-p.y) < kRealEpsilon )
			{
				// we intersect the vertex
				isIntersected[vertexIndex] = true;

				if ( a.x > p.x + kRealEpsilon ) // point on the edge are not considered inside
				{
					intersectionCount++;
				}
			}
			else
			{
				isIntersected[vertexIndex] = false;
			}
		}

		for (int32 edgeIndex=0;edgeIndex<vertexCount;edgeIndex++)
		{
			const int32 vertex1 = edgeIndex;
			const int32 vertex2 = ((edgeIndex+1)%vertexCount);

			// if the line intersect one of the extremities, we do not need to check
			if ( !isIntersected[vertex1] && !isIntersected[vertex2])
			{
				const TVector2& a = vertices[polygon[vertex1]];
				const TVector2& b = vertices[polygon[vertex2]];

				// first we check if we intersect the extremities ( important to do it first for precision )

				if ( (a.y-p.y)*(b.y-p.y) < 0) // check if the line is inside the segment [a,b]
				{
					// the line intersect the segment, find position of the intersection relativ to p

					TVector2 n(b.y-a.y,a.x-b.x); // normal vector to the line

					if ( (n*p-n*a)*(n.x) < 0 )
					{
						intersectionCount++; // the intersection is to the right of p
					}
				}
			}
		}
		if ( (intersectionCount % 2) != 1 )
		{
			return false; // the point is outside the polygon
		}
	}
	// if we reach this point we know that at least part of the segment is inside the polygon
	// check if there is any intersection with this edge
	{
		const TVector2 N(p2.y-p1.y,p1.x-p2.x); // normal vector to the line
		const real32 D=N*p1;

		// line equation is N*P = D

		for (int32 edgeIndex=0;edgeIndex<vertexCount;edgeIndex++)
		{
			const TVector2& a = vertices[polygon[edgeIndex]];
			const TVector2& b = vertices[polygon[(edgeIndex+1)%vertexCount]];

			if ( (N*a-D)*(N*b-D) < kRealEpsilon ) // intersection with the vertices must be taken into account
			{
				// the line (vertex1,vertex2) intersect the edge ]a,b[
				const TVector2 n(b.y-a.y,a.x-b.x);
				const real32 d=n*a;

				if ( (n*p1-d)*(n*p2-d) < -kRealEpsilon ) // we only care about intersection strictly inside the segment
				{
					return false; // the segment crosses the edge of the polygon
				}
			}
		}
	}
	return true;
}

// triangulate a polygon
boolean Triangulator2D::Triangulate(const Polygon2D& poly)
{
	TMCArray<Vertex2DIndex> polygon(poly);

	while (polygon.GetElemCount()>3)
	{
		const int32 vertexCount = polygon.GetElemCount();

		boolean found=false;

		// try to find a vertex that can be removed from the polygon
		for (int32 i=0;i<vertexCount;i++)
		{
			int32 indexA = (i+vertexCount-1)%vertexCount;
			int32 indexB = (i+1)%vertexCount;

			Segment2D edge(polygon[indexA],polygon[indexB]);

			if (edge.IsInside(polygon,fVertices2D))
			{
				Triangle2D* triangle = new Triangle2D(polygon[indexA],polygon[i],polygon[indexB]);

				fTriangles2D.AddElem(triangle);

				polygon.RemoveElem(i,1);

				found=true;

				break;
			}
		}
		if (!found)
		{
			MCNotify("Polygon triangulation failed");
			return false; // to avoid infinite loop	
		}
	}
	MY_ASSERT(polygon.GetElemCount() == 3);

	Triangle2D* triangle = new Triangle2D(polygon[0],polygon[1],polygon[2]);

	fTriangles2D.AddElem(triangle);

	return true;
}

} // end namespace DelauneyCopy



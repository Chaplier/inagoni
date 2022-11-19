/****************************************************************************************************

		PRoofZone.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/16/2004

****************************************************************************************************/

#include "PRoofZone.h"

#include "PRoof.h"
#include "PPlan.h"
#include "PLevel.h"
#include "IShTokenStream.h"
#include "Utils.h"
#include "PTessellator.h"

RoofZone::RoofZone()
{
}
/*
RoofZone::RoofZone(const TMCCountedPtrArray<Point>& area, Roof* roof)
{
	SetArea(area, roof, true);
}
*/
void RoofZone::SetArea(const TMCCountedPtrArray<VPoint>& area, Roof* roof, const ERoofType roofType)
{
	fZoneSection.ArrayFree();

	// Clean up the area and build the zone points
	const int32 areaCount = area.GetElemCount();

	if(areaCount<3)
		return;

	// Keep the bounding rectangle (use for the scaling and the rectangle roofs)
	TBBox2D rectangle(area[0]->Position(),area[0]->Position());
	{
		for(int32 iArea=1 ; iArea<areaCount ; iArea++)
		{
			rectangle.AddPoint( area[iArea]->Position() );
		}
	}

	// 1: determine the global shape
	switch(roofType)
	{
	case eBorderRoof:
	case eLevelShapeRoof:
	case eLevelShapeClosedSpineRoof:
		{	// Follow the area but remove the aligned points
			fZoneSection.SetElemSpace(areaCount);

			VPoint* prev = area[0];
			VPoint* cur = area[1];

			// Remove aligned points
			for(int32 iArea=0 ; iArea<areaCount ; iArea++)
			{
				VPoint* next = area[(iArea+2)%areaCount];

				// if prev, cur and next are not aligned, add cur to the cleanedArea
				const TVector2 curPos = cur->Position();
				const TVector2 prevDir = prev->Position()-curPos;
				const TVector2 nextDir = next->Position()-curPos;
				if( RealAbs( prevDir^nextDir ) > kRealEpsilon)
				{
					ZoneSection& point = fZoneSection.AddElem();
					RoofPoint::CreateRoofPoint(&(point.fZonePoint),roof->GetData(),curPos,roof, false);
					RoofPoint::CreateRoofPoint(&(point.fSpinePoint),roof->GetData(),curPos,roof, true);
				}

				prev = cur;
				cur = next;
			}
		} break;
	case eRectangleRoof:
	case eRectangleRoofFlatExtremsX:
	case eRectangleRoofFlatExtremsY:
	case eHalfRectangleRoof1:
	case eHalfRectangleRoof2:
	case eHalfRectangleRoof3:
	case eHalfRectangleRoof4:
		{
			TVector2 corners[4];

			corners[0] = rectangle.fMin;
			corners[1].x = rectangle.fMin.x;
			corners[1].y = rectangle.fMax.y;
			corners[2] = rectangle.fMax;
			corners[3].x = rectangle.fMax.x;
			corners[3].y = rectangle.fMin.y;

			// Then build the 4 corners of the roof
			fZoneSection.SetElemCount(4);
			for(int32 i=0 ; i<4 ; i++)
			{
				ZoneSection& point = fZoneSection[i];
				RoofPoint::CreateRoofPoint(&(point.fZonePoint),roof->GetData(),corners[i],roof, false);
				RoofPoint::CreateRoofPoint(&(point.fSpinePoint),roof->GetData(),corners[i],roof, true);
			}
		} break;
	}

	// 2: move the spine point in the middle
	BuildCentralSpine();

	// 3: slightly move the zone points outside
	if(roofType!=eBorderRoof)
		AdjustZonePoints(rectangle, roof->GetData()); // do this after building the central spine, but before adjusting it

	DetermineZoneOrientation(roof);

	// 4: replace some of the spine points
	AdjustSpinePoints(roofType); // do this after building the central spine

	// 5: set a specific default profile
	if(roofType==eBorderRoof)
	{
		roof->SetTopProfile(eShape9); // none
		roof->SetBotProfile(eShape9); // wall
	}
}


inline int32 NegIndexCheck(const int32 value, const int32 base)
{
	if(value<0) return value+base;
	else return value;
}
inline int32 IndexCheck(const int32 value, const int32 base)
{
	if(value<0) return value+base;
	else return value%base;
}

// This should work for simple shapes. If the shape under becomes a little bit
// strange, the spine will be too => manual settings will be required
void RoofZone::BuildCentralSpine()
{
	// Build the central spine: every points of the surrounding will be moved
	// toward the center of the area
	const int32 zoneCount = fZoneSection.GetElemCount();

	if(zoneCount<3)
		return;
	// Prepare a tessellation of the area, we'll need it
	FlatMesh tessellationResult;

	{
		FlatPolygon polygon;
		for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
		{
			const TVector2& pos = fZoneSection[iZone].fZonePoint->Position();//fLevelPoint->Position();
			polygon.AddElem( pos );
		}
		TesselatePolygon( polygon, tessellationResult);
	}
/*
	TMCClassArray<Triangle> triangleArray;
	TMCClassArray<TVector2> positions(zoneCount);
	{
		FlatPolygonMaker polygonMaker;
		for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
		{
			const TVector2& pos = fZoneSection[iZone].fZonePoint->Position();//fLevelPoint->Position();
			positions.SetElem(iZone, pos );
			polygonMaker.AddPointToCountainer( pos, iZone );
		}
		// Build the polygons
		polygonMaker.MakePolygons();
		// Tesselate the polygons
		const int32 polyCount = polygonMaker.GetPolygonCount();
		for( int32 iPoly=0 ; iPoly<polyCount ; iPoly++ )
		{
			TMCClassArray<FlatPoint> polygon;
			polygonMaker.GetPolygon(iPoly, polygon);
			FlatPolygonTessellator tessellator(polygon);

			TMCClassArray<Triangle> result;
			tessellator.Tessellate(result);

			// Add the facet
			triangleArray.Append( result );
		}
	}*/

	// Find the bissectrice intersection the nearest from its side
	real32 smallestDist = kBigRealValue;
	int32 startIndex = 0;
	TVector2 bestIntersection=TVector2::kZero;
	TVector2 prev = fZoneSection[zoneCount-3].fZonePoint->Position();
	TVector2 cur0 = fZoneSection[zoneCount-2].fZonePoint->Position();
	TVector2 cur1 = fZoneSection[zoneCount-1].fZonePoint->Position();
	// Get the first bissectrice (cur0,prev , cur0,cur1)
	TVector2 bis0 = BissectDir2D(cur0,prev,cur1);
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const TVector2& next = fZoneSection[iZone].fZonePoint->Position();

		// Get the second bissectrice of (cur1,cur0 , cur1,next)
		const TVector2 bis1 = BissectDir2D(cur1,cur0,next);
		// Compute their intersection
		TVector2 intersection = TVector2::kZero;
		if( IntersectLineLine2(cur0,bis0,cur1,bis1,intersection) )
		{
			// Now the distance to the line cur-,cur1
			const real32 dist = PointLineSqrDistance(intersection, cur0, cur1);
			if(dist<smallestDist)
			{
				// Verify that the point is inside the area
				if(PointIsInTriangleArray(intersection,tessellationResult.mTriangles,tessellationResult.mVertices))
				{
					// Retain this one
					smallestDist = dist;
					startIndex = iZone-2;
					if(startIndex<0) startIndex+=zoneCount;
					bestIntersection = intersection;
				}
			}
		}

		// Move to next point
		prev = cur0;
		cur0 = cur1;
		cur1 = next;
		bis0 = bis1;
	}

	// Set the first known point
	fZoneSection[startIndex].fSpinePoint->SetPosition(bestIntersection);
	// Then move arround the zone using this first point as a reference
	int32 curIndex = (startIndex+1)%zoneCount;
	TVector2 prevPos = fZoneSection[startIndex].fZonePoint->Position();
	TVector2 curPos = fZoneSection[curIndex].fZonePoint->Position();

	TVector2 prevSpinePos = bestIntersection;
	// Then go arround the area and intersect the new pos
	for(int32 i=1 ; i<zoneCount ; i++) // i=1 => we already know the first pos
	{
		const int32 nextIndex = (curIndex+1)%zoneCount;
		const TVector2& nextPos = fZoneSection[nextIndex].fZonePoint->Position();

		// Compute the intersection between the parallel to prev and cur
		// going through the prevSpinePos and the bisectrice to cur,prev , cur,next
		// to determine the curSpinePos
		const TVector2 bis = BissectDir2D(curPos,prevPos,nextPos);
		TVector2 intersect=prevSpinePos;

		IntersectLineLine2( prevSpinePos,prevPos-curPos,
							curPos,bis,intersect );

		fZoneSection[curIndex].fSpinePoint->SetPosition(intersect);
		prevSpinePos = intersect;

		prevPos = curPos;
		curPos = nextPos;
		curIndex = nextIndex;
	}
}

void RoofZone::AdjustZonePoints(const TBBox2D& rectangle, BuildingPrimData* data)
{
	// Slightly scale the zone points to place them at the limit of the wall
	// Use the BBox 2D and the default wall thickness to evaluate the scaling
	const real32 size = MC_Min( rectangle.GetWidth(), rectangle.GetHeight() );
	const real32 thick = data->GetDefaultWallThickness();
	const real32 margin = thick;

	const real32 scaleFactor = (size+thick+margin)/size;

	const TVector2 scale(scaleFactor,scaleFactor);

	const int32 sectionCount = GetSectionCount();

	for(int32 iPt=0 ; iPt<sectionCount ; iPt++)
	{
		const ZoneSection& zoneSection = GetZoneSection(iPt);

		zoneSection.fZonePoint->SetPosition(ScalePoint(zoneSection.fZonePoint->Position(),scale,zoneSection.fSpinePoint->Position()));
	}
}

// try to colmat the holes in the spine
void RoofZone::AdjustSpinePoints(const ERoofType roofType)
{
	switch(roofType)
	{
	case eLevelShapeClosedSpineRoof:
		{
			const int32 zoneCount = fZoneSection.GetElemCount();
			
			if(zoneCount<5)
				return;

			CommonPoint* prevPrevPoint = fZoneSection[zoneCount-5].fSpinePoint;
			CommonPoint* prevPoint = fZoneSection[zoneCount-4].fSpinePoint;
			CommonPoint* cur0Point = fZoneSection[zoneCount-3].fSpinePoint;
			CommonPoint* cur1Point = fZoneSection[zoneCount-2].fSpinePoint;
			CommonPoint* nextPoint = fZoneSection[zoneCount-1].fSpinePoint;

			for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
			{
				CommonPoint* nextNextPoint = fZoneSection[iZone].fSpinePoint;
			
				if( RealAbs( (prevPoint->Position()-prevPrevPoint->Position())^
						(prevPoint->Position()-cur0Point->Position()) ) > kRealEpsilon &&
					RealAbs(	(nextPoint->Position()-nextNextPoint->Position())^
						(nextPoint->Position()-cur1Point->Position()) ) > kRealEpsilon )
				{

					const TVector2 prevDir = cur0Point->Position() - prevPoint->Position();
					const TVector2 curDir = cur1Point->Position() - cur0Point->Position();
					const TVector2 nextDir = nextPoint->Position() - cur1Point->Position();
					if( prevDir.GetSquaredNorm()>kRealEpsilon &&
						curDir.GetSquaredNorm()>kRealEpsilon &&
						nextDir.GetSquaredNorm()>kRealEpsilon )
					{
						// If parallel, move the points toward the line between the 2 parallels
						const real32 cur0dist = PointLineSqrDistance(cur0Point->Position(), cur1Point->Position(), nextPoint->Position());
						if(cur0dist>kRealEpsilon)
						{
			// Even if not parallel				if((prevDir^nextDir)<kRealEpsilon)
							{
								// Move the 4 points toward the middle line
								TVector2 proj;
								Project( prevPoint->Position(), cur1Point->Position(), nextPoint->Position(), proj );
								TVector2 offset = proj-prevPoint->Position();
								offset.Normalize();
					
								const real32 prevdist = PointLineSqrDistance(cur0Point->Position(), cur1Point->Position(), nextPoint->Position());
								const real32 cur1dist = PointLineSqrDistance(cur1Point->Position(), cur0Point->Position(), prevPoint->Position());
								const real32 nextdist = PointLineSqrDistance(nextPoint->Position(), cur0Point->Position(), prevPoint->Position());

								const real32 dist = .5*sqrt( MC_Min(MC_Min(prevdist,cur0dist),MC_Min(cur1dist,nextdist)) );
								offset*=dist;
								prevPoint->OffsetPosition(offset);
								cur0Point->OffsetPosition(offset);
								cur1Point->OffsetPosition(-offset);
								nextPoint->OffsetPosition(-offset);
							}
						}
					}
				}

				prevPrevPoint = prevPoint;
				prevPoint = cur0Point;
				cur0Point = cur1Point;
				cur1Point = nextPoint;
				nextPoint = nextNextPoint;
			}
		} break;
	case eRectangleRoofFlatExtremsX:
		{	// create 2 flat areas at the extremities for a rectangle roof
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const real32 y = .5*(min.y + max.y);

			{	// Modify the X values
				fZoneSection[0].fSpinePoint->GetPosition().x = min.x;
				fZoneSection[1].fSpinePoint->GetPosition().x = min.x;
				fZoneSection[2].fSpinePoint->GetPosition().x = max.x;
				fZoneSection[3].fSpinePoint->GetPosition().x = max.x;
				fZoneSection[0].fSpinePoint->GetPosition().y = 
				fZoneSection[1].fSpinePoint->GetPosition().y = 
				fZoneSection[2].fSpinePoint->GetPosition().y = 
				fZoneSection[3].fSpinePoint->GetPosition().y = y;
			}
		} break;
	case eRectangleRoofFlatExtremsY:
		{	// create 2 flat areas at the extremities for a rectangle roof
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const real32 x = .5*(min.x + max.x);

			{	// Modify the Y values
				fZoneSection[0].fSpinePoint->GetPosition().y = min.y;
				fZoneSection[1].fSpinePoint->GetPosition().y = max.y;
				fZoneSection[2].fSpinePoint->GetPosition().y = max.y;
				fZoneSection[3].fSpinePoint->GetPosition().y = min.y;
				fZoneSection[0].fSpinePoint->GetPosition().x = 
				fZoneSection[1].fSpinePoint->GetPosition().x = 
				fZoneSection[2].fSpinePoint->GetPosition().x = 
				fZoneSection[3].fSpinePoint->GetPosition().x = x;
			}
		} break;
	case eHalfRectangleRoof1:
		{	// Create 3 falt areas
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const TVector2 corner0(min.x,max.y);
			const TVector2 corner1(max.x,max.y);

			{	// Modify the values
				fZoneSection[0].fSpinePoint->GetPosition() = corner0;
				fZoneSection[1].fSpinePoint->GetPosition() = corner0;			
				fZoneSection[2].fSpinePoint->GetPosition() = corner1;		
				fZoneSection[3].fSpinePoint->GetPosition() = corner1;
			}
		} break;
	case eHalfRectangleRoof2:
		{	// Create 3 falt areas
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const TVector2 corner0(min.x,min.y);
			const TVector2 corner1(max.x,min.y);

			{	// Modify the values
				fZoneSection[0].fSpinePoint->GetPosition() = corner0;
				fZoneSection[1].fSpinePoint->GetPosition() = corner0;			
				fZoneSection[2].fSpinePoint->GetPosition() = corner1;		
				fZoneSection[3].fSpinePoint->GetPosition() = corner1;
			}
		} break;
	case eHalfRectangleRoof3:
		{	// Create 3 falt areas
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const TVector2 corner0(min.x,min.y);
			const TVector2 corner1(min.x,max.y);

			{	// Modify the values
				fZoneSection[0].fSpinePoint->GetPosition() = corner0;
				fZoneSection[1].fSpinePoint->GetPosition() = corner1;			
				fZoneSection[2].fSpinePoint->GetPosition() = corner1;		
				fZoneSection[3].fSpinePoint->GetPosition() = corner0;
			}
		} break;
	case eHalfRectangleRoof4:
		{	// Create 3 falt areas
			MY_ASSERT( fZoneSection.GetElemCount()==4 );

			const TVector2& min = fZoneSection[0].fZonePoint->Position();
			const TVector2& max = fZoneSection[2].fZonePoint->Position();

			const TVector2 corner0(max.x,min.y);
			const TVector2 corner1(max.x,max.y);

			{	// Modify the values
				fZoneSection[0].fSpinePoint->GetPosition() = corner0;
				fZoneSection[1].fSpinePoint->GetPosition() = corner1;			
				fZoneSection[2].fSpinePoint->GetPosition() = corner1;		
				fZoneSection[3].fSpinePoint->GetPosition() = corner0;
			}
		} break;
	}

}

void RoofZone::DetermineZoneOrientation(Roof* roof)
{
	const int32 zoneCount = fZoneSection.GetElemCount();
	int32 posCount = 0;
	int32 negCount = 0;

	ZoneSection section = fZoneSection[zoneCount-1];
	for(int32 iZone=0 ; iZone<zoneCount ; iZone++)
	{
		const ZoneSection& nextSection = fZoneSection[iZone];

		const TVector3& center = section.fZonePoint->Get3DPos();
		const TVector3 dir0 = nextSection.fZonePoint->Get3DPos()-center;
		const TVector3 dir1 = section.fSpinePoint->Get3DPos()-center;

		if((dir0^dir1).z>=0)	posCount++;
		else					negCount++;

		section = nextSection;
	}

	if(posCount>negCount)	roof->ClearFlag(eRoofZoneOrientedPositive);
	else					roof->SetFlag(eRoofZoneOrientedPositive);
}

RoofZone& RoofZone::Copy(const RoofZone& fromZone, Roof* roof)
{
	if(&fromZone != this)
	{
		const TMCClassArray<ZoneSection>& fromArray = fromZone.ZoneArray();
	
		const int32 sectionCount = fromArray.GetElemCount();
		fZoneSection.SetElemSpace(sectionCount); 
		// Then replace the pointers to one roof by the other
		for(int32 iPt=0 ; iPt<sectionCount ; iPt++)
		{
			ZoneSection& newSection = fZoneSection.AddElem();

			RoofPoint* fromZonePoint = fromArray[iPt].fZonePoint;
			RoofPoint* fromSpinePoint = fromArray[iPt].fSpinePoint;

			RoofPoint::CreateRoofPoint(&(newSection.fZonePoint),roof->GetData(),fromZonePoint->GetPosition(),roof, false);
			RoofPoint::CreateRoofPoint(&(newSection.fSpinePoint),roof->GetData(),fromSpinePoint->GetPosition(),roof, true);

			// Copy the point flags
			newSection.fZonePoint->SetFlags(fromZonePoint->GetFlags());
			newSection.fZonePoint->ClearFlag(eIsTargeted);
			newSection.fSpinePoint->SetFlags(fromSpinePoint->GetFlags());
			newSection.fSpinePoint->ClearFlag(eIsTargeted);
		}
	}
	return *this;
}

MCCOMErr RoofZone::Read(IShTokenStream* stream, Roof* roof)
{ 
	fZoneSection.ArrayFree();

	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	int32 index = 0;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'Coun':  
			{
				int32 ptCount=stream->GetInt32Token();
				fZoneSection.SetElemCount(ptCount);
				index=0;
				break;
			}
		case 'Sect':  
			{
				fZoneSection[index++].Read(stream, roof);
			} break;
			 
		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

MCCOMErr ZoneSection::Read(IShTokenStream* stream, Roof* roof)
{
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'Poin':  
			{	
				if(!fZonePoint)
				{
					RoofPoint::CreateRoofPoint(&fZonePoint,roof->GetData(),TVector2::kZero,roof, false);
					fZonePoint->Read(stream);
				}
				else
				{
					RoofPoint::CreateRoofPoint(&fSpinePoint,roof->GetData(),TVector2::kZero,roof, true);
					fSpinePoint->Read(stream);
				}
				break;
			}
		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;

}
/****************************************************************************************************

		Utils.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/31/2004

****************************************************************************************************/

#include "copyright.h"
#include "Utils.h"
struct IMCGraphicContext;
struct IMFResponder;
class TMCRect;
#include "I3DEditorHostPart.h"
#include "MCPoint.h"
#include "Geometry.h"
#include "I3DShObject.h"
#if (VERSIONNUMBER >= 0x050000)
#include "IShPartUtilities.h"
#endif




// Transform a point from the base (x,y,y)
// into the 2D plane (O,I,J), with K normal to this plane
void Base3ToBase2(const TVector3& point,
				  const TVector3& O,
				  const TVector3& I,
				  const TVector3& J,
				  const TVector3& K,
				  TVector2& result)
{
	// First project the point on the plane
	TVector3 projection;
	Project( point, O, K, projection );

	// Then express the 2 coordinates in the plane
	int32 X=0, Y=1, Z=2;
	// find a non null denom
	real64 denom = I[X]*J[Y]-I[Y]*J[X];
	if( RealAbs(denom)<kRealEpsilon )
	{
		X=1; Y=2; Z=0;
		denom = I[X]*J[Y]-I[Y]*J[X];
		if( RealAbs(denom)<kRealEpsilon )
		{
			X=2; Y=0; Z=1;
			denom = I[X]*J[Y]-I[Y]*J[X];
		}
	}

	result[0] = ( J[Y]*(point[X]-O[X] ) - J[X]*(point[Y]-O[Y]) ) / denom;
	result[1] = ( I[X]*(point[Y]-O[Y] ) - I[Y]*(point[X]-O[X]) ) / denom;
}

boolean TrianglePick(const TVector3 pos0,
					 const TVector3 pos1,
					 const TVector3 pos2,
					 real &alpha, const FatRay& fatRay, TVector3& hitPosition) 
{
	const TVector3 a=pos1-pos0;
	const TVector3 b=pos2-pos0;
	const TVector3 n=a^b;
	const real64 denominator=n*fatRay.fDirection;

	if (denominator == kRealZero)
		return false;

	alpha=((pos0-fatRay.fOrigin)*n)/denominator;
	const TVector3 hp=fatRay.fOrigin+fatRay.fDirection*alpha;
	const TVector3 q=hp-pos0;
	const TVector3 c=a^n;
	const real64 denominator2=b*c;
	if (denominator2 == kRealZero)
		return false;

	const real64 gamma=c*q/denominator2;
	const real64 denominator3=a*a;
	if (denominator3 == kRealZero)
		return false;

	const real64 beta=(q*a-(b*a)*gamma)/denominator3;
	if (beta < kRealZero || gamma < kRealZero || beta+gamma > kRealOne)
		return false;

	// Eliminate hitpos behind
	if ( ((hp-fatRay.fOrigin)*fatRay.fDirection) < kRealZero)
		return false;

	hitPosition=hp;
	return true;
}

boolean BasicTrianglePick(	const TVector3 pos0,
							const TVector3 pos1,
							const TVector3 pos2,
							real64 &alpha,
							const TVector3& origin,
							const TVector3& direction,
							TVector3& hitPosition ) 
{
	const TVector3 a=pos1-pos0;
	const TVector3 b=pos2-pos0;
	const TVector3 n=a^b;
	const real64 denominator=n*direction;

	if (denominator == kRealZero)
		return false;

	alpha=((pos0-origin)*n)/denominator;
	const TVector3 hp=origin+direction*alpha;
	const TVector3 q=hp-pos0;
	const TVector3 c=a^n;
	const real64 denominator2=b*c;
	if (denominator2 == kRealZero)
		return false;

	const real64 gamma=c*q/denominator2;
	const real64 denominator3=a*a;
	if (denominator3 == kRealZero)
		return false;

	const real64 beta=(q*a-(b*a)*gamma)/denominator3;
	if (beta < kRealZero || gamma < kRealZero || beta+gamma > kRealOne)
		return false;

	// Eliminate hitpos behind
	if ( ((hp-origin)*direction) < kRealZero)
		return false;

	hitPosition=hp;
	return true;
}

void MakeRayPlanes(	TMCArray<Plane> &rayPlanes,
					I3DEditorHostPanePart* panePart,
					const TMCPoint& min,
					const TMCPoint& max,
				   	const TVector3& origin,
					const TVector3& direction)
{
	TMCPoint corner;

	//	top left
	corner.x=min.x; corner.y=min.y;
	TVector3 directionTL,originTL;
	panePart->PixelRay(corner,originTL, directionTL);

	//	top right
	corner.x=max.x;
	TVector3 directionTR,originTR;
	panePart->PixelRay(corner,originTR, directionTR);

	//	bottom right
	corner.y=max.y;
	TVector3 directionBR,originBR;
	panePart->PixelRay(corner,originBR, directionBR);

	//	bottom left
	corner.x=min.x;
	TVector3 directionBL,originBL;
	panePart->PixelRay(corner,originBL, directionBL);

	//	top clip plane
	Plane rayPlane;
	const TVector3 tL=originTL+directionTL;
	const TVector3 tR=originTR+directionTR;
	const TVector3 top((originTL+originTR)*kRealOneHalf);
	rayPlane.fPoint=top;
	rayPlane.fNormal=(tL-top)^(tR-top);
	rayPlanes.AddElem(rayPlane);

	//	bottom clip plane
	const TVector3 bL=originBL+directionBL;
	const TVector3 bR=originBR+directionBR;
	const TVector3 bottom((originBL+originBR)*kRealOneHalf);
	rayPlane.fPoint=bottom;
	rayPlane.fNormal=(bR-bottom)^(bL-bottom);
	rayPlanes.AddElem(rayPlane);

	//	left clip plane
	const TVector3 left((originBL+originTL)*kRealOneHalf);
	rayPlane.fPoint=left;
	rayPlane.fNormal=(bL-left)^(tL-left);
	rayPlanes.AddElem(rayPlane);

	//	right clip plane
	const TVector3 right((originBR+originTR)*kRealOneHalf);
	rayPlane.fPoint=right;
	rayPlane.fNormal=(tR-right)^(bR-right);
	rayPlanes.AddElem(rayPlane);

	// we clip by the plane of the camera for perspective camera only
	if (directionBL != directionTR)
	{
		rayPlane.fPoint=origin;
		rayPlane.fNormal=direction;
		rayPlanes.AddElem(rayPlane);
	}
}

boolean PointIsInTriangleArray(const TVector2& point, 
							   const TMCClassArray<Triangle>& triangleArray,
							   const TMCClassArray<TVector2>& positions)
{
	const int32 facetCount = triangleArray.GetElemCount();
	for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
	{
		const Triangle& triangle = triangleArray[iFacet];
		if( PointIsInTriangle(	point,
								positions[triangle.pt1], 
								positions[triangle.pt2], 
								positions[triangle.pt3] ) )
		{
			return true;
		}
	}

	return false;
}

TVector2 BissectDir2D(const TVector2& center,const TVector2& pt0,const TVector2& pt1)
{
	TVector2 vec0 = pt0-center;
	vec0.Normalize();
	TVector2 vec1 = pt1-center;
	vec1.Normalize();

	const TVector2 sum = vec0+vec1;
	if(sum.GetSquaredNorm()>kRealEpsilon) // !=TVector3::kZero
		return sum;
	else
		return TVector2(-vec0.y,vec0.x); // 3 points aligned case
}

TVector3 BissectDir2(TVector3 dir0, TVector3 dir1)
{
	dir0.Normalize();
	dir1.Normalize();

	const TVector3 sum = dir0+dir1;
	if(sum.GetSquaredNorm()>kRealEpsilon) // !=TVector3::kZero
		return sum;
	else
	{	// 3 points aligned case
		if(RealAbs(dir0.x)>0 || RealAbs(dir0.y)>0)
			return TVector3(-dir0.y,dir0.x,dir0.z);
		else
			return TVector3::kUnitX;
	}
}

TVector3 BissectDir1(const TVector3& center,const TVector3& pt0,const TVector3& pt1)
{
	TVector3 vec0 = pt0-center;
	vec0.Normalize();
	TVector3 vec1 = pt1-center;
	vec1.Normalize();

	const TVector3 sum = vec0+vec1;
	if(sum.GetSquaredNorm()>kRealEpsilon) // !=TVector3::kZero
		return sum;
	else
	{	// 3 points aligned case
		if(RealAbs(vec0.x)>0 || RealAbs(vec0.y)>0)
			return TVector3(-vec0.y,vec0.x,vec0.z);
		else
			return TVector3::kUnitX;
	}
}

//
boolean SegmentIntersectPolygonSegment(	const TVector2& pt0,  const TVector2& pt1,
										const TMCClassArray<TVector2>& polygon )
{
	if (pt0 == pt1)
		return false;

	const int32 pointCount = polygon.GetElemCount();
	TVector2 prev = polygon[pointCount-1];
    for (int32 iPt=0; iPt < pointCount; iPt++)  // process polygon edge V[i]V[i+1]
    {
		const TVector2& cur = polygon[iPt];
		if( cur!=pt0 && cur!=pt1 &&
			prev!=pt0 && prev!=pt1) // avoid the case where one of the points is a point of the polygon
		{
			if( Intersect( pt0, pt1, prev, cur) )
				return true;
		}
				
		prev = cur;
	}

	return false;
}

// In 2D, work only for convex polygons
boolean SegmentIntersectPolygon(const TVector2& pt0,  const TVector2& pt1,
								const TMCClassArray<TVector2>& polygon )
{
    if (pt0 == pt1) 
	{	// the segment S is a single point
        // test for inclusion of S.P0 in the polygon
        return false; // TO DO cn_PnPoly( S.P0, V, n );  // March 2001 Algorithm
    }

    real64  tE = 0;             // the maximum entering segment parameter
    real64  tL = 1;             // the minimum leaving segment parameter
    real64  t, N, D;            // intersect parameter t = N / D
    TVector2 dS = pt1 - pt0;   // the segment direction vector
    // Vector ne;              // edge outward normal (not explicit in code)

	const int32 pointCount = polygon.GetElemCount();
	TVector2 prev = polygon[pointCount-1];
    for (int32 iPt=0; iPt < pointCount; iPt++)  // process polygon edge V[i]V[i+1]
    {
		const TVector2& cur = polygon[iPt];
		TVector2 edge = cur - prev;// edge vector
		N = -edge*(pt0-prev);	// = -dot(ne, S.P0-V[i])
        D = edge*dS;      // = dot(ne, dS)
        if (RealAbs(D) < kRealEpsilon) { // S is nearly parallel to this edge
            if (N < 0)             // P0 is outside this edge, so
                return false;      // S is outside the polygon
            else					// S cannot cross this edge, so
            {						// ignore this edge
				prev = cur;
				continue; 
			}
        }

        t = N / D;
        if (D < 0) {           // segment S is entering across this edge
            if (t > tE) {      // new max tE
                tE = t;
                if (tE > tL)   // S enters after leaving polygon
                    return false;
            }
        }
        else {                 // segment S is leaving across this edge
            if (t < tL) {      // new min tL
                tL = t;
                if (tL < tE)   // S leaves before entering polygon
                    return false;
            }
        }
    }

    // tE <= tL implies that there is a valid intersection subsegment
//    IS->P0 = S.P0 + tE * dS;   // = P(tE) = point where S enters polygon
//    IS->P1 = S.P0 + tL * dS;   // = P(tL) = point where S leaves polygon
    return true;
}

// In 2D, work only for convex polygons
boolean SegmentIntersectPolygon(const TVector2& pt0,  const TVector2& pt1,
								const TMCClassArray<TVector2>& polygon, const boolean backward,
								int32& ptOut0,  int32& ptOut1, TVector2& intersectionPt )
{
	const int32 pointCount = polygon.GetElemCount();

	if(backward)
	{
		TVector2 prev = polygon[0];
		for (int32 iPt=pointCount-1; iPt >= 0; iPt--)
		{
			const TVector2& cur = polygon[iPt];

			if( GetSegmentsIntersection( intersectionPt, pt0, pt1, prev, cur ) )
			{
				ptOut0 = (iPt-1+pointCount)%pointCount;
				ptOut1 = iPt;
				return true;
			}
		}
	}
	else
	{
		TVector2 prev = polygon[pointCount-1];
		for (int32 iPt=0; iPt < pointCount; iPt++)
		{
			const TVector2& cur = polygon[iPt];

			if( GetSegmentsIntersection( intersectionPt, pt0, pt1, prev, cur ) )
			{
				ptOut0 = (iPt-1+pointCount)%pointCount;
				ptOut1 = iPt;
				return true;
			}
		}
	}

    return false;
}

// http://paulbourke.net/geometry/clockwise/index.html
// If the area is >0, the points are in conterclockwise order
// If <0, clockwise order
double PolygonArea( const TMCClassArray<TVector2>& polygon )
{
	const int32 pointCount = polygon.GetElemCount();
	if( pointCount<=2 )
		return 0;

	TVector2 prevPoint = polygon[pointCount-1];
	double area=0;
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		const TVector2& curPoint = polygon[iPoint];
		
		area+=(prevPoint.x*curPoint.y)-(prevPoint.y*curPoint.x);

		prevPoint = curPoint;
	}

	area*=0.5;

	return area;
}

////////////////////////////////////////////////////////////////////////////
//
// Tree utils
//

//int32 GetSonCount(I3DShTreeElement* treeElement)
//{
//	TMCCountedPtr<I3DShTreeElement> sonTree;
//	treeElement->GetFirst(&sonTree);
//	if(!sonTree)
//		return 0;
//
//	int32 sonIndex = 0;
//
//	while(sonTree)
//	{
//		sonIndex++;
//		// Get the next son
//		sonTree->GetRight(&sonTree);		
//	}
//
//	return sonIndex;
//}
//
//int32 GetSonCountAndNames(I3DShTreeElement* treeElement, TMCClassArray<TMCDynamicString>& masterObjectNames)
//{
//	TMCCountedPtr<I3DShTreeElement> sonTree;
//	treeElement->GetFirst(&sonTree);
//	if(!sonTree)
//		return 0;
//
//	int32 sonIndex = 0;
//
//	while(sonTree)
//	{
//		sonIndex++;
//		// Get the master object name
//		TMCCountedPtr<I3DShInstance> sonInstance;
//		sonTree->QueryInterface(IID_I3DShInstance, (void**) &sonInstance); 
//		ThrowIfNil(sonInstance);
//		TMCCountedPtr<I3DShObject> masterObject;			
//		sonInstance->Get3DObject(&masterObject);
//		masterObject->GetName(masterObjectNames.AddElem());
//
//		// Get the next son
//		sonTree->GetRight(&sonTree);		
//	}
//
//	return sonIndex;
//}

////////////////////////////////////////////////////////////////////////////
//
// UI Utils
//

#include "MiscComUtilsImpl.h"
#include "IMFPart.h"
#include "IMFWindow.h"

boolean OpenDialog(IMFDialogPart** dialog, const IDType dialogID)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCCountedPtr<IMFPart> dialogPart;
	gPartUtilities->CreatePartByResource(&dialogPart, kMFDialogResourceType, dialogID);

	if(!dialogPart)
		return false;
	
	TMCCountedPtr<IMFWindow> theDialogWindow;
	dialogPart->QueryInterface(IID_IMFWindow, (void**)&theDialogWindow);
	if(!theDialogWindow)
		return false;

	theDialogWindow->Center(true, true);
	TMCCountedPtr<IMFDialogPart> theDialog;	
	dialogPart->QueryInterface(IID_IMFDialogPart, (void**)&theDialog);
	if(!theDialog)
		return false;

	if(MCVerify(dialog))
	{
		TMCCountedGetHelper<IMFDialogPart> result(dialog);
		result=theDialog;
		return true;
	}
	return false;
}

void SetDialogValue(IMFDialogPart* dialog, const real32 value, const uint32 stringID)
{
	TMCCountedPtr<IMFPart> theDialogPart;
	dialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
	TMCCountedPtr<IMFPart> stringPart;
	theDialogPart->FindChildPartByID(&stringPart, stringID);
	if (!stringPart)
		return;

	stringPart->SetValue( (void*)&value, kReal32ValueType, true, false);
}

void SetDialogString(IMFDialogPart* dialog, const TMCString& string, const uint32 stringID)
{
	TMCCountedPtr<IMFPart> theDialogPart;
	dialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
	TMCCountedPtr<IMFPart> stringPart;
	theDialogPart->FindChildPartByID(&stringPart, stringID);
	if (!stringPart)
		return;

	stringPart->SetValue( (void*)&string, kStringValueType, true, false);
}

boolean GetDialogString(IMFDialogPart* dialog, TMCString& string, const uint32 stringID)
{
	TMCCountedPtr<IMFPart> theDialogPart;
	dialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
	TMCCountedPtr<IMFPart> stringPart;
	theDialogPart->FindChildPartByID(&stringPart, stringID);
	if (!stringPart)
		return false;

	stringPart->GetValue( (void*)&string, kStringValueType);
	return true;
}

boolean GetDialogValue(IMFDialogPart* dialog, real32& value, const uint32 stringID)
{
	TMCCountedPtr<IMFPart> theDialogPart;
	dialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
	TMCCountedPtr<IMFPart> valuePart;
	theDialogPart->FindChildPartByID(&valuePart, stringID);
	if (!valuePart)
		return false;

	valuePart->GetValue( (void*)&value, kReal32ValueType);
	return true;
}

boolean GetDialogValue(IMFDialogPart* dialog, int32& value, const uint32 stringID)
{
	TMCCountedPtr<IMFPart> theDialogPart;
	dialog->QueryInterface(IID_IMFPart, (void**)&theDialogPart);
	TMCCountedPtr<IMFPart> stringPart;
	theDialogPart->FindChildPartByID(&stringPart, stringID);
	if (!stringPart)
		return false;

	stringPart->GetValue( (void*)&value, kInt32ValueType);
	return true;
}

// Debug method
boolean CheckVertexPos(const TMCClassArray<Vertex>& vertices)
{
	const int32 vtxCount = vertices.GetElemCount();

	for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		const TVector3&	pos = vertices[iVtx].Position();

		if(pos.x>10000) return false;
		if(pos.y>10000) return false;
		if(pos.z>10000) return false;
		if(pos.x<-10000) return false;
		if(pos.y<-10000) return false;
		if(pos.z<-10000) return false;
	}

	return true;
}

// Debug method
boolean CheckTriangles(const TMCClassArray<Triangle>& triangles,
					   const TMCClassArray<Vertex>& vertices )
{
	const uint32 maxIndex = vertices.GetElemCount();

	const int32 tglCount = triangles.GetElemCount();

	for( int32 iTgl=0 ; iTgl<tglCount ; iTgl++ )
	{
		const Triangle& triangle = triangles[iTgl];
		if( triangle.pt1>=maxIndex ||
			triangle.pt2>=maxIndex || 
			triangle.pt3>=maxIndex )
			return false;
	}

	return true;
}


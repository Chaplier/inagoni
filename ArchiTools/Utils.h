/****************************************************************************************************

		Utils.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/31/2004

****************************************************************************************************/

#ifndef __Utils__
#define __Utils__

#if CP_PRAGMA_ONCE
#pragma once
#endif


#if WIN32
#include <assert.h>
#define MY_ASSERT(e)			( assert(e) );
#else // assert crashes the application on Mac
#define MY_ASSERT(e)
#endif

#include "MCAssert.h"
#include "Vector2.h"
#include "I3dShFacetMesh.h" // For the class Triangle
#include "MCClassArray.h"
#include "PVertex.h"

// Constants
static const real32 kBigRealValue = 1e20f;
static const real32 kNearlyOne = kRealOne-kRealEpsilon;

struct I3DEditorHostPanePart;

inline bool IsEven(int intValue)
{
  return ((intValue & 1) == 0);
}

inline bool IsOdd(int intValue)
{
  return ((intValue & 1) == 1);
}

struct Line2D
{
	TVector2 fPoint;
	TVector2 fDir;
};

inline TMCColorRGBA8 GetFacetedColor(const TVector3& normal, 
									const TMCColorRGBA8& fromColor)
{
	//	modeler illumination model
	real32 shading=0.8f+0.05f*normal[0]+0.05f*normal[1]+0.2f*normal[2];
	if (shading>kRealOne) shading=kRealOne;

	return fromColor*shading;
}
inline TMCColorRGBA8 GetFacetedBackColor(const TVector3& normal, 
									const TMCColorRGBA8& fromColor)
{
	//	modeler illumination model
	real32 shading=0.8f-0.05f*normal[0]-0.05f*normal[1]-0.2f*normal[2];
	if (shading>kRealOne) shading=kRealOne;

	return fromColor*shading;
}

inline real32 Snap( const real32 pos, const int32 divisions, const real32 length)
{
	return (real32)(((int32)((pos*divisions)/length + .5)) * length/divisions);
}

inline real32 Proj(const TVector2& vec0, const TVector2& vec1)
{
	return (vec0.x*vec1.x+vec0.y*vec1.y);
}

TVector2 BissectDir2D(const TVector2& center,const TVector2& pt0,const TVector2& pt1);

TVector3 BissectDir1(const TVector3& center,const TVector3& pt0,const TVector3& pt1);

TVector3 BissectDir2(TVector3 dir0, TVector3 dir1);

inline bool ArePointsAligned(const TVector2& point0, const TVector2& point1, const TVector2& point2)
{
	const TVector2 axe = point2-point1;
	const float value1 = (axe^(point0-point1));

	if(value1==0)
		return true;

	return false;
}

inline real32 GetPositiveAngle(const TVector2& vec1, const TVector2& vec2)
{
	const real32 cos = vec1 * vec2;
	const real32 sin = vec1 ^ vec2;
	real32 angle = 0;
	RealArcSinCos(sin,cos,angle);
	if(angle<0) angle+=360;

	return angle;
}

// return >0 if one point on each side 
// return =0 if both points on same side
// return <0 if one or two point is on the axis
enum ESideType
{
	eBothSides,
	eSameSide,
	ePoint1Aligned,
	ePoint2Aligned,
	eFourPointsAligned
};

inline ESideType ArePointsOnBothSides(const TVector2& axePoint1, const TVector2& axePoint2,
									const TVector2& point1, const TVector2& point2)
{
	TVector2 axe = axePoint2-axePoint1;
	const float value1 = (axe^(point1-axePoint1));
	const float value2 = (axe^(point2-axePoint1));

	if(value1==0)
	{
		if(value2==0)
			return eFourPointsAligned;
		else
			return ePoint1Aligned;
	}
	else if(value2==0)
		return ePoint2Aligned;
	else if( value1*value2 > 0)
		return eSameSide; // the 2 points are on the same side

	else return eBothSides; // one point on each side 
}

enum EIntersectType
{
	eNoIntersection,
	eA1Aligned, // A1 is between B points.
	eA2Aligned, // A2 is between B points.
	eB1Aligned, // B1 is between A points.
	eB2Aligned, // B2 is between A points.
	eFourPointAligned, // maybe no intersection
	eIntersection
};
// return true if there's an intersection
// Warning: this doesn't work with points aligned: it returns No Intersection
inline EIntersectType Intersect(	const TVector2& APoint1, const TVector2& APoint2,
							const TVector2& BPoint1, const TVector2& BPoint2)
{
	const ESideType sideType1 = ArePointsOnBothSides(APoint1,APoint2,BPoint1,BPoint2);
	if(sideType1 == eFourPointsAligned )
		return eFourPointAligned;

	const ESideType sideType2 = ArePointsOnBothSides(BPoint1,BPoint2,APoint1,APoint2);
	if(sideType2 == eFourPointsAligned )
		return eFourPointAligned;

	if(sideType1==eSameSide||sideType2==eSameSide)
		return eNoIntersection;

	if(sideType1==eBothSides)
	{
		if(sideType2==eBothSides)
			return eIntersection;
		else if(sideType2==ePoint1Aligned)
			return eB1Aligned;
		else if(sideType2==ePoint2Aligned)
			return eB2Aligned;
	}
	else if(sideType1==ePoint1Aligned)
	{
		return eA1Aligned;
	}
	else if(sideType1==ePoint2Aligned)
	{
		return eA2Aligned;
	}
	
	return eNoIntersection;
}

// return the intersection of 2 segments. 
// Need improvment
inline EIntersectType GetSegmentsIntersection( TVector2& result,
							const TVector2& A, const TVector2& C, // first segment
							const TVector2& B, const TVector2& D ) // second segment
{
	EIntersectType intersectType = Intersect(A,C,B,D);
	if(intersectType == eIntersection)
	{
		const TVector2 DB = B-D;
		const TVector2 CA = A-C;
		
		const real32 denom = DB.y*CA.x - CA.y*DB.x;
		if(denom!=0)
		{
			const real32 coef1 = A.x*C.y-C.x*A.y;
			const real32 coef2 = B.x*D.y-D.x*B.y;
			result.x = ( coef1*DB.x - coef2*CA.x )/denom;
			result.y = ( coef1*DB.y - coef2*CA.y )/denom;
			return intersectType;
		}
	}
	else if(intersectType == eA1Aligned)
	{
		result = A;
		return intersectType;
	}
	else if(intersectType == eA2Aligned)
	{
		result = C;
		return intersectType;
	}
	else if(intersectType == eB1Aligned)
	{
		result = B;
		return intersectType;
	}
	else if(intersectType == eB2Aligned)
	{
		result = D;
		return intersectType;
	}
	else if(intersectType == eFourPointAligned)
	{
		return intersectType;
	}

	return eNoIntersection;
}

inline boolean IntersectSegmentLine(	const TVector2& P0, // first point of the segment
								const TVector2& P1, // second point of the segment
								const TVector2& V0, // point on the line
								const TVector2& dir, // dir of the line
								TVector2& result	)
{
	const TVector2 normal( dir.y, -dir.x );
	const TVector2 u = (P1-P0);

	// Check first if the segment is parallel to the line
	real64 scalar = normal*u;
	if(scalar == 0) return false; // No intersection

	real32 x=((normal*(V0-P0))/(scalar));

	result = P0 + x*u;

	if(x>=0 && x<=1) return true;
	else return false;
}

inline real32 PointSegmentSqrDistance(const TVector2& pos,
							const TVector2& p0,
							const TVector2& p1)
{
		const TVector2 p0p1 = p1-p0;
		const TVector2 posp0 = p0-pos;
		const TVector2 posp1 = p1-pos;
		if( p0p1*posp0>=0 )
			return posp0.GetSquaredNorm();
		else if( p0p1*posp1<=0 )
			return posp1.GetSquaredNorm();
		else
		{	// Distance Point-Line
			const real32 value = RealAbs(p0p1^posp0);
			return value*value/(p0p1.GetSquaredNorm());
		}
}

inline real32 PointLineSqrDistance(const TVector2& pos,
									const TVector2& p0,
									const TVector2& p1)
{
		const TVector2 p0p1 = p1-p0;
		const TVector2 posp0 = p0-pos;
		{	// Distance Point-Line
			const real32 value = RealAbs(p0p1^posp0);
			return value*value/(p0p1.GetSquaredNorm());
		}
}

inline real32 PointLineDistance(const TVector2& pos,
								const TVector2& linePos,
								const TVector2& lineDir) // normalized
{
	{	// Distance Point-Line
		return RealAbs(lineDir^(linePos-pos));
	}
}

// project a pos on the plane defined by (center,I,J)
inline TVector2 ProjectIn(TVector3 pos,const TVector3& center,const TVector3& I,const TVector3& J)
{
	pos-=center;
	return TVector2( I*pos, J*pos);
}

inline void Project(	const TVector3& point, //point
						const TVector3& O, // point on the plane
						const TVector3& normal, // normal to the plane
						TVector3& result	)
{
	result = point + ( normal*(O-point) )*normal;
}

inline void ProjectOnLine(	const TVector3& point, //point
						const TVector3& pointOnLine0, // 
						const TVector3& pointOnLine1, // 
						TVector3& result	)
{
	TVector3 u = pointOnLine1-pointOnLine0;
	const TVector3 v = point - pointOnLine0;
	u.Normalize();

	result = (u * v) * u;
}

inline bool ArePointsAligned(	const TVector3& point0, //point
						const TVector3& point1, // 
						const TVector3& point2 )
{
	TVector3 u = point0-point1;
	const TVector3 v = point0-point2;
	if( !u.Normalize() )
		return true;

	real64 dotProduct =  u * v;
	if( dotProduct==v.GetMagnitude() )
		return true;

	return false;
}

// Transform a point from the base (x,y,y)
// into the 2D plane (O,I,J), with K normal to this plane
void Base3ToBase2(const TVector3& point,
				  const TVector3& O,
				  const TVector3& I,
				  const TVector3& J,
				  const TVector3& K,
				  TVector2& result);

inline void Project(	const TVector2& point, //point
						const TVector2& p0, // 1st point
						const TVector2& p1, // 2nd point
						TVector2& result	)
{
	TVector2 dir=p1-p0;
	dir.Normalize();
	result = p0 + (dir*(point-p0))*dir;
}

inline void Project2(	const TVector2& point, //point
						const TVector2& linePos, // line point
						const TVector2& lineDir, // normalize
						TVector2& result	)
{
	result = linePos + (lineDir*(point-linePos))*lineDir;
}

inline boolean IntersectLineLine(	const TVector2& P0, // first point of the line
									const TVector2& P1, // second point of the line
									const TVector2& V0, // point on the 2nd line
									const TVector2& dir, // dir of the 2nd line
									TVector2& result	)
{
	const TVector2 normal( dir.y, -dir.x );
	const TVector2 u = (P1-P0);

	// Check first if the segment is parallel to the line
	real64 scalar = normal*u;
	if(scalar == 0) return false; // No intersection

	real32 x=((normal*(V0-P0))/(scalar));

	result = P0 + x*u;

	return true;
}

inline boolean IntersectLineLine2(	const TVector2& pt0, // point on the 1st line
									const TVector2& dir0, // dir of the 1st line
									const TVector2& pt1, // point on the 2nd line
									const TVector2& dir1, // dir of the 2nd line
									TVector2& result	)
{
	const TVector2 normal( dir1.y, -dir1.x );

	// Check first if the segment is parallel to the line
	real64 scalar = normal*dir0;
	if(scalar == 0) return false; // No intersection

	real32 x=((normal*(pt1-pt0))/(scalar));

	result = pt0 + x*dir0;

	return true;
}


inline TVector2 ScalePoint(const TVector2& pt,const TVector2& scale,const TVector2& center)
{
	TVector2 result=pt;
	result-=center;
	result.x*=scale.x;
	result.y*=scale.y;
	result+=center;
	return result;
}

inline TVector2 RotatePoint(const TVector2& pt,const TVector2& cosSin,const TVector2& center)
{
	TVector2 result=pt;
	result-=center;
	TVector2 normal(-result.y,result.x);
	result = cosSin.x*result + cosSin.y*normal;
	result+=center;
	return result;
}

inline boolean SameSide(const TVector2& p1, const TVector2& p2,
						const TVector2& a, const TVector2& b )
{
	const TVector2 ab = b-a;
    const real32 cp1 = ab^(p1-a);
    const real32 cp2 = ab^(p2-a);
   if ( (cp1*cp2) >= 0 )
		return true;
    else
		return false;
}

inline boolean PointIsInTriangle( const TVector2& p, 
								  const TVector2& a, 
								  const TVector2& b,
								  const TVector2& c )
{
    if( SameSide(p,a,b,c) &&
		SameSide(p,b,a,c) &&
        SameSide(p,c,a,b) )
		return true;
    else
		return false;
}

boolean PointIsInTriangleArray(const TVector2& point, 
							   const TMCClassArray<Triangle>& triangleArray,
							   const TMCClassArray<TVector2>& positions);		

// A plane define by a point and a normal
struct Plane
{
	TVector3 fPoint;
	TVector3 fNormal;
};

struct FatRay
{
	TVector3 fDirection;
	TVector3 fOrigin;
	TMCArray<Plane> fRayPlanes;			// large zone arround the selection point
	TMCArray<Plane> fRayPlanesPrecise;	// smaller zone: if something is found in it, it's going to be prioritary
										// over others objects in the area.
};

void MakeRayPlanes(	TMCArray<Plane> &rayPlanes,
					I3DEditorHostPanePart* panePart,
					const TMCPoint& min,
					const TMCPoint& max,
				   	const TVector3& origin,
					const TVector3& direction);

boolean TrianglePick(const TVector3 pos0,
					 const TVector3 pos1,
					 const TVector3 pos2,
					 real &alpha, const FatRay& fatRay, TVector3& hitPosition);

boolean BasicTrianglePick(	const TVector3 pos0,
							const TVector3 pos1,
							const TVector3 pos2,
							real64 &alpha,
							const TVector3& origin,
							const TVector3& direction,
							TVector3& hitPosition ); 
/*
inline void Get3DBase0(	const real32	distToGround,
					const TVector2& zonePoint0,
					const TVector2& zonePoint1,
					TVector3& O, TVector3& I, TVector3& J, TVector3& K )
{
	O.SetFromXY(.5*(zonePoint0+zonePoint1), distToGround);
	I.SetFromXY(zonePoint1-zonePoint0, 0);
	I.Normalize();
	J.x = I.y;
	J.y = -I.x;
	J.z = 0;
	K = TVector3::kUnitZ;
}
*/
inline real32 GetRoofBase(const TVector3& curZonePos, const TVector3& nextZonePos,
					   const real32 fact, // Orientation
					   TVector3& direction, 
					   TVector3& inNormal, // normal inside
					   TVector3& center )
{
	direction = nextZonePos-curZonePos;
	const real32 length = direction.Normalize(direction);

	inNormal.x = -fact*direction.y;
	inNormal.y = fact*direction.x;
	inNormal.z = 0; 

	center = .5*(curZonePos+nextZonePos);

	return length;
}

inline void Get3DBase1(	const real32 topToGround,
					const TVector2& zonePoint0,
					const TVector2& zonePoint1,
					const TVector2& zoneSpine0,
					const TVector2& zoneSpine1,
					TVector3& O, TVector3& I, TVector3& J, TVector3& K )
{
	O.SetFromXY(.5*(zoneSpine0+zoneSpine1), topToGround);
	if(zoneSpine0==zoneSpine1)
	{
		I.SetFromXY(zonePoint1-zonePoint0, 0);
	}
	else
	{
		I.SetFromXY(zoneSpine1-zoneSpine0, 0);
	}
	I.Normalize();
	J.x = I.y;
	J.y = -I.x;
	J.z = 0;
	K = TVector3::kUnitZ;
}

inline boolean BBoxIntersect(const TBBox3D& bb1,const TBBox3D& bb2)
{
	return (	bb1.fMin[0] <= bb2.fMax[0]
			&&	bb1.fMin[1] <= bb2.fMax[1]
			&&	bb1.fMin[2] <= bb2.fMax[2]
			&&	bb1.fMax[0] >= bb2.fMin[0]
			&&	bb1.fMax[1] >= bb2.fMin[1]
			&&	bb1.fMax[2] >= bb2.fMin[2]
			);
}

// Intersection between a sement and one of the edges of the polygon
boolean SegmentIntersectPolygonSegment(	const TVector2& pt0,  const TVector2& pt1,
										const TMCClassArray<TVector2>& polygon );

// Works only with convex polygons
boolean SegmentIntersectPolygon(const TVector2& pt0,  const TVector2& pt1,
								const TMCClassArray<TVector2>& polygon );
// Works with all kind of polygons
boolean SegmentIntersectPolygon(const TVector2& pt0,  const TVector2& pt1,
								const TMCClassArray<TVector2>& polygon, const boolean backward,
								int32& ptOut0,  int32& ptOut1, TVector2& intersectionPt );

// Works with all kind of polygons
// Counter clockwise order: area is >0
// Clockwise ordre: area is <0
double PolygonArea( const TMCClassArray<TVector2>& polygon );


////////////////////////////////////////////////////////////////////////////
//
// Polyline utils
//
boolean BuildArc(TMCClassArray<TVector2>& arc, 
			  const TVector2& from, const TVector2& to,
			  int pointCount, float curve);

////////////////////////////////////////////////////////////////////////////
//
// Tree utils
//
#include "I3DShTreeElement.h"

//int32 GetSonCount(I3DShTreeElement* treeElement);
//int32 GetSonCountAndNames(I3DShTreeElement* treeElement, TMCClassArray<TMCDynamicString>& masterObjectNames);

////////////////////////////////////////////////////////////////////////////
//
// UI Utils
//

#include "IMFDialogPart.h"

boolean	OpenDialog(IMFDialogPart** dialog, const IDType dialogID);
void	SetDialogValue(IMFDialogPart* dialog, const real32 value, const uint32 stringID);
void	SetDialogString(IMFDialogPart* dialog, const TMCString& string, const uint32 stringID);
boolean	GetDialogString(IMFDialogPart* dialog, TMCString& string, const uint32 stringID);
boolean GetDialogValue(IMFDialogPart* dialog, real32& value, const uint32 stringID);
boolean GetDialogValue(IMFDialogPart* dialog, int32& value, const uint32 stringID);

////////////////////////////////////////////////////////////////////////////
//
// Debug Utils
//

boolean CheckVertexPos(const TMCClassArray<Vertex>& vertices);
boolean CheckTriangles(const TMCClassArray<Triangle>& triangles,
					   const TMCClassArray<Vertex>& vertices );

////////////////////////////////////////////////////////////////////////////
//
// Unit Utils
//
#include "mfunits.h"
/*
enum EUnits
{
	kUNIT_Default = -1,
	kUNIT_Pixels = 0,
	kUNIT_First = kUNIT_Pixels, 
	kUNIT_Points = 1,
	kUNIT_Picas = 2,
	kUNIT_Inches = 3,
	kUNIT_Feet = 4,
	kUNIT_Miles = 5,
	kUNIT_Millimeters= 6,
	kUNIT_Centimeters = 7,
	kUNIT_Meters = 8,
	kUNIT_Last = kUNIT_Meters, 
	kUNIT_Links = 9,
	kUNIT_Fathoms = 10
};
*/
inline real32 GetValueWithUnit(real32 inValue, int32 inUnits, boolean inRound)
{
	real32	num;

	num = inValue;

	switch ( inUnits )
	{
		case kUNIT_Pixels:
		case kUNIT_Points:
			break;
		case kUNIT_Picas:
			num *= (real32)(6.00);
			break;
		case kUNIT_Inches:
			num *= (real32)(1.00);
			break;
		case kUNIT_Feet:
			num *= (real32)(1.00 / (12.0));
			break;
		case kUNIT_Miles:
			num *= (real32)(1.00 / (5280.0 * 12.0));
			break;
		case kUNIT_Millimeters:
			num *= (real32)((2.54 * 10));
			break;
		case kUNIT_Centimeters:
			num *= (real32)((2.54 * 1));
			break;
		case kUNIT_Meters:
			num *= (real32)((2.54 / 100));
			break;
		case kUNIT_Links:
			num *= (real32)(1.00 / (7.92));		/*	1L = 7.92"	*/
			break;
		case kUNIT_Fathoms:
			num *= (real32)(1.00 / (12 * 6.0));	/*	1F = 6'		*/
			break;
	}

	if (inRound)
		num = (real32)(floor( num * 1000 + 0.5) / 1000);

	return(num);
}

inline real32 ConvertValue(real32 inValue, int32 inUnits, boolean inRound)
{
	real32	num;

	num = inValue;

	switch ( inUnits )
	{
		case kUNIT_Pixels:
		case kUNIT_Points:
			break;
		case kUNIT_Picas:
			num *= (real32)(1.00 / 6.00);
			break;
		case kUNIT_Inches:
			num *= (real32)(1.00);
			break;
		case kUNIT_Feet:
			num *= (real32)(12.0);
			break;
		case kUNIT_Miles:
			num *= (real32)(5280.0 * 12.0);
			break;
		case kUNIT_Millimeters:
			num *= (real32)(1.00 / (2.54 * 10));
			break;
		case kUNIT_Centimeters:
			num *= (real32)(1.00 / (2.54 * 1));
			break;
		case kUNIT_Meters:
			num *= (real32)(1.00 / (2.54 / 100));
			break;
		case kUNIT_Links:
			num *= (real32)(7.92);		/*	1L = 7.92"	*/
			break;
		case kUNIT_Fathoms:
			num *= (real32)(12 * 6.0);	/*	1F = 6'		*/
			break;
	}

	if (inRound)
		num = (real32)(floor( num * 1000 + 0.5) / 1000);

	return(num);
}

#endif

/****************************************************************************************************

		PCircleArc.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	24/03/2007

****************************************************************************************************/

#include "PCircleArc.h"

#include "real.h"
#include "Utils.h"


CircleArc::CircleArc()
{
	mFrom = TVector2::kZero;
	mTo = TVector2::kZero;
	mPointCount = 0;
	mOffset = 0;
	mArcLength = 0;
	mCacheIsValid = false;
}

CircleArc::~CircleArc()
{
}

void CircleArc::Validate()
{
	if(mCacheIsValid)
		return;

	mCacheIsValid = true;

	mCachedArc.SetElemCount(mPointCount);
	mArcLength = 0;

	if(mPointCount<2)
		return;

	if(mPointCount == 2)
	{
		mCachedArc[0] = mFrom;
		mCachedArc[1] = mTo;
		mArcLength = (mFrom-mTo).GetNorm();

		mCachedRadius = -1; // Invalid arc
		mCachedCenter = mFrom; // Invalid arc
		return;
	}

	if(mOffset==0)
	{	// Special case: a straight line
		const TVector2 dir = mTo-mFrom;
		TVector2 step;
		mArcLength = dir.Normalize(step);
		mCachedArc[0] = mFrom;
		real32 stepLength = mArcLength/(real32)(mPointCount-1);
		step *= stepLength;
		for(int iPoint=1 ; iPoint<mPointCount-1 ; iPoint++)
		{
			mCachedArc[iPoint] = mCachedArc[iPoint-1] + step;
		}
		mCachedArc[mPointCount-1] = mTo;

		mCachedRadius = -1; // Invalid arc
		mCachedCenter = mFrom; // Invalid arc
		return;
	}

	// Center and radius of the circle:
	// Point on the normal to (mFrom, mTo) at a distance x.
	// We call:
	// l: mOffset
	// d: length/2
	// Then :
	// x = l*cos/(1-cos)
	// with:
	// cos = (1-l*l/d*d)/(1+l*l/d*d)
	// And the radius is X + l

	const TVector2 segment = mTo-mFrom;
	const real32 length = segment.GetMagnitude();

	const real32 halfLength = 0.5 * length;
	const real32 sqrHalfLength = halfLength*halfLength;
	const real32 sqrOffset = mOffset*mOffset;
	const real32 param = sqrOffset / sqrHalfLength;

	const real32 cos = (1-param)/(1+param);
	const real32 X = mOffset * cos / (1-cos);

	TVector2 normal(segment.y, -segment.x);
	normal.Normalize();

	mCachedCenter = 0.5 * (mTo + mFrom) - X * normal;
	mCachedRadius = X + mOffset;

	// Create the n points of the arc
	TVector2 dirOA = mFrom-mCachedCenter;
	TVector2 dirOB = mTo-mCachedCenter;

	dirOA.Normalize();
	dirOB.Normalize();

	// in degrees
	real32 teta = GetPositiveAngle(dirOA, dirOB);
	if(mOffset<0)
	{
		// negative angle (turn in the other direction)
		teta -= 360;
	}

	real32 alpha = teta / (mPointCount - 1);

	// in rad
	real32 alphaRad = alpha*kRealDegToRad;

	const real32 cosAlpha = RealCos(alphaRad);
	const real32 sinAlpha = RealSin(alphaRad);

	mCachedArc[0] = mFrom;

	TVector2 prev = mFrom;

	const int32 lastIndex = mPointCount-1;
	for(int iPoint=1 ; iPoint<lastIndex ; iPoint++)
	{
		// Rotate the previous point
		TVector2 tmp = prev - mCachedCenter;
		TVector2 result;
		result.x = tmp.x * cosAlpha - tmp.y * sinAlpha;
		result.y = tmp.x * sinAlpha + tmp.y * cosAlpha;
		result += mCachedCenter;

		mCachedArc[iPoint] = result;

		mArcLength += (prev-result).GetNorm();

		// Move to the next point
		prev = result;
	}

	mArcLength += (prev-mTo).GetNorm();

	MY_ASSERT(mArcLength>=length);
		
	mCachedArc[lastIndex] = mTo;
}

boolean CircleArc::GetPosOnArc(real32 abscissa, TVector2& position)
{
	const TMCClassArray<TVector2>& arc = GetArc();
	
	const int32 pointCount = arc.GetElemCount();

	if(pointCount<2)
		return false; // something's wrong

	if(abscissa<0)
	{
		// Get the first segment to get the tangent
		TVector2 direction = arc[0]-arc[1];
		direction.Normalize();
		position = arc[0] - abscissa*direction;
		return true;

	}
	else if(abscissa>mArcLength)
	{
		// Get the last segment to get the tangent
		TVector2 direction = arc[pointCount-1]-arc[pointCount-2];
		direction.Normalize();
		position = arc[pointCount-1] + (abscissa-mArcLength)*direction;
		return true;
	}
	else
	{
		real32 percent = abscissa/mArcLength;
		return GetPosOnArc(percent, position);
	}
}

boolean CircleArc::GetPosOnArcFromPercent(real32 percent, TVector2& position)
{
	const TMCClassArray<TVector2>& arc = GetArc();

	const int32 pointCount = arc.GetElemCount();

	if(pointCount<2)
		return false; // something's wrong

	// Some special cases

	if( percent<=0 )
	{
		position = arc[0];
		return true;
	}

	if( percent>=1 )
	{
		position = arc[pointCount-1];
		return true;
	}

	TVector2 prev = arc[0];
	real32 prevLength = 0;

	const real32 targetLength = percent * mArcLength;

	for(int32 iPoint=1 ; iPoint<pointCount ; iPoint++)
	{
		// check if the pos we're looking for is between these 2 points
		TVector2 cur = arc[iPoint];
		TVector2 segment = cur - prev;
		real32 segLength = segment.Normalize();

		if( targetLength<prevLength+segLength )
		{	// the position is on this segment

			real32 localLength = targetLength-prevLength;

			position = segment * localLength;
			return true;
		}

		prevLength+=segLength;
		prev = cur;
	}

	// Something went wrong
	position = arc[pointCount-1];
	return false;
}

boolean CircleArc::GetPercentFromArc(int32 onSegment, real32 percentOnSegment, real32& percentOut)
{
	percentOut = 0;

	return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// ThickCircleArc
//
///////////////////////////////////////////////////////////////////////////////

ThickCircleArc::ThickCircleArc()
{
	mLeftThickness = 0;
	mRightThickness = 0;

	mThicknessIsValid = false;

	mHasStartLimit = false;
	mHasEndLimit = false;

	mStartLimitPercent = 0;
	mEndLimitPercent = 1;

}

ThickCircleArc::~ThickCircleArc()
{
}

void ThickCircleArc::Validate()
{
	// Then the data to compute the thickness
	if(!mThicknessIsValid)
	{
		mThicknessIsValid = true;

		// First compute the arc
		CircleArc::Validate();

		const TMCClassArray<TVector2>& arc = GetArc();

		const int32 pointCount = arc.GetElemCount();

		if(pointCount<2)
			return; // something's wrong

		const int32 segCount = pointCount-1;
	
		mCachedSegmentData.SetElemCount(segCount);

		// There's n-1 segment
		// First compute the start and the end limits, in case they're 
		// bigger than the segment length.
		SegmentData& firstSegData = mCachedSegmentData[0];
		SegmentData& lastSegData = mCachedSegmentData[segCount-1];

		ComputeLimits(	firstSegData.mLimitMin, firstSegData.mMinCenter, 
						lastSegData.mLimitMax, lastSegData.mMaxCenter);

		for(int32 iSeg=0 ; iSeg<segCount ; iSeg++)
		{
			ComputeSegmentData(iSeg, segCount, firstSegData.mLimitMin, lastSegData.mLimitMax);
		}

	}
	else
	{
		CircleArc::Validate();
	}
}

void ThickCircleArc::ComputeLimits(real32& startLimit, TVector2& startCenter, 
								   real32& endLimit, TVector2& endCenter)
{
	const TMCClassArray<TVector2>& arc = GetArc();

	if(mHasStartLimit)
	{
		// Compute the start limit and the center of orientation

		const TVector2& point0 = arc[0];
		const TVector2& point1 = arc[1];

		TVector2 segment = point1 - point0;
		TVector2 segmentDir;
	
		real32 length = segment.Normalize(segmentDir);

		// 1. Get the abscissa of the projection of the 2 intersection points
		// on the segment.

		real32 abscissaLeft = segmentDir*(mLeftStartLimit-point0);
		real32 abscissaRight = segmentDir*(mRightStartLimit-point0);

		boolean useLeft = abscissaLeft>abscissaRight?true:false;
		if(useLeft)
		{
			startLimit = abscissaLeft;
			startCenter = mLeftStartLimit;
		}
		else
		{
			startLimit = abscissaRight;
			startCenter = mRightStartLimit;
		}

		// Get the minimum authorised abscissa on the arc
		// intersection of the lines (point0, point1) and (mRightEndLimit, mLeftEndLimit)
		// then distance to the extremity point0
		TVector2 intersection;
		if( IntersectLineLine(	mLeftStartLimit, mRightStartLimit, point0, segmentDir, intersection ) )
		{
			real32 dist = segmentDir*(intersection-point0);
			// convert the distance in a percent
			real32 percent = dist/mArcLength;

			mStartLimitPercent = percent;
		}
	}
	else
	{
		startLimit = -kRealBig;
		startCenter = arc[0]; // Not used, so set any value here
	}

	if(mHasEndLimit)
	{
		// Compute the end limit and the center of orientation

		const int32 pointCount = arc.GetElemCount();
		const TVector2& point0 = arc[pointCount-1];
		const TVector2& point1 = arc[pointCount-2];

		TVector2 segment = point1 - point0;
		TVector2 segmentDir;
	
		real32 length = segment.Normalize(segmentDir);

		// 1. Get the abscissa of the projection of the 2 intersection points
		// on the segment.

		real32 abscissaLeft = length-segmentDir*(mLeftEndLimit-point0);
		real32 abscissaRight = length-segmentDir*(mRightEndLimit-point0);

		boolean useLeft = abscissaLeft<abscissaRight?true:false;
		if(useLeft)
		{
			endLimit = abscissaLeft;
			endCenter = mLeftEndLimit;
		}
		else
		{
			endLimit = abscissaRight;
			endCenter = mRightEndLimit;
		}

		// Get the maximum authorised abscissa on the arc
		// intersection of the lines (point0, point1) and (mRightEndLimit, mLeftEndLimit)
		// then distance to the extremity point0
		TVector2 intersection;
		if( IntersectLineLine(	mLeftEndLimit, mRightEndLimit, point0, segmentDir, intersection ) )
		{
			real64 dist = segmentDir*(intersection-point0);
			// convert the distance in a percent
			real64 percent = dist/mArcLength;

			mEndLimitPercent = 1-percent;
		}

	}
	else
	{
		endLimit = kRealBig;
		endCenter = arc[1]; // Not used, so set any value here
	}
}

void ThickCircleArc::ComputeSegmentData(int32 iSeg, int32 segCount, real32 startLimit, real32 endLimit)
{
	SegmentData& segData = mCachedSegmentData[iSeg];

	const TMCClassArray<TVector2>& arc = GetArc();
	const TVector2& point0 = arc[iSeg];
	const TVector2& point1 = arc[iSeg+1];

	TVector2 segment = point1 - point0;
	TVector2 segmentDir;

	segData.mLength = segment.Normalize(segmentDir);

	const boolean isFirst = (iSeg==0);
	const boolean isLast = (iSeg==segCount-1);

	if(!isFirst)
	{
		const TVector2& prevPoint = arc[iSeg-1];
		TVector2 prevDir = prevPoint - point0;
		prevDir.Normalize();

		TVector2 sum = segmentDir+prevDir;
		TVector2 bissectrice;
	
		real32 length = sum.Normalize(bissectrice);

		if(length<kRealEpsilon)
		{	// 3 points aligned case, no limit
			segData.mLimitMin = 0;
			segData.mMinCenter = point0;
		}
		else
		{
			// Distance on the bissectrice
			real32 sin = segmentDir ^ bissectrice;
			real32 dist = sin>0?mLeftThickness:mRightThickness;

			segData.mMinCenter = point0 + dist/RealAbs(sin) * bissectrice;

			// Abscissa
			real32 cos = segmentDir * bissectrice;
			MY_ASSERT(cos>=0);
			segData.mLimitMin = RealAbs(cos) * dist;
		}
	}


	if(!isLast)
	{
		const TVector2& nextPoint = arc[iSeg+2];
		TVector2 nextDir = nextPoint - point1;
		nextDir.Normalize();

		TVector2 sum = -segmentDir+nextDir;
		TVector2 bissectrice;
	
		real32 length = sum.Normalize(bissectrice);

		if(length<kRealEpsilon)
		{	// 3 points aligned case, no limit
			segData.mLimitMax = segData.mLength;
			segData.mMaxCenter = point1;
		}
		else
		{
			// Distance on the bissectrice
			real32 sin = -segmentDir ^ bissectrice;
			real32 dist = sin>0?mRightThickness:mLeftThickness;

			segData.mMaxCenter = point1 + dist/RealAbs(sin) * bissectrice;

			// Abscissa
			real32 cos = -segmentDir * bissectrice;
			MY_ASSERT(cos>=0);
			segData.mLimitMax = segData.mLength - RealAbs(cos) * dist;
		}
	}
}

real32 ThickCircleArc::GetLength()
{
	Validate();

	const int32 segCount = mCachedSegmentData.GetElemCount();

	real32 totalLength = 0;

	for(int32 iSeg=0 ; iSeg<segCount ; iSeg++)
	{
		const SegmentData& data = mCachedSegmentData[iSeg];
		totalLength += data.mLength;
	}
	
	return totalLength;
}

// Normal: the normal to the segment (and not the normal to the circle arc)
real32 ThickCircleArc::GetDataOnArc( real32 percent, TVector2& position, TVector2& normal, TVector2& direction )
{
	// It's not the actual normal, but one that is modified according to the surrounding

	// Check on which segment we're.

	Validate();

	const real32 abscissa = percent * GetLength();

	const int32 segCount = mCachedSegmentData.GetElemCount();

	real32 totalLength = 0;

	for(int32 iSeg=0 ; iSeg<segCount ; iSeg++)
	{
		const SegmentData& data = mCachedSegmentData[iSeg];
		if(abscissa<totalLength + data.mLength)
		{
			real32 localAbscissa = abscissa-totalLength;
			GetDataOnArc( localAbscissa, iSeg, position,  normal, direction );

			totalLength += localAbscissa;
			return totalLength;
		}

		totalLength += data.mLength;
	}
	
	// after the last segment
	real32 lastLength = mCachedSegmentData[segCount-1].mLength;
	real32 localAbscissa = abscissa+lastLength-totalLength;
	GetDataOnArc( localAbscissa, segCount-1, position,  normal, direction );

	totalLength -= lastLength;
	totalLength += localAbscissa;

	return totalLength;
}

// Normal: the normal to the segment (and not the normal to the circle arc)
void ThickCircleArc::GetDataOnArc( real32 localAbscissa, int32 iSeg, TVector2& position, TVector2& normal, TVector2& direction )
{
	Validate();

	// 3 cases:
	// - before or after the extremities limits
	// - along a segment

	const SegmentData& data = mCachedSegmentData[iSeg];
	const TVector2& point0 = mCachedArc[iSeg];
	const TVector2& point1 = mCachedArc[iSeg+1];

	TVector2 segment = point1 - point0;
	segment.Normalize();
	position = point0 + localAbscissa*segment;

	// The normal to the segment
	normal.x = segment.y;
	normal.y = -segment.x;

	// The direction can be different from the normal
	if(localAbscissa<data.mLimitMin)
	{
		direction = position - data.mMinCenter;
		direction.Normalize();
	}
	else if(localAbscissa>data.mLimitMax)
	{
		direction = position - data.mMaxCenter;
		direction.Normalize();
	}
	else
	{
		direction = normal;
	}
}

// normalOnArc: the normal to the segment (and not the normal to the circle arc)
// directionOnArc: direction for the thickness construction. Can be different from the normal
void ThickCircleArc::GetPosAroundArc(real32 percent, 
										int32 segmentID,
										real32& leftAbscissa,
										real32& rightAbscissa,
										TVector2& posOnArc,
										TVector2& leftPos, 
										TVector2& rightPos,
										TVector2& normalOnArc, // on the right
										TVector2& directionOnArc,// on the right
										bool		clamp)
{
	if(clamp)
	{
		percent = MC_Clamp(percent, mStartLimitPercent, mEndLimitPercent);
	}

	const real32 arcLength = GetLength();

	const real32 abscissa = percent * arcLength;

	if(segmentID==kUnusedIndex)
	{	// Look everywhere
		GetDataOnArc( percent, posOnArc, normalOnArc, directionOnArc);
	}
	else
	{
		// Compute the local abscissa
		real32 totalLengthBeforeSeg = 0;

		for(int32 iSeg=0 ; iSeg<segmentID ; iSeg++)
		{
			const SegmentData& data = mCachedSegmentData[iSeg];
			totalLengthBeforeSeg += data.mLength;
		}

		real32 locaAbscissa = abscissa-totalLengthBeforeSeg;
		GetDataOnArc( locaAbscissa, segmentID, posOnArc, normalOnArc, directionOnArc);
	}

	rightAbscissa = abscissa;
	leftAbscissa = abscissa;

	// Adapt the thickness: thickness is right only if the normal is perpendicular to the surface.
	//const real32 cos = vec1 * vec2;
	//const real32 sin = vec1 ^ vec2;
	const real32 cos = normalOnArc * directionOnArc;
	double epsilon = 0.001; // kRealEpsilon;
	if(	RealAbs(cos)<1-epsilon &&
		RealAbs(cos)>epsilon) // just to be safe, check for 0 value (normal and direction aligned => shouldn't occured)
	{
		rightPos = posOnArc + (mRightThickness/cos) * directionOnArc;
		leftPos = posOnArc - (mLeftThickness/cos) * directionOnArc;

		const real32 sin = normalOnArc ^ directionOnArc;

		rightAbscissa += (mRightThickness*sin/cos);
		leftAbscissa -= (mLeftThickness*sin/cos);
	}
	else
	{
		rightPos = posOnArc + mRightThickness*normalOnArc;
		leftPos = posOnArc - mLeftThickness*normalOnArc;
	}
}


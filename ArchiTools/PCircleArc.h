/****************************************************************************************************

		PCircleArc.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	24/03/2007

****************************************************************************************************/

#ifndef __PCircleArc__
#define __PCircleArc__

#include "Vector2.h"
#include "MCClassArray.h"

// A segmented circle arc
class CircleArc
{
public:
	CircleArc();
	virtual ~CircleArc();

	const TVector2&		GetFrom()		const { return mFrom; }
	const TVector2&		GetTo()			const { return mTo; }
	int32				GetPointCount()	const { return mPointCount; }
	real32				GetOffset()		const { return mOffset; }

	void				SetFrom(const TVector2&	value)	{ mFrom = value; Invalidate();}
	void				SetTo(const TVector2& value)	{ mTo = value; Invalidate(); }
	void				SetPointCount(int32 value)		{ mPointCount = value; Invalidate(); }
	void				SetOffset(real32 value)			{ mOffset = value; Invalidate(); }


	const TMCClassArray<TVector2>&	GetArc() {Validate(); return mCachedArc; }
	const TVector2&					GetCenter() const {return mCachedCenter;} 
	const real32					GetRadius() const {return mCachedRadius;} 

	// Returns a position on the arc for an abscissa in the current unit.
	boolean				GetPosOnArc(real32 abscissa, TVector2& position);
	// Returns a position on the arc for an abscissa between 0 and 1.
	boolean				GetPosOnArcFromPercent(real32 percent, TVector2& position);

	// Returns a value between 0 and 1 for a position on one of the
	// segments of the arc. A position is given by the ID of the segment, and the
	// abscissa on it (a value between 0 and 1)
	boolean				GetPercentFromArc(int32 onSegment, real32 percentOnSegment, real32& percentOut);

protected:

	virtual void		Invalidate(){ mCacheIsValid = false; }
	virtual void		Validate();

	inline boolean CurveSign() const {return (mOffset>=0);}

	TVector2	mFrom;
	TVector2	mTo;
	int32		mPointCount;
	real32		mOffset; // The distance to the (mFrom, mTo) segment. Can be positive or negative. 0 is a straight line.

	boolean		mCacheIsValid;
	TMCClassArray<TVector2> mCachedArc;
	TVector2	mCachedCenter;
	real32		mCachedRadius;
	real32		mArcLength; 
};

// A circle arc with the tools to compute the thickness
class ThickCircleArc : public CircleArc
{
public:
	ThickCircleArc();
	virtual ~ThickCircleArc();


	void SetHasStartLimit( boolean has ) { mHasStartLimit = has; }
	void SetHasEndLimit( boolean has ) { mHasEndLimit = has; }

	void SetLeftThickness( real32 value ) { mLeftThickness = value; }
	void SetRightThickness( real32 value ) { mRightThickness = value; }

	void SetLeftStartLimit( const TVector2& limit ) {mLeftStartLimit = limit; InvalidateThickness();}
	void SetLeftEndLimit( const TVector2& limit ) {mLeftEndLimit = limit; InvalidateThickness();}
	void SetRightStartLimit( const TVector2& limit ) {mRightStartLimit = limit; InvalidateThickness();}
	void SetRightEndLimit( const TVector2& limit ) {mRightEndLimit = limit; InvalidateThickness();}

	// segmentID: use kUnusedIndex to look everywhere
	// posOnArc: the position on a segment of the arc
	// normalOnArc: the normal to the segment (and not the normal to the circle arc)
	// directionOnArc: can be different from the normal
	// clamp: clamp the percent between 0 and 1
	void GetPosAroundArc(real32 percent, 
							int32 segmentID,
							real32& leftAbscissa,
							real32& rightAbscissa,
							TVector2& posOnArc,
							TVector2& leftPos, 
							TVector2& rightPos,
							TVector2& normalOnArc,
							TVector2& directionOnArc,
							bool	clamp);

	// Returns the abscissa on the arc
	real32 GetDataOnArc( real32 percent, TVector2& position, TVector2& normal, TVector2& direction );

	// Return the position on the segment, the normal to the segment
	// and a direction that can be different from the normal
	void GetDataOnArc( real32 localAbscissa, int32 iSeg, TVector2& position, TVector2& normal, TVector2& direction );

	real32	GetLength();

	real32	GetStartLimitPercent(){ return mStartLimitPercent;} // can be <0
	real32	GetEndLimitPercent(){ return mEndLimitPercent;}  // can be >1

protected:

	struct SegmentData
	{
		SegmentData() : mLength(0), mLimitMin(0), mLimitMax(0) {}

		real32 mLength;
		real32 mLimitMin; // can be negative (for the first segment)
		real32 mLimitMax; // can be >mLength (for the last segment)
		TVector2 mMinCenter;
		TVector2 mMaxCenter;
	};

	virtual void		Invalidate(){ CircleArc::Invalidate(); InvalidateThickness();}

	void				InvalidateThickness(){ mThicknessIsValid = false; }

	virtual void		Validate();

	void				ComputeLimits(	real32& startLimit, TVector2& startCenter, 
										real32& endLimit, TVector2& endCenter);
	void				ComputeSegmentData(int32 iSeg, int32 segCount, 
										real32 startLimit, real32 endLimit);

	real32		mLeftThickness;
	real32		mRightThickness;

	boolean		mThicknessIsValid;

	// Cached data
	boolean		mHasStartLimit;
	boolean		mHasEndLimit;
	TMCClassArray<SegmentData> mCachedSegmentData;

	// The extremities corners 
	TVector2	mLeftStartLimit;
	TVector2	mLeftEndLimit;
	TVector2	mRightStartLimit;
	TVector2	mRightEndLimit;

	// Use this to clamp the percent when asking GetDataOnArc
	real32		mStartLimitPercent; // can be <0
	real32		mEndLimitPercent; // can be >1
};

#endif
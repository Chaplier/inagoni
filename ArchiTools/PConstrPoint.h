/****************************************************************************************************

		PConstrPoint.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	24/03/2007

****************************************************************************************************/

#ifndef __PConstrPoint__
#define __PConstrPoint__

#include "MCAssert.h"
#include "Vector2.h"

class Wall;
class VPoint;

// Base class for the construction point
// A construction point is a position in a plan. It helps delimiting
// the rooms
class ConstrPoint
{
public:
	ConstrPoint();
	virtual ~ConstrPoint();

	virtual const TVector2& Position() const = 0;
protected:

};

// Construnction point inside a wall.
// For non straight walls
class WallConstrPoint : public ConstrPoint
{
public:
	WallConstrPoint(Wall*, const TVector2& );
	virtual ~WallConstrPoint();

	virtual const TVector2& Position() const {return mPosition;}
	inline void			SetPosition( const TVector2& pos){ mPosition=pos; }
protected:

	Wall*		mOnWall;
	TVector2	mPosition;

};

// Construction point on a Point (extremity of 1 or more walls)
class PointConstrPoint : public ConstrPoint
{
public:
	PointConstrPoint(VPoint*);
	virtual ~PointConstrPoint();

	virtual const TVector2& Position() const;
protected:

	VPoint*		mOnPoint;
};

#endif
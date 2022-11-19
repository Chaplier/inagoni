/****************************************************************************************************

		PConstrPoint.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	24/03/2007

****************************************************************************************************/

#include "PConstrPoint.h"

#include "PPoint.h"

ConstrPoint::ConstrPoint()
{
}

ConstrPoint::~ConstrPoint()
{
}

WallConstrPoint::WallConstrPoint(Wall* wall, const TVector2& pos)
{
	mPosition = pos;
	mOnWall = wall;
}

WallConstrPoint::~WallConstrPoint()
{
}

PointConstrPoint::PointConstrPoint(VPoint* point)
{
	mOnPoint = point;
}

PointConstrPoint::~PointConstrPoint()
{
}

const TVector2& PointConstrPoint::Position() const
{
	return mOnPoint->Position();
}

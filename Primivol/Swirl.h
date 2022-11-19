/****************************************************************************************************

		Swirl.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/7/2004

****************************************************************************************************/

#ifndef __Swirl__
#define __Swirl__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Vector2.h"
#include "Vector3.h"

enum ESwirlAxis
{
	eX,
	eY,
	eZ,
	eMinusX,
	eMinusY,
	eMinusZ,
};

class Swirl
{
public:
	Swirl();

	void SwirlPoint(TVector3& point, const real32 dist) const;

	void SetPosition(const TVector3& pos){fPosition = pos;}
	void SetAxis(const ESwirlAxis axis){fAxis = axis;InitIndexes();}
	void SetRadius(const real32 radius){fRadius = radius;InitConst();}

	void SetStep(const real32 step){fSpiralStep = step;}
	void SetAngle(const real32 angle){fSpiralAngle = angle;}

protected:
	void InitIndexes();
	void InitConst();

	ESwirlAxis fAxis;
	real32 fRadius;
	TVector3 fPosition;

	real32 fSpiralStep;
	real32 fSpiralAngle;

	// Cache
	real32 fRadiusSqr;
	real32 fInvRadiusSqr;
	int32 fIndexes[3];
	boolean fFlip;
};



#endif

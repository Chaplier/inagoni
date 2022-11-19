/****************************************************************************************************

		Swirl.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/7/2004

****************************************************************************************************/

#include "Swirl.h"

Swirl::Swirl()
{
	fAxis = eX;
	fRadius = .5f;
	fPosition = TVector3::kZero;

	fSpiralStep = .1f;
	fSpiralAngle = 60;
 
	fFlip=false;
	InitIndexes();
	InitConst();
}

void Swirl::InitIndexes()
{
	switch(fAxis)
	{
	case eX: // x axis
		{ fIndexes[0] = 1; fIndexes[1] = 2; fIndexes[2] = 0; fFlip=false; } break;
	case eY: // y axis
		{ fIndexes[0] = 2; fIndexes[1] = 0; fIndexes[2] = 1; fFlip=false; } break;
	case eZ: // z axis
		{ fIndexes[0] = 0; fIndexes[1] = 1; fIndexes[2] = 2; fFlip=false; } break;
	case eMinusX: // x axis
		{ fIndexes[0] = 2; fIndexes[1] = 1; fIndexes[2] = 0; fFlip=true; } break;
	case eMinusY: // y axis
		{ fIndexes[0] = 0; fIndexes[1] = 2; fIndexes[2] = 1; fFlip=true; } break;
	case eMinusZ: // z axis
		{ fIndexes[0] = 1; fIndexes[1] = 0; fIndexes[2] = 2; fFlip=true; } break;
	}
};

void Swirl::InitConst()
{
	fRadiusSqr = fRadius*fRadius;
	fInvRadiusSqr = 1/fRadiusSqr;
}

void Swirl::SwirlPoint(TVector3& point, const real32 swirlPercent) const
{
	const real32 oriU = point[fIndexes[0]];
	const real32 oriV = point[fIndexes[1]];
	const real32 oriW = point[fIndexes[2]]; // axis

	// Get the distance from the axis
	const real32 posU = fPosition[fIndexes[0]];
	const real32 posV = fPosition[fIndexes[1]];
	const real32 distU = oriU-posU;
	const real32 distV = oriV-posV;
	const real32 pointToAxisSqr = distU*distU + distV*distV;

	if(pointToAxisSqr>=fRadiusSqr)
		return; // outside

	real32 rampOff = (fRadiusSqr - pointToAxisSqr)*fInvRadiusSqr;
	rampOff*=rampOff;

	const real32 tethaSwirl = (fFlip?-1:1)*DegToRad(rampOff*fSpiralAngle*swirlPercent);
	const real32 cosTetha = RealCos(tethaSwirl);
	const real32 sinTetha = RealSin(tethaSwirl);

	// swirl: rotate arroun fPosition
	const TVector3 oriPt = point;
	const real32 transU = oriU - posU;
	const real32 transV = oriV - posV;
	const real32 rotU = transU*cosTetha - transV*sinTetha;
	const real32 rotV = transU*sinTetha + transV*cosTetha;
	point[fIndexes[0]] = rotU + posU;
	point[fIndexes[1]] = rotV + posV;
	point[fIndexes[2]] += fSpiralStep*tethaSwirl;
}


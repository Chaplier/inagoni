/****************************************************************************************************

		Engine3.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __Engine3__
#define __Engine3__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"



#include "copyright.h"

#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"

struct PMapGridShader3;
struct PMapTilingShader3;

class Engine3 
{
public :

	Engine3();

	void PreProcess(const PMapGridShader3& pmap);
	void PreProcess(const PMapTilingShader3& pmap);

	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, boolean& flip ); // retrun a UV center on the hexagon

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);

	real32 GetLocalTileU(const real32 uu, const int32 iU);
	real32 GetLocalTileV(const real32 vv, const int32 iV);

	boolean fFlippedTile; // true if u,v is in a vertical tile
	boolean fSwitch;

	real32	fSideLength;
	real32	fHalfSideLength;
	real32	fMortarLenght;

	real32	fPeriodicity;
	real32	fTwoPeriodicity;

	real32	fRescaledLTileStart; // rescaled on a [0,1] space
	real32	fRescaledLTileEnd; // rescaled on a [0,1] space
	real32	fRescaledLEndPlateau;// rescaled on a [0,1] space
	real32	fRescaledLStartPlateau;// rescaled on a [0,1] space

	real32	fRescaledHTileStart; // rescaled on a [0,1] space
	real32	fRescaledHTileEnd; // rescaled on a [0,1] space
	real32	fRescaledHEndPlateau;// rescaled on a [0,1] space
	real32	fRescaledHStartPlateau;// rescaled on a [0,1] space

	real32	fMortarBumpDepth;
	real32	fMortarBottomSlope;
	real32	fMortarTopSlope;

	int32	fType;
	real32	fAmplitude;

	boolean	fSmooth;
	boolean fRandomUVOrigin;
	boolean fProportionnalTileShading;
	boolean	fNeedCenter;

	TVector2 fTileCenterUV;

	TTransform2D	f2DTransform;
};
 

#endif

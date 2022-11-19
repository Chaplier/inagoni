/****************************************************************************************************

		Engine2.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __Engine2__
#define __Engine2__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "MCPtrArray.h"
#include "copyright.h"
#include "Transform2D.h"
#include "NoiseBase.h"

struct PMapGridShader2;
struct PMapTilingShader2;

class Engine2
{
public :

	Engine2();

	void PreProcess(const PMapGridShader2& pmap);
	void PreProcess(const PMapTilingShader2& pmap);

	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, boolean& flip );

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);
	real32 ComputeOption7(const real32 uu, const real32 vv);
	real32 ComputeOption8(const real32 uu, const real32 vv); // for 8 and 9
	real32 ComputeOption10(const real32 uu, const real32 vv);

	real32 ComputeOption11(const real32 uu, const real32 vv);
	real32 ComputeOption12(const real32 uu, const real32 vv);


	real32 GetLocalTileU(const real32 uu, const int32 iU);
	real32 GetLocalTileV(const real32 vv, const int32 iV);

	real32	fTotalLength;
	real32	fHeight;

	real32	fHeightOnTotalLength;
	real32	fHalfHeightOnTotalLength;
	real32	fTwoHeightOnTotalLength;

	// Rectangular tile param
	boolean fFirstTile;
	real32	fLength1;
	real32	fLength2;
	real32	fHalfLength1;
	real32	fLength1PlusHalfLength2;
	real32	fLength1OnTotalLength;

	// Oblique tile param
	real32	fSlopeCoeff;

// Auto block params
	real32	fScaledMortarHor;
	real32	fScaledMortarVer;
	real32	fScaleObliqueMortar;
	real32	fScaledFlat; // half flat part length
	real32	fScaledGap; // distance from the center to the bottom of the gap
	// Other usefull preprocess values
	real32	fScaledMorStartHor;
	real32	fScaledMorStartVer;
	real32	fScaledHalfMinusMorHor;
	real32	fScaledHalfMinusStaHor;
	real32	fScaledBeginMorVer;
	real32	fScaledEndMorVer;
	real32	fScaledBegin2MorVer;
	real32	fScaledEnd2MorVer;
	real32	fScaledHalfMinusFlat;
	real32	fScaledOneMinusGap;

	real32	fA; // oblique part is y = fA*x + fB
	real32	fB;
	real32	fCosA;
	real32	fSinA;
	real32	fTanB;
	real32	fCosB;
	real32	fSinB;

	real32	fMortarBumpDepth;
	real32	fMortarBottomSlope;
	real32	fMortarTopSlope;

	real32	fHDelta;
	real32	fLDelta;

	//
	int32	fType;
	real32	fShifting;
	real32	fMortarPlateau;
	real32	fHMortar;
	real32	fLMortar;
	boolean	fRandomUVOrigin;
	boolean	fProportionnalTileShading;
	boolean	fSmooth;
	boolean	fNeedCenter;

	// center of the tile in global uv coordinate
	TVector2	fTileCenterUV;

	TTransform2D	f2DTransform;
	TNoiseBase		fNoise; // the same noise is used at different levels

};
                           
#endif // __Engine2__


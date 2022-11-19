/****************************************************************************************************

		Engine4.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __Engine4__
#define __Engine4__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"


#include "copyright.h"

#include "VoronoiBase.h"
#include "2DTransformBase.h"

struct PMapGridShader4;
struct PMapTilingShader4;

class Engine4
{
public :

	Engine4();

	void PreProcess(const PMapGridShader4& pmap);
	void PreProcess(const PMapTilingShader4& pmap);

	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV ); // retrun a UV center on the hexagon

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);

	real32 GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	real32 GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV);


	int32	fTileID;

	real32	fLength;

	// Values needed for the Rock tiling
	real32	fUStretch;
	real32	fVStretch;
	uint32	fId1; // Id associate to a tile

	real32	fSuperSamplingQualityU;
	real32	fSuperSamplingQualityV;

	real32	fMortarBumpDepth;
	real32	fMortarBottomSlope;
	real32	fMortarTopSlope;

	int32	fType;
	real32	fMortarSize;
	real32	fMortarPlateau;
	boolean fRandomUVOrigin;
	boolean fProportionnalTileShading;
	boolean fNeedCenter;

	TVector2 fTileCenterUV;

	TTransform2D	f2DTransform;
	TVoronoiBase	fVoronoi; // for the Rock shader
};

#endif

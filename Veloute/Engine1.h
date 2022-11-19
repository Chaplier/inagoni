/****************************************************************************************************

		Engine1.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __Engine1__
#define __Engine1__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "MCPtrArray.h"
#include "copyright.h"
#include "Transform2D.h"

struct PMapGridShader1;
struct PMapTilingShader1;

// Compute the pattern for TilingShader1 and GridShader1
class Engine1
{
public :

	Engine1();

	void PreProcess(const PMapGridShader1& pmap);
	void PreProcess(const PMapTilingShader1& pmap);

	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV ); // retrun a UV center on the hexagon
	void GetLocalUVCoordOption1(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption2(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption3(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption4(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);

	void GetLocalUVCoordOption5(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption6(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption7(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption8(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption9(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption10(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption11(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption12(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption13(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption14(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption15(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption16(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption17(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption18(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption19(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);
	void GetLocalUVCoordOption20(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV);

	void ComputeTileCenter(const int32 tileU, const int32 tileV, const int32 iU, const int32 iV, const real32 decalU, const real32 decalV);
	int32 GetEquilateralTriangle(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, TVector2& hexagonCenter );

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);
	real32 ComputeOption1(const real32 uu, const real32 vv);
	real32 ComputeOption2(const real32 uu, const real32 vv);
	real32 ComputeOption3(const real32 uu, const real32 vv);
	real32 ComputeOption4(const real32 uu, const real32 vv); // Hexa tiling

	real32 ComputeOption5(const real32 uu, const real32 vv);
	real32 ComputeOption6(const real32 uu, const real32 vv);
	real32 ComputeOption7(const real32 uu, const real32 vv);
	real32 ComputeOption8(const real32 uu, const real32 vv);
	real32 ComputeOption9(const real32 uu, const real32 vv);
	real32 ComputeOption10(const real32 uu, const real32 vv);
	real32 ComputeOption11(const real32 uu, const real32 vv);
	real32 ComputeOption12(const real32 uu, const real32 vv);
	real32 ComputeOption13(const real32 uu, const real32 vv);
	real32 ComputeOption14(const real32 uu, const real32 vv);
	real32 ComputeOption15(const real32 uu, const real32 vv);
	real32 ComputeOption16(const real32 uu, const real32 vv);
	real32 ComputeOption17(const real32 uu, const real32 vv);
	real32 ComputeOption18(const real32 uu, const real32 vv);
	real32 ComputeOption19(const real32 uu, const real32 vv);
	real32 ComputeOption20(const real32 uu, const real32 vv);


	inline real32 PositiveStep(const real32 origin, const real32 local);
	inline real32 NegativeStep(const real32 origin, const real32 local);

	real32 GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption1(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption2(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption3(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);

	inline real32 GetLocalTileUOption6(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption7(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption11(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption13(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption14(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption15(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption16(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption17(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption18(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption19(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileUOption20(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV);


	real32 GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption1(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption2(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption3(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption4(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);

	inline real32 GetLocalTileVOption6(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption7(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption11(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption13(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption14(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption15(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption16(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption17(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption18(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption19(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileVOption20(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV);


	int32	fTileID;

	real32	fLength;
	real32	fSize; // size of the small square tile, from 0 to 0.5
	real32	fHalfMinusSize;
	real32	fHalfMinusHalfSize;

	// Value needed for the 3 first arrangments
	real32	fRescaledMortar;

	// values needed for the hexagonal tiling
	real32	fHalfLength; // is equal to the triangle height
	real32	fTriangleSide;
	real32	fHalfTriangleSide;
	real32	fScaledTriangleHeight;
	real32	fRescaledTileStart; // rescaled on a [0,2] space
	real32	fRescaledTileEnd; // rescaled on a [0,2] space
	real32	fRescaledEndPlateau;// rescaled on a [0,2] space
	real32	fRescaledStartPlateau;// rescaled on a [0,2] space

	real32	fMortarBumpDepth;
	real32	fMortarBottomSlope;
	real32	fMortarTopSlope;
	real32	fMortarPlateau;

	int32	fType;
	boolean fSmooth;
	boolean	fRandomUVOrigin;
	boolean	fProportionnalTileShading;
	boolean fNeedCenter;

	TVector2		fTileCenterUV;
	TTransform2D	f2DTransform;
};
                           






#endif

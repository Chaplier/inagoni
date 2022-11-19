/****************************************************************************************************

		Engine4.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "Engine4.h"


#include "math.h"
#include "Utils.h"
#include "MCRandom.h"
#include "Veloute.h"
#include "GridShader4.h"
#include "TilingShader4.h"

#define SQRT3 			1.7320508075688772935274463415059		/* sqrt(3.0)	*/
#define HALF_SQRT3		0.86602540378443864676372317075294		/* sqrt(3.0)/2	*/
#define	PI3				1.0471975511965977461542144610932		/* PI/3			*/
#define	TWO_PI3			2.0943951023931954923084289221863		/* 2*PI/3		*/
#define HALF_SQRT2		0.70710678118654752440084436210485		/* sqrt(2.0)/2	*/

Engine4::Engine4()	// We just initialize the values
{
	fLength = 0;

	fMortarBumpDepth=0;
	fMortarBottomSlope = 0;
	fMortarTopSlope = 0;

	fTileID = 0;

	// Values needed for the Rock tiling
	fUStretch=0;
	fVStretch=0;
	fId1=0; // Id associate to a tile

	fSuperSamplingQualityU=0;
	fSuperSamplingQualityV=0;

	fMortarSize=0;
	fMortarPlateau=0;
	fRandomUVOrigin=0;
	fProportionnalTileShading=0;
	fNeedCenter=0;

	fTileCenterUV = TVector2::kZero;
}
	
void Engine4::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	newV = fromUV.y/fLength; // it's not the real tile count but it's an approximation
	newU = fromUV.x/fLength;
}

real32 Engine4::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	int32 distanceMethod = 0;

	switch(fType)
	{
	case 'Opt1': distanceMethod = 0; break; // Straight
	case 'Opt2': distanceMethod = 5; break; // Ondulated
	case 'Opt3': distanceMethod = 12; break;// More Ondulated
	case 'Opt4': distanceMethod = 10; break;// Angular 1
	case 'Opt5': distanceMethod = 8; break; // Angular 2
	}

	//	Harsh corners version
	uint32 id2=0;
	real32 F1=0, F2=0;
	fVoronoi.GetF1AndF2(uu,vv,fUStretch, fVStretch, distanceMethod, F1, F2, fId1,id2, fTileCenterUV);
	const real32 value = RealAbs(F2-F1);

	if( fNeedCenter)
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		fTileCenterUV *= fLength;
		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}

	return fMortarBumpDepth*(SmoothStepWithTan(	fMortarSize*fMortarPlateau,
													fMortarSize,
													fMortarTopSlope,fMortarBottomSlope,value) );
}

real32 Engine4::GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalU = (fRandomUVOrigin?floor(fId1/516432834.0):0);
	if(fProportionnalTileShading)
		return (decalU + uu); // fId1 is <4294967296.0
	else return (decalU + uu*fUStretch); // fId1 is <4294967296.0
}

real32 Engine4::GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalV = (fRandomUVOrigin?floor(fId1/516432834.0):0);
	if(fProportionnalTileShading)
		return (decalV + vv);
	else
		return (decalV + vv*fVStretch);
}

void Engine4::PreProcess(const PMapGridShader4& pmap)
{
	fLength = 100/(real32)(pmap.fTileCount);

	fMortarBumpDepth = pmap.fBumpDepth/(real32)(pmap.fTileCount);

	// Stretching
	switch(pmap.fType)
	{
	case 'Opt1': // Straight
	case 'Opt2': // Ondulated
		{
			if(pmap.fStretch>0)
			{
				fUStretch = RealPow(10, 2*pmap.fStretch );
				fVStretch = 1;
			}
			else
			{
				fUStretch = 1;
				fVStretch = RealPow(10, -2*pmap.fStretch );
			}
		}  break;
	case 'Opt3': // More Ondulated
	case 'Opt4': // Angular 1
	case 'Opt5': // Angular 2
		{
			if(pmap.fStretch>0)
			{
				fUStretch = .5*RealPow(10, 2*pmap.fStretch );
				fVStretch = 1;
			}
			else
			{
				fUStretch = 1;
				fVStretch = .5*RealPow(10, -2*pmap.fStretch );
			}
		}  break;
	}
	// Noise
	fVoronoi.SetSeed(pmap.fSeed);

	// Get the slopes values
	switch(pmap.fMiddleSlope)
	{
	case 'Opt1': fMortarBottomSlope = 0; break;
	case 'Opt2': fMortarBottomSlope = 1; break;
	case 'Opt3': fMortarBottomSlope = 2; break;
	}
	switch(pmap.fSidesSlope)
	{
	case 'Opt1': fMortarTopSlope = 0; break;
	case 'Opt2': fMortarTopSlope = 1; break;
	case 'Opt3': fMortarTopSlope = 2; break;
	}

	fType = pmap.fType;
	fMortarSize = pmap.fSectionSize;
	fMortarPlateau = pmap.fSectionPlateau;
	fRandomUVOrigin = false;
	fProportionnalTileShading = false;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}

void Engine4::PreProcess(const PMapTilingShader4& pmap)
{
	fLength = 100/(real32)(pmap.fTileCount);

	fMortarBumpDepth = pmap.fMortarDepth/(real32)(pmap.fTileCount);

	// Stretching
	switch(pmap.fType)
	{
	case 'Opt1': // Straight
	case 'Opt2': // Ondulated
		{
			if(pmap.fStretch>0)
			{
				fUStretch = RealPow(10, 2*pmap.fStretch );
				fVStretch = 1;
			}
			else
			{
				fUStretch = 1;
				fVStretch = RealPow(10, -2*pmap.fStretch );
			}
		}  break;
	case 'Opt3': // More Ondulated
	case 'Opt4': // Angular 1
	case 'Opt5': // Angular 2
		{
			if(pmap.fStretch>0)
			{
				fUStretch = .5*RealPow(10, 2*pmap.fStretch );
				fVStretch = 1;
			}
			else
			{
				fUStretch = 1;
				fVStretch = .5*RealPow(10, -2*pmap.fStretch );
			}
		}  break;
	}
	// Noise
	fVoronoi.SetSeed(pmap.fSeed);

	// Get the slopes values
	switch(pmap.fBottomSlope)
	{
	case 'Opt1': fMortarBottomSlope = 0; break;
	case 'Opt2': fMortarBottomSlope = 1; break;
	case 'Opt3': fMortarBottomSlope = 2; break;
	}
	switch(pmap.fTopSlope)
	{
	case 'Opt1': fMortarTopSlope = 0; break;
	case 'Opt2': fMortarTopSlope = 1; break;
	case 'Opt3': fMortarTopSlope = 2; break;
	}

	fType = pmap.fType;
	fMortarSize = pmap.fMortarSize;
	fMortarPlateau = pmap.fMortarPlateau;
	fRandomUVOrigin = pmap.fRandomUVOrigin;
	fProportionnalTileShading = pmap.fProportionnalTileShading;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}



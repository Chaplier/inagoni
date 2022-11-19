/****************************************************************************************************

		NoiseBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

#ifndef __NoiseBase__
#define __NoiseBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "Vector2.h"
#include "Vector3.h"
#include "MCArray.h"
#include "MCClassArray.h"

const int32 kTabSize = 256;
const int32 kTabMask = kTabSize-1;


class TNoiseBase
{
public:
	TNoiseBase();
	virtual ~TNoiseBase();

	// 3D implementations
	real32	GetValueLinear(const TVector3& point);
	real32	GetValueSmooth(const TVector3& point);
	real32	GetValueVerySmooth(const TVector3& point);
	real32	GetSum(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetLinearSum(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetTurbulence(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetPlasma( const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetPlasmaBis( const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetSinWaveNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetBoxNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetSmoothBoxNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate);
	// Extra noises
	real32	RidgedMultifractal(const TVector3& point, const real32 samplingRate);

	// 2D implementations
	real32	GetValueLinear2D(const TVector2& point);
	real32	GetValueSmooth2D(const TVector2& point);
	real32	GetValueVerySmooth2D(const TVector2& point);
	real32	GetValueCosSmooth2D(const TVector2& point);
	real32	GetSum2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetLinearSum2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetTurbulence2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetPlasma2D( TVector2 point, const real32 fractalInc, const real32 samplingRate);
	real32	GetPlasmaBis2D( TVector2 point, const real32 fractalInc, const real32 samplingRate);
	real32	GetSinWaveNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetBoxNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);
	real32	GetSmoothBoxNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate);


	void	SetSeed(int32 seed) {fSeed=seed; fTabIsValid=false;}
	void	SetMaxFrequency(const int32 maxF){fMaxFrequency=maxF;}
	void	InitGradientTab();

protected:

	real32	GradientLattice(	const int32	ix,	
								const int32 iy, 
								const int32 iz,
								const real32 fx, 
								const real32 fy, 
								const real32 fz );
	int32	Index(	const int32	ix,	
					const int32 iy, 
					const int32 iz );

	real32	GradientLattice2D(	const int32	ix,	
								const int32 iy, 
								const real32 fx, 
								const real32 fys );
	int32	Index2D(	const int32	ix,	
						const int32 iy );

	int32	PermutedIndex( const int32 index );

	int32	fPermutationTab[kTabSize];
	real32	fGradientTab[kTabSize*3];	
	boolean	fTabIsValid;
	int32	fMaxFrequency;

	// !!! Last field for the PMap !!!
	int32	fSeed;
};







#endif

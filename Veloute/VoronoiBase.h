/****************************************************************************************************

		VoronoiBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

#ifndef __VoronoiBase__
#define __VoronoiBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "Vector3.h"
#include "MCArray.h"
#include "MCClassArray.h"
#include "MCRandom.h"

// The poison table as it is in the book. We'll need to make our own method
// to generate it ( a possible method is explain in the book ).
static int32 gPointsPerCube[256] = 
{	4,3,1,1,1,2,4,2,2,2,5,1,0,2,1,2,2,0,4,3,2,1,2,1,3,2,2,4,2,2,5,1,2,3,
	2,2,2,2,2,3,2,4,2,5,3,2,2,2,5,3,3,5,2,1,3,3,4,4,2,3,0,4,2,2,2,1,3,2,
	2,2,3,3,3,1,2,0,2,1,1,2,2,2,2,5,3,2,3,2,3,2,2,1,0,2,1,1,2,1,2,2,1,3,
	4,2,2,2,5,4,2,4,2,2,5,4,3,2,2,5,4,3,3,3,5,2,2,2,2,2,3,1,1,4,2,1,3,3,
	4,3,2,4,3,3,3,4,5,1,4,2,4,3,1,2,3,5,3,2,1,3,1,3,3,3,2,3,1,5,5,4,2,2,
	4,1,3,4,1,5,3,3,5,3,4,3,2,2,1,1,1,1,1,2,4,5,4,5,4,2,1,5,1,1,2,3,3,3,
	2,5,2,3,3,2,0,2,1,1,4,2,1,3,2,1,2,2,3,2,5,5,3,4,5,5,2,4,4,5,3,2,2,2,
	1,4,2,3,3,4,2,5,4,2,4,2,2,2,4,5,3,2
};
static int32 gPointsPerCube2[256] = 
{	2,2,1,1,1,2,2,2,2,2,3,1,0,2,1,2,2,0,2,2,2,1,2,1,2,2,2,2,2,2,3,1,2,2,
	2,2,2,2,2,2,2,2,2,3,2,2,2,2,3,2,2,3,2,1,2,2,2,2,2,2,0,2,2,2,2,1,2,2,
	2,2,2,2,2,1,2,0,2,1,1,2,2,2,2,3,2,2,2,2,2,2,2,1,0,2,1,1,2,1,2,2,1,2,
	2,2,2,2,3,2,2,2,2,2,3,2,2,2,2,3,2,2,2,2,3,2,2,2,2,2,2,1,1,2,2,1,2,2,
	2,2,2,2,2,2,2,2,3,1,2,2,2,2,1,2,2,3,2,2,1,2,1,2,2,2,2,2,1,3,3,2,2,2,
	2,1,2,2,1,3,2,2,3,2,2,2,2,2,1,1,1,1,1,2,2,3,2,3,2,2,1,3,1,1,2,2,2,2,
	2,3,2,2,2,2,0,2,1,1,2,2,1,2,2,1,2,2,2,2,3,3,2,2,3,3,2,2,2,3,2,2,2,2,
	1,2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,2
};

// This value is choosen to make sure that the mean value of f[0] is 1.0
static const real32 kDensityAdjustment = .39815f;

static const int32 gMaxValues=5;

class TVoronoiBase
{
public:
	TVoronoiBase();
	virtual ~TVoronoiBase();

	void	SetSeed(int32 seed) {	MCSetRandomSeed(seed);
									fSeed1=MCRandom();
									fSeed2=MCRandom();
									fSeed3=MCRandom();}
	void	SetMaxFrequency(const int32 maxF){fMaxFrequency=maxF;}

	// Optimized versions in 3D
	void	GetF1AndF2(	const real32 x, const real32 y, const real32 z,
						const int32 distanceMethod,
						real32& F1, real32& F2 );
	void	GetFractalF1AndF2(	const real32 x, const real32 y, const real32 z,
								const int32 distanceMethod, const real32 fractalInc, const real32 samplingRate,
								real32& F1, real32& F2 );

	// Optimized versions in 2D
	void	GetF1AndF2(	const real32 U, const real32 V,
						const int32 distanceMethod,
						real32& F1, real32& F2 );
	void	GetFractalF1AndF2(	const real32 U, const real32 V,
								const int32 distanceMethod, const real32 fractalInc, const real32 samplingRate,
								real32& F1, real32& F2 );

	// Optimized version to return F2-F1 (and the IDs and center) in a 2D space
	// Used for tiling
	void GetF1AndF2(const real32 U, const real32 V,
					  const real32 UStretch, const real32 VStretch,
					  const int32 distanceMethod,
					  real32& F1, real32& F2,
					  uint32& id1, uint32& id2,
					  TVector2& center1);

	// Development version
	real32 GetResult(const TVector3& point,
					 const real32 param1, const real32 param2, const real32 param3, 
					 const int32 flag1, const int32 flag2 );
	TVector3 GetDelta(const TVector3& point,
					 const real32 param1, const real32 param2, const real32 param3, 
					 const int32 flag1, const int32 flag2 );

protected:
	// Compute the n first distance values F. n must be <gMaxValues
	// return an ID that labels the point
	uint32 GetFValues( const TVector3& point, const int32 n );

	void TestCube(const int32 iX, const int32 iY, const int32 iZ, const int32 n,
							const TVector3& localPoint );

	// Optimized version to return F1 (and the ID) in a 3D space
	void TestCubeForF1AndF2(	const int32 iX, const int32 iY, const int32 iZ,
								const real32 localX, const real32 localY, const real32 localZ,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2 );

	// Optimized version to return F1 (and the ID) in a 2D space
	void TestSquareForF1AndF2(	const int32 iX, const int32 iY,
								const real32 localU, const real32 localV,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2 );

	// Optimized version to return F2-F1 (and the IDs and center) in a 2D space
	// Used for tiling
	void TestSquareForF1AndF2(	const int32 iX, const int32 iY,
								const real32 localU, const real32 localV,
								const real32 UStretch, const real32 VStretch,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2, 
								uint32& currentID1, uint32& currentID2,
								TVector2& center1 );

	// Development version of the methods
	uint32 GetFValues( const TVector3& point, const int32 n,
							 const real32 param1, const real32 param2, const real32 param3, 
							 const int32 flag1, const int32 flag2 );

	void TestCube(const int32 iX, const int32 iY, const int32 iZ, const int32 n,
							const TVector3& localPoint,
							 const real32 param1, const real32 param2, const real32 param3, 
							 const int32 flag1, const int32 flag2 );


	TMCArray<real32> fFValues;
	TMCArray<real32> fIDs;
	TMCArray<TVector3> fDeltas;

	int32	fMaxFrequency;
	int32	fSeed2;
	int32	fSeed3;

	// !!! Last field for the PMap !!!
	int32	fSeed1;
};







#endif

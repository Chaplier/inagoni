/****************************************************************************************************

		VoronoiBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

/*	Copyright 1994, 2002 by Steven Worley
	This software may be modified and redistributed without restriction
	provided this comment header remains intact in the source code.
	This code is provided with no warrantee, express or implied, for any purpose.

	A detailed description and application examples can be found in the 
	1996 SIGGRAPH paper "A Cellular Texture Basis Function" and 
	especially in the 2003 book  Texture & Modeling, A Procedural
	Approach, 3rd edition. There is also extra information on the web
	site http://www.worley.com/cellular.html.

	If you do find interinsting uses for this tool, and especially if
	you enhance it, please drop me an email at steve@worley.com.
*/

#include "VoronoiBase.h"

#include "Utils.h"
#include "MCRandom.h"
#include "Vector2.h"

TVoronoiBase::TVoronoiBase()
{
	fFValues.SetElemCount(gMaxValues);
	fIDs.SetElemCount(gMaxValues);
	fDeltas.SetElemCount(gMaxValues);

	fSeed1=0; //
	fSeed2=0; //
	fSeed3=0; //
}

TVoronoiBase::~TVoronoiBase()
{
}

////////////////////////////////////////////////

////////////////////////////////////////////////
// Optimized for F1 and F2 in 2D with some others data
void TVoronoiBase::GetF1AndF2(const real32 U, const real32 V,
								  const real32 UStretch, const real32 VStretch,
								  const int32 distanceMethod,
								  real32& F1, real32& F2,
								  uint32& id1, uint32& id2,
								  TVector2& center1)
{
	// local copy
	const real32 localU = kDensityAdjustment*U;
	const real32 localV = kDensityAdjustment*V;

	// Find the integer square holding this point
	const int32 iU = (int32)RealFloor(localU);
	const int32 iV = (int32)RealFloor(localV);

	F1=kRealMax;
	F2=kRealMax;

	// Test the central cube for closest points
	TestSquareForF1AndF2(iU,iV,localU, localV, UStretch, VStretch, distanceMethod, 
						F1, F2, id1, id2, center1);

	// Test if neighbor cube are possible contributors
	real32 x2 = localU-iU;
	real32 y2 = localV-iV;
	const real32 mx2 = (1-x2)*(1-x2);
	const real32 my2 = (1-y2)*(1-y2);
	x2*=x2;
	y2*=y2;

	// Test the 4 closest neighbors
	if(x2<F2) TestSquareForF1AndF2(iU-1,iV,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(y2<F2) TestSquareForF1AndF2(iU,iV-1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(mx2<F2) TestSquareForF1AndF2(iU+1,iV,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(my2<F2) TestSquareForF1AndF2(iU,iV+1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );

	// Test the 4 corners
	if(x2+y2<F2) TestSquareForF1AndF2(iU-1,iV-1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(x2+my2<F2) TestSquareForF1AndF2(iU-1,iV+1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(mx2+y2<F2) TestSquareForF1AndF2(iU+1,iV-1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );
	if(mx2+my2<F2) TestSquareForF1AndF2(iU+1,iV+1,localU, localV, UStretch, VStretch, distanceMethod, F1, F2, id1, id2, center1 );

	// Done: convert the result
	F1 = RealSqrt(F1)/kDensityAdjustment;
	F2 = RealSqrt(F2)/kDensityAdjustment;
	center1.x /= kDensityAdjustment;
	center1.y /= kDensityAdjustment;

//	return result2-result1;
}

void TVoronoiBase::TestSquareForF1AndF2(const int32 iX, const int32 iY,
								const real32 localU, const real32 localV,
								const real32 UStretch, const real32 VStretch,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2, 
								uint32& currentID1, uint32& currentID2,
								TVector2& center1 )
{
	// It's just a fast way to get a seed
	uint32 seed=fSeed1*iX + fSeed2*iY;

	// How many points in this square
	const int32 pointCount = 3;//gPointsPerCube[seed>>24];

	// churn the seed
	seed=fSeed2+fSeed1*seed;

	// Test and insert each point in our solution
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		const uint32 thisID = seed;

		// Compute the 0 .. 1 feature point location
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fx = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fy = (seed+.5)/4294967296.0;

		seed=fSeed2+fSeed1*seed; // churn the seed

		// delta from feature point to sample location
		const real32 dx = iX+fx-localU;
		const real32 dy = iY+fy-localV;

		// Distance computation
//		const real32 distance = UStretch*dx*dx + VStretch*dy*dy; // euclid biased
		// Distance computation
		real32 distance = 0;
		switch( distanceMethod )
		{
		case 0: distance = UStretch*dx*dx + VStretch*dy*dy; break; // euclid
		case 5: distance = 1.4*(RealAbs(UStretch*dx*dx*dx)+RealAbs(VStretch*dy*dy*dy)); break;
		case 8: distance = (MC_Max(RealAbs(UStretch*dx),RealAbs(VStretch*dy))); break;
		case 10: distance = (RealAbs(UStretch*dx)+RealAbs(VStretch*dy)); break;
		case 12: distance = (UStretch*RealPow( .3f,RealAbs(.3f/(dx)) )+VStretch*RealPow( .3f,RealAbs(.3f/(dy)) ))*.5; break;
		}

		// is this point close enought to remember
		if(distance<currentSolution2)
		{
			// Insert the information
			if(distance<currentSolution1)
			{
				currentSolution2 = currentSolution1;
				currentID2 = currentID1;
				
				currentSolution1 = distance;
				currentID1 = thisID;

				center1.x = iX+fx;
				center1.y = iY+fy;
			}
			else
			{
				currentSolution2 = distance;
				currentID2 = thisID;
			}
		}
	}
}

//////////////////////////////////////////////
// Optimized for F1 and F2 in 3D
void TVoronoiBase::GetF1AndF2(	const real32 x, const real32 y, const real32 z,
								const int32 distanceMethod,
								real32& F1, real32& F2 )
{
	// local copy
	const real32 localX = kDensityAdjustment*x;
	const real32 localY = kDensityAdjustment*y;
	const real32 localZ = kDensityAdjustment*z;

	// Find the integer square holding this point
	const int32 iPointX = (int32)RealFloor(localX);
	const int32 iPointY = (int32)RealFloor(localY);
	const int32 iPointZ = (int32)RealFloor(localZ);

	F1=kRealMax;
	F2=kRealMax;

	// Test the central cube for closest points
	TestCubeForF1AndF2(iPointX,iPointY,iPointZ, localX, localY, localZ, distanceMethod, F1, F2 );

	// Test if neighbor cube are possible contributors
	real32 x2 = localX-iPointX;
	real32 y2 = localY-iPointY;
	real32 z2 = localZ-iPointZ;
	const real32 mx2 = (1-x2)*(1-x2);
	const real32 my2 = (1-y2)*(1-y2);
	const real32 mz2 = (1-z2)*(1-z2);
	x2*=x2;
	y2*=y2;
	z2*=z2;

	// Test the 6 closest neighbors
	if(x2<F2) TestCubeForF1AndF2(iPointX-1,iPointY,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(y2<F2) TestCubeForF1AndF2(iPointX,iPointY-1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(z2<F2) TestCubeForF1AndF2(iPointX,iPointY,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2<F2) TestCubeForF1AndF2(iPointX+1,iPointY,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(my2<F2) TestCubeForF1AndF2(iPointX,iPointY+1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mz2<F2) TestCubeForF1AndF2(iPointX,iPointY,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );

	// Test, if necessary, the 12 next closest cubes
	if(x2+y2<F2) TestCubeForF1AndF2(iPointX-1,iPointY-1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+z2<F2) TestCubeForF1AndF2(iPointX-1,iPointY,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(y2+z2<F2) TestCubeForF1AndF2(iPointX,iPointY-1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+my2<F2) TestCubeForF1AndF2(iPointX+1,iPointY+1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+mz2<F2) TestCubeForF1AndF2(iPointX+1,iPointY,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(my2+mz2<F2) TestCubeForF1AndF2(iPointX,iPointY+1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+my2<F2) TestCubeForF1AndF2(iPointX-1,iPointY+1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+mz2<F2) TestCubeForF1AndF2(iPointX-1,iPointY,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(y2+mz2<F2) TestCubeForF1AndF2(iPointX,iPointY-1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+y2<F2) TestCubeForF1AndF2(iPointX+1,iPointY-1,iPointZ,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+z2<F2) TestCubeForF1AndF2(iPointX+1,iPointY,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(my2+z2<F2) TestCubeForF1AndF2(iPointX,iPointY+1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );

	// Test finally the 8 corner cubes
	if(x2+y2+z2<F2) TestCubeForF1AndF2(iPointX-1,iPointY-1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+y2+mz2<F2) TestCubeForF1AndF2(iPointX-1,iPointY-1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+my2+z2<F2) TestCubeForF1AndF2(iPointX-1,iPointY+1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(x2+my2+mz2<F2) TestCubeForF1AndF2(iPointX-1,iPointY+1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+y2+z2<F2) TestCubeForF1AndF2(iPointX+1,iPointY-1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+y2+mz2<F2) TestCubeForF1AndF2(iPointX+1,iPointY-1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+my2+z2<F2) TestCubeForF1AndF2(iPointX+1,iPointY+1,iPointZ-1,localX,localY,localZ, distanceMethod, F1, F2 );
	if(mx2+my2+mz2<F2) TestCubeForF1AndF2(iPointX+1,iPointY+1,iPointZ+1,localX,localY,localZ, distanceMethod, F1, F2 );

	// Done: convert the result
	F1 = RealSqrt(F1)/kDensityAdjustment;
	F2 = RealSqrt(F2)/kDensityAdjustment;
}

// Does a fractal sum
void TVoronoiBase::GetFractalF1AndF2(	const real32 x, const real32 y, const real32 z,
										const int32 distanceMethod, const real32 fractalInc, const real32 samplingRate,
										real32& F1, real32& F2 )
{
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	real32 absorber = 1;
	real32 localF1=0, localF2=0;
	const real32 freqMax=.5*cutoff;
	int32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		GetF1AndF2( f*x, f*y, f*z, distanceMethod, localF1, localF2 );
	
		F1 += coeff*(localF1)/f;
		F2 += coeff*(localF2)/f;
		absorber += coeff;
		coeff*=fractalInc;
	}
	{
		const real32 fadedCoeff = coeff*Clamp(0,1,2*(cutoff-f)/cutoff);
		GetF1AndF2( f*x, f*y, f*z, distanceMethod, localF1, localF2 );
	
		F1 += fadedCoeff*(localF1)/f;
		F2 += fadedCoeff*(localF2)/f;
		absorber += fadedCoeff;
	}

	// Bring the value back to a [0,2] domain
	F1/=absorber;
	F2/=absorber;
}
	


void TVoronoiBase::TestCubeForF1AndF2(const int32 iX, const int32 iY, const int32 iZ,
								const real32 localX, const real32 localY, const real32 localZ,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2 )
{
	// It's just a fast way to get a seed
	uint32 seed=fSeed1*iX + fSeed2*iY + fSeed3*iZ;

	// How many points in this square
	const int32 pointCount = gPointsPerCube[seed>>24];//4;

	// churn the seed
	seed=fSeed2+fSeed1*seed;

	// Test and insert each point in our solution
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		// Compute the 0 .. 1 feature point location
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fx = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fy = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fz = (seed+.5)/4294967296.0;

		seed=fSeed2+fSeed1*seed; // churn the seed

		// delta from feature point to sample location
		const real32 dx = iX+fx-localX;
		const real32 dy = iY+fy-localY;
		const real32 dz = iZ+fz-localZ;

		// Distance computation
		real32 distance = 0;
		switch( distanceMethod )
		{
		case 0: distance = dx*dx + dy*dy + dz*dz; break; // euclid
		case 4: distance = (RealAbs(dx*dx*dx+dy*dy*dy+dz*dz*dz)); break;
		case 5: distance = 1.4*(RealAbs(dx*dx*dx)+RealAbs(dy*dy*dy)+RealAbs(dz*dz*dz)); break;
		case 6: distance = RealAbs(2*dx*dy*dz); break;
		case 7: distance = (MC_Min(RealAbs(dx),RealAbs(dy),RealAbs(dz))); break;
		case 8: distance = (MC_Max(RealAbs(dx),RealAbs(dy),RealAbs(dz))); break;
		case 9: distance = RealAbs((dx+dy+dz)); break;
		case 10: distance = (RealAbs(dx)+RealAbs(dy)+RealAbs(dz)); break;
		case 11: distance = (RealPow( .9f,RealAbs(.3f/dx) )+RealPow( .9f,RealAbs(.3f/dy) )+RealPow( .9f,RealAbs(.3f/dz) ))*.5; break;
		case 12: distance = (RealPow( .3f,RealAbs(.3f/dx) )+RealPow( .3f,RealAbs(.3f/dy) )+RealPow( .3f,RealAbs(.3f/dz) ))*.5; break;
		}

		// is this point close enought to remember
		if(distance<currentSolution2)
		{
			// Insert the information
			if(distance<currentSolution1)
			{
				currentSolution2 = currentSolution1;
				
				currentSolution1 = distance;
			}
			else
			{
				currentSolution2 = distance;
			}
		}
	}
}

//////////////////////////////////////////////
// Optimized for F1 and F2 in 2D
void TVoronoiBase::GetF1AndF2(	const real32 U, const real32 V,
								const int32 distanceMethod,
								real32& F1, real32& F2 )
{
	// local copy
	const real32 localU = kDensityAdjustment*U;
	const real32 localV = kDensityAdjustment*V;

	// Find the integer square holding this point
	const int32 iU = (int32)RealFloor(localU);
	const int32 iV = (int32)RealFloor(localV);

	F1=kRealMax;
	F2=kRealMax;

	// Test the central cube for closest points
	TestSquareForF1AndF2(iU,iV,localU, localV, distanceMethod, F1, F2 );

	// Test if neighbor cube are possible contributors
	real32 x2 = localU-iU;
	real32 y2 = localV-iV;
	const real32 mx2 = (1-x2)*(1-x2);
	const real32 my2 = (1-y2)*(1-y2);
	x2*=x2;
	y2*=y2;

	// Test the 4 closest neighbors
	if(x2<F2) TestSquareForF1AndF2(iU-1,iV,localU, localV, distanceMethod, F1, F2 );
	if(y2<F2) TestSquareForF1AndF2(iU,iV-1,localU, localV, distanceMethod, F1, F2 );
	if(mx2<F2) TestSquareForF1AndF2(iU+1,iV,localU, localV, distanceMethod, F1, F2 );
	if(my2<F2) TestSquareForF1AndF2(iU,iV+1,localU, localV, distanceMethod, F1, F2 );

	// Test the 4 corners
	if(x2+y2<F2) TestSquareForF1AndF2(iU-1,iV-1,localU, localV, distanceMethod, F1, F2 );
	if(x2+my2<F2) TestSquareForF1AndF2(iU-1,iV+1,localU, localV, distanceMethod, F1, F2 );
	if(mx2+y2<F2) TestSquareForF1AndF2(iU+1,iV-1,localU, localV, distanceMethod, F1, F2 );
	if(mx2+my2<F2) TestSquareForF1AndF2(iU+1,iV+1,localU, localV, distanceMethod, F1, F2 );

	// Done: convert the result
	F1 = RealSqrt(F1)/kDensityAdjustment;
	F2 = RealSqrt(F2)/kDensityAdjustment;
}

// Does a fractal sum
void TVoronoiBase::GetFractalF1AndF2(	const real32 U, const real32 V,
										const int32 distanceMethod, const real32 fractalInc, const real32 samplingRate,
										real32& F1, real32& F2 )
{
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	real32 absorber = 1;
	real32 localF1=0, localF2=0;
	const real32 freqMax=.5*cutoff;
	int32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		GetF1AndF2( f*U, f*V, distanceMethod, localF1, localF2 );
	
		F1 += coeff*(localF1)/f;
		F2 += coeff*(localF2)/f;
		absorber += coeff;
		coeff*=fractalInc;
	}
	{
		const real32 fadedCoeff = coeff*Clamp(0,1,2*(cutoff-f)/cutoff);
		GetF1AndF2( f*U, f*V, distanceMethod, localF1, localF2 );
	
		F1 += fadedCoeff*(localF1)/f;
		F2 += fadedCoeff*(localF2)/f;
		absorber += fadedCoeff;
	}

	// Bring the value back to a [0,2] domain
	F1/=absorber;
	F2/=absorber;
}
	


void TVoronoiBase::TestSquareForF1AndF2(const int32 iX, const int32 iY,
								const real32 localU, const real32 localV,
								const int32 distanceMethod,
								real32& currentSolution1, real32& currentSolution2 )
{
	// It's just a fast way to get a seed
	uint32 seed=fSeed1*iX + fSeed2*iY;

	// How many points in this square
	const int32 pointCount = gPointsPerCube[seed>>24];//4;

	// churn the seed
	seed=fSeed2+fSeed1*seed;

	// Test and insert each point in our solution
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		// Compute the 0 .. 1 feature point location
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fx = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fy = (seed+.5)/4294967296.0;

		seed=fSeed2+fSeed1*seed; // churn the seed

		// delta from feature point to sample location
		const real32 dx = iX+fx-localU;
		const real32 dy = iY+fy-localV;

		// Distance computation
		real32 distance = 0;
		switch( distanceMethod )
		{
		case 0: distance = dx*dx + dy*dy; break; // euclid
		case 4: distance = (RealAbs(dx*dx*dx+dy*dy*dy)); break;
		case 5: distance = 1.4*(RealAbs(dx*dx*dx)+RealAbs(dy*dy*dy)); break;
		case 6: distance = RealAbs(2*dx*dy); break;
		case 7: distance = (MC_Min(RealAbs(dx),RealAbs(dy))); break;
		case 8: distance = (MC_Max(RealAbs(dx),RealAbs(dy))); break;
		case 9: distance = RealAbs((dx+dy)); break;
		case 10: distance = (RealAbs(dx)+RealAbs(dy)); break;
		case 11: distance = (RealPow( .9f,RealAbs(.3f/dx) )+RealPow( .9f,RealAbs(.3f/dy) ))*.5; break;
		case 12: distance = (RealPow( .3f,RealAbs(.3f/dx) )+RealPow( .3f,RealAbs(.3f/dy) ))*.5; break;
		}

		// is this point close enought to remember
		if(distance<currentSolution2)
		{
			// Insert the information
			if(distance<currentSolution1)
			{
				currentSolution2 = currentSolution1;
				
				currentSolution1 = distance;
			}
			else
			{
				currentSolution2 = distance;
			}
		}
	}
}

///////////////////////////////////////////
// Default implementation optimized to return 5 distances, 5 deltas and 5 ids
// If less or more is needed, a specific can be made from this one
uint32 TVoronoiBase::GetFValues( const TVector3& point, const int32 n )
{
	for( int32 index=0 ; index<n ; index++ )
	{
		fFValues[index] = kRealMax;
	}

	// local copy
	TVector3 localPoint = kDensityAdjustment*point;

	// Find the integer cube holding this point
	const int32 iPointX = (int32)RealFloor(localPoint[0]);
	const int32 iPointY = (int32)RealFloor(localPoint[1]);
	const int32 iPointZ = (int32)RealFloor(localPoint[2]);

	// Test the central cube for closest points
	TestCube(iPointX,iPointY,iPointZ,n,localPoint);

	// Test if neighbor cube are possible contributors
	real32 x2 = localPoint[0]-iPointX;
	real32 y2 = localPoint[1]-iPointY;
	real32 z2 = localPoint[2]-iPointZ;
	const real32 mx2 = (1-x2)*(1-x2);
	const real32 my2 = (1-y2)*(1-y2);
	const real32 mz2 = (1-z2)*(1-z2);
	x2*=x2;
	y2*=y2;
	z2*=z2;

	const int32 nMinusOne = n-1;

	// Test the 6 closest neighbors
	if(x2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ,n,localPoint);
	if(y2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ,n,localPoint);
	if(z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY,iPointZ-1,n,localPoint);
	if(mx2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ,n,localPoint);
	if(my2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ,n,localPoint);
	if(mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY,iPointZ+1,n,localPoint);

	// Test, if necessary, the 12 next closest cubes
	if(x2+y2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ,n,localPoint);
	if(x2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ-1,n,localPoint);
	if(y2+z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ-1,n,localPoint);
	if(mx2+my2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ,n,localPoint);
	if(mx2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ+1,n,localPoint);
	if(my2+mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ+1,n,localPoint);
	if(x2+my2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ,n,localPoint);
	if(x2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ+1,n,localPoint);
	if(y2+mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ+1,n,localPoint);
	if(mx2+y2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ,n,localPoint);
	if(mx2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ-1,n,localPoint);
	if(my2+z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ-1,n,localPoint);

	// Test finally the 8 corner cubes
	if(x2+y2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ-1,n,localPoint);
	if(x2+y2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ+1,n,localPoint);
	if(x2+my2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ-1,n,localPoint);
	if(x2+my2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ+1,n,localPoint);
	if(mx2+y2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ-1,n,localPoint);
	if(mx2+y2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ+1,n,localPoint);
	if(mx2+my2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ-1,n,localPoint);
	if(mx2+my2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ+1,n,localPoint);

	// Done: convert the values
	for( int32 iValue=0 ; iValue<n ; iValue++ )
	{
		fFValues[iValue] = RealSqrt(fFValues[iValue])/kDensityAdjustment;
		fDeltas[iValue] /= kDensityAdjustment;
	}

	return 0;
}

void TVoronoiBase::TestCube(const int32 iX, const int32 iY, const int32 iZ, const int32 n,
							const TVector3& localPoint )
{
	// It's just a fast way to get a seed
	uint32 seed=fSeed1*iX + fSeed2*iY + fSeed3*iZ;

	// How many points in this cube
	const int32 pointCount = gPointsPerCube[seed>>24];

	// churn the seed
	seed=fSeed2+fSeed1*seed; // churn the seed

	// Test and insert each point in our solution
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		const uint32 thisID = seed;


		// Compute the 0 .. 1 feature point location
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fx = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fy = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fz = (seed+.5)/4294967296.0;

		seed=fSeed2+fSeed1*seed; // churn the seed

		// delta from feature point to sample location
		const real32 dx = iX+fx-localPoint[0];
		const real32 dy = iY+fy-localPoint[1];
		const real32 dz = iZ+fz-localPoint[2];

		// Distance computation
		// Try with Biased, Manhattan, RadialManhattan, SuperQuadric, ...
		const real32 distance = dx*dx + dy*dy + dz*dz; // euclid
		// const real32 distance = 2*dx*dx + 2.5*dy*dy + 3*dz*dz; // Biased
		// const real32 distance = RealAbs(dx) + RealAbs(dy) + RealAbs(dz); //Manhattan
		// const real32 distance = RealPow(RealAbs(dx),2) + RealPow(RealAbs(dy),2.5) + RealPow(RealAbs(dz),3); // Superquadric
		// const real32 distance = (dx+dy+dz)*(dx+dy+dz); // kind of zebra
		// const real32 distance = RealPow(RealAbs(dx),RealAbs(dx))+RealPow(RealAbs(dy),RealAbs(dy))+RealPow(RealAbs(dz),RealAbs(dz));

		// is this point close enought to remember
		if(distance<fFValues[n-1])
		{
			// insert the information
			int32 index = n;
			while(index>0 && distance<fFValues[index-1]) index--;
			
			for(int32 i=n ; i-->index;)
			{
				fFValues[i+1] = fFValues[i];
				fIDs[i+1] = fIDs[i];
				fDeltas[i+1] = fDeltas[i];
			}
			fFValues[index] = distance;
			fIDs[index] = thisID;
			fDeltas[index] = TVector3(dx,dy,dz);
		}
	}
}

///
TVector3 TVoronoiBase::GetDelta(const TVector3& point,
	 const real32 param1, const real32 param2, const real32 param3, 
	 const int32 flag1, const int32 flag2 )
{
			GetFValues( point, 1, param1, param2, param3, flag1, flag2 );
			return fDeltas[0];
}
// return  a result between 0 and 2
real32 TVoronoiBase::GetResult(const TVector3& point,
	 const real32 param1, const real32 param2, const real32 param3, 
	 const int32 flag1, const int32 flag2 )
{
	// Fractal test version
	real32 value0 = 0;
	real32 value1 = 0;

	real32 samplingRate = 0.01f;
	real32 fMaxFrequency = 30; // level of detail

	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	// SUM
	real32 absorber = 1;
	const real32 freqMax=.5*cutoff;
	int32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		GetFValues( f*point, 2, param1, param2, param3, flag1, 0 );
	
		value0 += coeff*(fFValues[0])/f;
		value1 += coeff*(fFValues[1])/f;
		absorber += coeff;
		coeff*=param1;
	}
	{
		const real32 fadedCoeff = coeff*Clamp(0,1,2*(cutoff-f)/cutoff);
		GetFValues( f*point, 2, param1, param2, param3, flag1, 0 );
	
		value0 += fadedCoeff*(fFValues[0])/f;
		value1 += fadedCoeff*(fFValues[1])/f;
		absorber += fadedCoeff;
	}

	// Bring the value back to a [0,2] domain
	value0/=absorber;
	value1/=absorber;
	

	switch(flag2)
	{
	case 0: return value0;
	case 1: return value1;
	case 2:
		{
			GetFValues( point, 3, param1, param2, param3, flag1, 0 );
			return fFValues[2];
		} 
	case 3:
		{
			GetFValues( point, 4, param1, param2, param3, flag1, 0 );
			return fFValues[3];
		} 
	case 4:
		{
			GetFValues( point, 5, param1, param2, param3, flag1, 0 );
			return fFValues[4];
		} 
	case 5: return (value1-value0);
	case 6:
		{
			const real32 a = value0*.5;
			const real32 b = value1*.5;
			return 2*(b*b-a*a);
		} 
	case 7: return (value0+value1)*.5;
	case 8: return (value0*value1)*.5;
	case 9: return (1+cos(value0*PI*.5));
	case 10: return (2*sin(value0*PI*.5));
	case 11:
		{
			const real32 a = (1+cos(value0*PI*.5))*.5;
			const real32 b = (sin(value1*PI*.5))*.5;
			return (a+b);
		} 
	case 12: return (1+cos(value0*PI*10));
	}

	switch(flag2)
	{
	case 0:
		{
			GetFValues( point, 1, param1, param2, param3, flag1, 0 );
			return fFValues[0];
		} 
	case 1:
		{
			GetFValues( point, 2, param1, param2, param3, flag1, 0 );
			return fFValues[1];
		} 
	case 2:
		{
			GetFValues( point, 3, param1, param2, param3, flag1, 0 );
			return fFValues[2];
		} 
	case 3:
		{
			GetFValues( point, 4, param1, param2, param3, flag1, 0 );
			return fFValues[3];
		} 
	case 4:
		{
			GetFValues( point, 5, param1, param2, param3, flag1, 0 );
			return fFValues[4];
		} 
	case 5:
		{
			GetFValues( point, 2, param1, param2, param3, flag1, 0 );
			return (fFValues[1]-fFValues[0]);
		} 
	case 6:
		{
			GetFValues( point, 3, param1, param2, param3, flag1, 0 );
			const real32 a = fFValues[0]*.5;
			const real32 b = fFValues[1]*.5;
			return 2*(b*b-a*a);
		} 
	case 7:
		{
			GetFValues( point, 2, param1, param2, param3, flag1, 0 );
			return (fFValues[1]+fFValues[0])*.5;
		} 
	case 8:
		{
			GetFValues( point, 2, param1, param2, param3, flag1, 0 );
			return (fFValues[1]*fFValues[0])*.5;
		} 
	case 9:
		{
			GetFValues( point, 1, param1, param2, param3, flag1, 0 );
			return (1+cos(fFValues[0]*PI*.5));
		} 
	case 10:
		{
			GetFValues( point, 1, param1, param2, param3, flag1, 0 );
			return (2*sin(fFValues[0]*PI*.5));
		} 
	case 11:
		{
			GetFValues( point, 2, param1, param2, param3, flag1, 0 );
			const real32 a = (1+cos(fFValues[0]*PI*.5))*.5;
			const real32 b = (sin(fFValues[1]*PI*.5))*.5;
			return (a+b);
		} 
	case 12:
		{
			GetFValues( point, 1, param1, param2, param3, flag1, 0 );
			return (1+cos(fFValues[0]*PI*10));
		} 
	}

	return 1.0;
}

// Development version: not well optimized
uint32 TVoronoiBase::GetFValues( const TVector3& point, const int32 n,
								 const real32 param1, const real32 param2, const real32 param3, 
								 const int32 flag1, const int32 flag2 )
{
	for( int32 index=0 ; index<n ; index++ )
	{
		fFValues[index] = kRealMax;
	}

	// local copy
	TVector3 localPoint = kDensityAdjustment*point;

	// Find the integer cube holding this point
	const int32 iPointX = (int32)RealFloor(localPoint[0]);
	const int32 iPointY = (int32)RealFloor(localPoint[1]);
	const int32 iPointZ = (int32)RealFloor(localPoint[2]);

	// Test the central cube for closest points
	TestCube(iPointX,iPointY,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);

	// Test if neighbor cube are possible contributors
	real32 x2 = localPoint[0]-iPointX;
	real32 y2 = localPoint[1]-iPointY;
	real32 z2 = localPoint[2]-iPointZ;
	const real32 mx2 = (1-x2)*(1-x2);
	const real32 my2 = (1-y2)*(1-y2);
	const real32 mz2 = (1-z2)*(1-z2);
	x2*=x2;
	y2*=y2;
	z2*=z2;

	const int32 nMinusOne = n-1;

	// Test the 6 closest neighbors
	if(x2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(y2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(my2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);

	// Test, if necessary, the 12 next closest cubes
	if(x2+y2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(y2+z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+my2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(my2+mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+my2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(y2+mz2<fFValues[nMinusOne]) TestCube(iPointX,iPointY-1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+y2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(my2+z2<fFValues[nMinusOne]) TestCube(iPointX,iPointY+1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);

	// Test finally the 8 corner cubes
	if(x2+y2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+y2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY-1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+my2+z2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(x2+my2+mz2<fFValues[nMinusOne]) TestCube(iPointX-1,iPointY+1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+y2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+y2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY-1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+my2+z2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ-1,n,localPoint, param1, param2, param3, flag1, flag2);
	if(mx2+my2+mz2<fFValues[nMinusOne]) TestCube(iPointX+1,iPointY+1,iPointZ+1,n,localPoint, param1, param2, param3, flag1, flag2);

	// Done: convert the values
	for( int32 iValue=0 ; iValue<n ; iValue++ )
	{
		fFValues[iValue] = RealSqrt(fFValues[iValue])/kDensityAdjustment;
		fDeltas[iValue] /= kDensityAdjustment;
	}

	return 0;
}

// development version: not optimized
void TVoronoiBase::TestCube(const int32 iX, const int32 iY, const int32 iZ, const int32 n,
							const TVector3& localPoint,
							 const real32 param1, const real32 param2, const real32 param3, 
							 const int32 flag1, const int32 flag2  )
{
	// It's just a fast way to get a seed
	uint32 seed=fSeed1*iX + fSeed2*iY + fSeed3*iZ;

	// How many points in this cube
	const int32 pointCount = gPointsPerCube[seed>>24];

	// churn the seed
		seed=fSeed2+fSeed1*seed; // churn the seed

	// Test and insert each point in our solution
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		const uint32 thisID = seed;


		// Compute the 0 .. 1 feature point location
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fx = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fy = (seed+.5)/4294967296.0;
		seed=fSeed2+fSeed1*seed; // churn the seed
		const real32 fz = (seed+.5)/4294967296.0;

		seed=fSeed2+fSeed1*seed; // churn the seed

		// delta from feature point to sample location
		const real32 dx = iX+fx-localPoint[0];
		const real32 dy = iY+fy-localPoint[1];
		const real32 dz = iZ+fz-localPoint[2];

		// Distance computation
		// Try with Biased, Manhattan, RadialManhattan, SuperQuadric, ...
		real32 distance = dx*dx + dy*dy + dz*dz; // euclid
		if(flag1 == 1)		distance = param1*dx*dx + param2*dy*dy + param3*dz*dz; // Biased
		else if(flag1 == 2)	distance = RealAbs(dx) + RealAbs(dy) + RealAbs(dz); //Manhattan
		else if(flag1 == 3)	distance = param1*RealAbs(dx) + param2*RealAbs(dy) + param3*RealAbs(dz); //Manhattan
		else if(flag1 == 4)	distance = RealPow(RealAbs(dx),param3) + RealPow(RealAbs(dy),param3) + RealPow(RealAbs(dz),param3); // Superquadric
		else if(flag1 == 5)	distance = RealAbs(dx*dx*dx*1.4)+RealAbs(dy*dy*dy*1.4)+RealAbs(dz*dz*dz*1.4);
		else if(flag1 == 6)	distance = RealAbs(10*dx*dy);
		else if(flag1 == 7)	distance = (MC_Min(RealAbs(dx),RealAbs(dy)));
		else if(flag1 == 8)	distance = (MC_Max(RealAbs(dx),RealAbs(dy)));
		else if(flag1 == 9)	distance = RealAbs((dx+dy)); // note: dx/dy does some kind of vertical strips
		else if(flag1 ==10)	distance = (RealAbs(dx)+RealAbs(dy));
		else if(flag1 ==11)	distance = (RealPow( .9f,RealAbs(.3f/dx) )+RealPow( .9f,RealAbs(.3f/dy) ))*.5;
		else if(flag1 ==12)	distance = (RealPow( .3f,RealAbs(.3f/dx) )+RealPow( .3f,RealAbs(.3f/dy) ))*.5;

		// is this point close enought to remember
		if(distance<fFValues[n-1])
		{
			// insert the information
			int32 index = n;
			while(index>0 && distance<fFValues[index-1]) index--;
			
			for(int32 i=n ; i-->index;)
			{
				fFValues[i+1] = fFValues[i];
				fIDs[i+1] = fIDs[i];
				fDeltas[i+1] = fDeltas[i];
			}
			fFValues[index] = distance;
			fIDs[index] = thisID;
			fDeltas[index] = TVector3(dx,dy,dz);
		}
	}
}
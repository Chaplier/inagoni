/****************************************************************************************************

		NoiseBase.cpp
		Copyright: (c) 2004 . All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

#include "NoiseBase.h"

#include "Utils.h"
#include "MCRandom.h"

static const int32 kPermutationTabSeed=127; // any number is OK, it's just to build always the same permutation table

void Rotate( real32& x,real32& y, const real32 angle) // in rad
{
	const real32 cos = RealCos(angle);
	const real32 sin = RealSin(angle);
	const real32 newX = cos * x - sin * y;
	const real32 newY = sin * x + cos * y;
	x = newX;
	y = newY;
}

TNoiseBase::TNoiseBase()
{
	fTabIsValid = false;
	// first we fill the permutation table with default index, later we're going to 
	for(int32 index=0 ; index<kTabSize ; index++)
	{
		fPermutationTab[index]=index;
	}
	fSeed = MCRandom();

	fMaxFrequency = 100;
}

TNoiseBase::~TNoiseBase()
{
}

// return a value between -1 and 1
real32 TNoiseBase::GetValueLinear(const TVector3& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = BoxStep( kRealZero, kRealOne, fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy =BoxStep( kRealZero, kRealOne, fy0 );

	const int32 iz = RealFloor( point.z );
	const real32 fz0 = point.z - iz;
	const real32 fz1 = fz0 - 1;
	const real32 wz = BoxStep( kRealZero, kRealOne, fz0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0, vz0=0, vz1=0;

	vx0 = GradientLattice( ix, iy, iz, fx0, fy0, fz0 );
	vx1 = GradientLattice( ix+1, iy, iz, fx1, fy0, fz0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz, fx0, fy1, fz0 );
	vx1 = GradientLattice( ix+1, iy+1, iz, fx1, fy1, fz0 );
	vy1 = Lerp(wx, vx0, vx1);

	vz0 = Lerp(wy, vy0, vy1);

	vx0 = GradientLattice( ix, iy, iz+1, fx0, fy0, fz1 );
	vx1 = GradientLattice( ix+1, iy, iz+1, fx1, fy0, fz1 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz+1, fx0, fy1, fz1 );
	vx1 = GradientLattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 );
	vy1 = Lerp(wx, vx0, vx1);

	vz1 = Lerp(wy, vy0, vy1);

	return Lerp(wz, vz0, vz1);
}

// return a value between -1 and 1
// Normal implementation of the Noise
real32 TNoiseBase::GetValueSmooth(const TVector3& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = SmoothStep( fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = SmoothStep( fy0 );

	const int32 iz = RealFloor( point.z );
	const real32 fz0 = point.z - iz;
	const real32 fz1 = fz0 - 1;
	const real32 wz = SmoothStep( fz0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0, vz0=0, vz1=0;

	vx0 = GradientLattice( ix, iy, iz, fx0, fy0, fz0 );
	vx1 = GradientLattice( ix+1, iy, iz, fx1, fy0, fz0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz, fx0, fy1, fz0 );
	vx1 = GradientLattice( ix+1, iy+1, iz, fx1, fy1, fz0 );
	vy1 = Lerp(wx, vx0, vx1);

	vz0 = Lerp(wy, vy0, vy1);

	vx0 = GradientLattice( ix, iy, iz+1, fx0, fy0, fz1 );
	vx1 = GradientLattice( ix+1, iy, iz+1, fx1, fy0, fz1 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz+1, fx0, fy1, fz1 );
	vx1 = GradientLattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 );
	vy1 = Lerp(wx, vx0, vx1);

	vz1 = Lerp(wy, vy0, vy1);

	return Lerp(wz, vz0, vz1);
}

// return a value between -1 and 1
// This one is usefull for the bump: SmootherStep is C4
real32 TNoiseBase::GetValueVerySmooth(const TVector3& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = SmootherStep( fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = SmootherStep( fy0 );

	const int32 iz = RealFloor( point.z );
	const real32 fz0 = point.z - iz;
	const real32 fz1 = fz0 - 1;
	const real32 wz = SmootherStep( fz0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0, vz0=0, vz1=0;

	vx0 = GradientLattice( ix, iy, iz, fx0, fy0, fz0 );
	vx1 = GradientLattice( ix+1, iy, iz, fx1, fy0, fz0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz, fx0, fy1, fz0 );
	vx1 = GradientLattice( ix+1, iy+1, iz, fx1, fy1, fz0 );
	vy1 = Lerp(wx, vx0, vx1);

	vz0 = Lerp(wy, vy0, vy1);

	vx0 = GradientLattice( ix, iy, iz+1, fx0, fy0, fz1 );
	vx1 = GradientLattice( ix+1, iy, iz+1, fx1, fy0, fz1 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz+1, fx0, fy1, fz1 );
	vx1 = GradientLattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 );
	vy1 = Lerp(wx, vx0, vx1);

	vz1 = Lerp(wy, vy0, vy1);

	return Lerp(wz, vz0, vz1);
}

real32 TNoiseBase::GetSum(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*(GetValueVerySmooth(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*(GetValueVerySmooth(point*f))/f;
	return value;
}

real32 TNoiseBase::GetLinearSum(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*(GetValueLinear(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*(GetValueLinear(point*f))/f;
	return value;
}

real32 TNoiseBase::GetTurbulence(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*RealAbs(GetValueVerySmooth(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*RealAbs(GetValueVerySmooth(point*f))/f;
	return value;
}

real32 TNoiseBase::GetPlasma( const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	real32 exp = 0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value += GetValueVerySmooth(point*f) * RealPow(kFractalIncrement, exp);
		exp-=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*GetValueVerySmooth(point*f) * RealPow(kFractalIncrement, exp);

	return value;
}

real32 TNoiseBase::GetPlasmaBis( const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	real32 exp = 0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value += GetValueVerySmooth(point*f) * RealPow(f, exp);
		exp-=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*GetValueVerySmooth(point*f) * RealPow(f, exp);

	return value;
}


real32 TNoiseBase::GetSinWaveNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	const real32 offset = .5;
	real32 xphase = .9f;
	real32 yphase = .7f;
	real32 zphase = .8f;
	real32 xfreq = (real32)(TWO_PI * .023);
	real32 yfreq = (real32)(TWO_PI * .021);
	real32 zfreq = (real32)(TWO_PI * .022);
	real32 A = .75;
	int32 i=0;

	real32 x = point.x*10;
	real32 y = point.y*10;
	real32 z = point.z*10;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement, i++ )
	{
		const real32 fx = A*(sin(xfreq*(x + xphase)));
		const real32 fy = A*(sin(yfreq*(y + yphase)));
		const real32 fz = A*(sin(zfreq*(z + zphase)));

		value+=fx*fy*fz;

		xphase = PI2 * .9 * cos(yfreq * y);
		yphase = PI2 * 1.1 * cos(zfreq * z);
		zphase = PI2 * 1.05 * cos(xfreq * x);

		xfreq *= 1.9 + i*.06;
		yfreq *= 2.2 - i*.08;
		zfreq *= 1.8 + i*.07;

		Rotate(x,y, 2.5);
		Rotate(y,z, .5);
		Rotate(z,x, 2);

		A *= fractalInc;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		const real32 fx = A*(sin(xfreq*(x + xphase)));
		const real32 fy = A*(sin(yfreq*(y + yphase)));
		const real32 fz = A*(sin(zfreq*(z + zphase)));

		value+=fade*fx*fy*fz;
	}

	return value;
}

real32 TNoiseBase::GetBoxNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	real32 x = point.x;
	real32 y = point.y;
	real32 z = point.z;

	TVector3 localPoint(floor(x)+.5,floor(y)+.5,floor(z)+.5);

	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=GetValueLinear(f*localPoint);

		x*=fractalInc;
		y*=fractalInc;
		z*=fractalInc;
		localPoint.x = floor(x)+.5;
		localPoint.y = floor(y)+.5;
		localPoint.z = floor(z)+.5;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		value+=fade*GetValueLinear(f*localPoint);
	}

	return value;
}

real32 TNoiseBase::GetSmoothBoxNoise(const TVector3& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	real32 x = point.x;
	real32 y = point.y;
	real32 z = point.z;
	int32 iX = floor(x);
	int32 iY = floor(y);
	int32 iZ = floor(z);

	real32 sphereCenterX = iX + .5;
	real32 sphereCenterY = iY + .5;
	real32 sphereCenterZ = iZ + .5;

	real32 localx = x-iX;
	real32 localy = y-iY;
	real32 localz = z-iZ;

	TVector3 localPoint(iX+.5,iY+.5,iZ+.5);

	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		const real32 intensity = GetValueLinear(f*localPoint);

		value += intensity	*cos( PI*RealAbs(x-sphereCenterX) )
							*cos( PI*RealAbs(y-sphereCenterY) )
							*cos( PI*RealAbs(z-sphereCenterZ) );
		
		x*=fractalInc;
		y*=fractalInc;
		z*=fractalInc;

		iX = floor(x);
		iY = floor(y);
		iZ = floor(z);

		sphereCenterX = iX + .5;
		sphereCenterY = iY + .5;
		sphereCenterZ = iZ + .5;

		localx = x-iX;
		localy = y-iY;
		localz = z-iZ;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		const real32 intensity = GetValueLinear(f*localPoint);

		value += fade*intensity	*cos( PI*RealAbs(x-sphereCenterX) )
								*cos( PI*RealAbs(y-sphereCenterY) )
								*cos( PI*RealAbs(z-sphereCenterZ) );
	}

	return value;;
}

////////////////////////////////////////////////////////////////////////////////
// 2D implementations

// return a value between -1 and 1
real32 TNoiseBase::GetValueLinear2D(const TVector2& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = BoxStep( kRealZero, kRealOne, fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy =BoxStep( kRealZero, kRealOne, fy0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0;

	vx0 = GradientLattice2D( ix, iy, fx0, fy0 );
	vx1 = GradientLattice2D( ix+1, iy, fx1, fy0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice2D( ix, iy+1, fx0, fy1 );
	vx1 = GradientLattice2D( ix+1, iy+1, fx1, fy1 );
	vy1 = Lerp(wx, vx0, vx1);

	return Lerp(wy, vy0, vy1);
}

// return a value between -1 and 1
// Normal implementation of the Noise
real32 TNoiseBase::GetValueSmooth2D(const TVector2& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = SmoothStep( fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = SmoothStep( fy0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0;

	vx0 = GradientLattice2D( ix, iy, fx0, fy0 );
	vx1 = GradientLattice2D( ix+1, iy, fx1, fy0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice2D( ix, iy+1, fx0, fy1 );
	vx1 = GradientLattice2D( ix+1, iy+1, fx1, fy1 );
	vy1 = Lerp(wx, vx0, vx1);

	return Lerp(wy, vy0, vy1);
}

// return a value between -1 and 1
// This one is usefull for the bump: SmootherStep is C4
real32 TNoiseBase::GetValueVerySmooth2D(const TVector2& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = SmootherStep( fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = SmootherStep( fy0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0;

	vx0 = GradientLattice2D( ix, iy, fx0, fy0 );
	vx1 = GradientLattice2D( ix+1, iy, fx1, fy0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice2D( ix, iy+1, fx0, fy1 );
	vx1 = GradientLattice2D( ix+1, iy+1, fx1, fy1 );
	vy1 = Lerp(wx, vx0, vx1);

	return Lerp(wy, vy0, vy1);
}

real32 TNoiseBase::GetValueCosSmooth2D(const TVector2& point)
{
	if(!fTabIsValid)
		InitGradientTab();

	const int32 ix = RealFloor( point.x );
	const real32 fx0 = point.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = CosStep( fx0 );

	const int32 iy = RealFloor( point.y );
	const real32 fy0 = point.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = CosStep( fy0 );

	real32 vx0=0, vx1=0, vy0=0, vy1=0;

	vx0 = GradientLattice2D( ix, iy, fx0, fy0 );
	vx1 = GradientLattice2D( ix+1, iy, fx1, fy0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice2D( ix, iy+1, fx0, fy1 );
	vx1 = GradientLattice2D( ix+1, iy+1, fx1, fy1 );
	vy1 = Lerp(wx, vx0, vx1);

	return Lerp(wy, vy0, vy1);
}

real32 TNoiseBase::GetSum2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*(GetValueVerySmooth2D(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*(GetValueVerySmooth2D(point*f))/f;
	return value;
}

real32 TNoiseBase::GetLinearSum2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*(GetValueLinear2D(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*(GetValueLinear2D(point*f))/f;
	return value;
}

real32 TNoiseBase::GetTurbulence2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	real32 coeff = 1;
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=coeff*RealAbs(GetValueVerySmooth2D(point*f))/f;
		coeff*=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*coeff*RealAbs(GetValueVerySmooth2D(point*f))/f;
	return value;
}

real32 TNoiseBase::GetPlasma2D( TVector2 point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	real32 exp = 0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value += GetValueVerySmooth2D(point*f) * RealPow(kFractalIncrement, exp);
		exp-=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*GetValueVerySmooth2D(point*f) * RealPow(kFractalIncrement, exp);

	return value;
}

real32 TNoiseBase::GetPlasmaBis2D( TVector2 point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;
	real32 exp = 0;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value += GetValueVerySmooth2D(point*f) * RealPow(f, exp);
		exp-=fractalInc;
	}
	real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
	value+=fade*GetValueVerySmooth2D(point*f) * RealPow(f, exp);

	return value;
}
/*
real32 TNoiseBase::GetPlasma2D( TVector2 point, const real32 fractalInc, const real32 lacunarity, const real32 octaves)
{
	real32 value = 0;
	int32 index=0;

	real32 exp = 0;

	for( index=0 ; index<octaves ; index++ )
	{
		value += GetValueVerySmooth2D(point) * RealPow(lacunarity, exp);
		point*=lacunarity;
		exp-=fractalInc;
	}

	const real32 remainder = octaves - (int32)octaves;
	if(remainder)
	{
		value += remainder * GetValueVerySmooth2D(point) * RealPow(lacunarity, exp);
	}

	return value;
}
*/


real32 TNoiseBase::GetSinWaveNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	const real32 offset = .5;
	real32 xphase = .9f;
	real32 yphase = .7f;
	real32 xfreq = (real32)(TWO_PI * .023);
	real32 yfreq = (real32)(TWO_PI * .021);
	real32 A = .75;
	int32 i=0;

	real32 x = point.x*10;
	real32 y = point.y*10;
	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement, i++ )
	{
		const real32 fx = A*(sin(xfreq*(x + xphase)));
		const real32 fy = A*(sin(yfreq*(y + yphase)));

		value+=fx*fy;

		xphase = PI2 * .9 * cos(yfreq * y);
		yphase = PI2 * 1.1 * cos(xfreq * x);

		xfreq *= 1.9 + i*.06;
		yfreq *= 2.2 - i*.08;

		Rotate(x,y, 2.5);

		A *= fractalInc;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		const real32 fx = A*(sin(xfreq*(x + xphase)));
		const real32 fy = A*(sin(yfreq*(y + yphase)));

		value+=fade*fx*fy;
	}

	return value;
}

real32 TNoiseBase::GetBoxNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	real32 x = point.x;
	real32 y = point.y;

	TVector2 localPoint(floor(x)+.5,floor(y)+.5);

	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		value+=GetValueLinear2D(f*localPoint);

		x*=fractalInc;
		y*=fractalInc;
		localPoint.x = floor(x)+.5;
		localPoint.y = floor(y)+.5;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		value+=fade*GetValueLinear2D(f*localPoint);
	}

	return value;
}

real32 TNoiseBase::GetSmoothBoxNoise2D(const TVector2& point, const real32 fractalInc, const real32 samplingRate)
{
	real32 value=0;

	real32 x = point.x;
	real32 y = point.y;
	int32 iX = floor(x);
	int32 iY = floor(y);

	real32 sphereCenterX = iX + .5;
	real32 sphereCenterY = iY + .5;

	real32 localx = x-iX;
	real32 localy = y-iY;

	TVector2 localPoint(iX+.5,iY+.5);

	const real32 cutoff = Clamp(2,fMaxFrequency,.5/samplingRate);
	const real32 freqMax=.5*cutoff;
	real32 f;
	for( f=kMinFrequency; f<freqMax ; f*=kFractalIncrement )
	{
		const real32 intensity = GetValueLinear2D(f*localPoint);

		value += intensity*cos( PI*RealAbs(x-sphereCenterX) )*cos( PI*RealAbs(y-sphereCenterY) );
		
		x*=fractalInc;
		y*=fractalInc;

		iX = floor(x);
		iY = floor(y);

		sphereCenterX = iX + .5;
		sphereCenterY = iY + .5;

		localx = x-iX;
		localy = y-iY;
	}
	{
		real32 fade=Clamp(0,1,2*(cutoff-f)/cutoff);
		const real32 intensity = GetValueLinear2D(f*localPoint);

		value += fade*intensity*cos( PI*RealAbs(x-sphereCenterX) )*cos( PI*RealAbs(y-sphereCenterY) );
	}

	return value;;
}


////////////////////////////////////////////////////////////////////////////////
void TNoiseBase::InitGradientTab()
{
	if(fTabIsValid)
		return;

	// shake the permutation table
	MCSetRandomSeed(fSeed/*kPermutationTabSeed*/);
	for(int32 iPerm=0 ; iPerm<kTabSize ; iPerm++)
	{
		const int32 randIndex = MCRandom()&kTabMask;
		MCAssert(randIndex>=0 && randIndex<kTabSize);

		// Switch the 2 indexes
		const int32 tmp = fPermutationTab[iPerm];
		fPermutationTab[iPerm] = fPermutationTab[randIndex];
		fPermutationTab[randIndex] = tmp;
	}

	// init the gradient table
	MCSetRandomSeed(fSeed);
	real32 radius=0, theta=0, random=0;

	real32* tabPtr=fGradientTab;
	for(int32 index=0 ; index<kTabSize ; index++)
	{
		random=1-2*FixedToReal(MCRandom()&0xFFFF);
		MCAssert(random>=-1 && random<=1);

		radius=RealSqrt(1-random*random);
		theta=kRealTwoPI*random;
		*tabPtr++ = radius * RealCos(theta);
		*tabPtr++ = radius * RealSin(theta);
		*tabPtr++ = random;
	}

	fTabIsValid = true;
}

inline real32 TNoiseBase::GradientLattice(	const int32	ix,	
											const int32 iy, 
											const int32 iz,
											const real32 fx, 
											const real32 fy, 
											const real32 fz )
{
	real32* ptr = &fGradientTab[Index(ix,iy,iz)*3];
	return ptr[0]*fx + ptr[1]*fy + ptr[2]*fz; 
}

// Permutation table
inline int32 TNoiseBase::Index(	const int32	ix,	
								const int32 iy, 
								const int32 iz)
{
	return PermutedIndex(ix+PermutedIndex(iy+PermutedIndex(iz)));
}
inline int32 TNoiseBase::PermutedIndex( const int32 index )
{
	MCAssert(fTabIsValid);
	return fPermutationTab[(index)&kTabMask];
}

// 2D implementation
inline real32 TNoiseBase::GradientLattice2D(const int32	ix,	
											const int32 iy, 
											const real32 fx, 
											const real32 fy )
{
	real32* ptr = &fGradientTab[Index2D(ix,iy)*3];
	return ptr[0]*fx*fy + ptr[1]*fx + ptr[2]*fy; 
}
inline int32 TNoiseBase::Index2D(	const int32	ix,	
									const int32 iy )
{
	return PermutedIndex(ix+PermutedIndex(iy));
}


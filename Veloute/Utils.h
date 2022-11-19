/****************************************************************************************************

		Utils.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

#ifndef __Utils__
#define __Utils__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "MCBasicTypes.h"
#include "MCBasicDefines.h"
#include "MCColorRGBA.h"
#include "MCAssert.h"
#include "Real.h"
#include "Vector2.h"

// Fractal defines
static const int32 kMinFrequency = 1; // Use 1 to base the noise on its original shape
static const real32 kFractalIncrement = 2.13f; // !=2 to destroy the regularities

static const real32 kOneSixth = (real32)(1.0/6.0);
static const real32 kOneThird = (real32)(1.0/3.0);
static const real32 kTwoThird = (real32)(2.0/3.0);

#define SQRT3 			1.7320508075688772935274463415059		/* sqrt(3.0)	*/
#define HALF_SQRT3		0.86602540378443864676372317075294		/* sqrt(3.0)/2	*/
#define	PI3				1.0471975511965977461542144610932		/* PI/3			*/
#define	TWO_PI3			2.0943951023931954923084289221863		/* 2*PI/3		*/
#define HALF_SQRT2		0.70710678118654752440084436210485		/* sqrt(2.0)/2	*/

// Basic functions

// used to create a sharp transition between 2 textures
inline real32 Step(const real32 a, const real32 x){return (real32)(x>=a);}

inline real32 Pulse(const real32 a, const real32 b, const real32 x){return (Step(a,x)-Step(b,x));}

inline real32 Clamp(const real32 a, const real32 b, const real32 x){return (x<a?a:(x>b?b:x));}


// Antialiased functions

inline real32 BoxStep(const real32 a, const real32 b, const real32 x)
{
	return Clamp(0,1,((x-a)/(b-a)));
}
inline real32 SmoothStep(const real32 x)
{
	return (x*x*(3-2*x));
}
inline real32 SmoothStepDerived(const real32 x)
{
	return (x*6*(1-x));
}
inline real32 SmootherStep(const real32 x)
{	// Used for the bump ( avoid discontinuity in the tangent )
	return (x*x*x*(10-15*x+6*x*x));
}
inline real32 CosStep(const real32 x)
{
	return .5*(1-RealCos(PI*x));
}
inline real32 SmoothStep(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 0;
	if(x>=b) return 1;
	return SmoothStep((x-a)/(b-a));
}
inline real32 SmootherStep(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 0;
	if(x>=b) return 1;
	return SmootherStep((x-a)/(b-a));
}
inline real32 CosStep(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 0;
	if(x>=b) return 1;
	return CosStep((x-a)/(b-a));
}
inline real32 SmoothStepDerived(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 0;
	if(x>=b) return 0;
	const real32 y=(x-a)/(b-a);
	return (y*6*(1-y));
}
// A smoothing where the slope at the bottom and top can be set
inline real32 ControledSmoothStepUp(	const real32 a, const real32 b, const real32 x,
										const real32 Sb, const real32 St )
{
	if(x<a) return 0;
	if(x>=b) return 1;
	const real32 y=(x-a)/(b-a);
	return (1+y*(Sb+y*(y*(2+Sb+St)-(3+2*Sb+St))));
}
inline real32 ControledSmoothStepDown(	const real32 a, const real32 b, const real32 x,
										const real32 Sb, const real32 St )
{
	if(x<a) return 0;
	if(x>=b) return 1;
	const real32 y=(x-a)/(b-a);
	return (y*(St+y*((3-Sb-2*St)+y*(Sb+St-2))));
}
inline real32 SmoothStepWithTan(	const real32 a, const real32 b,
									const real32 tan1, const real32 tan2, const real32 x )
{
	if(x<a) return 0;
	if(x>=b) return 1;
	const real32 y=(x-a)/(b-a);
	return (y*(tan2+y*((3-tan1-2*tan2)+y*(tan1+tan2-2))));
}

// Pulse from 0 to max
inline real32 BoxPulse(const real32 a, const real32 aDelta, 
					   const real32 b, const real32 bDelta, 
					   const real32 max, const real32 x)
{return (max*(BoxStep(a,aDelta,x)-BoxStep(b,bDelta,x)));}
inline real32 SmoothPulse(const real32 a, const real32 aDelta, const real32 b, const real32 bDelta, const real32 x)
{return (SmoothStep(a,aDelta,x)-SmoothStep(b,bDelta,x));}
inline real32 CosPulse(const real32 a, const real32 aDelta, const real32 b, const real32 bDelta, const real32 x)
{return (CosStep(a,aDelta,x)-CosStep(b,bDelta,x));}
// Pulse from 0 to max
inline real32 SmoothPulseWithTan(const real32 a, const real32 aDelta, 
								 const real32 b, const real32 bDelta, 
								 const real32 tan1, const real32 tan2, 
								 const real32 max, const real32 x)
{return (max*(SmoothStepWithTan(a,aDelta,tan1,tan2,x)-SmoothStepWithTan(b,bDelta,tan2,tan1,x)));}
// Periodic functions

// tooth shape function when call as Mod(x,a)/a
// can be used to build a periodic function such as Pulse(a,b,Mod(x,a)/a)
inline real32 Mod(real32 a, const real32 b)
{
	const int32 n=(int32)(a/b);
	a-=n*b;
	if(a<0) a+=b;
	return a;
}

// Mapping from [0,1] to itself

// identity when gamma=1
inline real32 GammaCorrect( const real32 gamma, const real32 x)
{
	MCAssert(x>=0 && x<=1);
	return RealPow(x,1/gamma);
}
const real32 log05 = log(0.5);
inline real32 Bias( const real32 a, const real32 x)
{
	return pow(x, RealLog(a)/log05);
}
// identity when a=.5
inline real32 Gain( const real32 g, const real32 x)
{
	if(x<.5)
		return .5 * Bias(1-g,2*x);
	else
		return 1 - .5 * Bias(1-g,2-2*x);
}

// fractional part
inline real32 Lerp( const real32 t, const real32 x0, const real32 x1 )
{
	return (x0 + t*(x1-x0));
}

inline void BlendValue(real& mixedValue,real& value1, real a)
{
	mixedValue=(kRealOne-a)*mixedValue+a*value1;
}

inline void	BlendColor(TMCColorRGBA &publicMixedColor, TMCColorRGBA &publicColor1, real a)
{
	if (a == kRealOne)
	{
		publicMixedColor = publicColor1;
	}
	else if (a == kRealZero)
	{
	}
	else
	{		
		publicMixedColor.R += a * (publicColor1.R - publicMixedColor.R );
		publicMixedColor.G += a * (publicColor1.G - publicMixedColor.G );
		publicMixedColor.B += a * (publicColor1.B - publicMixedColor.B );
		publicMixedColor.A += a * (publicColor1.A - publicMixedColor.A );
	}
}

inline void	BlendColor(TMCColorRGB &publicMixedColor, TMCColorRGB &publicColor1, real a)
{
	if (a == kRealOne)
	{
		publicMixedColor = publicColor1;
	}
	else if (a == kRealZero)
	{
	}
	else
	{		
		publicMixedColor.R += a * (publicColor1.R - publicMixedColor.R );
		publicMixedColor.G += a * (publicColor1.G - publicMixedColor.G );
		publicMixedColor.B += a * (publicColor1.B - publicMixedColor.B );
	}
}

inline void SwitchValues(real32& a, real32& b)
{
	const real32 tmp = a;
	a=b;
	b=tmp;
}

inline void SwitchValues(int32& a, int32& b)
{
	const int32 tmp = a;
	a=b;
	b=tmp;
}

inline void SwitchVectors(TVector2& a, TVector2& b)
{
	const TVector2 tmp = a;
	a=b;
	b=tmp;
}

inline void SwitchVectors(TVector3& a, TVector3& b)
{
	const TVector3 tmp = a;
	a=b;
	b=tmp;
}

inline real32 GetPositiveAngle(const TVector2& vec1, const TVector2& vec2)
{
	const real32 cos = vec1 * vec2;
	const real32 sin = vec1 ^ vec2;
	real32 angle = 0;
	RealArcSinCos(sin,cos,angle);
	if(angle<0) angle+=360;

	return angle;
}



#endif

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

#include "MCBasicTypes.h"
#include "MCBasicDefines.h"
#include "MCColorRGB.h"
#include "MCAssert.h"
#include "Real.h"

// Fractal defines
static const int32 kMinFrequency = 1; // Use 1 to base the noise on its original shape
static const real32 kFractalIncrement = 2.13f; // !=2 to destroy the regularities


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
inline real32 BoxStep(const real32 x)
{
	return Clamp(0,1,x);
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

// r is the distance from something
// return 1 when r=0 and 0 when r=R
inline real32 DensityBlender(const real32 r2, const real32 R2)
{
	return (1 + ( (( -4*r2/(R2*R2) + 17/(R2))*r2 - 22)*r2 )/(9*R2));
}
// Warning: this step is from 1 to 0 (!= smoothstep)
inline real32 BlenderInvertStep(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 1;
	if(x>=b) return 0;

	const real32 r = (x-a);
	const real32 R = (b-a);
	return DensityBlender(r,R);
}
// Warning: this step is from 1 to 0 (!= smoothstep)
inline real32 LinearInvertStep(const real32 a, const real32 b, const real32 x)
{
	if(x<a) return 1;
	if(x>=b) return 0;
	return ((x-b)/(a-b));
}


#include "ShaderTypes.h"
inline void InitShadingIn(ShadingIn& shadingIn)
{
	shadingIn.fPoint = TVector3::kZero;
	shadingIn.fGNormal = TVector3::kUnitZ;
	shadingIn.fPointLoc = TVector3::kZero;
	shadingIn.fNormalLoc = TVector3::kUnitZ;
	shadingIn.fUV = TVector2::kZero;
	shadingIn.fPointx = TVector3::kUnitX;
	shadingIn.fPointy = TVector3::kUnitY;
	shadingIn.fNormalx = TVector3::kUnitX;
	shadingIn.fNormaly = TVector3::kUnitY;
	shadingIn.fUVx = TVector2::kUnitX;
	shadingIn.fUVy = TVector2::kUnitY;
	shadingIn.fPointLocx = TVector3::kUnitX;
	shadingIn.fPointLocy = TVector3::kUnitY;
	shadingIn.fNormalLocx = TVector3::kUnitX;
	shadingIn.fNormalLocy = TVector3::kUnitY;
	shadingIn.fIsoU = TVector3::kUnitX;
	shadingIn.fIsoV = TVector3::kUnitY;
	shadingIn.fUVSpaceID=0;
	shadingIn.fBumpOn=false;
	shadingIn.fCurrentCompletionMask=0;
	shadingIn.fInstance=NULL;
}

#include "I3DShScene.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
inline void GetScene(I3DShScene** scene)
{
	// Compare the actual lights with the stored infos
	// Get the scene
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(scene);
}



#endif

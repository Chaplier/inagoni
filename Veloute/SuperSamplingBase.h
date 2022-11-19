/****************************************************************************************************

		SuperSamplingBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	2/12/2004

****************************************************************************************************/

#ifndef __SuperSamplingBase__
#define __SuperSamplingBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Vector3.h"
#include "ShaderTypes.h"
#include "Transform2D.h"
#include "Basic3DCOMImplementations.h" // Transform3D

//////////////////////////////////////
//
// This is a hack to allow me to communicate between my shaders
// this flag is passed in fCurrentCompletionMask (there's still
// room for other flags in it)
static const uint32 gCannotSuperSample = 256; 


const real64 kRootDerivativeLimit = 10*kRealEpsilon;
const real64 kSquareDerivativeLimit = kRootDerivativeLimit*kRootDerivativeLimit;
const real32 kMaxLimit = .05f;// 12jan05: try something bigger.01f; // the use of this one need to be check again

//////////////////////////////////////////////////////////////////////////
//
class TSuperSamplingBase
{
public:
	TSuperSamplingBase();

	void SetSuperSamplingQuality(const TTransform2D& transform, const real32 sU, const real32 sV);
	void SetSuperSamplingQuality(const TTransform3D& transform, const real32 sU, const real32 sV, const real32 sW);

	// in UV
	inline real32 GetSample( const real32 uu, const real32 vv, ShadingIn& shadingIn );
	void GetPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4);
	void SuperSampling(const real32 uu, const real32 vv, ShadingIn& shadingIn,
						real32& value1, real32& value2, real32& value3, real32& value4 );

	// in global coordinates
	inline real32 GetGlobalSample( const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn );
	void GetGlobalPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4);
	void GlobalSuperSampling(const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn,
						real32& value1, real32& value2, real32& value3, real32& value4 );
	// in local coordinates
	inline real32 GetLocalSample( const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn );
	void GetLocalPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4);
	void LocalSuperSampling(const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn,
						real32& value1, real32& value2, real32& value3, real32& value4 );

	inline real32 GetDhDrInUV(const real32 value1, const real32 value2, const real32 value3, const real32 value4) const 
		{return (0.25*(value1+value2-value3-value4)/(fRescaledUVx.GetMagnitude())); }
	inline real32 GetDhDsInUV(const real32 value1, const real32 value2, const real32 value3, const real32 value4) const 
		{return (0.25*(value1+value3-value2-value4)/(fRescaledUVy.GetMagnitude())); }

protected:
	virtual real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate){ return 0;}
	virtual real32 ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate){ return 0;}
	virtual real32 ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate){ return 0;}

	// Derived class should say if they want the sampling rate to be computed
	boolean fNeedSamplingRate;

private:
	real32	fSuperSamplingU;
	real32	fSuperSamplingV;
	real32	fSuperSamplingW;

	// UV based textures used fSuperSamplingU, fSuperSamplingV and the following values:
	TTransform2D	fPrivate2DTransform;
	TVector2		fRescaledUVx;
	TVector2		fRescaledUVy;

	// XYZ based textures used fSuperSamplingU, fSuperSamplingV, fSuperSamplingW and the following values:
	TTransform3D	fPrivate3DTransform;
	TVector3		fRescaledDx;
	TVector3		fRescaledDy;
};
//
//////////////////////////////////////////////////////////////////////////

inline real32 TSuperSamplingBase::GetSample( const real32 uu, const real32 vv, ShadingIn& shadingIn )
{
	if(shadingIn.fCurrentCompletionMask & gCannotSuperSample)
	{
		real32 sRate = 0;
		const real32 normX = shadingIn.fUVx.GetNorm();
		if( normX>kRootDerivativeLimit )
		{
			sRate = RealAbs(normX + shadingIn.fUVy.GetNorm());
		}
		else
		{
			if(shadingIn.fUVx.x==0 && shadingIn.fUVx.y==0)
			{
				// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
				sRate = .002f; 
			}
			else
			{	// the derivatives are too small to be significant: use a small value
				sRate = 2*kRootDerivativeLimit;
			}
		}
		return ComputeOneSample(uu, vv,  sRate);
	}
	else
	{
		// Antialiasing
		real32 value1=0,value2=0,value3=0,value4=0;
		SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);

		return (value1+value2+value3+value4)*.25;
	}
}

inline real32 TSuperSamplingBase::GetLocalSample( const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn )
{
	if(shadingIn.fCurrentCompletionMask & gCannotSuperSample)
	{
		real32 sRate = 0;
		const real32 normX = shadingIn.fPointLocx.GetNorm();
		if( normX > kSquareDerivativeLimit )
		{
			sRate = RealAbs(normX + shadingIn.fPointLocy.GetNorm());
		}
		else
		{
			if(shadingIn.fPointLocx.x==0 && shadingIn.fPointLocx.y==0 && shadingIn.fPointLocx.z==0)
			{
				// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
				sRate = .002f; 
			}
			else
			{	// the derivatives are too small to be significant: use a small value
				sRate = 2*kRootDerivativeLimit;
			}
		}
		return ComputeOneLocalSample(x, y, z, sRate);
	}
	else
	{
		// Antialiasing
		real32 value1=0,value2=0,value3=0,value4=0;
		LocalSuperSampling(x, y, z, shadingIn, value1, value2, value3, value4);

		return (value1+value2+value3+value4)*.25;
	}
}

inline real32 TSuperSamplingBase::GetGlobalSample( const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn )
{
	if(shadingIn.fCurrentCompletionMask & gCannotSuperSample)
	{
		real32 sRate = 0;
		const real32 normX = shadingIn.fPointx.GetNorm();
		if( normX > kSquareDerivativeLimit )
		{
			sRate = RealAbs(normX + shadingIn.fPointy.GetNorm());
		}
		else
		{
			if(shadingIn.fPointx.x==0 && shadingIn.fPointx.y==0 && shadingIn.fPointx.z==0)
			{
				// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
				sRate = .002f; 
			}
			else
			{	// the derivatives are too small to be significant: use a small value
				sRate = 2*kRootDerivativeLimit;
			}
		}
		return ComputeOneGlobalSample(x, y, z, sRate);
	}
	else
	{
		// Antialiasing
		real32 value1=0,value2=0,value3=0,value4=0;
		GlobalSuperSampling(x, y, z, shadingIn, value1, value2, value3, value4);

		return (value1+value2+value3+value4)*.25;
	}
}







#endif

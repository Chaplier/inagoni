/****************************************************************************************************

		RandomLines3DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#include "RandomLines3DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_RandomLines3DShader(R_CLSID_RandomLines3DShader);

PMapRandomLines3DShader::PMapRandomLines3DShader()
{
	Init();
}

void PMapRandomLines3DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();

	fType = 'Opt1';
	fSeed = 3462234;
	fDisturbance = .5;
	fCount = 10;
	fAA = false;
}

RandomLines3DShader::RandomLines3DShader()
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}

boolean RandomLines3DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return (
		(fPMap.fType == ((RandomLines3DShader*)aShader)->fPMap.fType) &&
		(fPMap.fAA == ((RandomLines3DShader*)aShader)->fPMap.fAA) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((RandomLines3DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr RandomLines3DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsPointLoc = true;
	theFlags.fNeedsPoint = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput RandomLines3DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr RandomLines3DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr RandomLines3DShader::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (source->GetInstanceID() == 'SEED')
	{
		fPMap.fSeed = MCRandom();
		fNoise.SetSeed(fPMap.fSeed);
		fNoise.InitGradientTab();
		return MC_S_OK;
	}
	else
		return GrayScaleBase::HandleEvent( message, source, data);
}

real RandomLines3DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	if(!fPMap.fAA)
	{
		// Disable anti aliasing
		shadingIn.fCurrentCompletionMask |= gCannotSuperSample;
	}

	if(fPMap.mTransform.IsGlobalSpace())
	{
		// Transform the point
		TVector3 point = f3DTransform.TransformPoint(shadingIn.fPoint);


		point.x = point.x - floor(point.x);
		point.y = point.y - floor(point.y);
		point.z = point.z - floor(point.z);

		result = GetGlobalSample(point.x,point.y,point.z,shadingIn);
	}
	else
	{
		// Transform the point
		TVector3 point = f3DTransform.TransformPoint(shadingIn.fPointLoc);

/*
		point.x = point.x - floor(point.x);
		point.y = point.y - floor(point.y);
		point.z = point.z - floor(point.z);
*/
		result = GetLocalSample(point.x,point.y,point.z,shadingIn);
	}

	fullArea = true;

	PostProcess(result, fPMap.mGrayScale);

	return 1.0f;
}

real RandomLines3DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 value1=0,value2=0,value3=0,value4=0;
	if(fPMap.mTransform.IsGlobalSpace())
	{
		// Transform the point
		TVector3 point = f3DTransform.TransformPoint(shadingIn.fPoint);


		point.x = point.x - floor(point.x);
		point.y = point.y - floor(point.y);
		point.z = point.z - floor(point.z);

		GlobalSuperSampling(point.x, point.y, point.z, shadingIn, value1, value2, value3, value4);

		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetGlobalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}
	else
	{
		// Transform the point
		TVector3 point = f3DTransform.TransformPoint(shadingIn.fPointLoc);


	/*	point.x = point.x - floor(point.x);
		point.y = point.y - floor(point.y);
		point.z = point.z - floor(point.z);
*/
		LocalSuperSampling(point.x, point.y, point.z, shadingIn, value1, value2, value3, value4);

		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetLocalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}

	return 1.0f;
}

real32 RandomLines3DShader::ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	return ComputeOneLocalSample(x,y,z,samplingRate);
}

real32 RandomLines3DShader::GetIrisValue(const real32 u, const real32 v)
{
	TVector2 dir(u,v);
	dir.Normalize();

	// get the positive angle in deg
	real32 angle = GetPositiveAngle(TVector2::kUnitX, dir)/3.6;

	const real32 radScaled = angle*fPeriod;
	TVector2 point(.5, fDisturb*radScaled);
	return .5+.5*sin((radScaled + fNoise.GetValueLinear2D(point))*TWO_PI);
}

real32 RandomLines3DShader::ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 value=0;
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			const real32 xScaled = x*fPeriod;
			TVector3 point(fDisturb*xScaled, .5, .5);
			value = .5+.5*sin((xScaled + fNoise.GetValueLinear(point))*TWO_PI);
		} break;
	case 'Opt2':
		{
			const real32 yScaled = y*fPeriod;
			TVector3 point(.5, fDisturb*yScaled, .5);
			value = .5+.5*sin((yScaled + fNoise.GetValueLinear(point))*TWO_PI);
		} break;
	case 'Opt3':
		{
			const real32 zScaled = z*fPeriod;
			TVector3 point(.5, .5, fDisturb*zScaled);
			value = .5+.5*sin((zScaled + fNoise.GetValueLinear(point))*TWO_PI);
		} break;
	case 'Opt4':
		{	// iris
			value = GetIrisValue(x-.5, y-.5);
		} break;
	case 'Opt5':
		{	// iris
			value = GetIrisValue(y-.5, z-.5);
		} break;
	case 'Opt6':
		{	// iris
			value = GetIrisValue(z-.5, x-.5);
		} break;
	}

	return value;
}

void RandomLines3DShader::PreProcess()
{
	// Perlin noise
	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();

	switch(fPMap.fType)
	{
	case 'Opt1':
	case 'Opt2':
	case 'Opt3':
		{
			fDisturb = 100*fPMap.fDisturbance;
			fPeriod = fPMap.fCount*.1;
		} break;
	case 'Opt4':
	case 'Opt5':
	case 'Opt6':
		{
			fDisturb = 10*fPMap.fDisturbance;
			fPeriod = fPMap.fCount*.01;
		} break;
	}

	fPMap.mTransform.Get3DTransform( f3DTransform );

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f3DTransform, 1, 1, 1 ); // works when lowered to 1. Need to understand how it works!, 100, 100, 100 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

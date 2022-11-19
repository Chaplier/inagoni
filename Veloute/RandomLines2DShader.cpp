/****************************************************************************************************

		RandomLines2DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "RandomLines2DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_RandomLines2DShader(R_CLSID_RandomLines2DShader);

PMapRandomLines2DShader::PMapRandomLines2DShader()
{
	Init();
}

void PMapRandomLines2DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();

	fType = 'Opt1';
	fSeed = 3462234;
	fDisturbance = .5;
	fCount = 10;
	fAA = false;
}

RandomLines2DShader::RandomLines2DShader()
{
	fNeedSamplingRate = false;
	f2DTransform = TMatrix33::kIdentity;
	fPreprocessed = false;
}

boolean RandomLines2DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return (
		(fPMap.fType == ((RandomLines2DShader*)aShader)->fPMap.fType) &&
		(fPMap.fAA == ((RandomLines2DShader*)aShader)->fPMap.fAA) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((RandomLines2DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr RandomLines2DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput RandomLines2DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr RandomLines2DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr RandomLines2DShader::HandleEvent(MessageID message, IMFResponder* source, void* data)
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

real RandomLines2DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	TVector2 uv = shadingIn.fUV;
	// translate and rotate
	uv = f2DTransform*uv;
	// Scale the point
	uv.x /= f2DTransform[2][0];
	uv.y /= f2DTransform[2][1];


	uv/=100;

//	uv.x = uv.x - floor(uv.x);
//	uv.y = uv.y - floor(uv.y);

	uv*=100;

	if(!fPMap.fAA)
	{
		// Disable anti aliasing
		shadingIn.fCurrentCompletionMask |= gCannotSuperSample;
	}

	result = GetSample(uv.x,uv.y,shadingIn);

	fullArea = true;

	PostProcess(result, fPMap.mGrayScale);

	return 1.0f;
}

real RandomLines2DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	TVector2 uv = shadingIn.fUV;
	// translate and rotate
	uv = f2DTransform*uv;
	// Scale the point
	uv.x /= f2DTransform[2][0];
	uv.y /= f2DTransform[2][1];


	uv/=100;

//	uv.x = uv.x - floor(uv.x);
//	uv.y = uv.y - floor(uv.y);

	uv*=100;

	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uv.x, uv.y, shadingIn, value1, value2, value3, value4);

	PostProcess(value1, fPMap.mGrayScale);
	PostProcess(value2, fPMap.mGrayScale);
	PostProcess(value3, fPMap.mGrayScale);
	PostProcess(value4, fPMap.mGrayScale);

	GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);

	return 1.0f;
}

real32 RandomLines2DShader::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	real32 value=0;
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			const real32 uuScaled = uu*fPeriod;
			TVector2 point(fDisturb*uuScaled, .5);
			value = .5+.5*sin((uuScaled + fNoise.GetValueLinear2D(point))*TWO_PI);
		} break;
	case 'Opt2':
		{
			const real32 vvScaled = vv*fPeriod;
			TVector2 point(.5, fDisturb*vvScaled);
			value = .5+.5*sin((vvScaled + fNoise.GetValueLinear2D(point))*TWO_PI);
		} break;
	case 'Opt3':
		{	// iris
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);

			TVector2 dir(newU,newV);
			dir.Normalize();

			// get the positive angle in deg
			real32 angle = GetPositiveAngle(TVector2::kUnitX, dir)/3.6;

			const real32 radScaled = angle*fPeriod;
			TVector2 point(.5, fDisturb*radScaled);
			value = .5+.5*sin((radScaled + fNoise.GetValueLinear2D(point))*TWO_PI);
		} break;
	}
	
	return value;
}

void RandomLines2DShader::PreProcess()
{
	// Perlin noise
	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();

	fDisturb = 10*fPMap.fDisturbance;
	fPeriod = fPMap.fCount*.01;

	fPMap.mTransform.Get2DTransform( f2DTransform );

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f2DTransform, 1, 1 ); // works when lowered to 1. Need to understand how it works! 100, 100 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

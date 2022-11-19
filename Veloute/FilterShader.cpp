/****************************************************************************************************

		FilterShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "FilterShader.h"

#include "MCRandom.h"
#include "Veloute.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_FilterShader(R_CLSID_FilterShader);
#else
const MCGUID CLSID_FilterShader={R_CLSID_FilterShader};
#endif


PMapFilterShader::PMapFilterShader()
{
	Init();
}

void PMapFilterShader::Init()
{
	mGrayScale.Init();
}

FilterShader::FilterShader()
{
	fNeedSamplingRate = false;

	fPreprocessed = false;
}

boolean FilterShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return ( 
		fPMap.fSubShaderComponent == ((FilterShader*)aShader)->fPMap.fSubShaderComponent  &&
		GrayScaleBase::IsEqualTo(aShader) );
}

MCCOMErr FilterShader::GetShadingFlags(ShadingFlags& theFlags)
{
	if(fSubShader)
	{
		fSubShader->GetShadingFlags(theFlags);
		theFlags.fConstantChannelsMask = kNoChannel;
	}
	else
	{
		theFlags.fNeedsUV = true;		// We need UV coordinates
		theFlags.fConstantChannelsMask = kNoChannel;
	}
	return MC_S_OK;
}

EShaderOutput FilterShader::GetImplementedOutput()
{
	if(fSubShader)
		return (EShaderOutput)(fSubShader->GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
	else
		return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetColor
						|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr FilterShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

real FilterShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{
		fSubShader->GetValue(result, fullArea, shadingIn);

		fullArea = true;

		PostProcess(result, fPMap.mGrayScale);
	}

	return 1.0f;
}

#if (VERSIONNUMBER >= 0x040000)
real FilterShader::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#else
real FilterShader::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if (IsLocked())
	{
#if (VERSIONNUMBER >= 0x040000)
		result.Set(0.0f, 0.0f, 0.0f, 0.0f);
#else
		result.Set(0.0f, 0.0f, 0.0f);
#endif
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{
		TMCColorRGBA innerResult;
		fSubShader->GetColor(innerResult, fullArea, shadingIn);
		result = innerResult;
		fSubShader->GetColor(result, fullArea, shadingIn);

		real32 h=0,l=0,s=0;
		result.GetHLS(h,l,s);
		PostProcess(l, fPMap.mGrayScale);
		result.SetHLS(h,l,s);
	}

	return 1.0f;
}

real FilterShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	fShadingIn = shadingIn;
	
	real32 value1=0,value2=0,value3=0,value4=0;

	if(fUsesUVs)
	{
		SuperSampling(shadingIn.fUV.x*100, shadingIn.fUV.y*100, shadingIn, value1, value2, value3, value4);
	
		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}
	else if(fUsesLoc)
	{
		LocalSuperSampling(shadingIn.fPointLoc.x, shadingIn.fPointLoc.y, shadingIn.fPointLoc.z, shadingIn, value1, value2, value3, value4);

		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetLocalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}
	else if(fUsesGlo)
	{
		GlobalSuperSampling(shadingIn.fPoint.x, shadingIn.fPoint.y, shadingIn.fPoint.z, shadingIn, value1, value2, value3, value4);

		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetGlobalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}

	return 1.0f;
}

real32 FilterShader::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	real32 result = 0;
	if(fSubShader)
	{
		fShadingIn.fUV.x = uu/100;
		fShadingIn.fUV.y = vv/100;
		boolean fullAreaDone;
		fSubShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

real32 FilterShader::ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 result = 0;
	if(fSubShader)
	{
		fShadingIn.fPoint.x = x;
		fShadingIn.fPoint.y = y;
		fShadingIn.fPoint.z = z;
		boolean fullAreaDone;
		fSubShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

real32 FilterShader::ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 result = 0;
	if(fSubShader)
	{
		fShadingIn.fPointLoc.x = x;
		fShadingIn.fPointLoc.y = y;
		fShadingIn.fPointLoc.z = z;
		boolean fullAreaDone;
		fSubShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

void FilterShader::PreProcess()
{
	if(fPMap.fSubShaderComponent )
	{
		fPMap.fSubShaderComponent->QueryInterface(IID_I3DShShader, (void**)&(fSubShader));
	}
	else
		fSubShader=NULL;

	if(fSubShader)
	{
		fSubShader->GetShadingFlags(fShadingFlags);
	}

	// Many subshaders say they need all the values, just to use one. If so, determine which one
	fUsesUVs = fShadingFlags.fNeedsUV;
	fUsesLoc = fShadingFlags.fNeedsPointLoc;
	fUsesGlo = fShadingFlags.fNeedsPoint;

	if( (fUsesUVs&&fUsesLoc) ||
		(fUsesLoc&&fUsesGlo) ||
		(fUsesGlo&&fUsesUVs) )
	{
		const int32 precision = 16000;
		const int32 security = 10;
		int32 UVsHint = 0;
		int32 LocHint = 0;
		int32 GloHint = 0;
		for(int32 i=0 ; i<precision ; i++)
		{
			const real32 randomX1 = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomY1 = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomZ1 = FixedToReal(MCRandom()&0xFFFF);
		
			const real32 randomX2 = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomY2 = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomZ2 = FixedToReal(MCRandom()&0xFFFF);
			
			ShadingIn fakeShading;
			// Disable anti aliasing
			fakeShading.fCurrentCompletionMask |= gCannotSuperSample;
			boolean fullArea = true;

			if(fUsesUVs)
			{
				fakeShading.fUV.x = randomX1;
				fakeShading.fUV.y = randomY1;
				real32 result1=0;
				fSubShader->GetValue(result1, fullArea, fakeShading);

				fakeShading.fUV.x = randomX2;
				fakeShading.fUV.y = randomY2;
				real32 result2=0;
				fSubShader->GetValue(result2, fullArea, fakeShading);

				fakeShading.fUV.x = 0;
				fakeShading.fUV.y = 0;

				if(result1!=result2)
					UVsHint++;
			}
			
			if(fUsesLoc)
			{
				fakeShading.fPointLoc.x = randomX1*1234;
				fakeShading.fPointLoc.y = randomY1*1234;
				fakeShading.fPointLoc.z = randomZ1*1234;
				real32 result1=0;
				fSubShader->GetValue(result1, fullArea, fakeShading);

				fakeShading.fPointLoc.x = randomX2*1234;
				fakeShading.fPointLoc.y = randomY2*1234;
				fakeShading.fPointLoc.z = randomZ2*1234;
				real32 result2=0;
				fSubShader->GetValue(result2, fullArea, fakeShading);

				fakeShading.fPointLoc.x = 0;
				fakeShading.fPointLoc.y = 0;
				fakeShading.fPointLoc.z = 0;

				if(result1!=result2)
					LocHint++;
			}
			
			if(fUsesGlo)
			{
				fakeShading.fPoint.x = randomX1*1234;
				fakeShading.fPoint.y = randomY1*1234;
				fakeShading.fPoint.z = randomZ1*1234;
				real32 result1=0;
				fSubShader->GetValue(result1, fullArea, fakeShading);

				fakeShading.fPoint.x = randomX2*1234;
				fakeShading.fPoint.y = randomY2*1234;
				fakeShading.fPoint.z = randomZ2*1234;
				real32 result2=0;
				fSubShader->GetValue(result2, fullArea, fakeShading);

				fakeShading.fPoint.x = 0;
				fakeShading.fPoint.y = 0;
				fakeShading.fPoint.z = 0;

				if(result1!=result2)
					GloHint++;
			}

			if( (GloHint-LocHint)>security &&  (GloHint-UVsHint)>security )
			{
				fUsesUVs=false;
				fUsesLoc=false;
				fUsesGlo=true;

				break;
			}
			else if( (LocHint-GloHint)>security &&  (LocHint-UVsHint)>security )
			{
				fUsesUVs=false;
				fUsesLoc=true;
				fUsesGlo=false;

				break;
			}
			else if( (UVsHint-GloHint)>security &&  (UVsHint-LocHint)>security )
			{
				fUsesUVs=true;
				fUsesLoc=false;
				fUsesGlo=false;

				break;
			}
		}
	}

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

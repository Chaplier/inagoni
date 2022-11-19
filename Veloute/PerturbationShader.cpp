/****************************************************************************************************

		PerturbationShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "PerturbationShader.h"

#include "MCRandom.h"
#include "Veloute.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_PerturbationShader(R_CLSID_PerturbationShader);
#else
const MCGUID CLSID_PerturbationShader={R_CLSID_PerturbationShader};
#endif

PMapPerturbationShader::PMapPerturbationShader()
{
	// SHADERS_PLUS params: choose default value so the basic version looks similar
	fAmplitudeX = 1;
	fAmplitudeY = 1;
	fAmplitudeZ = 1;
	fMethod = kScalarMethod;
	// Basic version param
	fAmplitude = .01f;
}

PerturbationShader::PerturbationShader()
{
	fPreprocessed = false;

	fXCoeff = 1;
	fYCoeff = 1;
	fZCoeff = 1;
}

boolean PerturbationShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return ( 
		fPMap.fAmplitude == ((PerturbationShader*)aShader)->fPMap.fAmplitude  &&
		fPMap.fTextureShaderComponent == ((PerturbationShader*)aShader)->fPMap.fTextureShaderComponent  &&
		fPMap.fPerturbShaderComponent == ((PerturbationShader*)aShader)->fPMap.fPerturbShaderComponent  );
}

MCCOMErr PerturbationShader::GetShadingFlags(ShadingFlags& theFlags)
{
	if(!fPreprocessed)
		PreProcess();

	if(fTextureShader)
	{
		fTextureShader->GetShadingFlags(theFlags);
		theFlags.fConstantChannelsMask = kNoChannel;
	}
	else
	{
		theFlags.fNeedsUV = true;		// We need UV coordinates
		theFlags.fConstantChannelsMask = kNoChannel;
	}
	return MC_S_OK;
}

EShaderOutput PerturbationShader::GetImplementedOutput()
{
// Bug: at this stage, fPMap is always empty (=> fTextureShader is always NULL)
//	if(!fPreprocessed)
//		PreProcess();
//
//	if(fTextureShader)
//		return (EShaderOutput)(fTextureShader->GetImplementedOutput()|kUsesGetVector|kUsesDoShade);	// We use GetValue with shading Area
//	else
		return (EShaderOutput)(kUsesGetValue|kUsesGetColor|kUsesGetVector|kUsesDoShade);
}

MCCOMErr PerturbationShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

void PerturbationShader::AddPerturbation(ShadingIn& newShading, ShadingIn& oldShading)
{
	if( fPerturbShader )
	{
		if(fPMap.fMethod == kVectorielMethod)
		{
			// Method similar to the bump mapping: get 4 values arround
			real32 value1=0,value2=0,value3=0,value4=0;

			fShadingIn = oldShading;
	
			if(fShadingFlags.fNeedsUV)
			{
				SuperSampling(oldShading.fUV.x*100, oldShading.fUV.y*100, oldShading, value1, value2, value3, value4);
			
				// dH/dr when sample at the 4 corners (r==x)
				const real32 dHdr = GetDhDrInUV(value1,value2,value3,value4);
				// dH/ds when sample at the 4 corners (s==y)
				const real32 dHds = GetDhDsInUV(value1,value2,value3,value4);
			
				newShading.fUV.x += dHdr*fXCoeff;
				newShading.fUV.y += dHds*fYCoeff;
			}

			if(fShadingFlags.fNeedsPointLoc)
			{
				LocalSuperSampling(oldShading.fPointLoc.x, oldShading.fPointLoc.y, oldShading.fPointLoc.z, oldShading, value1, value2, value3, value4);
				TVector3 vector;
				GetLocalPerturbationVector(vector, oldShading, value1, value2, value3, value4);
		
				newShading.fPointLoc.x += vector.x*fXCoeff;
				newShading.fPointLoc.y += vector.y*fYCoeff;
				newShading.fPointLoc.z += vector.z*fZCoeff;
			}

			if(fShadingFlags.fNeedsPoint)
			{
				GlobalSuperSampling(oldShading.fPoint.x, oldShading.fPoint.y, oldShading.fPoint.z, oldShading, value1, value2, value3, value4);
				TVector3 vector;
				GetGlobalPerturbationVector(vector, oldShading, value1, value2, value3, value4);
			
				newShading.fPoint.x += vector.x*fXCoeff;
				newShading.fPoint.y += vector.y*fYCoeff;
				newShading.fPoint.z += vector.y*fZCoeff;
			}
		}
		else
		{
			real32 value=0;
			boolean fullArea=true;
			fPerturbShader->GetValue(value, fullArea, oldShading);
			
			value = (value-.5);

			if(fShadingFlags.fNeedsUV)
			{
				newShading.fUV.x += value*fXCoeff;
				newShading.fUV.y += value*fYCoeff;
			}
			
			if(fShadingFlags.fNeedsPointLoc)
			{
				newShading.fPointLoc.x += value*fXCoeff;
				newShading.fPointLoc.y += value*fYCoeff;
				newShading.fPointLoc.z += value*fZCoeff;
			}
		
			if(fShadingFlags.fNeedsPoint)
			{
				newShading.fPoint.x += value*fXCoeff;
				newShading.fPoint.y += value*fYCoeff;
				newShading.fPoint.z += value*fZCoeff;
			}
		}
	}
}

real PerturbationShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	ShadingIn shading = shadingIn;
	AddPerturbation(shading, shadingIn);

	if(fTextureShader)
	{
		fTextureShader->GetValue(result, fullArea, shading);

		fullArea = true;
	}

	return 1.0f;
}

#if (VERSIONNUMBER >= 0x040000)
real PerturbationShader::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#else
real PerturbationShader::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
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

	ShadingIn shading = shadingIn;
	AddPerturbation(shading, shadingIn);

	if(fTextureShader)
	{
		fTextureShader->GetColor(result, fullArea, shading);
	}

	return 1.0f;
}

real PerturbationShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	ShadingIn shading = shadingIn;
	AddPerturbation(shading, shadingIn);

	if(fTextureShader)
	{
		fTextureShader->GetVector(result, shading);
	}

	return 1.0f;
}

MCCOMErr PerturbationShader::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.Clear();
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	ShadingIn shading = shadingIn;
	AddPerturbation(shading, shadingIn);

	if(fTextureShader)
	{
		fTextureShader->DoShade(result, shading);
		shadingIn.fCurrentCompletionMask = shading.fCurrentCompletionMask;
	}

	return MC_S_OK;
}

void PerturbationShader::PreProcess()
{
	if(fPMap.fTextureShaderComponent )
	{
		fPMap.fTextureShaderComponent->QueryInterface(IID_I3DShShader, (void**)&(fTextureShader));
	}
	else
		fTextureShader=NULL;

	if(fPMap.fPerturbShaderComponent )
	{
		fPMap.fPerturbShaderComponent->QueryInterface(IID_I3DShShader, (void**)&(fPerturbShader));
	}
	else
		fPerturbShader=NULL;

	if(fTextureShader)
	{
		fTextureShader->GetShadingFlags(fShadingFlags);

		// Init the supersampling if necessary
		if(fPMap.fMethod == kVectorielMethod)
		{
		//	if(fShadingFlags.fNeedsUV)
			{
				TTransform2D result = TMatrix33::kIdentity;
				result[2][0] = .01f;//*fPMap.fAmplitude*fPMap.fAmplitudeX; // Default value <=> 100% <=> u in [0,100]
				result[2][1] = .01f;//*fPMap.fAmplitude*fPMap.fAmplitudeY; // Default value <=> 100% <=> v in [0,100]
				SetSuperSamplingQuality( result, 100, 100 );
			}
		}
	}

	fXCoeff = fPMap.fAmplitude*fPMap.fAmplitudeX;
	fYCoeff = fPMap.fAmplitude*fPMap.fAmplitudeY;
	fZCoeff = fPMap.fAmplitude*fPMap.fAmplitudeZ;

	fPreprocessed=true;
}

real32 PerturbationShader::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	real32 result = 0;
	if(fPerturbShader)
	{
		fShadingIn.fUV.x = uu/100;
		fShadingIn.fUV.y = vv/100;
		boolean fullAreaDone;
		fPerturbShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

real32 PerturbationShader::ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 result = 0;
	if(fPerturbShader)
	{
		fShadingIn.fPoint.x = x;
		fShadingIn.fPoint.y = y;
		fShadingIn.fPoint.z = z;
		boolean fullAreaDone;
		fPerturbShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

real32 PerturbationShader::ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 result = 0;
	if(fPerturbShader)
	{
		fShadingIn.fPointLoc.x = x;
		fShadingIn.fPointLoc.y = y;
		fShadingIn.fPointLoc.z = z;
		boolean fullAreaDone;
		fPerturbShader->GetValue(result, fullAreaDone, fShadingIn);
	}
	return result;
}

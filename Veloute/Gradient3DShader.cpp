/****************************************************************************************************

		Gradient3DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#include "Gradient3DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_Gradient3DShader(R_CLSID_Gradient3DShader);

PMapGradient3DShader::PMapGradient3DShader()
{
	Init();
}

void PMapGradient3DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();

	fType = 'Opt1'; // linear
	fRepeat = 1;
	fSmooth = false; // Linear interpolation
	fAA = false; // Always off: I couldn't see any diference
}

Gradient3DShader::Gradient3DShader()
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}

boolean Gradient3DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return (
		(fPMap.fType == ((Gradient3DShader*)aShader)->fPMap.fType) &&
		(fPMap.fRepeat == ((Gradient3DShader*)aShader)->fPMap.fRepeat) &&
		(fPMap.fSmooth == ((Gradient3DShader*)aShader)->fPMap.fSmooth) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((Gradient3DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr Gradient3DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsPointLoc = true;
	theFlags.fNeedsPoint = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Gradient3DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr Gradient3DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

real Gradient3DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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

		if(fPMap.fRepeat)
		{	// Normal case: several gradient can be visible if scaling>100%
			point.x = point.x - floor(point.x);
			point.y = point.y - floor(point.y);
			point.z = point.z - floor(point.z);
		}
		else
		{	// Special case: only 1 gradient is visible
			point.x = MC_Clamp(point.x,(real32) 0,(real32) 1);
			point.y = MC_Clamp(point.y,(real32) 0,(real32) 1);
			point.z = MC_Clamp(point.z,(real32) 0,(real32) 1);
		}

		result = GetGlobalSample(point.x,point.y,point.z,shadingIn);
	}
	else
	{
		// Transform the point
		TVector3 point = f3DTransform.TransformPoint(shadingIn.fPointLoc);

		if(fPMap.fRepeat)
		{	// Normal case: several gradient can be visible if scaling>100%
			point.x = point.x - floor(point.x);
			point.y = point.y - floor(point.y);
			point.z = point.z - floor(point.z);
		}
		else
		{	// Special case: only 1 gradient is visible

			point.x = MC_Clamp(point.x,(real32) 0,(real32) 1);
			point.y = MC_Clamp(point.y,(real32) 0,(real32) 1);
			point.z = MC_Clamp(point.z,(real32) 0,(real32) 1);
		}

		result = GetLocalSample(point.x,point.y,point.z,shadingIn);
	}

	fullArea = true;

	PostProcess(result, fPMap.mGrayScale);

	return 1.0f;
}

real Gradient3DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
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

		if(fPMap.fRepeat)
		{	// Normal case: several gradient can be visible if scaling>100%
			point.x = point.x - floor(point.x);
			point.y = point.y - floor(point.y);
			point.z = point.z - floor(point.z);
		}
		else
		{	// Special case: only 1 gradient is visible
			point.x = MC_Clamp(point.x,(real32) 0,(real32) 1);
			point.y = MC_Clamp(point.y,(real32) 0,(real32) 1);
			point.z = MC_Clamp(point.z,(real32) 0,(real32) 1);
		}

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

		if(fPMap.fRepeat)
		{	// Normal case: several gradient can be visible if scaling>100%
			point.x = point.x - floor(point.x);
			point.y = point.y - floor(point.y);
			point.z = point.z - floor(point.z);
		}
		else
		{	// Special case: only 1 gradient is visible
			point.x = MC_Clamp(point.x,(real32) 0,(real32) 1);
			point.y = MC_Clamp(point.y,(real32) 0,(real32) 1);
			point.z = MC_Clamp(point.z,(real32) 0,(real32) 1);
		}

		LocalSuperSampling(point.x, point.y, point.z, shadingIn, value1, value2, value3, value4);

		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetLocalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}

	return 1.0f;
}

real32 Gradient3DShader::ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	return ComputeOneLocalSample(x,y,z,samplingRate);
}

real32 Gradient3DShader::ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	real32 value=0;
	int32 axis = 2;
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*x;
				value = value - floor( value );
			}
			else
				value = x;
		} break;
	case 'Opt2':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*x;
				value = value - floor( value );
			}
			else
				value = x;

			value = 2*(RealAbs(value-.5));
		} break;
	case 'Opt3':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*y;
				value = value - floor( value );
			}
			else
				value = y;
		} break;
	case 'Opt4':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*y;
				value = value - floor( value );
			}
			else
				value = y;

			value = 2*(RealAbs(value-.5));
		} break;
	case 'Opt5':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*z;
				value = value - floor( value );
			}
			else
				value = z;
		} break;
	case 'Opt6':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*z;
				value = value - floor( value );
			}
			else
				value = z;

			value = 2*(RealAbs(value-.5));
		} break;
	case 'Opt7':
		{
			const real32 newX = (x-.5);
			const real32 newY = (y-.5);
			const real32 newZ = (z-.5);
	
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*RealSqrt( (newX*newX + newY*newY + newZ*newZ)/.25 );
				value = value - floor( value );
			}
			else
				value = RealSqrt((newX*newX + newY*newY + newZ*newZ)/.25);
		} break;
	case 'Opt8':
		{
			const real32 newX = (x-.5);
			const real32 newY = (y-.5);
			const real32 newZ = (z-.5);
	
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*RealSqrt((newX*newX + newY*newY + newZ*newZ)/.25);
				value = value - floor( value );
			}
			else
				value = RealSqrt((newX*newX + newY*newY + newZ*newZ)/.25);

			value = 2*(RealAbs(value-.5));
		} break;

	case 'Opt9':
	case 'Op10': axis--; // x axis -> go to 0
	case 'Op11':
	case 'Op12': axis--; // y axis -> go to 1
	case 'Op13':
	case 'Op14':
		{	// Cylinder
			real32 val0 = 0;
			real32 val1 = 0;
			if(axis==0) // X axis
			{
				val0 = (y-.5);
				val1 = (z-.5);
			}
			else if(axis==1) // y axis
			{
				val0 = (z-.5);
				val1 = (x-.5);
			}
			else // z axis
			{
				val0 = (x-.5);
				val1 = (y-.5);
			}
	
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*RealSqrt((val0*val0 + val1*val1)/.25);
				value = value - floor( value );
			}
			else
				value = RealSqrt((val0*val0 + val1*val1)/.25);

			if(fPMap.fType=='Op10'||fPMap.fType=='Op12'||fPMap.fType=='Op14')
				value = 2*(RealAbs(value-.5));
		} break;
	case 'Op15':
	case 'Op16':
		{	//Square
			real32 newX = (x-.5);
			real32 newY = (y-.5);
			real32 newZ = (z-.5);
	
			// Symmetry
			newX = RealAbs(newX);
			newY = RealAbs(newY);
			newZ = RealAbs(newZ);
			if(newZ>newX) SwitchValues(newX,newZ);
			if(newY>newX) SwitchValues(newX,newY);
		
			if(fPMap.fRepeat)
			{
				value = 2*fPMap.fRepeat*newX;
				value = value - floor( value );
			}
			else
				value = 2*newX;

			if(fPMap.fType=='Op16')
				value = 2*(RealAbs(value-.5));
		} break;

	}

	if(fPMap.fSmooth)
		return SmoothStep(MC_Clamp(value,kRealZero, kRealOne));
	else
		return MC_Clamp(value,kRealZero, kRealOne);
}

void Gradient3DShader::PreProcess()
{
	fPMap.mTransform.Get3DTransform( f3DTransform );

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f3DTransform, 100, 100, 100 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

/****************************************************************************************************

		Gradient2DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "Gradient2DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_Gradient2DShader(R_CLSID_Gradient2DShader);


PMapGradient2DShader::PMapGradient2DShader()
{
	Init();
}

void PMapGradient2DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();

	fType = 'Opt1'; // linear
	fRepeat = 1;
	fSmooth = false; // Linear interpolation
	fAA = false; // Always off: I couldn't see any diference
}

Gradient2DShader::Gradient2DShader()
{
	fNeedSamplingRate = false;
	f2DTransform = TMatrix33::kIdentity;
	fPreprocessed = false;
	fRepeatCoeff = 1.0;
}

boolean Gradient2DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise
{
	return (
		(fPMap.fType == ((Gradient2DShader*)aShader)->fPMap.fType) &&
		(fPMap.fRepeat == ((Gradient2DShader*)aShader)->fPMap.fRepeat) &&
		(fPMap.fSmooth == ((Gradient2DShader*)aShader)->fPMap.fSmooth) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((Gradient2DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr Gradient2DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput Gradient2DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr Gradient2DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

real Gradient2DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
	if(fPMap.fRepeat)
	{	// Normal case: several gradient can be visible if scaling>100%
		uv.x = uv.x - floor(uv.x);
		uv.y = uv.y - floor(uv.y);
	}
	else
	{	// Special case: only 1 gradient is visible
		uv.x = MC_Clamp(uv.x,(real32) 0,(real32) 1);
		uv.y = MC_Clamp(uv.y,(real32) 0,(real32) 1);
	}
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

real Gradient2DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
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
	if(fPMap.fRepeat)
	{	// Normal case: several gradient can be visible if scaling>100%
		uv.x = uv.x - floor(uv.x);
		uv.y = uv.y - floor(uv.y);
	}
	else
	{	// Special case: only 1 gradient is visible

		uv.x = MC_Clamp(uv.x,(real32) 0,(real32) 1);
		uv.y = MC_Clamp(uv.y,(real32) 0,(real32) 1);
	}
	uv*=100;
/*
	uv.x = uv.x - floor(uv.x/100);
	uv.y = uv.y - floor(uv.y/100);
*/
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uv.x, uv.y, shadingIn, value1, value2, value3, value4);

	PostProcess(value1, fPMap.mGrayScale);
	PostProcess(value2, fPMap.mGrayScale);
	PostProcess(value3, fPMap.mGrayScale);
	PostProcess(value4, fPMap.mGrayScale);

	GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);

	return 1.0f;
}

real32 Gradient2DShader::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	real32 value=0;
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*uu/(100);
				value = value - floor( value );
			}
			else
				value = uu/100;
		} break;
	case 'Opt2':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*uu/100;
				value = value - floor( value );
			}
			else
				value = uu/100;

			value = 2*(RealAbs(value-.5));
		} break;
	case 'Opt3':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*vv/100;
				value = value - floor( value );
			}
			else
				value = vv/100;
		} break;
	case 'Opt4':
		{
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*vv/100;
				value = value - floor( value );
			}
			else
				value = vv/100;

			value = 2*(RealAbs(value-.5));
		} break;
	case 'Opt5':
		{
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);
	
			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*RealSqrt( (newU*newU + newV*newV)/2500 );
				value = value - floor( value );
			}
			else
				value = RealSqrt( (newU*newU + newV*newV)/2500 );
		} break;
	case 'Opt6':
		{
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);

			if(fPMap.fRepeat)
			{
				value = fPMap.fRepeat*RealSqrt( (newU*newU + newV*newV)/2500 );
				value = value - floor( value );
			}
			else
				value = RealSqrt( (newU*newU + newV*newV)/2500 );

			value = 2*(RealAbs(value-.5));
		} break;

	case 'Opt7':
		{	// Conical
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);

			TVector2 dir(newU,newV);
			dir.Normalize();

			// get the positive angle in deg
			real32 angle = GetPositiveAngle(TVector2::kUnitX, dir);

			value = fRepeatCoeff*angle;
			value = value - floor( value );
		} break;
	case 'Opt8':
		{	// Conical with symmetry
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);

			TVector2 dir(newU,newV);
			dir.Normalize();

			// get the positive angle in deg
			real32 angle = GetPositiveAngle(TVector2::kUnitX, dir)*fRepeatCoeff;
			angle = angle - floor( angle );

			value = 2*angle;
			if(value>1)
				value = 2-value;
		} break;
	case 'Opt9':
	case 'Op10':
		{	// Square
			real32 newU = (uu-50);
			real32 newV = (vv-50);

			// Symmetry
			newU = RealAbs(newU);
			newV = RealAbs(newV);
			if(newV>newU)
				SwitchValues(newU,newV);

			// then linear
			if(fPMap.fRepeat)
			{
				value = fRepeatCoeff*newU;
				value = value - floor( value );
			}
			else
				value = newU/50;

			if(fPMap.fType == 'Op10')
				value = 2*(RealAbs(value-.5));
		} break;
	case 'Op11':
	case 'Op12':
	case 'Op13':
	case 'Op14':
		{	// Spiral (right and left)		
			const real32 newU = (uu-50);
			const real32 newV = (vv-50);

			TVector2 dir(newU,newV);
			TVector2 unitDir;
			real32 dist = fRepeatCoeff*dir.Normalize(unitDir);
	
			// get the positive angle in deg
			real32 angle = GetPositiveAngle(TVector2::kUnitX, unitDir)/360;
			if(fPMap.fType == 'Op12' || fPMap.fType == 'Op14')
				angle*=-1;

			value = (dist-angle);
			value = value - floor( value );
		
			if(fPMap.fType == 'Op13' || fPMap.fType == 'Op14')
				value = 2*(RealAbs(value-.5));
		} break;
	}


	if(fPMap.fSmooth)
		return SmoothStep(value);
	else
		return value;
}

void Gradient2DShader::PreProcess()
{
	fPMap.mTransform.Get2DTransform( f2DTransform );

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f2DTransform, 100, 100 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	switch(fPMap.fType)
	{
	case 'Opt7':fRepeatCoeff = (fPMap.fRepeat?fPMap.fRepeat/360:1.0/360); break;
	case 'Opt8':fRepeatCoeff = (fPMap.fRepeat?fPMap.fRepeat/360:1.0/360); break; // Optioin 8 (conical with symmetry)
	case 'Opt9':
	case 'Op10':
	case 'Op11':
	case 'Op12':
	case 'Op13':
	case 'Op14':fRepeatCoeff = (fPMap.fRepeat?fPMap.fRepeat/50:1.0/50); break;
	}
	
	

	fPreprocessed=true;
}

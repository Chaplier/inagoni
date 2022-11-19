/****************************************************************************************************

		BumpNoise3DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/
#include "BumpNoise3DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_BumpNoise3DShader(R_CLSID_BumpNoise3DShader);

PMapBumpNoise3DShader::PMapBumpNoise3DShader()
{
	Init();
}

void PMapBumpNoise3DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();
	
	fSeed = 379485;
	fType = 'Noi1'; // Gradient noise
	
	fParam1 = .5; // Roughness
	fParam2 = 0; // Fractal depth

	fAA = false;
}

BumpNoise3DShader::BumpNoise3DShader()
{
	fPreprocessed = false;
}

boolean BumpNoise3DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise3DShader
{
	return (
		(fPMap.fSeed == ((BumpNoise3DShader*)aShader)->fPMap.fSeed) &&
		(fPMap.fType == ((BumpNoise3DShader*)aShader)->fPMap.fType) &&
		(fPMap.fParam1 == ((BumpNoise3DShader*)aShader)->fPMap.fParam1) &&
		(fPMap.fParam2 == ((BumpNoise3DShader*)aShader)->fPMap.fParam2) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((BumpNoise3DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr BumpNoise3DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsPointLoc = true;
	theFlags.fNeedsPoint = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput BumpNoise3DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr BumpNoise3DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr BumpNoise3DShader::HandleEvent(MessageID message, IMFResponder* source, void* data)
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

real BumpNoise3DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
		const TVector3 point = f3DTransform.TransformPoint(shadingIn.fPoint);

		result = GetGlobalSample(point.x,point.y,point.z,shadingIn);
	}
	else
	{
		// Transform the point
		const TVector3 point = f3DTransform.TransformPoint(shadingIn.fPointLoc);

		result = GetLocalSample(point.x,point.y,point.z,shadingIn);
	}

	fullArea = true;

	PostProcess(result, fPMap.mGrayScale);

	return 1.0f;
}

real BumpNoise3DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
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
		const TVector3 point = f3DTransform.TransformPoint(shadingIn.fPoint);

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
		const TVector3 point = f3DTransform.TransformPoint(shadingIn.fPointLoc);

		LocalSuperSampling(point.x, point.y, point.z, shadingIn, value1, value2, value3, value4);
	
		PostProcess(value1, fPMap.mGrayScale);
		PostProcess(value2, fPMap.mGrayScale);
		PostProcess(value3, fPMap.mGrayScale);
		PostProcess(value4, fPMap.mGrayScale);

		GetLocalPerturbationVector(result, shadingIn, value1, value2, value3, value4);
	}

	return 1.0f;
}

real32 BumpNoise3DShader::ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	return ComputeOneLocalSample(x, y, z, samplingRate);
}

real32 BumpNoise3DShader::ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate)
{
	switch(fPMap.fType)
	{
	case 'Noi1':
		{
			TVector3 point(x,y,z);
			real32 result=0;
			if(fPMap.fParam2) // Fractal version
				result = fNoise.GetSum(point,fRoughness, samplingRate);
			else
				result = fNoise.GetValueVerySmooth(point);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		} 
	case 'Noi2':
		{
			TVector3 point(x,y,z);
			if(fPMap.fParam2) // Fractal version
				return fNoise.GetTurbulence(point,fRoughness, samplingRate);
			else
				return RealAbs(fNoise.GetValueVerySmooth(point));
		} 
	case 'Noi3':
		{
			TVector3 point(x,y,z);
			if(fPMap.fParam2) // Fractal version
				return RealAbs( fNoise.GetSum(point,fRoughness, samplingRate) ); // Fractal
			else
				return RealAbs(fNoise.GetValueVerySmooth(point));
		} 
	case 'Noi4':
		{
			TVector3 point(x,y,z);
			real32 result=0;
			if(fPMap.fParam2) // Fractal version
				result = fNoise.GetLinearSum(point,fRoughness, samplingRate);
			else
				result = fNoise.GetValueLinear(point);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Sinus
	case 'Sin1':
		{
			TVector3 point(x,y,z);
			real32 result=0;
			// Fractal version
			result = fNoise.GetSinWaveNoise(point,fRoughness*.707, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Squares
	case 'Box1':
		{
			TVector3 point(x,y,z);
			real32 result=0;
			// Fractal version
			result = fNoise.GetBoxNoise(point,fRoughness, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
	case 'Sph1':
		{
			TVector3 point(x,y,z);
			real32 result=0;
			// Fractal version
			result = fNoise.GetSmoothBoxNoise(point,fRoughness, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Cellules
	case 'Cel1': return F1(x,y,z,0,samplingRate); // Harsh cellules
	case 'Cel2': return F1(x,y,z,5,samplingRate); // Soft cellules
	case 'Cel3': return Product(x,y,z,0,samplingRate); // Irregular harsh cellules
	case 'Cel4': return Average(x,y,z,5,samplingRate); // Irregular soft cellules
	case 'Cel5': return Average(x,y,z,12,samplingRate); //  Rough cellules 1
	case 'Cel6': return F2(x,y,z,0,samplingRate); // Rough cellules 2
	case 'Cel7': return F2(x,y,z,5,samplingRate); // Rough cellules 3
	case 'Cel8': return F2(x,y,z,12,samplingRate); // Rough cellules 4
// Puzzles
	case 'Puz1': return Puzzle(x,y,z,0,samplingRate); // Tiles
	case 'Puz2': return Puzzle(x,y,z,5,samplingRate); // Rounded Tiles
	case 'Puz3': return Puzzle(x,y,z,12,samplingRate); // More Rounded Tiles
	case 'Puz4': return Puzzle(x,y,z,10,samplingRate); // Triangle and Square Tiles
	case 'Puz5': return Puzzle(x,y,z,8,samplingRate); // Geometric Tiles
// Synthetic patterns
	case 'Syn1': return F2(x,y,z,8,samplingRate); // Synthetique Tiles
	case 'Syn2': return Product(x,y,z,10,samplingRate); // Synthetique Tiles
	case 'Syn3': return HoleAma(x,y,z,7,samplingRate); // Synthetique Tiles
	case 'Syn4': return PureAma(x,y,z,7,samplingRate); // Synthetique Tiles
	case 'Syn5': return Puzzle(x,y,z,7,samplingRate); // Synthetique Tiles
	case 'Syn6': return PureAma(x,y,z,10,samplingRate); // Synthetique Tiles
	case 'Syn7': return HoleAma(x,y,z,10,samplingRate); // Synthetique Tiles
	case 'Syn8': return HoleAma(x,y,z,11,samplingRate); // Synthetique Tiles
	case 'Syn9': return Average(x,y,z,9,samplingRate); // Synthetique Tiles
// Regular patterns
	case 'Reg1': return RegularPattern(x,y,z,0,samplingRate); // Circles
	case 'Reg2': return RegularPattern(x,y,z,5,samplingRate); // Rounded squares
	case 'Reg3': return RegularPattern(x,y,z,8,samplingRate); // Squares
	case 'Reg4': return RegularPattern(x,y,z,7,samplingRate); // Rect+squares
	case 'Reg5': return RegularPattern(x,y,z,9,samplingRate); // Diagonals
// Globule amas
	case 'Ama1': return PureAma(x,y,z,0,samplingRate); // Pure Spheres
	case 'Ama2': return PureAma(x,y,z,5,samplingRate); // Pure globules
	case 'Ama3': return SoftAma(x,y,z,0,samplingRate); // Soft spheres
	case 'Ama4': return SoftAma(x,y,z,5,samplingRate); // Soft globules
	case 'Ama5': return HoleAma(x,y,z,0,samplingRate); // Donuts
	case 'Ama6': return HoleAma(x,y,z,5,samplingRate); // Squary holes
	}

	return .1f;
}

real32 BumpNoise3DShader::F1(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*F1;
}

real32 BumpNoise3DShader::F2(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*F2;
}

real32 BumpNoise3DShader::Puzzle(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return RealAbs(F2-F1);
}

real32 BumpNoise3DShader::PuzzleBis(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	const real32 a = F1;
	const real32 b = F2;
	return .25*RealAbs(b*b-a*a);
}

real32 BumpNoise3DShader::Average(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .25*(F2+F1);
}

real32 BumpNoise3DShader::Product(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .25*F2*F1;
}

real32 BumpNoise3DShader::RegularPattern(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return (.5+.5*cos(F1*PI*10)); // Note: the 10 can be modify to have more or less ondulations
}

real32 BumpNoise3DShader::PureAma(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*(1+cos(F1*PI*.5));
}

real32 BumpNoise3DShader::HoleAma(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return (sin(F1*PI*.5));
}

real32 BumpNoise3DShader::SoftAma(const real32 x, const real32 y, const real32 z, const int32 distanceMethod, const real32 samplingRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(x,y,z,distanceMethod,fRoughness, samplingRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(x,y,z,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	const real32 a = (1+cos(F1*PI*.5));
	const real32 b = (sin(F2*PI*.5));
	return (a+b)*.25;
}

void BumpNoise3DShader::PreProcess()
{
	// Perlin noise
	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();
	fNoise.SetMaxFrequency(2*RealPow(2,fPMap.fParam2));

	// Voronoi noise
	fVoronoi.SetSeed(fPMap.fSeed);
	fVoronoi.SetMaxFrequency(2*RealPow(2,fPMap.fParam2));

	fPMap.mTransform.Get3DTransform( f3DTransform );

	fRoughness=fPMap.fParam1*2;

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f3DTransform, 1, 1, 1 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

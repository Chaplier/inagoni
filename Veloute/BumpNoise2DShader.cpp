/****************************************************************************************************

		BumpNoise2DShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#include "BumpNoise2DShader.h"

#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_BumpNoise2DShader(R_CLSID_BumpNoise2DShader);

PMapBumpNoise2DShader::PMapBumpNoise2DShader()
{
	Init();
}

void PMapBumpNoise2DShader::Init()
{
	mGrayScale.Init();
	mTransform.Init();

	fSeed = 379485;
	fType = 'Noi1'; // Gradient noise
	
	fParam1 = .5; // Roughness
	fParam2 = 0; // Fractal depth

	fAA = false;
}

BumpNoise2DShader::BumpNoise2DShader()
{
//	f2DTransform = TMatrix33::kIdentity;
	fPreprocessed = false;
}

boolean BumpNoise2DShader::IsEqualTo(I3DExShader* aShader)		// Use it to compare two BumpNoise2DShader
{
	return (
		(fPMap.fSeed == ((BumpNoise2DShader*)aShader)->fPMap.fSeed) &&
		(fPMap.fType == ((BumpNoise2DShader*)aShader)->fPMap.fType) &&
		(fPMap.fParam1 == ((BumpNoise2DShader*)aShader)->fPMap.fParam1) &&
		(fPMap.fParam2 == ((BumpNoise2DShader*)aShader)->fPMap.fParam2) &&
		GrayScaleBase::IsEqualTo(aShader) &&
		(fPMap.mTransform.IsEqualTo( &((BumpNoise2DShader*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr BumpNoise2DShader::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput BumpNoise2DShader::GetImplementedOutput()
{
	return (EShaderOutput)(GrayScaleBase::GetImplementedOutput()|kUsesGetVector);	// We use GetValue with shading Area
}

MCCOMErr BumpNoise2DShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr BumpNoise2DShader::HandleEvent(MessageID message, IMFResponder* source, void* data)
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

real BumpNoise2DShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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

real BumpNoise2DShader::GetVector(TVector3& result, ShadingIn& shadingIn)
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

	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uv.x, uv.y, shadingIn, value1, value2, value3, value4);

	PostProcess(value1, fPMap.mGrayScale);
	PostProcess(value2, fPMap.mGrayScale);
	PostProcess(value3, fPMap.mGrayScale);
	PostProcess(value4, fPMap.mGrayScale);

	GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);

	return 1.0f;
}

real32 BumpNoise2DShader::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	TVector2 point;

	point.x = uu;
	point.y = vv;

	switch(fPMap.fType)
	{
	case 'Noi1':
		{
			real32 result=0;
			if(fPMap.fParam2) // Fractal version
				result = fNoise.GetSum2D(point,fRoughness, samplingRate);
			else
				result = fNoise.GetValueVerySmooth2D(point);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		} 
	case 'Noi2':
		{
			if(fPMap.fParam2) // Fractal version
				return fNoise.GetTurbulence2D(point,fRoughness, samplingRate);
			else
				return RealAbs(fNoise.GetValueVerySmooth2D(point));
		} 
	case 'Noi3':
		{
			if(fPMap.fParam2) // Fractal version
				return RealAbs( fNoise.GetSum2D(point,fRoughness, samplingRate) ); // Fractal
			else
				return RealAbs(fNoise.GetValueVerySmooth2D(point));
		} 
	case 'Noi4':
		{
			real32 result=0;
			if(fPMap.fParam2) // Fractal version
				result = fNoise.GetLinearSum2D(point,fRoughness, samplingRate);
			else
				result = fNoise.GetValueLinear2D(point);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Sinus
	case 'Sin1':
		{
			real32 result=0;
			// Fractal version
			result = fNoise.GetSinWaveNoise2D(point,fRoughness*.707, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Squares
	case 'Box1':
		{
			real32 result=0;
			// Fractal version
			result = fNoise.GetBoxNoise2D(point,fRoughness, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
	case 'Sph1':
		{
			real32 result=0;
			// Fractal version
			result = fNoise.GetSmoothBoxNoise2D(point,fRoughness, samplingRate);

			// Rescale the value on a [0,1] space
			result*= .5;
			result += .5;
			return result;
		}
// Cellules
	case 'Cel1': return F1(uu,vv,0,samplingRate); // Harsh cellules
	case 'Cel2': return F1(uu,vv,5,samplingRate); // Soft cellules
	case 'Cel3': return Product(uu,vv,0,samplingRate); // Irregular harsh cellules
	case 'Cel4': return Average(uu,vv,5,samplingRate); // Irregular soft cellules
	case 'Cel5': return Average(uu,vv,12,samplingRate); //  Rough cellules 1
	case 'Cel6': return F2(uu,vv,0,samplingRate); // Rough cellules 2
	case 'Cel7': return F2(uu,vv,5,samplingRate); // Rough cellules 3
	case 'Cel8': return F2(uu,vv,12,samplingRate); // Rough cellules 4
// Puzzles
	case 'Puz1': return Puzzle(uu,vv,0,samplingRate); // Tiles
	case 'Puz2': return Puzzle(uu,vv,5,samplingRate); // Rounded Tiles
	case 'Puz3': return Puzzle(uu,vv,12,samplingRate); // More Rounded Tiles
	case 'Puz4': return Puzzle(uu,vv,10,samplingRate); // Triangle and Square Tiles
	case 'Puz5': return Puzzle(uu,vv,8,samplingRate); // Geometric Tiles
// Synthetic patterns
	case 'Syn1': return F2(uu,vv,8,samplingRate); // Synthetique Tiles
	case 'Syn2': return Product(uu,vv,10,samplingRate); // Synthetique Tiles
	case 'Syn3': return HoleAma(uu,vv,7,samplingRate); // Synthetique Tiles
	case 'Syn4': return PureAma(uu,vv,7,samplingRate); // Synthetique Tiles
	case 'Syn5': return Puzzle(uu,vv,7,samplingRate); // Synthetique Tiles
	case 'Syn6': return PureAma(uu,vv,10,samplingRate); // Synthetique Tiles
	case 'Syn7': return HoleAma(uu,vv,10,samplingRate); // Synthetique Tiles
	case 'Syn8': return HoleAma(uu,vv,11,samplingRate); // Synthetique Tiles
	case 'Syn9': return Average(uu,vv,9,samplingRate); // Synthetique Tiles
// Regular patterns
	case 'Reg1': return RegularPattern(uu,vv,0,samplingRate); // Circles
	case 'Reg2': return RegularPattern(uu,vv,5,samplingRate); // Rounded squares
	case 'Reg3': return RegularPattern(uu,vv,8,samplingRate); // Squares
	case 'Reg4': return RegularPattern(uu,vv,7,samplingRate); // Rect+squares
	case 'Reg5': return RegularPattern(uu,vv,9,samplingRate); // Diagonals
// Globule amas
	case 'Ama1': return PureAma(uu,vv,0,samplingRate); // Pure Spheres
	case 'Ama2': return PureAma(uu,vv,5,samplingRate); // Pure globules
	case 'Ama3': return SoftAma(uu,vv,0,samplingRate); // Soft spheres
	case 'Ama4': return SoftAma(uu,vv,5,samplingRate); // Soft globules
	case 'Ama5': return HoleAma(uu,vv,0,samplingRate); // Donuts
	case 'Ama6': return HoleAma(uu,vv,5,samplingRate); // Squary holes
	}
	return .1f;
}

real32 BumpNoise2DShader::F1(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*F1;
}

real32 BumpNoise2DShader::F2(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*F2;
}

real32 BumpNoise2DShader::Puzzle(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return RealAbs(F2-F1);
}

real32 BumpNoise2DShader::PuzzleBis(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	const real32 a = F1;
	const real32 b = F2;
	return .25*RealAbs(b*b-a*a);
}

real32 BumpNoise2DShader::Average(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .25*(F2+F1);
}

real32 BumpNoise2DShader::Product(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .25*F2*F1;
}

real32 BumpNoise2DShader::RegularPattern(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return (.5+.5*cos(F1*PI*10)); // Note: the 10 can be modify to have more or less ondulations
}

real32 BumpNoise2DShader::PureAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return .5*(1+cos(F1*PI*.5));
}

real32 BumpNoise2DShader::HoleAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	return (sin(F1*PI*.5));
}

real32 BumpNoise2DShader::SoftAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplinRate)
{
	real32 F1=0, F2=0;
	if(fPMap.fParam2) // Fractal version
		fVoronoi.GetFractalF1AndF2(uu,vv,distanceMethod,fRoughness, samplinRate,F1,F2);
	else
		fVoronoi.GetF1AndF2(uu,vv,distanceMethod,F1,F2);

	// Rescale the value on a [0,1] space
	const real32 a = (1+cos(F1*PI*.5));
	const real32 b = (sin(F2*PI*.5));
	return (a+b)*.25;
}

void BumpNoise2DShader::PreProcess()
{
	// Perlin noise
	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();
	fNoise.SetMaxFrequency(2*RealPow(2,fPMap.fParam2));

	// Voronoi noise
	fVoronoi.SetSeed(fPMap.fSeed);
	fVoronoi.SetMaxFrequency(2*RealPow(2,fPMap.fParam2));

	fPMap.mTransform.Get2DTransform( f2DTransform );

	fRoughness=fPMap.fParam1*2;

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f2DTransform, 100, 100 );

	GrayScaleBase::PreProcess(fPMap.mGrayScale);

	fPreprocessed=true;
}

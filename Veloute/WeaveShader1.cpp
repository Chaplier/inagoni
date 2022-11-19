/****************************************************************************************************

		WeaveShader1.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/4/2005

****************************************************************************************************/

#include "WeaveShader1.h"

#include "math.h"
#include "Utils.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"

const MCGUID CLSID_WeaveShader1(R_CLSID_WeaveShader1);
const MCGUID CLSID_GlobalWeaveShader1(R_CLSID_GlobalWeaveShader1);

static const int32 kOutOfThread = -1;

static const int32 kSShape = 'SSha';
static const int32 kZShape = 'ZSha';

PMapWeaveShader1::PMapWeaveShader1()
{
	fType = 'Opt1';

	fWeaveCountU=10;
	fWeaveCountV=10;

	fSpacingU = .5f;
	fSpacingV = .5f;

	fFlatArea = 0;

	fShape = kSShape;

	fBumpAmplitude = .5f;

	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

WeaveShader1::WeaveShader1()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;

	fULength = 0;
	fVLength = 0;
	fMinLocalU = 0;
	fMinLocalV = 0;

	fiU = kOutOfThread;
	fiV = kOutOfThread;

	fThreadOriginUV = TVector2::kZero;
}
	
boolean WeaveShader1::IsEqualTo(I3DExShader* aShader)		// Use it to compare two WeaveShader1
{
	return (
		(fPMap.fType == ((WeaveShader1*)aShader)->fPMap.fType) &&
		(fPMap.fWeaveCountU == ((WeaveShader1*)aShader)->fPMap.fWeaveCountU) &&
		(fPMap.fWeaveCountV == ((WeaveShader1*)aShader)->fPMap.fWeaveCountV) &&
		(fPMap.fSpacingU == ((WeaveShader1*)aShader)->fPMap.fSpacingU) &&
		(fPMap.fSpacingV == ((WeaveShader1*)aShader)->fPMap.fSpacingV) &&
		(fPMap.fFlatArea == ((WeaveShader1*)aShader)->fPMap.fFlatArea) &&
		(fPMap.fBumpAmplitude == ((WeaveShader1*)aShader)->fPMap.fBumpAmplitude) &&
		(fPMap.fShadersComponents[0] == ((WeaveShader1*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((WeaveShader1*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((WeaveShader1*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((WeaveShader1*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr WeaveShader1::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput WeaveShader1::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr WeaveShader1::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

void WeaveShader1::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	iU = iV = kOutOfThread;

	// First bring back the UV into a 1*1 pattern, to see if we're in a U thread, a V thread, on both or outside
	const real32 localU = fromUV.x / fULength;
	const real32 localV = fromUV.y / fVLength;

	iU = floor(localU); 
	iV = floor(localV); 

	real32 oneU = localU-iU;
	real32 oneV = localV-iV;

	// Symetry around u=.5 and v=.5
	// Not now: need to keep U and V for the shading
//	if(oneU>.5) oneU = 1-oneU;
//	if(oneV>.5) oneV = 1-oneV;

	// We're now in 1 intersection between 2 threads.

	if(oneU<fMinLocalU || oneU>1-fMinLocalU)
		iU = kOutOfThread; // outside the thread
	if(oneV<fMinLocalV || oneV>1-fMinLocalV)
		iV = kOutOfThread; // outside the thread

	if(iU!=kOutOfThread && iV!=kOutOfThread)
	{	// the value is in 2 threads: see which on is over
		switch(fPMap.fType)
		{
		case 'Opt1':
			{
				// O U
				// U O

				// Bring back the UV into a 2*2 pattern
				real32 patternU = .5*localU;
				real32 patternV = .5*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if( (patternU<.5 && patternV<.5) ||
					(patternU>.5 && patternV>.5) )
					iU=kOutOfThread;
				else
					iV=kOutOfThread;
			} break;
		case 'Opt2':
			{
				// O O U U
				// O O U U
				// U U O O
				// U U O O

				// Bring back the UV into a 4*4 pattern
				real32 patternU = .25*localU;
				real32 patternV = .25*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if( (patternU<.5 && patternV<.5) ||
					(patternU>.5 && patternV>.5) )
					iU=kOutOfThread;
				else
					iV=kOutOfThread;
			} break;
		case 'Opt3':
			{
				// O U U
				// U O O
				// U O O

				// Bring back the UV into a 3*3 pattern
				real32 patternU = kOneThird*localU;
				real32 patternV = kOneThird*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if( (patternU<kOneThird && patternV<kOneThird) ||
					(patternU>kOneThird && patternV>kOneThird) )
					iU=kOutOfThread;
				else
					iV=kOutOfThread;
			} break;
		case 'Opt4':
			{
				// O U U U
				// U O O O
				// U O O O
				// U O O O

				// Bring back the UV into a 4*4 pattern
				real32 patternU = .25*localU;
				real32 patternV = .25*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if( (patternU<.25 && patternV<.25) ||
					(patternU>.25 && patternV>.25) )
					iU=kOutOfThread;
				else
					iV=kOutOfThread;
			} break;
		case 'Opt5':
			{
				// O O U U
				// O U U O
				// U U O O
				// U O O U

				// Bring back the UV into a 4*4 pattern
				real32 patternU = .25*localU;
				real32 patternV = .25*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if(fPMap.fShape == kSShape)
				{
					real32 diff = patternV - .25*floor(4*patternU);
					if(diff<0) diff+=1;
					if( diff<.5 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
				else
				{
					real32 sum = patternV + .25*floor(4*patternU);
					if(sum>1) sum-=1;
					if( sum>.5 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
			} break;
		case 'Opt6':
			{
				// O O O U
				// O O U O
				// O U O O
				// U O O O

				// Bring back the UV into a 4*4 pattern
				real32 patternU = .25*localU;
				real32 patternV = .25*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if(fPMap.fShape == kSShape)
				{
					real32 diff = patternV - .25*floor(4*patternU);
					if(diff<0) diff+=1;
					if( diff<.25 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
				else
				{
					real32 sum = patternV + .25*floor(4*patternU);
					if(sum>1) sum-=1;
					if( sum>.75 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
			} break;
		case 'Opt7':
			{
				// O O U
				// O U O
				// U O O

				// Bring back the UV into a 3*3 pattern
				real32 patternU = kOneThird*localU;
				real32 patternV = kOneThird*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if(fPMap.fShape == kSShape)
				{
					real32 diff = patternV - kOneThird*floor(3*patternU);
					if(diff<0) diff+=1;
					if( diff<kOneThird )	iU=kOutOfThread;
					else					iV=kOutOfThread;
				}
				else
				{
					real32 sum = patternV + kOneThird*floor(3*patternU);
					if(sum>1) sum-=1;
					if( sum>kTwoThird )	iU=kOutOfThread;
					else				iV=kOutOfThread;
				}
			} break;
		case 'Opt8':
			{
				// O O O O U
				// O U O O O
				// O O O U O
				// U O O O O
				// O O U O O

				// Bring back the UV into a 5*5 pattern
				real32 patternU = .2*localU;
				real32 patternV = .2*localV;
				
				real32 jU = floor(patternU); 
				real32 jV = floor(patternV); 

				patternU -= jU;
				patternV -= jV;

				if(fPMap.fShape == kSShape)
				{
					real32 diff = patternV - .4*floor(5*patternU);
					if(diff<-1) diff+=2;
					if(diff<0) diff+=1;
					if( diff<.2 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
				else
				{
					real32 sum = patternV + .4*floor(5*patternU);
					if(sum>2) sum-=2;
					if(sum>1) sum-=1;
					if( sum>.8 )	iU=kOutOfThread;
					else			iV=kOutOfThread;
				}
			} break;
		}
	}

	fThreadOriginUV = TVector2::kZero;
	if(iV!=kOutOfThread)
	{	// Switch the U and V values to have a similar shading harizontaly and verticaly
		// : not here (otherwise need to flip all the UV data in the shadingIn structure)
		newU = oneU;//fromUV.y;
		newV = oneV;//fromUV.x;

		fThreadOriginUV.y = (iV+.5)*fVLength*.01;
	}
	else
	{
		newU = oneU;//fromUV.x;
		newV = oneV;//fromUV.y;

		if(iU!=kOutOfThread)
		{
			fThreadOriginUV.x = (iU+.5)*fULength*.01;
		}
	}
}

#if (VERSIONNUMBER >= 0x040000)
real WeaveShader1::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real WeaveShader1::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#endif
{
	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, fiU, fiV );

	if(fiU==kOutOfThread && fiV==kOutOfThread)
	{
		// Return the background shader
		fShaders.GetMortarColor(result, fullAreaDone, shadingIn);
	}
	else
	{
		const real32 value = fAmplitudeOffset+fPMap.fBumpAmplitude*GetSample(uu,vv,shadingIn);
		// Fake shading of the color using the value
		if(fiU!=kOutOfThread)
		{
			// We're on a horizontal thread, flip the values 
			const real32 tmp = shadingIn.fUV.x; shadingIn.fUV.x = shadingIn.fUV.y; shadingIn.fUV.y = tmp;
			const TVector2 vect = shadingIn.fUVx; shadingIn.fUVx = shadingIn.fUVy; shadingIn.fUVy = vect;
		}
		fShaders.GetTileColor(result, fullAreaDone, shadingIn, fThreadOriginUV);
		result *= value;
	}

	return 1.0f;
}

real WeaveShader1::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, fiU, fiV );

	if(fiU==kOutOfThread && fiV==kOutOfThread)
	{
		// Return the background shader
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else
	{
		const real32 value = fAmplitudeOffset+fPMap.fBumpAmplitude*GetSample(uu,vv,shadingIn);
		// Fake shading of the color using the value
		if(fiU!=kOutOfThread)
		{
			// We're on a horizontal thread, flip the values 
			const real32 tmp = shadingIn.fUV.x; shadingIn.fUV.x = shadingIn.fUV.y; shadingIn.fUV.y = tmp;
			const TVector2 vect = shadingIn.fUVx; shadingIn.fUVx = shadingIn.fUVy; shadingIn.fUVy = vect;
		}
		fShaders.GetTileValue(result, fullArea, shadingIn, 1);
		result *= value;
	}

	return 1.0f;
}

real WeaveShader1::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, fiU, fiV );

	if(fiU==kOutOfThread && fiV==kOutOfThread)
	{
		// Return the background shader
		fShaders.GetMortarVector(result, shadingIn);
	}
	else
	{
		// Fake shading of the color using the value
		if(fiU!=kOutOfThread)
		{
			// We're on a horizontal thread, flip the values 
			const real32 tmp = shadingIn.fUV.x; shadingIn.fUV.x = shadingIn.fUV.y; shadingIn.fUV.y = tmp;
			const TVector2 vect = shadingIn.fUVx; shadingIn.fUVx = shadingIn.fUVy; shadingIn.fUVy = vect;
		}
		fShaders.GetTileVector(result, shadingIn);
	
		// Modify the bump
		real32 value1=0,value2=0,value3=0,value4=0;
		SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
		TVector3 bumpPerturbation;
		GetPerturbationVector(bumpPerturbation, shadingIn, value1, value2, value3, value4);
		result = fPMap.fBumpAmplitude*bumpPerturbation + result;
	}

	return 1.0f;
}

real32 WeaveShader1::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	if(fiU == kOutOfThread)
	{	// In V
	const real32 min = fMinLocalV;
	const real32 max = .5 - (.5-fMinLocalV)*fPMap.fFlatArea;

	// Symetry around uu = .5
	const real32 v = (vv>.5)?1-vv:vv;

	return SmoothStepWithTan(min,max, 0, 2, // the slopes at the top and the bottom
											v);
	}
	else
	{	// In U
	const real32 min = fMinLocalU;
	const real32 max = .5 - (.5-fMinLocalU)*fPMap.fFlatArea;

		// Symetry around uu = .5
		const real32 u = (uu>.5)?1-uu:uu;

		return SmoothStepWithTan(min,max, 0, 2, // the slopes at the top and the bottom
												u);
	}
}

void WeaveShader1::PreProcess()
{
	fPMap.mTransform.Get2DTransform( f2DTransform );

	// SuperSampling quality
	SetSuperSamplingQuality( f2DTransform, 100,100);

	// The U and V value will be in a [0-100] space, preprocess some data for that
	fULength = 100/(real32)fPMap.fWeaveCountU;
	fVLength = 100/(real32)fPMap.fWeaveCountV;

	// The threads limits in a 1 y 1 space
	fMinLocalU = fPMap.fSpacingU*.5;
	fMinLocalV = fPMap.fSpacingV*.5;

	// 
	fAmplitudeOffset = 1-fPMap.fBumpAmplitude;

	// Get the sub shaders interfaces
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		if(fPMap.fShadersComponents[iShader] )
		{
			fPMap.fShadersComponents[iShader]->QueryInterface(IID_I3DShShader, (void**)&(fShaders.fShaders[iShader]));
		}
		else
			fShaders.fShaders[iShader]=NULL;
	}
	fPreprocessed=true;
}

///////////////////////////////////////////////////////////////////////////////////////////
GlobalWeaveShader1::GlobalWeaveShader1()
{
	// Default background shader is a black with transparency
	TMCCountedPtr<I3DShShader> colorShader;
	TMCColorRGB color = TMCColorRGB::kBlack;
	gShell3DUtilities->CreateColorShader(color, &colorShader);
	TMCCountedPtr<I3DShShader> valueShader;
	gShell3DUtilities->CreateValueShader(1, &valueShader);
	TMCCountedPtr<I3DShShader> backShader;
	gShell3DUtilities->CreateMultiChannelShader(colorShader, NULL, NULL, NULL, NULL, valueShader, NULL, NULL,
												 &backShader);

	backShader->QueryInterface( IID_IShParameterComponent, (void**)&(fPMap.fShadersComponents[1]) );

}

EShaderOutput GlobalWeaveShader1::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalWeaveShader1::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, fiU, fiV );

	if(fiU==kOutOfThread && fiV==kOutOfThread)
	{
		// Return the background shader
		fShaders.DoMortarShade(result, shadingIn);
	}
	else
	{
		if(fiU!=kOutOfThread)
		{
			// We're on a horizontal thread, flip the values 
			const real32 tmp = shadingIn.fUV.x; shadingIn.fUV.x = shadingIn.fUV.y; shadingIn.fUV.y = tmp;
			const TVector2 vect = shadingIn.fUVx; shadingIn.fUVx = shadingIn.fUVy; shadingIn.fUVy = vect;
		}
		fShaders.DoTileShade(result, shadingIn,fThreadOriginUV);

		// Modify the bump
		real32 value1=0,value2=0,value3=0,value4=0;
		SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
		TVector3 bumpPerturbation;
		GetPerturbationVector(bumpPerturbation, shadingIn, value1, value2, value3, value4);
		result.fChangedNormalLoc = fPMap.fBumpAmplitude*bumpPerturbation + result.fChangedNormalLoc;
	}

	return MC_S_OK;
}

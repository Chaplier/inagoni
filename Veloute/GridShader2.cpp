/****************************************************************************************************

		GridShader2.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "GridShader2.h"

#include "math.h"
#include "Utils.h"
#include "Veloute.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "MCRandom.h"

const MCGUID CLSID_GridShader2(R_CLSID_GridShader2);
const MCGUID CLSID_GlobalGridShader2(R_CLSID_GlobalGridShader2);

PMapGridShader2::PMapGridShader2()
{
	fSeed = 23459023;

	fType = 'Opt7';
	fHorizontal=20;
	fVertical=20;
	fTileProp=.5;
	fShifting=.5f;
	fSlope=.5f;
	fGapDepth=-.5f; // Autoblock tile option
	fGapInclination=.5f; // Autoblock tile option
	fThickness=.2f;
					
	fLSection=0.15f;
	fHSection=0.15f;
	fSectionPlateau=0;
	fMiddleSlope='Opt1';
	fSidesSlope='Opt3';
	fBumpDepth=.1f;
					
	fSmoothSection=true;
					
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

GridShader2::GridShader2()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;

	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShaders.fShaders[iShader]=NULL;
	}
}
	
boolean GridShader2::IsEqualTo(I3DExShader* aShader)		// Use it to compare two GridShader2
{
	return (
		(fPMap.fType == ((GridShader2*)aShader)->fPMap.fType) &&
		(fPMap.fHorizontal == ((GridShader2*)aShader)->fPMap.fHorizontal) &&
		(fPMap.fVertical == ((GridShader2*)aShader)->fPMap.fVertical) &&
		(fPMap.fTileProp == ((GridShader2*)aShader)->fPMap.fTileProp) &&
		(fPMap.fShifting == ((GridShader2*)aShader)->fPMap.fShifting) &&
		(fPMap.fSlope == ((GridShader2*)aShader)->fPMap.fSlope) &&
		(fPMap.fGapDepth == ((GridShader2*)aShader)->fPMap.fGapDepth) &&
		(fPMap.fGapInclination == ((GridShader2*)aShader)->fPMap.fGapInclination) &&
		(fPMap.fLSection == ((GridShader2*)aShader)->fPMap.fLSection) &&
		(fPMap.fHSection == ((GridShader2*)aShader)->fPMap.fHSection) &&
		(fPMap.fSectionPlateau == ((GridShader2*)aShader)->fPMap.fSectionPlateau) &&
		(fPMap.fBumpDepth == ((GridShader2*)aShader)->fPMap.fBumpDepth) &&
		(fPMap.fMiddleSlope == ((GridShader2*)aShader)->fPMap.fMiddleSlope) &&
		(fPMap.fSidesSlope == ((GridShader2*)aShader)->fPMap.fSidesSlope) &&
		(fPMap.fSmoothSection == ((GridShader2*)aShader)->fPMap.fSmoothSection) &&
		(fPMap.fShadersComponents[0] == ((GridShader2*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((GridShader2*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((GridShader2*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((GridShader2*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr GridShader2::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fNeedsIsoUV = true;	// For supersampling
	theFlags.fNeedsNormalDerivative = true;	// For bug fix
	theFlags.fNeedsNormal = true;	// For bug fix
	theFlags.fNeedsNormalLoc = true;	// For bug fix
	theFlags.fNeedsNormalLocDerivative = true;	// For bug fix
	theFlags.fChangesNormal = true;	// For bug fix
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput GridShader2::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr GridShader2::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr GridShader2::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (source->GetInstanceID() == 'SEED')
	{
		fPMap.fSeed = MCRandom();
		fPreprocessed=false;
		return MC_S_OK;
	}
	else
		return ShaderBase::HandleEvent( message, source, data);
}

#if (VERSIONNUMBER >= 0x040000)
real GridShader2::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real GridShader2::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
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

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	boolean flip=false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a color depending on the value
	if(value == 0)
	{	// we're in the grid
		fShaders.GetTileColor(result, fullAreaDone, shadingIn, fEngine.fTileCenterUV);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the background
		fShaders.GetMortarColor(result, fullAreaDone, shadingIn);
	}
	else
	{	// we're between the 2: darken the color
#if (VERSIONNUMBER >= 0x040000)
		TMCColorRGBA color1 = TMCColorRGBA::kWhiteNoAlpha;
		TMCColorRGBA color2 = TMCColorRGBA::kBlackNoAlpha;
#else
		TMCColorRGB color1 = TMCColorRGB::kWhite;
		TMCColorRGB color2 = TMCColorRGB::kBlack;
#endif
		fShaders.GetTileColor(color1, fullAreaDone, shadingIn, fEngine.fTileCenterUV);

		const real32 coeff = fPMap.fBumpDepth*value/fEngine.fMortarBumpDepth;
		result.Interpolate(color1,color2,coeff);
	}

	return 1.0f;
}

real GridShader2::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	boolean flip=false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a value depending on the value
	if(value == 0)
	{	// we're in the grid
		fShaders.GetTileValue(result, fullArea, shadingIn, 1);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the background
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else
	{	// we're between the 2: darken the value
		real32 value1;
		fShaders.GetTileValue(value1, fullArea, shadingIn, 1);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result = (1-coeff)*value1;
	}

	return 1.0f;
}

real GridShader2::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	boolean flip=false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	const TVector2 uvX = shadingIn.fUVx;
	const TVector2 uvY = shadingIn.fUVy;
	TVector2 flipX, flipY;
	if(flip)
	{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
		flipX = -shadingIn.fUVy;
		flipY = shadingIn.fUVx;

		shadingIn.fUVx = flipX;
		shadingIn.fUVy = flipY;
	}

	// Now determine if the point is in a mortar area or a tile area
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);

	if(flip)
	{	// restore the real values
		shadingIn.fUVx = uvX;
		shadingIn.fUVy = uvY;
	}

	// Now get a vector depending on the result
	const real32 value = (value1+value2+value3+value4)*.25;

	if(value == 0)
	{	// we're in the grid
		fShaders.GetTileVector(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the background
		fShaders.GetMortarVector(result, shadingIn);
	}
	else
	{	// we're between the 2: smooth
		GetPerturbationVector(result, shadingIn, 1-value1, 1-value2, 1-value3, 1-value4);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result += (1-coeff)*result;
	}

	return 1.0f;
}

void GridShader2::PreProcess()
{
	fPMap.mTransform.Get2DTransform( fEngine.f2DTransform );

	fEngine.PreProcess(fPMap);

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( fEngine.f2DTransform, 100/*(fTotalLength)*/, 100/*(fHeight)*/ );

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
GlobalGridShader2::GlobalGridShader2()
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

EShaderOutput GlobalGridShader2::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalGridShader2::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.Clear();
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	boolean flip=false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	const TVector2 uvX = shadingIn.fUVx;
	const TVector2 uvY = shadingIn.fUVy;
	TVector2 flipX, flipY;
	if(flip)
	{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
		flipX = -shadingIn.fUVy;
		flipY = shadingIn.fUVx;

		shadingIn.fUVx = flipX;
		shadingIn.fUVy = flipY;
	}

	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
	real32 value = (value1+value2+value3+value4)*.25;

	if(flip)
	{	// restore the real values
		shadingIn.fUVx = uvX;
		shadingIn.fUVy = uvY;
	}

	// Get the shading depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.DoTileShade(result, shadingIn,fEngine.fTileCenterUV);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile: define a UV space local to the tile
		fShaders.DoMortarShade(result, shadingIn);
	}
	else
	{	// we're between the 2: bump
		fShaders.DoTileShade(result, shadingIn,fEngine.fTileCenterUV);

		const real32 coeff = value/fEngine.fMortarBumpDepth;

		// ChangedNormal
		TVector3 bumpPerturbation;
		GetPerturbationVector(bumpPerturbation, shadingIn, 1-value1, 1-value2, 1-value3, 1-value4);
		result.fChangedNormalLoc = bumpPerturbation + (1-coeff)*result.fChangedNormalLoc;
	}

	return MC_S_OK;
}

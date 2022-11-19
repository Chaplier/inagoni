/****************************************************************************************************

		GridShader4.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "GridShader4.h"

#include "Veloute.h"
#include "Utils.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"

const MCGUID CLSID_GridShader4(R_CLSID_GridShader4);
const MCGUID CLSID_GlobalGridShader4(R_CLSID_GlobalGridShader4);

PMapGridShader4::PMapGridShader4()
{
	fSeed=12345;

	fTileCount=10;

	fSectionSize=.15f;
	fSectionPlateau=0;
	fMiddleSlope='Opt1';
	fSidesSlope='Opt3';
	fBumpDepth=.1f;

	fType = 'Opt1';
	fStretch = 0;
	
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

GridShader4::GridShader4()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}
	
boolean GridShader4::IsEqualTo(I3DExShader* aShader)		// Use it to compare two GridShader4
{
	return (
		(fPMap.fType == ((GridShader4*)aShader)->fPMap.fType) &&
		(fPMap.fSeed == ((GridShader4*)aShader)->fPMap.fSeed) &&
		(fPMap.fTileCount == ((GridShader4*)aShader)->fPMap.fTileCount) &&
		(fPMap.fStretch == ((GridShader4*)aShader)->fPMap.fStretch) &&
		(fPMap.fSectionSize == ((GridShader4*)aShader)->fPMap.fSectionSize) &&
		(fPMap.fSectionPlateau == ((GridShader4*)aShader)->fPMap.fSectionPlateau) &&
		(fPMap.fBumpDepth == ((GridShader4*)aShader)->fPMap.fBumpDepth) &&
		(fPMap.fMiddleSlope == ((GridShader4*)aShader)->fPMap.fMiddleSlope) &&
		(fPMap.fSidesSlope == ((GridShader4*)aShader)->fPMap.fSidesSlope) &&
		(fPMap.fShadersComponents[0] == ((GridShader4*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((GridShader4*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((GridShader4*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((GridShader4*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr GridShader4::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput GridShader4::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr GridShader4::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr GridShader4::HandleEvent(MessageID message, IMFResponder* source, void* data)
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
real GridShader4::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real GridShader4::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
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
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

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

real GridShader4::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

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

real GridShader4::GetVector(TVector3& result, ShadingIn& shadingIn)
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
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

	// Now determine if the point is in a mortar area or a tile area
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);

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

void GridShader4::PreProcess()
{
	fPMap.mTransform.Get2DTransform( fEngine.f2DTransform );

	fEngine.PreProcess(fPMap);

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( fEngine.f2DTransform, 100,100);//(fLength), (fLength) );

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
GlobalGridShader4::GlobalGridShader4()
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

EShaderOutput GlobalGridShader4::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalGridShader4::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
	real32 value = (value1+value2+value3+value4)*.25;

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

/****************************************************************************************************

		TilingShader1.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "TilingShader1.h"

#include "math.h"
#include "Utils.h"
#include "MCRandom.h"
#include "Veloute.h"

const MCGUID CLSID_TilingShader1(R_CLSID_TilingShader1);
const MCGUID CLSID_GlobalTilingShader1(R_CLSID_GlobalTilingShader1);

static const real32 INV_SQRT10 =	0.31622776601683793320f; /* 1.0/sqrt(10)*/
static const real32 kSinAlpha = INV_SQRT10;
static const real32 kCosAlpha = 3*INV_SQRT10;

PMapTilingShader1::PMapTilingShader1()
{
	fTileCount=10;

	fMortarSize=.05f;
	fMortarPlateau=.5;
	fBottomSlope='Opt1';
	fTopSlope='Opt1';
	fMortarDepth=.1f;

	fTileSize=.5;
	fType = 'Opt1';

	fSmoothMortar=true;
	fProportionnalTileShading=true;
	fRandomUVOrigin=true;
	
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

TilingShader1::TilingShader1()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}
	
boolean TilingShader1::IsEqualTo(I3DExShader* aShader)		// Use it to compare two TilingShader1
{
	return (
		(fPMap.fType == ((TilingShader1*)aShader)->fPMap.fType) &&
		(fPMap.fTileCount == ((TilingShader1*)aShader)->fPMap.fTileCount) &&
		(fPMap.fTileSize == ((TilingShader1*)aShader)->fPMap.fTileSize) &&
		(fPMap.fProportionnalTileShading == ((TilingShader1*)aShader)->fPMap.fProportionnalTileShading) &&
		(fPMap.fRandomUVOrigin == ((TilingShader1*)aShader)->fPMap.fRandomUVOrigin) &&
		(fPMap.fMortarSize == ((TilingShader1*)aShader)->fPMap.fMortarSize) &&
		(fPMap.fMortarPlateau == ((TilingShader1*)aShader)->fPMap.fMortarPlateau) &&
		(fPMap.fMortarDepth == ((TilingShader1*)aShader)->fPMap.fMortarDepth) &&
		(fPMap.fBottomSlope == ((TilingShader1*)aShader)->fPMap.fBottomSlope) &&
		(fPMap.fTopSlope == ((TilingShader1*)aShader)->fPMap.fTopSlope) &&
		(fPMap.fSmoothMortar == ((TilingShader1*)aShader)->fPMap.fSmoothMortar) &&
		(fPMap.fShadersComponents[0] == ((TilingShader1*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((TilingShader1*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((TilingShader1*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((TilingShader1*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr TilingShader1::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput TilingShader1::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr TilingShader1::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x040000)
real TilingShader1::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real TilingShader1::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
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
	{	// we're at the bottom of the mortar
		fShaders.GetMortarColor(result, fullAreaDone, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileColor(result, fullAreaDone, tileShading,fEngine.fTileCenterUV);
	}
	else
	{	// we're between the 2: average
#if (VERSIONNUMBER >= 0x040000)
		TMCColorRGBA color1;
		TMCColorRGBA color2;
#else
		TMCColorRGB color1;
		TMCColorRGB color2;
#endif
		fShaders.GetMortarColor(color1, fullAreaDone, shadingIn);

		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileColor(color2, fullAreaDone, tileShading,fEngine.fTileCenterUV);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result.Interpolate(color1,color2,coeff);
	}

	return 1.0f;
}

real TilingShader1::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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

	//real32 value =  ComputeOneSample(uu, vv);

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a value depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileValue(result, fullArea, tileShading, 1/*fPMap.fMortarDepth*/);
	}
	else
	{	// we're between the 2: average
		real32 value1;
		fShaders.GetMortarValue(value1, fullArea, shadingIn);

		real32 value2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileValue(value2, fullArea, tileShading, 1/*fPMap.fMortarDepth*/);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result = (1-coeff)*value1 + coeff*value2;
	}

	return 1.0f;
}

real TilingShader1::GetVector(TVector3& result, ShadingIn& shadingIn)
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
	{	// we're at the bottom of the mortar
		fShaders.GetMortarVector(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileVector(result, tileShading);
	}
	else
	{	// we're between the 2: average
		// The perturbation due to the transition between tile and mortar
		GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);

		// add the perturbation contained in the shaders
		TVector3 vector1;
		fShaders.GetMortarVector(vector1, shadingIn);

		TVector3 vector2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileVector(vector2, tileShading);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result += (1-coeff)*vector1 + coeff*vector2;
	}

	return 1.0f;
}

void TilingShader1::PreProcess()
{
	fPMap.mTransform.Get2DTransform( fEngine.f2DTransform );

	fEngine.PreProcess(fPMap);

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( fEngine.f2DTransform, 100,100); //(fLength), (fLength) );

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
EShaderOutput GlobalTilingShader1::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalTilingShader1::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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
		fShaders.DoMortarShade(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile: define a UV space local to the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.DoTileShade(result, tileShading,fEngine.fTileCenterUV);
		shadingIn.fCurrentCompletionMask = tileShading.fCurrentCompletionMask;
	}
	else
	{	// we're between the 2: average
		ShadingOut out1 = result;		
		fShaders.DoMortarShade(out1, shadingIn);

		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.DoTileShade(result, tileShading,fEngine.fTileCenterUV);

		const real32 coeff = 1-value/fEngine.fMortarBumpDepth;

		// Color
		BlendColor(result.fColor, out1.fColor, coeff);
		// Specular color
		BlendColor(result.fSpecularColor, out1.fSpecularColor, coeff);
		// Specular size
		BlendValue(result.fSpecularSize, out1.fSpecularSize, coeff);
		// Reflexion
		BlendColor(result.fReflection.fReflection, out1.fReflection.fReflection, coeff);
		// Transparency
		BlendColor(result.fTransparency.fIntensity, out1.fTransparency.fIntensity, coeff);
		// Refraction
		BlendValue(result.fRefractiveIndex, out1.fRefractiveIndex, coeff);
		// Glow
		BlendColor(result.fGlow, out1.fGlow, coeff);
		// ChangedNormal
		TVector3 bumpPerturbation;
		GetPerturbationVector(bumpPerturbation, shadingIn, value1, value2, value3, value4);
		result.fChangedNormalLoc = bumpPerturbation +
							(1-coeff)*result.fChangedNormalLoc + coeff*out1.fChangedNormalLoc;
		// Opacity
#if (VERSIONNUMBER >= 0x040000)
		BlendValue(result.fLayerMask , out1.fLayerMask , coeff);
#else
		BlendValue(result.fOpacity, out1.fOpacity, coeff);
#endif	
	}

	return MC_S_OK;
}

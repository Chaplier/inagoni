/****************************************************************************************************

		TilingShader3.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "TilingShader3.h"

#include "math.h"
#include "Utils.h"
#include "Veloute.h"


const MCGUID CLSID_TilingShader3(R_CLSID_TilingShader3);
const MCGUID CLSID_GlobalTilingShader3(R_CLSID_GlobalTilingShader3);

PMapTilingShader3::PMapTilingShader3()
{
	fType='Opt1';
	fTileCount=10;
	fPeriod='Opt2';
	fAmplitude=.05f;
	fProportionnalTileShading=true;
	fRandomUVOrigin=true;

	fMortarSize=.05f;
	fMortarPlateau=.5;
	fBottomSlope='Opt1';
	fTopSlope='Opt1';
	fMortarDepth=.1f;
	fSmoothMortar=true;

	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

TilingShader3::TilingShader3()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}
	
boolean TilingShader3::IsEqualTo(I3DExShader* aShader)		// Use it to compare two TilingShader3
{
	return (
		(fPMap.fType == ((TilingShader3*)aShader)->fPMap.fType) &&
		(fPMap.fTileCount == ((TilingShader3*)aShader)->fPMap.fTileCount) &&
		(fPMap.fPeriod == ((TilingShader3*)aShader)->fPMap.fPeriod) &&
		(fPMap.fAmplitude == ((TilingShader3*)aShader)->fPMap.fAmplitude) &&
		(fPMap.fProportionnalTileShading == ((TilingShader3*)aShader)->fPMap.fProportionnalTileShading) &&
		(fPMap.fRandomUVOrigin == ((TilingShader3*)aShader)->fPMap.fRandomUVOrigin) &&
		(fPMap.fMortarSize == ((TilingShader3*)aShader)->fPMap.fMortarSize) &&
		(fPMap.fMortarPlateau == ((TilingShader3*)aShader)->fPMap.fMortarPlateau) &&
		(fPMap.fMortarDepth == ((TilingShader3*)aShader)->fPMap.fMortarDepth) &&
		(fPMap.fBottomSlope == ((TilingShader3*)aShader)->fPMap.fBottomSlope) &&
		(fPMap.fTopSlope == ((TilingShader3*)aShader)->fPMap.fTopSlope) &&
		(fPMap.fSmoothMortar == ((TilingShader3*)aShader)->fPMap.fSmoothMortar) &&
		(fPMap.fShadersComponents[0] == ((TilingShader3*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((TilingShader3*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((TilingShader3*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((TilingShader3*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr TilingShader3::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput TilingShader3::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr TilingShader3::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x040000)
real TilingShader3::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real TilingShader3::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
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
	boolean flip = false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a color depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarColor(result, fullAreaDone, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		fShaders.GetTileColor(color2, fullAreaDone, tileShading,fEngine.fTileCenterUV);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result.Interpolate(color1,color2,coeff);
	}

	return 1.0f;
}

real TilingShader3::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
	boolean flip = false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip );

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a value depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		fShaders.GetTileValue(result, fullArea, shadingIn, 1/*fPMap.fMortarDepth*/);
	}
	else
	{	// we're between the 2: average
		real32 value1;
		fShaders.GetMortarValue(value1, fullArea, shadingIn);

		real32 value2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		fShaders.GetTileValue(value2, fullArea, tileShading, 1/*fPMap.fMortarDepth*/);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result = (1-coeff)*value1 + coeff*value2;
	}

	return 1.0f;
}

real TilingShader3::GetVector(TVector3& result, ShadingIn& shadingIn)
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
	boolean flip = false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip);

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
	{	// we're at the bottom of the mortar
		fShaders.GetMortarVector(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		if(flip)
		{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
			tileShading.fUVx = flipX;
			tileShading.fUVy = flipY;
		}
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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		if(flip)
		{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
			tileShading.fUVx = flipX;
			tileShading.fUVy = flipY;
		}
		fShaders.GetTileVector(vector2, tileShading);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result += (1-coeff)*vector1 + coeff*vector2;
	}

	return 1.0f;
}

void TilingShader3::PreProcess()
{
	fPMap.mTransform.Get2DTransform( fEngine.f2DTransform );

	fEngine.PreProcess(fPMap);

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( fEngine.f2DTransform, 100,100);//(fSideLength), (fSideLength) );

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
EShaderOutput GlobalTilingShader3::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalTilingShader3::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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
	boolean flip = false;
	fEngine.GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV, flip);

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
		fShaders.DoMortarShade(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile: define a UV space local to the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		if(flip)
		{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
			tileShading.fUVx = flipX;
			tileShading.fUVy = flipY;
		}

		fShaders.DoTileShade(result, tileShading,fEngine.fTileCenterUV);
		shadingIn.fCurrentCompletionMask = tileShading.fCurrentCompletionMask;
	}
	else
	{	// we're between the 2: average
		ShadingOut out1 = result;		
		fShaders.DoMortarShade(out1, shadingIn);

		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu, iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv, iV);
		if(flip)
		{	// the tile is rotates by 90 degree, rotates the vecor before getting the 4 values arround the point
			tileShading.fUVx = flipX;
			tileShading.fUVy = flipY;
		}
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
		BlendValue(result.fLayerMask, out1.fLayerMask, coeff);
#else
		BlendValue(result.fOpacity, out1.fOpacity, coeff);
#endif	
	}

	return MC_S_OK;
}


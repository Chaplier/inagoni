/****************************************************************************************************

		TilingShader2.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "math.h"
#include "TilingShader2.h"
#include "Utils.h"
#include "Veloute.h"
#include "MCRandom.h"

const MCGUID CLSID_TilingShader2(R_CLSID_TilingShader2);
const MCGUID CLSID_GlobalTilingShader2(R_CLSID_GlobalTilingShader2);

PMapTilingShader2::PMapTilingShader2()
{
	fSeed = 23459023; // SHADERS_PLUS

	fType = 'Opt7';
	fHorizontal=20;
	fVertical=20;
	fTileProp=.5;
	fShifting=.5f;
	fSlope=.5f;
	fGapDepth=-.5f; // Autoblock tile option
	fGapInclination=.5f; // Autoblock tile option
	fThickness=.2f;
					
	fLMortar=0.05f;
	fHMortar=0.05f;
	fMortarPlateau=.5;
	fBottomSlope='Opt1';
	fTopSlope='Opt1';
	fMortarDepth=.1f;
					
	fSmoothMortar=true;
	fProportionnalTileShading=true;
	fRandomUVOrigin=true;
					
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

TilingShader2::TilingShader2()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;

	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShaders.fShaders[iShader]=NULL;
	}
}
	
boolean TilingShader2::IsEqualTo(I3DExShader* aShader)		// Use it to compare two TilingShader2
{
	return (
		(fPMap.fType == ((TilingShader2*)aShader)->fPMap.fType) &&
		(fPMap.fHorizontal == ((TilingShader2*)aShader)->fPMap.fHorizontal) &&
		(fPMap.fVertical == ((TilingShader2*)aShader)->fPMap.fVertical) &&
		(fPMap.fTileProp == ((TilingShader2*)aShader)->fPMap.fTileProp) &&
		(fPMap.fShifting == ((TilingShader2*)aShader)->fPMap.fShifting) &&
		(fPMap.fSlope == ((TilingShader2*)aShader)->fPMap.fSlope) &&
		(fPMap.fGapDepth == ((TilingShader2*)aShader)->fPMap.fGapDepth) &&
		(fPMap.fGapInclination == ((TilingShader2*)aShader)->fPMap.fGapInclination) &&
		(fPMap.fProportionnalTileShading == ((TilingShader2*)aShader)->fPMap.fProportionnalTileShading) &&
		(fPMap.fRandomUVOrigin == ((TilingShader2*)aShader)->fPMap.fRandomUVOrigin) &&
		(fPMap.fLMortar == ((TilingShader2*)aShader)->fPMap.fLMortar) &&
		(fPMap.fHMortar == ((TilingShader2*)aShader)->fPMap.fHMortar) &&
		(fPMap.fMortarPlateau == ((TilingShader2*)aShader)->fPMap.fMortarPlateau) &&
		(fPMap.fMortarDepth == ((TilingShader2*)aShader)->fPMap.fMortarDepth) &&
		(fPMap.fBottomSlope == ((TilingShader2*)aShader)->fPMap.fBottomSlope) &&
		(fPMap.fTopSlope == ((TilingShader2*)aShader)->fPMap.fTopSlope) &&
		(fPMap.fSmoothMortar == ((TilingShader2*)aShader)->fPMap.fSmoothMortar) &&
		(fPMap.fShadersComponents[0] == ((TilingShader2*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((TilingShader2*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((TilingShader2*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((TilingShader2*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr TilingShader2::GetShadingFlags(ShadingFlags& theFlags)
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

EShaderOutput TilingShader2::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr TilingShader2::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}


MCCOMErr TilingShader2::HandleEvent(MessageID message, IMFResponder* source, void* data)
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


real TilingShader2::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)

{
	if (IsLocked())
	{

		result.Set(0.0f, 0.0f, 0.0f, 0.0f);

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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
		fShaders.GetTileColor(result, fullAreaDone, tileShading, fEngine.fTileCenterUV);
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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
		fShaders.GetTileColor(color2, fullAreaDone, tileShading, fEngine.fTileCenterUV);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result.Interpolate(color1,color2,coeff);
	}

	return 1.0f;
}

real TilingShader2::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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

// 	real32 value = 0;ComputeOneSample(uu, vv);

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a value depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
		fShaders.GetTileValue(result, fullArea, tileShading,1/*fPMap.fMortarDepth*/);
	}
	else
	{	// we're between the 2: average
		real32 value1;
		fShaders.GetMortarValue(value1, fullArea, shadingIn);

		real32 value2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
		fShaders.GetTileValue(value2, fullArea, tileShading,1/*fPMap.fMortarDepth*/);

		const real32 coeff = value/fEngine.fMortarBumpDepth;
		result = (1-coeff)*value1 + coeff*value2;
	}

	return 1.0f;
}

real TilingShader2::GetVector(TVector3& result, ShadingIn& shadingIn)
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
	{	// we're at the bottom of the mortar
		fShaders.GetMortarVector(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
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

void TilingShader2::PreProcess()
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
EShaderOutput GlobalTilingShader2::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalTilingShader2::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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
		fShaders.DoMortarShade(result, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile: define a UV space local to the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
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
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU);
		tileShading.fUV.y = fEngine.GetLocalTileV(vv,iV);
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
		if(flip)
		{	// the tile is flipped, flip the vectors
			shadingIn.fUV.x *= -1;
		}
	}

	return MC_S_OK;
}

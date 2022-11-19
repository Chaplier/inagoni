/****************************************************************************************************

		TilingShader4.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "TilingShader4.h"

#include "Veloute.h"
#include "Utils.h"

const MCGUID CLSID_TilingShader4(R_CLSID_TilingShader4);
const MCGUID CLSID_GlobalTilingShader4(R_CLSID_GlobalTilingShader4);

PMapTilingShader4::PMapTilingShader4()
{
	fSeed=12345;

	fTileCount=10;

	fMortarSize=.05f;
	fMortarPlateau=.5;
	fBottomSlope='Opt1';
	fTopSlope='Opt1';
	fMortarDepth=.1f;

	fType = 'Opt1';
	fStretch = 0;

	fProportionnalTileShading=true;
	fRandomUVOrigin=true;
	
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

TilingShader4::TilingShader4()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;
}
	
boolean TilingShader4::IsEqualTo(I3DExShader* aShader)		// Use it to compare two TilingShader4
{
	return (
		(fPMap.fType == ((TilingShader4*)aShader)->fPMap.fType) &&
		(fPMap.fSeed == ((TilingShader4*)aShader)->fPMap.fSeed) &&
		(fPMap.fTileCount == ((TilingShader4*)aShader)->fPMap.fTileCount) &&
		(fPMap.fStretch == ((TilingShader4*)aShader)->fPMap.fStretch) &&
		(fPMap.fProportionnalTileShading == ((TilingShader4*)aShader)->fPMap.fProportionnalTileShading) &&
		(fPMap.fRandomUVOrigin == ((TilingShader4*)aShader)->fPMap.fRandomUVOrigin) &&
		(fPMap.fMortarSize == ((TilingShader4*)aShader)->fPMap.fMortarSize) &&
		(fPMap.fMortarPlateau == ((TilingShader4*)aShader)->fPMap.fMortarPlateau) &&
		(fPMap.fMortarDepth == ((TilingShader4*)aShader)->fPMap.fMortarDepth) &&
		(fPMap.fBottomSlope == ((TilingShader4*)aShader)->fPMap.fBottomSlope) &&
		(fPMap.fTopSlope == ((TilingShader4*)aShader)->fPMap.fTopSlope) &&
		(fPMap.fShadersComponents[0] == ((TilingShader4*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((TilingShader4*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((TilingShader4*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((TilingShader4*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr TilingShader4::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput TilingShader4::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr TilingShader4::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

MCCOMErr TilingShader4::HandleEvent(MessageID message, IMFResponder* source, void* data)
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
real TilingShader4::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real TilingShader4::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
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

real TilingShader4::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
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
	{	// we're at the bottom of the mortar
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else if(value>=fEngine.fMortarBumpDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = fEngine.GetLocalTileU(uu,iU,vv,iV);
		tileShading.fUV.y = fEngine.GetLocalTileV(uu,iU,vv,iV);
		fShaders.GetTileValue(result, fullArea, shadingIn, 1/*fPMap.fMortarDepth*/);
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

real TilingShader4::GetVector(TVector3& result, ShadingIn& shadingIn)
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

void TilingShader4::PreProcess()
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
EShaderOutput GlobalTilingShader4::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalTilingShader4::DoShade(ShadingOut& result,ShadingIn& shadingIn)
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

//	real32 value = ComputeOneSample(uu, vv);
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
	real32 value = (value1+value2+value3+value4)*.25;
//	value *= .5;

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
		BlendValue(result.fLayerMask, out1.fLayerMask, coeff);
#else
		BlendValue(result.fOpacity, out1.fOpacity, coeff);
#endif	
	}

	return MC_S_OK;
}

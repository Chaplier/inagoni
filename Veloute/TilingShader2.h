/****************************************************************************************************

		TilingShader2.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __TilingShader2__
#define __TilingShader2__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "Engine2.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"
// Define the TilingShader2 CLSID ( see the TilingShader2Def.h file to get R_CLSID_TilingShader2 value )

extern const MCGUID CLSID_TilingShader2;
extern const MCGUID CLSID_GlobalTilingShader2;

struct PMapTilingShader2
{
	PMapTilingShader2();

	T2DTransformBase mTransform;

	int32 fSeed; // for SHADER_PLUS
	int32 fType;
	int32 fHorizontal; // nb horizontal tiles
	int32 fVertical; // nb vertical tiles
	real32 fTileProp; // Rectangular tiles: second tile size (in % of the first)
	real32 fShifting; // Rectangular tiles: for the next row of tiles
	real32 fSlope; // Oblique tiles option
	real32 fGapDepth; // Autoblock tile option
	real32 fGapInclination; // Autoblock tile option
	real32 fThickness; // for option 11, 100% represent half the smallest length
	boolean fProportionnalTileShading;
	boolean fRandomUVOrigin;

	real32 fLMortar; // Length of the mortar area in % of the 1st length
	real32 fHMortar; // height of the mortar area in % of the height
	real32 fMortarPlateau; // Plateau size in %
	real32 fMortarDepth; // 
	int32 fBottomSlope; // to determine the smooth shape of the mortar
	int32 fTopSlope; // to determine the smooth shape of the mortar
	boolean fSmoothMortar;

	// First shader: tile shader
	// Second shader: mortar shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class TilingShader2 :	public ShaderBase,
						public TSuperSamplingBase
{
public :

	TilingShader2();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapTilingShader2));}
	MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	MCCOMErr		MCCOMAPI	HandleEvent				(MessageID message, IMFResponder* source, void* data);

	real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
	{ return fEngine.ComputeOneSample(uu, vv, samplingRate); }

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapTilingShader2 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	Engine2	fEngine;

	TThreeShaders	fShaders;
};

// A global version of the tile shader for the DoShade
class GlobalTilingShader2 : public TilingShader2
{
public :
	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};
                           
#endif // __TilingShader2__


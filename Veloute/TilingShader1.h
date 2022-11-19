/****************************************************************************************************

		TilingShader1.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __TilingShader1__
#define __TilingShader1__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "Engine1.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"
// Define the TileShader CLSID ( see the TileShaderDef.h file to get R_CLSID_TileShader value )

extern const MCGUID CLSID_TilingShader1;
extern const MCGUID CLSID_GlobalTilingShader1;

struct PMapTilingShader1
{
	PMapTilingShader1();

	T2DTransformBase mTransform;

	// Settings
	int32 fType;
	int32 fTileCount; // nb tile between u=0 and u=1
	real32 fTileSize; // size of the small square tile ( from 0% to 100% )
	boolean fProportionnalTileShading;
	boolean fRandomUVOrigin;

	// Mortar params
	real32 fMortarSize; // widthh of the mortar area in % of the size of a tile
	real32 fMortarPlateau; // Plateau size in %
	real32 fMortarDepth; // 
	int32 fBottomSlope; // to determine the smooth shape of the mortar
	int32 fTopSlope; // to determine the smooth shape of the mortar
	boolean	fSmoothMortar;

	// First shader: tile shader
	// Second shader: mortar shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class TilingShader1 :	public ShaderBase,
					public TSuperSamplingBase
{
public :

	TilingShader1();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapTilingShader1));}
	MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	void PreProcess();

	inline real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
	{ return fEngine.ComputeOneSample(uu, vv, samplingRate); }

	// PMap is the first field: we complete the T2DTransformBase pMap
	PMapTilingShader1 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	Engine1	fEngine;

	real32	fSuperSamplingQualityU;
	real32	fSuperSamplingQualityV;

	TThreeShaders	fShaders;
};


// A global version of the tile shader for the DoShade
class GlobalTilingShader1 : public TilingShader1
{
public :
	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};
                           






#endif

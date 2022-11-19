/****************************************************************************************************

		GridShader4.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __GridShader4__
#define __GridShader4__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "Engine4.h"
#include "VoronoiBase.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"
// Define the TileShader CLSID ( see the TileShaderDef.h file to get R_CLSID_TileShader value )

extern const MCGUID CLSID_GridShader4;
extern const MCGUID CLSID_GlobalGridShader4;

struct PMapGridShader4
{
	PMapGridShader4();

	T2DTransformBase mTransform;

	// Settings
	int32 fType;
	int32 fSeed;	// Seed is used for the Rock style
	int32 fTileCount; // nb tile between u=0 and u=1
	real32 fStretch; // from -1 to +1, used for the Rock style.

	// Mortar params
	real32 fSectionSize; // widthh of the mortar area in % of the size of a tile
	real32 fSectionPlateau; // Plateau size in %
	real32 fBumpDepth; // 
	int32 fMiddleSlope; // to determine the smooth shape of the mortar
	int32 fSidesSlope; // to determine the smooth shape of the mortar

	// First shader: tile shader
	// Second shader: mortar shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class GridShader4 :	public ShaderBase,
						public TSuperSamplingBase
{
public :

	GridShader4();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapGridShader4));}
	MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

	// TBasicDataExchanger functions
	virtual MCCOMErr 			MCCOMAPI HandleEvent			(MessageID message, IMFResponder* source, void* data);

protected:

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
	{ return fEngine.ComputeOneSample(uu, vv, samplingRate); }

	// PMap is the first field: we complete the T2DTransformBase pMap
	PMapGridShader4 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	Engine4	fEngine;

	TThreeShaders	fShaders;
};


// A global version of the tile shader for the DoShade
class GlobalGridShader4 : public GridShader4
{
public :
	GlobalGridShader4();

	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};

#endif

/****************************************************************************************************

		GridShader3.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __GridShader3__
#define __GridShader3__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "Engine3.h"
#include "NoiseBase.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"
// Define the TileShader CLSID ( see the TileShaderDef.h file to get R_CLSID_TileShader value )

extern const MCGUID CLSID_GridShader3;
extern const MCGUID CLSID_GlobalGridShader3;

struct PMapGridShader3
{
	PMapGridShader3();

	T2DTransformBase mTransform;

	int32 fType; // to choose between the 3 possibles arangments
	int32 fTileCount; // nb tile between u=0 and u=1
	int32 fPeriod; // ondulation periodicity
	real32 fAmplitude; // ondulation amplitude in %

	real32 fSectionSize; // widthh of the mortar area in % of the size of a tile
	real32 fSectionPlateau; // Plateau size in %
	real32 fBumpDepth; // 
	int32 fMiddleSlope; // to determine the smooth shape of the mortar
	int32 fSidesSlope; // to determine the smooth shape of the mortar
	boolean	fSmoothSection;

	// First shader: tile shader
	// Second shader: mortar shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class GridShader3 :	public ShaderBase,
					public TSuperSamplingBase
{
public :

	GridShader3();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapGridShader3));}
	MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
	{ return fEngine.ComputeOneSample(uu, vv, samplingRate); }

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapGridShader3 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	Engine3	fEngine;

	TThreeShaders	fShaders;
};


// A global version of the tile shader for the DoShade
class GlobalGridShader3 : public GridShader3
{
public :
	GlobalGridShader3();

	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};

#endif

/****************************************************************************************************

		GridShader2.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __GridShader2__
#define __GridShader2__

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
// Define the GridShader2 CLSID ( see the GridShader2Def.h file to get R_CLSID_GridShader2 value )

extern const MCGUID CLSID_GridShader2;
extern const MCGUID CLSID_GlobalGridShader2;

struct PMapGridShader2
{
	PMapGridShader2();

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

	real32 fLSection; // Length of the mortar area in % of the 1st length
	real32 fHSection; // height of the mortar area in % of the height
	real32 fSectionPlateau; // Plateau size in %
	real32 fBumpDepth; // 
	int32 fMiddleSlope; // to determine the smooth shape of the mortar
	int32 fSidesSlope; // to determine the smooth shape of the mortar
	boolean fSmoothSection;

	// First shader: tile shader
	// Second shader: mortar shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class GridShader2 :	public ShaderBase,
						public TSuperSamplingBase
{
public :

	GridShader2();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapGridShader2));}
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
	PMapGridShader2 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	Engine2	fEngine;

	TThreeShaders	fShaders;
};

// A global version of the tile shader for the DoShade
class GlobalGridShader2 : public GridShader2
{
public :
	GlobalGridShader2();

	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};
                      
#endif // __GridShader2__


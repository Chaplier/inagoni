/****************************************************************************************************

		RoofShader1.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/

#ifndef __RoofShader1__
#define __RoofShader1__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "IShComponent.h"
#include "copyright.h"

#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"

#include "NoiseBase.h"

extern const MCGUID CLSID_RoofShader1;
extern const MCGUID CLSID_GlobalRoofShader1;

struct PMapRoofShader1
{
	PMapRoofShader1();

	T2DTransformBase mTransform;

	int32 fSeed; // for SHADER_PLUS
	int32 fType; // to choose between the 3 possibles arangments
	int32 fHorTileCount; // nb tile between u=0 and u=1
	int32 fVerTileCount; // nb tile between v=0 and v=1
	real32 fGap; // gap beetween 2 tiles
	real32 fAmplitude; // for tiles with oscillation
	int32 fPeriod; // for tiles with oscillation
	int32 fShift; // for tiles with oscillation
	real32 fSelfShadow; // for tiles with oscillation
	real32 fProportion; // for the tiles with 1 ocillation
	boolean fProportionnalTileShading;
	boolean fRandomUVOrigin;

	real32 fHorShadow; // width of the shadow area in % of the size of a tile
	real32 fVerShadow; // width of the shadow area in % of the size of a tile
	real32 fShadowDepth; // 
	int32 fBottomSlope; // to determine the smooth shape of the shadow
	int32 fTopSlope; // to determine the smooth shape of the shadow
	boolean	fSmoothShadow;

	// First shader: tile shader
	// Second shader: shadow shader
	// Third shader: tile tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class RoofShader1 :	public ShaderBase,
					public TSuperSamplingBase
{
public :

	RoofShader1();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapRoofShader1));}
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
	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV ); // retrun a UV center on the hexagon

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);

	inline real32 GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV);
	inline real32 GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapRoofShader1 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	real32	fLength;
	real32	fWidth;

	real32	fWidthOnLength;

	real32	fActualGap;
	real32	fSlope;
	real32	fSmallSize;
	real32	fUFrequency;
	real32	fRealShifting;
	real32	fScaledProportion;

	real32	fAu;
	real32	fAv;
	real32	fBu;
	real32	fBv;
	real32	fR1;
	real32	fR2;
	real32	fMax;
	real32	fMin;

	// Random rectangular tile param
	boolean fFirstTile;
	real32	fLength1;
	real32	fLength2;
	real32	fHalfLength1;
	real32	fLength1PlusHalfLength2;
	real32	fLength1OnTotalLength;

	real32	fActualAmplitude;

	real32	fShadowDepth; // a rescaled value of the depth
	real32	fShadowBottomSlope;
	real32	fShadowTopSlope;

	real32	fSelfShadow;

	TVector2 fTileCenterUV;

	TThreeShaders	fShaders;

	TTransform2D	f2DTransform;

	TNoiseBase		fNoise; // for the random tile
};


// A global version of the roof shader for the DoShade
class GlobalRoofShader1 : public RoofShader1
{
public :
	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};
                           






#endif

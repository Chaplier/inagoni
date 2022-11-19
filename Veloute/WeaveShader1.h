/****************************************************************************************************

		WeaveShader1.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/4/2005

****************************************************************************************************/

#ifndef __WeaveShader1__
#define __WeaveShader1__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "ShaderBase.h"

#include "2DTransformBase.h"
#include "SuperSamplingBase.h"
#include "SubShadersUtils.h"
#include "IShComponent.h"


extern const MCGUID CLSID_WeaveShader1;
extern const MCGUID CLSID_GlobalWeaveShader1;

struct PMapWeaveShader1
{
	PMapWeaveShader1();

	T2DTransformBase mTransform;

	// Settings
	int32 fType;
	int32 fShape; // S or Z type
	int32 fWeaveCountU; // nb weave between u=0 and u=1
	int32 fWeaveCountV; // nb weave between v=0 and v=1
	real32 fSpacingU; // spacing in %
	real32 fSpacingV; // spacing in %
	real32 fFlatArea; // flat area in % (when 0, the thread is circular, when 1, the thread is flat)
	real32 fBumpAmplitude;

	// First shader: weave shader
	// Second shader: background shader
	// Third shader: weave tinting
	TMCCountedPtr<IShParameterComponent> fShadersComponents[3];
};

class WeaveShader1 :	public ShaderBase,
						public TSuperSamplingBase
{
public :

	WeaveShader1();

	STANDARD_RELEASE;
  
	boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapWeaveShader1));}
	MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean	MCCOMAPI	WantsTransform			(){ return false; }
	real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:


	void PreProcess();
	void GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV ); // retrun a UV center on the hexagon

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplinRate);

	// PMap is the first field: we complete the T2DTransformBase pMap
	PMapWeaveShader1 fPMap;
	// PMap end here

	boolean	fPreprocessed;

	real32	fULength;
	real32	fVLength;
	real32	fMinLocalU; // in a 1*1 space, to limit the thread in U
	real32	fMinLocalV; // in a 1*1 space, to limit the thread in U
	real32	fAmplitudeOffset;

	int32	fiU;
	int32	fiV;

	TVector2 fThreadOriginUV;

	TThreeShaders	fShaders;

	TTransform2D	f2DTransform;
};


// A global version of the tile shader for the DoShade
class GlobalWeaveShader1 : public WeaveShader1
{
public :
	GlobalWeaveShader1();

	STANDARD_RELEASE;

	MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	EShaderOutput	MCCOMAPI	GetImplementedOutput	();
};

#endif

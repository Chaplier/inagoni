/****************************************************************************************************

		Gradient2DShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __Gradient2DShader__
#define __Gradient2DShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"

#include "GrayScaleBase.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"

extern const MCGUID CLSID_Gradient2DShader;

struct PMapGradient2DShader
{
	PMapGradient2DShader();

	void Init();

	GrayScaleBasePMap mGrayScale;
	T2DTransformBase mTransform;

	int32	fType; // linear, circular, ...
	real32	fRepeat;
	boolean fSmooth; // linear or smooth interpolation
	boolean	fAA;
};

class Gradient2DShader :	public TSuperSamplingBase,
							public GrayScaleBase
{
public :

	Gradient2DShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapGradient2DShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapGradient2DShader fPMap;
	// PMap end here

	// Cache data
	real32 fRepeatCoeff;

	boolean fPreprocessed;

	TTransform2D	f2DTransform;
};









#endif

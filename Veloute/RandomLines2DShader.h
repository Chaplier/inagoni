/****************************************************************************************************

		RandomLines2DShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __RandomLines2DShader__
#define __RandomLines2DShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "BasicShader.h"
#include "I3DShShader.h"

#include "GrayScaleBase.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"

#include "NoiseBase.h"

extern const MCGUID CLSID_RandomLines2DShader;

struct PMapRandomLines2DShader
{
	PMapRandomLines2DShader();

	void Init();

	GrayScaleBasePMap mGrayScale;
	T2DTransformBase mTransform;

	int32	fType; // X axis, Y, ...
	int32	fSeed;
	real32	fDisturbance;
	real32	fCount;
	boolean	fAA;
};

class RandomLines2DShader :	public TSuperSamplingBase,
							public GrayScaleBase
{
public :

	RandomLines2DShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapRandomLines2DShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	HandleEvent				(MessageID message, IMFResponder* source, void* data);
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapRandomLines2DShader fPMap;
	// PMap end here

	boolean fPreprocessed;

	// Cached data
	real32	fDisturb;
	real32	fPeriod;

	TTransform2D	f2DTransform;

	TNoiseBase		fNoise;
};

#endif

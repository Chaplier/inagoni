/****************************************************************************************************

		RandomLines3DShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __RandomLines3DShader__
#define __RandomLines3DShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "BasicShader.h"
#include "I3DShShader.h"

#include "GrayScaleBase.h"
#include "3DTransformBase.h"
#include "SuperSamplingBase.h"

#include "NoiseBase.h"

extern const MCGUID CLSID_RandomLines3DShader;

struct PMapRandomLines3DShader
{
	PMapRandomLines3DShader();
	
	void Init();

	GrayScaleBasePMap mGrayScale;
	T3DTransformBase mTransform;

	int32	fType; // X axis, Y, ...
	int32	fSeed;
	real32	fDisturbance;
	real32	fCount;
	boolean	fAA;
};

class RandomLines3DShader :	public TSuperSamplingBase,
							public GrayScaleBase
{
public :

	RandomLines3DShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapRandomLines3DShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual MCCOMErr		MCCOMAPI	HandleEvent				(MessageID message, IMFResponder* source, void* data);
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);
	real32 ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);

	real32 GetIrisValue(const real32 u, const real32 v);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapRandomLines3DShader fPMap;
	// PMap end here

	boolean fPreprocessed;

	// Cached data
	real32	fDisturb;
	real32	fPeriod;

	TTransform3D	f3DTransform;

	TNoiseBase		fNoise;
};

#endif

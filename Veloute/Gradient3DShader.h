/****************************************************************************************************

		Gradient3DShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __Gradient3DShader__
#define __Gradient3DShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"

#include "GrayScaleBase.h"
#include "3DTransformBase.h"
#include "SuperSamplingBase.h"

extern const MCGUID CLSID_Gradient3DShader;

struct PMapGradient3DShader
{
	PMapGradient3DShader();

	void Init();

	GrayScaleBasePMap mGrayScale;
	T3DTransformBase mTransform;

	int32	fType; // linear, circular, ...
	real32	fRepeat;
	boolean fSmooth; // linear or smooth interpolation
	boolean	fAA;
};

class Gradient3DShader :	public TSuperSamplingBase,
							public GrayScaleBase
{
public :

	Gradient3DShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapGradient3DShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);
	real32 ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapGradient3DShader fPMap;
	// PMap end here

	boolean fPreprocessed;

	TTransform3D	f3DTransform;
};









#endif

/****************************************************************************************************

		FilterShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __FilterShader__
#define __FilterShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCCountedPtr.h"
#include "IShComponent.h"
#include "copyright.h"

#include "GrayScaleBase.h"
#include "SuperSamplingBase.h"

extern const MCGUID CLSID_FilterShader;

struct PMapFilterShader
{
	PMapFilterShader();

	void Init();

	GrayScaleBasePMap mGrayScale;
	TMCCountedPtr<IShParameterComponent> fSubShaderComponent;
};

class FilterShader :	public TSuperSamplingBase,
						public GrayScaleBase
{
public :

	FilterShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapFilterShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate);
	real32 ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);
	real32 ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapFilterShader fPMap;
	// PMap end here

	// A local copy of the shading infos
	ShadingIn fShadingIn;
	boolean fUsesUVs;
	boolean fUsesLoc;
	boolean fUsesGlo;

	boolean fPreprocessed;
	ShadingFlags fShadingFlags;

	TMCCountedPtr<I3DShShader> fSubShader;
};









#endif

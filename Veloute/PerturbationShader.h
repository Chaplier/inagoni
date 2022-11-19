/****************************************************************************************************

		PerturbationShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __PerturbationShader__
#define __PerturbationShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"
#include "MCCountedPtr.h"
#include "IShComponent.h"
#include "copyright.h"

//#include "GrayScaleBase.h"
#include "SuperSamplingBase.h"

extern const MCGUID CLSID_PerturbationShader;

static const int32 kScalarMethod = 'Scal';
static const int32 kVectorielMethod = 'Vect';

struct PMapPerturbationShader
{
	PMapPerturbationShader();

	// Basic version param
	real32	fAmplitude;
	real32	fAmplitudeX;
	real32	fAmplitudeY;
	real32	fAmplitudeZ;
	int32	fMethod;

	TMCCountedPtr<IShParameterComponent> fTextureShaderComponent;
	TMCCountedPtr<IShParameterComponent> fPerturbShaderComponent;
};

class PerturbationShader :	public TSuperSamplingBase, // to access the methods for the Vectoriel case
						 	public ShaderBase
{
public :

	PerturbationShader();

	STANDARD_RELEASE;

	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return (sizeof(PMapPerturbationShader));}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual MCCOMErr		MCCOMAPI	DoShade					(ShadingOut& result,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	virtual real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	void PreProcess();

	void AddPerturbation(ShadingIn& newShading, ShadingIn& oldShading);

	// For the vectoriel method
	virtual real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate);
	virtual real32 ComputeOneGlobalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);
	virtual real32 ComputeOneLocalSample(const real32 x, const real32 y, const real32 z, const real32 samplingRate);

	// PMap is the first field: we complete the GrayScaleBasePmap
	PMapPerturbationShader fPMap;
	// PMap end here

	boolean fPreprocessed;

	real32	fXCoeff;
	real32	fYCoeff;
	real32	fZCoeff;

	ShadingFlags fShadingFlags;

	ShadingIn	fShadingIn;

	TMCCountedPtr<I3DShShader> fTextureShader;
	TMCCountedPtr<I3DShShader> fPerturbShader;
};









#endif

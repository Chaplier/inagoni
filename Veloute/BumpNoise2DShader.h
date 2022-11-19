/****************************************************************************************************

		BumpNoise2DShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __BumpNoise2DShader__
#define __BumpNoise2DShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "ShaderBase.h"
#include "I3DShShader.h"

#include "NoiseBase.h"
#include "VoronoiBase.h"
#include "GrayScaleBase.h"
#include "2DTransformBase.h"
#include "SuperSamplingBase.h"

extern const MCGUID CLSID_BumpNoise2DShader;

struct PMapBumpNoise2DShader
{
	PMapBumpNoise2DShader();

	void Init();

	GrayScaleBasePMap mGrayScale;
	T2DTransformBase mTransform;

	int32 fSeed;
	int32 fType; // Linear, Smooth, VerySmooth
	real32 fParam1;
	real32 fParam2;
	boolean fAA;
};

class BumpNoise2DShader :	public TSuperSamplingBase,
							public GrayScaleBase
{
public :

	BumpNoise2DShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	InitComponent() { fPMap.Init(); return MC_S_OK; }
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(PMapBumpNoise2DShader);}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);
	// TBasicDataExchanger functions
	virtual MCCOMErr 			MCCOMAPI HandleEvent			(MessageID message, IMFResponder* source, void* data);

protected:

	virtual void ResearchMinMax() {ResearchMinMaxImp(fPMap.mGrayScale);}

	void PreProcess();

	real32 ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate);

	//
	real32 F1(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 0
	real32 F2(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 1
	real32 Puzzle(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 5
	real32 PuzzleBis(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 6
	real32 Average(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 7
	real32 Product(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 8
	real32 PureAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 9
	real32 HoleAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 10
	real32 SoftAma(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); // 11
	real32 RegularPattern(const real32 uu, const real32 vv, const int32 distanceMethod, const real32 samplingRate); //12

	// PMap start here
	PMapBumpNoise2DShader fPMap;
	// PMap end here

	boolean fPreprocessed;

	real32	fRoughness;

	TTransform2D	f2DTransform;
	TNoiseBase		fNoise;
	TVoronoiBase	fVoronoi;

};









#endif

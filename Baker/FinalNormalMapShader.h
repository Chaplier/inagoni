/****************************************************************************************************

		FinalNormalMapShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#ifndef __FinalNormalMapShader__
#define __FinalNormalMapShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "BasicShader.h"
#include "MCCountedPtr.h"
#include "IShComponent.h"
#include "I3DShShader.h"

extern const MCGUID CLSID_FinalNormalMapShader;

enum ENormalFlags
{
	eClearFlags =	0x00000000,
	eXRed =			0x00000001,
	eXGreen =		0x00000002,
	eXBlue =		0x00000004,
	eYRed =			0x00000008,
	eYGreen =		0x00000010,
	eYBlue =		0x00000020,
// Not used: if X and Y are known, this one too	eZRed =			0x00000040,
// Not used: if X and Y are known, this one too	eZGreen =		0x00000080,
// Not used: if X and Y are known, this one too	eZBlue =		0x00000100,
	eNegX =			0x00000200,
	eNegY =			0x00000400,
	eNegZ =			0x00000800,
	eIsRelative =	0x00001000,
};

static const int32 kBakerPreset = eXRed+eYGreen;
static const int32 kBakerRelativePreset = eXRed+eYGreen+eIsRelative;
static const int32 kZBrush1Preset = eXRed+eYBlue+eNegX;
static const int32 kZBrush2Preset = eXRed+eYBlue+eNegX+eNegY;

struct PMapFinalNormalMapShader
{
	PMapFinalNormalMapShader();

	TMCCountedPtr<IShParameterComponent>	fSubShaderComponent;
	boolean									fFlip;
	int32									fType;

	inline void			ClearFlag(const int32 flag)	{fType&=~flag;}
	inline void			SetFlag(const int32 flag)	{fType|=flag;}
	inline boolean		Flag(const int32 flag)	const	{return ((fType&flag) != 0 );}
};

// Helper fonction
static const int32 kRedID = 'Red ';
static const int32 kGreenID = 'Gree';
static const int32 kBlueID = 'Blue';
inline int32 GetZColor(const int32 flags)
{
	if(flags&eXRed)
	{
		if(flags&eYGreen) return kBlueID;
		else return kGreenID;
	}
	else if(flags&eXGreen)
	{
		if(flags&eYRed) return kBlueID;
		else return kRedID;
	}
	else
	{
		if(flags&eYRed) return kGreenID;
		else return kRedID;
	}
}

class FinalNormalMapShader :	public TBasicShader
{
public :

	FinalNormalMapShader();

	STANDARD_RELEASE;

	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(PMapFinalNormalMapShader);}
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual real			MCCOMAPI	GetValue				(real& result,boolean& fullArea,ShadingIn& shadingIn);
#if (VERSIONNUMBER >= 0x040000)
	virtual real			MCCOMAPI	GetColor				(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn);
#else
	virtual real			MCCOMAPI	GetColor				(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn);
#endif
	virtual real			MCCOMAPI	GetVector				(TVector3& result, ShadingIn& shadingIn);

protected:

	void				PreProcess();

	void				BackwardCompatibility();

	PMapFinalNormalMapShader	fPMap;

	int32	fRedAxis;
	int32	fGreenAxis;
	int32	fBlueAxis;

	boolean						fPreprocessed;
	TMCCountedPtr<I3DShShader>	fSubShader;

	// Serial number warning
	static boolean				gFirstTime;
};









#endif

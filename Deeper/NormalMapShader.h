/****************************************************************************************************

		NormalMapShader.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#ifndef __NormalMapShader__
#define __NormalMapShader__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "BasicShader.h"
#include "MCCountedPtr.h"
#include "IShComponent.h"
#include "I3DShShader.h"

extern const MCGUID CLSID_NormalMapShader;

struct PMapNormalMapShader
{
	PMapNormalMapShader();

	TMCCountedPtr<IShParameterComponent>	fSubShaderComponent;
	boolean									fFlip;
};

class NormalMapShader :	public TBasicShader
{
public :

	NormalMapShader();

	STANDARD_RELEASE;

	virtual MCCOMErr		MCCOMAPI	HandleEvent				(MessageID message, IMFResponder* source, void* data);
	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(PMapNormalMapShader);}
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

	PMapNormalMapShader	fPMap;

	boolean						fPreprocessed;
	TMCCountedPtr<I3DShShader>	fSubShader;
};









#endif

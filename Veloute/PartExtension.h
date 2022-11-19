/****************************************************************************************************

		PartExtension.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	2/9/2004

****************************************************************************************************/

#ifndef __PartExtension__
#define __PartExtension__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"

#include "BasicMCFCOMImplementations.h"

extern const MCGUID CLSID_TParamChooserPart1;
extern const MCGUID CLSID_TParamChooserPart2;
extern const MCGUID CLSID_TParamChooserPart3;

// Part that show or hides parameter depending on the current type of shading
class TParamChooserPart : public TBasicPart
{
public :
	TParamChooserPart();
	virtual ~TParamChooserPart();

	STANDARD_RELEASE;

	virtual void	MCCOMAPI	SelfPrepareToDestroy();
	virtual void	MCCOMAPI	FinishCreateFromResource();

	virtual boolean	MCCOMAPI	Receive(int32 message, IMFResponder* source, void* data);
// Can't use this one, there's a bug in Carrara. We use SelfDraw instead 
// virtual void	MCCOMAPI	SetShown(boolean inShown);
	
	virtual void	MCCOMAPI	SelfDraw(IMCGraphicContext* graphicContext, const TMCRect& inZone);
private :

	virtual void SetOption( const int32 selection ) = 0;

	TMCCountedPtr<IMFPart>		fTypePart;
	int32						fCurrentOption;
};

// For TilingShader1
class TParamChooserPart1 : public TParamChooserPart
{
public :
	STANDARD_RELEASE;

	virtual MCErr	MCCOMAPI	QueryInterface	(const MCIID& riid, void** ppvObj);
private :

	void SetOption( const int32 selection );
};

// For TilingShader2
class TParamChooserPart2 : public TParamChooserPart
{
public :
	STANDARD_RELEASE;

	virtual MCErr	MCCOMAPI	QueryInterface	(const MCIID& riid, void** ppvObj);
private :

	void SetOption( const int32 selection );
};

// For Roof Shader
class TParamChooserPart3 : public TParamChooserPart
{
public :
	STANDARD_RELEASE;

	virtual MCErr	MCCOMAPI	QueryInterface	(const MCIID& riid, void** ppvObj);
private :

	void SetOption( const int32 selection );
};


extern const MCGUID CLSID_TParamChooserPart4;

// For Weave Shader
class TParamChooserPart4 : public TParamChooserPart
{
public :
	STANDARD_RELEASE;

	virtual MCErr	MCCOMAPI	QueryInterface	(const MCIID& riid, void** ppvObj);
private :

	void SetOption( const int32 selection );
};

#endif

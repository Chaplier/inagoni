/****************************************************************************************************

		GrayScaleBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __GrayScaleBase__
#define __GrayScaleBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "MCGradient.h"
#include "ShaderBase.h"
#include "Utils.h"
// Basic implementation of a shader based on a grayscale:
// contains: brightness, contrast, min, max, ...

struct GrayScaleBasePMap
{
	real32			fGain;
	real32			fBias;
	TVector2		fMinMax;
 
	boolean			fInvert;

	// With x64, the pmap size need to be a multiple of 64 to stack the pmap properly in several diferent class
	//real32			fDummy;

	GrayScaleBasePMap();

	void Init();
	boolean IsEqualTo(const GrayScaleBasePMap& other) const;
};

class GrayScaleBase : public ShaderBase
{
public :

	GrayScaleBase();

	STANDARD_RELEASE;

	//virtual MCCOMErr		MCCOMAPI	InitComponent();
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	
	virtual boolean			MCCOMAPI	WantsTransform(){ return false; }
	
	// TBasicDataExchanger functions
	MCCOMErr 				MCCOMAPI HandleEvent			(MessageID message, IMFResponder* source, void* data);

protected:

	// Call only once
	void PreProcess( const GrayScaleBasePMap& pMap);
	// Call for each value
	void PostProcess(real32& value,  const GrayScaleBasePMap& pMap);

	virtual void ResearchMinMax() = 0;

	void ResearchMinMaxImp( GrayScaleBasePMap& pMap);

	real32			fAmplitude;
	real32			fOffset;
	real32			fRealGain; // = fGain but avoid 0 and 1 values
	real32			fRealBias; // = fBias but avoid 0 and 1 values
};





#endif

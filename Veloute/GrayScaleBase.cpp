/****************************************************************************************************

		GrayScaleBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#include "GrayScaleBase.h"
#include "MCClassArray.h"

#include "MCRandom.h"
#include "SuperSamplingBase.h" // just for gCannotSuperSample

GrayScaleBasePMap::GrayScaleBasePMap()
{
	Init();
}

void GrayScaleBasePMap::Init()
{
	fGain = .5;
	fBias = .5;
	fMinMax.x = 0;
	fMinMax.y = 1;

	fInvert = false;
}

boolean GrayScaleBasePMap::IsEqualTo(const GrayScaleBasePMap& other) const		// Use it to compare two GrayScaleBase
{
	return (
		(fGain == other.fGain) &&
		(fBias == other.fBias) &&
		(fMinMax ==other.fMinMax) &&
		(fInvert == other.fInvert) );
}

GrayScaleBase::GrayScaleBase()
{
}

EShaderOutput GrayScaleBase::GetImplementedOutput()
{
	return EShaderOutput(kUsesGetValue);
}

MCCOMErr GrayScaleBase::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (source->GetInstanceID() == 'Sear')
	{
		ResearchMinMax();
		return MC_S_OK;
	}
	return ShaderBase::HandleEvent(message, source, data);
}

void GrayScaleBase::PreProcess( const GrayScaleBasePMap& pMap)
{
	fAmplitude = pMap.fMinMax.y-pMap.fMinMax.x;
	fOffset = (pMap.fMinMax.y+pMap.fMinMax.x)*.5;
	fRealGain = Clamp( 0.0001f, 0.9999f, pMap.fGain );
	fRealBias = Clamp( 0.0001f, 0.9999f, pMap.fBias );
}

// value is between 0 and 1
void GrayScaleBase::PostProcess(real32& value,  const GrayScaleBasePMap& pMap)
{
	// Modify the result value. 

	// Min/Max
	value = (value-.5)*fAmplitude + fOffset;
	// Clamp
	value = Clamp(kRealZero,kRealOne,value);
	// Gain == contrast
	if(fRealGain!=.5) value = Gain( fRealGain, value);
	// Bias == brightness
	if(fRealBias!=.5) value = Bias( fRealBias, value);

	// Invertion
	if(pMap.fInvert) value = kRealOne-value;
}

void GrayScaleBase::ResearchMinMaxImp(  GrayScaleBasePMap& pMap)
{
	// Get randomly some samples to evaluate the min and the max of the map
	real32 min = kRealBig;
	real32 max = -kRealBig;

	real32 prevGain = pMap.fGain;
	real32 prevBias = pMap.fBias;
	pMap.fGain = .5;
	pMap.fBias = .5;
	pMap.fMinMax.x = 0;
	pMap.fMinMax.y = 1;
	PreProcess(pMap);

	const int32 precision = 16000;
	for(int32 i=0 ; i<precision ; i++)
	{
		const real32 randomX = FixedToReal(MCRandom()&0xFFFF);
		const real32 randomY = FixedToReal(MCRandom()&0xFFFF);
		const real32 randomZ = FixedToReal(MCRandom()&0xFFFF);
		
		ShadingIn fakeShading;

		fakeShading.fUV.x = randomX;
		fakeShading.fUV.y = randomY;
		
		fakeShading.fPointLoc.x = randomX*1234;
		fakeShading.fPointLoc.y = randomY*1234;
		fakeShading.fPointLoc.z = randomZ*1234;
		
		fakeShading.fPoint.x = randomX*1234;
		fakeShading.fPoint.y = randomY*1234;
		fakeShading.fPoint.z = randomZ*1234;

		// Disable anti aliasing
		fakeShading.fCurrentCompletionMask |= gCannotSuperSample;

		real32 result=0;
		boolean fullArea = true;
		GetValue(result, fullArea, fakeShading);

		if(result<min)
			min=result;
		if(result>max)
			max=result;
	}

	real32 amplitude = max-min;
	// error margin
// Not really necessary
//	const real32 error = amplitude/100;
//	min+=error;
//	max-=error;
//	amplitude -= 2*error;

	pMap.fMinMax.x = -min/amplitude ;
	pMap.fMinMax.y = (1-min)/amplitude;

	pMap.fGain = prevGain;
	pMap.fBias = prevBias;
}

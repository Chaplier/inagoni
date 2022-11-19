/****************************************************************************************************

		Modifier.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/26/2004

****************************************************************************************************/

#include "copyright.h"
#include "Modifier.h"
#include "GasDef.h"

#include "IMFResponder.h"
#include "MCCountedPtrHelper.h"
#include "MCRandom.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_VolumeModifier(R_CLSID_VolumeModifier);
const MCGUID CLSID_Taper(R_CLSID_Taper);
const MCGUID CLSID_Wave(R_CLSID_Wave);
const MCGUID CLSID_Bulge(R_CLSID_Bulge);
const MCGUID CLSID_Punch(R_CLSID_Punch);
const MCGUID CLSID_Twist(R_CLSID_Twist);
const MCGUID CLSID_Bend(R_CLSID_Bend);
const MCGUID CLSID_Shift(R_CLSID_Shift);
const MCGUID CLSID_Noise(R_CLSID_Noise);
const MCGUID CLSID_Stretch(R_CLSID_Stretch);
const MCGUID CLSID_Scale(R_CLSID_Scale);
#else
const MCGUID CLSID_VolumeModifier={R_CLSID_VolumeModifier};
const MCGUID CLSID_Taper={R_CLSID_Taper};
const MCGUID CLSID_Wave={R_CLSID_Wave};
const MCGUID CLSID_Bulge={R_CLSID_Bulge};
const MCGUID CLSID_Punch={R_CLSID_Punch};
const MCGUID CLSID_Twist={R_CLSID_Twist};
const MCGUID CLSID_Bend={R_CLSID_Bend};
const MCGUID CLSID_Shift={R_CLSID_Shift};
const MCGUID CLSID_Noise={R_CLSID_Noise};
const MCGUID CLSID_Stretch={R_CLSID_Stretch};
const MCGUID CLSID_Scale={R_CLSID_Scale};
#endif

static const int32 kXAxis = 'XAXI';
static const int32 kYAxis = 'YAXI';
static const int32 kZAxis = 'ZAXI';

////////////////////////////////////////////////////////////////////
//
// Basic
//
BasicVolumeModifier::BasicVolumeModifier()
{
	fInvalid = true;
}

MCCOMErr BasicVolumeModifier::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualCLSID(riid, CLSID_VolumeModifier))
	{
		TMCCountedGetHelper<BasicVolumeModifier> result(ppvObj);
		result = static_cast<BasicVolumeModifier*>(this);
		return MC_S_OK;
	}
	return TBasicDataComponent::QueryInterface(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////
//
// Taper
//
TaperModifier::TaperModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;

	// PMap
	fPMap.fStrengh = .5f;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
	fPMap.fFlip = false;
	fPMap.fInWidth = true;
	fPMap.fInLength = true;
}

void TaperModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = 2*fPMap.fLimit[0] - 1;
	fMax = 2*fPMap.fLimit[1] - 1;
}

void TaperModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	const real32 axisVal = fPMap.fFlip?-point[fAxisIndex]:point[fAxisIndex];

	const real32 coeff = ( fPMap.fStrengh*SmoothStep(fMin,fMax,axisVal)  + (1-fPMap.fStrengh));	
	if(coeff>kRealEpsilon)
	{
		if(fPMap.fInWidth) result[fU]/=coeff;	
		if(fPMap.fInLength) result[fV]/=coeff;	
	}
	else
	{
		if(fPMap.fInWidth) result[fU]=kRealBig;	
		if(fPMap.fInLength) result[fV]=kRealBig;	
	}
}

////////////////////////////////////////////////////////////////////
//
// Wave
//
static const int32 kPlanarAWave = 'Opt1';
static const int32 kPlanarBWave = 'Opt2';
static const int32 kRadialWave = 'Opt3';
static const int32 kAxialAWave = 'Opt4';

WaveModifier::WaveModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;
	fCoeff1 = 0;
	fCoeff2 = 0;
	fCoeff3 = 0;

	// PMap
	fPMap.fStrengh = .5f;
	fPMap.fWaveCount = 1;
	fPMap.fPhase = 0;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
	fPMap.fType = kAxialAWave;
}

void WaveModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}

	switch(fPMap.fType)
	{
	case kPlanarAWave:
	case kPlanarBWave:
	case kAxialAWave:
		{
			fMin = 2*fPMap.fLimit[0]-1;
			fMax = 2*fPMap.fLimit[1]-1;
		} break;
	case kRadialWave:
		{
			fMin = fPMap.fLimit[0];
			fMax = fPMap.fLimit[1];
		} break;
	}

	fCoeff1 = PI*fPMap.fWaveCount/(.5*(fMax-fMin));
	fCoeff2 = fPMap.fPhase*PI;
	fCoeff3 = fPMap.fStrengh*.5;
}

void WaveModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	// Note: the bbox is always the same 2x2x2 box.
	// And the transform is the opposite from what's visible on screen
	switch(fPMap.fType)
	{
	case kPlanarAWave:
	case kPlanarBWave:
	case kRadialWave:
		{	// oscillate in the fU or fV, and fAxisIndex plane
			real32 dist = 0;
			if(fPMap.fType==kPlanarAWave)
				dist = point[fV];
			else if(fPMap.fType==kPlanarBWave)
				dist = point[fU];
			else if(fPMap.fType==kRadialWave)
				dist = RealSqrt(point[fU]*point[fU]+point[fV]*point[fV]);

			real32 coeff = 0;
			if(dist<fMin)
			{
				coeff = ( fCoeff3*(1+RealSin( fMin*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}
			else if(dist>fMax)
			{
				coeff = ( fCoeff3*(1+RealSin( fMax*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}
			else
			{
				coeff = ( fCoeff3*(1+RealSin( dist*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}

			if(coeff>kRealEpsilon)
			{
				result[fAxisIndex]/=coeff;	
			}
			else
			{
				result[fAxisIndex]=kRealBig;	
			}
		} break;
	case kAxialAWave:
		{	// scale away from fAxisIndex
			const real32 axisVal = point[fAxisIndex];

			// val from 0 to 1, between fMin and fMax.
			real32 coeff = 0;
			if(axisVal<fMin)
			{
				coeff = ( fCoeff3*(1+RealSin( fMin*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}
			else if(axisVal>fMax)
			{
				coeff = ( fCoeff3*(1+RealSin( fMax*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}
			else
			{
				coeff = ( fCoeff3*(1+RealSin( axisVal*fCoeff1 + fCoeff2))  + (1-fPMap.fStrengh));	
			}

			if(coeff>kRealEpsilon)
			{
				result[fU]/=coeff;	
				result[fV]/=coeff;	
			}
			else
			{
				result[fU]=kRealBig;	
				result[fV]=kRealBig;	
			}
		} break;
	}
}

////////////////////////////////////////////////////////////////////
//
// Bulge
//
BulgeModifier::BulgeModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;
	fA = fB = fC = 0;

	// PMap
	fPMap.fStrengh = .5f;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
	fPMap.fInWidth = true;
	fPMap.fInLength = true;
}

void BulgeModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = 2*fPMap.fLimit[0] - 1;
	fMax = 2*fPMap.fLimit[1] - 1;

	// coeff of the quadric
	fA = -4.0*fPMap.fStrengh/((fMax-fMin)*(fMax-fMin));
	fB = -(fMax+fMin)*fA;
	fC = (fMax*fMin)*fA;
}

void BulgeModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	const real32 axisVal = point[fAxisIndex];

	// Use a function with:
	// f(fMin)=0, f((fMin+fMax)/2)=1, f(fMax)=0
	const real32 val = (fA*axisVal+fB)*axisVal+fC;

	real32 coeff = 0;
	if(fPMap.fStrengh>0)
		coeff = ( val  + (1-fPMap.fStrengh));
	else
		coeff = 1+val;

	if(coeff>kRealEpsilon)
	{
		if(fPMap.fInWidth) result[fU]/=coeff;	
		if(fPMap.fInLength) result[fV]/=coeff;	
	}
	else
	{
		if(fPMap.fInWidth) result[fU]=kRealBig;	
		if(fPMap.fInLength) result[fV]=kRealBig;	
	}
}

////////////////////////////////////////////////////////////////////
//
// Punch
//
PunchModifier::PunchModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;

	// PMap
	fPMap.fStrengh = .5f;
	fPMap.fRadius = .5f;
	fPMap.fAxis = kZAxis;
	fPMap.fFlip = false;
}

void PunchModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
}

void PunchModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	const real32 axisVal = fPMap.fFlip?-point[fAxisIndex]:point[fAxisIndex];

	if(axisVal>0)
	{
		real32 fCoef = kRealOne / fPMap.fRadius;
		real32 coeff = 1;
		TVector2 radial = TVector2(point[fU]*fCoef, point[fV]*fCoef);
		const real32 distToAxis = (radial[0] * radial[0] + radial[1] * radial[1]);
		if(fPMap.fStrengh>0)
		{	// A punch toward the inside of the volume
			coeff = kRealOne + fPMap.fStrengh * (.5*( (distToAxis-kRealOne)/(distToAxis+kRealOne) - kRealOne) );
		}
		else
		{	// A punch toward the outside
			coeff = kRealOne + fPMap.fStrengh + fPMap.fStrengh * (.5*( (distToAxis-kRealOne)/(distToAxis+kRealOne) - kRealOne) );
		}

		if(coeff>kRealEpsilon)
		{
			result[fAxisIndex]/=coeff;	
		}
		else
		{
			result[fAxisIndex]=kRealBig;	
		}
	}
}

////////////////////////////////////////////////////////////////////
//
// Twist
//
TwistModifier::TwistModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;
	fMinMaxCoeff = 1;
	fScale = 1;

	// PMap
	fPMap.fAngle = (real32)PI;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
}

void TwistModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = fPMap.fLimit[0];
	fMax = fPMap.fLimit[1];
	fMinMaxCoeff = 1/(fMax-fMin);
	fScale = (real32)(SQRT2);
}

void TwistModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	// a value between 0 and 1
	const real32 axisVal = MC_Clamp( (real32)(fMinMaxCoeff*( .5*(point[fAxisIndex]+1) - fMin )), kRealZero, kRealOne );

	const real32 cos = RealCos(axisVal*fPMap.fAngle);
	const real32 sin = RealSin(axisVal*fPMap.fAngle);

	const real32 coeffU = point[fU]*cos - point[fV]*sin;
	const real32 coeffV = point[fU]*sin + point[fV]*cos;

	result[fU]=coeffU;	
	result[fV]=coeffV;	
	result[fU]*=fScale;	
	result[fV]*=fScale;	
}

////////////////////////////////////////////////////////////////////
//
// Bend
//
BendModifier::BendModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;

	// PMap
	fPMap.fAngle = (real32)PI;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
}

void BendModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = fPMap.fLimit[0];
	fMax = fPMap.fLimit[1];
	fMinMaxCoeff = 1/(fMax-fMin);
	fScale = (real32)(2);// tmp, to do
}

void BendModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	// a value between 0 and 1
	const real32 axisVal = MC_Clamp( (real32)(fMinMaxCoeff*( .5*(point[fAxisIndex]+1) - fMin )), kRealZero, kRealOne );

	// Offset over and under .5
	const real32 rescaledVal = axisVal - .5;
/*
	const real32 cos = RealCos(axisVal*fAngle);
	const real32 sin = RealSin(axisVal*fAngle);

	const real32 coeffU = point[fU]*cos - point[fV]*sin;
	const real32 coeffV = point[fU]*sin + point[fV]*cos;

	result[fU]=coeffU;	
	result[fV]=coeffV;	

	result[fU] += rescaledVal*fStrengthU;	
	result[fV] += rescaledVal*fStrengthV;	

	result[fU]*=fScaleU;	
	result[fV]*=fScaleV;	*/
}

////////////////////////////////////////////////////////////////////
//
// Shift
//
ShiftModifier::ShiftModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;
	fMinMaxCoeff = 1;
	fScaleU = 1;
	fScaleV = 1;

	// PMap
	fPMap.fStrengthU = 1;
	fPMap.fStrengthV = 1;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
}

void ShiftModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = fPMap.fLimit[0];
	fMax = fPMap.fLimit[1];
	fMinMaxCoeff = 1/(fMax-fMin);
	
	fScaleU = fPMap.fStrengthU+1;
	fScaleV = fPMap.fStrengthV+1;
}

void ShiftModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	// a value between 0 and 1
	const real32 axisVal = MC_Clamp( (real32)(fMinMaxCoeff*( .5*(point[fAxisIndex]+1) - fMin )), kRealZero, kRealOne );

	// Offset over and under .5
	const real32 rescaledVal = axisVal - .5;

	result[fU] += rescaledVal*fPMap.fStrengthU;	
	result[fV] += rescaledVal*fPMap.fStrengthV;	

	result[fU]*=fScaleU;	
	result[fV]*=fScaleV;	
}

////////////////////////////////////////////////////////////////////
//
// Noise
//
NoiseModifier::NoiseModifier()
{
	// PMap
	fPMap.fStrengthX = 1;
	fPMap.fStrengthY = 1;
	fPMap.fStrengthZ = 1;

	fPMap.fGlobalScale = 20;
	fPMap.fXScale = 100;
	fPMap.fYScale = 100;
	fPMap.fZScale = 100;

	fPMap.fXOffset = 0;
	fPMap.fYOffset = 0;
	fPMap.fZOffset = 0;

	fPMap.fXRotation = 0;
	fPMap.fYRotation = 0;
	fPMap.fZRotation = 0;

	fPMap.fSeed = MCRandom();
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');
	gResourceUtilities->GetIndString(fPMap.fMasterShaderName, kStrings, 8 ); // None
	fPMap.fCustomNoiseIndex = 1; // None

	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();

	InitShadingIn(fShadingIn);
}

MCCOMErr NoiseModifier::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (MCVerify(source))
	{
		if(source->GetInstanceID() == 'SEED')
		{
			fPMap.fSeed = MCRandom();
			fNoise.SetSeed(fPMap.fSeed);
			fNoise.InitGradientTab();
			return true;
		}
	}

	return false;
}

void NoiseModifier::PreProcess()
{
	fInvalid = false;
	
	// Get the master shader
	if(!fPMap.fCustomNoiseIndex && fPMap.fMasterShaderName.Length())
	{
		TMCCountedPtr<I3DShScene> scene;
		GetScene(&scene);
		scene->GetMasterShaderByName(&fMasterShader, fPMap.fMasterShaderName);

		if(fMasterShader)
		{
			fMasterShader->GetShader(&fShader);			
		}
	}
	else
	{
		fShader = NULL;
		fMasterShader = NULL;
	}

	// Translation
	fTransform.fTranslation.x = fPMap.fXOffset;
	fTransform.fTranslation.y = fPMap.fYOffset;
	fTransform.fTranslation.z = fPMap.fZOffset;
	// Scaling
	fTransform.fRotationAndScale = TMatrix33::kIdentity;
	fTransform.fRotationAndScale[0][0]=RealAbs(fPMap.fXScale)>kRealEpsilon?100.0/fPMap.fXScale:0;
	fTransform.fRotationAndScale[1][1]=RealAbs(fPMap.fYScale)>kRealEpsilon?100.0/fPMap.fYScale:0;
	fTransform.fRotationAndScale[2][2]=RealAbs(fPMap.fZScale)>kRealEpsilon?100.0/fPMap.fZScale:0;
	// rotation
	real32 phyRad = DegToRad(fPMap.fXRotation);
	real32 thetaRad= DegToRad(fPMap.fYRotation);
	real32 psyRad =DegToRad(fPMap.fZRotation);
	TMatrix33 rotation;
	real64 sinphy = RealSin(phyRad);
	real64 cosphy = RealCos(phyRad);
	real64 sintheta = RealSin(thetaRad);
	real64 costheta = RealCos(thetaRad);
	real64 sinpsy = RealSin(psyRad);
	real64 cospsy = RealCos(psyRad);
	rotation.SetColumn( 0, TVector3(	(cosphy * costheta), (sinphy * costheta), (-sintheta)));
	rotation.SetColumn( 1, TVector3(	(cosphy * sintheta * sinpsy - sinphy * cospsy), 
										(sinphy * sintheta * sinpsy + cosphy * cospsy), 
										(costheta * sinpsy)));
	rotation.SetColumn( 2, TVector3(	(cosphy * sintheta * cospsy + sinphy * sinpsy), 
										(sinphy * sintheta * cospsy - cosphy * sinpsy),
										(costheta * cospsy)));

	fTransform.fRotationAndScale = fTransform.fRotationAndScale*rotation;
	fTransform.fRotationAndScale = (RealAbs(fPMap.fGlobalScale)>kRealEpsilon?(100.0/fPMap.fGlobalScale):0)*fTransform.fRotationAndScale;
}

void NoiseModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	real32 noise = 1;
	if(fShader)
	{
		fShadingIn.fPoint = fShadingIn.fPointLoc = point;
		boolean fullArea=false;
		fShader->GetValue(noise,fullArea,fShadingIn);
	}
	else
	{
		noise = fNoise.GetValueSmooth(fTransform.TransformPoint(point));
	}

	result.x += noise*fPMap.fStrengthX;
	result.y += noise*fPMap.fStrengthY;
	result.z += noise*fPMap.fStrengthZ;
}

////////////////////////////////////////////////////////////////////
//
// Stretch
//
StretchModifier::StretchModifier()
{
	// Cache
	fAxisIndex = 2;
	fU = 0;
	fV = 1;
	fMin = 0;
	fMax = 1;
	fCoeff = 0;
	f0 = 0;
	f1 = 0;

	// PMap
	fPMap.fStrength = .5f;
	fPMap.fCenter = 0;
	fPMap.fLimit = TVector2::kUnitY;
	fPMap.fAxis = kZAxis;
}

void StretchModifier::PreProcess()
{
	fInvalid = false;
	
	switch(fPMap.fAxis)
	{
		case kXAxis: fAxisIndex = 0; fU = 1; fV = 2; break;
		case kYAxis: fAxisIndex = 1; fU = 2; fV = 0; break;
		case kZAxis: fAxisIndex = 2; fU = 0; fV = 1; break;
	}
	fMin = fPMap.fLimit[0];
	fMax = fPMap.fLimit[1];

	// This works for fStrength>0
	fCoeff = RealExp(fPMap.fStrength) - 1;

	f0 = Function(0);
	f1 = Function(1);
}

void StretchModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	const real32 axisVal = .5*point[fAxisIndex]+.5;

	const real32 fX = Function(axisVal);

	// Rescale to keep the value inside the bbox
	result[fAxisIndex] = 2* ((fX - f0)/((f1 - f0)) + f0) - 1;
}

////////////////////////////////////////////////////////////////////
//
// Scale
//
ScaleModifier::ScaleModifier()
{
	// PMap
	fPMap.fScaleX = 1;
	fPMap.fScaleY = 1;
	fPMap.fScaleZ = 1;
}

void ScaleModifier::PreProcess()
{
	fInvalid = false;
}

void ScaleModifier::DeformPoint(const TVector3 & point,  TVector3 & result)
{
	if(fInvalid)
		PreProcess();
	
	// Init
	result = point;

	result.x /= fPMap.fScaleX;
	result.y /= fPMap.fScaleY;
	result.z /= fPMap.fScaleZ;
}





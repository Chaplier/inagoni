/****************************************************************************************************

		Modifier.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/26/2004

****************************************************************************************************/

#ifndef __Modifier__
#define __Modifier__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicDataComponent.h"
#include "Utils.h"

extern const MCGUID CLSID_VolumeModifier;

extern const MCGUID CLSID_Taper;
extern const MCGUID CLSID_Wave;
extern const MCGUID CLSID_Bulge;
extern const MCGUID CLSID_Punch;
extern const MCGUID CLSID_Twist;
extern const MCGUID CLSID_Bend;
extern const MCGUID CLSID_Shift;
extern const MCGUID CLSID_Noise;
extern const MCGUID CLSID_Stretch;
extern const MCGUID CLSID_Scale;

class BasicVolumeModifier : public TBasicDataComponent
{
public:	
	virtual MCCOMErr	MCCOMAPI QueryInterface	(const MCIID &riid, void** ppvObj);

	BasicVolumeModifier();
	STANDARD_RELEASE;

	virtual MCCOMErr	MCCOMAPI ExtensionDataChanged(){fInvalid = true; return MC_S_OK;}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result) = 0;

	//virtual MCCOMErr MCCOMAPI GetParameter(IDType keyword, void *parameter);
	//virtual MCCOMErr MCCOMAPI SetParameter(IDType keyword, void *parameter);
	//virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);
	//virtual MCCOMErr MCCOMAPI SimpleHandleEvent(MessageID message, IMFResponder* source, void* data);
	//virtual MCCOMErr MCCOMAPI CopyComponentExtraData(IExDataExchanger* dest);
protected:
	boolean		fInvalid;
};

////////////////////////////////////////////////////////////////////
//
// Taper
//
struct TaperModifierPMap
{
	real32		fStrengh;
	TVector2	fLimit;
	int32		fAxis;
	boolean		fFlip;
	boolean		fInWidth;
	boolean		fInLength;
};

#include "BasicModifiers.h"
class TaperModifier : public BasicVolumeModifier
{
public:	

	TaperModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(TaperModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	boolean		fInvalid;

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;

	// PMap
	TaperModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Wave
//
struct WaveModifierPMap
{
	real32		fStrengh;
	real32		fWaveCount;
	real32		fPhase;
	TVector2	fLimit;
	int32		fAxis;
	int32		fType; // planarA, B, radial, cylindrical
};

class WaveModifier : public BasicVolumeModifier
{
public:	

	WaveModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(WaveModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fCoeff1;
	real32		fCoeff2;
	real32		fCoeff3;

	// PMap
	WaveModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Bulge
//
struct BulgeModifierPMap
{
	real32		fStrengh;
	TVector2	fLimit;
	int32		fAxis;
	boolean		fInWidth;
	boolean		fInLength;
};

class BulgeModifier : public BasicVolumeModifier
{
public:	

	BulgeModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(BulgeModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fA; // coeff of the quadric
	real32		fB; // coeff of the quadric
	real32		fC; // coeff of the quadric

	// PMap
	BulgeModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Punch
//
struct PunchModifierPMap
{
	real32		fStrengh;
	real32		fRadius;
	int32		fAxis;
	boolean		fFlip;
};

class PunchModifier : public BasicVolumeModifier
{
public:	

	PunchModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(PunchModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;

	// PMap
	PunchModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Twist
//
struct TwistModifierPMap
{
	real32		fAngle;
	TVector2	fLimit;
	int32		fAxis;
};

class TwistModifier : public BasicVolumeModifier
{
public:	

	TwistModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(TwistModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fMinMaxCoeff;
	real32		fScale;

	// PMap
	TwistModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Bend
//
struct BendModifierPMap
{
	real32		fAngle;
	real32		fOrientation;
	TVector2	fLimit;
	int32		fAxis;
};

class BendModifier : public BasicVolumeModifier
{
public:	

	BendModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(BendModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fMinMaxCoeff;
	real32		fScale;

	// PMap
	BendModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Shift
//
struct ShiftModifierPMap
{
	real32		fStrengthU;
	real32		fStrengthV;
	TVector2	fLimit;
	int32		fAxis;
};

class ShiftModifier : public BasicVolumeModifier
{
public:	

	ShiftModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(ShiftModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fMinMaxCoeff;
	real32		fScaleU;
	real32		fScaleV;

	// PMap
	ShiftModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Noise
//
#include "NoiseBase.h"

struct NoiseModifierPMap
{
	real32		fStrengthX;
	real32		fStrengthY;
	real32		fStrengthZ;
	real32 fGlobalScale;
	real32 fXScale;
	real32 fYScale;
	real32 fZScale;
	real32 fXOffset;
	real32 fYOffset;
	real32 fZOffset;
	real32 fXRotation;
	real32 fYRotation;
	real32 fZRotation;
	int32		fSeed;
	TMCDynamicString	fMasterShaderName; // to replace the build in noise
	int32				fCustomNoiseIndex; // to replace the build in noise
};

class NoiseModifier : public BasicVolumeModifier
{
public:	

	NoiseModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(NoiseModifierPMap);}
	virtual MCCOMErr	MCCOMAPI SimpleHandleEvent(MessageID message, IMFResponder* source, void* data);

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	TNoiseBase	fNoise;
	
	// Cache
	TMCCountedPtr<I3DShMasterShader> fMasterShader;
	TMCCountedPtr<I3DShShader>	fShader; // replace the noise
	ShadingIn		fShadingIn; // replace the noise
	TTransform3D	fTransform;

	// PMap
	NoiseModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Stretch
//
struct StretchModifierPMap
{
	real32		fStrength;
	real32		fCenter;
	TVector2	fLimit;
	int32		fAxis;
};

class StretchModifier : public BasicVolumeModifier
{
public:	

	StretchModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(StretchModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	inline real32 Function(const real32 val) const
	{
		return val + fCoeff*Bias(MC_Clamp(1-fPMap.fCenter,kRealZero+kRealEpsilon, kRealOne-kRealEpsilon), SmoothStep(fMin,fMax,val) );
	}

	// Cache data
	int32		fAxisIndex;
	int32		fU;
	int32		fV;
	real32		fMin;
	real32		fMax;
	real32		fScale;
	real32		fCoeff;
	real32		f0;
	real32		f1;

	// PMap
	StretchModifierPMap fPMap;
};

////////////////////////////////////////////////////////////////////
//
// Scale
//
#include "NoiseBase.h"

struct ScaleModifierPMap
{
	real32		fScaleX;
	real32		fScaleY;
	real32		fScaleZ;
};

class ScaleModifier : public BasicVolumeModifier
{
public:	

	ScaleModifier();
	STANDARD_RELEASE;

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
	virtual int32		MCCOMAPI GetParamsBufferSize() const {return sizeof(ScaleModifierPMap);}

	virtual void DeformPoint(const TVector3 & point,  TVector3 & result);

protected:

	void PreProcess();

	// PMap
	ScaleModifierPMap fPMap;
};

#endif

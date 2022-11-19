/****************************************************************************************************

		GasBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#ifndef __GasBase__
#define __GasBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "GasDef.h"
#include "BasicPrimitive.h"
#include "MCBasicDefines.h"
#include "I3dExPrimitive.h"
#include "MCGradient.h"
#include "IShComponent.h"
#include "I3DShShader.h"
#include "I3DShModifier.h"
#include "I3DExModifier.h"
#include "IComponentChooser.h"
#include "IShTokenStream.h"
/* Doesn't work
#include "IComponentAnim.h" // for the undo system
#include "IMFParameterPart.h" // for the undo system
*/
#include "FacetGenerator.h"
#include "MCSMPCriticalSection.h"


#include "NoiseBase.h"
#include "VoronoiBase.h"
#include "Swirl.h"
#include "Modifier.h"

extern const MCGUID CLSID_GasBase;

boolean IsLocked();
boolean IsDemoVersion();


void GetNoiseName(const int32 index, TMCString& name);
int32 GetDensityFlag(const int32 densityPreset);

enum ERampOff
{
	eNoRampOff				= 0x00000000,
	ePosHalfSphereUp		= 0x00000001,
	ePosHalfSphereDown		= 0x00000002,
	eNegHalfSphereUp		= 0x00000004,
	eNegHalfSphereDown		= 0x00000008,
	ePosXCylinder			= 0x00000010,//16
	ePosYCylinder			= 0x00000020,//32
	ePosZCylinder			= 0x00000040,//64
	eNegXCylinder			= 0x00000080,//128
	eNegYCylinder			= 0x00000100,//256
	eNegZCylinder			= 0x00000200,//512
	ePosXDist				= 0x00000400,//1024
	ePosYDist				= 0x00000800,//2048
	ePosZDist				= 0x00001000,//4096
	eNegXDist				= 0x00002000,//1024
	eNegYDist				= 0x00004000,//2048
	eNegZDist				= 0x00008000,//4096
};

enum ESelfShadow
{
	eNoSelfShadow			= 'Opt0',
	eFakeSelfShadow			= 'Opt1', // use a fake distant light
	eDistantSelfShadow		= 'Opt2', // use the scene distant lighs
};

// Communication between part and comp
struct MultiCompData
{
	MultiCompData(const int32 message, IMultipleComponentChooser* compChooser){fMessage=message;fCompChooser=compChooser;}
	int32 fMessage;
	TMCCountedPtr<IMultipleComponentChooser> fCompChooser;
};

class Line : public TMCArray<real32>{};
class Plane : public TMCClassArray<Line>{};
class Volume : public TMCClassArray<Plane>{};

class LightInfo
{
public:
	boolean			fIsInfinite;
	TVector3		fPos;
	TVector3		fDirection;
	TMCColorRGBA	fIntensityColor;
};
void FillInLightInfoArray(TMCClassArray<LightInfo>&	lightInfo);

// Value between 0 and 1
// .5 is linear rampoff
struct RampOffFactors
{
	RampOffFactors();

	TVector2 fPosHalfSphereUp;
	TVector2 fPosHalfSphereDown;
	TVector2 fNegHalfSphereUp;
	TVector2 fNegHalfSphereDown;
	TVector2 fPosXCylinder;
	TVector2 fPosYCylinder;
	TVector2 fPosZCylinder;
	TVector2 fNegXCylinder;
	TVector2 fNegYCylinder;
	TVector2 fNegZCylinder;
	TVector2 fPosXDist;
	TVector2 fPosYDist;
	TVector2 fPosZDist;
	TVector2 fNegXDist;
	TVector2 fNegYDist;
	TVector2 fNegZDist;
};

static const int32 presetCount = 13;
class GradientPreset : public TMCClassArray<TMCGradient>
{
public:
	enum EGradient
	{
		eGBlack,
		eGWhite,
		eGFire1,
		eGFire2,
		eGFire3,
		eGFire4,
		eGFire5,
		eGFire6,
		eGSmog1,
		eGSmog2,
		eGSmog3,
		eGSmog4,
		eGSmog5,
	};

	GradientPreset();
};

extern GradientPreset*	gGradientPresets;

class PrimitiveSize
{
public:
	PrimitiveSize() : mCurrentSize(0) {}

	const TBBox3D& GetBBox(real32 primSize)
	{
		Init(primSize);
		return mDefaultBBox;
	};

	const real32 GetBBoxInvSize(real32 primSize) 
	{
		Init(primSize);
		return mBBoxInvSize;
	};

protected:
	void Init(real32 newSize)
	{
		if(mCurrentSize!=newSize)
		{
			mCurrentSize=newSize;

			real32 bBoxHalfSize = mCurrentSize; // default size

			real32 bBoxSize = 2*bBoxHalfSize;
			mBBoxInvSize = 1.0f/bBoxSize;
			TVector3 bBoxHalfDim = TVector3(bBoxHalfSize, bBoxHalfSize, bBoxHalfSize);
			mDefaultBBox.SetMin( -bBoxHalfDim );
			mDefaultBBox.SetMax(  bBoxHalfDim );
		}
	}

	real32 mCurrentSize;

	TBBox3D mDefaultBBox;
	real32 mBBoxInvSize;
};

class GasBasePMap
{
public:
	GasBasePMap();

	real32 fPrimitiveSize;

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

	// Gas params
	real32 fSmooth;
	real32 fQuality; // from 0 to over 100%
	real32 fDensityFactor; // 
	real32 fSelfIntensity; // 

	// Noise params
	real32 fSeed;
	real32 fFractalDepth;
	real32 fRealGain; // Contrast
	real32 fRealBias; // Brightness

	// Instead of the build-in noise:
	TMCDynamicString	fMasterShaderName;
	int32				fCustomNoiseIndex;
// this nearly works with the default UI system,
// but there' 20 pixel from the bar that are missing
// TMCCountedPtr<IShParameterComponent>	fSubShaderComponent;

	// The name of the tree element to use as an external shape
	TMCDynamicString	fMeshName;
	int32				fCustomMeshIndex; // 1 if the mesh name should be ignored (the name will be "None")
	real32				fMeshSmoothing;

	// Self shadow
	int32 fSelfShadowType;
	real32 fShadowIntensity;
	real32 fDiffuseLight; // simulate a diffuse illumination
	TMCGradient fShadowColorGradient; // shadow color: change with intensity
	real32 fFakeLightDirectionX;
	real32 fFakeLightDirectionY;
	real32 fFakeLightDirectionZ;

	// colors
	TMCGradient fColorGradient; // color can change with height, intensity, ... 
	int32 fColorFlag; // From ERampOff
	real32 fColorCoeff; // repartition between the rampoff and the intensity
	RampOffFactors fColorRamps;
	// Or instead of the color gradient
	TMCDynamicString	fColorMasterShaderName;
	int32				fColorCustomShaderIndex;

	// intensity
	int32 fRampOffFlag; // From ERampOff
	RampOffFactors fIntensityRamps;

	// Deformers
	TMCCountedPtrArray<IShParameterComponent> fParameterComponentList;
};

class GasBase : public TBasicPrimitive, 
				public IVolumePrimitive, 
				public I3DExNewVolumePrimitive
{
public:
	GasBase();
	virtual ~GasBase(){DeleteCS(mGasCriticalSection);}
	STANDARD_RELEASE
	virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID &riid, void** ppvObj);
	virtual uint32		MCCOMAPI AddRef() { return TBasicPrimitive::AddRef(); }

	virtual MCCOMErr	MCCOMAPI ExtensionDataChanged();
	virtual void		MCCOMAPI GetBoundingBox		(TBBox3D& bbox);
	virtual MCCOMErr	MCCOMAPI SetDeformedBoundingBox(const TVector3& min, const TVector3& max);
//	virtual MCCOMErr   MCCOMAPI EnumPatches			(EnumPatchesCallback callback, void* privData, boolean& closed){return MC_S_OK;}
//	virtual MCCOMErr   MCCOMAPI GetNbrLOD			(int16& nbrLod){nbrLod=1; return MC_S_OK;}
//	virtual MCCOMErr   MCCOMAPI GetLOD				(int16 lodIndex, real& lod){return MC_S_OK;}
//	virtual MCCOMErr   MCCOMAPI GetFacetMesh		(uint32 lodIndex, FacetMesh** outMesh){return MC_S_OK;}
//	virtual MCCOMErr   MCCOMAPI GetFacetMesh		(real lod, FacetMesh** outMesh){return MC_S_OK;}
//	virtual boolean    MCCOMAPI CanBeSplit			(){return false;};
//	virtual MCCOMErr   MCCOMAPI SplitPrimitive		(TMCCountedPtrArray<I3DExGeometricPrimitive>& subParts, TMCArray<TTransform3D>& subPartPositions){return MC_S_OK;}
//	virtual uint32     MCCOMAPI GetUVSpaceCount		(){return 1;}
//	virtual MCCOMErr   MCCOMAPI GetUVSpace			(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo){return MC_S_OK;}
//	virtual MCCOMErr   MCCOMAPI UV2XYZ				(TVector2* uv, uint32 uvSpaceID, TVector3* resultPosition, boolean* inUVSpace){return MC_E_NOTIMPL;}
//	virtual MCCOMErr   MCCOMAPI GetUVSpaceRDS5		(uint32 uvSpaceID, UVSpaceInfoRDS5* uvSpaceInfoRDS5){return MC_E_NOTIMPL;}
//	virtual MCCOMErr   MCCOMAPI AppendToRenderables	(const TTransform3D& worldFromModelTfm, TRenderableAndTfmArray& renderableAndTfm ){return MC_S_OK;}
	virtual boolean		MCCOMAPI IsBBoxPickable() const {return true;}
//	virtual boolean		MCCOMAPI IsExcludedFromTripleBuffer(){return false;}
//	virtual void MCCOMAPI GetDefaultShader(I3DShShader** shader) {};
	virtual void		MCCOMAPI ChangedData(); // this is called by the action
	virtual boolean		MCCOMAPI AutoSwitchToModeler() const {return true;}

	virtual void*		MCCOMAPI GetExtensionDataBuffer	(){return &fPMap;}
//	virtual int32		MCCOMAPI GetParamsBufferSize() const {return (sizeof(GasBasePMap));}

/* Doesn't work
	virtual	MCCOMErr	MCCOMAPI CopyComponentExtraData( IExDataExchanger* dest );
*/
	// IVolumePrimitive
	virtual MCCOMErr MCCOMAPI GetVolumeDensity		(const TVector3& point,TMCColorRGBA& filter,TMCColorRGBA& glow);
	virtual MCCOMErr MCCOMAPI GetVolumeAttenuation	(const TVector3& from,const TVector3& to,TMCColorRGBA& attenuation, TColorRGBLinearEffect* linearEffect, boolean isShadowCasting, boolean oldShadowFiltering);	

	// Doesn't receive the messages virtual MCCOMErr MCCOMAPI SimpleHandleEvent			(MessageID message, IMFResponder* source, void* data);
	virtual MCCOMErr MCCOMAPI SimpleHandleEvent			(MessageID message, IMFResponder* source, void* data);

/* Doesn't work
	// For the undo system
	void SetClonedComponent(IComponentAnim* comp){fClonedComponent = comp;}
	void GetClonedComponent(IComponentAnim** comp){ TMCCountedGetHelper<IComponentAnim> result(comp); result = fClonedComponent; }
*/
	// I3DExNewVolumePrimitive interface
	virtual void MCCOMAPI BeginRender();
	virtual void MCCOMAPI EndRender();
	virtual void MCCOMAPI TraceRay(	const Ray3D&			ray,
									const Ray3D&			localRay,
								    const real64&			tmin,
								    const real64&			tmax,
									TMCColorRGBA&			inOutColor,
									TColorRGBLinearEffect*	linearEffect,
									I3DExRaytracer*			raytracer,
									I3DShInstance*			instance,
									uint32					instanceNb,
									boolean					isShadowRay,
									boolean					isIndirectRay,
									boolean					oldShadowFiltering);
	virtual boolean  MCCOMAPI UsesShaders() const{return false;}

	//// IExStreamIO methods
	//MCCOMErr MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	//MCCOMErr MCCOMAPI Write(IShTokenStream* stream);
	//// set the primitive size to 4 before reading it (for old files compatibility)
	//MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext);
protected:
	void	PreProcess();
	virtual void	CustomPreProcess(){}
	void	InitNoise();
	void	InitModifiers();
	void	Prepare3DTransform();

	bool	UseMesh() const {return (fPMap.fCustomMeshIndex == 0);}

	void			InitSelfShadowTable();
	void			CheckShadowTable(); // check for lights animation

	real32		GetLightsIntensity(const TVector3& point);
	real32		GetLightIntensity(const TVector3& point, const LightInfo& light);
	inline real32	GetIntensity(const int32 iX, const int32 iY, const int32 iZ) const;
	real32		InterpolateIntensity(const TVector3& point);

	virtual real32	GetLocalDensity(const TVector3& point)=0;
	real32	GetGasValue(const TVector3& point);
	real32	GetTotalOpacity(const real32 volumeDensity,
							real32& densitySum);
	void	IncrementColorAndOpacity(const TVector3& currentPoint, const real32 stepLength,
				 real32& totalOpacity, real32& densitySum,
				 TMCColorRGBA& finalColor );

	void	GetGasColor(TMCColorRGBA&		gasColor,
							 const real32		localDensity,
							 const TVector3&	pos);

	real32	RampOff(const TVector3& point,
					const int32 rampFlag,
					const RampOffFactors& factors,
					const boolean smoothRamp) ;

	void	SetDensityPreset(const int32 densityPreset);
	void	SetColorPreset(const int32 colorPreset);

	void GetMasterShader();

	void ApplyModifiers(TVector3& point);

	void GetNoisedPoint(const TVector3& currentPoint, 
							 const real32 amplitude,
							 TVector3& outPoint);

/* Doesn't work	void PostAction(const IDType partID);
*/
	const inline real32 NoisedStepLength(const real32 stepLength, const real32 remainingLength, const TVector3& largePos);

	boolean			fGeneralInvalid;
	boolean			fNoiseInvalid;
	boolean			fModifiersInvalid;

	TTransform3D	fTransform;
	TTransform3D	fInvertTransform;
//	TVector3		fScaling; // fXScaling, fYScaling and fZScaling. Should maybe replace them in the pmap

	// Fake noise supersampling
	TVector3		fI;
	TVector3		fJ;

	TMCClassArray<Swirl>	fSwirlArray;

	TMCArray<real32> fContrastTable; // precompute the pow values
	TMCArray<real32> fBrightnessTable; // precompute the pow values
	real32			fOpticalDepth;
	real32			fActualDensity;
	TNoiseBase		fNoise;
// Not use yet		TVoronoiBase	fVoronoi;
	TMCCountedPtr<I3DShMasterShader> fColorMasterShader;
	TMCCountedPtr<I3DShShader>	fColorShader; // replace the color gradient
	TMCCountedPtr<I3DShMasterShader> fMasterShader;
	TMCCountedPtr<I3DShShader>	fShader; // replace the noise
	ShadingIn		fShadingIn; // replace the noise

	//
	boolean						fHasSelfShadow;
	real32						fShadowTableSize;
	Volume						fVolume;
	TMCClassArray<LightInfo>	fLightInfo;

	// Color gradient coeffs
	real32 fPond1;
	real32 fPond2;
	real32 fPond3;
	real32 fLastRampOff; // cache the ramp off for the color computation

	// For gas contained in an object
	FacetGenerator mFacetGenerator;

	// Undo system
/* Doesn't work
	boolean							fNeedToPost;
	TMCCountedPtr<IComponentAnim>	fClonedComponent;
	TMCCountedPtr<IMFParameterComponentPart> fCompUIPart;
*/
	// Critical section for multithrading
	TMCCriticalSection* mGasCriticalSection;

	// Modifiers interfaces
	TMCCountedPtrArray<BasicVolumeModifier> fModifierList;

	// A small utility to compute the primitive size from the PMap value
	PrimitiveSize fPrimSize;

	// PMap start here
	GasBasePMap		fPMap;
	// PMAP continues in derived classes
};

inline real32	GasBase::GetIntensity(const int32 iX, const int32 iY, const int32 iZ) const
{
	MCAssert(iX>=0 && iX<fShadowTableSize);
	MCAssert(iY>=0 && iY<fShadowTableSize);
	MCAssert(iZ>=0 && iZ<fShadowTableSize);
	return fVolume[iZ][iY][iX];
}

// Rampoff: Wyvill's density function (p271):
// 1 when r=0, 0 when r=R
//real32 Denstity(const real32 r2, const real32 R2)
//{
//	return (1+())
//}

#endif

/****************************************************************************************************

		GasBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/8/2004

****************************************************************************************************/

#include "GasBase.h"

//#include "AssembleRoomPart.h"
#include "copyright.h"
#include "Utils.h"
#include "Geometry.h"

#include "IMFPart.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "I3DShScene.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "I3dExLight.h"
#include "I3DShLightSource.h"
#include "I3DShTreeElement.h"
#include "MiscComUtilsImpl.h"
#include "IMFResponder.h"
#include "Copyright.h"
#include "IShThreadUtilities.h"
#include "ComMessages.h"
#include "PMapTypes.h"
#include "IMFDocument.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_GasBase(R_CLSID_GasBase);
#else
const MCGUID CLSID_GasBase={R_CLSID_GasBase};
#endif

static const int32 kShadowMaxTableSize = 64;

static const TVector3 kOnes = TVector3(1, 1, 1);
static const real32 kOneEpsilon = 1+kRealEpsilon;

static const TVector3 kHalf(.5f,.5f,.5f);

static const TVector2 init0_5(0, .5f);
static const TVector2 init0_1(0, .1f);
static const TVector2 init0_3(0, .3f);
static const TVector2 init3_5(.3f, .5f);

// powTable
static const int32 kPowTableSize = 10000;
static const int32 kPowTableSizeMinusOne = kPowTableSize - 1;

void GetNoiseName(const int32 index, TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');
	switch(index)
	{
	case 1: gResourceUtilities->GetIndString(name, kStrings, 3 ); break;
	case 2: gResourceUtilities->GetIndString(name, kStrings, 4 ); break;
	case 3: gResourceUtilities->GetIndString(name, kStrings, 5 ); break;
	case 4: gResourceUtilities->GetIndString(name, kStrings, 6 ); break;
	case 5: gResourceUtilities->GetIndString(name, kStrings, 8 ); break; // "None"
	default: MCNotify("Unknown Noise"); break;
	}
}

// initialise the light array
void FillInLightInfoArray(TMCClassArray<LightInfo>&	lightInfo)
{
	lightInfo.SetElemCount(0);

	// Get the scene
	TMCCountedPtr<I3DShScene> scene;
	GetScene(&scene);

	const int32 lightCount = scene->GetLightsourcesCount();


	for (int32 iLight=0 ; iLight<lightCount ; iLight++)
	{
		TMCCountedPtr<I3DShLightsource> lightSource;
		scene->GetLightsourceByIndex(&lightSource,iLight);

		TStandardLight lightFlag;
		lightSource->GetStandardLight(lightFlag);

		if(lightFlag.fLightType!=TStandardLight::kDistantLight)
			continue; // just use the distant lights

// Can't do that because of sun light
		//if(!lightSource->IsVisibleInPerspective())
		//	continue; // light not visible

		LightInfo& newInfo = lightInfo.AddElem();
		newInfo.fIsInfinite = true;
		real32 resultDistance=0;
		TVector3 dir;
		lightSource->GetDirection(TVector3::kZero, dir, resultDistance);
		newInfo.fDirection = -dir; // from the light to the object
		// 
		real32 ShadowIntensity=0;
		lightSource->GetColor(TVector3::kZero,dir,resultDistance, newInfo.fIntensityColor, ShadowIntensity);
	}
}

RampOffFactors::RampOffFactors()
{
	fPosHalfSphereUp = init0_5;
	fPosHalfSphereDown = init0_5;
	fNegHalfSphereUp = init0_5;
	fNegHalfSphereDown = init0_5;
	fPosXCylinder = init0_5;
	fPosYCylinder = init0_5;
	fPosZCylinder = init0_5;
	fNegXCylinder = init0_5;
	fNegYCylinder = init0_5;
	fNegZCylinder = init0_5;
	fPosXDist = init0_5;
	fPosYDist = init0_5;
	fPosZDist = init0_5;
	fNegXDist = init0_5;
	fNegYDist = init0_5;
	fNegZDist = init0_5;
}

inline real32 col(const real32 val){return(val/255.0f);}
GradientPreset::GradientPreset()
{	// Build some gradients
	SetElemCount(presetCount);
	TMCColorRGBA black=TMCColorRGBA(0,0,0,0);
	TMCColorRGBA white=TMCColorRGBA(1,1,1,0);
	{	// Black
		TMCGradient* gradient = Pointer(eGBlack);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey	(black, 0);
		gradient->AddColorKey	(black, 1);
#else
		gradient->SetColorKeyColor	(0, black);
		gradient->SetColorKeyColor	(1, black);
#endif
	}
	{	// White
		TMCGradient* gradient = Pointer(eGWhite);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey	(white, 0);
		gradient->AddColorKey	(white, 1);
#else
		gradient->SetColorKeyColor	(0, white);
		gradient->SetColorKeyColor	(1, white);
#endif
	}
	{	// Fire1
		TMCGradient* gradient = Pointer(eGFire1);
		TMCColorRGBA fire1c1=TMCColorRGBA(1, col(168), col(51), 0);
		TMCColorRGBA fire1c2=TMCColorRGBA(1, col(218), col(1), 0);
		TMCColorRGBA fire1c3=TMCColorRGBA(1, 1, col(253), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire1c1, .07f );
		gradient->AddColorKey		(fire1c2, .41f );
		gradient->AddColorKey		(fire1c3, .75f );
#else
		gradient->SetColorKeyColor	(0, fire1c1);
		gradient->SetColorKeyIndex	(0, 0.07f);
		gradient->SetColorKeyColor	(1, fire1c2);
		gradient->SetColorKeyIndex	(1, 0.41f);
		gradient->AddColorKey		(fire1c3, .75f );
#endif
	}
	{	// Fire2
		TMCGradient* gradient = Pointer(eGFire2);
		TMCColorRGBA fire2c1=TMCColorRGBA(1, col(186), col(76), 0);
		TMCColorRGBA fire2c2=TMCColorRGBA(1, col(249), col(76), 0);
		TMCColorRGBA fire2c3=TMCColorRGBA(1, col(242), col(166), 0);
		TMCColorRGBA fire2c4=TMCColorRGBA(1, 1, col(253), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire2c1, 0 );
		gradient->AddColorKey		(fire2c2, .16f );
		gradient->AddColorKey		(fire2c3, .4f );
		gradient->AddColorKey		(fire2c4, .75f );
#else
		gradient->SetColorKeyColor	(0, fire2c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, fire2c2);
		gradient->SetColorKeyIndex	(1, 0.16f);
		gradient->AddColorKey		(fire2c3, .4f );
		gradient->AddColorKey		(fire2c4, .75f );
#endif
	}
	{	// Fire3
		TMCGradient* gradient = Pointer(eGFire3);
		TMCColorRGBA fire3c1=TMCColorRGBA(col(3), col(3), col(3), 0);
		TMCColorRGBA fire3c2=TMCColorRGBA(col(165), col(148), col(141), 0);
		TMCColorRGBA fire3c3=TMCColorRGBA(1, col(88), col(26), 0);
		TMCColorRGBA fire3c4=TMCColorRGBA(1, col(175), col(24), 0);
		TMCColorRGBA fire3c5=TMCColorRGBA(1, col(253), col(24), 0);
		TMCColorRGBA fire3c6=TMCColorRGBA(1, 1, col(248), 0);
		TMCColorRGBA fire3c7=TMCColorRGBA(col(13), col(22), 1, 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire3c1, .15f );
		gradient->AddColorKey		(fire3c2, .32f );
		gradient->AddColorKey		(fire3c3, .43f );
		gradient->AddColorKey		(fire3c4, .67f );
		gradient->AddColorKey		(fire3c5, .8f );
		gradient->AddColorKey		(fire3c6, .92f );
		gradient->AddColorKey		(fire3c7, 1 );
#else
		gradient->SetColorKeyColor	(0, fire3c1);
		gradient->SetColorKeyIndex	(0, 0.15f);
		gradient->SetColorKeyColor	(1, fire3c2);
		gradient->SetColorKeyIndex	(1, 0.32f);
		gradient->AddColorKey		(fire3c3, .43f );
		gradient->AddColorKey		(fire3c4, .67f );
		gradient->AddColorKey		(fire3c5, .8f );
		gradient->AddColorKey		(fire3c6, .92f );
		gradient->AddColorKey		(fire3c7, 1 );
#endif
	}
	{	// Fire4
		TMCGradient* gradient = Pointer(eGFire4);
		TMCColorRGBA fire4c1=TMCColorRGBA(col(213), col(212), col(210), 0);
		TMCColorRGBA fire4c2=TMCColorRGBA(col(255), col(158), col(16), 0);
		TMCColorRGBA fire4c3=TMCColorRGBA(col(238), col(192), col(123), 0);
		TMCColorRGBA fire4c4=TMCColorRGBA(1, col(254), col(253), 0);
		TMCColorRGBA fire4c5=TMCColorRGBA(col(97), col(97), col(180), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire4c1, 0 );
		gradient->AddColorKey		(fire4c2, .16f );
		gradient->AddColorKey		(fire4c3, .38f );
		gradient->AddColorKey		(fire4c4, .54f );
		gradient->AddColorKey		(fire4c5, 1 );
#else
		gradient->SetColorKeyColor	(0, fire4c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, fire4c2);
		gradient->SetColorKeyIndex	(1, 0.16f);
		gradient->AddColorKey		(fire4c3, .38f );
		gradient->AddColorKey		(fire4c4, .54f );
		gradient->AddColorKey		(fire4c5, 1 );
#endif
	}
	{	// Fire5
		TMCGradient* gradient = Pointer(eGFire5);
		TMCColorRGBA fire5c1=TMCColorRGBA(col(55), col(52), col(2), 0);
		TMCColorRGBA fire5c2=TMCColorRGBA(col(117), col(117), col(117), 0);
		TMCColorRGBA fire5c3=TMCColorRGBA(col(146), col(103), col(103), 0);
		TMCColorRGBA fire5c4=TMCColorRGBA(col(254), col(255), col(255), 0);
		TMCColorRGBA fire5c5=TMCColorRGBA(col(255), col(166), col(50), 0);
		TMCColorRGBA fire5c6=TMCColorRGBA(col(250), col(253), col(151), 0);
		TMCColorRGBA fire5c7=TMCColorRGBA(col(255), col(255), col(252), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire5c1, 0 );
		gradient->AddColorKey		(fire5c2, .2f );
		gradient->AddColorKey		(fire5c3, .29f );
		gradient->AddColorKey		(fire5c4, .46f );
		gradient->AddColorKey		(fire5c5, .61f );
		gradient->AddColorKey		(fire5c6, .75f );
		gradient->AddColorKey		(fire5c7, .9f );
#else
		gradient->SetColorKeyColor	(0, fire5c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, fire5c2);
		gradient->SetColorKeyIndex	(1, 0.2f);
		gradient->AddColorKey		(fire5c3, .29f );
		gradient->AddColorKey		(fire5c4, .46f );
		gradient->AddColorKey		(fire5c5, .61f );
		gradient->AddColorKey		(fire5c6, .75f );
		gradient->AddColorKey		(fire5c7, .9f );
#endif
	}
	{	// Fire6
		TMCGradient* gradient = Pointer(eGFire6);
		TMCColorRGBA fire6c1=TMCColorRGBA(col(117), col(117), col(117), 0);
		TMCColorRGBA fire6c2=TMCColorRGBA(col(146), col(103), col(103), 0);
		TMCColorRGBA fire6c3=TMCColorRGBA(col(254), col(255), col(255), 0);
		TMCColorRGBA fire6c4=TMCColorRGBA(col(255), col(166), col(50), 0);
		TMCColorRGBA fire6c5=TMCColorRGBA(col(250), col(254), col(151), 0);
		TMCColorRGBA fire6c6=TMCColorRGBA(col(255), col(255), col(254), 0);
		TMCColorRGBA fire6c7=TMCColorRGBA(col(229), col(225), col(255), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(fire6c1, 0 );
		gradient->AddColorKey		(fire6c2, .07f );
		gradient->AddColorKey		(fire6c3, .23f );
		gradient->AddColorKey		(fire6c4, .38f );
		gradient->AddColorKey		(fire6c5, .51f );
		gradient->AddColorKey		(fire6c6, .74f );
		gradient->AddColorKey		(fire6c7, 1 );
#else
		gradient->SetColorKeyColor	(0, fire6c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, fire6c2);
		gradient->SetColorKeyIndex	(1, 0.07f);
		gradient->AddColorKey		(fire6c3, .23f );
		gradient->AddColorKey		(fire6c4, .38f );
		gradient->AddColorKey		(fire6c5, .51f );
		gradient->AddColorKey		(fire6c6, .74f );
		gradient->AddColorKey		(fire6c7, 1 );
#endif
	}

	{	// Smog1
		TMCGradient* gradient = Pointer(eGSmog1);
		TMCColorRGBA smog1c1=TMCColorRGBA(.5f, .4f, .4f, 0);
		TMCColorRGBA smog1c2=TMCColorRGBA(1, 1, 1, 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey	(smog1c1, 0);
		gradient->AddColorKey	(smog1c2, 1);
#else
		gradient->SetColorKeyColor	(0, smog1c1);
		gradient->SetColorKeyColor	(1, smog1c2);
#endif
	}
	{	// Smog2
		TMCGradient* gradient = Pointer(eGSmog2);
		TMCColorRGBA smog2c1=TMCColorRGBA(col(183), col(255), col(44), 0);
		TMCColorRGBA smog2c2=TMCColorRGBA(col(56), col(56), col(25), 0);
		TMCColorRGBA smog2c3=TMCColorRGBA(col(23), col(23), col(51), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(smog2c1, .42f );
		gradient->AddColorKey		(smog2c2, .52f );
		gradient->AddColorKey		(smog2c3, 1 );
#else
		gradient->SetColorKeyColor	(0,smog2c1);
		gradient->SetColorKeyIndex	(0, 0.42f);
		gradient->SetColorKeyColor	(1, smog2c2);
		gradient->SetColorKeyIndex	(1, 0.52f);
		gradient->AddColorKey		(smog2c3, 1 );
#endif
	}
	{	// Smog3
		TMCGradient* gradient = Pointer(eGSmog3);
		TMCColorRGBA smog3c1=TMCColorRGBA(col(0), col(0), col(0), 0);
		TMCColorRGBA smog3c2=TMCColorRGBA(col(157), col(145), col(124), 0);
		TMCColorRGBA smog3c3=TMCColorRGBA(col(206), col(195), col(176), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(smog3c1, 0 );
		gradient->AddColorKey		(smog3c2, .5f );
		gradient->AddColorKey		(smog3c3, 1 );
#else
		gradient->SetColorKeyColor	(0, smog3c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, smog3c2);
		gradient->SetColorKeyIndex	(1, 0.5);
		gradient->AddColorKey		(smog3c3, 1 );
#endif
	}
	{	// Smog4
		TMCGradient* gradient = Pointer(eGSmog4);
		TMCColorRGBA smog4c1=TMCColorRGBA(col(170), col(180), col(198), 0);
		TMCColorRGBA smog4c2=TMCColorRGBA(col(89), col(107), col(141), 0);
		TMCColorRGBA smog4c3=TMCColorRGBA(col(64), col(82), col(126), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(smog4c1, 0 );
		gradient->AddColorKey		(smog4c2, .5f );
		gradient->AddColorKey		(smog4c3, 1 );
#else
		gradient->SetColorKeyColor	(0, smog4c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, smog4c2);
		gradient->SetColorKeyIndex	(1, 0.5);
		gradient->AddColorKey		(smog4c3, 1 );
#endif
	}
	{	// Smog5
		TMCGradient* gradient = Pointer(eGSmog5);
		TMCColorRGBA smog5c1=TMCColorRGBA(col(253), col(90), col(89), 0);
		TMCColorRGBA smog5c2=TMCColorRGBA(col(255), col(181), col(97), 0);
		TMCColorRGBA smog5c3=TMCColorRGBA(col(255), col(248), col(169), 0);
		TMCColorRGBA smog5c4=TMCColorRGBA(col(255), col(254), col(241), 0);
#if (VERSIONNUMBER >= 0x040000)
		gradient->AddColorKey		(smog5c1, 0 );
		gradient->AddColorKey		(smog5c2, .34f );
		gradient->AddColorKey		(smog5c3, .7f );
		gradient->AddColorKey		(smog5c4, 1 );
#else
		gradient->SetColorKeyColor	(0, smog5c1);
		gradient->SetColorKeyIndex	(0, 0);
		gradient->SetColorKeyColor	(1, smog5c2);
		gradient->SetColorKeyIndex	(1, 0.5);
		gradient->AddColorKey		(smog5c3, 1 );
		gradient->AddColorKey		(smog5c4, 1 );
#endif
	}
}

GasBasePMap::GasBasePMap()
{
	fPrimitiveSize = 4; // halb bbox size

	fGlobalScale = 40;
	fXScale = 100;
	fYScale = 100;
	fZScale = 100;

	fXOffset = 0;
	fYOffset = 0;
	fZOffset = 0;

	fXRotation = 0;
	fYRotation = 0;
	fZRotation = 0;

	// Gas
	fSmooth = 1;
	fQuality = .2f; // 20%
	fDensityFactor = 1;
	fSelfIntensity = 1;
	
	// Noise
	fSeed = 777;
	fFractalDepth = 2;
	fRealGain = .5f;
	fRealBias = .5f;
	fCustomNoiseIndex = 1;
	GetNoiseName(fCustomNoiseIndex, fMasterShaderName);

	// 3D shape
	GetNoiseName(5, fMeshName);
	fCustomMeshIndex = 1; // 1 if the mesh name should be ignored (the name will be "None")
	fMeshSmoothing = .1f;

	fColorFlag = eNoRampOff;
	gGradientPresets->GetElem(GradientPreset::eGSmog1, fColorGradient);
	fColorCoeff = 0;
	fColorCustomShaderIndex = 1; // None
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');
	gResourceUtilities->GetIndString(fColorMasterShaderName, kStrings, 8 );
	

	fSelfShadowType = eNoSelfShadow;
	fShadowIntensity = 1;
	fDiffuseLight = 0;
	gGradientPresets->GetElem(GradientPreset::eGBlack, fShadowColorGradient);
	fFakeLightDirectionX = 0;
	fFakeLightDirectionY = 0;
	fFakeLightDirectionZ = -1;

	// Ramp params
	fRampOffFlag = eNoRampOff;
}

GasBase::GasBase()
{
	fGeneralInvalid = true;
	fNoiseInvalid = true;
	fModifiersInvalid = true;

	fContrastTable.SetElemCount(kPowTableSize);
	fBrightnessTable.SetElemCount(kPowTableSize);

	fOpticalDepth = 1.0f;
	fActualDensity = 1.0f;

	fShadowTableSize = 32;
	fHasSelfShadow = false;

	fPond1 = 0;
	fPond2 = 0;
	fPond3 = 0;

	// ShadingIn
	InitShadingIn(fShadingIn);

	SetDensityPreset('Opt3'); // Zero Edge

	// Default value for the primitive size param
	// Get the scene
	TMCCountedPtr<I3DShScene> scene;
	GetScene(&scene);
	fPMap.fPrimitiveSize = 4;
	if(scene)
	{	// Check the scene magnitude to modulate the primitive size
		real32 magnitude = scene->GetMagnitude();
		if(magnitude>0)
			fPMap.fPrimitiveSize *= magnitude;
	}

	// Critical section for multithreading
	mGasCriticalSection = NewCS();

	// Undo
//	fNeedToPost = false;
}

MCCOMErr GasBase::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualCLSID(riid, IID_I3DExNewVolumePrimitive))
	{
		TMCCountedGetHelper<I3DExNewVolumePrimitive> result(ppvObj);
		result = static_cast<I3DExNewVolumePrimitive*>(this);
		return MC_S_OK;
	}
	else if (MCIsEqualCLSID(riid, IID_IVolumePrimitive))
	{
		TMCCountedGetHelper<IVolumePrimitive> result(ppvObj);
		result = static_cast<IVolumePrimitive*>(this);
		return MC_S_OK;
	}
	else if(MCIsEqualCLSID(riid, CLSID_GasBase))
	{
		TMCCountedGetHelper<GasBase> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return TBasicPrimitive::QueryInterface(riid, ppvObj);
}



void GasBase::GetBoundingBox(TBBox3D& bbox)
{
	bbox= fPrimSize.GetBBox(fPMap.fPrimitiveSize); // kDefaultBBox;
}

MCCOMErr GasBase::SetDeformedBoundingBox(const TVector3& min, const TVector3& max)
{	// Never used ?
	return MC_S_OK;
}

MCCOMErr GasBase::ExtensionDataChanged()
{
	fGeneralInvalid = true;

	// Invalidate the modifiers for the animation
	const int32 modifCount = fModifierList.GetElemCount();
	for(int32 iModif=0 ; iModif<modifCount ; iModif++)
	{
		fModifierList[iModif]->ExtensionDataChanged();
	}
	return MC_S_OK;
}



void GasBase::Prepare3DTransform()
{
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

	fTransform.GetInverse(fInvertTransform);
}

void GasBase::InitNoise()
{
	fNoiseInvalid = false;

	// Perlin noise
	fNoise.SetSeed(fPMap.fSeed);
	fNoise.InitGradientTab();

	// Voronoi noise
// Not use yet	fVoronoi.SetSeed(fPMap.fSeed);
// Not use yet	fVoronoi.SetMaxFrequency(2*RealPow(2,fPMap.fFractalDepth));
}

void InvertValue(IShParameterComponent* component, const int32 id )
{
	real32 value= 0;
	component->GetParameter(id, (void*)&value);
	real32 invert = 1/value;
	component->SetParameter(id, (void*)&invert, true);
}

void NegateValue(IShParameterComponent* component, const int32 id )
{
	real32 value= 0;
	component->GetParameter(id, (void*)&value);
	real32 invert = -value;
	component->SetParameter(id, (void*)&invert, true);
}

void Negate100Value(IShParameterComponent* component, const int32 id )
{
	real32 value= 0;
	component->GetParameter(id, (void*)&value);
	real32 invert = -value+2;
	component->SetParameter(id, (void*)&invert, true);
}

void Offset180Value(IShParameterComponent* component, const int32 id )
{
	real32 value= 0;
	component->GetParameter(id, (void*)&value);
	real32 invert = value+180;
	if(invert>180)
		invert-=360;
	component->SetParameter(id, (void*)&invert, true);
}

void GasBase::InitModifiers()
{
	fModifiersInvalid = false;

	// Note the deformers applyed on the gas create a modification inverse from
	// the one visible on a facet mesh (we move a point in a 3D function space),
	// Since nothing exist to compute the inverse deformation, we have to get the
	// param responsible of the orientation of the deformation and flip it. And this 
	// for every deformer type. To keep the real deformers clean, we'll make the
	// changes on a clone list.

	// Extract the list of Modifiers
	fModifierList.ArrayFree();

	TMCCountedPtrArray<IShParameterComponent>::iterator iter(fPMap.fParameterComponentList);
	for (IShParameterComponent * paramComp = iter.First() ; iter.More() ; paramComp = iter.Next())
	{
		TMCCountedPtr<BasicVolumeModifier> modifier;
		paramComp->QueryInterface(CLSID_VolumeModifier, (void **)&modifier);
		if(modifier)
			fModifierList.AddElem(modifier);
	}
}

void GasBase::PreProcess()
{
	fGeneralInvalid = false;

	fNoise.SetMaxFrequency(2*RealPow(2,fPMap.fFractalDepth));

	// transform 
	Prepare3DTransform();

	// Pow table
	const real32 contrastFactor = 1.0f/(real32)kPowTableSizeMinusOne;
	const real32 brightnessFactor = 1.0f/(real32)kPowTableSizeMinusOne;
	fPMap.fRealBias = MC_Clamp(fPMap.fRealBias, kRealEpsilon, kRealOne-kRealEpsilon);
	for(int32 i=0 ; i<kPowTableSize ; i++)
	{
		fContrastTable[i] = Gain(fPMap.fRealGain,contrastFactor*(real32)i);
		fBrightnessTable[i] = Bias(fPMap.fRealBias,brightnessFactor*(real32)i);
	}

	if(fPMap.fSelfIntensity > kRealEpsilon)
		fOpticalDepth = 1/fPMap.fSelfIntensity;
	else
		fOpticalDepth = 0;

	// Ponderators
	fPond1 = 1 + fPMap.fColorCoeff*(-3+2*fPMap.fColorCoeff);
	fPond2 = fPMap.fColorCoeff*(-1+2*fPMap.fColorCoeff);
	fPond3 = fPMap.fColorCoeff*(4-4*fPMap.fColorCoeff);

	// It looks better when scaled. Plus the cube we're working in is a 2by2 cube, it could make sense.
	fActualDensity = 2*fPMap.fDensityFactor;

	GetMasterShader();

	// Finish the preprocess before building the shadow tables
	CustomPreProcess();

	fHasSelfShadow = (fPMap.fSelfShadowType!=eNoSelfShadow);

	fShadowTableSize = MC_Min( kShadowMaxTableSize, (int32)(fPMap.fQuality*50.0) );

	if(fHasSelfShadow)
	{
		// Init the scene lights
		if(fPMap.fSelfShadowType==eDistantSelfShadow)
		{	// get the scene distant lights
			FillInLightInfoArray(fLightInfo);
		}
		else if(fPMap.fSelfShadowType==eFakeSelfShadow)
		{	// make a fake distant light
			fLightInfo.SetElemCount(1);
			
			fLightInfo[0].fIsInfinite = true;
			fLightInfo[0].fPos = TVector3::kZero;
			fLightInfo[0].fDirection.x = fPMap.fFakeLightDirectionX;
			fLightInfo[0].fDirection.y = fPMap.fFakeLightDirectionY;
			fLightInfo[0].fDirection.z = fPMap.fFakeLightDirectionZ;
			fLightInfo[0].fDirection.Normalize();
			fLightInfo[0].fIntensityColor = TMCColorRGBA::kWhiteNoAlpha;
		}
		else // other cases are not handle
		{
			MCNotify("Unknown option");
			fHasSelfShadow = false;
			return;
		}

		// Init the shadow table
		fHasSelfShadow = false; // test
		InitSelfShadowTable();
		fHasSelfShadow = true; // test
	}

	// Prepare the mesh for the 3D shape if one is selected
	if(!UseMesh())
		mFacetGenerator.Initialize(NULL);
	else
	{	// Find the tree elem 
		// Get the scene
		TMCCountedPtr<I3DShScene> scene;
		GetScene(&scene);

		TMCCountedPtr<I3DShTreeElement> treeElem;
		scene->GetTreeElementByName(&treeElem, fPMap.fMeshName);
		if(treeElem)
		{
			TMCCountedPtr<I3DShInstance> instance;
			treeElem->QueryInterface(IID_I3DShInstance, (void **)&instance);

			if(instance && instance->GetInstanceKind() == I3DShInstance::kPrimitiveInstance)
			{
				// WARNING: maybe a ref counting problem ?
				mFacetGenerator.Initialize( instance /*instance->GetRenderingFacetMesh()*/ );
				mFacetGenerator.SetSmoothing(fPMap.fMeshSmoothing);
			}
		}
	}
}

void GasBase::ApplyModifiers(TVector3& point)
{
	const int32 modifCount = fModifierList.GetElemCount();
	for(int32 iModif=0 ; iModif<modifCount ; iModif++)
	{
		TVector3 result = point;
		fModifierList[iModif]->DeformPoint(point, result);
		point = result;
	}
}

void GasBase::GetMasterShader()
{
	if(fPMap.fCustomNoiseIndex && fPMap.fColorCustomShaderIndex)
	{
		fShader = NULL;
		fMasterShader = NULL;
		fColorShader = NULL;
		fColorMasterShader = NULL;
		return;
	}

	TMCCountedPtr<I3DShScene> scene;
	GetScene(&scene);

	// Get the master shader
	if(!fPMap.fCustomNoiseIndex && fPMap.fMasterShaderName.Length())
	{
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

	if(!fPMap.fColorCustomShaderIndex && fPMap.fColorMasterShaderName.Length())
	{
		scene->GetMasterShaderByName(&fColorMasterShader, fPMap.fColorMasterShaderName);

		if(fColorMasterShader)
		{
			fColorMasterShader->GetShader(&fColorShader);			
		}
	}
	else
	{
		fColorShader = NULL;
		fColorMasterShader = NULL;
	}
}

real32 GasBase::GetGasValue(const TVector3& scaledPoint)
{
	real32 value = 0;

	if(fShader||fColorShader)
	{
		// Do the init here for the 2 cases
		fShadingIn.fPoint = fShadingIn.fPointLoc = scaledPoint;
	}

	switch(fPMap.fCustomNoiseIndex)
	{
	case 0:
		{
			if(MCVerify(fShader))
			{
				boolean fullArea=false;
				fShader->GetValue(value,fullArea,fShadingIn);
			}
		}  break;
	case 1:
		{
			// 1.5 to give more amplitude
			value = 1.5*fNoise.GetPlasma(scaledPoint, .5f, .002f); // .5<=>roughness in Veloute
		} break;
	case 2:
		{
			// Use the build in plasma
			value = 1.5*fNoise.GetTurbulence(scaledPoint, .5f, .002f); // .5<=>roughness in Veloute
		} break;
	case 3: // fire
		{

			value = fNoise.RidgedMultifractal(scaledPoint, .002f);

//			// Use the build in plasma
//			value =  0.5+ .5*fNoise.GetPlasma(scaledPoint, .5f, .002f); // .5<=>roughness in Veloute
		} break;
	case 4:
		{
			// Use the build in noise
			// .6 to give more amplitude
			value =  0.5+ .6*fNoise.GetSum(scaledPoint, .5f, .002f); // .5<=>roughness in Veloute
		} break;
	}

// Note: Bias and Gain also use pow functions: do we need them all ?
	// Reshape: use a value store in the table to avoid calling the pow function each time
	value = MC_Clamp(value, kRealZero, kRealOne);

	// Brightness and contrast
	// Note: it's important to clamp before using these tables
	value = fContrastTable[(int32)(value*kPowTableSizeMinusOne)];
	value = fBrightnessTable[(int32)(value*kPowTableSizeMinusOne)];

	return value;
}

void GasBase::CheckShadowTable()
{
	if(fPMap.fSelfShadowType==eDistantSelfShadow)
	{
		boolean recomputeShadow=false;

		TMCCountedPtr<I3DShScene> scene;
		GetScene(&scene);

		const int32 lightCount = scene->GetLightsourcesCount();
		int32 lightInfoCount=0;

		for (int32 iLight=0 ; iLight<lightCount && !recomputeShadow ; iLight++)
		{
			TMCCountedPtr<I3DShLightsource> lightSource;
			scene->GetLightsourceByIndex(&lightSource,iLight);

			TStandardLight lightFlag;
			lightSource->GetStandardLight(lightFlag);

			if(lightFlag.fLightType!=TStandardLight::kDistantLight)
				continue; // just use the distant lights

//			if(!lightSource->IsVisibleInPerspective())
//				continue; // light not visible

			if(lightInfoCount<(int32)fLightInfo.GetElemCount())
			{
				LightInfo& info = fLightInfo[lightInfoCount++];
				real32 resultDistance=0;
				TVector3 dir;
				lightSource->GetDirection(TVector3::kZero, dir, resultDistance);

				if(info.fDirection != -dir)
				{
					recomputeShadow = true;
				}
				// 
				real32 ShadowIntensity=0;
				TMCColorRGBA intensityColor;
				lightSource->GetColor(TVector3::kZero,dir,resultDistance, intensityColor, ShadowIntensity);
				if( info.fIntensityColor.red != intensityColor.red &&
					info.fIntensityColor.green != intensityColor.green &&
					info.fIntensityColor.blue != intensityColor.blue )
				{
					recomputeShadow = true;
				}
			}
			else
			{
				recomputeShadow = true;
			}
		}
		
		if(recomputeShadow)
		{
			// get the scene distant lights
			FillInLightInfoArray(fLightInfo);

			// Init the shadow table
			InitSelfShadowTable();
		}
	}
}

MCCOMErr GasBase::GetVolumeDensity(const TVector3& point,
								   TMCColorRGBA& filter,
								   TMCColorRGBA& glow)
{
	// If GetVolumeAttenuation is implemented, we don't need to implement this one
	return MC_E_NOTIMPL;
}

const TVector3 test(1000,1000,1000);
void GasBase::GetNoisedPoint(const TVector3& currentPoint, 
							 const real32 amplitude,
							 TVector3& outPoint)
{
	const TVector3 scaledPos(currentPoint.x*1000,currentPoint.y*1000,currentPoint.z*1000);
	const real32 noise = amplitude*fNoise.GetValueLinear(scaledPos); // a value between -1 and 1
	// An artificial way to build 3 noise value with 1
	const real32 noiseX = sin(scaledPos.x)*noise;
	const real32 noiseY = sin(scaledPos.y)*noise;
	const real32 noiseZ = sin(scaledPos.z)*noise;

	outPoint.x = currentPoint.x+noiseX;
	outPoint.y = currentPoint.y+noiseY;
	outPoint.z = currentPoint.z+noiseZ;
}

void GasBase::IncrementColorAndOpacity(const TVector3& currentPoint, const real32 stepLength,
										 real32& totalOpacity, real32& densitySum,
										 TMCColorRGBA& finalColor )
{
	TMCColorRGBA gasColor;
	real32 localDensity = 0;
	if(IsDemoVersion())
	{	// Demo version
		TVector3 sphereCenter1(.7f, .7f, .7f);
		TVector3 sphereCenter2(-.7f, -.7f, -.7f);
		const real32 radius=.3f;
		const real32 coeff = 1/radius;
		TVector3 distToCenter1 = currentPoint-sphereCenter1;
		TVector3 distToCenter2 = currentPoint-sphereCenter2;
		real32 value1 = radius - distToCenter1.GetMagnitude();
		real32 value2 = radius - distToCenter2.GetMagnitude();
		if(value1>0)
		{
			localDensity = RealSqrt(coeff*value1);
			gasColor.Set(1, .2f, .2f, 1);
		}
		else if(value2>0)
		{
			localDensity = RealSqrt(coeff*value2);
			gasColor.Set(.2f, 1, .2f, 1);
		}
		else
		{
			localDensity = GetLocalDensity(currentPoint);
			GetGasColor(gasColor, localDensity, currentPoint );
		}
	}
	else
	{	// Normal version
		localDensity = GetLocalDensity(currentPoint);
		GetGasColor(gasColor, localDensity, currentPoint );
	}

	const real32 density = fActualDensity*stepLength*localDensity;

	if(density>kRealEpsilon)
	{
		// Compute the local opacity
		real32 zero = 0;
		gasColor.alpha = GetTotalOpacity(density,zero);

		// Total opacity
		totalOpacity = GetTotalOpacity(density, densitySum);

		real32 transparency = 1-totalOpacity;
		real32 coeff = density*transparency;

		// Alpha for multiple primitives
		finalColor.alpha += coeff;

		// Local Lighting
		// lightIntensity could be a light color (sum(intensityColors))
		
		if(fHasSelfShadow)
		{
			real32 lightIntensity = InterpolateIntensity(currentPoint);

			// Simulate a diffuse light: shadow is lighter when density ( localDensity ) is low
			const real32 shadowDiffused = fPMap.fShadowIntensity*MC_Clamp( (1+fPMap.fDiffuseLight*(localDensity-1)), kRealZero, kRealOne );

			// Shadow intensity factor ( 0 to 100% )
			lightIntensity = 1 + (lightIntensity-1)*shadowDiffused;
			
			// Shadow Color
			const real32 shadowIntensity = 1-lightIntensity;
			TMCColorRGBA shadowColor = TMCColorRGBA::kBlackNoAlpha;
			fPMap.fShadowColorGradient.GetColor(shadowIntensity, shadowColor );
			real32 shadowCoeff = coeff*shadowIntensity;

			coeff *= lightIntensity;

			finalColor.red += coeff*gasColor.red + shadowCoeff*shadowColor.red;
			finalColor.green += coeff*gasColor.green + shadowCoeff*shadowColor.green;
			finalColor.blue += coeff*gasColor.blue + shadowCoeff*shadowColor.blue;
		}
		else
		{	// No selfshadow: just add the color
			finalColor.red += coeff*gasColor.red;
			finalColor.green += coeff*gasColor.green;
			finalColor.blue += coeff*gasColor.blue;
		}
	}
}

const inline real32 GasBase::NoisedStepLength(const real32 stepLength, const real32 remainingLength, const TVector3& largePos) 
{
	return ( MC_Clamp( (real32)
		( stepLength*(1+fPMap.fSmooth*fNoise.GetValueLinear(largePos) ) ), kRealZero, remainingLength) );
}

void GasBase::BeginRender()
{
}
void GasBase::EndRender()
{
}
void GasBase::TraceRay(const Ray3D&			ray,
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
						boolean					oldShadowFiltering)
{
	TVector3 from = localRay.fOrigin + tmin * localRay.fDirection;
	TVector3 to = localRay.fOrigin + tmax * localRay.fDirection;

	GetVolumeAttenuation(from, to, inOutColor, linearEffect, true, oldShadowFiltering);

	// LinearEffect allow to draw primitives one over the other
	if( linearEffect )
	{
		linearEffect->fColorMult.R = 0;
		linearEffect->fColorMult.G = 0;
		linearEffect->fColorMult.B = 0;

		linearEffect->fColorAdd = inOutColor;
	}
}



bool GetIntersection( double fDst1, double fDst2, const TVector3& Origin, const TVector3& Dir, double& dist, TVector3 &Hit)
{
	double diff = (fDst2-fDst1);
	if( RealAbs(diff)<kRealEpsilon )
		return false;
	dist = -fDst1/(fDst2-fDst1);
	Hit = Origin + dist * Dir;
	return true;
}

bool InBox( const TVector3& Hit, const TVector3& B1, const TVector3& B2, const int Axis) {
if ( Axis==0 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) return true;
if ( Axis==1 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x) return true;
if ( Axis==2 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y) return true;
return false;
}

bool GetIntersectionAndCount( const TVector3& B1, const TVector3& B2, const TVector3& Origin, const TVector3& Dir, int axis, bool flip, 
							 int& intersectionCount, TVector3& Hit1, TVector3& Hit2, double& dist1, double& dist2 )
{
	TVector3& Hit = (intersectionCount==0?Hit1:Hit2);
	double& dist = (intersectionCount==0?dist1:dist2);
	const TVector3& B = flip?B2:B1;

	const TVector3& L1 = Origin;
	const TVector3 L2 = Origin+Dir;
	if ( GetIntersection( L1[axis]-B[axis], L2[axis]-B[axis], Origin, Dir, dist ,Hit) && InBox( Hit, B1, B2, axis ) )
	{
		intersectionCount++;
		if( intersectionCount==2 )
			return true; // stop looking for intersection
	}

	return false;
}


// returns true if line (L1, L2) intersects with the box (B1, B2)
// returns intersection points in Hit1 and Hit2
bool GetLineBBoxIntersections( const TVector3& B1, const TVector3& B2, const TVector3& Origin, const TVector3& Dir, 
							  TVector3& Hit1, TVector3& Hit2,double& dist1, double& dist2)
{
	int intersectionCount = 0;
	for( int iAxis=0 ; iAxis<3 ; iAxis++ )
	{
		if( GetIntersectionAndCount( B1, B2, Origin, Dir, iAxis, false, intersectionCount, Hit1, Hit2, dist1, dist2 ) )
			return true;
		if( GetIntersectionAndCount( B1, B2, Origin, Dir, iAxis, true, intersectionCount, Hit1, Hit2, dist1, dist2 ) )
			return true;
	}
	return false;
}

MCCOMErr GasBase::GetVolumeAttenuation(const TVector3& from,
									   const TVector3& to,
									   TMCColorRGBA& attenuation,
									   TColorRGBLinearEffect* linearEffect, 
									   boolean isShadowCasting, 
									   boolean oldShadowFiltering)
{
	if(IsLocked())
		return MC_S_OK;

	// There's a crash on multiproc plateforms. I don't know where it comes from, but this might help
	CWhileInCS cs(mGasCriticalSection);

	// from is the entrance point in the BBox
	// to is the exit point
	// attenuation is the original color

	// We need to modify the color to simulate the presence of a material

	if(fModifiersInvalid)
		InitModifiers();
	if(fNoiseInvalid)
		InitNoise();
	if(fGeneralInvalid)
		PreProcess();

	CheckShadowTable(); // when  lights are animated, we need to recompute the shadow table

	GetMasterShader(); // To have a real time update: there'may be a more efficient solution 

	// Volume Rendering:
	// for each increment along the ray
	//	get color, density and opacity of this element
	//	if SelefShadow
	//		retrieve the shadow for this element from the Shadow Table
	//	color = illumination of the gass using density, opacity, and a model
	//	finalColor += color
	//	sumDensity += density
	//	if transparency<.01
	//		stop
	//	increment sample point
	// create the a_buffer fragment

	// From the 8X8X8 bbox to the 2X2X2, both centered in 0
	const TVector3 actualStart = 2*fPrimSize.GetBBoxInvSize(fPMap.fPrimitiveSize)*(from);
	const TVector3 actualEnd   = 2*fPrimSize.GetBBoxInvSize(fPMap.fPrimitiveSize)*(to);

	// Direction of the ray
	const TVector3 vector = actualEnd-actualStart;
	TVector3 unit;
	const real32 thickness = vector.Normalize(unit);

	const real32 stepLength = MC_Clamp( (real32)(fPMap.fQuality>=.01?2/(fPMap.fQuality*100):2), kRealEpsilon, kRealTwo);
	const real32 halfStepLength = .5*stepLength;
	const TVector3 step = stepLength*unit;

	real32 densitySum = 0; // -log(1-attenuation.alpha)/fOpticalDepth; // 0
	real32 totalOpacity = 0; // attenuation.alpha; // 0

	real32 realStepCount = (thickness/stepLength);
	TMCColorRGBA finalColor = TMCColorRGBA::kBlackNoAlpha;


	// Ray trace the mesh
	if(UseMesh())
	{
		// Find a point outside of the bbox to initialize the ray that will hit the mesh
		// The primitive bbox is 2x2x2 centered on 0.
		// we get the intersection with a slightly bigger bbox to be sure to have a ray starting from outside the primitive
		TVector3 bound = 1.5 * TVector3::kOnes;
		const TBBox3D bbox(-bound,bound);
		TVector3 Hit1;
		TVector3 Hit2;
		double dist1;
		double dist2;
		if( GetLineBBoxIntersections( bbox.fMin, bbox.fMax, actualStart, unit, Hit1, Hit2, dist1, dist2) )
		{
			if( dist1<dist2 )
				mFacetGenerator.RecordHits(Hit1, unit); 
			else
				mFacetGenerator.RecordHits(Hit2, unit);
		}
		else
		{ // Shouldn't occured: we didn't find the 2 intersection points with the bbox
			mFacetGenerator.RecordHits(actualStart, unit);
		}
	}

	{
		TVector3 currentPoint = actualStart;

		if(fPMap.fSmooth>0)
		{	// If all the rays start from the edge of the bbox, lines will appears
			// inside the volume. Let's start from a certain distance
			// We scale the point to have a kind of Random value
			const real32 firstStep = MC_Min( thickness,NoisedStepLength(stepLength, thickness, 1000.0*currentPoint) );

			currentPoint += .5*firstStep*unit;
			// Smaller density
			IncrementColorAndOpacity( currentPoint, firstStep, 
				totalOpacity, densitySum, finalColor );

			currentPoint += .5*firstStep*unit;

			realStepCount = ((thickness-firstStep)/stepLength);
		//	realStepCount -= firstStep;
		}

		currentPoint += halfStepLength*step;

		const int32 stepCount = (int32)(realStepCount);
		const real32 rest = realStepCount-stepCount;

		for( int32 iStep=0 ; iStep<stepCount ; )
		{
			// Local density
			IncrementColorAndOpacity( currentPoint, stepLength, 
				totalOpacity, densitySum, finalColor );

			if(totalOpacity>=0.99)
			{	// Gas is totally opac
				attenuation = finalColor;
				attenuation.alpha = 1; // totaly opaque
				return MC_S_OK;
			}

			// Go to next point
			if(++iStep<stepCount)
				currentPoint += step; // step = stepLength*unit
			else
				currentPoint += .5*step; // step = stepLength*unit
		}
		// Remaining partial step
		if(rest>kRealEpsilon)
		{
			//add the missing opacity
			const real32 restHalfLength = .5*stepLength*rest;
			currentPoint += restHalfLength*unit;
		
			// Smaller density
			IncrementColorAndOpacity( currentPoint, 2*restHalfLength, 
				totalOpacity, densitySum, finalColor );
		}
	}

//	finalColor.alpha = densitySum + attenuation.alpha;
//	finalColor.alpha =  MC_Clamp(totalOpacity,0,1);

	if(totalOpacity<1)
	{	// Gas is not totaly opac
		// Use the background color
		const real32 maxValue = totalOpacity;
		const real32 coeff = (1-maxValue);

		finalColor.red		+= coeff*attenuation.red;
		finalColor.green	+= coeff*attenuation.green;
		finalColor.blue		+= coeff*attenuation.blue;	
		finalColor.alpha	+= coeff*attenuation.alpha;
	}
	else
		finalColor.alpha = 1;

	attenuation = finalColor;

	return MC_S_OK;
}

void GasBase::GetGasColor(TMCColorRGBA&		gasColor,
				 const real32		localDensity,
				 const TVector3&	pos)
{
	if( fPMap.fColorCustomShaderIndex == 0 )
	{	// Special case: use a shader from the scene
		if(MCVerify(fColorShader))
		{
			boolean fullArea=false;
			fColorShader->GetColor(gasColor,fullArea,fShadingIn); // fShadingIn should already be initialized in GetGasValue
		}
	}
	else
	{	// regular case: use the gradient and the rampoff
//		const real32 rampOff = RampOff(pos, fPMap.fColorFlag, fPMap.fColorRamps, false);

		const real32 coeff = MC_Clamp( localDensity*fPond1 + fLastRampOff*fPond2 + localDensity*fLastRampOff*fPond3, kRealZero, kRealOne );
		fPMap.fColorGradient.GetColor(coeff, gasColor );
	}
}

// Ramp of the value to get a smooth transition between the gas part and the no gas part
// point is the coordinates in the -1,1 space
real32 GasBase::RampOff(const TVector3& point,
						const int32 rampFlag,
						const RampOffFactors& factors,
						const boolean smoothRamp) 
{
	// If we're using a mesh, get it's rampoff
	fLastRampOff = UseMesh()?mFacetGenerator.Density(point):1;

	// Many rampoff can occured at the same time, test them all

	// In all cases, cut what outside the box (for the modifiers to work properly)
	if(point.x<-kOneEpsilon) { fLastRampOff = 0; return 0; }
	if(point.y<-kOneEpsilon) { fLastRampOff = 0; return 0; }
	if(point.z<-kOneEpsilon) { fLastRampOff = 0; return 0; }
	if(point.x>kOneEpsilon) { fLastRampOff = 0; return 0; }
	if(point.y>kOneEpsilon) { fLastRampOff = 0; return 0; }
	if(point.z>kOneEpsilon) { fLastRampOff = 0; return 0; }

	if(point.z>=0)
	{
		if(rampFlag&ePosHalfSphereUp)
		{	// Spherical ramp off to do, get the squared dist to the center
			const real32 distSqr = point.GetMagnitudeSquared();
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (1 - SmootherStep( (1-factors.fPosHalfSphereUp.y), (1-factors.fPosHalfSphereUp.x), dist) );
				if(smoothRamp)
					fLastRampOff *= (BlenderInvertStep( (1-factors.fPosHalfSphereUp.y), (1-factors.fPosHalfSphereUp.x), dist) );
				else
					fLastRampOff *= (LinearInvertStep( (1-factors.fPosHalfSphereUp.y), (1-factors.fPosHalfSphereUp.x), dist) );
			}
			else { fLastRampOff = 0; return 0; }; // outside the sphere
		}
		else if(rampFlag&eNegHalfSphereUp)
		{
			const real32 distSqr = point.GetMagnitudeSquared();
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (SmootherStep( factors.fNegHalfSphereUp.y, factors.fNegHalfSphereUp.x, dist) );
				if(smoothRamp)
					fLastRampOff *= (1-BlenderInvertStep( factors.fNegHalfSphereUp.y, factors.fNegHalfSphereUp.x, dist) );
				else
					fLastRampOff *= (1-LinearInvertStep( factors.fNegHalfSphereUp.y, factors.fNegHalfSphereUp.x, dist) );
			}
			// else ramp is 1
		}
		else if(rampFlag&ePosZCylinder)
		{	// Cylindrical ramp off to do, get the squared dist to the center
			const real32 distSqr = point.x*point.x + point.y*point.y;
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (1 - SmootherStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
				if(smoothRamp)
					fLastRampOff *= (BlenderInvertStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
				else
					fLastRampOff *= (LinearInvertStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
			}
			else { fLastRampOff = 0; return 0; }; // outside the cylinder
		}
	}

	if(point.z<0)
	{
		if(rampFlag&ePosHalfSphereDown)
		{	// Spherical ramp off to do, get the squared dist to the center
			const real32 distSqr = point.GetMagnitudeSquared();
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (1 - SmootherStep( (1-factors.fPosHalfSphereDown.y), (1-factors.fPosHalfSphereDown.x), dist) );
				if(smoothRamp)
					fLastRampOff *= (BlenderInvertStep( (1-factors.fPosHalfSphereDown.y), (1-factors.fPosHalfSphereDown.x), dist) );
				else
					fLastRampOff *= (LinearInvertStep( (1-factors.fPosHalfSphereDown.y), (1-factors.fPosHalfSphereDown.x), dist) );
			}
			else { fLastRampOff = 0; return 0; }; // outside the sphere
		}
		else if(rampFlag&eNegHalfSphereDown)
		{	// Spherical ramp off to do, get the squared dist to the center
			const real32 distSqr = point.GetMagnitudeSquared();
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (SmootherStep( factors.fNegHalfSphereDown.y, factors.fNegHalfSphereDown.x, dist) );
				if(smoothRamp)
					fLastRampOff *= (1-BlenderInvertStep( factors.fNegHalfSphereDown.y, factors.fNegHalfSphereDown.x, dist) );
				else
					fLastRampOff *= (1-LinearInvertStep( factors.fNegHalfSphereDown.y, factors.fNegHalfSphereDown.x, dist) );
			}
			// else ramp is 1
		}
		else if(rampFlag&ePosZCylinder)
		{	// Cylindrical ramp off to do, get the squared dist to the center
			const real32 distSqr = point.x*point.x + point.y*point.y;
			if(distSqr<1)
			{
				const real32 dist = RealSqrt(distSqr);
//				ramp *= (1 - SmootherStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
				if(smoothRamp)
					fLastRampOff *= (BlenderInvertStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
				else
					fLastRampOff *= (LinearInvertStep( (1-factors.fPosZCylinder.y), (1-factors.fPosZCylinder.x), dist) );
			}
			else { fLastRampOff = 0; return 0; } // outside the cylinder
		}
	}

	if(rampFlag&ePosXCylinder)
	{	// Cylindrical ramp off to do, get the squared dist to the center
		const real32 distSqr = point.y*point.y + point.z*point.z;
		if(distSqr<1)
		{
			const real32 dist = RealSqrt(distSqr);
//			ramp *= (1 - SmootherStep( (1-factors.fPosXCylinder.y), (1-factors.fPosXCylinder.x), dist) );
			if(smoothRamp)
				fLastRampOff *= (BlenderInvertStep( (1-factors.fPosXCylinder.y), (1-factors.fPosXCylinder.x), dist) );
			else
				fLastRampOff *= (LinearInvertStep( (1-factors.fPosXCylinder.y), (1-factors.fPosXCylinder.x), dist) );
		}
		else { fLastRampOff = 0; return 0; } // outside the cylinder
	}

	if(rampFlag&ePosYCylinder)
	{	// Cylindrical ramp off to do, get the squared dist to the center
		const real32 distSqr = point.x*point.x + point.z*point.z;
		if(distSqr<1)
		{
			const real32 dist = RealSqrt(distSqr);
//			ramp *= (1 - SmootherStep( (1-factors.fPosYCylinder.y), (1-factors.fPosYCylinder.x), dist) );
			if(smoothRamp)
				fLastRampOff *= (BlenderInvertStep( (1-factors.fPosYCylinder.y), (1-factors.fPosYCylinder.x), dist) );
			else
				fLastRampOff *= (LinearInvertStep( (1-factors.fPosYCylinder.y), (1-factors.fPosYCylinder.x), dist) );
		}
		else { fLastRampOff = 0; return 0; } // outside the cylinder
	}

	if(rampFlag&eNegXCylinder)
	{	// Cylindrical ramp off to do, get the squared dist to the center
		const real32 distSqr = point.y*point.y + point.z*point.z;
		if(distSqr<1)
		{
			const real32 dist = RealSqrt(distSqr);
//			ramp *= (SmootherStep( (1-factors.fNegXCylinder.y), (1-factors.fNegXCylinder.x), dist) );
			if(smoothRamp)
				fLastRampOff *= (1-BlenderInvertStep( (1-factors.fNegXCylinder.y), (1-factors.fNegXCylinder.x), dist) );
			else
				fLastRampOff *= (1-LinearInvertStep( (1-factors.fNegXCylinder.y), (1-factors.fNegXCylinder.x), dist) );
		}
		// else ramp is 1
	}

	if(rampFlag&eNegYCylinder)
	{	// Cylindrical ramp off to do, get the squared dist to the center
		const real32 distSqr = point.x*point.x + point.z*point.z;
		if(distSqr<1)
		{
			const real32 dist = RealSqrt(distSqr);
//			ramp *= (SmootherStep( (1-factors.fNegYCylinder.y), (1-factors.fNegYCylinder.x), dist) );
			if(smoothRamp)
				fLastRampOff *= (1-BlenderInvertStep( (1-factors.fNegYCylinder.y), (1-factors.fNegYCylinder.x), dist) );
			else
				fLastRampOff *= (1-LinearInvertStep( (1-factors.fNegYCylinder.y), (1-factors.fNegYCylinder.x), dist) );
		}
		// else ramp is 1
	}

	if(rampFlag&eNegZCylinder)
	{	// Cylindrical ramp off to do, get the squared dist to the center
		const real32 distSqr = point.x*point.x + point.y*point.y;
		if(distSqr<1)
		{
			const real32 dist = RealSqrt(distSqr);
//			ramp *= (SmootherStep( (1-factors.fNegZCylinder.y), (1-factors.fNegZCylinder.x), dist) );
			if(smoothRamp)
				fLastRampOff *= (1-BlenderInvertStep( (1-factors.fNegZCylinder.y), (1-factors.fNegZCylinder.x), dist) );
			else
				fLastRampOff *= (1-LinearInvertStep( (1-factors.fNegZCylinder.y), (1-factors.fNegZCylinder.x), dist) );
		}
		// else ramp is 1
	}

	if(rampFlag&ePosXDist)
	{
		const real32 dist = .5+.5*point.x;
//		ramp *= (1-SmootherStep( (1-factors.fPosXDist.y), (1-factors.fPosXDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= BlenderInvertStep( (1-factors.fPosXDist.y), (1-factors.fPosXDist.x), dist);
		else
			fLastRampOff *= LinearInvertStep( (1-factors.fPosXDist.y), (1-factors.fPosXDist.x), dist);
	}

	if(rampFlag&eNegXDist)
	{
		const real32 dist = .5-.5*point.x;
//		ramp *= (1-SmootherStep( (1-factors.fNegXDist.y), (1-factors.fNegXDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= (BlenderInvertStep( (1-factors.fNegXDist.y), (1-factors.fNegXDist.x), dist) );
		else
			fLastRampOff *= (LinearInvertStep( (1-factors.fNegXDist.y), (1-factors.fNegXDist.x), dist) );
	}

	if(rampFlag&ePosYDist)
	{
		const real32 dist = .5+.5*point.y;
//		ramp *= (1-SmootherStep( (1-factors.fPosYDist.y), (1-factors.fPosYDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= (BlenderInvertStep( (1-factors.fPosYDist.y), (1-factors.fPosYDist.x), dist) );
		else
			fLastRampOff *= (LinearInvertStep( (1-factors.fPosYDist.y), (1-factors.fPosYDist.x), dist) );
	}

	if(rampFlag&eNegYDist)
	{
		const real32 dist = .5-.5*point.y;
//		ramp *= (1-SmootherStep( (1-factors.fNegYDist.y), (1-factors.fNegYDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= (BlenderInvertStep( (1-factors.fNegYDist.y), (1-factors.fNegYDist.x), dist) );
		else
			fLastRampOff *= (LinearInvertStep( (1-factors.fNegYDist.y), (1-factors.fNegYDist.x), dist) );
	}

	if(rampFlag&ePosZDist)
	{
		const real32 dist = .5+.5*point.z;
//		ramp *= (1-SmootherStep( (1-factors.fPosZDist.y), (1-factors.fPosZDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= (BlenderInvertStep( (1-factors.fPosZDist.y), (1-factors.fPosZDist.x), dist) );
		else
			fLastRampOff *= (LinearInvertStep( (1-factors.fPosZDist.y), (1-factors.fPosZDist.x), dist) );
	}

	if(rampFlag&eNegZDist)
	{
		const real32 dist = .5-.5*point.z;
//		ramp *= (1-SmootherStep( (1-factors.fNegZDist.y), (1-factors.fNegZDist.x), dist) );
		if(smoothRamp)
			fLastRampOff *= (BlenderInvertStep( (1-factors.fNegZDist.y), (1-factors.fNegZDist.x), dist) );
		else
			fLastRampOff *= (LinearInvertStep( (1-factors.fNegZDist.y), (1-factors.fNegZDist.x), dist) );
	}

	/*	if(rampFlag&eZNegRampOff && fPMap.fZNegRamp>0)
	{
		const real32 dist = .5-.5*point.z;
		if(dist>fPMap.fZNegRamp)
		{
			ramp*=(1-SmoothStep(fPMap.fZNegRamp,1,dist));
		}
	}*/

	return fLastRampOff;
}

real32 GasBase::GetTotalOpacity(const real32 volumeDensity,
								real32& densitySum)
{
	densitySum += volumeDensity;

	return 1.0f -  exp( - fOpticalDepth*densitySum );
}



void GasBase::InitSelfShadowTable()
{
	// Progress Bar
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'Fire');
	TMCCountedPtr<IMCUnknown> progressKey; // Progress bar key
	TMCString255 progressString;
	gResourceUtilities->GetIndString(progressString, 300, 2);
	gShellUtilities->BeginProgress(progressString, &progressKey);
	const real32 inc = 100.0f/(real32)(fShadowTableSize);

	const real32 step = 1.0f/(real32)(fShadowTableSize-1); // TO DO: cube dimension and pos

	fVolume.SetElemCount(fShadowTableSize);

	for(int32 iPlane=0 ; iPlane<fShadowTableSize ; iPlane++)
	{
		fVolume[iPlane].SetElemCount(fShadowTableSize);
	
		Plane& plane = fVolume[iPlane];

		const real32 z = -1 +2*iPlane*step;


		for(int32 iLine=0 ; iLine<fShadowTableSize ; iLine++)
		{
			plane[iLine].SetElemCount(fShadowTableSize);
		
			Line& line = plane[iLine];

			const real32 y = -1 +2*iLine*step;


			for(int32 iPoint=0 ; iPoint<fShadowTableSize ; iPoint++)
			{
				const real32 x = -1 +2*iPoint*step;

				// Compute the intensity at this point
				const TVector3 point(x,y,z);

				line[iPoint] = GetLightsIntensity(point);
			}
		}

#if (VERSIONNUMBER >= 0x040000)
		gShellThreadUtilities->YieldProcesses(15);   
#else
		gShellUtilities->YieldProcesses(15);   
#endif
		gShellUtilities->IncrementProgress(inc, progressKey);
		if (gShellUtilities->CheckForUserCancel())
		{
			gShellUtilities->EndProgress(progressKey);
			throw TMCException(kuserCanceledErr); // User cancelled
		}
	}
	
	gShellUtilities->EndProgress(progressKey);
}

// Compute the real intensity in one point (slow) to initialize the grid
// Build an intensity for one point of the grid
real32 GasBase::GetLightsIntensity(const TVector3& point)
{
	real32 totalIntensity = 0;

	const int32 lightCount = fLightInfo.GetElemCount();
	for(int32 iLight=0 ; iLight<lightCount ; iLight++)
	{
		LightInfo& light = fLightInfo[iLight];
		totalIntensity += GetLightIntensity(point, light);
	}

	return totalIntensity; // Let the intensity be greater than 1 ?
}

real32 GasBase::GetLightIntensity(const TVector3& point,
								  const LightInfo& light)
{
	// We just work with infinite lights for now, the other lights are then relative to
	// the instances.
	if(light.fIsInfinite)
	{
		const TVector3& lightDir = light.fDirection;

		TVector3 result;
		boolean has = IntersectHalfLinePlane( point, // first point of the line
								point-lightDir, // second point of the line
								lightDir.x<0?TVector3::kOnes:-TVector3::kOnes, // point on the plane
								TVector3::kUnitX, // normal to the plane
								result);

		if(!has || result.y>1 || result.z>1 || result.y<-1 || result.z<-1)
		{
			has = IntersectHalfLinePlane( point, // first point of the line
									point-lightDir, // second point of the line
									lightDir.y<0?TVector3::kOnes:-TVector3::kOnes, // point on the plane
									TVector3::kUnitY, // normal to the plane
									result);

			if(!has || result.x>1 || result.z>1 || result.x<-1 || result.z<-1)
			{
				has = IntersectHalfLinePlane( point, // first point of the line
										point-lightDir, // second point of the line
										lightDir.z<0?TVector3::kOnes:-TVector3::kOnes, // point on the plane
										TVector3::kUnitZ, // normal to the plane
										result);

			}
		}

		MCAssert(has);

		MCAssert(	result.x>=-(1+kRealEpsilon) && result.x<=(1+kRealEpsilon) &&
					result.y>=-(1+kRealEpsilon) && result.y<=(1+kRealEpsilon) &&
					result.z>=-(1+kRealEpsilon) && result.z<=(1+kRealEpsilon) );

#if 1
		// now compute the luminosity here
		const TVector3 vector = result-point;
		TVector3 unit;
		const real32 thickness = vector.Normalize(unit);

		const real32 stepLength = MC_Clamp( (real32)(fPMap.fQuality>=.01?2/(fPMap.fQuality*100):2), kRealEpsilon, kRealTwo);
		const real32 halfStepLength = .5*stepLength;
		const TVector3 step = stepLength*unit;
	
		real32 densitySum = 0;
		real32 totalOpacity = 0;

		real32 realStepCount = (thickness/stepLength);
		TVector3 currentPoint = point + halfStepLength*step;;
	
		const int32 stepCount = (int32)(realStepCount);
		const real32 rest = realStepCount-stepCount;

		for( int32 iStep=0 ; iStep<stepCount ; )
		{
			const real32 density = fActualDensity * stepLength * GetLocalDensity(currentPoint);

			if(density>kRealEpsilon)
			{
				totalOpacity = GetTotalOpacity(density, densitySum );

				real32 coeff = density*(1-totalOpacity);

				if(totalOpacity>=1)
				{	// Total opacity: no lighting
					return 0;
				}
			}

			// Go to next point
			if(++iStep<stepCount)
				currentPoint += step; // step = stepLength*unit
			else
				currentPoint += .5*step; // step = stepLength*unit
		}

		// Add the last part if one
		// Remaining partial step
		if(rest>kRealEpsilon)
		{
			//add the missing opacity
			const real32 restHalfLength = .5*stepLength*rest;
			currentPoint += restHalfLength*unit;

			const real32 density = fActualDensity * 2*restHalfLength * GetLocalDensity(currentPoint);

			totalOpacity = GetTotalOpacity(density, densitySum );
		}

		// Later: use the color instead of the intensity
		return (light.fIntensityColor.Intensity()*(1-totalOpacity)); // considere the light to be white. 
	}
#else
		// now compute the luminosity here
		const TVector3 vector = result-point;
		TVector3 unit;
		const real32 thickness = vector.Normalize(unit);

		const real32 precision = 1.0f/(real32)(fShadowTableSize-1); // TO DO: cube dimension and pos

		const TVector3 step = precision*unit;

		TVector3 currentPoint = point;
		const real32 realStepCount = thickness/precision;
		const int32 stepCount = (int32)(realStepCount);

	real32 testSum = 0;
		real32 densitySum = 0;
		real32 localOpacity = 0;
	real32 totalOpacity = 0;

		for( int32 iStep=0 ; iStep<stepCount ; iStep++ )
		{
//			const real32 density = /*fActualDensity * ?*/ precision*GetLocalDensity(currentPoint);
			const real32 density = fActualDensity * precision*GetLocalDensity(currentPoint);

			localOpacity = GetTotalOpacity(density, densitySum );

			if(localOpacity>=1)
			{	// Total opacity: no lighting
				return 0;
			}

			// Go to next point
			currentPoint += step;
		}

		// Add the last part if one
		real32 rest = realStepCount-stepCount;
		if(rest>kRealEpsilon)
		{
			// we added a little bit to much, remove
			currentPoint -= (precision-rest)*unit;;
		}

		// Later: use the color instead of the intensity
		return (light.fIntensityColor.Intensity()*(1-localOpacity)); // considere the light to be white. 
	}
#endif

	return 0;
}

// Compute a fake self shadowing based on the grid
// Do a trilinear interpolation for a point inside the one*one*one cube
real32 GasBase::InterpolateIntensity(const TVector3& point)
{
	TVector3 scaledPoint = .5*(TVector3::kOnes + point);

	if(scaledPoint.x<0)scaledPoint.x=0;
	if(scaledPoint.y<0)scaledPoint.y=0;
	if(scaledPoint.z<0)scaledPoint.z=0;
	if(scaledPoint.x>=1)scaledPoint.x=.99f;
	if(scaledPoint.y>=1)scaledPoint.y=.99f;
	if(scaledPoint.z>=1)scaledPoint.z=.99f;

	scaledPoint = (fShadowTableSize-1)*scaledPoint;

	const int32 ix = RealFloor( scaledPoint.x );
	const real32 fx0 = scaledPoint.x - ix;
	const real32 fx1 = fx0 - 1;
	const real32 wx = BoxStep( fx0 );

	const int32 iy = RealFloor( scaledPoint.y );
	const real32 fy0 = scaledPoint.y - iy;
	const real32 fy1 = fy0 - 1;
	const real32 wy = BoxStep( fy0 );

	const int32 iz = RealFloor( scaledPoint.z );
	const real32 fz0 = scaledPoint.z - iz;
	const real32 fz1 = fz0 - 1;
	const real32 wz = BoxStep( fz0 );

	real32 col_y0=0;
	real32 col_y1=0;
	real32 col_z0=0;
	real32 col_z1=0;

	const real32 col_000 = GetIntensity( ix, iy, iz );
	const real32 col_100 = GetIntensity( ix+1, iy, iz );
	col_y0 = Lerp(wx, col_000, col_100);

	const real32 col_010 = GetIntensity( ix, iy+1, iz );
	const real32 col_110 = GetIntensity( ix+1, iy+1, iz );
	col_y1 = Lerp(wx, col_010, col_110);

	col_z0 = Lerp(wy, col_y0, col_y1);

	const real32 col_001 = GetIntensity( ix, iy, iz+1 );
	const real32 col_101 = GetIntensity( ix+1, iy, iz+1 );
	col_y0 = Lerp(wx, col_001, col_101);

	const real32 col_011 = GetIntensity( ix, iy+1, iz+1 );
	const real32 col_111 = GetIntensity( ix+1, iy+1, iz+1 );
	col_y1 = Lerp(wx, col_011, col_111);

	col_z1 = Lerp(wy, col_y0, col_y1);

	return Lerp(wz, col_z0, col_z1);
/*	vx0 = GradientLattice( ix, iy, iz, fx0, fy0, fz0 );
	vx1 = GradientLattice( ix+1, iy, iz, fx1, fy0, fz0 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz, fx0, fy1, fz0 );
	vx1 = GradientLattice( ix+1, iy+1, iz, fx1, fy1, fz0 );
	vy1 = Lerp(wx, vx0, vx1);

	vz0 = Lerp(wy, vy0, vy1);

	vx0 = GradientLattice( ix, iy, iz+1, fx0, fy0, fz1 );
	vx1 = GradientLattice( ix+1, iy, iz+1, fx1, fy0, fz1 );
	vy0 = Lerp(wx, vx0, vx1);

	vx0 = GradientLattice( ix, iy+1, iz+1, fx0, fy1, fz1 );
	vx1 = GradientLattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 );
	vy1 = Lerp(wx, vx0, vx1);

	vz1 = Lerp(wy, vy0, vy1);

	return Lerp(wz, vz0, vz1);*/
}

void MCCOMAPI GasBase::ChangedData()
{
}

// Doesn't receive all the messages: MCCOMErr GasBase::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
MCCOMErr GasBase::SimpleHandleEvent(MessageID message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

//	TMCCountedPtr<IMFParameterComponentPart> compUIPart;

	switch (message)
	{
	case kMsg_CUIP_ComponentAttached:
		{	// Multi component chooser is being attached, fill it with our pmap
			if (source)
			{
				TMCCountedPtr<IMultipleComponentChooser> multiChooser;
				source->QueryInterface(IID_IMultipleComponentChooser, (void **)&multiChooser);
				if (multiChooser)
				{
					multiChooser->SetComponentList(&fPMap.fParameterComponentList);
				}
			}
/* Doesn't work
			// Init the cloned if it's not. 
			// This is just a convenient location to do it, but I'm sure it's not the 
			// best.
			if(!fClonedComponent)
			{
				TMCCountedPtr<IComponentAnim> animComponent;
				this->QueryInterface(IID_IComponentAnim, (void**) &animComponent);
				ThrowIfNil(animComponent);
				animComponent->Clone(&fClonedComponent, kWithAnim);
			}
*/
		} break;
	case EMFPartMessage::kMsg_PartValueChanged:
		{

			if (MCVerify(source))
			{
				// source->GetInstanceID()
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID = part->GetIMFPartID();

			/* Doesn't work
					// Check if its our custom part that sent a request to post an action
					if( partID!='Ctrl' && data) // don't handle the message from GasCommonPart, only the one for the pmap
					{
						IMFParameterComponentPart* compUIPart = static_cast<IMFParameterComponentPart*>(data);
						if(compUIPart)
						{	// This is a special message to fix a bug within Carrara:
							// the component was modified from the assemble room, we need 
							// to post an action to make it undoable (actions are posted 
							// only from the modeler room by default)
							fCompUIPart = compUIPart;
							
							
							fNeedToPost = true;

							return true; // we handled our custom message
						}
					}
					*/

					int32 densityRamp = -1;
			
					int32 colorRamp = -1;

					switch (partID)
					{
					case 'Ctrl':
						{	// A message from our GasCommonPart: check the data
							if(data)
							{
								const int32* dataID = static_cast<int32*>(data);
			// Hack not needed anymore, it was for the assemble room
			//					if(*dataID == EMFPartMessage::kMsg_StartFastDraw)
			//						break;

								MultiCompData* comData = static_cast<MultiCompData*>(data);
								if (comData->fCompChooser)
								{
									switch(comData->fMessage)
									{	// Multi Component Chooser modification: get the new list of components
									case kMsg_MCC_NewComponentAdded:
									case kMsg_MCC_ComponentDeleted:
									case kMsg_MCC_ComponentModified:
										{	// Update the list of component in the pmap
											comData->fCompChooser->GetComponentList(&fPMap.fParameterComponentList);
											fModifiersInvalid = true;
										} break;
									case kMsg_MCC_ComponentTracking:
										{	// realtime modif not needed
										} break;
									}

								}

								handledMessage = true;

								comData->fCompChooser = NULL;
								delete comData;
								data = comData = NULL;
							}
						} break;
				//	case 'LigX': break;// Fake light Dir X
				//	case 'LigY': break;// Fake light Dir Y
				//	case 'LigZ': break;// Fake light Dir Z
				
					// Density Ramp

					case 'DPHU': densityRamp = ePosHalfSphereUp; break;// Density ramp off: positive half sphere up
					case 'DPHD': densityRamp = ePosHalfSphereDown; break;// Density ramp off: positive half sphere up
					case 'DNHU': densityRamp = eNegHalfSphereUp; break;// Density ramp off: positive half sphere up
					case 'DNHD': densityRamp = eNegHalfSphereDown; break;// Density ramp off: positive half sphere up
				
					case 'DPXC': densityRamp = ePosXCylinder; break;// Density ramp off: positive half sphere up
					case 'DPYC': densityRamp = ePosYCylinder; break;// Density ramp off: positive half sphere up
					case 'DPZC': densityRamp = ePosZCylinder; break;// Density ramp off: positive half sphere up
					case 'DNXC': densityRamp = eNegXCylinder; break;// Density ramp off: positive half sphere up
					case 'DNYC': densityRamp = eNegYCylinder; break;// Density ramp off: positive half sphere up
					case 'DNZC': densityRamp = eNegZCylinder; break;// Density ramp off: positive half sphere up
				
					case 'DXPo': densityRamp = ePosXDist; break;// Density ramp off: positive half sphere up
					case 'DYPo': densityRamp = ePosYDist; break;// Density ramp off: positive half sphere up
					case 'DZPo': densityRamp = ePosZDist; break;// Density ramp off: positive half sphere up
					case 'DXNe': densityRamp = eNegXDist; break;// Density ramp off: positive half sphere up
					case 'DYNe': densityRamp = eNegYDist; break;// Density ramp off: positive half sphere up
					case 'DZNe': densityRamp = eNegZDist; break;// Density ramp off: positive half sphere up
				
					// Color Ramp

					case 'FPHU': colorRamp = ePosHalfSphereUp; break;// Density ramp off: positive half sphere up
					case 'FPHD': colorRamp = ePosHalfSphereDown; break;// Density ramp off: positive half sphere up
					case 'FNHU': colorRamp = eNegHalfSphereUp; break;// Density ramp off: positive half sphere up
					case 'FNHD': colorRamp = eNegHalfSphereDown; break;// Density ramp off: positive half sphere up
				
					case 'FPXC': colorRamp = ePosXCylinder; break;// Density ramp off: positive half sphere up
					case 'FPYC': colorRamp = ePosYCylinder; break;// Density ramp off: positive half sphere up
					case 'FPZC': colorRamp = ePosZCylinder; break;// Density ramp off: positive half sphere up
					case 'FNXC': colorRamp = eNegXCylinder; break;// Density ramp off: positive half sphere up
					case 'FNYC': colorRamp = eNegYCylinder; break;// Density ramp off: positive half sphere up
					case 'FNZC': colorRamp = eNegZCylinder; break;// Density ramp off: positive half sphere up
				
					case 'FXPo': colorRamp = ePosXDist; break;// Density ramp off: positive half sphere up
					case 'FYPo': colorRamp = ePosYDist; break;// Density ramp off: positive half sphere up
					case 'FZPo': colorRamp = ePosZDist; break;// Density ramp off: positive half sphere up
					case 'FXNe': colorRamp = eNegXDist; break;// Density ramp off: positive half sphere up
					case 'FYNe': colorRamp = eNegYDist; break;// Density ramp off: positive half sphere up
					case 'FZNe': colorRamp = eNegZDist; break;// Density ramp off: positive half sphere up
				
					case 'SEED':
					{
						fPMap.fSeed = MCRandom();
					//	fNoise.SetSeed(fPMap.fSeed);
					//	fNoise.InitGradientTab();
						fNoiseInvalid = true;
					} break; 

					case 'DePr':
						{	// Density Ramp Off Preset
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							SetDensityPreset(popupItem);

							// Unselect the button
							popupItem = 'NoPt';
							part->SetValue(&popupItem, kInt32ValueType, false, false);
						} break;

					case 'CoPr':
						{	// Color Ramp Off Preset
							int32 popupItem = -1;
							part->GetValue(&popupItem, kInt32ValueType);
							SetColorPreset(popupItem);

							// Unselect the button
							popupItem = 'NoPt';
							part->SetValue(&popupItem, kInt32ValueType, false, false);
						} break;
					default: break;
					}

					if(densityRamp>=0)
					{	// a density ramp was switch on or off
						boolean isOn = false;
						part->GetValue(&isOn, kBooleanValueType);

						{
							// Get the current ramp flag
							// Add/Remove the checked rampoff
							if( isOn )	{fPMap.fRampOffFlag|=densityRamp;}
							else		{fPMap.fRampOffFlag&=~densityRamp;}
						}
					}
					if(colorRamp>=0)
					{	// a color ramp was switch on or off
						boolean isOn = false;
						part->GetValue(&isOn, kBooleanValueType);

						{
							// Get the current ramp flag
							// Add/Remove the checked rampoff
							if( isOn )	{fPMap.fColorFlag|=colorRamp;}
							else		{fPMap.fColorFlag&=~colorRamp;}
						}
					}

				}
			}
		}
	}

	if(!handledMessage)
		TBasicPrimitive::SimpleHandleEvent( message, source, data);
	
	return MC_S_FALSE;// Carrara 6: need to return MC_S_FALSE otherwise the other message aren't called ?
	/* Doesn't work
	if(fNeedToPost && message == EMFPartMessage::kMsg_PartValueChanged)
	{
		if (MCVerify(source))
		{
			PostAction(source->GetInstanceID());
			fNeedToPost = false;
		}
	}
*/	
	// Carrara 6: need to return MC_S_FALSE otherwise the other message aren't called ?
//	return result ; // MC_S_FALSE; // result;
}

int32 GetDensityFlag(const int32 densityPreset)
{
	int32 flag = eNoRampOff;

	switch(densityPreset)
	{
		// Opti are for the density presets
	case 'Opt0': return flag;// Cube
	case 'Opt1': flag|=ePosHalfSphereUp; flag|=ePosHalfSphereDown; return flag;// Sphere
	case 'Opt2': flag|=ePosZCylinder; return flag;// Cylinder
	case 'Opt3':
		{	// Zero Edge
			flag|=ePosXDist;
			flag|=ePosYDist;
			flag|=ePosZDist;
			flag|=eNegXDist;
			flag|=eNegYDist;
			flag|=eNegZDist;

			return flag;
		}
	case 'Opt4': flag|=eNegXCylinder; return flag;// Tube
	case 'Opt5': flag|=ePosZDist; return flag;// Z
		// Prei are for the color presets
	case 'Pre0': return flag;// Cube
	case 'Pre1': flag|=ePosHalfSphereUp; flag|=ePosHalfSphereDown; return flag;// Sphere
	case 'Pre2': flag|=ePosZCylinder; return flag;// Cylinder
	case 'Pre3':
		{	// Zero Edge
			flag|=ePosXDist;
			flag|=ePosYDist;
			flag|=ePosZDist;
			flag|=eNegXDist;
			flag|=eNegYDist;
			flag|=eNegZDist;

			return flag;
		}
	case 'Pre4': flag|=eNegXCylinder; return flag;// Tube
	case 'Pre5': flag|=ePosZDist; return flag;// Z
	}

	return flag;
}

void GasBase::SetDensityPreset(const int32 densityPreset)
{
	// First reset the values
	fPMap.fRampOffFlag = GetDensityFlag(densityPreset);
	switch(densityPreset)
	{
	case 'Opt0':
		{	// Cube
		} break;
	case 'Opt1':
		{	// Sphere
			fPMap.fIntensityRamps.fPosHalfSphereUp = init0_3;
			fPMap.fIntensityRamps.fPosHalfSphereDown = init0_3;
		} break;
	case 'Opt2':
		{	// Cylinder
			fPMap.fIntensityRamps.fPosZCylinder = init0_3;
		} break;
	case 'Opt3':
		{	// Zero Edge
			fPMap.fIntensityRamps.fPosXDist = init0_1;
			fPMap.fIntensityRamps.fPosYDist = init0_1;
			fPMap.fIntensityRamps.fPosZDist = init0_1;
			fPMap.fIntensityRamps.fNegXDist = init0_1;
			fPMap.fIntensityRamps.fNegYDist = init0_1;
			fPMap.fIntensityRamps.fNegZDist = init0_1;
		} break;
	case 'Opt4':
		{	// Tube
			fPMap.fIntensityRamps.fNegXCylinder = init3_5;
		} break;
	case 'Opt5':
		{	// Z
			fPMap.fIntensityRamps.fPosZDist = init0_3;//.9f;
		} break;
	}
}

void GasBase::SetColorPreset(const int32 colorPreset)
{
	// First reset the values
	fPMap.fColorFlag = GetDensityFlag(colorPreset); // Note: use this function as long as the preset are the sames
	switch(colorPreset)
	{
	case 'Pre0':
		{	// Cube
		} break;
	case 'Pre1':
		{	// Sphere
			fPMap.fColorRamps.fPosHalfSphereUp = TVector2::kUnitY;
			fPMap.fColorRamps.fPosHalfSphereDown = TVector2::kUnitY;
		} break;
	case 'Pre2':
		{	// Cylinder
			fPMap.fColorRamps.fPosZCylinder = TVector2::kUnitY;
		} break;
	case 'Pre3':
		{	// Zero Edge
			fPMap.fColorRamps.fPosXDist = init0_5;
			fPMap.fColorRamps.fPosYDist = init0_5;
			fPMap.fColorRamps.fPosZDist = init0_5;
			fPMap.fColorRamps.fNegXDist = init0_5;
			fPMap.fColorRamps.fNegYDist = init0_5;
			fPMap.fColorRamps.fNegZDist = init0_5;
		} break;
	case 'Pre4':
		{	// Tube
			fPMap.fColorRamps.fNegXCylinder = TVector2::kUnitY;
		} break;
	case 'Pre5':
		{	// Z
			fPMap.fColorRamps.fPosZDist = TVector2::kUnitY;//.9f;
		} break;
	}
}

//// IExStreamIO methods
//MCCOMErr GasBase::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
//{
//	return MC_S_FALSE;
//}
//
//MCCOMErr GasBase::Write(IShTokenStream* stream)
//{
//	return MC_S_FALSE;
//}
//
//// set the primitive size to 4 before reading it (for old files compatibility)
//MCCOMErr GasBase::FinishRead(IStreamContext* streamContext) 
//{
//	fPMap.fPrimitiveSize = 4; 
//	return MC_S_OK;
//}

/* Doesn't work
void GasBase::PostAction(const IDType partID)
{
	TMCCountedPtr<IMFDocument>	document;
	gShell3DUtilities->GetActiveDocument(&document);
	TMCCountedPtr<IMFResponder>	responder;
	document->QueryInterface(IID_IMFResponder, (void**)&responder);
	
	TMCCountedPtr<IShAction> action;
	ModifyComponentAction::Create(&action, this, fCompUIPart, partID);
	gActionManager->PostAction(action, 1, responder);
}
*/
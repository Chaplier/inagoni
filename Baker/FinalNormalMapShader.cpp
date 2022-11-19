/****************************************************************************************************

		FinalNormalMapShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "FinalNormalMapShader.h"

#include "Baker.h"
#include "BakerDef.h"
#include "geometry.h"
#include "MiscComUtilsImpl.h"

//#if DEMO_VERSION
#include "IShChannel.h"
#include "IShTextureMap.h"
//#endif

// Serial number or demo version warning
boolean FinalNormalMapShader::gFirstTime = true;

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"
#endif

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_FinalNormalMapShader(R_CLSID_FinalNormalMapShader);
#else
const MCGUID CLSID_FinalNormalMapShader={R_CLSID_FinalNormalMapShader};
#endif

PMapFinalNormalMapShader::PMapFinalNormalMapShader()
{
	fFlip = false;
	fType = eXRed+eYGreen; // Default Baker kind of map
}

FinalNormalMapShader::FinalNormalMapShader()
{
	fPreprocessed = false;
}

boolean FinalNormalMapShader::IsEqualTo(I3DExShader* aShader)
{
	return ( 
		fPMap.fSubShaderComponent == ((FinalNormalMapShader*)aShader)->fPMap.fSubShaderComponent  &&
		fPMap.fFlip == ((FinalNormalMapShader*)aShader)->fPMap.fFlip );
}

MCCOMErr FinalNormalMapShader::GetShadingFlags(ShadingFlags& theFlags)
{
	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{
		fSubShader->GetShadingFlags(theFlags);
		theFlags.fConstantChannelsMask = kNoChannel;
	}
	else
	{
		theFlags.fNeedsUV = true;		// We need UV coordinates
		theFlags.fConstantChannelsMask = kNoChannel;
	}

	theFlags.fNeedsPointLoc = true;
	theFlags.fNeedsNormalLoc = true;

	return MC_S_OK;
}

EShaderOutput FinalNormalMapShader::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector+kUsesGetColor+kUsesGetValue);
}

MCCOMErr FinalNormalMapShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

real32 FinalNormalMapShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{
		fSubShader->GetValue(result, fullArea, shadingIn);
	}
	else
	{
		result = 0;
	}

	return 1.0f;
}

#if (VERSIONNUMBER >= 0x040000)
real32 FinalNormalMapShader::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#else
real32 FinalNormalMapShader::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{
		fSubShader->GetColor(result, fullArea, shadingIn);
	}
	else
	{
#if (VERSIONNUMBER >= 0x040000)
		result = TMCColorRGBA::kBlackNoAlpha;
#else
		result = TMCColorRGB::kBlack;
#endif
	}

	return 1.0f;
}

real32 FinalNormalMapShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if(!fPreprocessed)
		PreProcess();

	if(fSubShader)
	{	// Use the color to build the vector
		boolean fullArea=true;
#if (VERSIONNUMBER >= 0x040000)
		TMCColorRGBA color;
#else
		TMCColorRGB color;
#endif
		fSubShader->GetColor(color, fullArea, shadingIn);
#if 1
		TVector3 mapNormal;
		mapNormal[fRedAxis] = 2*(color.red-.5);
		mapNormal[fGreenAxis] = 2*(color.green-.5);
		mapNormal[fBlueAxis] = 2*(color.blue-.5);

		if(fPMap.Flag(eNegX)) mapNormal[fRedAxis]*=-1;
		if(fPMap.Flag(eNegY)) mapNormal[fGreenAxis]*=-1;
		if(fPMap.Flag(eNegZ)) mapNormal[fBlueAxis]*=-1;

		if(fPMap.Flag(eIsRelative))
		{
			// Quite similar to Deeper map: the normal is relative to a base attached to the surface
			// This kind of map is made with Baker

			// Red == X axis 
			// Green == Y axis 
			// Blue == Z axis 
			// 100% == 1, 0% == -1, 50% == 0
			// Blue can't be <50% (flipped normal)
			// result = color
			if(fPMap.fFlip)
			{
				mapNormal.x *= -1;
				mapNormal.y *= -1;
			}
			mapNormal.Normalize();

			// We build a 3D frame on the surface
			
			const TVector3 isoNormal  = shadingIn.fNormalLoc;

			TVector3 uDir = TVector3::kUnitX;
			const real32 uDist = shadingIn.fIsoU.Normalize(uDir);
			TVector3 vDir = TVector3::kUnitY;
			const real32 vDist = shadingIn.fIsoV.Normalize(vDir);

			// Note: we shouldn't use fIsoU and fIsoV directly, they're not necessary on a ortho frame with fNormalLoc. We should 
			// project them on fNormalLoc plane to get the actual 3D frame.

			result = mapNormal.x*uDir + mapNormal.y*vDir + mapNormal.z*isoNormal;
		}
		else
		{
			// Red == X axis 
			// Green == Y axis 
			// Blue == Z axis 
			// 100% == 1, 0% == -1, 50% == 0

			// This is the final normal as we would like it to be,
			// but GetVector ask for a bump perturbation that we need 
			// to recreate
			TVector3 point1 = shadingIn.fPointLoc+shadingIn.fNormalLoc;
			TVector3 point2 = point1;
			const boolean OK = IntersectLinePlane2(
								shadingIn.fPointLoc,
								mapNormal,
								shadingIn.fNormalLoc,
								point1,
								point2 );

			result = point2-point1;

			if(fPMap.fFlip)
			{
				result.x *= -1;
				result.y *= -1;
				result.z *= -1;
			}
		}
#else
		if(fPMap.fType=='Typ3')
		{
			// Quite similar to Deeper map: the normal is relative to a base attached to the surface
			// This kind of map is made with Baker

			// Red == X axis 
			// Green == Y axis 
			// Blue == Z axis 
			// 100% == 1, 0% == -1, 50% == 0
			// Blue can't be <50% (flipped normal)
			// result = color
			TVector3 mapNormal;
			mapNormal.x = 2*(color.red-.5);
			mapNormal.y = 2*(color.green-.5);
			mapNormal.z = 2*(color.blue-.5);
			if(fPMap.fFlip)
			{
				mapNormal.x *= -1;
				mapNormal.y *= -1;
			}
			mapNormal.Normalize();

			TVector3 isoNormal = shadingIn.fNormalLoc;
			TVector3 uDir, vDir;
			const real32 uDist = shadingIn.fIsoU.Normalize(uDir);
			isoNormal.BuildOrthonormalBase(vDir,uDir);// test

			result = mapNormal.x*uDir + mapNormal.y*vDir + mapNormal.z*isoNormal;
		}
		else
		{
			// Red == X axis 
			// Green == Y axis 
			// Blue == Z axis 
			// 100% == 1, 0% == -1, 50% == 0
			result[fRedAxis] = 2*(color.red-.5);
			result[fGreenAxis] = 2*(color.green-.5);
			result[fBlueAxis] = 2*(color.blue-.5);

			if(fPMap.fType=='Typ2') // ZBrush case
				result[fRedAxis]*=-1;

// ZBrush2 test			result[fGreenAxis]*=-1;

			// This is the final normal as we would like it to be,
			// but GetVector ask for a bump perturbation that we need 
			// to recreate
			TVector3 point1 = shadingIn.fPointLoc+shadingIn.fNormalLoc;
			TVector3 point2 = point1;
			const boolean OK = IntersectLinePlane2(
								shadingIn.fPointLoc,
								result,
								shadingIn.fNormalLoc,
								point1,
								point2 );

			result = point2-point1;

			if(fPMap.fFlip)
			{
				result.x *= -1;
				result.y *= -1;
				result.z *= -1;
			}
		}
#endif
	}
	else
	{
		result = TVector3::kZero;
	}

	return 1.0f;
}

void FinalNormalMapShader::PreProcess()
{
	fPreprocessed =true;

//#if !DEMO_VERSION
	if(!IsSerialValid())
	{	// Invalid serial number: tell the user it won't work
		if(gFirstTime)
		{
			CWhileInCompResFile myRes(kRID_ShaderFamilyID, 'FNSh');
			TMCDynamicString message;
			gResourceUtilities->GetIndString( message, kOtherNames, 8);
			TMCDynamicString empty;
			gShellUtilities->DoAlertAsync(kShNotificationAlert, message, empty, empty, empty);
			gFirstTime = false;
		}
		fSubShader=NULL;
		return;
	}
//#else
	// Demo version: doesn't work
	//if(gFirstTime)
	//{
	//	CWhileInCompResFile myRes(kRID_ShaderFamilyID, 'FNSh');
	//	TMCDynamicString message;
	//	gResourceUtilities->GetIndString( message, kOtherNames, 9);
	//	gPartUtilities->Alert(message);
	//	gFirstTime = false;
	//}
//#endif

	BackwardCompatibility(); // translate the data from older files

	// Default setiing:
	// Red == X axis 
	// Green == Y axis 
	// Blue == Z axis 
	// 100% == 1, 0% == -1, 50% == 0
	fRedAxis = -1;
	fGreenAxis = -1;
	fBlueAxis = -1;
	// Set the X axis
	if(fPMap.Flag(eXRed)) fRedAxis = 0;
	else if(fPMap.Flag(eXGreen)) fGreenAxis = 0;
	else fBlueAxis = 0;
	// Set the Y axis
	if(fPMap.Flag(eYRed)) fRedAxis = 1;
	else if(fPMap.Flag(eYGreen)) fGreenAxis = 1;
	else fBlueAxis = 1;
	// Set the Z axis (no else if() to avoid crashes)
	if(fRedAxis == -1) fRedAxis = 2;
	if(fGreenAxis == -1) fGreenAxis = 2;
	if(fBlueAxis == -1) fBlueAxis = 2;

	if(fPMap.fSubShaderComponent )
	{
		fPMap.fSubShaderComponent->QueryInterface(IID_I3DShShader, (void**)&(fSubShader));
	}
	else
		fSubShader=NULL;

//#if DEMO_VERSION
	if(!IsSerialValid())
	{
		// Check that the size of the texture is small enough
		if(fSubShader)
		{
			boolean ok = false;

			ShadingFlags theFlags;
			fSubShader->GetShadingFlags(theFlags);

			if(!theFlags.fUVSpaceShaders)
			{
				TMCCountedPtr<IShTextureMap> outMap;
				fSubShader->GetOriginalParametricTextureMap(0, 0, &outMap);
				if(outMap)
				{
					TMCCountedPtr<IShChannel> channel;
					outMap->GetChannel(&channel);

					if(channel)
					{
						TMCRect bounds;
						channel->GetBounds(bounds);

						if(bounds.GetWidth()<=128 && bounds.GetHeight()<=128)
							ok = true;
					}
				}

			}

			if(!ok)
			{
				fSubShader=NULL;
			}
		}
	}
//#endif
}

void FinalNormalMapShader::BackwardCompatibility()
{
	// fType was used to store presets:
	// 'Typ1' for maps made with Baker
	// 'Typ2' for maps made with ZBrush
	// 'Typ3' for relative maps made with Baker
	switch(fPMap.fType)
	{
	case 'Typ1':
		{
			// Red == X axis, Green == Y axis, Blue == Z axis 
			fPMap.fType = eClearFlags; 
			fPMap.SetFlag(eXRed);
			fPMap.SetFlag(eYGreen);
		} break;
	case 'Typ2':
		{
			// Red == -X axis, Green == Z axis, Blue == Y axis 
			fPMap.fType = eClearFlags; 
			fPMap.SetFlag(eXRed);
			fPMap.SetFlag(eYBlue);
			fPMap.SetFlag(eNegX);
		} break;
	case 'Typ3':
		{
			// Red == X axis, Green == Y axis, Blue == Z axis 
			fPMap.fType = eClearFlags; 
			fPMap.SetFlag(eXRed);
			fPMap.SetFlag(eYGreen);
			fPMap.SetFlag(eIsRelative);
		} break;
	}
}


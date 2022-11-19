/****************************************************************************************************

		NormalMapShader.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "NormalMapShader.h"

#include "NormalMapDef.h"
#include "Deeper.h"

#include "ComMessages.h"
#include "Geometry.h"
#include "RealQuat.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_NormalMapShader(R_CLSID_NormalMapShader);
#else
const MCGUID CLSID_NormalMapShader={R_CLSID_NormalMapShader};
#endif

PMapNormalMapShader::PMapNormalMapShader()
{
	fFlip = false;
}

NormalMapShader::NormalMapShader()
{
	fPreprocessed = false;
}

boolean NormalMapShader::IsEqualTo(I3DExShader* aShader)
{
	return ( 
		fPMap.fSubShaderComponent == ((NormalMapShader*)aShader)->fPMap.fSubShaderComponent  &&
		fPMap.fFlip == ((NormalMapShader*)aShader)->fPMap.fFlip );
}

MCCOMErr NormalMapShader::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (message == kMsg_CUIP_ComponentAttached)
		CheckSerial();

	return MC_S_FALSE;
}

MCCOMErr NormalMapShader::GetShadingFlags(ShadingFlags& theFlags)
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
	return MC_S_OK;
}

EShaderOutput NormalMapShader::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector+kUsesGetColor+kUsesGetValue);
}

MCCOMErr NormalMapShader::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}

real32 NormalMapShader::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if(!IsSerialValid())
		return 1.0f;

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
real32 NormalMapShader::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#else
real32 NormalMapShader::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if(!IsSerialValid())
		return 1.0f;

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

real32 NormalMapShader::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if(!IsSerialValid())
		return 1.0f;

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

		// This result is for fIsoU = kUnitX and fIsoV = kUnitY.
		// We need to modify it to take into account the real map orientation
		TVector3 uDir = TVector3::kUnitX;
		const real32 uDist = shadingIn.fIsoU.Normalize(uDir);
		TVector3 vDir = TVector3::kUnitY;
		const real32 vDist = shadingIn.fIsoV.Normalize(vDir);

		const real32 coeff = uDist/vDist;

		TVector3 isoNormal = uDir^vDir;
		if(isoNormal.z*shadingIn.fNormalLoc.z<0)
		{
			// Some object, like the sphere, seems to have the iso values flipped
			isoNormal.z*=-1;
		}

		// test
/*		TVector3 isoNormal = shadingIn.fNormalLoc;
		TVector3 uDir, vDir;
		const real32 uDist = shadingIn.fIsoU.Normalize(uDir);
		isoNormal.BuildOrthonormalBase(vDir,uDir);// test
*/

		result = mapNormal.x*uDir + mapNormal.y*vDir + mapNormal.z*isoNormal;
	}
	else
	{
		result = TVector3::kZero;
	}

	return 1.0f;
}

void NormalMapShader::PreProcess()
{
	if(fPMap.fSubShaderComponent )
	{
		fPMap.fSubShaderComponent->QueryInterface(IID_I3DShShader, (void**)&(fSubShader));
	}
	else
		fSubShader=NULL;
}
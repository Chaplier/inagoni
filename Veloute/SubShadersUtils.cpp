/****************************************************************************************************

		SubShadersUtils.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/16/2004

****************************************************************************************************/

#include "SubShadersUtils.h"

#include "utils.h"
#include "copyright.h"

TThreeShaders::TThreeShaders()
{
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShaders[iShader]=NULL;
	}
}

#if (VERSIONNUMBER >= 0x040000)
void TThreeShaders::GetMortarColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
void TThreeShaders::GetMortarColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#endif
{
	if( fShaders[1])
	{	// There's a shader for the tile. Use it's color value
		fShaders[1]->GetColor(result, fullAreaDone, shadingIn);
	}
	else
	{	// no tile subshader
#if (VERSIONNUMBER >= 0x040000)
		result = TMCColorRGBA::kBlackNoAlpha;
#else
		result = TMCColorRGB::kBlack;
#endif
	}
}

#if (VERSIONNUMBER >= 0x040000)
void TThreeShaders::GetTileColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn, const TVector2& center)
#else
void TThreeShaders::GetTileColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn, const TVector2& center)
#endif
{
	if( fShaders[0])
	{	// There's a shader for the tile. Use it's color value
		fShaders[0]->GetColor(result, fullAreaDone, shadingIn);
	}
	else
	{	// no tile subshader: use the value to create a grey
#if (VERSIONNUMBER >= 0x040000)
		result = TMCColorRGBA::kWhiteNoAlpha;
#else
		result = TMCColorRGB::kWhite;
#endif
	}

	// Use the Tint shader if there's one.
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA tintColor = TMCColorRGBA::kWhiteNoAlpha;
#else
	TMCColorRGB tintColor = TMCColorRGB::kWhite;
#endif
	if( GetTintColor(tintColor, shadingIn, center) )
	{
/*		// HLS tinting
		real32 Hsrc=0, Lsrc=0, Ssrc=0;
		result.GetHLS(Hsrc, Lsrc, Ssrc);
		if(Ssrc==0) Hsrc=0;
		real32 Htin=0, Ltin=0, Stin=0;
		tintColor.GetHLS(Htin, Ltin, Stin);
		if(Stin==0) Htin=0;
		
		const real32 coefH1 = Hsrc/(Hsrc+Htin);
		const real32 coefH2 = Htin/(Hsrc+Htin);
		const real32 coefL1 = Lsrc/(Lsrc+Ltin);
		const real32 coefL2 = Ltin/(Lsrc+Ltin);
		const real32 coefS1 = Ssrc/(Ssrc+Stin);
		const real32 coefS2 = Stin/(Ssrc+Stin);
		result.SetHLS(	coefH1*Hsrc+coefH2*Htin, 
						coefL1*Lsrc+coefL2*Ltin, 
						coefS1*Ssrc+coefS2*Stin);
*/
		// RGB tinting
		result.red = Bias(tintColor.red,result.red*.95+.025); // Slighly rescale the color to be able to modify pure red, green or blue
		result.green = Bias(tintColor.green,result.green*.95+.025);
		result.blue = Bias(tintColor.blue,result.blue*.95+.025);

	}
}

void TThreeShaders::GetMortarValue(real32& result,boolean &fullAreaDone,ShadingIn& shadingIn)
{
	if( fShaders[1])
	{	// There's a shader for the tile. Use it's color value
		fShaders[1]->GetValue(result, fullAreaDone, shadingIn);
	}
	else
	{	// no tile subshader
		result = 0;
	}
}

void TThreeShaders::GetTileValue(real32& result,boolean &fullAreaDone,ShadingIn& shadingIn, const real32 max)
{
	if( fShaders[0])
	{	// There's a shader for the tile. Use it's color value
		fShaders[0]->GetValue(result, fullAreaDone, shadingIn);
	}
	else
	{	// no tile subshader: use the max
		result = max;
	}
}

void TThreeShaders::GetMortarVector(TVector3& result,ShadingIn& shadingIn)
{
	if( fShaders[1])
	{	// There's a shader for the tile. Use it's color value
		fShaders[1]->GetVector(result, shadingIn);
	}
	else
	{	// no tile subshader
		result = TVector3::kZero;
	}
}

void TThreeShaders::GetTileVector(TVector3& result,ShadingIn& shadingIn)
{
	if( fShaders[0])
	{	// There's a shader for the tile. Use it's color value
		fShaders[0]->GetVector(result, shadingIn);
	}
	else
	{	// no tile subshader
		result = TVector3::kZero;
	}
}

void TThreeShaders::DoMortarShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if( fShaders[1])
	{	// There's a shader for the tile. Use it's color value
		fShaders[1]->DoShade(result, shadingIn);
	}
	else
	{	// no tile subshader
#if (VERSIONNUMBER >= 0x040000)
		result.fColor = TMCColorRGBA::kBlackNoAlpha;
#else
		result.fColor = TMCColorRGB::kBlack;
#endif
	}
}

void TThreeShaders::DoTileShade(ShadingOut& result,ShadingIn& shadingIn, const TVector2& center)
{
	if( fShaders[0])
	{	// There's a shader for the tile. Use it's color value
		fShaders[0]->DoShade(result, shadingIn);
	}
	else
	{	// no tile subshader: use the value to create a grey
#if (VERSIONNUMBER >= 0x040000)
		result.fColor = TMCColorRGBA::kBlackNoAlpha;
#else
		result.fColor = TMCColorRGB::kBlack;
#endif
	}

	// Use the Tint shader on the color channel if there's one.
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA tintColor = TMCColorRGBA::kWhiteNoAlpha;
#else
	TMCColorRGB tintColor = TMCColorRGB::kWhite;
#endif
	if( GetTintColor(tintColor, shadingIn, center) )
	{
		result.fColor.red = Bias(tintColor.red,result.fColor.red*.95+.025); // Slighly rescale the color to be able to modify pure red, green or blue
		result.fColor.green = Bias(tintColor.green,result.fColor.green*.95+.025);
		result.fColor.blue = Bias(tintColor.blue,result.fColor.blue*.95+.025);
	}
}

#if (VERSIONNUMBER >= 0x040000)
boolean TThreeShaders::GetTintColor( TMCColorRGBA& tintColor,ShadingIn& shadingIn, const TVector2& center )
#else
boolean TThreeShaders::GetTintColor( TMCColorRGB& tintColor,ShadingIn& shadingIn, const TVector2& center )
#endif
{
	if( fShaders[2])
	{	// Use the Tint shader if there's one.
		ShadingIn tileCenterShading = shadingIn;

		ShadingFlags shaderFlag;
		fShaders[2]->GetShadingFlags(shaderFlag);
		if(shaderFlag.fNeedsUV)
			tileCenterShading.fUV = center;
		if(shaderFlag.fNeedsPoint)
		{
			tileCenterShading.fPoint.x = center.x;
			tileCenterShading.fPoint.y = center.y;
			tileCenterShading.fPoint.z = 0;
		}
		if(shaderFlag.fNeedsPointLoc)
		{
			tileCenterShading.fPointLoc.x = center.x;
			tileCenterShading.fPointLoc.y = center.y;
			tileCenterShading.fPointLoc.z = 0;
		}

		boolean fullAreaDone;
		fShaders[2]->GetColor(tintColor, fullAreaDone, tileCenterShading);
		return true;
	}
	else 
		return false;
}

/****************************************************************************************************

		SubShadersUtils.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/16/2004

****************************************************************************************************/

#ifndef __SubShadersUtils__
#define __SubShadersUtils__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "I3DShShader.h"
#include "MCPtrArray.h"
#include "ExtraShadersDef.h"


//////////////////////////////////////////////////////////////////////////
//
// First shader: tile shader
// Second shader: mortar shader
// Third shader: tile tinting
class TThreeShaders
{
public:
	TThreeShaders();
	TMCCountedPtr<I3DShShader> fShaders[3];

#if (VERSIONNUMBER >= 0x040000)
	void	GetMortarColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	void	GetTileColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn, const TVector2& center);
#else
	void	GetMortarColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	void	GetTileColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn, const TVector2& center);
#endif

	void	GetMortarValue(real32& result,boolean &fullAreaDone,ShadingIn& shadingIn);
	void	GetTileValue(real32& result,boolean &fullAreaDone,ShadingIn& shadingIn, const real32 max);

	void	GetMortarVector(TVector3& result,ShadingIn& shadingIn);
	void	GetTileVector(TVector3& result,ShadingIn& shadingIn);

	void	DoMortarShade(ShadingOut& result,ShadingIn& shadingIn);
	void	DoTileShade(ShadingOut& result,ShadingIn& shadingIn, const TVector2& center);
	
#if (VERSIONNUMBER >= 0x040000)
	boolean	GetTintColor( TMCColorRGBA& tintColor,ShadingIn& shadingIn, const TVector2& center );
#else
	boolean	GetTintColor( TMCColorRGB& tintColor,ShadingIn& shadingIn, const TVector2& center );
#endif
};
//
//////////////////////////////////////////////////////////////////////////

#endif

/****************************************************************************************************

		ShaderBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/2/2004

****************************************************************************************************/

#ifndef __ShaderBase__
#define __ShaderBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ExtraShadersDef.h"
#include "BasicShader.h"
#include "I3DShShader.h"

class ShaderBase :	public TBasicShader
{
public :
	MCCOMErr		MCCOMAPI	HandleEvent				(MessageID message, IMFResponder* source, void* data);
};









#endif

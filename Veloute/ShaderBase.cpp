/****************************************************************************************************

		ShaderBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	29/9/2004

****************************************************************************************************/
#include "ShaderBase.h"

#include "Veloute.h"
#include "ComMessages.h"

MCCOMErr ShaderBase::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (message == kMsg_CUIP_ComponentAttached)
		CheckSerial();

	// Need to return FALSE if we want kMsg_CUIP_ComponentAttached to be called
	return MC_S_FALSE; // TBasicShader::HandleEvent( message, source, data);
}
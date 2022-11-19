/****************************************************************************************************

		NormalMap.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#include "ExternalAPI.r"
#include "External3DAPI.r" // For TABS and STR#
#include "MFRTypes.r"
#include "Copyright.h"
#include "NormalMapDef.h"
#include "interfaceids.h"


#ifndef qUsingResourceLinker
#ifdef MAC
include "Deeper.rsr";
#endif
#endif
#ifdef WIN
include "Deeper.rsr";
#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (551)
{
	kRID_SceneCommandFamilyID,
	'Fake',
	"Deeper Fake",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Normal Map Shader
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (550)
{
	{
		R_IID_I3DExShader,
		R_CLSID_NormalMapShader
	}
};

resource 'COMP' (550)
{
	kRID_ShaderFamilyID,
	'NMSh',
	"Deeper Normal Map",
	"Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (550)
{
	{
		'Sh00','comp',interpolate,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Flip','bool',noFlags,"flip","",
	}
};

resource 'CPUI' (550) 
{
	550,					// Id of your main part
	555,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Normal Map Renderer
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (650,"Normal Map Renderer")
{
	{
	R_IID_I3DExFinalRenderer,
	R_CLSID_NormalMapRenderer
	}
};

resource 'COMP' (650,"Normal Map Renderer")
{
	'frnd',        // extension type =renderer
	'NMRn',        // Class ID
	"Normal Map",
	"Required",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (650,"Normal Map Renderer")
{
	{
		'FBac','bool',interpolate,"Flip Backface","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Strings
//
resource 'STR#' (kStrings, purgeable) 
{
	{
		"Render Normals",			// 01
		"NormalMapData.dta",		// 02: File name
	}
};

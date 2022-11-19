/****************************************************************************************************

		Baker.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/30/2004

****************************************************************************************************/


#include "ExternalAPI.r"
#include "External3DAPI.r" // For STR#
#include "Copyright.h"
#include "BakerDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"

#if (VERSIONNUMBER >= 0x050000)
#include "StandardBindingIDs.h"
#endif

#ifndef qUsingResourceLinker
#ifdef MAC
include "Baker.rsr";
#endif
#endif
#ifdef WIN
include "Baker.rsr";
#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (850)
{
	kRID_SceneCommandFamilyID,
	'Baki',
#if (VERSIONNUMBER >= 0x050000)
	"Baking",
#else
	"Baking/F", // Shortcut: ctrl+F
#endif
	"", // "Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (850)
{
	kBakerCommandID,
	defaultMenu,// Target menu ID if the Action Number is -1
	{// ID of the room where the Scene Command should appear
		'3Dvw',		// 3D View room
	},
	kOtherNames,							//int16 ID of STR# resource to use to load the strings BnGp needs
	10, // kDefaultName,						//int16 Index of string in STR# resource to use for fGroupName
	kEditBindingGroupID, // kNoGroup,							//int32 fGroupID
	{
		kBakerCommandID,					//int32 (unique!)ID of the bound command or tool (fCommandID)
		10,								//int16 Index of string in STR# resource to use for fCommandName
		'F',							//int16 fChar
		kNoShift,						//int16 fShift
		kCtrl,							//int16 fControl
		kNoAlt,							//int16 fAlt
		kAnyPlatform,					//int16 fPlatforms	On which platforms is the shortut defined (use platform defines from CommonComDefines.h)
	},
	k3DViewBindingContextID // kNoContext
};
#else
resource 'scmd' (850)
{
	dynamicItem,
	defaultMenu,
	{
		'3Dvw',		// 3D View room
	}
};
#endif

resource 'PMap' (850)
{
	{
		'MapW','in32',noFlags, "Map Width","",
		'MapH','in32',noFlags, "Map Width","",
		'MapT','in32',noFlags, "Map type","",
		'Diff','bool',noFlags, "Diffuse channel","",
		'High','bool',noFlags, "Highlight channel","",
		'Shin','bool',noFlags, "Shininess channel","",
		'Bump','bool',noFlags, "Bump channel","",
		'Refl','bool',noFlags, "Reflexion channel","",
		'Tran','bool',noFlags, "Transparency channel","",
		'Refr','bool',noFlags, "Refraction channel","",
		'Glow','bool',noFlags, "Glow channel","",
		'UCam','bool',noFlags, "Use current Camera","",
//		'MerD','bool',noFlags, "Merge Shading Domains","",
//		'UseB','bool',noFlags, "Use Background Color","",
		'Type','in32',noFlags, "Normal Type (Classic or ZBrush)","",
		'Doma','in32',noFlags, "Shading Domain  treatment","",
		'BCol','colo',noFlags, "Background color","",
		'Spac','in32',noFlags, "Local or Global Space","",
		'Save','bool',noFlags, "Auto Save","",
		'Fold','Dstr',noFlags, "Save in folder","",
		'Open','bool',noFlags, "Open Image","",
	}
};

resource 'GUID' (850)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_BakingCommand
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Preferences
// CINF (extra component Info, see TComponentInfo in IExRegisterer.h)
//

// Preferences component
resource 'COMP' (851)
{
	kRID_PreferencesFamilyID,
	'pref',
	"",
	"", 
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

// Preferences GUID
resource 'GUID' (851)
{
	{
		R_IID_IExPrefsComponent,
		R_CLSID_BakingCommandPrefs
	}
};

// Parameters saved in the preferences
// Same as the PMap 850
resource 'PMap' (851)
{
	{
		'MapW','in32',noFlags, "Map Width","",
		'MapH','in32',noFlags, "Map Width","",
		'MapT','in32',noFlags, "Map type","",
		'Diff','bool',noFlags, "Diffuse channel","",
		'High','bool',noFlags, "Highlight channel","",
		'Shin','bool',noFlags, "Shininess channel","",
		'Bump','bool',noFlags, "Bump channel","",
		'Refl','bool',noFlags, "Reflexion channel","",
		'Tran','bool',noFlags, "Transparency channel","",
		'Refr','bool',noFlags, "Refraction channel","",
		'Glow','bool',noFlags, "Glow channel","",
		'UCam','bool',noFlags, "Use current Camera","",
//		'MerD','bool',noFlags, "Merge Shading Domains","",
//		'UseB','bool',noFlags, "Use Background Color","",
		'Type','in32',noFlags, "Normal Type (Classic or ZBrush)","",
		'Doma','in32',noFlags, "Shading Domain  treatment","",
		'BCol','colo',noFlags, "Background color","",
		'Spac','in32',noFlags, "Local or Global Space","",
		'Save','bool',noFlags, "Auto Save","",
		'Fold','Dstr',noFlags, "Save in folder","",
		'Open','bool',noFlags, "Open Image","",
	}
};

// Link the preferences to the command component
resource 'CINF' (850, "Baker Command")
{
	'pref',					// fPrefsCompID	: The ID of the component (from the kRID_PreferencesFamilyID family) that stores the preferences for this component (use kNoPrefs for no prefs)
	kAnyPlatform,			// fPlatforms	: On which platforms is the comp running (use platform defines from CommonComDefines.h)
	kAnyAppKind,			// fVersions	: On which version(s) (light/std/pro) is the component running (use host defines from CommonComDefines.h)
	kAnyAppMode,			// fVersionModes: On which mode(s) (normal/demo) is the component running (use host defines from CommonComDefines.h)
};


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Vector Map Shader
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (860)
{
	{
		R_IID_I3DExShader,
		R_CLSID_FinalNormalMapShader
	}
};

resource 'COMP' (860)
{
	kRID_ShaderFamilyID,
	'FNSh',
	"Baker Normal Map",
	"Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (860)
{
	{
		'Sh00','comp',interpolate,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Flip','bool',noFlags,"flip","",
		'Type','in32',noFlags,"Type","",
	}
};

resource 'CPUI' (860) 
{
	860,					// Id of your main part
	865,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Strings
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'STR#' (kChannelNames, purgeable)	
{
	{
		"Diffuse",	// 001
		"Highlight",	// 002
		"Shininess",	// 003
		"Bump",	// 004
		"Reflexion",	// 005
		"Transparency",	// 006
		"Refraction",	// 007
		"Glow",	// 008
	}
};

resource 'STR#' (kOtherNames, purgeable)	
{
	{
		"NormalMap",	// 001
		"LightMap",	// 002
		"Baking",	// 003
		"Merge Shading Domains", // 004
		"Split Shading Domains", // 005
		"Invalid Serial Number: Baker will work in Demo Version.", // 006
		"BakerData.dta", // 007
		"Demo Version: this shader isn't available.", // 008
		"Demo Version: size is limited to 128 by 128.", // 009
		"Baker",	// 010
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Part extension
//

// Scene command part

resource 'COMP' (870) 
{
	'part',
	'BakP',
	"Baker Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (870) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_BakerPart
	}
};

// Shader part

resource 'COMP' (880) 
{
	'part',
	'BaSP',
	"Baker Shader Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (880) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_BakerShaderPart
	}
};

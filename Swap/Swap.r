/****************************************************************************************************

		Swap.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#include "ExternalAPI.r"
#include "MFRtypes.r"
#include "Copyright.h"
#include "SwapDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"
#include "External3DAPI.r" // For STR#
#include "StandardBindingIDs.h"

#ifndef qUsingResourceLinker
#ifdef MAC
include "Swap.rsr";
#endif
#endif
#ifdef WIN
include "Swap.rsr";
#endif

resource 'COMP' (280)
{
	kRID_SceneCommandFamilyID,
	'SWAC',
	"Swap",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'scmd' (280)
{
	kSwapCommandID,
	defaultMenu,// Target menu ID if the Action Number is -1
	{// ID of the room where the Scene Command should appear
		'3Dvw',		// 3D View room
	},
	kStrings,							//int16 ID of STR# resource to use to load the strings BnGp needs
	4, // kDefaultName,						//int16 Index of string in STR# resource to use for fGroupName
	kEditBindingGroupID, // kNoGroup,							//int32 fGroupID
	{
		kSwapCommandID,					//int32 (unique!)ID of the bound command or tool (fCommandID)
		4,								//int16 Index of string in STR# resource to use for fCommandName
		'E',							//int16 fChar
		kNoShift,						//int16 fShift
		kCtrl,							//int16 fControl
		kNoAlt,							//int16 fAlt
		kAnyPlatform,					//int16 fPlatforms	On which platforms is the shortut defined (use platform defines from CommonComDefines.h)
	},
	k3DViewBindingContextID // kNoContext
};


resource 'PMap' (280)
{
	{
		'Type','in32',noFlags, "Swap Type","",
		'ObjN','Dstr',noFlags, "Object name","",
		'ShaN','Dstr',noFlags, "Shader name","",
		'TreI','in32',noFlags, "Tree Permanent ID","",
		'camc','comp',noFlags, "Camera Component","{fmly came Subm 0 MskI 0 MskE 0}",
		'ligc','comp',noFlags, "Light Component","{fmly lite Subm 0 MskI 0 MskE 0}",
		'OFit','bool',noFlags, "Object fit in","",
		'TFit','bool',noFlags, "Tree fit in","",
	}
};

resource 'GUID' (280)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_SwapCommand
	}
};


/////////////////////////////////////////////////////////////////////////////
//
// Part extension
//
resource 'COMP' (282) 
{
	'part',
	'SwaP',
	"Swap Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (282) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_SwapPart
	}
};


/////////////////////////////////////////////////////////////////////////////
//
// Strings
//

resource 'STR#' (kStrings, purgeable)	
{
	{
		"SwapData.dta",	// 01
		"Swapping...",	// 02
		"Swap shortcut",	// 03
		"Swap",	// 04
		"Select tree element",	// 05
		"Select a tree element to swap with",	// 06
	}
};


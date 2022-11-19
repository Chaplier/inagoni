/****************************************************************************************************

		Instanciator.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/


#include "ExternalAPI.r"
#include "MFRtypes.r"
#include "Copyright.h"
#include "InstanciatorDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"
#include "External3DAPI.r" // For STR#

#if (VERSIONNUMBER >= 0x050000)
#include "StandardBindingIDs.h"
#endif

#ifndef qUsingResourceLinker
#ifdef MAC
include "Replica.rsr";
#endif
#endif
#ifdef WIN
include "Replica.rsr";
#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'COMP' (750)
{
	kRID_SceneCommandFamilyID,
	'RepC',
	"Replica",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

#if (VERSIONNUMBER >= 0x050000)
resource 'scmd' (750)
{
	kReplicaCommandID,
	defaultMenu,// Target menu ID if the Action Number is -1
	{// ID of the room where the Scene Command should appear
		'3Dvw',		// 3D View room
	},
	kStrings,
	6,
	kEditBindingGroupID,
	{
		kReplicaCommandID,				//int32 (unique!)ID of the bound command or tool (fCommandID)
		6,								//int16 Index of string in STR# resource to use for fCommandName
		' ',							//int16 fChar
		kNoShift,						//int16 fShift
		kNoCtrl,						//int16 fControl
		kNoAlt,							//int16 fAlt
		kAnyPlatform,					//int16 fPlatforms	On which platforms is the shortut defined (use platform defines from CommonComDefines.h)
	},
	k3DViewBindingContextID
};
#else
resource 'scmd' (750)
{
	dynamicItem,// kReplicate, // Action Number to bind to or -1 for a dynamic new menu item
	defaultMenu,// Target menu ID if the Action Number is -1
	{// ID of the room where the Scene Command should appear
		'3Dvw',		// 3D View room
		'Stry',		// Storyboard room
	}
};
#endif

resource 'PMap' (750)
{
	{
		'RMod','in32',noFlags, "Replicate Mode","",

		'XCou','in32',noFlags, "X count","",
		'XSpa','re32',noFlags, "X spacing","",
		'YCou','in32',noFlags, "Y count","",
		'YSpa','re32',noFlags, "Y spacing","",
		'ZCou','in32',noFlags, "Z count","",
		'ZSpa','re32',noFlags, "Z spacing","",

		'RCou','in32',noFlags, "R count","",
		'Radi','re32',noFlags, "Radius","",
		'CZCo','in32',noFlags, "Cylinder Z count","",
		'CZSp','re32',noFlags, "Cylinder Z spacing","",
		'Angl','re32',noFlags, "Angle","",
		'torq','bool',noFlags, "Torque","",

		'TPid','in32',noFlags, "Tree Permanent ID","",
		'SuXs','re32',noFlags, "Surface X Density","",
		'SuYs','re32',noFlags, "Surface Y Density","",
		'alig','bool',noFlags, "Align normal","",
		'cube','bool',noFlags, "Cube Map","",
		'RMin','re32',noFlags, "Repartition Min","",
		'RMax','re32',noFlags, "Repartition Max","",

		'PPoX','re32',noFlags, "Perturb Pos X","",
		'PPoY','re32',noFlags, "Perturb Pos Y","",
		'PPoZ','re32',noFlags, "Perturb Pos Z","",
		'PScU','re32',noFlags, "Perturb Sca Uniform","",
		'PScX','re32',noFlags, "Perturb Sca X","",
		'PScY','re32',noFlags, "Perturb Sca Y","",
		'PScZ','re32',noFlags, "Perturb Sca Z","",
		'PRoX','re32',noFlags, "Perturb Rot X","",
		'PRoY','re32',noFlags, "Perturb Rot Y","",
		'PRoZ','re32',noFlags, "Perturb Rot Z","",
		'PSha','re32',noFlags, "Perturb Shading","",
		'ShaC','colo',noFlags, "Perturb Color","",
	
		'CoMx','in32',noFlags, "Max new shader count","",

		'SPoX','in32',noFlags, "shader ID for pos X","",
		'SPoY','in32',noFlags, "shader ID for pos Y","",
		'SPoZ','in32',noFlags, "shader ID for pos Z","",
		'SScU','in32',noFlags, "shader ID for sca Uniform","",
		'SScX','in32',noFlags, "shader ID for sca X","",
		'SScY','in32',noFlags, "shader ID for sca Y","",
		'SScZ','in32',noFlags, "shader ID for sca Z","",
		'SRoX','in32',noFlags, "shader ID for rot X","",
		'SRoY','in32',noFlags, "shader ID for rot Y","",
		'SRoZ','in32',noFlags, "shader ID for rot Z","",
		'SShM','in32',noFlags, "shader ID for shader modifier","",
		'ShIn','in32',noFlags, "Tree Permanent ID","",
//		'Sh00','comp',noFlags,"Shader Variation","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'GUID' (750)
{
	{
		R_IID_I3DExSceneCommand,
		R_CLSID_ReplicateCommand
	}
};

// Same PMAP than the scene command, to be able to save and load it
resource 'COMP' (751)
{
	kRID_DataComponentFamilyID,
	'RepD',
	"ReplicaForSave",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (751)
{
	{
		'RMod','in32',noFlags, "Replicate Mode","",

		'XCou','in32',noFlags, "X count","",
		'XSpa','re32',noFlags, "X spacing","",
		'YCou','in32',noFlags, "Y count","",
		'YSpa','re32',noFlags, "Y spacing","",
		'ZCou','in32',noFlags, "Z count","",
		'ZSpa','re32',noFlags, "Z spacing","",

		'RCou','in32',noFlags, "R count","",
		'Radi','re32',noFlags, "Radius","",
		'CZCo','in32',noFlags, "Cylinder Z count","",
		'CZSp','re32',noFlags, "Cylinder Z spacing","",
		'Angl','re32',noFlags, "Angle","",
		'torq','bool',noFlags, "Torque","",

		'TPid','in32',noFlags, "Tree Permanent ID","",
		'SuXs','re32',noFlags, "Surface X Density","",
		'SuYs','re32',noFlags, "Surface Y Density","",
		'alig','bool',noFlags, "Align normal","",
		'cube','bool',noFlags, "Cube Map","",
		'RMin','re32',noFlags, "Repartition Min","",
		'RMax','re32',noFlags, "Repartition Max","",

		'PPoX','re32',noFlags, "Perturb Pos X","",
		'PPoY','re32',noFlags, "Perturb Pos Y","",
		'PPoZ','re32',noFlags, "Perturb Pos Z","",
		'PScU','re32',noFlags, "Perturb Sca Uniform","",
		'PScX','re32',noFlags, "Perturb Sca X","",
		'PScY','re32',noFlags, "Perturb Sca Y","",
		'PScZ','re32',noFlags, "Perturb Sca Z","",
		'PRoX','re32',noFlags, "Perturb Rot X","",
		'PRoY','re32',noFlags, "Perturb Rot Y","",
		'PRoZ','re32',noFlags, "Perturb Rot Z","",
		'PSha','re32',noFlags, "Perturb Shading","",
		'ShaC','colo',noFlags, "Perturb Color","",
	
		'CoMx','in32',noFlags, "Max new shader count","",

		'SPoX','in32',noFlags, "shader ID for pos X","",
		'SPoY','in32',noFlags, "shader ID for pos Y","",
		'SPoZ','in32',noFlags, "shader ID for pos Z","",
		'SScU','in32',noFlags, "shader ID for sca Uniform","",
		'SScX','in32',noFlags, "shader ID for sca X","",
		'SScY','in32',noFlags, "shader ID for sca Y","",
		'SScZ','in32',noFlags, "shader ID for sca Z","",
		'SRoX','in32',noFlags, "shader ID for rot X","",
		'SRoY','in32',noFlags, "shader ID for rot Y","",
		'SRoZ','in32',noFlags, "shader ID for rot Z","",
		'SShM','in32',noFlags, "shader ID for shader modifier","",
		'ShIn','in32',noFlags, "Tree Permanent ID","",
//		'Sh00','comp',noFlags,"Shader Variation","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'data' (751)
{
	{
	}
};


resource 'GUID' (751)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_ReplicateData
	}
};


/////////////////////////////////////////////////////////////////////////////
//
// Tabs
//

resource 'TABS' (752)
{	
	{	// part, name, icon
		760, "Type", noIcon;
		761, "Perturbation", noIcon;
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Part extension
//

resource 'COMP' (770) 
{
	'part',
	'TypP',
	"Type Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (770) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_TypePart
	}
};

resource 'COMP' (775) 
{
	'part',
	'DisP',
	"Disturb Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (775) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_DisturbPart
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Toolbar
//
/*
resource 'TBAR' (19000, "Test", purgeable) 
{
	"Test", straightLayout,
	{
		720, 		"Replicate", noKey, noAction, alwaysEnabled, style_overlay { useShadow, 10052, 700};
	}
};
*/

/////////////////////////////////////////////////////////////////////////////
//
// Array Modifier
//


resource 'COMP' (752) {
	kRID_ModifierFamilyID,
	'COex',
	"Replica Array",
	"Inagoni",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (752) 
{
	{
		R_IID_I3DExModifier,         /*I3DExModifier*/
		R_CLSID_ArrayModifier
	}            
};

resource 'PMap' (752) {
	{	/* 5 UI elements plus 2 elements to save BBox */
		'Coun', 'in32', interpolate, "Element Count", "",
		'TarN', 'Dstr', noFlags, "Target Name", "",
		'Disp', 'bool', noFlags, "Display boxes only", "",
		'Tran', 'in32', noFlags, "Transformation mode", "",
		'gSca','re32',interpolate,"Global Scaling","",
		'xSca','re32',interpolate,"X Scaling","",
		'ySca','re32',interpolate,"Y Scaling","",
		'zSca','re32',interpolate,"Z Scaling","",
		'xOff','re32',interpolate,"X Offset","",
		'yOff','re32',interpolate,"Y Offset","",
		'zOff','re32',interpolate,"Z Offset","",
		'xRot','re32',interpolate,"X Rotation","",
		'yRot','re32',interpolate,"Y Rotation","",
		'zRot','re32',interpolate,"Z Rotation","",
		'bmin', 'vec3', noFlags, "BBox min", "",
		'bmax', 'vec3', noFlags, "BBox max", ""
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Strings
//

resource 'STR#' (kStrings, purgeable)	
{
	{
		"Default Color",	// 001
		"Default Noise",	// 002
		"InstanciatorData.dta",	// 003
		"_Replicate",	// 004
		"Replicate...",	// 005
		"Replica",	// 006
		"Choose Target",	// 007
		"Choose a target for the replication",	// 008
		"Demo Version: replication limited to 4 instances.", // 009
	}
};


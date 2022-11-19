/****************************************************************************************************

		Building.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#include "ExternalAPI.r"
#include "Copyright.h"
#include "BuildingDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"
#include "Types.r"
#include "External3DAPI.r"
#include "MFRTypes.r"  
#include "PropertiesRTypes.r"
#include "I3DEditorHostPartDefs.h"

#if (VERSIONNUMBER >= 0x050000)

#ifndef qUsingResourceLinker
#ifdef MAC
include "ArchiTools.rsr";
#endif
#endif
#ifdef WIN
include "ArchiTools.rsr";
#endif

#else

#ifndef qUsingResourceLinker
#ifdef MAC
include "Building_old.rsrc";
#endif
#endif
#ifdef WIN
include "Building_old.rsr";
#endif

#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Empty Building Primitive
//
resource 'COMP' (kBuildingPrimitiveResID)
{
	kRID_GeometricPrimitiveFamilyID,
	'BuiP',
	"Building",
	"My Sister Is Not A Boy",//"Hidden",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kBuildingPrimitiveResID)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_BuildingPrim
	}
};

// WARNING: keep kHousePrimitiveResID and kBuildingPrimitiveResID similar
resource 'PMap' (kBuildingPrimitiveResID)
{
	{
		'DLeH' , 're32',noFlags,"Default level height","",
		'DRMi' , 're32',noFlags,"Default roof min","",
		'DRMa' , 're32',noFlags,"Default roof max","",
		'DWaT' , 're32',noFlags,"Default wall thickness","",
		'DFlT' , 're32',noFlags,"Default floor thickness","",
		'DCeT' , 're32',noFlags,"Default ceiling thickness","",
		'DWiH' , 're32',noFlags,"Default window height","",
		'DWiL' , 're32',noFlags,"Default window length","",
		'DWiA' , 're32',noFlags,"Default window altitude","",
		'DDoH' , 're32',noFlags,"Default door height","",
		'DDoL' , 're32',noFlags,"Default door length","",
		'DStW' , 're32',noFlags,"Default Stairway width","",
		'DStL' , 're32',noFlags,"Default Stairway length","",
		'DSpa' , 're32',noFlags,"Default Grid Spacing","",
		'DWBX' , 're32',noFlags,"Default WB Size X","",
		'DWBY' , 're32',noFlags,"Default WB Size Y","",
		'DWBZ' , 're32',noFlags,"Default WB Size Z","",
		'DSnP' , 're32',noFlags,"Default Snap precision","",
		'DCoA' , 're32',noFlags,"Default Counstrain Angle","",
		'DSna' , 'bool',noFlags,"Default Snap To Grid","",
		'IsNe' , 'bool',noFlags,"Is New","",
		'PanC' , 'in32',noFlags,"Pane Config", "",
		'Show' , 'bool',noFlags,"Show All", "",
		'ShoL' , 'in32',noFlags,"Show Level", "",
		'FBEn' , 'bool',noFlags,"Front Back Backdrop enable", "",
		'LREn' , 'bool',noFlags,"Left Right Backdrop enable", "",
		'TBEn' , 'bool',noFlags,"Top Bottom Backdrop enable", "",
		'FBNa' , 'Dstr',noFlags,"Front Back Backdrop name", "",
		'LRNa' , 'Dstr',noFlags,"Left Right Backdrop name", "",
		'TBNa' , 'Dstr',noFlags,"Top Bottom Backdrop name", "",
		'Hole' , 'bool',noFlags,"Holes Edition On", "",
	}
};
#if (VERSIONNUMBER < 0x050000)
resource 'CPUI' (kBuildingPrimitiveResID) {
	0,						// BigPart  ID
	kAssembleRoomResID,		// MiniPart ID
	0,						// Style
	kParamsBeforeChildren,	//Where Param part is shown
	0						// Is Collapsable
};
#endif

///////////////////////////////////////////////////
//
// Default geom building primitive
//
/*
resource 'COMP' (kHousePrimitiveResID)
{
	kRID_GeometricPrimitiveFamilyID,
	'HouP',
	"House",
	"My Sister Is Not A Boy",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kHousePrimitiveResID)
{
	{
		R_IID_I3DExGeometricPrimitive,
		R_CLSID_HousePrim
	}
};

// WARNING: keep kHousePrimitiveResID and kBuildingPrimitiveResID similar
resource 'PMap' (kHousePrimitiveResID)
{
	{
		'DLeH' , 're32',noFlags,"Default level height","",
		'DRoH' , 're32',noFlags,"Default roof height","",
		'DWaT' , 're32',noFlags,"Default wall thickness","",
		'DFlT' , 're32',noFlags,"Default floor thickness","",
		'DCeT' , 're32',noFlags,"Default ceiling thickness","",
		'DWiH' , 're32',noFlags,"Default window height","",
		'DWiL' , 're32',noFlags,"Default window length","",
		'DWiA' , 're32',noFlags,"Default window altitude","",
		'DDoH' , 're32',noFlags,"Default door height","",
		'DDoL' , 're32',noFlags,"Default door length","",
		'DStW' , 're32',noFlags,"Default Stairway width","",
		'DStL' , 're32',noFlags,"Default Stairway length","",
		'DSpa' , 're32',noFlags,"Default Grid Spacing","",
		'DWBX' , 're32',noFlags,"Default WB Size X","",
		'DWBY' , 're32',noFlags,"Default WB Size Y","",
		'DWBZ' , 're32',noFlags,"Default WB Size Z","",
		'DSnP' , 're32',noFlags,"Default Snap precision","",
		'DCoA' , 're32',noFlags,"Default Counstrain Angle","",
		'DSna' , 'bool',noFlags,"Default Snap To Grid","",
		'IsNe' , 'bool',noFlags,"Is New","",
		'PanC' , 'in32',noFlags,"Pane Config", "",
		'Show' , 'bool',noFlags,"Show All", "",
		'ShoL' , 'in32',noFlags,"Show Level", "",
		'FBEn' , 'bool',noFlags,"Front Back Backdrop enable", "",
		'LREn' , 'bool',noFlags,"Left Right Backdrop enable", "",
		'TBEn' , 'bool',noFlags,"Top Bottom Backdrop enable", "",
		'FBNa' , 'Dstr',noFlags,"Front Back Backdrop name", "",
		'LRNa' , 'Dstr',noFlags,"Left Right Backdrop name", "",
		'TBNa' , 'Dstr',noFlags,"Top Bottom Backdrop name", "",
	}
};

resource 'CPUI' (kHousePrimitiveResID) {
	kAssembleRoomResID,		// BigPart  ID
	kAssembleRoomResID,		// MiniPart ID
	0,						// Style
	kParamsBeforeChildren, //Where Param part is shown
	0						// Is Collapsable
};
*/

/////////////////////////////////////////////////////////////////////////////

#if !NETWORK_RENDERING_VERSION

/////////////////////////////////////////////////////////////////////////////
//
// Building Modeler
//
resource 'COMP' (kBuildingModelerResID)
{
	'modu',				// External Module Family
	'BuiM',				// Class ID
	"Building Modeler",
	"Assemble",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'modu' (kBuildingModelerResID)
{
	0,							// Host window ID (here, main Perspective Window)
	kBuildingMenuBar,			// Menu bar ID (MBAR)
	kBuildingToolBar,			// Tool bar ID (TBAR)
	kBuildingPrefs,				// Preferences PMAP resource ID
	kBuildingPrefs,				// Preferences View ID
	kBuildingPrefs,				// Default Preferences values resource ID (prfs)
	'PeRs',						// Workspace class signature
	9410,						// Minimized iconID
	23,							// Window context menu ID (CMNU)
	'slav'
};



resource 'mdlr'	(kBuildingModelerResID)	
{
	cannotEditPatches,
	canEditFacets,	
	{
		'BuiP',	isNative,
	}
};

resource 'GUID' (kBuildingModelerResID)
{
	{
		R_IID_I3DExModule,
		R_CLSID_BuildingModeler
	}
};

// Kind of prefs for the modeler, saved in the file
resource 'PMap' (kBuildingModelerResID)
{
	{
		'Defa', 'in32', noFlags, "Default", "",
	}
};

//
// Modeler preferences
//

resource 'prfs' (kBuildingPrefs) 
{
	"{"
	" WBSX 720.0 WBSY 1080.0 WBSZ 432.0 SnaP 18 Spac 72 RotC 15 SptS 300 PtsS 33554431 3dun 4"
	"DefC 0.93,0.94,0.95 "		// Default color
	"ObjC 0.25,0.6,0.0 "		// Object color
	"SelC 1.0,0.2,0.2 "		// Select color
	"TarC 0.8,0.8,0.3 "		// Target color
	"FreC 0.3,0.3,0.8 "	// Freeze color
	"HelC 0.85,0.8,0.5 "		// Helper color
	"SnaC 0.6,0.5,0.2 "		// Snaped color
	"BRoC 0.30,0.30,0.70 "		// Roof bot color
	"TRoC 0.40,0.85,0.80 "		// Roof top color
	"2DFC 0.25,0.25,0.25 "		// 2D view Floor color
	"TxtC 0.99,0.1,0.1 "		// text color
	"TxtS 18 "					// text size
	"Snap 1 SDim 1 "
	"DLeH 108 DRMi 108 DRMa 252 DWaT 7.2 DFlT 3.6 DCeT 3.6 DWiH 43.2 DWiL 57.6 DWiA 50.4 DDoH 75.6 DDoL 43.2 DStW 72 DStL 72" // Dimension
	"}"
};

resource 'PMap' (kBuildingPrefs)
{
	{
		'WBSX', 're32', noFlags, "Working Box Size X", "",
		'WBSY', 're32', noFlags, "Working Box Size Y", "",
		'WBSZ', 're32', noFlags, "Working Box Size Z", "",
		'Spac', 're32', noFlags, "Grid Spacing", "",
		'SnaP', 're32', noFlags, "Snap Precision", "",
		'RotC', 're32', noFlags, "Rotation Constraint", "",
		'SptS', 'in32', noFlags, "Splitbar prefered size", "",
		'PtsS', 'in32', noFlags, "Points Shape", "",
		'3dun', 'in32', noFlags, "3D Units", "",
		'DefC', 'colo', noFlags, "Default Color", "",
		'ObjC', 'colo', noFlags, "Default Color", "",
		'SelC', 'colo', noFlags, "Select Color", "",
		'TarC', 'colo', noFlags, "Target Color", "",
		'FreC', 'colo', noFlags, "Freeze Color", "",
		'HelC', 'colo', noFlags, "Helper Color", "",
		'SnaC', 'colo', noFlags, "Snaped Color", "",
		'BRoC', 'colo', noFlags, "Bottom Roof Color", "",
		'TRoC', 'colo', noFlags, "Top Roof Color", "",
		'2DFC', 'colo', noFlags, "2D view floor Color", "",
		'TxtC', 'colo', noFlags, "Text Color", "",
		'TxtS', 'in32', noFlags, "Text Size", "",
		'Snap', 'bool', noFlags, "Snap To Grid", "",
		'SDim', 'bool', noFlags, "Show dimensions", "",
		'DLeH', 're32', noFlags, "Default Level Height", "",
		'DRMi', 're32', noFlags, "Default Roof Min", "",
		'DRMa', 're32', noFlags, "Default Roof Max", "",
		'DWaT', 're32', noFlags, "Default Wall Thickness", "",
		'DFlT', 're32', noFlags, "Default Floor Thickness", "",
		'DCeT', 're32', noFlags, "Default Ceiling Thickness", "",
		'DWiH', 're32', noFlags, "Default Window Height", "",
		'DWiL', 're32', noFlags, "Default Window Length", "",
		'DWiA', 're32', noFlags, "Default Window Altitude", "",
		'DDoH', 're32', noFlags, "Default Door Height", "",
		'DDoL', 're32', noFlags, "Default Door Length", "",
		'DStW', 're32', noFlags, "Default Stairway Width", "",
		'DStL', 're32', noFlags, "Default Stairway Length", "",
	}
};

//
// Modeler menus
//

// Menu bar
resource 'MBAR' (kBuildingMenuBar, "Building Modeler", purgeable)
{
	{kViewMenu, kCreateMenu, kSelectionMenu}
};

resource 'CMNU' (kViewMenu, "View", purgeable)
{ 
	kViewMenu,	textMenuProc,	0, enabled,
	"&View",
	{
		"&Zoom",			noIcon, noKey, noMark, plain, kaZoomSubMenu;
		"&Reset 2D Pan",	noIcon, noKey, noMark, plain, kaResetPan;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Display Active Level",	noIcon, noKey,	noMark, plain, kShowActiveLevel;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Show All",			noIcon, "\0x9c"/*Shortcut (Ctrl+)Shift+\*/,	noMark, plain, kShowAllMenuAction;
		"Show Walls",		noIcon, noKey,	noMark, plain, kShowWallsMenuAction;
		"Show Rooms",		noIcon, noKey,	noMark, plain, kShowRoomsMenuAction;
		"Show Roofs",		noIcon, noKey,	noMark, plain, kShowRoofsMenuAction;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Hide Selection",	noIcon, "\\"/*Shortcut (Ctrl+)\*/,	noMark, plain, kHideSelectionMenuAction;
		"Hide All",			noIcon, noKey,	noMark, plain, kHideAllMenuAction;
		"Hide Walls",		noIcon, noKey,	noMark, plain, kHideWallsMenuAction;
		"Hide Rooms",		noIcon, noKey,	noMark, plain, kHideRoomsMenuAction;
		"Hide Roofs",		noIcon, noKey,	noMark, plain, kHideRoofsMenuAction;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Holes edition handles",noIcon, "e"/*Shortcut (Ctrl+)E*/,	noMark, plain, kHolesEditionOnOff;
	}
};
// Shortcut didn't work with sub menu
//resource 'CMNU' (kShowSubmenu, "", purgeable)
//{ 
//	kShowSubmenu,	textMenuProc,	0, enabled,
//	"",
//	{
//		"Show All",			noIcon, "\0x9c"/*Shortcut (Ctrl+)Shift+\*/,	noMark, plain, kShowAllMenuAction;
//		"Show Walls",		noIcon, noKey,	noMark, plain, kShowWallsMenuAction;
//		"Show Rooms",		noIcon, noKey,	noMark, plain, kShowRoomsMenuAction;
//		"Show Roofs",		noIcon, noKey,	noMark, plain, kShowRoofsMenuAction;
//	}
//};
//
//resource 'CMNU' (kHideSubmenu, "", purgeable)
//{ 
//	kHideSubmenu,	textMenuProc,	0, enabled,
//	"",
//	{
//		"Hide Selection",	noIcon, "\\"/*Shortcut (Ctrl+)\*/,	noMark, plain, kHideSelectionMenuAction;
//		"Hide All",			noIcon, noKey,	noMark, plain, kHideAllMenuAction;
//		"Hide Walls",		noIcon, noKey,	noMark, plain, kHideWallsMenuAction;
//		"Hide Rooms",		noIcon, noKey,	noMark, plain, kHideRoomsMenuAction;
//		"Hide Roofs",		noIcon, noKey,	noMark, plain, kHideRoofsMenuAction;
//	}
//};

resource 'CMNU' (kCreateMenu, "Create", purgeable)
{ 
	kCreateMenu,	textMenuProc,	0, enabled,
	"&Create",
	{
		"Window",					noIcon, noKey,	noMark, plain, kInsertWindowMenuAction;
		"Door",						noIcon, noKey,	noMark, plain, kInsertDoorMenuAction;
		"Stairway",					noIcon, noKey,	noMark, plain, kInsertStairwayMenuAction;
		"Level",					noIcon, hierarchicalMenu, kInsertLevelSubmenuStr, plain, nocommand;
		"Room",						noIcon, noKey,	noMark, plain, kCreateRoomMenuAction;
		"Roof",						noIcon, hierarchicalMenu, kCreatetRoofSubmenuStr, plain, nocommand;
	}
};

resource 'CMNU' (kInsertLevelSubmenu, "", purgeable)
{ 
	kInsertLevelSubmenu,	textMenuProc,	0, enabled,
	"",
	{
		"New Floor", noIcon, noKey,	noMark, plain, kLevelTopMenuAction;
		"New Basement", noIcon, noKey,	noMark, plain, kLevelBottomMenuAction;
		"Duplicate current", noIcon, noKey,	noMark, plain, kLevelOnActiveMenuAction;
	}
};

resource 'CMNU' (kCreatetRoofSubmenu, "", purgeable)
{ 
	kCreatetRoofSubmenu,	textMenuProc,	0, enabled,
	"",
	{
		"Rectangle Roof",			74, noKey,	noMark, plain, kCreateRoof3MenuAction;
		"Rectangle Roof X",			75, noKey,	noMark, plain, kCreateRoof4MenuAction;
		"Rectangle Roof Y",			76, noKey,	noMark, plain, kCreateRoof5MenuAction;
		"Half Roof 1",			77, noKey,	noMark, plain, kCreateRoof6MenuAction;
		"Half Roof 2",			78, noKey,	noMark, plain, kCreateRoof7MenuAction;
		"Half Roof 3",			79, noKey,	noMark, plain, kCreateRoof8MenuAction;
		"Half Roof 4",			80, noKey,	noMark, plain, kCreateRoof9MenuAction;
		"Same shape, constant Slope",		81, noKey,	noMark, plain, kCreateRoof1MenuAction;
		"Same shape, no flat",				82, noKey,	noMark, plain, kCreateRoof2MenuAction;
		"Border",				83, noKey,	noMark, plain, kCreateRoof10MenuAction;
	}
};

resource 'CMNU' (kSelectionMenu, "Selection", purgeable)
{ 
	kSelectionMenu,	textMenuProc,	0, enabled,
	"&Selection",
	{
		"Set Level Height",			noIcon, noKey,	noMark, plain, kLevelHeightMenuAction;
		"Set Wall Thickness",		noIcon, noKey,	noMark, plain, kWallThicknessMenuAction;
		"Set Wall Height",			noIcon, noKey,	noMark, plain, kWallHeightMenuAction;
		"Set Wall Arc Offset",		noIcon, noKey,	noMark, plain, kWallArcOffsetMenuAction;
		"Set Wall Arc Segments",	noIcon, noKey,	noMark, plain, kWallArcSegmentsMenuAction;
		"Set Floor Thickness",		noIcon, noKey,	noMark, plain, kFloorThicknessMenuAction;
		"Set Ceiling Thickness",	noIcon, noKey,	noMark, plain, kCeilingThicknessMenuAction;
		"Set Roof Max Height",		noIcon, noKey,	noMark, plain, kRoofHeightMenuAction;
		"Set Roof Min Height",		noIcon, noKey,	noMark, plain, kRoofBaseMenuAction;
		"Set Child Object",			noIcon, noKey,	noMark, plain, kChildMenuAction;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Replace By",				noIcon, hierarchicalMenu,	kReplaceSubmenuStr, plain, 0;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"Rebuild",					noIcon, noKey,	noMark, plain, kRebuildMenuAction;
		"Split 2 Points",			noIcon, noKey,	noMark, plain, kSplitMenuAction;
		"Merge 2 Points",			noIcon, noKey,	noMark, plain, kMergeMenuAction;
		"Detach",					noIcon, noKey,	noMark, plain, kDetachMenuAction;
		"Move Over",				noIcon, noKey,	noMark, plain, kMoveOverMenuAction;
		"Move Under",				noIcon, noKey,	noMark, plain, kMoveUnderMenuAction;
//		"-", noIcon, noKey,	noMark, plain, nocommand;
//		"Import Curve...",			noIcon, noKey,	noMark, plain, kImportCurveMenuAction;
		"-", noIcon, noKey,	noMark, plain, nocommand;
		"&Invert Selection",		noIcon, "i"/*Shortcut (Ctrl+)I*/,	noMark, plain, kInvertSelectionMenuAction;
		"Select By",				noIcon, hierarchicalMenu,	kSelectSubmenuStr, plain, 0;
		"Deselect By",				noIcon, hierarchicalMenu,	kDeselectSubmenuStr, plain, 0;
	}
};

resource 'CMNU' (kReplaceSubmenu, "", purgeable) { 
	kReplaceSubmenu,	textMenuProc,	0, enabled,
	"",
	{
		"Simple Wall",				noIcon, noKey,	noMark, plain, kReplaceBySimpleWall;
		"Wall With Crenel",			noIcon, noKey,	noMark, plain, kReplaceByWallWithCrenel;
	}
};

resource 'CMNU' (kSelectSubmenu, "", purgeable) { 
	kSelectSubmenu,	textMenuProc,	0, enabled,
	"",
	{
		"Name...",				noIcon, noKey,	noMark, plain, kSelectByName;
		"Shading Domain...",	noIcon, noKey,	noMark, plain, kSelectByShadingDomain;
	}
};


resource 'CMNU' (kDeselectSubmenu, "", purgeable) { 
	kDeselectSubmenu,	textMenuProc,	0, enabled,
	"",
	{
		"Name...",				noIcon, noKey,	noMark, plain, kDeselectByName;
		"Shading Domain...",	noIcon, noKey,	noMark, plain, kDeselectByShadingDomain;
	}
};

#if (VERSIONNUMBER >= 0x050000)
resource 'ghmn' (1407, "Building Modeler Ghost Menu")
{
	{
		0, 2, 1307, 3001, "Wall",		kBuildWallTool, 
		1, 1, 1308, 3011, "Door XL",	kInsertDoubleDoorTool, 
		1, 2, 1309, 3008, "Door L",		kInsertDoorTool, 
		2, 0, 1310, 3010, "Win XL",		kInsertPanoWindowTool, 
		2, 1, 1311, 3007, "Win L",		kInsertWindowTool, 
		2, 2, 1311, 3009, "Win S",		kInsertNarrowWindowTool, 
		3, 0, 1313, 3017, "Square",		kInsertSquareStairwayTool, 
		3, 1, 1314, 3019, "Wide",		kInsertWideStairwayTool, 
		3, 2, 1315, 3018, "Large",		kInsertLargeStairwayTool, 
		4, 0, 1316, 3029, "Empty",		kInsertEmptyLevelTool, 
		4, 1, 1317, 3027, "Rect",		kInsertShellLevelTool, 
		4, 2, 1318, 3028, "Copy",		kInsertDuplicateLevelTool, 
		3, 3, 1319, 3037, "Delete",		kDeleteTool, 
		1, 4, 16101, 18005, "Scale",	18005, 
		1, 5, 16105, 5161, "Zoom",		5161, 
		2, 4, 16102, 18003, "Move",		18003, 
		2, 5, 16106, 5160, "Pan",		5160, 
		3, 4, 16103, 18004, "Rotate",	18004, 
		3, 5, 16107, 18009, "Dolly",	18009, 
	}
};
#endif

//
// Modeler tool bar
//

resource 'TBAR' (kBuildingToolBar, "Standard", purgeable) 
{
	"Standard", straightLayout,
	{
		kBuildWallTool,				"Wall",				"a"/*Shortcut a*/,	noAction, normalEnabling, style_overlay { useShadow, 10052, 307 };
		tBeginToolGroup,			"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kBuildWallWithCrenel1Tool,	"Wall With Rectangular Crenel",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 308 };
		kBuildWallWithCrenel2Tool,	"Wall With Small Top Crenel",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 309 };
		kBuildWallWithCrenel3Tool,	"Wall With Double Crenel",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 310 };
		kBuildWallWithCrenel4Tool,	"Wall With Slope Sides Crenel",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 311 };
		kBuildWallWithCrenel5Tool,	"Wall With Slop Top Crenel",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 312 };
		tEndToolGroup,				"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		tBeginToolGroup,			"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kInsertWindowTool, 			"Window",			""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 317 };
		kInsertNarrowWindowTool, 	"Narrow Window",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 319 };
		kInsertPanoWindowTool, 		"Panoramic Window",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 320 };
		kInsertArrowWindowTool, 	"Arrow Window",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 322 };
		kInsertCircleWindowTool, 	"Circle Window",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 326 };
		kInsert2CircleWindowTool, 	"Half Circle Window",""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 323 };
		kInsert4CircleLWindowTool, 	"Quater Circle Left Window",""/*Shortcut */,noAction, normalEnabling, style_overlay { useShadow, 10052, 324 };
		kInsert4CircleRWindowTool, 	"Quater Circle Right Window",""/*Shortcut */,noAction, normalEnabling, style_overlay { useShadow, 10052, 325 };
		tEndToolGroup,				"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		tBeginToolGroup,			"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kInsertDoorTool,			"Door",				""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 318 };
		kInsertDoubleDoorTool,		"Double Door",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 321 };
		kInsertArrowDoorTool, 		"Arrow Door",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 331 };
		kInsert2CircleDoorTool, 	"Half Circle Door",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 332 };
		kInsert4CircleLDoorTool, 	"Left Arch",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 333 };
		kInsert4CircleRDoorTool, 	"Right Arch",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 334 };
		tEndToolGroup,				"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		tBeginToolGroup,			"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kInsertSquareStairwayTool,	"Square Stairway",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 327 };
		kInsertLargeStairwayTool,	"Large Stairway",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 328 };
		kInsertWideStairwayTool,	"Wide Stairway",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 329 };
		kInsertCircleStairwayTool,	"Circle Stairway",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 330 };
		tEndToolGroup,				"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		tBeginToolGroup,			"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kInsertShellLevelTool,		"Rectangle Level",	""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 337 };
		kInsertDuplicateLevelTool,	"Duplicate Level Under", ""/*Shortcut */, noAction, normalEnabling, style_overlay { useShadow, 10052, 338 };
		kInsertEmptyLevelTool,		"Empty Level",		""/*Shortcut */,	noAction, normalEnabling, style_overlay { useShadow, 10052, 339 };
		tEndToolGroup,				"",					noKey,				noAction, normalEnabling, style_normal { noShadow, 0, 0, 0, 0, 0, 0 },
		kDeleteTool,				"Delete",			"f"/*Shortcut f*/,	noAction, normalEnabling, style_overlay { useShadow, 10052, 347 };
	}
};



/////////////////////////////////////////////////////////////////////////////
//
// Building modeler Pane Part Component Definition
//
resource 'COMP' (kBuildingModelerPanePartResID) 
{
	'part',
	'BMPP',
	"Building Modeler Pane Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kBuildingModelerPanePartResID) 
{
	{
		R_IID_IMFExPart,
		R_CLSID_BuildingPanePart
	}
};
//
/////////////////////////////////////////////////////////////////////////////

#endif // !NETWORK_RENDERING_VERSION

/////////////////////////////////////////////////////////////////////////////
//
// Roof Profile Data Comp
//
resource 'data' (kBuildingRoofProfileDataComp) 
{
	{	// applies to primitives, lights and cameras
//	'prim',
//	'li  ',
//	'ca  '
	}
};


resource 'GUID' (kBuildingRoofProfileDataComp) 
{
	{
	R_IID_I3DExDataComponent,
	R_CLSID_BuildingRoofProfileComp
	}
};


resource 'COMP' (kBuildingRoofProfileDataComp) 
{
	'data',
	'RooP',
	"Roof Profile",
	"Basic",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (kBuildingRoofProfileDataComp)
{
	{
	'BOCo','in32',noFlags,"Bottom Out Count","",
	'Vec0','vec2',noFlags,"Vector 0","",
	'Vec1','vec2',noFlags,"Vector 1","",
	'Vec2','vec2',noFlags,"Vector 2","",
	'Vec3','vec2',noFlags,"Vector 3","",
	'Vec4','vec2',noFlags,"Vector 4","",
	'Vec5','vec2',noFlags,"Vector 5","",
	}
};

#if !NETWORK_RENDERING_VERSION

/////////////////////////////////////////////////////////////////////////////
//
// Building modeler Properties
//
resource 'Prop' (kBuildingPropertiesResID) 
{
	Tabs
	{
		{
			"Modelisation", 'mode', 700,
			Panels
			{ 
				{
					'Leve', Panel { "Active Level", 3003, expanded };	
					'Posi', Panel { "Selection Properties", 3001, expanded };	
					'Shad', Panel { "Selection Shading", 3002, expanded };	
				}
			};
			"Default Parameters", 'defa', 700,
			Panels
			{ 
				{
					'DDim', Panel { "Dimensions",	3308, expanded };	
					'DWor', Panel { "Working Box",	3309, expanded };	
				}
			};
			"Global", 'glob', 700,
			Panels
			{ 
				{
					'Back', Panel { "Backdrop", 3340, expanded };	
					'ShDm', Panel { "Shading Domains Management", 3350, expanded };	
// Later					'Shdr', Panel { "Shaders For Object", kShadersPart, expanded };	
				}
			};
		}
	};
};
//
/////////////////////////////////////////////////////////////////////////////

// Current tool options tabs parts
resource 'TABS' (150)
{	
	{	// part, name, icon
		7301, "Active Tool", 700,			// 1
		7310, "Move", 700,					// 2
		7320, "Scale", 700,				// 3
		7330, "Rotate", 700,				// 4
		7301, "Camera", 700,				// 5
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Drag And Drop
//

// Drop candidate component
resource 'COMP' (kBuildingDropCandidateResID)
{
	'drpC',
	'BMDC',
	"Building Modeler Drop Candidate",	
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kBuildingDropCandidateResID)
{
	{
		R_IID_IMFExDropCandidate,
		R_CLSID_BuildingDropCandidate
	}
};

// Drop Area component
resource 'COMP' (kBuildingDropAreaResID) 
{
	'drpA',
	'BMDA',
	"Building Modeler Drop Area",	
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kBuildingDropAreaResID) 
{
	{
		R_IID_IMFExDropArea,
		R_CLSID_BuildingDropArea
	}
};
//
/////////////////////////////////////////////////////////////////////////////

#endif // !NETWORK_RENDERING_VERSION

/////////////////////////////////////////////////////////////////////////////
//
// Strings
//
resource 'STR#' (kModelerStrings, "Modeler Strings", purgeable) 
{
	{
		" of ",			// 01
		"Selection",	// 02
		"Move",			// 03
		"Scale",		// 04
		"Rotate",		// 05
		"Insert Object",// 06
		"Make Wall",	// 07
		"Delete",		// 08
		"Insert Level",	// 09
		"Create Room",	// 10
		"Level ",		// 11
		"Basement ",	// 12
		"No Child",		// 13
		"Level Height",	// 14
		"Attach Object",// 15
		"Show Hide",	// 16
		"Set Default Setting",	// 17
		"Create Roof",	// 18
		"Wall Height",	// 19
		"Roof Height",	// 20
		"Invert Selection",	// 21
		"Multiple domains",	// 22
		"New shading domain...",	// 23
		"Left:",	// 24
		"Right:",	// 25
		"Texture ",	// 26
		"Floor:",	// 27
		"Ceiling:",	// 28
		"Walls:",	// 29
		"Top:",		// 30
		"Middle:",	// 31
		"Bottom:",	// 32
		"Inside:",	// 33
		"Wall Thickness",	// 34
		"Roof Altitude",	// 35
		"Show Active Level",	// 36
		"Split",	// 37
		"Placement Type",	// 38
		"Multiple objects",	// 39
		"Draw Ceiling",	// 40
		"Merge",	// 41
		"Name",	// 42
		"Select By Name", // 43
		"Deselect By Name", // 44
		"Select By Domain", // 45
		"Deselect By Domain", // 46
		"Duplicate Over", // 47
		"Duplicate Under", // 48
		"Floor Thickness",	// 49
		"Ceiling Thickness",	// 50
		"Wall Height",	// 51
		"Roof Max Height",	// 52
		"Roof Min Height",	// 53
		"Edition Mode on/Off",	// 54
		"Wall Arc Offset",	// 55
		"Wall Arc Segments",	// 56
		"Wall Arc",	// 57
		"Crenel Height",	// 58
		"Crenel Width",	// 59
		"Crenel Spacing",	// 60
		"Crenel Offset",	// 61
		"Crenel Shape",	// 62
		"Do you want to unfold the UV map ?\nUnfolding UV can be usefull for lightmap baking.\nWarning: if the model is already shaded, this may change the way it renders.",	// 63
		"Replace Wall", // 64
	}
};

resource 'STR#' (kPrimitiveStrings, "Primitive Strings", purgeable) 
{
	{
		"Default",	// 01
		"cannot find the file:", // 02
		"Window", // 03
		"Narrow Window", // 04
		"Wide Window", // 05
		"Door", // 06
		"Double Door", // 07
		"Wall", // 08
		"Room", // 09
		"Roof", // 10
		"Stairway", // 11
		"Wall Point", // 12
		"Roof Point", // 13
		"Profile Point", // 14
		"Outline Point", // 15
	}
};

resource 'STR#' (kShadingDomainsStrings, "Modeler Strings", purgeable) 
{
	{
		"Wall Out",	// 01
		"Wall In ",	// 02
		"Floor",	// 03
		"Ceiling",	// 04
		"Roof Bottom",	// 05
		"Roof Middle",	// 06
		"Roof Top",	// 07
		"Roof Inside",	// 08
	}
};

resource 'STR#' (kAlertStrings, "Alert Strings", purgeable) 
{
	{
		"Select a wall first",	// 01
		"Select a room first",	// 02
		"The ArchiTools version instaled is a demo version: the building won't be saved.",	// 03
		"The ArchiTools version instaled is a demo version: the building won't be loaded.",	// 04
		"The ArchiTools version instaled is a network rendering version: please reinstall a proper version instead.",	// 05
		"The ArchiTools version instaled is not a network rendering version: please reinstall a proper version instead.",	// 06
		"The ArchiTools version instaled is a demo version: the building can only have a limited number of level and walls.",	// 07
		"The ArchiTools version instaled is a demo version: the building won't be exported.",	// 08
	}
};

//
/////////////////////////////////////////////////////////////////////////////

#if !NETWORK_RENDERING_VERSION

/////////////////////////////////////////////////////////////////////////////
//
// Assemble Room
//

resource 'COMP' (kAssembleRoomPartExtID) 
{
	'part',
	'AssP',
	"Assemble Room Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (kAssembleRoomPartExtID) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_AssembleRoomPart
	}
};

#endif // !NETWORK_RENDERING_VERSION

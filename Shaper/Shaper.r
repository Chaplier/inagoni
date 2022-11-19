/****************************************************************************************************

		Shaper.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/


#include "ExternalAPI.r"
#include "MFRtypes.r"
#include "Copyright.h"
#include "Shaper.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"
#include "External3DAPI.r" // For STR#
#include "StandardBindingIDs.h"

#ifndef qUsingResourceLinker
#ifdef MAC
include "Shaper.rsr";
#endif
#endif

#ifdef WIN
include "Shaper.rsr";
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Modifier
//


resource 'COMP' (810) {
	kRID_ModifierFamilyID,
	'FFDf',
	"Free Form Deformer",
	"Inagoni",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (810) 
{
	{
		R_IID_I3DExModifier,         /*I3DExModifier*/
		R_CLSID_FreeFormModifierNxN
	}            
};

resource 'PMap' (810) {
	{
		'xNum', 'in32', noFlags, "Number of section in X", "",
		'yNum', 'in32', noFlags, "Number of section in Y", "",
		'zNum', 'in32', noFlags, "Number of section in Z", "",
		'bmin','vec3',noFlags,"","",
		'bmax','vec3',noFlags,"","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Modifier 2x2
//


resource 'COMP' (812) {
	kRID_ModifierFamilyID,
	'FFD2',
	"Free Form Deformer 2x2",
	"Inagoni",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (812) 
{
	{
		R_IID_I3DExModifier,         /*I3DExModifier*/
		R_CLSID_FreeFormModifier2x2
	}            
};

resource 'PMap' (812) {
	{
		'h__1', 'vec3', interpolate, "Handle 1", "",
		'h__2', 'vec3', interpolate, "Handle 2", "",
		'h__3', 'vec3', interpolate, "Handle 3", "",
		'h__4', 'vec3', interpolate, "Handle 4", "",
		'h__5', 'vec3', interpolate, "Handle 5", "",
		'h__6', 'vec3', interpolate, "Handle 6", "",
		'h__7', 'vec3', interpolate, "Handle 7", "",
		'h__8', 'vec3', interpolate, "Handle 8", "",
		'bmin','vec3',noFlags,"","",
		'bmax','vec3',noFlags,"","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Modifier 3x3
//


resource 'COMP' (813) {
	kRID_ModifierFamilyID,
	'FFD3',
	"Free Form Deformer 3x3",
	"Inagoni",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (813) 
{
	{
		R_IID_I3DExModifier,         /*I3DExModifier*/
		R_CLSID_FreeFormModifier3x3
	}            
};

resource 'PMap' (813) {
	{
		'h__1', 'vec3', interpolate, "Handle 1", "",
		'h__2', 'vec3', interpolate, "Handle 2", "",
		'h__3', 'vec3', interpolate, "Handle 3", "",
		'h__4', 'vec3', interpolate, "Handle 4", "",
		'h__5', 'vec3', interpolate, "Handle 5", "",
		'h__6', 'vec3', interpolate, "Handle 6", "",
		'h__7', 'vec3', interpolate, "Handle 7", "",
		'h__8', 'vec3', interpolate, "Handle 8", "",
		'h__9', 'vec3', interpolate, "Handle 9", "",
		'h_10', 'vec3', interpolate, "Handle 10", "",
		'h_11', 'vec3', interpolate, "Handle 11", "",
		'h_12', 'vec3', interpolate, "Handle 12", "",
		'h_13', 'vec3', interpolate, "Handle 13", "",
		'h_14', 'vec3', interpolate, "Handle 14", "",
		'h_15', 'vec3', interpolate, "Handle 15", "",
		'h_16', 'vec3', interpolate, "Handle 16", "",
		'h_17', 'vec3', interpolate, "Handle 17", "",
		'h_18', 'vec3', interpolate, "Handle 18", "",
		'h_19', 'vec3', interpolate, "Handle 19", "",
		'h_20', 'vec3', interpolate, "Handle 20", "",
		'h_21', 'vec3', interpolate, "Handle 21", "",
		'h_22', 'vec3', interpolate, "Handle 22", "",
		'h_23', 'vec3', interpolate, "Handle 23", "",
		'h_24', 'vec3', interpolate, "Handle 24", "",
		'h_25', 'vec3', interpolate, "Handle 25", "",
		'h_26', 'vec3', interpolate, "Handle 26", "",
		'h_27', 'vec3', interpolate, "Handle 27", "",
		'bmin','vec3',noFlags,"","",
		'bmax','vec3',noFlags,"","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Modifier 4x4
//


resource 'COMP' (814) {
	kRID_ModifierFamilyID,
	'FFD4',
	"Free Form Deformer 4x4",
	"Inagoni",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (814) 
{
	{
		R_IID_I3DExModifier,         /*I3DExModifier*/
		R_CLSID_FreeFormModifier4x4
	}            
};

resource 'PMap' (814) {
	{
		'h__1', 'vec3', interpolate, "Handle 1", "",
		'h__2', 'vec3', interpolate, "Handle 2", "",
		'h__3', 'vec3', interpolate, "Handle 3", "",
		'h__4', 'vec3', interpolate, "Handle 4", "",
		'h__5', 'vec3', interpolate, "Handle 5", "",
		'h__6', 'vec3', interpolate, "Handle 6", "",
		'h__7', 'vec3', interpolate, "Handle 7", "",
		'h__8', 'vec3', interpolate, "Handle 8", "",
		'h__9', 'vec3', interpolate, "Handle 9", "",
		'h_10', 'vec3', interpolate, "Handle 10", "",
		'h_11', 'vec3', interpolate, "Handle 11", "",
		'h_12', 'vec3', interpolate, "Handle 12", "",
		'h_13', 'vec3', interpolate, "Handle 13", "",
		'h_14', 'vec3', interpolate, "Handle 14", "",
		'h_15', 'vec3', interpolate, "Handle 15", "",
		'h_16', 'vec3', interpolate, "Handle 16", "",
		'h_17', 'vec3', interpolate, "Handle 17", "",
		'h_18', 'vec3', interpolate, "Handle 18", "",
		'h_19', 'vec3', interpolate, "Handle 19", "",
		'h_20', 'vec3', interpolate, "Handle 20", "",
		'h_21', 'vec3', interpolate, "Handle 21", "",
		'h_22', 'vec3', interpolate, "Handle 22", "",
		'h_23', 'vec3', interpolate, "Handle 23", "",
		'h_24', 'vec3', interpolate, "Handle 24", "",
		'h_25', 'vec3', interpolate, "Handle 25", "",
		'h_26', 'vec3', interpolate, "Handle 26", "",
		'h_27', 'vec3', interpolate, "Handle 27", "",
		'h_28', 'vec3', interpolate, "Handle 28", "",
		'h_29', 'vec3', interpolate, "Handle 29", "",
		'h_30', 'vec3', interpolate, "Handle 30", "",
		'h_31', 'vec3', interpolate, "Handle 31", "",
		'h_32', 'vec3', interpolate, "Handle 32", "",
		'h_33', 'vec3', interpolate, "Handle 33", "",
		'h_34', 'vec3', interpolate, "Handle 34", "",
		'h_35', 'vec3', interpolate, "Handle 35", "",
		'h_36', 'vec3', interpolate, "Handle 36", "",
		'h_37', 'vec3', interpolate, "Handle 37", "",
		'h_38', 'vec3', interpolate, "Handle 38", "",
		'h_39', 'vec3', interpolate, "Handle 39", "",
		'h_40', 'vec3', interpolate, "Handle 40", "",
		'h_41', 'vec3', interpolate, "Handle 41", "",
		'h_42', 'vec3', interpolate, "Handle 42", "",
		'h_43', 'vec3', interpolate, "Handle 43", "",
		'h_44', 'vec3', interpolate, "Handle 44", "",
		'h_45', 'vec3', interpolate, "Handle 45", "",
		'h_46', 'vec3', interpolate, "Handle 46", "",
		'h_47', 'vec3', interpolate, "Handle 47", "",
		'h_48', 'vec3', interpolate, "Handle 48", "",
		'h_49', 'vec3', interpolate, "Handle 49", "",
		'h_50', 'vec3', interpolate, "Handle 50", "",
		'h_51', 'vec3', interpolate, "Handle 51", "",
		'h_52', 'vec3', interpolate, "Handle 52", "",
		'h_53', 'vec3', interpolate, "Handle 53", "",
		'h_54', 'vec3', interpolate, "Handle 54", "",
		'h_55', 'vec3', interpolate, "Handle 55", "",
		'h_56', 'vec3', interpolate, "Handle 56", "",
		'h_57', 'vec3', interpolate, "Handle 57", "",
		'h_58', 'vec3', interpolate, "Handle 58", "",
		'h_59', 'vec3', interpolate, "Handle 59", "",
		'h_60', 'vec3', interpolate, "Handle 60", "",
		'h_61', 'vec3', interpolate, "Handle 61", "",
		'h_62', 'vec3', interpolate, "Handle 62", "",
		'h_63', 'vec3', interpolate, "Handle 63", "",
		'h_64', 'vec3', interpolate, "Handle 64", "",
		'bmin','vec3',noFlags,"","",
		'bmax','vec3',noFlags,"","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Part extension
//

resource 'COMP' (910) 
{
	'part',
	'FFPN',
	"Free Form Part NxN",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (910) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_FreeFormPartExt
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Strings
//

resource 'STR#' (kStrings, purgeable)	
{
	{
		"Default",	// 001
		"Demo Version: only the handles at the bottom of the deformation box can be moved." // 002
	}
};


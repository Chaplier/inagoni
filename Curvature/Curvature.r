/****************************************************************************************************

		Curvature.r
		Copyright: (c) 2005 Alan Stafford. All rights reserved.

		Author:	Alan
		Date:	6/10/2005

****************************************************************************************************/


#include "ExternalAPI.r"
#include "MFRtypes.r"
#include "Copyright.h"
#include "CurvatureDef.h"
#include "interfaceids.h"
#include "External3DAPI.r" // For STR#

#ifndef qUsingResourceLinker
#ifdef MAC
include "Curvature.rsr";
#endif
#endif
#ifdef WIN
include "Curvature.rsr";
#endif

#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

resource 'GUID' (950)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Curvature
	}
};

resource 'COMP' (950)
{
	kRID_ShaderFamilyID,
	'MDcv',
	"Curvature",
	"Geometry",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

  
resource 'PMap' (950)
{
	{
	'GCUR','bool',interpolate,"gaussian curvature","",
	'MCUR','bool',interpolate,"mean curvature","",
	'K1CR','bool',interpolate,"k1 principal curvature","",
	'K2CR','bool',interpolate,"k2 principal curvature","",
	'PVec','bool',interpolate,"ShowPrincipal","",
	'mCUR','bool',interpolate,"mixed curvature","",
	'CMAX','re32',interpolate,"max_pos","",
	'SMAX','re32',interpolate,"max_pos_per","",
	'PClp','bool',interpolate,"PosSoftClip","",
	'CMIN','re32',interpolate,"max_neg","",
	'SMIN','re32',interpolate,"max_neg_per","",
	'NClp','bool',interpolate,"NegSoftClip","",
	'Clsf','bool',interpolate,"Classify","",
	'Flip','bool',interpolate,"ReverseConvex","",
	'Defn','bool',interpolate,"Definition","",
	'RTog','bool',interpolate,"RedOn","",
	'GTog','bool',interpolate,"GreenOn","",
	'BTog','bool',interpolate,"BlueOn","",
	'RGry','bool',interpolate,"RedGray","",
	'GGry','bool',interpolate,"GreenGray","",
	'BGry','bool',interpolate,"BlueGray","",
	'SQRT','bool',interpolate,"SqRoot","",
	'Enab','bool',interpolate,"Enable","",
	'Norm','bool',interpolate,"Nomals","",
	'Seam','bool',interpolate,"Seams","",
	'Auto','bool',interpolate,"Auto","",
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Strings
//

resource 'STR#' (kStrings, purgeable)	
{
	{
		"Curvature.dta",	// 001
	}
};

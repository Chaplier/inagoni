/****************************************************************************************************

		Gas.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/20/2004

****************************************************************************************************/


#include "ExternalAPI.r"
#include "External3DAPI.r" // For STR#
#include "Copyright.h"
#include "GasDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"



#ifndef qUsingResourceLinker
#ifdef MAC
include "Primivol.rsr";
#endif
#endif
#ifdef WIN
include "Primivol.rsr";
#endif


#if (VERSIONNUMBER < 0x040000)
#define FIRSTVERSION 1
#endif

///////////////////////////////////////////////////////////////////
//
// Fire
//
resource 'GUID' (200)
{
	{
		R_IID_I3DExGeometricPrimitive, //	R_IID_I3DExVolumetricEffect,
		R_CLSID_Fire
	}
};

resource 'COMP' (200)
{
	'prim', // 'Volu',
	'Fire',
	"Primivol: Fire",
	"Inagoni Volume",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (200)
{
	{
	'Size','re32',noFlags,"Primitive Size","",

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

	// Gas
	'smoo','re32',interpolate,"Smooth","",
	'qual','re32',interpolate,"Quality","",
	'dens','re32',interpolate,"Density","",
	'SelI','re32',interpolate,"Self Intensity","",

	// Noise
	'seed','re32',noFlags,"Seed","",
	'frac','re32',interpolate,"Fractal Depth","",
	'gain','re32',interpolate,"Contrast","",
	'bias','re32',interpolate,"Brightness","",
	'ShaN','Dstr',noFlags,"Shader Name","",
	'CusI','in32',noFlags,"Custom Index","",
//	'Sh00','comp',noFlags,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",

	// 3D mesh
	'MeNa','Dstr',noFlags,"Tree element Name","",
	'MeID','in32',noFlags,"Custom mesh Index","",
	'MeSm','re32',interpolate,"Mesh Smoothing","",

	// Self Shadow
	'ShaT','in32',noFlags,"Self Shadow Type","",
	'ShaI','re32',interpolate,"Self Shadow Intensity","",
	'ShaD','re32',interpolate,"Fake Diffuse Lighting","",
	'ShaC','grad',interpolate,"Shadow Color Gradient","",
	'FLiX','re32',interpolate,"Fake Light Direction X","",
	'FLiY','re32',interpolate,"Fake Light Direction Y","",
	'FLiZ','re32',interpolate,"Fake Light Direction Z","",

	// Color
	'GasC','grad',interpolate,"Gas Color Gradient","",
	'ColR','in32',noFlags,"Color Ramp Off Flag","",
	'CoCo','re32',interpolate,"Color repartition","",
	'CPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'CPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'CNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'CNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'CPXC','vec2',interpolate,"Positive X Cylindrical","",
	'CPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'CPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'CNXC','vec2',interpolate,"Negative X Cylindrical","",
	'CNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'CNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'CXPo','vec2',interpolate,"X positive Ramp","",
	'CYPo','vec2',interpolate,"Y positive Ramp","",
	'CZPo','vec2',interpolate,"Z positive Ramp","",
	'CXNe','vec2',interpolate,"X Negative Ramp","",
	'CYNe','vec2',interpolate,"Y Negative Ramp","",
	'CZNe','vec2',interpolate,"Z Negative Ramp","",
	'ShNa','Dstr',noFlags,"Shader Name","",
	'PaID','in32',noFlags,"Custom shader Index","",

	// Intensity
	'Ramp','in32',noFlags,"Ramp Off Flag","",
	'RPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'RPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'RNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'RNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'RPXC','vec2',interpolate,"Positive X Cylindrical","",
	'RPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'RPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'RNXC','vec2',interpolate,"Negative X Cylindrical","",
	'RNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'RNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'RXPo','vec2',interpolate,"X positive Ramp","",
	'RYPo','vec2',interpolate,"Y positive Ramp","",
	'RZPo','vec2',interpolate,"Z positive Ramp","",
	'RXNe','vec2',interpolate,"X Negative Ramp","",
	'RYNe','vec2',interpolate,"Y Negative Ramp","",
	'RZNe','vec2',interpolate,"Z Negative Ramp","",

	// Modifiers
//	'Test','comp',interpolate,"Test","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
//	'Modi','cmp#',interpolate,"Deformers","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	'MoCo','scc#',interpolate,"Deformers","{fmly data Subm 1 MskI 32}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32

	// Custom Params
	'RAcc','re32',interpolate,"Raising Acceleration","",
	}
};

resource 'CPUI' (200) 
{
	200,					// Id of your main part
	0,//400,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

///////////////////////////////////////////////////////////////////
//
// Smog
//
resource 'GUID' (203)
{
	{
		R_IID_I3DExGeometricPrimitive, //	R_IID_I3DExVolumetricEffect,
		R_CLSID_Smog
	}
};

resource 'COMP' (203)
{
	'prim', // 'Volu',
	'Smog',
	"Primivol: Fog",
	"Inagoni Volume",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (203)
{
	{
	'Size','re32',noFlags,"Primitive Size","",

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

	// Gas
	'smoo','re32',interpolate,"Smooth","",
	'qual','re32',interpolate,"Quality","",
	'dens','re32',interpolate,"Density","",
	'SelI','re32',interpolate,"Self Intensity","",

	// Noise
	'seed','re32',noFlags,"Seed","",
	'frac','re32',interpolate,"Fractal Depth","",
	'gain','re32',interpolate,"Contrast","",
	'bias','re32',interpolate,"Brightness","",
	'ShaN','Dstr',noFlags,"Shader Name","",
	'CusI','in32',noFlags,"Custom Index","",
//	'Sh00','comp',noFlags,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",

	// 3D mesh
	'MeNa','Dstr',noFlags,"Tree element Name","",
	'MeID','in32',noFlags,"Custom mesh Index","",
	'MeSm','re32',interpolate,"Mesh Smoothing","",

	// Self Shadow
	'ShaT','in32',noFlags,"Self Shadow Type","",
	'ShaI','re32',interpolate,"Self Shadow Intensity","",
	'ShaD','re32',interpolate,"Fake Diffuse Lighting","",
	'ShaC','grad',interpolate,"Shadow Color Gradient","",
	'FLiX','re32',interpolate,"Fake Light Direction X","",
	'FLiY','re32',interpolate,"Fake Light Direction Y","",
	'FLiZ','re32',interpolate,"Fake Light Direction Z","",

	// Color
	'GasC','grad',interpolate,"Gas Color Gradient","",
	'ColR','in32',noFlags,"Color Ramp Off Flag","",
	'CoCo','re32',interpolate,"Color repartition","",
	'CPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'CPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'CNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'CNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'CPXC','vec2',interpolate,"Positive X Cylindrical","",
	'CPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'CPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'CNXC','vec2',interpolate,"Negative X Cylindrical","",
	'CNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'CNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'CXPo','vec2',interpolate,"X positive Ramp","",
	'CYPo','vec2',interpolate,"Y positive Ramp","",
	'CZPo','vec2',interpolate,"Z positive Ramp","",
	'CXNe','vec2',interpolate,"X Negative Ramp","",
	'CYNe','vec2',interpolate,"Y Negative Ramp","",
	'CZNe','vec2',interpolate,"Z Negative Ramp","",
	'ShNa','Dstr',noFlags,"Shader Name","",
	'PaID','in32',noFlags,"Custom shader Index","",

	// Intensity
	'Ramp','in32',noFlags,"Ramp Off Flag","",
	'RPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'RPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'RNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'RNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'RPXC','vec2',interpolate,"Positive X Cylindrical","",
	'RPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'RPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'RNXC','vec2',interpolate,"Negative X Cylindrical","",
	'RNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'RNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'RXPo','vec2',interpolate,"X positive Ramp","",
	'RYPo','vec2',interpolate,"Y positive Ramp","",
	'RZPo','vec2',interpolate,"Z positive Ramp","",
	'RXNe','vec2',interpolate,"X Negative Ramp","",
	'RYNe','vec2',interpolate,"Y Negative Ramp","",
	'RZNe','vec2',interpolate,"Z Negative Ramp","",

	// Modifiers
//	'Test','comp',interpolate,"Test","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
//	'Modi','cmp#',interpolate,"Deformers","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	'MoCo','scc#',interpolate,"Deformers","{fmly data Subm 1 MskI 32}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	}
};

resource 'CPUI' (203) 
{
	203,					// Id of your main part
	0,//403,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

///////////////////////////////////////////////////////////////////
//
// Rising Smoke
//
resource 'GUID' (202)
{
	{
		R_IID_I3DExGeometricPrimitive, //	R_IID_I3DExVolumetricEffect,
		R_CLSID_RisingSmoke
	}
};

resource 'COMP' (202)
{
	'prim', // 'Volu',
	'RiSm',
	"Primivol: Rising Smoke",
	"Inagoni Volume",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (202)
{
	{
	'Size','re32',noFlags,"Primitive Size","",
	
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

	// Gas
	'smoo','re32',interpolate,"Smooth","",
	'qual','re32',interpolate,"Quality","",
	'dens','re32',interpolate,"Density","",
	'SelI','re32',interpolate,"Self Intensity","",

	// Noise
	'seed','re32',noFlags,"Seed","",
	'frac','re32',interpolate,"Fractal Depth","",
	'gain','re32',interpolate,"Contrast","",
	'bias','re32',interpolate,"Brightness","",
	'ShaN','Dstr',noFlags,"Shader Name","",
	'CusI','in32',noFlags,"Custom Index","",
//	'Sh00','comp',noFlags,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",

	// 3D mesh
	'MeNa','Dstr',noFlags,"Tree element Name","",
	'MeID','in32',noFlags,"Custom mesh Index","",
	'MeSm','re32',interpolate,"Mesh Smoothing","",

	// Self Shadow
	'ShaT','in32',noFlags,"Self Shadow Type","",
	'ShaI','re32',interpolate,"Self Shadow Intensity","",
	'ShaD','re32',interpolate,"Fake Diffuse Lighting","",
	'ShaC','grad',interpolate,"Shadow Color Gradient","",
	'FLiX','re32',interpolate,"Fake Light Direction X","",
	'FLiY','re32',interpolate,"Fake Light Direction Y","",
	'FLiZ','re32',interpolate,"Fake Light Direction Z","",

	// Color
	'GasC','grad',interpolate,"Gas Color Gradient","",
	'ColR','in32',noFlags,"Color Ramp Off Flag","",
	'CoCo','re32',interpolate,"Color repartition","",
	'CPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'CPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'CNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'CNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'CPXC','vec2',interpolate,"Positive X Cylindrical","",
	'CPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'CPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'CNXC','vec2',interpolate,"Negative X Cylindrical","",
	'CNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'CNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'CXPo','vec2',interpolate,"X positive Ramp","",
	'CYPo','vec2',interpolate,"Y positive Ramp","",
	'CZPo','vec2',interpolate,"Z positive Ramp","",
	'CXNe','vec2',interpolate,"X Negative Ramp","",
	'CYNe','vec2',interpolate,"Y Negative Ramp","",
	'CZNe','vec2',interpolate,"Z Negative Ramp","",
	'ShNa','Dstr',noFlags,"Shader Name","",
	'PaID','in32',noFlags,"Custom shader Index","",

	// Intensity
	'Ramp','in32',noFlags,"Ramp Off Flag","",
	'RPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'RPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'RNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'RNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'RPXC','vec2',interpolate,"Positive X Cylindrical","",
	'RPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'RPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'RNXC','vec2',interpolate,"Negative X Cylindrical","",
	'RNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'RNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'RXPo','vec2',interpolate,"X positive Ramp","",
	'RYPo','vec2',interpolate,"Y positive Ramp","",
	'RZPo','vec2',interpolate,"Z positive Ramp","",
	'RXNe','vec2',interpolate,"X Negative Ramp","",
	'RYNe','vec2',interpolate,"Y Negative Ramp","",
	'RZNe','vec2',interpolate,"Z Negative Ramp","",

	// Modifiers
//	'Test','comp',interpolate,"Test","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
//	'Modi','cmp#',interpolate,"Deformers","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	'MoCo','scc#',interpolate,"Deformers","{fmly data Subm 1 MskI 32}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32

	// Custom
	'RRiD','re32',interpolate,"Rising Diameter","",
	'RRiR','re32',interpolate,"Rising Ramp","",
	}
};

resource 'CPUI' (202) 
{
	202,					// Id of your main part
	0,//402,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

///////////////////////////////////////////////////////////////////
//
// Clouds
//
resource 'GUID' (201)
{
	{
		R_IID_I3DExGeometricPrimitive, //	R_IID_I3DExVolumetricEffect,
		R_CLSID_Clouds
	}
};

resource 'COMP' (201)
{
	'prim', // 'Volu',
	'Clou',
	"Primivol: Clouds",
	"Inagoni Volume",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (201)
{
	{
	'Size','re32',noFlags,"Primitive Size","",

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

	// Gas
	'smoo','re32',interpolate,"Smooth","",
	'qual','re32',interpolate,"Quality","",
	'dens','re32',interpolate,"Density","",
	'SelI','re32',interpolate,"Self Intensity","",

	// Noise
	'seed','re32',noFlags,"Seed","",
	'frac','re32',interpolate,"Fractal Depth","",
	'gain','re32',interpolate,"Contrast","",
	'bias','re32',interpolate,"Brightness","",
	'ShaN','Dstr',noFlags,"Shader Name","",
	'CusI','in32',noFlags,"Custom Index","",
//	'Sh00','comp',noFlags,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",

	// 3D mesh
	'MeNa','Dstr',noFlags,"Tree element Name","",
	'MeID','in32',noFlags,"Custom mesh Index","",
	'MeSm','re32',interpolate,"Mesh Smoothing","",

	// Self Shadow
	'ShaT','in32',noFlags,"Self Shadow Type","",
	'ShaI','re32',interpolate,"Self Shadow Intensity","",
	'ShaD','re32',interpolate,"Fake Diffuse Lighting","",
	'ShaC','grad',interpolate,"Shadow Color Gradient","",
	'FLiX','re32',interpolate,"Fake Light Direction X","",
	'FLiY','re32',interpolate,"Fake Light Direction Y","",
	'FLiZ','re32',interpolate,"Fake Light Direction Z","",

	// Color
	'GasC','grad',interpolate,"Gas Color Gradient","",
	'ColR','in32',noFlags,"Color Ramp Off Flag","",
	'CoCo','re32',interpolate,"Color repartition","",
	'CPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'CPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'CNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'CNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'CPXC','vec2',interpolate,"Positive X Cylindrical","",
	'CPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'CPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'CNXC','vec2',interpolate,"Negative X Cylindrical","",
	'CNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'CNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'CXPo','vec2',interpolate,"X positive Ramp","",
	'CYPo','vec2',interpolate,"Y positive Ramp","",
	'CZPo','vec2',interpolate,"Z positive Ramp","",
	'CXNe','vec2',interpolate,"X Negative Ramp","",
	'CYNe','vec2',interpolate,"Y Negative Ramp","",
	'CZNe','vec2',interpolate,"Z Negative Ramp","",
	'ShNa','Dstr',noFlags,"Shader Name","",
	'PaID','in32',noFlags,"Custom shader Index","",

	// Intensity
	'Ramp','in32',noFlags,"Ramp Off Flag","",
	'RPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'RPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'RNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'RNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'RPXC','vec2',interpolate,"Positive X Cylindrical","",
	'RPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'RPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'RNXC','vec2',interpolate,"Negative X Cylindrical","",
	'RNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'RNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'RXPo','vec2',interpolate,"X positive Ramp","",
	'RYPo','vec2',interpolate,"Y positive Ramp","",
	'RZPo','vec2',interpolate,"Z positive Ramp","",
	'RXNe','vec2',interpolate,"X Negative Ramp","",
	'RYNe','vec2',interpolate,"Y Negative Ramp","",
	'RZNe','vec2',interpolate,"Z Negative Ramp","",

	// Modifiers
//	'Test','comp',interpolate,"Test","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
//	'Modi','cmp#',interpolate,"Deformers","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	'MoCo','scc#',interpolate,"Deformers","{fmly data Subm 1 MskI 32}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32

	// Custom
	'SphC','in32',noFlags,"Sphere count","",
	'SphS','in32',noFlags,"Sphere seed","",
	'MixT','in32',noFlags,"Mix type","",
	'Blen','re32',interpolate,"Blend dist","",
	'Coef','re32',interpolate,"Sphere Coeff","",
	'Core','re32',interpolate,"Core Density","",
	}
};

resource 'CPUI' (201) 
{
	201,					// Id of your main part
	0,//401,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

// Test primitive
/*
resource 'GUID' (310)
{
	{
		R_IID_I3DExGeometricPrimitive, //	R_IID_I3DExVolumetricEffect,
		R_CLSID_GasTest
	}
};

resource 'COMP' (310)
{
	'prim', // 'Volu',
	'GasT',
	"Gas Test",
	"Inagoni Volume",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (310)
{
	{
	'Size','re32',noFlags,"Primitive Size","",

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

	// Gas
	'smoo','re32',interpolate,"Smooth","",
	'qual','re32',interpolate,"Quality","",
	'dens','re32',interpolate,"Density","",
	'SelI','re32',interpolate,"Self Intensity","",

	// Noise
	'seed','re32',noFlags,"Seed","",
	'frac','re32',interpolate,"Fractal Depth","",
	'gain','re32',interpolate,"Contrast","",
	'bias','re32',interpolate,"Brightness","",
	'ShaN','Dstr',noFlags,"Shader Name","",
	'CusI','in32',noFlags,"Custom Index","",
//	'Sh00','comp',noFlags,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",

	// 3D mesh
	'MeNa','Dstr',noFlags,"Tree element Name","",
	'MeID','in32',noFlags,"Custom mesh Index","",
	'MeSm','re32',interpolate,"Mesh Smoothing","",

	// Self Shadow
	'ShaT','in32',noFlags,"Self Shadow Type","",
	'ShaI','re32',interpolate,"Self Shadow Intensity","",
	'ShaD','re32',interpolate,"Fake Diffuse Lighting","",
	'ShaC','grad',interpolate,"Shadow Color Gradient","",
	'FLiX','re32',interpolate,"Fake Light Direction X","",
	'FLiY','re32',interpolate,"Fake Light Direction Y","",
	'FLiZ','re32',interpolate,"Fake Light Direction Z","",

	// Color
	'GasC','grad',interpolate,"Gas Color Gradient","",
	'ColR','in32',noFlags,"Color Ramp Off Flag","",
	'CoCo','re32',interpolate,"Color repartition","",
	'CPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'CPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'CNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'CNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'CPXC','vec2',interpolate,"Positive X Cylindrical","",
	'CPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'CPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'CNXC','vec2',interpolate,"Negative X Cylindrical","",
	'CNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'CNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'CXPo','vec2',interpolate,"X positive Ramp","",
	'CYPo','vec2',interpolate,"Y positive Ramp","",
	'CZPo','vec2',interpolate,"Z positive Ramp","",
	'CXNe','vec2',interpolate,"X Negative Ramp","",
	'CYNe','vec2',interpolate,"Y Negative Ramp","",
	'CZNe','vec2',interpolate,"Z Negative Ramp","",
	'ShNa','Dstr',noFlags,"Shader Name","",
	'PaID','in32',noFlags,"Custom shader Index","",

	// Intensity
	'Ramp','in32',noFlags,"Ramp Off Flag","",
	'RPHU','vec2',interpolate,"Positive Half Sphere Up","",
	'RPHD','vec2',interpolate,"Positive Half Sphere Down","",
	'RNHU','vec2',interpolate,"Negative Half Sphere Up","",
	'RNHD','vec2',interpolate,"Negative Half Sphere Down","",
	'RPXC','vec2',interpolate,"Positive X Cylindrical","",
	'RPYC','vec2',interpolate,"Positive Y Cylindrical","",
	'RPZC','vec2',interpolate,"Positive Z Cylindrical","",
	'RNXC','vec2',interpolate,"Negative X Cylindrical","",
	'RNYC','vec2',interpolate,"Negative Y Cylindrical","",
	'RNZC','vec2',interpolate,"Negative Z Cylindrical","",
	'RXPo','vec2',interpolate,"X positive Ramp","",
	'RYPo','vec2',interpolate,"Y positive Ramp","",
	'RZPo','vec2',interpolate,"Z positive Ramp","",
	'RXNe','vec2',interpolate,"X Negative Ramp","",
	'RYNe','vec2',interpolate,"Y Negative Ramp","",
	'RZNe','vec2',interpolate,"Z Negative Ramp","",

	// Modifiers
//	'Test','comp',interpolate,"Test","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
//	'Modi','cmp#',interpolate,"Deformers","{fmly modi}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	'MoCo','scc#',interpolate,"Deformers","{fmly data Subm 1 MskI 32}", //  Subm 0 bnon 0 Sort 1 Mini 1 Tree 1 MskI 98 Drop 5 }", // 64 + 32
	}
};

resource 'CPUI' (310) 
{
	310,					// Id of your main part
	0,//311,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};
*/

resource 'STR#' (kStrings, purgeable)	
{
	{
	"GasData.dta", // File name
	"Compute Selfshadows", // 2
	"Plasma", // 3
	"Turbulence", // 4
	"Ridged Turbulence", // 5
	"Noise", // 6
	"GasData.dta", // 7
	"None", // 8
	"Edit Volume Primitive", // 9
	"The scene doesn't countain any distant light: the volume won't have any self shadow.", // 10
	"Primivol demo version is installed:A red and a green sphere will be displayed in the corners of the primitive. Also Primivol will only evaluate a limited number of values, after that it won't render the volumetric primitives until you restart Carrara.", // 11
	}
};


/////////////////////////////////////////////////////////////////////////////
//
// Part extension
//
/* Doesn't work
resource 'COMP' (290) 
{
	'part',
	'ARPa',
	"Assemble Room Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (290) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_AssembleRoomPart
	}
};
*/

resource 'COMP' (300) 
{
	'part',
	'GasP',
	"Gas Common Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (300) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_GasCommonPart
	}
};

// Shader chooser menu

resource 'COMP' (310) 
{
	'part',
	'SPop',
	"Shader Popup Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (310) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_ShaderPopupPart
	}
};

resource 'COMP' (320) 
{
	'part',
	'ShCh',
	"Shader Chooser Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (320) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_ShaderChooserPart
	}
};

// Mesh chooser menu

resource 'COMP' (330) 
{
	'part',
	'MPop',
	"Mesh Popup Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (330) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_MeshPopupPart
	}
};

resource 'COMP' (340) 
{
	'part',
	'MeCh',
	"Mesh Chooser Part",
	"",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (340) 
{
	{
	R_IID_IMFExPart,
	R_CLSID_MeshChooserPart
	}
};

/////////////////////////////////////////////////////////////////////////////
//
// Tabs
//

// Small part (assemble)
resource 'TABS' (300)
{	
	{	// part, name, icon
		315, "Description", noIcon;
		316, "Deformation", noIcon;
	}
};

// Big part (modeler)
resource 'TABS' (301)
{	
	{	// part, name, icon
		305, "General", noIcon;
		306, "Ramp Off", noIcon;
		307, "Color", noIcon;
	}
};


/////////////////////////////////////////////////////////////////////////////
//
// Data comp (personal modifiers)
//

// Taper

resource 'GUID' (501)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Taper
	}
};

resource 'COMP' (501)
{
	'data',
	'DTap',
	"Taper",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (501)
{
	{
		'stre','re32',interpolate,"","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"","",
		'flip','bool',noFlags,"","",
		'inWi','bool',noFlags,"","",
		'inLe','bool',noFlags,"","",
	}
};

resource 'data' (501)
{
	{
	}
};

// Wave

resource 'GUID' (502)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Wave
	}
};

resource 'COMP' (502)
{
	'data',
	'DWav',
	"Wave",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (502)
{
	{
		'stre','re32',interpolate,"Strength","",
		'wcou','re32',interpolate,"Wave Count","",
		'wpha','re32',interpolate,"Wave Phase","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"","",
		'type','in32',noFlags,"Type","",
	}
};

resource 'data' (502)
{
	{
	}
};

// Bulge

resource 'GUID' (503)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Bulge
	}
};

resource 'COMP' (503)
{
	'data',
	'DBul',
	"Bulge",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (503)
{
	{
		'stre','re32',interpolate,"","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"","",
		'inWi','bool',noFlags,"","",
		'inLe','bool',noFlags,"","",
	}
};

resource 'data' (503)
{
	{
	}
};

// Punch

resource 'GUID' (504)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Punch
	}
};

resource 'COMP' (504)
{
	'data',
	'DPun',
	"Punch",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (504)
{
	{
		'stre','re32',interpolate,"Strength","",
		'radi','re32',interpolate,"Radius","",
		'axis','in32',noFlags,"Axis","",
		'flip','bool',noFlags,"Apply on other side","",
	}
};

resource 'data' (504)
{
	{
	}
};

// Twist

resource 'GUID' (505)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Twist
	}
};

resource 'COMP' (505)
{
	'data',
	'DTwi',
	"Twist",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (505)
{
	{
		'angl','re32',interpolate,"Angle","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"Axis","",
	}
};

resource 'data' (505)
{
	{
	}
};

// Bend
/* TO DO
resource 'GUID' (506)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Bend
	}
};

resource 'COMP' (506)
{
	'data',
	'DBen',
	"Bend",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (506)
{
	{
		'angl','re32',interpolate,"Angle","",
		'orie','re32',interpolate,"Orientation","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"Axis","",
	}
};

resource 'data' (506)
{
	{
	}
};
*/
// Shift

resource 'GUID' (507)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Shift
	}
};

resource 'COMP' (507)
{
	'data',
	'DShi',
	"Shift",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (507)
{
	{
		'strU','re32',interpolate,"Strength U","",
		'strV','re32',interpolate,"Strength V","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"Axis","",
	}
};

resource 'data' (507)
{
	{
	}
};

// Noise

resource 'GUID' (508)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Noise
	}
};

resource 'COMP' (508)
{
	'data',
	'DNoi',
	"Noise",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (508)
{
	{
		'strX','re32',interpolate,"Strength X","",
		'strY','re32',interpolate,"Strength Y","",
		'strZ','re32',interpolate,"Strength Z","",
		'gScN','re32',interpolate,"Global Scaling","",
		'xScN','re32',interpolate,"X Scaling","",
		'yScN','re32',interpolate,"Y Scaling","",
		'zScN','re32',interpolate,"Z Scaling","",
		'xOfN','re32',interpolate,"X Offset","",
		'yOfN','re32',interpolate,"Y Offset","",
		'zOfN','re32',interpolate,"Z Offset","",
		'xRoN','re32',interpolate,"X Rotation","",
		'yRoN','re32',interpolate,"Y Rotation","",
		'zRoN','re32',interpolate,"Z Rotation","",
		'seed','in32',noFlags,"seed","",
		'ShNa','Dstr',noFlags,"Shader Name","",
		'PaID','in32',noFlags,"Custom shader Index","",
	}
};

resource 'data' (508)
{
	{
	}
};

// Stretch

resource 'GUID' (509)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Stretch
	}
};

resource 'COMP' (509)
{
	'data',
	'DSte',
	"Stretch",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (509)
{
	{
		'stre','re32',interpolate,"Strength","",
		'cent','re32',interpolate,"Center","",
		'limi','vec2',interpolate,"","",
		'axis','in32',noFlags,"","",
	}
};

resource 'data' (509)
{
	{
	}
};

// Scale

resource 'GUID' (510)
{
	{
		R_IID_I3DExDataComponent,
		R_CLSID_Scale
	}
};

resource 'COMP' (510)
{
	'data',
	'DSca',
	"Scale",
	"Modifiers # 32",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (510)
{
	{
		'scaX','re32',interpolate,"Scale X","",
		'scaY','re32',interpolate,"Scale Y","",
		'scaZ','re32',interpolate,"Scale Z","",
	}
};

resource 'data' (510)
{
	{
	}
};


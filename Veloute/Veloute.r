/****************************************************************************************************

		ExtraShaders.r
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/30/2003

****************************************************************************************************/

#include "ExternalAPI.r"
#include "External3DAPI.r" // For TABS
#include "Copyright.h"
#include "ExtraShadersDef.h"
#include "interfaceids.h"
#include "MCFInterfaceIDs.h"


#ifndef qUsingResourceLinker
#ifdef MAC
include "Veloute.rsr";
#endif
#endif

#ifdef WIN
include "Veloute.rsr";
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Tiling Shader 1
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (600)
{
	{
		R_IID_I3DExShader,
		R_CLSID_TilingShader1
	}
};

resource 'COMP' (600)
{
	kRID_ShaderFamilyID,
	'Til1',
	"Tiling: Complex",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (600)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Coun','in32',interpolate,"Tile count","",
		'Size','re32',interpolate,"Size","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (600) 
{
	600,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (600)
{	
	{
		601, "Tile Shape", noIcon,
		206, "Mortar Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Tiling shader 1

resource 'GUID' (700)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalTilingShader1
	}
};

resource 'COMP' (700)
{
	kRID_ShaderFamilyID,
	'GTi1',
	"Multi Channels Tiling: Complex",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (700)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Coun','in32',interpolate,"Tile count","",
		'Size','re32',interpolate,"Size","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (700) 
{
	600,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Tiling Shader 2
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (605)
{
	{
		R_IID_I3DExShader,
		R_CLSID_TilingShader2
	}
};

resource 'COMP' (605)
{
	kRID_ShaderFamilyID,
	'Til2',
	"Tiling: Basic",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (605)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Type","",
		'Hori','in32',interpolate,"Horizontal","",
		'Vert','in32',interpolate,"Vertical","",
		'Seco','re32',interpolate,"Second tile length","",
		'Shif','re32',interpolate,"Row Shifting","",
		'Slop','re32',interpolate,"Slope","",
		'GapS','re32',noFlags,"Gap Depth","",
		'GapI','re32',noFlags,"Gap Inclination","",
		'Thic','re32',interpolate,"Thickness","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'LMor','re32',interpolate,"Mortar Length","",
		'HMor','re32',interpolate,"Mortar Height","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (605) 
{
	605,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (605)
{	
	{
		606, "Tile Shape", noIcon,
		203, "Mortar Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of Tiling Shader 2

resource 'GUID' (705)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalTilingShader2
	}
};

resource 'COMP' (705)
{
	kRID_ShaderFamilyID,
	'GTi2',
	"Multi Channels Tiling: Basic",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (705)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Type","",
		'Hori','in32',interpolate,"Horizontal","",
		'Vert','in32',interpolate,"Vertical","",
		'Seco','re32',interpolate,"Second tile length","",
		'Shif','re32',interpolate,"Row Shifting","",
		'Slop','re32',interpolate,"Slope","",
		'GapS','re32',noFlags,"Gap Depth","",
		'GapI','re32',noFlags,"Gap Inclination","",
		'Thic','re32',interpolate,"Thickness","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'LMor','re32',interpolate,"Mortar Length","",
		'HMor','re32',interpolate,"Mortar Height","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (705) 
{
	605,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Tiling Shader 3
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (610)
{
	{
		R_IID_I3DExShader,
		R_CLSID_TilingShader3
	}
};

resource 'COMP' (610)
{
	kRID_ShaderFamilyID,
	'Til3',
	"Tiling: Undulate",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (610)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Coun','in32',interpolate,"Tile count","",
		'Peri','in32',noFlags,"Undulation Period","",
		'Ampl','re32',noFlags,"Undulation Amplitude","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (610) 
{
	610,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (610)
{	
	{
		611, "Tile Shape", noIcon,
		206, "Mortar Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Hexa Tile shader

resource 'GUID' (710)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalTilingShader3
	}
};

resource 'COMP' (710)
{
	kRID_ShaderFamilyID,
	'GTi3',
	"Multi Channels Tiling: Undulate",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (710)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Coun','in32',interpolate,"Tile count","",
		'Peri','in32',noFlags,"Undulation Period","",
		'Ampl','re32',noFlags,"Undulation Amplitude","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (710) 
{
	610,					// Id of your main part
	500,					// Id of your mini-part
	1,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Tiling Shader 4
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (615)
{
	{
		R_IID_I3DExShader,
		R_CLSID_TilingShader4
	}
};

resource 'COMP' (615)
{
	kRID_ShaderFamilyID,
	'Til4',
	"Tiling: Random",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (615)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Seed','in32',interpolate,"Seed","",
		'Coun','in32',interpolate,"Tile count","",
		'Stre','re32',interpolate,"Stretch","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (615) 
{
	615,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (615)
{	
	{
		616, "Tile Shape", noIcon,
		213, "Mortar Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Tiling shader 4

resource 'GUID' (715)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalTilingShader4
	}
};

resource 'COMP' (715)
{
	kRID_ShaderFamilyID,
	'GTi4',
	"Multi Channels Tiling: Random",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (715)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Seed','in32',interpolate,"Seed","",
		'Coun','in32',interpolate,"Tile count","",
		'Stre','re32',interpolate,"Stretch","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (715) 
{
	615,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Roof Shader 1
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (630)
{
	{
		R_IID_I3DExShader,
		R_CLSID_RoofShader1
	}
};

resource 'COMP' (630)
{
	kRID_ShaderFamilyID,
	'Roo1',
	"Roof",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (630)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Tile type","",
		'HCou','in32',interpolate,"Horizontal Tile count","",
		'VCou','in32',interpolate,"Vertical Tile count","",
		'GapS','re32',interpolate,"Gap Size","",
		'Ampl','re32',interpolate,"Amplitude","",
		'Peri','in32',interpolate,"Period","",
		'Shif','in32',interpolate,"Shifting","",
		'Inte','re32',interpolate,"Self Shadow Intensity","",
		'Prop','re32',interpolate,"Proportion","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'HSha','re32',interpolate,"Horizontal Shadow Size","",
		'VSha','re32',interpolate,"Vertical Shadow Size","",
		'DSha','re32',interpolate,"Shadow Depth","",
		'BotS','in32',noFlags,"Shadow Bottom Slope","",
		'TopS','in32',noFlags,"Shadow Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Section","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (630) 
{
	630,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (630)
{	
	{
		631, "Tile Shape", noIcon,
		212, "Shadow Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Roof shader 1

resource 'GUID' (730)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalRoofShader1
	}
};

resource 'COMP' (730)
{
	kRID_ShaderFamilyID,
	'GRo1',
	"Multi Channels Roof",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (730)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Tile type","",
		'HCou','in32',interpolate,"Horizontal Tile count","",
		'VCou','in32',interpolate,"Vertical Tile count","",
		'GapS','re32',interpolate,"Gap Size","",
		'Ampl','re32',interpolate,"Amplitude","",
		'Peri','in32',interpolate,"Period","",
		'Shif','in32',interpolate,"Shifting","",
		'Inte','re32',interpolate,"Self Shadow Intensity","",
		'Prop','re32',interpolate,"Proportion","",
		'TiSh','bool',noFlags,"Proportionnal Tile Shading","",
		'RaUV','bool',noFlags,"Randomize UV on Tile Shading","",
		'HSha','re32',interpolate,"Horizontal Shadow Size","",
		'VSha','re32',interpolate,"Vertical Shadow Size","",
		'DSha','re32',interpolate,"Shadow Depth","",
		'BotS','in32',noFlags,"Shadow Bottom Slope","",
		'TopS','in32',noFlags,"Shadow Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Section","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (730) 
{
	630,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};


/////////////////////////////////////////////////////////////////////////////////////////
//
//	Bump Noise 2D
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (400)
{
	{
		R_IID_I3DExShader,
		R_CLSID_BumpNoise2DShader
	}
};

resource 'COMP' (400)
{
	kRID_ShaderFamilyID,
	'2DBN',
	"2D Random Map",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (400)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed","",
		'Type','in32',noFlags,"Type","",
		'Pa01','re32',interpolate,"Param 1","",
		'Pa02','re32',interpolate,"Param 2","",
		'AAOn','bool',interpolate,"Anti Aliasing On","",
	}
};


resource 'CPUI' (400) 
{
	400,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (400)
{	
	{
		401, "Noise Settings", noIcon,
		231, "2D Transform", noIcon,
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Bump Noise 3D
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (450)
{
	{
		R_IID_I3DExShader,
		R_CLSID_BumpNoise3DShader
	}
};

resource 'COMP' (450)
{
	kRID_ShaderFamilyID,
	'3DBN',
	"3D Random Map",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (450)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 3DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclX','re32',interpolate,"Scale X","",
		'SclY','re32',interpolate,"Scale Y","",
		'SclZ','re32',interpolate,"Scale Z","",
		'OffX','re32',interpolate,"Offset X","",
		'OffY','re32',interpolate,"Offset Y","",
		'OffZ','re32',interpolate,"Offset Z","",
		'RotX','re32',interpolate,"Rotation X","",
		'RotY','re32',interpolate,"Rotation Y","",
		'RotZ','re32',interpolate,"Rotation Z","",
		'Spac','in32',noFlags,"Space","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed","",
		'Type','in32',noFlags,"Type","",
		'Pa01','re32',interpolate,"Param 1","",
		'Pa02','re32',interpolate,"Param 2","",
		'AAOn','bool',interpolate,"Anti Aliasing On","",
	}
};


resource 'CPUI' (450) 
{
	450,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (450)
{	
	{
		451, "Noise Settings", noIcon,
		232, "3D Transform", noIcon,
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Filter Shader
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (405)
{
	{
		R_IID_I3DExShader,
		R_CLSID_FilterShader
	}
};

resource 'COMP' (405)
{
	kRID_ShaderFamilyID,
	'Filt',
	"Filter",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (405)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// Shader pmap
		'Sh00','comp',interpolate,"SubShader","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};


resource 'CPUI' (405) 
{
	405,					// Id of your main part
	502,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	2D Gradient Noise
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (420)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Gradient2DShader
	}
};

resource 'COMP' (420)
{
	kRID_ShaderFamilyID,
	'2DGR',
	"2D Gradient",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (420)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Repe','re32',interpolate,"Repeat","",
		'Smoo','bool',interpolate,"Smooth interpolation","",
		'AAOn','bool',interpolate,"Anti Aliasing On","",
	}
};


resource 'CPUI' (420) 
{
	420,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (420)
{	
	{
		421, "Gradient Settings", noIcon,
		231, "2D Transform", noIcon,
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	3D Gradient Noise
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (460)
{
	{
		R_IID_I3DExShader,
		R_CLSID_Gradient3DShader
	}
};

resource 'COMP' (460)
{
	kRID_ShaderFamilyID,
	'3DGR',
	"3D Gradient",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (460)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 3DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclX','re32',interpolate,"Scale X","",
		'SclY','re32',interpolate,"Scale Y","",
		'SclZ','re32',interpolate,"Scale Z","",
		'OffX','re32',interpolate,"Offset X","",
		'OffY','re32',interpolate,"Offset Y","",
		'OffZ','re32',interpolate,"Offset Z","",
		'RotX','re32',interpolate,"Rotation X","",
		'RotY','re32',interpolate,"Rotation Y","",
		'RotZ','re32',interpolate,"Rotation Z","",
		'Spac','in32',noFlags,"Space","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Repe','re32',interpolate,"Repeat","",
		'Smoo','bool',interpolate,"Smooth interpolation","",
		'AAOn','bool',interpolate,"Anti Aliasing On","",
	}
};


resource 'CPUI' (460) 
{
	460,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (460)
{	
	{
		461, "Gradient Settings", noIcon,
		232, "3D Transform", noIcon,
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Perturbation Shader
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (440)
{
	{
		R_IID_I3DExShader,
		R_CLSID_PerturbationShader
	}
};

resource 'COMP' (440)
{
	kRID_ShaderFamilyID,
	'Pert',
	"Perturbation",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (440)
{
	{
		// Shader pmap
		'Ampl','re32',interpolate,"Amlitude","",
		// SHADERS_PLUS params
		'AmpX','re32',interpolate,"Amlitude X","",
		'AmpY','re32',interpolate,"Amlitude Y","",
		'AmpZ','re32',interpolate,"Amlitude Z","",
		'Meth','in32',noFlags,"Method","",
		// end of SHADERS_PLUS params
		'Sh00','comp',interpolate,"Texture","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Perturbation","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};


resource 'CPUI' (440) 
{
	440,					// Id of your main part
	441,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

///////////////////////////////////////////////////////////////////////
// Veloute Tile strings
resource 'STR#' (200, purgeable)	
{
	{
		"Tiles numbers between U=0 to U=1",	// 001
		"Tiles numbers between V=0 to V=1",	// 002
		"Size of the 2nd tile in % of the first",	// 003
		"Shift between 2 adjacent rows of tiles",	// 004
		"Check to keep the proportions of the shading on a tile. Uncheck to stretch the shader on the whole tile.",	// 005
		"Mortar width in U in % of the first tile length",	// 006
		"Mortar width in V in % of the tile height",	// 007
		"Flat area size in % of the mortar width",	// 008
		"Influence only the bump shape",	// 009
		"Slope at the top of the mortar",	// 010
		"Slope at the bottom of the mortar",	// 011
		"Modify the shape of the tile corners",	// 012
		"No undulation", // 013
		"Light undulation", // 014
		"Strong undulation", // 015
		"Alternates", // 016
		"Squares", // 017
		"Squares in corners", // 018
		"Square in center", // 019
		"Spiral", // 020
		"Hexagon", // 021
		"Random", // 022
		"Surrounding tiles size in % of the middle one", // 023
		"Mortar width in % of the first tile length",	// 024
		"Two tiles", // 025
		"Oblique 1", // 026
		"Oblique 2", // 027
		"Autoblock", // 028
		"Undulated 1", // 029
		"Undulated 2", // 030
		"Undulated 3", // 031
		"Check to set a different UV origin on each tile. Uncheck to have the same UV origin on every tiles.",	// 032
		"Straight", // 033
		"Smooth", // 034
		"Supple", // 035
		"Geometric 1", // 036
		"Geometric 2", // 037
		"Slates 1", // 038
		"Slates 2", // 039
		"Tiles 1", // 040
		"Tiles 2", // 041
		"Tiles 3", // 042
		"Amplitude of the oscillation",	// 043
		"Number of oscillations in 1 tile", // 044
		"Shifting beetween 2 rows of Tiles. WARNING: this is effective only for even number of oscillations.", // 045
		"Shadow produced by the oscillation", // 046
		"Relation beetween the 2 parts of the tile", // 047
		"Stretch the tile in the U or the V direction", // 48
		"This is a demo version: Veloute will render only a limited number of times.", // 49
	}
};

///////////////////////////////////////////////////////////////////////
// Veloute Tools Shaders strings
resource 'STR#' (201, purgeable)	
{
	{
		"Contrast",	// 001
		"Brightness",	// 002
		"Invert",	// 003
		"Reset",	// 004
		"Determine the average value",	// 005
		"Fractal Depth",	// 006
		"Roughness (for non zero fractal depth)",	// 007
		"Shuffle",	// 008
		"U zero-one",	// 009
		"U zero-one-zero",	// 010
		"V zero-one",	// 011
		"V zero-one-zero",	// 012
		"Circular zero-one",	// 013
		"Circular zero-one-zero",	// 014
		"Check to get a smooth bump",	// 015
		"Repeat",	// 016
		"Perturbation Amplitude",	// 017
		"Local Space",	// 018
		"Global Space",	// 019
		"Evaluate the min and max values for the maximum filling of the visible domain.",	// 020
		"X zero-one",	// 021
		"X zero-one-zero",	// 022
		"Y zero-one",	// 023
		"Y zero-one-zero",	// 024
		"Z zero-one",	// 025
		"Z zero-one-zero",	// 026
		"Check to turn texture Anti Aliasing On",	// 027
	}
};

///////////////////////////////////////////////////////////////////////
// Noise names strings
resource 'STR#' (202, purgeable)	
{
	{
		"Perlin Noise",	// 001
		"Plasma Noise",	// 002
		"Turbulence Noise",	// 003
		"Linear Noise",	// 004
		"Stiff Corpuscules",	// 005
		"Supple Corpuscules",	// 006
		"Split Stiff Corpuscules",	// 007
		"Split Supple Corpuscules",	// 008
		"Thick Edges",	// 009
		"Thin Edges",	// 010
		"Straight Cells",	// 011
		"Smooth Cells",	// 012
		"Sharp Cells 1",	// 013
		"Sharp Cells 2",	// 014
		"Rough Squares",	// 015
		"Straight Facets",	// 016
		"Smooth Facets",	// 017
		"Dense Facets",	// 018
		"Squared Facets",	// 019
		"Diamond Facets",	// 020
		"Black Wires",	// 021
		"White Wires",	// 022
		"Circuits",	// 023
		"Spikes",	// 024
		"Diamonds",	// 025
		"Stars",	// 026
		"Shreds",	// 027
		"Straight Tiles",	// 028
		"Smooth Tiles",	// 029
		"Supple Tiles",	// 030
		"Geometric Tiles 1",	// 031
		"Geometric Tiles 2",	// 032
		"Circlar Vibe",	// 033
		"Rounded Vibe",	// 034
		"Squared Vibe",	// 035
		"Rectangular Vibe",	// 036
		"Chopped Vibe",	// 037
		"Sin Wave",	// 038
		"Squares",	// 039
		"Smoothed Squares",	// 040
	}
};

resource 'STR#' (203, purgeable)	
{
	{
	"ExtraData.dta", // File name
	}
};

///////////////////////////////////////////////////////////////////////
//
// Part Extensions
//
resource 'COMP' (1000)
{
	'part',				  			// External Part Family
	'PCh1',							// Class ID
	"",								// Family
	"TParamChooserPart1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (1000)
{
	{
	R_IID_IMFExPart,
	R_CLSID_TParamChooserPart1
	}
};

resource 'COMP' (1001)
{
	'part',				  			// External Part Family
	'PCh2',							// Class ID
	"",								// Family
	"TParamChooserPart2",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (1001)
{
	{
	R_IID_IMFExPart,
	R_CLSID_TParamChooserPart2
	}
};

resource 'COMP' (1002)
{
	'part',				  			// External Part Family
	'PCh3',							// Class ID
	"",								// Family
	"TParamChooserPart3",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (1002)
{
	{
	R_IID_IMFExPart,
	R_CLSID_TParamChooserPart3
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Grid Shader 1: use Tiling Shader 1 engine
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (1600)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GridShader1
	}
};

resource 'COMP' (1600)
{
	kRID_ShaderFamilyID,
	'Gri1',
	"Grid: Complex",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1600)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Coun','in32',interpolate,"Tile count","",
		'Size','re32',interpolate,"Size","",
		'SMor','re32',interpolate,"Section Size","",
		'PMor','re32',interpolate,"Section Plateau","",
		'DMor','re32',interpolate,"Bump Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Connections","",
		// Shading
		'Sh00','comp',interpolate,"Grid","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Background","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
// Not used (maybe later)		'Sh02','comp',interpolate,"Grid color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1600) 
{
	1600,					// Id of your main part
	1500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (1600)
{	
	{
		1601, "Grid Shape", noIcon,
		1206, "Section Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Grid shader 1

resource 'GUID' (1700)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalGridShader1
	}
};

resource 'COMP' (1700)
{
	kRID_ShaderFamilyID,
	'GGr1',
	"Multi Channels Grid: Complex",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1700)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Coun','in32',interpolate,"Tile count","",
		'Size','re32',interpolate,"Size","",
		'SMor','re32',interpolate,"Section Size","",
		'PMor','re32',interpolate,"Section Plateau","",
		'DMor','re32',interpolate,"Bump Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Connections","",
		// Shading
		'Sh00','comp',interpolate,"Grid","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Background","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
// Not used (maybe later)		'Sh02','comp',interpolate,"Grid color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1700) 
{
	1700,					// Id of your main part
	1500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Grid Shader 2, from Tiling Shader 2
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (1605)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GridShader2
	}
};

resource 'COMP' (1605)
{
	kRID_ShaderFamilyID,
	'Gri2',
	"Grid: Basic",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1605)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Type","",
		'Hori','in32',interpolate,"Horizontal","",
		'Vert','in32',interpolate,"Vertical","",
		'Seco','re32',interpolate,"Second hole length","",
		'Shif','re32',interpolate,"Row Shifting","",
		'Slop','re32',interpolate,"Slope","",
		'GapS','re32',noFlags,"Gap Depth","",
		'GapI','re32',noFlags,"Gap Inclination","",
		'Thic','re32',interpolate,"Thickness","",
		'LMor','re32',interpolate,"Section Length","",
		'HMor','re32',interpolate,"Section Height","",
		'PMor','re32',interpolate,"Section Plateau","",
		'DMor','re32',interpolate,"Bump Depth","",
		'BotS','in32',noFlags,"Section Middle Slope","",
		'TopS','in32',noFlags,"Section Sides Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Grid","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Background","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
// not used, maybe later		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1605) 
{
	1605,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (1605)
{	
	{
		1606, "Grid Shape", noIcon,
		1203, "Section Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of Grid Shader 2

resource 'GUID' (1705)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalGridShader2
	}
};

resource 'COMP' (1705)
{
	kRID_ShaderFamilyID,
	'GGr2',
	"Multi Channels Grid: Basic",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1705)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Seed','in32',noFlags,"Seed for SHADERS_PLUS","",
		'Type','in32',noFlags,"Type","",
		'Hori','in32',interpolate,"Horizontal","",
		'Vert','in32',interpolate,"Vertical","",
		'Seco','re32',interpolate,"Second hole length","",
		'Shif','re32',interpolate,"Row Shifting","",
		'Slop','re32',interpolate,"Slope","",
		'GapS','re32',noFlags,"Gap Depth","",
		'GapI','re32',noFlags,"Gap Inclination","",
		'Thic','re32',interpolate,"Thickness","",
		'LMor','re32',interpolate,"Section Length","",
		'HMor','re32',interpolate,"Section Height","",
		'PMor','re32',interpolate,"Section Plateau","",
		'DMor','re32',interpolate,"Bump Depth","",
		'BotS','in32',noFlags,"Section Middle Slope","",
		'TopS','in32',noFlags,"Section Sides Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
// Not used, maybe later		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1705) 
{
	1605,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Grid Shader 3 from Tiling Shader 3
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (1610)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GridShader3
	}
};

resource 'COMP' (1610)
{
	kRID_ShaderFamilyID,
	'Gri3',
	"Grid: Undulate",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1610)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Coun','in32',interpolate,"Tile count","",
		'Peri','in32',noFlags,"Undulation Period","",
		'Ampl','re32',noFlags,"Undulation Amplitude","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1610) 
{
	1610,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (1610)
{	
	{
		1611, "Grid Shape", noIcon,
		206, "Section Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Grid 3 shader

resource 'GUID' (1710)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalGridShader3
	}
};

resource 'COMP' (1710)
{
	kRID_ShaderFamilyID,
	'GGr3',
	"Multi Channels Grid: Undulate",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1710)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Coun','in32',interpolate,"Tile count","",
		'Peri','in32',noFlags,"Undulation Period","",
		'Ampl','re32',noFlags,"Undulation Amplitude","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		'Smoo','bool',noFlags,"Smooth Corners","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1710) 
{
	1710,					// Id of your main part
	500,					// Id of your mini-part
	1,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Grid Shader 4 from Tiling Shader 4
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (1615)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GridShader4
	}
};

resource 'COMP' (1615)
{
	kRID_ShaderFamilyID,
	'Gri4',
	"Grid: Random",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1615)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Seed','in32',interpolate,"Seed","",
		'Coun','in32',interpolate,"Tile count","",
		'Stre','re32',interpolate,"Stretch","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1615) 
{
	1615,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (1615)
{	
	{
		1616, "Grid Shape", noIcon,
		1213, "Section Shape", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Grid shader 4

resource 'GUID' (1715)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalGridShader4
	}
};

resource 'COMP' (1715)
{
	kRID_ShaderFamilyID,
	'GGr4',
	"Multi Channels Grid: Random",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (1715)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Tile type","",
		'Seed','in32',interpolate,"Seed","",
		'Coun','in32',interpolate,"Tile count","",
		'Stre','re32',interpolate,"Stretch","",
		'SMor','re32',interpolate,"Mortar Size","",
		'PMor','re32',interpolate,"Mortar Plateau","",
		'DMor','re32',interpolate,"Mortar Depth","",
		'BotS','in32',noFlags,"Mortar Bottom Slope","",
		'TopS','in32',noFlags,"Mortar Top Slope","",
		// Shading
		'Sh00','comp',interpolate,"Tile","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Mortar","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Tile color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (1715) 
{
	1615,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	Weave Shader 1
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (800)
{
	{
		R_IID_I3DExShader,
		R_CLSID_WeaveShader1
	}
};

resource 'COMP' (800)
{
	kRID_ShaderFamilyID,
	'Wea1',
	"Weave: Basic",
	"Veloute Tile",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (800)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Weave type","",
		'Shap','in32',noFlags,"Shape","",
		'UCou','in32',interpolate,"Weave count U","",
		'VCou','in32',interpolate,"Weave count V","",
		'USpa','re32',interpolate,"Spacing U","",
		'VSpa','re32',interpolate,"Spacing V","",
		'Flat','re32',interpolate,"Flat Area","",
		'Bump','re32',interpolate,"Flat Area","",
		// Shading
		'Sh00','comp',interpolate,"Weave","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Background","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Weave color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (800) 
{
	800,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (800)
{	
	{
		801, "Weave Type", noIcon,
		231, "2D Transform", noIcon,
	}
};

// Global version of the Weave shader 1

resource 'GUID' (900)
{
	{
		R_IID_I3DExShader,
		R_CLSID_GlobalWeaveShader1
	}
};

resource 'COMP' (900)
{
	kRID_ShaderFamilyID,
	'GWe1',
	"Multi Channels Weave: Basic",
	"Veloute Tile Multi Channel # 1",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (900)
{
	{
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Weave type","",
		'Shap','in32',noFlags,"Shape","",
		'UCou','in32',interpolate,"Weave count U","",
		'VCou','in32',interpolate,"Weave count V","",
		'USpa','re32',interpolate,"Spacing U","",
		'VSpa','re32',interpolate,"Spacing V","",
		'Flat','re32',interpolate,"Flat Area","",
		'Bump','re32',interpolate,"Flat Area","",
		// Shading
		'Sh00','comp',interpolate,"Weave","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh01','comp',interpolate,"Background","{fmly shdr MskE 0 Subm 1 Sort 1 Mini 1 Drop 3}",
		'Sh02','comp',interpolate,"Weave color tint","{fmly shdr MskE 1047 Subm 1 Sort 1 Mini 1 Drop 3}",
	}
};

resource 'CPUI' (900) 
{
	900,					// Id of your main part
	500,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	2D Random Lines
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (425)
{
	{
		R_IID_I3DExShader,
		R_CLSID_RandomLines2DShader
	}
};

resource 'COMP' (425)
{
	kRID_ShaderFamilyID,
	'2DRL',
	"2D Random Lines",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (425)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 2DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclU','re32',interpolate,"Scale U","",
		'SclV','re32',interpolate,"Scale V","",
		'OffU','re32',interpolate,"Offset U","",
		'OffV','re32',interpolate,"Offset V","",
		'Rota','re32',interpolate,"Rotation","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Seed','in32',noFlags,"Type","",
		'Dist','re32',interpolate,"Disturbance","",
		'Coun','re32',interpolate,"Disturbance","",
		'AAOn','bool',noFlags,"Anti Aliasing On","",
	}
};


resource 'CPUI' (425) 
{
	425,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (425)
{	
	{
		426, "Lines Settings", noIcon,
		231, "2D Transform", noIcon,
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
//
//	3D Random Lines
//
/////////////////////////////////////////////////////////////////////////////////////////

resource 'GUID' (465)
{
	{
		R_IID_I3DExShader,
		R_CLSID_RandomLines3DShader
	}
};

resource 'COMP' (465)
{
	kRID_ShaderFamilyID,
	'3DRL',
	"3D Random Lines",
	"Veloute Tools",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'PMap' (465)
{
	{
		// PMap from GrayScaleBase
		'Gain','re32',interpolate,"Gain","",
		'Bias','re32',interpolate,"Bias","",
		'MiMa','vec2',interpolate,"Min Max","",
		'Inve','bool',interpolate,"Invert","",
		//'DUMM','re32',noFlags,"Dummy for x64 support","",
		// PMap from 3DTransform
		'Scle','re32',interpolate,"Global Scale","",
		'SclX','re32',interpolate,"Scale X","",
		'SclY','re32',interpolate,"Scale Y","",
		'SclZ','re32',interpolate,"Scale Z","",
		'OffX','re32',interpolate,"Offset X","",
		'OffY','re32',interpolate,"Offset Y","",
		'OffZ','re32',interpolate,"Offset Z","",
		'RotX','re32',interpolate,"Rotation X","",
		'RotY','re32',interpolate,"Rotation Y","",
		'RotZ','re32',interpolate,"Rotation Z","",
		'Spac','in32',noFlags,"Space","",
		// Shader pmap
		'Type','in32',noFlags,"Type","",
		'Seed','in32',noFlags,"Type","",
		'Dist','re32',interpolate,"Disturbance","",
		'Coun','re32',interpolate,"Disturbance","",
		'AAOn','bool',noFlags,"Anti Aliasing On","",
	}
};


resource 'CPUI' (465) 
{
	465,					// Id of your main part
	503,					// Id of your mini-part
	0,						// Style
	kParamsBeforeChildren,	// Where Param part is shown
	1						// Is Collapsable ?
};

resource 'TABS' (465)
{	
	{
		466, "Lines Settings", noIcon,
		232, "3D Transform", noIcon,
	}
};

///////////////////////////////////////////////////////////////////////
//
// Part Extensions
//

// For Weave 1 Shader
resource 'COMP' (1003)
{
	'part',				  			// External Part Family
	'PCh4',							// Class ID
	"",								// Family
	"TParamChooserPart4",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};

resource 'GUID' (1003)
{
	{
	R_IID_IMFExPart,
	R_CLSID_TParamChooserPart4
	}
};


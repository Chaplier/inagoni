/****************************************************************************************************

		OSGPlug.r

		Author:	Julien Chaplier
		Date:	16/08/2008

****************************************************************************************************/


#include "External3DAPI.r"
#include "Copyright.h"
#include "OSGPlugDef.h"
#include "interfaceids.h"
#include "External3DAPI.r"
#include "imageAPI.r"

// Component description for the exporter
// Note: Carrara doesn't know how to handle multiple extensions for 1 exporter, so we need to 
// create here 2 exporters: 1 for the .osg extention, the other for the .ive file.

// .osg exporter description

resource 'GUID' (195)
{
	{
		R_IID_I3DExExportFilter,
		GUID_OSGExporter
	}
};

resource 'COMP' (195)
{
	kRID_ExportFilterFamilyID,
	'OsgC',
	"OSG Export",
	"Ascii file format",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Dou' (195) {
	'3Dou',
	'OsgC',
	isAlienType,
	'ttxt',				// creator
	{
		'OSG ', 		// Reference
		"OpenSceneGraph Ascii",	// Name
		{'TEXT' },		// List of MacOS types
		{"osg" },		// List of extensions
	};
};

// .ive exporter description

resource 'GUID' (196)
{
	{
		R_IID_I3DExExportFilter,
		GUID_IVEExporter
	}
};

resource 'COMP' (196)
{
	kRID_ExportFilterFamilyID,
	'IveC',
	"OSG Export",
	"Binary file format",
	FIRSTVERSION,
	VERSIONSTRING,
	COPYRIGHT,
	kRDAPIVersion
};
  
resource '3Dou' (196) {
	'3Dou',
	'IveC',
	isAlienType,
	'ttxt',				// creator
	{
		'IVE ', 		// Reference
		"OpenSceneGraph Binary",	// Name
		{'TEXT' },		// List of MacOS types
		{ "ive" },		// List of extensions
	};
};

// Strings used in the plugin that can be translated

resource 'STR#' (131)
{
	{
		"Exporting OpenSceneGraph",		// string  1
	}
};


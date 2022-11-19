/****************************************************************************************************

		OSGPlug.cpp

		Author:	Julien Chaplier
		Date:	16/08/2008

****************************************************************************************************/

#include "OSGExporter.h"

#include "MiscComUtilsImpl.h"
#include "IShUtilities.h"

void Extension3DInit(IMCUnknown* utilities)
{
	// Initialize the plugin path
	// If OSG is installed on the computer, the plugins are found using 
	// the OSG_ROOT variable. But if not, we can use OSG_LIBRARY_PATH variable.
	// We make it point on the Carrara Extensions folder
	TMCCountedPtr<IMCFile> extentionFolder;
	gFileUtilities->GetExtensionsFolder	(&extentionFolder);
	if( extentionFolder )
	{
		TMCDynamicString folderStr;
		extentionFolder->GetFileFullPathName( folderStr );
		std::string envName ( "OSG_LIBRARY_PATH=" );
		envName += std::string( folderStr.StrGet() );
		if( getenv( "OSG_LIBRARY_PATH" ) )
		{
			envName+=";";
			envName+=getenv( "OSG_LIBRARY_PATH" );
		}
		putenv( envName.c_str() );
	}
}

void Extension3DCleanup()
{
	// Perform any nec clean-up here
}


TBasicUnknown* MakeCOMObject(const MCCLSID& classId)	// This method instanciate
{														// the object COM
	TBasicUnknown* res = NULL;
	
	if (classId == MCGUID_OSGExporter) res = new OSGExporter(false);
	else if (classId == MCGUID_IVEExporter) res = new OSGExporter(true);
	
	return res;
}

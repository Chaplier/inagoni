/****************************************************************************************************

		OSGExporter.h

		Author:	Julien Chaplier
		Date:	16/08/2008

****************************************************************************************************/

#pragma once

#include "OSGPlugDef.h"
#include "OSGLightManager.h"
#include "MCCompObj.h"
#include "Basic3DImportExport.h"
#include "MCBasicTypes.h"
#include "APITypes.h"
#include "COMUtilities.h"
#include "I3DShScene.h"


const MCGUID MCGUID_OSGExporter(GUID_OSGExporter);
const MCGUID MCGUID_IVEExporter(GUID_IVEExporter);

class OSGExporter : public TBasic3DExportFilter
{
public:
	OSGExporter( bool binary ) : mBinary(binary) {}

	STANDARD_RELEASE;

	virtual int32		MCCOMAPI GetParamsBufferSize	() const	{ return 0;}

	// for 3D exporters elem maps to I3DShScene and subElem maps toI3DShTreeElement through QueryInterface
	virtual MCCOMErr 	MCCOMAPI DoExport	(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree);

protected:

	bool	mBinary; // If true: save as IVE, else save as OSG

	osg::StateSet* ConvertMasterShader( I3DShMasterShader* masterShader );
	osg::Node* ConvertTree( I3DShTreeElement* tree );
	bool ConvertLightInstance( I3DShInstance* lightInstance );
	osg::Node* ConvertPrimitiveInstance( I3DShInstance* instance );

	OSGLightManager mLightManager;

	std::map<I3DShMasterShader*,osg::StateSet*> mConvertedMasterShaders;

	// Progress bar
	TMCCountedPtr<IMCUnknown> mProgressKey;
};

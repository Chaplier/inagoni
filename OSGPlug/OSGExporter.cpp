/****************************************************************************************************

		OSGExporter.cpp

		Author:	Julien Chaplier
		Date:	16/08/2008

****************************************************************************************************/

#include "OSGExporter.h"

#include <math.h>

#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Texture2D>
#include <osg/Material>
#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include "MCColorRGBA.h"
#include "I3DShScene.h"
#include "I3DShGroup.h"
#include "I3DShInstance.h"
#include "I3DShShader.h"
#include "ShaderDefs.h"
#include "ISHUtilities.h"
#include "MCBasicTypes.h"
#include "PublicUtilities.h"
#include "I3DShCamera.h"
#include "I3DShLightSource.h"
#include "I3DShObject.h"
#include "MCColor16.h"
#include "ColorRGB214.h"
#include "IShColorConversionUtilities.h"
#include "COMSafeUtilities.h"
#include "I3DShTreeElement.h"
#include "IShComponent.h"
#include "I3dExLight.h"
#include "IShTextureMap.h"
#include "PMapTypes.h"
#include "UVMaps.h"

MCCOMErr OSGExporter::DoExport(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree)
{
	try
	{
		// Progress bar
		TMCString255 progressMsg;

		// Find the string in the ressources
		void* oldResources = NULL;
		gResourceUtilities->SetupComponentResources('3Dou', 'OsgC', &oldResources);
		gResourceUtilities->GetIndString(progressMsg, 131, 1);		// string  1 of STR# 131 (see OSGPlug.r)
		gResourceUtilities->RestoreComponentResources(oldResources);

		const uint32 nbInst = scene->GetInstanceListCount();
		gShellUtilities->BeginProgress(progressMsg, &mProgressKey, nbInst);

		// Open the file
		TMCDynamicString fullPathName;
		file->GetFileFullPathName(fullPathName);

		TMCDynamicString fileExt;
		file->GetExtension(fileExt);
		if( fileExt!=TMCString15("osg") && fileExt!=TMCString15("ive") )
		{	// Carrara save the scene in a .tmp file, then change the name itself
			file->SetExtension(mBinary?TMCString15("ive"):TMCString15("osg"));
			file->GetFileFullPathName(fullPathName);
		}

		// Convert the scene
		osg::Group* globalRoot = new osg::Group();
		mLightManager.SetSceneRoot( globalRoot );

		osg::Node* scene = ConvertTree( fatherTree );
		globalRoot->addChild( scene );

		// Save it
		std::string fileNameStr((char*)fullPathName.StrGet());
		osgDB::Registry* registry = osgDB::Registry::instance();
		// Is there a bug in this version of OSG? When getenv("OSG_LIBRARY_PATH") is called from within the
		// registry, it always return NULL, and the library path is not initialized. So I do it from here 
		// one more time
		if( getenv( "OSG_LIBRARY_PATH") )
		{
			registry->setLibraryFilePathList( std::string( getenv( "OSG_LIBRARY_PATH") ) );
		}

		osgDB::ReaderWriter::WriteResult res = registry->writeNode(*globalRoot, fileNameStr, registry->getOptions());

		gShellUtilities->EndProgress(mProgressKey);

		// Carrara has a weird way to save the scene: it saves it into a .tmp file, then rename the
		// .tmp file into the wanted file format.
		// So we need to rename the .osg file into a .tmp file.
		// Restore the previous extension
		file->SetExtension(fileExt);
		file->GetFileFullPathName(fullPathName);
		int result = rename ( fileNameStr.c_str(), (char*)fullPathName.StrGet() );
	}
	catch (TMCException& exception)
	{
		gShellUtilities->EndProgress(mProgressKey);
		throw exception;
	}
	return MC_S_OK;
}

// Conversion utilities

osg::Matrix ConvertTransform( const TMatrix33& rotation, const TVector3& translation )
{
	return osg::Matrix( rotation[0][0], rotation[0][1], rotation[0][2], 0, 
						rotation[1][0], rotation[1][1], rotation[1][2], 0, 
						rotation[2][0], rotation[2][1], rotation[2][2], 0, 
						translation[0], translation[1], translation[2], 1 ); 
}

osg::Vec3f Convert( const TVector3& vector )
{
	return osg::Vec3f(vector.x, vector.y, vector.z);
}

osg::Vec2f Convert( const TVector2& vector )
{
	return osg::Vec2f(vector.x, vector.y);
}

osg::Vec4d ConvertColor( const TMCColorRGB& color )
{
	return osg::Vec4d( color.red, color.green, color.blue, 1.0 );
}

osg::Vec4d ConvertColor( const TMCColorRGBA8& color )
{
	return osg::Vec4d( (double)(color.red)/255.0, (double)(color.green)/255.0, (double)(color.blue)/255.0, (double)(color.alpha)/255.0 );
}

osg::MatrixTransform* GetTransform( I3DShTreeElement* tree, osg::ref_ptr<osg::MatrixTransform>& osgScaling )
{
	TTreeTransform treeTransform = tree->GetLocalTreeTransform();
	
	TMatrix33 scaling = TMatrix33::kIdentity;

	TVector3 scalingXYZ = treeTransform.GetXYZScaling();
	if( scalingXYZ!=TVector3::kOnes )
	{
		// Carrara has a XYZ scaling that applies only to the primitive instances, 
		// but not to the scene graph

		treeTransform.SetXYZScaling( TVector3::kOnes );
	}

	float uniformScaling = treeTransform.GetUniformScaling();
	if( uniformScaling!=1.0 )
	{
		// Carrara has a uniform scaling that applies only to the primitive instances
		// and to the offset of the childrens, but not to the other elements of the scene graph

		treeTransform.SetUniformScaling( 1.0 );
	}

	scalingXYZ *= uniformScaling;
	scaling.SetDiagonal( scalingXYZ.x, scalingXYZ.y, scalingXYZ.z );
	osgScaling = new osg::MatrixTransform;
	osgScaling->setName( tree->GetName().StrGet() );
	osgScaling->setMatrix( ConvertTransform( scaling, TVector3::kZero ) );

	TMatrix33 rotation = treeTransform.GetRotation();
	TVector3 translation = treeTransform.GetOffset();

	if( rotation == TMatrix33::kIdentity && translation == TVector3::kZero )
		return NULL;

	osg::MatrixTransform* osgTransform = new osg::MatrixTransform;
	osgTransform->setName( tree->GetName().StrGet() );
	osgTransform->setMatrix( ConvertTransform( rotation, translation ) );

	return osgTransform;
}

bool OSGExporter::ConvertLightInstance( I3DShInstance* lightInstance )
{
	TMCCountedPtr<I3DShLightsource> lightSource;
	lightInstance->QueryInterface( IID_I3DShLightsource , (void **) &lightSource);
	if(lightSource)
	{
		TStandardLight lightFlag;
		lightSource->GetStandardLight(lightFlag);

		TTransform3D transform;
		lightSource->GetLightGlobalTransform(&transform);

		// Use the LightManager to see if there's still an OpenGL light available
		osg::Light* osgLight = mLightManager.GetNextAvailableLight();
		if( !osgLight )
			return false;

		switch(lightFlag.fLightType)
		{
		case TStandardLight::kAmbientLight:
			{
				osgLight->setAmbient(ConvertColor(lightFlag.fDiffuse));
			} break;
		case TStandardLight::kDistantLight:
			{
				osgLight->setDiffuse(ConvertColor(lightFlag.fDiffuse));
				osgLight->setSpecular(ConvertColor(lightFlag.fSpecular));

				TVector3 dir = transform.TransformVector( TVector3::kUnitZ );
				osgLight->setPosition( osg::Vec4( dir.x, dir.y, dir.z, 0.0 ) ); // 0.0 for a directional light
			} break;
		case TStandardLight::kPointLight:
			{
				osgLight->setDiffuse(ConvertColor(lightFlag.fDiffuse));
				osgLight->setSpecular(ConvertColor(lightFlag.fSpecular));

				osgLight->setPosition( osg::Vec4( transform.fTranslation.x, transform.fTranslation.y, transform.fTranslation.z, 1.0 ) );
			} break;
		case TStandardLight::kSpotLight:
			{
				osgLight->setDiffuse(ConvertColor(lightFlag.fDiffuse));
				osgLight->setSpecular(ConvertColor(lightFlag.fSpecular));

				osgLight->setPosition( osg::Vec4( transform.fTranslation.x, transform.fTranslation.y, transform.fTranslation.z, 1.0 ) );
				TVector3 dir = transform.TransformVector( -TVector3::kUnitZ );
				osgLight->setDirection( Convert(dir) );
				osgLight->setSpotCutoff(lightFlag.fConeAngleDegreesMax);
			} break;
		}

		return true;
	}

	return false;
}

// Run through the shading graph to find a TextureMap component
bool FindTextureMapInShader( I3DShShader* shader, std::string& textureFileFullName )
{
	if(shader)
	{
		boolean ok = false;

		ShadingFlags theFlags;
		shader->GetShadingFlags(theFlags);

		if(!theFlags.fUVSpaceShaders)
		{
			TMCCountedPtr<IShTextureMap> outMap;
			shader->GetOriginalParametricTextureMap(0, 0, &outMap);
			if(outMap)
			{
				TMCCountedPtr<IMCFile> textureFile;
				IDType format;
				outMap->GetFile(&textureFile, format);
				if( textureFile )
				{
					TMCDynamicString pathName;
					textureFile->GetFileFullPathName( pathName );

					textureFileFullName = pathName.StrGet();
					return true;
				}
			}

		}
	}

	TMCCountedPtr<IShMinimalParameterComponent> shaderMinComponent;
	shader->QueryInterface(IID_IShMinimalParameterComponent,(void**)&shaderMinComponent);
	if(shaderMinComponent)
	{
		uint32 paramCount = shaderMinComponent->GetParameterCount();
		for( uint32 iParam=0 ; iParam<paramCount ; iParam++ )
		{
			TPMapElement info;
			shaderMinComponent->GetParameterInfo(iParam, info);
			if( info.fParamType == kComponentParamType )
			{
				TMCCountedPtr<IShMinimalParameterComponent> subComponent;
				shaderMinComponent->GetParameter( info.fPartID, &subComponent );
				{
					if(subComponent)
					{
						TMCCountedPtr<I3DShShader> subShader;
						subComponent->QueryInterface(IID_I3DShShader, (void**)&subShader);
						if( FindTextureMapInShader( subShader, textureFileFullName ) )
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool GetCompositeShaderChannel( IShMinimalParameterComponent* shaderMinComponent, int channelId, I3DShShader** shaderOut )
{
	TMCCountedPtr<IShMinimalParameterComponent> component;
	shaderMinComponent->GetParameter( channelId, &component );
	if(component)
	{
		component->QueryInterface(IID_I3DShShader, (void**)shaderOut);
		if( *shaderOut )
		{
			return true;
		}
	}

	return false;
}

osg::StateSet* OSGExporter::ConvertMasterShader( I3DShMasterShader* masterShader )
{
	// Check if this one was already converted
	if( mConvertedMasterShaders.find( masterShader )!=mConvertedMasterShaders.end() )
	{
		return mConvertedMasterShaders[masterShader];
	}

	// Convert the master shader into a stateSet
	osg::StateSet* shaderStateset = new osg::StateSet();
	TMCDynamicString shaderName;
	masterShader->GetName(shaderName); 
	shaderStateset->setName( shaderName.StrGet() );

	osg::Material* material = NULL; 

	TMCCountedPtr<I3DShShader> shader;
	masterShader->GetShader(&shader);
	if( shader )
	{
		TMCCountedPtr<IShMinimalParameterComponent> shaderMinComponent;
		shader->QueryInterface(IID_IShMinimalParameterComponent,(void**)&shaderMinComponent);
		if(shaderMinComponent)
		{
			if(shaderMinComponent->GetClassSignature()==kCompositeShader)
			{	// Regular Multi Channel shader
				// Convert the channels that make sens in OSG

				TMCColorRGBA color;
				ShadingIn shadingIn;
				
				// Get its color shader.
				// From it, ask a ShadingApproximation
				// and try to find a texture map in the shading tree
				TMCCountedPtr<I3DShShader> colorShader;
				if( GetCompositeShaderChannel(shaderMinComponent, kCompositeShader_Color, &colorShader) )
				{
					colorShader->GetShaderApproxColor( color, shadingIn );

					if( !material )
						material = new osg::Material();

					material->setAmbient( osg::Material::FRONT_AND_BACK, ConvertColor(color) );
					material->setDiffuse( osg::Material::FRONT_AND_BACK, ConvertColor(color) );

					// Texture map
					std::string textureFileFullName;
					if( FindTextureMapInShader( colorShader, textureFileFullName ) )
					{	
						osg::Image* image = osgDB::readImageFile(textureFileFullName);
						if( image )
						{
							osg::Texture2D* texture = new osg::Texture2D;
							texture->setImage(image);
						    
							shaderStateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
						}
						else
						{
							// Tell the user that the file format is not supported
						}
					}
				}

				TMCCountedPtr<I3DShShader> highLightShader;
				if( GetCompositeShaderChannel(shaderMinComponent, kCompositeShader_Highlight, &highLightShader) )
				{
					highLightShader->GetShaderApproxColor( color, shadingIn );

					if( !material )
						material = new osg::Material();

					material->setSpecular( osg::Material::FRONT_AND_BACK, ConvertColor(color) );
				}

				TMCCountedPtr<I3DShShader> shininessShader;
				if( GetCompositeShaderChannel(shaderMinComponent, kCompositeShader_Shininess, &shininessShader) )
				{
					shininessShader->GetShaderApproxColor( color, shadingIn );

					if( !material )
						material = new osg::Material();

					material->setShininess( osg::Material::FRONT_AND_BACK, color.Intensity() );
				}

				TMCCountedPtr<I3DShShader> glowShader;
				if( GetCompositeShaderChannel(shaderMinComponent, kCompositeShader_Glow, &glowShader) )
				{
					glowShader->GetShaderApproxColor( color, shadingIn );

					if( !material )
						material = new osg::Material();

					material->setEmission( osg::Material::FRONT_AND_BACK, ConvertColor(color) );
				}
			}
		}
	}

	if( material )
	    shaderStateset->setAttributeAndModes( material, osg::StateAttribute::ON );

	mConvertedMasterShaders[masterShader] = shaderStateset;

	return shaderStateset;
}

osg::Node* OSGExporter::ConvertPrimitiveInstance( I3DShInstance* instance )
{
	FacetMesh* facetMesh = instance->GetRenderingFacetMesh();
	if( facetMesh && facetMesh->fFacets.GetElemCount() )
	{
		osg::Geometry* meshGeom = new osg::Geometry();

		// The triangles
		const TMCArray<Triangle>& meshTgl = facetMesh->fFacets;

		const TMCArray<TVector3>& meshVtx = facetMesh->fVertices;
		const TMCArray<TVector3>& meshNor = facetMesh->fNormals;
		const TMCArray<TVector2>& meshUVs = facetMesh->fuv;

		const int tglCount = meshTgl.GetElemCount();

		osg::Vec3Array* osgVertices = new osg::Vec3Array();
		osgVertices->reserve( 3*tglCount );

		osg::Vec3Array* osgNormals = new osg::Vec3Array();
		osgNormals->reserve( 3*tglCount );

		osg::Vec2Array* osgUVs = new osg::Vec2Array();
		osgUVs->reserve( 3*tglCount );

		for( int iTgl=0 ; iTgl<tglCount ; iTgl++ )
		{
			const Triangle& curTgl = meshTgl[iTgl];

			osgVertices->push_back( Convert(meshVtx[curTgl[0]]) );
			osgVertices->push_back( Convert(meshVtx[curTgl[1]]) );
			osgVertices->push_back( Convert(meshVtx[curTgl[2]]) );

			osgNormals->push_back( Convert(meshNor[curTgl[0]]) );
			osgNormals->push_back( Convert(meshNor[curTgl[1]]) );
			osgNormals->push_back( Convert(meshNor[curTgl[2]]) );

			osgUVs->push_back( Convert(meshUVs[curTgl[0]]) );
			osgUVs->push_back( Convert(meshUVs[curTgl[1]]) );
			osgUVs->push_back( Convert(meshUVs[curTgl[2]]) );
		}

		meshGeom->setVertexArray(osgVertices); 
		meshGeom->setNormalArray(osgNormals); 
		meshGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
		meshGeom->setTexCoordArray(0,osgUVs);
		meshGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,osgVertices->size()));

		// Set a default color
		const TMCArray<TMCColorRGBA8>& colors = facetMesh->fVertexColors;
		osg::ref_ptr<osg::Vec4Array> defaultColor = new osg::Vec4Array;
		if( colors.GetElemCount()==meshVtx.GetElemCount() )
		{
			for( uint32 iVtx=0 ; iVtx<meshVtx.GetElemCount() ; iVtx++ )
			{
				defaultColor->push_back(ConvertColor(colors[iVtx]));
			}
			meshGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		}
		else if( colors.GetElemCount()==1 )
		{
			defaultColor->push_back(ConvertColor(colors[0]));
			meshGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		}
		else
		{
			defaultColor->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
			meshGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		}
		meshGeom->setColorArray(defaultColor.get());

		// create the Geode (Geometry Node) to contain our osg::Geometry object.
		osg::Geode* geode = new osg::Geode();
		geode->addDrawable(meshGeom);

		TMCCountedPtr<I3DShObject> object;
		instance->Get3DObject(&object);
		if( object )
		{
			TMCDynamicString objectName;
			object->GetName(objectName);
			geode->setName( objectName.StrGet() );
		}

		// Convert the material
		TMCCountedPtr<I3DShMasterShader> masterShader;
		instance->GetShader(&masterShader);
		if( masterShader )
		{
			geode->setStateSet( ConvertMasterShader( masterShader ) );
		}

		return geode;
	}

	return NULL;
}

osg::Node* OSGExporter::ConvertTree( I3DShTreeElement* tree )
{
	if(!tree)
		return NULL;

	// If the object has a transform, get it here
	osg::ref_ptr<osg::MatrixTransform> osgScaling;
	osg::Group* osgGroup = GetTransform( tree, osgScaling );
	if( !osgGroup )
	{
		osgGroup = new osg::Group();
		osgGroup->setName( tree->GetName().StrGet() );
	}
	
	TMCCountedPtr<I3DShTreeElement> child;
	tree->GetFirst(&child);
	if( child )
	{
		if( !osgGroup )
		{
			osgGroup = new osg::Group();
			osgGroup->setName( tree->GetName().StrGet() );
		}

		while( child )
		{
			osg::Node* childNode = ConvertTree(child);
			if( childNode )
				osgGroup->addChild( childNode );

			// Get the next child
			child->GetRight(&child);
		}
	}

	TMCCountedPtr<I3DShInstance> instance;
	tree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
	if(instance)
	{
		osg::Node* osgInstance = NULL;

		switch( instance->GetInstanceKind() )
		{
		case I3DShInstance::kCameraInstance: 
			{
				return NULL;
			} break;
		case I3DShInstance::kLightInstance: 
			{
				// Light are not inserted in the scene graph, but at the Root, with the light manager
				ConvertLightInstance(instance);
			} break;
		case I3DShInstance::kSceneInstance: 
			{
				return NULL;
			} break;
		case I3DShInstance::kPrimitiveInstance: 
			{
				osgInstance = ConvertPrimitiveInstance(instance);
				osgScaling->addChild( osgInstance );
				osgGroup->addChild( osgScaling.get() );
			
				// increment the progress bar
				gShellUtilities->IncrementProgress(1, mProgressKey);

			} break;
		default:
			{
			} break;
		}
	}

	return osgGroup;
}


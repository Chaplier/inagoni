/****************************************************************************************************

		OSGLightManager.h

		Author:	Julien Chaplier
		Date:	16/08/2008

****************************************************************************************************/

#pragma once

#include <vector>
#include <osg/LightSource>
#include <osg/Light>

class OSGLightManager
{
public:

	OSGLightManager();
	~OSGLightManager() {}

	void SetSceneRoot( osg::Group * root ) {mSceneRoot = root;}
	// Returns NULL if 8 lights were already set
	osg::Light * GetNextAvailableLight();

protected:
	osg::ref_ptr<osg::Group>					mSceneRoot;
	std::vector<osg::ref_ptr<osg::LightSource>>		mLights;
};

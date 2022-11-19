#include "OSGLightManager.h"

OSGLightManager:: OSGLightManager()
{
}

osg::Light * OSGLightManager::GetNextAvailableLight()
{
	if( mLights.size()>=8 )
		return NULL; // no more available light

	//       Create a new light, and set the light number according to our parameter.
	osg::Light * light = new osg::Light();
	light->setLightNum(mLights.size());

	osg::LightSource * lightsource = new osg::LightSource();
	lightsource->setLight(light);
	lightsource->setStateSetModes(*mSceneRoot->getOrCreateStateSet(), osg::StateAttribute::ON);
	if( mSceneRoot.get() )
		mSceneRoot->addChild(lightsource);

	mLights.push_back( lightsource );

	// Return a pointer to the osg::Light so the user may modify the properties.
	return light;
}

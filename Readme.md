Inagoni plugins were developed for Carrara, distributed by Dax3D. They can be used on the Windows or the MacOSX version of Carrara.

## License

GNU GPL v3

## The plugins

ArchiTools is an advanced building modeler designed for 3D artists. Its UI has been especially conceived to make it intuitive, easy to learn and fun to use. Just a few clicks are necessary to create a complete building, and this new modeler being fully procedural, all its elements can be tuned and modified at any time during the construction process.

Velouté is a powerful procedural texture builder for Carrara. It contains more than 160 basic functions shared by 18 shaders that can be easily combined to create a huge variety of textures. Random Lines, Gradient, Grid, Weave, Tile, Fractal Noise are some of Velouté's shaders.
All Velouté's shaders implement their own bump mapping resulting in better looking effect than Carrara's default shaders.

Primivol contains the following volumetric primitives: Fire, Cloud, Smog, Rising Smoke. Through these primitive, Primivol can also render any 3D shader as a volume and can alter its shape with a set of modifiers and ramp-off functions. Lighting, self-shadow, density, color and many other parameters are accessible through the UI and can be precisely adjusted and animated.

Baker is a texture baking tool. Using the UV mapping of an object, it can extract flat texture maps, light maps or normal maps from any Carrara shader. 

Deeper is a small rendering engine to produce normal map from a 3D scene.

Replica clones objects in arrays or on existing surfaces, and each clone can even have a randomized or controlled position, scaling, orientation or even shading.

Shaper adds Free Form Deformation capabilities to Carrara. Shaper is a very handy deformer that allows you to deform any geometric object with a cubed grid of points.

Swap can replace any kind of element in the scene hierarchy by another one: it can replace lights with cameras, cameras with lights, or any objects with another one.

OSGPlug was an OpenSceneGraph exporter for Carrara and was built at the time for Carrara 6 Windows. It was made for a specific use case but was never properly finished. The code is still available as a sample.

## The code

The Carrara SDK is required to build the plugins and some familiarity with it is needed. 
Once the Carrara SDK is installed and its samples can be compiled and run in Carrara, move inagoni plugins code into the Carrara SDK next to the provided samples. For example, with Carrara 8 SDK, move the code into :
C8_SDK_8.0.0.215\Samples\AllProjects

The code is crossplatform and project for Windows and MacOSX are provided:
* On Windows platforms, open CompleteVC2008.sln to load all the projects at once. 
* On MacOSX platforms, each project need to be independantly opened with xcode.

## Running the plugins

The plugins are still using a serial number mechanism. It should be removed in a future update, but meanwhile these numbers can be used to run the plugins:

ArchiTools: 13B8-CIB1-220M-V0BC
Velouté: 54O2-CUD0-6402-R0VB
Primivol: 53X7-CNQ0-U10A-I0G8
Baker: 73B4-C3P0-L00A-J0BO
Deeper: 13D5-CIY1-J50O-W0NS
Replica: 53N9-COO0-030A-U0BK
Shaper: JP27-ATQ0-870H-U0S3
Swap: V367-CWQ0-870W-I0SU

/************************************************************************************************************

  	NormalMapRenderer.h

	Copyright (c)1990 - 1999 MetaCreations. All rights reserved.

	Author:	Mael Sicsic

	Date:	8/10/99

	Revision History

	when		who			what & why
	=========== ===========	=============================================================

************************************************************************************************************/
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "BasicFinalRenderer.h"
#include "NormalMapDef.h"
#include "BasicShader.h"

#include "MCColorRGB.h"
#include "Transforms.h"
#include "I3dShFacetMesh.h"
#include "I3DShEnvironment.h"
#include "I3DShInstance.h"
#include "I3DShGroup.h"
#include "I3DShCamera.h"
#include "RealAffine3.h"
#include "I3DShTreeElement.h"
#include "I3DShLightSource.h"
#include "IShRasterLayer.h"
#include "MCCountedPtrArray.h"
#include "MCException.h"

#define FPOSINF 1e20f
#define FNEGINF -1e20f
#define IPOSINF 2000000000
#define INEGINF -2000000000


// define the WireRenderer CLSID

extern const MCGUID CLSID_NormalMapRenderer;


class LightInfo
{
public:
	int fExists;
	struct I3DExLightsource *fExLight;
	class TGel *fGelComponent;

	LightInfo() : fExists(-1) {}
};

enum
{ 
	kDefaultMapping		=	-1,
	kParametricMapping	=	0,
	kBoxMapping			=	1,
	kCylindricalMapping	=	2,
	kSphericalMapping	=	3,
	kPassThruMapping	=	4
};


// This struct will be used as a buffer where we can draw all our image
struct FlatBuffer
{
	uint8* red;
	uint8* green;
	uint8* blue;
};

struct RendPMap
{	//Create a PMap-like structure
	RendPMap(){fFlipBackFace=false;}

	boolean			fFlipBackFace;
};


typedef I3DShLightsource* LightTable[100];


struct InstanceAndTransform			// This structure will be usefull during the rendering
{
	I3DShInstance*	fInstance;
	TTransform3D	fT;
};


class NormalMapRenderer : public TBasicFinalRenderer, public I3DExTileRenderer
{
	public:

		NormalMapRenderer();
		// IUnknown methods
		virtual MCErr    MCCOMAPI QueryInterface		(const MCIID& riid, void** ppvObj)	{ return TBasicFinalRenderer::QueryInterface(riid,ppvObj); }
		virtual uint32   MCCOMAPI AddRef				()									{ return TBasicFinalRenderer::AddRef(); }
		STANDARD_RELEASE;

		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer	();
		virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(RendPMap);}
		virtual int16	 MCCOMAPI GetResID					();

		// TBasicFinalRenderer methods
		virtual MCCOMErr MCCOMAPI SetTreeTop				(I3DShGroup* treeTop);
		virtual MCCOMErr MCCOMAPI SetCamera					(I3DShCamera* camera);
		virtual MCCOMErr MCCOMAPI SetEnvironment			(I3DShEnvironment* environment);
		virtual MCCOMErr MCCOMAPI SetAmbientLight			(const TMCColorRGB& ambiantColor);
		virtual MCCOMErr MCCOMAPI Dehydrate					(int16 level);
		virtual uint32   MCCOMAPI GetRenderingTime			();
		virtual MCCOMErr MCCOMAPI SetFieldRenderingData		(int32 useFieldRendering,int16 firstFrame);
		virtual void	 MCCOMAPI GetRenderStatistics		(I3DRenderStatistics** renderstats);
		virtual MCCOMErr MCCOMAPI PrepareDraw				(const TMCPoint& size,
															 const TBBox2D& uvBox,
															 const TBBox2D& productionFrame,
															 boolean (*callback) (int16 vv, void* priv),
															 void* priv);
		virtual MCCOMErr MCCOMAPI FinishDraw				();

		// called by the shell (renderingmodule) to tell the renderer to abort
		// this allow the renderer to cleanly abort and to release everything
		virtual MCCOMErr MCCOMAPI Abort					() { fAbort=true; return MC_S_OK; }
		virtual void	 MCCOMAPI GetTileRenderer		(I3DExTileRenderer** tileRenderer);

		// I3DExTileRenderer
		virtual void	 MCCOMAPI Init					(const TMCRect& rect,const TBBox2D& uvBox);
		virtual boolean  MCCOMAPI GetNextSubTile		(TMCRect& rect);
		virtual void	 MCCOMAPI RenderSubTile			(RTData& pixels);
		virtual void	 MCCOMAPI FinishRender			(const RTData& pixels);

	protected:

		//All items that are requested by the shader i.e:by Doshade() method

		real	*rightX, *leftX;
		real	*rightZ, *leftZ;
		real	*rightGlobalPosX, *leftGlobalPosX;
		real	*rightGlobalPosY, *leftGlobalPosY;
		real	*rightGlobalPosZ, *leftGlobalPosZ;
		
		real	*rightGlobalNorX, *leftGlobalNorX;
		real	*rightGlobalNorY, *leftGlobalNorY;
		real	*rightGlobalNorZ, *leftGlobalNorZ;
		
		real	*rightLocPosX, *leftLocPosX;
		real	*rightLocPosY, *leftLocPosY;
		real	*rightLocPosZ, *leftLocPosZ;
		
		real	*rightLocNorX, *leftLocNorX;
		real	*rightLocNorY, *leftLocNorY;
		real	*rightLocNorZ, *leftLocNorZ;
		
		real	*rightU, *leftU;
		real	*rightV, *leftV;
		

		//Other member data
		real			*fZbuffer;			//Array for the Zbuffer
		int32			*fScreenBuffer;		//Image buffer		
//		int32			fFrontier;			//limit between Wire and Phong
		int32			fNbLights;			
		RendPMap		fPMap;			// Data buffer corresponding to the PMap
		LightTable		fLights;
//		TMCCountedPtrArray<I3DShLightsource> fLights;

		ShadingFlags	fFlags;				//Flags given by the shader
		boolean			fParametricMapping; //Given an instance, TRUE for Parametric Mapping and FALSE for Projection Mapping
		TVector3		fScreenPoint[3];	// Current facet in Screen Space Coord: projection, and dist to screen
		TVector3		fScreenCoordPoint[3]; // In 3D
		TVertex3D		fGlobalVertex[3];	//Current facet in Global System coord
		int32			fIndex;

		
		// Methods of the wire renderer.
		void	Rasterize	(const TFacet3D& facet, const TTransform3D& L2G, const TTransform3D& G2L, const TTransform3D& L2C, const TTransform3D& G2C, I3DShInstance *CurrentInstance );
		void	RenderScene	(const TMCRect* area);
		void	BlocCopy	(const RTData* pixels);
		void	PixelSet	(int32 x, int32 y, TMCColorRGBA* color);
		void	ZBufferSet	(int32 x, int32 y, real val);
		void	AllocMem();
		void	Interpolate(real , real , long , long , long, real *);
		void	FillPoly(TFacet3D facet, I3DShInstance *CurrentInstance, RayHit3D *myShadingIn, int numVertices, const TTransform3D& L2G, const TTransform3D& G2L, const TTransform3D& L2C, const TTransform3D& G2C);
		void 	PixelColor(const ShadingOut *, const TVertex3D, const boolean, const TVector3, const TVector3, TMCColorRGBA& color);
		void	BackgroundColor(TVector2 theScreenUV, TMCColorRGBA& colorResult);
		void	InitZbuffer();
		void	ReleaseMem();

#if (VERSIONNUMBER >= 0x040000)
		void CheckAbort(){if( fAbort ){throw TMCException(kAbortThreadException);}}
#else
		void CheckAbort(){if( fAbort ){throw TMCException(kAbortRenderingException);}}
#endif
		
		// Datas of the renderer
		TMCCountedPtr<I3DShTreeElement>		fTree;
		TMCCountedPtr<I3DShGroup>			fTreeTop;
		const TRenderableAndTfmArray*		fInstances;
		
		TMCCountedPtr<IShRasterLayer>		fImage;			// Image to render
		TMCCountedPtr<I3DShCamera>			fCamera;		// Camera used for the rendering
//		TMCCountedPtr<I3DShEnvironment>		fBackground;	// Environment image used for the rendering
//		TMCCountedPtr<I3DShEnvironment>		fBackdrop;		// Screen Background image used for the rendering
//		TMCCountedPtr<I3DShEnvironment>		fAtmosphere;	// Atmosphere effect used for the rendering

		TMCColorRGB			fAmbientColor;	// Ambient color for the rendering
		TVector2 			fZoom;			// Zoom factor Univers / OffScreen
		TMCRect				fImageArea;		// The full image obtained by the rendering process
		TMCRect				fDrawingArea;	// An area, used by the renderer, which can be a tile of the fImageArea
		int16				fMode;			// Rendering mode, i.e. Wireframe, Ray Tracer, ...
		int16				fImageWidth;	// Dimensions of the image to be drawn
		int16				fImageHeight;	
		int16				fDepth;			// Number of bits to code the color
		TVector2			fOffscrOffset;	// Indicates a distance between the offscreen and the bitmapArea
		TBBox2D				fUVMinMax;		// Dimension of backdrop
		FlatBuffer			fBuffer;

		TMCRect				fCurrentTileRect;	// rectangle of the current tile
		TBBox2D				fCurrentUVBox;		// uv box of the current tile

		// Abort mechanism
		boolean				fAbort;
};

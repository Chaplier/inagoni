/************************************************************************************************************
	NormalMapRenderer.cpp
	Copyright (c)1990 - 1999 MetaCreations. All rights reserved.

	Author:	Mael Sicsic

	Date:	8/10/99
	Revision History

	when		who			what & why
	=========== ===========	=============================================================
	08/20/99	cph			Put in Extension3DCleanup() methods

************************************************************************************************************/


#include "copyright.h"
#include "math.h"
#include "NormalMapRenderer.h"
#include "MCCountedPtrHelper.h"
#include "I3DShUtilities.h"
#if (VERSIONNUMBER >= 0x040000)
#include "IShThreadUtilities.h"
#endif
#include "I3DShScene.h"
#include "ToolBox.h"
#include "PublicUtilities.h"
#include "IShRasterLayerUtilities.h"
#include "MiscComUtilsImpl.h"
#include "RenderTypes.h"
#if (VERSIONNUMBER >= 0x050000)
#include "I3DShRenderable.h"
#include "RealQuat.h"
#endif

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_NormalMapRenderer(R_CLSID_NormalMapRenderer);
#else
const MCGUID CLSID_NormalMapRenderer={R_CLSID_NormalMapRenderer};
#endif


// Constructor of the renderer
NormalMapRenderer::NormalMapRenderer()
{
	fAmbientColor.R = 0.5f;	
	fAmbientColor.G = 0.5f;
	fAmbientColor.B = 0.5f;
//	fAtmosphere = NULL;
//	fBackdrop = NULL;
//	fBackground = NULL;
	fCamera = NULL;
	fDrawingArea.left = 0;
	fDrawingArea.right = 0;
	fDrawingArea.top = 0;
	fDrawingArea.bottom = 0;
	fImageArea.left = 0;
	fImageArea.right = 0;
	fImageArea.top = 0;
	fImageArea.bottom = 0;
	fMode = 0;
	fOffscrOffset[0] = 0.0f;
	fOffscrOffset[1] = 0.0f;
	fTreeTop = NULL;
	fZoom[0] = 1.0f;
	fZoom[1] = 1.0f;
//	fPMap.frontier = 320;

	fAbort = false;
}

//*****************************************************************
//				BasicDataExchanger Methods
//*****************************************************************

void* NormalMapRenderer::GetExtensionDataBuffer()
{
	return &fPMap;
}

int16 NormalMapRenderer::GetResID()
{
	return 650;
}	//this is the ID in the resource file (typically .DTA)

//*****************************************************************
//				I3DExRenderer Methods
//*****************************************************************

// specifies which tree needs to be rendered (generally the universe)
MCCOMErr NormalMapRenderer::SetTreeTop(I3DShGroup* treeTop)
{
	fTreeTop = treeTop;
	return MC_S_OK;
}

// specifies the rendering camera
MCCOMErr NormalMapRenderer::SetCamera (I3DShCamera* camera)
{
	fCamera = camera;
	return MC_S_OK;
} 

// gives the background, backdrop & atmospheric shader.
MCCOMErr NormalMapRenderer::SetEnvironment (I3DShEnvironment* environment)
{
/*	if (!environment || environment->HasBackground()) fBackground=environment;
	if (!environment || environment->HasBackdrop()) fBackdrop=environment;
	if (!environment || environment->HasAtmosphere()) fAtmosphere=environment;
*/	return MC_S_OK;
}	



// sets the ambient light color

MCCOMErr NormalMapRenderer::SetAmbientLight (const TMCColorRGB &ambientColor)

{

	fAmbientColor = ambientColor;

	return MC_S_OK;

}



// used to specify which buffers owned by the component could be freed in case Kwak would lack memory

MCCOMErr NormalMapRenderer::Dehydrate (int16 level)

{

	return MC_S_OK;

}



// this corresponds to an option in Scene Settings/Output

uint32 NormalMapRenderer::GetRenderingTime ()

{

	return 0;

}



// multiple frames for video compositing (option in Scene Settings/Renderer)

MCCOMErr NormalMapRenderer::SetFieldRenderingData (int32 useFieldRendering,int16 firstFrame)

{

	return MC_S_OK;

}





void NormalMapRenderer::InitZbuffer()

{

	//Seems very clear, we initialize the Zbuffer

	fZbuffer = new real[fImageHeight*fImageWidth];

	

	for ( int i=0; i<fImageHeight*fImageWidth; i++)

		fZbuffer[i] = FPOSINF;

}





void NormalMapRenderer::AllocMem()

{

	//According to the shading flags we instanciate only the requested items array

	if ( rightX == NULL)

	{

		rightX = new real[fImageHeight];

		leftX = new real[fImageHeight];

		

		rightZ = new real[fImageHeight];

		leftZ = new real[fImageHeight];

		

		rightGlobalPosX = new real[fImageHeight];

		leftGlobalPosX = new real[fImageHeight];

		rightGlobalPosY = new real[fImageHeight];

		leftGlobalPosY = new real[fImageHeight];

		rightGlobalPosZ = new real[fImageHeight];

		leftGlobalPosZ = new real[fImageHeight];

		

		rightGlobalNorX = new real[fImageHeight];

		leftGlobalNorX = new real[fImageHeight];

		rightGlobalNorY = new real[fImageHeight];

		leftGlobalNorY = new real[fImageHeight];

		rightGlobalNorZ = new real[fImageHeight];

		leftGlobalNorZ = new real[fImageHeight];

	}

	

	if (( fFlags.fNeedsUV == (uint)true ) && ( rightU == NULL))

	{

		rightU = new real[fImageHeight];

		leftU = new real[fImageHeight];

		rightV = new real[fImageHeight];

		leftV = new real[fImageHeight];

	}

	

	if (( fFlags.fNeedsPointLoc == (uint)true ) && ( rightLocPosX == NULL ))

	{

		rightLocPosX = new real[fImageHeight];

		leftLocPosX = new real[fImageHeight];

		rightLocPosY = new real[fImageHeight];

		leftLocPosY = new real[fImageHeight];

		rightLocPosZ = new real[fImageHeight];

		leftLocPosZ = new real[fImageHeight];

	}

	

	if (( fFlags.fNeedsNormalLoc == (uint)true ) && ( rightLocNorX == NULL )) 

	{

		rightLocNorX = new real[fImageHeight];

		leftLocNorX = new real[fImageHeight];

		rightLocNorY = new real[fImageHeight];

		leftLocNorY = new real[fImageHeight];

		rightLocNorZ = new real[fImageHeight];

		leftLocNorZ = new real[fImageHeight];

	}

}





void NormalMapRenderer::ReleaseMem()

{
	//According to the shading flags we instanciate only the requested items array
	if ( leftX != NULL) {delete []leftX; leftX=NULL;}
	if ( leftZ != NULL) {delete []leftZ; leftZ=NULL;}
	if ( leftU != NULL) {delete []leftU; leftU=NULL;}
	if ( leftV != NULL) {delete []leftV; leftV=NULL;}
	if ( leftGlobalPosX != NULL) {delete []leftGlobalPosX; leftGlobalPosX=NULL;}
	if ( leftGlobalPosY != NULL) {delete []leftGlobalPosY; leftGlobalPosY=NULL;}
	if ( leftGlobalPosZ != NULL) {delete []leftGlobalPosZ; leftGlobalPosZ=NULL;}
	if ( leftGlobalNorX != NULL) {delete []leftGlobalNorX; leftGlobalNorX=NULL;}
	if ( leftGlobalNorY != NULL) {delete []leftGlobalNorY; leftGlobalNorY=NULL;}
	if ( leftGlobalNorZ != NULL) {delete []leftGlobalNorZ; leftGlobalNorZ=NULL;}
	if ( leftLocPosX != NULL) {delete []leftLocPosX; leftLocPosX=NULL;}
	if ( leftLocPosY != NULL) {delete []leftLocPosY; leftLocPosY=NULL;}
	if ( leftLocPosZ != NULL) {delete []leftLocPosZ; leftLocPosZ=NULL;}
	if ( leftLocNorX != NULL) {delete []leftLocNorX; leftLocNorX=NULL;}
	if ( leftLocNorY != NULL) {delete []leftLocNorY; leftLocNorY=NULL;}
	if ( leftLocNorZ != NULL) {delete []leftLocNorZ; leftLocNorZ=NULL;}

	if ( rightX != NULL) {delete []rightX; rightX=NULL;}
	if ( rightZ != NULL) {delete []rightZ; rightZ=NULL;}
	if ( rightU != NULL) {delete []rightU; rightU=NULL;}
	if ( rightV != NULL) {delete []rightV; rightV=NULL;}
	if ( rightGlobalPosX != NULL) {delete []rightGlobalPosX; rightGlobalPosX=NULL;}
	if ( rightGlobalPosY != NULL) {delete []rightGlobalPosY; rightGlobalPosY=NULL;}
	if ( rightGlobalPosZ != NULL) {delete []rightGlobalPosZ; rightGlobalPosZ=NULL;}
	if ( rightGlobalNorX != NULL) {delete []rightGlobalNorX; rightGlobalNorX=NULL;}
	if ( rightGlobalNorY != NULL) {delete []rightGlobalNorY; rightGlobalNorY=NULL;}
	if ( rightGlobalNorZ != NULL) {delete []rightGlobalNorZ; rightGlobalNorZ=NULL;}
	if ( rightLocPosX != NULL) {delete []rightLocPosX; rightLocPosX=NULL;}
	if ( rightLocPosY != NULL) {delete []rightLocPosY; rightLocPosY=NULL;}
	if ( rightLocPosZ != NULL) {delete []rightLocPosZ; rightLocPosZ=NULL;}
	if ( rightLocNorX != NULL) {delete []rightLocNorX; rightLocNorX=NULL;}
	if ( rightLocNorY != NULL) {delete []rightLocNorY; rightLocNorY=NULL;}
	if ( rightLocNorZ != NULL) {delete []rightLocNorZ; rightLocNorZ=NULL;}
}

//*****************************************************************

//				WireRenderer Methods

//*****************************************************************



// This method is called by the the PrepareDraw method. It allows us to draw the image in our buffer

void NormalMapRenderer::RenderScene(const TMCRect* area)
{
	TMCCountedPtr<IMCUnknown>	progressKey; // Progress bar key

	try
	{
		// Progress Bar
		TMCString255				progressString;
		CWhileInCompResFile compResFile('frnd', 'NMRn');
		gResourceUtilities->GetIndString(progressString, kStrings, 1);
		gShellUtilities->BeginProgress(progressString, &progressKey);

		TTransform3D G2C ;		//Transformation from Global to Camera 
		TTransform3D L2C ;		//Transformation from Local to Camera
		TTransform3D L2G ;		//Transformation from Local to Global
		TTransform3D G2L ;

		TMCCountedPtr<FacetMesh> amesh;	// pointer to IFacetMesh interface

		TFacet3D		facet;
		TMCColorRGBA	color, notused;
		int32			theKind=0;	//Used to stock the kind of an instance

		fTreeTop->QueryInterface(IID_I3DShTreeElement,(void**)&fTree);
		ThrowIfNil(fTree);
		fTree->BeginGetRenderables(fInstances);

		fCamera->GetGlobalToCameraTransform(&G2C) ; //We get transformation from Global to Camera,

		//actually in Screen Coord 

		FacetMeshFacetIterator facetIterator;
		TRenderableAndTfmArray::const_iterator	iter = fInstances->Begin();

		const int32 instancesCount = fInstances->GetElemCount()-2;// average: usualy one light and one camera
		const real32 progressPerInstance = 100/(real32)(instancesCount>0?instancesCount:1);

		for (const TRenderableAndTfm* current = iter.First(); iter.More(); current = iter.Next())
		{
			theKind = current->fInstance->GetInstanceKind();
			if ((theKind == 3) || (theKind == 4)) 
				continue;

			L2G = current->fT ;		//In InstanceAndTransform fT is not a Translation as we may imagine
									//but a AFFINETRANSFORM cad Translation + Rotation
									//we get transformation from Local to Global system		

			//Then we get our complete transformation from local to Camera Coordinate
			L2C = G2C*L2G;		
			G2L = L2G.GetInverse();

			current->fInstance->GetShadingFlags(fFlags); //We get flags from shaders
			current->fInstance->GetIndex(fIndex);

			fParametricMapping = true;
			
			AllocMem(); //We allocate the right buffers for the right items
			
			current->fInstance->GetMainColors(color,notused);
#if (VERSIONNUMBER >= 0x050000)
			amesh = current->fInstance->GetRenderingFacetMesh();
#else
			current->fInstance->GetFMesh(0.0, &amesh) ;		//get a pointer to IFacetMesh interface
#endif			
			facetIterator.Initialize( amesh ) ;					//Initialization for a using of a facet iterator
			
			const int32 facetCount = amesh->FacetsNbr();
			int32 incOne = facetCount/progressPerInstance;
			if(incOne<1)incOne=1;
			int32 iFacet = 0;

			for ( facetIterator.First(); facetIterator.More(); facetIterator.Next())
			{ // browsing into facets list
				facet = facetIterator.GetFacet();
				Rasterize( facet, L2G, G2L, L2C, G2C, current->fInstance );	

				// let the main thread take the lead
#if (VERSIONNUMBER >= 0x040000)
				gShellThreadUtilities->YieldProcesses(15);   
#else
				gShellUtilities->YieldProcesses(15);   
#endif
				
				// Throw an abort exeption
				CheckAbort();
				// Increment progress bar
				iFacet++;
				if(iFacet%incOne == 0)
					gShellUtilities->IncrementProgress(1, progressKey);
			} 
			
			//We're now filling the pixels which don't refer to any object and thus have been left aside
			for (int y = 0; y < fImageHeight ; y++)
			{
				TMCColorRGBA color;
				int line = y*fImageWidth;
				TVector2 screenXY;
				screenXY[1] = (real)y;
				for (int x=0; x<fImageWidth; x++)
				{
					if (fZbuffer[line+x] == FPOSINF)
					{
						color.R = 0;
						color.G = 0;
						color.B = 0;
						screenXY[0] = (real)x;
						BackgroundColor(screenXY,color);
						PixelSet(x,y,&color);
					}
				}
			}				
			ReleaseMem();
		}	
		fTree->EndGetRenderables();
		fTree = NULL;
		fTreeTop = NULL;

		// Progress bar
		gShellUtilities->EndProgress(progressKey);
	}
	catch(TMCException& exception)
	{	// Can catch an Abort exception
		ReleaseMem();

		fTree->EndGetRenderables();
		fTree = NULL;
		fTreeTop = NULL;

		// Progress bar
		gShellUtilities->EndProgress(progressKey);

		throw exception;
	}
}


// This method is called just at the beginning of the render
MCCOMErr NormalMapRenderer::PrepareDraw (const TMCPoint& size,const TBBox2D& uvBox,const TBBox2D& productionFrame, boolean (*callback) (int16 vv, void *priv),void* priv)
{	
	int scrBufferSize = fImageHeight*fImageWidth;

	//In order to use the powerful Raydream shader we are supposed to interpolate the right items 
	//at the right time,it's the only difficulty in this code.
	//Then, we have chosen one temporary array per item


	rightX = NULL; leftX = NULL;
	rightZ = NULL; leftZ = NULL;

	rightGlobalPosX = NULL; leftGlobalPosX = NULL;
	rightGlobalPosY = NULL; leftGlobalPosY = NULL;
	rightGlobalPosZ = NULL; leftGlobalPosZ = NULL;


	rightGlobalNorX = NULL; leftGlobalNorX = NULL;
	rightGlobalNorY = NULL; leftGlobalNorY = NULL;
	rightGlobalNorZ = NULL; leftGlobalNorZ = NULL;

	rightLocPosX = NULL; leftLocPosX = NULL;
	rightLocPosY = NULL; leftLocPosY = NULL;
	rightLocPosZ = NULL; leftLocPosZ = NULL;

	rightLocNorX = NULL; leftLocNorX = NULL;
	rightLocNorY = NULL; leftLocNorY = NULL;
	rightLocNorZ = NULL; leftLocNorZ = NULL;

	rightU = NULL; leftU = NULL;
	rightV = NULL; leftV = NULL;

	fImageArea = TMCRect(0,0,size.x,size.y);

	fBuffer.red   = NULL;
	fBuffer.green = NULL;
	fBuffer.blue  = NULL;

	fZbuffer  = NULL;

	fImageWidth  = size.x;	// We get the dimensions of
	fImageHeight = size.y;	// the image to be drawn
	fDepth = 8;				// For compatibility with Put32

	fZoom[0] = size.x/2;
	fZoom[1] = size.y/2;

	fZoom[0] = real(size.x) / (uvBox.fMax[0] - uvBox.fMin[0]);
	fZoom[1] = real(size.y) / (uvBox.fMax[1] - uvBox.fMin[1]);

	fOffscrOffset[0] = real(size.x) * (-uvBox.fMin[0] / (uvBox.fMax[0] - uvBox.fMin[0]));
	fOffscrOffset[1] = real(size.y) * (uvBox.fMax[1] / (uvBox.fMax[1] - uvBox.fMin[1]));

	fUVMinMax = productionFrame;
	
//	GetLights();
	InitZbuffer();

	// init the screen buffer

	fBuffer.red   = new uint8[size.x*size.y];
	fBuffer.green = new uint8[size.x*size.y];
	fBuffer.blue  = new uint8[size.x*size.y];

	for (int32 i=0; i<fImageHeight*fImageWidth; i++)
	{
		fBuffer.red[i]=0;
		fBuffer.green[i]=120;
		fBuffer.blue[i]=120;
	}

	RenderScene(&fImageArea);

	return MC_S_OK;

}

void NormalMapRenderer::GetTileRenderer(I3DExTileRenderer** tileRenderer)
{
	TMCCountedGetHelper<I3DExTileRenderer> result(tileRenderer);
	result = this;
}

void NormalMapRenderer::Init(const TMCRect& rect,const TBBox2D& uvBox)
{
	fCurrentTileRect = rect;
	fCurrentUVBox	 = uvBox;
}

boolean NormalMapRenderer::GetNextSubTile(TMCRect& rect)
{
	return false;
}

void NormalMapRenderer::RenderSubTile(RTData& pixels)
{
	MCNotify("Bad Call!"); // should never be called
}


// This method allows us to fill the RTData. Then we have our image to the screen
void NormalMapRenderer::FinishRender(const RTData& pixels)
{
	//Be careful a slight difference exists between Camera Coord and Screen Coord. Basically,
	//both are the camera view, the only change is for Camera coord view axis is -z whereas for
	//screen coord view axis is y for further details see DataBase Overview in CookBook
	//For the following, Camera means Screen Coord

	fDrawingArea = fCurrentTileRect;
	BlocCopy(&pixels);
}


// drawing time is over, we have to delete temporary components
MCCOMErr NormalMapRenderer::FinishDraw ()

{
	if (fBuffer.red)	delete [] fBuffer.red;
	if (fBuffer.green)	delete [] fBuffer.green;
	if (fBuffer.blue)	delete [] fBuffer.blue;

	if (fZbuffer!=NULL) delete [] fZbuffer;

	ReleaseMem();

	return MC_S_OK;
}	



// This method is called by the DrawRect call. It copys a tile of our buffer to the RTData

void NormalMapRenderer::BlocCopy(const RTData* pixels)

{
	int32 i,j;

	int32 height=fDrawingArea.bottom-fDrawingArea.top;
	int32 width	=fDrawingArea.right-fDrawingArea.left;

	uint8 *red		= pixels->red;
	uint8 *green	= pixels->green;
	uint8 *blue		= pixels->blue;
	uint8 *alpha	= pixels->alpha;

	int32 columnStep = 1;

	int32 toNextLine = pixels->rowBytes;

	for (j=0; j<height; j++)
	{
		for (i=0; i<width; i++)
		{
			red  [i*columnStep]	= fBuffer.red[i+fDrawingArea.left + (j+fDrawingArea.top)*fImageWidth];
			green[i*columnStep]	= fBuffer.green[i+fDrawingArea.left + (j+fDrawingArea.top)*fImageWidth];
			blue [i*columnStep]	= fBuffer.blue[i+fDrawingArea.left + (j+fDrawingArea.top)*fImageWidth];
		}
		if (alpha)
		{
			for (i=0; i<width; i++)
			{
				alpha[i*columnStep] = 255;
			}
			alpha += toNextLine;
		}
		red		+=toNextLine;
		green	+=toNextLine;
		blue	+=toNextLine;
	}
}


// This method is used to draw the facets in our buffer
void NormalMapRenderer::Rasterize(const TFacet3D& facet, 
								  const TTransform3D& L2G, const TTransform3D& G2L, const TTransform3D& L2C, const TTransform3D& G2C, 
								  I3DShInstance* CurrentInstance )
{	
//	TVector3		screenCoordPoint[3];
	TMCColorRGBA	color, notused;
	RayHit3D		myShadingIn;
	TVector2		screenVertex[3];
	real			zOut;
	int				numVertices = 3;

	myShadingIn.fCalcInfo = NULL;

	//We transform local Vertex to Vertex in Camera view,i.e. in Screen Coord using L2S
	MixTransformPoint(fScreenCoordPoint[0], L2C, facet.fVertices[0].fVertex);
	MixTransformPoint(fScreenCoordPoint[1], L2C, facet.fVertices[1].fVertex);
	MixTransformPoint(fScreenCoordPoint[2], L2C, facet.fVertices[2].fVertex);
	
	
	//We transform vertex in Local to Vertex in Global Coord using L2G
	//Since we use Phong model, we always need normals
	//In that case we need normals and positions in Global coord System
	MixTransformPoint(fGlobalVertex[0].fVertex, L2G, facet.fVertices[0].fVertex);
	MixTransformPoint(fGlobalVertex[1].fVertex, L2G, facet.fVertices[1].fVertex);
	MixTransformPoint(fGlobalVertex[2].fVertex, L2G, facet.fVertices[2].fVertex);
	
	MixTransformVector(fGlobalVertex[0].fNormal, L2C, facet.fVertices[0].fNormal);
	MixTransformVector(fGlobalVertex[1].fNormal, L2C, facet.fVertices[1].fNormal);
	MixTransformVector(fGlobalVertex[2].fNormal, L2C, facet.fVertices[2].fNormal);
	
	//We have to realize that we are supposed to give the right the parameters to the shaders
	//i.e we have to fill a ShadingIn structure in order to get ShadingOut structure. Thus, we can compute the Illumination
	//See Data Stucture Reference in Reference Manual.	
	
	
	myShadingIn.fUVSpaceID = facet.fUVSpace;
	
	//Contrary to others items, we compute IsoU and IsoV for the facet
	//Indeed, others items are computed at the pixel level
	//We only deal with isoU and isoV when the mapping is parametric.
	//If the mapping is projectional, Doshade will compute isoU and isoV
	
	if (( fFlags.fNeedsIsoUV == (uint)true ))
		GetIsoUV (myShadingIn.fIsoU, myShadingIn.fIsoV, facet);
	
	UVSpaceInfo myUVSpaceInfo;
	myShadingIn.fUVInfo = &myUVSpaceInfo;
	
	//We transform Vertex in Screen Coord in Vertex in Screen Space, from 3D to 2D
	boolean result1 = fCamera->Project3DTo2D(&fScreenCoordPoint[0], &screenVertex[0], &zOut);
	boolean result2 = fCamera->Project3DTo2D(&fScreenCoordPoint[1], &screenVertex[1], &zOut);
	boolean result3 = fCamera->Project3DTo2D(&fScreenCoordPoint[2], &screenVertex[2], &zOut);
	
	
	//Before displaying each pixel, we have to apply Zoom transformation, and to center
	fScreenPoint[0][0] = fOffscrOffset[0] + (screenVertex[0][0] * fZoom[0]) ;
	fScreenPoint[0][1] = fOffscrOffset[1] - (screenVertex[0][1] * fZoom[1]) ;
	fScreenPoint[0][2] = -fScreenCoordPoint[0][2]; //We just get the z coordinate ( == zOut: distance to screen )
	
	fScreenPoint[1][0] = fOffscrOffset[0] + (screenVertex[1][0] * fZoom[0]) ;
	fScreenPoint[1][1] = fOffscrOffset[1] - (screenVertex[1][1] * fZoom[1]) ;
	fScreenPoint[1][2] = -fScreenCoordPoint[1][2]; //We just get the z coordinate ( == zOut: distance to screen )
	
	fScreenPoint[2][0] = fOffscrOffset[0] + (screenVertex[2][0] * fZoom[0]) ;
	fScreenPoint[2][1] = fOffscrOffset[1] - (screenVertex[2][1] * fZoom[1]) ;
	fScreenPoint[2][2] = -fScreenCoordPoint[2][2]; //We just get the z coordinate ( == zOut: distance to screen )

//	if ((fScreenPoint[0][0] > 0) && 
//		(fScreenPoint[1][0] > 0) && 
//		(fScreenPoint[2][0] > 0) )
		FillPoly(facet, CurrentInstance, &myShadingIn, numVertices, L2G, G2L, L2C, G2C);
}





void NormalMapRenderer::GetRenderStatistics(I3DRenderStatistics** renderstats)

{

	TMCCountedGetHelper<I3DRenderStatistics> result(renderstats);

	result = static_cast<I3DRenderStatistics *>(NULL);

}





// This method allow us to change the value of a pixel.

void NormalMapRenderer::PixelSet (int32 x, int32 y, TMCColorRGBA* color)

{
	TGray16 red(color->red);
	TGray16 green(color->green);
	TGray16 blue(color->blue);

	if ((x>=0)&&(x<fImageWidth)&&(y>=0)&&(y<fImageHeight))

	{
		fBuffer.red[x + fImageWidth*y] = red.To8();
		fBuffer.green[x + fImageWidth*y] = green.To8();
		fBuffer.blue[x + fImageWidth*y] = blue.To8();
	}
}

void NormalMapRenderer::ZBufferSet (int32 x, int32 y, real val)
{
	if ((x>=0)&&(x<fImageWidth)&&(y>=0)&&(y<fImageHeight))
	{
		fZbuffer[x + fImageWidth*y] = val;
	}
}

//A mere interpolation

void NormalMapRenderer::Interpolate(real A, real B, long y1, long y2, long dy, real *array)

{
	real32 slope = (A - B)/dy;
	real32 x = ( dy < 0 )? x=A : x=B;

	for (int32 j = y1 ; j <= y2 ; j++ )
	{
		if ( j >= 0 && j < fImageHeight) //Clipping y axis in ScreenPixel Space
			array[j] = x; 
		x += slope;
	}
}

// Barycentric (or parametric) coordinates : 
// from http://softsurfer.com/Archive/algorithm_0104/algorithm_0104.htm
// Interpolate the values of vect0,1 and 2 for the point P in the V1,V2,V3 triangle
TVector3 GetVectorFromXY(	const TVector3& vect0,
							const TVector3& vect1,
							const TVector3& vect2,
							const real32 denom,	// denom = v[0]*u[1] - v[1]*u[0]
							const TVector2& u,	// u = V1-V0
							const TVector2& v,	// v = V2-V0
							const TVector2& w)	// w = P -V0
{
	const real32 s = (w[1]*v[0] - w[0]*v[1])/denom;
	const real32 t = (w[0]*u[1] - w[1]*u[0])/denom;

	// Barycentric coordinates are then (1-s-t,s,t)
	return ( (1-s-t)*vect0 + s*vect1 + t*vect2 );
}
TVector2 GetVectorFromXY(	const TVector2& vect0,
							const TVector2& vect1,
							const TVector2& vect2,
							const real32 denom,	// denom = v[0]*u[1] - v[1]*u[0]
							const TVector2& u,	// u = V1-V0
							const TVector2& v,	// v = V2-V0
							const TVector2& w)	// w = P -V0
{
	const real32 s = (w[1]*v[0] - w[0]*v[1])/denom;
	const real32 t = (w[0]*u[1] - w[1]*u[0])/denom;

	// Barycentric coordinates are then (1-s-t,s,t)
	return ( (1-s-t)*vect0 + s*vect1 + t*vect2 );
}

inline TVector2 ProjectIn(TVector3 pos,const TVector3& center,const TVector3& I,const TVector3& J)
{
	pos-=center;
	return TVector2( I*pos, J*pos);
}
#include "Geometry.h"

void NormalMapRenderer::FillPoly(TFacet3D facet, I3DShInstance *CurrentInstance, 
								 RayHit3D *myShadingIn, int numVertices,
								 const TTransform3D& L2G, const TTransform3D& G2L, const TTransform3D& L2C, const TTransform3D& G2C)
{
#if 1 // Barycentric (parametric) coordinates
	
	//Be careful, facets are not oriented, and our polygon-filling needs counterclockwise oriented triangles
	int32 pointOrder[3] = { 0, 1, 2};
	
	TVector3 res = MixNormalFacet(fScreenPoint[0], fScreenPoint[1], fScreenPoint[2]);
	
	if ( res[2] > 0 )
	{
		pointOrder[1] = 2;
		pointOrder[2] = 1;
	}
	
	int32 ymin=IPOSINF, ymax=INEGINF;
	
	TVector3 tgleScreenPoint[3];

	for ( int32 iVtx = 0 ; iVtx < numVertices ; iVtx++ )
	{
		const int32 cur = pointOrder[iVtx];
		const int32 next = pointOrder[(iVtx+1)%numVertices];
	
		const int32 y1 = (int32)(fScreenPoint[cur][1]+0.5);
		const int32 y2 = (int32)(fScreenPoint[next][1]+0.5);
		
		const int32 x1 = (int32)(fScreenPoint[cur][0]+0.5);
		tgleScreenPoint[cur] = fScreenCoordPoint[cur];

		if ( y1 > y2 )
		{
			if ( y2 < ymin ) ymin = y2;
			if ( y1 > ymax ) ymax = y1;
			
			const int32 dy = y2-y1;
			
			Interpolate(fScreenPoint[next][0], fScreenPoint[cur][0], y2, y1, dy, rightX);
			Interpolate(fScreenPoint[next][2], fScreenPoint[cur][2], y2, y1, dy, rightZ);			
		}		
		else if ( y1 < y2 )
		{
			if ( y1 < ymin ) ymin = y1 ;
			if ( y2 > ymax ) ymax = y2 ;
			
			const int32 dy = y2 - y1;
			
			Interpolate(fScreenPoint[next][0], fScreenPoint[cur][0], y1, y2, dy, leftX);
			Interpolate(fScreenPoint[next][2], fScreenPoint[cur][2], y1, y2, dy, leftZ);			
		}
	}
	
	// Clip in height
	if ( ymin < 0 ) ymin = 0;
	if ( ymax > fImageHeight) ymax = fImageHeight-1;

	// Prepare the barycentric interpolation
	// triangle V0, V1, V2 (tglePoint), inside the triangle space
	TVector3 I,J,K,center;
	center = TVector3::kZero; // Good enough for the use
	I = tgleScreenPoint[1] - tgleScreenPoint[0];
	J = tgleScreenPoint[2] - tgleScreenPoint[0];
	if( !I.Normalize() )
		return;
	if( !J.Normalize() )
		return;
	K = I^J; // normal
	J = K^I; // To have an ortho base

	const TVector2 V0 = ProjectIn(tgleScreenPoint[0],tgleScreenPoint[0],I,J);
	const TVector2 u = ProjectIn(tgleScreenPoint[1],tgleScreenPoint[0],I,J) - V0;
	const TVector2 v = ProjectIn(tgleScreenPoint[2],tgleScreenPoint[0],I,J) - V0;
	const real32 denom = v[0]*u[1] - v[1]*u[0];

	TStandardCameraInfo cameraInfo;
	fCamera->GetStandardCameraInfo(cameraInfo);
	const EProjectionType projectionType = cameraInfo.fProjectionType;

	//We fill and interpolate horizontally from left array to right array
	for (int32 y = ymin  ; y <= ymax ; y++ )
	{
		const real32 deltaX = leftX[y] - rightX[y];
		//Slope initialization for each item
		if ( deltaX != 0 )
		{
			//From left array to right array
			const int32 xMin = (int32)(leftX[y]+0.5);
			const int32 xMax = (int32)(rightX[y]+0.5);

			//First value initialization
			real32 zValue = leftZ[y];
							
			const int32 width = (xMax-xMin);
			if(width==0)
				continue;

			const real32 incZ = (leftZ[y]-rightZ[y])/(leftX[y]-rightX[y]);
			
			for ( int32 x = xMin ; x < xMax ; x++ )
			{	
				// Clipping
				if ( x >= 0 && x < fImageWidth )
				{
					const int32 bufferIndex = y*fImageWidth + x;
					
					//Zbuffer test
					if ( zValue < fZbuffer[bufferIndex] )
					{
						// Shading: needed for the bump mapping
						// We do a barycentric interpolation of the value we need
						TVector2 pos;					
						pos[0] = x - fOffscrOffset[0];
						pos[0] /= fZoom[0];
						pos[1] = -y + fOffscrOffset[1];
						pos[1] /= fZoom[1];
						//We get resultDirection giving the view direction for each pixel
						TVector2 screenDerivate(1,1);
						Ray3D ray;
						ray.Init();
						fCamera->CreateRay(pos,screenDerivate,ray);

						Ray3D localRay;
						localRay.Init();
						localRay.fOrigin	= TransformPoint(G2L, ray.fOrigin);
						localRay.fDirection = TransformVector(G2L, ray.fDirection);
				
						RayHitParameters rayHitParam;
						rayHitParam.tmin = 0;
						rayHitParam.tmax = 1e30f;
						rayHitParam.ray = &localRay;
						rayHitParam.hit  = myShadingIn; // RayHit3D
						rayHitParam.originInstanceIndex = -1;
						rayHitParam.originSubIndexIndex = -1;
						rayHitParam.originFacetIndex = -1;

						myShadingIn->ft = 0;
						myShadingIn->fInstance = CurrentInstance;
						myShadingIn->fFacetMesh = CurrentInstance->GetRenderingFacetMesh();

						// Hit the object

						if( CurrentInstance->RayHit( rayHitParam) )
						{
							myShadingIn->fPointLoc		= localRay.fOrigin+localRay.fDirection*myShadingIn->ft;
							myShadingIn->fInstance		= CurrentInstance;
							myShadingIn->fT				= L2G;
							myShadingIn->fInvT			= G2L;
							myShadingIn->fInstanceIndex	= -1;
						}


						// Other elem
						myShadingIn->CalcInfo(fFlags, true, ray );

						// There's a problem somewhere: the derivative values are too big
						const real coeffTest = 0.001f;
						myShadingIn->fPointx*=coeffTest;
						myShadingIn->fPointy*=coeffTest;
						myShadingIn->fPointLocx*=coeffTest;
						myShadingIn->fPointLocy*=coeffTest;


						//We fill ShadingIn structure and we get ShadingOut structure
						ShadingOut myShadingOut;
						myShadingOut.SetDefaultValues();
						myShadingIn->fBumpOn=true; // Get the bump mapping to add the normal
						CurrentInstance->DoShade(myShadingOut,*myShadingIn);

						// The normal is in camera space
						TVector3 normal = TVector3::kUnitZ;
						if(myShadingOut.fChangedNormalLoc.z!=0)
						{
							MixTransformVector(myShadingOut.fChangedNormal, L2C, myShadingOut.fChangedNormalLoc);

							normal = myShadingOut.fChangedNormal; // thePoint.fNormal;
						}
						else
						{	// no bumpmap
							if ( fFlags.fNeedsNormal == (uint)true )
								normal = myShadingIn->fGNormal;
							//else
							//	normal = GetVectorFromXY(fGlobalVertex[0].fNormal,
							//							fGlobalVertex[1].fNormal,
							//							fGlobalVertex[2].fNormal,
							//							denom,u,v,w);
						}

						if(projectionType == kProjectionType_kPerspective)
						{
							// In the perspective camera case, we can use
							// fCameraDir and resultDirection to reorient the normal
							// resultDirection is in camera space => -kUnitZ at the middle
							TVector3 resultOrigin, resultDirection;					
							resultDirection = G2C.TransformVector(ray.fDirection);
							TVector3 axis = resultDirection^TVector3::kNegativeZ;
							axis.Normalize();
							real32 sin=0; real32 cos=1;
							AngleBetweenVectors(resultDirection,TVector3::kNegativeZ,axis, sin, cos);
							TUnitComplex angle;
							angle.SetFromSinCos(sin, cos);
							TUnitQuaternion rot;
							rot.SetFromAxis(axis, angle);

							// now rotate it
							TUnitQuaternion normalQ(normal.x, normal.y, normal.z, 0);
							TUnitQuaternion resultQ = (rot*normalQ)*(rot.Inverse());
							resultQ.GetAxis(normal);
						}

						if (normal[2]<kRealEpsilon)
						{
							// we're looking at a back face
							if(fPMap.fFlipBackFace) // flip it
							{
								normal[0] = -normal[0];
								normal[1] = -normal[1];
								normal[2] = -normal[2];
							}
						//	else
						//		continue;
						}

						//We get color pixel
						TMCColorRGBA color;

						color.red = .5 * (normal[0]+1);
						color.green = .5 * (normal[1]+1);
						color.blue = .5 * (normal[2]+1);

						//We display each pixel
						PixelSet(x,y,&color);
						
						// Update the zbuffer
						fZbuffer[bufferIndex] = zValue;
					}				
				}

				zValue += incZ;
			}
		}	
	}
#else // Bilinear interpolation
	long ymin , ymax;
	long y1,y2, dy;
	ymin = IPOSINF ;
	ymax = INEGINF ;
	
	//Be careful, facets are not oriented, and our polygon-filling needs counterclockwise oriented triangles
	int PointOrder[3] = { 0, 1, 2};
	
	TVector3 res = MixNormalFacet(fScreenPoint[0], fScreenPoint[1], fScreenPoint[2]);
	
	if ( res[2] > 0 )
	{
		PointOrder[1] = 2;
		PointOrder[2] = 1;
	}
	
	//We interpolate along triangle edges, ie we interpolate vertically 
	//We just interpolate items wanted by the RayDream Shader
	//Basically, to interpolate, we use two arrays : one left and one right.
	//If the next vertex (counterclockwisely speaking), is below we use left array
	//else we use right array (See drawing in Cookbook).
	//Roughly speaking, we use bilinear interpolation
	for ( int k = 0 ; k < numVertices ; k++ )
	{
		int i = PointOrder[k];
		y1 = (long)(fScreenPoint[i][1]+0.5);
		int t = PointOrder[(k+1)%numVertices];
		y2 = (long)(fScreenPoint[t][1]+0.5);
		
		if ( y1 > y2 )
		{
			if ( y2 < ymin ) ymin = y2;
			if ( y1 > ymax ) ymax = y1;
			
			dy = y2-y1;
			
			Interpolate(fScreenPoint[t][0], fScreenPoint[i][0], y2, y1, dy, rightX);
			Interpolate(fScreenPoint[t][2], fScreenPoint[i][2], y2, y1, dy, rightZ);
			
			Interpolate(fGlobalVertex[t].fVertex[0], fGlobalVertex[i].fVertex[0], y2, y1, dy, rightGlobalPosX);
			Interpolate(fGlobalVertex[t].fVertex[1], fGlobalVertex[i].fVertex[1], y2, y1, dy, rightGlobalPosY);
			Interpolate(fGlobalVertex[t].fVertex[2], fGlobalVertex[i].fVertex[2], y2, y1, dy, rightGlobalPosZ);
			
			Interpolate(fGlobalVertex[t].fNormal[0], fGlobalVertex[i].fNormal[0], y2, y1, dy, rightGlobalNorX);
			Interpolate(fGlobalVertex[t].fNormal[1], fGlobalVertex[i].fNormal[1], y2, y1, dy, rightGlobalNorY);
			Interpolate(fGlobalVertex[t].fNormal[2], fGlobalVertex[i].fNormal[2], y2, y1, dy, rightGlobalNorZ);
			
			if ( fFlags.fNeedsUV == (uint)true )
			{
				Interpolate(facet.fVertices[t].fUV[0], facet.fVertices[i].fUV[0], y2, y1, dy, rightU);
				Interpolate(facet.fVertices[t].fUV[1], facet.fVertices[i].fUV[1], y2, y1, dy, rightV);
			}
			
			if ( fFlags.fNeedsPointLoc == (uint)true )
			{
				
				Interpolate(facet.fVertices[t].fVertex[0], facet.fVertices[i].fVertex[0], y2, y1, dy, rightLocPosX);
				Interpolate(facet.fVertices[t].fVertex[1], facet.fVertices[i].fVertex[1], y2, y1, dy, rightLocPosY);
				Interpolate(facet.fVertices[t].fVertex[2], facet.fVertices[i].fVertex[2], y2, y1, dy, rightLocPosZ);
			}
			
			if ( fFlags.fNeedsNormalLoc == (uint)true )
			{
				Interpolate(facet.fVertices[t].fNormal[0], facet.fVertices[i].fNormal[0], y2, y1, dy, rightLocNorX);
				Interpolate(facet.fVertices[t].fNormal[1], facet.fVertices[i].fNormal[1], y2, y1, dy, rightLocNorY);
				Interpolate(facet.fVertices[t].fNormal[2], facet.fVertices[i].fNormal[2], y2, y1, dy, rightLocNorZ);
			}		
		}
		
		else if ( y1 < y2 )
		{
			if ( y1 < ymin ) ymin = y1 ;
			if ( y2 > ymax ) ymax = y2 ;
			
			dy = y2 - y1;
			
			Interpolate(fScreenPoint[t][0], fScreenPoint[i][0], y1, y2, dy, leftX);
			Interpolate(fScreenPoint[t][2], fScreenPoint[i][2], y1, y2, dy, leftZ);
			
			Interpolate(fGlobalVertex[t].fVertex[0], fGlobalVertex[i].fVertex[0], y1, y2, dy, leftGlobalPosX);
			Interpolate(fGlobalVertex[t].fVertex[1], fGlobalVertex[i].fVertex[1], y1, y2, dy, leftGlobalPosY);
			Interpolate(fGlobalVertex[t].fVertex[2], fGlobalVertex[i].fVertex[2], y1, y2, dy, leftGlobalPosZ);
			
			Interpolate(fGlobalVertex[t].fNormal[0], fGlobalVertex[i].fNormal[0], y1, y2, dy, leftGlobalNorX);
			Interpolate(fGlobalVertex[t].fNormal[1], fGlobalVertex[i].fNormal[1], y1, y2, dy, leftGlobalNorY);
			Interpolate(fGlobalVertex[t].fNormal[2], fGlobalVertex[i].fNormal[2], y1, y2, dy, leftGlobalNorZ);
			
			
			if ( fFlags.fNeedsUV == (uint)true )
			{
				Interpolate(facet.fVertices[t].fUV[0], facet.fVertices[i].fUV[0], y1, y2, dy, leftU);
				Interpolate(facet.fVertices[t].fUV[1], facet.fVertices[i].fUV[1], y1, y2, dy, leftV);
			}
			
			if ( fFlags.fNeedsPointLoc == (uint)true )
			{
				
				Interpolate(facet.fVertices[t].fVertex[0], facet.fVertices[i].fVertex[0], y1, y2, dy, leftLocPosX);
				Interpolate(facet.fVertices[t].fVertex[1], facet.fVertices[i].fVertex[1], y1, y2, dy, leftLocPosY);
				Interpolate(facet.fVertices[t].fVertex[2], facet.fVertices[i].fVertex[2], y1, y2, dy, leftLocPosZ);
			}
			
			if ( fFlags.fNeedsNormalLoc == (uint)true )
			{
				Interpolate(facet.fVertices[t].fNormal[0], facet.fVertices[i].fNormal[0], y1, y2, dy, leftLocNorX);
				Interpolate(facet.fVertices[t].fNormal[1], facet.fVertices[i].fNormal[1], y1, y2, dy, leftLocNorY);
				Interpolate(facet.fVertices[t].fNormal[2], facet.fVertices[i].fNormal[2], y1, y2, dy, leftLocNorZ);
			}
			
		}
	}
	
	//n-gones filling
	
	real	delta;
	real	z;
	real	deltaZ, deltaGlobalPosX, deltaGlobalPosY, deltaGlobalPosZ, 
		deltaGlobalNorX, deltaGlobalNorY, deltaGlobalNorZ,
		deltaLocPosX, deltaLocPosY, deltaLocPosZ,
		deltaLocNorX, deltaLocNorY, deltaLocNorZ,
		deltaU, deltaV;
	
	real	GlobalPosX, GlobalPosY, GlobalPosZ,
		GlobalNorX, GlobalNorY, GlobalNorZ,
		LocPosX, LocPosY, LocPosZ,
		LocNorX, LocNorY, LocNorZ,
		U,V;
	
	
	TVector2 uv;
	
	if ( ymin < 0 ) ymin = 0;
	if ( ymax > fImageHeight) ymax = fImageHeight-1;
	
	//We fill and interpolate horizontally from left array to right array
	for (long y = ymin  ; y <= ymax ; y++ )
	{
		
		//Slope initialization for each item
		if ( (delta = leftX[y] - rightX[y]) == 0 )
		{
			deltaZ = 0;
			deltaGlobalPosX = 0; deltaGlobalPosY = 0; deltaGlobalPosZ = 0;
			deltaGlobalNorX = 0; deltaGlobalNorY = 0; deltaGlobalNorZ = 0;
			deltaU = 0; deltaV = 0;
			deltaLocPosX = 0; deltaLocPosY = 0; deltaLocPosZ = 0;
			deltaLocNorX = 0; deltaLocNorY = 0; deltaLocNorZ = 0;
		}
		
		else
		{
			deltaZ = (leftZ[y]-rightZ[y])/delta;
			deltaGlobalPosX = (leftGlobalPosX[y]-rightGlobalPosX[y])/delta;
			deltaGlobalPosY = (leftGlobalPosY[y]-rightGlobalPosY[y])/delta;
			deltaGlobalPosZ = (leftGlobalPosZ[y]-rightGlobalPosZ[y])/delta;
			
			deltaGlobalNorX = (leftGlobalNorX[y]-rightGlobalNorX[y])/delta;
			deltaGlobalNorY = (leftGlobalNorY[y]-rightGlobalNorY[y])/delta;
			deltaGlobalNorZ = (leftGlobalNorZ[y]-rightGlobalNorZ[y])/delta;
			
			if ( fFlags.fNeedsUV == (uint)true )
			{
				deltaU = (leftU[y]-rightU[y])/delta;
				deltaV = (leftV[y]-rightV[y])/delta;
			}
			
			if ( fFlags.fNeedsPointLoc == (uint)true )
			{
				deltaLocPosX = (leftLocPosX[y]-rightLocPosX[y])/delta;
				deltaLocPosY = (leftLocPosY[y]-rightLocPosY[y])/delta;
				deltaLocPosZ = (leftLocPosZ[y]-rightLocPosZ[y])/delta;
			}
			
			if ( fFlags.fNeedsNormalLoc == (uint)true )
			{
				deltaLocNorX = (leftLocNorX[y]-rightLocNorX[y])/delta;
				deltaLocNorY = (leftLocNorY[y]-rightLocNorY[y])/delta;
				deltaLocNorZ = (leftLocNorZ[y]-rightLocNorZ[y])/delta;
			}
		}
		
		//First value initialization
		z = leftZ[y];
		
		GlobalPosX = leftGlobalPosX[y];
		GlobalPosY = leftGlobalPosY[y];
		GlobalPosZ = leftGlobalPosZ[y];
		
		GlobalNorX = leftGlobalNorX[y];
		GlobalNorY = leftGlobalNorY[y];
		GlobalNorZ = leftGlobalNorZ[y];
		
		if ( fFlags.fNeedsPointLoc == (uint)true )
		{
			LocPosX = leftLocPosX[y];
			LocPosY = leftLocPosY[y];
			LocPosZ = leftLocPosZ[y];
		}
		if ( fFlags.fNeedsNormalLoc == (uint)true )
		{
			LocNorX = leftLocNorX[y];
			LocNorY = leftLocNorY[y];
			LocNorZ = leftLocNorZ[y];
		}
		if ( fFlags.fNeedsUV == (uint)true )
		{
			U = leftU[y];
			V = leftV[y];
		}
		
		//From left array to right array
		for ( int x = (int)(leftX[y]+0.5) ; x < (int)(rightX[y]+0.5) ; x++ )
		{	
			
			if ( x >= 0 && x < fImageWidth )
			{	//Clipping in x-axis
				
				int ind = y*fImageWidth + x;
				
				//Zbuffer test
				if ( z < fZbuffer[ind] )
				{
					fZbuffer[ind] = z;
					
					if ( fFlags.fNeedsPoint == (uint)true )
					{
						myShadingIn->fPoint[0] = GlobalPosX;
						myShadingIn->fPoint[1] = GlobalPosY;
						myShadingIn->fPoint[2] = GlobalPosZ;
					}
					
					if ( fFlags.fNeedsNormal == (uint)true )
					{
						myShadingIn->fGNormal[0] = GlobalNorX;
						myShadingIn->fGNormal[1] = GlobalNorY;
						myShadingIn->fGNormal[2] = GlobalNorZ;
					}
					
					if ( fFlags.fNeedsUV == (uint)true )
					{
						uv[0] = U;
						uv[1] = V;
						myShadingIn->fUV = uv;
					}
					
					if ( fFlags.fNeedsPointLoc == (uint)true )
					{
						myShadingIn->fPointLoc[0] = LocPosX;
						myShadingIn->fPointLoc[1] = LocPosY;
						myShadingIn->fPointLoc[2] = LocPosZ;
					}
					
					if ( fFlags.fNeedsNormalLoc == (uint)true )
					{
						myShadingIn->fNormalLoc[0] = LocNorX;
						myShadingIn->fNormalLoc[1] = LocNorY;
						myShadingIn->fNormalLoc[2] = LocNorZ;
					}
					//We fill ShadingIn structure and we get ShadingOut structure
					ShadingOut myShadingOut;
					myShadingIn->fBumpOn=true; // Get the bump mapping to add the normal
					CurrentInstance->DoShade(myShadingOut,*myShadingIn);

					TVertex3D thePoint;
					thePoint.fVertex[0] = GlobalPosX;
					thePoint.fVertex[1] = GlobalPosY;
					thePoint.fVertex[2] = GlobalPosZ;

					thePoint.fNormal[0] = GlobalNorX;
					thePoint.fNormal[1] = GlobalNorY;
					thePoint.fNormal[2] = GlobalNorZ;

					
					TVector2 pos;
					
					pos[0] = x - fOffscrOffset[0];
					pos[0] /= fZoom[0];
					pos[1] = -y + fOffscrOffset[1];
					pos[1] /= fZoom[1];
					
					TVector3 resultOrigin, resultDirection;
					
					//We get resultDirection giving the view direction for each pixel
					fCamera->CreateRay(&pos, &resultOrigin, &resultDirection);
					
						// The normal is in camera space
						TVector3 normal = TVector3::kUnitZ;
						if(myShadingOut.fChangedNormalLoc.z!=0)
						{
							MixTransformVector(myShadingOut.fChangedNormal, L2C, myShadingOut.fChangedNormalLoc);

							normal = myShadingOut.fChangedNormal; // thePoint.fNormal;
						}
						else
						{	// no bumpmap
							if ( fFlags.fNeedsNormal == (uint)true )
								normal = myShadingIn->fGNormal;
							else
							{
								normal.x = GlobalNorX;
								normal.y = GlobalNorY;
								normal.z = GlobalNorZ;
							}
						}
					if (normal[2]<kRealEpsilon)
					{ // we're looking at a back face, flip it
						normal[0] = -normal[0];
						normal[1] = -normal[1];
						normal[2] = -normal[2];
					}

					// Add the bump
				//	normal[0] = -normal[0];
				//	normal[1] = -normal[1];
				//	normal[2] = -normal[2];

					//We get color pixel
					TMCColorRGBA color;

					color.red = .5 * (normal[0]+1);
					color.green = .5 * (normal[1]+1);
					color.blue = .5 * (normal[2]+1);

				/*	color.R = 10000;
					color.G = 00;
					color.B = 00;
					PixelColor(&myShadingOut, thePoint, fFlags.fChangesNormal,
						resultDirection, resultOrigin, color);
				*/	
					//We display each pixel
					PixelSet(x,y,&color);
				}				
			}

			z += deltaZ;
			
			GlobalPosX += deltaGlobalPosX;
			GlobalPosY += deltaGlobalPosY;
			GlobalPosZ += deltaGlobalPosZ;
			
			GlobalNorX += deltaGlobalNorX;
			GlobalNorY += deltaGlobalNorY;
			GlobalNorZ += deltaGlobalNorZ;
			
			LocPosX += deltaLocPosX;
			LocPosY += deltaLocPosY;
			LocPosZ += deltaLocPosZ;
			
			LocNorX += deltaLocNorX;
			LocNorY += deltaLocNorY;
			LocNorZ += deltaLocNorZ;
			U += deltaU;
			V += deltaV;
		}	
	}
#endif
}

/*
void NormalMapRenderer::PixelColor(const ShadingOut *theShadingOut, const TVertex3D thePoint, const boolean changedNormals,
							  const TVector3 eyeDirection, const TVector3 eyePosition, TMCColorRGBA& resultColor)
{
	TMCColorRGBA tempColor;
	TVector3 newNormal;
	
	resultColor.R = theShadingOut->fColor.R * fAmbientColor.R * theShadingOut->fAmbient + theShadingOut->fGlow.R;
	resultColor.G = theShadingOut->fColor.G * fAmbientColor.G * theShadingOut->fAmbient + theShadingOut->fGlow.G;
	resultColor.B = theShadingOut->fColor.B * fAmbientColor.B * theShadingOut->fAmbient + theShadingOut->fGlow.B;
	
	tempColor.R = theShadingOut->fColor.R * theShadingOut->fLambert;
	tempColor.G = theShadingOut->fColor.G * theShadingOut->fLambert;
	tempColor.B = theShadingOut->fColor.B * theShadingOut->fLambert;
	
	if (changedNormals)
	{
		newNormal[0] = theShadingOut->fChangedNormal[0];
		newNormal[1] = theShadingOut->fChangedNormal[1];
		newNormal[2] = theShadingOut->fChangedNormal[2];
	}
	else
	{
		newNormal[0] = thePoint.fNormal[0];
		newNormal[1] = thePoint.fNormal[1];
		newNormal[2] = thePoint.fNormal[2];
	}
	
	TVector3	reflectedLightDir;
	real		specAdjust = 48.0;	//48 gives a good reflective value
	real		specularCoeff;
	
	for (int index=0; index < fNbLights; index++)
	{
		TVector3		lightDirection;
		real			distanceToLight=10.0;
		real			lightNormalDP;
		TMCColorRGBA	lightColor;
		real			ShadowIntensity;
		int truc;
		

		truc=fLights[index]->GetDirection(thePoint.fVertex, lightDirection, distanceToLight);
		if (!truc)
			continue;

		truc=fLights[index]->GetColor(thePoint.fVertex, lightDirection, distanceToLight, lightColor, ShadowIntensity);
		if (!truc)
			continue;

		lightNormalDP = lightDirection*newNormal;

		//We must care for a possible atmospheric shader. The color of a lightsource may be modified when
		//getting through the atmosphere
		if (fAtmosphere)
		{
			TVector3	lightPos;
			real norm = sqrt(lightDirection[0]*lightDirection[0] + lightDirection[1]*lightDirection[1]
				+ lightDirection[2]*lightDirection[2]);
			real ratio = distanceToLight / norm;
			lightPos[0] = thePoint.fVertex[0] + lightDirection[0]*ratio;
			lightPos[1] = thePoint.fVertex[1] + lightDirection[1]*ratio;
			lightPos[2] = thePoint.fVertex[2] + lightDirection[2]*ratio;
			fAtmosphere->SegmentFilter(thePoint.fVertex, lightPos, lightColor,true,false);
		}

		//Diffuse color

		resultColor.R += tempColor.R * lightNormalDP * lightColor.R;
		resultColor.G += tempColor.G * lightNormalDP * lightColor.G;
		resultColor.B += tempColor.B * lightNormalDP * lightColor.B;

		

		//This is one method to calculate the reflected light direction

		lightNormalDP *= 2.0;

		reflectedLightDir[0] = -lightDirection[0] + lightNormalDP*newNormal[0];

		reflectedLightDir[1] = -lightDirection[1] + lightNormalDP*newNormal[1];

		reflectedLightDir[2] = -lightDirection[2] + lightNormalDP*newNormal[2];

		

		//Specular color

		real eyeReflectedLightDP = -eyeDirection[0] * reflectedLightDir[0] - eyeDirection[1] * reflectedLightDir[1] - eyeDirection[2] * reflectedLightDir[2];

		

		if (eyeReflectedLightDP > 0.0)

		{

			specularCoeff = (real)pow(eyeReflectedLightDP,theShadingOut->fSpecularSize*specAdjust);

			

			if (specularCoeff <= 0.0)

				continue;

			if (specularCoeff > 1.0)

				specularCoeff = 1.0;

			

			specularCoeff *= 2;		//Better specular highlights

			

			resultColor.R += theShadingOut->fSpecularColor.R * lightColor.R * specularCoeff;

			resultColor.G += theShadingOut->fSpecularColor.G * lightColor.G * specularCoeff;

			resultColor.B += theShadingOut->fSpecularColor.B * lightColor.B * specularCoeff;

			

		}

	}

	

	if (resultColor.R > 1.0)

		resultColor.R = 1.0;

	else if (resultColor.R < 0.0)

		resultColor.R = 0.0;	

	if (resultColor.G > 1.0)

		resultColor.G = 1.0;

	else if (resultColor.G < 0.0)

		resultColor.G = 0.0;	

	if (resultColor.B > 1.0)

		resultColor.B = 1.0;

	else if (resultColor.B < 0.0)

		resultColor.B = 0.0;	

	if (fAtmosphere)
	{
		fAtmosphere->SegmentFilter(eyePosition,thePoint.fVertex,resultColor,false,true);
	}
}
*/


//Used to fill pixels of infinite depth in Zbuffer

void NormalMapRenderer::BackgroundColor(TVector2 theScreenUV, TMCColorRGBA& resultColor)
{
	TVector2	UVPoint;
	TVector3	rayOrigin, rayDirection;

	resultColor.R = (real)0.5f;
	resultColor.G = (real)0.5f;
	resultColor.B = (real)1.0f;

	/*
	UVPoint[0] = theScreenUV[0]-fOffscrOffset[0];
	UVPoint[0] /= fZoom[0];
	UVPoint[1] = -theScreenUV[1]+fOffscrOffset[1];
	UVPoint[1] /= fZoom[1];

	fCamera->CreateRay(&UVPoint, &rayOrigin, &rayDirection);

	boolean fullAreaDone=false;

	//If there is no backdrop but a background, it appears behind the scene
	if (fBackground)
	{
		fBackground->GetEnvironmentColor(rayDirection,fullAreaDone,resultColor);
	}



	//Overrides the background
	if (fBackdrop)
	{
		fBackdrop->GetBackdropColor(UVPoint, fullAreaDone,fUVMinMax, resultColor);
	}

	if (fAtmosphere)
	{
		fAtmosphere->DirectionFilter(rayOrigin,rayDirection,resultColor,true);
	}
	*/
}
/****************************************************************************************************

		BakingCommand.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/30/2004

****************************************************************************************************/

#ifndef __BakingCommand__
#define __BakingCommand__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BakerDef.h"

#include "Basic3DCOMImplementations.h"
#include "BitField.h"
#include "I3DShFacetMesh.h"
#include "I3DShScene.h"
#include "I3DExModifier.h"
#include "ISceneSelection.h"
#include "IChangeManagement.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "MCCountedPtrArray.h"
#include "MCCountedPtr.h"
#include "IShComponent.h" // for IShParameterComponent
#include "IShRasterLayer.h"
#include "IShChannel.h"
#include "I3DImageDocument.h"
#include "UV2XYZ.h"
#include "I3dExLight.h"
#include "I3dExGel.h"
#include "I3dExFinalRenderer.h"

#include "Copyright.h"// From Carrara 5 build
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "ShaderTypes.h"
#include "UVMaps.h"
#endif

struct IShRasterLayer;
struct I3DShPrimitive;


// define the SceneOp CLSID

extern const MCGUID CLSID_BakingCommand;

enum EMapType
{
	eShadingMap =			'shad',
	eLightMap =				'ligh',
	eNormalMap =			'norm',
};

static const int32 kChannelCount = 8;

enum ShadingChannel
{
	eNoChannel				= 0x00000000,
	eDiffuseChannel			= 0x00000001,
	eHighlightChannel		= 0x00000002,
	eShininessChannel		= 0x00000004,
	eBumpChannel			= 0x00000008,
	eReflectionChannel		= 0x00000010, // 16
	eTransparencyChannel	= 0x00000020, // 32
	eRefractionChannel		= 0x00000040, // 64
	eGlowChannel			= 0x00000080, // 128
};

struct BakingPMap
{
	BakingPMap();

	int32			fMapWidth;
	int32			fMapHeight;

	EMapType		fMapType; // Shading, Light or Normal
	boolean			fDiffChannel;
	boolean			fHighChannel;
	boolean			fShinChannel;
	boolean			fBumpChannel;
	boolean			fReflChannel;
	boolean			fTranChannel;
	boolean			fRefrChannel;
	boolean			fGlowChannel;

	boolean			fUseCamera; // Lightmap option: use camera point of view

	int32			fNormalType; // Classic or ZBrush
	int32			fShadingDomainTreatment; // 0: merge all, 1: spli, 3 and next: ids+3

	TMCColorRGBA	fBackgroundColor;

	int32			fSpace;

	boolean			fAutoSave;
	TMCDynamicString	fFolderName;
	boolean			fOpenAfterSave;

	boolean		MergeDomain(){return(fShadingDomainTreatment==0);}
	boolean		SplitDomain(){return(fShadingDomainTreatment==1);}
	boolean		OneDomain(){return(fShadingDomainTreatment>2);}
	int32		DomainID(){return fShadingDomainTreatment<3?0:fShadingDomainTreatment-3;}

	boolean		IsLocalSpace(){return (fSpace=='Loca');}
	boolean		IsGlobalSpace(){return (fSpace=='Glob');}
};
 
// SceneOp Object :
class BakingCommand : public TBasicSceneCommand
{
	public :  
		BakingCommand();
		virtual ~BakingCommand();

		STANDARD_RELEASE;

		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer	(){ return &fPMap; }
		virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(BakingPMap);}
		virtual MCCOMErr MCCOMAPI ExtensionDataChanged		();
		virtual int16	 MCCOMAPI GetResID					(){ return 850; }

		virtual void	 MCCOMAPI GetMenuCallBack			(ISelfPrepareMenuCallBack** callBack);
		virtual MCCOMErr MCCOMAPI Init						(ISceneDocument* sceneDocument);
		virtual MCCOMErr MCCOMAPI Prepare					();

		virtual boolean  MCCOMAPI CanUndo					(){return false;}
		virtual boolean  MCCOMAPI Do						();

	private :

		void GetSelectedTrees();
		void Validate();

		void BakeMap(	I3DShTreeElement*			tree,
						I3DShInstance*				instance,
						I3DShPrimitive*				primitive,
						I3DShMasterShader*			masterShader);

		boolean PrepareShadingPixel(uint32					ii,
							uint32					ij,
							const ShadingFlags &	shadingFlags, 
							UVMaps &				maps, 
							const TMCRealRect &		uvRect,
							ShadingIn &				shadingIn,
							UV2XYZ&					mapToPoint,
							const UVSpaceInfo&		uvSpaceInfo,
							const TTransform3D&		G2L) const ;
		
		void GetBumpShader(I3DShShader* shader, I3DShShader** bumpShader);

		void GetRealLighting(	TMCColorRGBA&	lighting, 
							RayHit3D&		shadingIn,
							ShadingOut& shadingOut,
							I3DShInstance*	instance,
							TTransform3D& L2G,
							TTransform3D& G2L);

		// Data

		BakingPMap		fPMap;

		boolean			fIsValid;
		boolean			fGetShader;
		boolean			fGetLights;
		boolean			fGetNormal;
		boolean			fDoShading;
		int32			fShadingChannel; // a translation of fPMap.fMapChannel
		real32			fPrecision; // for the lighting rayhit

		TMCCountedPtr<ISceneDocument>	fSceneDocument;
		TMCCountedPtr<I3DShScene>		fScene;
		TMCCountedPtr<ISceneSelection>	fSelection;
		TMCCountedPtr<IChangeChannel>	fSelectionChannel;
		TMCCountedPtr<IChangeChannel>	fTreePropertyChannel;

		TMCCountedPtrArray<I3DShTreeElement> fSelectedTrees;

		// Bake lightning 
		void InitRenderer();
		void EndRenderer();
		TVector3 fCameraPos;
		TMCCountedPtr< I3DShCamera>  fRenderingCamera;
		TMCCountedPtr<I3DExFinalRenderer> fRenderer;
		TMCCountedPtr<I3DExRaytracer> fRaytracer;

};

// we first allocate the buckets, then fill them,
// then feed them to the rasterLayer
class BufferData
{
public:
	BufferData(){fIsInitialized=false;}

	void AllocData(const int32 w, const int32 h, const int32 layerCount, const TMCString&		name);
	void MoveImageFromBucketToRasterLayer();

	void SaveIn( const TMCString& folder, bool close );

	boolean fIsInitialized;
	int32	fLayerCount; // Can be 3 or 4

	TMCCountedPtr<I3DImageDocument> fImageDocument;
	TMCCountedPtr<IShRasterLayer> fRasterLayer;
			
	TChannelDataBucket fPlanarBuckets[4]; // its our TMCPixelBucket

protected:
	void GetNewRasterLayer(IShRasterLayer**	rasterLayer, 
						  const int32		layerCount,
							const int32 w, const int32 h,
						  const TMCString&		name);

};

#endif

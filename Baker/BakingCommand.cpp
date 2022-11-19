/****************************************************************************************************

		BakingCommand.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/30/2004

****************************************************************************************************/

#include "BakingCommand.h"

#include "copyright.h"
#include "Baker.h"
#include "UV2XYZ.h"
#include "I3DShGroup.h"
#include "COMUtilities.h"
#include "COM3DUtilities.h"
#include "I3DImageDocument.h"
#include "I3dShUtilities.h"
#include "I3DShCamera.h"
#include "I3DShShader.h"
#include "IShRasterLayer.h"
#include "I3dShGel.h"
#include "UVMaps.h"
#include "I3dShObject.h"
#include "I3DShTreeElement.h"
#include "I3DShLightsource.h"
#include "I3dShEnvironment.h"
#include "IMFDocument.h"
#include "MiscComUtilsImpl.h"
#include "ShaderDefs.h"
#include "ShaderTypes.h"
#include "Utils.h"
#include "Geometry.h"

#include "I3DRenderingModule.h"
#include "I3dExRenderFeature.h"
#include "RenderTypes.h"
#include "BakingCommandPrefs.h"

#include <map>

#if WIN32
#include <assert.h>
#define MY_ASSERT(e)			( assert(e) );
#else // assert crashes the application on Mac
#define MY_ASSERT(e)
#endif

#include "IShPartUtilities.h"

const MCGUID CLSID_BakingCommand(R_CLSID_BakingCommand);

const int32 kLayerCount = 4;

#define kSpecularPowerCoef 4096.0f

// PMap
BakingPMap::BakingPMap()
{
	if(!IsSerialValid())
	{
		fMapWidth=128;
		fMapHeight=128;
	}
	else
	{
		fMapWidth=256;
		fMapHeight=256;
	}

	fMapType=eShadingMap;
	fDiffChannel = true;
	fHighChannel = false;
	fShinChannel = false;
	fBumpChannel = false;
	fReflChannel = false;
	fTranChannel = false;
	fRefrChannel = false;
	fGlowChannel = false;

	fUseCamera = false;

	fNormalType = 'Typ1';

	fShadingDomainTreatment = 0;

	fBackgroundColor = TMCColorRGBA::kWhiteNoAlpha;

	fSpace = 'Loca';

	fAutoSave = false;
	fFolderName = "Choose a folder...";
	fOpenAfterSave = true;
}

// Callback to prepare the menus

class BakingCommandCallBack : public TBasicMenuCallBack
{
public:
	virtual boolean MCCOMAPI SelfPrepareMenu(ISceneDocument* sceneDocument);
};

boolean  BakingCommandCallBack::SelfPrepareMenu(ISceneDocument* sceneDocument)
{
	if (!sceneDocument)
		return false;

	TMCCountedPtr<ISceneSelection> selection;
	sceneDocument->GetSceneSelection(&selection);

	if (!selection)
		return false;

	const int32 leavesCount = selection->GetObjectCount();

	for (int32 leafIndex= 0; leafIndex < leavesCount; leafIndex++)
	{
		ISelectableObject* leafObject = selection->GetSelectableObjectByIndex(leafIndex);

		if (MCVerify(leafObject))
		{
			TMCCountedPtr<I3DShTreeElement> tree;
			leafObject->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
			
			if (tree)
			{
				return true;
			}
		}
	}

	return false;
}

// Scene command

BakingCommand::BakingCommand()
{
	// Data from BasicCommand
	fNeedsSelection = true;

	fIsValid = false;

	fDoShading = true;
	fGetShader = true;
	fGetLights = false;
	fGetNormal = false;

	fPrecision = kRealBig;
}
  
BakingCommand::~BakingCommand()
{
}

MCCOMErr BakingCommand::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;
	
	fIsValid = false;

	return result;
}


void BakingCommand::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
	TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);

	result = new BakingCommandCallBack;
}

MCCOMErr BakingCommand::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;

	if (fSceneDocument)
	{
		fSceneDocument->GetSceneSelection(&fSelection);

		fSceneDocument->GetScene(&fScene);

		fScene->GetTreePropertyChangeChannel(&fTreePropertyChannel);
		ThrowIfNil(fTreePropertyChannel);

		fSceneDocument->GetSceneSelectionChannel(&fSelectionChannel);
		ThrowIfNil(fSelectionChannel);
	}

	return MC_S_OK;
}

MCCOMErr BakingCommand::Prepare()
{
	if(!IsSerialValid())
	{	// Invalid serial number: tell the user it's a demo version
		CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'Baki');
		TMCDynamicString message;
		gResourceUtilities->GetIndString( message, kOtherNames, 6);
		gPartUtilities->Alert(message);
	}

	// Get the preferences
	IShParameterComponent* myPrefsComp= GetMyPrefsComponent();
	if (MCVerify(myPrefsComp))
	{
		PrefsPMap prefs;
		myPrefsComp->GetParameter('MapW', &prefs.fMapWidth);
		myPrefsComp->GetParameter('MapH', &prefs.fMapHeight);
		myPrefsComp->GetParameter('MapT', &prefs.fMapType);
		myPrefsComp->GetParameter('Diff', &prefs.fDiffChannel);
		myPrefsComp->GetParameter('High', &prefs.fHighChannel);
		myPrefsComp->GetParameter('Shin', &prefs.fShinChannel);
		myPrefsComp->GetParameter('Bump', &prefs.fBumpChannel);
		myPrefsComp->GetParameter('Refl', &prefs.fReflChannel);
		myPrefsComp->GetParameter('Tran', &prefs.fTranChannel);
		myPrefsComp->GetParameter('Refr', &prefs.fRefrChannel);
		myPrefsComp->GetParameter('Glow', &prefs.fGlowChannel);
		myPrefsComp->GetParameter('UCam', &prefs.fUseCamera);
		myPrefsComp->GetParameter('Type', &prefs.fNormalType);
		myPrefsComp->GetParameter('Doma', &prefs.fShadingDomainTreatment);
		myPrefsComp->GetParameter('BCol', &prefs.fBackgroundColor);
		myPrefsComp->GetParameter('Spac', &prefs.fSpace);
		myPrefsComp->GetParameter('Save', &prefs.fAutoSave);
		myPrefsComp->GetParameter('Fold', &prefs.fFolderName);
		myPrefsComp->GetParameter('Open', &prefs.fOpenAfterSave);

		fPMap.fMapWidth = prefs.fMapWidth;
		fPMap.fMapHeight = prefs.fMapHeight;
		fPMap.fMapType = prefs.fMapType;
		fPMap.fDiffChannel = prefs.fDiffChannel;
		fPMap.fHighChannel = prefs.fHighChannel;
		fPMap.fShinChannel = prefs.fShinChannel;
		fPMap.fBumpChannel = prefs.fBumpChannel;
		fPMap.fReflChannel = prefs.fReflChannel;
		fPMap.fTranChannel = prefs.fTranChannel;
		fPMap.fRefrChannel = prefs.fRefrChannel;
		fPMap.fGlowChannel = prefs.fGlowChannel;
		fPMap.fUseCamera = prefs.fUseCamera;
		fPMap.fNormalType = prefs.fNormalType; 
		fPMap.fShadingDomainTreatment = prefs.fShadingDomainTreatment;
		fPMap.fBackgroundColor = prefs.fBackgroundColor;
		fPMap.fSpace = prefs.fSpace;
		fPMap.fAutoSave = prefs.fAutoSave;
		fPMap.fFolderName = prefs.fFolderName;
		fPMap.fOpenAfterSave = prefs.fOpenAfterSave;
	}

	// Get a list of the selected instances
	fSelectedTrees.ArrayFree();

	const int32 leavesCount = fSelection->GetObjectCount();

	for (int32 leafIndex= 0; leafIndex < leavesCount; leafIndex++)
	{
		ISelectableObject* leafObject = fSelection->GetSelectableObjectByIndex(leafIndex);

		if (MCVerify(leafObject))
		{
			TMCCountedPtr<I3DShTreeElement> tree;
			leafObject->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
			
			if (tree)
			{
				fSelectedTrees.AddElem(tree);
			}
		}
	}

	return MC_S_OK;
}

void BakingCommand::Validate()
{
	if(!fIsValid)
	{
		fIsValid = true;

		// Translate the PMap data
		fShadingChannel = eNoChannel;

		if(fPMap.fDiffChannel) fShadingChannel|=eDiffuseChannel;
		if(fPMap.fHighChannel) fShadingChannel|=eHighlightChannel;
		if(fPMap.fShinChannel) fShadingChannel|=eShininessChannel;
		if(fPMap.fBumpChannel) fShadingChannel|=eBumpChannel;
		if(fPMap.fReflChannel) fShadingChannel|=eReflectionChannel;
		if(fPMap.fTranChannel) fShadingChannel|=eTransparencyChannel;
		if(fPMap.fRefrChannel) fShadingChannel|=eRefractionChannel;
		if(fPMap.fGlowChannel) fShadingChannel|=eGlowChannel;

		fDoShading = (fShadingChannel != eNoChannel);
		fGetShader = (fPMap.fMapType == eShadingMap);
		fGetLights = (fPMap.fMapType == eLightMap);
		fGetNormal = (fPMap.fMapType == eNormalMap);
	}
	if(!IsSerialValid())
	{
		if(fPMap.fMapWidth>128 || fPMap.fMapHeight>128)
		{	// demo version: size is limited to 128
			CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'Baki');
			TMCDynamicString message;
			gResourceUtilities->GetIndString( message, kOtherNames, 9);
			gPartUtilities->Alert(message);
			fPMap.fMapWidth = 128;
			fPMap.fMapHeight = 128;
		}
	}
}

boolean BakingCommand::Do()
{
	Validate();

	// Save the new preferences
	IShParameterComponent* myPrefsComp= GetMyPrefsComponent();
	if (MCVerify(myPrefsComp))
	{
		myPrefsComp->SetParameter('MapW', (void**)&fPMap.fMapWidth);
		myPrefsComp->SetParameter('MapH', (void**)&fPMap.fMapHeight);
		myPrefsComp->SetParameter('MapT', (void**)&fPMap.fMapType);
		myPrefsComp->SetParameter('Diff', (void**)&fPMap.fDiffChannel);
		myPrefsComp->SetParameter('High', (void**)&fPMap.fHighChannel);
		myPrefsComp->SetParameter('Shin', (void**)&fPMap.fShinChannel);
		myPrefsComp->SetParameter('Bump', (void**)&fPMap.fBumpChannel);
		myPrefsComp->SetParameter('Refl', (void**)&fPMap.fReflChannel);
		myPrefsComp->SetParameter('Tran', (void**)&fPMap.fTranChannel);
		myPrefsComp->SetParameter('Refr', (void**)&fPMap.fRefrChannel);
		myPrefsComp->SetParameter('Glow', (void**)&fPMap.fGlowChannel);
		myPrefsComp->SetParameter('UCam', (void**)&fPMap.fUseCamera);
		myPrefsComp->SetParameter('Type', (void**)&fPMap.fNormalType);
		myPrefsComp->SetParameter('Doma', (void**)&fPMap.fShadingDomainTreatment);
		myPrefsComp->SetParameter('BCol', (void**)&fPMap.fBackgroundColor);
		myPrefsComp->SetParameter('Spac', (void**)&fPMap.fSpace);
		myPrefsComp->SetParameter('Save', (void**)&fPMap.fAutoSave);
		myPrefsComp->SetParameter('Fold', (void**)&fPMap.fFolderName);
		myPrefsComp->SetParameter('Open', (void**)&fPMap.fOpenAfterSave);
	}

	const int32 treeCount = fSelectedTrees.GetElemCount();

	if(fGetLights)
	{
		InitRenderer();
	}

	for(int32 iTree=0 ; iTree<treeCount ; iTree++)
	{
		TMCCountedPtr<I3DShInstance> instance;
		fSelectedTrees[iTree]->QueryInterface(IID_I3DShInstance, (void**) &instance); 

		if(MCVerify(instance))
		{
			// Get the geometric primitive
			TMCCountedPtr<I3DShObject> object;
			instance->GetDeformed3DObject(&object);
			if(!object)
				continue;

			TMCCountedPtr<I3DShPrimitive> primitive;
			object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
			if(!primitive)
				continue;

			// Get other data
			boolean hasData = true;

			TMCCountedPtr<I3DShMasterShader> masterShader;

			if(fDoShading)
			{
				instance->GetShader(&masterShader);

				if(!masterShader)
					hasData = false;  // No data
			}

			if(fGetNormal)
			{	// Maybe need the object and the transform 
				// But if we already has bump information in the
				// shader, I'm not sure it's necessary
			}

			if(fGetLights)
			{
				TBBox3D bbox;
				object->GetBoundingBox(bbox);
				const real32 newPrecision = bbox.GetMax()/100;
				if(newPrecision<fPrecision)
					fPrecision = newPrecision;
			}

			if(hasData)
			{
				try
				{
					BakeMap(fSelectedTrees[iTree],
							instance, 
							primitive, 
							masterShader);
				}
				catch(TMCException&)
				{
				}
			}
		}
	}

	if(fGetLights)
	{
		EndRenderer();
	}

//	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	return true;
}

bool CheckPoint( int32 index, int32 otherIndex, const BufferData& buffer )
{
	const uint8* redBase		= buffer.fPlanarBuckets[0].BaseAddress();
	const uint8* greenBase		= buffer.fPlanarBuckets[1].BaseAddress();
	const uint8* blueBase		= buffer.fPlanarBuckets[2].BaseAddress();
	const uint8* alphaBase		= buffer.fPlanarBuckets[3].BaseAddress();

	const uint8* X = redBase+index;
	const uint8* Y = greenBase+index;
	const uint8* Z = blueBase+index;
	const uint8* A = alphaBase+index;

	const uint8* nextX = redBase+otherIndex;
	const uint8* nextY = greenBase+otherIndex;
	const uint8* nextZ = blueBase+otherIndex;
	const uint8* nextA = alphaBase+otherIndex;

	if(*nextX!=*X || *nextY!=*Y || *nextZ!=*Z || *nextA!=*A)
	{
		return true;
	}

	return false;
}

void GetMissingPointsToClean(std::map<int32,int32>& cleanedMissingPoints,
						const TMCArray<int32>& missingPoints,
						const BufferData& buffer,
						const int32 w, const int32 h)
{
	const int32 missPointCount = missingPoints.GetElemCount();
	for(int32 iPoint=0 ; iPoint<missPointCount ; iPoint+=2)
	{
		const int32 iLine = missingPoints[iPoint];
		const int32 iCol = missingPoints[iPoint+1];
		
		const int32 index = iLine*w+iCol;

		// First look over
		if(iLine<h-1)
		{
			const int32 otherIndex = (iLine+1)*w+iCol;
			if( CheckPoint( index, otherIndex, buffer ) )
			{
				cleanedMissingPoints[index] = otherIndex;
				continue;
			}
		}
		// Then after
		if(iCol<w-1)
		{
			const int32 otherIndex = iLine*w+iCol+1;
			if( CheckPoint( index, otherIndex, buffer ) )
			{
				cleanedMissingPoints[index] = otherIndex;
				continue;
			}
		}
		// under
		if(iLine>0)
		{
			const int32 otherIndex = (iLine-1)*w+iCol;
			if( CheckPoint( index, otherIndex, buffer ) )
			{
				cleanedMissingPoints[index] = otherIndex;
				continue;
			}
		}
		// before
		if(iCol>0)
		{
			const int32 otherIndex = iLine*w+iCol-1;
			if( CheckPoint( index, otherIndex, buffer ) )
			{
				cleanedMissingPoints[index] = otherIndex;
				continue;
			}
		}
	}
}

void RepareMissingPoints(const std::map<int32,int32>& missingPoints, BufferData& buffer)
{
	if( !missingPoints.size() )
		return;

	uint8* redBase		= buffer.fPlanarBuckets[0].BaseAddress();
	uint8* greenBase	= buffer.fPlanarBuckets[1].BaseAddress();
	uint8* blueBase		= buffer.fPlanarBuckets[2].BaseAddress();
	uint8* alphaBase	= buffer.fPlanarBuckets[3].BaseAddress();

	std::map<int32,int32>::const_iterator itr;
	for( itr=missingPoints.begin() ; itr!=missingPoints.end() ; itr++ )
	{
		const int32 index = (*itr).first;

		uint8* X = redBase+index;
		uint8* Y = greenBase+index;
		uint8* Z = blueBase+index;
		uint8* A = alphaBase+index;

		const int32 otherIndex = (*itr).second;

		*X = *(redBase+otherIndex);
		*Y = *(greenBase+otherIndex);
		*Z = *(blueBase+otherIndex);
		*A = *(alphaBase+otherIndex);
	}
}

void BakingCommand::BakeMap(I3DShTreeElement*			tree,
							I3DShInstance*				instance, 
							I3DShPrimitive*				primitive,
							I3DShMasterShader*			masterShader)
{
	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'Baki');

	MY_ASSERT(tree);
	MY_ASSERT(instance);
	MY_ASSERT(primitive);

	const int32 w = fPMap.fMapWidth;
	const int32 h = fPMap.fMapHeight;

	const boolean getBumpValue = (fShadingChannel&eBumpChannel && fPMap.fMapType==eShadingMap);
	// Do not get the shading for the lightMap: will do a ShadeAndLight instead
	const boolean getShading = (fPMap.fMapType==eShadingMap && !(fShadingChannel==eBumpChannel)) ||
								(fPMap.fMapType==eNormalMap && fShadingChannel!=eNoChannel );

	TMCCountedPtr<I3DShShader> bumpShader;
	if(getBumpValue)
	{
		TMCCountedPtr<I3DShShader> shader;
		masterShader->GetShader(&shader);
		GetBumpShader(shader, &bumpShader);
	}

	TMCDynamicString treeName;
	tree->GetName(treeName);

	// Create a UV to XYZ transformer
	UV2XYZ mapToPoint(primitive);

	// Get the shadingDomain count, we need to create one
	// file per domain, unless the option 'draw all in one'
	// is checked

	const uint32 domainCount = instance->GetUVSpaceCount();

	TMCCountedPtr<IMCUnknown> progressKey; // Progress bar key
	TMCString255 progressString;
	gResourceUtilities->GetIndString(progressString, kOtherNames, 3);
	gShellUtilities->BeginProgress(progressString, &progressKey);
	const real32 inc = 100.0f/(real32)(domainCount*h);

	uint32 mapCount = domainCount;
	uint32 mapStart = 0;
	if(fPMap.MergeDomain())
	{
		mapCount = 1;
	}
	else if(fPMap.OneDomain())
	{
		mapStart = fPMap.DomainID();
		mapCount = mapStart+1;
	}

	for(uint32 iDomain=mapStart ; iDomain<mapCount ; iDomain++)
	{
		boolean cancel = false;

		UVSpaceInfo uvSpaceInfo;
		instance->GetUVSpace(iDomain, &uvSpaceInfo);

		// Init the Shading channel shaders
		BufferData channelBufferData[kChannelCount];

		TMCString15 domainStr;
		domainStr.FromInt32(iDomain);

		if(fGetShader)
		{	// Create and allocate the ones we want
			for(int32 iChannel=0 ; iChannel<kChannelCount ; iChannel++)
			{
				const int32 flag = (int32)RealPow(2,iChannel);
		
				if(fShadingChannel&flag)
				{
					TMCString31 channelStr;
					gResourceUtilities->GetIndString( channelStr, kChannelNames, iChannel+1);
					TMCDynamicString name = treeName+channelStr+domainStr;
				
					BufferData& data = channelBufferData[iChannel];

					data.AllocData(w,h,kLayerCount,name);
				}
			}
		}

		// Init the normal map buffer
		BufferData normalBufferData;
		if(fGetNormal)
		{
			TMCString31 channelStr;
			gResourceUtilities->GetIndString( channelStr, kOtherNames, 1);
			TMCDynamicString name = treeName+channelStr+domainStr;

			normalBufferData.AllocData(w,h,kLayerCount,name);
		}

		// Init the light map buffer
		BufferData lightingBufferData;
		if(fGetLights)
		{
			TMCString31 channelStr;
			gResourceUtilities->GetIndString( channelStr, kOtherNames, 2);
			TMCDynamicString name = treeName+channelStr+domainStr;

			lightingBufferData.AllocData(w,h,kLayerCount,name);
		}

		// Fill in the raster layers

		// Get the global transform
		TTransform3D L2G;
		tree->GetGlobalTransform3D(L2G);
		TTransform3D G2L = Inverse(L2G);
		// TEST
		TGlobalTreeTransform mat;
		tree->GetGlobalTreeTransform2(mat);
		mat.GetTransform3D(L2G);
		mat.GetInverseTransform3D(G2L);
		TTreeTransform test;
		tree->GetGlobalTreeTransform(test);
		test.GetTransform3D(L2G);
		test.GetInverseTransform3D(G2L);

		// TEST
		//real32 magnitude = fScene->GetMagnitude();

		// Get some shading data
		ShadingFlags shadingFlags;
		instance->GetShadingFlags(shadingFlags);
		// Note: the shading flags are wrong since Carrara 5: some stuff are missing (with the perturbation shader for example).
		// So I just put everything to on.
		shadingFlags.fNeedsColor = 1;
		shadingFlags.fNeedsPoint = 1;
		shadingFlags.fNeedsNormal = 1;
		shadingFlags.fNeedsNormalDerivative = 1;
		shadingFlags.fNeedsIsoUV = 1;
		shadingFlags.fNeedsUV = 1;
		shadingFlags.fNeedsPointLoc = 1;
		shadingFlags.fNeedsNormalLoc = 1;
		shadingFlags.fNeedsNormalLocDerivative = 1;
		shadingFlags.fNeedsPixelRatio = 1;
		shadingFlags.fChangesNormal = 1;
		shadingFlags.fUVSpaceShaders = 1;

		if(fGetLights)
		{
			shadingFlags.fNeedsNormal = true; // Lights are in global space, use the global normal
			shadingFlags.fNeedsPoint = true; // Lights are in global space, use the global point		
		}
		if(fGetNormal)
		{
			if(fPMap.IsLocalSpace())
				shadingFlags.fNeedsNormalLoc = true;
			else
				shadingFlags.fNeedsNormal = true;
		}


		// ShadingIn shadingIn;
		RayHit3D shadingIn;

		shadingIn.fUVSpaceID	= iDomain; // Useless: it's initialize later
		shadingIn.fUVx			= TVector2::kUnitX;		
		shadingIn.fUVy			= TVector2::kUnitY;		
		if(fShadingChannel&eBumpChannel)
			shadingIn.fBumpOn=true;

		ShadingOut shadingOut;

		TMCRealRect uvRect(kRealZero, kRealZero, kRealOne, kRealOne);

		UVMaps maps;
		maps.Set(w,h);

		// Pointer on the buffers

		uint8* redLine[kChannelCount];
		uint8* greenLine[kChannelCount];
		uint8* blueLine[kChannelCount];
		uint8* alphaLine[kChannelCount];
		{
			for(int32 iChannel=0; iChannel<kChannelCount; iChannel++)
			{
				if(channelBufferData[iChannel].fIsInitialized)
				{
					redLine[iChannel]	= channelBufferData[iChannel].fPlanarBuckets[0].BaseAddress();
					greenLine[iChannel]	= channelBufferData[iChannel].fPlanarBuckets[1].BaseAddress();
					blueLine[iChannel]	= channelBufferData[iChannel].fPlanarBuckets[2].BaseAddress();
					alphaLine[iChannel]	= channelBufferData[iChannel].fPlanarBuckets[3].BaseAddress();
				}
				else
				{
					redLine[iChannel]	= NULL;
					greenLine[iChannel]	= NULL;
					blueLine[iChannel]	= NULL;
					alphaLine[iChannel]	= NULL;
				}
			}
		}
		// Pointers on the normal map buffer
		uint8* normalRed	= fGetNormal?normalBufferData.fPlanarBuckets[0].BaseAddress():NULL;
		uint8* normalGreen	= fGetNormal?normalBufferData.fPlanarBuckets[1].BaseAddress():NULL;
		uint8* normalBlue	= fGetNormal?normalBufferData.fPlanarBuckets[2].BaseAddress():NULL;
		uint8* normalAlpha	= fGetNormal?normalBufferData.fPlanarBuckets[3].BaseAddress():NULL;
		// Pointers on the lighting map buffer
		uint8* lightRed		= fGetLights?lightingBufferData.fPlanarBuckets[0].BaseAddress():NULL;
		uint8* lightGreen	= fGetLights?lightingBufferData.fPlanarBuckets[1].BaseAddress():NULL;
		uint8* lightBlue	= fGetLights?lightingBufferData.fPlanarBuckets[2].BaseAddress():NULL;
		uint8* lightAlpha	= fGetLights?lightingBufferData.fPlanarBuckets[3].BaseAddress():NULL;

		const int32 rowBytes = w;

		TMCArray<int32> missingPoints; // At the end, we filter the map to fill in some missing points (to avoid sides effects)

		for (int32 iLine=0 ; iLine<h ; iLine++)
		{
			// Get a pointer on the 4 lines

			// For the shaders buffer
			uint8* pixelRed[kChannelCount];
			uint8* pixelGreen[kChannelCount];
			uint8* pixelBlue[kChannelCount];
			uint8* pixelAlpha[kChannelCount];
			{
				for(int32 iChannel=0; iChannel<kChannelCount; iChannel++)
				{
					pixelRed[iChannel]	= redLine[iChannel];
					pixelGreen[iChannel]	= greenLine[iChannel];
					pixelBlue[iChannel]	= blueLine[iChannel];
					pixelAlpha[iChannel]	= alphaLine[iChannel];
				}
			}

			for (int32 iCol=0 ; iCol<w ; iCol++)
			{
				// Get the shading for this pixel
				boolean hasPoint = 
					PrepareShadingPixel(iCol, iLine, shadingFlags, 
					maps, uvRect, shadingIn, 
					mapToPoint, uvSpaceInfo, L2G);

				if(fPMap.MergeDomain())
				{
					for( uint32 id=1 ; id<domainCount && !hasPoint ; id++)
					{
						uvSpaceInfo.fID = id;
						hasPoint = 
							PrepareShadingPixel(iCol, iLine, shadingFlags, 
							maps, uvRect, shadingIn, 
							mapToPoint, uvSpaceInfo, L2G);
					}
				}

				if(!hasPoint)
				{
					missingPoints.AddElem(iLine);
					missingPoints.AddElem(iCol);
				}
			
				if(getShading)
				{
					shadingOut.SetDefaultValues();
					instance->DoShade(shadingOut, shadingIn);
				}

				if(fPMap.MergeDomain())
				{	// Reinitialize the domain ID
					uvSpaceInfo.fID = iDomain;
				}

				// Copy the color in the buffer
				if(fGetShader)
				{
					if(fShadingChannel&eDiffuseChannel)
					{
						TMCColorRGBA8 intColor(hasPoint?shadingOut.fColor:fPMap.fBackgroundColor);
						*(pixelRed[0]++) = intColor.red;
						*(pixelGreen[0]++) = intColor.green;
						*(pixelBlue[0]++) = intColor.blue;
						*(pixelAlpha[0]++) = intColor.alpha;
					}
					if (fShadingChannel&eHighlightChannel)
					{
						TMCColorRGBA8 intColor(hasPoint?shadingOut.fSpecularColor:fPMap.fBackgroundColor);
						*(pixelRed[1]++) = intColor.red;
						*(pixelGreen[1]++) = intColor.green;
						*(pixelBlue[1]++) = intColor.blue;
						*(pixelAlpha[1]++) = intColor.alpha;
					}
					if (fShadingChannel&eShininessChannel)
					{
						if( hasPoint )
						{
							*(pixelRed[2]++) = 
							*(pixelGreen[2]++) = 
							*(pixelBlue[2]++) = 
							*(pixelAlpha[2]++) = shadingOut.fSpecularSize*255;
						}
						else
						{
							TMCColorRGBA8 intColor(fPMap.fBackgroundColor);
							*(pixelRed[2]++) = intColor.red;
							*(pixelGreen[2]++) = intColor.green;
							*(pixelBlue[2]++) = intColor.blue;
							*(pixelAlpha[2]++) = intColor.alpha;
						}
					}
					if (fShadingChannel&eBumpChannel)
					{
						if( getBumpValue && hasPoint)
						{
							real32 bumpValue=0;
							if(bumpShader)
							{
								boolean fullArea = false;
								bumpShader->GetValue(bumpValue,fullArea,shadingIn );
							}
							*(pixelRed[3]++) = 
							*(pixelGreen[3]++) =
							*(pixelBlue[3]++) = 
							*(pixelAlpha[3]++) = 255*bumpValue;
						}
						else
						{
							TMCColorRGBA8 intColor(fPMap.fBackgroundColor);
							*(pixelRed[3]++) = intColor.red;
							*(pixelGreen[3]++) = intColor.green;
							*(pixelBlue[3]++) = intColor.blue;
							*(pixelAlpha[3]++) = intColor.alpha;
						}
					}
					if (fShadingChannel&eReflectionChannel)
					{
						TMCColorRGBA8 intColor(hasPoint?shadingOut.fReflection.fReflection:fPMap.fBackgroundColor);
						*(pixelRed[4]++) = intColor.red;
						*(pixelGreen[4]++) = intColor.green;
						*(pixelBlue[4]++) = intColor.blue;
						*(pixelAlpha[4]++) = intColor.alpha;
					}
					if (fShadingChannel&eTransparencyChannel)
					{
						TMCColorRGBA8 intColor(hasPoint?shadingOut.fTransparency.fIntensity:fPMap.fBackgroundColor);
						*(pixelRed[5]++) = intColor.red;
						*(pixelGreen[5]++) = intColor.green;
						*(pixelBlue[5]++) = intColor.blue;
						*(pixelAlpha[5]++) = intColor.alpha;
					}
					if (fShadingChannel&eRefractionChannel)
					{
						if( hasPoint )
						{
							*(pixelRed[6]++) = 
							*(pixelGreen[6]++) = 
							*(pixelBlue[6]++) = 
							*(pixelAlpha[6]++) = shadingOut.fRefractiveIndex*255;
						}
						else
						{
							TMCColorRGBA8 intColor(fPMap.fBackgroundColor);
							*(pixelRed[6]++) = intColor.red;
							*(pixelGreen[6]++) = intColor.green;
							*(pixelBlue[6]++) = intColor.blue;
							*(pixelAlpha[6]++) = intColor.alpha;
						}
					}
					if (fShadingChannel&eGlowChannel)
					{
						TMCColorRGBA8 intColor(hasPoint?shadingOut.fGlow:fPMap.fBackgroundColor);
						*(pixelRed[7]++) = intColor.red;
						*(pixelGreen[7]++) = intColor.green;
						*(pixelBlue[7]++) = intColor.blue;
						*(pixelAlpha[7]++) = intColor.alpha;
					}
				}

				if( fGetNormal )
				{	// Fill in the Normal Map bucket
					if( hasPoint )
					{
						if(fPMap.fNormalType == 'Typ3')
						{	// Deeper type
							// We need to rotate the bump vector: the normal to the surface is brought
							// back on the Z axis
							
							// First get the rotation axis
							TVector3 vector = shadingOut.fChangedNormalLoc;
					
							// Normal
							TVector3 isoN = shadingIn.fNormalLoc;
							TVector3 isoU, isoV;
							isoN.BuildOrthonormalBase(isoU,isoV);

																 
							/* Working with the IsoU and V doesn't work. Apparently
							we're not doing the same thing than in Carrara, so it won't work
							TVector3 isoU = shadingIn.fIsoU;
							isoU.Normalize();
							TVector3 isoV = shadingIn.fIsoV;
							isoV.Normalize();
							TVector3 isoN = isoU^isoV;
							if(isoN*shadingIn.fNormalLoc<0)
								isoN.z*=-1;
							*/

							// Solution to the system A*x = b is x = invA*b
							TMatrix33 A;
							A.SetColumns(isoU,isoV,isoN);
							boolean success=true;
							TMatrix33 invA = A.Inverse(&success);
							if(success)
								vector = invA*vector;
							else
								vector = TVector3::kUnitZ;

							vector.Normalize();
							if(vector.z<0)
							{
								vector.z*=-1;
							}

							*(normalRed++)		= .5*(1+vector.x)*255;
							*(normalGreen++)	= .5*(1+vector.y)*255;
							*(normalBlue++)		= .5*(1+vector.z)*255;
							*(normalAlpha++)	= 255; // Opacity
						}
						else
						{	// Classic Baker type and ZBrush type

							TVector3 normal=TVector3::kZero;
							if(fShadingChannel&eBumpChannel)
							{
								if( fPMap.IsLocalSpace() )
								{
									normal=shadingOut.fChangedNormalLoc;
								}
								else
								{
									normal=TransformVector(L2G, shadingOut.fChangedNormalLoc);
									normal.Normalize();
								}
							}
							else
							{
								normal=fPMap.IsLocalSpace()?shadingIn.fNormalLoc:
															shadingIn.fGNormal;
							}

							// Use the bump vector if there's one
							if(fPMap.fNormalType == 'Typ1')
							{	// classic type
								*(normalRed++)		= .5*(1+normal.x)*255;
								*(normalGreen++)	= .5*(1+normal.y)*255;
								*(normalBlue++)		= .5*(1+normal.z)*255;
								*(normalAlpha++)	= 255; // Opacity
							}
							else if(fPMap.fNormalType == 'Typ2')
							{	// ZBrush type
								*(normalRed++)		= .5*(1-normal.x)*255;
								*(normalGreen++)	= .5*(1+normal.z)*255;
								*(normalBlue++)		= .5*(1+normal.y)*255;
								*(normalAlpha++)	= 255; // Opacity
							}
						}
					}
					else
					{
						TMCColorRGBA8 intColor(fPMap.fBackgroundColor);
						*(normalRed++)		= intColor.red;
						*(normalGreen++)	= intColor.green;
						*(normalBlue++)		= intColor.blue;
						*(normalAlpha++)	= 0; // Transparency
					}
				}

				if( fGetLights )
				{	// Fill in the Light Map bucket
					if( hasPoint )
					{
						TMCColorRGBA lighting = TMCColorRGBA::kBlackNoAlpha;
						shadingOut.SetDefaultValues();
						GetRealLighting(lighting, shadingIn, shadingOut, instance, L2G, G2L);

						TMCColorRGBA8 intColor(lighting);
						// Use the bump vector if there's one
						*(lightRed++)	= intColor.red;
						*(lightGreen++)	= intColor.green;
						*(lightBlue++)	= intColor.blue;
						*(lightAlpha++)	= 255; // Opacity
					}
					else
					{
						TMCColorRGBA8 intColor(fPMap.fBackgroundColor);
						*(lightRed++)	= intColor.red;
						*(lightGreen++)	= intColor.green;
						*(lightBlue++)	= intColor.blue;
						*(lightAlpha++)	= 0; // Transparency
					}
				}
			}

			// Go to next line
			{
				// For the shader buffers
				for(int32 iChannel=0; iChannel<kChannelCount; iChannel++)
				{
					if(channelBufferData[iChannel].fIsInitialized)
					{
						redLine[iChannel] += rowBytes;
						greenLine[iChannel] += rowBytes;
						blueLine[iChannel] += rowBytes;
						alphaLine[iChannel] += rowBytes;
					}
				}
			}
				
			gShellUtilities->IncrementProgress(inc, progressKey);
			if (gShellUtilities->CheckForUserCancel())
			{
				cancel = true;
				break;
			}
		}

		// Filter the maps to fill in the side pixels
		// Get the indexes of the point we'll be able to clean
		std::map<int32,int32> missingPointsToClean;
		bool pointsFound = false;
		if(fGetShader)
		{
			for(int32 iChannel=0 ; iChannel<kChannelCount && !pointsFound ; iChannel++)
			{
				if(channelBufferData[iChannel].fIsInitialized)
				{
					GetMissingPointsToClean(missingPointsToClean, missingPoints, channelBufferData[iChannel], w, h);
					pointsFound = true;
				}
			}
		}
		if(fGetNormal && !pointsFound)
		{
			GetMissingPointsToClean(missingPointsToClean, missingPoints, normalBufferData, w, h);
			pointsFound = true;
		}
		if(fGetLights && !pointsFound)
		{
			GetMissingPointsToClean(missingPointsToClean, missingPoints, lightingBufferData, w, h);
			pointsFound = true;
		}
		// Replace the points
		if(pointsFound)
		{
			if(fGetShader)
			{
				for(int32 iChannel=0 ; iChannel<kChannelCount ; iChannel++)
				{
					if(channelBufferData[iChannel].fIsInitialized)
					{
						RepareMissingPoints(missingPointsToClean, channelBufferData[iChannel]);
					}
				}
			}
			if(fGetNormal)
			{
				RepareMissingPoints(missingPointsToClean, normalBufferData);
			}
			if(fGetLights)
			{
				RepareMissingPoints(missingPointsToClean, lightingBufferData);
			}
		}

		// Fill in the raster layer
		{
			// Shader buffer
			for(int32 iChannel=0; iChannel<kChannelCount; iChannel++)
			{
				channelBufferData[iChannel].MoveImageFromBucketToRasterLayer();
			}
			// Normal buffer
			normalBufferData.MoveImageFromBucketToRasterLayer();
			// Light buffer
			lightingBufferData.MoveImageFromBucketToRasterLayer();
		}
		if( fPMap.fAutoSave )
		{
			// Shader buffer
			for(int32 iChannel=0; iChannel<kChannelCount; iChannel++)
			{
				channelBufferData[iChannel].SaveIn(fPMap.fFolderName, !fPMap.fOpenAfterSave );
			}
			// Normal buffer
			normalBufferData.SaveIn(fPMap.fFolderName, !fPMap.fOpenAfterSave );
			// Light buffer
			lightingBufferData.SaveIn(fPMap.fFolderName, !fPMap.fOpenAfterSave );
		}

		if(cancel)
		{
			gShellUtilities->EndProgress(progressKey);
			throw TMCException(kuserCanceledErr); // User cancelled
		}
	}

	gShellUtilities->EndProgress(progressKey);
}

boolean BakingCommand::PrepareShadingPixel(uint32					ii,
									uint32					jj,
									const ShadingFlags &	shadingFlags, 
									UVMaps &				maps, 
									const TMCRealRect &		uvRect,
									ShadingIn &				shadingIn,
									UV2XYZ&					mapToPoint,
									const UVSpaceInfo&		uvSpaceInfo,
									const TTransform3D&		L2G) const
{
	// Added all this initializations here, because the last DoShade could had changed this values:
	shadingIn.fUVSpaceID	= uvSpaceInfo.fID;
	shadingIn.fUVx			= TVector2::kUnitX;
	shadingIn.fUVy			= TVector2::kUnitY;		
	shadingIn.fPointx		= TVector3::kUnitX;
	shadingIn.fPointy		= TVector3::kUnitY;
	shadingIn.fNormalx		= TVector3::kUnitX;
	shadingIn.fNormaly		= TVector3::kUnitY;
	shadingIn.fPointLocx	= TVector3::kUnitX;
	shadingIn.fPointLocy	= TVector3::kUnitY;
	shadingIn.fNormalLocx   = TVector3::kUnitX;
	shadingIn.fNormalLocy	= TVector3::kUnitY;

	shadingIn.fCurrentCompletionMask = 0; // This one is very important !!!!! If it's no checked, if you have a layer list, we will never have the correct do shade.

	MY_ASSERT( shadingIn.fUVSpaceID == uvSpaceInfo.fID );

	const boolean needPoint = shadingFlags.fNeedsPoint || shadingFlags.fNeedsPointLoc;
	const boolean needNormal = shadingFlags.fNeedsNormal || shadingFlags.fNeedsNormalLoc;
	const boolean needSpecials = needPoint || needNormal;
	
	if (needSpecials || shadingFlags.fNeedsUV)
	{
		TVector2 uv;

		MY_ASSERT(maps.fWidth != 0 );
		MY_ASSERT(maps.fHeight!= 0 );

		float uStep = uvRect.GetWidth()/Int32ToReal(maps.fWidth);
		float vStep = uvRect.GetHeight()/Int32ToReal(maps.fHeight);

		// +0.5: to get the UV at the middle of the pixel, and not in its corner
		uv[0] = (Int32ToReal(ii)+0.5f)*uStep + uvRect.left;
		uv[1] = (Int32ToReal(jj)+0.5f)*vStep + uvRect.top;
//		if(fFlags&eSwapV)
		uv[1]=1-uv[1]; // put the bottom at the bottom
//InZeroOne(uv);	
		shadingIn.fUV = uv;

		// UV derivatives
		shadingIn.fUVx *= uStep;
		shadingIn.fUVy *= vStep;

		if(needSpecials)
		{
			bool needGlobDerivative = false;
			bool needLocDerivative = false;

			if( mapToPoint.GetPointAndNormal(uv, uvSpaceInfo, 
											shadingIn.fPointLoc, needPoint,
											shadingIn.fNormalLoc, needNormal,
											shadingIn.fIsoU, shadingIn.fIsoV, shadingFlags.fNeedsIsoUV) )
			{
				if(shadingFlags.fNeedsPoint)
				{
					shadingIn.fPoint = TransformPoint(L2G, shadingIn.fPointLoc);

					needGlobDerivative = true;
				}

				
				if( shadingFlags.fNeedsPointLoc )
				{	
					needLocDerivative = true;
				}

				if(shadingFlags.fNeedsNormal)
				{
					shadingIn.fGNormal = TransformVector(L2G, shadingIn.fNormalLoc);
					shadingIn.fGNormal.Normalize();

				}
				if( shadingFlags.fNeedsNormalDerivative )
				{
					needGlobDerivative = true;
				}
				if( shadingFlags.fNeedsNormalLocDerivative )
				{
					needLocDerivative = true;
				}
			}
			else
			{	// The UV has no correspondace in the model -> put an arbitrary color
				return false;
			}

			if( needGlobDerivative || needLocDerivative )
			{
				// This are the wrong values, but it's just to put something usable

				TVector3 dirX, dirY;
				shadingIn.fNormalLoc.BuildOrthonormalBase(dirX,dirY);
				
				shadingIn.fPointLocx = 0.01*dirX;
				shadingIn.fPointLocy = 0.01*dirY;
				shadingIn.fNormalLocx = 0.01*dirX;
				shadingIn.fNormalLocy = 0.01*dirY;
				
				if( needGlobDerivative )
				{
					shadingIn.fGNormal.BuildOrthonormalBase(dirX,dirY);
					
					shadingIn.fPointx = 0.01*dirX;
					shadingIn.fPointy = 0.01*dirY;

					shadingIn.fNormalx = 0.01*dirX;
					shadingIn.fNormaly = 0.01*dirY;
				}
			}
		}
	}

	//isoU and isoV not supported, supply default values
/*	if (shadingFlags.fNeedsIsoUV)
	{
		shadingIn.fIsoU = TVector3::kUnitX;

		shadingIn.fIsoV = TVector3::kUnitY;
	}*/

	return true;
}

void BakingCommand::GetBumpShader(I3DShShader* shader, I3DShShader** bumpShader)
{
	// This is not the right solution. We sould do the oposite of GetPerturbationVector
	// => get 4 normals around our point and compute the value from them

	// Find the bump channel and return it value

	TMCCountedPtr<IShMinimalParameterComponent> shaderMinComponent;
	shader->QueryInterface(IID_IShMinimalParameterComponent,(void**)&shaderMinComponent);
	if(shaderMinComponent)
	{
		if(shaderMinComponent->GetClassSignature()==kCompositeShader)
		{	// Regular Multi Channel shader: get its bump shader
			TMCCountedPtr<IShMinimalParameterComponent> component;
			shaderMinComponent->GetParameter( kCompositeShader_Bump, &component );
			if(component)
			{
				component->QueryInterface(IID_I3DShShader, (void**)bumpShader);
			}
		}

		// Shader list cases: 1 per domain

		// Global Mixer case

		// Veloute Tiling and Roofing
	}
}

////////////////////////////////////////////////////////////////////////////
//
// BufferData struct
//

void BufferData::AllocData(const int32 w, const int32 h, const int32 layerCount, const TMCString& docName)
{
	fLayerCount = layerCount;
					
	GetNewRasterLayer(&fRasterLayer, fLayerCount, w,h, docName);

	void *buffers[4];
	for(int32 i=0; i < layerCount; i++)
	{
		buffers[i] = MCmalloc(w*h*sizeof(int8));
		fPlanarBuckets[i].BuildCore((uint8 *)buffers[i], w, w, h, 8, 1, true);
	}

	fIsInitialized = true;
}


void BufferData::MoveImageFromBucketToRasterLayer()
{
	if(fIsInitialized)
	{
		const TMCRect rect(0,0,fPlanarBuckets[0].Width(),fPlanarBuckets[0].Height());
		if(kLayerCount==3)
			fRasterLayer->PutRGBData(fPlanarBuckets , rect , eTileWrite ); 
		else
			fRasterLayer->PutRGBAData(fPlanarBuckets , rect , eTileWrite ); 
	
		// Release the memory
		for(int32 iBuff=0; iBuff < fLayerCount; iBuff++)
		{
			MCfree(fPlanarBuckets[iBuff].BaseAddress());
		}
	
		// Reput it in front, the progress bar came over
		fImageDocument->UpdateRect(rect);
	}
}

void BufferData::GetNewRasterLayer(IShRasterLayer**	rasterLayer, 
								  const int32		layerCount,
								   const int32 w, const int32 h,
								  const TMCString&		name)
{
	// Create an image document and get its rasterlayer

	const real32 dpi = 72.0f;

	gShell3DUtilities->CreateImageDocument(&fImageDocument);
	ThrowIfNil(fImageDocument);
			
	fImageDocument->SetIsPreview(false);
	
	// Build a default name: treeName + domain + rendering type + channel
	TMCCountedPtr<IMFDocument> document;
	if (fImageDocument->QueryInterface(IID_IMFDocument, (void**)&document) == MC_S_OK)
	{
		document->SetTitle(name);
	}

	// In some cases (C4 diffuse layer for exemple)
	if(layerCount==3)
		fImageDocument->InitializeRasterLayer(w, h, eLayerRGB, dpi);
	else
		fImageDocument->InitializeRasterLayer(w, h, eLayerRGBA, dpi);

	fImageDocument->GetRasterLayer(rasterLayer);
	fImageDocument->ShowWindow();
}

void BufferData::SaveIn( const TMCString& folder, bool close )
{
	if(fIsInitialized)
	{
		TMCCountedPtr<IMCFile> file;
		gFileUtilities->CreateIMCFile(&file);

		TMCCountedPtr<IMFDocument> document;
		TMCDynamicString fileName;
		if (fImageDocument->QueryInterface(IID_IMFDocument, (void**)&document) == MC_S_OK)
		{
			document->GetTitle(fileName);
		}

		TMCDynamicString fullPathName = folder + fileName + TMCDynamicString(".jpg");
		file->SetWithFullPathName( fullPathName );

		fImageDocument->SaveToDisk(file,'JPEG',false, NULL, NULL);

		if( close )
			fImageDocument->CloseWindow();
	}
}

void BakingCommand::InitRenderer()
{
	TMCCountedPtr< I3DRenderingModule> renderingModule;
	fScene->GetSceneRenderingModule( &renderingModule );

	renderingModule->GetRenderingCamera(&fRenderingCamera) ;
	renderingModule->GetRenderer( &fRenderer) ;

	I3DShTreeElement* elem = fRenderingCamera->GetTreeElement	();
	TTransform3D transform;
	elem->GetGlobalTransform3D(transform);
	fCameraPos = TransformPoint( transform, TVector3::kZero );

	fRenderer->SetCamera(fRenderingCamera);

	TMCCountedPtr< I3DShGroup> treeRoot;
	fScene->GetTreeRoot(&treeRoot) ;
	fRenderer->SetTreeTop(treeRoot) ;

	TMCColorRGB acolor = fScene->GetAmbientLightColor( );
	fRenderer->SetAmbientLight(acolor) ;

	// this call is used to disable volumetric rendering (for instance in the lightcone preview)
	fRenderer->RenderVolumetrics( false);

	TMCCountedPtr<I3DShEnvironment> environment;
	fScene->GetEnvironment(&environment );
	fRenderer->SetEnvironment(environment);

	//TMCPoint size(fRenderArea- >fWidth,fRenderA rea->fHeight) ;
	//TBBox2D uvBox= fRenderArea- >fuv;
	TMCPoint size(fPMap.fMapWidth,fPMap.fMapHeight) ;
	TBBox2D uvBox(TVector2::kZero, TVector2::kOnes);
	fRenderer->PrepareDraw(size,uvBox, uvBox, NULL, NULL);

	fRenderer->CreateRaytracer(&fRaytracer) ;
}

void BakingCommand::EndRenderer()
{
	if( fRenderer )
		fRenderer->FinishDraw();

	fRenderer=NULL;
}

void BakingCommand::GetRealLighting(TMCColorRGBA&	lighting, 
								RayHit3D&		shadingIn,
								ShadingOut& shadingOut,
								I3DShInstance*	instance,
								TTransform3D& L2G,
								TTransform3D& G2L)
{
	if(fRaytracer)
	{
		// Initialize the lighting context
		LightingDetail lightingResult(shadingOut);
		RTLevelInfo level;
		LightingContext lightingContext(fRaytracer,
							fRaytracer->GetIllumSettings(),
							fRaytracer->GetRendEnv(),
							level,
							0);
		lightingContext.fFirstHitInfo = NULL;
		lightingContext.fScreenCoordinates.x = 0.5;
		lightingContext.fScreenCoordinates.y = 0.5;
		
		// Prepare the RayHit
		Ray3D ray;
		ray.Init();
		TVector3 direction = shadingIn.fGNormal;
		double distance = 0.5;
		if( fPMap.fUseCamera && fRenderingCamera )
		{
			direction = fCameraPos - shadingIn.fPoint;
			distance = direction.GetMagnitude();
			direction/=distance; // Normalize
			// Check if the camera is behind
			//float check = direction * shadingIn.fGNormal;
			//if( check<0 )
			//	distance = 0;
		}
		// TODO: 0.5*scaling of the object
		ray.fDirection	= -direction;
		ray.fOrigin = shadingIn.fPoint + distance * direction;

		//const TVector3	rayDir = -shadingIn.fNormalLoc;
		//const TVector3	rayOri = shadingIn.fPointLoc + 0.01 * shadingIn.fNormalLoc;

		Ray3D localRay;
		localRay.Init();
		localRay.fOrigin	= TransformPoint(G2L, ray.fOrigin);
		localRay.fDirection = TransformVector(G2L, ray.fDirection);

		lightingContext.fIncomingRay = &localRay;

		RayHit3D hitResult(shadingIn);
		
		RayHitParameters rayHitParam;
		rayHitParam.tmin = 0;
		rayHitParam.tmax = 1e30f;
		rayHitParam.ray = &localRay;
		rayHitParam.hit  = &hitResult; // RayHit3D
		rayHitParam.originInstanceIndex = -1;
		rayHitParam.originSubIndexIndex = -1;
		rayHitParam.originFacetIndex = -1;

		hitResult.ft = 0;
		hitResult.fInstance = instance;
		hitResult.fFacetMesh = instance->GetRenderingFacetMesh();

		// Hit the object
		if( instance->RayHit( rayHitParam) )
		{
			hitResult.fPointLoc		= localRay.fOrigin+localRay.fDirection*hitResult.ft;
			hitResult.fInstance		= instance;
			hitResult.fT				= L2G;
			hitResult.fInvT			= G2L;
			hitResult.fInstanceIndex	= -1;
	
			ShadingFlags flags;
		//	instance->GetShadingFlags(flags);
			flags.fNeedsColor				= true;				
			flags.fNeedsPoint				= true;				
			flags.fNeedsNormal				= true;				
			flags.fNeedsNormalDerivative	= true;		
			flags.fNeedsIsoUV				= true;				
			flags.fNeedsUV					= true;					
			flags.fNeedsPointLoc			= true;				
			flags.fNeedsNormalLoc			= true;			
			flags.fNeedsNormalLocDerivative	=true;	
			flags.fNeedsPixelRatio			= true;			
			flags.fChangesNormal			= true;				
			flags.fUVSpaceShaders			= true;
	
			hitResult.CalcInfo(flags, true, ray );

			// Complete the lighting context
			lightingContext.fHit = &hitResult;
			lightingContext.fNormalFlipped = false;
			lightingContext.fNormal = hitResult.fGNormal;
			lightingContext.fReflectDir= hitResult.fGNormal;
			lightingContext.fRendContext.fNumberOfRays = 0;
			lightingContext.fRendContext.fRandomSeed = 0;

			// Compute the lighting
			instance->ShadeAndLight2(lightingResult ,lightingContext, NULL );

			lighting = lightingResult.fResColor;
			lighting.Clamp();
		}
		else
		{
			// No hit
			lighting = TMCColorRGBA::kBlackNoAlpha;
		}
	}
}




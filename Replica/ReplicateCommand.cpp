/****************************************************************************************************

		ReplicateCommand.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/13/2004

****************************************************************************************************/

#include "ReplicateCommand.h"

#include "copyright.h"
#include "Replica.h"
#include "InstanciatorDef.h"
#include "I3DShGroup.h"
#include "IShUtilities.h"
#include "COMUtilities.h"
#include "COM3DUtilities.h"
#include "I3dShUtilities.h"
#include "I3DShShader.h"
#include "IShRasterLayer.h"
#include "I3DShTreeElement.h"
#include "IShComponent.h"
#include "IMFDocument.h"
#include "Geometry.h"
#include "IMFResponder.h"
#include "MCRandom.h"
#include "MiscComUtilsImpl.h"
#include "NameUtilities.h"
#include "I3DShLightSource.h"
#include "I3DExLight.h"
#include "PMapTypes.h"
#include "AnimUtilities.h"
#include "I3DShKeyFrame.h"
#include "I3dShAnimationMethod.h"
#include "MFPartMessages.h"
#include "IMFPart.h"
#include "MCStream.h"
#include "IShTokenStream.h"
#include "I3dShRenderFeature.h"
#include "IShPartUtilities.h"

const MCGUID CLSID_ReplicateCommand(R_CLSID_ReplicateCommand);
const MCGUID CLSID_ReplicateData(R_CLSID_ReplicateData);

const int32 kLayerCount = 4;

#define kSpecularPowerCoef 4096.0f

// PMap
ReplicatePMap::ReplicatePMap()
{
	fReplicateMode = eCube;

	// Cube
	fXCount = 4;
	fXSpacing = 0;
	fYCount = 3;
	fYSpacing = 0;
	fZCount = 1;
	fZSpacing = 0;

	// Cylinder
	fRCount = 6;
	fRadius = 2;
	fCylZCount = 1;
	fCylZSpacing = 0;
	fAngle = 360;
	fTorque = false;

	// Surface 
	fTreePermanentID = 0;
	fSurfaceXDensity = 1;
	fSurfaceYDensity = 1;
	fAlignNormal = false;
	fCubeMap = false;
	fRepartitionMin = 0; 
	fRepartitionMax = 1; 

	// Perturbation Position 
	fMaxNewShaders = 10;
	fPerturpPosX = 0;
	fPerturpPosY = 0;
	fPerturpPosZ = 0;
	fPerturpScaU = 0;
	fPerturpScaX = 0;
	fPerturpScaY = 0;
	fPerturpScaZ = 0;
	fPerturpRotX = 0;
	fPerturpRotY = 0;
	fPerturpRotZ = 0;
	fPerturpShading = 0;

	fPerturbColor = TMCColorRGBA::kWhiteNoAlpha;

	fShaderPosX = -1;
	fShaderPosY = -1;
	fShaderPosZ = -1;
	fShaderScaU = -1;
	fShaderScaX = -1;
	fShaderScaY = -1;
	fShaderScaZ = -1;
	fShaderRotX = -1;
	fShaderRotY = -1;
	fShaderRotZ = -1;
	fShaderShaM = -1;
	fShaderShading = -1;
}

void InitShadingIn(ShadingIn& shadingIn)
{
	shadingIn.fPoint = TVector3::kZero;
	shadingIn.fGNormal = TVector3::kUnitZ;
	shadingIn.fPointLoc = TVector3::kZero;
	shadingIn.fNormalLoc = TVector3::kUnitZ;
	shadingIn.fUV = TVector2::kZero;
	shadingIn.fPointx = TVector3::kUnitX;
	shadingIn.fPointy = TVector3::kUnitY;
	shadingIn.fNormalx = TVector3::kUnitX;
	shadingIn.fNormaly = TVector3::kUnitY;
	shadingIn.fUVx = TVector2::kUnitX;
	shadingIn.fUVy = TVector2::kUnitY;
	shadingIn.fPointLocx = TVector3::kUnitX;
	shadingIn.fPointLocy = TVector3::kUnitY;
	shadingIn.fNormalLocx = TVector3::kUnitX;
	shadingIn.fNormalLocy = TVector3::kUnitY;
	shadingIn.fIsoU = TVector3::kUnitX;
	shadingIn.fIsoV = TVector3::kUnitY;
	shadingIn.fUVSpaceID=0;
	shadingIn.fBumpOn=false;
	shadingIn.fCurrentCompletionMask=0;
	shadingIn.fInstance=NULL;
}

// Perturbation cached data
CacheData::CacheData()
{
	fCachedValue=0;
	fValid=false;
	fMinMaxValid=false;
	fMinMax = TVector2::kZero;
	fInvMaxDist = 1;
	fOrigin = TVector2::kZero;

	// We'll ask a shader on a non existing surface, so we do as if it's a plane
	InitShadingIn(fShadingIn);
}

real32 CacheData::GetValue(const TVector3& pointIn)
{
	if(!fValid)
	{
		boolean fullArea=false;
		fShadingIn.fPoint = fShadingIn.fPointLoc = pointIn;
		fShadingIn.fUV = fInvMaxDist*(pointIn.CastToXY()-fOrigin);
		fShader->GetValue(fCachedValue,fullArea,fShadingIn);
		fCachedValue = 2*fCachedValue - 1; // we need a value between -1 and 1
		fValid = true;
	}
	
	return fCachedValue;
}

const TVector2& CacheData::GetMinMax()
{
	if(!fMinMaxValid)
	{
		fMinMaxValid = true;
	
		// Get randomly some samples to evaluate the min and the max of the map
		real32 min = kRealBig;
		real32 max = -kRealBig;

		fMinMax.x = 0;
		fMinMax.y = 1;

		const int32 precision = 16000;
		for(int32 i=0 ; i<precision ; i++)
		{
			const real32 randomX = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomY = FixedToReal(MCRandom()&0xFFFF);
			const real32 randomZ = FixedToReal(MCRandom()&0xFFFF);
			
			ShadingIn fakeShading;

			fakeShading.fUV.x = randomX;
			fakeShading.fUV.y = randomY;
			
			fakeShading.fPointLoc.x = randomX*1234;
			fakeShading.fPointLoc.y = randomY*1234;
			fakeShading.fPointLoc.z = randomZ*1234;
			
			fakeShading.fPoint.x = randomX*1234;
			fakeShading.fPoint.y = randomY*1234;
			fakeShading.fPoint.z = randomZ*1234;

			// Disable anti aliasing
//			fakeShading.fCurrentCompletionMask |= gCannotSuperSample;

			real32 result=0;
			boolean fullArea = true;
			fShader->GetValue(result, fullArea, fakeShading);

			if(result<min)
				min=result;
			if(result>max)
				max=result;
		}

		real32 amplitude = max-min;

		fMinMax.x = -min/amplitude ;
		fMinMax.y = (1-min)/amplitude;
	}

	return fMinMax;
}

// Callback to prepare the menus

class ReplicateCommandCallBack : public TBasicMenuCallBack
{
public:
	ReplicateCommandCallBack();

	virtual boolean MCCOMAPI SelfPrepareMenu(ISceneDocument* sceneDocument);

private:
/* Doesn't seem to work
	void InitMenuBar();

	boolean fInit;
	*/
};

ReplicateCommandCallBack::ReplicateCommandCallBack()
{
//	fInit = false;
}
/*
void ReplicateCommandCallBack::InitMenuBar()
{	// We build "manually" the items that go inside the Edit Menu,
	// because we just need to insert a couple of items inside it
	if(fInit)
		return;

	int16			menu=0, item=0;
	TMCString255	itemName;
//	f3DEditorHostPart->BuildMenuBar();

	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');

	// Get Duplicate item ID
	gMenuUtilities->MenuActionToMenuItem(cSymetry, menu, item);

	// Add Replicate...
	gResourceUtilities->GetIndString(itemName, kStrings, 5);
	gMenuUtilities->AddMenuItem(menu, itemName, item, kReplicate);

	fInit = true;
}
*/
boolean  ReplicateCommandCallBack::SelfPrepareMenu(ISceneDocument* sceneDocument)
{
//	if(!fInit)
//		InitMenuBar();

	if (!sceneDocument)
		return false;

	// If the toolbar does not exist, build it

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

ReplicateCommand::ReplicateCommand()
{
	// Data from BasicCommand
	fNeedsSelection = true;

	fIsValid = false;

	fUseRepartition = false;

	fPerturbationFlag = 0;

	// Init the shader caches (empty by default)
	// 3 pos, 3 scal, 3 rot
	const int32 cacheCount = 11;
	fCacheID.SetElemCount(cacheCount); 
	for(int32 iID=0 ; iID<cacheCount ; iID++)
	{
		fCacheID[iID] = kNoShader;
	}
}
  
ReplicateCommand::~ReplicateCommand()
{
}

MCCOMErr ReplicateCommand::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ReplicateCommand))
	{
		TMCCountedGetHelper<ReplicateCommand> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
/*	else if (MCIsEqualIID(riid, IID_I3DExWireFrame))
	{
		TMCCountedGetHelper<I3DExWireFrame> result(ppvObj);
		result = static_cast<I3DExWireFrame*>(this);
		return MC_S_OK;
	}*/

	return TBasicSceneCommand::QueryInterface(riid, ppvObj);
}

MCCOMErr ReplicateCommand::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;
	
	fIsValid = false;

	return result;
}

MCCOMErr ReplicateCommand::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	MCCOMErr result = MC_S_OK;

	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();

					switch (partID)
					{
					case 'save':
						{
							// open a file dialog to save the preset

							TMCCountedPtr<IMCFile> file;
							gFileUtilities->CreateIMCFile(&file);

							TMCDynamicString title("Save As");

							TMCArray<IDType> inFileTypes;
							inFileTypes.AddElem('CBRF');

							IDType outFormat=0;

							if (gFileFormatUtilities->SaveFileDialog(title,file,inFileTypes,outFormat) == MC_S_OK)
							{
								// Create a comp with the same values
								TMCCountedPtr<IShComponent> component;
								gComponentUtilities->CreateComponent(kRID_DataComponentFamilyID, 'RepD', &component);

								if (MCVerify(component != NULL))
								{
									TMCCountedPtr<ReplicateSaveData> saveData;
									component->QueryInterface(CLSID_ReplicateData,(void**)&saveData);
									saveData->SetFromReplicateCommand(*this);

									gShellUtilities->WriteComponentToBrowserFile(component,file);
								}
							}
						} break;
					case 'load':
						{
							// open a file dialog to save the preset

							TMCCountedPtr<IMCFile> file;
							gFileUtilities->CreateIMCFile(&file);

							TMCDynamicString title("Open");

							TMCArray<IDType> inFileTypes;
							inFileTypes.AddElem('CBRF');

							IDType outFormat=0;
							IDType inOutPopupSelection=0;

							if (gFileFormatUtilities->OpenFileDialog(title,file,inFileTypes,inOutPopupSelection,outFormat) == MC_S_OK)
							{
								// Create a comp with the same values
								TMCCountedPtr<IShComponent> component;
								gComponentUtilities->CreateComponent(kRID_DataComponentFamilyID, 'RepD', &component);

								if (MCVerify(component != NULL))
								{
									gShellUtilities->LoadComponentDataFromBrowserFile(component,file);

									TMCCountedPtr<ReplicateSaveData> saveData;
									component->QueryInterface(CLSID_ReplicateData,(void**)&saveData);
									saveData->SetToReplicateCommand(*this);
								}
							}
						} break;
					}
				}
			}
		}
	}

	return result;
}

void ReplicateCommand::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
	TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);

	result = new ReplicateCommandCallBack;
}

MCCOMErr ReplicateCommand::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;

	if (fSceneDocument)
	{
		fSceneDocument->GetSceneSelection(&fSelection);
		fSelection->Clone(&fCloneSelection);

		fSceneDocument->GetScene(&fScene);

		fScene->GetTreePropertyChangeChannel(&fTreePropertyChannel);
		ThrowIfNil(fTreePropertyChannel);

		fSceneDocument->GetSceneSelectionChannel(&fSelectionChannel);
		ThrowIfNil(fSelectionChannel);
	}

	return MC_S_OK;
}

// This method is called before the dialog opens
MCCOMErr ReplicateCommand::Prepare()
{
	if(!IsSerialValid())
	{	// Invalid serial number: tell the user it's a demo version
		CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');
		TMCDynamicString message;
		gResourceUtilities->GetIndString( message, kStrings, 9);
		gPartUtilities->Alert(message);
	}

	// Prepare a plausible radius value based on the bbox of the 1st selected obj
	TTreeSelectionIterator iter(fCloneSelection);	
	I3DShTreeElement* firstTree=iter.First();
	{
		TBBox3D bbox;
		firstTree->GetBoundingBox(bbox, kApplyAllDeformersAtInstanceLevel);
		TTreeTransform firstTreeTrans;
		firstTree->GetGlobalTreeTransform(firstTreeTrans);
		firstTree->GetBoundingBox(bbox, kApplyAllDeformersAtInstanceLevel);
		bbox = firstTreeTrans.TransformBBox(bbox);
		fPMap.fRadius = 2*bbox.GetMax();
	}

	return MC_S_OK;
}

int32 ReplicateCommand::AddShader(const int32 shaderID)
{
	if(shaderID<0)
		return kNoShader;

	int32 cacheIndex = kUnusedIndex;
	const int32 cacheCount = fCacheArray.GetElemCount();
	for(int32 iCache=0 ; iCache<cacheCount ; iCache++)
	{
		if(fCacheArray[iCache].fShaderIndex == shaderID)
			return iCache;
	}

	// Shader not found in the cache, add a new one
	TMCCountedPtr<I3DShMasterShader> perturbMasterShader;
	fScene->GetMasterShaderByIndex(&perturbMasterShader, shaderID);
	if(perturbMasterShader)
	{
		fCacheArray.AddElemCount(1);
		perturbMasterShader->GetShader(&(fCacheArray[cacheCount].fShader));
		return cacheCount;
	}

	return kNoShader;
}

void ReplicateCommand::Validate()
{
	if(!fIsValid)
	{
		fIsValid = true;

		// Perturbation flag
		fPerturbationFlag = 0;
		if(fPMap.fPerturpPosX) fPerturbationFlag|=ePosX;
		if(fPMap.fPerturpPosY) fPerturbationFlag|=ePosY;
		if(fPMap.fPerturpPosZ) fPerturbationFlag|=ePosZ;
		if(fPMap.fPerturpScaU) fPerturbationFlag|=eScaU;
		if(fPMap.fPerturpScaX) fPerturbationFlag|=eScaX;
		if(fPMap.fPerturpScaY) fPerturbationFlag|=eScaY;
		if(fPMap.fPerturpScaZ) fPerturbationFlag|=eScaZ;
		if(fPMap.fPerturpRotX) fPerturbationFlag|=eRotX;
		if(fPMap.fPerturpRotY) fPerturbationFlag|=eRotY;
		if(fPMap.fPerturpRotZ) fPerturbationFlag|=eRotZ;
		if(fPMap.fPerturpShading) fPerturbationFlag|=eShad;

		// Check the perturbation shaders
		fCacheArray.ArrayFree();
		fCacheID[0] = AddShader(fPMap.fShaderPosX);
		fCacheID[1] = AddShader(fPMap.fShaderPosY);
		fCacheID[2] = AddShader(fPMap.fShaderPosZ);
		fCacheID[3] = AddShader(fPMap.fShaderScaX);
		fCacheID[4] = AddShader(fPMap.fShaderScaY);
		fCacheID[5] = AddShader(fPMap.fShaderScaZ);
		fCacheID[6] = AddShader(fPMap.fShaderRotX);
		fCacheID[7] = AddShader(fPMap.fShaderRotY);
		fCacheID[8] = AddShader(fPMap.fShaderRotZ);
		fCacheID[9] = AddShader(fPMap.fShaderScaU);
		fCacheID[10] = AddShader(fPMap.fShaderShaM);

		// The new trees are placed in a group, create one
		TTreeSelectionIterator iter(fCloneSelection);	
		I3DShTreeElement* firstTree=iter.First();

		TMCCountedPtr<I3DShGroup> group;
		gComponentUtilities->CoCreateInstance(CLSID_StandardGroup, NULL, 1, IID_I3DShGroup, (void **)&group);
		ThrowIfNil(group);
		group->QueryInterface(IID_I3DShTreeElement, (void**)&fNewGroup);
		TMCDynamicString treeName;
		firstTree->GetName(treeName);
		TMCDynamicString replicate;
		gResourceUtilities->GetIndString(replicate, kStrings, 4 );
		treeName+=replicate;
		// Make the group name unique
		GetUniqueTreeNameByBaseName(fScene,treeName,treeName);
		fNewGroup->SetName(treeName);
		firstTree->InsertRight(fNewGroup);

		if(fPMap.fReplicateMode==eSurface)
		{
			if(fPMap.fRepartitionMin != 0 || fPMap.fRepartitionMax!=1)
			{
				// Get the target shader
//				TMCCountedPtr<I3DShTreeElement> targetTree;
//				fScene->GetTreeByPermanentID( &targetTree, fPMap.fTreePermanentID);
				I3DShTreeElement* targetTree = fScene->GetTreeByIndex( fPMap.fTreePermanentID);
				if(!targetTree)
					return;
	
				TMCCountedPtr<I3DShInstance> instance;
				targetTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
				if(!instance)
					return;

				TMCCountedPtr<I3DShMasterShader> masterShader;
				instance->GetShader(&masterShader);	
				if(!masterShader)
					return;

				masterShader->GetShader(&fRepartitionShader);

				fUseRepartition = (fRepartitionShader!=NULL);
				if(fUseRepartition)
					fImplementedOutput = fRepartitionShader->GetImplementedOutput();
			}
		}

	}
}

boolean ReplicateCommand::Do()
{
	fSelection->ClearSelection();
	
	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');

	Validate();

	TMCString255 progressString;
	gResourceUtilities->GetIndString(progressString, kStrings, 5);
	gShellUtilities->BeginProgress(progressString, &fProgressKey);

/*	{	// Deselect everything
		TTreeSelectionIterator selectionIter(fSelection);
		for(I3DShTreeElement* tree = selectionIter.First(); tree!=NULL ; tree = selectionIter.Next())
		{
			tree->PostMoveChange(); // here, we make sure that this tree element is in the triple buffer
		}
	}*/

	// Work at t=0
	real32 fTime;
	MicroTick currentTime = fScene->GetTime(&fTime);
	fScene->SetTime (0, false);
	
	{	// Replicate
		switch(fPMap.fReplicateMode)
		{
		case eCube:
			{
				CubeReplicate();
			} break;
		case eCylinder:
			{
				CylinderReplicate();
			} break;
		case eSurface:
			{
				SurfaceReplicate();
			} break;
		}
	}

	{
		// Close the group
		fNewGroup->OpenClose(false);
		fNewGroup->CenterHotPointOnElement();

		// Select the new group element
		TMCCountedPtr<ISelectableObject> newSelectable;
		fNewGroup->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
		newSelectable->AddToSelection(fSelection,true);
		// to redraw
		fNewGroup->PostMoveChange();
	}

	// Reset time
	fScene->SetTime (currentTime, false);

/*	{	// tell the Shell what object must be redrawn (it can not guess, but we easily can tell it)
		// Note :	using the selection-change-channel makes the interactive renderer add this I3DShTreeElement to the triple buffer, 
		//			and therefore redraws correctly.
		TTreeSelectionIterator selectionIter(fSelection);
		for(I3DShTreeElement* tree = selectionIter.First(); tree!=NULL ; tree = selectionIter.Next())
		{
			tree->PostMoveChange(); // here, we make sure that this tree element is in the triple buffer
		}
	}*/

	gShellUtilities->EndProgress(fProgressKey);

// maybe	gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	return true;
}

boolean ReplicateCommand::Undo()
{
	fSceneDocument->GetSceneSelection(&fSelection);

	// Clear the selection
	I3DShTreeElement* tree = NULL;
	TTreeSelectionIterator	selectionIter(fSelection);
	for (tree = selectionIter.First(); tree!=NULL ; tree = selectionIter.Next())
	{
		tree->PostMoveChange();
	}

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	fSelection->ClearSelection();

	// Prepare the redo info
	fRelinkInfo.ArrayFree();

	// Remove the new items from the scene
	// Tree elem
	TMCCountedPtrArray<I3DShTreeElement>::iterator	iter(fNewTrees);
	for (tree = iter.First(); iter.More(); tree = iter.Next())
	{
		RelinkTreeElementInfo info;
		TMCCountedPtr<I3DShTreeElement> unlinkedTree;
		tree->Unlink(&unlinkedTree, &info);
		fRelinkInfo.AddElem(info);
		gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, tree);
	}
	// The new group
	if(fNewGroup)
	{
		TMCCountedPtr<I3DShTreeElement> unlinkedTree;
		fNewGroup->Unlink(&unlinkedTree, &fRelinkGroupInfo);
		gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, fNewGroup);
	}
	// Master shaders
	if (fNewShaders.GetElemCount())
	{	
		TMCCountedPtrArray<I3DShMasterShader>::iterator shaders(fNewShaders);
		for (I3DShMasterShader* shader = shaders.First(); shaders.More(); shader = shaders.Next())
		{
			fScene->RemoveMasterShader(shader);
		}
	}

	// Restore the initial selection state
	fSelection->Select(fCloneSelection);

// maybe	gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	return true;
}

boolean ReplicateCommand::Redo()
{
	fSceneDocument->GetSceneSelection(&fSelection);

	// Clear the selection
	TTreeSelectionIterator	selectionIter(fSelection);
	for (I3DShTreeElement* tree = selectionIter.First(); tree!=NULL ; tree = selectionIter.Next())
	{
		tree->PostMoveChange();
	}

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	fSelection->ClearSelection();

	// Redo

	// Restore the group
	TTreeTransform globalT;
	fRelinkGroupInfo.fTreeElement->GetGlobalTreeTransform(globalT);
	fRelinkGroupInfo.fTreeElement->Relink(&fRelinkGroupInfo);
	fRelinkGroupInfo.fTreeElement->SetGlobalTreeTransform(globalT);

	TMCArray<RelinkTreeElementInfo>::iterator iter(fRelinkInfo);
	for (RelinkTreeElementInfo* info=iter.Last(); iter.MorePrev(); info=iter.Prev())
	{
		// Restore the instances position
		TTreeTransform globalT;
		info->fTreeElement->GetGlobalTreeTransform(globalT);
		info->fTreeElement->Relink(info);
		info->fTreeElement->SetGlobalTreeTransform(globalT);
		gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeAdded, info->fTreeElement);

		// Select the new tree element
//		TMCCountedPtr<ISelectableObject> newSelectable;
//		info->fTreeElement->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
//		newSelectable->AddToSelection(fSelection,true);

	}
	// Select the group
	TMCCountedPtr<ISelectableObject> newSelectable;
	fRelinkGroupInfo.fTreeElement->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
	newSelectable->AddToSelection(fSelection,true);
	gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeAdded, fRelinkGroupInfo.fTreeElement);

	// Master shaders
/* Apparently not necessary, they appear twice when this is called
	if (fNewShaders.GetElemCount())
	{	
		TMCCountedPtrArray<I3DShMasterShader>::iterator shaders(fNewShaders);
		for (I3DShMasterShader* shader = shaders.First(); shaders.More(); shader = shaders.Next())
		{
			fScene->InsertMasterShader(shader);
		}
	}*/


// maybe	gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	return true;
}

bool ReplicateCommand::CloneTree(I3DShTreeElement* iTree, I3DShTreeElement** newTree)
{
	// See if its a group
	TMCCountedPtr<I3DShGroup> group;
	iTree->QueryInterface(IID_I3DShGroup, (void**)&group);
	if(group)
	{
		iTree->CloneSubTree(kWithAnim, newTree);
	}
	else
	{
		iTree->ComClone(newTree, kWithAnim, true); // Clone the modifiers and other data
	}

	return (newTree!=NULL);
}


void ReplicateCommand::CubeReplicate()
{
	const int32 selectCount = fCloneSelection->GetObjectCount();
	const real32 inc = 100.0f/(real32)(selectCount*fPMap.fZCount*fPMap.fYCount);

	// Count to name the shaders properly
	int32 count = 0;

	TTreeSelectionIterator iter(fCloneSelection);
	for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
	{
		// Using the bounding box of the tree and the spacing value, compute the
		// delta X, Y and Z
		TTreeTransform iTreeTrans;
		iTree->GetGlobalTreeTransform(iTreeTrans);
		TBBox3D bbox;
		iTree->GetBoundingBox(bbox, kApplyAllDeformersAtInstanceLevel);
		bbox = iTreeTrans.TransformBBox(bbox);
		const real32 deltaX = bbox.GetWidth() + fPMap.fXSpacing;
		const real32 deltaY = bbox.GetHeight() + fPMap.fYSpacing;
		const real32 deltaZ = bbox.GetDepth() + fPMap.fZSpacing;

		const real32 perturbationFactor = MC_Max(deltaX,deltaY,deltaZ);

		const real32 invMaxXY = 1.0f/MC_Max(fPMap.fXCount*deltaX,fPMap.fYCount*deltaY);
		TVector3 center;
		bbox.GetCenter(center);
		const TVector2 origin( center.x - .5*bbox.GetWidth(), center.y - .5*bbox.GetHeight() );
		const int32 cacheCount = fCacheArray.GetElemCount();
		for(int32 iCache=0 ; iCache<cacheCount ; iCache++ )
		{
			fCacheArray[iCache].fInvMaxDist = invMaxXY;
			fCacheArray[iCache].fOrigin = origin;
		}
	
		// Noise the shading
		TMCCountedPtr<I3DShMasterShader> instanceMasterShader;
		int32 colorParamID = 0;
		TMCColorRGB originalLightColor;
		if(fPerturbationFlag&eShad)
		{
			TMCCountedPtr<I3DShInstance> instance;
			iTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(instance->GetInstanceKind() == I3DShInstance::kLightInstance)
				colorParamID = GetColorParamID(instance, originalLightColor);
			else
				instance->GetShader(&instanceMasterShader);
		}

		TTreeTransform iTreeLocalTransform;
		iTree->GetLocalTreeTransform(iTreeLocalTransform);
		const TVector3 originalOffset = iTreeLocalTransform.GetOffset();

		DemoLimiter limiter;

		for(int32 iZ=0 ; iZ<fPMap.fZCount ; iZ++)
		{
			for(int32 iY=0 ; iY<fPMap.fYCount ; iY++)
			{
				for(int32 iX=0 ; iX<fPMap.fXCount ; iX++)
				{
					if(!limiter.CanAdd())
					{
						// Reach the limit set by the demo mode
						return;
					}

					// Make a new tree elem
					TMCCountedPtr<I3DShTreeElement>	newTree;
					if( !CloneTree(iTree , &newTree ) )
						return;
				
					// Set the transform
					TVector3 offset = originalOffset;

					offset[0] += ((real32)iX) * deltaX;
					offset[1] += ((real32)iY) * deltaY;
					offset[2] += ((real32)iZ) * deltaZ;

					TTreeTransform treeTransform;
					newTree->GetLocalTreeTransform(treeTransform);
					if( !ApplyTransformNoise(offset, treeTransform, perturbationFactor, fPerturbationFlag) )
						continue;

					newTree->SetLocalTreeTransform(treeTransform);

					// Now we also offset all the key frames
					const TVector3 currentOffset = treeTransform.GetOffset() - originalOffset;
					OffsetAnimation(newTree, currentOffset);

					if(fPerturbationFlag&eShad)
					{
						TMCCountedPtr<I3DShInstance> instance;
						newTree->QueryInterface(IID_I3DShInstance, (void**) &instance);
						if(colorParamID!=0) // special case: we're replicating a light: modify its color
							NoiseLightColor(instance, offset, PertubShadingValue(offset),colorParamID, originalLightColor, count++);
						else if(instanceMasterShader)
							NoiseMasterShader(instance, instanceMasterShader, PertubShadingValue(offset), count);
					}
			
					// Insert in the scene
					fNewGroup->InsertLast(newTree);

					// Store for undo
					fNewTrees.AddElem(newTree);

					// Select the new tree element
//					TMCCountedPtr<ISelectableObject> newSelectable;
//					newTree->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
//					newSelectable->AddToSelection(fSelection,true);
				}
			
				// Increment progress bar
				gShellUtilities->IncrementProgress(inc, fProgressKey);
				if (gShellUtilities->CheckForUserCancel())
					return;
			}
		}
	}
}

void ReplicateCommand::CylinderReplicate()
{
	const int32 selectCount = fCloneSelection->GetObjectCount();
	const real32 inc = 100.0f/(real32)(selectCount*fPMap.fCylZCount);

	// Count to name the shaders properly
	int32 count = 0;

	TTreeSelectionIterator iter(fCloneSelection);
	for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
	{
		// Using the bounding box of the tree and the spacing value, compute the
		// delta X, Y and Z
		TTreeTransform iTreeTrans;
		iTree->GetGlobalTreeTransform(iTreeTrans);
		TBBox3D bbox;
		iTree->GetBoundingBox(bbox, kApplyAllDeformersAtInstanceLevel);
		bbox = iTreeTrans.TransformBBox(bbox);
		const real32 deltaZ = bbox.GetDepth() + fPMap.fCylZSpacing;
		real32 deltaPhi=0;
		if(RadToDeg(fPMap.fAngle)>359)
			deltaPhi = fPMap.fAngle/(real32)fPMap.fRCount;
		else if(fPMap.fRCount>1)// just a security test
			deltaPhi = fPMap.fAngle/(real32)(fPMap.fRCount-1);

		const real32 perturbationFactor = MC_Max(deltaPhi*fPMap.fRadius,deltaZ);

		const real32 invMaxXY = .5f/fPMap.fRadius;
		TVector3 center;
		bbox.GetCenter(center);
		const TVector2 origin( center.x - fPMap.fRadius, center.y - fPMap.fRadius );
		const int32 cacheCount = fCacheArray.GetElemCount();
		for(int32 iCache=0 ; iCache<cacheCount ; iCache++ )
		{
			fCacheArray[iCache].fInvMaxDist = invMaxXY;
			fCacheArray[iCache].fOrigin = origin;
		}
	
		// Noise the shading
		TMCCountedPtr<I3DShMasterShader> instanceMasterShader;
		TVector2 minMax;
		int32 colorParamID = 0;
		TMCColorRGB originalLightColor;
		if(fPerturbationFlag&eShad)
		{
			TMCCountedPtr<I3DShInstance> instance;
			iTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(instance->GetInstanceKind() == I3DShInstance::kLightInstance)
				colorParamID = GetColorParamID(instance, originalLightColor);
			else
				instance->GetShader(&instanceMasterShader);

			minMax = GetMinMax();
		}

		TTreeTransform iTreeLocalTransform;
		iTree->GetLocalTreeTransform(iTreeLocalTransform);
		const TVector3 originalOffset = iTreeLocalTransform.GetOffset();

		DemoLimiter limiter;

		for(int32 iZ=0 ; iZ<fPMap.fCylZCount ; iZ++)
		{
			for(int32 iR=0 ; iR<fPMap.fRCount ; iR++)
			{
				if(!limiter.CanAdd())
				{
					// Reach the limit set by the demo mode
					return;
				}

				// Make a new tree elem
				TMCCountedPtr<I3DShTreeElement>	newTree;
				if( !CloneTree(iTree , &newTree ) )
					return;

				// Set the transform
				TVector3 offset = originalOffset;

				const real32 phi = iR*deltaPhi;
				const real32 cosPhi = RealCos(phi);
				const real32 sinPhi = RealSin(phi);
			
				TTreeTransform treeTransform;
				newTree->GetLocalTreeTransform(treeTransform);
				if(fPMap.fTorque)
				{
					treeTransform.RotateAxis(treeTransform.GetHotPoint(), -sinPhi, cosPhi, TVector3::kUnitZ );
				}
			
				offset[0] += fPMap.fRadius * cosPhi;
				offset[1] += fPMap.fRadius * sinPhi;
				offset[2] += ((real32)iZ) * deltaZ;

				if( !ApplyTransformNoise(offset, treeTransform, perturbationFactor, fPerturbationFlag) )
					continue;

				newTree->SetLocalTreeTransform(treeTransform);
	
				// Now we also offset all the key frames
				const TVector3 currentOffset = treeTransform.GetOffset() - originalOffset;
				OffsetAnimation(newTree, currentOffset);

				if(fPerturbationFlag&eShad)
				{
					TMCCountedPtr<I3DShInstance> instance;
					newTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
					if(colorParamID!=0) // special case: we're replicating a light: modify its color
						NoiseLightColor(instance, offset, PertubShadingValue(offset),colorParamID, originalLightColor, count++);
					else if(instanceMasterShader)
						NoiseMasterShader(instance, instanceMasterShader, PertubShadingValue(offset), count);
				}
			
				// Insert in the scene
				fNewGroup->InsertLast(newTree);

				// Store for undo
				fNewTrees.AddElem(newTree);
			
				// Select the new tree element
//				TMCCountedPtr<ISelectableObject> newSelectable;
//				newTree->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
//				newSelectable->AddToSelection(fSelection,true);
			}
			
			// Increment progress bar
			gShellUtilities->IncrementProgress(inc, fProgressKey);
			if (gShellUtilities->CheckForUserCancel())
				return;
		}
	}
}

boolean ReplicateCommand::HitPos( RayHitParameters& hitParams,
			   I3DShInstance* instance,
			   TVector3& localHitPos )
{
	if( instance->RayHit(hitParams) )
	{
		localHitPos = hitParams.ray->fOrigin + hitParams.hit->ft*hitParams.ray->fDirection;
		hitParams.hit->fPointLoc = localHitPos;

		ShadingFlags flags;
		if(fUseRepartition)
		{
			fRepartitionShader->GetShadingFlags(flags);
		}
		// Replica needs the normal, force it computation
		flags.fNeedsNormal = true;
		// Some shaders needs other info, make the full computation
		const boolean normalDerivative = flags.fNeedsNormalDerivative||flags.fNeedsNormalLocDerivative;
		hitParams.hit->fShouldSetNormalDerivative=normalDerivative;
		hitParams.hit->CalcInfo(flags, normalDerivative, (const Ray3D &)(*(hitParams.hit)));

		return true;
	}

	return false;
}

boolean ReplicateCommand::IsInRepartitionArea(ShadingIn& shadingIn)
{
	if(fUseRepartition)
	{
		// Check that the shading color is in
		// our range
		boolean fullArea=false;
		if(fImplementedOutput&kUsesGetValue)
		{
			real32 result=0;
			fRepartitionShader->GetValue(result, fullArea, shadingIn);
			if( result<fPMap.fRepartitionMin)
				return false;
			if( result>fPMap.fRepartitionMax)
				return false;
		}
		else if(fImplementedOutput&kUsesGetColor)
		{
			TMCColorRGBA result;

			fRepartitionShader->GetColor(result, fullArea, shadingIn);
			const real32 intensity = result.Intensity();
			if( intensity<fPMap.fRepartitionMin)
				return false;
			if( intensity>fPMap.fRepartitionMax)
				return false;
		}
		else if(fImplementedOutput&kUsesDoShade)
		{
			ShadingOut result;
			fRepartitionShader->DoShade(result, shadingIn);
			const real32 intensity = result.fColor.Intensity();
			if( intensity<fPMap.fRepartitionMin)
				return false;
			if( intensity>fPMap.fRepartitionMax)
				return false;
		}
	}

	return true;
}

void ReplicateCommand::SurfaceReplicate()
{
	if(fPMap.fCubeMap)
	{
		SurfaceReplicate(eMinusXDir, true);
		SurfaceReplicate(eXDir, true);
		SurfaceReplicate(eMinusYDir, true);
		SurfaceReplicate(eYDir, true);
		SurfaceReplicate(eMinusZDir, true);
		SurfaceReplicate(eZDir, true);
	}
	else
		SurfaceReplicate(eMinusZDir, false);
}

TVector3 GetProjDir(const EProjection projType)
{
	switch(projType)
	{
	case eXDir:			return TVector3::kUnitX;
	case eMinusXDir:	return -TVector3::kUnitX;
	case eYDir:			return TVector3::kUnitY;
	case eMinusYDir:	return -TVector3::kUnitY;
	case eZDir:			return TVector3::kUnitZ;
	case eMinusZDir:	return -TVector3::kUnitZ;
	default:			return -TVector3::kUnitZ;
	}
}

void GetBoxDimensions(const EProjection projType,
				 const TBBox3D& bbox,
				 real32& width,
				 real32& height,
				 real32& depth,
				 const boolean forceOnZ = false) // use this when the fAlignNormal is used
{
	if(forceOnZ)
	{
		width = bbox.GetWidth();
		height = bbox.GetHeight();
		depth = bbox.GetDepth();
		return;
	}

	switch(projType)
	{
	case eXDir:
	case eMinusXDir:
		{
			width = bbox.GetHeight();
			height = bbox.GetDepth();
			depth = bbox.GetWidth();
		} break;
	case eYDir:
	case eMinusYDir:
		{
			width = bbox.GetDepth();
			height = bbox.GetWidth();
			depth = bbox.GetHeight();
		} break;
	case eZDir:
	case eMinusZDir:
		{
			width = bbox.GetWidth();
			height = bbox.GetHeight();
			depth = bbox.GetDepth();
		} break;
	}
}

real GetIndexesAndFactor(const EProjection projType,
						 int32&	xIndex,int32&	yIndex,int32&	zIndex)
{
	real factor = 1;
	switch(projType)
	{
	case eMinusXDir: factor = -1;
	case eXDir:
		{
			xIndex = 1;
			yIndex = 2;
			zIndex = 0;
		} break;
	case eMinusYDir: factor = -1;
	case eYDir:
		{
			xIndex = 2;
			yIndex = 0;
			zIndex = 1;
		} break;
	case eMinusZDir: factor = -1;
	case eZDir:
		{
			xIndex = 0;
			yIndex = 1;
			zIndex = 2;
		} break;
	}

	return factor;
}

void ReplicateCommand::SurfaceReplicate(const EProjection projType, const boolean toCenter)
{
	int32 xIndex, yIndex, zIndex;
	real factor = GetIndexesAndFactor(projType, xIndex, yIndex, zIndex);

	const int32 selectCount = fCloneSelection->GetObjectCount();

	// Get the surface to use
//	TMCCountedPtr<I3DShTreeElement> targetTree;
//	fScene->GetTreeByPermanentID( &targetTree, fPMap.fTreePermanentID);
	I3DShTreeElement* targetTree = fScene->GetTreeByIndex( fPMap.fTreePermanentID);
	if(!targetTree)
		return;

	TMCCountedPtr<I3DShInstance> targetInstance;
	targetTree->QueryInterface(IID_I3DShInstance, (void**) &targetInstance); 

	// get its bbox to build a density function
	TTreeTransform targetTreeGlobalTrans;
	targetTree->GetGlobalTreeTransform(targetTreeGlobalTrans);

	TBBox3D targetBbox;
	targetTree->GetBoundingBox(targetBbox, kApplyAllDeformersAtInstanceLevel);

	const TBBox3D worldTargetBbox = targetTreeGlobalTrans.TransformBBox(targetBbox);

	TVector3 localTargetCenter;
	targetBbox.GetCenter(localTargetCenter);
	TVector3 worldTargetCenter;
	worldTargetBbox.GetCenter(worldTargetCenter);
	real32 worldTargetU, worldTargetV, worldTargetW;
	GetBoxDimensions(projType, worldTargetBbox, worldTargetU, worldTargetV, worldTargetW);

	const real32 invMaxXY = 1.0f/MC_Max(worldTargetU,worldTargetV);
	const int32 cacheCount = fCacheArray.GetElemCount();
	for(int32 iCache=0 ; iCache<cacheCount ; iCache++ )
	{
		fCacheArray[iCache].fInvMaxDist = invMaxXY;
	}

	// Prepare the Ray
	const TVector3 worldProjDir = GetProjDir(projType);
	// The ray hit is done in local coordinates, so transform the dir
	TVector3 localProjDir;
	targetTreeGlobalTrans.InverseTransformV(worldProjDir, localProjDir);
	Ray3D localRay(TVector3::kZero,localProjDir);

	RayHit3D localHit;
	RayHitParameters hitParams(&localHit, &localRay);
	hitParams.hit->fInstance = targetInstance; // for terrain shader, they crash otherwise 
	// TO be sure to hit everything, we start the ray a little bit outside the bbox and finish 
	// a little bit after the bbox
	const real32 margin = MC_Max(.05f*MC_Min(worldTargetU,worldTargetV,worldTargetW), kRealEpsilon);
	hitParams.tmin = 0;
	hitParams.tmax = worldTargetW+2*margin;


	// Count to name the shader properly
	int32 count = 0;

	// get its shader to adjust the density 

	TTreeSelectionIterator iter(fCloneSelection);
	for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
	{
		// Using the bounding box of the tree and the spacing value, compute the
		// delta X, Y and Z
		TTreeTransform iTreeGlobalTrans;
		iTree->GetGlobalTreeTransform(iTreeGlobalTrans);

		TVector3 iTreeGlobalHotPoint;
		iTreeGlobalTrans.Transform(iTreeGlobalTrans.GetHotPoint(), iTreeGlobalHotPoint);

		TBBox3D iTreeBbox;
		iTree->GetBoundingBox(iTreeBbox, kApplyAllDeformersAtInstanceLevel);
		const TBBox3D iTreeWorldBbox = iTreeGlobalTrans.TransformBBox(iTreeBbox);

		real32 iTreeWorldU, iTreeWorldV, iTreeWorldW;
		GetBoxDimensions(projType, iTreeWorldBbox, iTreeWorldU, iTreeWorldV, iTreeWorldW, fPMap.fAlignNormal);

		int32 uCount = (int32)(fPMap.fSurfaceXDensity * worldTargetU/iTreeWorldU);
//		const real32 deltaU = iTreeWorldU + (worldTargetU - uCount*iTreeWorldU)/(uCount - 1);
		const real32 deltaU = iTreeWorldU + (worldTargetU - uCount*iTreeWorldU)/(uCount);

		int32 vCount = (int32)(fPMap.fSurfaceYDensity * worldTargetV/iTreeWorldV);
//		const real32 deltaV = iTreeWorldV + (worldTargetV - vCount*iTreeWorldV)/(vCount - 1);
		const real32 deltaV = iTreeWorldV + (worldTargetV - vCount*iTreeWorldV)/(vCount);

		// Progress bar
		const real32 inc = 100.0f/(real32)(selectCount*vCount);
// Later maybe		const real32 deltaZ = bbox.GetDepth() + fPMap.fZSpacing;
	
		const real32 perturbationFactor = MC_Max(deltaU,deltaV);

		const TVector3& iTreeGlobalTranslation = iTreeGlobalTrans.GetOffset();

		// Compute the first corner pos in UV
		TVector3 startPos = TVector3::kZero;
//		startPos[xIndex] = worldTargetCenter[xIndex] - .5*(worldTargetU-iTreeWorldU);
//		startPos[yIndex] = worldTargetCenter[yIndex] - .5*(worldTargetV-iTreeWorldV);
		startPos[xIndex] = worldTargetCenter[xIndex] - .5*(worldTargetU-deltaU);
		startPos[yIndex] = worldTargetCenter[yIndex] - .5*(worldTargetV-deltaV);
		startPos[zIndex] = worldTargetCenter[zIndex] - factor*(.5*worldTargetW + margin);

	
		// Get the shading to noise it
		TMCCountedPtr<I3DShMasterShader> instanceMasterShader;
		int32 colorParamID = 0;
		TMCColorRGB originalLightColor;
		if(fPerturbationFlag&eShad)
		{
			TMCCountedPtr<I3DShInstance> instance;
			iTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(instance->GetInstanceKind() == I3DShInstance::kLightInstance)
				colorParamID = GetColorParamID(instance, originalLightColor);
			else
				instance->GetShader(&instanceMasterShader);
		}

		// Get original offset to offset the keyframes in the copies
		TTreeTransform iTreeLocalTransform;
		iTree->GetLocalTreeTransform(iTreeLocalTransform);
		const TVector3 originalOffset = iTreeLocalTransform.GetOffset();

		DemoLimiter limiter;

// Later maybe		for(int32 iZ=0 ; iZ<fPMap.fZCount ; iZ++)
		{
			for(int32 iV=0 ; iV<vCount ; iV++)
			{
				for(int32 iU=0 ; iU<uCount ; iU++)
				{
					if(!limiter.CanAdd())
					{
						// Reach the limit set by the demo mode
						return;
					}

					TVector3 worldRayOrigin = TVector3::kZero;

					worldRayOrigin[xIndex] = startPos[xIndex] + ((real32)iU) * deltaU;
					worldRayOrigin[yIndex] = startPos[yIndex] + ((real32)iV) * deltaV;
					worldRayOrigin[zIndex] = startPos[zIndex];// + targetBbox.fMax.z;

					// Noise the XY position before the ray hit
// TO DO: adapt the perturbation in function of the indexX, Y and Z
					if(fPerturbationFlag&ePosX)
						PerturbPos(worldRayOrigin,perturbationFactor*fPMap.fPerturpPosX, xIndex);
					if(fPerturbationFlag&ePosY)
						PerturbPos(worldRayOrigin,perturbationFactor*fPMap.fPerturpPosY, yIndex);
						
					// In global coordinates, the ray start from worldRayOrigin, and goes
					// toward worldProjDir or toward the center of the bbox. 
					// We need to bring back this coordinates into local space for the ray hit function
					TVector3 localRayOrigin = worldRayOrigin;

					targetTreeGlobalTrans.InverseTransform(worldRayOrigin, localRayOrigin);

					// Get the z altitude with RayHit
					hitParams.ray->fOrigin = localRayOrigin;
					if(toCenter)
					{	// do not use the vertical direction, but one from the point on the surface of the bbox
						// toward the center
						TVector3 direction = localTargetCenter-localRayOrigin;
						direction.Normalize();
						hitParams.ray->fDirection = direction;
					}

					TVector3 localHit=localRayOrigin;
					if( HitPos(hitParams, targetInstance, localHit) )
					{
						if(!IsInRepartitionArea(*hitParams.hit))
							continue;

						TVector3 globalHit;
						targetTreeGlobalTrans.Transform(localHit,globalHit);

						TVector3 treeTranslation = globalHit - iTreeGlobalTranslation;

						// Make a new tree elem
						TMCCountedPtr<I3DShTreeElement>	newTree;
						if( !CloneTree(iTree , &newTree ) )
							return;
					
						// Set the transform
					
						// If we need the normal
						TTreeTransform newTreeTransform = iTreeLocalTransform;
						if(fPMap.fAlignNormal)
						{
							TVector3 globalNormal;
							targetTreeGlobalTrans.TransformV(hitParams.hit->fNormalLoc, globalNormal);
							globalNormal.Normalize();

							if(hitParams.hit->fNormalFlipped) // I don't understand why the normal seems always flipped ?
								globalNormal*=-1;

							const real32 dotProduct = globalNormal*TVector3::kUnitZ;
							if(globalNormal*TVector3::kUnitZ<-(1-kRealEpsilon))
							{	// 180 degrees rotation: we use this  methode because the 
								// other one is bugged
								newTreeTransform.RotateAxis(iTreeGlobalHotPoint, 
									0, -1, TVector3::kUnitX);
							}
							else if(globalNormal*TVector3::kUnitZ<(1-kRealEpsilon))
							{
								TVector3 axis = globalNormal^TVector3::kUnitZ;
								if(axis.GetMagnitudeSquared()<kRealEpsilon)
									axis = TVector3::kUnitX;
								newTreeTransform.Rotate(iTreeGlobalTranslation /*+ newTreeTransform.GetHotPoint()*/,
								   		  TVector3::kUnitZ , globalNormal , axis );
							}

							newTreeTransform.Translate(treeTranslation);
						}
						else
						{	// Just offset it
							newTreeTransform.Translate(treeTranslation);
						}

						if( !ApplyTransformNoise(newTreeTransform.GetOffset(), 
							newTreeTransform, 
							perturbationFactor,
							fPerturbationFlag&ePerturbAllButXY) )
							continue;
							
						newTree->SetGlobalTreeTransform(newTreeTransform);
				
						// Now we also offset all the key frames
						const TVector3 currentOffset = newTreeTransform.GetOffset() - originalOffset;
						OffsetAnimation(newTree, currentOffset);

						if(fPerturbationFlag&eShad)
						{
							TMCCountedPtr<I3DShInstance> instance;
							newTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
							if(colorParamID!=0) // special case: we're replicating a light: modify its color
								NoiseLightColor(instance, localRayOrigin, PertubShadingValue(localRayOrigin),colorParamID, originalLightColor, count++);
							else if(instanceMasterShader)
								NoiseMasterShader(instance, instanceMasterShader, PertubShadingValue(localRayOrigin), count);
						}
				
						// Insert in the scene
						fNewGroup->InsertLast(newTree);

						// Store for undo
						fNewTrees.AddElem(newTree);

						// Select the new tree element
//						TMCCountedPtr<ISelectableObject> newSelectable;
//						newTreeElem->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
//						newSelectable->AddToSelection(fSelection,true);
					}
				}
			
				// Increment progress bar
				gShellUtilities->IncrementProgress(inc, fProgressKey);
				if (gShellUtilities->CheckForUserCancel())
					return;
			}
		}
	}
}

boolean ReplicateCommand::ApplyTransformNoise(const TVector3& point, 
										   TTreeTransform& treeTransform, 
										   const real32 perturbationFactor,
										   const int32 perturbationFlag )
{
	// Position
	TVector3 offset = point;
	if(perturbationFlag&ePosX)
		PerturbPos(offset,perturbationFactor*fPMap.fPerturpPosX, 0);
	if(perturbationFlag&ePosY)
		PerturbPos(offset,perturbationFactor*fPMap.fPerturpPosY, 1);
	if(perturbationFlag&ePosZ)
		PerturbPos(offset,perturbationFactor*fPMap.fPerturpPosZ, 2);
	treeTransform.SetOffset(offset);

	// Uniform scaling
	real32 uniScale = treeTransform.GetUniformScaling();
	if(perturbationFlag&eScaU)
	{
		PerturbScaU(uniScale,point,fPMap.fPerturpScaU);
		if(RealAbs(uniScale)<kRealEpsilon)
		{
			// Invalidate data for the next call
			InvalidateCache();
			return false; // the instance is too small to be visible, just don't insert it
		}
	}
	treeTransform.SetUniformScaling(uniScale);

	// Scaling
	TVector3 scaling = treeTransform.GetXYZScaling();
	if(perturbationFlag&eScaX)
	{
		PerturbScaX(scaling,point,fPMap.fPerturpScaX);
		if(RealAbs(scaling.x)<kRealEpsilon)
		{
			// Invalidate data for the next call
			InvalidateCache();
			return false; // the instance is too small to be visible, just don't insert it
		}
	}
	if(perturbationFlag&eScaY)
	{
		PerturbScaY(scaling,point,fPMap.fPerturpScaY);
		if(RealAbs(scaling.y)<kRealEpsilon)
		{
			// Invalidate data for the next call
			InvalidateCache();
			return false; // the instance is too small to be visible, just don't insert it
		}
	}
	if(perturbationFlag&eScaZ)
	{
		PerturbScaZ(scaling,point,fPMap.fPerturpScaZ);
		if(RealAbs(scaling.z)<kRealEpsilon)
		{
			// Invalidate data for the next call
			InvalidateCache();
			return false; // the instance is too small to be visible, just don't insert it
		}
	}
	treeTransform.SetXYZScaling(scaling);
	
	// Rotation
	if((perturbationFlag&eRotX)||(perturbationFlag&eRotY)||(perturbationFlag&eRotZ))
	{
		real32 xRot=0, yRot=0, zRot=0;
		boolean isDirect=true;
		treeTransform.GetPhyThetaPsy(zRot, yRot, xRot, isDirect);
		if(perturbationFlag&eRotX)
			PerturbRotX(xRot,point,fPMap.fPerturpRotX);
		if(perturbationFlag&eRotY)
			PerturbRotY(yRot,point,fPMap.fPerturpRotY);
		if(perturbationFlag&eRotZ)
			PerturbRotZ(zRot,point,fPMap.fPerturpRotZ);
		treeTransform.SetPhyThetaPsy(zRot, yRot, xRot, isDirect);
	}

	// Invalidate data for the next call
	InvalidateCache();
	return true;
}

void ReplicateCommand::InvalidateCache()
{
	// Invalidate data for the next call
	const int32 cacheCount = fCacheArray.GetElemCount();
	for(int32 iCache=0 ; iCache<cacheCount ; iCache++)
	{
		fCacheArray[iCache].Invalidate();
	}
}

I3DShMasterShader* ReplicateCommand::GetOtherShader()
{
	if(!fNewShaders.GetElemCount())
	{
		TMCCountedPtr<I3DShMasterShader> otherMasterShader;

		if(fPMap.fShaderShading>=0)
		{
			fScene->GetMasterShaderByIndex(&otherMasterShader, fPMap.fShaderShading);
		}
		if(!otherMasterShader)
		{	// Default case: use the color
			TMCCountedPtr<I3DShShader> colorShader;
			gShell3DUtilities->CreateColorShader(fPMap.fPerturbColor, &colorShader);
			
			// Create a master shader to add it to the scene
			gShell3DUtilities->CreateMasterShader(colorShader, fScene, &otherMasterShader);
			ThrowIfNil(otherMasterShader);

			TMCDynamicString name;
			gResourceUtilities->GetIndString(name, kStrings, 1);
			// Make the name unique
			GetUniqueMasterShaderNameByBaseName(fScene,name,name);
			otherMasterShader->SetName(name);

// it's added twice when this is called			fScene->InsertMasterShader(otherMasterShader);
		}

		// Undo infos
		fNewShaders.AddElem(otherMasterShader);
	}

	return fNewShaders[0];
}

void ReplicateCommand::NoiseMasterShader(I3DShInstance* instance, 
										 I3DShMasterShader* instanceMasterShader, 
										 const real32 noiseValue, 
										 int32& nameCounter)
{
	// Look in the existing cache if we already got something build on this master shader
	int32 index = -1;
	const int32 factCount = fShaderFactories.GetElemCount();
	for(int32 iFact=0 ; iFact<factCount ; iFact++)
	{
		if(fShaderFactories[iFact].BasedOnNoAddRef() == instanceMasterShader)
		{	// Use this one
			index=iFact;
			break;
		}
	}

	if(index<0)
	{	// A cache based on this shader need to be created
		index = fShaderFactories.GetElemCount();

		ShaderFactory& newFact = fShaderFactories.AddElem();
		newFact.Init( fPMap.fMaxNewShaders, GetMinMax(), instanceMasterShader);
	}

	I3DShMasterShader* masterShader = fShaderFactories[index].GetMasterShader(noiseValue,nameCounter,
												  GetOtherShader(),
												  fScene);
	instance->SetShader(masterShader);

	// Undo infos
	fNewShaders.AddElem(masterShader);

#if 0 // moved away
	// Create a new master shader countaining a global mixer.
	// The global mixer will mix the current instance shader with
	// our private shader, using the perturbation value to mix them


	// 1: Create a Reference Shader on the original shader
	TMCCountedPtr<IShParameterComponent> ref1ParamComp;
	{
		TMCCountedPtr<IShComponent> refComponent;
		gComponentUtilities->CreateComponent(kShaderFamily, kReferenceShdr, &refComponent);
		// Set its data
		refComponent->QueryInterface(IID_IShParameterComponent, (void**)&ref1ParamComp);	
		ActionNumber shaderPtr = (ActionNumber)*&instanceMasterShader;
		TMCString255 masterName;
		instanceMasterShader->GetName(masterName);
		ref1ParamComp->SetParameter( 'SRef' , &shaderPtr); // Pointer to master shader	
		ref1ParamComp->SetParameter( 'Name' , &masterName); // Name of Master Shader	
	}

	// 2: Create a Value Shader
	TMCCountedPtr<IShComponent> valParamComp;
	{
		TMCCountedPtr<I3DShShader> valShader;
		gShell3DUtilities->CreateValueShader(noiseValue, &valShader);

		valShader->QueryInterface(IID_IShParameterComponent, (void**)&valParamComp);
	}

	// 3: Get the other shader or create a color shader
	I3DShMasterShader* otherMasterShader = GetOtherShader();

	// 4: Create a second Reference Shader on the other one
	TMCCountedPtr<IShParameterComponent> ref2ParamComp;
	{
		TMCCountedPtr<IShComponent> refComponent;
		gComponentUtilities->CreateComponent(kShaderFamily, kReferenceShdr, &refComponent);
		// Set its data
		refComponent->QueryInterface(IID_IShParameterComponent, (void**)&ref2ParamComp);	
		ActionNumber shaderPtr = (ActionNumber)*&otherMasterShader;
		TMCString255 masterName;
		otherMasterShader->GetName(masterName);
		ref2ParamComp->SetParameter( 'SRef' , &shaderPtr); // Pointer to master shader	
		ref2ParamComp->SetParameter( 'Name' , &masterName); // Name of Master Shader	
	}

	// 5: Create a global mixer
	TMCCountedPtr<I3DShShader> globalMixer;
	{
		TMCCountedPtr<IShComponent> component;
		gComponentUtilities->CreateComponent(kShaderFamily, kGlobalMixShdr, &component);
		TMCCountedPtr<IShParameterComponent> paramComp;
		component->QueryInterface(IID_IShParameterComponent, (void**)&paramComp);
		paramComp->SetParameter( 'Sh00' , &ref1ParamComp);	
		paramComp->SetParameter( 'Sh01' , &ref2ParamComp);	
		paramComp->SetParameter( 'Sh02' , &valParamComp);	
//		gShaderUtils::ForceExtensionDataChanged(paramComp);

		component->QueryInterface(IID_I3DShShader, (void**)&globalMixer);
	}

	// 4: Crate the master shader to add to the scene
	{
		TMCCountedPtr<I3DShMasterShader> masterShader;
		gShell3DUtilities->CreateMasterShader(globalMixer, fScene, &masterShader);
		ThrowIfNil(masterShader);

		TMCDynamicString name;
		TMCString15 index;index.FromInt32( instanceIndex );
		instanceMasterShader->GetName(name);
		name+=index;
		GetUniqueMasterShaderNameByBaseName(fScene,name,name);
		masterShader->SetName(name);

		instance->SetShader(masterShader);

		// Undo infos
		fNewShaders.AddElem(masterShader);
	}
#endif
}

void ReplicateCommand::NoiseLightColor(I3DShInstance* lightInstance,
										const TVector3& point,
										const real32 noiseValue, 
										const int32 colorParamID,
										TMCColorRGB& originalLightColor,
										const int32 instanceIndex)
{
	if(RealAbs( noiseValue )<kRealEpsilon)
		return;

	// Merge the original color with the other shader one
	TMCCountedPtr<I3DShMasterShader> otherMasterShader;
	if(fPMap.fShaderShading>=0)
	{
		fScene->GetMasterShaderByIndex(&otherMasterShader, fPMap.fShaderShading);
	}
	TMCColorRGBA result;

	if(!otherMasterShader)
	{	// Default case: use the color
		result = fPMap.fPerturbColor;
	}
	else
	{
		// Note: we could store it in the Cache
		TMCCountedPtr<I3DShShader> shader;
		otherMasterShader->GetShader(&shader);
		boolean fullArea=false;
		ShadingIn shadingIn;
		InitShadingIn(shadingIn);
		shadingIn.fPoint = shadingIn.fPointLoc = point;
		shadingIn.fUV = point.CastToXY();//fInvMaxDist*(point.CastToXY()-fOrigin);
		shader->GetColor(result,fullArea,shadingIn);
	}

	// Set the color of the light
	TMCCountedPtr<I3DShLightsource> lightSource;
	lightInstance->QueryInterface( IID_I3DShLightsource , (void **) &lightSource);
	if(lightSource)
	{
		I3DExLightsource* lightEx = lightSource->GetValidExLightNoAddRef();
		if(lightEx)
		{
			TMCCountedPtr<IShParameterComponent> component;
			lightEx->QueryInterface(IID_IShParameterComponent,(void**)&component);
			if(MCVerify(component))
			{
				// then set the color
				TMCColorRGBA lCol(originalLightColor.red,originalLightColor.green,originalLightColor.blue,0);
				result.Interpolate(lCol,result,noiseValue);

				component->SetParameter(colorParamID, &result);
			}
		}
	}
}

int32 ReplicateCommand::GetColorParamID(I3DShInstance* lightInstance, TMCColorRGB& color)
{
	TMCCountedPtr<I3DShLightsource> lightSource;
	lightInstance->QueryInterface( IID_I3DShLightsource , (void **) &lightSource);
	if(lightSource)
	{
		I3DExLightsource* lightEx = lightSource->GetValidExLightNoAddRef();
		if(lightEx)
		{
			TMCCountedPtr<IShParameterComponent> component;
			lightEx->QueryInterface(IID_IShParameterComponent,(void**)&component);
			if(MCVerify(component))
			{
				const int32 paramCount = component->GetParameterCount();
				for(int32 iParam=0 ; iParam<paramCount ; iParam++)
				{
					TPMapElement paramInfo;
					component->GetParameterInfo(iParam, paramInfo);
					if(paramInfo.fParamType == 'colo')
					{
						component->GetParameter(paramInfo.fPartID, (void*)&color);
						return paramInfo.fPartID;
					}
				}
			}
		}
	}

	return 0;
}

void OffsetTimelineKeyframes(I3DShParamTimeLine* aTimeLine, const real32 offset)
{
	if(aTimeLine)
	{		
		const int32 chainLinkCount = aTimeLine->GetTweenerChainLinkCount();
		for(int32 iLink=0 ; iLink<chainLinkCount ; iLink++)
		{
			if(iLink==0)
				continue; // t=0 keyframe, it's already the right position
			TMCCountedPtr<I3DShTweenerChainLink> tweenerChainLink;
			aTimeLine->GetTweenerChainLinkByIndex( iLink, &tweenerChainLink );
			if(!tweenerChainLink)
				continue;
			TMCCountedPtr<I3DShKeyFrame> keyFrame;
			tweenerChainLink->GetKeyFrame(&keyFrame);
			if(!keyFrame)
				continue;
			// Offset the keyframe value
			boolean changed=false;
			real32 value=0;
			keyFrame->GetParam(&value,kReal32ParamType,changed); 
			value += offset;
			keyFrame->SetParam(&value,kReal32ParamType);  
		}
	}
}
void ReplicateCommand::OffsetAnimation(I3DShTreeElement* tree, const TVector3& offset)
{
	TMCCountedPtr<I3DShAnimationMethod> animationMethod;
	tree->GetAnimationMethod(&animationMethod);
	if (animationMethod == (I3DShAnimationMethod*)NULL)
		return;

	{	// X timeline
		TMCCountedPtr<I3DShParamTimeLine> aTimeLine;
		animationMethod->GetXTranslationAnimationTrack(&aTimeLine);

		OffsetTimelineKeyframes(aTimeLine, offset.x);
	}
	{	// Y timeline
		TMCCountedPtr<I3DShParamTimeLine> aTimeLine;
		animationMethod->GetYTranslationAnimationTrack(&aTimeLine);

		OffsetTimelineKeyframes(aTimeLine, offset.y);
	}
	{	// Z timeline
		TMCCountedPtr<I3DShParamTimeLine> aTimeLine;
		animationMethod->GetZTranslationAnimationTrack(&aTimeLine);

		OffsetTimelineKeyframes(aTimeLine, offset.z);
	}
}

ReplicatePMap& ReplicatePMap::operator=(const ReplicatePMap& pmap)
{
	fReplicateMode = pmap.fReplicateMode;

	// Cube
	fXCount = pmap.fXCount; // >=1
	fXSpacing = pmap.fXSpacing; // space between 2 bbox
	fYCount = pmap.fYCount; // >=1
	fYSpacing = pmap.fYSpacing; // space between 2 bbox
	fZCount = pmap.fZCount; // >=1
	fZSpacing = pmap.fZSpacing; // space between 2 bbox
	// Cylinder
	fRCount = pmap.fRCount; // >=1, elements arround
	fRadius = pmap.fRadius;
	fCylZCount = pmap.fCylZCount; // >=1
	fCylZSpacing = pmap.fCylZSpacing; // space between 2 bbox
	fAngle = pmap.fAngle;
	fTorque = pmap.fTorque;
	// Surface
	fTreePermanentID = pmap.fTreePermanentID;
	fSurfaceXDensity = pmap.fSurfaceXDensity; 
	fSurfaceYDensity = pmap.fSurfaceYDensity; 
	fAlignNormal = pmap.fAlignNormal;
	fCubeMap = pmap.fCubeMap;
	fRepartitionMin = pmap.fRepartitionMin; 
	fRepartitionMax = pmap.fRepartitionMax; 

	// Randomize params
	// Position
	fPerturpPosX = pmap.fPerturpPosX;
	fPerturpPosY = pmap.fPerturpPosY;
	fPerturpPosZ = pmap.fPerturpPosZ;
	fPerturpScaU = pmap.fPerturpScaU;
	fPerturpScaX = pmap.fPerturpScaX;
	fPerturpScaY = pmap.fPerturpScaY;
	fPerturpScaZ = pmap.fPerturpScaZ;
	fPerturpRotX = pmap.fPerturpRotX;
	fPerturpRotY = pmap.fPerturpRotY;
	fPerturpRotZ = pmap.fPerturpRotZ;
	fPerturpShading = pmap.fPerturpShading;
	fPerturbColor = pmap.fPerturbColor;

	fMaxNewShaders = pmap.fMaxNewShaders;

	fShaderPosX = pmap.fShaderPosX;
	fShaderPosY = pmap.fShaderPosY;
	fShaderPosZ = pmap.fShaderPosZ;
	fShaderScaU = pmap.fShaderScaU;
	fShaderScaX = pmap.fShaderScaX;
	fShaderScaY = pmap.fShaderScaY;
	fShaderScaZ = pmap.fShaderScaZ;
	fShaderRotX = pmap.fShaderRotX;
	fShaderRotY = pmap.fShaderRotY;
	fShaderRotZ = pmap.fShaderRotZ;
	fShaderShaM = pmap.fShaderShaM;
	fShaderShading = pmap.fShaderShading;

	return *this;
}

/////////////////////////////////////////////////////////////////////////
//
// For Save/Load
//

ReplicateSaveData::ReplicateSaveData()
{
}

ReplicateSaveData::~ReplicateSaveData()
{
}

MCCOMErr ReplicateSaveData::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ReplicateData))
	{
		TMCCountedGetHelper<ReplicateSaveData> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return TBasicDataComponent::QueryInterface(riid, ppvObj);
}

void ReplicateSaveData::SetFromReplicateCommand(ReplicateCommand& command)
{
	fPMap = *(ReplicatePMap*)(command.GetExtensionDataBuffer());
}

void ReplicateSaveData::SetToReplicateCommand(ReplicateCommand& command)
{
	 *(ReplicatePMap*)command.GetExtensionDataBuffer() = fPMap;
}

/****************************************************************************************************

		SwapCommand.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#include "SwapCommand.h"

#include "copyright.h"
#include "SwapDef.h"
#include "MiscComUtilsImpl.h"
#include "I3dShObject.h"
#include "I3DShCamera.h"
#include "I3DShLightsource.h"
#include "I3DShMasterGroup.h"
#include "I3DShShader.h"
#include "I3DShModifier.h"
#include "I3DShConstraint.h"
#include "I3dShRenderFeature.h"
#include "I3DShAnimationMethod.h"
#include "I3dShAnimation.h"
#include "I3DShGroup.h"
#include "I3DShInstance.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_SwapCommand(R_CLSID_SwapCommand);
#else
const MCGUID CLSID_SwapCommand={R_CLSID_SwapCommand};
#endif

// PMap
SwapPMap::SwapPMap()
{
	fType = 'Opt1';
	fTreePermanentID = -1;
	fObjectFitIn = false;
	fTreeFitIn = false;
}

// Callback to prepare the menus

class SwapCommandCallBack : public TBasicMenuCallBack
{
public:
	SwapCommandCallBack();

	virtual boolean MCCOMAPI SelfPrepareMenu(ISceneDocument* sceneDocument);

private:
};

SwapCommandCallBack::SwapCommandCallBack()
{
}

boolean  SwapCommandCallBack::SelfPrepareMenu(ISceneDocument* sceneDocument)
{

	if (!sceneDocument)
		return false;

	// If the toolbar does not exist, build it

	TMCCountedPtr<ISceneSelection> selection;
	sceneDocument->GetSceneSelection(&selection);

	if (!selection)
		return false;

#if (VERSIONNUMBER >= 0x050000)
	const int32 leavesCount = selection->GetObjectCount();
#elif (VERSIONNUMBER >= 0x040000)
	TMCPtrArray<ISelectableObject> leaves;
	selection->GetSelectables(	NULL, &leaves);
	
	const int32 leavesCount= leaves.GetElemCount();
#else
	TMCClassArray<TSelectionNode> leaves;
	selection->GetLeaves(leaves);
	
	const int32 leavesCount= leaves.GetElemCount();
#endif

	for (int32 leafIndex= 0; leafIndex < leavesCount; leafIndex++)
	{
#if (VERSIONNUMBER >= 0x050000)
		ISelectableObject* leafObject = selection->GetSelectableObjectByIndex(leafIndex);
#elif (VERSIONNUMBER >= 0x040000)
		ISelectableObject* leafObject = leaves[leafIndex];
#else
		ISelectableObject* leafObject = leaves[leafIndex].fData;
#endif

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

SwapCommand::SwapCommand()
{
	// Data from BasicCommand
	fNeedsSelection = true;

	fIsValid = false;

}
  
SwapCommand::~SwapCommand()
{
}

MCCOMErr SwapCommand::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_SwapCommand))
	{
		TMCCountedGetHelper<SwapCommand> result(ppvObj);
		result = this;
		return MC_S_OK;
	}

	return TBasicSceneCommand::QueryInterface(riid, ppvObj);
}

MCCOMErr SwapCommand::ExtensionDataChanged()
{
	MCCOMErr result = MC_S_OK;
	
	fIsValid = false;

	return result;
}


void SwapCommand::GetMenuCallBack(ISelfPrepareMenuCallBack** callBack)
{
	TMCCountedCreateHelper<ISelfPrepareMenuCallBack> result(callBack);

	result = new SwapCommandCallBack;
}

MCCOMErr SwapCommand::Init(ISceneDocument* sceneDocument)
{
	fSceneDocument = sceneDocument;

	try
	{
		if (fSceneDocument)
		{
			fSceneDocument->GetSceneSelection(&fSelection);
			fSelection->Clone(&fCloneSelection);

			fSceneDocument->GetScene(&fScene);

			fScene->GetTreePropertyChangeChannel(&fTreePropertyChannel);
			ThrowIfNil(fTreePropertyChannel);

			fSceneDocument->GetSceneSelectionChannel(&fSelectionChannel);
			ThrowIfNil(fSelectionChannel);
			sceneDocument->GetTreeHierarchyChannel(&fHierarchyChannel);
			ThrowIfNil(fHierarchyChannel);
		}
	}
	catch(TMCException&)
	{
		// Couldn't init
	}

	return MC_S_OK;
}

// This method is called before the dialog opens
MCCOMErr SwapCommand::Prepare()
{
	// Fill in the default param in the pmap
	TMCCountedPtr<IShComponent> defaultCamera;
	gComponentUtilities->CreateComponent('came', 'coni', &defaultCamera);
	defaultCamera->QueryInterface(IID_IShParameterComponent, (void**) &(fPMap.fCameraComponent));

	TMCCountedPtr<IShComponent> defaultLight;
	gComponentUtilities->CreateComponent('lite', 'spot', &defaultLight);
	defaultLight->QueryInterface(IID_IShParameterComponent, (void**) &(fPMap.fLightComponent));

	// Set the default param in the PMap
	if(fScene->Get3DObjectsCount())
	{
		TMCCountedPtr<I3DShObject> object;
		fScene->Get3DObjectByIndex(&object, 0);

		object->GetName(fPMap.fObjectName);
	}
	if(fScene->GetMasterShadersCount())
	{
		TMCCountedPtr<I3DShMasterShader> masterShader;
		fScene->GetMasterShaderByIndex(&masterShader, 0);

		masterShader->GetName(fPMap.fShaderName);
	}
#if (VERSIONNUMBER >= 0x050000)
#else
	if(fScene->GetInstanceListCount())
	{
		TMCCountedPtr<I3DShInstance> instance;
		fScene->GetInstanceByIndex(&instance, 0);

		fPMap.fTreePermanentID = instance->GetTreeElement()->GetTreePermanentID();
	}
#endif


	// See what is selected: if it's a light, set the panel to camera,
	// if it's a camera, to light, otherwise to obj

	TTreeSelectionIterator iter(fCloneSelection);
	I3DShTreeElement* firstTree=iter.First();
	if(firstTree)
	{
		TMCCountedPtr<I3DShInstance> instance;
		firstTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
		if(!instance)
				return MC_S_OK;

		if( instance->GetInstanceKind() == I3DShInstance::kCameraInstance )
		{
			fPMap.fType = 'Opt4';
		}
		else if( instance->GetInstanceKind() == I3DShInstance::kLightInstance )
		{
			fPMap.fType = 'Opt3';
		}

	}

	return MC_S_OK;
}


void SwapCommand::Validate()
{
	if(!fIsValid)
	{
		fIsValid = true;

	}
}

boolean SwapCommand::Do()
{
	try
	{
		CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'SWAC');

		Validate();

		TMCString255 progressString;
		gResourceUtilities->GetIndString(progressString, kStrings, 2);
		gShellUtilities->BeginProgress(progressString, &fProgressKey);

		// Work at t=0
		real32 fTime;
		MicroTick currentTime = fScene->GetTime(&fTime);
		fScene->SetTime (0, false);
		
		if(fPMap.fType == 'Opt2' ) // Shader
			SetShaders();
		else
			Replace();

		// Reset time
		fScene->SetTime (currentTime, false);

		gShellUtilities->EndProgress(fProgressKey);

		gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
		gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	}
	catch(TMCException&)
	{
		// Couldn't replace
	}


	return true;
}

void SwapCommand::SetShaders()
{
	// Get the master shader
	TMCCountedPtr<I3DShMasterShader> masterShader;
	fScene->GetMasterShaderByName(&masterShader, fPMap.fShaderName);
	if(!MCVerify(masterShader))
		return;

	// replace all the selected instance shaders
	const int32 leavesCount = fCloneSelection->GetObjectCount();
	{ // Set the shaders
		TTreeSelectionIterator iter(fCloneSelection);
		for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
		{
			// See if the tree is a shadable tree
			TMCCountedPtr<I3DShInstance> instance;
			iTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(!instance)
				continue;

			TMCCountedPtr<I3DShMasterShader> treeMasterShader;
			instance->GetShader(&treeMasterShader);	
			if(!treeMasterShader)
				continue;

			// Store for undo
			fUndoShaders.AddElem(treeMasterShader);

			// Set the new shader
			instance->SetShader(masterShader);	
		
			gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeMasterShader, iTree);
		}
	}
}

struct SwapData
{
	boolean fObj;
	boolean fTre;
	boolean fCam;
	boolean fLig;

	TMCCountedPtr<I3DShObject>		fObject;
	TMCCountedPtr<I3DShMasterGroup>	fMasterGroup; // if the object is a master group, we need to create a scene instance
	TMCCountedPtr<I3DShTreeElement>	fTree;
	TMCCountedPtr<IShComponent>		fCameraComp;
	TMCCountedPtr<IShComponent>		fLightComp;
};

boolean CloneHierachy(I3DShTreeElement* parent, I3DShTreeElement** newTree )
{
	// Clone the tree
	parent->ComClone( newTree, kWithAnim, true );

	// then clone the children
	boolean hasChild = false;
	TMCCountedPtr<I3DShTreeElement> child;
	parent->GetFirst(&child);
	while(child)
	{
		// Clone the child
		TMCCountedPtr<I3DShTreeElement> newChild;
		CloneHierachy(child, &newChild);

		// Add it to the tree
		(*newTree)->InsertLast(newChild);

		// Get the next child
		child->GetRight(&child);

		hasChild = true;
	}

	return hasChild;
}

boolean IsAnimated( I3DShTreeElement* tree )
{
	// Animation method: this will erease any previous transform
	TMCCountedPtr<I3DShAnimationMethod> animationMethod;
	tree->GetAnimationMethod(&animationMethod);
	if( animationMethod )
	{
		TMCCountedPtr<I3DShParamTimeLine> animationTrack;
		animationMethod->GetXTranslationAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetYTranslationAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetZTranslationAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetHotPointAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetUniformScalingAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetRotationAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetXScalingAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetYScalingAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		animationMethod->GetZScalingAnimationTrack(&animationTrack);
		if( animationTrack && animationTrack->IsAnimated() )
			return true;
		TMCCountedPtr<I3DShParamTimeLine> xAnimationTrack;
		TMCCountedPtr<I3DShParamTimeLine> yAnimationTrack;
		TMCCountedPtr<I3DShParamTimeLine> zAnimationTrack;
		TMCCountedPtr<I3DShParamTimeLine> mirrorAnimationTrack;
		EEulerAnglesOrder anglesOrder;
		animationMethod->GetRotationAnimationTrack(&xAnimationTrack,
							 &yAnimationTrack,
							 &zAnimationTrack,
							 &mirrorAnimationTrack, anglesOrder);
		if( xAnimationTrack && xAnimationTrack->IsAnimated() )
			return true;
		if( yAnimationTrack && yAnimationTrack->IsAnimated() )
			return true;
		if( zAnimationTrack && zAnimationTrack->IsAnimated() )
			return true;
	}

	return false;
}
void SwapCommand::SwapTree(I3DShTreeElement* tree, const SwapData& data )
{
	TMCCountedPtr<I3DShTreeElement> newTreeElement;

	// Save the previous transform (for fit in option)
	TTreeTransform prevTreeTransform;
	tree->GetLocalTreeTransform(prevTreeTransform);
	TVector3 prevHotPoint;
	tree->GetHotPoint( prevHotPoint );

	if(data.fObj)
	{
		TMCCountedPtr<I3DShInstance> newInstance;
		if(data.fMasterGroup)
			gComponentUtilities->CoCreateInstance(CLSID_StandardSceneInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&newInstance);	
		else
			gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&newInstance);
		
		ThrowIfNil(newInstance);
		
		newInstance->Set3DObject(data.fObject);
		newInstance->QueryInterface(IID_I3DShTreeElement, (void**) &newTreeElement);
	}
	else if(data.fTre)
	{
		/*isHierarchy = */CloneHierachy(data.fTree, &newTreeElement);
	}
	else if(data.fCam)
	{
		TMCCountedPtr<I3DShCamera> camera;
		switch (fPMap.fCameraComponent->GetClassSignature())
		{
		case 'coni':
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardConicalCamera, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShCamera, (void**)&camera);
			} break;
		case 'iso ':
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardIsometricCamera, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShCamera, (void**)&camera);
			} break;
		default:
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardExternalCamera, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShCamera, (void**)&camera);
			} break;
		}
		camera->SetCameraComponent(data.fCameraComp, kWithAnim);
		camera->QueryInterface(IID_I3DShTreeElement, (void**) &newTreeElement);
	}
	else if(data.fLig)
	{
		TMCCountedPtr<I3DShLightsource> lightSource;
		switch (fPMap.fLightComponent->GetClassSignature())
		{
		case 'spot':
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardSpotLight, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShLightsource, (void**)&lightSource);
			} break;
		case 'bulb':
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardBulbLight, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShLightsource, (void**)&lightSource);
			} break;
		case 'dist':
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardDistantLight, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShLightsource, (void**)&lightSource);
			} break;
		default:
			{
				gComponentUtilities->CoCreateInstance(CLSID_StandardExternalLight, nil, MC_CLSCTX_INPROC_SERVER, IID_I3DShLightsource, (void**)&lightSource);
			} break;
		}
		lightSource->SetLightComponent(data.fLightComp, kWithAnim);
		lightSource->QueryInterface(IID_I3DShTreeElement, (void**) &newTreeElement);
	}

	ThrowIfNil(newTreeElement);

	// Set the name
	TMCDynamicString name;
	tree->GetName(name);
	newTreeElement->SetName(name);
	
	TTreeTransform newTreeTransform;
	if( data.fTre )
		newTreeElement->GetLocalTreeTransform(newTreeTransform);
	else
		newTreeTransform = prevTreeTransform;

	if(data.fObj || data.fTre)
	{
		// Fit in option
		const boolean doFitIn = (fPMap.fObjectFitIn && data.fObj) || (fPMap.fTreeFitIn && data.fTre);
			
		// Adjust the transform:
		// - in all the cases, makes the bbox center match, so the object will appear at the same position.
		// - if the fitIn option is on, add some scaling to to adjust the object size

		{
			TBBox3D prevBbox;
			tree->GetBoundingBox(prevBbox, kApplyAllDeformersAtInstanceLevel, true);
			prevBbox = prevTreeTransform.TransformBBox(prevBbox);// The bbox of the tree doesn't take into account the transform of the tree => need to transform it
			
			TBBox3D newBbox;
			newTreeElement->GetBoundingBox(newBbox, kApplyAllDeformersAtInstanceLevel, true);
			newBbox = newTreeTransform.TransformBBox(newBbox);

			// Compute the scaling between the 2 bbox
			TVector3 XYZScaling = TVector3::kOnes;
			if(doFitIn)
			{
				XYZScaling.x = prevBbox.GetWidth() /	newBbox.GetWidth();
				XYZScaling.y = prevBbox.GetHeight() /newBbox.GetHeight();
				XYZScaling.z = prevBbox.GetDepth() /	newBbox.GetDepth();
			}
			
			// Compute the offset between the 2 bbox
			TVector3 offset;
			TVector3 oldCenter; prevBbox.GetCenter(oldCenter);
			TVector3 newCenter; newBbox.GetCenter(newCenter);
			offset = newTreeTransform.GetOffset() - newCenter;
			offset.x *= XYZScaling.x;
			offset.y *= XYZScaling.y;
			offset.z *= XYZScaling.z;
			offset += oldCenter;

			// Modify the treeTransform
			newTreeTransform.SetOffset( offset );
			if(doFitIn)
				newTreeTransform.SetXYZScaling(XYZScaling % newTreeTransform.GetXYZScaling());
		}
	}

	// insert the new tree
	// Insert the tree before setting the data comp: there was a crash with SetIsShown
	// Insert the tree before setting its transform: there's a bug with the Groups otherwise (wrong position)
	tree->InsertLeft(newTreeElement);

	boolean isAnimated = false;
	if(!data.fTre)
	{
		// Animation method: this will erease any previous transform
		isAnimated = IsAnimated( tree ) ;
		if( isAnimated )
		{
			TMCCountedPtr<I3DShAnimationMethod> animationMethod;
			tree->GetAnimationMethod(&animationMethod);
			if( animationMethod )
			{
				newTreeElement->SetAnimationMethod(animationMethod, kWithAnim);
				animationMethod->SetTree( newTreeElement );
			//	animationMethod->OffsetAnimation(newTreeTransform);
			}
		}

		// Set the modifiers
		const int32 modifCount = tree->GetModifiersCount();
		TMCCountedPtrArray<I3DShModifier> modifiers;
		modifiers.SetElemCount(modifCount);
		for(int32 iModif=0 ; iModif<modifCount ; iModif++)
		{
			tree->GetModifierByIndex((modifiers.Pointer(iModif)), iModif);
		}
		newTreeElement->SetModifiers(&modifiers, kWithAnim);

		// Set the constrains
		TMCCountedPtr<I3DShConstraint> constrain;
		tree->GetConstraint(&constrain);
		newTreeElement->SetConstraint(constrain, kWithAnim);

		// Set the data comp
		const int32 compCount = tree->GetDataComponentsCount();
		for(int32 iComp=0 ; iComp<compCount ; iComp++)
		{
			TMCCountedPtr<I3DShDataComponent> dataComp;
			tree->GetDataComponentByIndex(&dataComp, iComp);
			try
			{
				newTreeElement->InsertDataComponent(dataComp);
			}
			catch(TMCException& )
			{
				// Data component on objects and master objects aren't fully compatibles.
				// Some throw an exeption
			}
		}

		// Other params
		newTreeElement->SetVisibilityFlag(tree->GetVisibilityFlag());
		newTreeElement->SetIsShown(tree->GetIsShown());
		newTreeElement->SetShadowCasting(tree->GetShadowCasting());
		newTreeElement->SetReceiveShadow(tree->GetReceiveShadow());
		newTreeElement->SetAnimateTree(tree->GetAnimateTree());
	}

	if( !isAnimated )
	{
		// Force scaling
		if(data.fObj || data.fTre)
		{
			TVector3 forceXYZScaling = newTreeTransform.GetXYZScaling();
			newTreeElement->SetXYZScaling(forceXYZScaling);
		}

		newTreeElement->SetHotPoint( prevHotPoint );
		newTreeElement->SetLocalTreeTransform(newTreeTransform, 
											kIgnoreConstraint, 
											false, 
											true, 
											kXTreeBehaviorDefault,
											false,
											true);
	}

	// Select the new tree
	TMCCountedPtr<ISelectableObject> newSelectable;
	newTreeElement->QueryInterface(IID_ISelectableObject, (void**)&newSelectable);
	newSelectable->AddToSelection(fSelection,true);

	// Store for undo
	fNewTrees.AddElem(newTreeElement);
}

void SwapCommand::UnlinkTree(I3DShTreeElement* tree)
{
	if( fNewTrees.FindElem(tree) == kUnusedIndex ) // do not remove the new trees
	{
		RelinkTreeElementInfo info;
		TMCCountedPtr<I3DShTreeElement> unlinkedTree;
		tree->Unlink(&unlinkedTree, &info);
		fUndoRelinkInfo.AddElem(info);
		gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, tree);
	}
}

void SwapCommand::Replace()
{
	SwapData data;
	data.fObj = (fPMap.fType == 'Opt1' ); // Object
	data.fTre = (fPMap.fType == 'Opt0' ); // tree
	data.fCam = (fPMap.fType == 'Opt3' ); // Camera
	data.fLig = (fPMap.fType == 'Opt4' ); // Light
	
	// Clear the selection
	I3DShTreeElement* tree = NULL;
	TTreeSelectionIterator	selectionIter(fSelection);
	for (tree = selectionIter.First(); tree!=NULL ; tree = selectionIter.Next())
	{
		tree->PostMoveChange();
	}

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);

	fSelection->ClearSelection();

	if(data.fObj)
	{
		// Get the master object
		fScene->Get3DObjectByName(&(data.fObject), fPMap.fObjectName);
		if(!MCVerify(data.fObject))
			return;
		data.fObject->QueryInterface(IID_I3DShMasterGroup,(void**)&(data.fMasterGroup));
	}
	else if(data.fTre)
	{
		// Get the tree
		if(fPMap.fTreePermanentID<0)
			return;
		data.fTree = fScene->GetTreeByIndex(fPMap.fTreePermanentID);

		if(!MCVerify(data.fTree))
			return;
	}
	else if(data.fCam)
	{
		if(!MCVerify(fPMap.fCameraComponent))
			return;
		fPMap.fCameraComponent->QueryInterface(IID_IShComponent, (void**) &(data.fCameraComp));
	}
	else if(data.fLig)
	{
		if(!MCVerify(fPMap.fLightComponent))
			return;
		fPMap.fLightComponent->QueryInterface(IID_IShComponent, (void**) &(data.fLightComp));
	}

	// replace all the selected instances with the object
	const int32 selectCount = fCloneSelection->GetObjectCount();

	{ // Create the new trees
		TTreeSelectionIterator iter(fCloneSelection);
		for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
		{
			SwapTree(iTree, data);
		}
	}

	// Prepare the undo info
	fUndoRelinkInfo.ArrayFree();

	{ // Remove the old trees
		TTreeSelectionIterator iter(fCloneSelection);
		for (tree = iter.First(); tree; tree = iter.Next())
		{
			UnlinkTree(tree);
		}
	}

	gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
	gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
}


boolean SwapCommand::Undo()
{
	if(fPMap.fType == 'Opt2')
	{
		// Reset the shaders on the previous selection
		const int32 selectCount = fCloneSelection->GetObjectCount();

		int32 shaderIndex=0;
		int32 shaderCount = fUndoShaders.GetElemCount();

		// Set the shaders
		TTreeSelectionIterator iter(fCloneSelection);
		for(I3DShTreeElement* iTree=iter.First(); iTree!=NULL ; iTree=iter.Next())
		{
			if(shaderIndex>shaderCount)
				break;

			// See if the tree is a shadable tree
			TMCCountedPtr<I3DShInstance> instance;
			iTree->QueryInterface(IID_I3DShInstance, (void**) &instance); 
			if(!instance)
				continue;

			TMCCountedPtr<I3DShMasterShader> treeMasterShader;
			instance->GetShader(&treeMasterShader);	
			if(!treeMasterShader)
				continue;

			// Set the new shader
			instance->SetShader(fUndoShaders[shaderIndex++]);	
		
			gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeMasterShader, iTree);
		}
	}
	else
	{

		{	// Restore the previous ones
			TMCArray<RelinkTreeElementInfo>::iterator iter(fUndoRelinkInfo);
			for (RelinkTreeElementInfo* info=iter.Last(); iter.MorePrev(); info=iter.Prev())
			{
				try
				{
					info->fTreeElement->Relink(info);
					gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeAdded, info->fTreeElement);
				}
				catch(TMCException& )
				{
					// Exeption thrown with undo on invisible tree
				}
			}
		}

		// Prepare the redo info
		fRedoRelinkInfo.ArrayFree();

		{	// Remove the new items from the scene
			TMCCountedPtrArray<I3DShTreeElement>::iterator	iter(fNewTrees);
			for (I3DShTreeElement* tree = iter.First(); iter.More(); tree = iter.Next())
			{
				RelinkTreeElementInfo info;
				TMCCountedPtr<I3DShTreeElement> unlinkedTree;
				tree->Unlink(&unlinkedTree, &info);
				fRedoRelinkInfo.AddElem(info);
				gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, tree);
			}
		}

		
		gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
		gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	}

	return true;
}

boolean SwapCommand::Redo()
{
	if(fPMap.fType == 'Opt2')
	{
		// Reset the shaders on the previous selection
		SetShaders();
	}
	else
	{
		{	// Restore the new ones
			TMCArray<RelinkTreeElementInfo>::iterator iter(fRedoRelinkInfo);
			for (RelinkTreeElementInfo* info=iter.Last(); iter.MorePrev(); info=iter.Prev())
			{
				info->fTreeElement->Relink(info);
				gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeAdded, info->fTreeElement);
			}
		}

		{	// Remove the old items from the scene
			TMCArray<RelinkTreeElementInfo>::iterator iter(fUndoRelinkInfo);
			for (RelinkTreeElementInfo* info=iter.Last(); iter.MorePrev(); info=iter.Prev())
			{
				TMCCountedPtr<I3DShTreeElement> unlinkedTree;
				info->fTreeElement->Unlink(&unlinkedTree);
				gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, info->fTreeElement);
			}
		}

		gChangeManager->PostChange(fSelectionChannel, 0, fSelection);
		gChangeManager->PostChange(fHierarchyChannel, 0, NULL);
	}

	return true;
}

/****************************************************************************************************

		TypePart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "TypePart.h"

#include "copyright.h"
#include "InstanciatorDef.h"
#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "IMFTextPopupPart.h"
#include "MiscComUtilsImpl.h"
#include "ISceneDocument.h"
#include "InterfaceIDs.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "I3DShTreeElement.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "ISceneSelection.h"
#include "IMCGraphicContext.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
const MCGUID CLSID_TypePart(R_CLSID_TypePart);
#else
const MCGUID CLSID_TypePart = {R_CLSID_TypePart};
#endif

static const int32 kCubeType = 'cube';
static const int32 kCylinderType = 'cyli';
static const int32 kSurfaceType = 'surf';

static const int32 kCubeOptionsPart = 'CubO';
static const int32 kCylinderOptionsPart = 'CylO';
static const int32 kSurfaceOptionsPart = 'SurO';

static const int32 kNoOption = 'NoPt';

// TypePart class

TypePart::TypePart()
{
	fEraseRectCount = 0;
	fIsInit = false;
}

void TypePart::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr TypePart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_TypePart))
	{
		TMCCountedGetHelper<TypePart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void TypePart::FinishCreateFromResource()
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	// Get the rectangle for the background color
	{	// 3 main types
		TMCArray<int32> ids;
		const int32 idCount = 3;
		ids.SetElemCount(idCount);
		ids[0] = 'fra0';
		ids[1] = 'fra1';
		ids[2] = 'fra2';
		fModeRects.SetElemCount(idCount);
		for(int32 i=0 ; i<idCount ; i++)
		{
			TMCCountedPtr<IMFPart> modePart;
			thisPart->FindChildPartByID(&modePart, ids[i]);
			ThrowIfNil(modePart);
			modePart->GetBoundsInWindow(fModeRects[i]);
			TMCPoint pos;
			thisPart->GetPositionInWindow(pos);
			fModeRects[i].Offset(-pos.x, -pos.y);
		}
	}
	{	// 6 cube and cylinder preset presets
		TMCArray<int32> ids;
		const int32 idCount = 6;
		ids.SetElemCount(idCount);
		ids[0] = 'CuF0';
		ids[1] = 'CuF1';
		ids[2] = 'CuF2';
		ids[3] = 'CuF3';
		ids[4] = 'CuF4';
		ids[5] = 'CuF5';
		const int32 offset = fModeRects.GetElemCount();
		fModeRects.AddElemCount(idCount);
		for(int32 i=0 ; i<idCount ; i++)
		{
			TMCCountedPtr<IMFPart> modePart;
			thisPart->FindChildPartByID(&modePart, ids[i]);
			ThrowIfNil(modePart);
			modePart->GetBoundsInWindow(fModeRects[offset+i]);
			TMCPoint pos;
			thisPart->GetPositionInWindow(pos);
			fModeRects[offset+i].Offset(-pos.x, -pos.y);
		}
	}

	// Build the Tree popup menu for the surface type
	BuildTreePopupMenu();

	// Default option setting
	DisplayOption(kCubeType);

	// Min max for the repartition slider
	TMCCountedPtr<IMFPart> slider;
	thisPart->FindChildPartByID(&slider, 'repa');
	ThrowIfNil(slider);
	slider->SetValue(&(TVector2::kUnitY), kVector2ValueType, true, false);

	TBasicPart::FinishCreateFromResource();
}

void TypePart::SelfDraw(IMCGraphicContext* inContext, const TMCRect& inZone)
{
	const int32 rectCount = fModeRects.GetElemCount();
	if(fEraseRectCount<=rectCount)
	{
		inContext->SetEraseColor( gPersonalityUtilities->GetUIColor(kUIColor_MajorAccent) );
		for(int32 i=0 ; i<fEraseRectCount ; i++)
		{
			inContext->EraseRect(fModeRects[i]);
		}
	}
}


boolean	TypePart::Receive(int32 message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

	switch (message)
	{
	case EMFPartMessage::kMsg_SendToAllInWindow:
		{
			// When loading data from file, need to update the ui
			IMFPart* thisPart = GetThisPartNoAddRef();
			if(!thisPart)
				break;
			TMCCountedPtr<IMFPart> modePart;
			thisPart->FindChildPartByID(&modePart, 'RMod');
			if(!modePart)
				break;
			int32 option = 0;
			modePart->GetValue(&option, kInt32ValueType);
			DisplayOption(option);
		} break;
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					if(!fIsInit)
					{
						SetTreeItem(0);
						fIsInit = true;
					}

					const IDType partID= part->GetIMFPartID();

					switch (partID)
					{
					case 'Tree':
						{	// Get the tree ID and record it

							int32 item = 0;
							part->GetValue(&item, kInt32ValueType);

							SetTreeItem(item);
						} break;
					case 'RMod':
						{
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);
							DisplayOption(option);
						} break;
					case 'CuPr':
						{	// Cube presets
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);

							if(option!=kNoOption)
							{
								SetPreset(eCube, option);
							
								option = kNoOption;
								part->SetValue(&option, kInt32ValueType, true, true);

								handledMessage = true;
							}
						} break;
					case 'CyPr':
						{	// Cylinder presets
							int32 option = 0;
							part->GetValue(&option, kInt32ValueType);

							if(option!=kNoOption)
							{
								SetPreset(eCylinder, option);
							
								option = kNoOption;
								part->SetValue(&option, kInt32ValueType, true, true);

								handledMessage = true;
							}
						} break;
					case 'repa':
						{
							// Get the min and max for the repartition
							TVector2 minMax;
							part->GetValue(&minMax, kVector2ValueType);
							real32 min=0,max=0;
							if(minMax.x<minMax.y)
							{
								min = minMax.x;
								max = minMax.y;
							}
							else
							{
								min = minMax.y;
								max = minMax.x;
							}
						
							IMFPart* thisPart = GetThisPartNoAddRef();
							if(!thisPart)
								break;
							TMCCountedPtr<IMFPart> minPart;
							thisPart->FindChildPartByID(&minPart, 'RMin');
							ThrowIfNil(minPart);
							minPart->SetValue(&min, kReal32ValueType, true, true);
							TMCCountedPtr<IMFPart> maxPart;
							thisPart->FindChildPartByID(&maxPart, 'RMax');
							ThrowIfNil(maxPart);
							maxPart->SetValue(&max, kReal32ValueType, true, true);

						}break;
					default: break;
					}
				}
			}
		}
	}

	return handledMessage;
}

void TypePart::SetTreeItem(const int32 item)
{
	if(item>=(int32)fTreePermanentID.GetElemCount())
		return;

	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;
	TMCCountedPtr<IMFPart> treeIDPart;
	thisPart->FindChildPartByID(&treeIDPart, 'TPid');
	ThrowIfNil(treeIDPart);

	treeIDPart->SetValue(&(fTreePermanentID[item]), kInt32ValueType, true, true);
}

void TypePart::SetPreset(EReplicateMode mode, const int32 option)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	switch(mode)
	{
	case eCube:
		{
			// Find the cube parts and set their new values
			TMCCountedPtr<IMFPart> XSpace;
			thisPart->FindChildPartByID(&XSpace, 'XSpa');
			ThrowIfNil(XSpace);

			TMCCountedPtr<IMFPart> YSpace;
			thisPart->FindChildPartByID(&YSpace, 'YSpa');
			ThrowIfNil(YSpace);

			TMCCountedPtr<IMFPart> ZSpace;
			thisPart->FindChildPartByID(&ZSpace, 'ZSpa');
			ThrowIfNil(ZSpace);

			TMCCountedPtr<IMFPart> XCount;
			thisPart->FindChildPartByID(&XCount, 'XCou');
			ThrowIfNil(XCount);

			TMCCountedPtr<IMFPart> YCount;
			thisPart->FindChildPartByID(&YCount, 'YCou');
			ThrowIfNil(YCount);

			TMCCountedPtr<IMFPart> ZCount;
			thisPart->FindChildPartByID(&ZCount, 'ZCou');
			ThrowIfNil(ZCount);

			int32 xValue = 1;
			int32 yValue = 1;
			int32 zValue = 1;
			switch(option)
			{
			case 'Opt0':
				{	// X line
					xValue = 5;
				} break;
			case 'Opt1':
				{	// Yline
					yValue = 5;
				} break;
			case 'Opt2':
				{	// Z line
					zValue = 5;
				} break;
			case 'Opt3':
				{	// small plane
					xValue = 5;
					yValue = 5;
				} break;
			case 'Opt4':
				{	// large plane
					xValue = 10;
					yValue = 10;
				} break;
			case 'Opt5':
				{	// cube
					xValue = 5;
					yValue = 5;
					zValue = 5;
				} break;
			}
			XCount->SetValue(&xValue, kInt32ValueType, true, true);
			YCount->SetValue(&yValue, kInt32ValueType, true, true);
			ZCount->SetValue(&zValue, kInt32ValueType, true, true);
		} break;
	case eCylinder:
		{
			// Find the cylinder parts and set their new values
			TMCCountedPtr<IMFPart> RCount;
			thisPart->FindChildPartByID(&RCount, 'RCou');
			ThrowIfNil(RCount);

			TMCCountedPtr<IMFPart> ZCount;
			thisPart->FindChildPartByID(&ZCount, 'CZCo');
			ThrowIfNil(ZCount);

			TMCCountedPtr<IMFPart> anglePart;
			thisPart->FindChildPartByID(&anglePart, 'Angl');
			ThrowIfNil(anglePart);

			TMCCountedPtr<IMFPart> torquePart;
			thisPart->FindChildPartByID(&torquePart, 'torq');
			ThrowIfNil(torquePart);

			int32 rValue = 16;
			int32 zValue = 1;
			real32 angle = 360;
			boolean torque = false;
			switch(option)
			{
			case 'Opt0':
				{	// quarter
					angle = 90;
					rValue = 5;
				} break;
			case 'Opt1':
				{	// half circle
					angle = 180;
					rValue = 9;
				} break;
			case 'Opt2':
				{	// 3/4
					angle = 270;
					rValue = 13;
				} break;
			case 'Opt3':
				{	// Normal circle
				} break;
			case 'Opt4':
				{	// Torque circle
					torque = true;
				} break;
			case 'Opt5':
				{	// Cylinder
					zValue = 4;
				} break;
			}
			angle = DegToRad(angle);
			RCount->SetValue(&rValue, kInt32ValueType, true, true);
			ZCount->SetValue(&zValue, kInt32ValueType, true, true);
			anglePart->SetValue(&angle, kReal32ValueType, true, true);
			torquePart->SetValue(&torque, kBooleanValueType, true, true);
		} break;
	}
}

void TypePart::BuildTreePopupMenu()
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCCountedPtr<IMFPart> popup;
	thisPart->FindChildPartByID(&popup, 'Tree');
	ThrowIfNil(popup);

	TMCCountedPtr<IMFTextPopupPart> treeDomainPopup;
	popup->QueryInterface( IID_IMFTextPopupPart, (void **)&treeDomainPopup );
	ThrowIfNil(treeDomainPopup);

	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');

	// Clean the current popup menu
	treeDomainPopup->RemoveAll();

	// Add the trees with a geometry
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();

	// Do not include the current selection
	TMCCountedPtr<ISceneSelection> selection;
	currentDoc->GetSceneSelection(&selection);
	ThrowIfNil(selection);

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

	if (currentDoc)
	{
		// Get the scene
		TMCCountedPtr<I3DShScene> scene;
		currentDoc->GetScene(&scene);

		TMCArray<int32> selectedTreeIDs;
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
	//				selectedTreeIDs.AddElem(tree->GetTreePermanentID());
					selectedTreeIDs.AddElem(scene->GetTreeIndex(tree));
				}
			}
		}

		const int32 treeCount = scene->GetTreesCount();
		for (int32 iTree = 0; iTree<treeCount; iTree++)
		{
			I3DShTreeElement* tree = scene->GetTreeByIndex(iTree);

			TMCCountedPtr<I3DShInstance> instance;
			tree->QueryInterface(IID_I3DShInstance, (void **)&instance);

			if(instance && instance->GetInstanceKind() == I3DShInstance::kPrimitiveInstance)
			{
				// Check that instance has a geometry
				TMCCountedPtr<I3DShObject> object;
				instance->Get3DObject(&object);

				if(object)
				{
					if(selectedTreeIDs.FindElem(iTree) == kUnusedIndex)
					{
						fTreePermanentID.AddElem(iTree);

						TMCDynamicString name;
						tree->GetName(name);

						treeDomainPopup->AppendMenuItem(name);
					}
				}
			}
		}

/*		const int32 instanceCount = scene->GetInstanceListCount();

		// Fill the menu with the shading domains
		for (int32 iInstance = 0; iInstance<instanceCount; iInstance++)
		{
			TMCCountedPtr<I3DShInstance> instance;
			scene->GetInstanceByIndex(&instance, iInstance);

			if(instance->GetInstanceKind() == I3DShInstance::kPrimitiveInstance)
			{
				// Check that instance has a geometry
				TMCCountedPtr<I3DShObject> object;
				instance->Get3DObject(&object);

				if(object)
				{
					// Get the tree name and ID
					TMCCountedPtr<I3DShTreeElement> tree;
					instance->QueryInterface(IID_I3DShTreeElement, (void **)&tree);

					const int32 treeID = tree->GetTreePermanentID();
					if(selectedTreeIDs.FindElem(treeID) == kUnusedIndex)
					{
						fTreePermanentID.AddElem(treeID);

						TMCDynamicString name;
						tree->GetName(name);

						treeDomainPopup->AppendMenuItem(name);
					}
				}
			}
		}*/
	}
}

void TypePart::DisplayOption( const int32 option )
{
	// Get all the parts
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;

	TMCCountedPtr<IMFPart> cubePart;
	thisPart->FindChildPartByID(&cubePart,kCubeOptionsPart);
	TMCCountedPtr<IMFPart> cylinderPart;
	thisPart->FindChildPartByID(&cylinderPart,kCylinderOptionsPart);
	TMCCountedPtr<IMFPart> surfacePart;
	thisPart->FindChildPartByID(&surfacePart,kSurfaceOptionsPart);

	cubePart->SetShown(false);
	cylinderPart->SetShown(false);
	surfacePart->SetShown(false);

	// Show/Hide depending on the kind of map
	switch(option)
	{
	case kCubeType:
		{
			fEraseRectCount = 9;
			cubePart->SetShown(true);
		} break;
	case kCylinderType:
		{
			fEraseRectCount = 9;
			cylinderPart->SetShown(true);
		} break;
	case kSurfaceType:
		{
			fEraseRectCount = 3;
			surfacePart->SetShown(true);
		} break;
	}
}


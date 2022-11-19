/****************************************************************************************************

		MChildrenActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/9/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MChildrenActions.h"

#include "I3DShObject.h"
#include "MiscComUtilsImpl.h"
#include "MBuildingAction.h"

///////////////////////////////////////////////////////////////////////////
//
//

AttachObjectAction::AttachObjectAction( BuildingModeler*	modeler,
										TMCString&			objectName,
										const boolean		isObject)
									:
									ModelerAction(modeler)//,
								//	BuildingRecorder(modeler->GetBuildingNoAddRef())
{
	fObjectName = objectName;
	fRefreshGeometry = true;
	fIsObject = isObject;
}

void AttachObjectAction::Create(IShAction**			outAction,
								BuildingModeler*	modeler,
								TMCString&			objectName,
								const boolean		isObject)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new AttachObjectAction(modeler,objectName, isObject));
}

MCCOMErr AttachObjectAction::Do()
{
	try
	{
//		SaveBuilding(fBuildingPrimitive);

		// Get the object
		I3DShScene* scene = fBuildingModeler->GetScene();
		TMCCountedPtr<I3DShObject> sceneObject;
		scene->Get3DObjectByName(&sceneObject, fObjectName);

		fBuildingPrimitive->AttachObjectToSelection(sceneObject, kAllLevels, fPrevObjects);

		fBuildingPrimitive->InvalidateStatus();
	
		fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
	//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("AttachObjectAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("AttachObjectAction::Do"));
	}

	return ModelerAction::Do();
}

MCCOMErr AttachObjectAction::Undo()
{
//	SwapBuilding(fBuildingPrimitive);	
	TMCCountedPtrArray<I3DShInstance> instances;
	fBuildingPrimitive->GetInstances(instances);
	I3DShScene* scene = fBuildingModeler->GetScene();

	const int32 levelCount = fBuildingPrimitive->GetLevelCount();

	int32 index=0;
	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fBuildingPrimitive->GetLevel(iLevel);

		if (level->Hidden())
			continue;

		TMCCountedPtrArray<WallSubObject> wallObj;
		TMCCountedPtrArray<RoomSubObject> roomObj;
		level->LevelPlan().GetSelectedObjects(wallObj, roomObj);

		{	// Attach to the selected wall objects
			const int32 wallObjCount = wallObj.GetElemCount();
			for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
			{
				SubObject* subObject = wallObj[iObj];

				// Remove the instance we added
				subObject->SetSceneObject(NULL);

				// Hook the previous one
				// Get the object
				TMCCountedPtr<I3DShObject> sceneObject;
				scene->Get3DObjectByName(&sceneObject, fPrevObjects[index++]);

				/*I3DShInstance* newInstance =*/ subObject->SetSceneObject(sceneObject);
			}
		}

		{	// Attach to the selected room objects
			const int32 roomObjCount = roomObj.GetElemCount();
			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				// First unhook the previous instance used for this object
				SubObject* subObject = roomObj[iObj];

				// Remove the instance we added
				subObject->SetSceneObject(NULL);

				// Hook the previous one
				// Get the object
				TMCCountedPtr<I3DShObject> sceneObject;
				scene->Get3DObjectByName(&sceneObject, fPrevObjects[index++]);

				/*I3DShInstance* newInstance =*/ subObject->SetSceneObject(sceneObject);
			}
		}
	}

	fBuildingPrimitive->InvalidateStatus();

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerAction::Undo();
}

MCCOMErr AttachObjectAction::Redo()
{
//	SwapBuilding(fBuildingPrimitive);
	// Get the object
	I3DShScene* scene = fBuildingModeler->GetScene();
	TMCCountedPtr<I3DShObject> sceneObject;
	scene->Get3DObjectByName(&sceneObject, fObjectName);

	fBuildingPrimitive->AttachObjectToSelection(sceneObject, kAllLevels, fPrevObjects);
	fBuildingPrimitive->InvalidateStatus();

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);
	
	return ModelerAction::Redo();
}

MCCOMErr AttachObjectAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 15);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

PlaceObjectChildAction::PlaceObjectChildAction( BuildingModeler*	modeler,
											const EPlacementType placement )
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fPlacement = placement;
	fRefreshGeometry = true;
}

void PlaceObjectChildAction::Create(IShAction**			outAction,
								BuildingModeler*	modeler,
								const EPlacementType placement )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new PlaceObjectChildAction(modeler,placement) );
}

MCCOMErr PlaceObjectChildAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fBuildingPrimitive->SetPlacementType(fPlacement,kAllLevels);
		fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("PlaceObjectChildAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("PlaceObjectChildAction::Do"));
	}

	return ModelerAction::Do();
}

MCCOMErr PlaceObjectChildAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	
	return ModelerAction::Undo();
}

MCCOMErr PlaceObjectChildAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	return ModelerAction::Redo();
}

MCCOMErr PlaceObjectChildAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 38);

	return MC_S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
//

CustomPlaceObjectChildAction::CustomPlaceObjectChildAction( BuildingModeler*	modeler,
											const real32		value,
											const int32			id)
									:
									ModelerAction(modeler),
									BuildingRecorder()
{
	fValue = value;
	fID = id;
	fRefreshGeometry = true;
}

void CustomPlaceObjectChildAction::Create(IShAction**			outAction,
								BuildingModeler*	modeler,
								const real32		value,
								const int32			id)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new CustomPlaceObjectChildAction(modeler,value, id) );
}

MCCOMErr CustomPlaceObjectChildAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		fBuildingPrimitive->SetPlacement(fValue,fID,kAllLevels);
		fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("CustomPlaceObjectChildAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("CustomPlaceObjectChildAction::Do"));
	}

	return ModelerAction::Do();
}

MCCOMErr CustomPlaceObjectChildAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	
	return ModelerAction::Undo();
}

MCCOMErr CustomPlaceObjectChildAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	return ModelerAction::Redo();
}

MCCOMErr CustomPlaceObjectChildAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 38);

	return MC_S_OK;
}

#endif // !NETWORK_RENDERING_VERSION


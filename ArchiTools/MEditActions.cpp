/****************************************************************************************************

		MEditActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	6/3/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MEditActions.h"
#include "MFPublicDefs.h"
#include "COMUtilities.h"
#include "MBuildingAction.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "COMSafeUtilities.h"
#include "BasicMCFCOMImplementations.h"
#endif

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
const MCGUID CLSID_BuildingClip(R_CLSID_BuildingClip);
#else
const MCGUID CLSID_BuildingClip={R_CLSID_BuildingClip};
#endif

void BuildingClipping::Create(BuildingClipping** outClipping, const ClipboardData& clipData )
{
	TMCCountedCreateHelper<BuildingClipping> result(outClipping);
	result = new BuildingClipping('Buil', clipData);
}

BuildingClipping::BuildingClipping( IDType inDataType, const ClipboardData& clipData)
:	fType(inDataType)
{
	fData.fLevels = clipData.fLevels;
	fData.fWallObjects = clipData.fWallObjects;
	fData.fRoomObjects = clipData.fRoomObjects;
	fData.fBotProfilePoints = clipData.fBotProfilePoints;
	fData.fTopProfilePoints = clipData.fTopProfilePoints;
	fData.fBotInsidePoints = clipData.fBotInsidePoints;
	fData.fTopInsidePoints = clipData.fTopInsidePoints;
}

BuildingClipping::BuildingClipping()
://	fData(NULL),
	fType(kMFClipping_Empty)
{
}

MCCOMErr BuildingClipping::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IMFClipping))
	{
		TMCCountedGetHelper<IMFClipping> result(ppvObj);
		result = static_cast<IMFClipping*>(this);
		return MC_S_OK;
	}
	else if (MCIsEqualIID(riid, CLSID_BuildingClip))
	{
		TMCCountedGetHelper<BuildingClipping> result(ppvObj);
		result = static_cast<BuildingClipping*>(this);
		return MC_S_OK;
	}
	return TBasicUnknown::QueryInterface(riid, ppvObj);
}

Roof* GetFirstSelectedRoofPoint(BuildingPrim* buildingPrimitive, int32& index, boolean& inside, boolean& top)
{
	const int32 levelCount = buildingPrimitive->GetLevelCount();
	for(int32 iLevel=levelCount-1 ; iLevel>=0 ; iLevel--)
	{
		Level* level = buildingPrimitive->GetLevel(iLevel);
		const int32 roofCount = level->GetRoofCount();
		for(int32 iRoof=0 ; iRoof<roofCount ; iRoof++)
		{
			Roof* roof = level->GetRoof(iRoof);

			// Check the profile selection
			{
				const int32 count = roof->GetBotProfilePointCount();
				for(int32 i=0 ; i<count ; i++)
				{
					if(roof->GetBotProfilePoint(i)->Selected())
					{
						index = i;
						inside = false;
						top = false;
						return roof;
					}
				}
			}
			{
				const int32 count = roof->GetTopProfilePointCount();
				for(int32 i=0 ; i<count ; i++)
				{
					if(roof->GetTopProfilePoint(i)->Selected())
					{
						index = i;
						inside = false;
						top = true;
						return roof;
					}
				}
			}
			{
				const int32 count = roof->GetBotInsidePointCount();
				for(int32 i=0 ; i<count ; i++)
				{
					if(roof->GetBotInsidePoint(i)->Selected())
					{
						index = i;
						inside = true;
						top = false;
						return roof;
					}
				}
			}
			{
				const int32 count = roof->GetTopInsidePointCount();
				for(int32 i=0 ; i<count ; i++)
				{
					if(roof->GetTopInsidePoint(i)->Selected())
					{
						index = i;
						inside = true;
						top = true;
						return roof;
					}
				}
			}
		}
	}

	return NULL;
}

boolean Copy(	ClipboardData& clipData, BuildingPrim* buildingPrimitive)
{
	const int32 levelCount = buildingPrimitive->GetLevelCount();

	PrimitiveStatus* status = buildingPrimitive->GetStatus();

	if( status->fSelectedPointCount ||
		status->fSelectedRoofCount )
	{	// Copy Walls, Rooms and Roofs
		clipData.fLevels.SetElemCount(levelCount);
		
		{
			for(int32 iLevel=0;iLevel<levelCount;iLevel++)
			{
				TMCCountedPtr<Level> clonedLevel;
				buildingPrimitive->GetLevel(iLevel)->Clone(&clonedLevel,buildingPrimitive, eNoChild); // Record the ID for a later use during the Paste

				clipData.fLevels.SetElem( iLevel, clonedLevel );

				// Parse the clone to arrange things:
				// If a room is selected, we must have the walls arround selected too
				const int32 roomCount = clonedLevel->GetRoomCount();
				for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
				{
					Room* room = clonedLevel->GetRoom(iRoom);
					if(room->Selected())
					{
						room->SetWallFlag(eIsSelected);
					}
				}
			}
		}

		// Now parse the cloned levels to remove unselected parts
		{
			for(int32 iLevel=0;iLevel<levelCount;iLevel++)
			{
				TMCCountedPtr<Level> copy;
				copy = clipData.fLevels[iLevel];

				copy->ShowHide(eShowAll);
				copy->LevelPlan().InvertSelection();
				copy->LevelPlan().DeleteSelection();
				copy->SetSelection(true);
			}
		}
	}

	if( clipData.fLevels.GetElemCount() == 0 ||
		(clipData.fLevels.GetElemCount() == 1 &&
		 clipData.fLevels[0]->GetPointCount() == 0 &&
		 clipData.fLevels[0]->GetRoofCount() == 0) )
	{	// the copy is empty, maybe we were trying to copy an object:
		// reparse the primitive but keep only the selected object ( and place them on unselected
		// wall or room ?)
		clipData.fLevels.ArrayFree(); // be safe

		if( status->fSelectedRoomObjectCount ||
			status->fSelectedWallObjectCount )
		{
			TMCCountedPtrArray<WallSubObject> wallObjPtrs;
			TMCCountedPtrArray<RoomSubObject> roomObjPtrs;
			for(int32 iLevel=0;iLevel<levelCount;iLevel++)
			{
				// this return pointers on the objects, we then need to clone them
				buildingPrimitive->GetLevel(iLevel)->LevelPlan().GetSelectedObjects(wallObjPtrs, roomObjPtrs);
			}
			{
				const int32 wallObjCount = wallObjPtrs.GetElemCount();
				clipData.fWallObjects.SetElemCount(wallObjCount);
				for(int32 iObj=0 ; iObj<wallObjCount ; iObj++)
				{
					wallObjPtrs[iObj]->Clone(clipData.fWallObjects.Pointer(iObj), NULL, eNoChild); // Record the ID for a later use during the Paste
				}
			}
			{
				const int32 roomObjCount = roomObjPtrs.GetElemCount();
				clipData.fRoomObjects.SetElemCount(roomObjCount);
				for(int32 iObj=0 ; iObj<roomObjCount ; iObj++)
				{
					roomObjPtrs[iObj]->Clone(clipData.fRoomObjects.Pointer(iObj), NULL, eNoChild); // Record the ID for a later use during the Paste
				}
			}
		}
	}
	else
		return true;

	if( !clipData.fLevels.GetElemCount() &&
		!clipData.fWallObjects.GetElemCount() &&
		!clipData.fRoomObjects.GetElemCount() )
	{
		if( status->fSelectedProfilePointCount )
		{
			// Copy the profile points: find the first roof with selection on the profile
			int32 index=0;
			boolean top=false, in=false;
			Roof* onRoof = GetFirstSelectedRoofPoint(buildingPrimitive,index,in,top);

			if(onRoof)
			{
				// Copy the selected points
				{
					const int32 count = onRoof->GetBotProfilePointCount();
					for(int32 i=0 ; i<count ; i++)
					{
						if(onRoof->GetBotProfilePoint(i)->Selected())
						{
							const int32 index = clipData.fBotProfilePoints.GetElemCount();
							clipData.fBotProfilePoints.AddElemCount(1);
							onRoof->GetBotProfilePoint(i)->Clone(clipData.fBotProfilePoints.Pointer(index), NULL);
						}
					}
				}
				{
					const int32 count = onRoof->GetTopProfilePointCount();
					for(int32 i=0 ; i<count ; i++)
					{
						if(onRoof->GetTopProfilePoint(i)->Selected())
						{
							const int32 index = clipData.fTopProfilePoints.GetElemCount();
							clipData.fTopProfilePoints.AddElemCount(1);
							onRoof->GetTopProfilePoint(i)->Clone(clipData.fTopProfilePoints.Pointer(index), NULL);
						}
					}
				}
				{
					const int32 count = onRoof->GetBotInsidePointCount();
					for(int32 i=0 ; i<count ; i++)
					{
						if(onRoof->GetBotInsidePoint(i)->Selected())
						{
							const int32 index = clipData.fBotInsidePoints.GetElemCount();
							clipData.fBotInsidePoints.AddElemCount(1);
							onRoof->GetBotInsidePoint(i)->Clone(clipData.fBotInsidePoints.Pointer(index), NULL);
						}
					}
				}
				{
					const int32 count = onRoof->GetTopInsidePointCount();
					for(int32 i=0 ; i<count ; i++)
					{
						if(onRoof->GetTopInsidePoint(i)->Selected())
						{
							const int32 index = clipData.fTopInsidePoints.GetElemCount();
							clipData.fTopInsidePoints.AddElemCount(1);
							onRoof->GetTopInsidePoint(i)->Clone(clipData.fTopInsidePoints.Pointer(index), NULL);
						}
					}
				}
		
				return true;
			}
		}
		else if( status->fSelectedRoofPointCount )
		{	// Copy the roof section
		// TO DO
				return false;
		}
	}
	else
		return true;

	return false;
}

boolean Paste( const ClipboardData& clipData, BuildingPrim* buildingPrimitive)
{
	const TMCCountedPtrArray<Level>& levels = clipData.fLevels;
	// Parse array and clone...
	const int32 levelCount = levels.GetElemCount();
	if(levelCount)
	{
		buildingPrimitive->SetSelection(false);

		for(int32 iLevel=levelCount-1;iLevel>=0;iLevel--) // Start by the end in case we add a level under
		{
			TMCCountedPtr<Level> copy;
			copy = levels[iLevel];

			TMCCountedPtr<Level> clonedLevel;
			copy->Clone( &clonedLevel,buildingPrimitive, eCloneChild ); // The children ID's should have been recorded during the Copy
			if(!clonedLevel->IsEmpty())
			{
				buildingPrimitive->MergeLevel( clonedLevel, clipData.fLevelOffset );

				// Set the extended selection
				buildingPrimitive->BuildExtendedSelection();
			}

			buildingPrimitive->InvalidateBBox();
		}
	}
	else
	{	// We may have objects to paste (paste on a selected wall or room)
		const TMCCountedPtrArray<WallSubObject>& wallObjs = clipData.fWallObjects;
		const int32 wallObjCount = wallObjs.GetElemCount();
		if(wallObjCount)
		{
			// Clone them in the first selected wall
			Wall* onWall = buildingPrimitive->GetFirstSelectedWall(kAllLevels);
			if(onWall)
			{
				TMCCountedPtrArray<WallSubObject> newObjects;
				newObjects.SetElemCount(wallObjCount);
				{
					for(int32 iObj=0 ; iObj<wallObjCount ; iObj++)
					{
						wallObjs[iObj]->Clone(newObjects.Pointer(iObj), onWall, eCloneChild ); // The children ID's should have been recorded during the Copy
						// move the object so they fit in the wall
						newObjects[iObj]->CheckObjectPosition();
					}
				}
				// Then select them
				buildingPrimitive->SetSelection(false);
				{
					for(int32 iObj=0 ; iObj<wallObjCount ; iObj++)
					{
						newObjects[iObj]->SetSelection(true);
					}
				}

			}
		}

		// Room objects
		const TMCCountedPtrArray<RoomSubObject>& roomObjs = clipData.fRoomObjects;
		const int32 roomObjCount = roomObjs.GetElemCount();
		if(roomObjCount)
		{
			// Clone them in the first selected wall
			Room* inRoom = buildingPrimitive->GetFirstSelectedRoom( kAllLevels );
			if(inRoom)
			{
				TMCCountedPtrArray<RoomSubObject> newObjects;
				newObjects.SetElemCount(roomObjCount);
				{
					for(int32 iObj=0 ; iObj<roomObjCount ; iObj++)
					{
						roomObjs[iObj]->Clone(newObjects.Pointer(iObj), inRoom, eCloneChild ); // The children ID's should have been recorded during the Copy
						// move the object so they fit in the wall
						newObjects[iObj]->CheckObjectPosition();
					}
				}
				// Then select them
				buildingPrimitive->SetSelection(false);
				{
					for(int32 iObj=0 ; iObj<roomObjCount ; iObj++)
					{
						newObjects[iObj]->SetSelection(true);
					}
				}

			}
		}

		if(!wallObjCount && !roomObjCount)
		{	// Check if we have profile points to paste
			// If we've got a roof selected, just paste them after the one already existing
			// else if we've got profile point selected, paste it after the selection
			const TMCCountedPtrArray<ProfilePoint>& outBot = clipData.fBotProfilePoints;
			const TMCCountedPtrArray<ProfilePoint>& outTop = clipData.fTopProfilePoints;
			const TMCCountedPtrArray<ProfilePoint>& inBot = clipData.fBotInsidePoints;
			const TMCCountedPtrArray<ProfilePoint>& inTop = clipData.fTopInsidePoints;
			const int32 outBotCount = outBot.GetElemCount();
			const int32 outTopCount = outTop.GetElemCount();
			const int32 inBotCount = inBot.GetElemCount();
			const int32 inTopCount = inTop.GetElemCount();

			if(outBotCount || outTopCount || inBotCount || inTopCount)
			{
				PrimitiveStatus* status = buildingPrimitive->GetStatus();
				if(status->fSelectedRoofCount)
				{
					Roof* onRoof = buildingPrimitive->GetFirstSelectedRoof(kAllLevels);
					{
						for(int32 i=0 ; i<outBotCount ; i++)
						{
							TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetBotProfile();
							const int32 index = profile.GetElemCount();
							profile.AddElemCount(1);
							outBot[i]->Clone(profile.Pointer(index), onRoof);
						}
					}
					{
						for(int32 i=0 ; i<outTopCount ; i++)
						{
							TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetTopProfile();
							const int32 index = profile.GetElemCount();
							profile.AddElemCount(1);
							outTop[i]->Clone(profile.Pointer(index), onRoof);
						}
					}
					{
						for(int32 i=0 ; i<inBotCount ; i++)
						{
							TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetBotInside();
							const int32 index = profile.GetElemCount();
							profile.AddElemCount(1);
							inBot[i]->Clone(profile.Pointer(index), onRoof);
						}
					}
					{
						for(int32 i=0 ; i<inTopCount ; i++)
						{
							TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetTopInside();
							const int32 index = profile.GetElemCount();
							profile.AddElemCount(1);
							inTop[i]->Clone(profile.Pointer(index), onRoof);
						}
					}
					buildingPrimitive->BuildExtendedSelection();
					buildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				}
				else if(status->fSelectedProfilePointCount)
				{
					int32 index=0;
					boolean top=false, in=false;
					Roof* onRoof = GetFirstSelectedRoofPoint(buildingPrimitive,index,in,top);
					if(onRoof)
					{
						buildingPrimitive->SetSelection(false);
						if(in)
						{
							if(top && inTopCount)
							{
								for(int32 i=0 ; i<inTopCount ; i++)
								{
									TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetTopInside();
									TMCCountedPtr<ProfilePoint> newPoint;
									inTop[i]->Clone(&newPoint, onRoof);
									profile.InsertElem(index++, newPoint);
									newPoint->ClearFlag(eIsSelected);
									newPoint->SetSelection(true);
								}
							}
							else if(inBotCount)
							{
								for(int32 i=0 ; i<inBotCount ; i++)
								{
									TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetBotInside();
									TMCCountedPtr<ProfilePoint> newPoint;
									inBot[i]->Clone(&newPoint, onRoof);
									profile.InsertElem(index++, newPoint);
									newPoint->ClearFlag(eIsSelected);
									newPoint->SetSelection(true);
								}
							}
						}
						else
						{
							if(top && outTopCount)
							{
								for(int32 i=0 ; i<outTopCount ; i++)
								{
									TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetTopProfile();
									TMCCountedPtr<ProfilePoint> newPoint;
									outTop[i]->Clone(&newPoint, onRoof);
									profile.InsertElem(index++, newPoint);
									newPoint->ClearFlag(eIsSelected);
									newPoint->SetSelection(true);
								}
							}
							else if(outBotCount)
							{
								for(int32 i=0 ; i<outBotCount ; i++)
								{
									TMCCountedPtrArray<ProfilePoint>& profile = onRoof->GetBotProfile();
									TMCCountedPtr<ProfilePoint> newPoint;
									outBot[i]->Clone(&newPoint, onRoof);
									profile.InsertElem(index++, newPoint);
									newPoint->ClearFlag(eIsSelected);
									newPoint->SetSelection(true);
								}
							}
						}
					}
					buildingPrimitive->BuildExtendedSelection();
					buildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				}
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
//
// Cut Copy Action
//
CutCopyAction::CutCopyAction( BuildingModeler*	modeler, const int32 actionNumber )
							:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fActionNumber = actionNumber;
	fRefreshGeometry = true;
}

void CutCopyAction::Create(IShAction** outAction, BuildingModeler*	modeler, const int32 actionNumber)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new CutCopyAction(modeler, actionNumber);
}

CutCopyAction::~CutCopyAction () 
{
}

MCCOMErr MCCOMAPI CutCopyAction::Do()	
{
	SaveBuilding(fBuildingPrimitive);
	
	ClipboardData clipData;
	if( Copy(clipData,fBuildingPrimitive) )
	{
		// Cut case: delete the selection
		if(fActionNumber==kaCut) 
		{
			fBuildingPrimitive->DeleteSelection(kAllLevels);
		}

		// Copy to clipboard...
		TMCCountedPtr<BuildingClipping> buildingClipping;
		BuildingClipping::Create( &buildingClipping, clipData);
		ThrowIfNil( buildingClipping );

		gClipboardUtilities->Put( buildingClipping, 0 );

		return ModelerAction::Do();
	}

	return MC_S_FALSE;
}

MCCOMErr CutCopyAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr CutCopyAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

///////////////////////////////////////////////////////////////////////////
//
// 
PasteAction::PasteAction(BuildingModeler*	modeler )
									:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
}

void PasteAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new PasteAction(modeler);
}

MCCOMErr PasteAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		TMCCountedPtr<IMFClipping> clip;
		if ( gClipboardUtilities->Get('Buil', &clip, 0 ) == kNoErr)
		{
			TMCCountedPtr<BuildingClipping> buildingClip;
			
			clip->QueryInterface( CLSID_BuildingClip, (void**)&buildingClip );
			ThrowIfNil( buildingClip );
			
			ClipboardData* data = static_cast<ClipboardData*>(buildingClip->GetData());

			if ( data!= NULL )
			{	
				Paste( *data, fBuildingPrimitive);
			}
		}
	}
	catch(TMCException& )
	{
		MCNotify("Paste Action exception");
	}

	return ModelerAction::Do();
}

MCCOMErr PasteAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr PasteAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

///////////////////////////////////////////////////////////////////////////
//
// 
DuplicateAction::DuplicateAction(BuildingModeler*	modeler, const boolean sym, const int32 levelOffset )
									:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fSymetrie = sym;
	fLevelOffset = levelOffset;
}

void DuplicateAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler, 
							const boolean		sym,
							const int32			levelOffset)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new DuplicateAction(modeler, sym, levelOffset);
}

MCCOMErr DuplicateAction::Do()
{
	SaveBuilding(fBuildingPrimitive);

	ClipboardData clipData(fLevelOffset);
	if( Copy(clipData,fBuildingPrimitive) )
	{

		fBuildingPrimitive->SetSelection(false);

		Paste( clipData, fBuildingPrimitive);

		if(fSymetrie)
		{
			// Get the symetrie center
			TBBox3D bbox;
			fBuildingPrimitive->GetBoundingBox(bbox,false,true);
			TVector3 center;
			bbox.GetCenter(center);
			// Get the symetrie plane. If it's the ground plane, do a double symetie
			const int32 planeIndex = fBuildingModeler->GetCurrentPlane();
			int32 axis = 0;
			switch(planeIndex)
			{
			case 0: axis = eZAxis; break;
			case 1: axis = eXAxis; break;
			case 2: axis = eYAxis; break;
			}

			fBuildingPrimitive->FlipSelection(center.CastToXY(), axis, kAllLevels);
		}

		return ModelerAction::Do();
	}
	return MC_S_FALSE;
}

MCCOMErr DuplicateAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr DuplicateAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

///////////////////////////////////////////////////////////////////////////
//
// 
CutAndPasteAction::CutAndPasteAction(BuildingModeler*	modeler, const int32 levelOffset )
									:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fLevelOffset = levelOffset;
}

void CutAndPasteAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler, 
							const int32			levelOffset)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new CutAndPasteAction(modeler, levelOffset);
}

MCCOMErr CutAndPasteAction::Do()
{
	SaveBuilding(fBuildingPrimitive);

	ClipboardData clipData(fLevelOffset); // build with the offset
	if( Copy(clipData,fBuildingPrimitive) )
	{
		// Cut the selection
		fBuildingPrimitive->DeleteSelection(kAllLevels);

		fBuildingPrimitive->SetSelection(false);

		// Paste it
		Paste( clipData, fBuildingPrimitive);

		return ModelerAction::Do();
	}
	return MC_S_FALSE;
}

MCCOMErr CutAndPasteAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	return ModelerAction::Undo();
}

MCCOMErr CutAndPasteAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	return ModelerAction::Redo();
}

///////////////////////////////////////////////////////////////////////////
//
// 
ShadingDomainAction::ShadingDomainAction(BuildingModeler*	modeler, 
										 const int32		domain,
										 const int32		menuID,
										 const TMCString&	name )
									:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fShadingDomain = domain;
	fMenuID = menuID;
	fDomainName = name;
}

void ShadingDomainAction::Create(	IShAction**	outAction,
							BuildingModeler*	modeler, 
							const int32			domain,
							const int32			menuID,
							const TMCString&	name )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new ShadingDomainAction(modeler, domain, menuID, name);
}

MCCOMErr ShadingDomainAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		if(fShadingDomain == kNoDomains)
		{	// Create a new domain
			fShadingDomain = fBuildingPrimitive->AddShadingDomain(fDomainName);
			fBuildingModeler->InvalidateDomainList();
		}
		
		// Set the domains on the selection
		if(fMenuID>=0)
			fBuildingPrimitive->SetShadingDomain(fShadingDomain,fMenuID);
	}
	catch(TMCException&)
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr ShadingDomainAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	if(fShadingDomain == kNoDomains)
		fBuildingModeler->InvalidateDomainList();

	return ModelerAction::Undo();
}

MCCOMErr ShadingDomainAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);
	
	if(fShadingDomain == kNoDomains)
		fBuildingModeler->InvalidateDomainList();

	return ModelerAction::Redo();
}

DelShadingDomainAction::DelShadingDomainAction(BuildingModeler*	modeler, 
										 const int32		domain )
									:
							ModelerAction(modeler),
							BuildingRecorder()
{
	fRefreshGeometry = true;
	fShadingDomain = domain;
}

void DelShadingDomainAction::Create(	IShAction**	outAction,
							BuildingModeler*	modeler, 
							const int32			domain  )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = (IShAction*) new DelShadingDomainAction(modeler, domain);
}

MCCOMErr DelShadingDomainAction::Do()
{
	try
	{
		SaveBuilding(fBuildingPrimitive);

		// Remove the domain
		fBuildingPrimitive->DelShadingDomain(fShadingDomain, kNoDomains);
		fBuildingModeler->InvalidateDomainList();
	}
	catch(TMCException& )
	{
		MCNotify("Catch exception");
	}

	return ModelerAction::Do();
}

MCCOMErr DelShadingDomainAction::Undo()
{
	SwapBuilding(fBuildingPrimitive);	

	fBuildingModeler->InvalidateDomainList();

	return ModelerAction::Undo();
}

MCCOMErr DelShadingDomainAction::Redo()
{
	SwapBuilding(fBuildingPrimitive);

	fBuildingModeler->InvalidateDomainList();

	return ModelerAction::Redo();
}

#endif // !NETWORK_RENDERING_VERSION


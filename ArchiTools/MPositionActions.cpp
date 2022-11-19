/****************************************************************************************************

		MPositionActions.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/4/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MPositionActions.h"
#include "MBuildingAction.h"

#include "BuildingModeler.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "Geometry.h"
#include "PBuildingVisitor.h"

void CheckScaleValue(TVector2& scale, const TVector2& diff, const boolean shift)
{
	// Some security check: <0 forbidden and near 0 too
	if(scale.x<0) scale.x*=-1;
	if(scale.y<0) scale.y*=-1;
	if(scale.x<.1f) scale.x=.1f;
	if(scale.y<.1f) scale.y=.1f;
	// Constraints
	if(!shift)
	{
		const real32 distX = RealAbs(diff.x); // Compare to the center
		const real32 distY = RealAbs(diff.y); // Compare to the center
		if(distX<.66*distY)
			scale.x = 1;
		else if(distY<.66*distX)
			scale.y = 1;
		else
		{
			scale.x = scale.y = .5*(scale.x + scale.y);
		}
	}
}

PositionRecorder::PositionRecorder()
{
}

void PositionRecorder::SavePosition(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

	PrimitiveStatus* status = primitive->GetStatus();

	fRecordedPointPos.SetElemCount(status->fSelectedCommonPointCount);

	int32 indexPoint = 0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		// Record the point positions
		const int32 pointCount = level->GetPointCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			if(level->GetPoint(iPoint)->Selected())
				fRecordedPointPos[indexPoint++] = level->GetPoint(iPoint)->Position();
		}

		// Record the roof point positions
		const int32 roofCount = level->GetRoofCount();
		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			// Record the zone pos
			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zonePoint = roof->GetRoofZoneSection(iPt);

				if(zonePoint.fZonePoint->Selected())
					fRecordedPointPos[indexPoint++] = zonePoint.fZonePoint->Position();

				if(zonePoint.fSpinePoint->Selected())
					fRecordedPointPos[indexPoint++] = zonePoint.fSpinePoint->Position();
			}

			// Record the profile pos
			const int32 botCount = roof->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				ProfilePoint* point = roof->GetBotProfilePoint(iBot);
				if( point->Selected())
					fRecordedPointPos[indexPoint++] = point->Position();
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				ProfilePoint* point = roof->GetTopProfilePoint(iTop);
				if( point->Selected())
					fRecordedPointPos[indexPoint++] = point->Position();
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
				if( point->Selected())
					fRecordedPointPos[indexPoint++] = point->Position();
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
				if( point->Selected())
					fRecordedPointPos[indexPoint++] = point->Position();
			}
		}

		// Record the positions of the attached room objects
		const int32 roomCount = level->GetRoomCount();

		for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
		{
			const Room* room = level->GetRoom(iRoom);

			const int32 objCount = room->GetObjectCount();
			for(int32 iObj=0 ; iObj<objCount ; iObj++)
			{
				RoomSubObject* obj = room->GetObject(iObj);
				if(obj->Selected())
				{
					fRecordedAttachedObjPos.AddElem(obj->GetPolylineCenter());
				}
			}
		}
		
	}
}

void PositionRecorder::SwapPosition(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

	//PrimitiveStatus* status = primitive->GetStatus();

	int32 index = 0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		const int32 pointCount = level->GetPointCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			if(level->GetPoint(iPoint)->Selected())
			{
				TVector2 pos = level->GetPoint(iPoint)->Position();
				level->GetPoint(iPoint)->SetPosition( fRecordedPointPos[index] );
				fRecordedPointPos[index] = pos;
				index++;
			}
		}

		// Restore the roof point positions
		const int32 roofCount = level->GetRoofCount();
		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			// Restore the zone pos
			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zonePoint = roof->GetRoofZoneSection(iPt);

				if(zonePoint.fZonePoint->Selected())
				{
					const TVector2 pos = zonePoint.fZonePoint->Position();
					zonePoint.fZonePoint->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
				if(zonePoint.fSpinePoint->Selected())
				{
					const TVector2 pos = zonePoint.fSpinePoint->Position();
					zonePoint.fSpinePoint->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
			}

			// Restore the profile pos
			const int32 botCount = roof->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				ProfilePoint* point = roof->GetBotProfilePoint(iBot);
				if( point->Selected())
				{
					const TVector2 pos = point->Position();
					point->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				ProfilePoint* point = roof->GetTopProfilePoint(iTop);
				if( point->Selected())
				{
					const TVector2 pos = point->Position();
					point->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
				if( point->Selected())
				{
					const TVector2 pos = point->Position();
					point->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
				if( point->Selected())
				{
					const TVector2 pos = point->Position();
					point->SetPosition( fRecordedPointPos[index] );
					fRecordedPointPos[index++] = pos;
				}
			}
		}
	
		// Restore the positions of the room objects
		const int32 roomCount = level->GetRoomCount();

		real32 objIndex=0;
		for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
		{
			const Room* room = level->GetRoom(iRoom);

			const int32 objCount = room->GetObjectCount();
			for(int32 iObj=0 ; iObj<objCount ; iObj++)
			{
				RoomSubObject* obj = room->GetObject(iObj);
				if(obj->Selected())
				{
					const TVector2 center = obj->GetPolylineCenter();
					obj->SetPolylineCenter(fRecordedAttachedObjPos[objIndex], false);
					fRecordedAttachedObjPos[objIndex] = center;
					objIndex++;
				}
			}
		}
	}

	primitive->InvalidateExtendedSelection(kAllLevels);
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
MoveMouseAction::MoveMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()//,
								//	BuildingRecorder(modeler->GetBuildingNoAddRef())
{
	fRefreshGeometry = true;
}

void MoveMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MoveMouseAction(modeler, pane, picked, mousePos);
}

void MoveMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
		//	SaveBuilding();
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);
			fIsWallPoint = true;
			// Get the reference point for snaping
			const TVector2 flatHitPos = fHitPos.CastToXY();
			switch(fPicked.GetPickedType())
			{
			case eRoofPointPicked: fIsWallPoint = false;
			case ePointPicked:
				{	// Use the picked point
					fReferencePoint = static_cast<CommonPoint*>(fPicked.PickedObject());
				} break;
			case eWallPicked:
			case eEdgePicked:
				{	// Use the nearest extremity
					Wall* wall = static_cast<Wall*>(fPicked.PickedObject());
					const TVector2& p0 = wall->GetPoint(0)->Position();
					const TVector2& p1 = wall->GetPoint(1)->Position();
					if((p0-flatHitPos).GetSquaredNorm()<(p1-flatHitPos).GetSquaredNorm())
						fReferencePoint = wall->GetPoint(0);
					else
						fReferencePoint = wall->GetPoint(1);
				} break;
			case eRoomFloorPicked:
			case eRoomCeilingPicked:
				{	// Snap the nearest point
					Room* room = static_cast<Room*>(fPicked.PickedObject());
					const int32 pathCount = room->GetPathPointCount();
					real32 minDist=kBigRealValue;
					for(int32 iPt=0 ; iPt<pathCount ; iPt++)
					{
						const TVector2& pos=room->GetPathPoint(iPt)->Position();
						const real32 dist=(pos-flatHitPos).GetSquaredNorm();
						if(dist<minDist)
						{
							minDist=dist;
							fReferencePoint=room->GetPathPoint(iPt);
						}
					}
				} break;
			default:
				fReferencePoint=NULL;
				break;
			}

			// For the CHeck consistency and to remove them from the picking
			fBuildingPrimitive->SetSelectionHelper( true,kAllLevels );
		/*	if(fReferencePoint && fIsWallPoint)
			{
				Point* point = static_cast<Point*>((void*)fReferencePoint);
				const int32 wallCount = point->GetWallCount();
				for(int32 iWall=0 ; iWall<wallCount ; iWall++)
				{	// to remove them from the picking
					point->GetWall(iWall)->SetFlag(eWallHelper);
				}
			}*/

			// prepare some snapping data
			fBuildingModeler->PreparePointConstraints(fReferencePoint, fRefDirections);

			// See if we're in a Side view
			fPlaneNormal = TVector3::kUnitZ;
			switch( fPanePart->GetCameraType() )
			{
			case kCanonicalCameraType_Left:
			case kCanonicalCameraType_Right: fPlaneNormal = TVector3::kUnitX; break;
			case kCanonicalCameraType_Front:
			case kCanonicalCameraType_Back: fPlaneNormal = TVector3::kUnitY; break;
			}

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
		//		if(!fReferencePoint)
		//			break;

				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);
				TVector2 firstRenderSpace=TVector2::kZero;
				TVector2 curRenderSpace=TVector2::kZero;

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// Wall, Room and Point: move only alowed in the Level plane
				// => the offset is the intersection of the (origin,direction)
				// with the plane (hitPos,Z) minus the hitpos.
				TVector3 offset;
				if( IntersectLinePlane2(origin,
										direction,
										fPlaneNormal,
										fHitPos,
										offset	) )
				{
					offset-=fHitPos;
					TVector2 flatOffset = offset.CastToXY();
					// Reset the pos
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					// Snap the reference point
					if(!(shift&&!command))
					{
						if(fReferencePoint)
						{	// Usual case
							const TVector2 prevPos = fReferencePoint->Position();
							fReferencePoint->OffsetPosition(flatOffset);
						
							fBuildingPrimitive->ClearPointFlag(eSnapedPosition,kAllLevels);
							fBuildingPrimitive->ClearWallFlag(eSnapedPosition,kAllLevels);
						
						//	if(fIsWallPoint)
						//		fBuildingModeler->SnapPoint(static_cast<Point*>((void*)fReferencePoint));
						//	else
								fBuildingModeler->SnapCommonPoint(fReferencePoint, fRefDirections, command, shift);
						//	fReferencePoint = fBuildingModeler->SnapPointPos(picked,fPanePart,cur,fReferencePoint,fReferencePoint->GetLevelIndex(),false,onPoint);
							flatOffset = fReferencePoint->Position()-prevPos;
							fReferencePoint->SetPosition(prevPos);
						}
					}
					// Offset the selection
					fBuildingPrimitive->SetSelectionObjPos( fRecordedAttachedObjPos,kAllLevels );
					if(flatOffset!=TVector2::kZero)
						fBuildingPrimitive->OffsetSelection(flatOffset, kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			// Check consistency on the selection
//	Not done anymore during the movement
//			if(fBuildingModeler->AutoMerge())
//				fBuildingPrimitive->CheckSelectionConsistency( kAllLevels );


			fBuildingPrimitive->SetSelectionHelper( false,kAllLevels );
			if(fReferencePoint && fIsWallPoint)
			{
				VPoint* point = static_cast<VPoint*>((void*)fReferencePoint);
				const int32 wallCount = point->GetWallCount();
				for(int32 iWall=0 ; iWall<wallCount ; iWall++)
				{
					point->GetWall(iWall)->ClearFlag(eWallHelper);
				}
			}

			// Clear this flag (was used for the point and wall color)
			fBuildingPrimitive->ClearPointFlag(eSnapedPosition,kAllLevels);
			fBuildingPrimitive->ClearWallFlag(eSnapedPosition,kAllLevels);

			fBuildingModeler->InvalidateMeshesAttributes(true);	
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr MoveMouseAction::Undo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive); // can be used only when no merging is done
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr MoveMouseAction::Redo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive); // can be used only when no merging is done
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr MoveMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 3);

	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////
//
WallCurveRecorder::WallCurveRecorder()
{
}

void WallCurveRecorder::SaveWallCurve(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	int32 indexWallObj= 0;
	int32 indexRoomObj= 0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		// Record the wall curve settings
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall( iWall );

			if (wall->Hidden())
				continue;

			if (wall->Selected())
				fRecordedWallCurves.AddElem(WallCurve(iLevel, iWall, wall->GetArcOffset(), wall->GetArcSegmentCount()));
		}
	}
}

void WallCurveRecorder::SwapWallCurve(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	const int32 wallCurveRecCount = fRecordedWallCurves.GetElemCount();

	int32 wallRecIndex=0;

	for(int32 iRec=0;iRec<wallCurveRecCount;iRec++)
	{
		WallCurve& curRec = fRecordedWallCurves[iRec];
		Level* level= primitive->GetLevel( curRec.fInLevel );

		if (!level) // ERROR
			continue;

		Wall* wall = level->GetWall( curRec.fOnWallIndex );
		
		if (!wall) // ERROR
			continue;

		// Store the data for the next Swap
		real32 arcOffset = wall->GetArcOffset();
		int32 arcSegCount = wall->GetArcSegmentCount();

		// Restore the wall curves
		wall->SetArcOffset(curRec.fOffset);
		wall->SetArcSegmentCount(curRec.fSegmentCount);

		curRec.fOffset = arcOffset;
		curRec.fSegmentCount = arcSegCount;
	}

//	primitive->BuildExtendedSelection();
//	primitive->InvalidateExtendedSelection(kAllLevels);
}

void WallCurveRecorder::ResetWallCurve(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	const int32 wallCurveRecCount = fRecordedWallCurves.GetElemCount();

	int32 wallRecIndex=0;

	for(int32 iRec=0;iRec<wallCurveRecCount;iRec++)
	{
		WallCurve& curRec = fRecordedWallCurves[iRec];
		Level* level= primitive->GetLevel( curRec.fInLevel );

		if (!level) // ERROR
			continue;

		Wall* wall = level->GetWall( curRec.fOnWallIndex );
		
		if (!wall) // ERROR
			continue;

		// Restore the wall curves
		wall->SetArcOffset(curRec.fOffset);
		wall->SetArcSegmentCount(curRec.fSegmentCount);
	}

//	primitive->BuildExtendedSelection();
//	primitive->InvalidateExtendedSelection(kAllLevels);

}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
MoveWallHandleMouseAction::MoveWallHandleMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									WallCurveRecorder()
{
	fRefreshGeometry = true;
}

void MoveWallHandleMouseAction::Create(	IShMouseAction**	outAction,
										BuildingModeler*	modeler,
										BuildingPanePart*	pane,
										const Picking&		picked,
										const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MoveWallHandleMouseAction(modeler, pane, picked, mousePos);
}

void MoveWallHandleMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SaveWallCurve(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			Wall* wall = static_cast<Wall*>(fPicked.PickedObject());
			TVector2 dir = wall->GetStraightDirection(wall->GetPoint(0));
			fFlatNormal.SetValues(dir.y,-dir.x);
			fFlatMiddle = wall->GetMidPos();

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);
				TVector2 firstRenderSpace=TVector2::kZero;
				TVector2 curRenderSpace=TVector2::kZero;

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				Wall* wall = static_cast<Wall*>(fPicked.PickedObject());
				// 
				TVector3 intersection;
				if( IntersectLinePlane2(origin,
										direction,
										TVector3::kUnitZ,
										fHitPos,
										intersection	) )
				{
					TVector2 flatPos = intersection.CastToXY();

					// Compute the offset corresponding to this pos:
					// project the pos on the normal to the wall
					real32 offset = (fFlatNormal*(flatPos-fFlatMiddle));

					// Snap when near the wall
					if(RealAbs( offset/wall->GetStraightLength() ) < 0.05f)
						offset = 0;

					// Compare this offset to the lenght of the wall to choose the
					// number of segments 
					fBuildingPrimitive->SetWallOffset(offset, true, kAllLevels);

					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->InvalidateMeshesAttributes(true);	
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr MoveWallHandleMouseAction::Undo()
{
	SwapWallCurve(fBuildingPrimitive);
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr MoveWallHandleMouseAction::Redo()
{
	SwapWallCurve(fBuildingPrimitive);
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr MoveWallHandleMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 57);

	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void GetOIJ(ProfilePoint* point, const TVector3& hitPos, TVector3& OOut, TVector3& IOut, TVector3& JOut)
{
	OOut = hitPos;
	IOut = TVector3::kUnitX;
	JOut = TVector3::kUnitY;

	Roof* roof = point->GetRoof();

	const real32 baseToGround = roof->GetMinToGround();
	const real32 topToGround = roof->GetMaxToGround();

	const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

	const int32 botProfileCount = roof->GetBotProfilePointCount();
	const int32 topProfileCount = roof->GetTopProfilePointCount();
	const int32 botInsideCount = roof->GetBotInsidePointCount();
	const int32 topInsideCount = roof->GetTopInsidePointCount();

	ZoneSection prevZoneSection = roof->GetRoofZoneSection(zoneSectionCount-1);
	TVector2 prevZonePos = prevZoneSection.fZonePoint->Position();
	TVector2 prevSpinePos = prevZoneSection.fSpinePoint->Position();
	for(int32 iSection=0 ; iSection<zoneSectionCount ; iSection++)
	{
		const ZoneSection& zoneSection = roof->GetRoofZoneSection(iSection);
		const TVector2& zonePos = zoneSection.fZonePoint->Position();
		const TVector2& spinePos = zoneSection.fSpinePoint->Position();

		if(zonePos != prevZonePos)
		{
			if(point->Flag(eTopProfileFlag))
			{
				TVector3 O,I,J,K;
				if(zoneSection.GetIsVertical())
				{	// Special case: this portion of the roof is a wall: draw the handles on the side
					const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
					const ZoneSection& prev = roof->GetRoofZoneSection(prevIndex);
			
					O = prev.fSpinePoint->Get3DPos();
					roof->GetVerticalBase(	prev.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),
										J,I,prev.fSpinePoint->Get3DPos(),zoneSection.fSpinePoint->Get3DPos());
					K = TVector3::kUnitZ;
				}
				else
				{	// Normal case
					Get3DBase1(	topToGround, prevZonePos, zonePos, prevSpinePos, spinePos, O, I, J, K);
				}

				for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
				{
					ProfilePoint* point = roof->GetTopProfilePoint(iPt);
					const TVector2& pos = point->Position();
					const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
					if( posInSpace == hitPos )
					{
						OOut = O; IOut = I; JOut = J;
						return;
					}
				}

				for(int32 iIn=0 ; iIn<topInsideCount ; iIn++)
				{
					ProfilePoint* point = roof->GetTopInsidePoint(iIn);
					const TVector2& pos = point->Position();
					const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
					if( posInSpace == hitPos )
					{
						OOut = O; IOut = I; JOut = J;
						return;
					}
				}
			}
			else
			{
				TVector3 O,I,J,K=TVector3::kUnitZ;
				const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
				const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);
		
				if(zoneSection.GetIsVertical())
				{	// Special case: this portion of the roof is a wall: draw the handles on the side
					O = prevSection.fZonePoint->Get3DPos();
					roof->GetVerticalBase(prevSection.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),
						J,I,prevSection.fSpinePoint->Get3DPos(), zoneSection.fSpinePoint->Get3DPos());
					K = TVector3::kUnitZ;
				}
				else
				{	// Normal case
					GetRoofBase(prevSection.fZonePoint->Get3DPos(), 
									zoneSection.fZonePoint->Get3DPos(), 
									roof->Flag(eRoofZoneOrientedPositive)?-1:1,
									I, J, O);
				}

				for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
				{
					ProfilePoint* point = roof->GetBotProfilePoint(iPt);
					const TVector2& pos = point->Position();
					const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
					if( posInSpace == hitPos )
					{
						OOut = O; IOut = I; JOut = J;
						return;
					}
				}

				for(int32 iIn=0 ; iIn<botInsideCount ; iIn++)
				{
					ProfilePoint* point = roof->GetBotInsidePoint(iIn);
					const TVector2& pos = point->Position();
					const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
					if( posInSpace == hitPos )
					{
						OOut = O; IOut = I; JOut = J;
						return;
					}
				}
			}
		}

		// Go to next section
		prevZoneSection = zoneSection;
		prevZonePos = zonePos;
		prevSpinePos = spinePos;
	}
}

//////////////////////////////////////////////////////////////////////
//
MoveProfileMouseAction::MoveProfileMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()
{
	fRefreshGeometry = true;
}

void MoveProfileMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MoveProfileMouseAction(modeler, pane, picked, mousePos);
}

void MoveProfileMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			// Need to find the X axis for the movement
			ProfilePoint* point = static_cast<ProfilePoint*>(fPicked.PickedObject());
			GetOIJ(point,fHitPos,fO,fI,fJ);

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				TVector3 offset;
				if( IntersectLinePlane2(origin,
										direction,
										fI,
										fHitPos,
										offset	) )
				{
					offset-=fHitPos;
					TVector2 flatOffset(Proj(fJ.CastToXY(),offset.CastToXY()),offset.z);
					if(!shift)
					{	// Constrain in the hor or ver direction
						if(RealAbs(flatOffset.x)>RealAbs(flatOffset.y))
							flatOffset.y = 0;
						else
							flatOffset.x = 0;
					}

					// Reset the pos
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					// Offset the selection
					fBuildingPrimitive->OffsetSelection(flatOffset, kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr MoveProfileMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerMouseAction::Undo();
}

MCCOMErr MoveProfileMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

	return ModelerMouseAction::Redo();
}

MCCOMErr MoveProfileMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 3);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
HolePointMouseAction::HolePointMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									ObjectPosRecorder(false)
{
	fRefreshGeometry = true;
	fO = TVector3::kZero;
	fI = TVector3::kUnitX;
	fJ = TVector3::kUnitY;
	fK = TVector3::kUnitZ;
}

void HolePointMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);

			// Need to find the X axis for the movement
			OutlinePoint* point = static_cast<OutlinePoint*>(fPicked.PickedObject());
			SubObject* object = point->GetSubObject();
			if(MCVerify(object))
			{
				object->GetBase(fO,fI,fJ,fK);
			}

			fPicked.GetHitPosition(f3DHitPos);
			Base3ToBase2(f3DHitPos, fO, fI, fJ, fK, f2DHitPos);

			// Checking: result should be equal to point pos

			PrepareMove(point, object);

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option	= gActionManager->IsOptionDown();
				const boolean shift		= gActionManager->IsShiftDown();
				const boolean command	= gActionManager->IsCommandDown();

				TVector3 normal = fK;
				if(RealAbs(direction*normal)<kRealEpsilon)
					normal = direction;

				TVector3 newIntersection;
				if( IntersectLinePlane2(origin,
										direction,
										normal,
										f3DHitPos,
										newIntersection	) )
				{
					// Reset the pos
					ResetPosition(fBuildingPrimitive);

					// Do the move, scale or rotate
					TrackMove(newIntersection, option, shift, command);

					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr HolePointMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerMouseAction::Undo();
}

MCCOMErr HolePointMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

	return ModelerMouseAction::Redo();
}

MCCOMErr MoveHolePointMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, fStringID);

	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////
void MoveHolePointMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MoveHolePointMouseAction(modeler, pane, picked, mousePos);
}

void MoveHolePointMouseAction::TrackMove(	const TVector3& newIntersection,
											const boolean option,
											const boolean shift,
											const boolean command )
{
	// Transform the pos into the 2D plane
	TVector2 pos2D;
	Base3ToBase2(newIntersection, fO, fI, fJ, fK, pos2D);

	TVector2 offset = pos2D - f2DHitPos;

	if(!shift)
	{	// Constrain in the hor or ver direction
		if(RealAbs(offset.x)>RealAbs(offset.y))
			offset.y = 0;
		else
			offset.x = 0;
	}

	// Offset the selection
	fBuildingPrimitive->OffsetObj(offset, kAllLevels);
}
//////////////////////////////////////////////////////////////////////
void ScaleHolePointMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new ScaleHolePointMouseAction(modeler, pane, picked, mousePos);
}

void ScaleHolePointMouseAction::PrepareMove(OutlinePoint* point, SubObject* object)
{
	const TVector2& pointPos = point->GetPosition();

	object->GetSelectionBBox().GetCenter(fCenter);
	
	fFirstDiff = fCenter-pointPos;
	if(RealAbs(fFirstDiff.x)<.25) fFirstDiff.x=.25f;
	if(RealAbs(fFirstDiff.y)<.25) fFirstDiff.y=.25f;

	if(fFirstDiff.GetSquaredNorm()<.5)
	{
		fFirstDiff.x=.5f;
		fFirstDiff.y=.5f;
	}
}

void ScaleHolePointMouseAction::TrackMove(	const TVector3& newIntersection,
											const boolean option,
											const boolean shift,
											const boolean command )
{
	// Transform the pos into the 2D plane
	TVector2 pos2D;
	Base3ToBase2(newIntersection, fO, fI, fJ, fK, pos2D);

	TVector2 diff = fCenter-pos2D;
	TVector2 scale( diff.x/fFirstDiff.x, diff.y/fFirstDiff.y );
	CheckScaleValue(scale, diff, shift);

	// Offset the selection
	fBuildingPrimitive->ScaleObj(scale, kAllLevels);
}
MCCOMErr ScaleHolePointMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, fStringID);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////
void RotateHolePointMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new RotateHolePointMouseAction(modeler, pane, picked, mousePos);
}

void RotateHolePointMouseAction::PrepareMove(OutlinePoint* point, SubObject* object)
{
	// Get the 2D bbox of the selection (on the PointOutline only)
	fFirstDir = point->GetPosition();	
	object->GetSelectionBBox().GetCenter(fCenter);
	fFirstDir -= fCenter;
	fFirstDir.Normalize();

	fConstraintAngle = fBuildingModeler->GetRotationConstraint();
}

void RotateHolePointMouseAction::TrackMove(	const TVector3& newIntersection,
											const boolean option,
											const boolean shift,
											const boolean command )
{
	// Transform the pos into the 2D plane
	TVector2 pos2D;
	Base3ToBase2(newIntersection, fO, fI, fJ, fK, pos2D);

	TVector2 cosSin=TVector2::kZero;

	const TVector2& vec1 = fFirstDir;
	TVector2 vec2 = pos2D-fCenter;
	vec2.Normalize();

	cosSin.x = vec1 * vec2;
	cosSin.y = vec1 ^ vec2;
	if(!shift)
	{
		real32 angle = 0;
		RealArcSinCos(cosSin.y, cosSin.x, angle);
		if(angle>0)
			angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle+.5));
		else
			angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle-.5));
		RealSinCos(angle, cosSin.y, cosSin.x);
	}
	// Offset the selection
	fBuildingPrimitive->RotateObj(cosSin, kAllLevels);
}
MCCOMErr RotateHolePointMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, fStringID);

	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////
//
MoveAction::MoveAction(	BuildingModeler*	modeler,
						const TVector2&		offset,
						CommonPoint*		point):
						ModelerAction(modeler),
						PositionRecorder()
{
	fOffset = offset;
	fPoint = point;
	fRefreshGeometry = true;
}

void MoveAction::Create(IShAction**			outAction,
						BuildingModeler*	modeler,
						const TVector2&		offset,
						CommonPoint*		point)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new MoveAction(modeler, offset, point) );
}

MCCOMErr MoveAction::Do()
{
	SavePosition(fBuildingPrimitive);

	if(fPoint)
	{
		fPoint->OffsetPosition(fOffset);
	}
	else
	{
		fBuildingPrimitive->OffsetSelection(fOffset, kAllLevels);
	}
	
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Do();
}

MCCOMErr MoveAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Undo();
}

MCCOMErr MoveAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Redo();
}

MCCOMErr MoveAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 3);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ScaleMouseAction::ScaleMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()//,
								//	BuildingRecorder(modeler->GetBuildingNoAddRef())
{
	fRefreshGeometry = true;
}

void ScaleMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new ScaleMouseAction(modeler, pane, picked, mousePos);
}

void ScaleMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
		//	SaveBuilding();
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);
			TBBox3D bbox;
			fBuildingPrimitive->GetBoundingBox(bbox, false, true);
			TVector3 center;
			bbox.GetCenter(center);
			fCenter = center.CastToXY();
				
			fFirstDiff = fCenter-fHitPos.CastToXY();
			if(RealAbs(fFirstDiff.x)<.25) fFirstDiff.x=.25f;
			if(RealAbs(fFirstDiff.y)<.25) fFirstDiff.y=.25f;

			if(fFirstDiff.GetSquaredNorm()<.5)
			{
				fFirstDiff.x=.5f;
				fFirstDiff.y=.5f;
			}

			fPrecision = fBuildingModeler->GetSnapPrecision();

			// See if we're in a Side view
			fPlaneNormal = TVector3::kUnitZ;
			switch( fPanePart->GetCameraType() )
			{
			case kCanonicalCameraType_Left:
			case kCanonicalCameraType_Right: fPlaneNormal = TVector3::kUnitX; break;
			case kCanonicalCameraType_Front:
			case kCanonicalCameraType_Back: fPlaneNormal = TVector3::kUnitY; break;
			}

			// For the concistency check
			fBuildingPrimitive->SetSelectionHelper( true,kAllLevels );

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// Put the eOption1Mode by default, it's more usefull for the roofs
				const EOptionMode scalingMode = command?eRegularMode:eOption1Mode;

				// Wall, Room and Point: move only alowed in the Level plane
				// => the offset is the intersection of the (origin,direction)
				// with the plane (hitPos,Z) minus the hitpos.
				TVector3 intersection;
				if( IntersectLinePlane2(origin,
										direction,
										fPlaneNormal,
										fHitPos,
										intersection ) )
				{
					TVector2 scale = intersection.CastToXY();
					TVector2 diff = fCenter-scale;
					scale.x=diff.x/fFirstDiff.x;
					scale.y=diff.y/fFirstDiff.y;
					// Some security check: <0 forbidden and near 0 too
					CheckScaleValue(scale, diff, shift);

					// Scale the selection
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					fBuildingPrimitive->SetSelectionObjPos( fRecordedAttachedObjPos,kAllLevels );
					fBuildingPrimitive->ScaleSelection(scale, fCenter,scalingMode,kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
//	Not done anymore during the movement
//			if(fBuildingModeler->AutoMerge())
//				fBuildingPrimitive->CheckSelectionConsistency( kAllLevels );

			// For the concistency check
			fBuildingPrimitive->SetSelectionHelper( false,kAllLevels );

			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr ScaleMouseAction::Undo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive); // can be used only when no merging is done
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr ScaleMouseAction::Redo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive);
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr ScaleMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ScaleProfileMouseAction::ScaleProfileMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()
{
	fRefreshGeometry = true;
}

void ScaleProfileMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new ScaleProfileMouseAction(modeler, pane, picked, mousePos);
}

void ScaleProfileMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			// Need to find the X axis for the movement
			ProfilePoint* point = static_cast<ProfilePoint*>(fPicked.PickedObject());
			GetOIJ(point,fHitPos,fO,fI,fJ);
				
			const TVector3 dif = fO-fHitPos;
			fFirstDiff.x = Proj(fJ.CastToXY(),dif.CastToXY());
			fFirstDiff.y = dif.z;
			if(RealAbs(fFirstDiff.x)<.25) fFirstDiff.x=.25f;
			if(RealAbs(fFirstDiff.y)<.25) fFirstDiff.y=.25f;

			if(fFirstDiff.GetSquaredNorm()<.5)
			{
				fFirstDiff.x=.5f;
				fFirstDiff.y=.5f;
			}

			fCenter.x = Proj(fJ.CastToXY(),fO.CastToXY());
			fCenter.y = dif.z;

			fPrecision = fBuildingModeler->GetSnapPrecision();

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// Put the eOption1Mode by default, it's more usefull for the roofs
				const EOptionMode scalingMode = command?eRegularMode:eOption1Mode;

				TVector3 scale3D;
				if( IntersectLinePlane2(origin,
										direction,
										fI,
										fHitPos,
										scale3D	) )
				{
					const TVector3 diff3D = fO-scale3D;
					const TVector2 diff(Proj(fJ.CastToXY(),diff3D.CastToXY()),diff3D.z);
					TVector2 scale( diff.x/fFirstDiff.x, diff.y/fFirstDiff.y);
					// Some security check: near 0 forbidden
					CheckScaleValue(scale, diff, shift);

					// Scale the selection
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					fBuildingPrimitive->SetSelectionObjPos( fRecordedAttachedObjPos,kAllLevels );
					fBuildingPrimitive->ScaleSelection(scale,fCenter,scalingMode,kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr ScaleProfileMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr ScaleProfileMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr ScaleProfileMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
RotateProfileMouseAction::RotateProfileMouseAction(	BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()
{
	fRefreshGeometry = true;
}

void RotateProfileMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new RotateProfileMouseAction(modeler, pane, picked, mousePos);
}

void RotateProfileMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			// Need to find the X axis for the movement
			ProfilePoint* point = static_cast<ProfilePoint*>(fPicked.PickedObject());
			GetOIJ(point,fHitPos,fO,fI,fJ);
				
			const TVector3 dif = fO-fHitPos;
			fFirstPos.x = Proj( fJ.CastToXY(),dif.CastToXY() );
			fFirstPos.y = dif.z;
			fFirstPos.Normalize();

			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				TVector3 newPoint;
				if( IntersectLinePlane2(origin,
										direction,
										fI,
										fHitPos,
										newPoint ) )
				{
					TVector2 cosSin=TVector2::kZero;

					const TVector3 dif = fO-newPoint;
					TVector2 vec(Proj( fJ.CastToXY(),dif.CastToXY() ),dif.z);
					vec.Normalize();

					cosSin.x = fFirstPos * vec;
					cosSin.y = fFirstPos ^ vec;
					// Rotate the selection
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					fBuildingPrimitive->SetSelectionObjPos( fRecordedAttachedObjPos,kAllLevels );
					fBuildingPrimitive->RotateSelection(cosSin,TVector2::kZero,kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr RotateProfileMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr RotateProfileMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr RotateProfileMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 5);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
RotateMouseAction::RotateMouseAction(BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos)
									:
									fPicked(picked),ModelerMouseAction(modeler,pane),
									PositionRecorder()//,
								//	BuildingRecorder(modeler->GetBuildingNoAddRef())
{
	fRefreshGeometry = true;
}

void RotateMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new RotateMouseAction(modeler, pane, picked, mousePos);
}

void RotateMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
		//	SaveBuilding();
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);
			TBBox3D bbox;
			fBuildingPrimitive->GetBoundingBox(bbox, false, true);
			TVector3 center;
			bbox.GetCenter(center);
			fCenter = center.CastToXY();

			fConstraintAngle = fBuildingModeler->GetRotationConstraint();
				
			// See if we're in a Side view
			fPlaneNormal = TVector3::kUnitZ;
			switch( fPanePart->GetCameraType() )
			{
			case kCanonicalCameraType_Left:
			case kCanonicalCameraType_Right: fPlaneNormal = TVector3::kUnitX; break;
			case kCanonicalCameraType_Front:
			case kCanonicalCameraType_Back: fPlaneNormal = TVector3::kUnitY; break;
			}

			// For the CHeck consistency and to remove them from the picking
			fBuildingPrimitive->SetSelectionHelper( true,kAllLevels );
		
			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// Wall, Room and Point: move only alowed in the Level plane
				// => the offset is the intersection of the (origin,direction)
				// with the plane (hitPos,Z) minus the hitpos.
				TVector3 newPoint;
				if( IntersectLinePlane2(origin,
										direction,
										fPlaneNormal,
										fHitPos,
										newPoint ) )
				{
					TVector2 cosSin=TVector2::kZero;

					TVector2 vec1 = fHitPos.CastToXY()-fCenter;
					vec1.Normalize();
					TVector2 vec2 = newPoint.CastToXY()-fCenter;
					vec2.Normalize();

					cosSin.x = vec1 * vec2;
					cosSin.y = vec1 ^ vec2;
					if(!shift)
					{
						// Constrain
						real32 angle = 0;
						RealArcSinCos(cosSin.y, cosSin.x, angle);
						if(angle>0)
							angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle+.5));
						else
							angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle-.5));
						RealSinCos(angle, cosSin.y, cosSin.x);
					}
					// Rotate the selection
					fBuildingPrimitive->SetSelectionPos( fRecordedPointPos,kAllLevels );
					fBuildingPrimitive->SetSelectionObjPos( fRecordedAttachedObjPos,kAllLevels );
					fBuildingPrimitive->RotateSelection(cosSin,fCenter,kAllLevels);
					fBuildingPrimitive->CheckExtendedSelection(kAllLevels);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			// Check consistency on the selection
//	Not done anymore during the movement
//			if(fBuildingModeler->AutoMerge())
//				fBuildingPrimitive->CheckSelectionConsistency( kAllLevels );

			fBuildingPrimitive->SetSelectionHelper( false,kAllLevels );

			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr RotateMouseAction::Undo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive); // can be used only when no merging is done
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Undo();
}

MCCOMErr RotateMouseAction::Redo()
{
//	SwapBuilding(); //	Merging not done anymore during the movement

	SwapPosition(fBuildingPrimitive); // can be used only when no merging is done
				
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr RotateMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 5);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
FlipAction::FlipAction(	BuildingModeler*	modeler):
						ModelerAction(modeler),
						PositionRecorder()
{
	fRefreshGeometry = true;
}

void FlipAction::Create(IShAction**			outAction,
						BuildingModeler*	modeler )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new FlipAction(modeler) );
}

MCCOMErr FlipAction::Do()
{
	SavePosition(fBuildingPrimitive);

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
				
	return ModelerAction::Do();
}

MCCOMErr FlipAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
				
	return ModelerAction::Undo();
}

MCCOMErr FlipAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
				
	return ModelerAction::Redo();
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
ObjectPosRecorder::ObjectPosRecorder(const boolean recordCenter)
{
	fRecordCenter = recordCenter;
}

void ObjectPosRecorder::SavePosition(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	int32 indexWallObj= 0;
	int32 indexRoomObj= 0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		// Record the wall object positions
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall( iWall );

			if (wall->Hidden())
				continue;

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				WallSubObject* obj = wall->GetObject( iObj );
				if( obj->Selected() )
				{	// Record its position
					if(fRecordCenter)
						fRecordedWallObjPos.AddElem(WallObjPos(iLevel, iWall, obj->GetPolylineCenter()));
					else // Record all the points of the polyline
						fRecordedWallObjPos.AddElem(WallObjPos(iLevel, iWall, obj->GetOutline()));
				}
				else if( obj->Hidden() && obj->HasSelectedHolePoint())
				{
					// Record all the points of the polyline
					fRecordedWallObjPos.AddElem(WallObjPos(iLevel, iWall, obj->GetOutline()));
				}
			}
		}

		// Record the room object positions
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom( iRoom );

			if (room->Hidden())
				continue;

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				RoomSubObject* obj = room->GetObject( iObj );
				if( obj->Selected() )
				{	// Record its position
					if(fRecordCenter)
						fRecordedRoomObjPos.AddElem(RoomObjPos(iLevel, iRoom, obj->GetPolylineCenter()));
					else // Record all the points of the polyline
						fRecordedRoomObjPos.AddElem(RoomObjPos(iLevel, iRoom, obj->GetOutline()));
				}
				else if( obj->Hidden() && obj->HasSelectedHolePoint())
				{
					// Record all the points of the polyline
					fRecordedRoomObjPos.AddElem(RoomObjPos(iLevel, iRoom, obj->GetOutline()));
				}
			}
		}
	}
}

void ObjectPosRecorder::SwapPosition(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	TMCCountedPtrArray<WallSubObject> wallObjToModify;
	TMCArray<int32> wallIndex; // Get also the index of the walls
	TMCArray<int32> wallLevelIndex; // And the level they belong to
	TMCCountedPtrArray<RoomSubObject> roomObjToModify;
	TMCArray<int32> roomIndex; // Get also the index of the walls
	TMCArray<int32> roomLevelIndex; // And the level they belong to

	const int32 wallObjRecCount = fRecordedWallObjPos.GetElemCount();
	const int32 roomObjRecCount = fRecordedRoomObjPos.GetElemCount();

	wallObjToModify.SetElemCount(wallObjRecCount);
	wallIndex.SetElemCount(wallObjRecCount);;
	wallLevelIndex.SetElemCount(wallObjRecCount);;
	roomObjToModify.SetElemCount(roomObjRecCount);
	roomIndex.SetElemCount(roomObjRecCount);;
	roomLevelIndex.SetElemCount(roomObjRecCount);;

	int32 wallRecIndex=0;
	int32 roomRecIndex=0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		// Restore the wall object positions
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall( iWall );

			if (wall->Hidden())
				continue;

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				WallSubObject* obj = wall->GetObject( iObj );
				if( obj->Selected() || (obj->Hidden()&&obj->HasSelectedHolePoint()) )
				{
					wallIndex.SetElem(wallRecIndex, iWall);
					wallLevelIndex.SetElem(wallRecIndex, iLevel);
					wallObjToModify.SetElem(wallRecIndex++, obj);
				}
			}
		}

		// Restore the room object positions
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom( iRoom );

			if (room->Hidden())
				continue;

			const int32 objCount = room->GetObjectCount();	
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				RoomSubObject* obj = room->GetObject( iObj );
				if( obj->Selected() || (obj->Hidden()&&obj->HasSelectedHolePoint()) )
				{
					roomIndex.SetElem(roomRecIndex, iRoom);
					roomLevelIndex.SetElem(roomRecIndex, iLevel);
					roomObjToModify.SetElem(roomRecIndex++, obj);
				}
			}
		}
	}

	MY_ASSERT(wallRecIndex==wallObjRecCount);
	MY_ASSERT(roomRecIndex==roomObjRecCount);

	{
		for( int32 iObj=0 ; iObj< wallObjRecCount ; iObj++ )
		{
			// Restore its position	
			WallSubObject* obj = wallObjToModify[iObj];

			// get a copy of the data before erasing them
			const TVector2 newCenter = obj->GetPolylineCenter();
			TMCCountedPtrArray<OutlinePoint> newOutline = obj->GetOutline();

			WallObjPos& wallObjPos = fRecordedWallObjPos[iObj];
			if(fRecordCenter)
				obj->SetPolylineCenter(wallObjPos.fPositions[0]->GetPosition(),false);
			else
				obj->SetOutline( wallObjPos.fPositions );

			const int32 curLevelIndex = wallLevelIndex[iObj];
			const int32 curWallIndex = wallIndex[iObj];
			if( curLevelIndex!=wallObjPos.fInLevel ||
				curWallIndex!=wallObjPos.fOnWallIndex )
			{	// Restore the previous wall
				Wall* recWall = primitive->GetLevel(wallObjPos.fInLevel)->GetWall(wallObjPos.fOnWallIndex);
				obj->SetOnWall(recWall);
			}

			// Store the data for the next Swap
			wallObjPos.fInLevel = curLevelIndex;
			wallObjPos.fOnWallIndex = curWallIndex;
			if(fRecordCenter)
				wallObjPos.fPositions[0]->GetPosition() = newCenter;
			else
				CopyPointArray( newOutline, wallObjPos.fPositions );
		}
	}
	{	
		for( int32 iObj=0 ; iObj< roomObjRecCount ; iObj++ )
		{
			// Restore its position						
			RoomSubObject* obj = roomObjToModify[iObj];
			const TVector2 newCenter = obj->GetPolylineCenter();
			TMCCountedPtrArray<OutlinePoint> newOutline = obj->GetOutline();

			RoomObjPos& roomObjPos = fRecordedRoomObjPos[iObj];
			if(fRecordCenter)
				obj->SetPolylineCenter(roomObjPos.fPositions[0]->GetPosition(),false);
			else // Restore all the points of the polyline
				obj->SetOutline( roomObjPos.fPositions );

			const int32 curLevelIndex = roomLevelIndex[iObj];
			const int32 curRoomIndex = roomIndex[iObj];
			if( curLevelIndex!=roomObjPos.fInLevel ||
				curRoomIndex!=roomObjPos.fInRoomIndex )
			{	// Restore the previous room
				Room* recRoom = primitive->GetLevel(roomObjPos.fInLevel)->GetRoom(roomObjPos.fInRoomIndex);
				obj->SetInRoom(recRoom);
			}

			//	roomObjPos.fInRoom = newRoom;
			roomObjPos.fInLevel = curLevelIndex;
			roomObjPos.fInRoomIndex = curRoomIndex;
			if(fRecordCenter)
				roomObjPos.fPositions[0]->GetPosition() = newCenter;
			else
				CopyPointArray( newOutline, roomObjPos.fPositions );
		}
	}
	primitive->BuildExtendedSelection();
	primitive->InvalidateExtendedSelection(kAllLevels);
//	primitive->InvalidateObjectSelection(kAllLevels);
}

void ObjectPosRecorder::ResetPosition(BuildingPrim* primitive)
{
	const int32 levelCount = primitive->GetLevelCount();

//	PrimitiveStatus* status = primitive->GetStatus();

	TMCCountedPtrArray<WallSubObject> wallObjToModify;
	TMCArray<int32> wallIndex; // Get also the index of the walls
	TMCArray<int32> wallLevelIndex; // And the level they belong to
	TMCCountedPtrArray<RoomSubObject> roomObjToModify;
	TMCArray<int32> roomIndex; // Get also the index of the walls
	TMCArray<int32> roomLevelIndex; // And the level they belong to

	const int32 wallObjRecCount = fRecordedWallObjPos.GetElemCount();
	const int32 roomObjRecCount = fRecordedRoomObjPos.GetElemCount();

	wallObjToModify.SetElemCount(wallObjRecCount);
	wallIndex.SetElemCount(wallObjRecCount);;
	wallLevelIndex.SetElemCount(wallObjRecCount);;
	roomObjToModify.SetElemCount(roomObjRecCount);
	roomIndex.SetElemCount(roomObjRecCount);;
	roomLevelIndex.SetElemCount(roomObjRecCount);;

	int32 wallRecIndex=0;
	int32 roomRecIndex=0;

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= primitive->GetLevel( iLevel );

		if (level->Hidden())
			continue;

		// Restore the wall object positions
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall( iWall );

			if (wall->Hidden())
				continue;

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				WallSubObject* obj = wall->GetObject( iObj );
				if( obj->Selected() || (obj->Hidden()&&obj->HasSelectedHolePoint()))
				{
					wallIndex.SetElem(wallRecIndex, iWall);
					wallLevelIndex.SetElem(wallRecIndex, iLevel);
					wallObjToModify.SetElem( wallRecIndex++, obj );
				}
			}
		}

		// Restore the room object positions
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom( iRoom );

			if (room->Hidden())
				continue;

			const int32 objCount = room->GetObjectCount();	
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				RoomSubObject* obj = room->GetObject( iObj );
				if( obj->Selected() || (obj->Hidden()&&obj->HasSelectedHolePoint()))
				{
					roomIndex.SetElem(roomRecIndex, iRoom);
					roomLevelIndex.SetElem(roomRecIndex, iLevel);
					roomObjToModify.SetElem( roomRecIndex++, obj );
				}
			}
		}
	}


	MY_ASSERT(wallRecIndex==wallObjRecCount);
	MY_ASSERT(roomRecIndex==roomObjRecCount);

	{
		for( int32 iObj=0 ; iObj< wallObjRecCount ; iObj++ )
		{
			// Restore its position						
			WallSubObject* obj = wallObjToModify[iObj];
			WallObjPos& wallObjPos = fRecordedWallObjPos[iObj];
			if(fRecordCenter)
				obj->SetPolylineCenter(wallObjPos.fPositions[0]->GetPosition(),false);
			else // Restore all the points of the polyline
				obj->SetOutline( wallObjPos.fPositions );
		
			const int32 curLevelIndex = wallLevelIndex[iObj];
			const int32 curWallIndex = wallIndex[iObj];
			if( curLevelIndex!=wallObjPos.fInLevel ||
				curWallIndex!=wallObjPos.fOnWallIndex )
			{	// Restore the previous wall
				Wall* recWall = primitive->GetLevel(wallObjPos.fInLevel)->GetWall(wallObjPos.fOnWallIndex);
				obj->SetOnWall(recWall);
			}
		}
	}
	{					
		for( int32 iObj=0 ; iObj< roomObjRecCount ; iObj++ )
		{
			// Restore its position	
			RoomSubObject* obj = roomObjToModify[iObj];
			RoomObjPos& roomObjPos = fRecordedRoomObjPos[iObj];
			if(fRecordCenter)
				obj->SetPolylineCenter(roomObjPos.fPositions[0]->GetPosition(),false);
			else // Restore all the points of the polyline
				obj->SetOutline( roomObjPos.fPositions );
			const int32 curLevelIndex = roomLevelIndex[iObj];
			const int32 curRoomIndex = roomIndex[iObj];
			if( curLevelIndex!=roomObjPos.fInLevel ||
				curRoomIndex!=roomObjPos.fInRoomIndex )
			{	// Restore the previous room
				Room* recRoom = primitive->GetLevel(roomObjPos.fInLevel)->GetRoom(roomObjPos.fInRoomIndex);
				obj->SetInRoom(recRoom);
			}
		}
	}
	primitive->BuildExtendedSelection();
	primitive->InvalidateExtendedSelection(kAllLevels);
//	primitive->InvalidateObjectSelection(kAllLevels);

}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
MoveObjectMouseAction::MoveObjectMouseAction(	BuildingModeler*	modeler,
												BuildingPanePart*	pane,
												const Picking&		picked,
												const TMCPoint&		mousePos)
												:
												fPicked(picked),ModelerMouseAction(modeler,pane),
												ObjectPosRecorder(true)
{
	fBuildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();

	fRefreshGeometry = true;
}

void MoveObjectMouseAction::Create(	IShMouseAction**	outAction,
									BuildingModeler*	modeler,
									BuildingPanePart*	pane,
									const Picking&		picked,
									const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new MoveObjectMouseAction(modeler, pane, picked, mousePos);
}

void MoveObjectMouseAction::Track(	IMCGraphicContext* gc, 
									int16 stage, 
									TMCPoint& first, 
									TMCPoint &prev, 
									TMCPoint &cur,
									boolean moved, 
									IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);
			fBuildingModeler->BeginImmediateUpdate();
			
			// Compute some usefull data for the tracking
			SubObject* obj = static_cast<SubObject*>(fPicked.PickedObject());
			obj->GetBase(fObjO, fObjI, fObjJ, fObjK);
			fWallObjPicked = ( fPicked.GetPickedType() == eWallObjectPicked );
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;

				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);
	
				// We need to compute the intersection of this ray with the walls
				// and with the rooms.
				// If one or both are missing, we'll work with an horizontal plane
				TVector3 newPickPos;
				real32 minAlpha=kBigRealValue;
				Wall* onWall=NULL;
				Room* inRoom=NULL;
				TVector3 projPickPos;
				if( fWallObjPicked )
				{
					onWall = fBuildingPrimitive->BasicPickBestWall(origin,direction,newPickPos,kAllLevels);
				
					// Project the pickedPos on the original wall (or room) to dertemine the
					// general offset for all the other selected objects
					Project(newPickPos, fObjO, fObjK, projPickPos ) ; // direction, projPickPos);
				}
				else
				{
					// else Pick the rooms
					inRoom = fBuildingPrimitive->BasicPickBestRoom(origin,direction,newPickPos,kAllLevels);
					projPickPos = newPickPos;
				}
		
				boolean hasHit = (onWall!=NULL) || (inRoom!=NULL);

				// If shift is pressed, authorized free movments, otherwise constrain the
				// positon to vertical or the horizontal axis
				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// => the offset is the intersection of the (origin,direction)
				// with the plane (hitPos,Z) minus the hitpos.
				if( !hasHit )
				{
					hasHit = IntersectLinePlane2(	origin,
														direction,
														TVector3::kUnitZ,
														fHitPos,
														newPickPos	);
					if(hasHit)
					{
						// No projection to do now, we're most probably in the top view
						projPickPos = newPickPos;
					
						if( fWallObjPicked )
						{
							// project newPickPos on the nearest wall or room
							/*onWall =*/ fBuildingPrimitive->ProjectOnNearestWall(newPickPos);
						}
						else
						{	// Room object case
							/*inRoom =*/ fBuildingPrimitive->GetNearestRoom(newPickPos);
						}
					}
				}

				if(hasHit)
				{
					// Need to prepare some data to move all the selected object

					// Define a rotation so the primitive can redefine the direction of the 
					// projection based on the wall normal. This rotation will be used each time
					// an object gets out of its original wall
					// Axis of rotation:
					//TVector3 axis = fObjK^direction;
					//axis.Normalize();
					//real32 sin=0; real32 cos=1;
					//AngleBetweenVectors(fObjK,direction,axis, sin, cos);
					//TUnitComplex angle;
					////angle.SetFromSinCos(-sin, RealAbs(cos));
					//angle.SetFromSinCos(sin, cos);
					TUnitQuaternion rot;
					// rot.SetFromAxis(axis, angle);
					rot.SetFromTransition(fObjK, direction);

					TVector3 actualOffset = newPickPos/*projPickPos*/ - fHitPos;
				
					TVector2 projOffset = ProjectIn(actualOffset,fObjO,fObjI,fObjJ);

					// Offset the selection
					ResetPosition(fBuildingPrimitive);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

					if(fWallObjPicked)
						fBuildingPrimitive->OffsetWallObjJump(rot, actualOffset, (origin-projPickPos).GetNorm(), kAllLevels, !shift, command);
					else
						fBuildingPrimitive->OffsetRoomObjJump(projOffset, kAllLevels);

					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr MoveObjectMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerMouseAction::Undo();
}

MCCOMErr MoveObjectMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerMouseAction::Redo();
}

MCCOMErr MoveObjectMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 3);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
MoveObjectAction::MoveObjectAction(	BuildingModeler*	modeler,
									const TVector2&		offset ) :
									ModelerAction(modeler),
									ObjectPosRecorder(true)
{
	fOffset = offset;
	fBuildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();

	fRefreshGeometry = true;
}

void MoveObjectAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const TVector2&		offset )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new MoveObjectAction(modeler,offset) );
}

MCCOMErr MoveObjectAction::Do()
{
	SavePosition(fBuildingPrimitive);

	fBuildingPrimitive->OffsetObj(fOffset, kAllLevels);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
			
	return ModelerAction::Do();
}

MCCOMErr MoveObjectAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerAction::Undo();
}

MCCOMErr MoveObjectAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerAction::Redo();
}

MCCOMErr MoveObjectAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 3);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ScaleObjectMouseAction::ScaleObjectMouseAction(	BuildingModeler*	modeler,
											BuildingPanePart*	pane,
											const Picking&		picked,
											const TMCPoint&		mousePos)
											:
											fPicked(picked),ModelerMouseAction(modeler,pane),
											ObjectPosRecorder(false)
{
	fBuildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();

	fRefreshGeometry = true;
}

void ScaleObjectMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new ScaleObjectMouseAction(modeler, pane, picked, mousePos);
}

void ScaleObjectMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			// Compute some usefull data for the tracking
			SubObject* obj = static_cast<SubObject*>(fPicked.PickedObject());
			obj->GetBase(fObjO, fObjI, fObjJ, fObjK);
			fWallObjPicked = ( fPicked.GetPickedType() == eWallObjectPicked );
		
			fCenter = obj->GetPolylineCenter();
			obj->MinCorner(fFirstDiff); 
			fFirstDiff-=fCenter;
			
			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if (moved)	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				// Wall, Room and Point: move only alowed in the Level plane
				// => the offset is the intersection of the (origin,direction)
				// with the plane (hitPos,ojNormal) minus the hitpos.
				TVector3 diff;
				if( IntersectLinePlane2(origin,
										direction,
										fObjK,
										fHitPos,
										diff	) )
				{
					TVector2 projDiff = ProjectIn(diff,fObjO,fObjI,fObjJ);			
					projDiff-=fCenter;
					TVector2 scale(projDiff.x/fFirstDiff.x,projDiff.y/fFirstDiff.y);

					// Some security check: <0 forbidden and near 0 too
					if(scale.x<0) scale.x*=-1;
					if(scale.y<0) scale.y*=-1;
					if(scale.x<.1f) scale.x=.1f;
					if(scale.y<.1f) scale.y=.1f;
					// Scale the selection
					ResetPosition(fBuildingPrimitive);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

					fBuildingPrimitive->ScaleObj(scale, kAllLevels);

					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr ScaleObjectMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerMouseAction::Undo();
}

MCCOMErr ScaleObjectMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerMouseAction::Redo();
}

MCCOMErr ScaleObjectMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
ScaleObjectAction::ScaleObjectAction(BuildingModeler*	modeler,
									const TVector2&		scale ) :
									ModelerAction(modeler),
									ObjectPosRecorder(false)
{
	fScale = scale;
	fRefreshGeometry = true;
}

void ScaleObjectAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const TVector2&		scale )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new ScaleObjectAction(modeler,scale) );
}

MCCOMErr ScaleObjectAction::Do()
{
	SavePosition(fBuildingPrimitive);

	fBuildingPrimitive->ScaleObj(fScale, kAllLevels);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);
				
	return ModelerAction::Do();
}

MCCOMErr ScaleObjectAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerAction::Undo();
}

MCCOMErr ScaleObjectAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

	return ModelerAction::Redo();
}

MCCOMErr ScaleObjectAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
RotateObjectMouseAction::RotateObjectMouseAction(BuildingModeler*	modeler,
											BuildingPanePart*	pane,
											const Picking&		picked,
											const TMCPoint&		mousePos)
											:
											fPicked(picked),ModelerMouseAction(modeler,pane),
											ObjectPosRecorder(false)
{
	fBuildingPrimitive = fBuildingModeler->GetBuildingNoAddRef();

	fRefreshGeometry = true;
}

void RotateObjectMouseAction::Create(	IShMouseAction**	outAction,
								BuildingModeler*	modeler,
							    BuildingPanePart*	pane,
								const Picking&		picked,
								const TMCPoint&		mousePos )
{
	TMCCountedCreateHelper<IShMouseAction> result(outAction);
	result = (IShMouseAction*) new RotateObjectMouseAction(modeler, pane, picked, mousePos);
}

void RotateObjectMouseAction::Track(IMCGraphicContext* gc, 
							int16 stage, 
							TMCPoint& first, 
							TMCPoint &prev, 
							TMCPoint &cur,
							boolean moved, 
							IShMouseAction**	nextAction)
{
	switch(stage)	
	{
		case kBeginTracking:	
		{
			SavePosition(fBuildingPrimitive);
			fPicked.GetHitPosition(fHitPos);

			// Compute some usefull data for the tracking
			SubObject* obj = static_cast<SubObject*>(fPicked.PickedObject());
		
			fCenter = obj->GetPolylineCenter();
			
			fConstraintAngle = fBuildingModeler->GetRotationConstraint();
				
			fBuildingModeler->BeginImmediateUpdate();	
		}	break;

		
		case kContinueTracking:	
		{
			if( moved && (first-cur).Norm1()>2 )	
			{
				fHasMoved = true;
				TVector3 origin, direction;
				fPanePart->Get3DEditorHostPanePart()->PixelRay(cur,origin,direction);

				const boolean option=gActionManager->IsOptionDown();
				const boolean shift=gActionManager->IsShiftDown();
				const boolean command=gActionManager->IsCommandDown();

				TVector3 newPoint=TVector3::kZero;
				if( IntersectLinePlane2(origin,
										direction,
										TVector3::kUnitZ,
										fHitPos,
										newPoint ) )
				{
					TVector2 cosSin=TVector2::kZero;

					TVector2 vec1 = fHitPos.CastToXY()-fCenter;
					vec1.Normalize();
					TVector2 vec2 = newPoint.CastToXY()-fCenter;
					vec2.Normalize();

					cosSin.x = vec1 * vec2;
					cosSin.y = vec1 ^ vec2;
					if(!shift)
					{
						real32 angle = 0;
						RealArcSinCos(cosSin.y, cosSin.x, angle);
						if(angle>0)
							angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle+.5));
						else
							angle = (real32)fConstraintAngle*((int32)(angle/fConstraintAngle-.5));
						RealSinCos(angle, cosSin.y, cosSin.x);
					}
					// Rotate the selection
					ResetPosition(fBuildingPrimitive);
					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//					fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);

					fBuildingPrimitive->RotateObj(cosSin, kAllLevels);

					fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//					fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);
				
					fBuildingModeler->InvalidateMeshesAttributes(true);	
					fBuildingModeler->PostImmediateUpdate(true,true);
				}
			}
		} break;

		case kFinishTracking:
		{
			fBuildingModeler->EndImmediateUpdate();	
		} break;

	}

	ModelerMouseAction::Track(gc,stage,first,prev,cur,moved,nextAction);
}

MCCOMErr RotateObjectMouseAction::Undo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);				
				
	return ModelerMouseAction::Undo();
}

MCCOMErr RotateObjectMouseAction::Redo()
{
	SwapPosition(fBuildingPrimitive);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
//	fBuildingPrimitive->InvalidateObjectSelection(kAllLevels);				

	return ModelerMouseAction::Redo();
}

MCCOMErr RotateObjectMouseAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 5);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
ScaleAction::ScaleAction(BuildingModeler*	modeler,
						const TVector2&		scale ) :
						ModelerAction(modeler),
						PositionRecorder()
{
	fScale = scale;
	fRefreshGeometry = true;
}

void ScaleAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler,
							const TVector2&		scale )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new ScaleAction(modeler,scale) );
}

MCCOMErr ScaleAction::Do()
{
	SavePosition(fBuildingPrimitive);

	PrimitiveStatus* status = fBuildingPrimitive->GetStatus();

	TVector3 center;
	status->fSelectionBBox.GetCenter(center);

	// Option1 mahes more sense for the roof
	fBuildingPrimitive->ScaleSelection(fScale, center.CastToXY(),eOption1Mode,kAllLevels);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Do();
}

MCCOMErr ScaleAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				
				
	return ModelerAction::Undo();
}

MCCOMErr ScaleAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerAction::Redo();
}

MCCOMErr ScaleAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
RotateAction::RotateAction(BuildingModeler*	modeler,
						const TVector2&		cosSin ) :
						ModelerAction(modeler),
						PositionRecorder()
{
	fCosSin = cosSin;
	fRefreshGeometry = true;
}

void RotateAction::Create(	IShAction**			outAction,
							BuildingModeler*	modeler,
							const TVector2&		cosSin )
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new RotateAction(modeler,cosSin) );
}

MCCOMErr RotateAction::Do()
{
	SavePosition(fBuildingPrimitive);

	PrimitiveStatus* status = fBuildingPrimitive->GetStatus();

	TVector3 center;
	status->fSelectionBBox.GetCenter(center);

	fBuildingPrimitive->RotateSelection(fCosSin, center.CastToXY(),kAllLevels);
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Do();
}

MCCOMErr RotateAction::Undo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				
				
	return ModelerAction::Undo();
}

MCCOMErr RotateAction::Redo()
{
	SwapPosition(fBuildingPrimitive);

	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);				

	return ModelerAction::Redo();
}

MCCOMErr RotateAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 4);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
LevelHeightAction::LevelHeightAction(BuildingModeler*	modeler,
									const TMCArray<int32>& levels, 
									const real32 height) :
									ModelerAction(modeler)
{
	fHeight = height;
	fLevels = levels;
	fRefreshGeometry = true;
}

void LevelHeightAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const TMCArray<int32>&	levels, 
								const real32		height)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new LevelHeightAction(modeler,levels,height) );
}

MCCOMErr LevelHeightAction::Do()
{
	SetHeight();
				
	return ModelerAction::Do();
}

MCCOMErr LevelHeightAction::Undo()
{
	const int32 levelCount = fLevels.GetElemCount();
	for(int32 iLevel=0 ; iLevel<levelCount ; iLevel++)
	{
		Level* level = fBuildingPrimitive->GetLevel( fLevels[iLevel] );
		level->SetLevelHeight(fPrevHeight[iLevel]);
	}

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Undo();
}

MCCOMErr LevelHeightAction::Redo()
{
	SetHeight();

	return ModelerAction::Redo();
}

void LevelHeightAction::SetHeight()
{
	const int32 levelCount = fLevels.GetElemCount();
	for(int32 iLevel=0 ; iLevel<levelCount ; iLevel++)
	{
		Level* level = fBuildingPrimitive->GetLevel( fLevels[iLevel] );
		fPrevHeight.AddElem( level->GetLevelHeight() );
		level->SetLevelHeight(fHeight);
	}

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
}

MCCOMErr LevelHeightAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	gResourceUtilities->GetIndString( name, kModelerStrings, 14);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
DimensionAction::DimensionAction(BuildingModeler*	modeler,
								const real32 dimension,
								const EDimensionType	type) :
									ModelerAction(modeler),
									BuildingRecorder()
{
	fDimension = dimension;
	fDimensionType = type;
	fRefreshGeometry = true;
}

void DimensionAction::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const real32		dimension,
								const EDimensionType	type)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new DimensionAction(modeler,dimension,type) );
}

MCCOMErr DimensionAction::Do()
{
	SaveBuilding(fBuildingPrimitive);

	switch(fDimensionType)
	{
	case eFloorThickness:
		fBuildingPrimitive->SetFloorThickness(fDimension, fPreviousDimension, kAllLevels);
		break;
	case eCeilingThickness:
		fBuildingPrimitive->SetCeilingThickness(fDimension, fPreviousDimension, kAllLevels);
		break;
	case eWallThickness:
		fBuildingPrimitive->SetWallThickness(fDimension, fPreviousDimension, kAllLevels);
		break;
	case eWallHeight:
		fBuildingPrimitive->SetWallHeight(fDimension, fPreviousDimension, kAllLevels);
		break;
	case eWallOffset:
		{
			SetWallArcOffsetVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eWallSegments:
		{
			SetWallArcSegmentCountVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eCrenelHeight:
		{
			SetWallCrenelHeightVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eCrenelWidth:
		{
			SetWallCrenelWidthVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eCrenelSpacing:
		{
			SetWallCrenelSpacingVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eCrenelOffset:
		{
			SetWallCrenelOffsetVisitor visitor( fDimension, &fPreviousDimension );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	case eRoofMax:
		fBuildingPrimitive->SetRoofMax(fDimension, fPreviousDimension, kAllLevels);
		break;
	case eRoofMin:
		fBuildingPrimitive->SetRoofMin(fDimension, fPreviousDimension, kAllLevels);
		break;
	}

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr DimensionAction::Undo()
{
	SwapBuilding(fBuildingPrimitive); // Most of the setting can modify object pos, we need to restore everything

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Undo();
}

MCCOMErr DimensionAction::Redo()
{
	SwapBuilding(fBuildingPrimitive); // Most of the setting can modify object pos, we need to restore everything

	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Redo();
}

MCCOMErr DimensionAction::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	int32 stringID = 34;
	switch(fDimensionType)
	{
	case eFloorThickness: stringID = 49; break;
	case eCeilingThickness: stringID = 50; break;
	case eWallThickness: stringID = 34; break;
	case eWallHeight: stringID = 51; break;
	case eWallOffset: stringID = 55; break;
	case eWallSegments: stringID = 56; break;
	case eCrenelHeight: stringID = 58; break;
	case eCrenelWidth: stringID = 59; break;
	case eCrenelSpacing: stringID = 60; break;
	case eCrenelOffset: stringID = 61; break;
	case eCrenelShape: stringID = 62; break;
	case eRoofMax: stringID = 52; break;
	case eRoofMin: stringID = 53; break;
	}
	gResourceUtilities->GetIndString( name, kModelerStrings, stringID);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
UInt32Action::UInt32Action(BuildingModeler*	modeler,
							const uint32 newValue,
							const EDimensionType	type) :
								ModelerAction(modeler),
								BuildingRecorder()
{
	fNewvalue = newValue;
	fDimensionType = type;
	fRefreshGeometry = true;
}

void UInt32Action::Create(	IShAction**			outAction,
								BuildingModeler*	modeler,
								const uint32		newValue,
								const EDimensionType	type)
{
	TMCCountedCreateHelper<IShAction> result(outAction);
	result = static_cast<IShAction*>( new UInt32Action(modeler,newValue,type) );
}

MCCOMErr UInt32Action::Do()
{
	SaveBuilding(fBuildingPrimitive);

	switch(fDimensionType)
	{
	case eCrenelShape:
		{
			SetWallCrenelShapeVisitor visitor( fNewvalue );
			visitor.TraverseSelection( fBuildingPrimitive, kAllLevels );
		} break;
	}

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);

	return ModelerAction::Do();
}

MCCOMErr UInt32Action::Undo()
{
	SwapBuilding(fBuildingPrimitive); // Most of the setting can modify object pos, we need to restore everything

	// Invalidation
	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Undo();
}

MCCOMErr UInt32Action::Redo()
{
	SwapBuilding(fBuildingPrimitive); // Most of the setting can modify object pos, we need to restore everything

	fBuildingPrimitive->InvalidateBBox();
	fBuildingPrimitive->InvalidateStatus();
	fBuildingPrimitive->InvalidateExtendedSelection(kAllLevels);
				
	return ModelerAction::Redo();
}

MCCOMErr UInt32Action::GetName(TMCString& name)
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	int32 stringID = 34;
	switch(fDimensionType)
	{
	case eCrenelShape: stringID = 62; break;
	}
	gResourceUtilities->GetIndString( name, kModelerStrings, stringID);

	return MC_S_OK;
}
//////////////////////////////////////////////////////////////////////

#endif // !NETWORK_RENDERING_VERSION

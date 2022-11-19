/****************************************************************************************************

		BuildingPrimitiveData.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/19/2004

****************************************************************************************************/

#include "BuildingPrimitiveData.h"

#include "ArchiTools.h"

#include "PSubObject.h"
#include "I3DShObject.h"

void PrimitiveStatus::AddObj(SubObject* obj)
{
	const real32 width = obj->Get2DWidth();
	const real32 height = obj->Get2DHeight();
	const TVector2& center = obj->GetPolylineCenter();
	const EPlacementType placement = obj->GetPlacement();
	const TVector3& offset = obj->Offset();
	const TVector3& scale = obj->Scale();
	const TVector3& rotate = obj->Rotate();
	const TMCString& name = obj->GetName();
				
	fAutoFlip |= obj->AutoFlip();

	// Get the sub obj name
	TMCDynamicString objName;
	I3DShInstance* currentInstance = obj->GetChildNoAddRef();
	TMCCountedPtr<I3DShObject> currentObject;
	if(currentInstance)
	{
		currentInstance->Get3DObject(&currentObject);
		currentObject->GetName(objName);
	}


	if(!fSelectedRoomObjectCount && !fSelectedWallObjectCount)
	{
		f2DObjWidth = width;
		f2DObjHeight = height;
		f2DObjCenter = center;
		fObjPlacement = placement;
		fObjOffset = offset;
		fObjScale = scale;
		fObjRotate = rotate;
		fSceneObjectName = objName;
		fObjectSelectionName = name;
	}
	else
	{
		if(f2DObjWidth!=width) f2DObjWidth = kMultipleValues;
		if(f2DObjHeight!=height) f2DObjHeight = kMultipleValues;
		if(f2DObjCenter.x!=center.x) f2DObjCenter.x = kMultiVecField;
		if(f2DObjCenter.y!=center.y) f2DObjCenter.y = kMultiVecField;
		if(fObjPlacement!=placement) fObjPlacement = eMultiPlacement;
		if(fObjOffset.x!=offset.x) fObjOffset.x = kMultiVecField;
		if(fObjOffset.y!=offset.y) fObjOffset.y = kMultiVecField;
		if(fObjOffset.z!=offset.z) fObjOffset.z = kMultiVecField;
		if(fObjScale.x!=scale.x) fObjScale.x = kMultiVecField;
		if(fObjScale.y!=scale.y) fObjScale.y = kMultiVecField;
		if(fObjScale.z!=scale.z) fObjScale.z = kMultiVecField;
		if(fObjRotate.x!=rotate.x) fObjRotate.x = kMultiVecField;
		if(fObjRotate.y!=rotate.y) fObjRotate.y = kMultiVecField;
		if(fObjRotate.z!=rotate.z) fObjRotate.z = kMultiVecField;
		if(fSceneObjectName!=objName) fSceneObjectName = kMultiName;
		if(fObjectSelectionName!=name) fObjectSelectionName = kMultiName;
	}
}

BuildingPrimData::BuildingPrimData()
{
	// Actual default values are in the resource file (see kBuildingPrefs)
	fDefaultWallThickness= .2f;
	fDefaultFloorThickness= .1f;
	fDefaultCeilingThickness=.1f;
	fDefaultLevelHeight=3;
	fDefaultRoofMin=3;
	fDefaultRoofMax=7;

	fDefaultWindowHeight=1.2f;
	fDefaultWindowLength=1.6f;
	fDefaultWindowAltitude=1.4f;

	fDefaultDoorHeight=2.1f;
	fDefaultDoorLength=1.2f;

	fDefaultStairwayWidth=2;
	fDefaultStairwayLength=2;

	fDefaultGridSpacing = 1;
	fDefaultWBSizeX = 30;
	fDefaultWBSizeY = 30;
	fDefaultWBSizeZ = 30;
	fDefaultSnapPrecision = .5;
	fDefaultConstrainAngle = 15;
	fDefaultSnap = true;

	fIsNew = true;

	fDefaultValid = false;

	fStatusValid = false;

	// Default colors (not used anymore, they're set from the prefs)
	fDefCol.Set(gRedD,gGreenD,gBlueD,255);
	fSelCol.Set(gRedS,gGreenS,gBlueS,255);
	fTarCol.Set(gRedT,gGreenT,gBlueT,255);
	fFreCol.Set(gRedF,gGreenF,gBlueF,255);
	fHelCol.Set(gRedH,gGreenH,gBlueH,255);
	fSnaCol.Set(gRedSn,gGreenSn,gBlueSn,255);
	fFloCol.Set(gRedD,gGreenD,gBlueD,255);

	// View settings
	fConfig=kTwoPaneHorizontal;

	// Display setting
	fARShowAll = true;
	fARShowLevel = 0;

	fFBEnable = false;
	fLREnable = false;
	fTBEnable = false;

	fHoleEditEnable = false;

	fSceneMagnitude = 36;

	Validate();
}

void BuildingPrimData::SetDefaultLevelHeight( const real32 value, const boolean check )
{
	if(value==fDefaultLevelHeight)
		return;

	if(check)
	{
		// Check that the doors and windows can still be inserted
		const real32 minLevelHeight1 = fDefaultFloorThickness+fDefaultDoorHeight+fDefaultCeilingThickness;
		const real32 minLevelHeight2 = fDefaultWindowAltitude+.5*fDefaultWindowHeight;
		const real32 minLevelHeight = MC_Max(minLevelHeight1,minLevelHeight2);

		if(value<minLevelHeight)
			fDefaultLevelHeight = minLevelHeight;
		else	
			fDefaultLevelHeight = value;
	}
	else	
		fDefaultLevelHeight = value;
}

void BuildingPrimData::SetDefaultRoofMin( const real32 value )
{
	if(value==fDefaultRoofMin)
		return;

	fDefaultRoofMin = value;
}

void BuildingPrimData::SetDefaultRoofMax( const real32 value )
{
	if(value==fDefaultRoofMax)
		return;

	fDefaultRoofMax = value;
}

void BuildingPrimData::SetDefaultWallThickness( const real32 value )
{
	if(value==fDefaultWallThickness)
		return;

	fDefaultWallThickness=value;
}

void BuildingPrimData::SetDefaultFloorThickness( const real32 value, const boolean check )
{
	if(value==fDefaultFloorThickness)
		return;

	if(check)
	{
		// Check that the doors and windows can still be inserted
		const real32 maxFloorThickness1 = fDefaultLevelHeight-(fDefaultDoorHeight+fDefaultCeilingThickness);
		const real32 maxFloorThickness2 = fDefaultWindowAltitude-.5*fDefaultWindowHeight;
		const real32 maxFloorThickness = MC_Min(maxFloorThickness1,maxFloorThickness2);

		if(value>maxFloorThickness)
			fDefaultFloorThickness = maxFloorThickness;
		else
			fDefaultFloorThickness=value;
	}
	else
		fDefaultFloorThickness=value;

	Validate(); // to rebuild the door shapes
}

void BuildingPrimData::SetDefaultCeilingThickness( const real32 value, const boolean check )
{
	if(value==fDefaultCeilingThickness)
		return;

	if(check)
	{
		// Check that the doors and windows can still be inserted
		const real32 maxCeilingThickness1 = fDefaultLevelHeight-(fDefaultDoorHeight+fDefaultFloorThickness);
		const real32 maxCeilingThickness2 = fDefaultLevelHeight-(fDefaultWindowAltitude+.5*fDefaultWindowHeight);
		const real32 maxCeilingThickness = MC_Min(maxCeilingThickness1,maxCeilingThickness2);

		if(value>maxCeilingThickness)
			fDefaultFloorThickness = maxCeilingThickness;
		else
			fDefaultCeilingThickness=value;
	}
	else
		fDefaultCeilingThickness=value;
}

void BuildingPrimData::SetDefaultWindowHeight( const real32 value, const boolean check )
{
	if(value==fDefaultWindowHeight)
		return;

	if(check)
	{
		// Check that the windows can still be inserted
		const real32 maxWindowHeight1 = 2*(fDefaultWindowAltitude-fDefaultFloorThickness);
		const real32 maxWindowHeight2 = 2*(fDefaultWindowAltitude-fDefaultFloorThickness);
		const real32 maxWindowHeight = MC_Min(maxWindowHeight1,maxWindowHeight2);

		if(value>maxWindowHeight)
			fDefaultWindowHeight = maxWindowHeight;
		else
			fDefaultWindowHeight=value;
	}
	else
		fDefaultWindowHeight=value;

	Validate();
}

void BuildingPrimData::SetDefaultWindowLength( const real32 value )
{
	if(value==fDefaultWindowLength)
		return;

	fDefaultWindowLength=value;

	Validate();
}

void BuildingPrimData::SetDefaultWindowAltitude( const real32 value, const boolean check )
{
	if(value==fDefaultWindowAltitude)
		return;

	if(check)
	{
		// Check that the windows can still be inserted
		const real32 maxWindowAltitude = fDefaultLevelHeight-(.5*fDefaultWindowHeight+fDefaultCeilingThickness);
		const real32 minWindowAltitude = .5*fDefaultWindowHeight+fDefaultFloorThickness;

		if(value>maxWindowAltitude)
			fDefaultWindowAltitude = maxWindowAltitude;
		else if(value<minWindowAltitude)
			fDefaultWindowAltitude = minWindowAltitude;
		else
			fDefaultWindowAltitude = value;
	}
	else
			fDefaultWindowAltitude = value;

	Validate();
}

void BuildingPrimData::SetDefaultDoorHeight( const real32 value, const boolean check )
{
	if(value==fDefaultDoorHeight)
		return;

	if(check)
	{
		const real32 maxDoorHeight = fDefaultLevelHeight-fDefaultCeilingThickness-fDefaultFloorThickness;

		if(value>maxDoorHeight)
			fDefaultDoorHeight = maxDoorHeight;
		else
			fDefaultDoorHeight = value;
	}
	else
		fDefaultDoorHeight = value;

	Validate();
}

void BuildingPrimData::SetDefaultDoorLength( const real32 value )
{
	if(value==fDefaultDoorLength)
		return;

	fDefaultDoorLength = value;

	Validate();
}

void BuildingPrimData::SetDefaultStairwayWidth( const real32 value )
{
	if(value==fDefaultStairwayWidth)
		return;
	
	fDefaultStairwayWidth = value;

	Validate();
}

void BuildingPrimData::SetDefaultStairwayLength( const real32 value )
{
	if(value==fDefaultStairwayLength)
		return;

	fDefaultStairwayLength = value;

	Validate();
}

void BuildingPrimData::InitOutlineData(TMCCountedPtrArray<OutlinePoint>& outline)
{
	const int32 ptCount = outline.GetElemCount();

	for(int32 iPt=0 ; iPt<ptCount ; iPt++)
		outline[iPt]->Init(this);
}

void BuildingPrimData::AddPoint(TMCCountedPtrArray<OutlinePoint>& inArray, const TVector2& pos )
{
	TMCCountedPtr<OutlinePoint> newPoint;
	OutlinePoint::CreatePoint(&newPoint, this, pos, NULL);
	inArray.AddElem(newPoint);
}

void BuildingPrimData::Validate()
{
	// Recompute the default window
	real32 minX = - fDefaultWindowLength * .5f;
	real32 maxX = -minX;
	real32 minY = fDefaultWindowAltitude - .5*fDefaultWindowHeight;
	real32 maxY = minY + fDefaultWindowHeight;
	fDefaultWindow.SetElemCount(0);
	AddPoint(fDefaultWindow, TVector2(minX,minY));
	AddPoint(fDefaultWindow, TVector2(maxX,minY));
	AddPoint(fDefaultWindow, TVector2(maxX,maxY));
	AddPoint(fDefaultWindow, TVector2(minX,maxY));
	InitOutlineData(fDefaultWindow);

	// Recompute the default door
	minX = - fDefaultDoorLength * .5f;
	maxX = -minX;
	minY = fDefaultFloorThickness;
	maxY = minY + fDefaultDoorHeight;
	fDefaultDoor.SetElemCount(0);
	AddPoint(fDefaultDoor, TVector2(minX,minY));
	AddPoint(fDefaultDoor, TVector2(maxX,minY));
	AddPoint(fDefaultDoor, TVector2(maxX,maxY));
	AddPoint(fDefaultDoor, TVector2(minX,maxY));
	InitOutlineData(fDefaultDoor);


	// Recompute the default stairway
	minX = - fDefaultStairwayWidth * .5f;
	maxX = - minX;
	minY = - fDefaultStairwayLength * .5f;
	maxY = - minY;
	fDefaultStairway.SetElemCount(0);
	AddPoint(fDefaultStairway, TVector2(minX,minY));
	AddPoint(fDefaultStairway, TVector2(maxX,minY));
	AddPoint(fDefaultStairway, TVector2(maxX,maxY));
	AddPoint(fDefaultStairway, TVector2(minX,maxY));
	InitOutlineData(fDefaultStairway);

	// Default circle
	fDefaultCircle.SetElemCount(0);
	const float pointCount = 4*8; // 4*xx => easy to do half circle or quarter circle
	for( int iPoint=0 ; iPoint<pointCount ; iPoint++)
	{ 
		float angle = iPoint*2*M_PI/pointCount;
		AddPoint(fDefaultCircle, TVector2( 0.5*cos(angle), 0.5*sin(angle) ) );
	}
	InitOutlineData(fDefaultCircle);
}

void ScalePolyline(const TVector2& scaleCenter, const TVector2& scaleValue,
				   TMCCountedPtrArray<OutlinePoint> polyline)
{
	const int32 pointCount = polyline.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		polyline[iPoint]->GetPosition() = ScalePoint(polyline[iPoint]->GetPosition(),scaleValue,scaleCenter);
	}
}

void OffsetPolyline(const TVector2& offsetValue,
				   TMCCountedPtrArray<OutlinePoint> polyline)
{
	const int32 pointCount = polyline.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		polyline[iPoint]->GetPosition() += offsetValue;
	}
}

TMCCountedPtrArray<OutlinePoint> BuildingPrimData::GetDefaultPolyline(EObjectType type)
{
	TMCCountedPtrArray<OutlinePoint> polyline;

	switch(type)
	{
	case eWindow: CopyPointArray(fDefaultWindow, polyline); break;
	case eNarrowWindow: 
		{
			CopyPointArray(fDefaultWindow, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(.5,1), polyline );
		} break;
	case ePanoramicWindow:
		{
			CopyPointArray(fDefaultWindow, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(2,1), polyline );
		} break;
	case eArrowWindow:  
		{
			CopyPointArray(fDefaultWindow, polyline); 

			// Add a point in the middle of the top
			TMCCountedPtr<OutlinePoint> newPoint;
			OutlinePoint::CreatePoint(&newPoint, this, TVector2(0,polyline[3]->GetPosition().y + .25*fDefaultWindowHeight), NULL);
			polyline.InsertElem(3, newPoint);
			newPoint->Init(this);
		} break;
	case eCircle16Window:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(fDefaultWindowHeight,fDefaultWindowHeight), polyline );
	
			OffsetPolyline( TVector2(0,fDefaultWindowAltitude), polyline );
		} break;
	case e2Circle16Window:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(fDefaultWindowHeight,fDefaultWindowHeight), polyline );

			// Half circle on the top : keep the points 0 to 8
			// Add 2 points at the botom of the window => 10 points
			const int pointCount = fDefaultCircle.GetElemCount();
			const int half = pointCount/2;
			polyline.SetElemCount(half+3);

			const real32 halfHeight = .5 * fDefaultWindowHeight;
			const real32 left = polyline[half]->GetPosition().x;
			const real32 right = polyline[0]->GetPosition().x;
			TVector2 pos(left, -halfHeight);
			polyline[half+1]->SetPosition(pos);
			pos.x = right;
			polyline[half+2]->SetPosition(pos);
	
			OffsetPolyline( TVector2(0,fDefaultWindowAltitude), polyline );
		} break;
	case e4Circle16LWindow:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(fDefaultWindowHeight,fDefaultWindowHeight), polyline );
	
			// Quater circle on the left : keep the points 4 to 8
			// Add 2 points at the botom of the window
			const int pointCount = fDefaultCircle.GetElemCount();
			const int half = pointCount/2;
			const int quarter = pointCount/4;
			polyline.SetElemCount(half+3);

			polyline.RemoveElem(0, quarter);

			const real32 halfHeight = .5 * fDefaultWindowHeight;
			const real32 left = polyline[quarter]->GetPosition().x;
			const real32 right = polyline[0]->GetPosition().x;

			TVector2 pos(left, -halfHeight);
			polyline[quarter+1]->SetPosition(pos);
			pos.x = right;
			polyline[quarter+2]->SetPosition(pos);

			OffsetPolyline( TVector2(0,fDefaultWindowAltitude), polyline );
		} break;
	case e4Circle16RWindow:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(fDefaultWindowHeight,fDefaultWindowHeight), polyline );
	
			// Quater circle on the right : keep the points 0 to 4
			// Add 2 points at the botom of the window
			const int pointCount = fDefaultCircle.GetElemCount();
			//const int half = pointCount/2;
			const int quarter = pointCount/4;
			polyline.SetElemCount(quarter+3);

			const real32 halfHeight = .5 * fDefaultWindowHeight;
			const real32 left = polyline[quarter]->GetPosition().x;
			const real32 right = polyline[0]->GetPosition().x;

			TVector2 pos(left, -halfHeight);
			polyline[quarter+1]->SetPosition(pos);
			pos.x = right;
			polyline[quarter+2]->SetPosition(pos);

			OffsetPolyline( TVector2(0,fDefaultWindowAltitude), polyline );
		} break;
	case eDoor:  CopyPointArray(fDefaultDoor, polyline); break;
	case eDoubleDoor:
		{
			CopyPointArray(fDefaultDoor, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(2,1), polyline );
		} break;
	case eArrowDoor:  
		{
			CopyPointArray(fDefaultDoor, polyline); 

			// Add a point in the middle of the top
			TMCCountedPtr<OutlinePoint> newPoint;
			// Offset of windowHeight/4, to have a similar look from the windows
			OutlinePoint::CreatePoint(&newPoint, this, TVector2(0,polyline[3]->GetPosition().y), NULL);
			polyline[2]->GetPosition().y -= .25*fDefaultWindowHeight;
			polyline[3]->GetPosition().y -= .25*fDefaultWindowHeight;
			polyline.InsertElem(3, newPoint);
			newPoint->Init(this);
		} break;
	case e2Circle16Door:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(fDefaultDoorLength,fDefaultDoorLength), polyline );

			// Half circle on the top : keep the points 0 to 8
			// Add 2 points at the botom of the window => 10 points
			const int pointCount = fDefaultCircle.GetElemCount();
			const int half = pointCount/2;
			//const int quarter = pointCount/4;
			polyline.SetElemCount(half+3);

			const real32 startHeight = fDefaultDoorHeight - .5*fDefaultDoorLength;
			OffsetPolyline( TVector2(0,startHeight), polyline );

			const real32 left = polyline[half]->GetPosition().x;
			const real32 right = polyline[0]->GetPosition().x;
			TVector2 pos(left, 0 );
			polyline[half+1]->SetPosition(pos);
			pos.x = right;
			polyline[half+2]->SetPosition(pos);
		} break;
	case e4Circle16LDoor:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(2*fDefaultDoorLength,2*fDefaultDoorHeight), polyline );
	
			// Quater circle on the left : keep the points 4 to 8
			// Add 1 point at the botom to finish the corner
			const int pointCount = fDefaultCircle.GetElemCount();
			const int half = pointCount/2;
			const int quarter = pointCount/4;
			polyline.SetElemCount(half+2);

			polyline.RemoveElem(0, quarter);

			const real32 right = polyline[0]->GetPosition().x;

			TVector2 pos(right, 0);
			polyline[quarter+1]->SetPosition(pos);

			OffsetPolyline( TVector2(0,fDefaultFloorThickness), polyline );
		} break;
	case e4Circle16RDoor:  
		{
			CopyPointArray(fDefaultCircle, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(2*fDefaultDoorLength,2*fDefaultDoorHeight), polyline );
	
			// Quater circle on the right : keep the points 0 to 4
			// Add 2 points at the botom of the window
			const int pointCount = fDefaultCircle.GetElemCount();
			//const int half = pointCount/2;
			const int quarter = pointCount/4;
			polyline.SetElemCount(quarter+2);

			const real32 left = polyline[quarter]->GetPosition().x;

			TVector2 pos(left, 0);
			polyline[quarter+1]->SetPosition(pos);

			OffsetPolyline( TVector2(0,fDefaultFloorThickness), polyline );
		} break;
	case eSquareStairway:  CopyPointArray(fDefaultStairway, polyline); break;
	case eLargeStairway:
		{
			CopyPointArray(fDefaultStairway, polyline);
			ScalePolyline( TVector2::kZero, TVector2(2,1), polyline );
		} break;
	case eWideStairway:
		{
			CopyPointArray(fDefaultStairway, polyline); 
			ScalePolyline( TVector2::kZero, TVector2(1,2), polyline );
		} break;
	case eCircle16Stairway:
		{
			CopyPointArray(fDefaultCircle, polyline);
			ScalePolyline( TVector2::kZero, TVector2(fDefaultStairwayWidth,fDefaultStairwayLength), polyline );
		} break;
	}

	return polyline;
}

// for the roof:
// Top and Bottom part : 
// pos: x is the pos along the roof, y is the cummulated dist along the profile
// Middle part: 
// pos: x is the projected distance relative to the first point
//		y is the actual distance to the v=0 axis

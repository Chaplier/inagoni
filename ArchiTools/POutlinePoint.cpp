/****************************************************************************************************

		POutlinePoint.cpp
		Copyright: (c) 2005 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	8/28/2005

****************************************************************************************************/

#include "POutlinePoint.h"

#include "BuildingDef.h"
#include "BuildingPrimitiveData.h"
#include "InterfaceIDs.h"
#include "MiscComUtilsImpl.h"

void CopyPointArray(const TMCCountedPtrArray<OutlinePoint>& fromArray, 
					TMCCountedPtrArray<OutlinePoint>& toArray)
{
	toArray.ArrayFree();

	const int32 pointCount = fromArray.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		// Copy the point
		TMCCountedPtr<OutlinePoint> newPoint;
		OutlinePoint* curPoint = fromArray[iPoint];
		OutlinePoint::CreatePoint(&newPoint, curPoint->GetData(), curPoint->GetPosition(), curPoint->GetSubObject());

		newPoint->Copy( curPoint );

		toArray.AddElem(newPoint);
	}
}

void OutlinePoint::CreatePoint( OutlinePoint **point, 
								BuildingPrimData* data, 
								const TVector2& pos, 
								SubObject* object)
{
	TMCCountedCreateHelper<OutlinePoint> result(point);

	result = new OutlinePoint(data,pos,object);
	ThrowIfNoMem(result);
}

OutlinePoint::OutlinePoint() : CommonPoint(TVector2::kZero, NULL)
{
	fSubObject = NULL;
}

OutlinePoint::OutlinePoint(BuildingPrimData* data, const TVector2& pos, SubObject* object) 
	: CommonPoint(pos, data)
{
	fSubObject = object;
	Init(data);
}

void OutlinePoint::Copy(OutlinePoint* copyFrom)
{
	CommonPoint::Copy(copyFrom);

	fSubObject = copyFrom->fSubObject;
}

void OutlinePoint::Init(BuildingPrimData*	data)
{
	fData = data;

	if(fData)
	{
		// Default name
		CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
		TMCDynamicString objectName;
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 15);
		SetName(fData->fDictionary, objectName);
	}
}

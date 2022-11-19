/****************************************************************************************************

		PRoofPoint.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/18/2004

****************************************************************************************************/

#include "PRoofPoint.h"

#include "PRoof.h"
#include "IShTokenStream.h"
#include "MiscComUtilsImpl.h"
#include "BuildingDef.h"

RoofPoint::RoofPoint(BuildingPrimData* data, const TVector2& pos, Roof* roof, const boolean onSpine )
: CommonPoint(pos,data)
{
	fRoof = roof;

	if(onSpine) SetFlag(eIsOnSpine);
	else		ClearFlag(eIsOnSpine);

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 13);
	SetName(fData->fDictionary, objectName);
}

RoofPoint::~RoofPoint()
{
}

void  RoofPoint::SetRoof(Roof* roof)
{
	fRoof= roof;
}
	
void RoofPoint::CreateRoofPoint(RoofPoint **point, BuildingPrimData* data, const TVector2& pos, Roof* roof, const boolean onSpine)
{
	TMCCountedCreateHelper<RoofPoint> result(point);

	result = new RoofPoint(data,pos,roof,onSpine);
	ThrowIfNoMem(result);
}

void RoofPoint::DeleteRoofPoint()
{
	const int32 zoneSectionCount = fRoof->GetRoofZoneSectionCount();

	for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
	{
		ZoneSection& zoneSection = fRoof->GetRoofZoneSection(iPt);

		if( zoneSection.fZonePoint == this ||
			zoneSection.fSpinePoint == this )
		{
			fRoof->RemoveRoofZoneSection(iPt);
			return;
		}
	}
}

TVector3	RoofPoint::Get3DPos() const
{
	const real32 z=Flag(eIsOnSpine)?fRoof->GetMaxToGround():fRoof->GetMinToGround();
	return TVector3(fPosition.x,fPosition.y,z);
}

void RoofPoint::SetSelection(const boolean select)
{
	if( select != Selected() )
	{
		if(select)
		{
			SetFlag(eIsSelected);
			fRoof->SelectIfPossible();
		}
		else
		{
			ClearFlag(eIsSelected);
			fRoof->ClearFlag(eIsSelected);
		}

		fData->InvalidateStatus();
	}
}

boolean RoofPoint::SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode) 
{
	if( Hidden() )
		return false;

	boolean outside=false;

	const TVector3 pos = Get3DPos();

	const int32 planeCount = rayPlanes.GetElemCount();
	for( int32 iPlane=0; iPlane<planeCount; iPlane++ )
	{
		const Plane& rayPlane = rayPlanes[iPlane];
		if( ((pos-rayPlane.fPoint)*rayPlane.fNormal) < kRealZero)
		{
			outside=true;
			break;
		}
	}

	if(outside) // Outside the selection area: restaure the selection as it was before
	{
		// Restore the selection on the point
		RestoreSelection();

		// Restore the selection on the roof
		fRoof->RestoreSelection();
	}
	else // Inside the area: select or deselect
	{
		SetSelection(true);

	/*	if( marqueeMode == kMarqueeSelect )
		{
			if (fSelectionFlags&kMarqueeVertex)
				Select(false, false);
			return Select(true);
		}
		else if (marqueeMode == kMarqueeSwap) 
		{
			if (fSelectionFlags&kMarqueeVertex)
				return Select(false);
			else
				return Select(true);
		}
		else if (marqueeMode == kMarqueeDeselect)
		{
			if (fSelectionFlags&kMarqueeVertex)
				Select(true, false);
			return Select(false);
		}*/
	}

	return true;
}

void RoofPoint::GetSurroundingPoints(TMCCountedPtrArray<CommonPoint>& pointsArround)
{
	const int32 sectionCount = fRoof->GetRoofZoneSectionCount();

	if(Flag(eIsOnSpine))
	{
		for(int32 iSection=0 ; iSection<sectionCount ; iSection++ )
		{
			ZoneSection& section = fRoof->GetRoofZoneSection(iSection);
			if(section.fSpinePoint == this)
			{
				const int32 prevIndex = iSection>0?iSection-1:sectionCount-1;
				const int32 nextIndex = (iSection+1)%sectionCount;

				RoofPoint* prevPoint = fRoof->GetRoofZoneSection(prevIndex).fSpinePoint;
			//	if( !prevPoint->Selected() )
					pointsArround.AddElem(prevPoint);

				RoofPoint* nextPoint = fRoof->GetRoofZoneSection(prevIndex).fSpinePoint;
			//	if( !nextPoint->Selected() )
					pointsArround.AddElem(nextPoint);

				return;
			}
		}
	}
	else
	{
		for(int32 iSection=0 ; iSection<sectionCount ; iSection++ )
		{
			ZoneSection& section = fRoof->GetRoofZoneSection(iSection);
			if(section.fZonePoint == this)
			{
				const int32 prevIndex = iSection>0?iSection-1:sectionCount-1;
				const int32 nextIndex = (iSection+1)%sectionCount;
			
				RoofPoint* prevPoint = fRoof->GetRoofZoneSection(prevIndex).fZonePoint;
			//	if( !prevPoint->Selected() )
					pointsArround.AddElem(prevPoint);

				RoofPoint* nextPoint = fRoof->GetRoofZoneSection(prevIndex).fZonePoint;
			//	if( !nextPoint->Selected() )
					pointsArround.AddElem(nextPoint);

				return;
			}
		}
	}
}

void RoofPoint::GetSurroundingPoints(TMCClassArray<TVector2>& points)
{
	// Give the direction toward the prex and next point
	const int32 sectionCount = fRoof->GetRoofZoneSectionCount();

	if(Flag(eIsOnSpine))
	{
		for(int32 iSection=0 ; iSection<sectionCount ; iSection++ )
		{
			ZoneSection& section = fRoof->GetRoofZoneSection(iSection);
			if(section.fSpinePoint == this)
			{
				const int32 prevIndex = iSection>0?iSection-1:sectionCount-1;
				const int32 nextIndex = (iSection+1)%sectionCount;
				const TVector2& prevPos = fRoof->GetRoofZoneSection(prevIndex).fSpinePoint->Position();
				const TVector2& nextPos = fRoof->GetRoofZoneSection(prevIndex).fSpinePoint->Position();
				if(prevPos!=fPosition) points.AddElem(prevPos);
				if(nextPos!=fPosition) points.AddElem(nextPos);
			}
		}
	}
	else
	{
		for(int32 iSection=0 ; iSection<sectionCount ; iSection++ )
		{
			ZoneSection& section = fRoof->GetRoofZoneSection(iSection);
			if(section.fZonePoint == this)
			{
				const int32 prevIndex = iSection>0?iSection-1:sectionCount-1;
				const int32 nextIndex = (iSection+1)%sectionCount;
				const TVector2& prevPos = fRoof->GetRoofZoneSection(prevIndex).fZonePoint->Position();
				const TVector2& nextPos = fRoof->GetRoofZoneSection(prevIndex).fZonePoint->Position();
				if(prevPos!=fPosition) points.AddElem(prevPos);
				if(nextPos!=fPosition) points.AddElem(nextPos);
			}
		}
	}
}

inline void	RoofPoint::InvalidateTessellation(const boolean extraInvalidation)
{
	fRoof->ClearFlag(eRoofTessellated);
}


///////////////////////////////////////////////////////////////////////////
//
//

void RoofPoint::Clone(RoofPoint** newPoint, Roof* roof)
{
	RoofPoint::CreateRoofPoint(newPoint,roof->GetData(),fPosition,roof,Flag(eIsOnSpine));

	TMCCountedPtr<RoofPoint> pointPtr;	
	pointPtr = *newPoint;	
	pointPtr->SetFlags(fFlags);
	pointPtr->SetNamePtr(fName);

	pointPtr->ClearFlag(eIsTargeted);
}


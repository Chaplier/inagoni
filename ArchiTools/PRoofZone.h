/****************************************************************************************************

		PRoofZone.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/16/2004

****************************************************************************************************/

#ifndef __PRoofZone__
#define __PRoofZone__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCCountedObject.h"
#include "MCCompObj.h"
#include "BasicCOMImplementations.h"
#include "MCClassArray.h"
#include "Vector2.h"
#include "PRoofPoint.h"
#include "PPoint.h"
#include "BuildingModelerEnums.h"

struct IShTokenStream;


class ZoneSection
{
public:
	ZoneSection(){fIsVertical=false;}

	MCCOMErr			Read(IShTokenStream* stream, Roof* roof);

	TMCCountedPtr<RoofPoint>	fZonePoint;// Editable pos on the surounding
	TMCCountedPtr<RoofPoint>	fSpinePoint;// Editable pos on the central spine

	inline void			SetIsVertical( const boolean isVertical ) { fIsVertical = isVertical; }
	inline boolean		GetIsVertical() const {return fIsVertical;}

protected:

	// The zone is vertical if the spine point is is further out than the zone point
	boolean				fIsVertical;
};

class RoofZone : public TMCCountedObject
{
public:
	// Usualy pass in the surounding points of the top level
	RoofZone();
//	RoofZone(const TMCCountedPtrArray<Point>& area, Roof* roof);

	void				SetArea(const TMCCountedPtrArray<VPoint>& area, Roof* roof, const ERoofType roofType);

	// Data access
	inline int32		GetSectionCount() const { return fZoneSection.GetElemCount(); }
	inline ZoneSection&	GetZoneSection(const int32 i) {return fZoneSection[i];}
	const inline TMCClassArray<ZoneSection>&	ZoneArray() const {return fZoneSection;}
	inline TMCClassArray<ZoneSection>&	GetZoneArray() {return fZoneSection;}

	inline void			ClearData(){fZoneSection.ArrayFree();}

//	inline RoofZone&	operator=(const RoofZone& fromZone);
	RoofZone&			Copy(const RoofZone& fromZone, Roof* roof);

	MCCOMErr			Read(IShTokenStream* stream, Roof* roof);

protected:

	void				DetermineZoneOrientation(Roof* roof);
	void				BuildCentralSpine();
	void				AdjustZonePoints(const TBBox2D& rectangle, BuildingPrimData* data);
	void				AdjustSpinePoints(const ERoofType roofType);

	TMCClassArray<ZoneSection>	fZoneSection;
};


#endif

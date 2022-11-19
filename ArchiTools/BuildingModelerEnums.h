/****************************************************************************************************

		BuildingModelerEnums.h

		Author:	Arnaud
		Date:	10/13/2004

****************************************************************************************************/

#ifndef __BuildingModelerEnums__
#define __BuildingModelerEnums__

#if CP_PRAGMA_ONCE
#pragma once
#endif


enum ERoofFlags
{
	eRoofTessellated = 0x00010000,
	eRoofExtendedSelection = 0x00020000,
	eRoofZoneOrientedPositive = 0x00040000,
	eRoofNoElasticZone = 0x00080000
};

enum ERoofType
{
	eLevelShapeRoof,
	eLevelShapeClosedSpineRoof,
	eRectangleRoof,
	eRectangleRoofFlatExtremsX,
	eRectangleRoofFlatExtremsY,
	eHalfRectangleRoof1,
	eHalfRectangleRoof2,
	eHalfRectangleRoof3,
	eHalfRectangleRoof4,
	eBorderRoof,
};





#endif

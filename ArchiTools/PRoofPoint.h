/****************************************************************************************************

		PRoofPoint.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/18/2004

****************************************************************************************************/

#ifndef __PRoofPoint__
#define __PRoofPoint__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCAssert.h"
#include "MCCountedObject.h"
#include "Vector2.h"
#include "Vector3.h"
#include "PCommonBase.h"
#include "Utils.h"

class Roof;
struct IShTokenStream;

enum ERoofPointFlags
{
	eIsOnSpine = 0x00010000,
};

class RoofPoint :	public CommonPoint
{
public:

	static void			CreateRoofPoint(RoofPoint **point, BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean onSpine);
	void				DeleteRoofPoint();

	void				Clone(RoofPoint** newPoint, Roof* onRoof);

	TVector3			Get3DPos() const;

	boolean				SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode);
//	void				HidePoint();
	virtual void		SetSelection(const boolean select);

	virtual void		GetSurroundingPoints(TMCClassArray<TVector2>& points);
	virtual void		GetSurroundingPoints(TMCCountedPtrArray<CommonPoint>& pointsArround);

	virtual void		InvalidateTessellation(const boolean extraInvalidation=false);

	void				SetRoof(Roof* roof);
protected:

	RoofPoint(BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean onSpine);
	virtual ~RoofPoint();

	TMCCountedPtr<Roof> fRoof;
};

#endif

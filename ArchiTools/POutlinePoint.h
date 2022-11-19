/****************************************************************************************************

		POutlinePoint.h
		Copyright: (c) 2005 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	8/28/2005

****************************************************************************************************/

#ifndef __POutlinePoint__
#define __POutlinePoint__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedObject.h"
#include "PCommonBase.h"
//#include "PSubObject.h"
class SubObject;

// the 16 last bits are for the common flags
enum EOutilinePointFlags
{
	eSmoothPoint		= 0x00010000,
};

class OutlinePoint : public CommonPoint
{
public:
	static void	CreatePoint(OutlinePoint **point, 
							BuildingPrimData* data, 
							const TVector2& pos, 
							SubObject* onObject);

	void Init(BuildingPrimData*	data);

	void Copy(OutlinePoint* copyFrom);

	SubObject* GetSubObject(){return fSubObject;}
	void SetObject(SubObject* object){fSubObject = object;}

	boolean IsSmoothed() const {return Flag(eSmoothPoint);}
protected:

	OutlinePoint();
	OutlinePoint(BuildingPrimData* data, const TVector2& pos, SubObject* object);

	// The subobject is needed to get the plane of displacement in 3D 
	SubObject* fSubObject; // not counted: it's the parent

	// use EOutilinePointFlags to add properties to this point

};

void CopyPointArray(const TMCCountedPtrArray<OutlinePoint>& fromArray, 
					TMCCountedPtrArray<OutlinePoint>& toArray);

#endif
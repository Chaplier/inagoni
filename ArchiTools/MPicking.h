/****************************************************************************************************

		MPicking.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/1/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MPicking__
#define __MPicking__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "ArchiTools.h"
#include "Utils.h"
#include "IShUtilities.h"

class BuildingModeler;
class BuildingPanePart;
//class Wall;
//class Room;
//class SubObject;
//class Point;
class CommonBase;

class Picking
{
public:
	Picking(	BuildingModeler*	modeler,
				BuildingPanePart*	panePart,
				const TMCPoint&		pos,
				const int32			pickingFilter );

	Picking(	const Picking& picked );

	Picking(	CommonBase* obj, EPickedType type);

	Picking(){fPickedType=eNothingPicked;fPickedObject=NULL;}

	const inline EPickedType	GetPickedType() const  {return fPickedType;}
	const inline boolean		GetHitPosition(TVector3& pos) const ;

	inline CommonBase*			PickedObject() const {return fPickedObject;}

	inline void					SetPickedObject(CommonBase* obj){fPickedObject=obj;}
	inline void					SetPickedType(EPickedType type){fPickedType=type;}
	inline void					SetHitPosition(const TVector3& pos){fHitPosition=pos;}
protected:

	EPickedType fPickedType;
	TMCCountedPtr<CommonBase> fPickedObject;

	TVector3 fHitPosition;
};

const inline boolean Picking::GetHitPosition(TVector3& pos) const
{
	if (fPickedType==eNothingPicked) return false;
	pos=fHitPosition;
	return true;
}


#endif

#endif // !NETWORK_RENDERING_VERSION

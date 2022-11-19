/****************************************************************************************************

		MPicking.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/1/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MPicking.h"

#include "MBuildingPanePart.h"
#include "BuildingModeler.h"
#include "PWall.h"
#include "PRoom.h"
#include "PSubObject.h"

Picking::Picking(	BuildingModeler*	modeler,
					BuildingPanePart*	panePart,
					const TMCPoint&		pos,
					const int32			pickingFilter )
{
	fHitPosition=TVector3::kZero;

	FatRay fatRay;

	// Get the origin and direction of the picking
	panePart->Get3DEditorHostPanePart()->PixelRay(pos,fatRay.fOrigin, fatRay.fDirection);

	// Build a precision zone arround the picked pixel
	const int32 hWidth=4;
	const int32 hSmallWidth=2;

	const TMCPoint diag(hWidth,hWidth);
	const TMCPoint corner1 = pos-diag;
	const TMCPoint corner2 = pos+diag;
	MakeRayPlanes(fatRay.fRayPlanes, panePart->Get3DEditorHostPanePart(), corner1, corner2, fatRay.fOrigin, fatRay.fDirection);

	const TMCPoint smallDiag(hSmallWidth,hSmallWidth);
	const TMCPoint smallCorner1 = pos-smallDiag;
	const TMCPoint smallCorner2 = pos+smallDiag;
	MakeRayPlanes(fatRay.fRayPlanesPrecise, panePart->Get3DEditorHostPanePart(), smallCorner1, smallCorner2, fatRay.fOrigin, fatRay.fDirection);

	fPickedType = modeler->GetBuildingNoAddRef()->Pick(&fPickedObject,pickingFilter,fatRay,fHitPosition,panePart->GetInLevel(), panePart->UsePlanMesh());
}

Picking::Picking( const Picking& picked )
{
	fPickedType = picked.GetPickedType();
	fPickedObject = picked.PickedObject();

	picked.GetHitPosition(fHitPosition);
}

Picking::Picking(CommonBase* obj, EPickedType type)
{
	fPickedType = type;
	fPickedObject = obj;

	fHitPosition = TVector3::kZero;
}

#endif // !NETWORK_RENDERING_VERSION


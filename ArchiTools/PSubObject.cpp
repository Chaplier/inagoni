/****************************************************************************************************

		PSubObject.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/31/2004

****************************************************************************************************/

#include "PSubObject.h"
#include "MCCountedPtrHelper.h"
#include "IShTokenStream.h"
#include "IShUtilities.h"
#include "COMUtilities.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShInstance.h"
#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3DShMasterGroup.h"
#include "I3DShShader.h"
#include "I3DShTreeElement.h"
#include "ISceneDocument.h"
#include "Matrix33.h"
#include "PLevel.h"
#include "BuildingPrim.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "COMSafeUtilities.h"
#endif

WallObjPos::WallObjPos(const int32 inLevel, const int32 wallIndex, TVector2& center)
{
	fInLevel=inLevel;
	fOnWallIndex=wallIndex;

	TMCCountedPtr<OutlinePoint> newPoint;
	OutlinePoint::CreatePoint(&newPoint, NULL, center, NULL);
	fPositions.AddElem(newPoint);
}

WallObjPos::WallObjPos(const int32 inLevel, const int32 wallIndex, const TMCCountedPtrArray<OutlinePoint>& pos)
{
	fInLevel=inLevel;
	fOnWallIndex=wallIndex;

	CopyPointArray(pos, fPositions);
}

RoomObjPos::RoomObjPos(const int32 inLevel, const int32 roomIndex, TVector2& center)
{
	fInLevel=inLevel;
	fInRoomIndex=roomIndex;

	TMCCountedPtr<OutlinePoint> newPoint;
	OutlinePoint::CreatePoint(&newPoint, NULL, center, NULL);
	fPositions.AddElem(newPoint);
}
RoomObjPos::RoomObjPos(const int32 inLevel, const int32 roomIndex, const TMCCountedPtrArray<OutlinePoint>& pos)
{
	fInLevel=inLevel;
	fInRoomIndex=roomIndex;

	CopyPointArray(pos, fPositions);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// SubObject
//
SubObject::SubObject() : fBuilding(NULL)
{
	fOrigin = TVector3::kZero;
	fWidth = TVector3::kZero;
	fLength = TVector3::kZero;
	fHeight = TVector3::kZero;
	fCenter = TVector3::kZero;

	fPolylineCenter = TVector2::kZero;

	fPlacement = eFitIn;
	fOffset = TVector3::kZero;
	fScale = TVector3::kOnes;
	fRotate = TVector3::kZero;

	SetFlag(eAutoFlipObj);

//	fChildIndex = kNoChild;
	fChildTreePath = TTreeIdPath::InvalidPath();

	fHoleBBox.fMin = fHoleBBox.fMax = TVector2::kZero;
	fSelBBox.fMin = fSelBBox.fMax = TVector2::kZero;
}

SubObject::~SubObject()
{
	fHoleOutline.ArrayFree();
}

void SubObject::Copy(SubObject* copyFrom, Level* inLevel, const ECloneChildrenMode cloneMode)
{
	// Copy the outline
	SetOutline(copyFrom->fHoleOutline);

	fHoleBBox=copyFrom->fHoleBBox;
	fSelBBox=copyFrom->fSelBBox;
	fFlags = copyFrom->fFlags;
	fName = copyFrom->fName;
	fPlacement = copyFrom->fPlacement;
	fOffset = copyFrom->fOffset;
	fScale = copyFrom->fScale;
	fRotate = copyFrom->fRotate;

/*	int32 workWithID = 0;
	if( cloneMode==eCloneChildrenFromTreeID ||
		cloneMode == eNoChildrenAndRecordID ||
		cloneMode == eNoChildren )
		workWithID = copyFrom->GetTreeID();
	else if(cloneMode==eCloneChildrenFromRecordedTreeID)
	{
		if(copyFrom->fRecoveryData)
		{
			workWithID = copyFrom->fRecoveryData->fRecordedTreeID;
		}
	}*/

//	if(workWithID)
//	fChildIndex = copyFrom->GetChildIndex();
	fChildTreePath = copyFrom->fChildTreePath;
//	if(fChildIndex!=kNoChild)
	if( fChildTreePath.isValid() )
	{
		switch(cloneMode)
		{
		case eCloneChild:
			{
				// Create a new instance and insert it in the scene as a child of the new object
				I3DShInstance* childInstance = copyFrom->GetChildNoAddRef(fBuilding);
				if(MCVerify(childInstance))
				{
					TMCCountedPtr<I3DShObject> childObject;
					childInstance->Get3DObject(&childObject);

					if(inLevel)
					{
						SetSceneObject(childObject);
					}
				}
			}break;
		case eNoChild:
			{
			} break;
		}
	}

	// Clear some flags
	Invalidate();
	ClearFlag(eIsTargeted);
}

void SubObject::SetOutline(const TMCCountedPtrArray<OutlinePoint>& outline)
{
	CopyPointArray(outline, fHoleOutline);
	const int32 pointCount = fHoleOutline.GetElemCount();
	for(int32 iPoint=0 ; iPoint<pointCount ; iPoint++)
	{
		fHoleOutline[iPoint]->SetObject(this);
	}
}

void SubObject::ValidateBBox()
{
	if(!Flag(eHoleBBoxIsValid)) 
	{
		const int32 pointCount = fHoleOutline.GetElemCount();
		MY_ASSERT(pointCount);
		fHoleBBox.fMin = fHoleBBox.fMax = fHoleOutline[0]->GetPosition();
		for(int32 iPoint=1 ; iPoint<pointCount ; iPoint++)
		{
			fHoleBBox.AddPoint(fHoleOutline[iPoint]->GetPosition());
		}
		SetFlag(eHoleBBoxIsValid);
	}
}

void SubObject::ValidateSelectionBBox()
{
	if(!Flag(eSelBBoxIsValid)) 
	{
		const int32 pointCount = fHoleOutline.GetElemCount();
		MY_ASSERT(pointCount);
		fSelBBox.fMin = fSelBBox.fMax = fHoleOutline[0]->GetPosition();
		for(int32 iPoint=1 ; iPoint<pointCount ; iPoint++)
		{
			if(fHoleOutline[iPoint]->Selected())
				fSelBBox.AddPoint(fHoleOutline[iPoint]->GetPosition());
		}
		SetFlag(eSelBBoxIsValid);
	}
}

void SubObject::GetCenter( TVector3& center)
{
	Validate();

	center = fCenter;
}

boolean SubObject::SetPolylineCenter( const TVector2& center, const boolean checkFirst)
{
	const TVector2 prevCenter = GetPolylineCenter();

	const TVector2 offset = center-prevCenter;
	return OffsetPolyline(offset, checkFirst);
}

void SubObject::ValidateCenter()
{
	if(!Flag(eObjCenterIsValid)) 
	{
		GetHoleBBox().GetCenter(fPolylineCenter);

		SetFlag(eObjCenterIsValid);
	}
}

boolean SubObject::HasSelectedHolePoint()
{

	const int32 pointCount = fHoleOutline.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		OutlinePoint* point = fHoleOutline[iPoint];
		if(point->Selected())
			return true;
	}

	return false;
}

boolean SubObject::OffsetPolylinePoints(const TVector2& offset)
{
	// Simple offset: nothing to check here
	{
		boolean invalidate = false;

		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			OutlinePoint* point = fHoleOutline[iPoint];
			if(point->Selected())
			{
				invalidate = true;
				point->GetPosition()+=offset;
			}
		}
		if(invalidate)
			Invalidate();
	}

	return true;
}

boolean SubObject::ScalePolylinePoints(const TVector2& scale, const boolean arroundObjCenter)
{
	// Simple scale: nothing to check here
	{
		boolean invalidate = false;

		TVector2 scaleCenter;
		if(arroundObjCenter)
			scaleCenter = GetPolylineCenter();
		else
			GetSelectionBBox().GetCenter(scaleCenter);

		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			OutlinePoint* point = fHoleOutline[iPoint];
			if(point->Selected())
			{
				invalidate = true;
				point->GetPosition() = ScalePoint(point->GetPosition(),scale,scaleCenter);
			}
		}
		if(invalidate)
			Invalidate();
	}

	return true;
}

boolean SubObject::RotatePolylinePoints(const TVector2& cosSin, const boolean arroundObjCenter)
{
	// Simple rotate: nothing to check here
	{
		boolean invalidate = false;

		TVector2 rotationCenter;
		if(arroundObjCenter)
			rotationCenter = GetPolylineCenter();
		else
			GetSelectionBBox().GetCenter(rotationCenter);

		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			OutlinePoint* point = fHoleOutline[iPoint];
			if(point->Selected())
			{
				invalidate = true;
				point->GetPosition() = RotatePoint(point->GetPosition(),cosSin,rotationCenter);
			}
		}
		if(invalidate)
			Invalidate();
	}

	return true;
}

boolean SubObject::SmoothPolylinePoints(const boolean smooth)
{
	boolean invalidate = false;

	const int32 pointCount = fHoleOutline.GetElemCount();
	for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
	{
		OutlinePoint* point = fHoleOutline[iPoint];
		if(point->Selected())
		{
			invalidate = true;
			if(smooth)
				point->SetFlag(eSmoothPoint);
			else
				point->ClearFlag(eSmoothPoint);
		}
	}
	if(invalidate)
	{
		// We smoothed an edge: the tessellation is invalid, not the bbox or center
		Invalidate();
		InvalidateTessellation();
	}

	return invalidate;
}

boolean SubObject::OffsetPolyline(const TVector2& offset, const boolean checkFirst)
{
	// Simple offset: nothing to check here
	{
		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			fHoleOutline[iPoint]->GetPosition()+=offset;
		}
		Invalidate();
	}

	return true;
}

boolean SubObject::ScalePolyline(const TVector2& scale, const boolean checkFirst)
{
	// Simple scale: nothing to check here
	{
		const TVector2& polylineCenter = GetPolylineCenter();

		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			fHoleOutline[iPoint]->GetPosition() = ScalePoint(fHoleOutline[iPoint]->GetPosition(),scale,polylineCenter);
		}
		Invalidate();
	}

	return true;
}

boolean SubObject::RotatePolyline(const TVector2& cosSin, const boolean checkFirst)
{
	// Simple rotate: nothing to check here
	{
		const TVector2 polylineCenter = GetPolylineCenter();

		const int32 pointCount = fHoleOutline.GetElemCount();
		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			fHoleOutline[iPoint]->GetPosition() = RotatePoint(fHoleOutline[iPoint]->GetPosition(),cosSin,polylineCenter);
		}
		Invalidate();
	}

	return true;
}

void SubObject::GetBox( TMCClassArray<TVector3>& pos )
{
	Validate();

	// Make the hole pos: 0,1,2,3 are at the bottom, 4,5,6,7 at the top
	pos.SetElemCount(8);
	pos[0] = fOrigin+fWidth;
	pos[1] = pos[0]+fLength;
	pos[2] = fOrigin+fLength;
	pos[3] = fOrigin;
	pos[4] = pos[0]+fHeight;
	pos[5] = pos[1]+fHeight;
	pos[6] = pos[2]+fHeight;
	pos[7] = pos[3]+fHeight;
}

// Return the 3D pos of the points of the hole. 
// One array for each side of the wall or roof,
// plus one array for the normals inside the hole
// Each point has 2 normals: one for before, one for after
// For the room: side0 is at the floor height, side1 at the ceiling height
void SubObject::Get3DOutlines(TMCClassArray<TVector3>& side0,
							  TMCClassArray<TVector3>& side1,
							  TMCClassArray<TVector3>& normals)
{
	Validate();

	side0 = fSide0;
	side1 = fSide1;
	normals = fNormals;
}

// Note: will need more parameters for the rooms: it currently
// doesn't compute the point where they can be drawn.
void SubObject::Compute3DOutlines( const TVector3& thickness )
{
	const int32 posCount = fHoleOutline.GetElemCount();

	fSide0.SetElemCount(posCount);
	fSide1.SetElemCount(posCount);
	fNormals.SetElemCount(2*posCount);

	// the hole bbox for reference
	const TBBox2D& bbox = GetHoleBBox();
	const TVector2 min = bbox.fMin;
	const TVector2 max = bbox.fMax;

	// A base attached to the wall or room
	TVector3 O,I,J,K;
	GetBase(O,I,J,K);

	{	// The points
		for(int32 iPos=0 ; iPos<posCount ; iPos++)
		{
			const TVector2 offset2D = (fHoleOutline[iPos]->GetPosition() - min);
			const TVector3 offset3D = offset2D.x*I + offset2D.y*J;
			fSide0[iPos] = fOrigin + offset3D;
			fSide1[iPos] = fSide0[iPos] + thickness;
		}
	}

	{	// The normals direction
		TVector3 prev = fSide0[posCount-1];
		TVector3 cur = fSide0[0];
		for(int32 iPos=0 ; iPos<posCount ; iPos++)
		{
			TVector3 next = fSide0[(iPos+1)%posCount];

			TVector3 prevNorm = prev-cur;
			prevNorm.Normalize();
			prevNorm = prevNorm^K;
			TVector3 nextNorm = cur-next;
			nextNorm.Normalize();
			nextNorm = nextNorm^K;

			if( fHoleOutline[iPos]->Flag(eSmoothPoint) )
			{	// Smooth the normal
				prevNorm += nextNorm;
				prevNorm.Normalize();
				nextNorm = prevNorm;
			}
			
			fNormals[2*iPos] = prevNorm;
			fNormals[2*iPos + 1] = nextNorm;

			// Move to the next point
			prev = cur;
			cur = next;
		}
		// Normalize them
//		const int32 normalCount = 2*posCount;
//		for(int32 iNor=0 ; iNor<normalCount ; iNor++)
//		{
//			fNormals[iNor].Normalize();
//		}
	}
}
// Normals in 2D. Use to compute the UV values inside the depth of the hole
void SubObject::Get2DNormals(TMCClassArray<TVector2>& normals)
{
	const int32 posCount = fHoleOutline.GetElemCount();

	normals.SetElemCount(posCount);
	TVector2 prev = fHoleOutline[0]->GetPosition();
	for(int32 iPos=0 ; iPos<posCount ; iPos++)
	{
		TVector2 cur = fHoleOutline[(iPos+1)%posCount]->GetPosition();
		TVector2 dir = cur-prev;
		dir.Normalize();
		normals[iPos].x = -dir.y;
		normals[iPos].y = dir.x;

		prev = cur;
	}
}

I3DShInstance* SubObject::SetSceneObject(I3DShObject* objectPrimitive)
{
	ClearFlag(eObjTransformIsValid);
	
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	if(fBuilding)
		fBuilding->GetInstances(buildingInstances);

	if(objectPrimitive)
	{
		TMCCountedPtr<I3DShInstance> newInstance;
		TMCCountedPtr<I3DShExternalPrimitive> extShPrimitive;
		objectPrimitive->QueryInterface(IID_I3DShExternalPrimitive, (void**) &extShPrimitive);
		if(extShPrimitive)
		{	// Classic Master Object case
			// Create instance and attach shell primitive to it
			gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&newInstance);			
		}
		else
		{
			TMCCountedPtr<I3DShMasterGroup> masterGroup;
			objectPrimitive->QueryInterface(IID_I3DShObject, (void **)&masterGroup);
			if(masterGroup)
			{	// The object is a master group -> create a scene instance
				gComponentUtilities->CoCreateInstance(CLSID_StandardSceneInstance, NULL, 1, IID_I3DShInstance, (void **)&newInstance);
			//	newTreeElement->SetMasterGroup(masterGroup);
			}
			else
			{
				return NULL;
			}
		}

		ThrowIfNil(newInstance);

		newInstance->Set3DObject(objectPrimitive);
	
		TMCCountedPtr<I3DShTreeElement> newTreeElement;
		newInstance->QueryInterface(IID_I3DShTreeElement, (void**) &newTreeElement); ThrowIfNil(newTreeElement);

		// Remember it's tree elem ID
		const int32 instanceCount = buildingInstances.GetElemCount();
		// If more than one instance, need to create new ones for the others
		if(instanceCount>1)
		{	// TODO: mutliple instances of the primitive
			MCNotify("Case not done: multiple instances of the primitive");
		}

		if(!MCVerify(instanceCount))
		{
			MCNotify("No instance found");
			return NULL;
		}

		TMCCountedPtr<I3DShTreeElement> treeElement;
		buildingInstances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement); 
		ThrowIfNil(treeElement);

		I3DShTreeElement* privateGroup = fBuilding->GetPrivateGroup( treeElement );
	//	const int32 curTreeCount = GetSonCount(treeElement);

		if( privateGroup->InsertLast(newTreeElement) != kNoErr )
		{
			MCNotify("Insert tree error");
		}

		// Now that the tree is inserted in the scene, we can record its ID
	//	SetTreeID( newTreeElement->GetTreePermanentID() );
	//	fChildIndex = curTreeCount;
		fChildTreePath = newTreeElement->GetTreeIDPath();

		{	// As a bonus, we can set the shader on the first instance of the object found
			// on the new instance
			const int32 curCount = objectPrimitive->GetInstancesCount();
			if(curCount>1) // If only 1, it's the one we just added
			{
				TMCCountedPtrArray<I3DShInstance> instances;
				objectPrimitive->GetInstanceArray(instances);
				TMCCountedPtr<I3DShMasterShader> masterShader;
				instances[0]->GetShader(&masterShader);
				newInstance->SetShader(masterShader);
			}
		}
	
		return newInstance;
	}
	else
	{	// No child: forget the previous child
		I3DShInstance* oldInstance = GetChildNoAddRef();
		if(oldInstance)
		{
			TMCCountedPtr<I3DShTreeElement> oldTreeElement;
			oldInstance->QueryInterface(IID_I3DShTreeElement, (void**) &oldTreeElement); ThrowIfNil(oldTreeElement);
	
			const int32 instanceCount = buildingInstances.GetElemCount();
			// If more than one instance, need to remove more than one
			if(instanceCount>1)
			{
				MCNotify("Case not done: multiple instances of the primitive");
			}

			//TMCCountedPtr<I3DShTreeElement> treeElement;
			//buildingInstances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement); ThrowIfNil(treeElement);
			//
			//I3DShTreeElement* privateGroup = fBuilding->GetPrivateGroup( treeElement );
	
			TMCCountedPtr<I3DShTreeElement> fatherTree;
			oldTreeElement->Unlink(&fatherTree);
// Should use this ?
// 		gChangeManager->PostChange(fTreePropertyChannel, kChange_TreeRemoved, tree);

			Invalidate();
		}

		fChildTreePath = TTreeIdPath::InvalidPath();
	}

	return NULL;
}

void SubObject::UpdateInstanceTransform()
{
	if(Flag(eObjTransformIsValid))
		return;

	I3DShInstance* childInstance=GetChildNoAddRef();
	if(!childInstance)
		return;

	if(fPlacement!=eFreePlacement)
	{
		// Get the object dimensions
		TMCCountedPtr<I3DShObject> object;
		childInstance->Get3DObject(&object);
		TBBox3D bbox;
		object->GetBoundingBox(bbox);

		// Set the transform
		TMCCountedPtr<I3DShTreeElement> treeElement;
		childInstance->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement);
		ThrowIfNil(treeElement);

		TTreeTransform treeTransform;
		ComputeTreeTransform(treeTransform, bbox);
		treeElement->SetLocalTreeTransform(treeTransform);
	}

	SetFlag(eObjTransformIsValid);
}

// Split successive polyline points that are selected
void SubObject::Split()
{
	const int32 posCount = fHoleOutline.GetElemCount();

	boolean prevSelected = fHoleOutline[0]->Selected();
	for(int32 iPos=posCount-1 ; iPos>=0 ; iPos--)
	{
		const boolean curSelected = fHoleOutline[iPos]->Selected();

		if(curSelected && prevSelected)
		{	// Add a new point in between
			TMCCountedPtr<OutlinePoint> newPoint;
			TVector2 newPos = .5*(fHoleOutline[iPos]->GetPosition() + fHoleOutline[(iPos+1)%posCount]->GetPosition());
			OutlinePoint::CreatePoint(&newPoint, fHoleOutline[0]->GetData(), newPos, this);
			fHoleOutline.InsertElem((iPos+1)%posCount, newPoint);
			Invalidate();
		}

		prevSelected = curSelected;
	}
}

void SubObject::InitMarqueeSelection(const boolean onPoints)
{
	if(onPoints)
	{	// Init on the points
		const int32 ptCount = fHoleOutline.GetElemCount();
		for(int32 iPt=0 ; iPt<ptCount ; iPt++)
		{
			fHoleOutline[iPt]->InitMarqueeSelection();
		}
	}
	else
	{
		// Init on the object handle
		CommonBase::InitMarqueeSelection();
	}
}

boolean IsOutside( const TVector3& pos, const TMCArray<Plane> &rayPlanes )
{
	boolean outside=false;

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

	return outside;
}

void SubObject::DeleteSelectedPoints()
{
	const int32 ptCount = fHoleOutline.GetElemCount();

	if(ptCount<3)
		return ;

	for(int32 iPt = ptCount-1 ; iPt>=0 ; iPt--)
	{
		if(fHoleOutline.GetElemCount()<=3)
			return ;

		if(fHoleOutline[iPt]->Selected())
		{
			fHoleOutline.RemoveElem(iPt, 1);
			Invalidate();
			InvalidateTessellation();
		}
	}
}

boolean SubObject::SetMarqueeOnPoints(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode) 
{
	Validate();

	const int32 ptCount = fHoleOutline.GetElemCount();

	for(int32 iPt = 0 ; iPt<ptCount ; iPt++)
	{
		boolean outside = IsOutside(fSide0[iPt], rayPlanes);
		if(outside)
			outside = IsOutside(fSide1[iPt], rayPlanes);

		if(outside) // Outside the selection area: restaure the selection as it was before
		{
			// Restore the selection on the point
			fHoleOutline[iPt]->RestoreSelection();
		}
		else // Inside the area: select or deselect
		{
			fHoleOutline[iPt]->SetSelection(true);

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
	}

	return true;	
}

boolean SubObject::SetMarqueeOnObject(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode) 
{
	if( Hidden() )
		return false;

	boolean outside=IsOutside(fCenter, rayPlanes);

	if(outside) // Outside the selection area: restaure the selection as it was before
	{
		// Restore the selection on the point
		RestoreSelection();

		// Restore the selection on the sub objects
	}
	else // Inside the area: select or deselect
	{
		if(Flag(eWasSelected))
			SetSelection(false);
		else
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

void SubObject::InvertSelection()
{
	if( Flag(eWasSelected) )
		ClearFlag(eIsSelected);
	else
		SetFlag(eIsSelected);
}

void SubObject::SetOutlineFlag(const int32 flag)
{
	const int32 ptCount = fHoleOutline.GetElemCount();
	for(int32 iPt=0 ; iPt<ptCount ; iPt++)
	{
		fHoleOutline[iPt]->SetFlag(flag);
	}
}

void SubObject::ClearOutlineFlag(const int32 flag)
{
	const int32 ptCount = fHoleOutline.GetElemCount();
	for(int32 iPt=0 ; iPt<ptCount ; iPt++)
	{
		fHoleOutline[iPt]->ClearFlag(flag);
	}
}

ECollisionType SubObject::Collide(SubObject* otherObject, TVector2& problem)
{
	// Compare the 2 oulines to know if their's a conflict between the objects

	// There's a conflict if one point of the first one is in the second one or if
	// one point of the second one is in the first one or if there's an intersection
	// between the edges
	const int32 hereCount = fHoleOutline.GetElemCount();
	const TMCCountedPtrArray<OutlinePoint>& otherOutline = otherObject->GetOutline();
	const int32 otherCount = otherOutline.GetElemCount();
	for(int32 iHere=0 ; iHere<hereCount ; iHere++)
	{
		const TVector2& here0 = fHoleOutline[iHere]->GetPosition();
		if(otherObject->PointIn(here0))
		{
			problem = here0;
			return ePointHereInOtherObject;
		}

		const TVector2& here1 = fHoleOutline[(iHere+1)%hereCount]->GetPosition();
		for(int32 iOther=0 ; iOther<otherCount ; iOther++)
		{
			const TVector2& other0 = otherOutline[iOther]->Position();
			if(PointIn(other0))
			{
				problem = other0;
				return ePointOtherInHereObject;
			}

			const TVector2& other1 = otherOutline[(iOther+1)%otherCount]->Position();
			if(GetSegmentsIntersection(problem,here0,here1,other0,other1)== eIntersection)
			{	// there's a conflict between the 2 objects
				return eSideIntersection;
			}
		}
	}

	return eNoCollision;
}

boolean SubObject::PointIn(const TVector2& point) 
{
	// Basic check: see if the point is inside the bbox
	return GetHoleBBox().IsInside(point);
	// TO DO: a more precise check: is the point inside the polyine ?
/*
	const real32 xMax = MC_Max(fHoleOutline[0].x, fHoleOutline[2].x);
	const real32 xMin = MC_Min(fHoleOutline[0].x, fHoleOutline[2].x);
	const real32 yMax = MC_Max(fHoleOutline[0].y, fHoleOutline[2].y);
	const real32 yMin = MC_Min(fHoleOutline[0].y, fHoleOutline[2].y);
	return( (point.x>xMin) &&
			(point.x<xMax) &&
			(point.y<yMax) &&
			(point.y>yMin) );*/
}

void SubObject::MinCorner(TVector2& corner)
{
	corner = GetHoleBBox().fMin;
}

void SubObject::MaxCorner(TVector2& corner)
{
	corner = GetHoleBBox().fMax;
}

// inBuildingPrimitive is used only when the object is not attached to something yet
I3DShInstance* SubObject::GetChildNoAddRef(BuildingPrim* inBuildingPrimitive)
{
	if( fChildTreePath.isValid() )
	{
		BuildingPrim* buildingPrim = GetParentBuilding();
		if( !buildingPrim )
		{
			buildingPrim = inBuildingPrimitive;
		}
		if( !buildingPrim )
			return NULL;

		TMCCountedPtr<I3DShScene> scene;
		buildingPrim->GetScene( &scene );

		TMCCountedPtr<I3DShTreeElement> sonTree;
		scene->GetTreeByIDPath(&sonTree, fChildTreePath);
		
		if( !sonTree )
			return NULL;

		TMCCountedPtr<I3DShInstance> childInstance;
		sonTree->QueryInterface(IID_I3DShInstance, (void**) &childInstance); 
		return childInstance;
	}
/*
	if(fChildIndex!=kNoChild)
	{
		// We first need to check that we've got enough children
		TMCCountedPtrArray<I3DShInstance> instances;
		GetInstances(instances, inBuildingPrimitive);
		// We work only with the first instance for now. Might do more in the future
		if(instances.GetElemCount())
		{
			I3DShTreeElement* buildingTree = instances[0]->GetTreeElement();
			if(!MCVerify(buildingTree))
				return NULL;

			TMCCountedPtr<I3DShTreeElement> sonTree;
			buildingTree->GetFirst(&sonTree);
			if(!MCVerify(sonTree))
			{	// No sons, something went wrong, forget the child index
				fChildIndex = kNoChild;
				return NULL;
			}

			int32 sonIndex = 0;

			while(sonIndex!=fChildIndex)
			{
				// Get the next son
				sonTree->GetRight(&sonTree);
				
				if(!MCVerify(sonTree))
				{	// No enough sons
					fChildIndex = kNoChild;
					return NULL;
				}
					
				sonIndex++;
			}

			TMCCountedPtr<I3DShInstance> childInstance;
			sonTree->QueryInterface(IID_I3DShInstance, (void**) &childInstance); 
			MY_ASSERT(childInstance);
			return childInstance;
		}
	}
*/
	return NULL;

}

MCCOMErr SubObject::Write(IShTokenStream* stream)
{
	MCCOMErr result=MC_S_OK;

	// Outline
	const int32 outCount = fHoleOutline.GetElemCount();
	for(int32 iPt=0 ; iPt<outCount ; iPt++)
	{
		result = fHoleOutline[iPt]->Write(stream);
		if (result) return result;
	}

/*	result=stream->PutKeyword('OutC');
	if (result) return result;
	result=stream->PutLong(outCount);
	if (result) return result;

	result=stream->PutKeyword('Outl');
	if (result) return result;

	// Point pos
	for(int32 iPt=0 ; iPt<outCount ; iPt++)
	{
		result = stream->PutPoint2D(fHoleOutline[iPt].GetPosition().x,fHoleOutline[iPt].GetPosition().y);
		if (result) return result;
	}
*/
	// Write the child instance ID
/*	result=stream->PutKeyword('ChIn');
	if (result) return result;
	result=stream->PutLong(fChildIndex);
	if (result) return result;
*/
	fChildTreePath.Write( stream->GetStream(), 'Chil' );

	// Placement
	stream->PutInt32Attribute('Plac', fPlacement);

	// Transformation
	result=stream->PutKeyword('POff');
	if (result) return result;
	result=stream->PutPoint3D(fOffset.x,fOffset.y,fOffset.z);
	if (result) return result;
	result=stream->PutKeyword('PSca');
	if (result) return result;
	result=stream->PutPoint3D(fScale.x,fScale.y,fScale.z);
	if (result) return result;
	result=stream->PutKeyword('PRot');
	if (result) return result;
	result=stream->PutPoint3D(fRotate.x,fRotate.y,fRotate.z);
	if (result) return result;

	// Common
	result=CommonBase::Write(stream);
	if (result) return result;

	return result;
}

MCCOMErr SubObject::Read(IShTokenStream* stream, const int32 keyword, BuildingPrim* building)
{ 
	MCCOMErr result=MC_S_OK;

	BuildingPrimData* data = &building->GetData();

	switch (keyword) 
	{
	case 'Poin':
		{
			TMCCountedPtr<OutlinePoint> newPoint;
			OutlinePoint::CreatePoint(&newPoint, data, TVector2::kZero, this);
			fHoleOutline.AddElem(newPoint);
			newPoint->Read(stream);
		} break;
	case 'OutC':
		{	// Not used anymore
			// Keep this for backward compatibility
			int32 count=0;
			fHoleOutline.SetElemCount( 0 );
			count = stream->GetInt32Token();
			for(int32 iPt=0 ; iPt<count ; iPt++)
			{
				TMCCountedPtr<OutlinePoint> newPoint;
				OutlinePoint::CreatePoint(&newPoint, data, TVector2::kZero, this);
				fHoleOutline.AddElem(newPoint);
			}
		} break;
	case 'Outl':
		{	// Not used anymore
			// Keep this for backward compatibility
			const int32 outCount = fHoleOutline.GetElemCount();
			for(int32 iPt=0 ; iPt<outCount ; iPt++)
			{
				TVector2 pos;
				result = stream->GetPoint2D(&pos.x,&pos.y);
				if (result) return result;
				fHoleOutline[iPt]->SetPosition(pos);
			}
		} break;
	case 'ChIn':
		{
			int32 childIndex = stream->GetInt32Token();
			if (result) return result;
			// Doesn't work OLD::ChildRecovery::AddSubObjectReference( this, childIndex );
		} break;
	case 'Chil':
		{
			fChildTreePath = TTreeIdPath::Read( stream->GetStream() );
			if (result) return result;
		} break;
	case 'Plac':
		{
			fPlacement = (EPlacementType)stream->GetInt32Token();
		} break;
	case 'POff':
		{
			result = stream->GetPoint3D(&fOffset.x, &fOffset.y, &fOffset.z);
			if (result) return result;
		} break;
	case 'PSca':
		{
			result = stream->GetPoint3D(&fScale.x, &fScale.y, &fScale.z);
			if (result) return result;
		} break;
	case 'PRot':
		{
			result = stream->GetPoint3D(&fRotate.x, &fRotate.y, &fRotate.z);
			if (result) return result;
		} break;
	default:
		CommonBase::Read(stream,keyword,data);
		break;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////
//
WallSubObject::WallSubObject(Wall* onWall)
{
	fOnLeftSide = true;

	fOnWall = onWall;

	if(fOnWall)
		fOnWall->AddObjectReference(this);
}

WallSubObject::~WallSubObject()
{
	SetSceneObject(NULL);
}

void WallSubObject::CreateWallSubObject(WallSubObject **newObject, Wall* onWall, BuildingPrim* building)
{
	TMCCountedCreateHelper<WallSubObject> result(newObject);

	result = new WallSubObject(onWall);
	ThrowIfNoMem(result);

	result->fBuilding = building;
}

void WallSubObject::GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K)
{
	fOnWall->GetBase(O,I,J,K);
}

real32 WallSubObject::GetThickness() const
{
	return fOnWall->GetThickness();
}

void SubObject::GetInstances( TMCCountedPtrArray<I3DShInstance>& instances )
{
	if(fBuilding)
		fBuilding->GetInstances(instances);
}

void WallSubObject::SetOnWall( Wall* onWall )
{
	if(fOnWall==onWall)
		return;

	if(onWall)
		onWall->AddObjectReference(this);
	if(fOnWall)
		fOnWall->RemoveObjectReference(this);

	fOnWall=onWall;

	Invalidate();
}

void WallSubObject::InvalidateTessellation()
{
	fOnWall->ClearFlag(eWallTessellated);
}

boolean WallSubObject::CheckObjectPosition()
{
	TVector2 possibleCenter = GetPolylineCenter();
	
	int32 tryCount=0;
	while( !SetPolylineCenter(possibleCenter,true) ) // true: check if there's enough space
	{ // Couldn't find enough space for the object: scale it ?
		tryCount++;
		if(tryCount>10)
		{
			// No more space for this object: delete it
			// Remove the reference in the wall

			TMCCountedPtr<Wall> onWall; 
			onWall = GetOnWall(); // get a counted pointer to be sure that the wall is still here
			
			if(onWall)
			{
				// Remove ref
				onWall->RemoveObjectReference(this);
			}

			return false;
		}
		else
		{
			// scale it
			const TVector2 scale(.9f,.9f);
			ScalePolyline(scale,false);
		}
	}

	// Possible optimisation: we could invalidate only if the polyline has been modified
	InvalidateTessellation();

	return true;
}

boolean WallSubObject::SetCenter( const TVector3& center, const boolean checkFirst )
{
	Validate();

	// Compute the 2D value of the polyline using this 3D center

	// A base attached to the wall
	TVector3 O,I,J,K;
	GetBase(O,I,J,K);

	// Project the point on the wall to get the new center
	const TVector2 center2D = ProjectIn(center,O,I,J);

	// Set the new center for the polyline
	return SetPolylineCenter(center2D,checkFirst);
}

boolean WallSubObject::OffsetPolyline(const TVector2& offset, const boolean checkFirst)
{
	if(checkFirst)
		return fOnWall->OffsetThisObject(this, offset);
	else // Simple offset
		return SubObject::OffsetPolyline(offset,checkFirst);
}

boolean WallSubObject::ScalePolyline(const TVector2& scale, const boolean checkFirst)
{
	if(checkFirst)
		return fOnWall->ScaleThisObject(this, scale);
	else // Simple scaling
		return SubObject::ScalePolyline(scale,checkFirst);
}

void WallSubObject::Validate()
{
	if(!Flag(eObjPosAreValid))
	{
		// A base attached to the wall
		TVector3 O;
		TVector3 I;
		TVector3 J;
		TVector3 K;
		GetBase(O,I,J,K);

		const real32 halfThickness = fOnWall->GetThickness()*.5;

		// Use the hole bbox to compute these values
		const TBBox2D& bbox = GetHoleBBox();
		const TVector2 min = bbox.fMin;
		const TVector2 max = bbox.fMax;

		const TVector3 pos0 = O +	min.x*I + min.y*J;
		const TVector3 pos1 = O +	max.x*I + min.y*J;
		const TVector3 rightThickOffset = .5*fOnWall->GetThickness()*K;
		const TVector3 leftThickOffset = -.5*fOnWall->GetThickness()*K;
		fOrigin = pos0+leftThickOffset;
		fWidth = rightThickOffset-leftThickOffset;
		fLength = pos1-pos0;
		fHeight = O +	max.x*I + max.y*J - pos1;
		fCenter = fOrigin+fHeight*.5+fWidth*.5+fLength*.5;

		Compute3DOutlines(fWidth);

		SetFlag(eObjPosAreValid);
	}

	UpdateInstanceTransform();
}

void WallSubObject::ComputeTreeTransform(TTreeTransform& treeTransform, 
										 const TBBox3D& originBBox)
{
	switch(fPlacement)
	{
	case eFitIn:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fRotate = TVector3::kZero;
		} break;
	case eSmaller:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fScale.z = .5f; // Coeff for the scaling in the wall normal direction
			fRotate = TVector3::kZero;
		} break;
	case eSlightlySmaller:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fScale.z = .8f; // Coeff for the scaling in the wall normal direction
			fRotate = TVector3::kZero;
		} break;
	case eSlightlyBigger:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fScale.z = 1.2f; // Coeff for the scaling in the wall normal direction
			fRotate = TVector3::kZero;
		} break;
	case eBigger:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fScale.z = 2.0f; // Coeff for the scaling in the wall normal direction
			fRotate = TVector3::kZero;
		} break;
	}

	// Outline dimensions
	const real32 outlineWidth = Get2DWidth(); // in the XY plane
	const real32 outlineHeight = Get2DHeight(); // The Z axis
	// 3rd dimension
	const real32 outlineThick = GetThickness(); // in the XY plane

	// Dimensions of the object in the scene
	const real32 originX = originBBox.GetWidth();
	const real32 originY = originBBox.GetHeight();
	const real32 originZ = originBBox.GetDepth();

	// By default, try to get the rotations from the most similar distances
	const real32 originMin = MC_Min(originX,originY);
	const real32 originMax = MC_Max(originX,originY);
	const real32 targetMin = MC_Min(outlineWidth,outlineThick);
	const real32 targetMax = MC_Max(outlineWidth,outlineThick);


	boolean pivot=false;
	if(Flag( eAutoFlipObj ))
	{
		if(originMin==originX)
		{
			if(targetMin==outlineThick) pivot=true;		
		}
		else if(originMin==originY)
		{
			if(targetMin==outlineWidth) pivot=true;
		}
	}

	// Apply the scaling to the dimensions
	const real32 width = outlineWidth*fScale.x; // in the XY plane
	const real32 height = outlineHeight*fScale.y; // The Z axis
	const real32 thick = outlineThick*fScale.z; // in the XY plane

	TVector3 XYZScaling(pivot?thick/originX:width/originX,
						pivot?width/originY:thick/originY,
						height/originZ);

	TVector3 bboxCenter;
	originBBox.GetCenter(bboxCenter);

	MY_ASSERT(fCenter.z>=fOnWall->GetLevel()->GetDistanceToGround());
	MY_ASSERT(fCenter.z<=fOnWall->GetLevel()->GetDistanceToGround()+
		fOnWall->GetLevel()->GetLevelHeight());
	
	// Ask the Wall where the object should be
	// Use directly fCenter instead of GetCenter(): this method is called after fCenter is updated
	TVector2 position2D;
	TVector2 leftPos2D;
	TVector2 rightPos2D;
	TVector2 normal2D;
	TVector2 direction2D;
	fOnWall->GetWallData(fCenter.CastToXY(),
						true,
						position2D,
						leftPos2D,
						rightPos2D,
						normal2D,
						direction2D );
	TVector3 onArcCenter(position2D.x, position2D.y, fCenter.z);

	// zRot is determine by the orientation of the wall
	real32 zRot=pivot?(real32)PI2:0;
	zRot+=DegToRad( GetPositiveAngle(-TVector2::kUnitY, normal2D) );
	zRot+=DegToRad( fRotate.y );

	const real32 cos = RealCos(zRot);
	const real32 sin = RealSin(zRot);

	const real32 offX = sin*fOffset.z + cos*fOffset.x;
	const real32 offY = -cos*fOffset.z + sin*fOffset.x;

	// Use directly fCenter instead of GetCenter(): this method is called after fCenter is updated

	const TVector3 offset(	pivot?onArcCenter.x+offY:onArcCenter.x+offX,
							pivot?onArcCenter.y+offX:onArcCenter.y+offY,
							onArcCenter.z+fOffset.y);

	treeTransform.SetOffset(offset);
	treeTransform.SetHotPoint(bboxCenter);
	treeTransform.SetXYZScaling(XYZScaling);
	//treeTransform.SetRotation//Use SetPhyThetaPsy instead
	treeTransform.SetPhyThetaPsy(zRot,pivot?DegToRad(fRotate.x):DegToRad(fRotate.z),pivot?DegToRad(fRotate.z):DegToRad(fRotate.x),true);	
}

void WallSubObject::Clone(WallSubObject** newObject, Wall* onWall, const ECloneChildrenMode cloneMode)
{
	WallSubObject::CreateWallSubObject(newObject, onWall, fBuilding);

	TMCCountedPtr<WallSubObject> objPtr;
	objPtr = *newObject;

	objPtr->SetOnLeftSide(fOnLeftSide);

	Level* level = NULL;
	if(fOnWall)
		level = fOnWall->GetLevel();
	else if(onWall)
		level = onWall->GetLevel();

	objPtr->Copy(this, level, cloneMode);
}

//////////////////////////////////////////////////////////

MCCOMErr WallSubObject::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('WObj');
	if (result) return result;

	// Orientation
	stream->PutInt32Attribute('Left', fOnLeftSide);

	SubObject::Write(stream);

	result=stream->PutEnd();

	return result;
}

MCCOMErr WallSubObject::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'Left':
			{
				int32 onLeft = stream->GetInt32Token();
				fOnLeftSide = onLeft?true:false;
			} break;
		default:
			SubObject::Read(stream,keyword,fBuilding);
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	Invalidate();

	return result;
}

//////////////////////////////////////////////////////////////////////////////
//
RoomSubObject::RoomSubObject(Room* inRoom)
{
	fInRoom = inRoom;

	if(fInRoom)
		fInRoom->AddObjectReference(this);
}

RoomSubObject::~RoomSubObject()
{
	SetSceneObject(NULL);
}

void RoomSubObject::CreateRoomSubObject(RoomSubObject **newObject, Room* inRoom, BuildingPrim* building)
{
	TMCCountedCreateHelper<RoomSubObject> result(newObject);

	result = new RoomSubObject(inRoom);
	ThrowIfNoMem(result);

	result->fBuilding = building;
}

real32 RoomSubObject::GetThickness() const 
{
	return fInRoom->GetFloorToCeiling();
}

void RoomSubObject::SetInRoom( Room* inRoom )
{
	if(fInRoom==inRoom)
		return;

	if(inRoom)
		inRoom->AddObjectReference(this);
	if(fInRoom)
		fInRoom->RemoveObjectReference(this);
	
	fInRoom=inRoom;
	
	Invalidate();
}

void RoomSubObject::GetBase(TVector3& O, TVector3& I, TVector3& J, TVector3& K)
{
	SubObject::GetBase(O,I,J,K);
	O.z = fInRoom->GetLevel()->GetDistanceToGround();
}

void RoomSubObject::Validate()
{
	if(!Flag(eObjPosAreValid))
	{
		// A base attached to the room
		TVector3 O,I,J,K;
		GetBase(O,I,J,K);

		const real32 floorThickness = fInRoom->GetFloorThickness();
		const real32 floorToCeiling = fInRoom->GetFloorToCeiling();

		// Use the hole bbox to compute these values
		const TBBox2D& bbox = GetHoleBBox();
		const TVector2 min = bbox.fMin;
		const TVector2 max = bbox.fMax;

		const TVector3 pos0 = O +	min.x*I + min.y*J;
		const TVector3 pos1 = O +	max.x*I + min.y*J;
		const TVector3 pos2 = O +	max.x*I + max.y*J;
		const TVector3 floorOffset = floorThickness*K;
		const TVector3 ceilingOffset =floorToCeiling*K;

		fOrigin = pos0+floorOffset;
		fWidth = pos2-pos1;
		fLength = pos1-pos0;
		fHeight = ceilingOffset;
		fCenter = fOrigin+fHeight*.5+fWidth*.5+fLength*.5;

		Compute3DOutlines(fHeight);

		SetFlag(eObjPosAreValid);
	}

	UpdateInstanceTransform();
}

void RoomSubObject::InvalidateTessellationOver()
{
	Level* over = fInRoom->GetLevel()->GetLevelOver();
	if(over)
	{
		Room* room = over->LevelPlan().PosInRoom(GetPolylineCenter());
		if(room)
		{
			room->ClearFlag(eRoomTessellated);
		}
	}
}

void RoomSubObject::InvalidateTessellation()
{
	fInRoom->ClearFlag(eWallTessellated);
	InvalidateTessellationOver();
}

boolean RoomSubObject::CheckObjectPosition()
{
	TVector2 possibleCenter = GetPolylineCenter();
	
	int32 tryCount=0;
	while( !SetPolylineCenter(possibleCenter,true) ) // true: check if there's enough space
	{ // Couldn't find enough space for the object: scale it ?
		tryCount++;
		if(tryCount>10)
		{
			// No more space for this object: delete it
			TMCCountedPtr<Room> inRoom;
			inRoom = GetInRoom(); // get a counted pointer to be sure that the wall is still here
			if(inRoom)
			{
				// Forget about this room (not necessary, but maybe cleaner)
				SetInRoom(NULL);
				// Remove ref
				inRoom->RemoveObjectReference(this);
			}

			return false;
		}
		else
		{
			// scale it
			const TVector2 scale(.9f,.9f);
			ScalePolyline(scale,false);
		}
	}

	// Possible optimisation: we could invalidate only if the polyline has been modified
	InvalidateTessellation();

	return true;
}

boolean RoomSubObject::SetCenter( const TVector3& center, const boolean checkFirst )
{
	Validate();

	// Compute the 2D value of the polyline using this 3D center

	// A base attached to the room
/*	TVector3 O,I,J,K;
	GetBase(O,I,J,K);*/

	// Project the point on the floor to get the new center
	const TVector2 center2D = center.CastToXY();//ProjectIn(center,O,I,J);

	// Set the new center for the polyline
	return SetPolylineCenter(center2D,checkFirst);
}

boolean RoomSubObject::OffsetPolyline(const TVector2& offset, const boolean checkFirst)
{
	if(checkFirst)
		return fInRoom->OffsetThisObject(this, offset);
	else // Simple offset
		return SubObject::OffsetPolyline(offset,checkFirst);
}

boolean RoomSubObject::ScalePolyline(const TVector2& scale, const boolean checkFirst)
{
	if(checkFirst)
		return fInRoom->ScaleThisObject(this, scale);
	else // Simple scaling
		return SubObject::ScalePolyline(scale,checkFirst);
}

void RoomSubObject::ComputeTreeTransform(TTreeTransform& treeTransform, 
										 const TBBox3D& originBBox)
{
	switch(fPlacement)
	{
	case eFitIn:
		{
			fOffset = TVector3::kZero;
			fScale = TVector3::kOnes;
			fRotate = TVector3::kZero;
		} break;
	case eSmaller:
		{
			fOffset = TVector3::kZero;
			fScale.SetValues(.8f,.8f,.8f);
			fRotate = TVector3::kZero;
		} break;
	case eSlightlySmaller:
		{
			fOffset = TVector3::kZero;
			fScale.SetValues(.95f,.95f,.95f);
			fRotate = TVector3::kZero;
		} break;
	case eSlightlyBigger:
		{
			fOffset = TVector3::kZero;
			fScale.SetValues(1.05f,1.05f,1.05f);
			fRotate = TVector3::kZero;
		} break;
	case eBigger:
		{
			fOffset = TVector3::kZero;
			fScale.SetValues(1.2f,1.2f,1.2f);
			fRotate = TVector3::kZero;
		} break;
	}

	// Outline dimensions
	const real32 outlineWidth = Get2DWidth(); // in the XY plane
	const real32 outlineHeight = Get2DHeight(); // The Z axis
	// 3rd dimension
	const real32 outlineThick = GetThickness(); // in the XY plane

	// Dimensions of the object in the scene
	const real32 originX = originBBox.GetWidth();
	const real32 originY = originBBox.GetHeight();
	const real32 originZ = originBBox.GetDepth();

	// By default, try to get the rotations from the most similar distances
	// beetween X and Y
	const real32 originMin = MC_Min(originX,originY);
	const real32 targetMin = MC_Min(outlineWidth,outlineHeight);

	boolean pivot=false;
	if(Flag( eAutoFlipObj ))
	{
		if(originMin==originX)
		{
			if(targetMin==outlineHeight) pivot=true;
		}
		else if(originMin==originY)
		{
			if(targetMin==outlineWidth) pivot=true;
		}
	}

	// Apply the scaling to the dimensions
	const real32 width = outlineWidth*fScale.x; // in the XY plane
	const real32 height = outlineHeight*fScale.y; // The Z axis
	const real32 thick = outlineThick*fScale.z; // in the XY plane

	TVector3 XYZScaling(pivot?height/originX:width/originX,
						pivot?width/originY:height/originY,
						thick/originZ);

	TVector3 bboxCenter;
	originBBox.GetCenter(bboxCenter);

	real32 zRot=pivot?(real32)PI2:0;
	zRot+=DegToRad( fRotate.z );

	const real32 cos = RealCos(zRot);
	const real32 sin = RealSin(zRot);

	const real32 offX = sin*fOffset.y + cos*fOffset.x;
	const real32 offY = -cos*fOffset.y + sin*fOffset.x;

	MY_ASSERT(fCenter.z>=fInRoom->GetLevel()->GetDistanceToGround());
	MY_ASSERT(fCenter.z<=fInRoom->GetLevel()->GetDistanceToGround()+
		fInRoom->GetLevel()->GetLevelHeight());
	
	// Use directly fCenter instead of GetCenter(): this method is called after fCenter is updated

	const TVector3 offset(	pivot?fCenter.x+offY:fCenter.x+offX,
							pivot?fCenter.y+offX:fCenter.y+offY,
							fCenter.z+fOffset.z);

	treeTransform.SetOffset(offset);
	treeTransform.SetHotPoint(bboxCenter);
	treeTransform.SetXYZScaling(XYZScaling);
	//treeTransform.SetRotation//SetPhyThetaPsy
	treeTransform.SetPhyThetaPsy(zRot,pivot?DegToRad(fRotate.y):DegToRad(fRotate.x),pivot?DegToRad(fRotate.x):DegToRad(fRotate.y),true);	
}

void RoomSubObject::Clone(RoomSubObject** newObject, Room* inRoom, const ECloneChildrenMode cloneMode)
{
	RoomSubObject::CreateRoomSubObject(newObject, inRoom, fBuilding);

	TMCCountedPtr<RoomSubObject> objPtr;
	objPtr = *newObject;

	Level* level = NULL;
	if(fInRoom)
		level = fInRoom->GetLevel();
	else if(inRoom)
		level = inRoom->GetLevel();

	objPtr->Copy(this, level, cloneMode);
}

//////////////////////////////////////////////////////////

MCCOMErr RoomSubObject::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('RObj');
	if (result) return result;

	SubObject::Write(stream);

	result=stream->PutEnd();

	return result;
}

MCCOMErr RoomSubObject::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		SubObject::Read(stream,keyword, fBuilding);

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	Invalidate();

	return result;
}


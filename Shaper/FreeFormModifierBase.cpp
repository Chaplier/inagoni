/****************************************************************************************************

		FreeFormModifierBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormModifierBase.h"

#include "Copyright.h"
#include "Shaper.h"
#include "IShUtilities.h"
#include "IShComponent.h"
#include "MFPartTypes.h"
#include "I3DEditorHostPartDefs.h"

#include "Common/SerialNumber.h"

const MCGUID CLSID_FreeFormModifierBase(R_CLSID_FreeFormModifierBase);

static const TMCColorRGB kHandleColor( 68/255.0f, 36/255.0f, 0 );
static const TMCColorRGB kSelHandleColor( 252/255.0f, 147/255.0f, 19/255.0f );
//static const TMCColorRGB kEdgeColor( 255/255.0, 223/255.0, 187/255.0 );
static const TMCColorRGB kEdgeColor( 33/255.0f, 16/255.0f, 0/255.0f );

size_t FreeFormModifierBase::ConvertIndex( const size_t ix, const size_t iy, const size_t iz ) const
{
	return iz + GetZCount()*iy + GetZCount()*GetYCount()*ix;
}

void FreeFormModifierBase::ConvertIndex( const size_t index, size_t& ix , size_t& iy, size_t& iz ) const
{
	iz = (index%GetZCount());
	iy = (index%(GetYCount()*GetZCount()))/GetZCount();
	ix = (index%(GetXCount()*GetYCount()*GetZCount()))/(GetYCount()*GetZCount());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////:

FreeFormModifierBase::FreeFormModifierBase()
{
	mIsValid = false;
	mIsRendering = false;

	mCurrentTool = kSelectToolID;
}

MCCOMErr FreeFormModifierBase::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormModifierBase))
	{
		TMCCountedGetHelper<FreeFormModifierBase> result(ppvObj);
		result = this;
		return MC_S_OK;
	}	
	if ( (TBasicWireframeSet::QueryInterface(riid, ppvObj) == MC_S_OK) && (*ppvObj) )
	{
		return MC_S_OK;
	}

	return TBasicModifier::QueryInterface(riid, ppvObj);
}

MCCOMErr FreeFormModifierBase::ExtensionDataChanged()
{
	mIsValid = false;

	return MC_S_OK;
}

MCCOMErr FreeFormModifierBase::Apply(I3DShTreeElement* tree)
{
	return MC_S_OK;
}

void FreeFormModifierBase::SetBoundingBox(const TBBox3D& bbox)
{
	BoundingBox() = bbox;
}

MCCOMErr FreeFormModifierBase::DeformPoint(const TVector3& point, TVector3& result)
{
//	mTensor.Apply( mG2L.TransformPoint( point ) );
	return MC_S_OK;
}

MCCOMErr FreeFormModifierBase::DeformBBox(const TBBox3D& in, TBBox3D& out)
{
	// Preprocess the data and check if there's a deformation
	if( !Preprocess() )
	{	// No deformation: return the same box
		out= in;
		return MC_S_OK;
	}

	out = mBBoxAfter;
	return MC_S_OK;
}

MCCOMErr FreeFormModifierBase::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh)
{
	// Preprocess the data and check if there's a deformation
	if( !Preprocess() )
	{	// No deformation: return the same mesh
		TMCCountedGetHelper<FacetMesh> result(outMesh);
		result= in;
		return MC_S_OK;
	}

	// Clone the orignal one
	in->Clone(outMesh);
	FacetMesh* outMeshPtr = *outMesh;

	// The vertices and there normals
	const int32 vtxCount = outMeshPtr->VerticesNbr();
	for (int32 iVtx=0; iVtx<vtxCount; iVtx++)
	{
		// Move in the (0,0,0)(1,1,1) space
		const TVector3 locPos = mG2L.TransformPoint( outMeshPtr->fVertices[iVtx] );
		const TVector3 locNor = mG2L.TransformVector( outMeshPtr->fNormals[iVtx] );
		// Apply the transformation
		const TVector3 transLocPos  = mTensor.Apply(locPos);
		TVector3 transLocNor =  mTensor.Apply(locPos+locNor)-transLocPos;
		transLocNor.Normalize();

		// Move back to the object space
		outMeshPtr->fVertices[iVtx] = mL2G.TransformPoint( transLocPos );
		outMeshPtr->fNormals[iVtx] = mL2G.TransformVector( transLocNor );
	}

	// Display the current selection data
	if( mPartExtension )
		mPartExtension->DisplayValue( GetSelectionPos() );

	return MC_S_OK;
}

boolean FreeFormModifierBase::Preprocess()
{
	if(!mIsValid)
	{
		// transform from 0-1 space to bbox space
		Compute3DTransform();
		
		// Compute the actual handle position
		ComputeHandlePosition();

		// Axis index for the selection
		PrepareAxis();

		mIsValid = true;
	}

	return true;
}

void FreeFormModifierBase::BeginRendering()
{
	mIsRendering = true;
}

void FreeFormModifierBase::EndRendering()
{
	mIsRendering = false;
}

// WARNING: when implementing this, other objects can't be selected.
boolean	 FreeFormModifierBase::HandlesTool(int16 inTool)
{
	mCurrentTool = inTool;
	switch( inTool )
	{
	case kSelectToolID:
	case kMoveToolID:
	case kRotateToolID:
	case kScaleToolID:
		return mSelectedHandles.size()>0; // Handle the tool only if handles are selected. Otherwise It's not possible for Carrara to select another object.

	}
	return false;
}

//bool FreeFormModifierBase::SelectedHandle(std::set<int32>& result, int handleIndex, int axisIndex) const 
//{ 
//	if( handleIndex>=0 )
//	{
//		result.insert( handleIndex );
//	}
//	else if( axisIndex>=0 )
//	{
//		result = mAxis[axisIndex].mHandles;
//	}
//	return (result.size()>0);
//}

TVector3 FreeFormModifierBase::GetSelectionPos()
{
	Preprocess();

	TVector3 pos(0,0,0);

	std::set<int32> handles;
	if( GetSelectedHandle(handles) )
	{
		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{ 
			size_t xIndex;
			size_t yIndex;
			size_t zIndex;
			ConvertIndex( *itr, xIndex, yIndex, zIndex );

			pos += mComputedPositionArray[xIndex][yIndex][zIndex];

			itr++;
		}
	}

	if( handles.size() )
		pos/=handles.size();

	return pos;
}

void FreeFormModifierBase::OffsetSelectionPos(const TVector3& offset)
{
	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();

	std::set<int32> handles;
	if( GetSelectedHandle(handles) )
	{
		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{ 
			size_t xIndex;
			size_t yIndex;
			size_t zIndex;
			ConvertIndex( *itr, xIndex, yIndex, zIndex );

			const TVector3& curPos = mComputedPositionArray[xIndex][yIndex][zIndex];
			const TVector3 newPos = curPos + offset;

			double initXPos = (double)xIndex/((double)xCount-1.0);
			double initYPos = (double)yIndex/((double)yCount-1.0);
			double initZPos = (double)zIndex/((double)zCount-1.0);

			// Initial pos in 0-1 space
			TVector3 localInitialPos( initXPos, initYPos, initZPos );
			// Initial pos in bbox space
			TVector3 initPos = mL2G.TransformPoint(localInitialPos);

			SetOffset( xIndex, yIndex, zIndex, newPos-initPos );

			itr++;
		}
	}
}

//void FreeFormModifierBase::SetSelectionPos(const TVector3& pos)
//{
//	size_t xIndex;
//	size_t yIndex;
//	size_t zIndex;
//	ConvertIndex( GetSelectedHandle(), xIndex, yIndex, zIndex );
//
//	const size_t xCount = GetXCount();
//	const size_t yCount = GetYCount();
//	const size_t zCount = GetZCount();
//
//	double initXPos = (double)xIndex/((double)xCount-1.0);
//	double initYPos = (double)yIndex/((double)yCount-1.0);
//	double initZPos = (double)zIndex/((double)zCount-1.0);
//
//	// Initial pos in 0-1 space
//	TVector3 localInitialPos( initXPos, initYPos, initZPos );
//	// Initial pos in bbox space
//	TVector3 initPos = mL2G.TransformPoint(localInitialPos);
//
//	SetOffset( xIndex, yIndex, zIndex, pos-initPos );
//}
size_t FreeFormModifierBase::FindAxisForSegment( size_t segmentIndex )
{
	const size_t axisCount = mAxis.size();
	for( size_t iAxis=0 ; iAxis<axisCount ; iAxis++ )
	{
		const std::set<int32>& seg = mAxis[iAxis].mSegments;
		std::set<int32>::const_iterator itr = seg.begin();
		while( itr!=seg.end() )
		{
			if( *itr==segmentIndex )
				return iAxis;

			itr++;
		}
	}

	return -1;
}

size_t FreeFormModifierBase::FindHandleForPoint( size_t listIndex, size_t pointIndex )
{
	std::map<HandleKey,size_t>::iterator itr = mHandleMap.find( HandleKey(listIndex, pointIndex) );
	if(itr!=mHandleMap.end())
		return itr->second;
	else
		return -1;
}


inline TVector2 RotatePoint(const TVector2& pt,const TVector2& cosSin,const TVector2& center)
{
	TVector2 result=pt;
	result-=center;
	TVector2 normal(-result.y,result.x);
	result = cosSin.x*result + cosSin.y*normal;
	result+=center;
	return result;
}

TVector2 GetRotationCosSin( const TVector2& initialPos, const TVector2& newPos, const TVector2& center2D, double constrainAngle=0 )
{
	TVector2 cosSin=TVector2::kZero;

	TVector2 vec1 = initialPos-center2D;
	vec1.Normalize();
	TVector2 vec2 = newPos-center2D;
	vec2.Normalize();

	cosSin.x = vec1 * vec2;
	cosSin.y = vec1 ^ vec2;
	if(constrainAngle>0)
	{
		// Constrain
		real32 angle = 0;
		RealArcSinCos(cosSin.y, cosSin.x, angle);
		if(angle>0)
			angle = (real32)constrainAngle*((int32)(angle/constrainAngle+.5));
		else
			angle = (real32)constrainAngle*((int32)(angle/constrainAngle-.5));
		RealSinCos(angle, cosSin.y, cosSin.x);
	}

	return cosSin;
}

void GetIndexes( const TVector3& vec1, const TVector3& vec2, int& index1, int& index2)
{
	if( vec1.x==vec2.x )
	{
		index1 = 1;
		index2 = 2;
	}
	else if( vec1.y==vec2.y )
	{
		index1 = 2;
		index2 = 0;
	}
	else
	{
		index1 = 0;
		index2 = 1;
	}
}

void SetNonZero( real32& value, double tolerance )
{
	if( abs(value)<tolerance )
	{
		if( value<0 )
			value = -tolerance;
		else
			value = tolerance;
	}
}

MCCOMErr FreeFormModifierBase::TrackWireFrame (I3DShWireFrameSet* wireFrame,int16 proj,I3DShTreeElement* tree,const TWFHitInfo& handle,
										   const TRACKINFO& startinfo,const TRACKINFO& previnfo,const TRACKINFO& nextinfo)
{
	switch( nextinfo.fStage )
	{
	case kShBeginTracking :
		{
			// Record the current selection
			std::set<int32> handlesToSelect; 
			if( handle.fElemType == kWFPoint )
			{
				handlesToSelect.insert( (int)FindHandleForPoint(handle.fListIndex, handle.fElemIndex) );
			}
			else if( handle.fElemType == kWFEdge )
			{
				handlesToSelect = mAxis[(int)FindAxisForSegment( handle.fElemIndex )].mHandles;
			}
			// Selection modifiers
			bool selectionChanged = false;
			if( nextinfo.fModifiers == kWFShift ||// Add to selection
				nextinfo.fModifiers == kWFOption ) 
			{	// Note: Shift is not avalaible in Modifier Mode => it deselect the object.
				// So I added the kWFOption (Alt) on the test
				selectionChanged = AddToSelection( handlesToSelect );
			}
			else if( nextinfo.fModifiers == kWFCommand ) // Ctrl key on windows: flip selection
			{
				selectionChanged = FlipSelection( handlesToSelect );
			}
			else
			{
				selectionChanged = SetToSelection( handlesToSelect );
			}
			if( selectionChanged )
			{
				PATCH_StoreExtraData();
				InvalidateComponent();
			}

			// Store the initial offsets
			std::set<int32> handles;
			if( GetSelectedHandle(handles) )
			{
				if( mInitialOffset.size() != handles.size() )
					mInitialOffset.resize( handles.size() );

				if( mInitialPosition.size() != handles.size() )
					mInitialPosition.resize( handles.size() );

				std::set<int32>::const_iterator itr = handles.begin();
				size_t iHandle = 0;
				while( itr!=handles.end() )
				{ 
					size_t xIndex;
					size_t yIndex;
					size_t zIndex;
					ConvertIndex( *itr, xIndex, yIndex, zIndex );

					mInitialOffset[iHandle] = GetOffset( xIndex, yIndex, zIndex );
					mInitialPosition[iHandle] = mComputedPositionArray[xIndex][yIndex][zIndex];

					itr++;
					iHandle++;
				}
			}
			// Rotation and scaling center
			mTransformCenter = GetSelectionPos();
		} break;

	case kShContinueTracking:
		{
			std::set<int32> handles;
			if( GetSelectedHandle(handles) )
			{
				// Prepare 
				TVector3 offset;
				TVector3 scaling;
				int index1=0, index2=1;
				TVector2 cosSin;
				switch( mCurrentTool )
				{
				case kRotateToolID:
					{
						// Find the rotation plane
						TVector3 vec1 = startinfo.fLoc;
						TVector3 vec2 = nextinfo.fLoc;
						if( proj==3 && // 3D view: the ctrl key is used to move vertically
							nextinfo.fModifiers == kWFCommand ) // Ctrl key on windows 
						{
							vec1 = startinfo.fOrthoLoc;
							vec2 = nextinfo.fOrthoLoc;
						}
						GetIndexes( vec1, vec2, index1, index2);
						TVector2 initialPos(vec1[index1], vec1[index2]);
						TVector2 newPos(vec2[index1], vec2[index2]);
						TVector2 center2D(mTransformCenter[index1],mTransformCenter[index2]);
						cosSin = GetRotationCosSin( initialPos, newPos, center2D, 0 );
					} break;
				case kScaleToolID:
					{
						// Compute the X,Y,Z scaling factors
						TVector3 vec1 = nextinfo.fLoc - mTransformCenter;
						TVector3 vec2 = startinfo.fLoc - mTransformCenter;
						if( proj==3 && // 3D view: the ctrl key is used to move vertically
							nextinfo.fModifiers == kWFCommand ) // Ctrl key on windows 
						{
							vec1 = nextinfo.fOrthoLoc - mTransformCenter;
							vec2 = startinfo.fOrthoLoc - mTransformCenter;
						}
						SetNonZero( vec2.x, 0.01 );
						SetNonZero( vec2.y, 0.01 );
						SetNonZero( vec2.z, 0.01 );
						scaling.x = vec1.x/vec2.x;
						scaling.y = vec1.y/vec2.y;
						scaling.z = vec1.z/vec2.z;
					} break;
				default: // Move
					{			
						offset = (nextinfo.fLoc-startinfo.fLoc) ;
						if( proj==3 && // 3D view: the ctrl key is used to move vertically
							nextinfo.fModifiers == kWFCommand ) // Ctrl key on windows 
						{
							offset = (nextinfo.fOrthoLoc-startinfo.fOrthoLoc) ;
						}

						if( nextinfo.fModifiers == kWFShift )
						{	// shift key: keep only the biggest value to constrain the handle to an axis
							if( abs(offset.x)>abs(offset.y) )	offset.y=0;
							else								offset.x=0;

							if( abs(offset.x)>abs(offset.z) )	offset.z=0;
							else								offset.x=0;

							if( abs(offset.y)>abs(offset.z) )	offset.z=0;
							else								offset.y=0;
						}
					} break;
				}

				// Apply on the handles
				std::set<int32>::const_iterator itr = handles.begin();
				size_t iHandle = 0;
				while( itr!=handles.end() )
				{ 
					size_t xIndex;
					size_t yIndex;
					size_t zIndex;
					ConvertIndex( *itr, xIndex, yIndex, zIndex );

					switch( mCurrentTool )
					{
					case kRotateToolID:
						{
							const TVector3& initPos = mInitialPosition[iHandle];
							TVector2 initPos2D(initPos[index1], initPos[index2]);
							TVector2 center2D(mTransformCenter[index1],mTransformCenter[index2]);
							TVector2 result2D = RotatePoint(initPos2D, cosSin, center2D);
							TVector3 newPos = initPos; 
							newPos[index1] = result2D.x;
							newPos[index2] = result2D.y;
							SetOffset( xIndex, yIndex, zIndex, mInitialOffset[iHandle] + newPos - initPos);
						} break;
					case kScaleToolID:
						{
							TVector3 vec1 = mInitialPosition[iHandle] - mTransformCenter;
							TVector3 newPos = mTransformCenter; 
							newPos.x += scaling.x * vec1.x;
							newPos.y += scaling.y * vec1.y;
							newPos.z += scaling.z * vec1.z;
							SetOffset( xIndex, yIndex, zIndex, mInitialOffset[iHandle] + newPos - mInitialPosition[iHandle]);
						} break;
					default: // Move
						{			
							SetOffset( xIndex, yIndex, zIndex, mInitialOffset[iHandle] + offset);
						} break;
					}

					// No need to modify the mesh, the invalidation will recompute everything

					itr++;
					iHandle++;
				}
			}
		} break;
	case kShFinishTracking:
		{
			mInitialPosition.clear();
			mInitialOffset.clear();
		} break;
	}

	// Display the current selection data
	//if( mPartExtension )
	//	mPartExtension->DisplayValue( GetSelectionPos() );

	return MC_S_OK;
}

void FreeFormModifierBase::InvalidateComponent()
{
	// Invalidate the component: everything will be computed again
	TMCCountedPtr<IShMinimalParameterComponent> minComp;
	QueryInterface(IID_IShMinimalParameterComponent,(void**)&minComp);
	if(minComp)
	{
		minComp->Inval( true, true );
		mIsValid = false;
	}
}

HandleAxis& FreeFormModifierBase::XAxis( size_t iY, size_t iZ )
{
	const size_t yCount = GetYCount();

	return mAxis[iZ*yCount + iY];
}

HandleAxis& FreeFormModifierBase::YAxis( size_t iX, size_t iZ )
{
	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();

	size_t offset = yCount*zCount;
	return mAxis[offset + iZ*xCount + iX];
}

HandleAxis& FreeFormModifierBase::ZAxis( size_t iX, size_t iY )
{
	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();

	size_t offset = yCount*zCount + xCount*zCount;
	return mAxis[offset + iY*xCount + iX];
}

void FreeFormModifierBase::PrepareAxis()
{
	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();

	// Clean up cache data
	mAxis.clear();
	mAxis.resize( yCount*zCount + xCount*zCount + xCount*yCount );

	int segmentIndex = 0;

	for( size_t iX=0 ; iX<xCount ; iX++)
	{
		const std::vector< std::vector<TVector3> >& xPos = mComputedPositionArray[iX];
		for( size_t iY=0 ; iY<yCount ; iY++)
		{
			const std::vector<TVector3>& yPos = xPos[iY];
			const size_t zCount = yPos.size();
			for( size_t iZ=0 ; iZ<zCount ; iZ++)
			{
				const TVector3& handle = yPos[iZ];

				const int index0 = (int)ConvertIndex(iX,iY,iZ);
				if( iX+1<xCount )
				{
					const int index1 = (int)ConvertIndex(iX+1,iY,iZ);
					// Store axis data
					HandleAxis& axis = XAxis(iY,iZ);
					axis.mHandles.insert( index0 );
					axis.mHandles.insert( index1 );
					axis.mSegments.insert( segmentIndex++ );
				}
				if( iY+1<yCount )
				{
					const int index1 = (int)ConvertIndex(iX,iY+1,iZ);
					// Store axis data
					HandleAxis& axis = YAxis(iX,iZ);
					axis.mHandles.insert( index0 );
					axis.mHandles.insert( index1 );
					axis.mSegments.insert( segmentIndex++ );
				}
				if( iZ+1<zCount )
				{
					const int index1 = (int)ConvertIndex(iX,iY,iZ+1);
					// Store axis data
					HandleAxis& axis = ZAxis(iX,iY);
					axis.mHandles.insert( index0 );
					axis.mHandles.insert( index1 );
					axis.mSegments.insert( segmentIndex++ );
				}
			}
		}
	}
}

MCCOMErr FreeFormModifierBase::DataToWireFrame(I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree)
{
	// Create the handles
	const size_t xCount = mComputedPositionArray.size();
	if( !xCount )
		return MC_S_FALSE; // nothing to draw

	const size_t yCount = mComputedPositionArray[0].size();
	if( !yCount )
		return MC_S_FALSE; // nothing to draw

	const size_t zCount = mComputedPositionArray[0][0].size();
	if( !zCount )
		return MC_S_FALSE; // nothing to draw

	const size_t totalSegmentCount =(xCount-1)*yCount*zCount +
									(yCount-1)*zCount*xCount +
									(zCount-1)*xCount*yCount;

	mHandleMap.clear();
									
	// TWFPointList, TWFConnectionList
	// 1 list per handle type
	// 1 list per color
	// 1st point list: regular display
	// 2nd point list: selected points
	wireFrame->AllocateLists(2, 1);

	TWFPointList& handlePoints = wireFrame->GetPointList(0);
	handlePoints.Clear();
	handlePoints.PreAllocatePoints((int32)(xCount*yCount*zCount)); // this do a SetElemSpace
	handlePoints.SetHandleType(kWFRoundHandle);
	handlePoints.SetDrawWBProjs(true);
	handlePoints.SetIsExcludedFromZBuffer(true);
	wireFrame->SetColor(kWFPointList, 0, kHandleColor );

	TWFPointList& selectedHandlePoints = wireFrame->GetPointList(1);
	selectedHandlePoints.Clear();
	selectedHandlePoints.SetHandleType(kWFRoundHandle);
	selectedHandlePoints.SetDrawWBProjs(true);
	selectedHandlePoints.SetIsExcludedFromZBuffer(true);
	selectedHandlePoints.SetIsSelected(true);
	wireFrame->SetColor(kWFPointList, 1, kSelHandleColor );

	TWFConnectionList& cubeSegments = wireFrame->GetConnectionList(0);
	cubeSegments.Clear();
	cubeSegments.PreAllocatePoints((int32)(xCount*yCount*zCount)); // this do a SetElemSpace
//	cubeSegments.PreAllocateConnections((int)totalSegmentCount); // this do a SetElemCount
	cubeSegments.SetIsSelectable( true );
	cubeSegments.SetDrawWBProjs(true);
	wireFrame->SetColor(kWFConnectionList, 0, kEdgeColor );

	std::set<int32> selectedHandles;
	GetSelectedHandle(selectedHandles);

	for( size_t iX=0 ; iX<xCount ; iX++)
	{
		const std::vector< std::vector<TVector3> >& xPos = mComputedPositionArray[iX];
		for( size_t iY=0 ; iY<yCount ; iY++)
		{
			const std::vector<TVector3>& yPos = xPos[iY];
			const size_t zCount = yPos.size();
			for( size_t iZ=0 ; iZ<zCount ; iZ++)
			{
				const TVector3& handle = yPos[iZ];
				const int index0 = (int)ConvertIndex(iX,iY,iZ);

				// Store the list index and the point index in the handle map
				if( selectedHandles.find(index0)!=selectedHandles.end() )
				{
					mHandleMap[HandleKey(1,selectedHandlePoints.GetPointsCount())] = index0;
					selectedHandlePoints.AddPoint( handle );
				}
				else
				{
					mHandleMap[HandleKey(0,handlePoints.GetPointsCount())] = index0;
					handlePoints.AddPoint( handle );
				}

				cubeSegments.AddPoint( handle );

				if( iX+1<xCount )
				{
					const int index1 = (int)ConvertIndex(iX+1,iY,iZ);
					cubeSegments.AddConnection( TWFConnection( index0, index1 ) );
				}
				if( iY+1<yCount )
				{
					const int index1 = (int)ConvertIndex(iX,iY+1,iZ);
					cubeSegments.AddConnection( TWFConnection( index0, index1 ) );
				}
				if( iZ+1<zCount )
				{
					const int index1 = (int)ConvertIndex(iX,iY,iZ+1);
					cubeSegments.AddConnection( TWFConnection( index0, index1 ) );
				}
			}
		}
	}

	// Check that the wireframe is valid
	if( wireFrame->HasNoEmptyList() )
		return MC_S_OK;
	else 
		return MC_S_FALSE;
}

MCCOMErr FreeFormModifierBase::WireFrameToData (I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree)
{
	return MC_S_OK;
}

MCCOMErr FreeFormModifierBase::GetWireFrameBBox(TBBox3D* outBBox, int16 proj, I3DShTreeElement* tree)
{
//	*outBBox = TBBox3D(TVector3(-kStdWidth,-kStdWidth,-kStdWidth), TVector3(kStdWidth,kStdWidth,kStdWidth));

	return MC_S_OK;
}

void FreeFormModifierBase::CopyComponentExtraDataBase (FreeFormModifierBase* destModifier)
{
	destModifier->mPartExtension = mPartExtension; // Need to copy this data otherwise it's forgotten during the DeformFacetMesh method
	
	// When the data is edited from the PartExtension, the ExtraData is lost somewhere in the process.
	// (Carrara missed a call ?). Because of that, the selection and the bbox are lost.
	// A workaround is to save the data somehere else.
	destModifier->mSelectedHandles = mSelectedHandles;

//	destModifier->mBoundingBox = mBoundingBox; // Need to copy this data otherwise it's forgotten during the DeformFacetMesh method
}

#include "ComMessages.h"
#include "MFPartMessages.h"
#include "IMFParameterPart.h"
#include "IMFResponder.h"
MCCOMErr FreeFormModifierBase::HandleEvent(int32 message, IMFResponder* source, void* data)
{
	switch (message)
	{
	case kMsg_CUIP_ComponentAttached:
		{	// Component attached: init the component pointer
			if (source)
			{
				TMCCountedPtr<IMFPart> sourcePart;
				source->QueryInterface(IID_IMFPart,(void**)&sourcePart);
				if( sourcePart )
				{
					TMCCountedPtr<IMFPart> childPart;
					sourcePart->FindChildPartByID( &childPart, 'ffpn' );
					if( childPart )
						childPart->QueryInterface( CLSID_FreeFormPartExt, (void**)&mPartExtension );
				}
			}

		} break;
	//case EMFPartMessage::kMsg_PartValueChanged:
	//	{
	//		if (MCVerify(source))
	//		{
	//			TMCCountedPtr<IMFPart> part;
	//			source->QueryInterface(IID_IMFPart, (void **)&part);
	//			if (part)
	//			{
	//				const IDType partID= part->GetIMFPartID();

	//				switch (partID)
	//				{
	//				case 'xPos':
	//				case 'yPos':
	//				case 'zPos':
	//					{
	//						real32 value=0;
	//						part->GetValue( &value,kReal32ValueType );
	//						const TVector3 prevPos = GetSelectionPos( );
	//						TVector3 newPos = prevPos;
	//						if( partID=='xPos') newPos.x = value;
	//						if( partID=='yPos') newPos.y = value;
	//						if( partID=='zPos') newPos.z = value;
	//						OffsetSelectionPos( newPos-prevPos );
	//					} break;
	//				default: break;
	//				}
	//			}
	//		}
	//	}
	}

	return MC_S_OK;
}


void FreeFormModifierBase::Compute3DTransform()
{
	TVector3 center;
	TVector3 delta;
	BoundingBox().GetCenter(center);
	BoundingBox().GetDelta(delta);

	// Translation
	TVector3 scaledCenter(	center.x/delta.x,
							center.y/delta.y,
							center.z/delta.z);
	mG2L.fTranslation = TVector3( .5,.5,.5 ) - scaledCenter;
	// Scaling
	mG2L.fRotationAndScale = TMatrix33::kIdentity;
	mG2L.fRotationAndScale[0][0]=1/delta.x;
	mG2L.fRotationAndScale[1][1]=1/delta.y;
	mG2L.fRotationAndScale[2][2]=1/delta.z;

	mL2G = mG2L.GetInverse();
}

void FreeFormModifierBase::ComputeHandlePosition()
{
	mBBoxAfter = TBBox3D(TVector3::kOnes, -TVector3::kOnes); //set as invalid bbox
	// position relative to the (0,0,0),(1,1,1) space for the tensor
	std::vector< std::vector< std::vector<TVector3> > > localControlPoints;

	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();


	mComputedPositionArray.resize( xCount );
	localControlPoints.resize( xCount );
	for( size_t iX=0 ; iX<xCount ; iX++)
	{
		double initXPos = (double)iX/((double)xCount-1.0);

		std::vector< std::vector<TVector3> >& curXPlane = mComputedPositionArray[iX];
		curXPlane.resize( yCount );
		localControlPoints[iX].resize( yCount );
		for( size_t iY=0 ; iY<yCount ; iY++)
		{
			double initYPos = (double)iY/((double)yCount-1.0);
	
			std::vector<TVector3>& curYPlane = curXPlane[iY];
			curYPlane.resize( zCount );
			localControlPoints[iX][iY].resize( zCount );
			for( size_t iZ=0 ; iZ<zCount ; iZ++)
			{
				double initZPos = (double)iZ/((double)zCount-1.0);
	
				// Initial pos in 0-1 space
				TVector3 localInitialPos( initXPos, initYPos, initZPos );
				// Initial pos in bbox space
				TVector3 initPos = mL2G.TransformPoint(localInitialPos);

				TVector3 offsetedPos = initPos + GetOffset(iX,iY,iZ);

				if(!SerialNumber::IsSerialValid() && iZ>0)
				{   // Demo version: only move the bottom plane
					offsetedPos = initPos;
				}

				curYPlane[iZ] = offsetedPos;

				localControlPoints[iX][iY][iZ] = mG2L.TransformPoint( offsetedPos  );
				mBBoxAfter.AddPoint( offsetedPos );
			}
		}
	}

	mTensor.Init( localControlPoints );
}

#pragma once

#include "Basic3DCOMImplementations.h"
#include "BasicDataComponent.h"
#include "MCBasicTypes.h"
#include "I3dExAnimation.h"
#include "AnimUtilities.h"

#include "BasicModifiers.h"   
#include "BasicWireFrameSet.h"
#include "MCPtr.h"
#include "IShTokenStream.h"

#include "BernsteinTensor.h"

#include <vector>
#include <set>
#include <map>
#include <utility>
#include "Vector3.h"

#include "FreeFormPartExt.h"

extern const MCGUID CLSID_FreeFormModifierBase;

struct HandleAxis
{
	std::set<int32> mHandles;
	std::set<int32> mSegments;
};

class FreeFormModifierBase :	public TBasicWireframeSet, 
								public TBasicDeformModifier
{
public:
	FreeFormModifierBase();
	virtual ~FreeFormModifierBase() {}

	virtual MCCOMErr 	MCCOMAPI QueryInterface		(const MCIID &riid, void** ppvObj);
	virtual uint32		MCCOMAPI AddRef				() { return TBasicModifier::AddRef(); }
	STANDARD_RELEASE;

	
  	// I3DExDataExchanger methods :
	virtual MCCOMErr		MCCOMAPI ExtensionDataChanged();
	virtual MCCOMErr		MCCOMAPI HandleEvent		(int32 message,
															 IMFResponder* source,
															 void* data);
	virtual MCCOMErr		MCCOMAPI InitComponent()							
	{
		return kNotImplementedError; 
	}

	// I3DExDeformer methods :
	virtual MCCOMErr		MCCOMAPI Apply(I3DShTreeElement* tree);
	virtual void			MCCOMAPI SetBoundingBox		(const TBBox3D& bbox);
	virtual MCCOMErr		MCCOMAPI DeformPoint		(const TVector3& point, TVector3& result);
	virtual MCCOMErr		MCCOMAPI DeformFacetMesh	(real lod,FacetMesh* in, FacetMesh** outMesh);
	virtual MCCOMErr		MCCOMAPI DeformBBox			(const TBBox3D& in, TBBox3D& out);
	virtual void			MCCOMAPI BeginRendering();
	virtual void			MCCOMAPI EndRendering();
	virtual boolean			MCCOMAPI ModifierChanged	() { return true; }
	virtual boolean			MCCOMAPI HandlesTool		(int16 inTool);


	// I3DExWireFrameSet functions
	virtual MCCOMErr		MCCOMAPI TrackWireFrame (I3DShWireFrameSet* wireFrame,int16 proj,I3DShTreeElement* tree,const TWFHitInfo& handle,const TRACKINFO& startinfo,const TRACKINFO& previnfo,const TRACKINFO& nextinfo);
	virtual MCCOMErr		MCCOMAPI DataToWireFrame	(I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree);
	virtual MCCOMErr		MCCOMAPI WireFrameToData	(I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree);
	virtual MCCOMErr		MCCOMAPI GetWireFrameBBox	(TBBox3D* outBBox, int16	proj, I3DShTreeElement*	tree);

	void SetPart( FreeFormPartExt* part ){ mPartExtension=part; }

	TVector3 GetSelectionPos();
	void OffsetSelectionPos(const TVector3& pos);

	void PATCH_RetoreExtraData( const std::set<int32>& selectedHandles )
	{
		mSelectedHandles = selectedHandles;
	}

protected:

	virtual boolean Preprocess();

	void Compute3DTransform();
	void ComputeHandlePosition();
	void PrepareAxis();

	void InvalidateComponent();

	void CopyComponentExtraDataBase (FreeFormModifierBase* destModifier);

	// Data access
	// Handle offset position in the object BBox space
	// X planes, then Y planes then Z planes
	virtual size_t GetXCount() const = 0;
	virtual size_t GetYCount() const = 0;
	virtual size_t GetZCount() const = 0;
	virtual const TVector3& GetOffset( size_t x, size_t y, size_t z ) const = 0;
	virtual void SetOffset( size_t x, size_t y, size_t z , const TVector3& value ) = 0;
	
	//int32 GetSelectedHandleIndex() const {return mSelectedHandle;}
	//int32 GetSelectedAxisIndex() const {return mSelectedAxis;}
	//void SetSelectedHandleIndex(int32 index) {mSelectedHandle=index;}
	//void SetSelectedAxisIndex(int32 index) { mSelectedAxis=index; }
	bool GetSelectedHandle(std::set<int32>& result) const { result = mSelectedHandles; return mSelectedHandles.size()>0; } //SelectedHandle( result, mSelectedHandle, mSelectedAxis ); }

	virtual TBBox3D& BoundingBox() = 0;
		
	bool AreHandlesSelected(const std::set<int32>& handles) const 
	{
		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{
			if( !IsHandleSelected( *itr ) )
				return false;

			itr++;
		}
		return true;
	}
	bool IsHandleSelected(int32 handle) const { return mSelectedHandles.find(handle)!=mSelectedHandles.end(); }
	bool SelectHandle(int32 handle) { return mSelectedHandles.insert(handle).second; } // return true if inserted, false if the handle was alreday selected
	bool DeselectHandle(int32 handle) { return mSelectedHandles.erase(handle)>0; } // return true if removed, false otherwise

	// Returns true if selection changed
	bool FlipSelection( const std::set<int32>& handles )
	{
		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{
			if( !IsHandleSelected( *itr ) )
				SelectHandle( *itr );
			else
				DeselectHandle( *itr );

			itr++;
		}

		return handles.size()>0;
	}

	bool AddToSelection( const std::set<int32>& handles )
	{
		bool selectionChanged = false;

		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{
			selectionChanged |= SelectHandle( *itr );
			itr++;
		}

		return selectionChanged;
	}

	bool SetToSelection( const std::set<int32>& handles )
	{
		if( handles.size()>0 && AreHandlesSelected(handles) )
			return false;

		mSelectedHandles.clear();

		std::set<int32>::const_iterator itr = handles.begin();
		while( itr!=handles.end() )
		{
			SelectHandle( *itr );
			itr++;
		}

		return true;
	}

	size_t ConvertIndex( const size_t ix, const size_t iy, const size_t iz ) const;
	void ConvertIndex( const size_t index, size_t& ix , size_t& iy, size_t& iz ) const;
	bool SelectedHandle(std::set<int32>& result, int handleIndex, int axisIndex) const ;
	size_t FindAxisForSegment( size_t segmentIndex );

	void PATCH_StoreExtraData()
	{
		if( mPartExtension )
		{
			mPartExtension->PATCH_StoreExtraData( mSelectedHandles );
		}
	}

	// Selected handles
	//int32 mSelectedHandle;
	//int32 mSelectedAxis;
	//std::vector< std::vector< std::vector<bool> > > mSelectionArray;
	std::set<int32> mSelectedHandles;

	// Cache
	// Computed position: the original position + the offset
	std::vector< std::vector< std::vector<TVector3> > > mComputedPositionArray;

	// Handles index map
	typedef std::pair<size_t,size_t> HandleKey; // Key: list index + point index
	std::map<HandleKey,size_t> mHandleMap;
	size_t FindHandleForPoint( size_t listIndex, size_t pointIndex );

	// Axis for multi selection
	// First the X axis, then the Y axis then the Z axis.
	// The yCount*zCount first ones are the X axis.
	// The xCount*zCount following ones are the Y axis.
	// The xCount*yCount last ones are the Z axis.
	std::vector<HandleAxis> mAxis;
	HandleAxis& XAxis( size_t iY, size_t iZ );
	HandleAxis& YAxis( size_t iX, size_t iZ );
	HandleAxis& ZAxis( size_t iX, size_t iY );

	// Tool to convert a point in (0,0,0)(1,1,1) bbox space 
	BernsteinTensor mTensor;

	TBBox3D mBBoxAfter; // BBox after the transformation
	TTransform3D mL2G; // convert from the (0,0,0)(1,1,1) bbox space to the object bbox space
	TTransform3D mG2L; // inverse transform

	// Tracking
	std::vector<TVector3> mInitialOffset;
	std::vector<TVector3> mInitialPosition;
	TVector3 mTransformCenter;

	// Other
	boolean mIsValid;
	boolean mIsRendering;

	int16 mCurrentTool;

	// IHM
	TMCCountedPtr<FreeFormPartExt> mPartExtension;
};
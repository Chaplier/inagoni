#pragma once

#include "FreeFormModifierBase.h"

extern const MCGUID CLSID_FreeFormModifierNxN;

struct FreeFormModifierPMap
{
	FreeFormModifierPMap();
	void Clear();

	int32 mXCount;
	int32 mYCount;
	int32 mZCount;

	TBBox3D mBoundingBox; // bbox to be transformed (object bbox)
};

class FreeFormModifierNxN :	public FreeFormModifierBase, 
							// Doesn't work public I3DExAnimated, // for a dynamic number of parameters with animation
							public IExStreamIO
{
public:
	FreeFormModifierNxN();
	virtual ~FreeFormModifierNxN() {}

	virtual MCCOMErr 	MCCOMAPI QueryInterface		(const MCIID &riid, void** ppvObj);
	virtual uint32		MCCOMAPI AddRef				() { return FreeFormModifierBase::AddRef(); }
	STANDARD_RELEASE;

	
  	// I3DExDataExchanger methods :
	virtual int16			MCCOMAPI GetResID();
	virtual void*			MCCOMAPI GetExtensionDataBuffer(){return &mPMap;}
	virtual int32			MCCOMAPI GetParamsBufferSize() const {return sizeof(FreeFormModifierPMap);}
	virtual void			MCCOMAPI Clone				(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr		MCCOMAPI CopyComponentExtraData (IExDataExchanger* dest);

	// IExStreamIO methods
	virtual MCCOMErr		MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr		MCCOMAPI Write(IShTokenStream* stream);
	virtual MCCOMErr		MCCOMAPI FinishRead		(IStreamContext* streamContext){return MC_S_OK;}

protected:

	virtual boolean Preprocess();

	void SetPlanes( size_t newXPlaneCount, size_t newYPlaneCount, size_t newZPlaneCount );

	// Data
	FreeFormModifierPMap		 mPMap;
	// Handle offset position in the object BBox space
	// X planes, then Y planes then Z planes
	std::vector< std::vector< std::vector<TVector3> > > mOffsetArray;

	virtual size_t GetXCount() const { return mPMap.mXCount; }
	virtual size_t GetYCount() const { return mPMap.mYCount; }
	virtual size_t GetZCount() const { return mPMap.mZCount; }
	virtual const TVector3& GetOffset( size_t x, size_t y, size_t z ) const { return mOffsetArray[x][y][z]; }
	virtual void SetOffset( size_t x, size_t y, size_t z , const TVector3& value ) { mOffsetArray[x][y][z]=value; InvalidateComponent(); }
	
	virtual TBBox3D& BoundingBox() { return mPMap.mBoundingBox; }
};
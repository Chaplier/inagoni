#pragma once

#include "FreeFormModifierBase.h"


extern const MCGUID CLSID_FreeFormModifier4x4;

struct FreeFormModifier4x4PMap
{
	FreeFormModifier4x4PMap();
	void Clear();

	TVector3 mOffset[64];

	TBBox3D mBoundingBox; // bbox to be transformed (object bbox)
};

class FreeFormModifier4x4 :	public FreeFormModifierBase
{
public:
	FreeFormModifier4x4();
	virtual ~FreeFormModifier4x4() {}

	virtual MCCOMErr 	MCCOMAPI QueryInterface		(const MCIID &riid, void** ppvObj);
	
  	// I3DExDataExchanger methods :
	virtual int16			MCCOMAPI GetResID();
	virtual void*			MCCOMAPI GetExtensionDataBuffer(){return &mPMap;}
	virtual int32			MCCOMAPI GetParamsBufferSize() const {return sizeof(FreeFormModifier4x4PMap);}
	virtual void			MCCOMAPI Clone				(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr		MCCOMAPI CopyComponentExtraData (IExDataExchanger* dest);

protected:

	// Data
	FreeFormModifier4x4PMap		 mPMap;

	virtual size_t GetXCount() const { return 4; }
	virtual size_t GetYCount() const { return 4; }
	virtual size_t GetZCount() const { return 4; }

	virtual const TVector3& GetOffset( size_t x, size_t y, size_t z ) const;
	virtual void SetOffset( size_t x, size_t y, size_t z , const TVector3& value );
	
	virtual TBBox3D& BoundingBox() { return mPMap.mBoundingBox; }
};
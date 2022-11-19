#pragma once

#include "FreeFormModifierBase.h"


extern const MCGUID CLSID_FreeFormModifier3x3;

struct FreeFormModifier3x3PMap
{
	FreeFormModifier3x3PMap();
	void Clear();

	TVector3 mOffset[27];

	TBBox3D mBoundingBox; // bbox to be transformed (object bbox)
};

class FreeFormModifier3x3 :	public FreeFormModifierBase
{
public:
	FreeFormModifier3x3();
	virtual ~FreeFormModifier3x3() {}

	virtual MCCOMErr 	MCCOMAPI QueryInterface		(const MCIID &riid, void** ppvObj);
	
  	// I3DExDataExchanger methods :
	virtual int16			MCCOMAPI GetResID();
	virtual void*			MCCOMAPI GetExtensionDataBuffer(){return &mPMap;}
	virtual int32			MCCOMAPI GetParamsBufferSize() const {return sizeof(FreeFormModifier3x3PMap);}
	virtual void			MCCOMAPI Clone				(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr		MCCOMAPI CopyComponentExtraData (IExDataExchanger* dest);

protected:

	// Data
	FreeFormModifier3x3PMap		 mPMap;

	virtual size_t GetXCount() const { return 3; }
	virtual size_t GetYCount() const { return 3; }
	virtual size_t GetZCount() const { return 3; }

	virtual const TVector3& GetOffset( size_t x, size_t y, size_t z ) const;
	virtual void SetOffset( size_t x, size_t y, size_t z , const TVector3& value );

	virtual TBBox3D& BoundingBox() { return mPMap.mBoundingBox; }
};
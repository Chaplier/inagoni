#pragma once

#include "FreeFormModifierBase.h"


extern const MCGUID CLSID_FreeFormModifier2x2;

struct FreeFormModifier2x2PMap
{
	FreeFormModifier2x2PMap();
	void Clear();

	TVector3 mOffset[8];

	TBBox3D mBoundingBox; // bbox to be transformed (object bbox)
};

class FreeFormModifier2x2 :	public FreeFormModifierBase
{
public:
	FreeFormModifier2x2();
	virtual ~FreeFormModifier2x2() {}

	virtual MCCOMErr 	MCCOMAPI QueryInterface		(const MCIID &riid, void** ppvObj);
	
  	// I3DExDataExchanger methods :
	virtual int16			MCCOMAPI GetResID();
	virtual void*			MCCOMAPI GetExtensionDataBuffer(){return &mPMap;}
	virtual int32			MCCOMAPI GetParamsBufferSize() const {return sizeof(FreeFormModifier2x2PMap);}
	virtual void			MCCOMAPI Clone				(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr		MCCOMAPI CopyComponentExtraData (IExDataExchanger* dest);

protected:

	// Data
	FreeFormModifier2x2PMap		 mPMap;

	virtual size_t GetXCount() const { return 2; }
	virtual size_t GetYCount() const { return 2; }
	virtual size_t GetZCount() const { return 2; }

	virtual const TVector3& GetOffset( size_t x, size_t y, size_t z ) const;
	virtual void SetOffset( size_t x, size_t y, size_t z , const TVector3& value );

	virtual TBBox3D& BoundingBox() { return mPMap.mBoundingBox; }
};
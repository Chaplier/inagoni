/****************************************************************************************************

		FacetGenerator.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	25/5/2006

****************************************************************************************************/

#ifndef __FacetGenerator__
#define __FacetGenerator__

#include "MCBasicTypes.h"
#include "RenderTypes.h"
#include "MCArray.h"
#include "TBBox.h"
#include "I3DShInstance.h"

struct Hitdata
{
	TVector3 mHitPostion;
	real32 mHitDistance;
	TVector3 mHitNormal;
	boolean mHitIn; // in or out
};

class FacetGenerator
{
public:
	FacetGenerator();

	void		Initialize(I3DShInstance* instance);
	void		SetSmoothing(real32 smoothPercent){mSmoothPercent=smoothPercent;ComputeSmoothingDistance();}

	boolean		RecordHits( const TVector3& origin, const TVector3& direction );
	real32		Density( const TVector3& pos ) const;

protected:

	boolean		PickFacetMesh(const Ray3D& ray3D);

	void		ComputeBBox();
	TVector3	FitToBBox( const TVector3& pos ) const;
	real32		Smooth(real32 value) const ;
	void		ComputeSmoothingDistance();

	TMCCountedPtr<I3DShInstance> mInstanceWithMesh;
	TBBox3D mMeshBBox;
	real32	mMaxLenght; // the diagonal of the bbox
	TVector3 mBBoxOffset;
	TVector3 mBBoxScale;

	Ray3D				mRay3D;
	TMCArray<Hitdata>	mHitData;
	real32				mSmoothPercent; // in percent
	real32				mSmoothDistance; // in distance
	real32				mSquaredDistance;

};

#endif

/****************************************************************************************************

		FacetGenerator.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	25/5/2006

****************************************************************************************************/

#include "FacetGenerator.h"

#include "I3DShScene.h"
#include "I3DShObject.h"
#include "I3dShFacetMesh.h"
#include "Copyright.h"

#include "Utils.h"

FacetGenerator::FacetGenerator()
{
	mSmoothDistance = 0;
	mSmoothPercent = 0;
	mSquaredDistance = 0;
	mMaxLenght = 0;
}

void FacetGenerator::Initialize(I3DShInstance* instance)
{
	mInstanceWithMesh = instance;

	ComputeBBox();

	ComputeSmoothingDistance();
}

void FacetGenerator::ComputeSmoothingDistance()
{
	if(mInstanceWithMesh)
	{
		const real32 maxDim = MC_Max( mMeshBBox.GetWidth(), mMeshBBox.GetHeight(), mMeshBBox.GetDepth() );

		mSmoothDistance = mSmoothPercent*maxDim*.5;

		mSquaredDistance = mSmoothDistance*mSmoothDistance;
	}
}

void FacetGenerator::ComputeBBox()
{
	if(!mInstanceWithMesh)
		return;

	mMeshBBox.fMin.x = mMeshBBox.fMin.y = mMeshBBox.fMin.z = 1e30f;
	mMeshBBox.fMax.x = mMeshBBox.fMax.y = mMeshBBox.fMax.z = -1e30f;

	mInstanceWithMesh->GetBoundingBox(mMeshBBox, kApplyAllDeformersAtInstanceLevel );

	mMeshBBox.GetCenter(mBBoxOffset);
	mBBoxScale = mMeshBBox.fMax - mBBoxOffset;

	TVector3 delta;
	mMeshBBox.GetDelta(delta);
	mMaxLenght = delta.GetMagnitude();
}

boolean FacetGenerator::RecordHits( const TVector3& origin, const TVector3& direction )
{
	// Prepare the ray
	mRay3D.fOrigin = FitToBBox(origin);
	TVector3 target = origin;
	target += direction;
	mRay3D.fDirection = FitToBBox(target) - mRay3D.fOrigin;
	mRay3D.fDirection.Normalize();
	

	mHitData.SetElemCount(0);

	// Hit the facet mesh
	if( PickFacetMesh(mRay3D) )
	{
		// Reorganize the hits using their distance
		const int32 hitCount = mHitData.GetElemCount();

		// Basic algo: the result should already be organized
/* Should already be organized
		for(int32 elem = 0 ; elem<hitCount ; elem++)
		{
			real32 minAlpha = mHitData[elem].mHitDistance;
			int32 minIndex = elem;
			for(int32 rest = elem+1 ; rest<hitCount ; rest++)
			{
				if(mHitData[rest].mHitDistance<minAlpha)
				{
					minAlpha = mHitData[rest].mHitDistance;
					minIndex = rest;
				}
			}

			if(minIndex != elem)
			{	// we found a smaller value, permut them
				TVector3 tmpPos = mHitData[minIndex].mHitPosition;
				real32 tmpAlpha = mHitData[minIndex].mHitDistance;

				mHitData[minIndex].mHitPosition = mHitData[elem].mHitPosition;
				mHitData[minIndex].mHitDistance = mHitData[elem].mHitDistance;

				mHitData[elem].mHitPosition = tmpPos;
				mHitData[elem].mHitDistance = tmpAlpha;
			}
		}
		*/

		return true;
	}

	return false;
}

TVector3 FacetGenerator::FitToBBox( const TVector3& pos ) const
{
	return pos%mBBoxScale + mBBoxOffset;
}

real32 FacetGenerator::Smooth(real32 dist) const
{
	if( dist>=mSmoothDistance )
		return 1;
	else
	{
		float r2 = mSmoothDistance-dist;
		r2 *= r2;
		return DensityBlender(r2, mSquaredDistance);
	}
}

TVector3 Project(	const TVector3& point, //point
				const TVector3& O, // point on the plane
				const TVector3& normal) // normal to the plane
{
	return point + ( normal*(O-point) )*normal;
}

real32 FacetGenerator::Density( const TVector3& pos ) const
{
	TVector3 fitPos = FitToBBox(pos);

	const TVector3 line = fitPos - mRay3D.fOrigin;

	const real32 distance = line.GetMagnitude();

	const int32 hitCount = mHitData.GetElemCount();

	for(int32 elem = 0 ; elem<hitCount ; elem++)
	{
		const Hitdata& nextHitData = mHitData[elem];
		if(distance<nextHitData.mHitDistance)
		{
			if(elem&0x0000001)
			{	// Odd number of hits: we're inside the mesh

				if(mSmoothDistance == 0)
					return 1;

				// We need to figure out how deep inside the mesh we are.
				// The distance along the ray is not a suficient information: we can be very near from the side.
				// By projecting the point on the in and out planes, we've got a better idea. But it doesn't work
				// with concave shapes: when we're deep inside, the distance is sometime very small and we think
				// we're near the side.
				// So we send a new ray in this direction to check how far from the side we are.

				const Hitdata& prevHitData = mHitData[elem-1];

				// throw a short ray in this direction
				// Prepare the ray
				Ray3D theRay;
				theRay.fOrigin = fitPos;
				theRay.fDirection = prevHitData.mHitNormal;
				real32 prevDist = mSmoothDistance;
				// The result
				RayHit3D theHit;
				theHit.ft = 0;
				theHit.fInstance = mInstanceWithMesh;
		//		theHit.fFacetMesh = mInstanceWithMesh->GetRenderingFacetMesh();
				RayHitParameters rayHitParam(kRealEpsilon,mSmoothDistance,&theHit,&theRay);
		//		rayHitParam.hit->fInstance = mInstanceWithMesh; // for terrain shader, they crash otherwise 
				if( mInstanceWithMesh->RayHit( rayHitParam) )
				{
					prevDist = theHit.ft;
				}
				theRay.fDirection = nextHitData.mHitNormal;
				real32 nextDist = mSmoothDistance;
				if( mInstanceWithMesh->RayHit( rayHitParam) )
				{
					nextDist = theHit.ft;
				}

				return Smooth(MC_Min(prevDist, nextDist));

			}
			else // Even number of hits: we're outside the mesh
				return 0;
		}
	}

	return 0;
}

boolean FacetGenerator::PickFacetMesh(const Ray3D& ray3D)
{
	// The ray
	Ray3D theRay;
	theRay.fOrigin = ray3D.fOrigin;
	theRay.fDirection = ray3D.fDirection;

	// The result
	RayHit3D theHit;
	theHit.fFacetIndex = -1;
	theHit.fBaryCoord[0]=theHit.fBaryCoord[1]=theHit.fBaryCoord[2]=0;
	theHit.fNormalLoc = TVector3::kZero;
	theHit.fInstance = mInstanceWithMesh;
//	theHit.fFacetMesh = mInstanceWithMesh->GetRenderingFacetMesh();

	RayHitParameters rayHitParam(kRealMinusOne,mMaxLenght+kRealOne,&theHit,&theRay);

	int32 hitCount = 0;
	// RayHit several times, until we're out of the box 
	while( mInstanceWithMesh->RayHit( rayHitParam) && hitCount<128 ) // hitCount<128: just a security
	{
		// Get the hit result

		// Compute the normal to the hit

		// I'm not sure theHit.fNormalLoc is always good, so fill it ourself.
		// Some primitive, like the cone, don't give any other data than the 
		// pos and the ft => give a default fake direction
		TVector3 normal = theHit.fNormalLoc;
#if (VERSIONNUMBER < 0x050000)
		TMCCountedPtr<FacetMesh> facetmesh;
		mInstanceWithMesh->GetFMesh(0, &facetmesh);
#else // From Carrara 5
		FacetMesh* facetmesh = mInstanceWithMesh->GetRenderingFacetMesh();
#endif
		bool hasNormal = false;
		if(facetmesh)
		{
			if( theHit.fFacetIndex>-1 &&
				theHit.fFacetIndex < (int32)facetmesh->fFacets.GetElemCount())
			{
				if( theHit.fBaryCoord[0]!=0 ||
					theHit.fBaryCoord[1]!=0 ||
					theHit.fBaryCoord[2]!=0 )
				{
					const Triangle& tgl = facetmesh->fFacets[theHit.fFacetIndex];
			
					normal =	theHit.fBaryCoord[0]*facetmesh->fNormals[tgl.pt1] + 
								theHit.fBaryCoord[1]*facetmesh->fNormals[tgl.pt2] + 
								theHit.fBaryCoord[2]*facetmesh->fNormals[tgl.pt3];

					hasNormal = true;
				}
			}
			
		}

		if(!hasNormal)
		{	
			if(theHit.fNormalLoc.GetSquaredNorm()<.5)
			{
				// a primitive that doesn't compute its normal: make a fake one
				normal = theHit.fPointLoc - mBBoxOffset;
				normal.Normalize();
			}
		}

		Hitdata hitData;
		hitData.mHitPostion		= ray3D.fOrigin + theHit.ft * ray3D.fDirection;
		hitData.mHitDistance	= theHit.ft;
		hitData.mHitNormal		= normal;
		hitData.mHitIn			= (hitCount&0x0000001);

		mHitData.AddElem(hitData);

		// Prepare the next ray hit
		rayHitParam.tmin = theHit.ft+kRealEpsilon;

		hitCount++;
	}
	return (mHitData.GetElemCount()>0);
}


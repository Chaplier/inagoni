/****************************************************************************************************

		UV2XYZ.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/1/2004

****************************************************************************************************/

#ifndef __UV2XYZ__
#define __UV2XYZ__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCAssert.h"
#include "Vector2.h"
#include "Vector3.h"
#include "PublicUtilities.h"

struct I3DShPrimitive;

// Cache some data
struct FacetData
{
	TFacet3D	fFacet;
	TVector2	fUV1; // = fFacet.fVertices[1].fUV - fFacet.fVertices[0].fUV;
	TVector2	fUV2; // = fFacet.fVertices[2].fUV - fFacet.fVertices[0].fUV;
	real64		fInvDet; // = 1.0f / (UV1 ^  UV2); or 0 if (UV1 ^  UV2)==0
};

class FacetArray : public TMCArray<FacetData>
{
};

class UV2XYZ
{
public:
	UV2XYZ(I3DShPrimitive* primitive);

	boolean GetPointAndNormal(const TVector2& uv,const UVSpaceInfo&	uvSpaceInfo,
							TVector3& thePos3D, const boolean needPoint,
							TVector3& normal, const boolean needNormal,
							TVector3& isoU, TVector3& isoV, const boolean needIsoUV);
protected:

	void InitializeGrid(const int32 facetCount);
	void RegisterfacetMesh(const FacetMesh* mesh, const TMCClassArray<UVSpaceInfo>& domainInfos);

	void RegisterFacet(
			  const TVector3& p0, const TVector3& p1, const TVector3& p2, 
			  const TVector3& n0, const TVector3& n1, const TVector3& n2, 
			  const TVector2& uv0, const TVector2& uv1, const TVector2& uv2, 
			  const int32 domainID);

	inline int32 Index(const real32 val) const ;

	TMCArray<FacetArray>	fFacetArrays; // a grid of facet for a faster search
	int32					fSideSegments;
//	TMCArray<FacetData>		fFacetArray;

	boolean					fCacheValid;
	FacetData				fCachedFacet;
	int32					fCachedIndex; // to look arround the previous facet
};

inline int32 UV2XYZ::Index(const real32 val) const
{
	const int32 index = (int32)(fSideSegments*val)%fSideSegments;
	if(index<0) return 0;
	else if(index>=fSideSegments) return fSideSegments-1;
	else return index;
}

#endif

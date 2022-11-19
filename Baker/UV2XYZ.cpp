/****************************************************************************************************

		UV2XYZ.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/1/2004

****************************************************************************************************/

#include "UV2XYZ.h"

#include "copyright.h"
#include "Utils.h"
#include "I3DShObject.h"
#include "MCArray.h"
#include "I3dShFacetMesh.h"
#include "PublicUtilities.h"
#include "ShaderTypes.h"
#include "IShComponent.h"
#include "I3DExVertexPrimitive.h"
#include "IPolymesh.h"

void GetIsoUV(TVector3& theIsoU, TVector3& theIsoV, const TFacet3D& aFacet)
{
	{
		const int32 indexA = 1;
		const int32 indexB = 2;
		const int32 indexC = 0;

		const TVector2& uv0 = aFacet.fVertices[indexA].fUV;
		const TVector2& uv1 = aFacet.fVertices[indexB].fUV;
		const TVector2& uv2 = aFacet.fVertices[indexC].fUV;
	
		TVector2 dlam, dmu;

		TVector2 AB = uv1 - uv0;
		TVector2 AC = uv2 - uv0;

		real det = AB ^ AC;

		if (det == kRealZero)
		{
			theIsoU[0] = kRealOne;
			theIsoU[1] = kRealZero;
			theIsoU[2] = kRealZero;

			theIsoV[0] = kRealZero;
			theIsoV[1] = kRealOne;
			theIsoV[2] = kRealZero;
		}
		else
		{
			TVector3 AB2, AC2;

			dlam[0] = DivClip(AC[1], det);
			dlam[1] = DivClip(-AC[0], det);
			dmu[0] = DivClip(-AB[1], det);
			dmu[1] = DivClip(AB[0], det);

			const TVector3& vtx0 = aFacet.fVertices[indexA].fVertex;
			const TVector3& vtx1 = aFacet.fVertices[indexB].fVertex;
			const TVector3& vtx2 = aFacet.fVertices[indexC].fVertex;

			AB2 = vtx1 - vtx0;
			AC2 = vtx2 - vtx0;

			theIsoU = AB2 * dlam[0] + AC2 * dmu[0];
			theIsoV = AB2 * dlam[1] + AC2 * dmu[1];
		}
	}
/*	TVector3	seg1, seg2;
	TVector2	UV1, UV2;
	real		det;

	seg1 = aFacet.fVertices[1].fVertex - aFacet.fVertices[0].fVertex;
	seg2 = aFacet.fVertices[2].fVertex - aFacet.fVertices[0].fVertex;

	UV1[0] = aFacet.fVertices[1].fUV[0] - aFacet.fVertices[0].fUV[0];
	UV1[1] = aFacet.fVertices[1].fUV[1] - aFacet.fVertices[0].fUV[1];
	UV2[0] = aFacet.fVertices[2].fUV[0] - aFacet.fVertices[0].fUV[0];
	UV2[1] = aFacet.fVertices[2].fUV[1] - aFacet.fVertices[0].fUV[1];

	det = UV1[0]*UV2[1] - UV1[1]*UV2[0];

	if (det != 0.0)
	{
		UV1[0] /= det;
		UV1[1] /= det;
		UV2[0] /= det;
		UV2[1] /= det;

		theIsoU[0] = seg1[0]*UV2[1] - seg2[0]*UV1[1];
		theIsoU[1] = seg1[1]*UV2[1] - seg2[1]*UV1[1];
		theIsoU[2] = seg1[2]*UV2[1] - seg2[2]*UV1[1];
		theIsoV[0] = seg2[0]*UV1[0] - seg1[0]*UV2[0];
		theIsoV[1] = seg2[1]*UV1[0] - seg1[1]*UV2[0];
		theIsoV[2] = seg2[2]*UV1[0] - seg1[2]*UV2[0];
	}
	else
	{
		theIsoU[0] = 1.0;
		theIsoU[1] = 0.0;
		theIsoU[2] = 0.0;
		theIsoV[0] = 0.0;
		theIsoV[1] = 1.0;
		theIsoV[2] = 0.0;
	}*/
}

static boolean FacetUV2XYZ(const FacetData& facetData, const TVector2& uv, const UVSpaceInfo& uvSpaceInfo, 
						   TVector3& thePos3D, const boolean needPoint,
						   TVector3& normal, const boolean needNormal,
						   TVector3& isoU, TVector3& isoV, const boolean needIsoUV)
{
	if(facetData.fInvDet==0) return false;

	const TFacet3D& aF = facetData.fFacet;
	if(aF.fUVSpace != uvSpaceInfo.fID) return false;

	const TVector2 UV0 = uv - aF.fVertices[0].fUV;
	const TVector2& UV1 = facetData.fUV1;
	const TVector2& UV2 = facetData.fUV2;

	const real64 detx = (UV0 ^ UV2);
	const real64 dety = (UV1 ^ UV0);

	const real64 beta = detx*facetData.fInvDet;
//	if(beta < kRealZero) return false;
	if(beta < -kRealEpsilon) return false;

	const real64 gamma = dety*facetData.fInvDet;
//	if(gamma < kRealZero) return false;
	if(gamma < -kRealEpsilon) return false;

	const real64 alpha = kRealOne - beta - gamma;
//	if(alpha < kRealZero) return false;
	if(alpha < -kRealEpsilon) return false;

	if(needPoint)
		thePos3D = aF.fVertices[0].fVertex * alpha + aF.fVertices[1].fVertex * beta + aF.fVertices[2].fVertex * gamma;
	if(needNormal)
		normal = aF.fVertices[0].fNormal * alpha + aF.fVertices[1].fNormal * beta + aF.fVertices[2].fNormal * gamma;
	if(needIsoUV)
		GetIsoUV(isoU,isoV,facetData.fFacet);
	return true;
/* or use:
	if( PointIsInTriangle( uv, 
						aF.fVertices[0].fUV, 
						aF.fVertices[1].fUV,
						aF.fVertices[2].fUV ))
	{
	// get point and normal
		return true;
	}
	else
		return false;
		*/
}


UV2XYZ::UV2XYZ(I3DShPrimitive* primitive)
{
	MCAssert(primitive);

	TMCCountedPtr<FacetMesh> amesh;
	primitive->GetFMesh( 0.0, &amesh );

	InitializeGrid(amesh->FacetsNbr());

	//Check if we're working with a vertex primitive
	TMCCountedPtr<I3DShExternalPrimitive> externalPrimitive;
	primitive->QueryInterface(IID_I3DShExternalPrimitive, (void**) &externalPrimitive);

	TMCCountedPtr<I3DExVertexPrimitive> vertexPrimitive;
	if (externalPrimitive)
	{
		TMCCountedPtr<IShComponent> component;		
		externalPrimitive->GetPrimitiveComponent(&component);
		if (component)
			component->QueryInterface(IID_I3DExVertexPrimitive, (void**) &vertexPrimitive);				
	}

#if (VERSIONNUMBER < 0x050000) // Before Carrara 5
	if (vertexPrimitive)
	{	// Vertex primitive oresent a special case: the projection mapping (and so the wrapping)
		// is set per Polymesh. So we have to record them separetly to get the right UV wrapping info
		// (the global one always return false)
		const int32 polymeshCount= vertexPrimitive->GetNbPolymeshes();	
		for(int32 iPolymesh=0; iPolymesh<polymeshCount; iPolymesh++)
		{
			TMCCountedPtr<IPolymesh> polymesh;
			vertexPrimitive->GetPolymesh(&polymesh, iPolymesh);

			if(polymesh)
			{
				TMCCountedPtr<FacetMesh> facetMesh;
				polymesh->GetPolyMeshFacetMesh( &facetMesh );
		
				TMCClassArray<UVSpaceInfo> domainInfos;
				domainInfos.SetElemCount(1);
				polymesh->GetUVSpace(0, &domainInfos[0]);

				RegisterfacetMesh(amesh, domainInfos);
			}
		}

	}
	else
#endif
	{
		TMCCountedPtr<I3DShObject> object;
		primitive->QueryInterface(IID_I3DShObject, (void**) &object);
		TMCClassArray<UVSpaceInfo> domainInfos;
		if(MCVerify(object))	
		{
			const int32 domainCount = object->GetUVSpaceCount();
			for(int32 iDomain=0 ; iDomain<domainCount ; iDomain++)
			{
				domainInfos.AddElem();
				object->GetUVSpace(iDomain,&domainInfos[iDomain]);
			}
		}

		RegisterfacetMesh(amesh, domainInfos);
	}

	fCacheValid = false;
/*	TMCCountedPtr<FacetMesh> amesh;
	primitive->GetFMesh( 0.0, &amesh );

	TMCCountedPtr<I3DShObject> object;
	primitive->QueryInterface(IID_I3DShObject, (void**) &object);
	TMCClassArray<UVSpaceInfo> domainInfos;
	if(MCVerify(object))	
	{
		const int32 domainCount = object->GetUVSpaceCount();
		for(int32 iDomain=0 ; iDomain<domainCount ; iDomain++)
		{
			domainInfos.AddElem();
			object->GetUVSpace(iDomain,&domainInfos[iDomain]);
		}
	}

	Initialize(amesh, domainInfos);

	fCacheValid = false;*/
}

boolean UV2XYZ::GetPointAndNormal(	const TVector2& uv, const UVSpaceInfo& uvSpaceInfo,
									TVector3& thePos3D, const boolean needPoint,
									TVector3& normal, const boolean needNormal,
									TVector3& isoU, TVector3& isoV, const boolean needIsoUV)
{
	// Try the cached facet first, there's some chance will find the UV in
//	const int32 tglCount = fFacetArray.GetElemCount();

	if(fCacheValid)
	{
		if (FacetUV2XYZ(fCachedFacet, uv, uvSpaceInfo, 
						thePos3D, needPoint,
						normal, needNormal,
						isoU, isoV, needIsoUV) )
		{
			return true;
		}
/*		// Try the facet before
		if(fCachedIndex>0)
		{
			if (FacetUV2XYZ(fFacetArray[fCachedIndex-1], uv, uvSpaceID, 
							thePos3D, needPoint,
							normal, needNormal))
			{
				fCachedIndex--;
				fCachedFacet = fFacetArray[fCachedIndex];
				return true;
			}
		}
		// Try the facet after
		if(fCachedIndex<tglCount-1)
		{
			if (FacetUV2XYZ(fFacetArray[fCachedIndex+1], uv, uvSpaceID, 
							thePos3D, needPoint,
							normal, needNormal))
			{
				fCachedIndex++;
				fCachedFacet = fFacetArray[fCachedIndex];
				return true;
			}
		}*/
	}

	const int32 colIndex = (int32)Index(uv.x);
	const int32 rowIndex = (int32)Index(uv.y);

	const FacetArray& facetArray = fFacetArrays[colIndex + fSideSegments*rowIndex];

	const int32 tglCount = facetArray.GetElemCount();

	for(int32 iTgl=0 ; iTgl<tglCount ; iTgl++)
	{
		const FacetData& curFacet = facetArray[iTgl];
		if (FacetUV2XYZ(curFacet, uv, uvSpaceInfo, 
						thePos3D, needPoint,
						normal, needNormal,
						isoU, isoV, needIsoUV) )
		{
			// Keep the facet in the cache, there's some chance it will be the one needed 
			// at the next call
			fCacheValid = true;
			fCachedFacet = curFacet;
			fCachedIndex = iTgl;
			return true;
		}
	}

	return false;
}

struct ThreeUVs
{
	TVector2 uv0;
	TVector2 uv1;
	TVector2 uv2;
};

void UV2XYZ::InitializeGrid(const int32 facetCount)
{
	// Build a grid in order to have about 100 facet per box
	const real32 averageFacetPerBox = 100;
	fSideSegments = 1+(int32)(RealSqrt(facetCount/averageFacetPerBox));

	// the grid has fSideSegments by fSideSegments elements
	const int32 boxCount = fSideSegments*fSideSegments;
	fFacetArrays.SetElemCount(boxCount);
	
	// Allocate an approximate amount of memory space
	for(int32 iBox=0 ; iBox<boxCount ; iBox++)
	{
		fFacetArrays[iBox].SetGrowSize(averageFacetPerBox);
		fFacetArrays[iBox].SetValid(true);// Why do I have to do that ?
	}
}

void UV2XYZ::RegisterfacetMesh(const FacetMesh* mesh, const TMCClassArray<UVSpaceInfo>& domainInfos)
{
	// Build a TFacet3D array, so we won't have to rebuild all these info
	// each time we parse the mesh

	const TMCArray<Triangle>& triangles = mesh->fFacets;
	const TMCArray<TVector3>& vertices = mesh->fVertices;
	const TMCArray<TVector3>& normals  = mesh->fNormals;
	const TMCArray<TVector2>& uv = mesh->fuv;
	const TMCArray<uint32>& domains = mesh->fUVSpaceID;

	const boolean hasNormals	= vertices.GetElemCount() == normals.GetElemCount();
	const boolean hasUVs		= vertices.GetElemCount() == uv.GetElemCount();
	const boolean hasDomains	= (domains.GetElemCount()>0);
	const boolean hasDomainInfo	= (domainInfos.GetElemCount()>1);

	const int32 tglCount = triangles.GetElemCount();

	// Get the facet and put them in the grid. One facet
	// can be in several boxes
	for(int32 iTgl=0 ; iTgl<tglCount ; iTgl++)
	{
		const Triangle& curTgl = triangles[iTgl];
		const uint32 i1 = curTgl.pt1;
		const uint32 i2 = curTgl.pt2;
		const uint32 i3 = curTgl.pt3;

		const int32 domainID = (hasDomains?domains[iTgl]:0);

		const TVector3& p0 = vertices[i1];
		const TVector3& p1 = vertices[i2];
		const TVector3& p2 = vertices[i3];

		TVector3 calcNormal = TVector3::kUnitZ;
		if(!hasNormals)
		{
			calcNormal= (p1 - p0)^(p2 - p0);
		}

		const int32 domainInfoIndex = hasDomainInfo?domainID:0;
		const boolean wrapU = domainInfos[domainInfoIndex].fWraparound[0];
		const boolean wrapV = domainInfos[domainInfoIndex].fWraparound[1];

		// If there's U or V wrapping, and the facet overlap a side, then we register
		// 2,3 or 4 facets instead of 1 (so the search algo after won't have to be modifyed)
		if (hasUVs)
		{
			TVector2 uv0 = uv[i1]; InZeroOne(uv0);
			TVector2 uv1 = uv[i2]; InZeroOne(uv1);
			TVector2 uv2 = uv[i3]; InZeroOne(uv2);

			TMCClassArray<ThreeUVs> uvTgls;
			ThreeUVs& firstTgl = uvTgls.AddElem();
			firstTgl.uv0 = uv0; firstTgl.uv1 = uv1; firstTgl.uv2 = uv2;

			if( wrapU && 
				(MC_Max(uv0.x,uv1.x,uv2.x)-MC_Min(uv0.x,uv1.x,uv2.x)>.5) )
			{
				// Must Wrap U: firstTgl becomes the near 0 tgle, newTgl the near 1
				ThreeUVs& newTgl = uvTgls.AddElem();
				newTgl.uv0 = uv0; newTgl.uv1 = uv1; newTgl.uv2 = uv2;
				ThreeUVs& preTgl = uvTgls[0];
				if(uv0.x>.5)	preTgl.uv0.x -= 1;
				else			newTgl.uv0.x += 1;
				if(uv1.x>.5)	preTgl.uv1.x -= 1;
				else			newTgl.uv1.x += 1;
				if(uv2.x>.5)	preTgl.uv2.x -= 1;
				else			newTgl.uv2.x += 1;
			}
			if( wrapV && 
				(MC_Max(uv0.y,uv1.y,uv2.y)-MC_Min(uv0.y,uv1.y,uv2.y)>.5) )
			{
				// Must Wrap V: the 1 or 2 existing get the V near 0, the new ones the V near 1
				const int32 curCount = uvTgls.GetElemCount();
				for(int32 i=0 ; i<curCount ; i++)
				{
					ThreeUVs& newTgl = uvTgls.AddElem();
					ThreeUVs& preTgl = uvTgls[i];
					newTgl.uv0 = preTgl.uv0; newTgl.uv1 = preTgl.uv1; newTgl.uv2 = preTgl.uv2;
					if(preTgl.uv0.y>.5)	preTgl.uv0.y -= 1;
					else				newTgl.uv0.y += 1;
					if(preTgl.uv1.y>.5)	preTgl.uv1.y -= 1;
					else			newTgl.uv1.y += 1;
					if(preTgl.uv2.y>.5)	preTgl.uv2.y -= 1;
					else				newTgl.uv2.y += 1;
				}
			}

			const int32 uvsCount = uvTgls.GetElemCount();
			for(int32 i=0 ; i<uvsCount ; i++)
			{
				RegisterFacet(
					  p0, p1, p2, 
					  hasNormals?normals[i1]:calcNormal, 
					  hasNormals?normals[i2]:calcNormal, 
					  hasNormals?normals[i3]:calcNormal, 
					  uvTgls[i].uv0, uvTgls[i].uv1, uvTgls[i].uv2, 
					  domainID);
			}
		}
		else
		{	// No UV case
			RegisterFacet(
				  p0, p1, p2, 
				  hasNormals?normals[i1]:calcNormal, 
				  hasNormals?normals[i2]:calcNormal, 
				  hasNormals?normals[i3]:calcNormal, 
				  TVector2::kZero, TVector2::kZero, TVector2::kZero, 
				  domainID);
		}

/*		TFacet3D& newFacet = newFacetData.fFacet;
		{
			newFacet.fVertices[0].fVertex  = vertices[i1];
			newFacet.fVertices[1].fVertex  = vertices[i2];
			newFacet.fVertices[2].fVertex  = vertices[i3];

			if (hasNormals)
			{
				newFacet.fVertices[0].fNormal  = normals[i1];
				newFacet.fVertices[1].fNormal  = normals[i2];
				newFacet.fVertices[2].fNormal  = normals[i3];
			}
			else
			{
				const TVector3 calcNormal= (newFacet.fVertices[1].fVertex - newFacet.fVertices[0].fVertex)^(newFacet.fVertices[2].fVertex - newFacet.fVertices[0].fVertex);
				newFacet.fVertices[0].fNormal  = calcNormal;
				newFacet.fVertices[1].fNormal  = calcNormal;
				newFacet.fVertices[2].fNormal  = calcNormal;
			}

			if (hasUVs)
			{
				newFacet.fVertices[0].fUV      = uv[i1];
				newFacet.fVertices[1].fUV      = uv[i2];
				newFacet.fVertices[2].fUV      = uv[i3];
			}
			else
			{
				newFacet.fVertices[0].fUV      = TVector2::kZero;
				newFacet.fVertices[1].fUV      = TVector2::kZero;
				newFacet.fVertices[2].fUV      = TVector2::kZero;
			}

			newFacet.fUVSpace = domainID;

			newFacet.fReserved = 0;

			// Cache other datas
			newFacetData.fUV1 = newFacet.fVertices[1].fUV - newFacet.fVertices[0].fUV;
			newFacetData.fUV2 = newFacet.fVertices[2].fUV - newFacet.fVertices[0].fUV;
			real32 det = (newFacetData.fUV1 ^  newFacetData.fUV2);
			if(RealAbs(det)<kRealEpsilon)
				newFacetData.fInvDet = 0;
			else
				newFacetData.fInvDet = 1.0f / det;
		}

		// Now check where this facet belongs
		const TVector2& uv0 = newFacet.fVertices[0].fUV;
		const TVector2& uv1 = newFacet.fVertices[1].fUV;
		const TVector2& uv2 = newFacet.fVertices[2].fUV;

		const int32 colIndex0 = Index(uv0.x);
		const int32 rowIndex0 = Index(uv0.y);
		const int32 index0 = colIndex0+fSideSegments*rowIndex0;
		const int32 colIndex1 = Index(uv1.x);
		const int32 rowIndex1 = Index(uv1.y);
		const int32 index1 = colIndex1+fSideSegments*rowIndex1;
		const int32 colIndex2 = Index(uv2.x);
		const int32 rowIndex2 = Index(uv2.y);
		const int32 index2 = colIndex2+fSideSegments*rowIndex2;

		if(index0==index1&&index0==index2)
			fFacetArrays[index0].AddElem(newFacetData);
		else
		{	// the UV triangle overlap several boxes
			// Parse them and see the one inside the triangle
			const int32 rowStart = MC_Min(rowIndex0,rowIndex1,rowIndex2);
			const int32 rowEnd = 1+MC_Max(rowIndex0,rowIndex1,rowIndex2);
			const int32 colStart = MC_Min(colIndex0,colIndex1,colIndex2);
			const int32 colEnd = 1+MC_Max(colIndex0,colIndex1,colIndex2);

			for(int32 iRow=rowStart ; iRow<rowEnd ; iRow++)
			{
				for(int32 iCol=colStart ; iCol<colEnd ; iCol++)
				{
					// Basic implementation: add the facet everywhere
					fFacetArrays[iCol+fSideSegments*iRow].AddElem(newFacetData);

					// Optimisation to do:

					// Check if the center of the square is in the triangle

					// If not, check the intersections between the sides of
					// the triangle and the square
				}
			}
		}
*/
	}
}

void UV2XYZ::RegisterFacet(
			  const TVector3& p0, const TVector3& p1, const TVector3& p2, 
			  const TVector3& n0, const TVector3& n1, const TVector3& n2, 
			  const TVector2& uv0, const TVector2& uv1, const TVector2& uv2, 
			  const int32 domainID)
{
	// Make the facet3D
	FacetData newFacetData;
	TFacet3D& newFacet = newFacetData.fFacet;
	{
		newFacet.fVertices[0].fVertex  = p0;
		newFacet.fVertices[1].fVertex  = p1;
		newFacet.fVertices[2].fVertex  = p2;

		newFacet.fVertices[0].fNormal  = n0;
		newFacet.fVertices[1].fNormal  = n1;
		newFacet.fVertices[2].fNormal  = n2;

		newFacet.fVertices[0].fUV      = uv0;
		newFacet.fVertices[1].fUV      = uv1;
		newFacet.fVertices[2].fUV      = uv2;

		newFacet.fUVSpace = domainID;

		newFacet.fReserved = 0;

		// Cache other datas
		newFacetData.fUV1 = uv1 - uv0;
		newFacetData.fUV2 = uv2 - uv0;
		real64 det = (newFacetData.fUV1 ^  newFacetData.fUV2);
	//	if(RealAbs(det)<kRealEpsilon)
		if(RealAbs(det)==0)
			newFacetData.fInvDet = 0;
		else
			newFacetData.fInvDet = 1.0f / det;
	}

	// Now check where this facet belongs
	const int32 colIndex0 = Index(uv0.x);
	const int32 rowIndex0 = Index(uv0.y);
	const int32 index0 = colIndex0+fSideSegments*rowIndex0;
	const int32 colIndex1 = Index(uv1.x);
	const int32 rowIndex1 = Index(uv1.y);
	const int32 index1 = colIndex1+fSideSegments*rowIndex1;
	const int32 colIndex2 = Index(uv2.x);
	const int32 rowIndex2 = Index(uv2.y);
	const int32 index2 = colIndex2+fSideSegments*rowIndex2;

	if(index0==index1&&index0==index2)
		fFacetArrays[index0].AddElem(newFacetData);
	else
	{	// the UV triangle overlap several boxes
		// Parse them and see the one inside the triangle
		const int32 rowStart = MC_Min(rowIndex0,rowIndex1,rowIndex2);
		const int32 rowEnd = 1+MC_Max(rowIndex0,rowIndex1,rowIndex2);
		const int32 colStart = MC_Min(colIndex0,colIndex1,colIndex2);
		const int32 colEnd = 1+MC_Max(colIndex0,colIndex1,colIndex2);

		for(int32 iRow=rowStart ; iRow<rowEnd ; iRow++)
		{
			for(int32 iCol=colStart ; iCol<colEnd ; iCol++)
			{
				// Basic implementation: add the facet everywhere
				fFacetArrays[iCol+fSideSegments*iRow].AddElem(newFacetData);

				// Optimisation to do:

				// Check if the center of the square is in the triangle

				// If not, check the intersections between the sides of
				// the triangle and the square
			}
		}
	}
}
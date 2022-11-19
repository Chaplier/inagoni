/****************************************************************************************************

		MBuildingCache.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingCache.h"
#include "I3dShFacetMesh.h"


BuildingCache::BuildingCache()
{
	fCacheIsValid = false;
	f2DDataAreValid = false;
	f3DDataAreValid = false;
	f2DPolygonColorsAreValid = false;
	f3DPolygonColorsAreValid = false;
}

BuildingCache::~BuildingCache()
{
}

void BuildingCache::SetPrimitive(BuildingPrim* primitive)
{
	if (primitive != fBuildingPrimitive)
	{
		InvalidateCache();
	}
	fBuildingPrimitive= primitive;
}


void BuildingCache::ClearCacheData()
{
	f2DFlatFacetMesh= NULL;
	f2DFacetMesh= NULL;
	f3DFlatFacetMesh= NULL;
	f3DFacetMesh= NULL;
	fBoundingBox= TBBox3D(TVector3::kOnes, -TVector3::kOnes); //set as invalid bbox
	
	f2DSegmentMesh.fVertex.ArrayFree();
	f2DSegmentMesh.fSegmentColors.ArrayFree();
	f2DSegmentMesh.fSegmentIndices.ArrayFree();
	
	f3DSegmentMesh.fVertex.ArrayFree();
	f3DSegmentMesh.fSegmentColors.ArrayFree();
	f3DSegmentMesh.fSegmentIndices.ArrayFree();
	
	f2DPointMesh.fVertex.ArrayFree();
	f2DPointMesh.fVertexColors.ArrayFree();
	f2DPointMesh.fVertexIndices.ArrayFree();

	f3DPointMesh.fVertex.ArrayFree();
	f3DPointMesh.fVertexColors.ArrayFree();
	f3DPointMesh.fVertexIndices.ArrayFree();
}


void BuildingCache::ValidateCache()
{
	if( !fCacheIsValid )
	{
		ClearCacheData();

		fCacheIsValid = true;
	}
}

void AddSegmentMesh( TSegmentMesh& onMesh, TSegmentMesh& fromMesh )
{
	const int32 offset = onMesh.fVertex.GetElemCount();
	onMesh.fVertex.Append(fromMesh.fVertex);
	onMesh.fVertexColors.Append(fromMesh.fVertexColors);

	onMesh.fSegmentColors.Append(fromMesh.fSegmentColors);

	const int32 fromICount = fromMesh.fSegmentIndices.GetElemCount();
	for( int32 i=0 ; i<fromICount ; i++ )
	{
		fromMesh.fSegmentIndices[i].x+=offset;
		fromMesh.fSegmentIndices[i].y+=offset;
	}

	onMesh.fSegmentIndices.Append(fromMesh.fSegmentIndices);
}

void AddPointMesh( TPointMesh& onMesh, TPointMesh& fromMesh )
{
	const int32 vtxCount = onMesh.fVertex.GetElemCount();
	onMesh.fVertex.Append(fromMesh.fVertex);
	onMesh.fVertexColors.Append(fromMesh.fVertexColors);
	const int32 offset = onMesh.fVertexIndices.GetElemCount();
	const int32 fromVtxCount = fromMesh.fVertex.GetElemCount();
	for( int32 iVtx=0 ; iVtx<fromVtxCount ; iVtx++ )
		fromMesh.fVertexIndices[iVtx]+=offset;

	onMesh.fVertexIndices.Append(fromMesh.fVertexIndices);

	// Be cautious before adding these
	const int32 onCount = onMesh.fMarkerID.GetElemCount();
	const int32 fromCount = fromMesh.fMarkerID.GetElemCount();
	if(onCount || fromCount)
	{
		if( onCount != vtxCount)
		{
			const int32 shape = onCount?onMesh.fMarkerID[0]:kSelectedHandle;
			onMesh.fMarkerID.SetElemCount(vtxCount);
			for(int32 i=onCount ; i<vtxCount ; i++)
			{
				onMesh.fMarkerID[i] = shape;
			}
		}
		if( onCount != fromVtxCount)
		{
			const int32 shape = fromCount?fromMesh.fMarkerID[0]:kSelectedHandle;
			fromMesh.fMarkerID.SetElemCount(fromVtxCount);
			for(int32 i=fromCount ; i<fromVtxCount ; i++)
			{
				fromMesh.fMarkerID[i] = shape;
			}
		}

		onMesh.fMarkerID.Append(fromMesh.fMarkerID);
	}
}

void BuildingCache::Validate2DData(const int32 level)
{
	if( !f2DDataAreValid )
	{
		if( fBuildingPrimitive )
		{
			TSegmentMesh objectSegmentMesh;
			TPointMesh objectPointMesh;
			
			fBuildingPrimitive->GetOther2DMeshes(	fBoundingBox,
													f2DSegmentMesh, 
													f2DPointMesh, 
													objectSegmentMesh, 
													objectPointMesh, 
													level );

			AddSegmentMesh( f2DSegmentMesh, objectSegmentMesh );
			AddPointMesh( f2DPointMesh, objectPointMesh );
			
			// Get the quotes
			fBuildingPrimitive->GetQuotation( fQuotation, level, fUnitSystem );

			MY_ASSERT(f2DPointMesh.fVertex.GetElemCount() == f2DPointMesh.fVertexColors.GetElemCount());
		}

		f2DDataAreValid = true;
	}
}

void BuildingCache::Validate3DData(const int32 level)
{
	if( !f3DDataAreValid )
	{
		if( fBuildingPrimitive )
		{
			TSegmentMesh objectSegmentMesh;
			TPointMesh objectPointMesh;
			
			fBuildingPrimitive->GetOther3DMeshes(	fBoundingBox,
													f3DSegmentMesh, 
													f3DPointMesh,
													level);

			MY_ASSERT(f3DPointMesh.fVertex.GetElemCount() == f3DPointMesh.fVertexColors.GetElemCount());

		}

		f3DDataAreValid = true;
	}
}

FacetMesh* BuildingCache::GetFacetMesh(const boolean mesh2D,const int32 level)
{ 
	ValidateCache();
	int32 flags = 0;
	if(mesh2D)
	{
		SET_FLAG(flags,e2DMesh);
		if (!f2DFacetMesh && fBuildingPrimitive)
		{
			fBuildingPrimitive->GetFacetMesh(0, &f2DFacetMesh, flags);
		}
		else if(!f2DPolygonColorsAreValid && fBuildingPrimitive)
		{
			fBuildingPrimitive->UpdateFacetMeshColors(f2DFacetMesh, flags );
		}
		f2DPolygonColorsAreValid = true;

		return f2DFacetMesh;
	}
	else
	{
		if (!f3DFacetMesh && fBuildingPrimitive)
		{
			fBuildingPrimitive->GetFacetMesh(0, &f3DFacetMesh, flags);
		}
		else if(!f3DPolygonColorsAreValid && fBuildingPrimitive)
		{
			fBuildingPrimitive->UpdateFacetMeshColors(f3DFacetMesh, flags );
		}
		f3DPolygonColorsAreValid = true;

		return f3DFacetMesh;
	}
}

FacetMesh* BuildingCache::GetFlatFacetMesh(const boolean mesh2D,const int32 level)
{
	ValidateCache();
	int32 flags = eFaceted;
	if(mesh2D)
	{
		SET_FLAG(flags,e2DMesh);
		if (!f2DFlatFacetMesh && fBuildingPrimitive)
		{
			fBuildingPrimitive->GetFacetMesh(0, &f2DFlatFacetMesh, flags);
		}
		else if(!f2DPolygonColorsAreValid && fBuildingPrimitive)
		{
			fBuildingPrimitive->UpdateFacetMeshColors(f2DFlatFacetMesh, flags );
		}
		f2DPolygonColorsAreValid = true;

		return f2DFlatFacetMesh;
	}
	else
	{
		if (!f3DFlatFacetMesh && fBuildingPrimitive)
		{
			fBuildingPrimitive->GetFacetMesh(0, &f3DFlatFacetMesh, flags);
		}
		else if(!f3DPolygonColorsAreValid && fBuildingPrimitive)
		{
			fBuildingPrimitive->UpdateFacetMeshColors(f3DFlatFacetMesh, flags );
		}
		f3DPolygonColorsAreValid = true;

		return f3DFlatFacetMesh;
	}
}

#endif // !NETWORK_RENDERING_VERSION


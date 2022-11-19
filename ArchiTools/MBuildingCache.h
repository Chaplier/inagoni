/****************************************************************************************************

		MBuildingCache.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingCache__
#define __MBuildingCache__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BuildingPrim.h"
#include "PQuotation.h"
#include "I3DShFacetMesh.h"

// Cache for the meshes (facets, lines and points)
class BuildingCache
{
public:
	BuildingCache();
	virtual ~BuildingCache();

	// Invalidate all the caches (FacetMesh and Modeler data)
	inline void		InvalidateCache()
	{	fCacheIsValid=false; 
		f2DDataAreValid=false; 
		f3DDataAreValid=false; 
		f2DPolygonColorsAreValid=false;
		f3DPolygonColorsAreValid=false;
	}

	// Keep the FacetMesh intact but invalidate everything else ( points, lines, facet color )
	inline void		InvalidateModelerData()
	{	f2DDataAreValid=false;
		f3DDataAreValid=false; 
		f2DPolygonColorsAreValid=false;
		f3DPolygonColorsAreValid=false;
	}

	inline void		Invalidate2DData()
	{	f2DDataAreValid=false;
		f2DFlatFacetMesh=false;
		f2DFacetMesh=false;
		f2DPolygonColorsAreValid=false;
	}
	
	//
	void			Release			(){fBuildingPrimitive=NULL;ClearCacheData();}
	void			SetPrimitive	(BuildingPrim* primitive);	
	void			SetHandleShape	(const uint32 shape){fHandleShape = shape;}	
	void			SetUnitSystem	(const EUnits unit){fUnitSystem = unit;}	

	boolean			IsCacheValid	()					{ return fCacheIsValid; }

	FacetMesh*				GetFlatFacetMesh(const boolean mesh2D,const int32 level);	//flat facet mesh
	FacetMesh*				GetFacetMesh	(const boolean mesh2D,const int32 level);	//regular facetmesh (with normals)
	inline TSegmentMesh*	GetSegmentMesh	(const boolean mesh2D,const int32 level);
	inline TPointMesh*		GetPointMesh	(const boolean mesh2D,const int32 level);
	void					GetBoundingBox	(TBBox3D& bbox)	{ ValidateCache(); Validate3DData(kAllLevels); bbox= fBoundingBox; }
	const Quotation&		GetQuotation() const {return fQuotation;}

protected:

	void			ClearCacheData();
	void			ValidateCache();
	void			Validate2DData(const int32 level); // for the Plan representation
	void			Validate3DData(const int32 level); // for the 3D view representation

	TMCCountedPtr<BuildingPrim>	fBuildingPrimitive;
	
	// Flags
	boolean						fCacheIsValid;
	boolean						f2DDataAreValid; // Data for vertices and edges display
	boolean						f3DDataAreValid; // Data for vertices and edges display
	boolean						f2DPolygonColorsAreValid;
	boolean						f3DPolygonColorsAreValid;
	uint32						fHandleShape;
	EUnits						fUnitSystem;
	int32						fActiveLevel;
	// Caches
	TMCCountedPtr<FacetMesh>	f2DFlatFacetMesh;
	TMCCountedPtr<FacetMesh>	f3DFlatFacetMesh;
	TMCCountedPtr<FacetMesh>	f2DFacetMesh;
	TMCCountedPtr<FacetMesh>	f3DFacetMesh;
	TSegmentMesh				f2DSegmentMesh;
	TSegmentMesh				f3DSegmentMesh;
	TPointMesh					f2DPointMesh;
	TPointMesh					f3DPointMesh;
	TBBox3D						fBoundingBox;
	Quotation					fQuotation;
};

inline TSegmentMesh* BuildingCache::GetSegmentMesh	(const boolean mesh2D,const int32 level)
{
	ValidateCache();
	if(mesh2D)
	{
		Validate2DData(level);
		return &f2DSegmentMesh;
	}
	else
	{
		Validate3DData(level);
		return &f3DSegmentMesh;
	}
}
inline TPointMesh* BuildingCache::GetPointMesh	(const boolean mesh2D,const int32 level)
{
	ValidateCache();
	if(mesh2D)
	{
		Validate2DData(level);
		return &f2DPointMesh;
	}
	else
	{
		Validate3DData(level);
		return &f3DPointMesh;
	}
}

#endif

#endif // !NETWORK_RENDERING_VERSION


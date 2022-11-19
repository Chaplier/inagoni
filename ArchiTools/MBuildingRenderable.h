/****************************************************************************************************

		MBuildingRenderable.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingRenderable__
#define __MBuildingRenderable__

#if CP_PRAGMA_ONCE
#pragma once
#endif



class BuildingModeler;
#include "BuildingPrim.h"

#include "I3dShFacetMesh.h"
#include "CountedRenderable.h"
class BuildingMeshRenderable : public TCountedRenderable
{
protected:

	BuildingMeshRenderable(BuildingModeler* modeler, I3DShRenderable::EType geomType);
	virtual ~BuildingMeshRenderable();

public:

	static void Create(	BuildingMeshRenderable**	renderable, 
						BuildingModeler*			modeler,
						I3DShRenderable::EType		geomType);

	I3DShRenderable::EType	MCCOMAPI GetGeometryType() const					{ return fGeometryType; }		
	TRenderableFlags		MCCOMAPI GetRenderableFlags( void ) const			{ return fRenderFlags; }
	MCCOMErr				MCCOMAPI GetFMesh(real lod, FacetMesh** outMesh);
	const TSegmentMesh*		MCCOMAPI GetSegmentMesh() const;	
	const TPointMesh*		MCCOMAPI GetPointMesh()	const;	
	MCCOMErr				MCCOMAPI GetFlatennedTexturesAsync(uint32 uvSpaceID, UVMaps & maps, TextureAvailableProc proc, void * privData, TMCRealRect * range = nil) const;
	void					MCCOMAPI GetBoundingBox(TBBox3D& bb);
	MCCOMErr				MCCOMAPI SetRenderableFlags(const TRenderableFlags renderFlags)		{ fRenderFlags	= renderFlags; return MC_S_OK; }
	
	uint32					MCCOMAPI GetUVSpaceCount(){return fBuildingPrimitive->GetUVSpaceCount();}

	void	SetGeometryType(I3DShRenderable::EType geomType)		{ fGeometryType	= geomType; }
	void	SetFlatFacets(const boolean isFlat){fIsFlatFacetMesh=isFlat;isFlat?fRenderFlags.SetMasked(TRenderableFlags::kUseScreenColorsMask):fRenderFlags.ClearMasked(TRenderableFlags::kUseScreenColorsMask);}
	void	Set2D(const boolean is2D){fIs2DMesh=is2D;}
	
protected:
	I3DShRenderable::EType			fGeometryType;
	TRenderableFlags				fRenderFlags;
	TMCCountedPtr<BuildingModeler>	fBuildingModeler;
	TMCCountedPtr<BuildingPrim>		fBuildingPrimitive;
	boolean							fIsFlatFacetMesh;
	boolean							fIs2DMesh;
};

//////////////////////////////////////////////////////////////////////
//
// Some extra renderables
//
class LevelGridRenderable : public TCountedRenderable
{
protected:

	LevelGridRenderable(BuildingModeler* modeler, const real32	altitude );
	virtual ~LevelGridRenderable();

public:

	static void Create(	LevelGridRenderable**	renderable, 
						BuildingModeler*		modeler,
						const real32			altitude );

	I3DShRenderable::EType	MCCOMAPI GetGeometryType() const {return kType_Segment;}		
	TRenderableFlags		MCCOMAPI GetRenderableFlags( void ) const {TRenderableFlags res;res.SetMasked(TRenderableFlags::kUseScreenColorsMask);return res; }
	const TSegmentMesh*		MCCOMAPI GetSegmentMesh() const;	
	void					MCCOMAPI GetBoundingBox(TBBox3D& bb);

	void					SetAltitude(const real32 altitude){fAltitude = altitude;}

protected:
	TMCCountedPtr<BuildingModeler>	fBuildingModeler;
	real32							fAltitude;
};

#endif

#endif // !NETWORK_RENDERING_VERSION

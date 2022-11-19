/****************************************************************************************************

		MBuildingRenderable.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingRenderable.h"

#include "BuildingModeler.h"
#include "I3DShObject.h"
#include "I3DShFacetmesh.h"

void BuildingMeshRenderable::Create(	BuildingMeshRenderable**	renderable, 
										BuildingModeler*			modeler,
										I3DShRenderable::EType		geomType)
{
	TMCCountedCreateHelper<BuildingMeshRenderable> result(renderable);
	result = new BuildingMeshRenderable(modeler, geomType);
}


BuildingMeshRenderable::BuildingMeshRenderable(BuildingModeler* modeler, I3DShRenderable::EType geomType)
{
	MY_ASSERT(modeler);
	fBuildingModeler		= modeler;
	fGeometryType			= geomType;	
	fIsFlatFacetMesh		= false;
	fIs2DMesh				= false;
	modeler->GetBuildingPrimitive(&fBuildingPrimitive);
	fRenderFlags.ClearMasked(TRenderableFlags::kUseScreenColorsMask); // for the normal rendering mode
//	fRenderFlags.SetMasked(TRenderableFlags::kUseScreenColorsMask); // for the flat rendering mode
}

BuildingMeshRenderable::~BuildingMeshRenderable()
{
}

MCCOMErr BuildingMeshRenderable::GetFMesh(real lod, FacetMesh** outMesh)
{
	if (MCVerify(fBuildingModeler))
	{
		fBuildingModeler->GetCurrentFacetMesh(outMesh, !fIsFlatFacetMesh, fIs2DMesh);
	}
	return MC_S_OK;
}

const TSegmentMesh*	BuildingMeshRenderable::GetSegmentMesh() const
{
	if (MCVerify(fBuildingModeler))
	{
		return fBuildingModeler->GetCurrentSegmentMesh(fIs2DMesh);
	}
	return NULL;
}

const TPointMesh* BuildingMeshRenderable::GetPointMesh()	const
{
	if (MCVerify(fBuildingModeler))
	{
		return fBuildingModeler->GetCurrentPointMesh(fIs2DMesh);
	}
	return NULL;
}

MCCOMErr BuildingMeshRenderable::GetFlatennedTexturesAsync(uint32 uvSpaceID,
														   UVMaps & maps,
														   TextureAvailableProc proc,
														   void * privData,
														   TMCRealRect * range) const
{
	I3DShPrimitive* prim = fBuildingModeler->GetPrimitiveNoAddRef();

	TMCCountedPtr<I3DShObject> object;
	prim->QueryInterface(IID_I3DShObject, (void**) &object);

	TMCCountedPtrArray<I3DShInstance> instances;
	object->GetInstanceArray(instances);

	const int32 index = 0;//fBuildingModeler->GetInstanceIndex()
	return instances[0]->GetFlatennedTexturesAsync(	uvSpaceID, maps, proc, privData, range);
}


void BuildingMeshRenderable::GetBoundingBox(TBBox3D& bb)
{
	if (MCVerify(fBuildingModeler))
	{
		fBuildingModeler->GetCurrentBoundingBox(bb);
	}
}

//////////////////////////////////////////////////////////////////////
//
// Some extra renderables
//

void LevelGridRenderable::Create(	LevelGridRenderable**	renderable, 
									BuildingModeler*		modeler,
									const real32			altitude )
{
	TMCCountedCreateHelper<LevelGridRenderable> result(renderable);
	result = new LevelGridRenderable(modeler,altitude);
}


LevelGridRenderable::LevelGridRenderable(BuildingModeler* modeler, const real32 altitude)
{
	MY_ASSERT(modeler);
	fBuildingModeler = modeler;
	fAltitude = altitude;
}

LevelGridRenderable::~LevelGridRenderable()
{
}

const TSegmentMesh*	LevelGridRenderable::GetSegmentMesh() const
{
	if (MCVerify(fBuildingModeler))
	{
		return fBuildingModeler->GetLevelGridSegmentMesh(fAltitude);
	}
	return NULL;
}

void LevelGridRenderable::GetBoundingBox(TBBox3D& bb)
{
	if (MCVerify(fBuildingModeler))
	{	// This is not the bbox of the level grid !!
		fBuildingModeler->GetCurrentBoundingBox(bb);
	}
}

#endif // !NETWORK_RENDERING_VERSION

/****************************************************************************************************

		BuildingPrim.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#include "BuildingPrim.h"

#include "MiscComUtilsImpl.h"
#include "PTessellator.h"
#include "IMFResponder.h"
#include "I3DShObject.h"
#include "I3DShScene.h"
#include "I3DShGroup.h"
#include "I3DShUtilities.h"
#include "COM3DUtilities.h"
#include "ISceneDocument.h"
#include "PQuotation.h"

#include "PCircleArc.h"
#include "PWallWithCrenel.h"
#include "I3DShFacetMesh.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"

const MCGUID CLSID_BuildingPrim(R_CLSID_BuildingPrim);
const MCGUID CLSID_HousePrim(R_CLSID_HousePrim);
#else
const MCGUID CLSID_BuildingPrim={R_CLSID_BuildingPrim};
const MCGUID CLSID_HousePrim={R_CLSID_HousePrim};
#endif

BuildingPrim::BuildingPrim()
{
	fResourceID = kBuildingPrimitiveResID; 
	fBBoxValid=false;
	fActiveLevel=0;
	fGroundLevelIndex=0;
	fShowAll=true;
	fPrivateGroupTreePath = TTreeIdPath::InvalidPath();
	// Build a default geom
	Init();
}
/*
BuildingPrim::BuildingPrim( int32 which )
{
	fResourceID = kHousePrimitiveResID; 

	fBBoxValid=false;
	fActiveLevel=0;
	fShowAll=true;
//	fShowLevel = kAllLevels;
	DefaultShadingDomainList();
	// Build a default geom
	Init();
}
*/
BuildingPrim::~BuildingPrim()
{	
	fLevelArray.ArrayFree();
}

MCCOMErr BuildingPrim::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_BuildingPrim) ||
		MCIsEqualIID(riid, CLSID_HousePrim) )
////	if (MCIsEqualIID(riid, CLSID_BuildingPrim) )
	{
		TMCCountedGetHelper<BuildingPrim> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	else if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	else
		return TBasicPrimitive::QueryInterface(riid, ppvObj);
}

void BuildingPrim::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	BuildingPrim* theClone = new BuildingPrim;
	if (theClone)
	{
		theClone->SetControllingUnknown(pUnkOuter);

		// Clean the clone (there's always a level 0 in it)
		theClone->fLevelArray.ArrayFree();

		// Copy the Data ( countain also the PMap )
		theClone->fData = fData;

		// Copy the remaining shading domains
		const int32 totalCount = fShadingDomains.GetElemCount();
		for(int32 iDom=kBasicDomainsCount ; iDom<totalCount ; iDom++)
		{
			theClone->AddShadingDomain(fShadingDomains[iDom].fName);
		}

		// Copy the geometry
		const int32 levelCount = fLevelArray.GetElemCount();

		{
			for(int32 iLevel=0;iLevel<levelCount;iLevel++)
			{
				TMCCountedPtr<Level> newLevel;
				fLevelArray[iLevel]->Clone(&newLevel,theClone,eNoChild); // Do not clone object children

				theClone->AddLevelToArray(newLevel, kLastLevel);
			}
		}

		const int32 groundLevelIndex = GetGroundLevelIndex();
		theClone->SetGroundLevelIndex(groundLevelIndex);
	}
	result=(IExDataExchanger*) theClone;
}

void* BuildingPrim::GetExtensionDataBuffer()
{
	return &this->fData;
}

MCCOMErr BuildingPrim::ExtensionDataChanged()
{
	// This is for the modifications made in the 3D view of the assemble room
	TMCCountedPtr<I3DShPrimitiveComponent> primComp;
	QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primComp);
	if(MCVerify(primComp))
	{
		TMCCountedPtr<I3DShPrimitive> primitive;
		primComp->GetPrimitive(&primitive);
		if(primitive)
		{
			primitive->ChangedData();
		}
	}
	return TBasicPrimitive::ExtensionDataChanged();
}

MCCOMErr BuildingPrim::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	return MC_S_OK;
}

int16 BuildingPrim::GetResID()
{
	return fResourceID;// always return -1, if you do not have a view
}

void BuildingPrim::GetBoundingBox(TBBox3D& bbox)
{
	GetBoundingBox(bbox, true, false);
}

void BuildingPrim::GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection)
{
	try
	{
		if(!onSelection&&fBBoxValid)
		{
			bbox = fBBox;
			return;
		}

		// Initialize with an invalid bounding box
		bbox.fMin = TVector3(1,1,1);
		bbox.fMax = TVector3(0,0,0);

		const int32 levelCount = fLevelArray.GetElemCount();

		if(levelCount)
		{
			boolean valid = false;

			TBBox3D levelBbox;
			for(int32 iLevel=0;iLevel<levelCount;iLevel++)
			{
				fLevelArray[iLevel]->GetBoundingBox(levelBbox, exact, onSelection);
				if(levelBbox.Valid())
				{
					if(valid)
						bbox+=levelBbox;
					else
					{
						bbox=levelBbox;
						valid = true;
					}
				}
			}
		}

		if(!bbox.Valid())
		{
			bbox.fMin = -TVector3::kOnes;
			bbox.fMax = TVector3::kOnes;
		}

		if(!onSelection)
		{
			fBBoxValid=true;
			fBBox = bbox;
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPrim::GetBoundingBox"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPrim::GetBoundingBox"));
	}
}
 
MCCOMErr BuildingPrim::GetNbrLOD(int16 &nbrLod)
{
	nbrLod = 1;
	return MC_S_OK;
}

MCCOMErr BuildingPrim::GetLOD(int16 lodIndex,real &lod)
{
	MY_ASSERT(lodIndex == 0);
	lod= 0;

	return MC_S_OK;
}

// Used by the shell
MCCOMErr BuildingPrim::GetFacetMesh(uint32 lodindex, FacetMesh** outMesh)
{
	return GetFacetMesh(lodindex, outMesh, eShellMesh);
}

MCCOMErr BuildingPrim::GetFacetMesh(uint32 lodindex, 
									FacetMesh** outMesh, 
									int32 meshFlags)
{
	try
	{
		// Create the facet mesh
		FacetMesh::Create(outMesh);
		TMCCountedPtr<FacetMesh> facetMesh;
		facetMesh = *outMesh;

		int32 useLevel = kAllLevels;
		if(FLAG(meshFlags,eShellMesh))
			useLevel = ShellMeshLevel();
		else if(FLAG(meshFlags,e2DMesh) || !fShowAll)
			useLevel = fActiveLevel;

		const int32 levelCount = LevelCount(useLevel);
		const int32 startLevel = StartLevel(useLevel);

		if(useLevel!=kAllLevels)
		{
			SET_FLAG(meshFlags, eNoTop);
		}

		for(int32 iLevel=startLevel;iLevel<levelCount;iLevel++)
		{
			Level* level= fLevelArray[iLevel];

			if (MCVerify(level))
			{
				if (level->Hidden())
					continue;

				TMCCountedPtr<FacetMesh> mesh;

				level->GetLevelFacetMesh(&mesh, lodindex, meshFlags);

				facetMesh->Append(*mesh);
			}
		}
		return MC_S_OK;
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPrim::GetFacetMesh"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPrim::GetFacetMesh"));
	}

	return MC_S_FALSE;
}

void BuildingPrim::Add2DObjectToMesh( SubObject* object,
									 TSegmentMesh&	objectSegmentMesh,
									 TPointMesh&	objectHandleMesh,
									 const boolean	useFreezeColor,
									 TMCClassArray<PosAndObj>& handlesCache )
{
	TMCClassArray<TVector3>	boxPos;
	object->GetBox( boxPos );

	// Draw the rectangle and its diagonals
	const int32 baseSegmentCount = objectSegmentMesh.fVertex.GetElemCount();

	// The square
	for(int32 i=4 ; i<8 ; i++) // We start at 4 to get the highest one of the room objects
	{
		objectSegmentMesh.fVertex.AddElem( boxPos[i] );
		int32 iMinus4 = i-4;
		const TIndex2 index( baseSegmentCount+iMinus4,baseSegmentCount+((iMinus4+1)%4) );
		objectSegmentMesh.fSegmentIndices.AddElem( index );
	}

	// the 2 diagonals
	const TIndex2 index4( baseSegmentCount,baseSegmentCount+2 );
	objectSegmentMesh.fSegmentIndices.AddElem( index4 );
	const TIndex2 index5( baseSegmentCount+1,baseSegmentCount+3 );
	objectSegmentMesh.fSegmentIndices.AddElem( index5 );

	// Put a handle in the middle
	const int32 baseHandleCount = objectHandleMesh.fVertex.GetElemCount();
	const TVector3 handlePos = .5*(boxPos[0]+boxPos[6]);
	objectHandleMesh.fVertex.AddElem( handlePos );
	objectHandleMesh.fVertexIndices.AddElem( baseHandleCount );

	// Store the handle position in the cache
	handlesCache.AddElem(PosAndObj(handlePos,object->GetOutline()[0])); // store the first point, it's just to keep an access toward the object

	// Colors
	TMCColorRGBA8 color = fData.fObjCol;
	if(useFreezeColor)color = fData.fFreCol;
	else if(object->Selected()) color = fData.fSelCol;
	else if(object->Targeted())	color = fData.fTarCol;

	for( int32 iSeg=0 ; iSeg<6 ; iSeg++)
		objectSegmentMesh.fSegmentColors.AddElem( color );
	objectHandleMesh.fVertexColors.AddElem( color );
}

void BuildingPrim::AddObjectToMesh( SubObject* object,
					 TSegmentMesh&	objectSegmentMesh,
					 TPointMesh&	objectHandleMesh,
					 const boolean	useFreezeColor,
					 TMCClassArray<PosAndObj>& handlesCache,
					 const boolean oneHandle)
{
	TMCClassArray<TVector3>	boxPos;
	object->GetBox( boxPos );

	const int32 base = objectSegmentMesh.fVertex.GetElemCount();

	for(int32 i=0 ; i<8 ; i++)
	{	// In the 3D view: 2 rectangles
		objectSegmentMesh.fVertex.AddElem( boxPos[i] );
	}

	// First rectangle
	{
		const TIndex2 index0( base,base+1 );
		objectSegmentMesh.fSegmentIndices.AddElem( index0 );
		const TIndex2 index1( base+1,base+5 );
		objectSegmentMesh.fSegmentIndices.AddElem( index1 );
		const TIndex2 index2( base+5,base+4 );
		objectSegmentMesh.fSegmentIndices.AddElem( index2 );
		const TIndex2 index3( base+4,base );
		objectSegmentMesh.fSegmentIndices.AddElem( index3 );

		if(!oneHandle)
		{
			// Diagonals
			const TIndex2 index4( base,base+5 );
			objectSegmentMesh.fSegmentIndices.AddElem( index4 );
			const TIndex2 index5( base+1,base+4 );
			objectSegmentMesh.fSegmentIndices.AddElem( index5 );

			// Put a handle in the middle
			const int32 baseHandleCount = objectHandleMesh.fVertex.GetElemCount();
			const TVector3 handlePos = .5*(boxPos[0]+boxPos[5]);
			objectHandleMesh.fVertex.AddElem( handlePos );
			objectHandleMesh.fVertexIndices.AddElem( baseHandleCount );

			// Store the handle position in the cache
			handlesCache.AddElem(PosAndObj(handlePos,object->GetOutline()[0]));
		}
	}

	// Second rectangle
	{
		const TIndex2 index0( base+2,base+3 );
		objectSegmentMesh.fSegmentIndices.AddElem( index0 );
		const TIndex2 index1( base+3,base+7 );
		objectSegmentMesh.fSegmentIndices.AddElem( index1 );
		const TIndex2 index2( base+7,base+6 );
		objectSegmentMesh.fSegmentIndices.AddElem( index2 );
		const TIndex2 index3( base+6,base+2 );
		objectSegmentMesh.fSegmentIndices.AddElem( index3 );

		if(!oneHandle)
		{
			// Diagonals
			const TIndex2 index4( base+2,base+7 );
			objectSegmentMesh.fSegmentIndices.AddElem( index4 );
			const TIndex2 index5( base+3,base+6 );
			objectSegmentMesh.fSegmentIndices.AddElem( index5 );

			// Put a handle in the middle
			const int32 baseHandleCount = objectHandleMesh.fVertex.GetElemCount();
			const TVector3 handlePos = .5*(boxPos[2]+boxPos[7]);
			objectHandleMesh.fVertex.AddElem( handlePos );
			objectHandleMesh.fVertexIndices.AddElem( baseHandleCount );

			// Store the handle position in the cache
			handlesCache.AddElem(PosAndObj(handlePos,object->GetOutline()[0]));
		}
	}

	if(oneHandle)
	{
		// Diagonals
		const TIndex2 index0( base,base+6 );
		objectSegmentMesh.fSegmentIndices.AddElem( index0 );
		const TIndex2 index1( base+1,base+7 );
		objectSegmentMesh.fSegmentIndices.AddElem( index1 );
		const TIndex2 index2( base+2,base+4 );
		objectSegmentMesh.fSegmentIndices.AddElem( index2 );
		const TIndex2 index3( base+3,base+5 );
		objectSegmentMesh.fSegmentIndices.AddElem( index3 );
	
		// Put a handle in the middle
		const int32 baseHandleCount = objectHandleMesh.fVertex.GetElemCount();
		const TVector3 handlePos = .5*(boxPos[0]+boxPos[6]);
		objectHandleMesh.fVertex.AddElem( handlePos );
		objectHandleMesh.fVertexIndices.AddElem( baseHandleCount );

		// Store the handle position in the cache
		handlesCache.AddElem(PosAndObj(handlePos,object->GetOutline()[0]));
	}

	// Colors
	TMCColorRGBA8 color = fData.fObjCol;
	if(useFreezeColor)color = fData.fFreCol;
	else if(object->Selected()) color = fData.fSelCol;
	else if(object->Targeted())	color = fData.fTarCol;

	for( int32 iSeg=0 ; iSeg<12 ; iSeg++)
		objectSegmentMesh.fSegmentColors.AddElem( color );
	objectHandleMesh.fVertexColors.AddElem( color );
	if(!oneHandle)
		objectHandleMesh.fVertexColors.AddElem( color );
}

// an object in Edit mode
void BuildingPrim::AddEditableObjectToMesh(	SubObject*		object,
											TSegmentMesh&	objectSegmentMesh,
											TPointMesh&		objectHandleMesh,
											const boolean	useFreezeColor,
											TMCClassArray<PosAndObj>& handlesCache )
{
	// We don't care about the normals, but we want to draw points on both sides of the volume
	TMCClassArray<TVector3> side0;
	TMCClassArray<TVector3> side1;
	TMCClassArray<TVector3> normals;
	object->Get3DOutlines(side0, side1, normals);

	// Get the outline to access the flags
	const TMCCountedPtrArray<OutlinePoint>& outline = object->GetOutline();

	// Draw the segments
	const int32 polyPointCount = side0.GetElemCount();
	const int32 baseSegmentCount = objectSegmentMesh.fVertex.GetElemCount();
	const int32 baseHandleCount = objectHandleMesh.fVertex.GetElemCount();

	for(int32 iPoint = 0 ; iPoint<polyPointCount ; iPoint++)
	{
		// The pos
		const TVector3& pos0 = side0[iPoint];
		const TVector3& pos1 = side1[iPoint];
		// The point
		OutlinePoint* point = outline[iPoint];

		// The segments indexes
		const int32 curIndex0 = baseSegmentCount + 2*iPoint;
		const int32 curIndex1 = curIndex0 + 1;
		const int32 nextIndex0 = baseSegmentCount + 2*((iPoint+1)%polyPointCount);
		const int32 nextIndex1 = nextIndex0+1;

		// Add the 2 positions
		objectSegmentMesh.fVertex.AddElem( pos0 );
		objectSegmentMesh.fVertex.AddElem( pos1 );
		// Store the handle position in the cache
		handlesCache.AddElem(PosAndObj(pos0,point));
		handlesCache.AddElem(PosAndObj(pos1,point));

		// Describe the 3 segments (2 to the next points, 1 between the 2 current points)
		const TIndex2 indexBetween( curIndex0, curIndex1 );
		objectSegmentMesh.fSegmentIndices.AddElem( indexBetween );
		const TIndex2 index0( curIndex0, nextIndex0 );
		objectSegmentMesh.fSegmentIndices.AddElem( index0 );
		const TIndex2 index1( curIndex1, nextIndex1 );
		objectSegmentMesh.fSegmentIndices.AddElem( index1 );

		// Colors: 3 segments
		objectSegmentMesh.fSegmentColors.AddElem(fData.fObjCol);
		objectSegmentMesh.fSegmentColors.AddElem(fData.fObjCol);
		objectSegmentMesh.fSegmentColors.AddElem(fData.fObjCol);

		// The Handles indexes
		const int32 handleIndex0 = baseHandleCount + 2*iPoint;
		const int32 handleIndex1 = handleIndex0 + 1;
		objectHandleMesh.fVertex.AddElem(pos0);
		objectHandleMesh.fVertexIndices.AddElem( handleIndex0 );
		objectHandleMesh.fVertex.AddElem(pos1);
		objectHandleMesh.fVertexIndices.AddElem( handleIndex1 );
		
		// Colors: 2 points
		// TO DO: check the polyline flags
		TMCColorRGBA8 color = fData.fObjCol;
		if(useFreezeColor)color = fData.fFreCol;
		else if(point->Selected()) color = fData.fSelCol;
		else if(point->Targeted()) color = fData.fTarCol;
		// Could set a color for Smoothed points too

		objectHandleMesh.fVertexColors.AddElem(color);
		objectHandleMesh.fVertexColors.AddElem(color);
	}
}

inline int32 PseudoModulo(const int32 value, const int32 modul)
{
	if(value<0)return value+modul;
	else return value%modul;
}

const TMCColorRGBA8& BuildingPrim::GetColor(CommonBase* obj)
{
	if(obj->Selected())					return fData.fSelCol;
	else if(obj->Targeted())			return fData.fTarCol;
	else if(obj->Flag(eSnapedPosition))	return fData.fSnaCol;
	else if(obj->Flag(eWallHelper))		return fData.fHelCol;
	else								return fData.fDefCol;
}

const TMCColorRGBA8& BuildingPrim::GetColor(RoofPoint* roofPoint, bool zone)
{
	if(roofPoint->Selected())			return fData.fSelCol;
	else if(roofPoint->Targeted())		return fData.fTarCol;
	else if(zone)						return fData.fBRoCol; // Bottom of the roof
	else								return fData.fTRoCol; // Top of the roof
}

void BuildingPrim::AddRoofTo2DMesh(Roof*			roof, 
								   TPointMesh&		pointMesh,
								   TSegmentMesh&	segmentMesh, 
								   real32			zOffset)
{
	// Segment mesh indexes
	int32 segVtxIndex = segmentMesh.fVertex.GetElemCount();
	int32 segSegIndex = segmentMesh.fSegmentIndices.GetElemCount();
	int32 segColIndex = segmentMesh.fSegmentColors.GetElemCount();

	// Point mesh indexes
	int32 pntVtxIndex = pointMesh.fVertex.GetElemCount();
	int32 pntColIndex = pointMesh.fVertexColors.GetElemCount();

	const TMCColorRGBA8& mRoCol = fData.fBRoCol*.5+fData.fTRoCol*.5;

	const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

	const int32 startIndex = segVtxIndex;

	const real32 zMin = zOffset+roof->GetRoofMin();
	const real32 zMax = zOffset+roof->GetRoofMax();
	for(int32 iZonePt=0 ; iZonePt<zoneSectionCount ; iZonePt++)
	{
		const ZoneSection& zoneSection = roof->GetRoofZoneSection(iZonePt);
		// Each ZonePoint countain 2 points (zone and spine)
		// And 3 segments

		//	Later	if (point->Hidden())
		//				continue;

		// 1: surrounding roof point
		TVector3 pos1; pos1.SetFromXY( zoneSection.fZonePoint->Position(), zMin);

		//points for points mesh
		pointMesh.fVertex.AddElem( pos1 );
		pointMesh.fVertexIndices.AddElem( pntVtxIndex++ );
		pointMesh.fVertexColors.AddElem( GetColor( zoneSection.fZonePoint, true ) );

		//points for segment mesh
		segmentMesh.fVertex.AddElem( pos1 );					
					
		// 2: top roof point
		TVector3 pos2; pos2.SetFromXY( zoneSection.fSpinePoint->Position(), zMax);

		//points for points mesh
		pointMesh.fVertex.AddElem( pos2 );
		pointMesh.fVertexIndices.AddElem( pntVtxIndex++ );
		pointMesh.fVertexColors.AddElem( GetColor( zoneSection.fSpinePoint, false ) );

		//points for segment mesh
		segmentMesh.fVertex.AddElem( pos2 );

		// Add the 3 segments
		const int32 modulo = zoneSectionCount*2;
		for( int32 iSgm=0 ; iSgm<3 ; iSgm++)
		{
			int32 index0 = startIndex, index1 = startIndex;
			switch(iSgm)
			{
			case 0:
				{
					index0+=2*iZonePt;index1+=2*iZonePt + 1;
					segmentMesh.fSegmentColors.AddElem( mRoCol );
				}break;
			case 1:
				{
					index0+=2*iZonePt;index1+=((2*iZonePt + 2)%modulo);
					segmentMesh.fSegmentColors.AddElem( fData.fBRoCol );
				}break;
			case 2:
				{
					index0+=2*iZonePt + 1;index1+=((2*iZonePt + 3)%modulo);
					segmentMesh.fSegmentColors.AddElem( fData.fTRoCol );
				}break;
			}
			const TIndex2 index2( index0, index1);

			segmentMesh.fSegmentIndices.AddElem( index2 );

//			segSegIndex++;
		}
	}
}

void BuildingPrim::AddWallTo2DMesh(	Wall*			wall, 
								    TPointMesh&		pointMesh, // to add the middle handle
								    TSegmentMesh&	segmentMesh,
								    float			zUp)
{
	const TMCColorRGBA8& color = GetColor(wall);
	
	const TMCClassArray<TVector2>& leftPos = wall->GetLeftPosProjection();
	const TMCClassArray<TVector2>& rightPos = wall->GetRightPosProjection();

	const int32 posCount = leftPos.GetElemCount();

	int32 curIndex = segmentMesh.fVertex.GetElemCount();
	int32 segIndex = segmentMesh.fSegmentIndices.GetElemCount();
	int32 colIndex = segmentMesh.fSegmentColors.GetElemCount();

	const int32 firstVtxIndex = curIndex;

	segmentMesh.fVertex.AddElemCount(2*posCount);
	segmentMesh.fSegmentIndices.AddElemCount( 2*(posCount-1) );
	segmentMesh.fSegmentColors.AddElemCount( 2*(posCount-1) );

	segmentMesh.fVertex[curIndex++].SetValues(leftPos[0].x, leftPos[0].y, zUp);
	segmentMesh.fVertex[curIndex++].SetValues(rightPos[0].x, rightPos[0].y, zUp);
	for(int32 iPos=1 ; iPos<posCount ; iPos++)
	{
		segmentMesh.fVertex[curIndex++].SetValues(leftPos[iPos].x, leftPos[iPos].y, zUp);
		segmentMesh.fVertex[curIndex++].SetValues(rightPos[iPos].x, rightPos[iPos].y, zUp);
		
		segmentMesh.fSegmentIndices[segIndex++].Set(curIndex-4, curIndex-2);
		segmentMesh.fSegmentIndices[segIndex++].Set(curIndex-3, curIndex-1);
		segmentMesh.fSegmentColors[colIndex++] = color;
		segmentMesh.fSegmentColors[colIndex++] = color;

	}
	
	VPoint* point0 = wall->GetPoint(0);
	VPoint* point1 = wall->GetPoint(1);

	const int32 lastIndex = firstVtxIndex + 2*posCount;

	// Straight line for curved walls
	if(posCount!=2)
	{	// Draw a straight line
		TVector3 pointPos;
		pointPos.SetFromXY( point0->Position(), zUp);
		segmentMesh.fVertex.AddElem(pointPos);
		pointPos.SetFromXY( point1->Position(), zUp);
		segmentMesh.fVertex.AddElem(pointPos);

		segmentMesh.fSegmentIndices.AddElem(TIndex2(lastIndex,lastIndex+1));
		segmentMesh.fSegmentColors.AddElem(color);
	}

	// Dead ends: draw the extremities

	if(point0->GetWallCount() == 1)
	{	// Add the first extremity
		segmentMesh.fSegmentIndices.AddElem(TIndex2(firstVtxIndex,firstVtxIndex+1));
		segmentMesh.fSegmentColors.AddElem(color);
	}

	if(point1->GetWallCount() == 1)
	{	// Add the last extremity
		segmentMesh.fSegmentIndices.AddElem(TIndex2(lastIndex-2,lastIndex-1));
		segmentMesh.fSegmentColors.AddElem(color);
	}

	// Add the handle to modify the shape of the wall
	TVector3 pointPos;
	pointPos.SetFromXY( wall->GetMidPointPos(), zUp);
	pointMesh.fVertexIndices.AddElem( pointMesh.fVertex.GetElemCount() );
	pointMesh.fVertex.AddElem( pointPos );
	pointMesh.fVertexColors.AddElem( color*.5 );
}

// Build the edge and point meshes needed for the 2D representation
// of the building
void BuildingPrim::GetOther2DMeshes(	TBBox3D&		bBox,
										TSegmentMesh&	geomSegmentMesh, 
										TPointMesh&		geomPointMesh,
										TSegmentMesh&	objectSegmentMesh, 
										TPointMesh&		objectHandleMesh,
										const int32		inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	// Evaluation of the number of points and segments

	int32 totalSegmentCount = 0; // Include Walls and Roof segments
	int32 totalPointCount = 0; // Include wall extremity and roof zone
	int32 totalSegmentPointCount = 0; // Include wall extremity and roof zone
	int32 wallPointCount = 0;
	int32 totalObjectCount = 0;

	{
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			const int32 pointCount = level->GetPointCount();
			wallPointCount += pointCount;
			totalPointCount += pointCount;
			for(int32 iPt=0 ; iPt<pointCount ; iPt++)
			{
				VPoint* point = level->GetPoint( iPt );
				const int32 wallCount = point->GetWallCount();
				totalSegmentPointCount += wallCount==1?2:wallCount; // for extremity
			}

			const int32 wallCount = level->GetWallCount();
			totalSegmentCount += 2*wallCount;

			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				totalObjectCount += level->GetWall( iWall )->GetObjectCount();
			}
		
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				totalObjectCount += level->GetRoom( iRoom )->GetObjectCount();
			}

			// Add points and segments for the roofs
			const int32 roofCount = level->GetRoofCount();
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);
				if(roof->Hidden())
					continue;
			
				const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

				totalPointCount += 2*zoneSectionCount;
				totalSegmentPointCount += 2*zoneSectionCount;
				totalSegmentCount += 3*zoneSectionCount;
			}
		}

		// Get some information in the level below we want to show
		if(inLevel>0)
		{
			Level* level = fLevelArray[inLevel-1];
		
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				totalObjectCount += level->GetRoom( iRoom )->GetObjectCount();
			}
		}
	}

	// Current representation of an object:
	// in Normal Mode : twice 6 edges and 1 point (on each side of the volume)
	// in Edit Mode : twice all the points and edges of the primitive
	//
	// -------
	// |\   /|
	// | \ / |
	// |  O  |	or all the points and edges
	// | / \ |
	// |/   \|
	// -------
	//

	// Space allocation : it's only an estimation
	geomSegmentMesh.fVertex.SetElemSpace(totalSegmentPointCount);
	geomSegmentMesh.fSegmentColors.SetElemSpace(totalSegmentCount);
	geomSegmentMesh.fSegmentIndices.SetElemSpace(totalSegmentCount);
	geomSegmentMesh.fVertex.SetElemCount(0);
	geomSegmentMesh.fSegmentColors.SetElemCount(0);
	geomSegmentMesh.fSegmentIndices.SetElemCount(0);

	// Space allocation : it's only an estimation
	geomPointMesh.fVertex.SetElemSpace(totalPointCount);
	geomPointMesh.fVertexIndices.SetElemSpace(totalPointCount);
	geomPointMesh.fVertexColors.SetElemSpace(totalPointCount);
	geomPointMesh.fVertex.SetElemCount(0);
	geomPointMesh.fVertexIndices.SetElemCount(0);
	geomPointMesh.fVertexColors.SetElemCount(0);

	// Space allocation : it's only an estimation
	objectSegmentMesh.fVertex.SetElemSpace(4*totalObjectCount);
	objectSegmentMesh.fSegmentColors.SetElemSpace(6*totalObjectCount);
	objectSegmentMesh.fSegmentIndices.SetElemSpace(6*totalObjectCount);
	objectSegmentMesh.fVertex.SetElemCount(0);
	objectSegmentMesh.fSegmentColors.SetElemCount(0);
	objectSegmentMesh.fSegmentIndices.SetElemCount(0);

	// Space allocation : it's only an estimation
	objectHandleMesh.fVertex.SetElemSpace(totalObjectCount);
	objectHandleMesh.fVertexIndices.SetElemSpace(totalObjectCount);
	objectHandleMesh.fVertexColors.SetElemSpace(totalObjectCount);
	objectHandleMesh.fVertex.SetElemCount(0);
	objectHandleMesh.fVertexIndices.SetElemCount(0);
	objectHandleMesh.fVertexColors.SetElemCount(0);
	objectHandleMesh.fMarkerID.SetElemCount(1);
	objectHandleMesh.fMarkerID[0] = kFullRoundHandle;

	//const TMCColorRGBA8& defCol = fData.fDefCol;
	//const TMCColorRGBA8& selCol = fData.fSelCol;
	//const TMCColorRGBA8& tarCol = fData.fTarCol;
	//const TMCColorRGBA8& snaCol = fData.fSnaCol;
	//const TMCColorRGBA8& helCol = fData.fHelCol;
	//const TMCColorRGBA8& freCol = fData.fFreCol;
	const TMCColorRGBA8& bRoCol = fData.fBRoCol;
	const TMCColorRGBA8& tRoCol = fData.fTRoCol;
	const TMCColorRGBA8& mRoCol = bRoCol*.5+tRoCol*.5;

	// Clean up the cached data: handles for the selection
	mWallObjectHandles2D.SetElemCount(0);
	mRoomObjectHandles2D.SetElemCount(0);

	{
		int32 pointIndex=0;
		int32 segmentIndex=0;

		// Data to build the segment mesh
		int32 pointSegmentIndex=0;
		TMCArray<int32> indexTable; // relationship between the pointIndex and the first pointSegmentIndex
		indexTable.SetElemSpace(wallPointCount); // Minimum space needed

		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			const real32 levelHeight=level->GetLevelHeight(); // to be sure that the edges and handles will be visible over the floor
			const real32 zUp = level->GetDistanceToGround()+levelHeight;
			// Build the point mesh
			const int32 pointCount = level->GetPointCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point = level->GetPoint( iPoint );

				if (point->Hidden())
					continue;

				TVector3 pointPos;
				pointPos.SetFromXY( point->Position(), zUp);

									
				//points for points mesh
				geomPointMesh.fVertex.AddElem( pointPos );
				geomPointMesh.fVertexIndices.AddElem( pointIndex );

				geomPointMesh.fVertexColors.AddElem( GetColor(point) );

				pointIndex++;
			}

			// Build the segment mesh
			const int32 wallCount = level->GetWallCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = level->GetWall( iWall );

				if (wall->Hidden())
					continue;

				AddWallTo2DMesh(wall, geomPointMesh, geomSegmentMesh, zUp);

				// Make the representation of the objects of the wall
				const int32 objectCount = wall->GetObjectCount();
				for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
				{
					WallSubObject* object = wall->GetObject(iObject);

					if(fData.GetHoleEditEnable())
						AddEditableObjectToMesh(object,objectSegmentMesh,objectHandleMesh,false, mWallObjectHandles2D);
					else
						Add2DObjectToMesh(object,objectSegmentMesh,objectHandleMesh,false, mWallObjectHandles2D);
				}
			}

			// Add the points and segments for the roofs
			const int32 roofCount = level->GetRoofCount();
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);
			
				if (roof->Hidden())
					continue;

				AddRoofTo2DMesh(roof, geomPointMesh, geomSegmentMesh, zUp);
			}

			// Make the representation of the objects of the rooms
			// We should also represent the objects from the
			// levels under that go through this one (stairways, chimey)
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom( iRoom );

				if (room->Hidden())
					continue;

				// Make the representation of the objects of the room
				const int32 objectCount = room->GetObjectCount();
				for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
				{
					RoomSubObject* object = room->GetObject(iObject);
	
					if(fData.GetHoleEditEnable())
						AddEditableObjectToMesh(object,objectSegmentMesh,objectHandleMesh,false, mRoomObjectHandles2D);
					else
						Add2DObjectToMesh(object,objectSegmentMesh,objectHandleMesh,false, mRoomObjectHandles2D);
				}
			}
		}

		// Get some information in the level below we want to show
		if(inLevel>0)
		{
			Level* level = fLevelArray[inLevel-1];
		
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom( iRoom );

				if (room->Hidden())
					continue;

				// Make the representation of the objects of the room
				const int32 objectCount = room->GetObjectCount();
				for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
				{
					RoomSubObject* object = room->GetObject(iObject);
	
					Add2DObjectToMesh(object,objectSegmentMesh,objectHandleMesh,true, mRoomObjectHandles2D);
				}
			}
		}
	}

	GetBoundingBox(bBox);
}

void BuildingPrim::GetOther3DMeshes(	TBBox3D&		bBox,
										TSegmentMesh&	segmentMesh, 
										TPointMesh&		pointMesh,
										const int32		inLevel ) // EFacetMeshFlags flag
{
	// Get the representation of the objects in 3D
	int32 totalObjectCount = 0;
	// And the representation of the roof profiles
	int32 totalProfilePointCount = 0;

	int32 useLevel=inLevel;
	if( !GetActualLevel(useLevel, inLevel) )
		return;

	const int32 levelCount = LevelCount(useLevel);
	const int32 startLevel = StartLevel(useLevel);

	{
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			const int32 wallCount = level->GetWallCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				totalObjectCount += level->GetWall( iWall )->GetObjectCount();
			}
		
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				totalObjectCount += level->GetRoom( iRoom )->GetObjectCount();
			}

			if(fShowAll)
			{
				const int32 roofCount = level->GetRoofCount();
				for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
				{
					Roof* roof = level->GetRoof( iRoof );
				//	totalObjectCount += roof->GetObjectCount();
			
					const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

					const int32 profilePointCount = roof->GetTotalProfilePointCount();

					totalProfilePointCount+=zoneSectionCount*profilePointCount;
				}
			}
		}
	}

	// Current representation of an object: 6 edges and 1 point
	//
	// -------
	// |\   /|
	// | \ / |
	// |  O  |
	// | / \ |
	// |/   \|
	// -------
	//

	// Space allocation: only an approximation
	segmentMesh.fVertex.SetElemSpace(4*totalObjectCount);
	segmentMesh.fSegmentColors.SetElemSpace(6*totalObjectCount);
	segmentMesh.fSegmentIndices.SetElemSpace(6*totalObjectCount);
	segmentMesh.fVertex.SetElemCount(0);
	segmentMesh.fSegmentColors.SetElemCount(0);
	segmentMesh.fSegmentIndices.SetElemCount(0);

	// Space allocation: only an approximation
	pointMesh.fVertex.SetElemSpace(totalObjectCount + totalProfilePointCount);
	pointMesh.fVertexIndices.SetElemSpace(totalObjectCount + totalProfilePointCount);
	pointMesh.fVertexColors.SetElemSpace(totalObjectCount + totalProfilePointCount);
	pointMesh.fVertex.SetElemCount(0);
	pointMesh.fVertexIndices.SetElemCount(0);
	pointMesh.fVertexColors.SetElemCount(0);
	pointMesh.fMarkerID.SetElemCount(1);
	pointMesh.fMarkerID[0] = kFullRoundHandle;

	const TMCColorRGBA8& defCol = fData.fDefCol;
	const TMCColorRGBA8& selCol = fData.fSelCol;
	const TMCColorRGBA8& tarCol = fData.fTarCol;

	// Clean up the cached data: handles for the selection
	mWallObjectHandles3D.SetElemCount(0);
	mRoomObjectHandles3D.SetElemCount(0);

	{	// Draw the objects
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			// Build the segment mesh
			const int32 wallCount = level->GetWallCount();
			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = level->GetWall( iWall );

				if (wall->Hidden())
					continue;

				// Make the representation of the objects of the wall
				const int32 objectCount = wall->GetObjectCount();
				for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
				{
					WallSubObject* object = wall->GetObject(iObject);
	
					if(fData.GetHoleEditEnable())
						AddEditableObjectToMesh(object,segmentMesh,pointMesh,false, mWallObjectHandles3D);
					else
						AddObjectToMesh(object,segmentMesh,pointMesh,false, mWallObjectHandles3D, false);
				}
			}

			// Make the representation of the objects of the rooms
			const int32 roomCount = level->GetRoomCount();
			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom( iRoom );

				if (room->Hidden())
					continue;

				// Make the representation of the objects of the room
				const int32 objectCount = room->GetObjectCount();
				for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
				{
					RoomSubObject* object = room->GetObject(iObject);
	
					if(fData.GetHoleEditEnable())
						AddEditableObjectToMesh(object,segmentMesh,pointMesh,false, mRoomObjectHandles3D);
					else
						AddObjectToMesh(object,segmentMesh,pointMesh,false, mRoomObjectHandles3D, true);
				}
			}
		}
	}

	{	// Draw the profile and the point helper
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			if(fShowAll)
			{
				// Make the representation of the objects of the roofs
				// and their profile
				const int32 roofCount = level->GetRoofCount();
				for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
				{
					Roof* roof = level->GetRoof( iRoof );

					if (roof->Hidden())
						continue;

					// Represent the profile
					const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
					const int32 botProfileCount = roof->GetBotProfilePointCount();
					const int32 topProfileCount = roof->GetTopProfilePointCount();
					const int32 botInsideCount = roof->GetBotInsidePointCount();
					const int32 topInsideCount = roof->GetTopInsidePointCount();

					const real32 levelToGround = level->GetDistanceToGround();
					const real32 baseToGround = levelToGround + roof->GetRoofMin();
					const real32 topToGround = levelToGround + roof->GetRoofMax();

					ZoneSection prevZoneSection = roof->GetRoofZoneSection(zoneSectionCount-1);
					TVector2 prevZonePos = prevZoneSection.fZonePoint->Position();
					TVector2 prevSpinePos = prevZoneSection.fSpinePoint->Position();
					for(int32 iSection=0 ; iSection<zoneSectionCount ; iSection++)
					{
						const ZoneSection& zoneSection = roof->GetRoofZoneSection(iSection);
						const TVector2& zonePos = zoneSection.fZonePoint->Position();
						const TVector2& spinePos = zoneSection.fSpinePoint->Position();

						if(zonePos != prevZonePos)
						{
							{	// Draw the handles in the bottom part
								TVector3 O,I,J,K=TVector3::kUnitZ;
							
								const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
								const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);
						
								boolean canDrawHandles = true;
								if(zoneSection.GetIsVertical())
								{	// Special case: this portion of the roof is a wall: draw the handles on the side
									if(prevSection.GetIsVertical())
										canDrawHandles = false;
									else
									{
										O = prevSection.fZonePoint->Get3DPos();
										roof->GetVerticalBase(prevSection.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),
											J,I,prevSection.fSpinePoint->Get3DPos(), zoneSection.fSpinePoint->Get3DPos());
										K = TVector3::kUnitZ;
									}
								}
								else
								{	// Normal case
									GetRoofBase(prevSection.fZonePoint->Get3DPos(), 
													zoneSection.fZonePoint->Get3DPos(), 
													roof->Flag(eRoofZoneOrientedPositive)?-1:1,
													I, J, O);
								}
								if(canDrawHandles)
								{
									// Build a base, then us x and z axi to find the pos
									for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
									{
										ProfilePoint* point = roof->GetBotProfilePoint(iPt);
										const TVector2& pos = point->Position();
										const int32 newIndex =  pointMesh.fVertex.GetElemCount(); 
										pointMesh.fVertex.AddElem(O + pos.x*J + pos.y*K);
										pointMesh.fVertexIndices.AddElem( newIndex );
										if(point->Selected()) pointMesh.fVertexColors.AddElem( selCol );
										else if(point->Targeted())	pointMesh.fVertexColors.AddElem( tarCol );
										else pointMesh.fVertexColors.AddElem( defCol );
									}

									for(int32 iIn=0 ; iIn<botInsideCount ; iIn++)
									{
										ProfilePoint* point = roof->GetBotInsidePoint(iIn);
										const TVector2& pos = point->Position();
										const int32 newIndex =  pointMesh.fVertex.GetElemCount(); 
										pointMesh.fVertex.AddElem(O + pos.x*J + pos.y*K);
										pointMesh.fVertexIndices.AddElem(newIndex);
										if(point->Selected()) pointMesh.fVertexColors.AddElem(selCol);
										else if(point->Targeted())	pointMesh.fVertexColors.AddElem(tarCol);
										else pointMesh.fVertexColors.AddElem(defCol);
									}
								}
								else
								{
									// Remove the unused handles points
								//	const int32 currentCount = pointMesh.fVertex.GetElemCount();
								//	pointMesh.fVertex.SetElemCount(currentCount-(botProfileCount+botInsideCount));
								}
							}

							{	// Draw the handles in the top part
								TVector3 O,I,J,K;
								boolean canDrawHandles = true;
								if(zoneSection.GetIsVertical())
								{	// Special case: this portion of the roof is a wall: draw the handles on the side
									const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
									const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);
							
									if(prevSection.GetIsVertical())
										canDrawHandles = false;
									else
									{
										O = prevSection.fSpinePoint->Get3DPos();
										roof->GetVerticalBase(prevSection.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),J,I,
											prevSection.fSpinePoint->Get3DPos(), zoneSection.fSpinePoint->Get3DPos());
										K = TVector3::kUnitZ;
									}
								}
								else
								{	// Normal case
									Get3DBase1(	topToGround, prevZonePos, zonePos, prevSpinePos, spinePos, O, I, J, K);
								}

								if(canDrawHandles)
								{
									for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
									{
										ProfilePoint* point = roof->GetTopProfilePoint(iPt);
										const TVector2& pos = point->Position();
										const int32 newIndex =  pointMesh.fVertex.GetElemCount(); 
										pointMesh.fVertex.AddElem(O + pos.x*J + pos.y*K);
										pointMesh.fVertexIndices.AddElem(newIndex);
										if(point->Selected()) pointMesh.fVertexColors.AddElem(selCol);
										else if(point->Targeted())	pointMesh.fVertexColors.AddElem(tarCol);
										else pointMesh.fVertexColors.AddElem(defCol);
									}

									for(int32 iIn=0 ; iIn<topInsideCount ; iIn++)
									{
										ProfilePoint* point = roof->GetTopInsidePoint(iIn);
										const TVector2& pos = point->Position();
										const int32 newIndex =  pointMesh.fVertex.GetElemCount(); 
										pointMesh.fVertex.AddElem(O + pos.x*J + pos.y*K);
										pointMesh.fVertexIndices.AddElem(newIndex);
										if(point->Selected()) pointMesh.fVertexColors.AddElem(selCol);
										else if(point->Targeted())	pointMesh.fVertexColors.AddElem(tarCol);
										else pointMesh.fVertexColors.AddElem(defCol);
									}
								}
								else
								{
									// Remove the unused handles points
								//	const int32 currentCount = pointMesh.fVertex.GetElemCount();
								//	pointMesh.fVertex.SetElemCount(currentCount-(topProfileCount+topInsideCount));
								}
							}
						}

						// Go to next section
						prevZoneSection = zoneSection;
						prevZonePos = zonePos;
						prevSpinePos = spinePos;
					}
				}
			}

			// Represent the Point Helper if there's one
			const int32 pointCount = level->GetPointCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point = level->GetPoint( iPoint );
				if (point->Flag(ePointHelper))
				{
					pointMesh.fVertex.AddElem(point->Get3DPos());
					pointMesh.fVertexIndices.AddElem(totalObjectCount);
					pointMesh.fVertexColors.AddElem(fData.fHelCol);
					break; // There's only one, no?
				}
			}
		}
	}

	GetBoundingBox(bBox);
}

void BuildingPrim::GetQuotation( Quotation& quotation, const int32 inLevel, const EUnits unit )
{
	quotation.ArrayFree();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;
	
		const int32 wallCount = level->GetWallCount();

		quotation.SetElemSpace(quotation.GetElemCount()+wallCount); // We'll allocate too much memory if some walls are hidden, but it's still more efficient

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			Dimension& newDimension = quotation.AddElem();

			const TVector2& pos0 = wall->GetPoint(0)->Position();
			const TVector2& pos1 = wall->GetPoint(1)->Position();

			const TVector2 vect = pos0-pos1;
			newDimension.fPosition = .5*(pos0+pos1);
			newDimension.fDimension = GetValueWithUnit(vect.GetNorm(), unit, false);
			newDimension.fXOriented = (RealAbs(vect.x)>RealAbs(vect.y));
		}
	}
}

boolean BuildingPrim::AutoSwitchToModeler() const
{
	return true; // (fLevelArray.GetElemCount() <= 1;
}

uint32 BuildingPrim::GetUVSpaceCount() 
{
	return fShadingDomains.GetElemCount();
}

MCCOMErr BuildingPrim::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo) 
{
	if (uvSpaceID < fShadingDomains.GetElemCount())
	{ 
		uvSpaceInfo->fID = fShadingDomains[uvSpaceID].fID;
		uvSpaceInfo->fWraparound[0] = fShadingDomains[uvSpaceID].fWraparound[0];
		uvSpaceInfo->fWraparound[1] = fShadingDomains[uvSpaceID].fWraparound[1];
		uvSpaceInfo->fName = fShadingDomains[uvSpaceID].fName;
		return MC_S_OK;
	}
	else
		return MC_S_FALSE;
}

void BuildingPrim::DefaultShadingDomainList()
{
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	fShadingDomains.ArrayFree();

	fShadingDomains.SetElemSpace(kBasicDomainsCount);
	for(int32 iDomain=0 ; iDomain<kBasicDomainsCount ; iDomain++)
	{
		UVSpaceInfo domain;
		domain.fID = iDomain;
		domain.fWraparound[0] = domain.fWraparound[1] = false;
		gResourceUtilities->GetIndString( domain.fName, kShadingDomainsStrings, iDomain+1);

		fShadingDomains.AddElem(domain);
	}
}

Wall* BuildingPrim::MakeWall( VPoint* point1, VPoint* point2, EWallType type, const int32 inLevel)
{
	if(inLevel<0)
		return NULL;

	Level* level = fLevelArray[inLevel];
	return level->MakeWall(point1, point2, type, false);
}

VPoint* BuildingPrim::MakePoint( const TVector2& pos, const int32 inLevel )
{
	if(inLevel<0)
		return NULL;

	Level* level = fLevelArray[inLevel];
	return level->MakePoint(pos);
}

void BuildingPrim::Init()
{
	// Create a default building with an empty ground floor
	TMCCountedPtr<Level> newlevel;
	Level::CreateLevel( &newlevel, this, NULL, NULL );
	MY_ASSERT(newlevel);

	fLevelArray.AddElem(newlevel);

	fGroundLevelIndex = 0;

	newlevel->SetDefaultName();

	DefaultShadingDomainList();
}

WallSubObject* BuildingPrim::MakeWallSubObject( Wall* onWall, const EObjectType objectKind )
{
	TMCCountedPtr<WallSubObject> newObject;
	WallSubObject::CreateWallSubObject(&newObject, onWall, this);
	
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCDynamicString objectName;

	newObject->SetOutline( fData.GetDefaultPolyline(objectKind) );

	switch(objectKind)
	{
	case eNarrowWindow:
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 4);
		break;
	case ePanoramicWindow:
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 5);
		break;
	case eDoor:
	case eArrowDoor:
	case e2Circle16Door:
	case e4Circle16RDoor:
	case e4Circle16LDoor:
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 6);
		break;
	case eDoubleDoor:
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 7);
		break;
	default:
		gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 3); // Window
		break;
	}
	
	newObject->SetName(fData.fDictionary, objectName);

	MY_ASSERT(newObject->GetPosCount());

	fData.InvalidateStatus();

	return newObject;
}

RoomSubObject* BuildingPrim::MakeRoomSubObject( Room* inRoom, const EObjectType objectKind )
{
	TMCCountedPtr<RoomSubObject> newObject;
	RoomSubObject::CreateRoomSubObject(&newObject, inRoom, this);

	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 11);
	newObject->SetName(fData.fDictionary, objectName);

	newObject->SetOutline( fData.GetDefaultPolyline(objectKind) );

	fData.InvalidateStatus();

	return newObject;
}

// Deleting a room or a wall does not necessarily delete the vertices under.
void BuildingPrim::DeleteSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		fLevelArray[iLevel]->LevelPlan().DeleteSelection();
	}

	InvalidateExtendedSelection(inLevel);

	// Rebuild the extended selection
	BuildExtendedSelection();
}

void BuildingPrim::InvertSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		fLevelArray[iLevel]->LevelPlan().InvertSelection();
	}

	fData.InvalidateStatus();
}

void BuildingPrim::SelectByName(const TMCString& name, const boolean select, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		fLevelArray[iLevel]->LevelPlan().SelectByName(name, select);
	}

	BuildExtendedSelection();
	fData.InvalidateStatus();
}

void BuildingPrim::SelectByDomain(const int32 domain, const boolean select, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		fLevelArray[iLevel]->LevelPlan().SelectByDomain(domain, select);
	}

	BuildExtendedSelection();
	fData.InvalidateStatus();
}

void BuildingPrim::FlipSelection(const TVector2& center, const int32 axis, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		fLevelArray[iLevel]->LevelPlan().FlipSelection(center,axis);
	}

	InvalidateExtendedSelection(inLevel);
}

// If atLevel == i, the level will be inserted at the i pos <=> under the
// previous ith level
void BuildingPrim::AddLevelToArray(Level* level, const int32 atLevel)
{
	const int32 curLevelCount = fLevelArray.GetElemCount();

	int32 levelIndex = atLevel;
	if( atLevel==kLastLevel )
		levelIndex = curLevelCount;
	else if(atLevel == kFirstLevel)
		levelIndex = 0;

	// insert the level
	{
		Level* levelOver = levelIndex<curLevelCount?fLevelArray[levelIndex]:NULL;
		Level* levelUnder = levelIndex>0?fLevelArray[levelIndex-1]:NULL;

		fLevelArray.InsertElem(levelIndex,level);
		const real32 altitude = GetAltitude(levelIndex);

		// Link levels
		if(levelUnder)
		{
			levelUnder->SetLevelOver(level);
			// invalidate the tesselation under
			levelUnder->InvalidateTessellation();
		}
		if(levelOver)
		{
			levelOver->SetLevelUnder(level);
			// invalidate the tesselation over
			levelOver->InvalidateTessellation();
		}
		level->SetLevelOver(levelOver);
		level->SetLevelUnder(levelUnder);


		// Offset the level index
		level->SetLevelIndex(levelIndex);
		for(int32 iLevel=levelIndex+1 ; iLevel<=curLevelCount ; iLevel++)
		{
			fLevelArray[iLevel]->SetLevelIndex(iLevel);
		}
		const int32 groundLevelIndex = GetGroundLevelIndex();
		if(levelIndex<=groundLevelIndex)
		{	// Inserting underground: offset
			SetGroundLevelIndex( groundLevelIndex+1 );

			level->SetDistanceToGround(altitude - level->GetLevelHeight());
		}
		else
		{
			level->SetDistanceToGround(altitude);
		}
	}

	BuildExtendedSelection();

	// Invalidate the bbox
	InvalidateBBox();
}

void BuildingPrim::RemoveLevelFromArray(const int32 index, const int32 count)
{
	const int32 groundIndex = GetGroundLevelIndex();

	for(int32 iLevel=index ; iLevel<index+count ; iLevel++)
	{
		fLevelArray[iLevel]->ReleaseReference();
	}

	fLevelArray.RemoveElem(index,count);

	// Update the level indexes and distance to ground

	const int32 levelCount = fLevelArray.GetElemCount();
	if(levelCount == 0)
	{
		fGroundLevelIndex = -1;
	}
	else if(groundIndex>=index )
	{	
		if(groundIndex<index+count)
		{	// The ground level was erased, find a new one
			if(index<levelCount && index>=0)
				fGroundLevelIndex = index;
			else if(index>0)
				fGroundLevelIndex = index-1;
		}
		else
		{
			fGroundLevelIndex -= count;
		}
	}

	for(int32 i=index ; i<levelCount ; i++)
	{
		fLevelArray[i]->SetLevelIndex(i);
	}

	BuildExtendedSelection();

	fData.InvalidateStatus();
	InvalidateBBox();
}

void BuildingPrim::SetGroundLevelIndex(const int32 index)
{
	const int32 levelCount = fLevelArray.GetElemCount();
	int32 levelIndex = index;
	if(index==kFirstLevel)
		levelIndex=0;
	else if(index==kLastLevel)
		levelIndex=levelCount-1;

	if( levelIndex<levelCount )
	{
		fGroundLevelIndex = levelIndex;
		// Invalidate the z positions
		SetAllLevelDistanceToGround();
	}
}

Level* BuildingPrim::GetGroundLevel() const
{
	if( fGroundLevelIndex>=0 && fGroundLevelIndex<(int32)fLevelArray.GetElemCount() )
	{
		return fLevelArray[fGroundLevelIndex];
	}

	return NULL;
}

const int32 BuildingPrim::GetGroundLevelIndex() const
{
	return fGroundLevelIndex;
}

const real32 BuildingPrim::GetAltitude(const int32 toLevel) const
{
	const int32 groundLevelIndex = GetGroundLevelIndex();

	const int32 levelCount = LevelCount(toLevel) - 1; // minus 1: altitude of the floor

	real32 alti=0;
	if(levelCount>groundLevelIndex)
	{
		for(int32 iLevel=groundLevelIndex ; iLevel<levelCount ; iLevel++)
		{
			alti+=fLevelArray[iLevel]->GetLevelHeight();
		}
	}
	else if(levelCount<groundLevelIndex)
	{
		for(int32 iLevel=groundLevelIndex-1 ; iLevel>=levelCount ; iLevel--)
		{
			alti-=fLevelArray[iLevel]->GetLevelHeight();
		}
	}

	return alti;
}

void BuildingPrim::MergeLevel( Level* level, const int32 levelOffset )
{
	const int32 levelIndex = level->GetLevelIndex() + levelOffset;

	const int32 curLevelCount = fLevelArray.GetElemCount();
	if(levelIndex<curLevelCount && levelIndex>=0)
	{
		fLevelArray[levelIndex]->LevelPlan().MergePlan(&level->LevelPlan());
	}
	else if(levelIndex==curLevelCount)
	{	// Add a level over
		InsertNewLevel(kLastLevel, false, eNewEmptyLevel);
		fLevelArray[curLevelCount]->LevelPlan().MergePlan(&level->LevelPlan());
	}
	else if(levelIndex==-1)
	{	// Add a level under
		InsertNewLevel(kFirstLevel, false, eNewEmptyLevel); // false: under
		fLevelArray[0]->LevelPlan().MergePlan(&level->LevelPlan());
	}
}

inline boolean BuildingPrim::AddPointSelection( const CommonPoint* point, int32& count )
{
	const TMCString& name = point->GetName();

	if(count==0)
	{
		fStatus.fFirstSelectedPos = point->Position();
			
		fStatus.fPointSelectionName = name;
	}
	else if(count==1)
	{
		fStatus.fSecondSelectedPos = point->Position();
			
		if(fStatus.fPointSelectionName!=name) fStatus.fPointSelectionName = kMultiName;
	}
	else
	{
		if(fStatus.fPointSelectionName!=name) fStatus.fPointSelectionName = kMultiName;
	}

	count++;
	return true;
}

PrimitiveStatus* BuildingPrim::GetStatus()
{
	if(!fData.IsStatusValid())
	{
		fStatus.fHasSelection = false;
		fStatus.fWallCount=0;
		fStatus.fRoomCount=0;
		fStatus.fRoofCount=0;
		fStatus.fWallObjectCount=0;
		fStatus.fRoomObjectCount=0;
	//	fStatus.fRoofObjectCount=0;
		fStatus.fPointCount=0;

		fStatus.fPartialySelectedLevelCount=0;

		fStatus.fSelectedLevelCount=0;
		fStatus.fSelectedWallGlobalCount=0;
		fStatus.fSelectedSimpleWallCount=0;
		fStatus.fSelectedWallWithCrenelCount=0;
		fStatus.fSelectedRoomCount=0;
		fStatus.fSelectedRoofCount=0;
		fStatus.fSelectedWallObjectCount=0;
		fStatus.fSelectedRoomObjectCount=0;
	//	fStatus.fSelectedRoofObjectCount=0;
		fStatus.fSelectedPointCount=0;
		fStatus.fSelectedRoofPointCount=0;
		fStatus.fSelectedProfilePointCount=0;
		fStatus.fSelectedWallHolePointCount=0;
		fStatus.fSelectedRoomHolePointCount=0;
		fStatus.fSelectedCommonPointCount=0;

		fStatus.fFirstSelectedPos=TVector2::kZero;
		fStatus.fSecondSelectedPos=TVector2::kZero;

		// Shading domains
		fStatus.fDomainRoomFloor = kNoDomains;
		fStatus.fDomainRoomCeiling = kNoDomains;
		fStatus.fDomainRoomWalls = kNoDomains;

		fStatus.fDomainWallLeft = kNoDomains;
		fStatus.fDomainWallRight = kNoDomains;
		fStatus.fDomainWallBetween = kNoDomains;

		fStatus.fDomainRoofOutTop = kNoDomains;
		fStatus.fDomainRoofOutMid = kNoDomains;
		fStatus.fDomainRoofOutBot = kNoDomains;

		// Multi selection data
		fStatus.fWallThickness = kNoValue;
		fStatus.fWallHeight = kNoValue;
		fStatus.fWallExtraHeight = kNoValue;
		fStatus.fWallOffset = kNoVecField; // can't use kNoValue because negative value are possible
		fStatus.fWallSegments = kNoValue;
		fStatus.mCrenel.mDepth = kMultiVecField;
		fStatus.mCrenel.mWidth = kMultiVecField;
		fStatus.mCrenel.mSpacing = kMultiVecField;
		fStatus.fFloorThickness = kNoValue;
		fStatus.fCeilingThickness = kNoValue;
		fStatus.fNoCeiling = false;
		fStatus.fAutoFlip = false;
		fStatus.fRoofMax = kNoValue;
		fStatus.fRoofMin = kNoValue;
		fStatus.f2DObjWidth = kNoValue;
		fStatus.f2DObjHeight = kNoValue;
		fStatus.fObjPlacement = eNoPlacement;
		fStatus.fObjOffset = kNoVector;
		fStatus.fObjScale = kNoVector;
		fStatus.fObjRotate = kNoVector;
		fStatus.fSceneObjectName = kNoName;
		fStatus.fWallSelectionName = kNoName;
		fStatus.fRoomSelectionName = kNoName;
		fStatus.fRoofSelectionName = kNoName;
		fStatus.fPointSelectionName = kNoName;
		fStatus.fObjectSelectionName = kNoName;
		fStatus.fIsSmoothed = kNoValue;

		fStatus.fLevelCount = fLevelArray.GetElemCount();
		for( int32 iLevel=0 ; iLevel<fStatus.fLevelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Selected())
			{
				fStatus.fSelectedLevelCount++;
			}
	
			const int32 wallCount = level->GetWallCount();
			fStatus.fWallCount+=wallCount;

			boolean hasSelection = false;

			for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
			{
				Wall* wall = level->GetWall(iWall);

				if (wall->Selected())
				{
					const int32 left = wall->GetLeftDomain();
					const int32 right = wall->GetRightDomain();
					const int32 between = wall->GetBetweenDomain();
					const real32 thick = wall->GetThickness();
					const real32 height = wall->GetWallHeight();
					const real32 arcOffset = wall->GetArcOffset();
					const int32 arcSegmentCount = wall->GetArcSegmentCount();
					const TMCString& name = wall->GetName();

					if(!fStatus.fSelectedWallGlobalCount)
					{
						fStatus.fDomainWallLeft = left;
						fStatus.fDomainWallRight = right;
						fStatus.fDomainWallBetween = between;

						fStatus.fWallThickness = thick;
						fStatus.fWallHeight = height;

						fStatus.fWallOffset = arcOffset;
						fStatus.fWallSegments = arcSegmentCount;
	
						fStatus.fWallSelectionName = name;
					}
					else
					{
						if(fStatus.fDomainWallLeft!=left) fStatus.fDomainWallLeft = kMultipleDomains;
						if(fStatus.fDomainWallRight!=right) fStatus.fDomainWallRight = kMultipleDomains;
						if(fStatus.fDomainWallBetween!=between) fStatus.fDomainWallBetween = kMultipleDomains;
						if(fStatus.fWallThickness!=thick) fStatus.fWallThickness = kMultipleValues;
						if(fStatus.fWallHeight!=height) fStatus.fWallHeight = kMultipleValues;
						if(fStatus.fWallOffset!=arcOffset) fStatus.fWallOffset = kMultiVecField;
						if(fStatus.fWallSegments!=arcSegmentCount) fStatus.fWallSegments = kMultipleValues;
						if(fStatus.fWallSelectionName!=name) fStatus.fWallSelectionName = kMultiName;
					}

					// Check what kind of wall it is
					WallWithCrenel* crenel = wall->GetWallWithCrenel();
					if( crenel )
					{
						real32 crenelDepth = crenel->GetCrenelHeight();
						real32 crenelWidth = crenel->GetCrenelWidth();
						real32 crenelSpacing = crenel->GetCrenelSpacing();
						real32 crenelOffset = crenel->GetCrenelOffset();
						ECrenelShape crenelShape = crenel->GetCrenelShape();

						if( !fStatus.fSelectedWallWithCrenelCount )
						{
							fStatus.mCrenel.mDepth=crenelDepth;
							fStatus.mCrenel.mWidth=crenelWidth;
							fStatus.mCrenel.mSpacing=crenelSpacing;
							fStatus.mCrenel.mOffset=crenelOffset;
							fStatus.mCrenel.mShape=crenelShape;
						}
						else
						{
							if(fStatus.mCrenel.mDepth!=crenelDepth) fStatus.mCrenel.mDepth = kMultiVecField;
							if(fStatus.mCrenel.mWidth!=crenelWidth) fStatus.mCrenel.mWidth = kMultiVecField;
							if(fStatus.mCrenel.mSpacing!=crenelSpacing) fStatus.mCrenel.mSpacing = kMultiVecField;
							if(fStatus.mCrenel.mOffset!=crenelOffset) fStatus.mCrenel.mOffset = kMultiVecField;
							if(fStatus.mCrenel.mShape!=crenelShape) fStatus.mCrenel.mShape = kMultipleValues;
						}


						fStatus.fSelectedWallWithCrenelCount++;
					}
					else
					{
						const int32 extraHeight = wall->Flag(eWallExtraHeight)?1:0;
						if(!fStatus.fSelectedSimpleWallCount)
						{
							fStatus.fWallExtraHeight = extraHeight;
						}
						else
						{
							if(fStatus.fWallExtraHeight!=extraHeight) fStatus.fWallExtraHeight = kMultipleValues;
						}

						fStatus.fSelectedSimpleWallCount++;
					}

					fStatus.fSelectedWallGlobalCount++;
					hasSelection=true;
				}

				const int32 objCount = wall->GetObjectCount();
				fStatus.fWallObjectCount+=objCount;
				for( int32 iObj=0 ; iObj< objCount ; iObj++ )
				{
					WallSubObject* obj = wall->GetObject( iObj );
					if( obj->Selected() )
					{
						fStatus.AddObj(obj);

						fStatus.fSelectedWallObjectCount++;
						hasSelection=true;
					}
					// Get the selection status on the outline
					const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
					const int32 ptCount = outline.GetElemCount();
					boolean prevSelected = outline[ptCount-1]->Selected();
					for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					{
						const boolean curSelected = outline[iPt]->Selected();

						if(curSelected)
							hasSelection=AddPointSelection(outline[iPt], fStatus.fSelectedWallHolePointCount);

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						if(curSelected)
						{
							if(fStatus.fIsSmoothed == kNoValue)
								fStatus.fIsSmoothed = (int32)outline[iPt]->IsSmoothed();
							else if(fStatus.fIsSmoothed != (int32)outline[iPt]->IsSmoothed())
								fStatus.fIsSmoothed = kMultipleValues;
						}

						prevSelected = curSelected;
					}
				}
			}

			const int32 roomCount = level->GetRoomCount();
			fStatus.fRoomCount+=roomCount;

			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom(iRoom);

				if (room->Selected())
				{
					const int32 floor = room->GetFloorDomain();
					const int32 ceiling = room->GetCeilingDomain();
					const int32 walls = room->GetWallsDomain();

					const real32 floorThick = room->GetFloorThickness();
					const real32 ceilingThick = room->GetCeilingThickness();
				
					const TMCString& name = room->GetName();

					fStatus.fNoCeiling |= room->NoCeiling();

					if(!fStatus.fSelectedRoomCount)
					{
						fStatus.fDomainRoomFloor = floor;
						fStatus.fDomainRoomCeiling = ceiling;
						fStatus.fDomainRoomWalls = walls;
						fStatus.fFloorThickness = floorThick;
						fStatus.fCeilingThickness = ceilingThick;
	
						fStatus.fRoomSelectionName = name;
					}
					else
					{
						if(fStatus.fDomainRoomFloor!=floor) fStatus.fDomainRoomFloor = kMultipleDomains;
						if(fStatus.fDomainRoomCeiling!=ceiling) fStatus.fDomainRoomCeiling = kMultipleDomains;
						if(fStatus.fDomainRoomWalls!=walls) fStatus.fDomainRoomWalls = kMultipleDomains;
						if(fStatus.fFloorThickness!=floorThick) fStatus.fFloorThickness = kMultipleValues;
						if(fStatus.fCeilingThickness!=ceilingThick) fStatus.fCeilingThickness = kMultipleValues;
						if(fStatus.fRoomSelectionName!=name) fStatus.fRoomSelectionName = kMultiName;
					}

					fStatus.fSelectedRoomCount++;
					hasSelection=true;
				}

				const int32 objCount = room->GetObjectCount();	
				fStatus.fRoomObjectCount+=objCount;
				for( int32 iObj=0 ; iObj< objCount ; iObj++ )
				{
					RoomSubObject* obj = room->GetObject( iObj );
					if( obj->Selected() )
					{
						fStatus.AddObj(obj);

						fStatus.fSelectedRoomObjectCount++;
						hasSelection=true;
					}
					// Get the selection status on the outline
					const TMCCountedPtrArray<OutlinePoint>& outline = obj->GetOutline();
					const int32 ptCount = outline.GetElemCount();
					boolean prevSelected = outline[ptCount-1]->Selected();
					for(int32 iPt=0 ; iPt<ptCount ; iPt++)
					{
						const boolean curSelected = outline[iPt]->Selected();

						if(curSelected)
							hasSelection=AddPointSelection(outline[iPt], fStatus.fSelectedRoomHolePointCount);

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						if(curSelected)
						{
							if(fStatus.fIsSmoothed == kNoValue)
								fStatus.fIsSmoothed = (int32)outline[iPt]->IsSmoothed();
							else if(fStatus.fIsSmoothed != (int32)outline[iPt]->IsSmoothed())
								fStatus.fIsSmoothed = kMultipleValues;
						}

						prevSelected = curSelected;
					}
				}
			}

			const int32 roofCount = level->GetRoofCount();
			fStatus.fRoofCount+=roofCount;

			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);

				if (roof->Selected())
				{
					const int32 top = roof->GetOutTopDomain();
					const int32 mid = roof->GetOutMidDomain();
					const int32 bot = roof->GetOutBotDomain();
					const int32 in = roof->GetInsideDomain();

					const real32 roofMax = roof->GetRoofMax();
					const real32 roofMin = roof->GetRoofMin();

					const TMCString& name = roof->GetName();

					if(!fStatus.fSelectedRoofCount)
					{
						fStatus.fDomainRoofOutTop = top;
						fStatus.fDomainRoofOutMid = mid;
						fStatus.fDomainRoofOutBot = bot;
						fStatus.fDomainRoofInside = in;
				
						fStatus.fRoofMax = roofMax;
						fStatus.fRoofMin = roofMin;
	
						fStatus.fRoofSelectionName = name;
					}
					else
					{
						if(fStatus.fDomainRoofOutTop!=top) fStatus.fDomainRoofOutTop = kMultipleDomains;
						if(fStatus.fDomainRoofOutMid!=mid) fStatus.fDomainRoofOutMid = kMultipleDomains;
						if(fStatus.fDomainRoofOutBot!=bot) fStatus.fDomainRoofOutBot = kMultipleDomains;
						if(fStatus.fDomainRoofInside!=in) fStatus.fDomainRoofInside = kMultipleDomains;
						if(fStatus.fRoofMax!=roofMax) fStatus.fRoofMax = kMultipleValues;
						if(fStatus.fRoofMin!=roofMin) fStatus.fRoofMin = kMultipleValues;
						if(fStatus.fRoofSelectionName!=name) fStatus.fRoofSelectionName = kMultiName;
					}

					fStatus.fSelectedRoofCount++;
					hasSelection=true;
				}

				const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

				if(zoneSectionCount)
				{
					const ZoneSection& lastSection = roof->GetRoofZoneSection(zoneSectionCount-1);
					boolean prevSelected = lastSection.fZonePoint->Selected() || lastSection.fSpinePoint->Selected();
					for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
					{
						const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

						boolean curSelected = false;

						if(zoneSection.fZonePoint->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(zoneSection.fZonePoint, fStatus.fSelectedRoofPointCount);
						}

						if(zoneSection.fSpinePoint->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(zoneSection.fSpinePoint, fStatus.fSelectedRoofPointCount);
						}

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						prevSelected = curSelected;
					}
				}

				const int32 botCount = roof->GetBotProfilePointCount();
				if(botCount)
				{
					boolean prevSelected = roof->GetBotProfilePoint(botCount-1)->Selected();
					for(int32 iBot=0 ; iBot<botCount ; iBot++)
					{
						boolean curSelected = false;
						ProfilePoint* point = roof->GetBotProfilePoint(iBot);
						if( point->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(point, fStatus.fSelectedProfilePointCount);
						}

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						prevSelected = curSelected;
					}
				}

				const int32 topCount = roof->GetTopProfilePointCount();
				if(topCount)
				{
					boolean prevSelected = roof->GetTopProfilePoint(topCount-1)->Selected();
					for(int32 iTop=0 ; iTop<topCount ; iTop++)
					{
						boolean curSelected = false;
						ProfilePoint* point = roof->GetTopProfilePoint(iTop);
						if( point->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(point, fStatus.fSelectedProfilePointCount);
						}

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						prevSelected = curSelected;
					}
				}

				const int32 bInCount = roof->GetBotInsidePointCount();
				if(bInCount)
				{
					boolean prevSelected = roof->GetBotInsidePoint(bInCount-1)->Selected();
					for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
					{
						boolean curSelected = false;
						ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
						if( point->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(point, fStatus.fSelectedProfilePointCount);
						}

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						prevSelected = curSelected;
					}
				}

				const int32 tInCount = roof->GetTopInsidePointCount();
				if(tInCount)
				{
					boolean prevSelected = roof->GetTopInsidePoint(tInCount-1)->Selected();
					for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
					{
						boolean curSelected = false;
						ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
						if( point->Selected())
						{
							curSelected = true;
							hasSelection=AddPointSelection(point, fStatus.fSelectedProfilePointCount);
						}

						if(prevSelected && curSelected)
							fStatus.fCouldSplit = true;

						prevSelected = curSelected;
					}
				}

			//	const int32 objCount = roof->GetObjectCount();	
			//	fStatus.fRoofObjectCount+=objCount;
			//	for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			//	{
			//		RoofSubObject* obj = roof->GetObject( iObj );
			//		if( obj->Selected() )
			//		{
			//			fStatus.AddObj(obj);
			//			fStatus.fSelectedRoofObjectCount++;
			//			hasSelection=true;
			//		}
			//	}
			}

			const int32 pointCount = level->GetPointCount();
			fStatus.fPointCount+=pointCount;

			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point = level->GetPoint(iPoint);

				if (point->Selected())
					hasSelection=AddPointSelection(point, fStatus.fSelectedPointCount);
			}

			if(hasSelection)
			{
				fStatus.fPartialySelectedLevelCount++;

				fStatus.fHasSelection = true;
			}
		}

		fStatus.fSelectedCommonPointCount = fStatus.fSelectedPointCount+
											fStatus.fSelectedProfilePointCount+
											fStatus.fSelectedRoofPointCount +
											fStatus.fSelectedWallHolePointCount +
											fStatus.fSelectedRoomHolePointCount;

		if(fStatus.fSelectedWallGlobalCount == 1)
			fStatus.fCouldSplit=true;

		if(fStatus.fCouldSplit)
		{	// keep this flag only if that's all
			if( fStatus.fSelectedPointCount!=2 &&
				fStatus.fSelectedProfilePointCount!=2 &&
				fStatus.fSelectedRoofPointCount !=2 &&
				fStatus.fSelectedWallHolePointCount !=2 &&
				fStatus.fSelectedRoomHolePointCount !=2 )
				fStatus.fCouldSplit = false;
		}

		// Get the bbox
		GetBoundingBox( fStatus.fSelectionBBox, false, true );

		fData.SetStatusIsValid();
	}

	return &fStatus;
}

Wall* BuildingPrim::GetFirstSelectedWall( const int32 inLevel )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Selected())
			{
				return wall;
			}
		}
	}

	return NULL;
}

Room* BuildingPrim::GetFirstSelectedRoom( const int32 inLevel )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if (room->Selected())
			{
				return room;
			}
		}
	}

	return NULL;
}

Roof* BuildingPrim::GetFirstSelectedRoof( const int32 inLevel )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if (roof->Selected())
			{
				return roof;
			}
		}
	}

	return NULL;
}

CommonPoint* BuildingPrim::GetFirstSelectedPoint( const int32 inLevel )
{
	return GetSelectedPoint(0,inLevel);
}

CommonPoint* BuildingPrim::GetSecondSelectedPoint( const int32 inLevel )
{
	return GetSelectedPoint(1,inLevel);
}

CommonPoint* BuildingPrim::GetSelectedPoint( const int32 pointIndex, const int32 inLevel )
{	
	int32 counter = 0;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		const int32 roofCount = level->GetRoofCount();
		fStatus.fRoofCount+=roofCount;

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);


			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			if(zoneSectionCount)
			{
				const ZoneSection& lastSection = roof->GetRoofZoneSection(zoneSectionCount-1);
				boolean prevSelected = lastSection.fZonePoint->Selected() || lastSection.fSpinePoint->Selected();
				for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
				{
					const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

					if(zoneSection.fZonePoint->Selected() && counter==pointIndex)
						return zoneSection.fZonePoint;
					else
						counter++;

					if(zoneSection.fSpinePoint->Selected() && counter==pointIndex)
						return zoneSection.fSpinePoint;
					else
						counter++;
				}
			}

			const int32 botCount = roof->GetBotProfilePointCount();
			if(botCount)
			{
				for(int32 iBot=0 ; iBot<botCount ; iBot++)
				{
					ProfilePoint* point = roof->GetBotProfilePoint(iBot);
					if( point->Selected() && counter==pointIndex)
						return point;
					else
						counter++;
				}
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			if(topCount)
			{
				for(int32 iTop=0 ; iTop<topCount ; iTop++)
				{
					ProfilePoint* point = roof->GetTopProfilePoint(iTop);
					if( point->Selected())
						return point;
					else
						counter++;
				}
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			if(bInCount)
			{
				for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
				{
					ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
					if( point->Selected())
						return point;
					else
						counter++;
				}
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			if(tInCount)
			{
				for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
				{
					ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
					if( point->Selected())
						return point;
					else
						counter++;
				}
			}
		}

		const int32 pointCount = level->GetPointCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point = level->GetPoint(iPoint);

			if (point->Selected() && counter==pointIndex)
				return point;
			else
				counter++;
		}
	}

	return NULL;
}


VPoint* BuildingPrim::GetPointHelper(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		VPoint* point = level->LevelPlan().GetPointHelper();

		if(point)
			return point;
	}

	return NULL;
}

void BuildingPrim::InvalidateSelection(const boolean andBBox, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().InvalidateExtendedSelection();
	}

	// Invalidate the BBox at the same time
	if(andBBox)
		InvalidateBBox();
	
	// Selection invalide, so status too.			
	fData.InvalidateStatus();
				
}

// This method need to be called each time the selection is modified somewhere in the database
void BuildingPrim::BuildExtendedSelection()
{
	const int32 levelCount = fLevelArray.GetElemCount();

	TMCArray<int32> overAndUnder;
	{
		for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			if( level->LevelPlan().HasPointSelection() )
			{
				if( iLevel>0 &&
					overAndUnder.FindElem(iLevel-1) == kUnusedIndex )
				{
					overAndUnder.AddElem(iLevel-1);
				}
				if( iLevel+1<levelCount &&
					overAndUnder.FindElem(iLevel+1) == kUnusedIndex )
				{
					overAndUnder.AddElem(iLevel+1);
				}
			}

			// Roof points (and set the extended selction on the roof at the same time)
			if( level->LevelPlan().CheckRoofPointSelection() )
			{
				if( overAndUnder.FindElem(iLevel) == kUnusedIndex )
					overAndUnder.AddElem(iLevel);
			}
		}
	}
	{
		for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			if(overAndUnder.FindElem(iLevel) != kUnusedIndex)
			{
				level->LevelPlan().AddWallsAndRoomsToExtendedSelection();
			}
			else
			{
				// And set the one arround the selected points
				level->LevelPlan().RestoreWallsAndRoomsExtendedSelection();
			}
		}
	}

}

void BuildingPrim::InvalidateExtendedSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().InvalidateExtendedSelection();
	}

	// Invalidate the BBox at the same time
	InvalidateBBox();
					
	fData.InvalidateStatus();			
}

void BuildingPrim::InvalidateAll(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().InvalidateAll();
	}

	// Invalidate the BBox at the same time
	InvalidateBBox();
}

// Check the topology of the extended selection:
// Verify that no wall are going through room objects
// Verify that the room objects are still in their room
// Maybe later: snap walls, points, ...
void BuildingPrim::CheckExtendedSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().CheckExtendedSelection();
	}
}

void BuildingPrim::InvalidateObjectSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().InvalidateObjectSelection();
	}

	// Invalidate the BBox at the same time
	InvalidateBBox();
}

void BuildingPrim::SetSelectionHelper( const boolean set, const int32 inLevel )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().SetSelectionHelper(set);
	}
}

void BuildingPrim::CheckSelectionConsistency( const int32 inLevel )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().CheckSelectionConsistency();
	}
}

void BuildingPrim::SetWallOffset(real32 offset, boolean computeSegCount, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().SetWallOffset(offset,computeSegCount);
	}
}

void BuildingPrim::SetSelectionPos( const TMCClassArray<TVector2>& positions, const int32 inLevel )
{
	int32 index=0;
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 pointCount = level->GetPointCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point = level->GetPoint(iPoint);

			if (point->Selected())
			{
				point->SetPosition(positions[index++]);
			}
		}

		// Set the Roofs points
		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			for(int32 iSc=0 ; iSc<zoneSectionCount ; iSc++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iSc);

				if(zoneSection.fZonePoint->Selected())
				{
					zoneSection.fZonePoint->SetPosition(positions[index++]);
				}
				if(zoneSection.fSpinePoint->Selected())
				{
					zoneSection.fSpinePoint->SetPosition(positions[index++]);
				}
			}

			const int32 botCount = roof->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount ; iBot++)
			{
				ProfilePoint* point = roof->GetBotProfilePoint(iBot);
				if( point->Selected())
					point->SetPosition(positions[index++]);
			}

			const int32 topCount = roof->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount ; iTop++)
			{
				ProfilePoint* point = roof->GetTopProfilePoint(iTop);
				if( point->Selected())
					point->SetPosition(positions[index++]);
			}

			const int32 bInCount = roof->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			{
				ProfilePoint* point = roof->GetBotInsidePoint(iBIn);
				if( point->Selected())
					point->SetPosition(positions[index++]);
			}

			const int32 tInCount = roof->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			{
				ProfilePoint* point = roof->GetTopInsidePoint(iTIn);
				if( point->Selected())
					point->SetPosition(positions[index++]);
			}
		}
	}
}

void BuildingPrim::SetSelectionObjPos( const TMCClassArray<TVector2>& objPpositions, const int32 inLevel )
{
	int32 index=0;
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		const int32 roomCount = level->GetRoomCount();

		real32 objIndex=0;
		for(int32 iRoom=0 ; iRoom<roomCount ; iRoom++)
		{
			const Room* room = level->GetRoom(iRoom);

			const int32 objCount = room->GetObjectCount();
			for(int32 iObj=0 ; iObj<objCount ; iObj++)
			{
				RoomSubObject* obj = room->GetObject(iObj);
				if(obj->Selected())
				{
					obj->SetPolylineCenter(objPpositions[objIndex++], false);
				}
			}
		}
	}
}

void BuildingPrim::OffsetSelection(const TVector2& offset, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().OffsetSelection(offset);
	}
}

// Each part of the building is scaled in its own plane
void BuildingPrim::ScaleSelection(const TVector2& scale, 
								  const TVector2& center, 
								  const EOptionMode mode, 
								  const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().ScaleSelection(scale,center,mode);
	}
}

// Each part of the building is rotated in its own plane
void BuildingPrim::RotateSelection(const TVector2& cosSin, const TVector2& center, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().RotateSelection(cosSin,center);
	}
}

void BuildingPrim::OffsetWallObjJump(const TUnitQuaternion& rot, 
									 const TVector3& offset, 
									 const real32 originDist, 
									 const int32 inLevel,
									 const boolean constrainDir,
									 const boolean constrainDist )
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	// Note: we're doing all the job at the BuildingPrim level because object can
	// jump on other walls during the process
	TMCCountedPtrArray<WallSubObject> wallObjToModify;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall< wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall( iWall );

			if (wall->Hidden())
				continue;

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				WallSubObject* obj = wall->GetObject( iObj );
				if( obj->Selected() )
				{
					wallObjToModify.AddElem(obj);
				}
			}
		}
	}

	{
		const int32 wallObjRecCount = wallObjToModify.GetElemCount();

		for( int32 iObj=0 ; iObj< wallObjRecCount ; iObj++ )
		{
			WallSubObject* obj = wallObjToModify[iObj];
			Wall* wall = obj->GetOnWall();

			// Offset the position of this obj. If when using the
			// offset the obj get out of the wall, find another
			// wall using the direction vector to do a picking
			{	// We're ouside the wall, find another one
				TVector3 direction;
				TVector3 O,I,J,K;
				wall->GetBase(O,I,J,K);
				//TUnitQuaternion normal(K.x, K.y, K.z, 0);
				//TUnitQuaternion resultQ = (rot*normal)*(rot.Inverse());
				//resultQ.GetAxis(direction);

				rot.Transform(K, direction);

				TVector3 center;
				obj->GetCenter(center);
				TVector3 newCenter = center+offset;
			
				TVector3 newPickPos;
				const TVector3 origin = newCenter-originDist*direction; // Get back a little bit to do a proper picking
				Wall* onWall=BasicPickBestWall(origin,direction,newPickPos,kAllLevels);
				if(!onWall)
				{	// Find the nearest one
					onWall = ProjectOnNearestWall(newCenter);
				}
				else
				{
					newCenter = newPickPos;
				}

				if(onWall)
				{	// put the object on this new wall
					Wall* prevWall = obj->GetOnWall();
					obj->SetOnWall(onWall);
					if(constrainDir)
					{
						const TVector3 finalOffset = newCenter-center;
						const real32 hor = finalOffset.CastToXY().GetSquaredNorm();
						const real32 ver = finalOffset.z*finalOffset.z;
						if(ver>hor)
						{
							newCenter.x = center.x;
							newCenter.y = center.y;

							
							if(constrainDist)
							{	// constrain to the 1/12 of the height
								const real32 refHeight = onWall->GetLevel()->GetDistanceToGround();
								const real32 wallHeight = onWall->GetWallHeight();
								const real32 curHeight = newCenter.z-refHeight;
								newCenter.z = refHeight+Snap(curHeight,12,wallHeight);
							}
						}
						else
						{
							// If we're not in the same floor anymore, offset from the difference
							const real32 offset =	prevWall->GetLevel()->GetDistanceToGround() -
													onWall->GetLevel()->GetDistanceToGround();
							newCenter.z = center.z-offset;
						
							if(constrainDist)
							{	// constrain to the 1/12 of the length
								const TVector2& origin = onWall->GetPoint(0)->Position();
								const TVector2& end = onWall->GetPoint(1)->Position();
								const TVector2 curPos(newCenter.x,newCenter.y);
								const real32 wallLength = onWall->GetStraightLength();
								const real32 curLength = (curPos-origin).GetNorm();
								const real32 snapLength = Snap(curLength,12,wallLength);
								TVector2 snapPos = origin + (snapLength/wallLength)*(end-origin);
								newCenter.x = snapPos.x;
								newCenter.y = snapPos.y;
							}
						}
					}

					if( !obj->SetCenter(newCenter, false) )// false: do not check for avaible space
					{	// the new pos couldn't be set=>restore the previous wall
						obj->SetOnWall(prevWall);
					}
				}
			}
		}
	}
}

void BuildingPrim::OffsetObj(const TVector2& offset, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	TMCCountedPtrArray<WallSubObject> wallObjToModify;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().OffsetObj(offset);
	}
}

void BuildingPrim::ScaleObj(const TVector2& scale, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	TMCCountedPtrArray<WallSubObject> wallObjToModify;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().ScaleObj(scale);
	}
}

void BuildingPrim::RotateObj(const TVector2& cosSin, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	TMCCountedPtrArray<WallSubObject> wallObjToModify;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().RotateObj(cosSin);
	}
}

void BuildingPrim::OffsetRoomObjJump(const TVector2& projOffset, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	TMCCountedPtrArray<RoomSubObject> roomObjToModify;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom< roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom( iRoom );

			if (room->Hidden())
				continue;

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				RoomSubObject* obj = room->GetObject( iObj );
				if( obj->Selected() )
				{
					roomObjToModify.AddElem(obj);
				}
			}
		}
	}

	{
		const int32 roomObjRecCount = roomObjToModify.GetElemCount();

		for( int32 iObj=0 ; iObj< roomObjRecCount ; iObj++ )
		{
			RoomSubObject* obj = roomObjToModify[iObj];
			Room* room = obj->GetInRoom();

			// Offset the position of this obj. The object might get out
			// of the room during this process so first determine in which 
			// room we're gonna be
			TVector2 polylineCenter = obj->GetPolylineCenter();
			polylineCenter+=projOffset;

			Room* inRoom = room->GetLevel()->LevelPlan().PosInRoom(polylineCenter);

			if(inRoom == room)
			{
				// we're still in the same room, just offset the object
				obj->OffsetPolyline(projOffset,true);
			}
			else if(inRoom)
			{	// We're in a new room, set it before offsetting
				Room* prevRoom = obj->GetInRoom();
				obj->SetInRoom(inRoom);
				if( !obj->SetPolylineCenter(polylineCenter, true) )// true: check for avaible space
				{	// the new pos couldn't be set=>restore the previous room
					obj->SetInRoom(prevRoom);
				}
			}
			else
			{	// We're nowhere, find the nearest room
				inRoom = room->GetLevel()->LevelPlan().GetNearestRoom(polylineCenter);
				if(inRoom)
				{	// Set this room before offseting the polyline
					Room* prevRoom = obj->GetInRoom();
					obj->SetInRoom(inRoom);
					if( !obj->SetPolylineCenter(polylineCenter, true) )// true: check for avaible space
					{	// the new pos couldn't be set=>restore the previous room
						obj->SetInRoom(prevRoom);
					}
				}
			}
		}
	}
}

void BuildingPrim::InsertNewLevel(const int32 inWhere, 
								  const boolean over, 
								  const int32 type)
{
	if(!CanCreateLevel(GetLevelCount()))
		return; // DEMO VERSION: limited number of levels

	// Create a new level
	TMCCountedPtr<Level> newLevel;
	Level::CreateLevel( &newLevel, this, NULL, NULL );
	MY_ASSERT(newLevel);

	switch(type)
	{
	case eNewEmptyLevel: break;
	case eNewDuplicateUnder:
		{
			const int32 curIndex = LevelIndex(inWhere);
			fLevelArray[curIndex>=0?curIndex:0]->Clone(&newLevel,this,eCloneChild); // Create new children instance for the new level
		} break;
	case eNewDuplicateShellUnder:
		{
			// Make 4 walls using the bbox of the level under to place them
			const int32 curIndex = LevelIndex(inWhere);
			Level* refLevel = fLevelArray[curIndex>=0?curIndex:0];
			TBBox3D levelBBox;
			refLevel->GetBoundingBox(levelBBox, false, false );
			if(levelBBox.Valid() && !levelBBox.IsEmpty())
			{
				const real32 xMin = levelBBox.fMin.x;
				const real32 yMin = levelBBox.fMin.y;
				const real32 xMax = levelBBox.fMax.x;
				const real32 yMax = levelBBox.fMax.y;
				TVector2 pos1(xMin,yMin); 
				TVector2 pos2(xMin,yMax); 
				Wall* wall1 = newLevel->MakeWall(pos1, pos2);
				TVector2 pos3(xMax,yMin); 
				TVector2 pos4(xMax,yMax); 
				Wall* wall2 = newLevel->MakeWall(pos3, pos4);

				newLevel->MakeWall(wall1->GetPoint(0), wall2->GetPoint(0), eBasic, false);
				newLevel->MakeWall(wall1->GetPoint(1), wall2->GetPoint(1), eBasic, true);
			}
		} break;
	}

	// Insert it in the array
	AddLevelToArray(newLevel, over?inWhere+1:inWhere);

	// Make its default name
	newLevel->SetDefaultName();
}

void BuildingPrim::SetWallHeight(const real32 height, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( wall->Selected() )
			{
				undoData.AddElem(wall->GetWallHeight());
				wall->SetWallHeight(height);
			}
		}
	}
}

void BuildingPrim::SetWallThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( wall->Selected() )
			{
				undoData.AddElem(wall->GetThickness());
				wall->SetThickness(thickness);
			}
		}
	}
}

void BuildingPrim::SetFloorThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( room->Selected() )
			{
				undoData.AddElem(room->GetFloorThickness());
				room->SetFloorThickness(thickness);
			}
		}
	}
}

void BuildingPrim::SetCeilingThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( room->Selected() )
			{
				undoData.AddElem(room->GetCeilingThickness());
				room->SetCeilingThickness(thickness);
			}
		}
	}
}

void BuildingPrim::SetRoofMax(const real32 max, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		boolean invalidate = false;

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( roof->Selected() )
			{
				undoData.AddElem(roof->GetRoofMax());
				roof->SetRoofMax(max);
				invalidate = true;
			}
		}

		if(invalidate)
			InvalidateExtendedSelection(iLevel);
	}
}

void BuildingPrim::SetRoofMin(const real32 min, TMCArray<real32>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		boolean invalidate = false;

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( roof->Selected() )
			{
				undoData.AddElem(roof->GetRoofMin());
				roof->SetRoofMin(min);
				invalidate = true;
			}
		}

		if(invalidate)
			InvalidateExtendedSelection(iLevel);
	}
}

void BuildingPrim::SetNoCeiling(const boolean noCeiling, TMCArray<boolean>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		boolean invalidate = false;

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( room->Selected() )
			{
				invalidate = true;
				undoData.AddElem(room->NoCeiling());
				room->NoCeiling(noCeiling);
			}
		}

		if(invalidate)
			InvalidateExtendedSelection(iLevel);
	}
}

void BuildingPrim::SetWallExtraHeight(const boolean extraHeight, TMCArray<boolean>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		boolean invalidate = false;

		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( wall->Selected() )
			{
				invalidate = true;
				undoData.AddElem(wall->ExtraHeight());
				wall->ExtraHeight(extraHeight);
			}
		}

		if(invalidate)
			InvalidateExtendedSelection(iLevel);
	}
}

void BuildingPrim::SetAutoFlipObjects(const boolean autoFlip, TMCArray<boolean>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				SubObject* obj = room->GetObject( iObj );

				if( obj->Selected() )
				{
					undoData.AddElem(obj->AutoFlip());
					obj->AutoFlip(autoFlip);
					obj->Invalidate();
				}
			}
		}

		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				SubObject* obj = wall->GetObject( iObj );

				if( obj->Selected() )
				{
					undoData.AddElem(obj->AutoFlip());
					obj->AutoFlip(autoFlip);
					obj->Invalidate();
				}
			}
		}
	}
}

boolean SetCommonObjectName(CommonBase* common,
						 const TMCString& name,
						 NameChainedList& dictionnary,
						 TMCClassArray<TMCDynamicString>& undoData)
{
	if (common->Hidden())
		return false;

	if( common->Selected() )
	{
		undoData.AddElem(common->GetName());
		common->SetName(dictionnary, name);
		return true;
	}

	return false;
}

void BuildingPrim::SetSelectionName(const TMCString& name, TMCClassArray<TMCDynamicString>& undoData, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	boolean nameSet=false;

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		// Wall and wall objects
		const int32 wallCount = level->GetWallCount();
		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);
			nameSet|=SetCommonObjectName( wall, name, fData.fDictionary, undoData);

			const int32 objCount = wall->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				nameSet|=SetCommonObjectName( wall->GetObject( iObj ), name, fData.fDictionary, undoData);
			}
		}

		// Room and room objects
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);
			nameSet|=SetCommonObjectName( room, name, fData.fDictionary, undoData);

			const int32 objCount = room->GetObjectCount();
			for( int32 iObj=0 ; iObj< objCount ; iObj++ )
			{
				nameSet|=SetCommonObjectName( room->GetObject( iObj ), name, fData.fDictionary, undoData);
			}
		}

		// Roof 
		const int32 roofCount = level->GetRoofCount();
		{
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				nameSet|=SetCommonObjectName( level->GetRoof(iRoof), name, fData.fDictionary, undoData);
			}
		}

		// Roof points (zone and profile)
		if(!nameSet)
		{
			for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
			{
				Roof* roof = level->GetRoof(iRoof);

				{
					const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
					for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
					{
						const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);
						SetCommonObjectName( zoneSection.fZonePoint, name, fData.fDictionary, undoData);
						SetCommonObjectName( zoneSection.fSpinePoint, name, fData.fDictionary, undoData);
					}
				}

				{
					const int32 botProfileCount = roof->GetBotProfilePointCount();
					for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
					{
						SetCommonObjectName( roof->GetBotProfilePoint(iPt), name, fData.fDictionary, undoData);
					}
				}

				{
					const int32 topProfileCount = roof->GetTopProfilePointCount();
					for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
					{
						SetCommonObjectName( roof->GetTopProfilePoint(iPt), name, fData.fDictionary, undoData);
					}
				}

				{
					const int32 botInsideCount = roof->GetBotInsidePointCount();
					for(int32 iPt=0 ; iPt<botInsideCount ; iPt++)
					{
						SetCommonObjectName( roof->GetBotInsidePoint(iPt), name, fData.fDictionary, undoData);
					}
				}

				{
					const int32 topInsideCount = roof->GetTopInsidePointCount();
					for(int32 iPt=0 ; iPt<topInsideCount ; iPt++)
					{
						SetCommonObjectName( roof->GetTopInsidePoint(iPt), name, fData.fDictionary, undoData);
					}
				}
			}

			// Points
			const int32 pointCount = level->GetPointCount();
			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				SetCommonObjectName( level->GetPoint(iPoint), name, fData.fDictionary, undoData);
			}
		}
	}
}

void BuildingPrim::SetRoofProfile(const ERoofProfileID profile, const boolean onTop, const boolean inside, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		boolean invalidate = false;

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( roof->Selected() )
			{
				if(onTop)
				{
					if(inside)
						roof->SetTopInside(profile);
					else
						roof->SetTopProfile(profile);
				}
				else
				{
					if(inside)
						roof->SetBotInside(profile);
					else
						roof->SetBotProfile(profile);
				}

				invalidate = true;
			}
		}

		if(invalidate)
		{
			InvalidateBBox();
			InvalidateExtendedSelection(iLevel);
		}
	}
}

void BuildingPrim::Split(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().Split();
	}
}

void BuildingPrim::Merge(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().Merge();
	}
}

EPickedType	BuildingPrim::Pick(CommonBase **object,
							   int32 flags, 
							   FatRay& fatRay, 
							   TVector3& hitPosition,
							   const int32 inLevel, 
							   const boolean inPlanView)
{
	int32 useLevel=inLevel;
	if( !GetActualLevel(useLevel,inLevel) )
		return eNothingPicked;

	if(flags&ePickLevel)
	{	// Level picking => even if inLevel!=kAllLevels we need to do
		// the picking to know if we're in the top or bottom part of the 
		// level
		flags += (ePickWall+ePickRoom);
	}

	Level* levelPicked=NULL;

	// If the facet are visible, try first to select the object
	// with facet.
	Wall* bestWall=NULL;
	real32 minAlpha=kBigRealValue;
	EPickedType pickedType = eNothingPicked;
	if(flags&ePickWall)
	{
		bestWall = PickedBestWall(fatRay,hitPosition,minAlpha,useLevel,&levelPicked);
	}
	
	// Object handle
	SubObject* bestObject=NULL;
	if(flags&ePickPoint && !fData.GetHoleEditEnable())
	{	// Hole edition Off: try to pick their handle
		OutlinePoint* point = PickedBestHandle( fatRay, hitPosition, minAlpha, useLevel, inPlanView?mWallObjectHandles2D:mWallObjectHandles3D);
		if(point)
		{
			bestObject = point->GetSubObject();
			pickedType=eWallObjectPicked;
		}
		else
		{
			OutlinePoint* point = PickedBestHandle( fatRay, hitPosition, minAlpha, useLevel, inPlanView?mRoomObjectHandles2D:mRoomObjectHandles3D);
			if(point)
			{
				bestObject = point->GetSubObject();
				pickedType=eRoomObjectPicked;
			}
		}
	}

	// Try to pick the points before the edges, they are prioritary
	CommonPoint* bestPoint=NULL;
	if(flags&ePickProfilePoint)
	{	// Roof Profile Point
		bestPoint = PickedBestProfilePoint( fatRay, hitPosition, minAlpha,pickedType,useLevel);
	}
	
	Wall* bestWallHandle=NULL;
	if(flags&ePickPoint && !bestPoint)
	{
		if(fData.GetHoleEditEnable()) // Hole edition On: try to pick their points
			bestPoint = PickedBestHandle( fatRay, hitPosition, minAlpha, useLevel, inPlanView?mWallObjectHandles2D:mWallObjectHandles3D);
		if(bestPoint)
			pickedType = eWallHolePointPicked;
		else
		{
			if(fData.GetHoleEditEnable()) // Hole edition On: try to pick their points
				bestPoint = PickedBestHandle( fatRay, hitPosition, minAlpha, useLevel, inPlanView?mRoomObjectHandles2D:mRoomObjectHandles3D);
			if(bestPoint)
				pickedType = eRoomHolePointPicked;
			else
				bestPoint = PickedBestPoint( fatRay, hitPosition, minAlpha,pickedType,useLevel);
		}

		if(inPlanView && !bestPoint)
		{	// Try to get a wall handle
			bestWallHandle = PickedBestWallHandle( fatRay, hitPosition, minAlpha,useLevel);
		}
	}


	Wall* bestEdge=NULL;
	if(flags&ePickEdge)
	{
		bestEdge = PickedBestEdge( fatRay, hitPosition, minAlpha,useLevel);
	}


	Room* bestRoom=NULL;
	TVector3 roomHit;
	if(flags&ePickRoom)
	{
		bestRoom = PickedBestRoom(fatRay,roomHit,minAlpha,useLevel,&levelPicked);
		if(bestRoom)
		{
			// we picked a room in front of or best wall => forget the wall
			bestWall = NULL;
			hitPosition = roomHit;
		}
	}

	Roof* bestRoof=NULL;
	TVector3 roofHit;
	if(flags&ePickRoof && fShowAll)
	{
		bestRoof = PickedBestRoof(fatRay,roofHit,minAlpha,useLevel,&levelPicked);
		if(bestRoof)
		{
			// we picked a roof in front of or best wall and Room => forget them
			bestWall = NULL;
			bestRoom = NULL;
			hitPosition = roofHit;
		}
	}

	if(flags&ePickLevel)
	{
		if(levelPicked)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(levelPicked);

			const real32 altitude = levelPicked->GetDistanceToGround();
			const real32 height = levelPicked->GetLevelHeight();

			const real32 hitZ = hitPosition.z;

			if(hitZ>(altitude+.5*height))
				return eLevelUpPicked;
			else
				return eLevelDownPicked;
		}
	}
	else
	{
		if(bestObject)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestObject);
			return pickedType;
		}	
		else if(bestPoint)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestPoint);
			return pickedType;// ePointPicked || eRoofPointPicked
		}
		else if(bestWallHandle)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestWallHandle);
			return eWallHandlePointPicked;
		}
		else if(bestEdge)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestEdge);
			return eWallPicked;
		}
		else if(bestRoof)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestRoof);
			return eBuildingRoof;
		}
		else if(bestRoom)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestRoom);
			return eRoomFloorPicked; // eRoomCeilingPicked to do
		}
		else if(bestWall)
		{
			TMCCountedGetHelper<CommonBase> result(object);
			result = dynamic_cast<CommonBase*>(bestWall);
			return eWallPicked;
		}
	}

	return eNothingPicked;
}

Wall* BuildingPrim::PickedBestWall(const FatRay& fatRay, 
								   TVector3& hitPosition, 
								   real32& minAlpha, const int32 inLevel,
								   Level** levelPicked) 
{
	Wall *bestWall=NULL;
	Level *bestLevel=NULL;
	TVector3 bestHitPosition;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			if (wall->Flag(eWallHelper))
				continue;

			const TMCClassArray<Triangle>& triangles = wall->Triangles();	
			const TMCClassArray<Vertex>& vertices = wall->Vertices();
			TVector3 hitPos;
			real32 alpha=kBigRealValue;
			if (PickTriangles(triangles,vertices, alpha,fatRay,hitPos)) 
			{
				// Select the closest one.
				// If 2 walls are at the same position (this happens after duplicating an object ): 
				// if one is selected, take it
				if(	(alpha < minAlpha) ||
					(alpha==minAlpha && wall->Selected() )) 
				{
					minAlpha=alpha;
					bestWall=wall;
					bestLevel=level;
					bestHitPosition=hitPos;
				}
			}
		}
	}

	if (bestWall)
	{
		hitPosition=bestHitPosition;
		*levelPicked=bestLevel;
	}

	return bestWall;
}

Room *BuildingPrim::PickedBestRoom( const FatRay& fatRay, 
								   TVector3& hitPosition, 
								   real32& minAlpha, const int32 inLevel, 
								   Level** levelPicked) 
{
	Room *bestRoom=NULL;
	Level *bestLevel=NULL;
	TVector3 bestHitPosition;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if( level->Hidden() )
			continue;

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( room->Hidden() )
				continue;

			TVector3 hitPos;
			real32 alpha=kBigRealValue;

			const boolean roomWithoutTop = !fShowAll||room->NoCeiling();
			// Don't build when no tgles
			if(roomWithoutTop&&room->GetTgleOffset()==0)
				continue;

			const int32 tgleOffset = room->GetTgleOffset();
			const int32 tgleCount = room->Triangles().GetElemCount();
			// test tgleOffset!=tgleCount to avoid copiing the entire indentical array

			if (PickTriangles((roomWithoutTop&&tgleOffset!=tgleCount)?TMCClassArray<Triangle>( room->Triangles(), 0, room->GetTgleOffset() ):room->Triangles(),room->Vertices(), alpha,fatRay,hitPos)) 
			{
				// Select the closest one.
				// If 2 walls are at the same position (this happens after duplicating an object ): 
				// if one is selected, take it
				if(	(alpha < minAlpha) ||
					(alpha==minAlpha && room->Selected() )) 
				{
					minAlpha=alpha;
					bestRoom=room;
					bestLevel=level;
					bestHitPosition=hitPos;
				}
			}
		}
	}

	if (bestRoom)
	{
		hitPosition=bestHitPosition;
		*levelPicked=bestLevel;
	}

	return bestRoom;
}

Roof *BuildingPrim::PickedBestRoof( const FatRay& fatRay, 
								   TVector3& hitPosition, 
								   real32& minAlpha, const int32 inLevel,
								   Level** levelPicked) 
{
	Roof *bestRoof=NULL;
	Level *bestLevel=NULL;
	TVector3 bestHitPosition;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if( level->Hidden() )
			continue;

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( roof->Hidden() )
				continue;

			const TMCClassArray<Triangle>& triangles = roof->Triangles();	
			const TMCClassArray<Vertex>& vertices = roof->Vertices();
			TVector3 hitPos;
			real32 alpha=kBigRealValue;
			if (PickTriangles(triangles,vertices, alpha,fatRay,hitPos)) 
			{
				// Select the closest one.
				// If 2 walls are at the same position (this happens after duplicating an object ): 
				// if one is selected, take it
				if(	(alpha < minAlpha) ||
					(alpha==minAlpha && roof->Selected() )) 
				{
					minAlpha=alpha;
					bestRoof=roof;
					bestLevel=level;
					bestHitPosition=hitPos;
				}
			}
		}
	}

	if (bestRoof)
	{
		hitPosition=bestHitPosition;
		*levelPicked=bestLevel;
	}

	return bestRoof;
}

boolean PickedOneEdge( const FatRay& fatRay, 
				    const real32 normDir,
					const TVector3& pos0,
					const TVector3& pos1,
					const boolean wallSelected,
				   TVector3& hitPosition, 
				   real32& alpha, 
				   real32& bestDistance)
{
	boolean outside=false;

	{
		TVector3 a=pos0;
		TVector3 b=pos1;

		const int32 planeCount = fatRay.fRayPlanes.GetElemCount();
		for (int32 planeIndex=0; planeIndex<planeCount; ++planeIndex)
		{
			const Plane& rayPlane = fatRay.fRayPlanes[planeIndex];
			real aa=(a-rayPlane.fPoint)*rayPlane.fNormal;
			real bb=(b-rayPlane.fPoint)*rayPlane.fNormal;
			if (aa <= kRealZero && bb <= kRealZero)
			{
				outside=true;
				break;
			}

			if (aa < kRealZero)
				a=(a*bb-b*aa)/(bb-aa);
			else if (bb < kRealZero)
				b=(b*aa-a*bb)/(aa-bb);
		}	
	}

	if (outside)
		return false;

	// Point at the minimum distance between the line defined by the edge and the ray
	TVector3 u=pos1-pos0;
	TVector3 v=fatRay.fDirection;
	TVector3 w0=pos0-fatRay.fOrigin;

	MY_ASSERT( ((u*u)*(v*v)-(u*v)*(u*v))!=0 );

	const TVector3 point=pos0+(((u*v)*(v*w0)-(v*v)*(u*w0))/((u*u)*(v*v)-(u*v)*(u*v)))*u;
	// Check if this point is on the edge
	const real32 normAB = u.GetNorm();
	const real32 normAP = (point-pos0).GetNorm();
	const real32 normPB = (pos1-point).GetNorm();
	if(normPB>normAB || normAP>normAB)
		return false; // Nearest point is outside the edge

	// Distance between the projection of the nearest point and the origin of the ray
	const TVector3 OP=point-fatRay.fOrigin;
	const real32 normOP = OP.GetNorm();
	real32 tmp = OP*fatRay.fDirection/(normOP*normDir);
	real32 dist=normOP*RealSqrt(1-tmp*tmp);

	// See if the point is in the 'precision' zone. If so, it will be prioritary
	// over other kind of selections
	const int32 numPlanesPrecise = fatRay.fRayPlanesPrecise.GetElemCount();
	for (int32 planeIndex=0; planeIndex<numPlanesPrecise; ++planeIndex)
	{
		const Plane& rayPlane = fatRay.fRayPlanesPrecise[planeIndex];
		if ( ((point-rayPlane.fPoint)*rayPlane.fNormal) < kRealZero)
		{
			outside=true;
			break;
		}
	}
	if (!outside)
	{
		dist=kRealZero;				
	}

	// If this edge is nearer than what we already got, take it.
	// If several edges are in the 'precision' zone, the nearest to the screen is selected
	// If 2 edges are at the same position (this happens after duplicating an object ): 
	// if one is selected, take it
	if(	dist<alpha ||
		(dist==alpha && normOP<bestDistance) ||
		(dist==alpha && normOP==bestDistance && wallSelected ) )
	{
		hitPosition=point;
		alpha=dist;
		bestDistance = normOP;
		return true;
	}

	return false;
			
}

boolean CheckPosHit(	const TVector3& pos, 
						real32& bestDistance, real32& alpha, TVector3& bestHitPosition,
						const FatRay& fatRay, const real32 dd, const real32 normDir,
						const boolean selected )
{
	const int32 planeCount = fatRay.fRayPlanes.GetElemCount();
	int32 planeIndex= 0;
	for (planeIndex=0; planeIndex<planeCount; planeIndex++)
	{
		const Plane& rayPlane = fatRay.fRayPlanes[planeIndex];
		if ( ((pos-rayPlane.fPoint)*rayPlane.fNormal) < kRealZero)
		{
			return false;
		}
	}		

	// We found a point inside the zone
	TVector3 OV=pos-fatRay.fOrigin;
	const real32 pointDistance=(fatRay.fDirection*OV)/dd;

	// Then work with the projection of the vertex on the screen.
	// Compute the distance from the projection of the vertex on the plane defined 
	// by fatRay.fOrigin and fatRay.fDirection to fatRay.fOrigin. We'll keep 
	// the nearest one 
	const real32 normOV = OV.GetNorm();
	const real32 tmp = OV*fatRay.fDirection/(normOV*normDir);
	real32 pointPrecision=normOV*RealSqrt(1-tmp*tmp);

	// See if the point is in the 'precision' zone. If so, it will be prioritary
	// over other kind of selections
	const int32 numPlanesPrecise = fatRay.fRayPlanesPrecise.GetElemCount();
	boolean outside=false;
	for (planeIndex=0; planeIndex<numPlanesPrecise; ++planeIndex)
	{
		const Plane& rayPlane = fatRay.fRayPlanesPrecise[planeIndex];
		if ( ((pos-rayPlane.fPoint)*rayPlane.fNormal) < kRealZero)
		{
			outside=true;
			break;
		}
	}
	if (!outside)
	{
		pointPrecision=kRealZero;				
	}

	// If 2 vertices are at the same position (this happens after duplicating an object ): 
	// if one is selected, take it
	if(pointPrecision<alpha ||
		(pointPrecision==alpha && pointDistance<bestDistance) ||
		(pointPrecision==alpha && pointDistance==bestDistance && selected ) )
	{
		bestDistance = pointDistance;
		alpha = pointPrecision;
		bestHitPosition=pos;
		return true;
	}

	return false;
}

// Wall handle is the point in the middle of the wall 
Wall* BuildingPrim::PickedBestWallHandle( const FatRay& fatRay, 
								   TVector3& hitPosition, 
								   real32& minAlpha, const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	Wall *bestWall=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();
		
		const real32 z=level->GetDistanceToGround();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			if (wall->Flag(eWallHelper))
				continue;

			// Get the point in middle
			TVector3 pos; 
			pos.SetFromXY( wall->GetMidPointPos(), z );

			if( CheckPosHit( pos, 
				bestDistance, alpha, bestHitPosition, 
				fatRay, dd, normDir, false) )
			{
				bestWall=wall;
			}

		}
	}

	if (bestWall)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestWall;
}

Wall* BuildingPrim::PickedBestEdge( const FatRay& fatRay, 
								   TVector3& hitPosition, 
								   real32& minAlpha, const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;

	Wall *bestWall=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();
		
		const real32 z=level->GetDistanceToGround();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			if (wall->Flag(eWallHelper))
				continue;

			// First check the straight line between the 2 points (flat waal)
			TVector3 pos0; pos0.SetFromXY( wall->GetPoint(0)->Position(), z );
			TVector3 pos1; pos1.SetFromXY( wall->GetPoint(1)->Position(), z );

			MY_ASSERT(pos0!=pos1);

			if( PickedOneEdge( fatRay, normDir, pos0,pos1, wall->Selected(),
				   bestHitPosition, alpha, bestDistance) )
			{
				bestWall=wall;
			}

			// Then check along the circle arc (curved wall)
			const int32 segCount = wall->GetArcSegmentCount();
			if( wall->GetArcOffset() != 0 && segCount>1 )
			{
				const TMCClassArray<TVector2>& arc = wall->GetCircleArc();

				for(int32 iSeg=0 ; iSeg<segCount ; iSeg++)
				{
					const TVector2& nextPos = arc[iSeg+1];
					pos1.x = nextPos.x;
					pos1.y = nextPos.y;
					if( PickedOneEdge( fatRay, normDir, pos0,pos1, wall->Selected(),
						   bestHitPosition, alpha, bestDistance) )
					{
						bestWall=wall;
					}

					// Move to the next segment
					pos0 = pos1;
				}
			}

		}
	}

	if (bestWall)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestWall;
}

Wall* BuildingPrim::BasicPickBestWall(const TVector3& origin,
									  const TVector3& direction,
									  TVector3& hitPosition,
									  const int32 inLevel) 
{
	real64 minAlpha=kBigRealValue;
	real64 alpha=0;
	Wall *bestWall=NULL;
	TVector3 bestHitPosition;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const real32 z=level->GetDistanceToGround();

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			TVector3 hitPos;
			const TVector3 height(0,0,wall->GetWallHeight());
			TVector3 pos0; pos0.SetFromXY( wall->GetPoint(0)->Position(), z );
			TVector3 pos1; pos1.SetFromXY( wall->GetPoint(1)->Position(), z );
			const TVector3 pos2 = pos1+height;
			const TVector3 pos3 = pos0+height;
			// Select the closest one.
			// If 2 walls are at the same position (this happens after duplicating an object ): 
			// if one is selected, take it
			if( BasicTrianglePick(pos0, pos1, pos2, alpha, origin, direction, hitPos ) )
			{
				if(	(alpha < minAlpha) || (alpha==minAlpha && wall->Selected() )) 
				{
					minAlpha=alpha;
					bestWall=wall;
					bestHitPosition=hitPos;
				}
			}
			else if( BasicTrianglePick(pos0, pos2, pos3, alpha, origin, direction, hitPos ) )
			{
				if(	(alpha < minAlpha) || (alpha==minAlpha && wall->Selected() )) 
				{
					minAlpha=alpha;
					bestWall=wall;
					bestHitPosition=hitPos;
				}
			}
		}
	}

	if (bestWall)
	{
		hitPosition=bestHitPosition;
	}

	return bestWall;
}

Wall* BuildingPrim::ProjectOnNearestWall(TVector3& pos)
{
	Level* level = fLevelArray[0];

	// Use the z value to now which level can be used
	const int32 levelCount = fLevelArray.GetElemCount();
	for(int32 iLevel=levelCount-1 ; iLevel>0 ; iLevel--)
	{
		if(pos.z>fLevelArray[iLevel]->GetDistanceToGround())
		{
			level = fLevelArray[iLevel];
			break;
		}
	}

	if (level->Hidden())
		return NULL;

	// Projection on a wall: the z value will stay unchanged => we can work in 2 dimention
	const TVector2 flatPos = pos.CastToXY();

	const int32 wallCount = level->GetWallCount();

	real32 minAlpha = kBigRealValue;
	Wall* bestWall=NULL;
	TVector2 bestPos;
	for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
	{
		Wall* wall = level->GetWall(iWall);

		if (wall->Hidden())
			continue;

		const TVector2 P0 = wall->GetPoint(0)->Position();
		const TVector2 P1 = wall->GetPoint(1)->Position();
		const TVector2 normal(P1.y-P0.y,P0.x-P1.x);
		TVector2 hitPos;
		if( IntersectSegmentLine( P0, P1, flatPos, normal, hitPos ) )
		{
			const real32 alpha = (flatPos-hitPos).GetSquaredNorm();

			if( alpha<minAlpha )
			{
				minAlpha = alpha;
				bestPos = hitPos;
				bestWall = wall;
			}
		}
	}

	if(bestWall)
		pos.SetFromXY(bestPos,pos.z);

	return bestWall;
}

Room* BuildingPrim::BasicPickBestRoom(const TVector3& origin,
									  const TVector3& direction,
									  TVector3& hitPosition,
									  const int32 inLevel) 
{
	real64 minAlpha=kBigRealValue;
	real64 alpha=0;
	Room *bestRoom=NULL;
	TVector3 bestHitPosition;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	// We want to get the intersection with the plane going through the middle of the
	// room, not the one at the 0 level => instead of offsetting all the point of the path,
	// we just offset the origin of the ray
	TVector3 offsetOrigin(origin);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const real32 z=level->GetDistanceToGround();

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if (room->Hidden())
				continue;
			
			// Tessellate the polygon of the room without the holes
			const TMCClassArray<Triangle>& triangleArray = room->FlatTriangles();
			const TMCClassArray<TVector2>& vertexArray = room->FlatVertices();

			// Then do a picking on these facets
			//TMCCountedPtrArray<Point>&	path = room->GetPath();
			offsetOrigin.z = origin.z-room->GetMidHeight();
			const int32 facetCount = triangleArray.GetElemCount();
			for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
			{
				const Triangle& triangle = triangleArray[iFacet];
				TVector3 hitPos;
				TVector3 pos0; pos0.SetFromXY( vertexArray[triangle.pt1], z );
				TVector3 pos1; pos1.SetFromXY( vertexArray[triangle.pt2], z );
				TVector3 pos2; pos2.SetFromXY( vertexArray[triangle.pt3], z );
				if( BasicTrianglePick(pos0, pos1, pos2, alpha, offsetOrigin, direction, hitPos ) )
				{
					if(	(alpha < minAlpha) || (alpha==minAlpha && room->Selected() )) 
					{
						minAlpha=alpha;
						bestRoom=room;
						bestHitPosition=hitPos;
					}
				}
			}
		}
	}

	if (bestRoom)
	{
		hitPosition=bestHitPosition;
	}

	return bestRoom;
}

// This method should be called only if the point is not in any room
Room* BuildingPrim::GetNearestRoom(const TVector3& pos)
{
	// Use the z value to now which level can be used
	//
	const real32 currentHeight = pos.z;
	const int32 levelCount = GetLevelCount();
	int32 levelIndex = 0;
	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];
		if( currentHeight>= level->GetDistanceToGround() &&
			currentHeight< level->GetDistanceToGround()+level->GetLevelHeight() )
		{
			levelIndex = iLevel;
			break;
		}
	}


	Level* level = fLevelArray[levelIndex];

	if (level->Hidden())
		return NULL;

	// Projection on a room: the z value will stay unchanged => we can work in 2 dimention
	const TVector2 flatPos = pos.CastToXY();

	return level->LevelPlan().GetNearestRoom(flatPos);
}



CommonPoint* BuildingPrim::PickedBestPoint( const FatRay& fatRay, 
									 TVector3& hitPosition, 
									 real32& minAlpha, 
									 EPickedType& pickedType, const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	CommonPoint *bestPoint=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const real32 z=level->GetDistanceToGround();

		const int32 pointCount = level->GetPointCount();

		for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
		{
			VPoint* point = level->GetPoint(iPoint);

			if(point->Hidden())
				continue;

			if(point->Flag(ePointHelper))
				continue;

			TVector3 pos;
			pos.SetFromXY( point->Position(), z );

			if( CheckPosHit( pos, 
				bestDistance, alpha, bestHitPosition, 
				fatRay, dd, normDir, point->Selected()) )
			{
				bestPoint=point;
				pickedType = ePointPicked;
			}
		}

		// Points on the roofs
		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if(roof->Hidden())
				continue;

			const real32 zMax = z+roof->GetRoofMax();

			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();

			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				// Picking of the zone point
				TVector3 pos0; pos0.SetFromXY( zoneSection.fZonePoint->Position(), z );
				if( CheckPosHit( pos0, 
					bestDistance, alpha, bestHitPosition, 
					fatRay, dd, normDir, zoneSection.fZonePoint->Selected()) )
				{
					bestPoint=zoneSection.fZonePoint;
					pickedType = eRoofPointPicked;
				}

				// Picking of the spine point
				TVector3 pos1; pos1.SetFromXY( zoneSection.fSpinePoint->Position(), zMax );
				if( CheckPosHit( pos1, 
					bestDistance, alpha, bestHitPosition, 
					fatRay, dd, normDir, zoneSection.fSpinePoint->Selected()) )
				{
					bestPoint=zoneSection.fSpinePoint;
					pickedType = eRoofPointPicked;
				}
			}
		}

	}

	if (bestPoint)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestPoint;
}

CommonPoint* BuildingPrim::PickedBestProfilePoint( const FatRay& fatRay, 
									 TVector3& hitPosition, 
									 real32& minAlpha, 
									 EPickedType& pickedType, const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	CommonPoint *bestPoint=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const real32 distToGround = level->GetDistanceToGround();

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if(roof->Hidden())
				continue;

			const real32 levelToGround = level->GetDistanceToGround();
			const real32 baseToGround = levelToGround + roof->GetRoofMin();
			const real32 topToGround = levelToGround + roof->GetRoofMax();

			const int32 zoneSectionCount = roof->GetRoofZoneSectionCount();
			const int32 botProfileCount = roof->GetBotProfilePointCount();
			const int32 topProfileCount = roof->GetTopProfilePointCount();
			const int32 botInsideCount = roof->GetBotInsidePointCount();
			const int32 topInsideCount = roof->GetTopInsidePointCount();

			for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
			{
				const ZoneSection& zoneSection = roof->GetRoofZoneSection(iPt);

				// Picke the profile points

				ZoneSection prevZoneSection = roof->GetRoofZoneSection(zoneSectionCount-1);
				TVector2 prevZonePos = prevZoneSection.fZonePoint->Position();
				TVector2 prevSpinePos = prevZoneSection.fSpinePoint->Position();
				for(int32 iSection=0 ; iSection<zoneSectionCount ; iSection++)
				{
					const ZoneSection& zoneSection = roof->GetRoofZoneSection(iSection);
					const TVector2& zonePos = zoneSection.fZonePoint->Position();
					const TVector2& spinePos = zoneSection.fSpinePoint->Position();

					if(zonePos != prevZonePos)
					{
						// Build a base, then us x and z axi to find the pos
						{
							TVector3 O,I,J,K=TVector3::kUnitZ;
							const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
							const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);
					
							boolean canDrawHandles = true;
							if(zoneSection.GetIsVertical())
							{	// Special case: this portion of the roof is a wall: draw the handles on the side
								if(prevSection.GetIsVertical())
									canDrawHandles = false;
								else
								{
									O = prevSection.fZonePoint->Get3DPos();
									roof->GetVerticalBase(prevSection.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),
										J,I,prevSection.fSpinePoint->Get3DPos(), zoneSection.fSpinePoint->Get3DPos());
									K = TVector3::kUnitZ;
								}
							}
							else
							{	// Normal case
								GetRoofBase(prevSection.fZonePoint->Get3DPos(), 
												zoneSection.fZonePoint->Get3DPos(), 
												roof->Flag(eRoofZoneOrientedPositive)?-1:1,
												I, J, O);
							}

							if(canDrawHandles)
							{
								for(int32 iPt=0 ; iPt<botProfileCount ; iPt++)
								{
									ProfilePoint* point = roof->GetBotProfilePoint(iPt);
									const TVector2& pos = point->Position();
									const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
									if( CheckPosHit( posInSpace, 
										bestDistance, alpha, bestHitPosition, 
										fatRay, dd, normDir, false) )
									{
										bestPoint=point;
										pickedType = eProfilePointPicked;
									}
								}

								for(int32 iIn=0 ; iIn<botInsideCount ; iIn++)
								{
									ProfilePoint* point = roof->GetBotInsidePoint(iIn);
									const TVector2& pos = point->Position();
									const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
									if( CheckPosHit( posInSpace, 
										bestDistance, alpha, bestHitPosition, 
										fatRay, dd, normDir, false) )
									{
										bestPoint=point;
										pickedType = eProfilePointPicked;
									}
								}
							}
						}

						{
							TVector3 O,I,J,K;
							boolean canDrawHandles = true;
							if(zoneSection.GetIsVertical())
							{	// Special case: this portion of the roof is a wall: draw the handles on the side
								const int32 prevIndex = iSection>0?iSection-1:zoneSectionCount-1;
								const ZoneSection& prevSection = roof->GetRoofZoneSection(prevIndex);
						
								if(prevSection.GetIsVertical())
									canDrawHandles = false;
								else
								{
									O = prevSection.fSpinePoint->Get3DPos();
									roof->GetVerticalBase(prevSection.fZonePoint->Get3DPos(), zoneSection.fZonePoint->Get3DPos(),
										J,I,prevSection.fSpinePoint->Get3DPos(), zoneSection.fSpinePoint->Get3DPos());
									K = TVector3::kUnitZ;
								}
							}
							else
							{	// Normal case
								Get3DBase1(	topToGround, prevZonePos, zonePos, prevSpinePos, spinePos, O, I, J, K);
							}

							if(canDrawHandles)
							{
								for(int32 iPt=0 ; iPt<topProfileCount ; iPt++)
								{
									ProfilePoint* point = roof->GetTopProfilePoint(iPt);
									const TVector2& pos = point->Position();
									const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
									if( CheckPosHit( posInSpace, 
										bestDistance, alpha, bestHitPosition, 
										fatRay, dd, normDir, false) )
									{
										bestPoint=point;
										pickedType = eProfilePointPicked;
									}
								}

								for(int32 iIn=0 ; iIn<topInsideCount ; iIn++)
								{
									ProfilePoint* point = roof->GetTopInsidePoint(iIn);
									const TVector2& pos = point->Position();
									const TVector3 posInSpace = (O + pos.x*J + pos.y*K);
									if( CheckPosHit( posInSpace, 
										bestDistance, alpha, bestHitPosition, 
										fatRay, dd, normDir, false) )
									{
										bestPoint=point;
										pickedType = eProfilePointPicked;
									}
								}
							}
						}
						}

					// Go to next section
					prevZoneSection = zoneSection;
					prevZonePos = zonePos;
					prevSpinePos = spinePos;
				}
			}
		}

	}

	if (bestPoint)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestPoint;
}

CommonPoint* BuildingPrim::PickedBestHolePoint(const FatRay& fatRay, 
										  TVector3& hitPosition, 
										  real32& minAlpha, 
										  EPickedType& pickedType,
										  const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	CommonPoint *bestPoint=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			const int32 objectCount = wall->GetObjectCount();
			for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
			{
				WallSubObject* object = wall->GetObject(iObject);

				TMCClassArray<TVector3> side0;
				TMCClassArray<TVector3> side1;
				TMCClassArray<TVector3> normals;
				object->Get3DOutlines(side0, side1, normals);

				const TMCCountedPtrArray<OutlinePoint>& outline = object->GetOutline();
				// Go arround the profile points in 3D
				const int32 profileCount = side0.GetElemCount();
				for(int32 iPt=0 ; iPt<profileCount ; iPt++)
				{
					const TVector3& pos0 = side0[iPt];
					if( CheckPosHit( pos0, 
						bestDistance, alpha, bestHitPosition, 
						fatRay, dd, normDir, false) )
					{
						bestPoint=(CommonPoint*)(outline[iPt]);
						pickedType = eWallHolePointPicked;
					}
					const TVector3& pos1 = side1[iPt];
					if( CheckPosHit( pos1, 
						bestDistance, alpha, bestHitPosition, 
						fatRay, dd, normDir, false) )
					{
						bestPoint=(CommonPoint*)(outline[iPt]);
						pickedType = eWallHolePointPicked;
					}
				}
			}
		}

		// Room Objects
		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if (room->Hidden())
				continue;

			const int32 objectCount = room->GetObjectCount();
			for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
			{
				RoomSubObject* object = room->GetObject(iObject);

				TMCClassArray<TVector3> side0;
				TMCClassArray<TVector3> side1;
				TMCClassArray<TVector3> normals;
				object->Get3DOutlines(side0, side1, normals);

				const TMCCountedPtrArray<OutlinePoint>& outline = object->GetOutline();
				// Go arround the profile points in 3D
				const int32 profileCount = side0.GetElemCount();
				for(int32 iPt=0 ; iPt<profileCount ; iPt++)
				{
					const TVector3& pos0 = side0[iPt];
					if( CheckPosHit( pos0, 
						bestDistance, alpha, bestHitPosition, 
						fatRay, dd, normDir, false) )
					{
						bestPoint=(CommonPoint*)(outline[iPt]);
						pickedType = eRoomHolePointPicked;
					}
					const TVector3& pos1 = side1[iPt];
					if( CheckPosHit( pos1, 
						bestDistance, alpha, bestHitPosition, 
						fatRay, dd, normDir, false) )
					{
						bestPoint=(CommonPoint*)(outline[iPt]);
						pickedType = eRoomHolePointPicked;
					}
				}
			}
		}

	}

	if (bestPoint)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestPoint;
}

OutlinePoint* BuildingPrim::PickedBestHandle(const FatRay& fatRay, 
										  TVector3& hitPosition, 
										  real32& minAlpha, 
										  const int32 inLevel,
										  TMCClassArray<PosAndObj>& handleList)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	OutlinePoint *bestPoint=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	
	{	// Walk through the handles
		const int32 handleCount = handleList.GetElemCount();
		for(int32 iHandle = 0 ; iHandle<handleCount ; iHandle++)
		{
			const TVector3& pos = handleList[iHandle].mPosition;
			OutlinePoint* point = handleList[iHandle].mPoint;
			if( CheckPosHit( pos, 
				bestDistance, alpha, bestHitPosition, 
				fatRay, dd, normDir, point->GetSubObject()->Selected()) )
			{
				bestPoint=point;
			}
		}
	}

	if (bestPoint)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestPoint;
}

/*
SubObject* BuildingPrim::PickedBestObject(const FatRay& fatRay, 
											  TVector3& hitPosition, 
											  real32& minAlpha, 
											  EPickedType& pickedType,
											  const int32 inLevel)
{
	real32 alpha=minAlpha; // distance in projection to the clicked point
	real32 bestDistance = kBigRealValue;
	const real32 dd=fatRay.fDirection*fatRay.fDirection;

	SubObject *bestObject=NULL;
	TVector3 bestHitPosition;

	const real32 normDir = fatRay.fDirection.GetNorm();

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if (wall->Hidden())
				continue;

			const int32 objectCount = wall->GetObjectCount();
			for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
			{
				WallSubObject* object = wall->GetObject(iObject);
	
				TVector3 pos;
				object->GetCenter(pos);

				if( CheckPosHit( pos, 
					bestDistance, alpha, bestHitPosition, 
					fatRay, dd, normDir, object->Selected()) )
				{
					bestObject=object;
					pickedType=eWallObjectPicked;
				}
			}
		}

		// Room Objects
		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if (room->Hidden())
				continue;

			const int32 objectCount = room->GetObjectCount();
			for( int32 iObject=0 ; iObject<objectCount ; iObject++ )
			{
				RoomSubObject* object = room->GetObject(iObject);
	
				TVector3 pos;
				object->GetCenter(pos);

				if( CheckPosHit( pos, 
					bestDistance, alpha, bestHitPosition, 
					fatRay, dd, normDir, object->Selected()) )
				{
					bestObject=object;
					pickedType=eRoomObjectPicked;
				}
			}
		}

	}

	if (bestObject)
	{
		hitPosition=bestHitPosition;
		minAlpha = alpha;
	}

	return bestObject;
}*/

boolean BuildingPrim::PickTriangles(const TMCClassArray<Triangle>& triangles,
									const TMCClassArray<Vertex>& vertices,
									real32 &alpha,const FatRay& fatRay, TVector3& hitPosition)
{
	TVector3 localHitPosition=TVector3::kZero;
	boolean hit=false;

	const int32 tglCount = triangles.GetElemCount();

	for( int32 iTgl=0 ; iTgl<tglCount ; iTgl++ )
	{
		const Triangle& triangle = triangles[iTgl];
		real32 localAlpha=kBigRealValue;
		if( TrianglePick(	vertices[triangle.pt1].Position(),
							vertices[triangle.pt2].Position(),
							vertices[triangle.pt3].Position(),
							localAlpha,fatRay,localHitPosition) )
		{
			hit=true;
			if(	(localAlpha < alpha) ) 
			{
				alpha=localAlpha;
				hitPosition=localHitPosition;
			}
		}
	}

	return hit;
}

void BuildingPrim::SetSelection(const boolean select)
{
	const int32 levelCount = fLevelArray.GetElemCount();
	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->SetSelection(select);
	}

	// Rebuild the selection outside of this method, otherwise we might do it for nothing in some cases
}

void BuildingPrim::ShowAll(const boolean all)
{
	if(all!=fShowAll)
	{
		fShowAll = all;

		// Deselect the hidden levels
		if(!fShowAll)
		{
			const int32 levelCount = fLevelArray.GetElemCount();
			for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
			{
				if(iLevel!=fActiveLevel)
					fLevelArray[iLevel]->SetSelection(false);
			}
		}
	}
}

void BuildingPrim::ActiveLevel(const int32 level)
{
	if(level!=fActiveLevel)
	{
		if(level==kAllLevels)
			ShowAll(true); // Special case (maybe shouldn't be used)
		else
		{
			// Deselect the hidden levels
			if(!fShowAll && fActiveLevel<(int32)fLevelArray.GetElemCount())
			{
				fLevelArray[fActiveLevel]->SetSelection(false);
			}

			fActiveLevel = level;
		}
	}
}
/*
void BuildingPrim::ShowLevel(const int32 level)
{
	if(level!=fShowLevel)
	{
		fShowLevel = level;

		// Deselect the hidden levels
		if(fShowLevel!=kAllLevels)
		{
			const int32 levelCount = fLevelArray.GetElemCount();
			for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
			{
				if(iLevel!=fShowLevel)
					fLevelArray[iLevel]->SetSelection(false);
			}
		}
	}
}
*/
void BuildingPrim::AssembleRoomShowLevel(const int32 level)
{
	// Check if the level exist
	if(level>=0 && level<(int32)fLevelArray.GetElemCount())
		fData.AssembleRoomShowLevel(level);
}

void BuildingPrim::ShowHide(const EShowHideOption option)
{
	const int32 levelCount = fLevelArray.GetElemCount();
	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		level->ShowHide(option);
	}
}

void BuildingPrim::Smooth(const boolean smooth, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		level->LevelPlan().Smooth(smooth);
	}
}

// get the selection flag on the polygons to set the right colors on the facet mesh
// Call only for vertex modeler use
void BuildingPrim::UpdateFacetMeshColors( FacetMesh* facetMesh, const int32 meshFlags )
{
	// if faceted, need to simulate a fake lighting
	const boolean faceted = FLAG(meshFlags, eFaceted);

	// Warning: order of the facet meshes here should be the same as in GetLevelFacetMesh
	
	const boolean is2DMesh = FLAG(meshFlags, e2DMesh);
	const boolean noTop = is2DMesh||!fShowAll;
	int32 useLevel = noTop?fActiveLevel:kAllLevels;

	const int32 levelCount = LevelCount(useLevel);
	const int32 startLevel = StartLevel(useLevel);

	//TMCArray<TMCColorRGBA8>& colors = facetMesh->fPolygonColors;

	int32 facetCount=0;

	const TMCColorRGBA8& defCol = is2DMesh?fData.fFloCol:fData.fDefCol;
	const TMCColorRGBA8& selCol = fData.fSelCol;
	const TMCColorRGBA8& tarCol = fData.fTarCol;
	const TMCColorRGBA8& snaCol = fData.fSnaCol;
	const TMCColorRGBA8& helCol = fData.fHelCol;

	if(faceted)
	{
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			const int32 pointCount = level->GetPointCount();

			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point= level->GetPoint(iPoint);

				if(point->Hidden())
					continue;

				TMCClassArray<Triangle>& triangles = point->Triangles();
				TMCClassArray<Vertex>& vertices = point->Vertices();

				const boolean selected = point->Selected();
				const boolean targeted = point->Targeted();

				const int32 count = triangles.GetElemCount();

				for (int32 iTgl=0;iTgl<count;iTgl++)
				{
					if(selected)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), selCol);
					else if(targeted)	facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), tarCol);
					else				facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), defCol);

					facetCount++;
				}
			}

			if(!is2DMesh)
			{
				const int32 wallCount = level->GetWallCount();

				for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
				{
					Wall* wall = level->GetWall(iWall);

					if (wall->Hidden())
						continue;

					TMCClassArray<Triangle>& triangles = wall->Triangles();
					TMCClassArray<Vertex>& vertices = wall->Vertices();

					const boolean selected = wall->Selected();
					const boolean targeted = wall->Targeted();
					const boolean snaped = wall->Flag(eSnapedPosition);
					const boolean helper = wall->Flag(eWallHelper);

					const int32 count = triangles.GetElemCount();

					for (int32 iTgl=0;iTgl<count;iTgl++)
					{
						if(selected)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), selCol);
						else if(targeted)	facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), tarCol);
						else if(snaped)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), snaCol);
						else if(helper)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), helCol);
						else				facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), defCol);

						facetCount++;
					}
				}
			}

			const int32 roomCount = level->GetRoomCount();

			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom(iRoom);

				if (room->Hidden())
					continue;

				TMCClassArray<Triangle>& triangles = room->Triangles();
				TMCClassArray<Vertex>& vertices = room->Vertices();

				const boolean selected = room->Selected();
				const boolean targeted = room->Targeted();

				const int32 count = noTop?room->GetTgleOffset():triangles.GetElemCount();

				for (int32 iTgl=0;iTgl<count;iTgl++)
				{
					if(selected)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), selCol);
					else if(targeted)	facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), tarCol);
					else				facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), defCol);

					facetCount++;
				}
			}

			if(!noTop)
			{
				// Roof facets
				const int32 roofCount = level->GetRoofCount();

				for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
				{
					Roof* roof = level->GetRoof(iRoof);

					if (roof->Hidden())
						continue;

					TMCClassArray<Triangle>& triangles = roof->Triangles();
					TMCClassArray<Vertex>& vertices = roof->Vertices();

					const boolean selected = roof->Selected();
					const boolean targeted = roof->Targeted();

					const int32 count = triangles.GetElemCount();

					for (int32 iTgl=0;iTgl<count;iTgl++)
					{
						if(selected)		facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), selCol);
						else if(targeted)	facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), tarCol);
						else				facetMesh->fPolygonColors[facetCount] = GetFacetedColor(vertices[triangles[iTgl].pt1].Normal(), defCol);

						facetCount++;
					}
				}
			}
		}
	}
	else
	{
		for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
		{
			Level* level = fLevelArray[iLevel];

			if (level->Hidden())
				continue;

			const int32 pointCount = level->GetPointCount();

			for( int32 iPoint=0 ; iPoint<pointCount ; iPoint++ )
			{
				VPoint* point = level->GetPoint(iPoint);

				if (point->Hidden())
					continue;

				TMCClassArray<Triangle>& triangles = point->Triangles();

				const boolean selected = point->Selected();
				const boolean targeted = point->Targeted();

				const int32 count = triangles.GetElemCount();

				for (int32 iTgl=0;iTgl<count;iTgl++)
				{
					if(selected)		facetMesh->fPolygonColors[facetCount] = selCol;
					else if(targeted)	facetMesh->fPolygonColors[facetCount] = tarCol;
					else				facetMesh->fPolygonColors[facetCount] = defCol;

					facetCount++;
				}
			}

			if(!is2DMesh)
			{
				const int32 wallCount = level->GetWallCount();

				for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
				{
					Wall* wall = level->GetWall(iWall);

					if (wall->Hidden())
						continue;

					TMCClassArray<Triangle>& triangles = wall->Triangles();

					const boolean selected = wall->Selected();
					const boolean targeted = wall->Targeted();
					const boolean snaped = wall->Flag(eSnapedPosition);
					const boolean helper = wall->Flag(eWallHelper);

					const int32 count = triangles.GetElemCount();

					for (int32 iTgl=0;iTgl<count;iTgl++)
					{
						if(selected)		facetMesh->fPolygonColors[facetCount] = selCol;
						else if(targeted)	facetMesh->fPolygonColors[facetCount] = tarCol;
						else if(snaped)		facetMesh->fPolygonColors[facetCount] = snaCol;
						else if(helper)		facetMesh->fPolygonColors[facetCount] = helCol;
						else				facetMesh->fPolygonColors[facetCount] = defCol;

						facetCount++;
					}
				}
			}

			const int32 roomCount = level->GetRoomCount();

			for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
			{
				Room* room = level->GetRoom(iRoom);

				if (room->Hidden())
					continue;

				TMCClassArray<Triangle>& triangles = room->Triangles();

				const boolean selected = room->Selected();
				const boolean targeted = room->Targeted();

				const int32 count = noTop?room->GetTgleOffset():triangles.GetElemCount();

				for (int32 iTgl=0;iTgl<count;iTgl++)
				{
					if(selected)		facetMesh->fPolygonColors[facetCount] = selCol;
					else if(targeted)	facetMesh->fPolygonColors[facetCount] = tarCol;
					else				facetMesh->fPolygonColors[facetCount] = defCol;

					facetCount++;
				}
			}

			if(!noTop)
			{
				// Roof facets
				const int32 roofCount = level->GetRoofCount();

				for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
				{
					Roof* roof = level->GetRoof(iRoof);

					if (roof->Hidden())
						continue;

					TMCClassArray<Triangle>& triangles = roof->Triangles();

					const boolean selected = roof->Selected();
					const boolean targeted = roof->Targeted();

					const int32 count = triangles.GetElemCount();

					for (int32 iTgl=0;iTgl<count;iTgl++)
					{
						if(selected)		facetMesh->fPolygonColors[facetCount] = selCol;
						else if(targeted)	facetMesh->fPolygonColors[facetCount] = tarCol;
						else				facetMesh->fPolygonColors[facetCount] = defCol;

						facetCount++;
					}
				}
			}
		}
	}
}

void BuildingPrim::InitMarqueeSelection(const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().InitMarqueeSelection();
	}

}

void BuildingPrim::SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode, const int32 inLevel) 
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().SetMarquee(rayPlanes,marqueeMode);
	}

	BuildExtendedSelection();

	fData.InvalidateStatus();
}

boolean BuildingPrim::SnapPosWithAxis(TVector2& pos, 
									  const TVector2& axis,
									  const TVector2& preferedProjDir,
									  const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		if( level->LevelPlan().SnapPosWithAxis(pos, axis, preferedProjDir) )
			return true; // Snap to the first found ( maybe we should take the nearest)
	}

	return false;
}

boolean BuildingPrim::BuildRoomFromSelection(const int32 inLevel)
{
	boolean couldBuild=false;

	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		if( level->LevelPlan().BuildRoomFromSelection() )
			couldBuild = true;
	}

	return couldBuild;
}

Roof* BuildingPrim::BuildRoof(const ERoofType roofType)
{
	const int32 levelCount = fLevelArray.GetElemCount();
	for(int32 iLevel = levelCount-1 ; iLevel>=0 ; iLevel--)
	{
		Level* level = fLevelArray[iLevel];
	
		TMCCountedPtrArray<VPoint> area;
		const int32 roomCount = level->GetRoomCount();
		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom( iRoom );

			if (room->Selected())
			{
				level->LevelPlan().GetSelectedRoomsPerimeter(area);
				break;
			}
		}

		if(area.GetElemCount()>2)
		{	// We've got enough points to build a roof
			return level->MakeRoof(area, roofType);
		}
	}

	{	// Not enough point selected => just build a roof over the last level

		// Get the current last level
		Level* level = fLevelArray.LastElemNoAddRef();

		TMCCountedPtrArray<VPoint> area;
		level->LevelPlan().GetRoomsPerimeter(area);

		if(area.GetElemCount()>2)
		{
			return level->MakeRoof(area, roofType);
		}
	}

	return NULL;
}

void BuildingPrim::ClearPointFlag(const int32 flag, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		level->LevelPlan().ClearPointFlag(flag);
	}
}

void BuildingPrim::ClearWallFlag(const int32 flag, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		level->LevelPlan().ClearWallFlag(flag);
	}
}

void BuildingPrim::ClearRoomFlag(const int32 flag, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level= fLevelArray[iLevel];

		level->LevelPlan().ClearRoomFlag(flag);
	}
}


void BuildingPrim::SetAllLevelDistanceToGround()
{
	Level* groundLevel = GetGroundLevel();
	if(groundLevel)
	{
		groundLevel->SetDistanceToGround(0);

		// Invalidate the bbox
		InvalidateBBox();
	}
}

void BuildingPrim::SetShadingDomain(const int32 domainID, const int32 selectionSubPart)
{
	const int32 levelCount = fLevelArray.GetElemCount();

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= fLevelArray[iLevel];

		if (MCVerify(level))
		{
			if (level->Hidden())
				continue;

			level->LevelPlan().SetShadingDomain(domainID, selectionSubPart);
		}
	}

	InvalidateSelection(false, kAllLevels);
}

// Use replaceByID==kNoDomains to set the default domain
void BuildingPrim::DelShadingDomain(const int32 domainID, int32 replaceByID)
{
	// Remove it from the array and modify the following (index changed)

	const int32 domainCount = fShadingDomains.GetElemCount();
	boolean exist = false;
	if( replaceByID >= 0 )
	{
		// See if this domain exist
		for( int32 index=0 ; index < domainCount ; index++ )
		{
			if(fShadingDomains[index].fID == (uint32)replaceByID)
			{
				exist =true;
				break;
			}
		}
	}

	if(!exist)
	{	// Use the default domain
		replaceByID = kNoDomains;
	}

	const int32 levelCount = fLevelArray.GetElemCount();

	for(int32 iLevel=0;iLevel<levelCount;iLevel++)
	{
		Level* level= fLevelArray[iLevel];

		if (MCVerify(level))
		{
			if (level->Hidden())
				continue;

			level->LevelPlan().DelShadingDomain(domainID, replaceByID);
		}
	}

	// If it's the last one, remove it. Otherwise modify the next one
	if(domainID == domainCount-1)
		fShadingDomains.RemoveElem(domainID, 1);
	else
	{
		fShadingDomains[domainID].fName = fShadingDomains[domainID+1].fName;
		//
		DelShadingDomain( domainID+1, domainID );
	}

	InvalidateStatus();
}

// In case one day the Id is different from the index
boolean IDAlreadyUsed(uint32 ID, TMCClassArray<UVSpaceInfo>& domains)
{
	const int32 domainCount = domains.GetElemCount();
	for( int32 i=0 ; i<domainCount ; i++ )
	{
		if(domains[i].fID == ID)
			return true;
	}

	return false;
}

int32 BuildingPrim::AddShadingDomain( const TMCString& domainName )
{
	const int32 spaceInfoCount = fShadingDomains.GetElemCount();

	int32 newID = spaceInfoCount;
	while( IDAlreadyUsed(newID, fShadingDomains) )
	{
		newID++;
	}

	UVSpaceInfo newDomain;
	newDomain.fID = newID;
	newDomain.fName = domainName;
	newDomain.fWraparound[0] = false;
	newDomain.fWraparound[1] = false;
	fShadingDomains.AddElem(newDomain);

	return newID;
}

void BuildingPrim::GetInstances( TMCCountedPtrArray<I3DShInstance>& instances )
{
	TMCCountedPtr<I3DShPrimitiveComponent> primComp;
	QueryInterface(IID_I3DShPrimitiveComponent, (void**)&primComp);

	TMCCountedPtr<I3DShPrimitive> primitive;
	primComp->GetPrimitive(&primitive);

	TMCCountedPtr<I3DShObject> primitiveObject;
	primitive->QueryInterface(IID_I3DShObject, (void **)&primitiveObject);

/*	This is problematic when working with several documents (might be a Carrara bug)
	Return the instances in the scene where it was created, so if pasted in another scene,
	instances are wrong
	primitiveObject->GetInstanceArray(instances);
*/
	// Instead, find the scene first
	TMCCountedPtr<I3DShScene> scene;
	GetScene(&scene);
	if(!MCVerify(scene))
		return;

	TMCDynamicString name;
	primitiveObject->GetName(name);

	MY_ASSERT(name.Length());

	TMCCountedPtr<I3DShObject> object;
	scene->Get3DObjectByName(&object, name);
	
	if(!MCVerify(object))
		return;

	object->GetInstanceArray(instances);
}

void BuildingPrim::GetScene(I3DShScene** scene)
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;

	currentDoc->GetScene(scene);
}

/*void BuildingPrim::RescueChildren( const int32 afterIndex )
{
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	GetInstances(buildingInstances);

	TMCCountedPtr<I3DShScene> scene;

	const int32 levelCount = fLevelArray.GetElemCount();

	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		TMCCountedPtrArray<WallSubObject> wallObj;
		TMCCountedPtrArray<RoomSubObject> roomObj;
		level->LevelPlan().GetSelectedObjects(wallObj, roomObj);

		{	// Attach to the selected wall objects
			const int32 wallObjCount = wallObj.GetElemCount();
			for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
			{
				SubObject* subObject = wallObj[iObj];
				if(subObject->GetChildIndex()>afterIndex)
				{
					const TMCString& name = subObject->GetRescueName();
					if(!scene)
						GetScene(&scene);

					TMCCountedPtr<I3DShObject> object;
					scene->Get3DObjectByName(&object, name);
					subObject->SetRescueName(kNullString); // Forget the name

					subObject->SetSceneObject(object, buildingInstances, this);
				}
			}
		}
		{	// Attach to the selected room objects
			const int32 roomObjCount = roomObj.GetElemCount();
			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				SubObject* subObject = roomObj[iObj];
				if(subObject->GetChildIndex()>afterIndex)
				{
					const TMCString& name = subObject->GetRescueName();
					if(!scene)
						GetScene(&scene);

					TMCCountedPtr<I3DShObject> object;
					scene->Get3DObjectByName(&object, name);
					subObject->SetRescueName(kNullString); // Forget the name

					subObject->SetSceneObject(object, buildingInstances, this);
				}
			}
		}
	}

}*/
/*
void BuildingPrim::DecalChildren( const int32 afterIndex )
{
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	GetInstances(buildingInstances);

	TMCCountedPtr<I3DShScene> scene;

	const int32 levelCount = fLevelArray.GetElemCount();

	for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		level->LevelPlan().DecalChildren(afterIndex);
	}
}*/

void BuildingPrim::AttachObjectToSelection(I3DShObject* sceneObject, const int32 inLevel, TMCClassArray<TMCDynamicString>&	undoData)
{
	undoData.ArrayFree();

	TMCCountedPtrArray<I3DShInstance> instances;
	GetInstances(instances);

	const int32 instanceCount = instances.GetElemCount();
	if(instanceCount>1)
	{
		MCNotify("Multiple instances of the building, case not done.");
	}
	
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		TMCCountedPtrArray<WallSubObject> wallObj;
		TMCCountedPtrArray<RoomSubObject> roomObj;
		level->LevelPlan().GetSelectedObjects(wallObj, roomObj);

		{	// Attach to the selected wall objects
			const int32 wallObjCount = wallObj.GetElemCount();
			for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
			{
				// First unhook the previous instance used for this object
				SubObject* subObject = wallObj[iObj];
				I3DShInstance* oldInstance = subObject->GetChildNoAddRef();
				if(oldInstance)
				{
					// Before removing it, record the name of the object
					TMCCountedPtr<I3DShObject> object;
					oldInstance->Get3DObject(&object);
					TMCDynamicString objectName;
					object->GetName(objectName);
					undoData.AddElem(objectName);

					// Now remove it
					subObject->SetSceneObject(NULL);
				}
				else
				{
					TMCDynamicString emptyName;
					undoData.AddElem(emptyName);
				}

				// Then hook the new one
				/*I3DShInstance* newInstance = */subObject->SetSceneObject(sceneObject);
			}
		}

		{	// Attach to the selected room objects
			const int32 roomObjCount = roomObj.GetElemCount();
			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				// First unhook the previous instance used for this object
				SubObject* subObject = roomObj[iObj];
				I3DShInstance* oldInstance = subObject->GetChildNoAddRef();
				if(oldInstance)
				{	
					// Before removing it, record the name of the object
					TMCCountedPtr<I3DShObject> object;
					oldInstance->Get3DObject(&object);
					TMCDynamicString objectName;
					object->GetName(objectName);
					undoData.AddElem(objectName);

					subObject->SetSceneObject(NULL);
				}
				else
				{
					TMCDynamicString emptyName;
					undoData.AddElem(emptyName);
				}

				// Then hook the new one
				/*I3DShInstance* newInstance = */subObject->SetSceneObject(sceneObject);
			}
		}
	}
}

void BuildingPrim::SetPlacementType(const EPlacementType placement, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		TMCCountedPtrArray<WallSubObject> wallObj;
		TMCCountedPtrArray<RoomSubObject> roomObj;
		level->LevelPlan().GetSelectedObjects(wallObj, roomObj);

		{
			const int32 wallObjCount = wallObj.GetElemCount();
			for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
			{
				wallObj[iObj]->SetPlacement(placement);
			}
		}

		{
			const int32 roomObjCount = roomObj.GetElemCount();
			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				roomObj[iObj]->SetPlacement(placement);
			}
		}
	}
}

void BuildingPrim::SetPlacement(const real32 value, const int32 id, const int32 inLevel)
{
	const int32 levelCount = LevelCount(inLevel);
	const int32 startLevel = StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = fLevelArray[iLevel];

		if (level->Hidden())
			continue;

		TMCCountedPtrArray<WallSubObject> wallObj;
		TMCCountedPtrArray<RoomSubObject> roomObj;
		level->LevelPlan().GetSelectedObjects(wallObj, roomObj);

		{
			const int32 wallObjCount = wallObj.GetElemCount();
			for( int32 iObj=0 ; iObj<wallObjCount ; iObj++ )
			{
				WallSubObject* obj = wallObj[iObj];
				switch(id)
				{
				case ePlacementOffsetX: obj->Offset().x = value; break;
				case ePlacementOffsetY: obj->Offset().y = value; break;
				case ePlacementOffsetZ: obj->Offset().z = value; break;
				case ePlacementScaleX: obj->Scale().x = value; break;
				case ePlacementScaleY: obj->Scale().y = value; break;
				case ePlacementScaleZ: obj->Scale().z = value; break;
				case ePlacementRotateX: obj->Rotate().x = value; break;
				case ePlacementRotateY: obj->Rotate().y = value; break;
				case ePlacementRotateZ: obj->Rotate().z = value; break;
				}
				obj->Invalidate();
			}
		}

		{
			const int32 roomObjCount = roomObj.GetElemCount();
			for( int32 iObj=0 ; iObj<roomObjCount ; iObj++ )
			{
				RoomSubObject* obj = roomObj[iObj];
				switch(id)
				{
				case ePlacementOffsetX: obj->Offset().x = value; break;
				case ePlacementOffsetY: obj->Offset().y = value; break;
				case ePlacementOffsetZ: obj->Offset().z = value; break;
				case ePlacementScaleX: obj->Scale().x = value; break;
				case ePlacementScaleY: obj->Scale().y = value; break;
				case ePlacementScaleZ: obj->Scale().z = value; break;
				case ePlacementRotateX: obj->Rotate().x = value; break;
				case ePlacementRotateY: obj->Rotate().y = value; break;
				case ePlacementRotateZ: obj->Rotate().z = value; break;
				}
				obj->Invalidate();
			}
		}
	}
}
/*
int32 BuildingPrim::GetChildrenCount()
{
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	GetInstances(buildingInstances);

	if(!MCVerify(buildingInstances.GetElemCount()))
	{
		MCNotify("No instance found");
		return 0;

	}
	TMCCountedPtr<I3DShTreeElement> treeElement;
	buildingInstances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement); ThrowIfNil(treeElement);

	return GetSonCount(treeElement);
}

int32 BuildingPrim::GetChildrenCountAndNames(TMCClassArray<TMCDynamicString>& masterObjectNames)
{
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	GetInstances(buildingInstances);

	if(!MCVerify(buildingInstances.GetElemCount()))
	{
		MCNotify("No instance found");
		return 0;

	}
	TMCCountedPtr<I3DShTreeElement> treeElement;
	buildingInstances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement); ThrowIfNil(treeElement);

	return GetSonCountAndNames(treeElement, masterObjectNames);
}

void BuildingPrim::SetChildrenCount(const int32 count)
{
	// Erase the children after count
	TMCCountedPtrArray<I3DShInstance> buildingInstances;
	GetInstances(buildingInstances);

	if(!MCVerify(buildingInstances.GetElemCount()))
	{
		MCNotify("No instance found");
		return;

	}
	TMCCountedPtr<I3DShTreeElement> treeElement;
	buildingInstances[0]->QueryInterface(IID_I3DShTreeElement, (void**) &treeElement); ThrowIfNil(treeElement);

	const int32 curCount = GetSonCount(treeElement);
	if(count<curCount)
	{
		if(count==0)
		{
			// Remove them all
			TMCCountedPtr<I3DShTreeElement> sonTree;
			treeElement->GetFirst(&sonTree);
			while(sonTree)
			{
				sonTree->Unlink(&treeElement);
				treeElement->GetFirst(&sonTree);
			}
		}
		else
		{
			// Remove the n last
			TMCCountedPtr<I3DShTreeElement> sonTree;
			treeElement->GetFirst(&sonTree);
			if(!sonTree)
				return;

			int32 sonIndex = 0;

			TMCCountedPtr<I3DShTreeElement> nextSonTree;
			while(sonIndex<curCount)
			{
				// Get the next son
				sonTree->GetRight(&nextSonTree);
				sonIndex++;
				
				if(sonIndex<count)
				{
					sonTree = nextSonTree;
				}
				else if(sonIndex<curCount)
				{
					if(MCVerify(nextSonTree))
						nextSonTree->Unlink(&treeElement);
				}

			}
		}
	}
}
*/

I3DShTreeElement* BuildingPrim::CreatePrivateGroup( I3DShTreeElement* buildingTreeElement )
{
	TMCCountedPtr<I3DShGroup> privateGroup;
	gComponentUtilities->CoCreateInstance(CLSID_StandardGroup, NULL, 1, IID_I3DShGroup, (void **)&privateGroup);
	ThrowIfNil(privateGroup);
	TMCCountedPtr<I3DShTreeElement> privateGroupTree;
	privateGroup->QueryInterface(IID_I3DShTreeElement, (void**)&privateGroupTree);
	privateGroupTree->SetName( TMCDynamicString( "Building Private" ) );

	buildingTreeElement->InsertLast( privateGroupTree );
	privateGroupTree->DoOpenClose(false);

	fPrivateGroupTreePath = privateGroupTree->GetTreeIDPath();

	return privateGroupTree;
}

// Find the private group to contain the children
// WARNING: works only with 1 instance of the building
I3DShTreeElement* BuildingPrim::GetPrivateGroup( I3DShTreeElement* buildingTreeElement )
{
	if( !fPrivateGroupTreePath.isValid() )
	{
		CreatePrivateGroup(buildingTreeElement);
	}

	TMCCountedPtr<I3DShScene> scene;
	GetScene( &scene );

	TMCCountedPtr<I3DShTreeElement> privateGroupTree;
	scene->GetTreeByIDPath(&privateGroupTree, fPrivateGroupTreePath);

	if( !privateGroupTree )
	{	// Something's wrong: maybe the private group under the building was erased manually.
		fPrivateGroupTreePath = TTreeIdPath::InvalidPath();
		privateGroupTree = CreatePrivateGroup(buildingTreeElement);
	}

	return privateGroupTree;
}


////////////////////////////////////////////////////////////////////////////////

MCCOMErr BuildingPrim::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	try
	{
		MCCOMErr result=MC_S_OK;

		if(!IsSerialValid())
		{
			stream->SkipTokenData();
			// Tell the user we won't read his building
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
			TMCDynamicString message;
			gResourceUtilities->GetIndString( message, kAlertStrings, 4);
			gPartUtilities->Alert(message);

			return result;
		}

		// Serial Number check (not used with the build for EOVIA)
		if( !IsSerialValid() )
		{
			stream->SkipTokenData();
			return result;
		}

		int8 token[256];
		result=stream->GetNextToken(token);
		if (result) return result;

		if (token[0] != '{') return MC_S_FALSE;

		return ReadContent(stream);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPrim::Read"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPrim::Read"));
	}

	return MC_S_FALSE;
}

MCCOMErr BuildingPrim::ReadContent(IShTokenStream* stream)
{
	MCCOMErr result=MC_S_OK;
	
	fData.Validate();

	// Remove any previous level
	fLevelArray.ArrayFree();
	fGroundLevelIndex = 0;

	DefaultShadingDomainList();

	real32 fileVersion = 0;

	int8 token[256];
	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token))
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		switch (keyword)
		{
		case 'Buil': // Start reading the building here
			{
				result=stream->GetNextToken(token);
				if (result) return result;

				if (token[0] != '{') return MC_S_FALSE;

				result=stream->GetNextToken(token);
				if (result) return result;

				while (!stream->IsEndToken(token))
				{
					int32 keyword;
					stream->CompactAttribute(token,&keyword);
			
					switch (keyword)
					{
					case 'Vers':
						{
							result=stream->GetQuickFix(&fileVersion);
 							if (result) return result;
							break;
						} break;
					case 'UVLe':
						{
							real32 uvLength=108;
							result=stream->GetQuickFix(&uvLength);
							fData.UVData().mDefaultUVLength = uvLength;
 							if (result) return result;
							break;
						} break;
					case 'Priv':
						{
							fPrivateGroupTreePath = TTreeIdPath::Read( stream->GetStream() );
							if (result) return result;
						} break;
					case 'GrdI':
						{
							SetGroundLevelIndex( stream->GetInt32Token() );
							break;
						}
					case 'Leve':
						{
							result=stream->GetNextToken(token);
							if (result) return result;

							if (token[0] != '{') return MC_S_FALSE;

							Level* newLevel = NULL;
							Level::CreateLevel( &newLevel, this, NULL, NULL );
							MY_ASSERT(newLevel);

							// Add the level before reading its content: when reading object (stairways), the level index is needed
							AddLevelToArray(newLevel, kLastLevel);

							newLevel->Read(stream);
							break;
						}

					case 'ShDo':
						{
							// Shading domain
							char name[256];
							result=stream->GetString(name);
							if (result) return result;
							TMCDynamicString nameStr;
							nameStr.FromCPtr(name);
							AddShadingDomain(nameStr);
							break;
						}

					case 'FBRP':
						{	// Front Back backdrop relative path name
							fData.SetFBBackdrop( ReadRelativePathName( stream->GetStream(), fData.GetFBBackdrop() ));
							break;
						}

					case 'LRRP':
						{	// Front Back backdrop relative path name
							fData.SetLRBackdrop( ReadRelativePathName( stream->GetStream(), fData.GetLRBackdrop() ));
							break;
						}

					case 'TBRP':
						{	// Front Back backdrop relative path name
							fData.SetTBBackdrop( ReadRelativePathName( stream->GetStream(), fData.GetTBBackdrop() ));
							break;
						}

					default:
						stream->SkipTokenData();
						break;
					}

				result=stream->GetNextToken(token);
				if (result)
					return result;
				}

			} break;

		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	if( fileVersion<1.3 )
	{	// Try to guess the best UV scaling
		const int32 levelCount = GetLevelCount();
		real32 totalHeight = 0;
		for( int32 iLevel=0 ; iLevel<levelCount ; iLevel++ )
		{
			totalHeight += fLevelArray[iLevel]->GetLevelHeight();
		}
		real32 averageHeight = totalHeight/levelCount;

		// in the old files, the default level height was 3. In the new ones, it's 108 (9 feet = 9*12 inches)

		if( averageHeight <=10 )
			fData.UVData().mDefaultUVLength = 10; // small size building: 10 instead of 108
	}
	
	return result;
}

MCCOMErr BuildingPrim::FinishRead		(IStreamContext* streamContext)
{
	return MC_S_OK;
}

TMCString255 BuildingPrim::ReadRelativePathName( TMCiostream& iostream, const TMCString& knownName)
{
	TMCString255 fullPathName = knownName;

	// read relative path
	int8 token[255]; // max size
	iostream.readFileRef(token);

	TMCString255 relativePath(token);

	// Get the file name stored in the PMap
	TMCCountedPtr<IMCFile> file;
	gFileUtilities->CreateIMCFile(&file);

	file->SetWithFullPathName(fullPathName);

	if ( !file->Exists() )
	{
		// first use the relative path to find the file
		TMCDynamicString fileName;
		file->GetFileName(fileName);

		relativePath+=fileName;

		file->SetWithFullPathName(relativePath);

		if (file->Exists())
		{
			file->GetFileFullPathName(fullPathName);
		}
		else
		{
			// we did not find the file, lets open a dialog
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');

			TMCDynamicString inTitle;	// string "cannot find the file:"
			gResourceUtilities->GetIndString(inTitle, kPrimitiveStrings , 2);
			inTitle += fileName;

			TMCArray<APITypeAndName> outTypes;
			gFileFormatUtilities->GetFileTypes(outTypes, 'imag', false); //false: save formats
			
			if (MCVerify(outTypes.GetElemCount() > 0))
			{
				TMCArray<IDType> inTypes;
				for (unsigned long i = 0 ; i < outTypes.GetElemCount(); i++)
				{
					inTypes.AddElem(outTypes[i].fID);
				}

				IDType format1,format2;
				if (gFileFormatUtilities->OpenFileDialog(inTitle,file,inTypes,format1,format2) == MC_S_OK)
				{
					file->GetFileFullPathName(fullPathName);
				}
			}
		}
	}

	return fullPathName;
}

MCCOMErr BuildingPrim::Write(IShTokenStream* stream)
{
	try
	{
		if(!IsSerialValid())
		{
			// Tell the user we won't save his building
			CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
			TMCDynamicString message;
			gResourceUtilities->GetIndString( message, kAlertStrings, 3);
			gPartUtilities->Alert(message);

			return MC_S_OK;
		}

		// Serial Number check (not used with the build for EOVIA)
		if( !IsSerialValid() )
		{
			return MC_S_OK;
		}

		return WriteContent(stream);
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("BuildingPrim::Write"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("BuildingPrim::Write"));
	}

	return MC_S_FALSE;

}

MCCOMErr BuildingPrim::WriteContent(IShTokenStream* stream)
{
	MCCOMErr result=MC_S_OK;

	// Start writing the primitive
	result = stream->PutKeywordAndBegin('Buil');	
	if (result) return result;

	// File version
	result=stream->PutKeyword('Vers');
	if (result) return result;
	result=stream->PutQuickFix(FILE_VERSION_NUMBER);
	if (result) return result;

	// Global UV length
	result=stream->PutKeyword('UVLe');
	if (result) return result;
	result=stream->PutQuickFix(fData.UVData().mDefaultUVLength);
	if (result) return result;

	// Private group under the tree element
	fPrivateGroupTreePath.Write( stream->GetStream(), 'Priv' );
	
	// Shading domains list
	const int32 totalCount = fShadingDomains.GetElemCount();
	for(int32 iDom=kBasicDomainsCount ; iDom<totalCount ; iDom++)
	{
		result=stream->PutKeyword('ShDo');
		if (result) return result;
		char name[256];
		fShadingDomains[iDom].fName.ToCPtr(name);
		result=stream->PutString(name);
		if (result) return result;
	}
	
	// Levels
	const int32 levelCount = fLevelArray.GetElemCount();
	for(int32 iLevel=0 ; iLevel<levelCount ; iLevel++)
	{
		result = fLevelArray[iLevel]->Write(stream);
		if (result) return result;
	}
	
	// Ground level index
	stream->PutInt32Attribute('GrdI', GetGroundLevelIndex());

	// Write the backdrops relative path names
	TMCCountedPtr<IMCFile> file;
	gFileUtilities->CreateIMCFile(&file);
	TMCiostream& iostream = stream->GetStream();

	const TMCDynamicString& FBName = fData.GetFBBackdrop();
	if(FBName.Length())
	{
		file->SetWithFullPathName(FBName);

		TMCDynamicString relativePath;
		file->GetPathName(relativePath);

		result=stream->PutKeyword('FBRP');
		if (result) return result;

		//TMCDynamicString outPath = value;	
		//outPath.TranslateBeforeWrite(outPath);

		iostream.writeFileRef(relativePath.StrGet());
	//	MCToken::PutPathRefAttribute(stream->GetStream(),'FBRP', relativePath);
	}

	const TMCDynamicString& LRName = fData.GetLRBackdrop();
	if(LRName.Length())
	{
		file->SetWithFullPathName(LRName);

		TMCDynamicString relativePath;
		file->GetPathName(relativePath);

		result=stream->PutKeyword('LRRP');
		if (result) return result;

		iostream.writeFileRef(relativePath.StrGet());
	}

	const TMCDynamicString& TBName = fData.GetTBBackdrop();
	if(TBName.Length())
	{
		file->SetWithFullPathName(TBName);

		TMCDynamicString relativePath;
		file->GetPathName(relativePath);

		result=stream->PutKeyword('TBRP');
		if (result) return result;

		iostream.writeFileRef(relativePath.StrGet());
	}

	result = stream->PutEnd();

	return result;
}

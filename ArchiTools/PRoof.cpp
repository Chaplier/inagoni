/****************************************************************************************************

		PRoof.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/17/2004

****************************************************************************************************/

#include "PRoof.h"

#include "MCCountedPtrHelper.h"
#include "IShTokenStream.h"
#include "PLevel.h"
#include "PTessellator.h"
#include "Geometry.h"
#include "MiscComUtilsImpl.h"
#include "BuildingDef.h"
#include "I3dShFacetMesh.h"

Roof::Roof(BuildingPrimData* data, Level* inLevel)
{
	fLevel = inLevel;
	fLevel->LevelPlan().AddRoofReference(this);

	fFlags = 0;

	fOutTopDomain = eOutsideTopRoofDomain;
	fOutMidDomain = eOutsideMidRoofDomain;
	fOutBotDomain = eOutsideBotRoofDomain;
	fInsideDomain = eInsideRoofDomain;

	fOutsideOffset = 0;

	fData = data;

	fRoofMin = kDefaultLevelHeight;
	fRoofMax = kDefaultRoofHeight;

	RoofProfile::CreateProfile(&fProfile, this, 0);

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 10);
	SetName(fData->fDictionary, objectName);
}

Roof::~Roof()
{
}

void Roof::CreateRoof(Roof **newRoof, BuildingPrimData* data, Level* inLevel)
{
	TMCCountedCreateHelper<Roof> result(newRoof);

	result = new Roof(data,inLevel);
	ThrowIfNoMem(result);
}

void Roof::DeleteRoof()
{
	fProfile->ClearData();
	fZone.ClearData();

	TMCCountedPtr<Level> levelPtr = fLevel;
	fLevel = NULL;
	levelPtr->LevelPlan().RemoveRoofReference(this);
}

void Roof::RemoveProfilePointReferences(ProfilePoint* point)
{
	ClearFlag(eRoofTessellated);

	if(point->Flag(eTopProfileFlag))
	{
		TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetTopProfile();
		const int32 profileCount = profile.GetElemCount();
		for(int32 iPt=0 ; iPt<profileCount ; iPt++)
		{
			if( profile[iPt] == point  )
			{
				profile.RemoveElem(iPt,1);
				return;
			}
		}
		TMCCountedPtrArray<ProfilePoint>& inside = fProfile->GetTopInside();
		const int32 insideCount = inside.GetElemCount();
		for(int32 iIn=0 ; iIn<insideCount ; iIn++)
		{
			if( inside[iIn] == point  )
			{
				inside.RemoveElem(iIn,1);
				return;
			}
		}
	}
	else
	{
		TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetBotProfile();
		const int32 profileCount = profile.GetElemCount();
		for(int32 iPt=0 ; iPt<profileCount ; iPt++)
		{
			if( profile[iPt] == point  )
			{
				profile.RemoveElem(iPt,1);
				return;
			}
		}
		TMCCountedPtrArray<ProfilePoint>& inside = fProfile->GetBotInside();
		const int32 insideCount = inside.GetElemCount();
		for(int32 iIn=0 ; iIn<insideCount ; iIn++)
		{
			if( inside[iIn] == point  )
			{
				inside.RemoveElem(iIn,1);
				return;
			}
		}
	}
}

bool Roof::GetRoofFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags)
{
	FacetMesh::Create(outMesh);

	TMCCountedPtr<FacetMesh> facetMesh;
	facetMesh = *outMesh;

	TMCClassArray<Vertex>& vertices = Vertices();
	const int32 vertexCount = vertices.GetElemCount();

	TMCClassArray<Triangle>& facets = Triangles();
	const int32 facetCount = facets.GetElemCount();
		
	if(facetCount==0)
	{
		return false;
	}

	const boolean needcolor = FLAG(meshFlags, eShellMesh)?false:true;

	TMCArray<TVector3>& meshVertices = facetMesh->fVertices;
	TMCArray<TVector2>& meshUVs = facetMesh->fuv;
	TMCArray<TVector3>& meshNormals = facetMesh->fNormals;
	TMCArray<Triangle>& meshFacets = facetMesh->fFacets;
	TMCArray<uint32>& meshUVSpaceIDs = facetMesh->fUVSpaceID;
	TMCArray<TMCColorRGBA8>& meshColors = facetMesh->fPolygonColors;

	meshVertices.SetElemCount(vertexCount); 
	meshUVs.SetElemCount(vertexCount); 
	meshNormals.SetElemCount(vertexCount); 
	meshFacets.SetElemCount(facetCount);   
	meshUVSpaceIDs.SetElemCount(facetCount);   
	if(needcolor)
		meshColors.SetElemCount(facetCount);
//	facetMesh->fPolygonBackColors.SetElemCount(facetCount);

	const TMCColorRGBA8 color = (Selected()?fData->fSelCol:
								(Targeted()?fData->fTarCol:
								Flag(eSnapedPosition)?fData->fSnaCol:fData->fDefCol));

	const boolean faceted = FLAG(meshFlags, eFaceted);

	// Fill the vertices positions
	for( int32 iVertex=0 ; iVertex<vertexCount ; iVertex++ )
	{
		const int32 index = iVertex;
		meshVertices[index] = vertices[iVertex].Position();
		meshUVs[index] = vertices[iVertex].UV();
		meshNormals[index] = vertices[iVertex].Normal();
	}

	// Fill the facets
	for( int32 iFacet=0 ; iFacet<facetCount ; iFacet++ )
	{
		meshFacets[iFacet] = facets[iFacet];
		meshUVSpaceIDs[iFacet] = fTriangleDomain[iFacet];
		if(needcolor)
		{
			if(faceted)
				meshColors[iFacet] = GetFacetedColor(meshNormals[meshFacets[iFacet].pt1], color);
			else
				meshColors[iFacet] = color;
		}
	}

	return true;
}
/*
boolean CheckVect(const TVector3& vect)
{
	if(vect.x<-10000){MCNotify("weird vector"); return false;}
	if(vect.y<-10000){MCNotify("weird vector"); return false;}
	if(vect.z<-10000){MCNotify("weird vector"); return false;}
	if(vect.x>10000){MCNotify("weird vector"); return false;}
	if(vect.y>10000){MCNotify("weird vector"); return false;}
	if(vect.z>10000){MCNotify("weird vector"); return false;}

	return true;
}
*/
void Roof::BuildRectangle(	const TVector3& pos0,
							const TVector3& pos1,
							const TVector3& pos2,
							const TVector3& pos3,
							const TVector3& normal,
							const TVector2& UV0,
							const TVector2& UV1,
							const TVector2& UV2,
							const TVector2& UV3,
							TMCClassArray<Triangle>& triangles)
{
	const int32 startPos = fVertexArray.GetElemCount();
	{
		Vertex vtx(pos0,normal,UV0);
		fVertexArray.AddElem(vtx);
	}
	{
		Vertex vtx(pos1,normal,UV1);
		fVertexArray.AddElem(vtx);
	}
	{
		Vertex vtx(pos2,normal,UV2);
		fVertexArray.AddElem(vtx);
	}
	{
		Vertex vtx(pos3,normal,UV3);
		fVertexArray.AddElem(vtx);
	}

	Triangle triangle1(startPos,startPos+1,startPos+3);
	triangles.AddElem( triangle1 );

	Triangle triangle2(startPos,startPos+3,startPos+2);
	triangles.AddElem( triangle2 );
}

real32 Roof::GetVerticalBase(const TVector3& curZonePos, const TVector3& nextZonePos, // Use these pos if possible
					   TVector3& direction, 
					   TVector3& inNormal, // normal inside
					   const TVector3& curSpinePos, const TVector3& nextSpinePos ) // Emergency point if the other are on the same pos
{
	direction = nextZonePos-curZonePos;
	const real32 length = direction.Normalize(direction);

	const real32 fact = Flag(eRoofZoneOrientedPositive)?-1:1;

	if(length<kRealEpsilon)
	{	// Use the 2 other points to have some data
		direction = nextSpinePos-curSpinePos;
		const real32 otherLength = direction.Normalize(direction);

		inNormal.x = -fact*direction.y;
		inNormal.y = fact*direction.x;
		inNormal.z = 0; 

		return otherLength;
	}

	inNormal.x = -fact*direction.y;
	inNormal.y = fact*direction.x;
	inNormal.z = 0; 

	return length;
}

TVector2 Roof::ComputeUV(const TVector2& pos)
{
	switch( fData->UVData().mMethod )
	{
	default:
	case eProportional: return fData->UVData().ComputeUV( pos );
	case eUnfold:
		{
			return fData->UVData().ComputeUV( TVector2(
				pos.x + mUnfoldUVData.mOffsetU , 
				pos.y + mUnfoldUVData.mOffsetV ) );
		}
	}
}

boolean Roof::BuildNextRectangle(	const TVector2& next,
								const TVector3& curNormal,
								const TVector3& curDir,
								const TVector3& prevPlaneNormal,
								const TVector3& nextPlaneNormal,
								const TVector3& curPos,
								const TVector3& nextPos,
								const EUVArea shadingArea,
								TVector3& curProj0,
								TVector3& curProj1,
								real32& cummulatedVOffset,
								real32& uDist0,
								real32& uDist1,
								TMCClassArray<Triangle>& triangles,
								const boolean flip) // normal orientation
{
	boolean build = true;

	const TVector3 newPos = curPos + next.x*curNormal +next.y*TVector3::kUnitZ;

	MY_ASSERT(RealAbs( prevPlaneNormal * curDir) > kRealEpsilon);

	TVector3 nextProj0=curProj0;
	if( !IntersectLinePlane2(newPos, curDir, prevPlaneNormal, curPos, nextProj0	) )
	{
		MCNotify("Intersection not found: error");
	}

	TVector3 nextProj1=curProj1;
	if( !IntersectLinePlane2(newPos, curDir, nextPlaneNormal, nextPos, nextProj1 ) )
	{
		MCNotify("Intersection not found: error");
	}

	TVector3 normal  = curDir^(nextProj0-curProj1);
	if( normal.Normalize() )
	{
		// UV determination
		const real32 VOffset = (curProj0-nextProj0).GetNorm();
		TVector3 projNext0 = TVector3::kZero;
		DistancePointToLine( nextProj0, curPos, curDir, projNext0);
		const real32 nextUDist0 = (projNext0-curPos)*curDir;
		TVector3 projNext1 = TVector3::kZero;
		DistancePointToLine( nextProj1, curPos, curDir, projNext1);
		const real32 nextUDist1 = (projNext1-curPos)*curDir;
		const TVector2 UV0 = ComputeUV(TVector2(uDist0,cummulatedVOffset)); // , shadingArea);
		const TVector2 UV1 = ComputeUV(TVector2(uDist1,cummulatedVOffset)); // , shadingArea);
		const TVector2 UV2 = ComputeUV(TVector2(nextUDist0,cummulatedVOffset+VOffset)); // , shadingArea);
		const TVector2 UV3 = ComputeUV(TVector2(nextUDist1,cummulatedVOffset+VOffset)); // , shadingArea);
		if(flip)
			BuildRectangle(nextProj0,nextProj1,curProj0,curProj1,-normal,
			UV2,UV3,UV0,UV1,triangles); // Order them to have the facet normal pointing outside
		else
			BuildRectangle(curProj0,curProj1,nextProj0,nextProj1,normal,
			UV0,UV1,UV2,UV3,triangles); // Order them to have the facet normal pointing outside
		
		// Go to next point
		cummulatedVOffset += VOffset;
		uDist0 = nextUDist0;
		uDist1 = nextUDist1;
	}

	// Go to next point
	curProj0 = nextProj0;
	curProj1 = nextProj1;

	return build;
}

// return the signed distance of a point on a ligne (the 3 points must be aligned)
real32 GetSignedDistance(const TVector2& point, 
						 const TVector2& origin, 
						 const TVector2& dir)
{
	const TVector2 vect = origin-point;
	const real32 dist = vect.GetNorm();
	if(dir*vect > 0)
		return -dist;
	else 
		return dist;
}

void Roof::GetRectangleUV( const TVector3& curBotProj0, 
						   const TVector3& curBotProj1, 
						   const TVector3& curTopProj0, 
						   const TVector3& curTopProj1,
						   real32& cummulatedVOffset,
						   TVector2& UV0,
						   TVector2& UV1,
						   TVector2& UV2,
						   TVector2& UV3, 
						   EUVArea shadingArea )
{
	// UV determination
	const TVector3 botAxis = curBotProj1 - curBotProj0;
	const TVector3 topAxis = curTopProj1 - curTopProj0;
	TVector3 botDir=botAxis;
	TVector3 topDir=topAxis;

	const real32 botLength = botAxis.Normalize(botDir);
	const real32 topLength = topAxis.Normalize(topDir);

	real32 dist = 0;

	TVector3& refAxis = botDir;
	if(botLength<kRealEpsilon)
	{	// 2 point in the same location: use the top to find the axis
		// Find another axis but keep refLength as is: it's use for the UV computation
		if(topLength>=kRealEpsilon) refAxis = topDir;
		else refAxis = TVector3::kUnitX; // a null surface
	}
	TVector3 projTop0 = TVector3::kZero;
	const real32 l0 = DistancePointToLine( curTopProj0, curBotProj0, refAxis, projTop0);
	const real32 nextUDist0 = (projTop0-curBotProj0)*refAxis - dist;
	TVector3 projTop1 = TVector3::kZero;
	const real32 l1 = DistancePointToLine( curTopProj1, curBotProj0, refAxis, projTop1);
	const real32 nextUDist1 = (projTop1-curBotProj0)*refAxis - dist;
	UV0 = ComputeUV(TVector2(0,cummulatedVOffset)); // , shadingArea);
	UV1 = ComputeUV(TVector2(botLength,cummulatedVOffset)); // , shadingArea);
	UV2 = ComputeUV(TVector2(nextUDist0,cummulatedVOffset+l0)); // , shadingArea);
	UV3 = ComputeUV(TVector2(nextUDist1,cummulatedVOffset+l1)); // , shadingArea);
}

//Arnaud: a 700 lines function !!! :-O
//Code warrior optimizer doesn't like this at all...
#if MAC
#pragma ppc_opt_defuse_mem_limit 150
#endif

void Roof::ValidateTessellation()
{
	if( Flag(eRoofTessellated) ) 
		return; // Tessellation is valid

	SetFlag(eRoofTessellated);

	// Clear previous data
	fTriangleArray.ArrayFree();
	fTriangleDomain.ArrayFree();
	fVertexArray.ArrayFree();

	mUnfoldUVData.mOffsetU = 0;

	TMCClassArray<Triangle> outsideTriangles; // we separate the inside from the outside to do some picking on one part or the other
	TMCArray<int32>			outsideDomains; // shading domains
	fOutsideOffset = 0;

	const real32 levelAltitude = fLevel->GetDistanceToGround();
	const real32 zMin = levelAltitude + GetRoofMin();
	const real32 zMax = levelAltitude + GetRoofMax();
	const real32 roofHeight = zMax-zMin;

	// 1 Extrude the polyline along the zone
	const int32 zonePointCount = fZone.GetSectionCount();
	MY_ASSERT(zonePointCount>=3);

	const int32 botCountOut = fProfile->GetBotProfilePointCount();
	const int32 topCountOut = fProfile->GetTopProfilePointCount();
	const int32 botCountIn = fProfile->GetBotInsidePointCount();
	const int32 topCountIn = fProfile->GetTopInsidePointCount();

	const real32 fact = Flag(eRoofZoneOrientedPositive)?1:-1;
			
	const ZoneSection& prev = fZone.GetZoneSection(zonePointCount-3);
	const ZoneSection& cur = fZone.GetZoneSection(zonePointCount-2);
	const ZoneSection& next = fZone.GetZoneSection(zonePointCount-1);

	// Pos
	TVector3 prevZonePos	= prev.fZonePoint->Get3DPos();
	TVector3 prevSpinePos	= prev.fSpinePoint->Get3DPos();
	TVector3 curZonePos		= cur.fZonePoint->Get3DPos();
	TVector3 curSpinePos	= cur.fSpinePoint->Get3DPos();
	TVector3 nextZonePos	= next.fZonePoint->Get3DPos();
	TVector3 nextSpinePos	= next.fSpinePoint->Get3DPos();

	// Other data
	TVector3 prevBottomDir		= TVector3::kUnitX;
	TVector3 prevBottomNormal	= TVector3::kUnitY;
	real32 prevBotLength = GetVerticalBase(prevZonePos, curZonePos,prevBottomDir,prevBottomNormal,prevSpinePos, curSpinePos);		
	boolean prevIsVertical = false;
	// Negative value and zero are vertical
	if( prevBottomNormal*(prevSpinePos-prevZonePos)<=kRealEpsilon && prevBottomNormal*(curSpinePos-curZonePos)<=kRealEpsilon )
		 prevIsVertical = true;

	TVector3 curBottomDir		= TVector3::kUnitY;
	TVector3 curBottomNormal	= TVector3::kUnitX;
	real32 curBotLength = GetVerticalBase(curZonePos, nextZonePos,curBottomDir,curBottomNormal,curSpinePos, nextSpinePos);		
	boolean curIsVertical = false;
	// Negative value and zero are vertical
	if( curBottomNormal*(curSpinePos-curZonePos)<=kRealEpsilon && curBottomNormal*(nextSpinePos-nextZonePos)<=kRealEpsilon )
		 curIsVertical = true;

	TVector3 botPrevPlaneNormal = TVector3::kUnitY;
	TVector3 topPrevPlaneNormal = TVector3::kUnitY;
	TVector3 dir0 = prevSpinePos-curSpinePos;
	if(dir0.GetSquaredNorm()<kRealEpsilon)
		dir0 = prevZonePos-curZonePos;
	TVector3 dir1 = nextSpinePos-curSpinePos;
	if(dir1.GetSquaredNorm()<kRealEpsilon)
		dir1 = nextZonePos-curZonePos;

	if(curIsVertical)
	{
		botPrevPlaneNormal = prevZonePos-curZonePos;
		if(botPrevPlaneNormal.GetSquaredNorm()<kRealEpsilon)
			botPrevPlaneNormal = dir0;
		topPrevPlaneNormal = dir0;
		botPrevPlaneNormal.Normalize();
		topPrevPlaneNormal.Normalize();
	}
	else if(prevIsVertical)
	{
		botPrevPlaneNormal = curZonePos-nextZonePos;
		if(botPrevPlaneNormal.GetSquaredNorm()<kRealEpsilon)
			botPrevPlaneNormal = dir1;
		topPrevPlaneNormal = dir1;
		botPrevPlaneNormal.Normalize();
		topPrevPlaneNormal.Normalize();
	}
	else // Usual case: get the bissectrice
	{
		TVector3 vec0 = prevZonePos-curZonePos;
		if(vec0.GetSquaredNorm()<kRealEpsilon)	vec0 = dir0;
		else									vec0.Normalize();

		TVector3 vec1 = nextZonePos-curZonePos;
		if(vec1.GetSquaredNorm()<kRealEpsilon)	vec1 = dir1;
		else									vec1.Normalize();

		const TVector3 prevBotBissect = BissectDir2(vec0,vec1);
		const TVector3 prevTopBissect = BissectDir2(dir0,dir1);

		botPrevPlaneNormal = prevBotBissect^TVector3::kUnitZ;
		topPrevPlaneNormal = prevTopBissect^TVector3::kUnitZ;
	}

	for(int32 iPt=0 ; iPt<zonePointCount ; iPt++)
	{
		const ZoneSection& nextNext = fZone.GetZoneSection(iPt);
		const int32 nextIndex = iPt>0?iPt-1:zonePointCount-1;
		ZoneSection& zoneSection = fZone.GetZoneSection(nextIndex);

		const TVector3 nextNextZonePos = nextNext.fZonePoint->Get3DPos();
		const TVector3 nextNextSpinePos = nextNext.fSpinePoint->Get3DPos();

		TVector3 nextBottomDir		= TVector3::kUnitX;
		TVector3 nextBottomNormal	= TVector3::kUnitY;
		const real32 nextBotLength = GetVerticalBase(nextZonePos, nextNextZonePos,nextBottomDir,nextBottomNormal,nextSpinePos, nextNextSpinePos);		
		boolean nextIsVertical = false;
		// Negative value and zero are vertical
		if( nextBottomNormal*(nextSpinePos-nextZonePos)<=kRealEpsilon && nextBottomNormal*(nextNextSpinePos-nextNextZonePos)<=kRealEpsilon )
			 nextIsVertical = true;

		TVector3 botNextPlaneNormal = TVector3::kUnitX;
		TVector3 topNextPlaneNormal = TVector3::kUnitX;
		TVector3 dir0 = curSpinePos-nextSpinePos;
		if(dir0.GetSquaredNorm()<kRealEpsilon)
			dir0 = curZonePos-nextZonePos;
		TVector3 dir1 = nextNextSpinePos-nextSpinePos;
		if(dir1.GetSquaredNorm()<kRealEpsilon)
			dir1 = nextNextZonePos-nextZonePos;

		if(curIsVertical)
		{
			botNextPlaneNormal = nextNextZonePos-nextZonePos;
			if(botNextPlaneNormal.GetSquaredNorm()<kRealEpsilon)
				botNextPlaneNormal = dir1;
			topNextPlaneNormal = dir1;
			botNextPlaneNormal.Normalize();
			topNextPlaneNormal.Normalize();
		}
		else if(nextIsVertical)
		{
			botNextPlaneNormal = curZonePos-nextZonePos;
			if(botNextPlaneNormal.GetSquaredNorm()<kRealEpsilon)
				botNextPlaneNormal = dir0;
			topNextPlaneNormal = dir0;
			botNextPlaneNormal.Normalize();
			topNextPlaneNormal.Normalize();
		}
		else // Usual case: get the bissectrice
		{			
			TVector3 vec0 = curZonePos-nextZonePos;
			if(vec0.GetSquaredNorm()<kRealEpsilon)	vec0 = dir0;
			else									vec0.Normalize();

			TVector3 vec1 = nextNextZonePos-nextZonePos;
			if(vec1.GetSquaredNorm()<kRealEpsilon)	vec1 = dir1;
			else									vec1.Normalize();

			const TVector3 nextBotBissect = BissectDir2(vec0,vec1);
			const TVector3 nextTopBissect = BissectDir2(dir0,dir1);
		
			botNextPlaneNormal = nextBotBissect^TVector3::kUnitZ;
			topNextPlaneNormal = nextTopBissect^TVector3::kUnitZ;
		}

		// Allways project the profile on the bissectrice of the angle, to avoid size modifications
		// Extrude the bottom of the profile
		// Get the proj direction at the bottom
		if(curIsVertical)
		{
			zoneSection.SetIsVertical(true);

			// The spine pos is outside the roof zone => build an extremity

			const int32 baseVtxIndex = fVertexArray.GetElemCount();
			int32 index = baseVtxIndex;
			const TVector2 nextBotPos2D((nextZonePos-curZonePos).GetNorm(),0);
			const TVector2 curBotPos2D(-(curZonePos-curZonePos).GetNorm(),0);

			const TVector2 curTopPos2D((curSpinePos.CastToXY()-curZonePos.CastToXY()).GetNorm(),roofHeight);
			const TVector2 nextTopPos2D((nextSpinePos.CastToXY()-curZonePos.CastToXY()).GetNorm(),roofHeight);

			// Build the polygons at the extremity: we need to determine the outside 
			// positions as in the normal case 

			// Separate the polygon in 3 parts: easier to tessellate and better normal if not flat
			// The zone in the middle is a big rectangle representing the elastic zone
			FlatPolygonMaker curBotPolygonMaker;
			FlatPolygonMaker curMidPolygonMaker;
			FlatPolygonMaker curTopPolygonMaker;
		
			FlatPolygonMaker nextBotPolygonMaker;
			FlatPolygonMaker nextMidPolygonMaker;
			FlatPolygonMaker nextTopPolygonMaker;
		
			const TVector2 curZoneOrigin2D = TVector2::kZero;
			const TVector2 nextZoneOrigin2D((nextZonePos.CastToXY()-curZonePos.CastToXY()).GetNorm(),0);
			const TVector2 curSpineOrigin2D((curSpinePos.CastToXY()-curZonePos.CastToXY()).GetNorm(),roofHeight);
			const TVector2 nextSpineOrigin2D((nextSpinePos.CastToXY()-curZonePos.CastToXY()).GetNorm(),roofHeight);

			int32 curBotInIndex = index;
			if(!prevIsVertical)
			{
				if(!botCountOut)
					curMidPolygonMaker.AddPointToCountainer( curZoneOrigin2D, index, false );

				curBotPolygonMaker.AddPointToCountainer( curZoneOrigin2D, index++, false );
				Vertex vtx1(curZonePos,-botPrevPlaneNormal,ComputeUV(curZoneOrigin2D)); // ,eRoofSection));
				fVertexArray.AddElem(vtx1);
			}
			int32 nextBotInIndex = index;
			if(!nextIsVertical)
			{
				if(!botCountOut)
					nextMidPolygonMaker.AddPointToCountainer( nextZoneOrigin2D, index, false );

				nextBotPolygonMaker.AddPointToCountainer( nextZoneOrigin2D, index++, false );
				Vertex vtx2(nextZonePos,-botNextPlaneNormal,ComputeUV(nextZoneOrigin2D)); // , eRoofSection));
				fVertexArray.AddElem(vtx2);
			}

			const TVector3 curBotAxis( -fact*botPrevPlaneNormal.y, fact*botPrevPlaneNormal.x,0);
			const TVector3 nextBotAxis( fact*botNextPlaneNormal.y, -fact*botNextPlaneNormal.x,0);
			const TVector3 curTopAxis( -fact*topPrevPlaneNormal.y, fact*topPrevPlaneNormal.x,0);
			const TVector3 nextTopAxis( fact*topNextPlaneNormal.y, -fact*topNextPlaneNormal.x,0);

			TMCArray<int32> curMidRectangleIndexes; 
			TMCArray<int32> nextMidRectangleIndexes; // Rmember these points, we'll compute their normal at the end
			curMidRectangleIndexes.SetElemSpace(4); // max count
			nextMidRectangleIndexes.SetElemSpace(4); // max count
			
			{	// Go up outside along the bottom part
				for(int32 iBot=0 ; iBot<botCountOut ; iBot++)
				{
					const TVector2& pos = fProfile->GetBotProfilePos(iBot);

					if(!prevIsVertical)
					{
						const TVector2 curPos = curZoneOrigin2D + pos; 
						curBotPolygonMaker.AddPointToCountainer( curPos, index++, false );
						Vertex vtx1(curZonePos+pos.x*curBotAxis+pos.y*TVector3::kUnitZ,-botPrevPlaneNormal,ComputeUV(curPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// Last point: add it for the middle rectangle
						if(iBot==botCountOut-1)
						{
							curMidRectangleIndexes.AddElem(index);
							curMidPolygonMaker.AddPointToCountainer( curPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}

					if(!nextIsVertical)
					{
						const TVector2 nextPos(nextZoneOrigin2D.x-pos.x,nextZoneOrigin2D.y+pos.y);
						nextBotPolygonMaker.AddPointToCountainer( nextPos, index++, false );
						Vertex vtx1(nextZonePos+pos.x*nextBotAxis+pos.y*TVector3::kUnitZ,-botNextPlaneNormal,ComputeUV(nextPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// Last point: add it for the middle rectangle
						if(iBot==botCountOut-1)
						{
							nextMidRectangleIndexes.AddElem(index);
							nextMidPolygonMaker.AddPointToCountainer( nextPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}
				}
			}
			{	// Then go down inside along the bottom part
				for(int32 iBot=botCountIn-1 ; iBot>=0 ; iBot--)
				{
					const TVector2& pos = fProfile->GetBotInsidePos(iBot);

					if(!prevIsVertical)
					{
						const TVector2 curPos = curZoneOrigin2D + pos;
						// First point: add it for the middle rectangle
						curBotPolygonMaker.AddPointToCountainer( curPos, index++, false );
						Vertex vtx1(curZonePos+pos.x*curBotAxis+pos.y*TVector3::kUnitZ,-botPrevPlaneNormal,ComputeUV(curPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						if(iBot==botCountIn-1)
						{
							curMidRectangleIndexes.AddElem(index);
							curMidPolygonMaker.AddPointToCountainer( curPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}

					if(!nextIsVertical)
					{
						const TVector2 nextPos(nextZoneOrigin2D.x-pos.x,nextZoneOrigin2D.y+pos.y);
						nextBotPolygonMaker.AddPointToCountainer( nextPos, index++, false );
						Vertex vtx1(nextZonePos+pos.x*nextBotAxis+pos.y*TVector3::kUnitZ,-botNextPlaneNormal,ComputeUV(nextPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// First point: add it for the middle rectangle
						if(iBot==botCountIn-1)
						{
							nextMidRectangleIndexes.AddElem(index);
							nextMidPolygonMaker.AddPointToCountainer( nextPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}
				}
			}
			if(!prevIsVertical)
			{
				if(!botCountIn)
					curMidPolygonMaker.AddPointToCountainer( curZoneOrigin2D, curBotInIndex, false );
			}
			if(!nextIsVertical)
			{
				if(!botCountIn)
					nextMidPolygonMaker.AddPointToCountainer( nextZoneOrigin2D, nextBotInIndex, false );
			}

			// Top part
			// For the upper part, we start by the inside to fill the middle rectangle in the right order
			int32 curTopInIndex = index;
			if(!prevIsVertical)
			{
				if(!topCountIn)
					curMidPolygonMaker.AddPointToCountainer( curSpineOrigin2D, index, false );

				curTopPolygonMaker.AddPointToCountainer( curSpineOrigin2D, index++, false );
				Vertex vtx1(curSpinePos,-topPrevPlaneNormal,ComputeUV(curSpineOrigin2D)); // , eRoofSection));
				fVertexArray.AddElem(vtx1);
			}
			int32 nextTopInIndex = index;
			if(!nextIsVertical)
			{
				if(!topCountIn)
					nextMidPolygonMaker.AddPointToCountainer( nextSpineOrigin2D, index, false );

				nextTopPolygonMaker.AddPointToCountainer( nextSpineOrigin2D, index++, false );
				Vertex vtx2(nextSpinePos,-topNextPlaneNormal,ComputeUV(nextSpineOrigin2D)); // , eRoofSection));
				fVertexArray.AddElem(vtx2);
			}

			{	// Go down the inside
				for(int32 iTop=0 ; iTop<topCountIn ; iTop++)
				{
					const TVector2& pos = fProfile->GetTopInsidePos(iTop);

					if(!prevIsVertical)
					{
						const TVector2 curPos = curSpineOrigin2D + pos;			
						// Last point: add it for the middle rectangle
						curTopPolygonMaker.AddPointToCountainer( curPos, index++, false );
						Vertex vtx1(curSpinePos+pos.x*curTopAxis+pos.y*TVector3::kUnitZ,-topPrevPlaneNormal,ComputeUV(curPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						if(iTop==topCountIn-1)
						{
							curMidRectangleIndexes.AddElem(index);
							curMidPolygonMaker.AddPointToCountainer( curPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}

					if(!nextIsVertical)
					{
						const TVector2 nextPos(nextSpineOrigin2D.x-pos.x,nextSpineOrigin2D.y+pos.y);
						nextTopPolygonMaker.AddPointToCountainer( nextPos, index++, false );
						Vertex vtx1(nextSpinePos+pos.x*nextTopAxis+pos.y*TVector3::kUnitZ,-topNextPlaneNormal,ComputeUV(nextPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// Last point: add it for the middle rectangle
						if(iTop==topCountIn-1)
						{
							nextMidRectangleIndexes.AddElem(index);
							nextMidPolygonMaker.AddPointToCountainer( nextPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}
				}
				
			}
			{	// Finaly go up outside
				for(int32 iTop=topCountOut-1 ; iTop>=0 ; iTop--)
				{
					const TVector2& pos = fProfile->GetTopProfilePos(iTop);

					if(!prevIsVertical)
					{
						const TVector2 curPos = curSpineOrigin2D + pos;			
						curTopPolygonMaker.AddPointToCountainer( curPos, index++, false );
						Vertex vtx1(curSpinePos+pos.x*curTopAxis+pos.y*TVector3::kUnitZ,-topPrevPlaneNormal,ComputeUV(curPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// First point: add it for the middle rectangle
						if(iTop==topCountOut-1)
						{
							curMidRectangleIndexes.AddElem(index);
							curMidPolygonMaker.AddPointToCountainer( curPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}

					if(!nextIsVertical)
					{
						const TVector2 nextPos(nextSpineOrigin2D.x-pos.x,nextSpineOrigin2D.y+pos.y);
						nextTopPolygonMaker.AddPointToCountainer( nextPos, index++, false );
						Vertex vtx1(nextSpinePos+pos.x*nextTopAxis+pos.y*TVector3::kUnitZ,-topNextPlaneNormal,ComputeUV(nextPos)); // , eRoofSection));
						fVertexArray.AddElem(vtx1);
						// First point: add it for the middle rectangle
						if(iTop==topCountOut-1)
						{
							nextMidRectangleIndexes.AddElem(index);
							nextMidPolygonMaker.AddPointToCountainer( nextPos, index++, false );
							fVertexArray.AddElem(vtx1); // we'll modify its normal later
						}
					}
				}
			}
			if(!prevIsVertical)
			{
				if(!topCountOut)
					curMidPolygonMaker.AddPointToCountainer( curSpineOrigin2D, curTopInIndex, false );
			}
			if(!nextIsVertical)
			{
				if(!topCountOut)
					nextMidPolygonMaker.AddPointToCountainer( nextSpineOrigin2D, nextTopInIndex, false );
			}

			// Compute the missing normal
			const int32 curMidCount = curMidRectangleIndexes.GetElemCount();
			if(curMidCount>2)
			{
				const TVector3& pos0 = fVertexArray[curMidRectangleIndexes[0]].Position();
				const TVector3& pos1 = fVertexArray[curMidRectangleIndexes[1]].Position();
				const TVector3& pos2 = fVertexArray[curMidRectangleIndexes[2]].Position();
				TVector3 normal = (pos0-pos1)^(pos0-pos2);
				normal.Normalize();
				for(int32 i=0 ; i<curMidCount ; i++)
				{
					fVertexArray[curMidRectangleIndexes[i]].SetNormal(-normal);
				}
			}
			const int32 nextMidCount = nextMidRectangleIndexes.GetElemCount();
			if(nextMidCount>2)
			{
				const TVector3& pos0 = fVertexArray[nextMidRectangleIndexes[0]].Position();
				const TVector3& pos1 = fVertexArray[nextMidRectangleIndexes[1]].Position();
				const TVector3& pos2 = fVertexArray[nextMidRectangleIndexes[2]].Position();
				TVector3 normal = (pos0-pos1)^(pos0-pos2);
				normal.Normalize();
				for(int32 i=0 ; i<nextMidCount ; i++)
				{
					fVertexArray[nextMidRectangleIndexes[i]].SetNormal(normal);
				}
			}


			int32 resultCount = 0;
			if(!prevIsVertical)
			{
				resultCount += curBotPolygonMaker.MakePolygons(fTriangleArray);
				resultCount += curMidPolygonMaker.MakePolygons(fTriangleArray);
				resultCount += curTopPolygonMaker.MakePolygons(fTriangleArray);
			}
		
			if(!nextIsVertical)
			{
				resultCount += nextBotPolygonMaker.MakePolygons(fTriangleArray);
				resultCount += nextMidPolygonMaker.MakePolygons(fTriangleArray);
				resultCount += nextTopPolygonMaker.MakePolygons(fTriangleArray);
			}
			
			TMCArray<int32> domain(resultCount,false);
			for(int32 iElem=0 ; iElem<resultCount ; iElem++)
				domain[iElem] = fOutBotDomain;
			fTriangleDomain.Append( domain );
		}
		else // Normal case: build the roof
		{
			zoneSection.SetIsVertical(false);

			// Extrude the profile between cur and next
			TVector3 curBotProj0 = curZonePos;
			TVector3 curBotProj1 = nextZonePos;
			// UV determination
			real32 cummulatedVOffset = 0;
			real32 uDist0 = 0;
			real32 uDist1 = curBotLength;
			for(int32 iBot=0 ; iBot<botCountOut ; iBot++)
			{
				const TVector2& next = fProfile->GetBotProfilePos(iBot);

				if( BuildNextRectangle(	next,
									curBottomNormal,
									curBottomDir,
									botPrevPlaneNormal,
									botNextPlaneNormal,
									curZonePos,
									nextZonePos,
									eRoofBottom,
									curBotProj0,
									curBotProj1,
									cummulatedVOffset,
									uDist0,
									uDist1,
									outsideTriangles,
									true) )
				{
					// We added 2 triangles
					outsideDomains.AddElem(fOutBotDomain);
					outsideDomains.AddElem(fOutBotDomain);
				}
			}

			// Extrude the top of the profile
			// Get the proj direction at the top
			TVector3 topDir = nextSpinePos - curSpinePos;
			const real32 topLength = topDir.GetMagnitude();
			if(topDir.GetSquaredNorm()<kRealEpsilon)
				topDir = curBottomDir; // The 2 spline points are at the same pos => use the other dir
			topDir.Normalize();
			const TVector3 curTopNormal(fact*topDir.y, -fact*topDir.x, 0);
			TVector3 curTopProj0 = curSpinePos;
			TVector3 curTopProj1 = nextSpinePos;
			// UV determination
			cummulatedVOffset = 0;
			uDist0 = 0;
			uDist1 = topLength;
			for(int32 iTop=0 ; iTop<topCountOut ; iTop++)
			{
				const TVector2& next = fProfile->GetTopProfilePos(iTop);

				if( BuildNextRectangle(	next,
									curTopNormal,
									topDir,
									topPrevPlaneNormal,
									topNextPlaneNormal,
									curSpinePos,
									nextSpinePos,
									eRoofTop,
									curTopProj0,
									curTopProj1,
									cummulatedVOffset,
									uDist0,
									uDist1,
									outsideTriangles,
									false) )
				{
					// We added 2 triangles
					outsideDomains.AddElem(fOutTopDomain);
					outsideDomains.AddElem(fOutTopDomain);
				}
			}

			// Fill in the elastic zone : link the last curBot and curTop
			if(!Flag(eRoofNoElasticZone))
			{
				TVector3 normal = curBottomDir^(curTopProj1-curBotProj0);
				normal.Normalize();
				TVector2 UV0=TVector2::kZero;
				TVector2 UV1=TVector2::kZero;
				TVector2 UV2=TVector2::kZero;
				TVector2 UV3=TVector2::kZero;
				GetRectangleUV( curBotProj0,curBotProj1,curTopProj0,curTopProj1,cummulatedVOffset,
						   UV0,UV1,UV2,UV3, eRoofOut );
				BuildRectangle(curTopProj0,curTopProj1,curBotProj0,curBotProj1,-normal,
					UV2,UV3,UV0,UV1,outsideTriangles); // Order them to have the facet normal pointing outside
				// We added 2 triangles
				outsideDomains.AddElem(fOutMidDomain);
				outsideDomains.AddElem(fOutMidDomain);
			}

			// Do the same for the inside
			if(GetTotalProfilePointCount()>0)
			{
				curBotProj0 = curZonePos;
				curBotProj1 = nextZonePos;
				cummulatedVOffset = 0;
				uDist0 = 0;
				uDist1 = curBotLength;
				for(int32 iBIn=0 ; iBIn<botCountIn ; iBIn++)
				{
					const TVector2& next = fProfile->GetBotInsidePos(iBIn);

					if( BuildNextRectangle(	next,
										curBottomNormal,
										curBottomDir,
										botPrevPlaneNormal,
										botNextPlaneNormal,
										curZonePos,
										nextZonePos,
										eRoofIn,
										curBotProj0,
										curBotProj1,
										cummulatedVOffset,
										uDist0,
										uDist1,
										fTriangleArray,
										false) )
					{
						// We added 2 triangles
						fTriangleDomain.AddElem(fInsideDomain);
						fTriangleDomain.AddElem(fInsideDomain);
					}
				}

				// Extrude the top of the profile
				// UV determination
				curTopProj0 = curSpinePos;
				curTopProj1 = nextSpinePos;
				cummulatedVOffset = 0;
				uDist0 = 0;
				uDist1 = topLength;
				for(int32 iTIn=0 ; iTIn<topCountIn ; iTIn++)
				{
					const TVector2 next = fProfile->GetTopInsidePos(iTIn);

					if( BuildNextRectangle(	next,
										curTopNormal,
										topDir,
										topPrevPlaneNormal,
										topNextPlaneNormal,
										curSpinePos,
										nextSpinePos,
										eRoofIn,
										curTopProj0,
										curTopProj1,
										cummulatedVOffset,
										uDist0,
										uDist1,
										fTriangleArray,
										true) )
					{
						// We added 2 triangles
						fTriangleDomain.AddElem(fInsideDomain);
						fTriangleDomain.AddElem(fInsideDomain);
					}
				}

				// Fill in the elastic zone : link the last curBot and curTop
				if(!Flag(eRoofNoElasticZone))
				{
					TVector3 normal = curBottomDir^(curTopProj1-curBotProj0);
					normal.Normalize();

					// UV determination
					TVector2 UV0=TVector2::kZero;
					TVector2 UV1=TVector2::kZero;
					TVector2 UV2=TVector2::kZero;
					TVector2 UV3=TVector2::kZero;
					GetRectangleUV( curBotProj0,curBotProj1,curTopProj0,curTopProj1,cummulatedVOffset,
							   UV0,UV1,UV2,UV3, eRoofIn );
					BuildRectangle(curBotProj0,curBotProj1,curTopProj0,curTopProj1,normal,
						UV0,UV1,UV2,UV3,fTriangleArray); // Order them to have the facet normal pointing outside
					// We added 2 triangles
					fTriangleDomain.AddElem(fInsideDomain);
					fTriangleDomain.AddElem(fInsideDomain);
				}
			}
		}
		
		// Go to the next point
		prevZonePos = curZonePos;
		curZonePos = nextZonePos;

		nextZonePos = nextNextZonePos;
		prevSpinePos = curSpinePos;
		curSpinePos = nextSpinePos;
		nextSpinePos = nextNextSpinePos;

		botPrevPlaneNormal = botNextPlaneNormal;
		topPrevPlaneNormal = topNextPlaneNormal;

		prevBottomDir = curBottomDir;
		curBottomDir = nextBottomDir;
		prevBotLength = curBotLength;
		curBotLength = nextBotLength;
		prevBottomNormal = curBottomNormal;
		curBottomNormal = nextBottomNormal;
		prevIsVertical = curIsVertical;
		curIsVertical = nextIsVertical;

		mUnfoldUVData.mOffsetU += curBotLength;
	}

	// 2: Fill in the possible hole in the zone spine: use the FlatPolygonTessellator
	if(!Flag(eRoofNoElasticZone))
	{
		std::vector<TVector3> polygon;
		for(int32 iSpine=0 ; iSpine<zonePointCount ; iSpine++)
		{
			const int32 spinePtIndex = fVertexArray.GetElemCount();
			const TVector3 pos = fZone.GetZoneSection(iSpine).fSpinePoint->Get3DPos();
			polygon.push_back( pos );
		}
		TMCClassArray<Triangle> result;
		if( TesselatePolygon( polygon, result ) )
		{
			int indexOffset = fVertexArray.GetElemCount();

			const size_t polyCount = polygon.size();
			for(size_t iVtx=0 ; iVtx<polyCount; iVtx++)
			{
				const TVector3& pos = polygon[iVtx];

				// UV determination: flat part on the top => use the same method as for Room
				Vertex vtx(pos,TVector3::kUnitZ,ComputeUV(pos.CastToXY())); // , eRoofFlat));
				fVertexArray.AddElem(vtx);
			}

			const int32 resultCount = result.GetElemCount();
			for(int32 iTgl=0 ; iTgl<resultCount ; iTgl++)
			{
				const Triangle& curTgl = result[iTgl];
				outsideTriangles.AddElem( Triangle( curTgl.pt1+indexOffset, curTgl.pt2+indexOffset, curTgl.pt3+indexOffset ) );
				outsideDomains.AddElem( fOutTopDomain );
			}
		}

		/*
		FlatPolygonMaker polygonMaker;

		for(int32 iSpine=0 ; iSpine<zonePointCount ; iSpine++)
		{
			const int32 spinePtIndex = fVertexArray.GetElemCount();
			const TVector3 pos = fZone.GetZoneSection(iSpine).fSpinePoint->Get3DPos();
			if( polygonMaker.AddPointToCountainer( pos.CastToXY(), spinePtIndex, true, (iSpine==(zonePointCount-1)) ) )
			{
				// UV determination: flat part on the top => use the same method as for Room
				Vertex vtx(pos,TVector3::kUnitZ,ComputeUV(pos.CastToXY())); // , eRoofFlat));
				fVertexArray.AddElem(vtx);
			}
		}
		polygonMaker.MakePolygons();
		if(polygonMaker.GetPolygonCount())
		{
			TMCClassArray<FlatPoint> polygon;
			polygonMaker.GetPolygon(0, polygon);
			FlatPolygonTessellator tessellator(polygon);

			TMCClassArray<Triangle> result;
			tessellator.Tessellate(result);

			const int32 resultCount = result.GetElemCount();
			TMCArray<int32> domain(resultCount,false);
			for(int32 iElem=0 ; iElem<resultCount ; iElem++)
				domain[iElem] = fOutTopDomain;

			// Add the facet
			outsideTriangles.Append( result );
			outsideDomains.Append( domain );
		}
		*/
	}

	// Append the outside triangles after the inside triangles
	fOutsideOffset = fTriangleArray.GetElemCount();
	fTriangleArray.Append(outsideTriangles);
	fTriangleDomain.Append(outsideDomains);
}

void Roof::SetRoofMin(const real32 min)
{
	if(min==fRoofMin)
		return;
	if(min==fData->GetDefaultRoofMin() )
	{
		if(fRoofMin==kDefaultLevelHeight)
			return;

		fRoofMin=kDefaultLevelHeight;
	}
	else
	{
		fRoofMin=min;
	}
	
	// Invalidation
	ClearFlag(eRoofTessellated);
}

void Roof::SetRoofMax(const real32 max)
{
	if(max==fRoofMax)
		return;
	if(max==fData->GetDefaultRoofMax() )
	{
		if(fRoofMax==kDefaultRoofHeight)
			return;

		fRoofMax=kDefaultRoofHeight;
	}
	else
	{
		fRoofMax=max;

//		if(fRoofMax<0)fRoofMax=0;
//		const real32 min = GetRoofMin();
//		if(fRoofMax<min)fRoofMax=min;
	}
	
	// Invalidation
	ClearFlag(eRoofTessellated);
}

void Roof::GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection)
{
	const real32 levelToground = fLevel->GetDistanceToGround();
	const real32 zMin = levelToground + GetRoofMin();
	const real32 zMax = levelToground + GetRoofMax();

	const int32 zoneSectionCount = fZone.GetSectionCount();

	if(exact)
	{	// Need to include the points of the profile
		// TO DO: exact bbox on selection (but not used for now)
		TMCClassArray<Vertex>& vertices = Vertices();
		const int32 vertexCount = vertices.GetElemCount();

		for( int32 iVertex=0 ; iVertex<vertexCount ; iVertex++ )
		{
			bbox.AddPoint( vertices[iVertex].Position() );
		}
	}
	else
	{
		// Check the profile points: if one of them is selected, we'll 
		// just text all the section for the bbox
		if(onSelection)
		{
			if(Hidden())
				return;

			boolean selectAll = false;
			const int32 botCount = fProfile->GetBotProfilePointCount();
			for(int32 iBot=0 ; iBot<botCount && !selectAll ; iBot++)
				if( fProfile->GetBotProfilePoint(iBot)->Selected()) selectAll=true;
			const int32 topCount = fProfile->GetTopProfilePointCount();
			for(int32 iTop=0 ; iTop<topCount && !selectAll ; iTop++)
				if( fProfile->GetTopProfilePoint(iTop)->Selected()) selectAll=true;
			const int32 bInCount = fProfile->GetBotInsidePointCount();
			for(int32 iBIn=0 ; iBIn<bInCount && !selectAll ; iBIn++)
				if( fProfile->GetBotInsidePoint(iBIn)->Selected()) selectAll=true;
			const int32 tInCount = fProfile->GetTopInsidePointCount();
			for(int32 iTIn=0 ; iTIn<tInCount && !selectAll ; iTIn++)
				if( fProfile->GetTopInsidePoint(iTIn)->Selected()) selectAll=true;

			for(int32 iSc=0 ; iSc<zoneSectionCount ; iSc++)
			{
				const ZoneSection& zoneSection = fZone.GetZoneSection(iSc);
				if(selectAll || zoneSection.fZonePoint->Selected())
				{
					const TVector3 pos = zoneSection.fZonePoint->Get3DPos();
					bbox.AddPoint(pos);
				}
				if(selectAll || zoneSection.fSpinePoint->Selected())
				{
					const TVector3 pos = zoneSection.fSpinePoint->Get3DPos();
					bbox.AddPoint(pos);
				}
			}
		}
		else
		{
			// Check the zone sections
			for(int32 iSc=0 ; iSc<zoneSectionCount ; iSc++)
			{
				const ZoneSection& zoneSection = fZone.GetZoneSection(iSc);
		
				const TVector3 posZ = zoneSection.fZonePoint->Get3DPos();
				bbox.AddPoint(posZ);

				const TVector3 posS = zoneSection.fSpinePoint->Get3DPos();
				bbox.AddPoint(posS);
			}
		}
	}

	// Later: check the selected objects if the roof isn't
}

void Roof::SetSelection(const boolean select)
{
	if( select && Selected() )
		return;

	if(select)
	{
		SetFlag(eIsSelected);

		const int32 zonePointCount = fZone.GetSectionCount();
		for(int32 iPt=0 ; iPt<zonePointCount ; iPt++)
		{
			fZone.GetZoneSection(iPt).fZonePoint->SetFlag(eIsSelected);
			fZone.GetZoneSection(iPt).fSpinePoint->SetFlag(eIsSelected);
		}
		const int32 botCount = fProfile->GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount ; iBot++)
			fProfile->GetBotProfilePoint(iBot)->SetFlag(eIsSelected);
		const int32 topCount = fProfile->GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount ; iTop++)
			fProfile->GetTopProfilePoint(iTop)->SetFlag(eIsSelected);
		const int32 bInCount = fProfile->GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			fProfile->GetBotInsidePoint(iBIn)->SetFlag(eIsSelected);
		const int32 tInCount = fProfile->GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			fProfile->GetTopInsidePoint(iTIn)->SetFlag(eIsSelected);
	}
	else
	{
		ClearFlag(eIsSelected);
		const int32 zonePointCount = fZone.GetSectionCount();
		for(int32 iPt=0 ; iPt<zonePointCount ; iPt++)
		{
			fZone.GetZoneSection(iPt).fZonePoint->ClearFlag(eIsSelected);
			fZone.GetZoneSection(iPt).fSpinePoint->ClearFlag(eIsSelected);
		}
		const int32 botCount = fProfile->GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount ; iBot++)
			fProfile->GetBotProfilePoint(iBot)->ClearFlag(eIsSelected);
		const int32 topCount = fProfile->GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount ; iTop++)
			fProfile->GetTopProfilePoint(iTop)->ClearFlag(eIsSelected);
		const int32 bInCount = fProfile->GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
			fProfile->GetBotInsidePoint(iBIn)->ClearFlag(eIsSelected);
		const int32 tInCount = fProfile->GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
			fProfile->GetTopInsidePoint(iTIn)->ClearFlag(eIsSelected);
	}

	fData->InvalidateStatus();
}

void Roof::SelectIfPossible()
{
	if(Hidden())
		return;

	const int32 zonePointCount = fZone.GetSectionCount();
	for(int32 iPt=0 ; iPt<zonePointCount ; iPt++)
	{
		if( !fZone.GetZoneSection(iPt).fZonePoint->Selected())
			return;
		if( !fZone.GetZoneSection(iPt).fSpinePoint->Selected())
			return;
	}

	SetFlag(eIsSelected);
}

void Roof::ShowRoof()
{
	if(!Hidden())
		return;

	ClearFlag(eIsHidden);
/*	// Clear objects flag
	const int32 objectCount = GetObjectCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		GetObject(iObj)->ClearFlag(eIsHidden);
	}*/
}

void Roof::HideRoof()
{
	if(Hidden())
		return;

	SetFlag(eIsHidden);
	ClearFlag(eIsSelected);

	// Deselect also all the points of the zone and the profiles
	const int32 zoneSectionCount = GetRoofZoneSectionCount();

	for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
	{
		const ZoneSection& zoneSection = GetRoofZoneSection(iPt);

		zoneSection.fZonePoint->SetSelection(false);
		zoneSection.fSpinePoint->SetSelection(false);
	}

	const int32 botCount = GetBotProfilePointCount();
	for(int32 iBot=0 ; iBot<botCount ; iBot++)
	{
		GetBotProfilePoint(iBot)->SetSelection(false);
	}

	const int32 topCount = GetTopProfilePointCount();
	for(int32 iTop=0 ; iTop<topCount ; iTop++)
	{
		GetTopProfilePoint(iTop)->SetSelection(false);
	}

	const int32 bInCount = GetBotInsidePointCount();
	for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
	{
		GetBotInsidePoint(iBIn)->SetSelection(false);
	}

	const int32 tInCount = GetTopInsidePointCount();
	for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
	{
		GetTopInsidePoint(iTIn)->SetSelection(false);
	}

/*	// Clear objects flag
	const int32 objectCount = GetObjectCount();
	for( int32 iObj=0 ; iObj<objectCount ; iObj++ )
	{
		GetObject(iObj)->SetFlag(eIsHidden);
		GetObject(iObj)->ClearFlag(eIsSelected);
	}*/
}

void Roof::OffsetSelection(const TVector2& offset)
{
	if(Hidden())
		return;

	const int32 zoneSectionCount = GetRoofZoneSectionCount();

	boolean horizontalOffset = false;

	for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
	{
		const ZoneSection& zoneSection = GetRoofZoneSection(iPt);

		if(zoneSection.fZonePoint->Selected())
		{
			horizontalOffset = true;
			zoneSection.fZonePoint->OffsetPosition(offset);
		}
		if(zoneSection.fSpinePoint->Selected())
		{
			horizontalOffset = true;
			zoneSection.fSpinePoint->OffsetPosition(offset);
		}
	}

	if(!horizontalOffset)
	{
		const int32 botCount = GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount ; iBot++)
		{
			ProfilePoint* point = GetBotProfilePoint(iBot);
			if( point->Selected())
				point->OffsetPosition(offset);
		}

		const int32 topCount = GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount ; iTop++)
		{
			ProfilePoint* point = GetTopProfilePoint(iTop);
			if( point->Selected())
				point->OffsetPosition(offset);
		}

		const int32 bInCount = GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
		{
			ProfilePoint* point = GetBotInsidePoint(iBIn);
			if( point->Selected())
				point->OffsetPosition(offset);
		}

		const int32 tInCount = GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
		{
			ProfilePoint* point = GetTopInsidePoint(iTIn);
			if( point->Selected())
				point->OffsetPosition(offset);
		}
	}
}

void Roof::ScaleSelection(const TVector2& scale, const TVector2& center, const EOptionMode mode)
{
	if(Hidden())
		return;

	const int32 zoneSectionCount = GetRoofZoneSectionCount();

	boolean horizontalScale = false;

	for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
	{
		const ZoneSection& zoneSection = GetRoofZoneSection(iPt);

		if(zoneSection.fZonePoint->Selected())
		{
			if(mode == eOption1Mode)
				zoneSection.fZonePoint->SetPosition(ScalePoint(zoneSection.fZonePoint->Position(),scale,center));
			else
				zoneSection.fZonePoint->SetPosition(ScalePoint(zoneSection.fZonePoint->Position(),scale,zoneSection.fSpinePoint->Position()));
			horizontalScale = true;
		}
		if(zoneSection.fSpinePoint->Selected())
		{
			if(mode == eOption1Mode)
				zoneSection.fSpinePoint->SetPosition(ScalePoint(zoneSection.fSpinePoint->Position(),scale,center));
			horizontalScale = true;
		}
	}

	if(!horizontalScale)
	{
		const int32 botCount = GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount ; iBot++)
		{
			ProfilePoint* point = GetBotProfilePoint(iBot);
			if( point->Selected())
				point->SetPosition(ScalePoint(point->Position(),scale,TVector2::kZero));
		}

		const int32 topCount = GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount ; iTop++)
		{
			ProfilePoint* point = GetTopProfilePoint(iTop);
			if( point->Selected())
				point->SetPosition(ScalePoint(point->Position(),scale,TVector2::kZero));
		}

		const int32 bInCount = GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
		{
			ProfilePoint* point = GetBotInsidePoint(iBIn);
			if( point->Selected())
				point->SetPosition(ScalePoint(point->Position(),scale,TVector2::kZero));
		}

		const int32 tInCount = GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
		{
			ProfilePoint* point = GetTopInsidePoint(iTIn);
			if( point->Selected())
				point->SetPosition(ScalePoint(point->Position(),scale,TVector2::kZero));
		}
	}
}

void Roof::RotateSelection(const TVector2& cosSin, const TVector2& center)
{
	if(Hidden())
		return;

	const int32 zoneSectionCount = GetRoofZoneSectionCount();

	boolean horizontalRot = false;

	for(int32 iPt=0 ; iPt<zoneSectionCount ; iPt++)
	{
		const ZoneSection& zoneSection = GetRoofZoneSection(iPt);

		if(zoneSection.fZonePoint->Selected())
		{
			horizontalRot = true;
			zoneSection.fZonePoint->SetPosition(RotatePoint(zoneSection.fZonePoint->Position(),cosSin,center));
		}
		if(zoneSection.fSpinePoint->Selected())
		{
			horizontalRot = true;
			zoneSection.fSpinePoint->SetPosition(RotatePoint(zoneSection.fSpinePoint->Position(),cosSin,center));
		}
	}

	if(!horizontalRot)
	{
		const int32 botCount = GetBotProfilePointCount();
		for(int32 iBot=0 ; iBot<botCount ; iBot++)
		{
			ProfilePoint* point = GetBotProfilePoint(iBot);
			if( point->Selected())
				point->SetPosition(RotatePoint(point->Position(),cosSin,center));
		}

		const int32 topCount = GetTopProfilePointCount();
		for(int32 iTop=0 ; iTop<topCount ; iTop++)
		{
			ProfilePoint* point = GetTopProfilePoint(iTop);
			if( point->Selected())
				point->SetPosition(RotatePoint(point->Position(),cosSin,center));
		}

		const int32 bInCount = GetBotInsidePointCount();
		for(int32 iBIn=0 ; iBIn<bInCount ; iBIn++)
		{
			ProfilePoint* point = GetBotInsidePoint(iBIn);
			if( point->Selected())
				point->SetPosition(RotatePoint(point->Position(),cosSin,center));
		}

		const int32 tInCount = GetTopInsidePointCount();
		for(int32 iTIn=0 ; iTIn<tInCount ; iTIn++)
		{
			ProfilePoint* point = GetTopInsidePoint(iTIn);
			if( point->Selected())
				point->SetPosition(RotatePoint(point->Position(),cosSin,center));
		}
	}
}

real32 Roof::GetMinToGround() const 
{
	return fLevel->GetDistanceToGround()+GetRoofMin();
}

real32 Roof::GetMaxToGround() const 
{
	return fLevel->GetDistanceToGround()+GetRoofMax();
}

void Roof::SetTopProfile(const ERoofProfileID profileID)
{
	// Erase the current top profile
	TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetTopProfile();
	profile.ArrayFree();

	real32 sceneMag = fData->GetGeneralSize();

	switch(profileID)
	{
	case eShape0: // Basic
		{	ClearFlag(eRoofNoElasticZone);
		} break;
	case eShape1:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(1);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.5f, -.2f),this,true,true);
		} break;
	case eShape2:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(2);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.5f, -.1f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.5f, -.2f),this,true,true);
		} break;
	case eShape3:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.6f, -.1f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.7f, -.2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.6f, -.3f),this,true,true);
		} break;
	case eShape4:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.7f, -.2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.6f, -.3f),this,true,true);
		} break;
	case eShape5:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(4);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .3f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.4f, .1f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.7f, -.2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.6f, -.3f),this,true,true);
		} break;
	case eShape6:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(4);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .3f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.05f, .15f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.6f, -.15f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.6f, -.3f),this,true,true);
		} break;
	case eShape7:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(5);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .3f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.2f, .27f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.32f, .2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.4f, .1f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(-.44f, 0),this,true,true);
		} break;
	case eShape8:
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(5);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .3f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.04f, .2f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.11f, .1f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.19f, .04f),this,true,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(-.3f, 0),this,true,true);
		} break;
	case eShape9: // None
		{	SetFlag(eRoofNoElasticZone);
		} break;
	}
}

void Roof::SetBotProfile(const ERoofProfileID profileID)
{
	// Erase the current bottom profile
	TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetBotProfile();
	profile.ArrayFree();

	real32 sceneMag = fData->GetGeneralSize();

	switch(profileID)
	{
	//	Bottom profile
	case eShape0: // Basic
		{
		} break;
	case eShape1:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.3f, 0),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.3f, .2f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(0, .2f),this,false,true);
		} break;
	case eShape2:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.25f, 0),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.3f, .05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(0, .2f),this,false,true);
		} break;
	case eShape3:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.4f, -.5f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.4f, -.4f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.05f, 0),this,false,true);
		} break;
	case eShape4:
		{	profile.SetElemCount(4);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.37f, -.46f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.42f, -.45f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.4f, -.4f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.05f, 0),this,false,true);
		} break;
	case eShape5:
		{	profile.SetElemCount(5);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.2f, 0),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.3f, .2f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.2f, .1f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.1f, .1f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(0, .2f),this,false,true);
		} break;
	case eShape6:
		{	profile.SetElemCount(5);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.25f, -.05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.15f, .05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.2f, .2f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.1f, .1f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(0, .2f),this,false,true);
		} break;
	case eShape7:
		{	profile.SetElemCount(5);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.05f, -.1f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.15f, -.05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.25f, .2f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.15f, .05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(0, .2f),this,false,true);
		} break;
	case eShape8:
		{	profile.SetElemCount(6);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.52f, -.6f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.5f, -.5f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.35f, -.2f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(3), fData, sceneMag * TVector2(-.4f, -.05f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(4), fData, sceneMag * TVector2(-.3f, -.1f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(5), fData, sceneMag * TVector2(-.1f, .1f),this,false,true);
		} break;
	case eShape9: // Wall
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, .5f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(.2f, .5f),this,false,true);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(.2f, 0),this,false,true);
		} break;
	}
}

void Roof::SetTopInside(const ERoofProfileID profileID)
{
	// Erase the current top profile
	TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetTopInside();
	profile.ArrayFree();

	real32 sceneMag = fData->GetGeneralSize();

	switch(profileID)
	{
	case eShape0: // Basic
		{	ClearFlag(eRoofNoElasticZone);
		} break;
	case eShape1: // None
		{	ClearFlag(eRoofNoElasticZone);
			profile.SetElemCount(1);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, -.1f),this,true,false);
		} break;
	case eShape2:
		{	profile.SetElemCount(1);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.3f, -.5f),this,true,false);
		} break;
	case eShape3:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(0, -.1f),this,true,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.1f, -.15f),this,true,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.15f, -.2f),this,true,false);
		} break;
	case eShape4:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(-.05f, -.1f),this,true,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(-.1f, -.15f),this,true,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(-.2f, -.2f),this,true,false);
		} break;
	case eShape5:
	case eShape6:
	case eShape7:
	case eShape8:
	case eShape9:
		{	ClearFlag(eRoofNoElasticZone);
		} break;
	}
}

void Roof::SetBotInside(const ERoofProfileID profileID)
{
	// Erase the current bottom profile
	TMCCountedPtrArray<ProfilePoint>& profile = fProfile->GetBotInside();
	profile.ArrayFree();

	real32 sceneMag = fData->GetGeneralSize();

	switch(profileID)
	{
	//	Bottom profile
	case eShape0: // Basic
		{
		} break;
	case eShape1: // Wall
		{	profile.SetElemCount(1);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(.1f, 0),this,false,false);
		} break;
	case eShape2:
		{	profile.SetElemCount(1);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(.5f, .3f),this,false,false);
		} break;
	case eShape3:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(.1f, 0),this,false,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(.18f, .01f),this,false,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(.25f, .05f),this,false,false);
		} break;
	case eShape4:
		{	profile.SetElemCount(3);
			ProfilePoint::CreateProfilePoint(profile.Pointer(0), fData, sceneMag * TVector2(.1f, .07f),this,false,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(1), fData, sceneMag * TVector2(.18f, .1f),this,false,false);
			ProfilePoint::CreateProfilePoint(profile.Pointer(2), fData, sceneMag * TVector2(.25f, .1f),this,false,false);
		} break;
	case eShape5:
	case eShape6:
	case eShape7:
	case eShape8:
	case eShape9:
		{
		} break;
	}
}

void Roof::SetLevel(Level* level) {fLevel = level;}
void Roof::SetData(BuildingPrimData* data) {fData = data;}

void Roof::GetInsideTriangleVertices(TMCArray<TriangleVertices>& triangles)
{
	// Use the triangles before fOutsideOffset.
	// If there's no inside part: use the outside triangles instead
	TMCClassArray<Triangle>& roofTriangles = Triangles();
	const int32 maxCount = fOutsideOffset>0?fOutsideOffset:roofTriangles.GetElemCount();

	triangles.SetElemCount(maxCount);

	for( int32 iTgle=0 ; iTgle<maxCount ; iTgle++ )
	{
		const Triangle& tgle = fTriangleArray[iTgle];
		TriangleVertices& triangleVtx = triangles[iTgle];
		triangleVtx.fVertices[0] = fVertexArray[tgle[0]].Position();
		triangleVtx.fVertices[1] = fVertexArray[tgle[1]].Position();
		triangleVtx.fVertices[2] = fVertexArray[tgle[2]].Position();
	}
}

void Roof::Clone(Roof** newRoof, Level* inLevel)
{
	Roof::CreateRoof( newRoof, inLevel->GetPrimitiveData(), inLevel );
	
	TMCCountedPtr<Roof> roofPtr;
	roofPtr = *newRoof;

	// Copy the data
	roofPtr->fZone.Copy( fZone, roofPtr );
	roofPtr->fProfile->Copy( fProfile, roofPtr );
	roofPtr->fRoofMin = fRoofMin;
	roofPtr->fRoofMax = fRoofMax;
	roofPtr->SetFlags(fFlags);
	roofPtr->SetNamePtr(fName);

	roofPtr->fOutTopDomain = fOutTopDomain;
	roofPtr->fOutMidDomain = fOutMidDomain;
	roofPtr->fOutBotDomain = fOutBotDomain;
	roofPtr->fInsideDomain = fInsideDomain;

	// Invalidate the tessellation
	roofPtr->ClearFlag(eRoofTessellated);

	// Clear flag
	roofPtr->ClearFlag(eIsTargeted);
}

//////////////////////////////////////////////////////////////////////////
//
//

MCCOMErr Roof::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Roof');
	if (result) return result;
	{
		// Write the base altitude
		result=stream->PutKeyword('Base');
		if (result) return result;
		result=stream->PutQuickFix(fRoofMin);
		if (result) return result;

		// Write the height
		result=stream->PutKeyword('Heig');
		if (result) return result;
		result=stream->PutQuickFix(fRoofMax);
		if (result) return result;

		// Domain
		stream->PutInt32Attribute('OTDo', fOutTopDomain);
		stream->PutInt32Attribute('OMDo', fOutMidDomain);
		stream->PutInt32Attribute('OBDo', fOutBotDomain);
		stream->PutInt32Attribute('InDo', fInsideDomain);
	
		// Common
		result=CommonBase::Write(stream);
		if (result) return result;

		// Write the Zone
		result=stream->PutKeywordAndBegin('Zone');
		{
			if (result) return result;

			const int32 zonePointCount = fZone.GetSectionCount();
			stream->PutInt32Attribute('Coun', zonePointCount);
			if (result) return result;

			for(int32 iPt=0 ; iPt<zonePointCount ; iPt++)
			{
				result=stream->PutKeywordAndBegin('Sect');

				const ZoneSection& section = fZone.GetZoneSection(iPt);
				// Zone Point
				section.fZonePoint->Write(stream);
				// Spine Point
				section.fSpinePoint->Write(stream);
	
				result=stream->PutEnd();
			}
		}
		result=stream->PutEnd();
		if (result) return result;

		// Write the Profile
		result=stream->PutKeywordAndBegin('Prof');
		if (result) return result;
		{
			// Bottom Outside
			result=stream->PutKeywordAndBegin('BoOu');
			if (result) return result;
			{
				const int32 botCount = fProfile->GetBotProfilePointCount();
				stream->PutInt32Attribute('Coun', botCount);
				for(int32 iPt=0 ; iPt<botCount ; iPt++)
				{
					fProfile->GetBotProfilePoint(iPt)->Write(stream);
				}
			}
			result=stream->PutEnd();
			if (result) return result;

			// Top Outside
			result=stream->PutKeywordAndBegin('ToOu');
			if (result) return result;
			{
				const int32 topCount = fProfile->GetTopProfilePointCount();
				stream->PutInt32Attribute('Coun', topCount);
				for(int32 iPt=0 ; iPt<topCount ; iPt++)
				{
					fProfile->GetTopProfilePoint(iPt)->Write(stream);
				}
			}
			result=stream->PutEnd();
			if (result) return result;

			// Bottom Inside
			result=stream->PutKeywordAndBegin('BoIn');
			if (result) return result;
			{
				const int32 botCount = fProfile->GetBotInsidePointCount();
				stream->PutInt32Attribute('Coun', botCount);
				for(int32 iPt=0 ; iPt<botCount ; iPt++)
				{
					fProfile->GetBotInsidePoint(iPt)->Write(stream);
				}
			}
			result=stream->PutEnd();
			if (result) return result;

			// Top Inside
			result=stream->PutKeywordAndBegin('ToIn');
			if (result) return result;
			{
				const int32 topCount = fProfile->GetTopInsidePointCount();
				stream->PutInt32Attribute('Coun', topCount);
				for(int32 iPt=0 ; iPt<topCount ; iPt++)
				{
					fProfile->GetTopInsidePoint(iPt)->Write(stream);
				}
			}
			result=stream->PutEnd();
			if (result) return result;

		}
		result=stream->PutEnd();
		if (result) return result;

	}
	result=stream->PutEnd();
	return result;
}

MCCOMErr Roof::Read(IShTokenStream* stream)
{ 
	MCCOMErr result=MC_S_OK;

	int8 token[256];

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
		case 'Heig':  
			{
				result = stream->GetQuickFix(&fRoofMax);
				if (result) return result;
				break;
			}
		case 'Base':  
			{
				result = stream->GetQuickFix(&fRoofMin);
				if (result) return result;
				break;
			}
		case 'OTDo':  
			{
				fOutTopDomain = stream->GetInt32Token();
				break;
			}
		case 'OMDo':  
			{
				fOutMidDomain = stream->GetInt32Token();
				break;
			}
		case 'OBDo':  
			{
				fOutBotDomain = stream->GetInt32Token();
				break;
			}
		case 'InDo':  
			{
				fInsideDomain = stream->GetInt32Token();
				break;
			}
		case 'Zone':  
			{
				result = fZone.Read(stream, this);
				if (result) return result;
				break;
			}
		case 'Prof':  
			{
				result = fProfile->Read(stream, this);
				if (result) return result;
				break;
			}
			 
		default:
			CommonBase::Read(stream,keyword,fData);
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	// Invalidate tessellation
	ClearFlag(eRoofTessellated);
	ClearFlag(eIsTargeted);

	return result;
}

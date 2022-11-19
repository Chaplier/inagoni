/****************************************************************************************************

		PBooleanPolygon.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/06/2007

****************************************************************************************************/

#include "PBooleanPolygon.h"
#include "Tessellation.h"
#include "ArchiTools.h"

BooleanPolygon::BooleanPolygon()
{
	 // set some global vals to arm the boolean engine
    double DGRID = 1000;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.001;   // snap with in this range points to lines in the intersection routines
    // should always be > DGRID  a  MARGE >= 10*DGRID is oke
    // this is also used to remove small segments and to decide when
    // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
    double CORRECTIONABER   = 1.0;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 10.0;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 1000.0; // leave as is, segments of this length in smoothen


    // DGRID is only meant to make fractional parts of input data which
    // are doubles, part of the integers used in vertexes within the boolean algorithm.
    // Within the algorithm all input data is multiplied with DGRID

    // space for extra intersection inside the boolean algorithms
    // only change this if there are problems
    int GRID = 10000;

    mBoolEng.SetMarge( MARGE );
    mBoolEng.SetGrid( GRID );
    mBoolEng.SetDGrid( DGRID );
    mBoolEng.SetCorrectionFactor( CORRECTIONFACTOR );
    mBoolEng.SetCorrectionAber( CORRECTIONABER );
    mBoolEng.SetSmoothAber(SMOOTHABER );
    mBoolEng.SetMaxlinemerge(MAXLINEMERGE );
    mBoolEng.SetRoundfactor( ROUNDFACTOR );
}

BooleanPolygon::~BooleanPolygon()
{
	CleanUpContours();
	CleanUpHoles();
}

void BooleanPolygon::AddPolygon(const FlatPolygon& polygon)
{
	if(polygon.mIsHole)
	{
		mHoles.push_back( polygon );
	}
	else
	{
		mContours.push_back( polygon );
	}
}

void BooleanPolygon::PreparePolygons( const std::vector<FlatPolygon>& polygons, GroupType group )
{
	try
	{
		const size_t polygonCount = polygons.size();
		for( size_t iPoly=0 ; iPoly<polygonCount ; iPoly++ )
		{
			mBoolEng.StartPolygonAdd(group);

			const FlatPolygon& polygon = polygons[iPoly];
			const int32 vtxCount = polygon.GetElemCount();
			for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
			{
				mBoolEng.AddPoint(polygon[iVtx].x,polygon[iVtx].y, 0);
			}

			mBoolEng.EndPolygonAdd();
		}
	}
	catch( Bool_Engine_Error & )
    {
		HandleException(NULL, TMCDynamicString("BooleanPolygon::PreparePolygons Bool_Engine_Error"));
    }
	catch( ... )
    {
		HandleException(NULL, TMCDynamicString("BooleanPolygon::PreparePolygons"));
    }
}

/*void StripToFlatMesh( const gpc_tristrip& triStrip, FlatMesh& result )
{
	// Build the triangle array
	const int32 stripCount = triStrip.num_strips;

	int32 vtxIndex = 0;

	gpc_vertex_list* stripPtr = triStrip.strip;
	for(int32 iStrip=0 ; iStrip<stripCount ; iStrip++)
	{
		const int32 vtxCount = stripPtr->num_vertices;
	
		gpc_vertex* vtxPtr = stripPtr->vertex;
		for(int32 iVtx=0 ; iVtx<vtxCount ; iVtx++)
		{
			result.mVertices.AddElem( TVector2( vtxPtr->x, vtxPtr->y ) );

			if(iVtx>=2)
			{
				if(IsOdd(iVtx))
					result.mTriangles.AddElem(Triangle(vtxIndex-2, vtxIndex-1, vtxIndex));
				else
					result.mTriangles.AddElem(Triangle(vtxIndex, vtxIndex-1, vtxIndex-2));
			}

			// Next vtx
			vtxPtr++;
			vtxIndex++;
		}


		// Next step
		stripPtr++;
	}
}*/

void BooleanPolygon::GetPolygons(TMCClassArray<FlatPolygon>& resultArray, bool wantsTessellation, FlatMesh& tessellationResult)
{
	try
	{
		// Prepare
		PreparePolygons(mContours, GROUP_A);
		PreparePolygons(mHoles, GROUP_B);

		// Do
		mBoolEng.Do_Operation(BOOL_A_SUB_B);

		// Get the result
		while(mBoolEng.StartPolygonGet())	//foreach resultant polygon
		{
			FlatPolygon& newPoly = resultArray.AddElem();
			newPoly.SetElemSpace( mBoolEng.GetNumPointsInPolygon() );

			//foreach point in the polygon
			while(mBoolEng.PolygonHasMorePoints())
			{
				newPoly.AddElem(TVector2(mBoolEng.GetPolygonXPoint(),mBoolEng.GetPolygonYPoint()));
			}
			mBoolEng.EndPolygonGet();
		}

		if( wantsTessellation )
		{
			const int polyCount = resultArray.GetElemCount();
			for( int iPoly=0 ; iPoly<polyCount ; iPoly++ )
			{
				TesselatePolygon( resultArray[iPoly], tessellationResult );
			}
		}
	}
	catch( Bool_Engine_Error & )
    {
		HandleException(NULL, TMCDynamicString("GetPolygons::PreparePolygons Bool_Engine_Error"));
    }
	catch( ... )
    {
		HandleException(NULL, TMCDynamicString("GetPolygons::PreparePolygons"));
    }
}

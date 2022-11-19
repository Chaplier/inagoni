/****************************************************************************************************

		Engine1.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "Engine1.h"

#include "math.h"
#include "Utils.h"
#include "MCRandom.h"
#include "Veloute.h"
#include "GridShader1.h"
#include "TilingShader1.h"

static const real32 INV_SQRT10 =	0.31622776601683793320f; /* 1.0/sqrt(10)*/
static const real32 kSinAlpha = INV_SQRT10;
static const real32 kCosAlpha = 3*INV_SQRT10;

Engine1::Engine1()	// We just initialize the values
{
	fTileID=0;

	fLength=0;
	fSize=0; // size of the small square tile, from 0 to 0.5
	fHalfMinusSize=0;
	fHalfMinusHalfSize=0;

	// Value needed for the 3 first arrangments
	fRescaledMortar=0;

	// values needed for the hexagonal tiling
	fHalfLength=0; // is equal to the triangle height
	fTriangleSide=0;
	fHalfTriangleSide=0;
	fScaledTriangleHeight=0;
	fRescaledTileStart=0; // rescaled on a [0,2] space
	fRescaledTileEnd=0; // rescaled on a [0,2] space
	fRescaledEndPlateau=0;// rescaled on a [0,2] space
	fRescaledStartPlateau=0;// rescaled on a [0,2] space

	fMortarBumpDepth=0;
	fMortarBottomSlope=0;
	fMortarTopSlope=0;
	fMortarPlateau=0;

	fType=0;
	fSmooth=0;
	fRandomUVOrigin=0;
	fProportionnalTileShading=0;
	fNeedCenter=0;

	fTileCenterUV = TVector2::kZero;
}
	
int32 Engine1::GetEquilateralTriangle(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, 
					   TVector2& hexagonCenter )
{
	real32 hTgleV = fromUV.y/fHalfLength;
	iV = floor( hTgleV );

	real32 up = (fromUV.y-SQRT3*fromUV.x)/(2*fHalfLength);
	int32 iUp = floor( up );

	real32 down = (fromUV.y+SQRT3*fromUV.x)/(2*fHalfLength);
	int32 iDown = floor( down );

	iU = iUp; // we need something for the noise, use iUp or iDown (note: the sum won't work: regularity visible)

	int32 iUpMod = 0;
	if(iUp<0)
		iUpMod = ( (iUp-3*iUp)%3 ); // % doesn't work with negative values
	else
		iUpMod = ( (iUp)%3 );
	int32 iDownMod=0;
	if(iDown<0)
		iDownMod = ( (iDown-3*iDown)%3 ); // % doesn't work with negative values
	else
		iDownMod = ( (iDown)%3 );
	int32 iTglVMod = 0;	
	if(iV<0)
		iTglVMod = ( (iV-3*iV)%3 ); // % doesn't work with negative values
	else
		iTglVMod = ( (iV)%3 );

	// Triangles ids map:
	//		*******
	//	   * * 2 * *
	//	  * 1 * * 3 *
	//	 *************
	//	  * 6 * * 4 *
	//     * * 5 * *
	//	    *******

	// When merge 3 by 3(tileThreeID is I, II or III)
	//	             *******
	//	            *   2   *
	//	           * 1     3 *
	//		*******     II    *
	//	   *   2     6     4 *
	//	  * 1     3     5   *
	//	 *     I           *
	//	  * 6     4     2   *
	//     *   5     1     3 *
	//	    *******    III    *
	//	           * 6     4 *
	//              *   5   *
	//	             *******

	int32 tileThreeID = 0;
	if(iDownMod == 0)
	{
		if(iUpMod == 0)
		{
			hexagonCenter.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==0)		{ hexagonCenter.y = iV*fHalfLength; fTileID=2; tileThreeID=3;}
			else if(iTglVMod==1){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=5; tileThreeID=2;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 1)
		{
			hexagonCenter.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==2)		{ hexagonCenter.y = iV*fHalfLength; fTileID=1; tileThreeID=2;}
			else if(iTglVMod==1){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=6; tileThreeID=2;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 2)
		{
			hexagonCenter.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==0)		{ hexagonCenter.y = iV*fHalfLength; fTileID=3; tileThreeID=3;}
			else if(iTglVMod==2){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=4; tileThreeID=3;}
			else MCAssert(NULL); // Shouldn't occured
		}
	}
	else if(iDownMod == 1)
	{
		if(iUpMod == 1)
		{
			hexagonCenter.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==2)		{ hexagonCenter.y = iV*fHalfLength; fTileID=2; tileThreeID=2;}
			else if(iTglVMod==0){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=5; tileThreeID=1;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 2)
		{
			hexagonCenter.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==1)		{ hexagonCenter.y = iV*fHalfLength; fTileID=1; tileThreeID=1;}
			else if(iTglVMod==0){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=6; tileThreeID=1;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 0)
		{
			hexagonCenter.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==2)		{ hexagonCenter.y = iV*fHalfLength; fTileID=3; tileThreeID=2;}
			else if(iTglVMod==1){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=4; tileThreeID=2;}
			else MCAssert(NULL); // Shouldn't occured
		}
	}
	else // iDownMod == 2
	{
		if(iUpMod == 2)
		{
			hexagonCenter.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==1)		{ hexagonCenter.y = iV*fHalfLength; fTileID=2; tileThreeID=1;}
			else if(iTglVMod==2){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=5; tileThreeID=3;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 0)
		{
			hexagonCenter.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==0)		{ hexagonCenter.y = iV*fHalfLength; fTileID=1; tileThreeID=3;}
			else if(iTglVMod==2){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=6; tileThreeID=3;}
			else MCAssert(NULL); // Shouldn't occured
		}
		else if(iUpMod == 1)
		{
			hexagonCenter.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==1)		{ hexagonCenter.y = iV*fHalfLength; fTileID=3; tileThreeID=1;}
			else if(iTglVMod==0){ hexagonCenter.y = (iV+1)*fHalfLength; fTileID=4; tileThreeID=1;}
			else MCAssert(NULL); // Shouldn't occured
		}
	}

	// Center UV on the hexagon
	newU = fromUV.x - hexagonCenter.x;
	newV = fromUV.y - hexagonCenter.y;

	// Rescale on a [0,1] space
	// In the scaled space, the triangle side length is 1 
	newU /= fTriangleSide;
	newV /= fTriangleSide;

	return tileThreeID;
}

void Engine1::ComputeTileCenter(const int32 tileU, const int32 tileV, const int32 iU, const int32 iV, const real32 decalU, const real32 decalV)
{
	if( fNeedCenter )
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		fTileCenterUV.x = ((real32)iU/(real32)tileU+decalU)*fLength;
		fTileCenterUV.y = ((real32)iV/(real32)tileV+decalV)*fLength;

		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}

void Engine1::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	switch(fType)
	{
	case 'Opt1': GetLocalUVCoordOption1(fromUV, newU, newV, iU, iV); break;
	case 'Opt2': GetLocalUVCoordOption2(fromUV, newU, newV, iU, iV); break;
	case 'Opt3': GetLocalUVCoordOption3(fromUV, newU, newV, iU, iV); break;
	case 'Opt4': GetLocalUVCoordOption4(fromUV, newU, newV, iU, iV); break;

	case 'Opt5': GetLocalUVCoordOption5(fromUV, newU, newV, iU, iV); break;
	case 'Opt6': GetLocalUVCoordOption6(fromUV, newU, newV, iU, iV); break;
	case 'Opt7': GetLocalUVCoordOption7(fromUV, newU, newV, iU, iV); break;
	case 'Opt8': GetLocalUVCoordOption8(fromUV, newU, newV, iU, iV); break;
	case 'Opt9': GetLocalUVCoordOption9(fromUV, newU, newV, iU, iV); break;
	case 'Op10': GetLocalUVCoordOption10(fromUV, newU, newV, iU, iV); break;
	case 'Op11': GetLocalUVCoordOption11(fromUV, newU, newV, iU, iV); break;
	case 'Op12': GetLocalUVCoordOption12(fromUV, newU, newV, iU, iV); break;
	case 'Op13': GetLocalUVCoordOption13(fromUV, newU, newV, iU, iV); break;
	case 'Op14': GetLocalUVCoordOption14(fromUV, newU, newV, iU, iV); break;
	case 'Op15': GetLocalUVCoordOption15(fromUV, newU, newV, iU, iV); break;
	case 'Op16': GetLocalUVCoordOption16(fromUV, newU, newV, iU, iV); break;
	case 'Op17': GetLocalUVCoordOption17(fromUV, newU, newV, iU, iV); break;
	case 'Op18': GetLocalUVCoordOption18(fromUV, newU, newV, iU, iV); break;
	case 'Op19': GetLocalUVCoordOption19(fromUV, newU, newV, iU, iV); break;
	case 'Op20': GetLocalUVCoordOption20(fromUV, newU, newV, iU, iV); break;

	}
}

void Engine1::GetLocalUVCoordOption1(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 5 possible centers
	if(newV>1-fSize-newU)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 1;
	}
	else if(newV<fSize-1+newU)
	{
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 2;
	}
	else if(newV<fSize-1-newU)
	{
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 3;
	}
	else if(newV>1-fSize+newU)
	{
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 4;
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption2(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 5 possible centers
	if(newV>0.5-fSize && newV>RealAbs(newU))
	{	// We're in the 1st diamant
		iU = 2*iU;
		iV = 2*iV+1;
		fTileID = 1;
	}
	else if(newU>fHalfMinusSize && newU>RealAbs(newV))
	{
		iU = 2*iU+1;
		iV = 2*iV;
		fTileID = 2;
	}
	else if(newV<-fHalfMinusSize && newV<-RealAbs(newU))
	{
		iU = 2*iU;
		iV = 2*iV-1;
		fTileID = 3;
	}
	else if(newU<-fHalfMinusSize && newU<-RealAbs(newV))
	{
		iU = 2*iU-1;
		iV = 2*iV;
		fTileID = 4;
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption3(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 5 possible centers
	if(newV>fHalfMinusSize && newU>-fHalfMinusSize)
	{	// upper
		iU = 2*iU;
		iV = 2*iV+1;
		decalU = .5+fSize;
		decalV = fHalfMinusSize;
		fTileID = 1;
	}
	else if(newU>fHalfMinusSize && newV<fHalfMinusSize)
	{	// right
		iU = 2*iU+1;
		iV = 2*iV;
		decalU = fHalfMinusSize;
		decalV = fHalfMinusSize;
		fTileID = 2;
	}
	else if(newV<-fHalfMinusSize && newU<fHalfMinusSize)
	{	// lower
		iU = 2*iU;
		iV = 2*iV-1;
		decalU = fHalfMinusSize;
		decalV = .5+fSize;
		fTileID = 3;
	}
	else if(newU<-fHalfMinusSize && newV>-fHalfMinusSize)
	{	// left
		iU = 2*iU-1;
		iV = 2*iV;
		decalU = .5+fSize;
		decalV = .5+fSize;
		fTileID = 4;
	}
	else
	{	// center
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption4(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{	// Hexagonal tiles case
	// first find in which equilateral triangle we're in
	real32 hTgleV = fromUV.y/fHalfLength;
	iV = floor( hTgleV );

	real32 up = (fromUV.y-SQRT3*fromUV.x)/(2*fHalfLength);
	int32 iUp = floor( up );
	iU = iUp; // need to choose one

	real32 down = (fromUV.y+SQRT3*fromUV.x)/(2*fHalfLength);
	int32 iDown = floor( down );

	int32 iUpMod = 0;
	if(iUp<0)
		iUpMod = ( (iUp-3*iUp)%3 ); // % doesn't work with negative values
	else
		iUpMod = ( (iUp)%3 );
	int32 iDownMod=0;
	if(iDown<0)
		iDownMod = ( (iDown-3*iDown)%3 ); // % doesn't work with negative values
	else
		iDownMod = ( (iDown)%3 );
	int32 iTglVMod = 0;	
	if(iV<0)
		iTglVMod = ( (iV-3*iV)%3 ); // % doesn't work with negative values
	else
		iTglVMod = ( (iV)%3 );

	if(iDownMod == 0)
	{
		if(iUpMod == 0)
		{
			fTileCenterUV.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==0) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==1) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 1)
		{
			fTileCenterUV.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==2) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==1) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 2)
		{
			fTileCenterUV.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==0) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==2) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
	}
	else if(iDownMod == 1)
	{
		if(iUpMod == 1)
		{
			fTileCenterUV.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==2) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==0) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 2)
		{
			fTileCenterUV.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==1) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==0) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 0)
		{
			fTileCenterUV.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==2) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==1) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
	}
	else // iDownMod == 2
	{
		if(iUpMod == 2)
		{
			fTileCenterUV.x = (iDown-iUp)*fHalfTriangleSide;
			if(iTglVMod==1) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==2) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 0)
		{
			fTileCenterUV.x = (iDown-iUp+1)*fHalfTriangleSide;
			if(iTglVMod==0) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==2) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
		else if(iUpMod == 1)
		{
			fTileCenterUV.x = (iDown-iUp-1)*fHalfTriangleSide;
			if(iTglVMod==1) fTileCenterUV.y = iV*fHalfLength;
			else if(iTglVMod==0) fTileCenterUV.y = (iV+1)*fHalfLength;
		}
	}

	// Center UV on the hexagon
	newU = fromUV.x - fTileCenterUV.x;
	newV = fromUV.y - fTileCenterUV.y;

	// Rescale on a [0,1] space
	newU /= fTriangleSide;
	newV /= fTriangleSide;

	if( fNeedCenter)
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}


void Engine1::GetLocalUVCoordOption5(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 9 possible centers
	if(newV>fHalfMinusSize && newU>-fHalfMinusSize)
	{	// upper
		// There're 2 tiles here
		iU = 2*iU;
		iV = 2*iV+1;
		decalU = .5+fSize;
		fTileID = 1;
		if(newV>fHalfMinusHalfSize)
		{ // further away
			decalV = fHalfMinusHalfSize;
		}
		else
		{
			decalV = fHalfMinusSize;
		}
	}
	else if(newU>fHalfMinusSize && newV<fHalfMinusSize)
	{	// right
		// There're 2 tiles here
		iU = 2*iU+1;
		iV = 2*iV;
		decalV = fHalfMinusSize;
		fTileID = 2;
		if(newU>fHalfMinusHalfSize)
		{ // further away
			decalU = fHalfMinusHalfSize;
		}
		else
		{
			decalU = fHalfMinusSize;
		}
	}
	else if(newV<-fHalfMinusSize && newU<fHalfMinusSize)
	{	// lower
		// There're 2 tiles here
		iU = 2*iU;
		iV = 2*iV-1;
		decalU = fHalfMinusSize;
		fTileID = 3;
		if(newV<-fHalfMinusHalfSize)
		{ // further away
			decalV = .5+.5*fSize;
		}
		else
		{
			decalV = .5+fSize;
		}
	}
	else if(newU<-fHalfMinusSize && newV>-fHalfMinusSize)
	{	// left
		// There're 2 tiles here
		iU = 2*iU-1;
		iV = 2*iV;
		decalV = .5+fSize;
		fTileID = 4;
		if(newU<-fHalfMinusHalfSize)
		{ // further away
			decalU = .5+.5*fSize;
		}
		else
		{
			decalU = .5+fSize;
		}
	}
	else
	{	// center
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption6(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 5 possible centers
	if(newV>.5-fSize && newU>.5-fSize)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 1;
	}
	else if(newV<-.5+fSize && newU>.5-fSize)
	{
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 2;
	}
	else if(newV<-.5+fSize && newU<-.5+fSize)
	{
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 3;
	}
	else if(newV>.5-fSize && newU<-.5+fSize)
	{
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 4;
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption7(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// There're 9 possible centers
	// 1 to 4: the corners
	// 5 to 8: the sides
	// 0: the square in the middle

	if(newV>.5-fSize)
	{	// Line on the top
		if(newU>.5-fSize)
		{	// We're in the 1st corner
			iU = 2*iU+1;
			iV = 2*iV+1;
			fTileID = 1;
		}
		else if(newU<fSize-.5)
		{	// We're in the 4th corner
			iU = 2*iU-1;
			iV = 2*iV+1;
			fTileID = 4;
		}
		else
		{	// we're in the top rectangle
			iU = 2*iU;
			iV = 2*iV+1;
			fTileID = 5;
		}
	}
	else if(newV<fSize-.5)
	{	// Line on the bottom
		if(newU>.5-fSize)
		{	// We're in the 2nd corner
			iU = 2*iU+1;
			iV = 2*iV-1;
			fTileID = 2;
		}
		else if(newU<fSize-.5)
		{	// We're in the 3rd corner
			iU = 2*iU-1;
			iV = 2*iV-1;
			fTileID = 3;
		}
		else
		{	// we're in the bottom rectangle
			iU = 2*iU;
			iV = 2*iV-1;
			fTileID = 7;
		}
	}
	else
	{	// Line on the middle
		if(newU>.5-fSize)
		{	// We're in the right rectangle
			iU = 2*iU+1;
			iV = 2*iV;
			fTileID = 6;
		}
		else if(newU<fSize-.5)
		{	// We're in the left rectangle
			iU = 2*iU-1;
			iV = 2*iV;
			fTileID = 8;
		}
		else
		{	// we're in the middle square
			iU = 2*iU;
			iV = 2*iV;
			fTileID = 0;
		}
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption8(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// Very similar to the option 7, but the rectangle is divided in 2 squares
	// => there're 13 centers
	// 1 to 4: the corners
	// 5 to 12: the sides
	// 0: the square in the middle

	decalU = kOneThird;
	decalV = kOneThird;

	if(newV>.5-fSize)
	{	// Line on the top
		if(newU>.5-fSize)
		{	// We're in the 1st corner
			iU = 3*iU+2;
			iV = 3*iV+2;
			fTileID = 1;
		}
		else if(newU<fSize-.5)
		{	// We're in the 4th corner
			iU = 3*iU-1;
			iV = 3*iV+2;
			fTileID = 4;
		}
		else if(newU>0)
		{	// we're in the top rectangle
			iU = 3*iU+1;
			iV = 3*iV+2;
			fTileID = 5;
		}
		else
		{	// we're in the top rectangle
			iU = 3*iU;
			iV = 3*iV+2;
			fTileID = 9;
		}
	}
	else if(newV<fSize-.5)
	{	// Line on the bottom
		if(newU>.5-fSize)
		{	// We're in the 2nd corner
			iU = 3*iU+2;
			iV = 3*iV-1;
			fTileID = 2;
		}
		else if(newU<fSize-.5)
		{	// We're in the 3rd corner
			iU = 3*iU-1;
			iV = 3*iV-1;
			fTileID = 3;
		}
		else if(newU>0)
		{	// we're in the bottom rectangle
			iU = 3*iU+1;
			iV = 3*iV-1;
			fTileID = 7;
		}
		else
		{	// we're in the bottom rectangle
			iU = 3*iU;
			iV = 3*iV-1;
			fTileID = 11;
		}
	}
	else
	{	// Line on the middle
		if(newU>.5-fSize)
		{	// We're in the right rectangle
			if(newV>0)
			{
				iU = 3*iU+2;
				iV = 3*iV+1;
				fTileID = 6;
			}
			else
			{
				iU = 3*iU+2;
				iV = 3*iV;
				fTileID = 10;
			}
		}
		else if(newU<fSize-.5)
		{	// We're in the left rectangle
			if(newV>0)
			{
				iU = 3*iU-1;
				iV = 3*iV+1;
				fTileID = 8;
			}
			else
			{
				iU = 3*iU-1;
				iV = 3*iV;
				fTileID = 12;
			}
		}
		else
		{	// we're in the middle square
			iU = 3*iU;
			iV = 3*iV;
			fTileID = 0;
			// to get the center
			decalU = .5;
			decalV = .5;
		}
	}

	ComputeTileCenter(3,3,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption9(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// Each corner contains 3 tiles
	// => there're 13 centers
	// 1 to 4: the corners (square part)
	// 5 to 12: the triangles in the corners
	// 0: the square in the middle
	// Note: we keep the same tileID than in the Opt2 to be able to use them in other methods
	if(newV>1-fSize-newU)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		if(newU<.5-.5*fSize)
		{	// top triangle
			fTileID = 5;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .75*fSize;
		}
		else if(newV<.5-.5*fSize)
		{	// side triangle
			fTileID = 6;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalV -= .75*fSize;
		}
		else
		{	// Square
			fTileID = 1;
		}
	}
	else if(newV<fSize-1+newU)
	{	// 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		if(newU<.5-.5*fSize)
		{	// bottom triangle
			fTileID = 7;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .75*fSize;
		}
		else if(newV>-.5+.5*fSize)
		{	// side triangle
			fTileID = 10;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalV += .75*fSize;
		}
		else
		{	// Square
			fTileID = 2;
		}
	}
	else if(newV<fSize-1-newU)
	{	// 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		if(newU>-.5+.5*fSize)
		{	// bottom triangle
			fTileID = 11;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU += .75*fSize;
		}
		else if(newV>-.5+.5*fSize)
		{	// side triangle
			fTileID = 12;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalV += .75*fSize;
		}
		else
		{	// Square
			fTileID = 3;
		}
	}
	else if(newV>1-fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		if(newU>-.5+.5*fSize)
		{	// top triangle
			fTileID = 9;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU += .75*fSize;
		}
		else if(newV<.5-.5*fSize)
		{	// side triangle
			fTileID = 8;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalV -= .75*fSize;
		}
		else
		{	// Square
			fTileID = 4;
		}
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption10(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// Very similar to option 3. Just need to decal the centers.
	// Each corner contains 3 tiles
	// => there're 13 centers
	// 1 to 4: the corners (square part)
	// 5 to 12: the triangles in the corners
	// 0: the square in the middle
	// Note: we keep the same tileID than in the Opt2 to be able to use them in other methods
	if(newV>1-fSize-newU)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		if(newU<.5-.5*fSize)
		{	// top triangle
			fTileID = 5;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .75*fSize;
			decalV -= .25*fSize;
		}
		else if(newV<.5-.5*fSize)
		{	// side triangle
			fTileID = 6;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .25*fSize;
			decalV -= .75*fSize;
		}
		else
		{	// Square
			fTileID = 1;
			// center of the small square
			decalU -= .25*fSize;
			decalV -= .25*fSize;
		}
	}
	else if(newV<fSize-1+newU)
	{	// 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		if(newU<.5-.5*fSize)
		{	// bottom triangle
			fTileID = 7;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .75*fSize;
			decalV += .25*fSize;
		}
		else if(newV>-.5+.5*fSize)
		{	// side triangle
			fTileID = 10;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU -= .25*fSize;
			decalV += .75*fSize;
		}
		else
		{	// Square
			fTileID = 2;
			decalU -= .25*fSize;
			decalV += .25*fSize;
		}
	}
	else if(newV<fSize-1-newU)
	{	// 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		if(newU>-.5+.5*fSize)
		{	// bottom triangle
			fTileID = 11;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalV += .25*fSize;
			decalU += .75*fSize;
		}
		else if(newV>-.5+.5*fSize)
		{	// side triangle
			fTileID = 12;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU += .25*fSize;
			decalV += .75*fSize;
		}
		else
		{	// Square
			fTileID = 3;
			decalU += .25*fSize;
			decalV += .25*fSize;
		}
	}
	else if(newV>1-fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		if(newU>-.5+.5*fSize)
		{	// top triangle
			fTileID = 9;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU += .75*fSize;
			decalV -= .25*fSize;
		}
		else if(newV<.5-.5*fSize)
		{	// side triangle
			fTileID = 8;
			// Decal to the triangle center. A triangle has many kind of centers, jsut use a convenient one
			decalU += .25*fSize;
			decalV -= .75*fSize;
		}
		else
		{	// Square
			fTileID = 4;
			decalU += .25*fSize;
			decalV -= .25*fSize;
		}
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption11(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// Each corner contains 2 tiles
	// => there're 9 centers
	// 1 to 4: the corners (square part)
	// 5 to 9: the strips in the corners
	// 0: the square in the middle
	// Note: we keep the same tileID than in the Opt2 to be able to use them in other methods
	if(newV>1-.5*fSize-newU)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 1;
	}
	else if(newV>1-fSize-newU)
	{	// We're in the strip behind the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 5;
		// Decal to the strip center. 
		decalU -= kOneThird*fSize;
		decalV -= kOneThird*fSize;
	}
	else if(newV<.5*fSize-1+newU)
	{	// 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 2;
	}
	else if(newV<fSize-1+newU)
	{	// strip in 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 6;
		// Decal to the strip center. 
		decalU -= kOneThird*fSize;
		decalV += kOneThird*fSize;
	}
	else if(newV<.5*fSize-1-newU)
	{	// 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 3;
	}
	else if(newV<fSize-1-newU)
	{	// strip in 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 7;
		// Decal to the strip center. 
		decalU += kOneThird*fSize;
		decalV += kOneThird*fSize;
	}
	else if(newV>1-.5*fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 4;
	}
	else if(newV>1-fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 8;
		// Decal to the strip center. 
		decalU += kOneThird*fSize;
		decalV -= kOneThird*fSize;
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption12(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// Very similar to the option 11. The diference is in the corner triangles: the center is now slightly offset
	// Each corner contains 2 tiles
	// => there're 9 centers
	// 1 to 4: the corners (square part)
	// 5 to 9: the strips in the corners
	// 0: the square in the middle
	// Note: we keep the same tileID than in the Opt2 to be able to use them in other methods
	if(newV>1-.5*fSize-newU)
	{	// We're in the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 1;
		// Decal to the triangle center. 
		decalU -= .125*fSize;
		decalV -= .125*fSize;
	}
	else if(newV>1-fSize-newU)
	{	// We're in the strip behind the 1st corner
		iU = 2*iU+1;
		iV = 2*iV+1;
		fTileID = 5;
		// Decal to the strip center. 
		decalU -= kOneThird*fSize;
		decalV -= kOneThird*fSize;
	}
	else if(newV<.5*fSize-1+newU)
	{	// 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 2;
		// Decal to the triangle center. 
		decalU -= .125*fSize;
		decalV += .125*fSize;
	}
	else if(newV<fSize-1+newU)
	{	// strip in 2nd corner
		iU = 2*iU+1;
		iV = 2*iV-1;
		fTileID = 6;
		// Decal to the strip center. 
		decalU -= kOneThird*fSize;
		decalV += kOneThird*fSize;
	}
	else if(newV<.5*fSize-1-newU)
	{	// 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 3;
		// Decal to the triangle center. 
		decalU += .125*fSize;
		decalV += .125*fSize;
	}
	else if(newV<fSize-1-newU)
	{	// strip in 3rd corner
		iU = 2*iU-1;
		iV = 2*iV-1;
		fTileID = 7;
		// Decal to the strip center. 
		decalU += kOneThird*fSize;
		decalV += kOneThird*fSize;
	}
	else if(newV>1-.5*fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 4;
		// Decal to the triangle center. 
		decalU += .125*fSize;
		decalV -= .125*fSize;
	}
	else if(newV>1-fSize+newU)
	{	// 4th corner
		iU = 2*iU-1;
		iV = 2*iV+1;
		fTileID = 8;
		// Decal to the strip center. 
		decalU += kOneThird*fSize;
		decalV -= kOneThird*fSize;
	}
	else
	{
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption13(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// A square split in 2 triangles: 2 centers
	if(newU>newV)
	{	// bottom-right tile
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 0;
		decalU += kOneThird;
		decalV -= kOneThird;
	}
	else
	{	// top-left tile
		iU = 2*iU;
		iV = 2*iV;
		fTileID = 1;
		decalU -= kOneThird;
		decalV += kOneThird;
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption14(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// A square split in 4 triangles: 4 centers
	// IDs: top:1,1 ; right:1,0 ; bot:0,0 ; left:0,1
	if(newU>newV)
	{	
		if(newV>-newU)
		{	// Right triangle
			iU = 2*iU;
			iV = 2*iV+1;
			fTileID = 0;
			decalU += kOneThird; // to rectify what's done with iU,iV
		}
		else
		{	// Bottom triangle
			iU = 2*iU;
			iV = 2*iV;
			fTileID = 1;
			decalV -= kOneThird; // to rectify what's done with iU,iV
		}
	}
	else
	{	
		if(newV>-newU)
		{	// Top triangle
			iU = 2*iU+1;
			iV = 2*iV+1;
			fTileID = 2;
			decalV += kOneThird; // to rectify what's done with iU,iV
		}
		else
		{	// Left triangle
			iU = 2*iU;
			iV = 2*iV+1;
			fTileID = 3;
			decalU -= kOneThird; // to rectify what's done with iU,iV
		}
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption15(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;

	// A square split in 8 triangles: 8 centers
	//
	// *********
	// ** 5*1 **
	// * * * * *
	// *4 *** 0*
	// *********
	// ** 7*3 **
	// * * * * *
	// *6 *** 2*
	// *********
	//
	if(newU>0)
	{
		if(newV>0)
		{	// first corner: 2 triangles
			iU = 2*iU+1;
			iV = 2*iV+1;
			if(newU>newV)
			{	// on the right
				fTileID = 0;
				decalU += kOneThird;
				decalV += kOneSixth;
			}
			else
			{	// on the top
				fTileID = 1;
				decalU += kOneSixth;
				decalV += kOneThird;
			}
		}
		else
		{	// second corner
			iU = 2*iU+1;
			iV = 2*iV;
			if(newU-.5>newV)
			{	// on the right
				fTileID = 2;
				decalU += kOneThird;
				decalV -= kOneThird;
			}
			else
			{	// in the middle
				fTileID = 3;
				decalU += kOneSixth;
				decalV -= kOneSixth;
			}
		}
	}
	else
	{
		if(newV>0)
		{	// third corner: 2 triangles
			iU = 2*iU;
			iV = 2*iV+1;
			if(-newU>newV)
			{	// on the left
				fTileID = 4;
				decalU -= kOneThird;
				decalV += kOneSixth;
			}
			else
			{	// on the top
				fTileID = 5;
				decalU -= kOneSixth;
				decalV += kOneThird;
			}
		}
		else
		{	// 4th corner
			iU = 2*iU;
			iV = 2*iV;
			if(-newU-.5>newV)
			{	// on the left
				fTileID = 6;
				decalU -= kOneThird;
				decalV -= kOneThird;
			}
			else
			{	// in the middle
				fTileID = 7;
				decalU -= kOneSixth;
				decalV -= kOneSixth;
			}
		}
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption16(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;
	
	// Similar to option 3 but one square is split the other way
	// A square split in 8 triangles: 8 centers
	//
	// *********
	// *5 **1 **
	// * * * * *
	// ** 4** 0*
	// *********
	// ** 7*3 **
	// * * * * *
	// *6 *** 2*
	// *********
	//
	if(newU>0)
	{
		if(newV>0)
		{	// first corner: 2 triangles
			iU = 2*iU+1;
			iV = 2*iV+1;
			if(newU>newV)
			{	// on the right
				fTileID = 0;
				decalU += kOneThird;
				decalV += kOneSixth;
			}
			else
			{	// on the top
				fTileID = 1;
				decalU += kOneSixth;
				decalV += kOneThird;
			}
		}
		else
		{	// second corner
			iU = 2*iU+1;
			iV = 2*iV;
			if(newU-.5>newV)
			{	// on the right
				fTileID = 2;
				decalU += kOneThird;
				decalV -= kOneThird;
			}
			else
			{	// in the middle
				fTileID = 3;
				decalU += kOneSixth;
				decalV -= kOneSixth;
			}
		}
	}
	else
	{
		if(newV>0)
		{	// third corner: 2 triangles
			// this one is different from the option 3
			iU = 2*iU;
			iV = 2*iV+1;
			if(newV>newU+.5)
			{	// on the corner
				fTileID = 4;
				decalU -= kOneThird;
				decalV += kOneThird;
			}
			else
			{	// on the middle
				fTileID = 5;
				decalU -= kOneSixth;
				decalV += kOneSixth;
			}
		}
		else
		{	// 4th corner
			iU = 2*iU;
			iV = 2*iV;
			if(-newU-.5>newV)
			{	// on the left
				fTileID = 6;
				decalU -= kOneThird;
				decalV -= kOneThird;
			}
			else
			{	// in the middle
				fTileID = 7;
				decalU -= kOneSixth;
				decalV -= kOneSixth;
			}
		}
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption17(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{
	newV = fromUV.y/fLength;
	iV = floor(newV); // in the iVeme tile
	newU = (fromUV.x/fLength);
	iU = floor(newU); // in the iUeme tile
	// Local coordinates
	newU -= iU;
	newV -= iV;

	// center on the middle of the tile
	newU -= .5;
	newV -= .5;

	// Note: we could compute all the center using only the decal parameter but it's better to offset
	// the iU and iV values in order to change the root value when a noise is added on a tile.
	real32 decalU = .5;
	real32 decalV = .5;
	
	// 12 triangles in a star shape arround the center
	iU = 2*iU;
	iV = 2*iV;
	if(newV>newU)
	{	// top left part
		if(newV>-newU)
		{	// top part: 3 triangles
			decalV+=kOneThird;
			if(newV<-3*newU)
			{	// left triangle
				fTileID = 0;
				decalU-=kOneSixth;
			}
			else if(newV<3*newU)
			{	// right triangle
				fTileID = 1;
				decalU+=kOneSixth;
			}
			else
			{	// middle triangle
				fTileID = 2;
			}
		}
		else
		{	// left part: 3 triangles
			decalU-=kOneThird;
			if(newV<kOneThird*newU)
			{	// bottom triangle
				decalV-=kOneSixth;
				fTileID = 3;
			}
			else if(newV>-kOneThird*newU)
			{	// top triangle
				decalV+=kOneSixth;
				fTileID = 4;
			}
			else
			{	// middle triangle
				fTileID = 5;
			}
		}
	}
	else
	{	// bottom right part
		if(newV>-newU)
		{	// right part: 3 triangles
			decalU+=kOneThird;
			if(newV<-kOneThird*newU)
			{	// bottom triangle
				decalV-=kOneSixth;
				fTileID = 6;
			}
			else if(newV>kOneThird*newU)
			{	// top triangle
				decalV+=kOneSixth;
				fTileID = 7;
			}
			else
			{	// middle triangle
				fTileID = 8;
			}
		}
		else
		{	// bottom part: 3 triangles
			decalV-=kOneThird;
			if(newV>3*newU)
			{	// left triangle
				decalU-=kOneSixth;
				fTileID = 9;
			}
			else if(newV>-3*newU)
			{	// right triangle
				decalU+=kOneSixth;
				fTileID = 10;
			}
			else
			{	// middle triangle
				fTileID = 11;
			}
		}
	}

	ComputeTileCenter(2,2,iU,iV,decalU,decalV);
}

void Engine1::GetLocalUVCoordOption18(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{	// Hexagonal tiles case with 3 branches joining the center
	
	// first find in which equilateral triangle we're in
	TVector2 hexagonCenter = TVector2::kZero;
	GetEquilateralTriangle(fromUV, newU, newV, iU, iV, hexagonCenter );

	// make the tiles continuous
	switch(fTileID)
	{
	case 3: iV-=1; break;
	// case 2: iU-=1; break;
	case 6: iU-=1; break;
	}

	if( fNeedCenter)
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		if(fTileID==1||fTileID==2)
		{	// first quadrilater
			fTileCenterUV.x = hexagonCenter.x - .125*fLength;
			fTileCenterUV.y = hexagonCenter.y + .5*fHalfLength; // half triangle height
		}
		else if(fTileID==3||fTileID==4)
		{	// second quadrilater
			fTileCenterUV.x = hexagonCenter.x + .25*fLength;
			fTileCenterUV.y = hexagonCenter.y;
		}
		else
		{	// third quadrilater
			fTileCenterUV.x = hexagonCenter.x - .125*fLength;
			fTileCenterUV.y = hexagonCenter.y - .5*fHalfLength; // half triangle height
		}

		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}

void Engine1::GetLocalUVCoordOption19(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{	// Hexagonal tiles case with 3 branches joining the center
	
	// first find in which equilateral triangle we're in
	TVector2 hexagonCenter = TVector2::kZero;
	GetEquilateralTriangle(fromUV, newU, newV, iU, iV, hexagonCenter );

	// Triangles ids map:
	//		*******
	//	   * * 2 * *
	//	  * 1 * * 3 *
	//	 *************
	//	  * 6 * * 4 *
	//     * * 5 * *
	//	    *******

	// We need to modify the ids to make something like that:
	//		*******
	//	   * 4 ** **
	//	  *  **   * *
	//	 ****   1 *2 *
	//	  *  **   * *
	//     * 3 ** **
	//	    *******
	int32 newTileID = 0;
	switch(fTileID)
	{
	case 3:
	case 4:
		{
			if(newU>.5) newTileID = 2;
			else newTileID = 1;
		} break;
	case 1:
	case 2:
		{
			if(newV>kTwoThird*fScaledTriangleHeight*(newU + 1))	newTileID = 4;
			else newTileID = 1;
		} break;
	case 5:
	case 6:
		{
			if(newV<-kTwoThird*fScaledTriangleHeight*(newU + 1))	newTileID = 3;
			else newTileID = 1;
		} break;
	}


	// make the tiles continuous
	switch(newTileID)
	{
	case 1:
		{
			switch(fTileID)
			{
				case 1:
				case 2: iV-=1; iU-=1; break;
				case 3: iV-=1; break;
				case 6: iU-=1; break;
			}
		} break;
	case 2:
		{
			if(fTileID == 3) iV-=1; 
		} break;
	case 3:
		{
			if(fTileID == 6) iU-=1;
		} break;
	}

	// Remember the new ID, that the one we want to get the uniform mapping
	fTileID = newTileID;

	if( fNeedCenter)
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		if(newTileID==1)
		{	// triangle in the middle
			fTileCenterUV = hexagonCenter;
		}
		else if(newTileID==2)
		{	// triangle on the side
			fTileCenterUV.x = hexagonCenter.x + .25*fLength;
			fTileCenterUV.y = hexagonCenter.y;
		}
		else if(newTileID==3)
		{	// triangle on the side
			fTileCenterUV.x = hexagonCenter.x - .125*fLength;
			fTileCenterUV.y = hexagonCenter.y - .5*fHalfLength; // half triangle height
		}
		else
		{	// triangle on the side
			fTileCenterUV.x = hexagonCenter.x - .125*fLength;
			fTileCenterUV.y = hexagonCenter.y + .5*fHalfLength; // half triangle height
		}

		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}

void Engine1::GetLocalUVCoordOption20(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV)
{	// Hexagonal tiles case with 3 branches joining the center
	
	// first find in which equilateral triangle we're in
	TVector2 hexagonCenter = TVector2::kZero;
	const int32 tileThreeID = GetEquilateralTriangle(fromUV, newU, newV, iU, iV, hexagonCenter );

	// Triangles ids map: (tileThreeID is I, II or III)
	//	             *******
	//	            *   2   *
	//	           * 1     3 *
	//		*******     II    *
	//	   *   2     6     4 *
	//	  * 1     3     5   *
	//	 *     I           *
	//	  * 6     4     2   *
	//     *   5     1     3 *
	//	    *******    III    *
	//	           * 6     4 *
	//              *   5   *
	//	             *******

	// make the tiles continuous: use iU and iV of the tile III,1
	switch(tileThreeID)
	{
	case 3:
		{
			switch(fTileID)
			{
				case 1: 
				case 2: break;
				case 3: iU+=1; break;
				case 4: 
				case 5: iU+=1; iV+=1; break;
				case 6: iV+=1; break;
			}
		} break;
	case 2:
		{
			switch(fTileID)
			{
				case 1: 
				case 2: iV-=2; iU-=1; break;
				case 3: iV-=2; break;
				case 4: 
				case 5: iV-=1; break;
				case 6: iU-=1; iV-=1; break;
			}
		} break;
	case 1:
		{
			switch(fTileID)
			{
				case 1: 
				case 2: iV-=1; iU-=2; break;
				case 3: iV-=1; iU-=1; break;
				case 4: 
				case 5: iU-=1; break;
				case 6: iU-=2; break;
			}
		} break;
	}

	// Remember the tileThreeID, that the one we want to get the uniform mapping
	fTileID = tileThreeID;

	if( fNeedCenter)
	{	// we're going to need the uv coordinate of the center of the tile, compute it now
		if(tileThreeID==1)
		{	
			fTileCenterUV.x = hexagonCenter.x + fTriangleSide;
			fTileCenterUV.y = hexagonCenter.y;
		}
		else if(tileThreeID==2)
		{
			fTileCenterUV.x = hexagonCenter.x - fHalfTriangleSide;
			fTileCenterUV.y = hexagonCenter.y - fHalfLength; // triangle height
		}
		else
		{
			fTileCenterUV.x = hexagonCenter.x - fHalfTriangleSide;
			fTileCenterUV.y = hexagonCenter.y + fHalfLength; // triangle height
		}

		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}

real32 Engine1::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	switch(fType)
	{
	case 'Opt1': return ComputeOption1(uu,vv);
	case 'Opt2': return ComputeOption2(uu,vv);
	case 'Opt3': return ComputeOption3(uu,vv);
	case 'Opt4': return ComputeOption4(uu,vv);

	case 'Opt5': return ComputeOption5(uu,vv);
	case 'Opt6': return ComputeOption6(uu,vv);
	case 'Opt7': return ComputeOption7(uu,vv);
	case 'Opt8': return ComputeOption8(uu,vv);
	case 'Opt9': return ComputeOption9(uu,vv);
	case 'Op10': return ComputeOption10(uu,vv);
	case 'Op11': return ComputeOption11(uu,vv);
	case 'Op12': return ComputeOption12(uu,vv);
	case 'Op13': return ComputeOption13(uu,vv);
	case 'Op14': return ComputeOption14(uu,vv);
	case 'Op15': return ComputeOption15(uu,vv);
	case 'Op16': return ComputeOption16(uu,vv);
	case 'Op17': return ComputeOption17(uu,vv);
	case 'Op18': return ComputeOption18(uu,vv);
	case 'Op19': return ComputeOption19(uu,vv);
	case 'Op20': return ComputeOption20(uu,vv);

	}

	return 0;
}

real32 Engine1::ComputeOption1(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>1-fSize-localU)
		{	// We're in the corner
			// Use a part of the next tile to smooth the corner
			// Rotate -45 degrees
			const real32 t1 = INV_SQRT2*((localV-fHalfMinusSize)-(localU-.5));
			const real32 t2 = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));

			const real32 h = SmoothStepWithTan(	fRescaledMortar*fMortarPlateau,
												fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,t2);
			const real32 l = SmoothStepWithTan(	fRescaledMortar*fMortarPlateau,
												fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,t1);
			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
		else
		{	// We're in the middle
			// Rotate 45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));

			const real32 h = 1-SmoothStepWithTan(	-fRescaledMortar,
													-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,t);
			const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
													.5-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>1-fSize-localU)
		{	// We're in the corner

			// Rotate -45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));

			return fMortarBumpDepth*SmoothStepWithTan(	fRescaledMortar*fMortarPlateau,
												fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,t);
		}
		else
		{	// We're in the middle
			if(localV>((1-2*fSize)*localU))
			{
				// Rotate 45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));

				return fMortarBumpDepth*(1-SmoothStepWithTan(	-fRescaledMortar,
											-fRescaledMortar*fMortarPlateau,
											fMortarBottomSlope,fMortarTopSlope,t));
			}
			else
			{
				return fMortarBumpDepth*(1-SmoothStepWithTan(	.5-fRescaledMortar,
											.5-fRescaledMortar*fMortarPlateau,
											fMortarBottomSlope,fMortarTopSlope,localU));
			}
		}
	}*/
}

real32 Engine1::ComputeOption2(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localU<fHalfMinusSize)
		{	// We're in the main square tile
			
			// Use the complete tile to smooth the corner

			const real32 l = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV);
			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
		else
		{	// We're in the diamand like tile
			// Rotate -45 degrees
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));

			if(localU<(.5-fSize*.5))
			{
				const real32 l = SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localU);
				const real32 h = 1-SmoothStepWithTan(	-fRescaledMortar,
													-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,t);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
			else
			{	// smooth with the piece of tile outside the domain
				const real32 t2 = INV_SQRT2*(+(localU-.5)+(localV-.5));
				const real32 l = 1-SmoothStepWithTan(	-fRescaledMortar,
													-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,t2);
				const real32 h = 1-SmoothStepWithTan(	-fRescaledMortar,
													-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,t);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localU<fHalfMinusSize)
		{	// We're in the main square tile
			return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU));
		}
		else
		{	// We're in the diamand like tile
			const real32 tmpU = localU-fHalfMinusSize;
			const real32 tmpV = localV-fHalfMinusSize;

			if(tmpV>-(1-2*fSize)*tmpU)
			{
				// Rotate -45 degrees
				const real32 t = INV_SQRT2*(-(tmpU)+(tmpV));

				return fMortarBumpDepth*( 1-SmoothStepWithTan(	-fRescaledMortar,
													-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,t));
			}
			else
			{
				return fMortarBumpDepth*SmoothStepWithTan( fRescaledMortar*fMortarPlateau,
															fRescaledMortar,
															fMortarBottomSlope,fMortarTopSlope,tmpU);
			}
		}
	}*/
}

real32 Engine1::ComputeOption3(const real32 uu, const real32 vv)
{
	// work in 1/4th of the tile ( 3 rotations )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if( (uu<0 && vv>0) || (vv<0 && uu>0) )
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if( localU<fHalfMinusSize && localV<fHalfMinusSize )
		{	// We re in the square in the middle
			const real32 l = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV);
			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
		else if( localV<fHalfMinusSize )
		{	// We're in the rectangle on the side
			const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV);
			if( localU<(.5-fSize*.5))
			{
				const real32 l = SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localU);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
			else
			{
				const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
		}
		else
		{	// We're in the top rectangle
			const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
													.5-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			if(localV<(.5-fSize*.5))
			{
				const real32 h = SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localV);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
			else
			{
				const real32 h = 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localV);
				if(fSmooth)	return h*l*fMortarBumpDepth;
				else		return fMortarBumpDepth*MC_Min(h,l);
			}
		}
	}
/*	else
	{	
		// Harsh corners method
		if( localU<fHalfMinusSize && localV<fHalfMinusSize )
		{	// We re in the square in the middle
			if(localU>localV)
			{
				return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
												fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU));
			}
			else
			{
				return fMortarBumpDepth*( 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
												fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV));
			}
		}
		else if( localV<fHalfMinusSize )
		{	// We're in the rectangle on the side
			if( localU<(.5-fSize*.5))
			{
				if(localU-fHalfMinusSize>-localV+fHalfMinusSize)
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
															fHalfMinusSize-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV) );
				}
				else
				{
					return fMortarBumpDepth*SmoothStepWithTan( fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localU);
				}
			}
			else
			{
				if(localU-.5<localV-fHalfMinusSize)
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
															fHalfMinusSize-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV) );
				}
				else
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU) );
				}
			}
		}
		else
		{	// We're in the top rectangle
			if(localV<(.5-fSize*.5))
			{
				if(-localU+.5<localV-fHalfMinusSize)
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU) );
				}
				else
				{
					return fMortarBumpDepth*( SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localV) );
				}
			}
			else
			{
				if(localU-.5>localV-.5)
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
													.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU) );
				}
				else
				{
					return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
													.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localV) );
				}
			}
		}
	}*/
}

real32 Engine1::ComputeOption4(const real32 uu, const real32 vv)
{
//	if(fSmooth)
	{
	// Smooth corners method: product of 3 pulses

	// Note: we use the fMortarDepth on only 1 pulse. When the 3 values are multiplied, 1*1*fMortarDepth = fMortarDepth

	// rotation of -60
	const real32 t1 = 1 + (-HALF_SQRT3*uu + 0.5*vv)/fScaledTriangleHeight;
	const real32 p1 = SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
					fRescaledTileEnd,fRescaledStartPlateau,
					fMortarTopSlope,fMortarBottomSlope,
					1, t1);
		
	// rotation of +60
	const real32 t2 = 1 + (HALF_SQRT3*uu + 0.5*vv)/fScaledTriangleHeight;
	const real32 p2 = SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
					fRescaledTileEnd,fRescaledStartPlateau,
					fMortarTopSlope,fMortarBottomSlope,
					1, t2);

	const real32 t3 = 1 + vv/fScaledTriangleHeight;
	const real32 p3 = SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
					fRescaledTileEnd,fRescaledStartPlateau,
					fMortarTopSlope,fMortarBottomSlope,
					1, t3);

	if(fSmooth)	return p1*p2*p3*fMortarBumpDepth;
	else		return fMortarBumpDepth*MC_Min(p1,p2,p3);
	}
/*	else
	{	
		// Harsh corners method: determine in which triangle we're in and do only one pulse
		
		// Angle
		real32 alpha = (real32)PI2;
		if(uu!=0)
			alpha = atan(vv/uu);
		const real32 alphaMinusPi = alpha-PI;

		if( ( alpha<PI3 && alpha>=0 ) ||
			( alphaMinusPi<PI3 && alphaMinusPi>=0 ) )
		{
			// rotation of +60
			const real32 t = 1 + (HALF_SQRT3*uu + 0.5*vv)/fScaledTriangleHeight;
			return SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
							fRescaledTileEnd,fRescaledStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, t);
		}

		if( ( alpha<0 && alpha>=-PI3 ) ||
			( alphaMinusPi<0 && alphaMinusPi>=-PI3 ) )
		{
			// rotation of -60
			const real32 t = 1 + (-HALF_SQRT3*uu + 0.5*vv)/fScaledTriangleHeight;
			return SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
							fRescaledTileEnd,fRescaledStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, t);
		}

		const real32 t = 1 + vv/fScaledTriangleHeight;
		return SmoothPulseWithTan(fRescaledEndPlateau,fRescaledTileStart,
						fRescaledTileEnd,fRescaledStartPlateau,
						fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, t);
	}*/
}

real32 Engine1::ComputeOption5(const real32 uu, const real32 vv)
{
	// This option is very similar to the 3. The rectangles arround are split in 2.

	// Symetrie: work in 1/4th of the tile
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if( (uu<0 && vv>0) || (vv<0 && uu>0) )
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if( localU<fHalfMinusSize && localV<fHalfMinusSize )
		{	// We re in the square in the middle
			const real32 l = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if( localV<fHalfMinusSize )
		{	// We're in the rectangle on the side
			const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
													fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV);
			// Split the tile in 2 pieces
			if( localU<(.5-fSize*.75) )
			{	// first step up
				const real32 l = SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if( localU<(.5-fSize*.5) )
			{	// First step down
				const real32 l = 1-SmoothStepWithTan(	fHalfMinusHalfSize-fRescaledMortar,
														fHalfMinusHalfSize-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if( localU<(.5-fSize*.25))
			{	// 2nd step up
				const real32 l = SmoothStepWithTan(	fHalfMinusHalfSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusHalfSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// 2nd step down
				const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
		else
		{	// We're in the top rectangle
			const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
													.5-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU);
			// Split the tile in 2 pieces
			if( localV<(.5-fSize*.75) )
			{	// first step up
				const real32 h = SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if( localV<(.5-fSize*.5) )
			{	// first step down
				const real32 h = 1-SmoothStepWithTan(	fHalfMinusHalfSize-fRescaledMortar,
														fHalfMinusHalfSize-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if(localV<(.5-fSize*.25))
			{	// 2nd step up
				const real32 h = SmoothStepWithTan(	fHalfMinusHalfSize+fRescaledMortar*fMortarPlateau,
													fHalfMinusHalfSize+fRescaledMortar,
													fMortarTopSlope,fMortarBottomSlope,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// 2nd step down
				const real32 h = 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
													fMortarBottomSlope,fMortarTopSlope,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
	}
/*	else
	{	
		// Harsh corners method
		if( localU<fHalfMinusSize && localV<fHalfMinusSize )
		{	// We re in the square in the middle
			if(localU>localV)
			{
				return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
												fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localU));
			}
			else
			{
				return fMortarBumpDepth*( 1-SmoothStepWithTan(	fHalfMinusSize-fRescaledMortar,
												fHalfMinusSize-fRescaledMortar*fMortarPlateau,
												fMortarBottomSlope,fMortarTopSlope,localV));
			}
		}
		else if( localV<fHalfMinusSize )
		{	// We're in the rectangle on the side

			// There's 2 rectangles
			if( localU<(.5-fSize*.5))
			{
				// First rectangle
				if( localU<(.5-fSize*.75))
				{
					if(localU-fHalfMinusSize>-localV+fHalfMinusSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
																fHalfMinusSize-fRescaledMortar*fMortarPlateau,
															fMortarBottomSlope,fMortarTopSlope,localV) );
					}
					else
					{
						return fMortarBumpDepth*SmoothStepWithTan( fHalfMinusSize+fRescaledMortar*fMortarPlateau,
														fHalfMinusSize+fRescaledMortar,
														fMortarTopSlope,fMortarBottomSlope,localU);
					}
				}
				else
				{
					if(localU-fHalfMinusHalfSize<localV-fHalfMinusSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
																fHalfMinusSize-fRescaledMortar*fMortarPlateau,
															fMortarBottomSlope,fMortarTopSlope,localV) );
					}
					else
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	fHalfMinusHalfSize-fRescaledMortar,
															fHalfMinusHalfSize-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
				}
			}
			else
			{	
				// Second rectangle
				if( localU<(.5-fSize*.25))
				{
					if(localU-fHalfMinusHalfSize>-localV+fHalfMinusSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
																fHalfMinusSize-fRescaledMortar*fMortarPlateau,
															fMortarBottomSlope,fMortarTopSlope,localV) );
					}
					else
					{
						return fMortarBumpDepth*SmoothStepWithTan( fHalfMinusHalfSize+fRescaledMortar*fMortarPlateau,
														fHalfMinusHalfSize+fRescaledMortar,
														fMortarTopSlope,fMortarBottomSlope,localU);
					}
				}
				else
				{
					if(localU-.5<localV-fHalfMinusSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan( fHalfMinusSize-fRescaledMortar,
																fHalfMinusSize-fRescaledMortar*fMortarPlateau,
															fMortarBottomSlope,fMortarTopSlope,localV) );
					}
					else
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
															.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
				}
			}
		}
		else
		{	// We're in the top rectangle

			// There're 2 rectangles
			if(localV<(.5-fSize*.5))
			{
				// First rectangle

				if(localV<(.5-fSize*.75))
				{			
					if(-localU+.5<localV-fHalfMinusSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
															.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
					else
					{
						return fMortarBumpDepth*( SmoothStepWithTan(	fHalfMinusSize+fRescaledMortar*fMortarPlateau,
														fHalfMinusSize+fRescaledMortar,
														fMortarTopSlope,fMortarBottomSlope,localV) );
					}
				}
				else
				{
					if(localU-.5>localV-fHalfMinusHalfSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
					else
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	fHalfMinusHalfSize-fRescaledMortar,
														fHalfMinusHalfSize-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV) );
					}
				}
			}
			else
			{
				// Second rectangle
				if(localV<(.5-fSize*.25))
				{
					
					if(-localU+.5<localV-fHalfMinusHalfSize)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
															.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
					else
					{
						return fMortarBumpDepth*( SmoothStepWithTan(	fHalfMinusHalfSize+fRescaledMortar*fMortarPlateau,
														fHalfMinusHalfSize+fRescaledMortar,
														fMortarTopSlope,fMortarBottomSlope,localV) );
					}
				}
				else
				{
					if(localU-.5>localV-.5)
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU) );
					}
					else
					{
						return fMortarBumpDepth*( 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV) );
					}
				}
			}
		}
	}*/
}

real32 Engine1::ComputeOption6(const real32 uu, const real32 vv)
{
	// A cross shape and 4 squares

	// Symetrie: work in 1/8th of the tile
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>.5-fSize)
		{	// We're in the corner
			const real32 l = SmoothStepWithTan(	fHalfMinusSize + fRescaledMortar*fMortarPlateau,
												fHalfMinusSize + fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,localU);
			const real32 h = SmoothStepWithTan(	fHalfMinusSize + fRescaledMortar*fMortarPlateau,
												fHalfMinusSize + fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// We're in the cross
			if(localU>.5-fSize)
			{
				const real32 h = 1-SmoothStepWithTan(	fHalfMinusSize - fRescaledMortar,
														fHalfMinusSize - fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV);
				const real32 l = 1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{
				const real32 h = 1-NegativeStep(fHalfMinusSize,localU);
				const real32 l = 1-NegativeStep(fHalfMinusSize,localV);
				if(fSmooth)	return fMortarBumpDepth*(1-l*h);
				else		return fMortarBumpDepth*(1-MC_Min(l,h));
			}
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>.5-fSize)
		{	// We're in the corner
			return fMortarBumpDepth*SmoothStepWithTan(	fHalfMinusSize + fRescaledMortar*fMortarPlateau,
												fHalfMinusSize + fRescaledMortar,
												fMortarTopSlope,fMortarBottomSlope,localV);
		}
		else
		{	// We're in the cross
			if(localV<localU-fSize)
			{
				return fMortarBumpDepth*(1-SmoothStepWithTan(	.5-fRescaledMortar,
														.5-fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localU));
			}
			else
			{
				return fMortarBumpDepth*(1-SmoothStepWithTan(	fHalfMinusSize - fRescaledMortar,
														fHalfMinusSize - fRescaledMortar*fMortarPlateau,
														fMortarBottomSlope,fMortarTopSlope,localV));
			}
		}
	}*/
}

real32 Engine1::ComputeOption7(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>.5-fSize)
		{	// We're in the corner
			const real32 h = PositiveStep(fHalfMinusSize,localU);
			const real32 l = PositiveStep(fHalfMinusSize,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localU>.5-fSize)
		{	// We're in the side rectangle
			const real32 h = PositiveStep(fHalfMinusSize,localU);
			const real32 l = NegativeStep(fHalfMinusSize,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// we're in the square in the middle
			const real32 h = NegativeStep(fHalfMinusSize,localU);
			const real32 l = NegativeStep(fHalfMinusSize,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>.5-fSize)
		{	// We're in the corner
			return fMortarBumpDepth*PositiveStep(fHalfMinusSize ,localV);
		}
		else if(localU>.5-fSize)
		{	// We're in the side rectangle
			if(localV+localU+2*fSize-1<0)
				return fMortarBumpDepth*PositiveStep(fHalfMinusSize ,localU);
			else
				return fMortarBumpDepth*NegativeStep(fHalfMinusSize ,localV);
		}
		else
		{	// we're in the square in the middle
			return fMortarBumpDepth*NegativeStep(fHalfMinusSize ,localU);
		}
	}*/
}

real32 Engine1::ComputeOption8(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>.5-fSize)
		{	// We're in the corner
			const real32 h = PositiveStep(fHalfMinusSize,localU);
			const real32 l = PositiveStep(fHalfMinusSize,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localU>.5-fSize)
		{	// We're in the side rectangle
			if(localV>.5*fSize)
			{	// upper part
				const real32 h = PositiveStep(fHalfMinusSize,localU);
				const real32 l = NegativeStep(fHalfMinusSize,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// lower part
				const real32 h = PositiveStep(fHalfMinusSize,localU);
				const real32 l = PositiveStep(0,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
		else
		{	// we're in the square in the middle
			const real32 h = NegativeStep(fHalfMinusSize,localU);
			const real32 l = NegativeStep(fHalfMinusSize,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>.5-fSize)
		{	// We're in the corner
			return fMortarBumpDepth*PositiveStep(fHalfMinusSize ,localV);
		}
		else if(localU>.5-fSize)
		{	// We're in the side rectangle
			if(localV>.5*(.5-fSize))
			{	// upper part
				if(localV+localU+2*fSize-1<0)
					return fMortarBumpDepth*PositiveStep(fHalfMinusSize ,localU);
				else
					return fMortarBumpDepth*NegativeStep(fHalfMinusSize ,localV);
			}
			else
			{	// lower part
				if(localV>localU+fSize-.5)
					return fMortarBumpDepth*PositiveStep(fHalfMinusSize ,localU);
				else
					return fMortarBumpDepth*PositiveStep(0 ,localV);
			}
		}
		else
		{	// we're in the square in the middle
			return fMortarBumpDepth*NegativeStep(fHalfMinusSize ,localU);
		}
	}*/
}

real32 Engine1::ComputeOption9(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>1-fSize-localU)
		{	// We're in the corner
			if(localV>fHalfMinusHalfSize)
			{	// We're in the small square
				const real32 h = PositiveStep(fHalfMinusHalfSize,localU);
				const real32 l = PositiveStep(fHalfMinusHalfSize,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if(localV>.5-.75*fSize)
			{	// in the triangle tip
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				const real32 h = PositiveStep(0,t);
				const real32 l = NegativeStep(fHalfMinusHalfSize,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{
				// Use a part of the next tile to smooth the corner
				// Rotate -45 degrees
				const real32 t1 = INV_SQRT2*((localV-fHalfMinusSize)-(localU-.5));
				const real32 t2 = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				const real32 h = PositiveStep(0,t1);
				const real32 l = PositiveStep(0,t2);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
		else
		{	// We're in the middle
			// Rotate 45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = NegativeStep(0,t);
			const real32 l = NegativeStep(.5,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>1-fSize-localU)
		{	// We're in the corner
			if(localV>fHalfMinusHalfSize)
			{	// We're in the small square
				return fMortarBumpDepth*PositiveStep(fHalfMinusHalfSize ,localV);
			}
			else if(localV>.75*(1-fSize)-.5*localU)
			{	// We're in the triangle, over the bissectrice
				return fMortarBumpDepth*NegativeStep(fHalfMinusHalfSize, localV);
			}
			else
			{	// We're in the triangle, under the bissectrice
				// Rotate -45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*PositiveStep(0 ,t);
			}
		}
		else
		{	// We're in the middle
			if(localV>((1-2*fSize)*localU))
			{
				// Rotate 45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*NegativeStep(0 ,t);
			}
			else
			{
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
		}
	}*/
}

real32 Engine1::ComputeOption10(const real32 uu, const real32 vv)
{
	// Similar to Opt9, but close the corner tiles
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>1-fSize-localU)
		{	// We're in the corner
			if(localV>fHalfMinusHalfSize)
			{	// We're in the small square: 3 corners to draw (we're in a 1/8th)
				if(localU<.5-.25*fSize)
				{
					const real32 h = PositiveStep(fHalfMinusHalfSize,localU);
					const real32 l = PositiveStep(fHalfMinusHalfSize,localV);
					if(fSmooth)	return fMortarBumpDepth*l*h;
					else		return fMortarBumpDepth*MC_Min(l,h);
				}
				else if(localV<.5-.25*fSize)
				{
					const real32 h = NegativeStep(.5,localU);
					const real32 l = PositiveStep(fHalfMinusHalfSize,localV);
					if(fSmooth)	return fMortarBumpDepth*l*h;
					else		return fMortarBumpDepth*MC_Min(l,h);
				}
				else
				{
					const real32 h = NegativeStep(.5,localU);
					const real32 l = NegativeStep(.5,localV);
					if(fSmooth)	return fMortarBumpDepth*l*h;
					else		return fMortarBumpDepth*MC_Min(l,h);
				}
			}
			else if(localV>1-.75*fSize-localU)
			{	// we're in the right angle part of the triangle
				const real32 h = NegativeStep(.5,localU);
				const real32 l = NegativeStep(fHalfMinusHalfSize,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if(localV<localU-.5*fSize)
			{	// We're in the lower part of the triangle
				// Rotate -45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				const real32 h = PositiveStep(0,t);
				const real32 l = NegativeStep(.5,localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// in the triangle tip
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				const real32 h = PositiveStep(0,t);
				const real32 l = NegativeStep(fHalfMinusHalfSize,localV);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
		else
		{	// We're in the middle
			// Rotate 45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = NegativeStep(0,t);
			const real32 l = NegativeStep(.5,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>1-fSize-localU)
		{	// We're in the corner
			if(localV>fHalfMinusHalfSize)
			{	// We're in the small square: 2 sides to draw
				if(localV>1-.5*fSize-localU)
					return fMortarBumpDepth*NegativeStep(.5 ,localU);
				else
					return fMortarBumpDepth*PositiveStep(fHalfMinusHalfSize ,localV);
			}
			else if( (localV<.75*(1-fSize)-.5*localU) &&
					(localV<1.5-fSize-2*localU) )
			{	// We're in the triangle, under the 2 bissectrices
				// Rotate -45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*PositiveStep(0 ,t);
			}
			else
			{	// We're in the triangle, over the 2 bissectrices
				if(localV>localU-.5*fSize)
					return fMortarBumpDepth*NegativeStep(fHalfMinusHalfSize, localV);
				else
					return fMortarBumpDepth*NegativeStep(.5, localU);
			}
		}
		else
		{	// We're in the middle
			if(localV>((1-2*fSize)*localU))
			{
				// Rotate 45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*NegativeStep(0, t);
			}
			else
			{
				return fMortarBumpDepth*NegativeStep(.5, localU);
			}
		}
	}*/
}

real32 Engine1::ComputeOption11(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>1-.5*fSize-localU)
		{	// We're in the corner
			// Use a part of the next tile to smooth the corner
			// Rotate -45 degrees
			const real32 t1 = INV_SQRT2*((localV-fHalfMinusHalfSize)-(localU-.5));
			const real32 t2 = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
			const real32 h = PositiveStep( 0 ,t1);
			const real32 l = PositiveStep( 0 ,t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>1-.75*fSize-localU)
		{	// we're in the upper part of the strip in the corner
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
			const real32 h = NegativeStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>1-fSize-localU)
		{	// we're in the lower part of the strip in the corner
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = PositiveStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// We're in the middle
			// Rotate 45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = NegativeStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>1-.5*fSize-localU)
		{	// We're in the corner
			// Rotate -45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
			return fMortarBumpDepth*PositiveStep( 0 ,t);
		}
		else if(localV>1-.75*fSize-localU)
		{	// we're in the upper part of the strip in the corner
			// See on which side of the bissectrice we are
			if(localV>.5*(localU + .5 - fSize))
			{	// We're over the bissectrice
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
				return fMortarBumpDepth*NegativeStep( 0 ,t);
			}
			else
			{	// We're under the bissectrice
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
		}
		else if(localV>1-fSize-localU)
		{	// we're in the lower part of the strip in the corner
			// See on which side of the bissectrice we'are
			if(localV>-2*localU+1.5-fSize)
			{	// We're over the bissectrice
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
			else
			{	// We're under the bissectrice
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*PositiveStep( 0 ,t);
			}
		}
		else
		{	// We're in the middle
			if(localV>((1-2*fSize)*localU))
			{
				// Rotate 45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*NegativeStep(0 ,t);
			}
			else
			{
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
		}
	}*/
}

real32 Engine1::ComputeOption12(const real32 uu, const real32 vv)
{
	// Very similar to Oprion 11, the corners are now closed
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV>1-.25*fSize-localU)
		{	// We're in the top part of the corner
			const real32 h = NegativeStep(.5 ,localU);
			const real32 l = NegativeStep(.5 ,localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>1-.5*fSize-localU)
		{	// We're in the bottom part of the corner
			// Rotate -45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
			const real32 h = PositiveStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>1-.75*fSize-localU)
		{	// we're in the upper part of the strip in the corner
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
			const real32 h = NegativeStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>1-fSize-localU)
		{	// we're in the lower part of the strip in the corner
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = PositiveStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// We're in the middle
			// Rotate 45 degrees
			const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
			const real32 h = NegativeStep( 0 ,t);
			const real32 l = NegativeStep(.5 ,localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>1-.5*fSize-localU)
		{	// We're in the corner
			if(localV>-2*localU+1.5-.5*fSize)
			{	// we're over the bissectrice
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
			else
			{
				// Rotate -45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
				return fMortarBumpDepth*PositiveStep( 0 ,t);
			}
		}
		else if(localV>1-.75*fSize-localU)
		{	// we're in the upper part of the strip in the corner
			// See on which side of the bissectrice we are
			if(localV>.5*(localU + .5 - fSize))
			{	// We're over the bissectrice
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusHalfSize));
				return fMortarBumpDepth*NegativeStep( 0 ,t);
			}
			else
			{	// We're under the bissectrice
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
		}
		else if(localV>1-fSize-localU)
		{	// we're in the lower part of the strip in the corner
			// See on which side of the bissectrice we'are
			if(localV>-2*localU+1.5-fSize)
			{	// We're over the bissectrice
				return fMortarBumpDepth*NegativeStep(.5 ,localU);
			}
			else
			{	// We're under the bissectrice
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*PositiveStep( 0 ,t);
			}
		}
		else
		{	// We're in the middle
			if(localV>((1-2*fSize)*localU))
			{
				// Rotate 45 degrees
				const real32 t = INV_SQRT2*((localU-.5)+(localV-fHalfMinusSize));
				return fMortarBumpDepth*NegativeStep( 0 ,t);
			}
			else
			{
				return fMortarBumpDepth*NegativeStep( .5 ,localU);
			}
		}
	}*/
}

real32 Engine1::ComputeOption13(const real32 uu, const real32 vv)
{
	// work in 1/4th of the tile ( 2 symetries )
	real32 localU=uu;
	real32 localV=vv;
	if(localV>localU)
	{
		localU=vv;
		localV=uu;
	}
	if(localV+localU<0)
	{
		real32 tmp = localU;
		localU=-localV;
		localV=-tmp;
	}

//	if(fSmooth)
	{
		if(localV>localU-.5)
		{	// triangle tip
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( .5, localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// Smooth with the corner outside the zone
			const real32 h = PositiveStep( -.5, localV);
			const real32 l = NegativeStep( .5, localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>2*localU-.5)
		{	// diagonal
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
		else
		{	// side 
			return fMortarBumpDepth*NegativeStep( .5, localU);
		}
	}*/

	return 0;
}

real32 Engine1::ComputeOption14(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localU>.25)
		{	// tip of the triangle
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( .5, localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// right angle of the triangle
			const real32 t1 = INV_SQRT2*(-(localU)+(localV));
			const real32 t2 = INV_SQRT2*(+(localU)+(localV));
			const real32 h = NegativeStep( 0, t1);
			const real32 l = PositiveStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV>2*localU-.5)
		{	// diagonal
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
		else
		{	// side 
			return fMortarBumpDepth*NegativeStep( .5, localU);
		}
	}*/

	return 0;
}

real32 Engine1::ComputeOption15(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=vv;
	if(localV<0)
	{
		localV+=.5;
	}
	if(localV>localU)
	{
		real32 tmp = localU;
		localU=localV;
		localV=tmp;
	}

//	if(fSmooth)
	{
		if(localV<localU-.25)
		{	// right angle of the triangle
			const real32 h = NegativeStep( .5, localU);
			const real32 l = PositiveStep( 0, localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>-localU+.5)
		{	// outside tip of the triangle
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( .5, localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// center tip of the triangle
			const real32 t = INV_SQRT2*(-(localU)+(localV));
			const real32 h = NegativeStep( 0, t);
			const real32 l = PositiveStep( 0, localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		// 3 zones
		if(localV<.5*localU)
		{
			if(localV<-localU+.5)
			{	// bottom
				return fMortarBumpDepth*PositiveStep( 0, localV);
			}
			else
			{	// side
				return fMortarBumpDepth*NegativeStep( .5, localU);
			}
		}
		else
		{
			if(localV>2*localU-.5)
			{	// diagonal
				const real32 t = INV_SQRT2*(-(localU)+(localV));
				return fMortarBumpDepth*NegativeStep( 0, t);
			}
			else
			{	// side
				return fMortarBumpDepth*NegativeStep( .5, localU);
			}
		}
	}*/

	return 0;
}

real32 Engine1::ComputeOption16(const real32 uu, const real32 vv)
{
	// Very similar to option 3, it's just the symetries that work diferently
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=uu;
	real32 localV=vv;
	if(localV<0)
	{
		localU=RealAbs(uu);
		localV+=.5;
	}
	else if(localU<0)
	{
		localU+=.5;
	}
	if(localV>localU)
	{
		real32 tmp = localU;
		localU=localV;
		localV=tmp;
	}

//	if(fSmooth)
	{
		if(localV<localU-.25)
		{	// right angle of the triangle
			const real32 h = NegativeStep( .5, localU);
			const real32 l = PositiveStep( 0, localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localV>-localU+.5)
		{	// outside tip of the triangle
			const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( .5, localU);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// center tip of the triangle
			const real32 t = INV_SQRT2*(-(localU)+(localV));
			const real32 h = NegativeStep( 0, t);
			const real32 l = PositiveStep( 0, localV);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method
		// 3 zones
		if(localV<.5*localU)
		{
			if(localV<-localU+.5)
			{	// bottom
				return fMortarBumpDepth*PositiveStep( 0, localV);
			}
			else
			{	// side
				return fMortarBumpDepth*NegativeStep( .5, localU);
			}
		}
		else
		{
			if(localV>2*localU-.5)
			{	// diagonal
				const real32 t = INV_SQRT2*(-(localU)+(localV));
				return fMortarBumpDepth*NegativeStep( 0, t);
			}
			else
			{	// side
				return fMortarBumpDepth*NegativeStep( .5, localU);
			}
		}
	}*/

	return 0;
}

real32 Engine1::ComputeOption17(const real32 uu, const real32 vv)
{
	// work in 1/8th of the tile ( 3 symetries )
	real32 localU=RealAbs(uu);
	real32 localV=RealAbs(vv);
	if(localV>localU)
	{
		localU=RealAbs(vv);
		localV=RealAbs(uu);
	}

//	if(fSmooth)
	{
		if(localV<kOneThird*localU)
		{	// Middle triangle
			if(localU<.25)
			{	// tip of the triangle. 2 rotations
				const real32 t1 = (-kSinAlpha*(localU)+kCosAlpha*(localV));
				const real32 t2 = (+kSinAlpha*(localU)+kCosAlpha*(localV));
				const real32 h = NegativeStep( 0, t1);
				const real32 l = PositiveStep( 0, t2);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// base of the triangle. 1 rotation of about 18 degrees
				const real32 t = (-kSinAlpha*(localU-.5)+kCosAlpha*(localV-.5*kOneThird));
				const real32 h = NegativeStep( 0, t);
				const real32 l = NegativeStep( .5, localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
		else
		{	// top triangle
			if(localU<.25)
			{	// tip of the triangle. 2 rotations of about 18 and 45 degrees
				const real32 t1 = INV_SQRT2*(-(localU-.5)+(localV-.5));
				const real32 t2 = (-kSinAlpha*(localU-.5)+kCosAlpha*(localV-.5*kOneThird));
				const real32 h = NegativeStep( 0, t1);
				const real32 l = PositiveStep( 0, t2);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else if(localV>kTwoThird*localU)
			{	// top part of the base. 1 Rotation of 45
				const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
				const real32 h = NegativeStep( 0, t);
				const real32 l = NegativeStep( .5, localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
			else
			{	// bottom part of the base. 1 rotation of about 18
				const real32 t = (-kSinAlpha*(localU-.5)+kCosAlpha*(localV-.5*kOneThird));
				const real32 h = PositiveStep( 0, t);
				const real32 l = NegativeStep( .5, localU);
				if(fSmooth)	return fMortarBumpDepth*l*h;
				else		return fMortarBumpDepth*MC_Min(l,h);
			}
		}
	}
/*	else
	{	
		// Harsh corners method
		if(localV<kOneThird*localU)
		{	// Middle triangle
			if(localV>9*localU/7.0 - 10.0/21.0) // equation of the bissectrice
			{
				const real32 t = (-kSinAlpha*(localU-.5)+kCosAlpha*(localV-.5*kOneThird));
				return fMortarBumpDepth*NegativeStep( 0, t);
			}
			else
			{
				return fMortarBumpDepth*NegativeStep( .5, localU);
			}
		}
		else
		{	// top triangle
			// 3 zones
			if(localV>3*localU/5.0)
			{
				if(localV>3*localU-1)
				{
					const real32 t = INV_SQRT2*(-(localU-.5)+(localV-.5));
					return fMortarBumpDepth*NegativeStep( 0, t);
				}
				else
				{	// Base of the triangle
					return fMortarBumpDepth*NegativeStep( .5, localU);
				}
			}
			else
			{
				if(localV<-localU+2.0/3.0)
				{
					const real32 t = (-kSinAlpha*(localU-.5)+kCosAlpha*(localV-.5*kOneThird));
					return fMortarBumpDepth*PositiveStep( 0, t);
				}
				else
				{	// Base of the triangle
					return fMortarBumpDepth*NegativeStep( .5, localU);
				}
			}
		}
	}*/

	return 0;
}

real32 Engine1::ComputeOption18(const real32 uu, const real32 vv)
{
	//		*******
	//	   *   2 * *
	//	  * 1   * 3 *
	//	 ********    *
	//	  * 6   * 4 *
	//     *   5 * *
	//	    *******

	// Several symetries to finaly work only in the area 3
	
	real32 localU=uu;
	real32 localV=RealAbs(vv); // U axis symmetry

	{	// diagonal
		if(localV>SQRT3*localU)
		{	// symmetry
			const real32 newLocalU = .5*(SQRT3*localV - localU);
			const real32 newLocalV = .5*(localV + SQRT3*localU);

			localU = newLocalU;
			localV = RealAbs(newLocalV); // U axis symmetry
		}
	}

//	if(fSmooth)
	{
		// 3 zones
		if(localV>.5*fScaledTriangleHeight)
		{	// tip of the triangle
			// rotation of -60 and +60
			const real32 t1 = (-HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			const real32 t2 = (+HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			const real32 h = NegativeStep( 0, t1);
			const real32 l = NegativeStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localU>.5)
		{	// outside corner, smooth with next triangle border
			// rotation of -60 and +60
			const real32 t1 = (-HALF_SQRT3*(localU-1) + 0.5*(localV));
			const real32 t2 = (+HALF_SQRT3*(localU-1) + 0.5*(localV));
			const real32 h = PositiveStep( 0, t1);
			const real32 l = NegativeStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// inside corner, smooth with next triangle border
			const real32 t1 = (-HALF_SQRT3*(localU) + 0.5*(localV));
			const real32 t2 = (+HALF_SQRT3*(localU) + 0.5*(localV));
			const real32 h = NegativeStep( 0, t1);
			const real32 l = PositiveStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method

		// 2 zones
		if(localU>.5)
		{	// After the bissectrice
			const real32 t = (HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
		else
		{	// Before the bissectrice
			const real32 t = (-HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
	}*/
	return 0;
}

real32 Engine1::ComputeOption19(const real32 uu, const real32 vv)
{
	//		*******
	//	   * 4 ** **
	//	  *  **   * *
	//	 ****   1 *2 *
	//	  *  **   * *
	//     * 3 ** **
	//	    *******

	// Several symetries to finaly work only in 1/6th of the area
	
	real32 localU=uu;
	real32 localV=RealAbs(vv); // U axis symmetry

	{	// diagonal
		if(localV>SQRT3*localU)
		{	// symmetry
			const real32 newLocalU = .5*(SQRT3*localV - localU);
			const real32 newLocalV = .5*(localV + SQRT3*localU);

			localU = newLocalU;
			localV = RealAbs(newLocalV); // U axis symmetry
		}
	}

//	if(fSmooth)
	{
		// 3 zones
		if(localU>.75f)
		{	// Tip of the isocel triangle
			const real32 t1 = (-HALF_SQRT3*(localU-1) + 0.5*(localV));
			const real32 t2 = (+HALF_SQRT3*(localU-1) + 0.5*(localV));
			const real32 h = PositiveStep( 0, t1);
			const real32 l = NegativeStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else if(localU>.5f)
		{	// Base of the isocel triangle
			const real32 t = (+HALF_SQRT3*(localU-.5f) + 0.5*(localV-fScaledTriangleHeight));
			const real32 h = NegativeStep( 0, t);
			const real32 l = PositiveStep( 0, localU-.5f);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// equilateral triangle
			const real32 t = (+HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( 0, localU-.5f);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method

		// 3 zones
		if(localV>SQRT3*(1.5-2*localU))
		{	// After the bissectrice of the isocel triangle
			const real32 t = (HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
		else if(localU>.5)
		{	// Before the bissectrice of the isocel triangle
			return fMortarBumpDepth*PositiveStep( 0, localU-.5);
		}
		else if(localV<SQRT3*localU)
		{	// equilateral triangle
			return fMortarBumpDepth*NegativeStep( 0, localU-.5);
		}
		else
		{	// equilateral triangle
			const real32 t = (+HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
	}*/
	return 0;
}

real32 Engine1::ComputeOption20(const real32 uu, const real32 vv)
{
	//	             *******
	//	            *   2   *
	//	           * 1     3 *
	//		*******     II    *
	//	   *   2     6     4 *
	//	  * 1     3     5   *
	//	 *     I           *
	//	  * 6     4     2   *
	//     *   5     1     3 *
	//	    *******    III    *
	//	           * 6     4 *
	//              *   5   *
	//	             *******

	// uu and vv are in 1 of the 3 hexagons, from -1 to 1 centered on the hexagon. 
	// fTileID is I, II or III

	// If II or III, rotate it to place it over I
	real32 localU=uu;
	real32 localV=vv;

	if(fTileID == 2)
	{
		// Rotate around its center
		localU = (-HALF_SQRT3*(vv) - 0.5*(uu));
		localV = (-HALF_SQRT3*(uu) + 0.5*(vv));
	}
	else if(fTileID == 3)
	{
		// Rotate around its center
		localU = (+HALF_SQRT3*(vv) - 0.5*(uu));
		localV = (-HALF_SQRT3*(uu) - 0.5*(vv));
	}

	// Then U axis symmetry
	localV = RealAbs(localV);

//	if(fSmooth)
	{
		// 3 zones
		if(localU>0)
		{	// concave corner
			const real32 t = (-HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			const real32 h = 1-NegativeStep( 0, t);
			const real32 l = 1-NegativeStep( 0, localV-fScaledTriangleHeight);
			if(fSmooth)	return fMortarBumpDepth*(1-l*h);
			else		return fMortarBumpDepth*(1-MC_Min(l,h));
		}
		else if(localV>.5*fScaledTriangleHeight)
		{	// convex corner 1
			const real32 t = (-HALF_SQRT3*(localU+.5) + 0.5*(localV-fScaledTriangleHeight));
			const real32 h = NegativeStep( 0, t);
			const real32 l = NegativeStep( 0, localV-fScaledTriangleHeight);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{	// convex corner 2
			const real32 t1 = (+HALF_SQRT3*(localU+1) + 0.5*(localV));
			const real32 t2 = (-HALF_SQRT3*(localU+1) + 0.5*(localV));
		
			const real32 h = PositiveStep( 0, t1);
			const real32 l = NegativeStep( 0, t2);
			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		// Harsh corners method

		// 3 zones
		if(localV<-SQRT3*localU)
		{
			const real32 t = (-HALF_SQRT3*(localU+1) + 0.5*(localV));
			return fMortarBumpDepth*NegativeStep( 0, t);
		}
		else if(localV<SQRT3*(1-localU))
		{	
			return fMortarBumpDepth*NegativeStep( 0, localV-fScaledTriangleHeight);
		}
		else
		{	
			const real32 t = (-HALF_SQRT3*(localU-.5) + 0.5*(localV-fScaledTriangleHeight));
			return fMortarBumpDepth*PositiveStep( 0, t);
		}
	}*/

	return 0;
}

inline real32 Engine1::PositiveStep(const real32 origin, const real32 local)
{
	return SmoothStepWithTan(	origin + fRescaledMortar*fMortarPlateau,
								origin + fRescaledMortar,
								fMortarTopSlope,fMortarBottomSlope,local);
}

inline real32 Engine1::NegativeStep(const real32 origin, const real32 local)
{
	return (1-SmoothStepWithTan(origin - fRescaledMortar,
								origin - fRescaledMortar*fMortarPlateau,
								fMortarTopSlope,fMortarBottomSlope,local));
}

real32 Engine1::GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	switch(fType)
	{
	case 'Opt1': return GetLocalTileUOption1(decalU,uu,iU,vv,iV);
	case 'Opt2': return GetLocalTileUOption2(decalU,uu,iU,vv,iV);
	case 'Opt3':
	case 'Opt5': return GetLocalTileUOption3(decalU,uu,iU,vv,iV);
	case 'Opt4': return (decalU + uu); // add n*iU so if a noise is used, it won't be the same on all the tiles

	case 'Opt6': return GetLocalTileUOption6(decalU,uu,iU,vv,iV);
	case 'Opt7': 
	case 'Opt8': 
	case 'Opt9': 
	case 'Op10': return GetLocalTileUOption7(decalU,uu,iU,vv,iV);
	case 'Op11': 
	case 'Op12': return GetLocalTileUOption11(decalU,uu,iU,vv,iV);
	case 'Op13': return GetLocalTileUOption13(decalU,uu,iU,vv,iV);
	case 'Op14': return GetLocalTileUOption14(decalU,uu,iU,vv,iV);
	case 'Op15': return GetLocalTileUOption15(decalU,uu,iU,vv,iV);
	case 'Op16': return GetLocalTileUOption16(decalU,uu,iU,vv,iV);
	case 'Op17': return GetLocalTileUOption17(decalU,uu,iU,vv,iV);
	case 'Op18': return GetLocalTileUOption18(decalU,uu,iU,vv,iV);
	case 'Op19': return GetLocalTileUOption19(decalU,uu,iU,vv,iV);
	case 'Op20': return GetLocalTileUOption20(decalU,uu,iU,vv,iV);

	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption1(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return (decalU + uu); // big tile.
		case 1: return decalU + HALF_SQRT2*((uu-.5) + (vv-.5));// upper right
		case 2: return decalU + HALF_SQRT2*((uu-.5) + (vv+.5)); // lower right
		case 3: return decalU + HALF_SQRT2*((uu+.5) + (vv+.5)); // lower left
		case 4: return decalU + HALF_SQRT2*((uu+.5) + (vv-.5)); // upper left
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalU + uu-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 2: return (decalU + uu-.5); // lower right
		case 3: // lower left
		case 4: return (decalU + uu+.5); // upper left
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption2(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		const int32 decalV=(fRandomUVOrigin?2*iV:0); // multiply by something so if a noise is used, it won't be the same on all the tiles
		switch(fTileID)
		{
		case 1: return (decalV + vv-.5); // upper
		case 3: return (decalV + vv+.5); // lower
		case 0: return (decalU + uu); // big tile
		case 2: return (decalU + uu-.5); // right
		case 4: return (decalU + uu+.5); // left
		}
	}
	else
	{
		switch(fTileID)
		{
		case 1: // upper
		case 3: // lower
		case 0: return (decalU + uu); // big tile
		case 2: return (decalU + uu-.5); // right
		case 4: return (decalU + uu+.5); // left
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption3(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(	fProportionnalTileShading &&
		(	fTileID == 1 || // upper
			fTileID == 3 ) ) // lower
	{
		const int32 decalV=(fRandomUVOrigin?2*iV:0); // multiply by something so if a noise is used, it won't be the same on all the tiles
		return (decalV + vv);
	}
	else
		return (decalU + uu); // add n*iU so if a noise is used, it won't be the same on all the tiles

	return 0;
}


inline real32 Engine1::GetLocalTileUOption6(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{	// proportional and non proportional are identicals: all the tiles are squares
	switch(fTileID)
	{
	case 0: return (decalU + uu-.5); // big tile. -.5 to shade on a [0,1] domain
	case 1: // upper right
	case 2: return (decalU + uu-.5); // lower right
	case 3: // lower left
	case 4: return (decalU + uu+.5); // upper left
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption7(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
		switch(fTileID)
		{
		case 0: return (decalU + uu-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 9: // top rect for Opt8
		case 7: // bottom rect
		case 11: // bottom rect for Opt8
		case 2: return (decalU + uu-.5); // lower right
		case 3: // lower left
		case 4: return (decalU + uu+.5); // upper left
		case 10: // left rect for Opt8
		case 6: return (decalV + vv-.5); // left rect
		case 12: // right rect for Opt8
		case 8: return (decalV + vv-.5); // right rect
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalU + uu-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 7: // bottom rect
		case 6: // left rect
		case 9: // top rect for Opt2
		case 11: // bottom rect for Opt2
		case 10: // left rect for Opt2
		case 2: return (decalU + uu-.5); // lower right
		case 3: // lower left
		case 8: // right rect
		case 12: // right rect for Opt2
		case 4: return (decalU + uu+.5); // upper left
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption11(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalU + HALF_SQRT2*((uu) + (vv));; // big tile.
		case 1: 
		case 5: return decalU + HALF_SQRT2*((uu-.5) + (vv-.5));// upper right
		case 2: 
		case 6: return decalU + HALF_SQRT2*((uu-.5) + (vv+.5)); // lower right
		case 3: 
		case 7: return decalU + HALF_SQRT2*((uu+.5) + (vv+.5)); // lower left
		case 4: 
		case 8: return decalU + HALF_SQRT2*((uu+.5) + (vv-.5)); // upper left
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalU + uu-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 7: // bottom rect
		case 6: // left rect
		case 9: // top rect for Opt2
		case 11: // bottom rect for Opt2
		case 10: // left rect for Opt2
		case 2: return (decalU + uu-.5); // lower right
		case 3: // lower left
		case 8: // right rect
		case 12: // right rect for Opt2
		case 4: return (decalU + uu+.5); // upper left
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption13(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalU + HALF_SQRT2*((uu-.5) + (vv+.5));
		case 1: return decalU + HALF_SQRT2*((uu+.5) + (vv-.5));
		}
	}
	else
	{
		return (decalU + uu-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption14(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 1: 
		case 2: return (decalU + uu-.5);
		case 0: 
		case 3:
			{
				const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
				return decalV + vv-.5;
			}
		}
	}
	else
	{
		return (decalU + uu-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption15(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalU + HALF_SQRT2*((uu-.5) + (vv));
		case 1: return decalU + HALF_SQRT2*((uu) + (vv-.5));
		case 2: return decalU + HALF_SQRT2*((uu-.5) + (vv+.5));
		case 3: return decalU + HALF_SQRT2*((uu) + (vv));
		case 4: return decalU + HALF_SQRT2*((uu+.5) - (vv));
		case 5: return decalU + HALF_SQRT2*((uu) - (vv-.5));
		case 6: return decalU + HALF_SQRT2*((uu+.5) - (vv+.5));
		case 7: return decalU + HALF_SQRT2*((uu) - (vv));
		}
	}
	else
	{
		return (decalU + uu-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption16(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalU + HALF_SQRT2*((uu-.5) + (vv));
		case 1: return decalU + HALF_SQRT2*((uu) + (vv-.5));
		case 2: return decalU + HALF_SQRT2*((uu-.5) + (vv+.5));
		case 3: return decalU + HALF_SQRT2*((uu) + (vv));
		case 4: return decalU + HALF_SQRT2*((uu) + (vv));
		case 5: return decalU + HALF_SQRT2*((uu+.5) + (vv-.5));
		case 6: return decalU + HALF_SQRT2*((uu+.5) - (vv+.5));
		case 7: return decalU + HALF_SQRT2*((uu) - (vv));
		}
	}
	else
	{
		return (decalU + uu-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption17(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: 
		case 1: 
		case 2:
		case 9:
		case 10:
		case 11:
			{
				const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
				return decalV + vv-.5;
			}
		default: return (decalU + uu-.5);
		}
	}
	else
	{
		return (decalU + uu-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption18(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// Tile 3-4 used as reference. Tiles 1-2 and 5-6 must rotate
		switch(fTileID)
		{
		case 3:
		case 4:
			{
				return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 1:
		case 2:
			{
				return (decalU + .5 + .25*(uu - SQRT3*vv)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 5:
		case 6:
			{
				return (decalU + .5 + .25*(uu + SQRT3*vv)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		}
	} 
	else
	{
		return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption19(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// tiles 1 and 2 straigh, rotate 3 and 4
		switch(fTileID)
		{
		case 1:
		case 2:
			{
				return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 3:
			{
				return (decalU + .5 + .25*(uu + SQRT3*vv)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 4:
			{
				return (decalU + .5 + .25*(uu - SQRT3*vv)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		}
	}
	else
	{
		return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	}

	return 0;
}

inline real32 Engine1::GetLocalTileUOption20(const int32 decalU, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// Switch the fTileID to offset the uu value
		if(fTileID == 2 || fTileID== 3)
			return (decalU + 1.25 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else
			return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	}
	else
	{	// Switch the fTileID to offset the uu value
		if(fTileID == 2 || fTileID== 3)
			return (decalU + 1.25 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else
			return (decalU + .5 + uu*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	}

	return 0;
}


real32 Engine1::GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	switch(fType)
	{
	case 'Opt1': return GetLocalTileVOption1(decalV,uu,iU,vv,iV);
	case 'Opt2': return GetLocalTileVOption2(decalV,uu,iU,vv,iV);
	case 'Opt3':
	case 'Opt5': return GetLocalTileVOption3(decalV,uu,iU,vv,iV);
	case 'Opt4': return GetLocalTileVOption4(decalV,uu,iU,vv,iV);

	case 'Opt6': return GetLocalTileVOption6(decalV,uu,iU,vv,iV);
	case 'Opt7': 
	case 'Opt8': 
	case 'Opt9': 
	case 'Op10': return GetLocalTileVOption7(decalV,uu,iU,vv,iV);
	case 'Op11': 
	case 'Op12': return GetLocalTileVOption11(decalV,uu,iU,vv,iV);
	case 'Op13': return GetLocalTileVOption13(decalV,uu,iU,vv,iV);
	case 'Op14': return GetLocalTileVOption14(decalV,uu,iU,vv,iV);
	case 'Op15': return GetLocalTileVOption15(decalV,uu,iU,vv,iV);
	case 'Op16': return GetLocalTileVOption16(decalV,uu,iU,vv,iV);
	case 'Op17': return GetLocalTileVOption17(decalV,uu,iU,vv,iV);
	case 'Op18': return GetLocalTileVOption18(decalV,uu,iU,vv,iV);
	case 'Op19': return GetLocalTileVOption19(decalV,uu,iU,vv,iV);
	case 'Op20': return GetLocalTileVOption20(decalV,uu,iU,vv,iV);

	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption1(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return (decalV + vv); // big tile.
		case 1: return decalV + HALF_SQRT2*((uu-.5) - (vv-.5)); // upper right, Execute a 45 degrees rotation
		case 4: return decalV + HALF_SQRT2*((uu+.5) - (vv-.5)); // upper left,Execute a 45 degrees rotation
		case 3: return decalV + HALF_SQRT2*((uu+.5) - (vv+.5)); // lower left
		case 2: return decalV + HALF_SQRT2*((uu-.5) - (vv+.5)); // lower right
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalV + vv-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 4: return (decalV + vv-.5); // upper left
		case 3: // lower left
		case 2: return (decalV + vv+.5); // lower right
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption2(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		const int32 decalU=(fRandomUVOrigin?2*iU:0); // multiply by something so if a noise is used, it won't be the same on all the tiles
		switch(fTileID)
		{
		case 2: // right
		case 4: // left
		case 0: return (decalV + vv); // big tile
		case 1: return (decalU + uu); // upper
		case 3: return (decalU + uu); // lower
		}
	}
	else
	{
		switch(fTileID)
		{
		case 2: // right
		case 4: // left
		case 0: return (decalV + vv); // big tile
		case 1: return (decalV + (vv-.5)); // upper
		case 3: return (decalV + (vv+.5)); // lower
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption3(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		if(	fTileID == 1 || // upper
			fTileID == 3 ) // lower
		{
			const int32 decalU=(fRandomUVOrigin?2*iU:0); // multiply by something so if a noise is used, it won't be the same on all the tiles
			return (decalU + uu);
		}
		else
			return (decalV + vv);
	}
	else
	{
		return (decalV + vv);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption4(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		return (decalV + vv);
	}
	else
	{
		return (decalV + vv*fScaledTriangleHeight);
	}

	return 0;
}


inline real32 Engine1::GetLocalTileVOption6(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	switch(fTileID)
	{
	case 0: return (decalV + vv-.5); // big tile. -.5 to shade on a [0,1] domain
	case 1: // upper right
	case 4: return (decalV + vv-.5); // upper left
	case 3: // lower left
	case 2: return (decalV + vv+.5); // lower right
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption7(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// Choose 2 rectangle and flip the mapping orientation on them
		const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
		switch(fTileID)
		{
		case 0: return (decalV + vv-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 9: // top rect for Opt2
		case 4: return (decalV + vv-.5); // upper left
		case 3: // lower left
		case 7: // bottom rect
		case 11: // bottom rect for Opt2
		case 2: return (decalV + vv+.5); // lower right
		// Flip on these 2
		case 10: // left rect for Opt2
		case 6: return (decalU + uu-.5); // left rect
		case 12:  // right rect for Opt2
		case 8: return (decalU + uu+.5); // right rect
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalV + vv-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 6: // left rect
		case 8: // right rect
		case 9: // top rect for Opt2
		case 10: // left rect for Opt2
		case 12: // right rect for Opt2
		case 4: return (decalV + vv-.5); // upper left
		case 3: // lower left
		case 7: // bottom rect
		case 11: // bottom rect for Opt2
		case 2: return (decalV + vv+.5); // lower right
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption11(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalV + HALF_SQRT2*((uu) - (vv)); // big tile.
		case 1: 
		case 5: return decalV + HALF_SQRT2*((uu-.5) - (vv-.5)); // upper right, Execute a 45 degrees rotation
		case 4:
		case 8: return decalV + HALF_SQRT2*((uu+.5) - (vv-.5)); // upper left,Execute a 45 degrees rotation
		case 3:
		case 7: return decalV + HALF_SQRT2*((uu+.5) - (vv+.5)); // lower left
		case 2:
		case 6: return decalV + HALF_SQRT2*((uu-.5) - (vv+.5)); // lower right
		}
	}
	else
	{
		switch(fTileID)
		{
		case 0: return (decalV + vv-.5); // big tile. -.5 to shade on a [0,1] domain
		case 1: // upper right
		case 5: // top rect
		case 6: // left rect
		case 8: // right rect
		case 9: // top rect for Opt2
		case 10: // left rect for Opt2
		case 12: // right rect for Opt2
		case 4: return (decalV + vv-.5); // upper left
		case 3: // lower left
		case 7: // bottom rect
		case 11: // bottom rect for Opt2
		case 2: return (decalV + vv+.5); // lower right
		}
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption13(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalV + HALF_SQRT2*((uu-.5) - (vv+.5));
		case 1: return decalV + HALF_SQRT2*((uu+.5) - (vv-.5));
		}
	}
	else
	{
		return (decalV + vv-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption14(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 1: 
		case 2: return (decalV + vv-.5);
		case 0: 
		case 3:
			{
				const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
				return decalU + (uu-.5);
			}
		}
	}
	else
	{
		return (decalV + vv-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption15(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalV + HALF_SQRT2*((uu-.5) - (vv));
		case 1: return decalV + HALF_SQRT2*((uu) - (vv-.5));
		case 2: return decalV + HALF_SQRT2*((uu-.5) - (vv+.5));
		case 3: return decalV + HALF_SQRT2*((uu) - (vv));
		case 4: return decalV + HALF_SQRT2*((uu+.5) + (vv));
		case 5: return decalV + HALF_SQRT2*((uu) + (vv-.5));
		case 6: return decalV + HALF_SQRT2*((uu+.5) + (vv+.5));
		case 7: return decalV + HALF_SQRT2*((uu) + (vv));
		}
	}
	else
	{
		return (decalV + vv-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption16(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: return decalV + HALF_SQRT2*((uu-.5) - (vv));
		case 1: return decalV + HALF_SQRT2*((uu) - (vv-.5));
		case 2: return decalV + HALF_SQRT2*((uu-.5) - (vv+.5));
		case 3: return decalV + HALF_SQRT2*((uu) - (vv));
		case 4: return decalV + HALF_SQRT2*((uu) - (vv));
		case 5: return decalV + HALF_SQRT2*((uu+.5) - (vv-.5));
		case 6: return decalV + HALF_SQRT2*((uu+.5) + (vv+.5));
		case 7: return decalV + HALF_SQRT2*((uu) + (vv));
		}
	}
	else
	{
		return (decalV + vv-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption17(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{
		switch(fTileID)
		{
		case 0: 
		case 1: 
		case 2:
		case 9:
		case 10:
		case 11:
			{
				const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
				return decalU + (uu-.5);
			}
		default: return (decalV + vv-.5);
		}
	}
	else
	{
		return (decalV + vv-.5);
	}

	return 0;
}

inline real32 Engine1::GetLocalTileVOption18(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// Tile 3-4 used as reference. Tiles 1-2 and 5-6 must rotate
		switch(fTileID)
		{
		case 3:
		case 4:
			{
				return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 1:
		case 2:
			{
				return (decalV + .5 + .25*(vv + SQRT3*uu)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 5:
		case 6:
			{
				return (decalV + .5 + .25*(vv - SQRT3*uu)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		}
	} 
	else
		return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain

	return 0;
}

inline real32 Engine1::GetLocalTileVOption19(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// tiles 1 and 2 straight, rotate 3 and 4
		switch(fTileID)
		{
		case 1:
		case 2:
			{
				return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 3:
			{
				return (decalV + .5 + .25*(vv - SQRT3*uu)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		case 4:
			{
				return (decalV + .5 + .25*(vv + SQRT3*uu)); // big tile. .5 + *.5 to shade on a [0,1] domain
			} break;
		}
	} 
	else
		return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain

	return 0;
}

inline real32 Engine1::GetLocalTileVOption20(const int32 decalV, const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	if(fProportionnalTileShading)
	{	// Switch the fTileID to offset the uu value
		if(fTileID == 2)
			return (decalV + .5 + fScaledTriangleHeight*.5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else if(fTileID== 3)
			return (decalV + .5 - fScaledTriangleHeight*.5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else
			return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	} 
	else
	{	// Switch the fTileID to offset the uu value
		if(fTileID == 2)
			return (decalV + .5 + fScaledTriangleHeight*.5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else if(fTileID== 3)
			return (decalV + .5 - fScaledTriangleHeight*.5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
		else
			return (decalV + .5 + vv*.5); // big tile. .5 + *.5 to shade on a [0,1] domain
	}

	return 0;
}


void Engine1::PreProcess( const PMapTilingShader1& pmap )
{
	fLength = 100/(real32)(pmap.fTileCount);

	fMortarBumpDepth = pmap.fMortarDepth/(real32)(pmap.fTileCount);

	fSize = pmap.fTileSize/2;
	fHalfMinusSize = .5-fSize;
	fHalfMinusHalfSize = .5-.5*fSize; 

	// Value needed for the 3 first arrangments
	fRescaledMortar = pmap.fMortarSize*.5;

	// values needed for the hexagonal tiling
	fHalfLength = fLength*.5; // is equal to the triangle height
	fTriangleSide = fHalfLength/HALF_SQRT3;
	fHalfTriangleSide = fTriangleSide*.5;
	fScaledTriangleHeight = fHalfLength/fTriangleSide;
	fRescaledTileStart = pmap.fMortarSize; // rescaled on a [0,2] space
	fRescaledTileEnd = 2-pmap.fMortarSize; // rescaled on a [0,2] space
	fRescaledEndPlateau = pmap.fMortarPlateau*pmap.fMortarSize;// rescaled on a [0,2] space
	fRescaledStartPlateau = 2-fRescaledEndPlateau;// rescaled on a [0,2] space

	// Get the slopes values
	switch(pmap.fBottomSlope)
	{
	case 'Opt1': fMortarBottomSlope = 0; break;
	case 'Opt2': fMortarBottomSlope = 1; break;
	case 'Opt3': fMortarBottomSlope = 2; break;
	}
	switch(pmap.fTopSlope)
	{
	case 'Opt1': fMortarTopSlope = 0; break;
	case 'Opt2': fMortarTopSlope = 1; break;
	case 'Opt3': fMortarTopSlope = 2; break;
	}

	if( pmap.fShadersComponents[2])
		fNeedCenter = true;
	else
		fNeedCenter = false;

	fType = pmap.fType;
	fSmooth = pmap.fSmoothMortar;
	fRandomUVOrigin = pmap.fRandomUVOrigin;
	fProportionnalTileShading = pmap.fProportionnalTileShading;
	fMortarPlateau = pmap.fMortarPlateau;
}

void Engine1::PreProcess( const PMapGridShader1& pmap )
{
	fLength = 100/(real32)(pmap.fTileCount);

	fMortarBumpDepth = pmap.fBumpDepth/(real32)(pmap.fTileCount);

	fSize = pmap.fTileSize/2;
	fHalfMinusSize = .5-fSize;
	fHalfMinusHalfSize = .5-.5*fSize; 

	// Value needed for the 3 first arrangments
	fRescaledMortar = pmap.fSectionSize*.5;

	// values needed for the hexagonal tiling
	fHalfLength = fLength*.5; // is equal to the triangle height
	fTriangleSide = fHalfLength/HALF_SQRT3;
	fHalfTriangleSide = fTriangleSide*.5;
	fScaledTriangleHeight = fHalfLength/fTriangleSide;
	fRescaledTileStart = pmap.fSectionSize; // rescaled on a [0,2] space
	fRescaledTileEnd = 2-pmap.fSectionSize; // rescaled on a [0,2] space
	fRescaledEndPlateau = pmap.fSectionPlateau*pmap.fSectionSize;// rescaled on a [0,2] space
	fRescaledStartPlateau = 2-fRescaledEndPlateau;// rescaled on a [0,2] space

	// Get the slopes values
	switch(pmap.fMiddleSlope)
	{
	case 'Opt1': fMortarBottomSlope = 0; break;
	case 'Opt2': fMortarBottomSlope = 1; break;
	case 'Opt3': fMortarBottomSlope = 2; break;
	}
	switch(pmap.fSideSlope)
	{
	case 'Opt1': fMortarTopSlope = 0; break;
	case 'Opt2': fMortarTopSlope = 1; break;
	case 'Opt3': fMortarTopSlope = 2; break;
	}

	if( pmap.fShadersComponents[2])
		fNeedCenter = true;
	else
		fNeedCenter = false;

	fType = pmap.fType;
	fSmooth = pmap.fSmoothConnection;
	fRandomUVOrigin = false;// not used for grids
	fProportionnalTileShading = false;// not used for grids
	fMortarPlateau = pmap.fSectionPlateau;
}

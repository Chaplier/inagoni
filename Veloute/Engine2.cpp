/****************************************************************************************************

		Engine2.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "math.h"
#include "Engine2.h"
#include "Utils.h"
#include "Veloute.h"
#include "GridShader2.h"
#include "TilingShader2.h"

#include "MCRandom.h"


Engine2::Engine2()	// We just initialize the values
{
	fTotalLength=0;
	fHeight=0;

	fHeightOnTotalLength=0;
	fHalfHeightOnTotalLength=0;
	fTwoHeightOnTotalLength=0;

	// Rectangular tile param
	fFirstTile=0;
	fLength1=0;
	fLength2=0;
	fHalfLength1=0;
	fLength1PlusHalfLength2=0;
	fLength1OnTotalLength=0;

	// Oblique tile param
	fSlopeCoeff=0;

// Auto block params
	fScaledMortarHor=0;
	fScaledMortarVer=0;
	fScaleObliqueMortar=0;
	fScaledFlat=0; // half flat part length
	fScaledGap=0; // distance from the center to the bottom of the gap
	// Other usefull preprocess values
	fScaledMorStartHor=0;
	fScaledMorStartVer=0;
	fScaledHalfMinusMorHor=0;
	fScaledHalfMinusStaHor=0;
	fScaledBeginMorVer=0;
	fScaledEndMorVer=0;
	fScaledBegin2MorVer=0;
	fScaledEnd2MorVer=0;
	fScaledHalfMinusFlat=0;
	fScaledOneMinusGap=0;

	fA=0; // oblique part is y = fA*x + fB
	fB=0;
	fCosA=0;
	fSinA=0;
	fTanB=0;
	fCosB=0;
	fSinB=0;

	fMortarBumpDepth=0;
	fMortarBottomSlope=0;
	fMortarTopSlope=0;

	fHDelta=0;
	fLDelta=0;

	//
	fType=0;
	fShifting=0;
	fMortarPlateau=0;
	fHMortar=0;
	fLMortar=0;
	fRandomUVOrigin=0;
	fProportionnalTileShading=0;
	fSmooth=0;
	fNeedCenter=0;
}
	
inline void BlockModifyUV(real32& newU, real32& newV, int32& iU, int32& iV)
{
	if(newU<0)
	{
		newU+=.5;
		if(newV<0)
		{
			iU -= 1;
			iV-=1;
			newV+=1;
		}
		else
		{
			iV+=1;
			newV-=1;
		}
	}
	else
	{
		newU-=.5;
		if(newV<0)
		{
			iV-=1;
			newV+=1;
		}
		else
		{
			iU += 1;
			iV+=1;
			newV-=1;
		}
	}
}

void Engine2::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, boolean& flip )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	// Build the tiles

	switch(fType)
	{
	case 'Opt7':
		{
			// First find in which set of 2 tiles ( one big and one small ) the point is
			newV = fromUV.y/fHeight;
			iV = floor(newV); // in the iVeme tile
			newU = (fromUV.x/fTotalLength + iV*fShifting);
			iU = floor(newU); // in the iUeme tile

			// Local coordinates
			newU -= iU;
			newV -= iV;

			// the first tile height determine the space
			if( newU<fLength1OnTotalLength )
			{
				fFirstTile = true;
				iU = 2*iU;
				newU /= fHeightOnTotalLength;
			}
			else
			{
				fFirstTile = false;
				flip = false;
				iU = 2*iU + 1;
				newU = (newU-fLength1OnTotalLength)/fHeightOnTotalLength;
			}


			if( fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.y=(iV+0.5)*fHeight;
				if(fFirstTile)
					fTileCenterUV.x=(iU/2 -iV*fShifting)*fTotalLength + fHalfLength1;
				else
					fTileCenterUV.x=(iU/2-iV*fShifting)*fTotalLength + fLength1PlusHalfLength2;
			}
		} break;
	case 'Opt8':
	case 'Opt9':
		{
			newU = fromUV.x/fTotalLength;
			iU = floor(newU); // in the iUeme tile

			// Shift the V value based on the newU
			if(fType == 'Opt8')
			{
				if(iU&0x00000001)
					newV = (fromUV.y - (fSlopeCoeff*(fromUV.x - (iU+1)*fTotalLength) + fHeight ))/fHeight;// - (2*fLength*fSlopeCoeff - fHeight )))/fHeight;
				else
					newV = (fromUV.y + (fSlopeCoeff*(fromUV.x - iU*fTotalLength)))/fHeight;
			}
			else
				newV = (fromUV.y + fSlopeCoeff*fromUV.x)/fHeight;

			iV = floor(newV); // in the iVeme tile

			// Local coordinates
			newU -= iU;
			newV -= iV;

			newU /= fHeightOnTotalLength;

			if( fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.y=(iV+0.5)*fHeight;
				fTileCenterUV.x=(iU+0.5)*fTotalLength;
			}
		}break;
	case 'Op10':
		{
			// First work with an average rectangular brick
			newV = fromUV.y/fHeight;
			iV = floor(newV); // in the iVeme tile
			newU = (fromUV.x/fTotalLength + iV*.5);
			iU = floor(newU); // in the iUeme tile
			// Local coordinates
			newU -= iU;
			newV -= iV;

			// now check that we are indeed in this brick and not the teton of a near one.

			// center
			newU-=.5;
			newV-=.5;
			// work in 1/4th of the tile ( 2 symetries )
			const real32 localU=RealAbs(newU);
			const real32 localV=RealAbs(newV);

			if( localU/fHeightOnTotalLength<fScaledFlat )
			{
				if(  localV>fScaledGap )
					BlockModifyUV(newU, newV, iU, iV);// we're in another tile
			}
			else if( localU/fHeightOnTotalLength>(1/fTwoHeightOnTotalLength-fScaledFlat) )
			{
				if(  localV>(fScaledOneMinusGap) )
					BlockModifyUV(newU, newV, iU, iV);// we're in another tile
			}
			else if( localV>(fA*localU/fHeightOnTotalLength+fB) )
			{
					BlockModifyUV(newU, newV, iU, iV);// we're in another tile
			}

			// Rescale keeping the height as a reference
			newU /= fHeightOnTotalLength;

			if( fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU+0.5 - iV*.5)*fTotalLength;
				fTileCenterUV.y = (iV+0.5)*fHeight;
			}
		}break;

	case 'Op11':
		{
			newV = fromUV.y/fHeight;
			iV = floor(newV); // in the iVeme tile
			const real32 shifting = fHeight/(fTotalLength);
			newU = (fromUV.x/fTotalLength + iV*shifting);
			iU = floor(newU); // in the iUeme tile

			// Local coordinates
			newU -= iU;
			newV -= iV;

			// the first tile height determine the space
			if( newU<fLength1OnTotalLength )
			{
				fFirstTile = true;
				flip = false;
				iU = 2*iU;
				newU /= fHeightOnTotalLength;
			}
			else
			{
				fFirstTile = false;
				flip = true;
				iU = 2*iU + 1;
				newU = (newU-fLength1OnTotalLength)/fHeightOnTotalLength;

				// in the 2nd tile, we need to rotate the U and V value of 90 degrees around a 'local' center
				// First determine which local center
				real32 uCube =  newU;
				const int32 iCube = floor(uCube);
				uCube-=iCube;
				newU = iCube + newV;
				newV = 1-uCube;

				// Readjust the iV param
				iV -= iCube;
			}

			if( fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.y=(iV+0.5)*fHeight;
				if(fFirstTile)
				{
					fTileCenterUV.x=(iU/2 -iV*fShifting)*fTotalLength + fHalfLength1;
				}
				else
				{
					fTileCenterUV.x=(iU/2-iV*fShifting)*fTotalLength + fLength1PlusHalfLength2;
				}
			}
		} break;
	case 'Op12':
		{
			// First find in which set of 2 tiles ( with a random separation between ) the point is
			newV = fromUV.y/fHeight;
			iV = floor(newV); // in the iVeme tile
			const real32 shifting = iV*iV*fShifting;
			newU = fromUV.x/fTotalLength + shifting; // add a shifting based on iV and weird % to create the random effect in iV
			iU = floor(newU); // in the iUeme tile

			// Local coordinates
			newU -= iU;
			newV -= iV;

			// use iU and iV to decide where the separation should appears
			const TVector2 fakePoint(iU*.432, iV*.371);
			// recompute cached data
			const real32 noiseValue = .5 + .5*fNoise.GetValueLinear2D(fakePoint);
			fLength1OnTotalLength = noiseValue;
			fLength1 = fLength1OnTotalLength * fTotalLength;
			fLength2 = fTotalLength - fLength1;
			fHalfLength1 = fLength1*.5;
			fLength1PlusHalfLength2 = fLength1 + 0.5*fLength2;

			// the tile height determine the space (height = 1, length = X)
			if( newU<fLength1OnTotalLength )
			{
				fFirstTile = true;
				iU = 2*iU;
				newU /= fHeightOnTotalLength;
			}
			else
			{
				fFirstTile = false;
				iU = 2*iU + 1;
				newU = (newU-fLength1OnTotalLength)/fHeightOnTotalLength;
			}


			if( fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.y=(iV+0.5)*fHeight;
				if(fFirstTile)
					fTileCenterUV.x=(iU/2 - shifting)*fTotalLength + fHalfLength1;
				else
					fTileCenterUV.x=(iU/2 - shifting)*fTotalLength + fLength1PlusHalfLength2;
			}
		} break;

	}

	if( fNeedCenter)
	{
		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}

	MCAssert( newU>=0 && newU<=1 );
	MCAssert( newV>=0 && newV<=1 );
}

real32 Engine2::GetLocalTileU(const real32 uu, const int32 iU)
{
	const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	if(fProportionnalTileShading)
		return (decalU + uu);
	else
		return (decalU + uu*fHeightOnTotalLength);
}

real32 Engine2::GetLocalTileV(const real32 vv,const int32 iV)
{
	const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	return decalV + vv;
}

// uu and vv go from 0 to 1 ( they were rescaled to a 2 bricks domain )
real32 Engine2::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	switch(fType)
	{
	case 'Opt7': return ComputeOption7(uu, vv);
	case 'Opt8':
	case 'Opt9': return ComputeOption8(uu, vv);
	case 'Op10': return ComputeOption10(uu, vv);

	case 'Op11': return ComputeOption11(uu, vv);
	case 'Op12': return ComputeOption12(uu, vv);

	}

	return 0;
}

real32 Engine2::ComputeOption7(const real32 uu, const real32 vv)
{
//	if(fSmooth)
	{	// Smooth corners method
		real32 lSize = 0;
		if(fFirstTile)
		{
			lSize = fLength1/fHeight;
		}
		else
		{
			lSize = fLength2/fHeight;
		}
		const real32 h = SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
						1, vv);
		
		const real32 l = SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
						1, uu);

		if(fSmooth)	return h*l*fMortarBumpDepth;
		else		return fMortarBumpDepth*MC_Min(h,l);
	}
/*	else
	{	// harsh corners method

		// Center the tile
		real32 remapU = uu;
		real32 remapV = vv-.5;

		real32 lSize = 0;
		if(fFirstTile)
		{
			lSize = fLength1/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		}
		else
		{
			lSize = fLength2/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		}

		if((remapU)>RealAbs(remapV))
		{
			return SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, uu);
		}
		else
		{
			return SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, vv);
		}
	}*/
}

real32 Engine2::ComputeOption8(const real32 uu, const real32 vv)
{
//	if(fSmooth)
	{
		// Smooth corners method: product of 2 pulses

		const real32 h = SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,
						fMortarTopSlope,fMortarBottomSlope,
						1, vv);
			
		const real32 lSize = fTotalLength/fHeight;
		const real32 l = SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,
						fMortarTopSlope,fMortarBottomSlope,
						1, uu);

		if(fSmooth)	return h*l*fMortarBumpDepth;
		else		return fMortarBumpDepth*MC_Min(h,l);
	}
/*	else
	{	
		// Harsh corners method
		// Center the tile
		real32 remapU = uu;
		real32 remapV = vv-.5;

		const real32 lSize = fTotalLength/fHeight;
		
		remapU-=lSize*.5;
		if(remapU<0)
		{
			remapU+=(lSize-1)*.5;
			remapU*=-1;
		}
		else
			remapU-=(lSize-1)*.5;

		if((remapU)>RealAbs(remapV))
			return SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,
						fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, uu);
		else
			return SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,
						fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, vv);
	}*/
}

real32 Engine2::ComputeOption10(const real32 uu, const real32 vv)
{
//	if(fSmooth)
	{
		// work in 1/4th of the tile ( 2 symetries )
		const real32 localU=RealAbs(uu);
		const real32 localV=RealAbs(vv);
	
		{
			// Rotate the oblique part to get the proper transition
			if(fA>0)
			{
				if(localV>.5)
				{
					// center of rotation is .5-fScaledFlat,1-fScaledGap
					const real t = (localU-fScaledHalfMinusFlat)*fCosA - (localV-fScaledOneMinusGap)*fSinA;

					// mortar due to the oblique part
					const real32 l = SmoothStepWithTan(	fScaleObliqueMortar*fMortarPlateau,
														fScaleObliqueMortar,
														fMortarTopSlope,fMortarBottomSlope,t);

					// mortar due to the vertical part
					const real32 l2 = SmoothStepWithTan(fScaledHalfMinusMorHor,fScaledHalfMinusStaHor,
								fMortarBottomSlope,fMortarTopSlope,localU);

					// mortar due to the horizontal part
					const real32 h = SmoothStepWithTan(	fScaledBeginMorVer,
														fScaledEndMorVer,
														fMortarBottomSlope,fMortarTopSlope,localV);
					
					if(fSmooth)	return fMortarBumpDepth*l*(1-h)*(1-l2);
					else		return fMortarBumpDepth*MC_Min(l,(1-h),(1-l2));
				}
				else
				{
					// center of rotation is fScaledFlat,fScaledGap
					const real t = (localU-fScaledFlat)*fCosA - (localV-fScaledGap)*fSinA;

					const real32 l = SmoothStepWithTan(	fScaleObliqueMortar*fMortarPlateau,
														fScaleObliqueMortar,
														fMortarTopSlope,fMortarBottomSlope,t);

					const real32 h = SmoothStepWithTan(	fScaledBegin2MorVer,
														fScaledEnd2MorVer,
														fMortarBottomSlope,fMortarTopSlope,localV);
					const real32 l2 = (SmoothStepWithTan(fScaledHalfMinusMorHor,fScaledHalfMinusStaHor,
								fMortarBottomSlope,fMortarTopSlope,localU));

					if(fSmooth)	return fMortarBumpDepth*(1-(1-l)*h)*(1-l2);
					else		return fMortarBumpDepth*MC_Min(1 - MC_Min((1-l),h),(1-l2));
				}
			}
			else
			{
				if(localV>.5)
				{
					// center of rotation is fScaledFlat,fScaledGap
					const real t = -(localU-fScaledFlat)*fCosA + (localV-fScaledGap)*fSinA;

					const real32 l = SmoothStepWithTan(	-fScaledMortarHor,
														-fScaledMorStartHor,
														fMortarTopSlope,fMortarBottomSlope,t);

					const real32 h = SmoothStepWithTan(	fScaledBegin2MorVer,
														fScaledEnd2MorVer,
														fMortarBottomSlope,fMortarTopSlope,localV);
				
					const real32 l2 = (SmoothStepWithTan(fScaledHalfMinusMorHor,fScaledHalfMinusStaHor,
								fMortarBottomSlope,fMortarTopSlope,localU));

					if(fSmooth)	return fMortarBumpDepth*(1-l)*(1-h)*(1-l2);
					else		return fMortarBumpDepth*MC_Min(1-l,1-h,(1-l2));
				}
				else
				{
					// center of rotation is .5-fScaledFlat,1-fScaledGap
					const real t = -(localU-fScaledHalfMinusFlat)*fCosA + (localV-fScaledOneMinusGap)*fSinA;

					const real32 l = SmoothStepWithTan(	-fScaledMortarHor,
														-fScaledMorStartHor,
														fMortarTopSlope,fMortarBottomSlope,t);
					const real32 h = SmoothStepWithTan(	fScaledBeginMorVer,
														fScaledEndMorVer,
														fMortarBottomSlope,fMortarTopSlope,localV);
					
					const real32 l2 = (SmoothStepWithTan(fScaledHalfMinusMorHor,fScaledHalfMinusStaHor,
								fMortarBottomSlope,fMortarTopSlope,localU));

					if(fSmooth)	return fMortarBumpDepth*(1-l*h)*(1-l2);
					else		return fMortarBumpDepth*MC_Min(1-MC_Min(l,h),(1-l2));
				}
			}
		}
	}
/* old method
	else
	{	
		// Harsh corners method

		// work in 1/4th of the tile ( 2 symetries )
		const real32 localU=RealAbs(uu);
		const real32 localV=RealAbs(vv);

		// 1
		real32 tmpU = localU-fScaledFlat;
		real32 tmpV = localV-fScaledGap;
		real32 tmpT = tmpU*fCosB - tmpV*fSinB;
		if(localU<fScaledHalfMinusMorHor && ( (fA>0&&tmpV<fTanB*tmpU) || (fA<0&&tmpV>fTanB*tmpU) ) )
		{	// we're in 1
			return fMortarBumpDepth*(1-SmoothStepWithTan( -fScaledMortarVer,
									-fScaledMorStartVer,
									fMortarBottomSlope,fMortarTopSlope,tmpV) );
		}
		else
		{
			// 2
			tmpU = localU-fScaledHalfMinusFlat;
			tmpV = localV-fScaledOneMinusGap;
			if(localU<fScaledHalfMinusMorHor && ( (fA>0&&tmpV<fTanB*tmpU) || (fA<0&&tmpV>fTanB*tmpU) ) )
			{	// we're in 2
				if(fA>0)
				{
					// rotate the oblique part
					const real t = tmpU*fCosA - tmpV*fSinA;
					return fMortarBumpDepth*(SmoothStepWithTan( fScaleObliqueMortar*fMortarPlateau,
											fScaleObliqueMortar,
											fMortarBottomSlope,fMortarTopSlope,t));
				}
				else
				{
					// rotate the oblique part
					const real t = -tmpU*fCosA + tmpV*fSinA;
					return fMortarBumpDepth*(1-SmoothStepWithTan( -fScaleObliqueMortar,
											-fScaleObliqueMortar*fMortarPlateau,
											fMortarBottomSlope,fMortarTopSlope,t));
				}
			}
			else
			{
				tmpU = localU-1/fTwoHeightOnTotalLength;
				// already done tmpV = localV-fScaledOneMinusGap;
				if(tmpV>tmpU)
				{	// we're in 3
					return fMortarBumpDepth*(1-SmoothStepWithTan( -fScaledMortarVer,
											-fScaledMorStartVer,
											fMortarBottomSlope,fMortarTopSlope,tmpV));
				}
				else
				{	// we're in 4
					return fMortarBumpDepth*(1-SmoothStepWithTan( -fScaledMortarHor,
											-fScaledMorStartHor,
											fMortarBottomSlope,fMortarTopSlope,tmpU));
				}
			}
		}
	}
	*/
}


real32 Engine2::ComputeOption11(const real32 uu, const real32 vv)
{
//	if(fSmooth)
	{	// Smooth corners method
		if(fFirstTile)
		{
			const real32 lSize = fLength1/fHeight;

			const real32 h = SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
							1-fHMortar,1-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
							1, vv);
			
			const real32 l = SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
							lSize-fLMortar,lSize-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
							1, uu);

			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
		else
		{
			const real32 lSize = fLength2/fHeight;

			const real32 h = SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
							1-fLMortar,1-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
							1, vv);
			
			const real32 l = SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
							lSize-fHMortar,lSize-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
							1, uu);

			if(fSmooth)	return h*l*fMortarBumpDepth;
			else		return fMortarBumpDepth*MC_Min(h,l);
		}
	}
/*	else
	{	// harsh corners method

		// Center the tile
		real32 remapU = uu;
		real32 remapV = vv-.5;

		real32 lSize = 0;

		real32 L = fLMortar;
		real32 H = fHMortar;

		if(fFirstTile)
		{
			lSize = fLength1/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		}
		else
		{
			lSize = fLength2/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		
			L = fHMortar;
			H = fLMortar;
		}

		if((remapU)>RealAbs(remapV))
		{
			return SmoothPulseWithTan(fMortarPlateau*L,L,
						lSize-L,lSize-fMortarPlateau*L,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, uu);
		}
		else
		{
			return SmoothPulseWithTan(fMortarPlateau*H,H,
						1-H,1-fMortarPlateau*H,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, vv);
		}
	}*/
}

real32 Engine2::ComputeOption12(const real32 uu, const real32 vv)
{	// Fake random length tiles
//	if(fSmooth)
	{	// Smooth corners method
		real32 lSize = 0;
		if(fFirstTile)
		{
			lSize = fLength1/fHeight;
		}
		else
		{
			lSize = fLength2/fHeight;
		}
		const real32 h = SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
						1, vv);
		
		const real32 l = SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
						1, uu);

		if(fSmooth)	return h*l*fMortarBumpDepth;
		else		return fMortarBumpDepth*MC_Min(h,l);
	}
/*	else
	{	// harsh corners method

		// Center the tile
		real32 remapU = uu;
		real32 remapV = vv-.5;

		real32 lSize = 0;
		if(fFirstTile)
		{
			lSize = fLength1/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		}
		else
		{
			lSize = fLength2/fHeight;
			remapU-=lSize*.5;
			if(remapU<0)
			{
				remapU+=(lSize-1)*.5;
				remapU*=-1;
			}
			else
				remapU-=(lSize-1)*.5;
		}

		if((remapU)>RealAbs(remapV))
		{
			return SmoothPulseWithTan(fMortarPlateau*fLMortar,fLMortar,
						lSize-fLMortar,lSize-fMortarPlateau*fLMortar,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, uu);
		}
		else
		{
			return SmoothPulseWithTan(fMortarPlateau*fHMortar,fHMortar,
						1-fHMortar,1-fMortarPlateau*fHMortar,fMortarTopSlope,fMortarBottomSlope,
						fMortarBumpDepth, vv);
		}
	}*/
}


void Engine2::PreProcess(const PMapTilingShader2& pmap)
{
	if(pmap.fType == 'Opt7' || pmap.fType == 'Op12')
	{
		fLength1 = 200/(pmap.fHorizontal*(1+pmap.fTileProp));
		fLength2 = pmap.fTileProp*200/(pmap.fHorizontal*(1+pmap.fTileProp));
		fTotalLength = fLength1+fLength2;
	
		fHalfLength1 = fLength1*.5;
		fLength1PlusHalfLength2 = fLength1 + 0.5*fLength2;
		fLength1OnTotalLength = fLength1/fTotalLength;
	}
	else if(pmap.fType == 'Op11')
	{
		fLength1 = 10 * pmap.fHorizontal * pmap.fThickness; // we need an integer number of time the thickness
		fLength2 = 10 * pmap.fVertical * pmap.fThickness; // we need an integer number of time the thickness
		fTotalLength = fLength1+fLength2;
		fLength1OnTotalLength = fLength1/fTotalLength;
	}
	else
		fTotalLength = 100/(real32)(pmap.fHorizontal);

	if(pmap.fType == 'Op11')
		fHeight = 10*pmap.fThickness;
	else
		fHeight = 100/(real32)pmap.fVertical;

	fMortarBumpDepth = pmap.fMortarDepth/(real32)(pmap.fVertical);

	fHeightOnTotalLength = fHeight/fTotalLength;
	fHalfHeightOnTotalLength = fHeightOnTotalLength*.5;
	fTwoHeightOnTotalLength = fHeightOnTotalLength*2;
	fSlopeCoeff = RealTan( PI2 * pmap.fSlope )/fHeightOnTotalLength;

// Auto block
	// Scaled value
	fScaledFlat = pmap.fGapInclination/(4*fHeightOnTotalLength);
	fScaledGap = (1+pmap.fGapDepth)/2;
	if(fScaledFlat<1/(4*fHeightOnTotalLength))
	{
		fA = (.5-fScaledGap)/(1/(4*fHeightOnTotalLength)-fScaledFlat);
		fB = (fScaledGap/fHeightOnTotalLength - 2*fScaledFlat)/(1/fHeightOnTotalLength-4*fScaledFlat);
	}
	else
	{
		fA = kRealBig;
		fB = kRealBig;
	}

	const real32 denom = RealSqrt(1+fA*fA);
	fCosA = fA/denom;
	fSinA = 1/denom;

	real32 Adegre=0;
	RealArcSinCos(fSinA,fCosA,Adegre);
	const real32 beta = -(DegToRad(Adegre) + PI*.5)*.5;
	fTanB = RealTan(beta);
	fCosB = RealCos(beta);
	fSinB = RealSin(beta);

	// Random tile
	if(pmap.fType == 'Op12')
	{
		// Perlin noise
		fNoise.SetSeed(pmap.fSeed);
		fNoise.InitGradientTab();
		fNoise.SetMaxFrequency(2);

		// Make a random shifting
		TVector2 fakePoint(.5,.5);
		fShifting = fNoise.GetValueLinear2D(fakePoint);
	}
	else
	{
		fShifting = pmap.fShifting;
	}

	fScaledMortarHor = pmap.fLMortar;
	fScaledMortarVer = pmap.fHMortar;//*(real32)(pmap.fVertical)/(real32)(pmap.fHorizontal);
	fScaleObliqueMortar = fScaledMortarHor*fCosA*fCosA + fScaledMortarVer*fSinA*fSinA;

	fScaledMorStartHor = fScaledMortarHor*pmap.fMortarPlateau;
	fScaledMorStartVer = fScaledMortarVer*pmap.fMortarPlateau;
	fScaledHalfMinusMorHor = 1/fTwoHeightOnTotalLength-fScaledMortarHor;//.5-fScaledMortarHor;
	fScaledHalfMinusStaHor = 1/fTwoHeightOnTotalLength-fScaledMorStartHor;//.5-fScaledMorStartHor;
	fScaledBeginMorVer = 1-fScaledGap-fScaledMortarVer;
	fScaledEndMorVer = 1-fScaledGap-fScaledMorStartVer;
	fScaledBegin2MorVer = fScaledGap-fScaledMortarVer;
	fScaledEnd2MorVer = fScaledGap-fScaledMorStartVer;
	fScaledHalfMinusFlat = 1/fTwoHeightOnTotalLength-fScaledFlat;//0.5-fScaledFlat;
	fScaledOneMinusGap = 1-fScaledGap;

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

	fType = pmap.fType;
	fMortarPlateau = pmap.fMortarPlateau;
	fHMortar = pmap.fHMortar;
	fLMortar = pmap.fLMortar;
	fRandomUVOrigin = pmap.fRandomUVOrigin;
	fProportionnalTileShading = pmap.fProportionnalTileShading;
	fSmooth = pmap.fSmoothMortar;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}

void Engine2::PreProcess(const PMapGridShader2& pmap)
{
	if(pmap.fType == 'Opt7' || pmap.fType == 'Op12')
	{
		fLength1 = 200/(pmap.fHorizontal*(1+pmap.fTileProp));
		fLength2 = pmap.fTileProp*200/(pmap.fHorizontal*(1+pmap.fTileProp));
		fTotalLength = fLength1+fLength2;
	
		fHalfLength1 = fLength1*.5;
		fLength1PlusHalfLength2 = fLength1 + 0.5*fLength2;
		fLength1OnTotalLength = fLength1/fTotalLength;
	}
	else if(pmap.fType == 'Op11')
	{
		fLength1 = 10 * pmap.fHorizontal * pmap.fThickness; // we need an integer number of time the thickness
		fLength2 = 10 * pmap.fVertical * pmap.fThickness; // we need an integer number of time the thickness
		fTotalLength = fLength1+fLength2;
		fLength1OnTotalLength = fLength1/fTotalLength;
	}
	else
		fTotalLength = 100/(real32)(pmap.fHorizontal);

	if(pmap.fType == 'Op11')
		fHeight = 10*pmap.fThickness;
	else
		fHeight = 100/(real32)pmap.fVertical;

	fMortarBumpDepth = pmap.fBumpDepth/(real32)(pmap.fVertical);

	fHeightOnTotalLength = fHeight/fTotalLength;
	fHalfHeightOnTotalLength = fHeightOnTotalLength*.5;
	fTwoHeightOnTotalLength = fHeightOnTotalLength*2;
	fSlopeCoeff = RealTan( PI2 * pmap.fSlope )/fHeightOnTotalLength;

// Auto block
	// Scaled value
	fScaledFlat = pmap.fGapInclination/(4*fHeightOnTotalLength);
	fScaledGap = (1+pmap.fGapDepth)/2;
	if(fScaledFlat<1/(4*fHeightOnTotalLength))
	{
		fA = (.5-fScaledGap)/(1/(4*fHeightOnTotalLength)-fScaledFlat);
		fB = (fScaledGap/fHeightOnTotalLength - 2*fScaledFlat)/(1/fHeightOnTotalLength-4*fScaledFlat);
	}
	else
	{
		fA = kRealBig;
		fB = kRealBig;
	}

	const real32 denom = RealSqrt(1+fA*fA);
	fCosA = fA/denom;
	fSinA = 1/denom;

	real32 Adegre=0;
	RealArcSinCos(fSinA,fCosA,Adegre);
	const real32 beta = -(DegToRad(Adegre) + PI*.5)*.5;
	fTanB = RealTan(beta);
	fCosB = RealCos(beta);
	fSinB = RealSin(beta);

	// Random tile
	if(pmap.fType == 'Op12')
	{
		// Perlin noise
		fNoise.SetSeed(pmap.fSeed);
		fNoise.InitGradientTab();
		fNoise.SetMaxFrequency(2);

		// Make a random shifting
		TVector2 fakePoint(.5,.5);
		fShifting = fNoise.GetValueLinear2D(fakePoint);
	}
	else
	{
		fShifting = pmap.fShifting;
	}

	fScaledMortarHor = pmap.fLSection;
	fScaledMortarVer = pmap.fHSection;//*(real32)(pmap.fVertical)/(real32)(pmap.fHorizontal);
	fScaleObliqueMortar = fScaledMortarHor*fCosA*fCosA + fScaledMortarVer*fSinA*fSinA;

	fScaledMorStartHor = fScaledMortarHor*pmap.fSectionPlateau;
	fScaledMorStartVer = fScaledMortarVer*pmap.fSectionPlateau;
	fScaledHalfMinusMorHor = 1/fTwoHeightOnTotalLength-fScaledMortarHor;//.5-fScaledMortarHor;
	fScaledHalfMinusStaHor = 1/fTwoHeightOnTotalLength-fScaledMorStartHor;//.5-fScaledMorStartHor;
	fScaledBeginMorVer = 1-fScaledGap-fScaledMortarVer;
	fScaledEndMorVer = 1-fScaledGap-fScaledMorStartVer;
	fScaledBegin2MorVer = fScaledGap-fScaledMortarVer;
	fScaledEnd2MorVer = fScaledGap-fScaledMorStartVer;
	fScaledHalfMinusFlat = 1/fTwoHeightOnTotalLength-fScaledFlat;//0.5-fScaledFlat;
	fScaledOneMinusGap = 1-fScaledGap;

	// Get the slopes values
	switch(pmap.fMiddleSlope)
	{
	case 'Opt1': fMortarBottomSlope = 0; break;
	case 'Opt2': fMortarBottomSlope = 1; break;
	case 'Opt3': fMortarBottomSlope = 2; break;
	}
	switch(pmap.fSidesSlope)
	{
	case 'Opt1': fMortarTopSlope = 0; break;
	case 'Opt2': fMortarTopSlope = 1; break;
	case 'Opt3': fMortarTopSlope = 2; break;
	}

	fType = pmap.fType;
	fMortarPlateau = pmap.fSectionPlateau;
	fHMortar = pmap.fHSection;
	fLMortar = pmap.fLSection;
	fRandomUVOrigin = false;
	fProportionnalTileShading = false;
	fSmooth = pmap.fSmoothSection;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}


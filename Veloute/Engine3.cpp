/****************************************************************************************************

		Engine3.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "Engine3.h"


#include "math.h"
#include "Utils.h"
#include "Veloute.h"
#include "TilingShader3.h"
#include "GridShader3.h"

Engine3::Engine3()	// We just initialize the values
{
	fSideLength = 0;
	fHalfSideLength = 0;

	fPeriodicity=0;
	fTwoPeriodicity=0;

	fMortarBumpDepth=0;
	fMortarBottomSlope = 0;
	fMortarTopSlope = 0;

	fMortarLenght=0;

	fFlippedTile=false;
	fSwitch=false;

	fRescaledLTileStart=0;
	fRescaledLTileEnd=0;
	fRescaledLEndPlateau=0;
	fRescaledLStartPlateau=0;

	fRescaledHTileStart=0;
	fRescaledHTileEnd=0;
	fRescaledHEndPlateau=0;
	fRescaledHStartPlateau=0;

	fType = 'Opt1';
	fAmplitude = 0;

	fSmooth = false;
	fRandomUVOrigin = false;
	fProportionnalTileShading = false;
	fNeedCenter = false;

	fTileCenterUV = TVector2::kZero;
}

void Engine3::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV, boolean& flip )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	fSwitch = false;

	switch( fType )
	{
	case 'Opt1':
		{
			newV = fromUV.y/fHalfSideLength;
			int32 tmpiV = floor(newV); // in the iVeme tile

			newU = (fromUV.x/fSideLength + tmpiV*.5);

			// we don't add directly the oscillation on the UV values because we don't want
			// to shake the shading of the tile
			real32 localNewV = newV + fAmplitude*sin(fTwoPeriodicity*(newU)+PI);
			real32 localNewU = newU + 0.5*fAmplitude*sin(fPeriodicity*(newV));

			iV = floor(localNewV); // in the iVeme tile				
			iU = floor(localNewU); // in the iUeme tile
			
			int32 decal=iV;

			if( !(iU&0x00000001) )
			{
				fFlippedTile = false;

				// Local coordinates
				newU -= iU;
				newV -= iV;

				if(newV>=1) // we're in another tile 50% shifted
				{
					if(newU<=.5)
					{
						fFlippedTile = true;
						SwitchValues(newU,newV);
						newU=0.5*(1-newU);
						newV=2*newV;
						iU-=1;
					}
					else if(newU>.5)
					{
						newU-=.5;
					}
				}
				else if(newV<0)
				{
					if(newU<=.5)
					{
						newU+=.5;
					}
					else if(newU>.5)
					{
						SwitchValues(newU,newV);
						newU=(1-.5*newU);
						newV=2*newV-1;
						iV+=1;
						iU+=1;
					}
				}

				// keep the domain at scale ( needed for the bump computation )
				newV*=.5;

				if( fNeedCenter )
				{	// we're going to need the uv coordinate of the center of the tile, compute it now
					fTileCenterUV.x = (iU+.5-.5*iV)*fSideLength;
					fTileCenterUV.y = (iV+.5)*fHalfSideLength;
				}
			}
			else
			{
				fFlippedTile = true;

				newV = fromUV.x/fHalfSideLength;
				int32 tmpiV = floor(newV); // in the iUeme tile

				newU = (fromUV.y/fSideLength + tmpiV*.5);

				newU = 200/fSideLength-newU;
				//newU = 250/fSideLength-newU;

				// we don't add directly the oscillation on the UV values because we don't want
				// to shake the shading of the tile
				real32 localNewU = newU + 0.5*fAmplitude*sin(fPeriodicity*(newV));
				real32 localNewV = newV + fAmplitude*sin(fTwoPeriodicity*(newU)+PI);

				int32 localiV = floor(localNewV);
				int32 localiU = floor(localNewU);

				// Local coordinates
				newU -= localiU;
				newV -= localiV;

				localNewU -= floor(localNewU);
				if(localNewU>.5)
					iV+=1;

				if(newV>=1) // we're in another tile 50% shifted
				{
					if(newU<=.5)
					{
						newU+=.5;
						iV+=1;
					}
					else if(newU>.5)
					{
						newU-=.5;
						iV-=1;
					}
				}
				else if(newV<0)
				{
					if(newU<=.5)
					{
						newU+=.5;
						iV+=1;
					}
					else if(newU>.5)
					{
						newU-=.5;
						iV-=1;
					}
				}
				
				if(	newU>=1 )
				{
					fFlippedTile = false;
					SwitchValues(newU,newV);
					newU=.5*newU;
					newV=2*(1-newV);
					iU+=1;
					iV-=1;
				}
				else if( newU<0 )
				{
					SwitchValues(newU,newV);
					newU=.5*newU+.5;
					newV=(1-2*newV);
					iU-=1;
				}

				// keep the domain at scale ( needed for the bump computation )
				newV*=.5;

				if(fNeedCenter)
				{	// we're going to need the uv coordinate of the center of the tile, compute it now
					fTileCenterUV.x = (iU+.5-.5*(iV))*fSideLength;
					fTileCenterUV.y = (iV+.5)*fHalfSideLength;
				}
			}

			if(fFlippedTile)
			{
				newV = .5-newV;
				newU = 1-newU;
			}
		} break;
	case 'Opt2':
		{
			newV = fromUV.y/fSideLength;
			int32 TwoTilesiV = floor(newV); // in the iVeme tile

			newU = fromUV.x/fSideLength;
			int32 TwoTilesiU = floor(newU); // in the iUeme tile

			iV = floor(newV); // in the iVeme tile				
			iU = floor(newU); // in the iUeme tile
			
			// Local coordinates
			newU -= iU;
			newV -= iV;

			if((TwoTilesiV+TwoTilesiU)&0x00000001)
			{	// Make 2 vertical tiles here
				fFlippedTile = true;

				SwitchValues(newU, newV);

				// Switch the V axis so the shading domain will look the same in all the tiles
				newV = 1-newV;

				boolean upper = false;
				if(newV>.5) // Upper tile
				{
					upper=true;
					newV-=.5;
				}
				else
					iU+=1;
				// we don't add directly the oscillation on the UV values because we don't want
				// to shake the shading of the tile
				real32 localNewU = newU + .5*fAmplitude*sin(fTwoPeriodicity*(newV));
				real32 localNewV = newV + .5*fAmplitude*sin(fTwoPeriodicity*(newU)+PI);

				if(localNewV>=.5) // we're in another tile
				{
					if(upper)
					{	// we're in another set of 2 tiles
						fFlippedTile=false;
						newV=1-newV;// flip the V axis
						SwitchValues(newU,newV);
						newU+=.5;
						if(newV>=.5)
						{
							newV-=.5;
							iV++;
						}
						iU--;
					}
					else
					{	// we're in the upper tile
						newV-=.5;
						iU--;
					}
				}
				else if(localNewV<0)
				{
					if(upper)
					{	// we re in the lower tile
						newV+=.5;
						iU++;
					}
					else
					{	// we're in another set of 2 tiles
						fFlippedTile=false;
						newV=1-newV;// flip the V axis
						SwitchValues(newU,newV);
						newU-=1;
						if(newV>=.5)
						{
							newV-=.5;
							iV++;
						}
					}
				}
				else if(localNewU>=1) // we're in another set of 2 tiles
				{
					fFlippedTile=false;
					newV=1-newV;// flip the V axis
					SwitchValues(newU,newV);
					newV-=1;
					if(upper)
						newU-=.5;
					else
						iU--;
					iV++;
				}
				else if(localNewU<0) // we're in another set of 2 tiles
				{
					fFlippedTile=false;
					newV=1-newV;// flip the V axis
					SwitchValues(newU,newV);
					newV+=.5;
					if(upper)
						newU-=.5;
					else
						iU--;
				}

				if(fNeedCenter)
				{	// we're going to need the uv coordinate of the center of the tile, compute it now
					real32 add = (upper?1.5:.5);
					fTileCenterUV.x = (iU-.5)*fSideLength + add*fHalfSideLength;
					fTileCenterUV.y = (iV+.5)*fSideLength;
				}
			}
			else
			{	// Make 2 horizontal tiles here
				fFlippedTile = false;

				boolean upper = false;
				if(newV>.5) // Upper tile
				{
					upper=true;
					newV-=.5;
					iV+=1;
				}

				// we don't add directly the oscillation on the UV values because we don't want
				// to shake the shading of the tile
				real32 localNewU = newU + .5*fAmplitude*sin(fTwoPeriodicity*(newV));
				real32 localNewV = newV + .5*fAmplitude*sin(fTwoPeriodicity*(newU)+PI);

				if(localNewV>=.5) // we're in another tile
				{
					if(upper)
					{	// we're in another set of 2 tiles
						fFlippedTile=true;
						SwitchValues(newU,newV);
						newU-=.5;
						if(newV>=.5)
						{
							newV-=.5;
							iU++;
						}
						newV=.5-newV;// flip the V axis
					}
					else
					{	// we're in the upper tile
						newV-=.5;
						iV++;
					}
				}
				else if(localNewV<0)
				{
					if(upper)
					{	// we re in the lower tile
						newV+=.5;
						iV--;
					}
					else
					{	// we're in another set of 2 tiles
						fFlippedTile=true;
						SwitchValues(newU,newV);
						newU+=1;
						if(newV>=.5)
						{
							newV-=.5;
							iU++;
						}
						newV=.5-newV;// flip the V axis
						iV--;
					}
				}
				else if(localNewU>=1) // we're in another set of 2 tiles
				{
					fFlippedTile=true;
					SwitchValues(newU,newV);
					newV-=1;
					newV=.5-newV;// flip the V axis
					if(upper)
					{
						newU+=.5;
						iV--;
					}
					iU++;
				}
				else if(localNewU<0) // we're in another set of 2 tiles
				{
					fFlippedTile=true;
					SwitchValues(newU,newV);
					newV+=.5;
					newV=.5-newV;// flip the V axis
					if(upper)
					{
						newU+=.5;
						iV--;
					}
				}

				if(fNeedCenter)
				{	// we're going to need the uv coordinate of the center of the tile, compute it now
					real32 add = (upper?.5:1.5);
					fTileCenterUV.x = (iU+.5)*fSideLength;
					fTileCenterUV.y = (iV-.5)*fSideLength + add*fHalfSideLength;
				}
			}
		} break;
	case 'Opt3':
		{
			newV = fromUV.y/fHalfSideLength;
			newU = (fromUV.x/fSideLength + floor(newV)*.5);

			// we don't add directly the oscillation on the UV values because we don't want
			// to shake the shading of the tile
			real32 localNewV = newV + fAmplitude*sin(fTwoPeriodicity*(newU)+PI);
			real32 localNewU = newU + 0.5*fAmplitude*sin(fPeriodicity*(newV));

			iV = floor(localNewV); // in the iVeme tile				
			iU = floor(localNewU); // in the iUeme tile

			fFlippedTile = false;

			// Local coordinates
			newU -= iU;
			newV -= iV;

			if(newV>=1) // we're in another tile 50% shifted
			{
				if(newU<=.5)
				{
					newU+=.5;
					iU-=1;
				}
				else if(newU>.5)
				{
					newU-=.5;
				}
			}
			else if(newV<0)
			{
				if(newU<=.5)
				{
					newU+=.5;
				}
				else if(newU>.5)
				{
					newU-=.5;
					iU+=1;
				}
			}

			// keep the domain at scale ( needed for the bump computation )
			newV*=.5;

			if(fNeedCenter)
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU+.5-.5*iV)*fSideLength;
				fTileCenterUV.y = (iV+.5)*fHalfSideLength;
			}
		} break;
	}

	flip = fFlippedTile;

	if(fNeedCenter)
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

real32 Engine3::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	// Add now the oscillation. We didn't add it before because we don't want to spoil the shading on the tile
	real32 localV = 0;
	real32 localU = 0;
	if(fFlippedTile)
	{
		{
			localV = uu + .5*fAmplitude*sin(fPeriodicity*(vv*2));
			localU = vv + .5*fAmplitude*sin(fTwoPeriodicity*(uu)+PI);
		}
	}
	else
	{
		{
			localU = uu + .5*fAmplitude*sin(fPeriodicity*(vv*2));
			localV = vv + .5*fAmplitude*sin(fTwoPeriodicity*(uu)+PI);
		}
	}

//	if(fSmooth)
	{
		// Smooth corners method: product of 2 pulses

		// Note: we use the fMortarDepth on only 1 pulse. When the 2 values are multiplied, 1*1*fMortarDepth = fMortarDepth

		if( fFlippedTile )
		{
			const real32 h = SmoothPulseWithTan(fRescaledHEndPlateau,fRescaledHTileStart,
							fRescaledHTileEnd,fRescaledHStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							1, localU);
				
			const real32 l = SmoothPulseWithTan(fRescaledLEndPlateau,fRescaledLTileStart,
							fRescaledLTileEnd,fRescaledLStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							1, localV);

			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
		else
		{
			const real32 h = SmoothPulseWithTan(fRescaledHEndPlateau,fRescaledHTileStart,
							fRescaledHTileEnd,fRescaledHStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							1, localV);
				
			const real32 l = SmoothPulseWithTan(fRescaledLEndPlateau,fRescaledLTileStart,
							fRescaledLTileEnd,fRescaledLStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							1, localU);

			if(fSmooth)	return fMortarBumpDepth*l*h;
			else		return fMortarBumpDepth*MC_Min(l,h);
		}
	}
/*	else
	{	
		if( fFlippedTile )
		{
			// Harsh corners method
			boolean hInfluence = false;
			if(localV<.5)
			{
				const real32 locV = localV - .25;
				if(locV<0)
				{
					const real32 locU = localU - .25;
					if(-locV>RealAbs(locU))
						hInfluence = true;
				}
			}
			else
			{
				const real32 locV = localV - .75;
				if(locV>0)
				{
					const real32 locU = localU - .25;
					if(locV>RealAbs(locU))
						hInfluence = true;
				}
			}

			if(hInfluence)
				return SmoothPulseWithTan(fRescaledLEndPlateau,fRescaledLTileStart,
							fRescaledLTileEnd,fRescaledLStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, localV);
			else
				return SmoothPulseWithTan(fRescaledHEndPlateau,fRescaledHTileStart,
							fRescaledHTileEnd,fRescaledHStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, localU);
		}
		else
		{
			// Harsh corners method
			boolean hInfluence = true;
			if(localU<.5)
			{
				const real32 locU = localU - .25;
				if(locU<0)
				{
					const real32 locV = localV - .25;
					if(-locU>RealAbs(locV))
						hInfluence = false;
				}
			}
			else
			{
				const real32 locU = localU - .75;
				if(locU>0)
				{
					const real32 locV = localV - .25;
					if(locU>RealAbs(locV))
						hInfluence = false;
				}
			}

			if(hInfluence)
				return SmoothPulseWithTan(fRescaledHEndPlateau,fRescaledHTileStart,
							fRescaledHTileEnd,fRescaledHStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, localV);
			else
				return SmoothPulseWithTan(fRescaledLEndPlateau,fRescaledLTileStart,
							fRescaledLTileEnd,fRescaledLStartPlateau,
							fMortarTopSlope,fMortarBottomSlope,
							fMortarBumpDepth, localU);
		}
	}*/
}

real32 Engine3::GetLocalTileU(const real32 uu, const int32 iU)
{
/* Try this maybe later for the bug with the wrong bump
	if(fFlippedTile)
	{
		const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
																// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
																// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
																// Now there's a precision issue: the U and V values are now quite big => use the
																// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
		return (decalV + vv);
	}
	else
	{
		const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
																// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
																// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
																// Now there's a precision issue: the U and V values are now quite big => use the
																// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
																// working with the derivatives (~= .00005).

		return (decalU + uu);
	}
*/
	const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	if(!fProportionnalTileShading && fFlippedTile)
	{
		return (decalU + 2*uu);
	}
	else
	{
		return (decalU + uu);
	}
}

real32 Engine3::GetLocalTileV(const real32 vv, const int32 iV)
{
/* Try this maybe later for the bug with the wrong bump
	if(fFlippedTile)
	{
		const int32 decalU=(fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
																// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
																// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
																// Now there's a precision issue: the U and V values are now quite big => use the
																// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
																// working with the derivatives (~= .00005).
		if(!fProportionnalTileShading)
		{
			return (decalU + 2*uu);
		}
		else
		{
			return (decalU + uu);
		}
	}
	else
	{
		const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
																// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
																// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
																// Now there's a precision issue: the U and V values are now quite big => use the
																// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
																// working with the derivatives (~= .00005).
		if(!fProportionnalTileShading)
		{
			return (decalV + 2*vv);
		}
		else
		{
			return (decalV + vv);
		}
	}
*/
	const int32 decalV=(fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	if(!fProportionnalTileShading && !fFlippedTile)
	{
		return (decalV + 2*vv);
	}
	else
	{
		return (decalV + vv);
	}
}

void Engine3::PreProcess(const PMapGridShader3& pmap)
{
	fSideLength = 100/(real32)(pmap.fTileCount);
	fHalfSideLength = fSideLength*.5;

	fMortarBumpDepth = pmap.fBumpDepth/(real32)(pmap.fTileCount);

	fMortarLenght = fSideLength*pmap.fSectionSize;

	// Scaled values
	const real32 lMortarSize = 0.25*pmap.fSectionSize;
	fRescaledLTileStart = lMortarSize; // rescaled on a [0,1] space
	fRescaledLTileEnd = 1-lMortarSize; // rescaled on a [0,1] space
	fRescaledLEndPlateau = pmap.fSectionPlateau*lMortarSize;// rescaled on a [0,1] space
	fRescaledLStartPlateau = 1-fRescaledLEndPlateau;// rescaled on a [0,1] space
	const real32 hMortarSize = .25*pmap.fSectionSize;
	fRescaledHTileStart = hMortarSize; // rescaled on a [0,.5] space
	fRescaledHTileEnd = .5-hMortarSize; // rescaled on a [0,.5] space
	fRescaledHEndPlateau = pmap.fSectionPlateau*hMortarSize;// rescaled on a [0,.5] space
	fRescaledHStartPlateau = .5-fRescaledHEndPlateau;// rescaled on a [0,.5] space

	switch(pmap.fPeriod)
	{
	case 'Opt1':
		{
			fPeriodicity=0;
			fTwoPeriodicity=0;
		}break;
	case 'Opt2':
		{
			fPeriodicity=(real32)(2*PI);
			fTwoPeriodicity=(real32)(4*PI);
		}break;
	case 'Opt3':
		{
			fPeriodicity=(real32)(4*PI);
			fTwoPeriodicity=(real32)(8*PI);
		}break;
	}

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
	fAmplitude = pmap.fAmplitude;

	fSmooth = pmap.fSmoothSection;
	fRandomUVOrigin = false;
	fProportionnalTileShading = false;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}

void Engine3::PreProcess(const PMapTilingShader3& pmap)
{
	fSideLength = 100/(real32)(pmap.fTileCount);
	fHalfSideLength = fSideLength*.5;

	fMortarBumpDepth = pmap.fMortarDepth/(real32)(pmap.fTileCount);

	fMortarLenght = fSideLength*pmap.fMortarSize;

	// Scaled values
	const real32 lMortarSize = 0.25*pmap.fMortarSize;
	fRescaledLTileStart = lMortarSize; // rescaled on a [0,1] space
	fRescaledLTileEnd = 1-lMortarSize; // rescaled on a [0,1] space
	fRescaledLEndPlateau = pmap.fMortarPlateau*lMortarSize;// rescaled on a [0,1] space
	fRescaledLStartPlateau = 1-fRescaledLEndPlateau;// rescaled on a [0,1] space
	const real32 hMortarSize = .25*pmap.fMortarSize;
	fRescaledHTileStart = hMortarSize; // rescaled on a [0,.5] space
	fRescaledHTileEnd = .5-hMortarSize; // rescaled on a [0,.5] space
	fRescaledHEndPlateau = pmap.fMortarPlateau*hMortarSize;// rescaled on a [0,.5] space
	fRescaledHStartPlateau = .5-fRescaledHEndPlateau;// rescaled on a [0,.5] space

	switch(pmap.fPeriod)
	{
	case 'Opt1':
		{
			fPeriodicity=0;
			fTwoPeriodicity=0;
		}break;
	case 'Opt2':
		{
			fPeriodicity=(real32)(2*PI);
			fTwoPeriodicity=(real32)(4*PI);
		}break;
	case 'Opt3':
		{
			fPeriodicity=(real32)(4*PI);
			fTwoPeriodicity=(real32)(8*PI);
		}break;
	}

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
	fAmplitude = pmap.fAmplitude;

	fSmooth = pmap.fSmoothMortar;
	fRandomUVOrigin = pmap.fRandomUVOrigin;
	fProportionnalTileShading = pmap.fProportionnalTileShading;
	fNeedCenter = (pmap.fShadersComponents[2]!=NULL);
}


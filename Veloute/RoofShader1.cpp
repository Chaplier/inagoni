/****************************************************************************************************

		RoofShader1.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/14/2004

****************************************************************************************************/
#include "RoofShader1.h"

#include "math.h"
#include "Utils.h"
#include "Veloute.h"
#include "MCRandom.h"

const MCGUID CLSID_RoofShader1(R_CLSID_RoofShader1);
const MCGUID CLSID_GlobalRoofShader1(R_CLSID_GlobalRoofShader1);

PMapRoofShader1::PMapRoofShader1()
{
	fSeed = 23459023; // SHADERS_PLUS

	fType='Opt1';
	fHorTileCount = 10;
	fVerTileCount = 10;
	fGap = .05f;
	fAmplitude = .5f;
	fPeriod = 1;
	fShift = 0;
	fSelfShadow = .15f;
	fProportion = .25f;
	fProportionnalTileShading=true;
	fRandomUVOrigin=true;

	fHorShadow=.05f;
	fVerShadow=.05f;
	fBottomSlope='Opt1';
	fTopSlope='Opt1';
	fShadowDepth=.1f;
	fSmoothShadow=true;

	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		fShadersComponents[iShader]=NULL;
	}
}

RoofShader1::RoofShader1()	// We just initialize the values
{
	fNeedSamplingRate = false;
	fPreprocessed = false;

	fShadowBottomSlope = 0;
	fShadowTopSlope = 0;

	fTileCenterUV = TVector2::kZero;
}
	
boolean RoofShader1::IsEqualTo(I3DExShader* aShader)		// Use it to compare two RoofShader1
{
	return (
		(fPMap.fType == ((RoofShader1*)aShader)->fPMap.fType) &&
		(fPMap.fHorTileCount == ((RoofShader1*)aShader)->fPMap.fHorTileCount) &&
		(fPMap.fVerTileCount == ((RoofShader1*)aShader)->fPMap.fVerTileCount) &&
		(fPMap.fGap == ((RoofShader1*)aShader)->fPMap.fGap) &&
		(fPMap.fAmplitude == ((RoofShader1*)aShader)->fPMap.fAmplitude) &&
		(fPMap.fPeriod == ((RoofShader1*)aShader)->fPMap.fPeriod) &&
		(fPMap.fShift == ((RoofShader1*)aShader)->fPMap.fShift) &&
		(fPMap.fSelfShadow == ((RoofShader1*)aShader)->fPMap.fSelfShadow) &&
		(fPMap.fProportion == ((RoofShader1*)aShader)->fPMap.fProportion) &&
		(fPMap.fProportionnalTileShading == ((RoofShader1*)aShader)->fPMap.fProportionnalTileShading) &&
		(fPMap.fRandomUVOrigin == ((RoofShader1*)aShader)->fPMap.fRandomUVOrigin) &&
		(fPMap.fHorShadow == ((RoofShader1*)aShader)->fPMap.fHorShadow) &&
		(fPMap.fVerShadow == ((RoofShader1*)aShader)->fPMap.fVerShadow) &&
		(fPMap.fShadowDepth == ((RoofShader1*)aShader)->fPMap.fShadowDepth) &&
		(fPMap.fBottomSlope == ((RoofShader1*)aShader)->fPMap.fBottomSlope) &&
		(fPMap.fTopSlope == ((RoofShader1*)aShader)->fPMap.fTopSlope) &&
		(fPMap.fSmoothShadow == ((RoofShader1*)aShader)->fPMap.fSmoothShadow) &&
		(fPMap.fShadersComponents[0] == ((RoofShader1*)aShader)->fPMap.fShadersComponents[0]) &&
		(fPMap.fShadersComponents[1] == ((RoofShader1*)aShader)->fPMap.fShadersComponents[1]) &&
		(fPMap.fShadersComponents[2] == ((RoofShader1*)aShader)->fPMap.fShadersComponents[2]) &&
		(fPMap.mTransform.IsEqualTo( &((RoofShader1*)aShader)->fPMap.mTransform ) ) );
}

MCCOMErr RoofShader1::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;		// We need UV coordinates
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput RoofShader1::GetImplementedOutput()
{
	return (EShaderOutput)(kUsesGetVector|kUsesGetColor|kUsesGetValue);
}

MCCOMErr RoofShader1::ExtensionDataChanged()
{
	fPreprocessed = false; 
	return MC_S_OK;
}


MCCOMErr RoofShader1::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	if (source->GetInstanceID() == 'SEED')
	{
		fPMap.fSeed = MCRandom();
		fPreprocessed=false;
		return MC_S_OK;
	}
	else
		return ShaderBase::HandleEvent( message, source, data);
}

inline boolean CheckUV( const real32 newU, const real32 newV, 
					   const real32 L, const real32 H, const real32 S )
{
	if( newU>=0 && newV>=0)
	{
		if( (newU<=L && newV<=S) || (newU<=S && newV<=H) )
		{
			return true;
		}
	}

	return false;
}

boolean OneTest(real32& newU, real32& newV, int32& iU, int32& iV, const real32 U, const real32 V,
					 const int32 a, const int32 b,
					 const real32 L, const real32 H, const real32 S)
{
	newU = U - (b)*S - (a)*L;
	newV = V - (b)*S + (a)*(H-S);
	if(CheckUV(newU,newV,L,H,S))
	{
		iU = b;
		iV = a;
		return true;
	}
	return false;
}

boolean checkKvalues( real32& newU, real32& newV, int32& iU, int32& iV, const real32 U, const real32 V,
					 const int32 k1, const int32 k2,
					 const real32 L, const real32 H, const real32 S )
{
	for(int32 i=-1;i<2;i++)
	{
		for(int32 j=-1;j<2;j++)
		{
			if(OneTest(newU,newV,iU,iV, U,V, k1+i,k2+j,L,H,S) )
				return true;
		}
	}

	return false;
}

boolean GetUVValues( real32& newU, real32& newV, int32& iU, int32& iV, const real32 U, const real32 V,
					 const int32 k1, const int32 k2, const int32 l1, const int32 l2,
					 const real32 L, const real32 H, const real32 S )
{
	const int32 max = 50; // could use nbTiles to set this
	for(int32 add=0;add<max;add++)
	{
		if( OneTest(newU,newV,iU,iV, U,V, k1+add,k2+add,L,H,S) )
			return true;
		if( OneTest(newU,newV,iU,iV, U,V, l1+add,l2+add,L,H,S) )
			return true;
		if(add!=0)
		{
			if( OneTest(newU,newV,iU,iV, U,V, k1-add,k2-add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, k1-add,k2+add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, k1+add,k2-add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1-add,l2-add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1-add,l2+add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1+add,l2-add,L,H,S) )
				return true;
		}
		for(int32 i=0 ; i<add ; i++)
		{
			// With k1,k2
			if( OneTest(newU,newV,iU,iV, U,V, k1+i,k2+add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, k1+add,k2+i,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, k1+i,k2-add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, k1-add,k2+i,L,H,S) )
				return true;
			// With l1,l2
			if( OneTest(newU,newV,iU,iV, U,V, l1+i,l2+add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1+add,l2+i,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1+i,l2-add,L,H,S) )
				return true;
			if( OneTest(newU,newV,iU,iV, U,V, l1-add,l2+i,L,H,S) )
				return true;
			if(i!=0)
			{
				// With k1,k2
				if( OneTest(newU,newV,iU,iV, U,V, k1-i,k2+add,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, k1+add,k2-i,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, k1-i,k2-add,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, k1-add,k2-i,L,H,S) )
					return true;
				// With l1,l2
				if( OneTest(newU,newV,iU,iV, U,V, l1-i,l2+add,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, l1+add,l2-i,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, l1-i,l2-add,L,H,S) )
					return true;
				if( OneTest(newU,newV,iU,iV, U,V, l1-add,l2-i,L,H,S) )
					return true;
			}
		}
	}

	return false;
}

void RoofShader1::GetLocalUVCoordinates(TVector2 fromUV, real32& newU, real32& newV, int32& iU, int32& iV )
{
	// translate and rotate
	fromUV = f2DTransform*fromUV;
	// Scale the point
	fromUV.x /= f2DTransform[2][0];
	fromUV.y /= f2DTransform[2][1];

	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			newV = fromUV.y/fWidth;
			iV = floor(newV);

			newU = fromUV.x/fLength + iV*.5;
			iU = floor(newU);

			newU -= iU;
			newV -= iV;

			// See if we're not in another tile
			if(newU<fActualGap)
			{
				newV += 1;
				newU += .5;

				iV--;
				iU--;
			}
			else if(newU>1-fActualGap)
			{
				newV += 1;
				newU -= .5;

				iV--;
			}

			newV *= fWidthOnLength;

			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU-iV*.5+.5)*fLength;
				fTileCenterUV.y = (iV+.5)*fWidth;
			}
		} break;
	case 'Opt2':
		{
			if(fSmallSize<=kRealEpsilon)
			{ // case where the tiles are all over each others
				newU = 0;
				newV = 0;
				fTileCenterUV.x = 0;
				fTileCenterUV.y = 0;
				return;
			}

			// 45deg rotation
			const real32 U = HALF_SQRT2*(fromUV.y-fromUV.x);
			const real32 V = HALF_SQRT2*(fromUV.x+fromUV.y);

			const real32 S = fSmallSize;
			const real32 H = fWidth;
			const real32 L = fLength;
			const real32 fUL = floor(U/L);
			const real32 fVS = floor(V/S);
			const real32 fUS = floor(U/S);
			const real32 fVH = floor(V/H);

			// First pair of approximation
			const int32 k1 = floor( S/(H+L-S) * (L/S * fUL - fVS) );
			const int32 k2 = floor( L/(H+L-S) * ((H-S)/S * fUL + fVS) );
			// Second pair of approximation
			const int32 l1 = H/(H+L-S) * (S/H * fUS - fVH);
			const int32 l2 = H/(H+L-S) * ((H-S)/H * fUS + L/S * fVH);

			if(!GetUVValues(newU,newV,iU,iV, U,V, k1,k2,l1,l2,L,H,S))
			{// Shouldn't occured
			}

			newU/= fLength;
			newV/= fWidth;

			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				// Apply the inverse equation
				real32 x = (fLength*.5+iU*S+iV*L);
				real32 y = (fWidth*.5+iU*S-iV*(H-S));
				// Then rotate and flip
				fTileCenterUV.x = 100-(x-y)/SQRT2;
				fTileCenterUV.y = (x+y)/SQRT2;
			}
		} break;
	case 'Opt3':
		{
			newV = fromUV.y/fWidth;
			iV = floor(newV);
			newV -= iV;

			newU = fromUV.x/fLength + iV*fRealShifting;
			iU = floor(newU);
			newU -= iU;

			// Add the oscillation to the tile
			newV -= fActualAmplitude*sin(fPMap.fPeriod*PI*newU);
			if(newV<0)
			{
				newV+=1;
				iV--;
				if(newU<fRealShifting)
					iU--;

				if(newU>fRealShifting)
				{
					newU -= fRealShifting;
				}
				else if(newU<1-fRealShifting)
				{
					newU += 1-fRealShifting;
				}
				else if(newU<fRealShifting)
				{
					newU += 1-fRealShifting;
				}
			}
			if(newV>1)
			{
				newV-=1;
				iV++;
				if(newU>1-fRealShifting)
					iU++;

				if(newU>1-fRealShifting)
				{
					newU -= 1-fRealShifting;
				}
				else if(newU<fRealShifting)
				{
					newU += fRealShifting;
				}
				else if(newU>fRealShifting)
				{
					newU += fRealShifting;
				}
			}

			// Keep V between 0 and 1
			newU *= fWidthOnLength;

			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU-iV*fRealShifting+.5)*fLength;
				fTileCenterUV.y = (iV+.5)*fWidth;
			}
		} break;
	case 'Opt4':
		{
			newV = fromUV.y/fWidth;
			iV = floor(newV);
			newV -= iV;

			newU = fromUV.x/fLength;
			iU = floor(newU);
			newU -= iU;

			// Add the oscillation to the tile
			if(newU<fPMap.fProportion)
			{
				newV -= fActualAmplitude*sin(PI*newU/fPMap.fProportion);
				if(newV<0)
				{
					newV+=1;
					iV--;
				}
				if(newV>1)
				{
					newV-=1;
					iV++;
				}
			}

			// Keep V between 0 and 1
			newU *= fWidthOnLength;

			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU+.5)*fLength;
				fTileCenterUV.y = (iV+.5)*fWidth;
			}
		} break;
	case 'Opt5':
		{
			newV = fromUV.y/fWidth;
			iV = floor(newV);
			newV -= iV;

			newU = fromUV.x/fLength;
			iU = floor(newU);
			newU -= iU;

			// Add the deformation to the tile
			if(newU<fPMap.fProportion)
			{
				newV -= fPMap.fAmplitude*(RealSqrt(fR1*fR1 - (newU-fAu)*(newU-fAu)) + fAv);
			}
			else
			{
				newV += fPMap.fAmplitude*(RealSqrt(fR2*fR2 - (newU-fBu)*(newU-fBu)) - fBv);
			}

			if(newV<0)
			{
				newV+=1;
				iV--;
			}
			else if(newV>1)
			{
				newV-=1;
				iV++;
			}

			// Keep V between 0 and 1
			newU *= fWidthOnLength;

			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.x = (iU+.5)*fLength;
				fTileCenterUV.y = (iV+.5)*fWidth;
			}
		} break;

	case 'Opt6':
		{
			// First find in which set of 2 tiles ( with a random separation between ) the point is
			newV = fromUV.y/fWidth;
			iV = floor(newV); // in the iVeme tile
			const real32 shifting = iV*iV*.37;
			newU = fromUV.x/fLength + shifting; // add a shifting based on iV and weird % to create the random effect in iV
			iU = floor(newU); // in the iUeme tile

			// Local coordinates
			newU -= iU;
			newV -= iV;

			// use iU and iV to decide where the separation should appears
			const TVector2 fakePoint(iU*.432, iV*.371);
			// recompute cached data
			const real32 noiseValue = .5 + .5*fNoise.GetValueLinear2D(fakePoint);
			fLength1OnTotalLength = noiseValue;
			fLength1 = fLength1OnTotalLength * fLength;
			fLength2 = fLength - fLength1;
			fHalfLength1 = fLength1*.5;
			fLength1PlusHalfLength2 = fLength1 + 0.5*fLength2;

			// the tile height determine the space (height = 1, length = X)
			if( newU<fLength1OnTotalLength )
			{
				fFirstTile = true;
				iU = 2*iU;
				newU /= fWidthOnLength;
			}
			else
			{
				fFirstTile = false;
				iU = 2*iU + 1;
				newU = (newU-fLength1OnTotalLength)/fWidthOnLength;
			}


			if( fShaders.fShaders[2])
			{	// we're going to need the uv coordinate of the center of the tile, compute it now
				fTileCenterUV.y=(iV+0.5)*fWidth;
				if(fFirstTile)
					fTileCenterUV.x=(iU/2 - shifting)*fLength + fHalfLength1;
				else
					fTileCenterUV.x=(iU/2 - shifting)*fLength + fLength1PlusHalfLength2;
			}
		} break;

	}

	if( fShaders.fShaders[2])
	{
		// apply the inverse transform
		fTileCenterUV.x *= f2DTransform[2][0];
		fTileCenterUV.y *= f2DTransform[2][1];

		real32 x = fTileCenterUV[0]*f2DTransform[0][0] + fTileCenterUV[1]*f2DTransform[1][0];
		real32 y = fTileCenterUV[0]*f2DTransform[0][1] + fTileCenterUV[1]*f2DTransform[1][1];

		fTileCenterUV.x = x+f2DTransform[0][2];
		fTileCenterUV.y = y+f2DTransform[1][2];
	}
}

#if (VERSIONNUMBER >= 0x040000)
real RoofShader1::GetColor(TMCColorRGBA& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#else
real RoofShader1::GetColor(TMCColorRGB& result,boolean &fullAreaDone,ShadingIn& shadingIn)
#endif
{
	if (IsLocked())
	{
#if (VERSIONNUMBER >= 0x040000)
		result.Set(0.0f, 0.0f, 0.0f, 0.0f);
#else
		result.Set(0.0f, 0.0f, 0.0f);
#endif
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a color depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarColor(result, fullAreaDone, shadingIn);
	}
	else if(value>=fShadowDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileColor(result, fullAreaDone, tileShading,fTileCenterUV);
	}
	else
	{	// we're between the 2: average
#if (VERSIONNUMBER >= 0x040000)
		TMCColorRGBA color1;
		TMCColorRGBA color2;
#else
		TMCColorRGB color1;
		TMCColorRGB color2;
#endif
		fShaders.GetMortarColor(color1, fullAreaDone, shadingIn);

		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileColor(color2, fullAreaDone, tileShading,fTileCenterUV);

		const real32 coeff = value/fShadowDepth;
		result.Interpolate(color1,color2,coeff);
	}

	return 1.0f;
}

real RoofShader1::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result = 0.0f;
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV );

//	real32 value = 0; ComputeOneSample(uu, vv);

	real32 value = GetSample(uu,vv,shadingIn);

	// Now get a value depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarValue(result, fullArea, shadingIn);
	}
	else if(value>=fShadowDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileValue(result, fullArea, shadingIn, 1/*fPMap.fShadowDepth*/);
	}
	else
	{	// we're between the 2: average
		real32 value1;
		fShaders.GetMortarValue(value1, fullArea, shadingIn);

		real32 value2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileValue(value2, fullArea, tileShading, 1/*fPMap.fShadowDepth*/);

		const real32 coeff = value/fShadowDepth;
		result = (1-coeff)*value1 + coeff*value2;
	}

	return 1.0f;
}

real RoofShader1::GetVector(TVector3& result, ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.SetValues(0.0f, 0.0f, 0.0f);
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV);

	// Build a self shadow independant from the tile shadow
	fSelfShadow = fPMap.fSelfShadow/fPMap.fShadowDepth;

	// Now determine if the point is in a mortar area or a tile area
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
	
	// Now get a vector depending on the result
	const real32 value = (value1+value2+value3+value4)*.25;

	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.GetMortarVector(result, shadingIn);
	}
	else if(value>=fShadowDepth && fSelfShadow<fShadowDepth)
	{	// we're in the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileVector(result, tileShading);
	}
	else
	{	// we're between the 2: average
		// The perturbation due to the transition between tile and mortar
		GetPerturbationVector(result, shadingIn, value1, value2, value3, value4);

		// add the perturbation contained in the shaders
		TVector3 vector1;
		fShaders.GetMortarVector(vector1, shadingIn);

		TVector3 vector2;
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.GetTileVector(vector2, tileShading);

		const real32 coeff = value/fShadowDepth;
		result += (1-coeff)*vector1 + coeff*vector2;
	}

	return 1.0f;
}

real32 RoofShader1::ComputeOneSample(const real32 uu, const real32 vv, const real32 samplingRate)
{
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			if(fPMap.fSmoothShadow)
			{
				const real32 l = SmoothPulseWithTan(fActualGap,fActualGap + fPMap.fHorShadow,
								1-(fActualGap + fPMap.fHorShadow),1-fActualGap,
								fShadowTopSlope,fShadowBottomSlope,
								1, uu);
				
				const real32 h = SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );

				return fShadowDepth*l*h;
			}
			else
			{	
				// Harsh corners method
				if(vv<(fSlope*(uu-fActualGap)) && vv<( -fSlope*(uu+(fActualGap-1))))
				{
					return fShadowDepth*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
				else
				{
					return fShadowDepth*SmoothPulseWithTan(fActualGap,fActualGap + fPMap.fHorShadow,
								1-(fActualGap + fPMap.fHorShadow),1-fActualGap,
								fShadowTopSlope,fShadowBottomSlope,
								1, uu);
				}
			}
		} break;
	case 'Opt2':
		{
			if(fPMap.fSmoothShadow)
			{
				const real32 l = SmoothStepWithTan(	0, fPMap.fHorShadow,
								fShadowTopSlope,fShadowBottomSlope,uu );
				
				const real32 h = SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );

				return fShadowDepth*l*h;
			}
			else
			{	
				// Harsh corners method
				if( vv<(fSlope*uu) )
				{
					return fShadowDepth*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
				else
				{
					return fShadowDepth*SmoothStepWithTan(	0, fPMap.fHorShadow,
								fShadowTopSlope,fShadowBottomSlope,uu );
				}
			}
		} break;
	case 'Opt3':
		{
			if(fPMap.fSmoothShadow)
			{
				// Self shadow
				real32 l2=1-fSelfShadow;
				if(fActualAmplitude>=0)
				{
					for(int32 i= 0 ; i<fPMap.fPeriod ; i++)
					{
						l2 += RealAbs(
							SmoothPulseWithTan(i*(fUFrequency),(i+.5)*(fUFrequency),
										(i+.5)*(fUFrequency),(i+1)*(fUFrequency),
										0,0,
										fSelfShadow, uu) );
					}
				}
				else // inverse the self shadow when the tile is hollow
				{
					l2 = 1;
					for(int32 i= 0 ; i<fPMap.fPeriod ; i++)
					{
						l2 -= RealAbs(
								SmoothPulseWithTan(i*(fUFrequency),(i+.5)*(fUFrequency),
											(i+.5)*(fUFrequency),(i+1)*(fUFrequency),
											0,0,
											fSelfShadow, uu) );
					}
				}

				const real32 l1 = SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
										
				const real32 h = SmoothStepWithTan(	0, fPMap.fVerShadow,
									fShadowTopSlope,fShadowBottomSlope,vv );

				return fShadowDepth*h*l1*l2;
			}
			else
			{	
				// Harsh corners method

				// Self shadow
				real32 l2=1-fSelfShadow;
				if(fActualAmplitude>=0)
				{
					for(int32 i= 0 ; i<fPMap.fPeriod ; i++)
					{
						l2 += RealAbs(
							SmoothPulseWithTan(i*(fUFrequency),(i+.5)*(fUFrequency),
										(i+.5)*(fUFrequency),(i+1)*(fUFrequency),
										0,0,
										fSelfShadow, uu) );
					}
				}
				else // inverse the self shadow when the tile is hollow
				{
					l2 = 1;
					for(int32 i= 0 ; i<fPMap.fPeriod ; i++)
					{
						l2 -= RealAbs(
								SmoothPulseWithTan(i*(fUFrequency),(i+.5)*(fUFrequency),
											(i+.5)*(fUFrequency),(i+1)*(fUFrequency),
											0,0,
											fSelfShadow, uu) );
					}
				}

				if(vv<(fSlope*(uu-0)) && vv<( -fSlope*(uu+(0-fWidthOnLength))))
				{
					return fShadowDepth*l2*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
				else
				{
					return fShadowDepth*l2*SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
					
				}
			}
		} break;
	case 'Opt4':
		{
			if(fPMap.fSmoothShadow)
			{
				// Self shadow
				real32 l2=1;
				if(uu<fScaledProportion)
				{
					if(fActualAmplitude>=0)
					{
						l2-=fSelfShadow;
						l2+=SmoothPulseWithTan(	0,.5*fScaledProportion,
												.5*fScaledProportion,fScaledProportion,
												0,0,
												fSelfShadow, uu );
					}
					else // inverse the self shadow when the tile is hollow
					{
						l2-=SmoothPulseWithTan(	0,.5*fScaledProportion,
												.5*fScaledProportion,fScaledProportion,
												0,0,
												fSelfShadow, uu );
					}
				}

				const real32 l1 = SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
										
				const real32 h = SmoothStepWithTan(	0, fPMap.fVerShadow,
									fShadowTopSlope,fShadowBottomSlope,vv );

				return fShadowDepth*h*l1*l2;
			}
			else
			{	
				// Harsh corners method

				// Self shadow
				real32 l2=1;
				if(uu<fScaledProportion)
				{
					if(fActualAmplitude>=0)
					{
						l2-=fSelfShadow;
						l2+=SmoothPulseWithTan(	0,.5*fScaledProportion,
												.5*fScaledProportion,fScaledProportion,
												0,0,
												fSelfShadow, uu );
					}
					else // inverse the self shadow when the tile is hollow
					{
						l2-=SmoothPulseWithTan(	0,.5*fScaledProportion,
												.5*fScaledProportion,fScaledProportion,
												0,0,
												fSelfShadow, uu );
					}
				}

				if(vv<(fSlope*(uu-0)) && vv<( -fSlope*(uu+(0-fWidthOnLength))))
				{
					return fShadowDepth*l2*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
				else
				{
					return fShadowDepth*l2*SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
					
				}
			}
		} break;
	case 'Opt5':
		{
			if(fPMap.fSmoothShadow)
			{
				// Self shadow
				real32 l2=1;
				if(fActualAmplitude>=0)
				{
					l2=1-fSelfShadow*MC_Max(fMax,-fMin)/(fMax-fMin);
					if(uu<fScaledProportion)
					{
						l2+=SmoothPulseWithTan(	0,fAu*fWidthOnLength,
												fAu*fWidthOnLength,fScaledProportion,
												0,0,
												fSelfShadow*fMax/(fMax-fMin), uu );
					}
					else
					{
						l2+=SmoothPulseWithTan(	fScaledProportion,fBu*fWidthOnLength,
												fBu*fWidthOnLength,1,
												0,0,
												-fSelfShadow*fMin/(fMax-fMin), uu );
					}
				}
				else // inverse the self shadow when the tile is hollow
				{
					if(uu<fScaledProportion)
					{
						l2-=SmoothPulseWithTan(	0,fAu*fWidthOnLength,
												fAu*fWidthOnLength,fScaledProportion,
												0,0,
												fSelfShadow*fMax/(fMax-fMin), uu );
					}
					else
					{
						l2-=SmoothPulseWithTan(	fScaledProportion,fBu*fWidthOnLength,
												fBu*fWidthOnLength,1,
												0,0,
												-fSelfShadow*fMin/(fMax-fMin), uu );
					}
				}

				const real32 l1 = SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
										
				const real32 h = SmoothStepWithTan(	0, fPMap.fVerShadow,
									fShadowTopSlope,fShadowBottomSlope,vv );

				return fShadowDepth*h*l1*l2;
			}
			else
			{	
				// Harsh corners method

				// Self shadow
				real32 l2=1;
				if(fActualAmplitude>=0)
				{
					l2=1-fSelfShadow*MC_Max(fMax,-fMin)/(fMax-fMin);
					if(uu<fScaledProportion)
					{
						l2+=SmoothPulseWithTan(	0,fAu*fWidthOnLength,
												fAu*fWidthOnLength,fScaledProportion,
												0,0,
												fSelfShadow*fMax/(fMax-fMin), uu );
					}
					else
					{
						l2+=SmoothPulseWithTan(	fScaledProportion,fBu*fWidthOnLength,
												fBu*fWidthOnLength,1,
												0,0,
												-fSelfShadow*fMin/(fMax-fMin), uu );
					}
				}
				else // inverse the self shadow when the tile is hollow
				{
					if(uu<fScaledProportion)
					{
						l2-=SmoothPulseWithTan(	0,fAu*fWidthOnLength,
												fAu*fWidthOnLength,fScaledProportion,
												0,0,
												fSelfShadow*fMax/(fMax-fMin), uu );
					}
					else
					{
						l2-=SmoothPulseWithTan(	fScaledProportion,fBu*fWidthOnLength,
												fBu*fWidthOnLength,1,
												0,0,
												-fSelfShadow*fMin/(fMax-fMin), uu );
					}
				}

				if(vv<(fSlope*(uu-0)) && vv<( -fSlope*(uu+(0-fWidthOnLength))))
				{
					return fShadowDepth*l2*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
				else
				{
					return fShadowDepth*l2*SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									fWidthOnLength-(0 + fPMap.fHorShadow*fWidthOnLength),fWidthOnLength-0,
									fShadowTopSlope,fShadowBottomSlope,
									1, uu);
					
				}
			}
		} break;
	case 'Opt6':
		{
			if(fPMap.fSmoothShadow)
			{	// Smooth corners method
				real32 lSize = 0;
				if(fFirstTile)
				{
					lSize = fLength1/fWidth;
				}
				else
				{
					lSize = fLength2/fWidth;
				}
				const real32 h = SmoothPulseWithTan(0,0 + fPMap.fHorShadow/fWidthOnLength,
								lSize-(0 + fPMap.fHorShadow/fWidthOnLength),lSize-0,
								fShadowTopSlope,fShadowBottomSlope,
								1, uu);
				
				const real32 l = SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );

				return h*l*fShadowDepth;
			}
			else
			{	// harsh corners method

				// Center the tile
				real32 remapU = uu;
				real32 remapV = vv-.5;

				real32 lSize = 0;
				if(fFirstTile)
				{
					lSize = fLength1/fWidth;
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
					lSize = fLength2/fWidth;
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
					return SmoothPulseWithTan(0,0 + fPMap.fHorShadow*fWidthOnLength,
									lSize-(0 + fPMap.fHorShadow*fWidthOnLength),lSize-0,
									fShadowTopSlope,fShadowBottomSlope,
									fShadowDepth, uu);
				}
				else
				{
					return fShadowDepth*SmoothStepWithTan(	0, fPMap.fVerShadow,
								fShadowTopSlope,fShadowBottomSlope,vv );
				}
			}
		} break;
	}


	return 0;
}

inline real32 RoofShader1::GetLocalTileU(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalU=(fPMap.fRandomUVOrigin?12*(iU%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			return (decalU + uu);
		}
	case 'Opt2':
		{
			if(fPMap.fProportionnalTileShading)
				return (decalU + uu);
			else // This is not what we're usually doing but it gives use a way to have a shading in the vertical way
				return decalU-.5+.5*(- uu + vv*fWidthOnLength);
		}
	case 'Opt3':
	case 'Opt4':
	case 'Opt5':
	case 'Opt6':
		{
			if(fPMap.fProportionnalTileShading)
				return (decalU + uu/(fWidthOnLength*fWidthOnLength));
			else
				return (decalU + uu/fWidthOnLength);
		}
	}

	return uu;
}

inline real32 RoofShader1::GetLocalTileV(const real32 uu, const int32 iU, const real32 vv, const int32 iV)
{
	const int32 decalV=(fPMap.fRandomUVOrigin?12*(iV%9):0); // multiply by something so if a noise is used, it won't be the same on all the tiles
															// 12 is a nice value: it can handle some scalings on regular patterns (like gradient)
															// with usual values (50%, 33%, 25%,...) cause we can devide by 2,3,4,6 and 12
															// Now there's a precision issue: the U and V values are now quite big => use the
															// %9 to limit them ( we'll still have 9*9 different tile shading ). The issue was when
															// working with the derivatives (~= .00005).
	switch(fPMap.fType)
	{
	case 'Opt1':
		{
			if(fPMap.fProportionnalTileShading)
				return (decalV + vv);
			else
				return (decalV + .5*vv/fWidthOnLength);
		}
	case 'Opt2':
		{
			if(fPMap.fProportionnalTileShading)
				return (decalV + vv*fWidthOnLength);
			else // This is not what we're usually doing but it gives use a way to have a shading in the vertical way
				return decalV+.5*( uu + vv*fWidthOnLength );
		}
	case 'Opt3':
	case 'Opt4':
	case 'Opt5':
	case 'Opt6':
		{
			return (decalV + vv);
		}
	}

	return vv;
}

void RoofShader1::PreProcess()
{
	fPMap.mTransform.Get2DTransform( f2DTransform );


	// computed later
	fLength1 = 1; 
	fLength2 = 1;
	fHalfLength1 = .5;
	fLength1PlusHalfLength2 = 1.5;
	fLength1OnTotalLength = 1;


	fLength = 100/(real32)(fPMap.fHorTileCount);
	fWidth = 100/(real32)(fPMap.fVerTileCount);

	fShadowDepth = fPMap.fShadowDepth/(real32)(fPMap.fVerTileCount);
	fSelfShadow = fPMap.fSelfShadow;

	fWidthOnLength = fWidth/fLength;

	fActualGap = fPMap.fGap/4; // We don't want the gap to be too big for a tile not being smaller than its gap
	fSlope = fPMap.fVerShadow / fPMap.fHorShadow;

	fSmallSize = (1-fPMap.fGap)*MC_Min(fWidth,fLength);
	
	fUFrequency = fWidthOnLength/(real32)(fPMap.fPeriod);
	if(fPMap.fPeriod)
	{	// General case
		if(fPMap.fPeriod&0x00000001)
			fRealShifting = 0;
		else
			fRealShifting = (real32)MC_Min(fPMap.fShift,(fPMap.fPeriod+1)/2)*2/(real32)(fPMap.fPeriod);
	}
	else
	{	// Special case without oscillation: use it as a percent
		fRealShifting = fPMap.fShift/(real32)100;
	}
	if(fRealShifting>=1)
		fRealShifting-=floor(fRealShifting);

	fScaledProportion = fPMap.fProportion*fWidthOnLength;

	//
	fAu=fPMap.fProportion*.5;
	fAv=-fPMap.fProportion*.5;
	fBu=(1+fPMap.fProportion)*.5;
	fBv=(1-fPMap.fProportion)*.5;
	fR1=fPMap.fProportion*HALF_SQRT2;
	fR2=(1-fPMap.fProportion)*HALF_SQRT2;
	fMax=fPMap.fProportion*.5*(SQRT2-1);
	fMin=(1-fPMap.fProportion)*.5*(1-SQRT2);

	// 
	fActualAmplitude = fPMap.fAmplitude/5;

	// SuperSampling quality ( empiric values found after several tests )
	SetSuperSamplingQuality( f2DTransform, 100, 100); //(fLength), (fWidth) );

	// Get the slopes values
	switch(fPMap.fBottomSlope)
	{
	case 'Opt1': fShadowBottomSlope = 0; break;
	case 'Opt2': fShadowBottomSlope = 1; break;
	case 'Opt3': fShadowBottomSlope = 2; break;
	}
	switch(fPMap.fTopSlope)
	{
	case 'Opt1': fShadowTopSlope = 0; break;
	case 'Opt2': fShadowTopSlope = 1; break;
	case 'Opt3': fShadowTopSlope = 2; break;
	}

	// Get the sub shaders interfaces
	for( int32 iShader=0; iShader<3 ; iShader++ )
	{
		if(fPMap.fShadersComponents[iShader] )
		{
			fPMap.fShadersComponents[iShader]->QueryInterface(IID_I3DShShader, (void**)&(fShaders.fShaders[iShader]));
		}
		else
			fShaders.fShaders[iShader]=NULL;
	}
	fPreprocessed=true;
}

///////////////////////////////////////////////////////////////////////////////////////////
EShaderOutput GlobalRoofShader1::GetImplementedOutput()
{
	return kUsesDoShade;
}

MCCOMErr GlobalRoofShader1::DoShade(ShadingOut& result,ShadingIn& shadingIn)
{
	if (IsLocked())
	{
		result.Clear();
		return 1.0f;
	}

	if(!fPreprocessed)
		PreProcess();

	real32 uu=0, vv=0;
	int32 iU=0, iV=0;
	GetLocalUVCoordinates(shadingIn.fUV, uu, vv, iU, iV);

//	real32 value = ComputeOneSample(uu, vv);
	real32 value1=0,value2=0,value3=0,value4=0;
	SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
	real32 value = (value1+value2+value3+value4)*.25;
//	value *= .5;

	// Get the shading depending on the value
	if(value == 0)
	{	// we're at the bottom of the mortar
		fShaders.DoMortarShade(result, shadingIn);
	}
	else if(value>=fShadowDepth && !fSelfShadow) // 
	{	// we're in the tile: define a UV space local to the tile
		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.DoTileShade(result, tileShading,fTileCenterUV);
		shadingIn.fCurrentCompletionMask = tileShading.fCurrentCompletionMask;
	}
	else
	{	// we're between the 2: average
		ShadingOut out1 = result;		
		fShaders.DoMortarShade(out1, shadingIn);

		ShadingIn tileShading = shadingIn;
		tileShading.fUV.x = GetLocalTileU(uu, iU, vv, iV);
		tileShading.fUV.y = GetLocalTileV(uu, iU, vv, iV);
		fShaders.DoTileShade(result, tileShading,fTileCenterUV);

		const real32 coeff = 1-value/fShadowDepth;

		// Color
		BlendColor(result.fColor, out1.fColor, coeff);
		// Specular color
		BlendColor(result.fSpecularColor, out1.fSpecularColor, coeff);
		// Specular size
		BlendValue(result.fSpecularSize, out1.fSpecularSize, coeff);
		// Reflexion
		BlendColor(result.fReflection.fReflection, out1.fReflection.fReflection, coeff);
		// Transparency
		BlendColor(result.fTransparency.fIntensity, out1.fTransparency.fIntensity, coeff);
		// Refraction
		BlendValue(result.fRefractiveIndex, out1.fRefractiveIndex, coeff);
		// Glow
		BlendColor(result.fGlow, out1.fGlow, coeff);
		// ChangedNormal 
		if( fSelfShadow )
		{	// We need to resample with the proper self shadow value
			// This is not very efficient but I couldn't figure out a better way to do it
			fSelfShadow = fPMap.fSelfShadow/fPMap.fShadowDepth;

		//	value = ComputeOneSample(uu, vv);
			SuperSampling(uu, vv, shadingIn, value1, value2, value3, value4);
			value = (value1+value2+value3+value4)*.25;
		//	value *= .5;

			// Restaure the value used for GetColor and GetValue
			fSelfShadow = fPMap.fSelfShadow;
		}
		TVector3 bumpPerturbation;
		GetPerturbationVector(bumpPerturbation, shadingIn, value1, value2, value3, value4);
		result.fChangedNormalLoc = bumpPerturbation +
							(1-coeff)*result.fChangedNormalLoc + coeff*out1.fChangedNormalLoc;
		// Opacity
#if (VERSIONNUMBER >= 0x040000)
		BlendValue(result.fLayerMask , out1.fLayerMask , coeff);
#else
		BlendValue(result.fOpacity, out1.fOpacity, coeff);
#endif
	}

	return MC_S_OK;
}
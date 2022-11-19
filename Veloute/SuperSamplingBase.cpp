/****************************************************************************************************

		SuperSamplingBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	2/12/2004

****************************************************************************************************/

#include "SuperSamplingBase.h"

#include "Utils.h"

TSuperSamplingBase::TSuperSamplingBase()
{
	fNeedSamplingRate = true;

	fSuperSamplingU=1;
	fSuperSamplingV=1;
	fSuperSamplingW=1;

	fPrivate2DTransform = TMatrix33::kIdentity;
	fPrivate2DTransform[2][0] = .01f; // Default value <=> 100% <=> u in [0,100]
	fPrivate2DTransform[2][1] = .01f; // Default value <=> 100% <=> v in [0,100]
}

void TSuperSamplingBase::SetSuperSamplingQuality(const TTransform2D& transform, const real32 sU, const real32 sV)
{
	fSuperSamplingU=sU;
	fSuperSamplingV=sV;
	fPrivate2DTransform = transform;

	// add the quality to the scaling
	// (note that I use the space avaible for it. but not in the same way 
	// than the 3d transform. I should maybe use the same notation for both
	// in order to avoid future mistakes )
	fPrivate2DTransform[2][0]*=fSuperSamplingU;
	fPrivate2DTransform[2][1]*=fSuperSamplingV;

	// Translation won't be used
	fPrivate2DTransform[0][2] = 0;
	fPrivate2DTransform[1][2] = 0;
}

void TSuperSamplingBase::SetSuperSamplingQuality(const TTransform3D& transform, const real32 sU, const real32 sV, const real32 sW)
{
	fSuperSamplingU=sU;
	fSuperSamplingV=sV;
	fSuperSamplingW=sW;
	fPrivate3DTransform = transform;

	// add the quality to the scaling
/*	fPrivate3DTransform.fRotationAndScale[0][0] /= fSuperSamplingU;
	fPrivate3DTransform.fRotationAndScale[0][1] /= fSuperSamplingU;
	fPrivate3DTransform.fRotationAndScale[0][2] /= fSuperSamplingU;

	fPrivate3DTransform.fRotationAndScale[1][0] /= fSuperSamplingV;
	fPrivate3DTransform.fRotationAndScale[1][1] /= fSuperSamplingV;
	fPrivate3DTransform.fRotationAndScale[1][2] /= fSuperSamplingV;

	fPrivate3DTransform.fRotationAndScale[2][0] /= fSuperSamplingW;
	fPrivate3DTransform.fRotationAndScale[2][1] /= fSuperSamplingW;
	fPrivate3DTransform.fRotationAndScale[2][2] /= fSuperSamplingW;*/
}

void TSuperSamplingBase::GetPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4)
{
	// dH/dr when sample at the 4 corners (r==x)
	const real32 dHdr = GetDhDrInUV(value1,value2,value3,value4);
	// dH/ds when sample at the 4 corners (s==y)
	const real32 dHds = GetDhDsInUV(value1,value2,value3,value4);

	if(shadingIn.fUVx.x!=0 || shadingIn.fUVx.y!=0)
	{
		// perturbation
		// Note: using fIsoU and fIsoV does not return the right result
		TVector3 dirU; shadingIn.fPointLocx.Normalize(dirU);
		TVector3 dirV; shadingIn.fPointLocy.Normalize(dirV);
		result = dHdr*dirU + dHds*dirV;
	}
	else
	{	// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors

		// perturbation
		TVector3 dirU;
		TVector3 dirV;
		shadingIn.fNormalLoc.BuildOrthonormalBase(dirU,dirV);
		result = dHdr*dirU + dHds*dirV;
	}
}


void TSuperSamplingBase::GetGlobalPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4)
{
	// dH/dr when sample at the 4 corners (r==x)
	const real32 dHdr = 0.25*(value1+value2-value3-value4)/(fRescaledDx.GetMagnitude());
	// dH/ds when sample at the 4 corners (s==y)
	const real32 dHds = 0.25*(value1+value3-value2-value4)/(fRescaledDy.GetMagnitude());

	if( shadingIn.fPointx.x!=0 || 
		shadingIn.fPointx.y!=0 ||
		shadingIn.fPointx.z!=0 )
	{
		// perturbation
		TVector3 dirX; shadingIn.fPointx.Normalize(dirX);
		TVector3 dirY; shadingIn.fPointy.Normalize(dirY);
		result = dHdr*dirX + dHds*dirY;
	}
	else
	{	// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors

		// perturbation
		TVector3 dirX;
		TVector3 dirY;
		shadingIn.fGNormal.BuildOrthonormalBase(dirX,dirY);
		result = dHdr*dirX + dHds*dirY;
	}
}

void TSuperSamplingBase::GetLocalPerturbationVector(TVector3& result,ShadingIn& shadingIn, const real32 value1, const real32 value2, const real32 value3, const real32 value4)
{
	// dH/dr when sample at the 4 corners (r==x)
	const real32 dHdr = 0.25*(value1+value2-value3-value4)/(fRescaledDx.GetMagnitude());
	// dH/ds when sample at the 4 corners (s==y)
	const real32 dHds = 0.25*(value1+value3-value2-value4)/(fRescaledDy.GetMagnitude());

	if( shadingIn.fPointLocx.x!=0 || 
		shadingIn.fPointLocx.y!=0 ||
		shadingIn.fPointLocx.z!=0 )
	{
		// perturbation
		TVector3 dirX; shadingIn.fPointLocx.Normalize(dirX);
		TVector3 dirY; shadingIn.fPointLocy.Normalize(dirY);
		result = dHdr*dirX + dHds*dirY;
	}
	else
	{	// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors

		// perturbation
		TVector3 dirX;
		TVector3 dirY;
		shadingIn.fNormalLoc.BuildOrthonormalBase(dirX,dirY);
		result = dHdr*dirX + dHds*dirY;
	}
}

void TSuperSamplingBase::SuperSampling(const real32 uu, const real32 vv, ShadingIn& shadingIn,
								 real32& value1, real32& value2, real32& value3, real32& value4 )
{
	// Avoid multi level supersampling
	shadingIn.fCurrentCompletionMask |= gCannotSuperSample;

	// Sampling Rate
	real32 sRate = 0;

	const real32 normX = shadingIn.fUVx.GetNorm();
	if( normX>kRootDerivativeLimit )
	{
		fRescaledUVx = shadingIn.fUVx;
		fRescaledUVy = shadingIn.fUVy;
			
		if(fNeedSamplingRate)
			sRate = RealAbs(normX + fRescaledUVy.GetNorm());
	}
	else
	{
		if(shadingIn.fUVx.x==0 && shadingIn.fUVx.y==0)
		{
			// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
			fRescaledUVx.x = .001f;
			fRescaledUVx.y = 0;
			fRescaledUVy.x = 0;
			fRescaledUVy.y = .001f;

			sRate = .002f; 
		}
		else
		{	// the derivatives are too small to be significant: use a small value
			const real64 Xsum = (RealAbs(shadingIn.fUVx.x)+RealAbs(shadingIn.fUVx.y));
			const real64 Xcoeffx = shadingIn.fUVx.x/Xsum;
			const real64 Xcoeffy = shadingIn.fUVx.y/Xsum;
			fRescaledUVx.x = Xcoeffx*kRootDerivativeLimit;
			fRescaledUVx.y = Xcoeffy*kRootDerivativeLimit;
			const real64 Ysum = (RealAbs(shadingIn.fUVy.x)+RealAbs(shadingIn.fUVy.y));
			const real64 Ycoeffx = shadingIn.fUVy.x/Ysum;
			const real64 Ycoeffy = shadingIn.fUVy.y/Ysum;
			fRescaledUVy.x = Ycoeffx*kRootDerivativeLimit;
			fRescaledUVy.y = Ycoeffy*kRootDerivativeLimit;

			sRate = 2*kRootDerivativeLimit;
		}
	}

	// rotate (but do not translate it! it's a vector, not a point)
	const real32 Xx = fRescaledUVx.x;
	const real32 Xy = fRescaledUVx.y;
	fRescaledUVx[0] = Xx*fPrivate2DTransform[0][0] + Xy*fPrivate2DTransform[0][1];
	fRescaledUVx[1] = Xx*fPrivate2DTransform[1][0] + Xy*fPrivate2DTransform[1][1];
	// Scale the vector
	fRescaledUVx.x /= (fPrivate2DTransform[2][0]);
	fRescaledUVx.y /= (fPrivate2DTransform[2][1]);

	// rotate (but do not translate it! it's a vector, not a point)
	const real32 Yx = fRescaledUVy.x;
	const real32 Yy = fRescaledUVy.y;
	fRescaledUVy[0] = Yx*fPrivate2DTransform[0][0] + Yy*fPrivate2DTransform[0][1];
	fRescaledUVy[1] = Yx*fPrivate2DTransform[1][0] + Yy*fPrivate2DTransform[1][1];
	// Scale the vector
	fRescaledUVy.x /= (fPrivate2DTransform[2][0]);
	fRescaledUVy.y /= (fPrivate2DTransform[2][1]);

	const real32 x1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledUVx.x);
	const real32 x2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledUVy.x);
	const real32 y1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledUVx.y);
	const real32 y2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledUVy.y);

	value1 = ComputeOneSample(	uu - x1 - x2, vv - y1 - y2, sRate );
	value2 = ComputeOneSample(	uu - x1 + x2, vv - y1 + y2, sRate );
	value3 = ComputeOneSample(	uu + x1 - x2, vv + y1 - y2, sRate );
	value4 = ComputeOneSample(	uu + x1 + x2, vv + y1 + y2, sRate );

	shadingIn.fCurrentCompletionMask &=~gCannotSuperSample;		
}

void TSuperSamplingBase::GlobalSuperSampling(const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn,
								 real32& value1, real32& value2, real32& value3, real32& value4 )
{
	// Sampling Rate
	real32 sRate = 0;

	// We maybe should also test the second vector (fPointy)
	const real32 normX = shadingIn.fPointx.GetNorm();
	if( normX>kRootDerivativeLimit )
	{
		fRescaledDx = shadingIn.fPointx;
		fRescaledDy = shadingIn.fPointy;
			
		if(fNeedSamplingRate)
			sRate = RealAbs(normX + shadingIn.fPointy.GetNorm());
	}
	else
	{
		if(shadingIn.fPointx.x==0 && shadingIn.fPointx.y==0 && shadingIn.fPointx.z==0 )
		{
			// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
			
			// Choose 2 directions tangent to the surface
			TVector3 dir1;
			TVector3 dir2;
			shadingIn.fGNormal.BuildOrthonormalBase(dir1,dir2);

			// And an offset on these directions
			fRescaledDx = .001f * dir1;
			fRescaledDy = .001f * dir2;

			sRate = .002f;
		}
		else
		{	// the derivatives are too small to be significant: use a small value
			const real64 Xsum = (shadingIn.fPointx.x+shadingIn.fPointx.y+shadingIn.fPointx.z);
			const real64 Xcoeffx = shadingIn.fPointx.x/Xsum;
			const real64 Xcoeffy = shadingIn.fPointx.y/Xsum;
			const real64 Xcoeffz = shadingIn.fPointx.z/Xsum;
			fRescaledDx.x = Xcoeffx*kRootDerivativeLimit;
			fRescaledDx.y = Xcoeffy*kRootDerivativeLimit;
			fRescaledDx.z = Xcoeffz*kRootDerivativeLimit;

			const real64 Ysum = (shadingIn.fPointy.x+shadingIn.fPointy.y+shadingIn.fPointy.z);
			const real64 Ycoeffx = shadingIn.fPointy.x/Ysum;
			const real64 Ycoeffy = shadingIn.fPointy.y/Ysum;
			const real64 Ycoeffz = shadingIn.fPointy.z/Ysum;
			fRescaledDy.x = Ycoeffx*kRootDerivativeLimit;
			fRescaledDy.y = Ycoeffy*kRootDerivativeLimit;
			fRescaledDy.z = Ycoeffz*kRootDerivativeLimit;

			sRate = 2*kRootDerivativeLimit;
		}
	}

	// scale and rotate
	fRescaledDx = fPrivate3DTransform.TransformVector(fRescaledDx);
	fRescaledDy = fPrivate3DTransform.TransformVector(fRescaledDy);

	const real32 x1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.x);
	const real32 x2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.x);
	const real32 y1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.y);
	const real32 y2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.y);
	const real32 z1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.z);
	const real32 z2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.z);

	// value at the 4 corners
	value1 = ComputeOneGlobalSample(	x - x1 - x2, y - y1 - y2, z - z1 - z2, sRate );
	value2 = ComputeOneGlobalSample(	x - x1 + x2, y - y1 + y2, z - z1 + z2, sRate );
	value3 = ComputeOneGlobalSample(	x + x1 - x2, y + y1 - y2, z + z1 - z2, sRate );
	value4 = ComputeOneGlobalSample(	x + x1 + x2, y + y1 + y2, z + z1 + z2, sRate );
}

void TSuperSamplingBase::LocalSuperSampling(const real32 x, const real32 y, const real32 z, ShadingIn& shadingIn,
								 real32& value1, real32& value2, real32& value3, real32& value4 )
{
	// Sampling Rate
	real32 sRate = 0;

	// We maybe should also test the second vector (fPointy)
	const real32 normX = shadingIn.fPointLocx.GetNorm();
	if( normX>kRootDerivativeLimit )
	{
		fRescaledDx = shadingIn.fPointLocx;
		fRescaledDy = shadingIn.fPointLocy;
		
		if(fNeedSamplingRate)
			sRate = RealAbs(normX + shadingIn.fPointLocy.GetNorm());
	}
	else
	{
		if(shadingIn.fPointLocx.x==0 && shadingIn.fPointLocx.y==0 && shadingIn.fPointLocx.z==0 )
		{
			// There's a bug in Carrara: in some cases, it doesn't give us the derivative vectors	
			
			// Choose 2 directions tangent to the surface
			TVector3 dir1;
			TVector3 dir2;
			shadingIn.fNormalLoc.BuildOrthonormalBase(dir1,dir2);

			// And an offset on these directions
			fRescaledDx = .001f * dir1;
			fRescaledDy = .001f * dir2;

			sRate = .002f;
		}
		else
		{	// the derivatives are too small to be significant: use a small value
			const real64 Xsum = (shadingIn.fPointLocx.x+shadingIn.fPointLocx.y+shadingIn.fPointLocx.z);
			const real64 Xcoeffx = shadingIn.fPointLocx.x/Xsum;
			const real64 Xcoeffy = shadingIn.fPointLocx.y/Xsum;
			const real64 Xcoeffz = shadingIn.fPointLocx.z/Xsum;
			fRescaledDx.x = Xcoeffx*kRootDerivativeLimit;
			fRescaledDx.y = Xcoeffy*kRootDerivativeLimit;
			fRescaledDx.z = Xcoeffz*kRootDerivativeLimit;

			const real64 Ysum = (shadingIn.fPointLocy.x+shadingIn.fPointLocy.y+shadingIn.fPointLocy.z);
			const real64 Ycoeffx = shadingIn.fPointLocy.x/Ysum;
			const real64 Ycoeffy = shadingIn.fPointLocy.y/Ysum;
			const real64 Ycoeffz = shadingIn.fPointLocy.z/Ysum;
			fRescaledDy.x = Ycoeffx*kRootDerivativeLimit;
			fRescaledDy.y = Ycoeffy*kRootDerivativeLimit;
			fRescaledDy.z = Ycoeffz*kRootDerivativeLimit;

			sRate = 2*kRootDerivativeLimit;
		}
	}

	// scale and rotate
	fRescaledDx = fPrivate3DTransform.TransformVector(fRescaledDx);
	fRescaledDy = fPrivate3DTransform.TransformVector(fRescaledDy);

	const real32 x1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.x);
	const real32 x2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.x);
	const real32 y1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.y);
	const real32 y2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.y);
	const real32 z1 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDx.z);
	const real32 z2 = Clamp(-kMaxLimit,kMaxLimit,fRescaledDy.z);

	// value at the 4 corners
	value1 = ComputeOneLocalSample(	x - x1 - x2, y - y1 - y2, z - z1 - z2, sRate );
	value2 = ComputeOneLocalSample(	x - x1 + x2, y - y1 + y2, z - z1 + z2, sRate );
	value3 = ComputeOneLocalSample(	x + x1 - x2, y + y1 - y2, z + z1 - z2, sRate );
	value4 = ComputeOneLocalSample(	x + x1 + x2, y + y1 + y2, z + z1 + z2, sRate );
}

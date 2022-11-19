/****************************************************************************************************

		3DTransformBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/5/2004

****************************************************************************************************/

#include "3DTransformBase.h"

T3DTransformBase::T3DTransformBase()
{
	Init();
}

void T3DTransformBase::Init()
{
	fGlobalScale = 100;
	fScaleX = 100;
	fScaleY = 100;
	fScaleZ = 100;
	fOffsetX = 0;
	fOffsetY = 0;
	fOffsetZ = 0;
	fRotationX = 0;
	fRotationY = 0;
	fRotationZ = 0;
	fSpace='Loca';
}

void T3DTransformBase::Get3DTransform(TTransform3D& transform)
{
	// Translation
	transform.fTranslation.x = fOffsetX;
	transform.fTranslation.y = fOffsetY;
	transform.fTranslation.z = fOffsetZ;
	// Scaling
	transform.fRotationAndScale = TMatrix33::kIdentity;
	transform.fRotationAndScale[0][0]=100.0/fScaleX;
	transform.fRotationAndScale[1][1]=100.0/fScaleY;
	transform.fRotationAndScale[2][2]=100.0/fScaleZ;
	// rotation
	real32 fPhyRad = DegToRad(fRotationZ);
	real32 fThetaRad= DegToRad(fRotationY);
	real32 fPsyRad =DegToRad(fRotationX);
	TMatrix33 rotation;
	real64 sinphy = RealSin(fPhyRad);
	real64 cosphy = RealCos(fPhyRad);
	real64 sintheta = RealSin(fThetaRad);
	real64 costheta = RealCos(fThetaRad);
	real64 sinpsy = RealSin(fPsyRad);
	real64 cospsy = RealCos(fPsyRad);
	rotation.SetColumn( 0, TVector3(	(cosphy * costheta), (sinphy * costheta), (-sintheta)));
	rotation.SetColumn( 1, TVector3(	(cosphy * sintheta * sinpsy - sinphy * cospsy), 
										(sinphy * sintheta * sinpsy + cosphy * cospsy), 
										(costheta * sinpsy)));
	rotation.SetColumn( 2, TVector3(	(cosphy * sintheta * cospsy + sinphy * sinpsy), 
										(sinphy * sintheta * cospsy - cosphy * sinpsy),
										(costheta * cospsy)));

	transform.fRotationAndScale = transform.fRotationAndScale*rotation;
	transform.fRotationAndScale = (100.0/fGlobalScale)*transform.fRotationAndScale;
}

boolean T3DTransformBase::IsEqualTo(T3DTransformBase* aTransform)		// Use it to compare two T2DTransformBase
{
	return (
		(fGlobalScale == aTransform->fGlobalScale) &&
		(fScaleX == aTransform->fScaleX) &&
		(fScaleY == aTransform->fScaleY) &&
		(fScaleZ == aTransform->fScaleZ) &&
		(fOffsetX == aTransform->fOffsetX) &&
		(fOffsetY == aTransform->fOffsetY) &&
		(fOffsetZ == aTransform->fOffsetZ) &&
		(fRotationX == aTransform->fRotationX) &&
		(fRotationY == aTransform->fRotationY) &&
		(fRotationZ == aTransform->fRotationZ) );
}


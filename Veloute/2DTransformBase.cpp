/****************************************************************************************************

		2DTransformBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/5/2004

****************************************************************************************************/

#include "2DTransformBase.h"

T2DTransformBase::T2DTransformBase()
{
	Init();
}

void T2DTransformBase::Init()
{
	fGlobalScale = 100;
	fScaleU = 100;
	fScaleV = 100;
	fOffsetU = 0;
	fOffsetV = 0;
	fRotation = 0;
}

void T2DTransformBase::Get2DTransform(TTransform2D& result)
{
	const real32 cosR=RealCos(DegToRad(fRotation));
	const real32 sinR=RealSin(DegToRad(fRotation));
	const TMatrix22 rotation(cosR, -sinR, sinR, cosR);
	result.SetRotation(rotation);
	const TVector2 translation(fOffsetU/100, fOffsetV/100);
	result.SetTranslation(translation);

	// We also include the scaling to the matrix
	result[2][0] = fGlobalScale*fScaleU/1000000; // the 2 values are %, and /100 to scale the [0,1] space to a [0,100] space
	result[2][1] = fGlobalScale*fScaleV/1000000;
}

boolean T2DTransformBase::IsEqualTo(T2DTransformBase* aTransform)		// Use it to compare two T2DTransformBase
{
	return (
		(fGlobalScale == aTransform->fGlobalScale) &&
		(fScaleU == aTransform->fScaleU) &&
		(fScaleV == aTransform->fScaleV) &&
		(fOffsetU == aTransform->fOffsetU) &&
		(fOffsetV == aTransform->fOffsetV) &&
		(fRotation == aTransform->fRotation) );
}


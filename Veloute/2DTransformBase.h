/****************************************************************************************************

		2DTransformBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/5/2004

****************************************************************************************************/

#ifndef __2DTransformBase__
#define __2DTransformBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Transform2D.h"

class T2DTransformBase
{
public:
	T2DTransformBase();

	void		Init();

	void		Get2DTransform(TTransform2D& result);

	boolean		IsEqualTo(T2DTransformBase* aTransform);

protected:
	// All these param are part of a pmap, do not add anything before or after that won't be in the pmap
	real32		fGlobalScale;
	real32		fScaleU;
	real32		fScaleV;
	real32		fOffsetU;
	real32		fOffsetV;
	real32		fRotation;
};








#endif

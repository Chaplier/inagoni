/****************************************************************************************************

		3DTransformBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	1/5/2004

****************************************************************************************************/

#ifndef __3DTransformBase__
#define __3DTransformBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Basic3DCOMImplementations.h"

class T3DTransformBase
{
public:
	T3DTransformBase();

	void		Init();

	void		Get3DTransform(TTransform3D& result);

	boolean		IsLocalSpace(){return (fSpace=='Loca');}
	boolean		IsGlobalSpace(){return (fSpace=='Glob');}

	boolean		IsEqualTo(T3DTransformBase* aTransform);

protected:
	// All these param are part of a pmap, do not add anything before or after that won't be in the pmap
	real32		fGlobalScale;
	real32		fScaleX;
	real32		fScaleY;
	real32		fScaleZ;
	real32		fOffsetX;
	real32		fOffsetY;
	real32		fOffsetZ;
	real32		fRotationX;
	real32		fRotationY;
	real32		fRotationZ;
	int32		fSpace;
};








#endif

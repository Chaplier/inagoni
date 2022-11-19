/************************************************************************************************************

  	ToolBox.h

	Copyright (c)1990 - 1999 MetaCreations. All rights reserved.

	Author:	Mael Sicsic

	Date:	8/10/99

	Revision History

	when		who			what & why
	=========== ===========	=============================================================

************************************************************************************************************/
#include "Transforms.h"
#include "APITypes.h"

// These calls are usefull during the rendering

void MixMultiply(TMatrix33 &res,const TMatrix33 &AA,const TMatrix33 &BB);
void MixTransform(TVector3 &res,const TMatrix33 &AA,const TVector3 &VV);
void MixTransformPoint(TVector3 &res,const TTransform3D &AA,const TVector3 &VV); 
void MixTransformVector(TVector3 &res,const TTransform3D &AA,const TVector3 &VV);
TVector3 MixNormalFacet(const TVector3 &A, const TVector3 &B, const TVector3 &C);
void GetIsoUV (TVector3& theIsoU, TVector3& theIsoV, const TFacet3D& aFacet);

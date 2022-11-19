/************************************************************************************************************

  	ToolBox.cpp

	Copyright (c)1990 - 1999 MetaCreations. All rights reserved.

	Author:	Mael Sicsic

	Date:	8/10/99

	Revision History

	when		who			what & why
	=========== ===========	=============================================================

************************************************************************************************************/

#include "ToolBox.h"
#include "PublicUtilities.h"



/*Normal of a facet*/
TVector3 MixNormalFacet(const TVector3 &A, const TVector3 &B, const TVector3 &C)
{
	TVector3 res;
	
	res = (B-A)^(C-A);
	return res;
}


/*Redefining a product between two 3D matrices */
void MixMultiply(TMatrix33 &res,const TMatrix33 &AA,const TMatrix33 &BB)
{
	TMatrix33 CC;

	CC[0][0]=AA[0][0]*BB[0][0] + AA[0][1]*BB[1][0] + AA[0][2]*BB[2][0];
	CC[0][1]=AA[0][0]*BB[0][1] + AA[0][1]*BB[1][1] + AA[0][2]*BB[2][1];
	CC[0][2]=AA[0][0]*BB[0][2] + AA[0][1]*BB[1][2] + AA[0][2]*BB[2][2];

	CC[1][0]=AA[1][0]*BB[0][0] + AA[1][1]*BB[1][0] + AA[1][2]*BB[2][0];
	CC[1][1]=AA[1][0]*BB[0][1] + AA[1][1]*BB[1][1] + AA[1][2]*BB[2][1];
	CC[1][2]=AA[1][0]*BB[0][2] + AA[1][1]*BB[1][2] + AA[1][2]*BB[2][2];

	CC[2][0]=AA[2][0]*BB[0][0] + AA[2][1]*BB[1][0] + AA[2][2]*BB[2][0];
	CC[2][1]=AA[2][0]*BB[0][1] + AA[2][1]*BB[1][1] + AA[2][2]*BB[2][1];
	CC[2][2]=AA[2][0]*BB[0][2] + AA[2][1]*BB[1][2] + AA[2][2]*BB[2][2];

	res=CC;
}

void MixTransformPoint(TVector3 &res,const TTransform3D &AA,const TVector3 &VV)
{
	MixTransform(res,AA.fRotationAndScale,VV);	
	res[0] += AA.fTranslation[0] ;
	res[1] += AA.fTranslation[1] ;
	res[2] += AA.fTranslation[2] ;
}



/*Used to apply a transformation to a mathematical vector*/
void MixTransformVector(TVector3 &res,const TTransform3D &AA,const TVector3 &VV)
{
	MixTransform(res,AA.fRotationAndScale,VV);

	//Be careful the result vector :res is not an unit vector
	//We have to normalize it
	real norm = sqrt(res[0]*res[0] +  res[1]*res[1] + res[2]*res[2]);
	res[0] /= norm;
	res[1] /= norm;
	res[2] /= norm;
}


/*Internal use */
void MixTransform(TVector3 &res,const TMatrix33 &AA,const TVector3 &VV)
{
	TVector3 QQ;

	QQ[0]=AA[0][0]*VV[0] + AA[0][1]*VV[1] + AA[0][2]*VV[2] ;
	QQ[1]=AA[1][0]*VV[0] + AA[1][1]*VV[1] + AA[1][2]*VV[2] ;
	QQ[2]=AA[2][0]*VV[0] + AA[2][1]*VV[1] + AA[2][2]*VV[2] ;

	res=QQ;
}


void GetIsoUV (TVector3& theIsoU, TVector3& theIsoV, const TFacet3D& aFacet)
{
	TVector3	seg1, seg2;
	TVector2	UV1, UV2;
	real		det;

	seg1 = aFacet.fVertices[1].fVertex - aFacet.fVertices[0].fVertex;
	seg2 = aFacet.fVertices[2].fVertex - aFacet.fVertices[0].fVertex;

	UV1[0] = aFacet.fVertices[1].fUV[0] - aFacet.fVertices[0].fUV[0];
	UV1[1] = aFacet.fVertices[1].fUV[1] - aFacet.fVertices[0].fUV[1];
	UV2[0] = aFacet.fVertices[2].fUV[0] - aFacet.fVertices[0].fUV[0];
	UV2[1] = aFacet.fVertices[2].fUV[1] - aFacet.fVertices[0].fUV[1];

	det = UV1[0]*UV2[1] - UV1[1]*UV2[0];

	if (det != 0.0) {
		UV1[0] /= det;
		UV1[1] /= det;
		UV2[0] /= det;
		UV2[1] /= det;

		theIsoU[0] = seg1[0]*UV2[1] - seg2[0]*UV1[1];
		theIsoU[1] = seg1[1]*UV2[1] - seg2[1]*UV1[1];
		theIsoU[2] = seg1[2]*UV2[1] - seg2[2]*UV1[1];
		theIsoV[0] = seg2[0]*UV1[0] - seg1[0]*UV2[0];
		theIsoV[1] = seg2[1]*UV1[0] - seg1[1]*UV2[0];
		theIsoV[2] = seg2[2]*UV1[0] - seg1[2]*UV2[0];
		}
	else {
		theIsoU[0] = 1.0;
		theIsoU[1] = 0.0;
		theIsoU[2] = 0.0;
		theIsoV[0] = 0.0;
		theIsoV[1] = 1.0;
		theIsoV[2] = 0.0;
		}
	}

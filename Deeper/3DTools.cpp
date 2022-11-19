/************************************************************************************************************

  	3DTools.cpp

	Copyright (c)1990 - 1999 MetaCreations. All rights reserved.

	Author:	Jerome Bignon

	Date:	19/07/99

	Revision History

	when		who			what & why
	=========== ===========	=============================================================
	19/07/99	Jerome		Creation

************************************************************************************************************/
#include "3dtools.h"
#include <math.h>

/*3DTOOLS is a mathematics-3D toolkit for third-party developers*/

/*Normal of a facet*/
TVector3 NormalFacet(const TVector3 &A, const TVector3 &B, const TVector3 &C) {
	TVector3 res;
	
	res = (B-A)*(C-A);
	return res;
	}

/*cross-product*/
TVector3 operator *(const TVector3 &A, const TVector3 &B) {
	TVector3 res;
	res[0] = A[1]*B[2] - A[2]*B[1];
	res[1] = A[2]*B[0] - A[0]*B[2];
	res[2] = A[0]*B[1] - A[1]*B[0];
	return res;
	}

TVector3 operator -(const TVector3 &A, const TVector3 &B) {
	TVector3 res;
	res[0] = A[0]-B[0];
	res[1] = A[1]-B[1];
	res[2] = A[2]-B[2];
	return res;
	}

/*Applies a same transformation to all vertices of a facet */
void TransformFacet(FACET3D &res, const TRANSFORM3D &AA, const FACET3D &VV) {

	TransformPoint(res.fVertices[0].fVertex, AA, VV.fVertices[0].fVertex);
	TransformPoint(res.fVertices[1].fVertex, AA, VV.fVertices[1].fVertex);
	TransformPoint(res.fVertices[2].fVertex, AA, VV.fVertices[2].fVertex);


	TransformVector(res.fVertices[0].fNormal, AA, VV.fVertices[0].fNormal);
	TransformVector(res.fVertices[1].fNormal, AA, VV.fVertices[1].fNormal);
	TransformVector(res.fVertices[2].fNormal, AA, VV.fVertices[2].fNormal);
	
	res.fUVSpace = VV.fUVSpace;

	}
	
/*Basically transforming a vertex, maybe the most useful function */	
void TransformPoint(VECTOR3D &res,const TRANSFORM3D &AA,const VECTOR3D &VV) {
	Transform(res,AA.fR,VV);	
	res[0] += AA.fT[0] ;
	res[1] += AA.fT[1] ;
	res[2] += AA.fT[2] ;
	}

/*Used to apply a transformation to a mathematical vector*/
void TransformVector(VECTOR3D &res,const TRANSFORM3D &AA,const VECTOR3D &VV) {
	Transform(res,AA.fR,VV);

	//Be careful the result vector :res is not an unit vector
	//We have to normalize it
	NUM3D norm = sqrt(res[0]*res[0] +  res[1]*res[1] + res[2]*res[2]);
	res[0] /= norm;
	res[1] /= norm;
	res[2] /= norm;
	}

/*Very useful to transform an AFFINETRANSFORM into a TRANSFORM3D : 
the 2 structures are fonctionnally the same but differ as memory is concerned */ 	
TRANSFORM3D AffineToTransform3D(const AFFINETRANSFORM &AA) {
	
	TRANSFORM3D res;
	res.fT[0] = AA.fT[0] ;
	res.fT[1] = AA.fT[1] ;
	res.fT[2] = AA.fT[2] ;
	res.fR.fix = AA.fR[0][0] ;
	res.fR.fjx = AA.fR[0][1] ;
	res.fR.fkx = AA.fR[0][2] ;
	res.fR.fiy = AA.fR[1][0] ;
	res.fR.fjy = AA.fR[1][1] ;
	res.fR.fky = AA.fR[1][2] ;
	res.fR.fiz = AA.fR[2][0] ;
	res.fR.fjz = AA.fR[2][1] ;
	res.fR.fkz = AA.fR[2][2];
	return res;
	
	}
	
/*Product between two transformations*/
TRANSFORM3D  operator *(const TRANSFORM3D &AA,const TRANSFORM3D &BB) {
	TRANSFORM3D  res;

	Multiply(res.fR,AA.fR,BB.fR);
	TransformPoint(res.fT,AA,BB.fT);
	return res;
	}

/*Redefining a product between two 3D matrices */
void Multiply(MATRIX3D &res,const MATRIX3D &AA,const MATRIX3D &BB) {
	MATRIX3D CC;

	CC.fix=AA.fix*BB.fix + AA.fjx*BB.fiy + AA.fkx*BB.fiz;
	CC.fjx=AA.fix*BB.fjx + AA.fjx*BB.fjy + AA.fkx*BB.fjz;
	CC.fkx=AA.fix*BB.fkx + AA.fjx*BB.fky + AA.fkx*BB.fkz;

	CC.fiy=AA.fiy*BB.fix + AA.fjy*BB.fiy + AA.fky*BB.fiz;
	CC.fjy=AA.fiy*BB.fjx + AA.fjy*BB.fjy + AA.fky*BB.fjz;
	CC.fky=AA.fiy*BB.fkx + AA.fjy*BB.fky + AA.fky*BB.fkz;

	CC.fiz=AA.fiz*BB.fix + AA.fjz*BB.fiy + AA.fkz*BB.fiz;
	CC.fjz=AA.fiz*BB.fjx + AA.fjz*BB.fjy + AA.fkz*BB.fjz;
	CC.fkz=AA.fiz*BB.fkx + AA.fjz*BB.fky + AA.fkz*BB.fkz;

	res=CC;
	}		

/*Internal use */
void Transform(VECTOR3D &res,const MATRIX3D &AA,const VECTOR3D &VV) {
	VECTOR3D QQ;
	QQ[0]=AA.fix*VV[0] + AA.fjx*VV[1] + AA.fkx*VV[2] ;
	QQ[1]=AA.fiy*VV[0] + AA.fjy*VV[1] + AA.fky*VV[2] ;
	QQ[2]=AA.fiz*VV[0] + AA.fjz*VV[1] + AA.fkz*VV[2] ;
	res=QQ;
	}

/*Basically used in Doshade method*/
/*coord of Facet */
void GetIsoUV (VECTOR3D& theIsoU, VECTOR3D& theIsoV, const FACET3D& aFacet) {
	VECTOR3D	seg1, seg2;
	VECTOR2D	UV1, UV2;
	NUM3D		det;

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

/*Blitscreen blits a personal buffer in the raster buffer*/
/*works only for an image depth = 32*/
 void BlitScreen32(IShRasterOffscreen* theImage, RECT3D DrawingArea, long *screenBuffer) {

	ULONG chunkH, chunkV, rowBytes, depth;
	
	int SizeH = DrawingArea.right - DrawingArea.left;
	int SizeV = DrawingArea.bottom - DrawingArea.top;
	
	BufferChunk* aChunk=NULL;
	RECT3D shortRect;

	IEnumChunk* iter = theImage->EnumChunks(&DrawingArea); //we initialize a chunk iterator

	while ( iter->Next(1,&aChunk,0) == S_OK ) {

		theImage->GetChunkRect(aChunk, &shortRect); //we get the localization of our chunk on our  screenBuffer
		theImage->LockChunk(aChunk); 

		long* data = (long*)theImage->GetChunkData(aChunk); //we get a pointer on the data of the chunk

		for (long vv = shortRect.top; vv < shortRect.bottom; vv++) {

			for (long hh = shortRect.left; hh < shortRect.right; hh++) {
				
				//Be careful, Drawing area dimensions are not necessary multiples of tiles dimensions
				//So, tile may go over the edge of the drawing area.
				//We have to test if we are in the drawing area
				if ( hh < SizeH && vv < SizeV ) {
					int ind = vv*SizeH+hh;
					(*data) = screenBuffer[ind]; //we blit our screenBuffer on the RayDream raster buffer
					}
				
				data++;

				} // for hh
			} // for vv
		theImage->UnlockChunk(aChunk);
		} // for iter
	iter->Release();
	}
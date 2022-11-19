
#include "Tessellation.h"
#include "Utils.h"

#ifndef CALLBACK
#define CALLBACK 
#endif

typedef void (CALLBACK *GLfn)();

struct TessInfo
{
public:
	TessInfo( std::vector<TVector3>& vertices, TMCClassArray<Triangle>& tgles, int offset )
		: mVertices(vertices), mTriangles(tgles), mOffset(offset) {  }

public:

	void CheckAndAdd(int iPoint0, int iPoint1, int iPoint2) 
	{
		const int iVtx0 = mPoints[iPoint0];
		const int iVtx1 = mPoints[iPoint1];
		const int iVtx2 = mPoints[iPoint2];

		{ // Check for flat triangles
			const int vtxCount = (int)mVertices.size();

			const int localIndex0 = iVtx0 - mOffset;
			const int localIndex1 = iVtx1 - mOffset;
			const int localIndex2 = iVtx2 - mOffset;
			const TVector3& vtx0 = mVertices[localIndex0];
			const TVector3& vtx1 = mVertices[localIndex1];
			const TVector3& vtx2 = mVertices[localIndex2];

			if( ArePointsAligned( vtx0,vtx1,vtx2 ) )
				return; // Flat triangle
		}

		mTriangles.AddElem(Triangle( iVtx0, iVtx1, iVtx2 ));
	}
//the polygon being constructed
	GLenum mType;
	std::vector<int> mPoints;

	int mOffset;

//where to output the triangles
	std::vector<TVector3>& mVertices;
	TMCClassArray<Triangle>& mTriangles;
};

void CALLBACK MyTessBegin(GLenum type, void* polygon_data)
{
	if(polygon_data)
	{
		TessInfo* Info = static_cast<TessInfo*>(polygon_data);

		Info->mType=type;
		Info->mPoints.clear();
	}
}

void CALLBACK MyTessVertex(void* vertex_data, void* polygon_data)
{
	if(polygon_data)
	{
		TessInfo* Info = static_cast<TessInfo*>(polygon_data);
		int64 VertexIndex = (int64)vertex_data;

		Info->mPoints.push_back(VertexIndex + Info->mOffset );  //Sum the offset and the VertexIndex
	}
}

void CALLBACK MyTessEnd(void* polygon_data)
{
	if(polygon_data)
	{
		TessInfo* Info = static_cast<TessInfo*>(polygon_data);

		switch (Info->mType)
		{
		case GL_TRIANGLES:
			for (int i=0 ; i<(int)Info->mPoints.size() ; i+=3)
			{
				Info->CheckAndAdd( i, i+1, i+2 );
			}
			break;
		case GL_TRIANGLE_FAN:
			for (int i=2 ; i<(int)Info->mPoints.size() ; i+=1)
			{
				Info->CheckAndAdd( 0, i-1, i );			
			}
			break;
		case GL_TRIANGLE_STRIP:
			for (int i=2 ; i<(int)Info->mPoints.size() ; i+=1)
			{
				if (i%2==0)
				{
					Info->CheckAndAdd( i-2, i-1, i );
				}
				else
				{
					Info->CheckAndAdd( i-1, i-2, i );
				}
			}
			break;
		}

		Info->mPoints.clear();
	}
}

void CALLBACK MyTessCombine(
	GLdouble coords[3],
	int d[4],
	GLfloat w[4],
	int* dataOut,
	void* polygon_data)
{
	TessInfo* Info = (TessInfo*)polygon_data;

	Info->mVertices.push_back(TVector3( coords[0], coords[1], coords[2] ) );

	*dataOut = ((int)Info->mVertices.size()-1) - Info->mOffset; //It's a new vertex. Modify the VertexIndice in order to have a correct value
}


void CALLBACK MyTessError(GLenum errno, void* polygon_data)
{
	// MY_ASSERT(false);
}

GLUtesselator* InitNewTesselator()
{
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLfn)&MyTessBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLfn)&MyTessVertex);
	gluTessCallback(tess, GLU_TESS_END_DATA, (GLfn)&MyTessEnd);
	gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLfn)&MyTessCombine);
	gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLfn)&MyTessError);
	return tess;
}

bool TesselatePolygon(const FlatPolygon& polygon, FlatMesh& result)
{
	GLUtesselator* tess = InitNewTesselator();
	
	std::vector<TVector3> vertices3D;
	const int pointCount = polygon.GetElemCount();
	vertices3D.reserve( pointCount );
	for (int i=0 ; i<pointCount ; i++)
	{
		const TVector2& curPos = polygon[i];

		vertices3D.push_back( TVector3( curPos.x, curPos.y, 0 ) );
	}

	const int offset = result.mVertices.GetElemCount();
	TessInfo Info(vertices3D, result.mTriangles, offset); 

	GLdouble coords[3] = {0,0,0};

	{
		gluTessBeginPolygon(tess, (void*)&Info);
		{
			gluTessBeginContour(tess);
			{
				for (int i=0 ; i<(int)polygon.GetElemCount() ; i++)
				{
					coords[0] = polygon[i].x;
					coords[1] = polygon[i].y;
					coords[2] = 0;
					gluTessVertex(tess, coords, (void*)i);
				}
			} // Note: could add holes with gluNextContour(tobj, GLU_INTERIOR); 
			gluTessEndContour(tess);
		}
		gluTessEndPolygon(tess);
	}

	gluDeleteTess(tess);

	const int vtxCount = (int)vertices3D.size();
	result.mVertices.SetElemCount( offset + vtxCount );
	for (int iVtx=0 ; iVtx<vtxCount ; iVtx++)
	{
		result.mVertices[iVtx+offset].x = vertices3D[iVtx].x;
		result.mVertices[iVtx+offset].y = vertices3D[iVtx].y;
	}

	return true;
}

bool TesselatePolygon( std::vector<TVector3>& vertices3D, TMCClassArray<Triangle>& result)
{
	GLUtesselator* tess = InitNewTesselator();
	
	const int offset = 0;
	TessInfo Info(vertices3D, result, offset); 

	GLdouble coords[3] = {0,0,0};

	{
		gluTessBeginPolygon(tess, (void*)&Info);
		{
			gluTessBeginContour(tess);
			{
				for (int i=0 ; i<(int)vertices3D.size() ; i++)
				{
					coords[0] = vertices3D[i].x;
					coords[1] = vertices3D[i].y;
					coords[2] = 0;
					gluTessVertex(tess, coords, (void*)i);
				}
			}
			gluTessEndContour(tess);
		}
		gluTessEndPolygon(tess);
	}

	gluDeleteTess(tess);

	return result.GetElemCount()>0;
}


 
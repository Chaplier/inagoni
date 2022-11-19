#pragma once

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif
#include <vector>

#include "Vector2.h"
#include "I3dShFacetMesh.h" // For the class Triangle

struct FlatMesh
{
	TMCClassArray<TVector2> mVertices;
	TMCClassArray<Triangle> mTriangles;
};

struct FlatPolygon : public TMCClassArray<TVector2>
{
	FlatPolygon() : mIsHole(false){}

	boolean mIsHole;
};

// GLU tessellation
bool TesselatePolygon(const FlatPolygon& polygon, FlatMesh& tessellationResult);
bool TesselatePolygon(std::vector<TVector3>& vertices3DInAndOut, TMCClassArray<Triangle>& result);

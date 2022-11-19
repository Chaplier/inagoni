/****************************************************************************************************

		PVertex.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "utils.h"
#include "PVertex.h"

const Vertex Vertex::kZero;

Vertex::Vertex(const TVector3& pos, const TVector3& normal, const TVector2& uv)
{
	MY_ASSERT(normal.Normalized());

	fPosition = pos;
	fNormal = normal;
	fUV = uv;
}
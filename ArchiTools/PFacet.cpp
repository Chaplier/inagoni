/****************************************************************************************************

		PFacet.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PFacet.h"

#include "PWall.h"
#include "PVertex.h"

Facet::Facet( const int32 v1, const int32 v2, const int32 v3 )
{
	fVertices[0]= v1;
	fVertices[1]= v2;
	fVertices[2]= v3;
}

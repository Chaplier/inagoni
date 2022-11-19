/****************************************************************************************************

		PFacet.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PFacet__
#define __PFacet__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "PWall.h"
#include "PVertex.h"

class Facet
{
public:

	Facet( const int32 v1, const int32 v2, const int32 v3 );
	
	int32	GetVertex(const int32 index){return fVertices[index];}

protected:
	int32	fVertices[3];
};






#endif

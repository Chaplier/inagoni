/****************************************************************************************************

		PQuotation.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/3/2004

****************************************************************************************************/

#ifndef __PQuotation__
#define __PQuotation__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Vector2.h"
#include "MCClassArray.h"

class Dimension
{
public:
	TVector2	fPosition; // position in the XY plane (middle of the wall)
	boolean		fXOriented; // To know if it's more along the X or the Y axis
	real32		fDimension; // the dimension value
};

// Countain all the dimension of a plan but the Wall Helper one (this one is done separatly for real time issues)
class Quotation : public TMCClassArray<Dimension>
{	
};


#endif

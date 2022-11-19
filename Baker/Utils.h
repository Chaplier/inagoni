/****************************************************************************************************

		Utils.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/1/2004

****************************************************************************************************/

#ifndef __Utils__
#define __Utils__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Vector2.h"

inline boolean SameSide(const TVector2& p1, const TVector2& p2,
						const TVector2& a, const TVector2& b )
{
	const TVector2 ab = b-a;
    const real32 cp1 = ab^(p1-a);
    const real32 cp2 = ab^(p2-a);
   if ( (cp1*cp2) >= 0 )
		return true;
    else
		return false;
}

inline boolean PointIsInTriangle( const TVector2& p, 
								  const TVector2& a, 
								  const TVector2& b,
								  const TVector2& c )
{
    if( SameSide(p,a,b,c) &&
		SameSide(p,b,a,c) &&
        SameSide(p,c,a,b) )
		return true;
    else
		return false;
}

inline void InZeroOne(TVector2& uv)
{
	if(uv.x>1 || uv.x<0)
		uv.x = uv.x - RealFloor( uv.x );
	if(uv.y>1 || uv.y<0)
		uv.y = uv.y - RealFloor( uv.y );
}


#endif

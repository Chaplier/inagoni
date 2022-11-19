/****************************************************************************************************

		PVertex.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PVertex__
#define __PVertex__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Vector2.h"
#include "Vector3.h"

class Vertex
{
public:
	static const Vertex kZero;

	Vertex(){fPosition=TVector3::kZero;fNormal=TVector3::kZero;fUV=TVector2::kZero;}

	Vertex(const TVector3& pos, const TVector3& normal, const TVector2& uv);

	inline void			SetNormal(const TVector3& normal){fNormal = normal;}
	inline const TVector3&	Normal() const {return fNormal;}

	inline void			SetPosition(const TVector3& position){fPosition = position;}
	inline const TVector3&	Position() const { return fPosition; }

	inline void			SetUV(const TVector2& uv){fUV = uv;}
	inline const TVector2& UV() const {return fUV;}

	real32&				XValue(){return fPosition.x;}
	real32&				YValue(){return fPosition.y;}
	real32&				ZValue(){return fPosition.z;}
protected:

	TVector3	fPosition;
	TVector2	fUV;
	TVector3	fNormal;
};


#endif

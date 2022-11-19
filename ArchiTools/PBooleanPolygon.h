/****************************************************************************************************

		PBooleanPolygon.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	24/03/2007

****************************************************************************************************/

#ifndef __PBooleanPolygon__
#define __PBooleanPolygon__

#include "MCArray.h"
#include "MCClassArray.h"
#include "Vector2.h"

#include <vector>

#include "Utils.h"
//Alan#include "kbool/booleng.h"
#include "booleng.h"
#include "Tessellation.h"

class BooleanPolygon
{
public:

	BooleanPolygon();
	virtual ~BooleanPolygon();

	void AddPolygon(const FlatPolygon& polygon);

	// To get the polygon and the tessellation
	void GetPolygons(TMCClassArray<FlatPolygon>& resultArray, bool wantsTessellation, FlatMesh& tessellationResult);

	void CleanUpContours() { mContours.clear(); }
	void CleanUpHoles() { mHoles.clear(); }

protected:

	void PreparePolygons( const std::vector<FlatPolygon>& polygons, GroupType group );
	
	std::vector<FlatPolygon> mContours;
	std::vector<FlatPolygon> mHoles;

	Bool_Engine mBoolEng;
};


#endif


/****************************************************************************************************

		PolygonToolBoolean.h
		Copyright: (c) Oktal SA. All rights reserved.

		Author:	Sapin
		Date:	mardi 24 Mars 2009

****************************************************************************************************/
#ifndef _PolygonToolBoolean_
#define _PolygonToolBoolean_


//Alan#include "kbool/booleng.h"
//Alan#include "kbool/_lnk_itr.h"
#include "booleng.h"
#include "_lnk_itr.h"


#include "3DGeneration/Triangularisation.h"

#include <vector>
#include "stk/Vector3.h"





class PolygonToolBoolean 
{
public :
	
	enum POLYGONE_TYPE
	{	
		Polygone_A,
		Polygone_B
	};

	enum BOOLEAN_OPERATION
	{	
		OP_BOOL_UNION, /*!< boolean OR operation */
		OP_BOOL_INTERSECTION, /*!< boolean AND operation */
		OP_BOOL_XOR, /*!< boolean EX_OR operation */
		OP_BOOL_DIFF, /*!< boolean Group A - Group B operation */
	};

	PolygonToolBoolean();	
	void AddPolygon(const std::vector<stk::Vector3>& lstpts3D, POLYGONE_TYPE Group) ;
	void Operation(BOOLEAN_OPERATION op) ;
	void GetTriangleStripResults(std::vector<RND::MMesh>& result, const RND::CTMaterial& matTerrain3D) ;
	void ClearPolyA() {}
	void ClearPolyB() {}
private :

	Bool_Engine mBoolEng;
	void GetPolygonsResults(std::vector<std::vector<stk::Vector3>>& result);

};


#endif

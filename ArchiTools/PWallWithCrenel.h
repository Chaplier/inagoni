/****************************************************************************************************

		PWallWithCrenel.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PWallWithCrenel__
#define __PWallWithCrenel__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "PWall.h"

enum ECrenelShape
{
	eUnknownShape = 0,
	eRectangular = 'Opt0',
	eSmallTop = 'Opt1',
	eDoubleTop = 'Opt2',
	eSlopeSides = 'Opt3',
	eSlopeTop = 'Opt4',
};

class WallWithCrenel :	public Wall
{
public:
	// To replace the dynamic cast
	virtual WallWithCrenel* GetWallWithCrenel() { return this; }

	static void			CreateWall(Wall **wall, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);

	virtual void		Clone(Wall** newWall, Level* inLevel, VPoint* point0, VPoint* point1, const ECloneChildrenMode cloneMode);
	virtual void		CopyFrom( Wall* otherWall, const ECloneChildrenMode cloneMode );

	virtual MCCOMErr	Write(IShTokenStream* stream);
	virtual MCCOMErr	Read(IShTokenStream* stream); 

	// No extraheight permitted on this kind of wall
	virtual void		ExtraHeight(const boolean extra){ClearFlag(eWallExtraHeight);}
	virtual real32		GetWallTotalHeight() const;

	real32 GetCrenelWidth() const { return mCrenelWidth; }
	real32 GetCrenelHeight() const { return mCrenelHeight; }
	real32 GetCrenelSpacing() const { return mCrenelSpacing; }
	real32 GetCrenelOffset() const { return mCrenelOffset; }
	ECrenelShape GetCrenelShape() const { return mCrenelShape; }

	void SetCrenelWidth(real32 value) { mCrenelWidth = value; }
	void SetCrenelHeight(real32 value) { mCrenelHeight = value; }
	void SetCrenelSpacing(real32 value) { mCrenelSpacing = value; }
	void SetCrenelOffset(real32 value) { mCrenelOffset = value; }
	void SetCrenelShape( ECrenelShape shape ) { mCrenelShape = shape; }

protected:

	WallWithCrenel(BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);
	virtual ~WallWithCrenel();

	virtual void AddExtraPolygones( BooleanPolygon& booleanPoly );

//Alan	bool WallWithCrenel::GetCrenelHole( float fromX, float limitLength, FlatPolygon& polygon );
	bool GetCrenelHole( float fromX, float limitLength, FlatPolygon& polygon );

	real32 mCrenelWidth;
	real32 mCrenelHeight;
	real32 mCrenelSpacing;
	real32 mCrenelOffset;
	ECrenelShape mCrenelShape;
};

#endif

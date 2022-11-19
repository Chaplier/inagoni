/****************************************************************************************************

		PBuildingVisitor.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PBuildingVisitor__
#define __PBuildingVisitor__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCArray.h"
#include "TBBox.h"

struct BuildingPrimData;
class BuildingPrim;
class Level;
class Wall;
class Room;
class Roof;
class VPoint;

class BuildingVisitor
{
public:
	void TraverseSelection( BuildingPrim* building, const int32 inLevel );
	void TraverseVisible( BuildingPrim* building, const int32 inLevel );

	virtual void Traverse( Level* level ){}
	virtual void Traverse( Wall* wall ){}
	virtual void Traverse( Room* room ){}
	virtual void Traverse( Roof* roof ){}
};

/////////////////////////////////////////////////////////////////////////////

class SetWallArcSegmentCountVisitor : public BuildingVisitor
{
public:
	SetWallArcSegmentCountVisitor(const int32 segments, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	int32				mSegments;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallArcOffsetVisitor : public BuildingVisitor
{
public:
	SetWallArcOffsetVisitor(const real32 offset, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	real32				mOffset;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallCrenelWidthVisitor : public BuildingVisitor
{
public:
	SetWallCrenelWidthVisitor(const real32 value, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	real32				mWidth;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallCrenelHeightVisitor : public BuildingVisitor
{
public:
	SetWallCrenelHeightVisitor(const real32 value, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	real32				mHeight;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallCrenelSpacingVisitor : public BuildingVisitor
{
public:
	SetWallCrenelSpacingVisitor(const real32 value, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	real32				mSpacing;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallCrenelOffsetVisitor : public BuildingVisitor
{
public:
	SetWallCrenelOffsetVisitor(const real32 value, TMCArray<real32>* undoData);

	virtual void Traverse( Wall* wall );

protected:
	real32				mOffset;
	TMCArray<real32>*	mUndoData;
};

/////////////////////////////////////////////////////////////////////////////

class SetWallCrenelShapeVisitor : public BuildingVisitor
{
public:
	SetWallCrenelShapeVisitor(const uint32 value);

	virtual void Traverse( Wall* wall );

protected:
	uint32				mShape;
};

/////////////////////////////////////////////////////////////////////////////

class ReplaceWallVisitor : public BuildingVisitor
{
public:

protected:
	void Replace(Wall* oldWall);

	virtual void CreateWall(Wall **result, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2) = 0;
};


class ReplaceBySimpleWallVisitor : public ReplaceWallVisitor
{
public:
	ReplaceBySimpleWallVisitor();

	virtual void Traverse( Wall* wall );

protected:
	virtual void CreateWall(Wall **result, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);
};

class ReplaceByWallWithCrenelVisitor : public ReplaceWallVisitor
{
public:
	ReplaceByWallWithCrenelVisitor();

	virtual void Traverse( Wall* wall );

protected:
	virtual void CreateWall(Wall **result, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2);
};


// Prepare the data to unfold the UVs
class PrepareUnfoldUV: public BuildingVisitor
{
public:
	PrepareUnfoldUV();

	virtual void Traverse( Level* level );
	virtual void Traverse( Wall* wall );
	virtual void Traverse( Room* room );
	virtual void Traverse( Roof* roof );

protected:
	Wall* mPrevWall;
	Room* mPrevRoom;
	Roof* mPrevRoof;

	TBBox3D mLevelBBox;
	real32 mRoomOffset;
	real32 mRoofOffset;
};



#endif

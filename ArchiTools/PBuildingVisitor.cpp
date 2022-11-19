/****************************************************************************************************

		PBuildingVisitor.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PBuildingVisitor.h"

#include "BuildingPrim.h"
#include "PWallWithCrenel.h"

void BuildingVisitor::TraverseSelection( BuildingPrim* building, const int32 inLevel )
{
	const int32 levelCount = building->LevelCount(inLevel);
	const int32 startLevel = building->StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = building->GetLevel( iLevel) ;

		if (level->Hidden())
			continue;

		if( level->Selected() )
			Traverse(level);

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( wall->Selected() )
			{
				Traverse(wall);
			}
		}

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( room->Selected() )
			{
				Traverse(room);
			}
		}

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( roof->Selected() )
			{
				Traverse(roof);
			}
		}
	}
}

void BuildingVisitor::TraverseVisible( BuildingPrim* building, const int32 inLevel )
{
	const int32 levelCount = building->LevelCount(inLevel);
	const int32 startLevel = building->StartLevel(inLevel);

	for( int32 iLevel=startLevel ; iLevel<levelCount ; iLevel++ )
	{
		Level* level = building->GetLevel( iLevel) ;

		if (level->Hidden())
			continue;

		Traverse(level);

		const int32 wallCount = level->GetWallCount();

		for( int32 iWall=0 ; iWall<wallCount ; iWall++ )
		{
			Wall* wall = level->GetWall(iWall);

			if( !wall->Hidden() )
			{
				Traverse(wall);
			}
		}

		const int32 roomCount = level->GetRoomCount();

		for( int32 iRoom=0 ; iRoom<roomCount ; iRoom++ )
		{
			Room* room = level->GetRoom(iRoom);

			if( !room->Hidden() )
			{
				Traverse(room);
			}
		}

		const int32 roofCount = level->GetRoofCount();

		for( int32 iRoof=0 ; iRoof<roofCount ; iRoof++ )
		{
			Roof* roof = level->GetRoof(iRoof);

			if( !roof->Hidden() )
			{
				Traverse(roof);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////

SetWallArcSegmentCountVisitor::SetWallArcSegmentCountVisitor(const int32 segments, TMCArray<real32>* undoData) :
	mSegments( segments ), mUndoData( undoData )
{
}

void SetWallArcSegmentCountVisitor::Traverse( Wall* wall )
{
	mUndoData->AddElem((real32)wall->GetArcSegmentCount());
	wall->SetArcSegmentCount(mSegments);
}

//////////////////////////////////////////////////////////////////////////////

SetWallArcOffsetVisitor::SetWallArcOffsetVisitor(const real32 offset, TMCArray<real32>* undoData) :
	mOffset( offset ), mUndoData( undoData )
{
}

void SetWallArcOffsetVisitor::Traverse( Wall* wall )
{
	mUndoData->AddElem(wall->GetArcOffset());
	wall->SetArcOffset(mOffset);
}

//////////////////////////////////////////////////////////////////////////////

SetWallCrenelWidthVisitor::SetWallCrenelWidthVisitor(const real32 value, TMCArray<real32>* undoData) :
	mWidth( value ), mUndoData( undoData )
{
}

void SetWallCrenelWidthVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{
		mUndoData->AddElem(wallWithCrenel->GetCrenelWidth());
		wallWithCrenel->SetCrenelWidth(mWidth);
	}
}

//////////////////////////////////////////////////////////////////////////////

SetWallCrenelHeightVisitor::SetWallCrenelHeightVisitor(const real32 value, TMCArray<real32>* undoData) :
	mHeight( value ), mUndoData( undoData )
{
}

void SetWallCrenelHeightVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{
		mUndoData->AddElem(wallWithCrenel->GetCrenelHeight());
		wallWithCrenel->SetCrenelHeight(mHeight);
	}
}

//////////////////////////////////////////////////////////////////////////////

SetWallCrenelSpacingVisitor::SetWallCrenelSpacingVisitor(const real32 value, TMCArray<real32>* undoData) :
	mSpacing( value ), mUndoData( undoData )
{
}

void SetWallCrenelSpacingVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{
		mUndoData->AddElem(wallWithCrenel->GetCrenelSpacing());
		wallWithCrenel->SetCrenelSpacing(mSpacing);
	}
}

//////////////////////////////////////////////////////////////////////////////

SetWallCrenelOffsetVisitor::SetWallCrenelOffsetVisitor(const real32 value, TMCArray<real32>* undoData) :
	mOffset( value ), mUndoData( undoData )
{
}

void SetWallCrenelOffsetVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{
		mUndoData->AddElem(wallWithCrenel->GetCrenelOffset());
		wallWithCrenel->SetCrenelOffset(mOffset);
	}
}

//////////////////////////////////////////////////////////////////////////////

SetWallCrenelShapeVisitor::SetWallCrenelShapeVisitor(const uint32 value) :
	mShape( value )
{
}

void SetWallCrenelShapeVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{
		wallWithCrenel->SetCrenelShape((ECrenelShape)mShape);
	}
}

//////////////////////////////////////////////////////////////////////////////
void ReplaceWallVisitor::Replace(Wall* oldWall)
{
	VPoint*	point0 = oldWall->GetPoint(0);
	VPoint*	point1 = oldWall->GetPoint(1);

	if( point0 )
	{
		point0->RemoveWallReference(oldWall);
	}

	if( point1 )
	{
		point1->RemoveWallReference(oldWall);
	}

	Room* room0 = oldWall->GetRoom(0);
	Room* room1 = oldWall->GetRoom(1);

	Wall* newWall = NULL;
	CreateWall(&newWall, oldWall->GetData(), oldWall->GetLevel(), point0, point1 ); 
	newWall->CopyFrom( oldWall, eCloneChild );

	newWall->SetRoom( 0, room0 );
	newWall->SetRoom( 1, room1 );

	oldWall->SetRoom( 0, NULL ); // Set the room to NULL to avoid deleting them when deleting the wall
	oldWall->SetRoom( 1, NULL ); // Set the room to NULL to avoid deleting them when deleting the wall

	oldWall->DeleteWall();
}

//////////////////////////////////////////////////////////////////////////////

ReplaceBySimpleWallVisitor::ReplaceBySimpleWallVisitor()
{
}

void ReplaceBySimpleWallVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( wallWithCrenel )
	{	// Replace the wall
		Replace( wall );
	}
}

void ReplaceBySimpleWallVisitor::CreateWall(Wall **result, BuildingPrimData* data, Level* level, VPoint* point1, VPoint* point2)
{
	Wall::CreateWall(result, data, level, point1, point2 ); 
}


//////////////////////////////////////////////////////////////////////////////

ReplaceByWallWithCrenelVisitor::ReplaceByWallWithCrenelVisitor()
{
}

void ReplaceByWallWithCrenelVisitor::Traverse( Wall* wall )
{
	WallWithCrenel* wallWithCrenel = wall->GetWallWithCrenel();
	if( !wallWithCrenel )
	{	// Replace the wall
		Replace( wall );
	}
}

void ReplaceByWallWithCrenelVisitor::CreateWall(Wall **result, BuildingPrimData* data, Level* level, VPoint* point1, VPoint* point2)
{
	WallWithCrenel::CreateWall(result, data, level, point1, point2 ); 
}


//////////////////////////////////////////////////////////////////////////////
PrepareUnfoldUV::PrepareUnfoldUV()
{
	mPrevWall = NULL;
	mPrevRoom = NULL;
	mPrevRoof = NULL;
	mLevelBBox.Init();
	mRoomOffset = 0;
	mRoofOffset = 0;
}

void PrepareUnfoldUV::Traverse( Level* level )
{
	mPrevWall = NULL;
	mPrevRoom = NULL;

	mRoomOffset += mLevelBBox.GetHeight();

	level->GetBoundingBox( mLevelBBox, true, false );
}

void PrepareUnfoldUV::Traverse( Wall* wall )
{
	if( mPrevWall )	wall->UnfoldUVData().mOffset = mPrevWall->UnfoldUVData().mOffset + 2*mPrevWall->GetStraightLength();
	else			wall->UnfoldUVData().mOffset = 0;

	wall->UnfoldUVData().mPrevWall = mPrevWall;

	mPrevWall  = wall;
}

void PrepareUnfoldUV::Traverse( Room* room )
{
	room->UnfoldUVData().mPrevRoom = mPrevRoom;
	room->UnfoldUVData().mOffset = mRoomOffset;

	mPrevRoom = room;
}

void PrepareUnfoldUV::Traverse( Roof* roof )
{
	roof->UnfoldUVData().mOffsetV = mRoofOffset;

	mRoofOffset += roof->GetRoofMax() - roof->GetRoofMin();
}

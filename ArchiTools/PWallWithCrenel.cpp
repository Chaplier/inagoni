/****************************************************************************************************

		PWallWithCrenel.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#include "PWallWithCrenel.h"

#include "IShTokenStream.h"

#include "PLevel.h"
#include "BuildingPrim.h"
#include "BuildingPrimitiveData.h"

WallWithCrenel::WallWithCrenel(BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2) :
	Wall( data, inLevel, point1, point2 )
{
	fHeight = fData->GetDefaultWallHeight()/3;
	
	// Default values
	mCrenelWidth = 0.5 * fHeight;
	mCrenelHeight = 0.5 * fHeight;
	mCrenelSpacing = 0.5 * fHeight;
	mCrenelOffset = 0;

	mCrenelShape = eRectangular;
}

WallWithCrenel::~WallWithCrenel()
{
}

void WallWithCrenel::CreateWall(Wall **wall, BuildingPrimData* data, Level* inLevel, VPoint* point1, VPoint* point2)
{
	TMCCountedCreateHelper<Wall> result(wall);

	result = new WallWithCrenel(data,inLevel,point1,point2);
	ThrowIfNoMem(result);
}

real32 WallWithCrenel::GetWallTotalHeight() const
{
	if( mCrenelHeight>0 )
		return GetWallHeight() + mCrenelHeight;
	else if( mCrenelHeight<0 )
		return GetWallHeight();
	else return 0;
}

bool WallWithCrenel::GetCrenelHole( float fromX, float limitLength, FlatPolygon& polygon )
{
	const double wallHeight = GetWallHeight();

	const float crenelStartLength = fromX + 0.5*mCrenelSpacing;
	const float crenelEndLength = crenelStartLength + mCrenelWidth;
	const float toX = fromX + mCrenelWidth + mCrenelSpacing + 0.4*mCrenelSpacing;

	const float halfHeight = 0.5 * mCrenelHeight;
	const float oneThirdLength = crenelStartLength + mCrenelWidth/3;
	const float twoThirdLength = crenelEndLength - mCrenelWidth/3;

	if( mCrenelHeight<0 )
	{	// dig the crenel shape into the wall
		switch( mCrenelShape )
		{
		default:
		case eRectangular:
			{
				polygon.SetElemCount(4);
				polygon[0].SetValues( crenelStartLength, wallHeight );
				polygon[1].SetValues( crenelStartLength, wallHeight+mCrenelHeight );
				polygon[2].SetValues( crenelEndLength, wallHeight+mCrenelHeight );
				polygon[3].SetValues( crenelEndLength, wallHeight );
			} break;
		case eSmallTop:
			{
				polygon.SetElemCount(8);
				polygon[0].SetValues( crenelStartLength, wallHeight );
				polygon[1].SetValues( crenelStartLength, wallHeight+halfHeight );
				polygon[2].SetValues( oneThirdLength, wallHeight+halfHeight );
				polygon[3].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[4].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[5].SetValues( twoThirdLength, wallHeight+halfHeight );
				polygon[6].SetValues( crenelEndLength, wallHeight+halfHeight );
				polygon[7].SetValues( crenelEndLength, wallHeight );
			} break;
		case eDoubleTop:
			{
				polygon.SetElemCount(8);
				polygon[0].SetValues( crenelStartLength, wallHeight );
				polygon[1].SetValues( crenelStartLength, wallHeight+mCrenelHeight );
				polygon[2].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[3].SetValues( oneThirdLength, wallHeight+halfHeight );
				polygon[4].SetValues( twoThirdLength, wallHeight+halfHeight );
				polygon[5].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[6].SetValues( crenelEndLength, wallHeight+mCrenelHeight );
				polygon[7].SetValues( crenelEndLength, wallHeight );
			} break;
		case eSlopeSides:
			{
				polygon.SetElemCount(4);
				polygon[0].SetValues( crenelStartLength, wallHeight );
				polygon[1].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[2].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[3].SetValues( crenelEndLength, wallHeight );
			} break;
		case eSlopeTop:
			{
				polygon.SetElemCount(6);
				polygon[0].SetValues( crenelStartLength, wallHeight );
				polygon[1].SetValues( crenelStartLength, wallHeight+halfHeight );
				polygon[2].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[3].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[4].SetValues( crenelEndLength, wallHeight+halfHeight );
				polygon[5].SetValues( crenelEndLength, wallHeight );
			} break;
		}
	}
	else if( mCrenelHeight>0 )
	{
		switch( mCrenelShape )
		{
		default:
		case eRectangular:
			{	// left and right rectangular shapes
				polygon.SetElemCount(8);
				polygon[0].SetValues( fromX, wallHeight+mCrenelHeight );
				polygon[1].SetValues( fromX, wallHeight );
				polygon[2].SetValues( crenelStartLength, wallHeight );
				polygon[3].SetValues( crenelStartLength, wallHeight+mCrenelHeight );
				polygon[4].SetValues( crenelEndLength, wallHeight+mCrenelHeight );
				polygon[5].SetValues( crenelEndLength, wallHeight );
				polygon[6].SetValues( toX, wallHeight );
				polygon[7].SetValues( toX, wallHeight+mCrenelHeight );
			} break;
		case eSmallTop:
			{
				polygon.SetElemCount(12);
				polygon[0].SetValues( fromX, wallHeight+mCrenelHeight );
				polygon[1].SetValues( fromX, wallHeight );
				polygon[2].SetValues( crenelStartLength, wallHeight );
				polygon[3].SetValues( crenelStartLength, wallHeight+halfHeight );
				polygon[4].SetValues( oneThirdLength, wallHeight+halfHeight );
				polygon[5].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[6].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[7].SetValues( twoThirdLength, wallHeight+halfHeight );
				polygon[8].SetValues( crenelEndLength, wallHeight+halfHeight );
				polygon[9].SetValues( crenelEndLength, wallHeight );
				polygon[10].SetValues( toX, wallHeight );
				polygon[11].SetValues( toX, wallHeight+mCrenelHeight );
			} break;
		case eDoubleTop:
			{
				polygon.SetElemCount(12);
				polygon[0].SetValues( fromX, wallHeight+mCrenelHeight );
				polygon[1].SetValues( fromX, wallHeight );
				polygon[2].SetValues( crenelStartLength, wallHeight );
				polygon[3].SetValues( crenelStartLength, wallHeight+mCrenelHeight );
				polygon[4].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[5].SetValues( oneThirdLength, wallHeight+halfHeight);
				polygon[6].SetValues( twoThirdLength, wallHeight+halfHeight );
				polygon[7].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[8].SetValues( crenelEndLength, wallHeight+mCrenelHeight );
				polygon[9].SetValues( crenelEndLength, wallHeight );
				polygon[10].SetValues( toX, wallHeight );
				polygon[11].SetValues( toX, wallHeight+mCrenelHeight );
			} break;
		case eSlopeSides:
			{
				polygon.SetElemCount(8);
				polygon[0].SetValues( fromX, wallHeight+mCrenelHeight );
				polygon[1].SetValues( fromX, wallHeight );
				polygon[2].SetValues( crenelStartLength, wallHeight );
				polygon[3].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[4].SetValues( twoThirdLength, wallHeight+mCrenelHeight );
				polygon[5].SetValues( crenelEndLength, wallHeight );
				polygon[6].SetValues( toX, wallHeight );
				polygon[7].SetValues( toX, wallHeight+mCrenelHeight );
			} break;
		case eSlopeTop:
			{
				polygon.SetElemCount(10);
				polygon[0].SetValues( fromX, wallHeight+mCrenelHeight );
				polygon[1].SetValues( fromX, wallHeight );
				polygon[2].SetValues( crenelStartLength, wallHeight );
				polygon[3].SetValues( crenelStartLength, wallHeight+halfHeight );
				polygon[4].SetValues( oneThirdLength, wallHeight+mCrenelHeight );
				polygon[5].SetValues( twoThirdLength, wallHeight+mCrenelHeight);
				polygon[6].SetValues( crenelEndLength, wallHeight+halfHeight );
				polygon[7].SetValues( crenelEndLength, wallHeight );
				polygon[8].SetValues( toX, wallHeight );
				polygon[9].SetValues( toX, wallHeight+mCrenelHeight );
			} break;
		}
	}
	else 
		return false;

	polygon.mIsHole = true;

	return true;
}

void WallWithCrenel::AddExtraPolygones( BooleanPolygon& booleanPoly )
{
	const real32 stepLength = mCrenelSpacing+mCrenelWidth;
	real32 offset = fmod(mCrenelOffset,stepLength);
	if( offset>0.5*stepLength )
		offset-=stepLength;

	const float limitLength = GetStraightLength();
	const int stepCount = (int)(limitLength/(stepLength)) + 1; // +1 for the extra incomplete step

	int firstStep = 0;
	if( offset>0 )
	{
		firstStep = -1;
	}

	for( int iStep=firstStep ; iStep<stepCount ; iStep++ )
	{
		FlatPolygon crenelPolygon;
		if( GetCrenelHole( iStep*stepLength+offset, limitLength, crenelPolygon ) )
			booleanPoly.AddPolygon( crenelPolygon );
	}

}

///////////////////////////////////////////////////////////////////////////
//
//

MCCOMErr WallWithCrenel::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('WCre');
	if (result) return result;

	Wall::Write(stream);

	// Crenel Depth
	result=stream->PutKeyword(eWallCrenelHeight);
	if (result) return result;
	result=stream->PutQuickFix(mCrenelHeight);
	if (result) return result;

	// Crenel Width
	result=stream->PutKeyword(eWallCrenelWidth);
	if (result) return result;
	result=stream->PutQuickFix(mCrenelWidth);
	if (result) return result;

	// Crenel Spacing
	result=stream->PutKeyword(eWallCrenelSpacing);
	if (result) return result;
	result=stream->PutQuickFix(mCrenelSpacing);
	if (result) return result;

	// Crenel Offset
	result=stream->PutKeyword(eWallCrenelOffset);
	if (result) return result;
	result=stream->PutQuickFix(mCrenelOffset);
	if (result) return result;

	// Crenel Shape
	stream->PutInt32Attribute(eWallCrenelShape, mCrenelShape);

	result=stream->PutEnd();
	return result;
}

MCCOMErr WallWithCrenel::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	Wall::Read(stream);

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		switch( keyword )
		{
		case eWallCrenelHeight:
			{
				result = stream->GetQuickFix(&mCrenelHeight);
				if (result) return result;
			} break;
		case eWallCrenelWidth:
			{
				result = stream->GetQuickFix(&mCrenelWidth);
				if (result) return result;
			} break;
		case eWallCrenelSpacing:
			{
				result = stream->GetQuickFix(&mCrenelSpacing);
				if (result) return result;
			} break;
		case eWallCrenelOffset:
			{
				result = stream->GetQuickFix(&mCrenelOffset);
				if (result) return result;
			} break;
		case eWallCrenelShape:
			{
				mCrenelShape = (ECrenelShape)stream->GetInt32Token();
			} break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////

void WallWithCrenel::Clone(Wall** newWall, Level* inLevel, VPoint* point0, VPoint* point1, const ECloneChildrenMode cloneMode)
{
	WallWithCrenel::CreateWall(newWall, inLevel->GetPrimitiveData(), inLevel,
		point0, point1);

	(*newWall)->CopyFrom( this, cloneMode );
}

void WallWithCrenel::CopyFrom( Wall* otherWall, const ECloneChildrenMode cloneMode )
{
	WallWithCrenel* otherWallWithCrenel = otherWall->GetWallWithCrenel();
	if( otherWallWithCrenel )
	{
		mCrenelHeight = otherWallWithCrenel->mCrenelHeight;
		mCrenelWidth = otherWallWithCrenel->mCrenelWidth;
		mCrenelSpacing = otherWallWithCrenel->mCrenelSpacing;
		mCrenelOffset = otherWallWithCrenel->mCrenelOffset;
		mCrenelShape = otherWallWithCrenel->mCrenelShape;
	}
	Wall::CopyFrom(otherWall, cloneMode);
}

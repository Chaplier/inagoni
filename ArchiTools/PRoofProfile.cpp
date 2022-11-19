/****************************************************************************************************

		PRoofProfile.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/16/2004

****************************************************************************************************/

#include "PRoofProfile.h"
#include "IShTokenStream.h"
#include "MCCountedPtrHelper.h"
#include "PRoof.h"
#include "PLevel.h"
#include "MiscComUtilsImpl.h"
#include "BuildingDef.h"

RoofProfile::RoofProfile(Roof* roof, const int32 type)
{
}

void RoofProfile::CreateProfile(RoofProfile **profile, Roof* roof, const int32 type)
{
	TMCCountedCreateHelper<RoofProfile> result(profile);

	result = new RoofProfile(roof,type);
	ThrowIfNoMem(result);
}

MCCOMErr RoofProfile::Read(IShTokenStream* stream, Roof* roof)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'BoOu':
			{
				result = ReadArray(stream,fBotProfilePoints, roof, false,true);
				if (result) return result;
			} break;
		case 'ToOu':
			{
				result = ReadArray(stream,fTopProfilePoints, roof, true,true);
				if (result) return result;
			} break;
		case 'BoIn':
			{
				result = ReadArray(stream,fBotInsidePoints, roof, false,false);
				if (result) return result;
			} break;
		case 'ToIn':
			{
				result = ReadArray(stream,fTopInsidePoints, roof, true,false);
				if (result) return result;
			} break;
		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

MCCOMErr RoofProfile::ReadArray(IShTokenStream* stream, TMCCountedPtrArray<ProfilePoint>& array, 
								Roof* roof, const boolean onTop, const boolean outside)
{ 
	array.ArrayFree();

	int8 token[256];

	int32 index=0;

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
		case 'Coun':
			{
				int32 ptCount=stream->GetInt32Token();
				array.SetElemCount(ptCount);
				index=0;
			} break;
		case 'Poin':  
			{	
				ProfilePoint::CreateProfilePoint(array.Pointer(index),roof->GetData(), TVector2::kZero,roof,onTop,outside);
				array[index++]->Read(stream);
			} break;
		default:
			stream->SkipTokenData();
			break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

void RoofProfile::Copy(RoofProfile* fromProfile, Roof* roof)
{
	fBotProfilePoints.ArrayFree();
	fTopProfilePoints.ArrayFree();
	fBotInsidePoints.ArrayFree();
	fTopInsidePoints.ArrayFree();
	{
		const int32 count = fromProfile->fBotProfilePoints.GetElemCount();
		fBotProfilePoints.SetElemCount(count);
		for(int32 iPt=0 ; iPt<count ; iPt++)
		{
			ProfilePoint::CreateProfilePoint(fBotProfilePoints.Pointer(iPt), roof->GetData(), fromProfile->fBotProfilePoints[iPt]->Position(),roof,false,true);
		}
	}
	{
		const int32 count = fromProfile->fTopProfilePoints.GetElemCount();
		fTopProfilePoints.SetElemCount(count);
		for(int32 iPt=0 ; iPt<count ; iPt++)
		{
			ProfilePoint::CreateProfilePoint(fTopProfilePoints.Pointer(iPt), roof->GetData(), fromProfile->fTopProfilePoints[iPt]->Position(),roof,true,true);
		}
	}
	{
		const int32 count = fromProfile->fBotInsidePoints.GetElemCount();
		fBotInsidePoints.SetElemCount(count);
		for(int32 iPt=0 ; iPt<count ; iPt++)
		{
			ProfilePoint::CreateProfilePoint(fBotInsidePoints.Pointer(iPt), roof->GetData(), fromProfile->fBotInsidePoints[iPt]->Position(),roof,false,false);
		}
	}
	{
		const int32 count = fromProfile->fTopInsidePoints.GetElemCount();
		fTopInsidePoints.SetElemCount(count);
		for(int32 iPt=0 ; iPt<count ; iPt++)
		{
			ProfilePoint::CreateProfilePoint(fTopInsidePoints.Pointer(iPt), roof->GetData(), fromProfile->fTopInsidePoints[iPt]->Position(),roof,true,false);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//
ProfilePoint::ProfilePoint(BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean onSpine, const boolean outside )
: CommonPoint(pos,data)
{
	MY_ASSERT(fData); // Needed for the name

	if(onSpine) SetFlag(eTopProfileFlag);
	else		ClearFlag(eTopProfileFlag);

	if(outside) SetFlag(eOutProfileFlag);
	else		ClearFlag(eOutProfileFlag);

	fRoof = onRoof;

	// Default name
	CWhileInCompResFile myRes(kRID_GeometricPrimitiveFamilyID, 'BuiP');
	TMCDynamicString objectName;
	gResourceUtilities->GetIndString(objectName, kPrimitiveStrings , 14);
	SetName(fData->fDictionary, objectName);
}

ProfilePoint::~ProfilePoint()
{
}

void ProfilePoint::CreateProfilePoint(ProfilePoint **point, BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean onSpine, const boolean outside)
{
	TMCCountedCreateHelper<ProfilePoint> result(point);

	result = new ProfilePoint(data,pos,onRoof,onSpine,outside);
	ThrowIfNoMem(result);
}

void ProfilePoint::DeleteProfilePoint()
{
	fRoof->RemoveProfilePointReferences(this);
	fRoof = NULL;
}

void ProfilePoint::SetSelection(const boolean select)
{
	if( select != Selected() )
	{
		if(select)
		{
			SetFlag(eIsSelected);
		//	fRoof->SelectIfPossible();
		}
		else
		{
			ClearFlag(eIsSelected);
		//	fRoof->ClearFlag(eIsSelected);
		}

		fData->InvalidateStatus();
	}
}

inline void	ProfilePoint::InvalidateTessellation(const boolean extraInvalidation)
{
	if(!Flag(eOutProfileFlag)) // Inside points modify the geometry of the walls under
		fRoof->GetLevel()->InvalidateTessellation();
	else
		fRoof->ClearFlag(eRoofTessellated);
}

void ProfilePoint::Clone(ProfilePoint** newPoint, Roof* roof)
{
	if(roof)
	{
		MY_ASSERT( roof->GetData() == fData ); // There maybe different in some cases. If it happens, see the one we want to keep
		// For now, I keep the data on the roof in case we copy from one primitive to another (I don't know if it's possible)
		ProfilePoint::CreateProfilePoint(newPoint,fData,fPosition,roof,Flag(eTopProfileFlag),Flag(eOutProfileFlag));
	}	
	else
		ProfilePoint::CreateProfilePoint(newPoint,fData,fPosition,NULL,Flag(eTopProfileFlag),Flag(eOutProfileFlag));

	TMCCountedPtr<ProfilePoint> pointPtr;	
	pointPtr = *newPoint;	
	pointPtr->SetFlags(fFlags);
	pointPtr->SetNamePtr(fName);

	pointPtr->ClearFlag(eIsTargeted);
}


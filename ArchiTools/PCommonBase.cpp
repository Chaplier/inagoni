/****************************************************************************************************

		PCommonBase.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/6/2004

****************************************************************************************************/

#include "PCommonBase.h"
#include "IShTokenStream.h"
#include "BuildingPrimitiveData.h"

void CommonBase::Copy(CommonBase* copyFrom)
{
	fFlags = copyFrom->fFlags;
	fName = copyFrom->fName;
}

const TMCString& CommonBase::GetName() const
{
	if( fName )
		return fName->GetName();
	return kNullString;
}


MCCOMErr CommonBase::Write(IShTokenStream* stream)
{
	// Flags
	stream->PutInt32Attribute('Flag', fFlags);

	// Name
	if(fName)
		fName->Write(stream);

	return MC_S_OK;
}

MCCOMErr CommonBase::Read(IShTokenStream* stream, const int32 keyword, BuildingPrimData* data)
{ 
	MCCOMErr result=MC_S_OK;

	switch (keyword) 
	{
	case 'Flag':
		{
			fFlags = stream->GetInt32Token();
			if (result) return result;
		} break;
	case 'Name':
	{
		char name[256];
		result=stream->GetString(name);
		if (result) return result;
		TMCDynamicString nameStr;
		nameStr.FromCPtr(name);
		SetName(data->fDictionary, nameStr);		
	} break;
	default:
		stream->SkipTokenData();
		break;
	}

	return result;
}


////////////////////////////////////////////////////////////////////

CommonPoint::CommonPoint(const TVector2& pos, BuildingPrimData* data)
{
	fPosition = pos;
	fData = data;
}

CommonPoint::~CommonPoint()
{
}

void CommonPoint::Copy(CommonPoint* copyFrom)
{
	CommonBase::Copy(copyFrom);

	fPosition = copyFrom->fPosition;
	fData = copyFrom->fData;
}

MCCOMErr CommonPoint::Write(IShTokenStream* stream)
{
	MCCOMErr result=stream->PutKeywordAndBegin('Poin');
	if (result) return result;

	result=stream->PutKeyword('Posi');
	if (result) return result;
	result = stream->PutPoint2D(fPosition[0],fPosition[1]);
	if (result) return result;

	// Common
	result=CommonBase::Write(stream);
	if (result) return result;

	result=stream->PutEnd();
	return result;
}

MCCOMErr CommonPoint::Read(IShTokenStream* stream)
{ 
	int8 token[256];

	MCCOMErr result=stream->GetNextToken(token);
	if (result) return result;

	if (token[0] != '{') return MC_S_FALSE;

	result=stream->GetNextToken(token);
	if (result) return result;

	while (!stream->IsEndToken(token)) 
	{
		int32 keyword=0;
		stream->CompactAttribute(token,&keyword);

		switch (keyword) 
		{
			case 'Posi':
			{
				result = stream->GetPoint2D(&fPosition[0],&fPosition[1]);
				if (result) return result;
			} break;

			default:
				CommonBase::Read(stream,keyword,fData);
				break;
		}

		result=stream->GetNextToken(token);
		if (result) return result;
	}

	return result;
}

/****************************************************************************************************

		PNames.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	8/13/2004

****************************************************************************************************/

#include "PNames.h"
#include "IShTokenStream.h"

Name::Name(const TMCString& name) 
{
	fName=name;

	fUsed=true;
	fNext=NULL;
}	

boolean Name::Ordered(Name* after)
{
	return true;
/* TO DO
	int8* cBefore=this->fName;
	int8* cAfter=after->fName;

	for (;;)
	{
		if (!(*cBefore)) return true;
		if (!(*cAfter)) return false;
		int32 c1=ToUpper(*cBefore);
		int32 c2=ToUpper(*cAfter);
		if (c1 > c2) return false;
		if (c1 < c2) return true;
		++cBefore;
		++cAfter;
	}*/
}

MCCOMErr Name::Write(IShTokenStream* stream) 
{
	MCCOMErr result;
	if (fName.Length()) 
	{
		result=stream->PutKeyword('Name');
		if (result)
			return result;

		result=stream->PutString(fName.StrGet());
		if (result)
			return result;
	}
	return MC_S_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//
// NameChainedList
//

NameChainedList::NameChainedList()
{
	fFirstName=nil;
	fNameCount=0;
}

NameChainedList::~NameChainedList()
{
}

Name* NameChainedList::GetOrAdd(const TMCString& name)
{
	Name* find = Find(name);
	if(find)
		return find;

	// The name wasn't found in the list, add a new one

	TMCCountedPtr<Name> newName;
	Name::Create(&newName, name);
	
	boolean ordered=false;
//	if (fFirstName)
//		ordered=res->Ordered(fFirstName);
//	else ordered=true;
//	if (ordered)
	{
		newName->SetNext(fFirstName);
		fFirstName=newName;
	}
/*	else
	{
		TVMName* before=fFirstName;
		TVMName* after;
		for (;;)
		{
			after=before->Next();
			if (!after)
			{
				before->SetNext(res);
				break;
			}
			if (res->Ordered(after))
			{
				before->SetNext(res);
				res->SetNext(after);
				break;
			}
			
			before=after;
		}
	}*/

	fNameCount++;
	return newName;
}

Name* NameChainedList::Find(const TMCString& name) const
{
	for( Name* current=fFirstName ; current ; current=current->GetNext() ) 
	{
		if( current->GetName()==name ) 
		{
			return current;
		}
	}

	return NULL;
}

const TMCString& NameChainedList::GetNameStr(int32 i) const
{
	for( Name* current=fFirstName ; current ; current=current->GetNext() )
	{
		if (i-- <= 0)
			return current->GetName();
	}
	return kNullString;
}

Name* NameChainedList::GetNamePtr(int32 i) const
{
	for( Name* current=fFirstName ; current ; current=current->GetNext() )
	{
		if (i-- <= 0)
			return current;
	}
	return NULL;
}

void NameChainedList::ClearUsed()
{
	for( Name* current=fFirstName ; current ; current=current->GetNext() )
	{
		current->SetUsed(false);
	}
}

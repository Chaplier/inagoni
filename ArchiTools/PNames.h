/****************************************************************************************************

		PNames.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	8/13/2004

****************************************************************************************************/

#ifndef __PNames__
#define __PNames__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedPtr.h"
#include "MCCountedPtrHelper.h"
#include "MCCountedObject.h"
#include "MCString.h"

struct IShTokenStream;

class Name : public TMCCountedObject
{
protected:
	Name(const TMCString& name);
	virtual ~Name(){}
public:
	static void Create(Name** outName, const TMCString& name) 
	{ 
		TMCCountedCreateHelper<Name> result(outName);
		result= new Name(name);
	}
	
	const TMCString& 	GetName	() const	{return fName;}
	Name*		GetNext	() const			{return fNext;}
	void		SetNext	(Name* next)		{fNext= next;}
	boolean		Ordered	(Name* after);
	boolean		GetUsed	() const			{return fUsed;}
	void		SetUsed	(boolean value)		{fUsed= value;}

	MCCOMErr	Write	(IShTokenStream* stream);

private:
	TMCDynamicString		fName;
	boolean					fUsed;
	TMCCountedPtr<Name>		fNext;
};

class NameChainedList
{
public:
	NameChainedList();
	virtual ~NameChainedList();

	Name*		GetFirstName()				{return fFirstName;}
	void		SetFirstName(Name* name)	{fFirstName= name;}
	int32		NameCount()	const			{return fNameCount;}

	const TMCString&	GetNameStr(int32 i) const;
	Name*				GetNamePtr(int32 i) const;
	Name*				GetOrAdd(const TMCString& name);
	Name*				Find(const TMCString& name) const;
	void				ClearUsed();

private:
	TMCCountedPtr<Name>	fFirstName;
	int32				fNameCount;
};








#endif

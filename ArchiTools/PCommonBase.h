/****************************************************************************************************

		PCommonBase.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/6/2004

****************************************************************************************************/

#ifndef __PCommonBase__
#define __PCommonBase__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCCompObj.h"
#include "MCCountedObject.h"
#include "BasicCOMImplementations.h"
#include "MCCountedPtrArray.h"

#include "PNames.h"

struct BuildingPrimData;

enum ECommonFlags
{
	eIsSelected = 0x00000001,
	eIsTargeted = 0x00000002,
	eIsHidden = 0x00000004,
	eWasSelected = 0x00000008, // used for the Marquee selection
	eSnapedPosition = 0x00000010,
	eReserved3 = 0x00000020,
	eReserved4 = 0x00000040,
	eReserved5 = 0x00000080,
	eReserved6 = 0x00000100,
	eReserved7 = 0x00000200,
	eReserved8 = 0x00000400,
	eReserved9 = 0x00000800,
	eReserved10= 0x00001000,
	eReserved11= 0x00002000,
	eReserved12= 0x00004000,
	eReserved13= 0x00008000,
};


class CommonBase : public TMCCountedObject
{
public:
	CommonBase(){fFlags=0;}

	void Copy(CommonBase* copyFrom);

	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream, const int32 keyword, BuildingPrimData* data); 

	// Flags access methods
	inline void			ClearFlag(const int32 flag)	{fFlags&=~flag;}
	inline void			SetFlag(const int32 flag)	{fFlags|=flag;}
	inline boolean		Flag(const int32 flag)	const	{return ((fFlags&flag) != 0 );}
	inline int32		GetFlags()const{return fFlags;}
	inline void			SetFlags(const int32 flags){fFlags=flags;}
	inline boolean		Selected()const{return Flag(eIsSelected);}
	inline boolean		Targeted()const{return Flag(eIsTargeted);}
	inline boolean		Hidden()const{return Flag(eIsHidden);}
	inline void			RestoreSelection(){	Flag(eWasSelected)? SetFlag(eIsSelected):ClearFlag(eIsSelected);}

	// Each type of object need to decide if it should select or
	// deselect other objects arround
	virtual void		SetSelection(const boolean select)
	{
		if(select)
		{
			SetFlag(eIsSelected);
			ClearFlag(eIsTargeted);
		}
		else
			ClearFlag(eIsSelected);
	}

	inline void			InitMarqueeSelection(){Selected()?SetFlag(eWasSelected):ClearFlag(eWasSelected);}

	// Name methods
	const TMCString&	GetName() const ;
	inline void			SetName(NameChainedList& dictionary, const TMCString& name) { fName=dictionary.GetOrAdd(name);}
	inline Name*		GetNamePtr() const  { return fName;}
	inline void			SetNamePtr(Name* name) { fName=name;}

protected:
	virtual ~CommonBase() {}

	int32 fFlags;
	TMCCountedPtr<Name> fName;
};

///////////////////////////////////////////////////////////////////////
//
//	CommonPoint
//
struct BuildingPrimData;
#include "Vector2.h"
#include "MCClassArray.h"

class CommonPoint :	public CommonBase
{
public:
	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	void Copy(CommonPoint* copyFrom);

	inline void			SetPosition( const TVector2& pos){ fPosition=pos; }
	inline void			OffsetPosition( const TVector2& offset){ fPosition+=offset; }
	inline TVector2&	GetPosition() { return fPosition; }
	inline const TVector2&	Position() const { return fPosition; }

	virtual void		GetSurroundingPoints(TMCClassArray<TVector2>& directions){}
	virtual void		GetSurroundingPoints(TMCCountedPtrArray<CommonPoint>& pointsArround){}

	virtual void		InvalidateTessellation(const boolean extraInvalidation=false){}

	// Usually, data is set at coonstruction time. But in some cases, we may want to overwrite it.
	void				SetData(BuildingPrimData* data){fData = data;}
	BuildingPrimData* 	GetData(){return fData;}
protected:

	CommonPoint(const TVector2& pos, BuildingPrimData* data);
	virtual ~CommonPoint();

	TVector2			fPosition;

	BuildingPrimData*	fData;
};


#endif

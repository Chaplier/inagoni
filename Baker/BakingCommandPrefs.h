/****************************************************************************************************

		BakingCommandPrefs.h
		Copyright: (c) 2008 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	11/23/2008

****************************************************************************************************/

#ifndef __BakingCommandPrefs__
#define __BakingCommandPrefs__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BakerDef.h"

#include "BakingCommand.h"

//#include "Basic3DCOMImplementations.h"
#include "BasicPrefsComponent.h"

extern const MCGUID CLSID_BakingCommandPrefs;

// Contains all the elements of the PMap
struct PrefsPMap : public BakingPMap
{
};
 
class BakingCommandPrefs : public TBasicPrefsComponent
{
	public :  
		BakingCommandPrefs();
		virtual ~BakingCommandPrefs();

		STANDARD_RELEASE;

		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer	(){ return &fPMap; }
		virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(PrefsPMap);}
		virtual MCCOMErr MCCOMAPI ExtensionDataChanged		(){return MC_S_OK;}
		virtual int16	 MCCOMAPI GetResID					(){ return 851; }

	private :

		// Data

		PrefsPMap		fPMap;
};

#endif

/****************************************************************************************************

		SerialNumber.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/9/2004

****************************************************************************************************/

#ifndef __SerialNumber__
#define __SerialNumber__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCString.h"
#include "MCClassArray.h"
#include "IMCFile.h"
#include "MCCountedPtr.h"

#include <vector>


struct KeyData
{
	KeyData(){}
	KeyData(const TMCString& info,
			const TMCString& fileName,
			const int32		modulo,
			const int32		resID,
			const IDType	familyID,
			const IDType	classID)
	{mProductInfo=info;mFileName=fileName;mModulo=modulo;mResID=resID;mFamilyID=familyID;mClassID=classID;}

	TMCString15 mProductInfo;
	TMCDynamicString mFileName;
	int32 mModulo;

	// To find the dialog in the .dta file
	int32	mResID;
	IDType	mFamilyID;
	IDType	mClassID;
};

class SerialNumber
{
public:
	// Initialize the serial checker
	SerialNumber(TMCClassArray<KeyData>& keyDatas);

	// Returns true if a serial number is valid
	boolean CheckSerial();

	// Utility: create a list of serial in "Licenses.txt"
	void	CreateKeyList(const KeyData& data, int32 createKeyCount = 1000);

	// Some license are banned (illegal ones)
	void AddBannedLicense( const TMCString15& block0,const TMCString15& block1, const TMCString15& block2, const TMCString15& block3 ); 

	static boolean IsSerialValid(){return gSerialNumberValid;}

protected:

	boolean CheckSerialInternal();

	boolean CheckSerialForData(const KeyData& data);

	void	LoadSerialNumber(const KeyData& data);
	void	SaveSerialNumber(const KeyData& data);
	void	GetSerialFile(IMCFile** file, const KeyData& data);

	boolean	AskFoKey();

	boolean IsKeyValid( const TMCClassArray<TMCString15>& keys, const KeyData& data ) const;
	boolean	IsBanned( const TMCClassArray<TMCString15>& keys ) const;

	// 4 strings with the 4 pieces of the seriql numbers
	TMCClassArray<TMCString15> fLicenseInfos;
	
	std::vector< TMCClassArray<TMCString15> > mBannedLicenses;

	// Usualy only 1 element: countains the information to 
	// read the serial number (where to find it and what to check)
	TMCClassArray<KeyData> fKeyDatas;

private:
	void BuildKey(int32 primary, TMCString& key, const KeyData& data);

	static boolean gSerialNumberValid;
};

class ExceptionLog
{
public:
	ExceptionLog(const TMCString& pluginName);
	virtual ~ExceptionLog();

	void HandleException(TMCException& exception, const TMCString& methodName);
	void HandleException(const TMCString& methodName);

protected:
	void NextLine();

	TMCCountedPtr<IMCFile> mFile;
};



#endif

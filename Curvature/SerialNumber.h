/****************************************************************************************************

		SerialNumber.h
		Copyright: (c) 2005 Alan Stafford. All rights reserved.

		Author:	Alan
		Date:	6/10/2005

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

class SerialNumber
{
public:
	SerialNumber();

	boolean CheckSerial();
protected:
	void	LoadSerialNumber();
	void	SaveSerialNumber();
	void	GetSerialFile(IMCFile** file);

	void	CreateKeyList();

	boolean	AskFoKey();

	TMCClassArray<TMCString15> fLicenseInfos;
};








#endif

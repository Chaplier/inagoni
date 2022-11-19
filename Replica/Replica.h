/****************************************************************************************************

		Instanciator.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	9/17/2004

****************************************************************************************************/

#ifndef __Instanciator__
#define __Instanciator__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"

boolean IsSerialValid();

class DemoLimiter
{
public:
	DemoLimiter() : m_count(0), m_limit(4) {}

	bool CanAdd()
	{
		if(IsSerialValid())
		{
			return true;
		}

		if(m_count++<m_limit)
		{
			return true;
		}

		return false;
	}
private:
	int m_limit;
	int m_count;
};

#endif

/****************************************************************************************************

		ShaderFactory.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/31/2004

****************************************************************************************************/

#ifndef __ShaderFactory__
#define __ShaderFactory__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCClassArray.h"
#include "I3DShShader.h"
#include "MCCountedPtr.h"

// Keep track of the new shaders to avoid adding to much of them
class MasterAndValue
{
public:
	MasterAndValue(){fValue=-1;}

	TMCCountedPtr<I3DShMasterShader>	fMaster;
	real32								fValue;
};

class ShaderFactory : public TMCClassArray<MasterAndValue>
{
public:

	ShaderFactory(){fMin=0;fMax=1;fInc=1.0;fMaxCount = 0;}

	void Init(const int32 maxCount, const TVector2& minMax, I3DShMasterShader* basedOnShader);

	// Look for an existing shader in the array, if none is found, create a new one
	I3DShMasterShader* GetMasterShader(const real32 value,
									  int32& nameCouter,
									  I3DShMasterShader* otherMasterShader,
									  I3DShScene* scene);

	I3DShMasterShader* BasedOnNoAddRef()const{return fBasedOnShader;}

protected:

	inline int32 Index(const real32 value) const ;
	inline real32 FlattenValue(const real32 value) const ;

	real32								fInc; // =(Max-Min)/count
	real32								fMin;
	real32								fMax;
	int32								fMaxCount;
	TMCCountedPtr<I3DShMasterShader>	fBasedOnShader;
};

inline int32 ShaderFactory::Index(const real32 value) const 
{
	int32 index = (int32)((value-fMin)/fInc);
	if(index<0) return 0;
	else if(index>=fMaxCount) return (fMaxCount-1);
	else return index;
}

inline real32 ShaderFactory::FlattenValue(const real32 index) const 
{
	return fMin+(index+.5)*fInc;
}









#endif

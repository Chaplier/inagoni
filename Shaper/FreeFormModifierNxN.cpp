/****************************************************************************************************

		FreeFormModifierNxN.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "FreeFormModifierNxN.h"

#include "Copyright.h"
#include "Shaper.h"
#include "IShUtilities.h"
#include "IShComponent.h"
#include "MFPartTypes.h"

const MCGUID CLSID_FreeFormModifierNxN(R_CLSID_FreeFormModifierNxN);

FreeFormModifierPMap::FreeFormModifierPMap()
{
	Clear();
}

void FreeFormModifierPMap::Clear()
{
	mXCount = 3;
	mYCount = 3;
	mZCount = 3;

	mBoundingBox.Init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////:

FreeFormModifierNxN::FreeFormModifierNxN()
{
}

MCCOMErr FreeFormModifierNxN::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_FreeFormModifierNxN))
	{
		TMCCountedGetHelper<FreeFormModifierNxN> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	if (MCIsEqualIID(riid, IID_IExStreamIO)) 
	{
		TMCCountedGetHelper<IExStreamIO>	result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	return FreeFormModifierBase::QueryInterface(riid, ppvObj);
}

int16 FreeFormModifierNxN::GetResID()
{
	return 810;		// This is the view ID in the resource file
}

boolean FreeFormModifierNxN::Preprocess()
{
	if(!mIsValid)
	{
		if( mPMap.mXCount<2 ) mPMap.mXCount=2;
		if( mPMap.mYCount<2 ) mPMap.mYCount=2;
		if( mPMap.mZCount<2 ) mPMap.mZCount=2;

		// Initalize the plane positions
		SetPlanes( mPMap.mXCount, mPMap.mYCount, mPMap.mZCount );

		FreeFormModifierBase::Preprocess();
	}

	return true;
}

void FreeFormModifierNxN::Clone(IExDataExchanger** res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	FreeFormModifierNxN* clone = new FreeFormModifierNxN();
	result = (IExDataExchanger*)clone;

	if (clone)
	{
	    clone->mPMap=mPMap; // copy the FreeFormModifierData
		CopyComponentExtraData( result );
		CloneData(clone, pUnkOuter);
//	clone->SetControllingUnknown(pUnkOuter);
	}
} 

//------------------------------------------------------
MCCOMErr FreeFormModifierNxN::CopyComponentExtraData (IExDataExchanger* dest)
{
	TMCCountedPtr<FreeFormModifierNxN> destModifier;
	dest->QueryInterface(CLSID_FreeFormModifierNxN, (void**)&destModifier);
	if (destModifier)
	{
		destModifier->mOffsetArray = mOffsetArray;
		CopyComponentExtraDataBase( destModifier );
	}
	return MC_S_OK;
}

MCCOMErr FreeFormModifierNxN::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	int8 token[256];
	MCCOMErr result = MC_S_OK;

	result = stream->GetBegin();
	if (result) return result;

	result=stream->GetNextToken(token);
	if (result) return result;

//	TMCiostream& ioStream = stream->GetStream();

	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();
	SetPlanes( xCount, yCount, zCount);

	size_t iX=0;
	size_t iY=0;
	size_t iZ=0;

	while (!stream->IsEndToken(token))
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword)
		{
		case 'Hndl':
			{	// Read the handle array
				result = stream->GetBegin();
				if (result) return result;

				result=stream->GetNextToken(token);
				if (result) return result;

				while (!stream->IsEndToken(token))
				{
					int32 keyword2;
					stream->CompactAttribute(token,&keyword2);
					if( keyword2=='hPos' )
					{
						// Read the position of the handle
						TVector3 newHandle;
						stream->GetVector3Token( newHandle );
						mOffsetArray[iX][iY][iZ] = newHandle;
						// Increase the conter
						iZ++;
						if( iZ==zCount )
						{
							iZ=0;
							iY++;
							if( iY==yCount )
							{
								iY=0;
								iX++;
							}
						}
					}
					result=stream->GetNextToken(token);
				}

			}	break;
		default:
			readUnknown(keyword, stream, privData);
			break;
		};
		result=stream->GetNextToken(token);
		if (result)
		{
			return result;
		}
	}

	return MC_S_OK;	
}

MCCOMErr FreeFormModifierNxN::Write (IShTokenStream* stream)
{
	TMCiostream& ioStream = stream->GetStream();

	MCCOMErr result=stream->PutKeywordAndBegin('Hndl');
	if (result) return result;

	// Write the mOffsetArray array
	const size_t xCount = GetXCount();
	const size_t yCount = GetYCount();
	const size_t zCount = GetZCount();

	for( size_t iX=0 ; iX<xCount ; iX++)
	{
		for( size_t iY=0 ; iY<yCount ; iY++)
		{
			for( size_t iZ=0 ; iZ<zCount ; iZ++)
			{
				stream->PutVector3Attribute('hPos', GetOffset(iX, iY, iZ) );
			}
		}
	}

	result=stream->PutEnd();

	return result;
}

//paramComp->SetParameter( 'Sh00' , &ref1ParamComp);

void FreeFormModifierNxN::SetPlanes( size_t newXPlaneCount, size_t newYPlaneCount, size_t newZPlaneCount )
{
	// If some planes are missing, add them
	bool setZeroValue = false;
	const size_t prevXPlaneCount = mOffsetArray.size();
	mOffsetArray.resize( newXPlaneCount );
	for( size_t iX=0 ; iX<newXPlaneCount ; iX++)
	{
		setZeroValue = ( iX>=prevXPlaneCount ); // we're in new planes: initialize them

		std::vector< std::vector<TVector3> >& curXPlane = mOffsetArray[iX];
		const size_t prevYPlaneCount = curXPlane.size();
		curXPlane.resize( newYPlaneCount );
		for( size_t iY=0 ; iY<newYPlaneCount ; iY++)
		{
			if( !setZeroValue )
				setZeroValue = ( iY>=prevYPlaneCount ); // we're in new planes: initialize them
			
			std::vector<TVector3>& curYPlane = curXPlane[iY];
			const size_t prevZPlaneCount = curYPlane.size();
			curYPlane.resize( newZPlaneCount );
			for( size_t iZ=0 ; iZ<newZPlaneCount ; iZ++)
			{
				if( !setZeroValue )
					setZeroValue = ( iZ>=prevZPlaneCount ); // we're in new planes: initialize them

				if( setZeroValue )
					curYPlane[iZ] = TVector3(0,0,0);
			}
		}
	}
}

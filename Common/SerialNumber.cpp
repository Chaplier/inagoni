/****************************************************************************************************

		SerialNumber.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/9/2004

****************************************************************************************************/

#include "SerialNumber.h"

#include "copyright.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "IShTokenStream.h"
#include "InterfaceIDs.h"
#include "IShPartUtilities.h"
#include "MCRandom.h"

#include "IMFPart.h"
#include "IMFWindow.h"
#include "IMFDialogPart.h"

// Base transformation methods
const uint16 gLetters[26] = {'A','B','C','D','E','F','G','H','I','J','K',
			'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

boolean SerialNumber::gSerialNumberValid = false;

inline int32 Convert10(const uint16 digit)
{
	if( digit<=57) return (int32)(digit-48);
	else return 10+(int32)digit-65;
}
inline uint16 ConvertBase(const int32 digit)
{
	if( digit<=9) return (uint16)(digit+48);
	else return (uint16)(digit-10)+65;
}

boolean VerifyBase( const int32 base, const TMCString& string )
{
	// Check that each letter of the string is a correct digit
	const int32 strLength = string.Length();
	for(int32 iLetter=0 ; iLetter<strLength ; iLetter++)
	{
		const uint16 letter = string[iLetter];

		if(Convert10(letter)>=base)
			return false;
	}
	return true;
}

boolean Base10(const int32 fromBase, const TMCString& fromString, int32& result)
{
	result=0;

	// Check that the passed string is nBase
	if(VerifyBase(fromBase, fromString))
	{
		// Convert the string
		const int32 length = fromString.Length();
		if(!length)
			return false;

		result = Convert10(fromString[0]);
		for( int32 iLetter=1 ; iLetter<length ; iLetter++ )
		{
			const int32 next = Convert10(fromString[iLetter]);
			result = result*fromBase + next;
		}

		return true;
	}
	
	return false;
}

boolean NewBase(const int32 fromValue, const int32 newBase, TMCString& newString)
{
	newString.EraseStr(0,newString.Length());

	TMCDynamicString invertString;
	int32 value = fromValue;
	do
	{
		invertString += ConvertBase(value%newBase);
		
		value = (int32)(value/newBase);
	} while(value);


	const int32 length = invertString.Length();
	for(int32 iLetter=0 ; iLetter<length ; iLetter++)
	{
		newString += invertString[length-1-iLetter];
	}

	return true;
}
//
boolean IsPrimary( const int32 value)
{
	if(!(value&0x00000001))
		return false;

	for( int32 modulo=3 ; modulo<=value/modulo ; modulo+=2 )
	{
		if(value%modulo==0)
			return false;
	}

	return true;
}

void MakePrimaryNumbers( const int32 nbr, TMCArray<int32>& primaries )
{
	primaries.SetElemCount(nbr);
	int32 i=0;

	int32 value = 5;

	for(;;)
	{
		if(IsPrimary(value))
		{
			primaries[i++] = value;
			if(i>=nbr) return;
		}
		value+=2;

		if(IsPrimary(value))
		{
			primaries[i++] = value;
			if(i>=nbr) return;
		}
		value+=4;
	}
}

//
SerialNumber::SerialNumber(TMCClassArray<KeyData>& keyDatas)
{
	// Need at least 1 element to know where to check for the key
	MCAssert(keyDatas.GetElemCount());
	fKeyDatas = keyDatas;

	fLicenseInfos.SetElemCount(4);
}

void SerialNumber::AddBannedLicense( const TMCString15& block0,const TMCString15& block1, const TMCString15& block2, const TMCString15& block3 )
{
	mBannedLicenses.push_back( TMCClassArray<TMCString15>() );
	TMCClassArray<TMCString15>& cur = mBannedLicenses.back();

	cur.AddElem( block0 );
	cur.AddElem( block1 );
	cur.AddElem( block2 );
	cur.AddElem( block3 );
}

boolean SerialNumber::IsBanned( const TMCClassArray<TMCString15>& keys ) const 
{
	const size_t count = mBannedLicenses.size();
	for( size_t i=0 ; i<count ; i++ )
	{
		const TMCClassArray<TMCString15>& cur = mBannedLicenses[i];
		if( keys[0] == cur[0] &&
			keys[1] == cur[1] &&
			keys[2] == cur[2] &&
			keys[3] == cur[3] )
			return true;
	}

	return false;
}

boolean SerialNumber::CheckSerial()
{
	SerialNumber::gSerialNumberValid = CheckSerialInternal();
	return SerialNumber::gSerialNumberValid;
}


boolean SerialNumber::CheckSerialInternal()
{
//	Use this method to build a list of keys
//	CreateKeyList(fKeyDatas[0], 20000);

	const int32 dataCount = fKeyDatas.GetElemCount();

	for(int32 iData=0 ; iData<dataCount ; iData++)
	{
		if( CheckSerialForData(fKeyDatas[iData]) )
			return true; // The right serial number was found, return
	}

	// No valid key was found, ask the user to feed one
	return AskFoKey(); // return true if the key is valid
}

boolean SerialNumber::CheckSerialForData(const KeyData& data)
{
	CWhileInCompResFile myRes(data.mFamilyID, data.mClassID);
	
	// First retreive the stored infos to see if there's allready a key registered
	LoadSerialNumber(data);

	// Then, if not, ask the user to enter a key
	if( !IsKeyValid(fLicenseInfos,data) )
		return false;
	else
		return true;

}


bool TryCreateFolder( const TMCString& folderName )
{
	TMCCountedPtr<IMCFile> folder;
	gFileUtilities->CreateIMCFile(&folder);
	folder->SetWithPathName( folderName, true );

	if(!folder->Exists())
	{
		folder->CreateOnDiskWithFullPath();
		if(!folder->Exists())
			return false;
	}

	return true;
}

TMCDynamicString GetFolder()
{
	TMCDynamicString appData;
	gShellUtilities->GetStandardPath( kUserApplicationData, appData ); 
	TMCDynamicString serialPath = appData + TMCDynamicString( "/inagoni/" );

	// If the folder doesn't exist, try to create it
	if( TryCreateFolder(serialPath) )
		return serialPath;

	// Couldn't create the folder, try to find another location
	serialPath = appData + TMCDynamicString( "/DAZ 3D/inagoni/" );
	if( TryCreateFolder(serialPath) )
		return serialPath;

	TMCDynamicString appPath;
	gFileUtilities->GetAppDirectory(appPath);

	serialPath = appPath + TMCDynamicString( "/inagoni/" );
	if( TryCreateFolder(serialPath) )
		return serialPath;

	TMCDynamicString doc;
	gShellUtilities->GetStandardPath( kUserDocuments, doc ); 
	serialPath = doc + TMCDynamicString( "/inagoni/" );
	if( TryCreateFolder(serialPath) )
		return serialPath;

	return serialPath;
}

void SerialNumber::GetSerialFile(IMCFile** file, const KeyData& data)
{
	// Get application path
	TMCDynamicString appPath = GetFolder(); 

	// Create a IMCFile
	gFileUtilities->CreateIMCFile(file);

	// Create the full path name
	appPath += data.mFileName;
	(*file)->SetWithFullPathName(appPath);
}

void SerialNumber::LoadSerialNumber(const KeyData& data)
{
	TMCCountedPtr<IMCFile> file;
	GetSerialFile(&file, data);
		
	if(file->Exists())
	{
		MCErr err = file->Open();
		if(file->IsOpen())
		{
			int8 buffer[17];
			file->Read(buffer, 17 );
			buffer[16]=NULL;

			int8* ptr = buffer;

			fLicenseInfos[0] = "0000";
			fLicenseInfos[0].FromText(ptr, 4);
			ptr+=4;
			fLicenseInfos[1] = "0000";
			fLicenseInfos[1].FromText(ptr, 4);
			ptr+=4;
			fLicenseInfos[2] = "0000";
			fLicenseInfos[2].FromText(ptr, 4);
			ptr+=4;
			fLicenseInfos[3] = "0000";
			fLicenseInfos[3].FromText(ptr, 4);
		}
		else
		{
			if(err == kfnfErr)
			{
				MCNotify("File Not Found Error");
			}
		}
		file->Close();
	}
}

void SerialNumber::SaveSerialNumber(const KeyData& data)
{
	TMCCountedPtr<IMCFile> file;
	GetSerialFile(&file, data);

	if(!file->Exists())
	{
		file->CreateOnDiskWithFullPath();
	}

	file->Open();
	if(file->IsOpen())
	{
		for( int32 iKey= 0 ; iKey<4 ; iKey++ )
		{
			file->Write(fLicenseInfos[iKey].StrGet(), fLicenseInfos[2].ByteLength() );
		}
	}
	file->Close();
}

boolean SerialNumber::AskFoKey()
{
	boolean status = true;
	TMCCountedPtr<IMFPart> dialogPart;
	gPartUtilities->CreatePartByResource(&dialogPart, kMFDialogResourceType, fKeyDatas[0].mResID);

	if (!dialogPart)
		return false;
	
	TMCCountedPtr<IMFWindow> theDialogWindow;
	dialogPart->QueryInterface(IID_IMFWindow, (void**)&theDialogWindow);
	if (!theDialogWindow)
		return false;

	theDialogWindow->Center(true, true);
	TMCCountedPtr<IMFDialogPart> theDialog;	
	dialogPart->QueryInterface(IID_IMFDialogPart, (void**)&theDialog);
	if (!theDialog)
		return false;

	boolean keyIsValid = false;

	do
	{
		status = theDialog->Go();

		if (status)
		{	// User hit OK: check the key
			TMCCountedPtr<IMFPart> editText;

			dialogPart->FindChildPartByID(&editText, 'Key1');
			if (!editText)
				return false;
			editText->GetValue( (void*)&fLicenseInfos[0], kStringValueType);	

			dialogPart->FindChildPartByID(&editText, 'Key2');
			if (!editText)
				return false;
			editText->GetValue( (void*)&fLicenseInfos[1], kStringValueType);	

			dialogPart->FindChildPartByID(&editText, 'Key3');
			if (!editText)
				return false;
			editText->GetValue( (void*)&fLicenseInfos[2], kStringValueType);	

			dialogPart->FindChildPartByID(&editText, 'Key4');
			if (!editText)
				return false;
			editText->GetValue( (void*)&fLicenseInfos[3], kStringValueType);
			
			// Check the key validity
			const int32 dataCount = fKeyDatas.GetElemCount();

			for(int32 iData=0 ; iData<dataCount && !keyIsValid; iData++)
			{
				if(IsKeyValid(fLicenseInfos, fKeyDatas[iData]))
				{	// the key is valid, save it in the appropriate file
					keyIsValid = true;
					SaveSerialNumber(fKeyDatas[iData]);
				}
			}
		}
	}
	while( status && !keyIsValid );
		
	theDialog->Finished();

	return status;
}

boolean SerialNumber::IsKeyValid( const TMCClassArray<TMCString15>& keys, const KeyData& data ) const
{
	if( IsBanned( keys ) )
		return false;

	if( keys[0].Length() == 4 &&
		keys[1].Length() == 4 &&
		keys[2].Length() == 4 &&
		keys[3].Length() == 4 )
	{
		// Build the correct keys
		TMCClassArray<TMCString15> uncoddedKeys(4);

		for(int32 iKey=0 ; iKey<4 ; iKey++)
		{
			uncoddedKeys[iKey] = "0000";
			for(int32 iLetter=0 ; iLetter<4 ; iLetter++)
			{
				uncoddedKeys[iKey].SetCharAt(keys[3-iLetter].GetCharAt((iKey+iLetter+2)%4), iLetter);
			}

			uncoddedKeys[iKey].ToUpper();

		}

		// Now check the 4 elements

		// 1: product info
		if(uncoddedKeys[0] != data.mProductInfo)
			return false;

		// 2: %modulo
		int32 value=0;
		if( !Base10(36,uncoddedKeys[1],value) )
			return false;
		if(value%data.mModulo != 0)
			return false;

		// 3: 2 letters, 2 figures
		int32 digit = Convert10(uncoddedKeys[2][0]);
		if( digit<10 ) return false;
		digit = Convert10(uncoddedKeys[2][1]);
		if( digit>9 ) return false;
		digit = Convert10(uncoddedKeys[2][2]);
		if( digit<10 ) return false;
		digit = Convert10(uncoddedKeys[2][3]);
		if( digit>9 ) return false;

		// 4: primary number
		int32 number=0;
		if( !Base10( 36, uncoddedKeys[3], number ) )
			return false;
		if( !IsPrimary(number) )
			return false;

		return true;
	}

	return false;
}

void SerialNumber::BuildKey(int32 primary, TMCString& key, const KeyData& data)
{
	TMCClassArray<TMCString15> keys(4);

	// 1: product info
	keys[0] = data.mProductInfo;

	// 2: modulo 'modulo'
	uint32 rand = (uint32)MCRandom()/3000;
	int32 val = rand%data.mModulo;
	val = rand-val;
	NewBase( val, 36, keys[1]);
	int32 k1Length = keys[1].Length();
	const TMCString15 zero("0");
	for( ; k1Length<4 ; k1Length++ )
	{
		keys[1].InsertStr(zero,0);
	}

	// 3: 2 letters, 2 figures
	keys[2].EraseStr(0,keys[2].Length());
	rand = (uint32)(MCRandom());
	keys[2] += (uint16)gLetters[rand%26];
	rand = (uint32)(MCRandom());
	keys[2] += (uint16)(rand%10) +48;
	rand = (uint32)(MCRandom());
	keys[2] += (uint16)gLetters[rand%26];
	rand = (uint32)(MCRandom());
	keys[2] += (uint16)(rand%10) +48;

	// 4: primary number
	NewBase( primary, 36, keys[3]);
	int32 k3Length = keys[3].Length();
	for( ; k3Length<4 ; k3Length++ )
	{
		keys[3].InsertStr(zero,0);
	}

	// Then stir a little bit
	TMCClassArray<TMCString15> coddedKeys(4);

	for(int32 iKey=0 ; iKey<4 ; iKey++)
	{
		coddedKeys[iKey] = "0000";
		for(int32 iLetter=0 ; iLetter<4 ; iLetter++)
		{
			uint16 letter = keys[(iKey+iLetter+3)%4].GetCharAt(3-iKey);
			coddedKeys[iKey].SetCharAt(letter,iLetter);
		}

	}

	// Check the key
	{
		if( IsKeyValid(coddedKeys, data) )
		{
			TMCString15 separator("-");
			key =	coddedKeys[0] + separator +
					coddedKeys[1] + separator + 
					coddedKeys[2] + separator + 
					coddedKeys[3];
		}
		else
		{
			key = TMCString15("Invalid key here");
		}
	}

}

// Utility : create a list of serial
void SerialNumber::CreateKeyList(const KeyData& data, int32 createKeyCount)
{
	// Get application path
	TMCDynamicString appPath = GetFolder();

	// Get the file name
	TMCDynamicString fileName = data.mProductInfo;
	fileName += TMCDynamicString("Licenses.txt");

	// Create a IMCFile
	TMCCountedPtr<IMCFile> file;
	gFileUtilities->CreateIMCFile(&file);

	// Create the full path name
	appPath += fileName;
	file->SetWithFullPathName(appPath);

	gPartUtilities->Alert(appPath);

	if(!file->Exists())
	{
		file->CreateOnDiskWithFullPath();
	}

	file->Open();
	if(file->IsOpen())
	{
		// Build an array of primary numbers
		TMCArray<int32> primaries;
		MakePrimaryNumbers( createKeyCount, primaries );
		TMCString31 enter("\r\n");
		for (int32 i=0 ; i<createKeyCount ; i++)
		{
			TMCString31 key;
			BuildKey(primaries[i], key, data);
			key+=enter;
			file->Write(key.StrGet(), key.ByteLength() );
		//	file->Write(enter.StrGet(), sizeof(int8) );
		}
	}
	file->Close();
}


ExceptionLog::ExceptionLog(const TMCString& pluginName)
{
	// Get application path
	TMCDynamicString inagoniFolder = GetFolder(); 

	// Create a IMCFile
	gFileUtilities->CreateIMCFile(&mFile);

	// Create the full path name
	inagoniFolder += pluginName;
	inagoniFolder += TMCDynamicString("Exceptions.txt");
	mFile->SetWithFullPathName(inagoniFolder);
}

ExceptionLog::~ExceptionLog()
{
	if(mFile->IsOpen())
	{
		mFile->Close();
	}
}

void ExceptionLog::HandleException(const TMCString& methodName )
{
	if(!mFile->Exists())
	{
		mFile->CreateOnDiskWithFullPath();
	}
	if(!mFile->IsOpen())
	{
		mFile->Open();
	}

	mFile->Write(methodName.StrGet(), methodName.ByteLength() );
	NextLine();
}

void ExceptionLog::HandleException(TMCException& exception, const TMCString& methodName )
{
	HandleException(methodName);

	if( exception.GetErrorString() )
	{
		mFile->Write(exception.GetErrorString()->StrGet(), exception.GetErrorString()->ByteLength() );
		NextLine();
	}

	TMCDynamicString inFileAtLine(exception.fFile);

	inFileAtLine += TMCDynamicString( " at line ");
	TMCDynamicString lineNumber;
	lineNumber.FromInt32(exception.fLine);
	inFileAtLine += lineNumber;

	mFile->Write(inFileAtLine.StrGet(), inFileAtLine.ByteLength() );
	NextLine();
}

void ExceptionLog::NextLine()
{
	TMCString31 nextLine("\r\n");
	mFile->Write(nextLine.StrGet(), nextLine.ByteLength() );
}

/****************************************************************************************************

		SerialNumber.cpp
		Copyright: (c) 2005 Alan Stafford. All rights reserved.

		Author:	Alan
		Date:	6/10/2005

****************************************************************************************************/

#include "SerialNumber.h"

#include "copyright.h"
#include "IShUtilities.h"
#include "MiscComUtilsImpl.h"
#include "IShTokenStream.h"
#include "InterfaceIDs.h"
//#include "CarraraResStreamUtil.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"
#endif
#include "MCRandom.h"

#include "IMFPart.h"
#include "IMFWindow.h"
#include "IMFDialogPart.h"
#include "CurvatureDef.h"

// Key data
static const TMCString15 productInfo("CURV");
static const int32 modulo = 101;
static const IDType familyID = kRID_ShaderFamilyID;
static const IDType classID = 'MDcv';
static const int32 resID = 950;
static const int32 stringResID = kStrings;
static const int32 fileNameID = 1;
// Number of key created		
static const int32 createKeyCount = 1000;

// Base transformation methods
const uint16 gLetters[26] = {'A','B','C','D','E','F','G','H','I','J','K',
			'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

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
boolean IsKeyValid( TMCClassArray<TMCString15>& keys );


//
SerialNumber::SerialNumber()
{
	fLicenseInfos.SetElemCount(4);
}

boolean SerialNumber::CheckSerial()
{
//	Use this method to build a list of keys
//	CreateKeyList();

	CWhileInCompResFile myRes(familyID, classID);
	
	// First retreive the stored infos to see if there's allready a key registered
	LoadSerialNumber();

	// Then, if not, ask the user to enter a key
	if( !IsKeyValid(fLicenseInfos) )
	{
		// Ask a key
		if( AskFoKey() )
		{
			// The key is OK, register it
			SaveSerialNumber();
			return true;
		}
		else
			return false;
	}
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

void SerialNumber::GetSerialFile(IMCFile** file)
{
	// Get application path
	TMCDynamicString appPath = GetFolder(); 

	// Get the file name
	TMCDynamicString fileName;
	gResourceUtilities->GetIndString(fileName, stringResID, fileNameID);

	if(!fileName.Length())
	{	// In case the resource is not found
		MCNotify("Resource Not Found");
		fileName = "Curvature.dta";
	}

	// Create a IMCFile
	gFileUtilities->CreateIMCFile(file);

	// Create the full path name
	appPath += fileName;
	(*file)->SetWithFullPathName(appPath);
}

void SerialNumber::LoadSerialNumber()
{
	TMCCountedPtr<IMCFile> file;
	GetSerialFile(&file);
		
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

void SerialNumber::SaveSerialNumber()
{
	TMCCountedPtr<IMCFile> file;
	GetSerialFile(&file);

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
	gPartUtilities->CreatePartByResource(&dialogPart, kMFDialogResourceType, resID);

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
		}
	}
	while( status && !IsKeyValid(fLicenseInfos) );
		
	theDialog->Finished();

	return status;
}

boolean IsKeyValid( TMCClassArray<TMCString15>& keys )
{
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
		if(uncoddedKeys[0] != productInfo)
			return false;

		// 2: %modulo
		int32 value=0;
		if( !Base10(36,uncoddedKeys[1],value) )
			return false;
		if(value%modulo != 0)
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

void BuildKey(int32 primary, TMCString& key)
{
	TMCClassArray<TMCString15> keys(4);

	// 1: product info
	keys[0] = productInfo;

	// 2: modulo 'modulo'
	uint32 rand = (uint32)MCRandom()/3000;
	int32 val = rand%modulo;
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
		if( IsKeyValid(coddedKeys) )
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
void SerialNumber::CreateKeyList()
{
	// Get application path
	TMCDynamicString appPath = GetFolder();

	// Get the file name
	TMCDynamicString fileName("CurvatureLicenses.txt");

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
			BuildKey(primaries[i], key);
			key+=enter;
			file->Write(key.StrGet(), key.ByteLength() );
		//	file->Write(enter.StrGet(), sizeof(int8) );
		}
	}
	file->Close();
}


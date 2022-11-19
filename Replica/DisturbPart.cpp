/****************************************************************************************************

		DisturbPart.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "DisturbPart.h"

#include "InstanciatorDef.h"
#include "Copyright.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "IMFResponder.h"
#include "MCClassArray.h"
#include "MiscComUtilsImpl.h"
#include "InterfaceIDs.h"
#include "I3DShShader.h"
#include "IMFTextPopupPart.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "ISceneDocument.h"
#include "IShComponent.h"
#include "IComponentChooser.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_DisturbPart(R_CLSID_DisturbPart);
#else
const MCGUID CLSID_DisturbPart = {R_CLSID_DisturbPart};
#endif

void GetShaderNames( I3DShScene* inScene, TMCClassArray<TMCDynamicString>& shaderNames )
{
	CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');

	const int32 shaderCount = inScene->GetMasterShadersCount();
	shaderNames.SetElemCount(shaderCount);

	for(int32 iShader=0 ; iShader<shaderCount ; iShader++)
	{
		TMCCountedPtr<I3DShMasterShader> masterShader;
		inScene->GetMasterShaderByIndex(&masterShader, iShader);

		TMCDynamicString name;
		masterShader->GetName(name);

		shaderNames[iShader] = name;
	}
}

void BuildShadersPopupMenu(IMFPart* thisPart,
						   const int32 partID, 
						const TMCClassArray<TMCDynamicString>& shaderNames)
{
	const int32 nameCount = shaderNames.GetElemCount();
	if(nameCount<1)
		return;

	TMCCountedPtr<IMFPart> part;
	thisPart->FindChildPartByID(&part, partID);
	ThrowIfNil(part);

	TMCCountedPtr<IMFTextPopupPart> popup;
	part->QueryInterface( IID_IMFTextPopupPart, (void **)&popup );
	ThrowIfNil(popup);
	

	// The popup menu will look like that:
	// Shader 0
	// Shader 1
	// ...
	// _____________________
	// Extra Name (None or Noise)

	// Clean the current popup menu
	popup->RemoveAll();

	const int32 lastName = nameCount-1;
	if(nameCount>1)
	{
		// Fill the menu with the shading domains
		for (int32 iName = 0; iName<lastName; iName++)
		{
			popup->AppendMenuItem(shaderNames[iName]);
		}

		// Add a separator
		popup->AppendSeparator();
	}

	// Add the last name (None or Noise)
	popup->AppendMenuItem(shaderNames[lastName]);

	popup->SetSelectedItem(lastName>0?nameCount:0,true);

}

// DisturbPart class

DisturbPart::DisturbPart()
{
}

void DisturbPart::SelfPrepareToDestroy()
{
	TBasicPart::SelfPrepareToDestroy();
}

MCErr DisturbPart::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_DisturbPart))
	{
		TMCCountedGetHelper<DisturbPart> result(ppvObj);
		result= this;
		return MC_S_OK;
	}
	else
		return TBasicPart::QueryInterface(riid, ppvObj);
}

void DisturbPart::FinishCreateFromResource()
{
	ISceneDocument*	currentDoc = gShell3DUtilities->GetLastActiveSceneDoc();
	if(!MCVerify(currentDoc))
		return;
	currentDoc->GetScene(&fScene);

	// Fill in the popup menus

	TMCClassArray<TMCDynamicString> shaderNames;
	GetShaderNames( fScene, shaderNames );

	// Last name: Default Noise
	const int32 shaderCount = shaderNames.GetElemCount();
	shaderNames.AddElemCount(1);
	TMCDynamicString str;
	gResourceUtilities->GetIndString(str, kStrings, 2 ); // Default Noise
	shaderNames[shaderCount] = str;

	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return ;
	{	// Position popup menu
		BuildShadersPopupMenu( thisPart, 'PoXS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'PoYS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'PoZS', shaderNames );
	}
	{	// Uniform Scale popup menu
		BuildShadersPopupMenu( thisPart, 'ScUS', shaderNames );
	}
	{	// Scale popup menu
		BuildShadersPopupMenu( thisPart, 'ScXS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'ScYS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'ScZS', shaderNames );
	}
	{	// Rotation popup menu
		BuildShadersPopupMenu( thisPart, 'RoXS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'RoYS', shaderNames );
		BuildShadersPopupMenu( thisPart, 'RoZS', shaderNames );
	}

	{	// Shading modifier popup menu
		BuildShadersPopupMenu( thisPart, 'ShMS', shaderNames );
	}

	// Last name: Default Color
	gResourceUtilities->GetIndString(str, kStrings, 1 ); // Default color
	shaderNames[shaderCount] = str;

	{ // Shading popup menu
		BuildShadersPopupMenu( thisPart, 'ShaS', shaderNames );
	}

	{ // Disable
		EnableOption(false,'PoXS');
		EnableOption(false,'PoYS');
		EnableOption(false,'PoZS');

		EnableOption(false,'ScUS');

		EnableOption(false,'ScXS');
		EnableOption(false,'ScYS');
		EnableOption(false,'ScZS');

		EnableOption(false,'RoXS');
		EnableOption(false,'RoYS');
		EnableOption(false,'RoZS');

		// Shading option
		EnableOption(false,'ShMS');

		EnableShadingOptions(false);
	}

	TBasicPart::FinishCreateFromResource();
}

boolean	DisturbPart::Receive(int32 message, IMFResponder* source, void* data)
{
	boolean handledMessage= false;

	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (MCVerify(source))
			{
				TMCCountedPtr<IMFPart> part;
				source->QueryInterface(IID_IMFPart, (void **)&part);
				if (part)
				{
					const IDType partID= part->GetIMFPartID();
					int32 shaderID = 0;
					int32 enablePartID = 0;

					switch (partID)
					{
						// When a slider is changed, check if we still need to keep the option enabled
					case 'PSha':
						{
							real32 value = 0;
							part->GetValue(&value, kInt32ValueType);
							EnableShadingOptions(value!=0);
							enablePartID = 'ShMS';
						} break;
					case 'PPoX': enablePartID = 'PoXS'; break;
					case 'PPoY': enablePartID = 'PoYS'; break;
					case 'PPoZ': enablePartID = 'PoZS'; break;
					case 'PScU': enablePartID = 'ScUS'; break;
					case 'PScX': enablePartID = 'ScXS'; break;
					case 'PScY': enablePartID = 'ScYS'; break;
					case 'PScZ': enablePartID = 'ScZS'; break;
					case 'PRoX': enablePartID = 'RoXS'; break;
					case 'PRoY': enablePartID = 'RoYS'; break;
					case 'PRoZ': enablePartID = 'RoZS'; break;
					case 'ShaS': shaderID = 'ShIn';  break;
					case 'PoXS': shaderID = 'SPoX';  break;
					case 'PoYS': shaderID = 'SPoY';  break;
					case 'PoZS': shaderID = 'SPoZ';  break;
					case 'ScUS': shaderID = 'SScU';  break;
					case 'ScXS': shaderID = 'SScX';  break;
					case 'ScYS': shaderID = 'SScY';  break;
					case 'ScZS': shaderID = 'SScZ';  break;
					case 'RoXS': shaderID = 'SRoX';  break;
					case 'RoYS': shaderID = 'SRoY';  break;
					case 'RoZS': shaderID = 'SRoZ';  break;
					case 'ShMS': shaderID = 'SShM';  break;
					default: break;
					}

					if(shaderID!=0)
					{
						int32 popupItem = -1;
						part->GetValue(&popupItem, kInt32ValueType);

						IMFPart* thisPart = GetThisPartNoAddRef();
						if(!thisPart)
							break;
						TMCCountedPtr<IMFPart> compPart;
						thisPart->FindChildPartByID(&compPart, shaderID);
						ThrowIfNil(compPart);
						TMCCountedPtr<I3DShMasterShader> masterShader;
						if(popupItem>=(int32)fScene->GetMasterShadersCount())
							popupItem = -1;
						compPart->SetValue(&popupItem, kInt32ValueType,true,true);
					}
					if(enablePartID!=0)
					{
						real32 value = 0;
						part->GetValue(&value, kInt32ValueType);
						EnableOption(value!=0, enablePartID);
					} break;
				}
			}
		}
	}

	return handledMessage;
}

void DisturbPart::EnableShadingOptions(const boolean enable)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return;
	{
		TMCCountedPtr<IMFPart> part;
		thisPart->FindChildPartByID(&part, 'ShaC');
		ThrowIfNil(part);
		part->Enable(enable);
	}
	{
		TMCCountedPtr<IMFPart> part;
		thisPart->FindChildPartByID(&part, 'ShaS');
		ThrowIfNil(part);
		part->Enable(enable);
	}
}

void DisturbPart::EnableOption(const boolean enable, const int32 partID)
{
	IMFPart* thisPart = GetThisPartNoAddRef();
	if(!thisPart)
		return;
	{
		TMCCountedPtr<IMFPart> part;
		thisPart->FindChildPartByID(&part, partID);
		ThrowIfNil(part);
		part->Enable(enable);
	}
}
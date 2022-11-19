/****************************************************************************************************

		ShaderFactory.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/31/2004

****************************************************************************************************/

#include "ShaderFactory.h"

#include "copyright.h"
#include "IShComponent.h"
#include "COMUtilities.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "COMSafeUtilities.h"
#endif
#include "IShUtilities.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "NameUtilities.h"
	


void ShaderFactory::Init(const int32 maxCount, const TVector2& minMax, I3DShMasterShader* basedOnShader)
{
	// TO DO: get the min and max value of the shader
	fMin=minMax.x;
	fMax=minMax.y;
	
	fInc=(fMax-fMin)/(real32)maxCount;
	fMaxCount = maxCount;
	SetElemCount(maxCount);
	fBasedOnShader=basedOnShader;
}

// Look for an existing shader in the array, if none is found, create a new one
I3DShMasterShader* ShaderFactory::GetMasterShader(const real32 value,
												  int32& nameCounter,
												  I3DShMasterShader* otherMasterShader,
												  I3DShScene* scene)
{
	const int32 shaderIndex = Index(value);

	if(fArrayData[shaderIndex].fValue<0)
	{
		// The shader doesn't exist yet, make it
		const real32 flattenValue = FlattenValue(shaderIndex);

		TMCString255 baseName;
		fBasedOnShader->GetName(baseName);

		TMCString255 otherName;
		otherMasterShader->GetName(otherName);

		// 1: Create a Reference Shader on the original shader
		TMCCountedPtr<IShParameterComponent> ref1ParamComp;
		{
			TMCCountedPtr<IShComponent> refComponent;
			gComponentUtilities->CreateComponent(kShaderFamily, kReferenceShdr, &refComponent);
			// Set its data
			refComponent->QueryInterface(IID_IShParameterComponent, (void**)&ref1ParamComp);	
			ref1ParamComp->SetParameter( 'SRef' , &fBasedOnShader); // Pointer to master shader	
			ref1ParamComp->SetParameter( 'Name' , &baseName); // Name of Master Shader	
		}

		// 2: Create a Value Shader
		TMCCountedPtr<IShComponent> valParamComp;
		{
			TMCCountedPtr<I3DShShader> valShader;
			gShell3DUtilities->CreateValueShader(flattenValue, &valShader);

			valShader->QueryInterface(IID_IShParameterComponent, (void**)&valParamComp);
		}

		// 3: Create a second Reference Shader on the other master shader
		TMCCountedPtr<IShParameterComponent> ref2ParamComp;
		{
			TMCCountedPtr<IShComponent> refComponent;
			gComponentUtilities->CreateComponent(kShaderFamily, kReferenceShdr, &refComponent);
			// Set its data
			refComponent->QueryInterface(IID_IShParameterComponent, (void**)&ref2ParamComp);	
			ref2ParamComp->SetParameter( 'SRef' , &otherMasterShader); // Pointer to master shader	
			ref2ParamComp->SetParameter( 'Name' , &otherName); // Name of Master Shader	
		}

		// 4: Create a global mixer
		TMCCountedPtr<I3DShShader> globalMixer;
		{
			TMCCountedPtr<IShComponent> component;
			gComponentUtilities->CreateComponent(kShaderFamily, kGlobalMixShdr, &component);
			TMCCountedPtr<IShParameterComponent> paramComp;
			component->QueryInterface(IID_IShParameterComponent, (void**)&paramComp);
			paramComp->SetParameter( 'Sh00' , &ref1ParamComp);	
			paramComp->SetParameter( 'Sh01' , &ref2ParamComp);	
			paramComp->SetParameter( 'Sh02' , &valParamComp);	

			component->QueryInterface(IID_I3DShShader, (void**)&globalMixer);
		}

		// 5: Crate the new master shader to be set on the instance
		{
			TMCCountedPtr<I3DShMasterShader> masterShader;
			gShell3DUtilities->CreateMasterShader(globalMixer, scene, &masterShader);
			ThrowIfNil(masterShader);

			TMCDynamicString name;
			// We can't use the index at the end of the name: the index is then ignored by GetUniqueMasterShaderNameByBaseName
			name.FromInt32( ++nameCounter ); // start indexing at 1, when there's a 0, it's removed during the saving/loading
			name+='_';
			name+=baseName;
			name+='_';
			name+=otherName;
			GetUniqueMasterShaderNameByBaseName(scene,name,name);
			masterShader->SetName(name);

			// Fill in the cache data
			fArrayData[shaderIndex].fMaster = masterShader;
			fArrayData[shaderIndex].fValue = flattenValue;
		}
	}

	return fArrayData[shaderIndex].fMaster;
}


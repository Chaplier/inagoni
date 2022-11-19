/****************************************************************************************************

		PopupParts.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/06/2006

****************************************************************************************************/

#ifndef __PopupParts__
#define __PopupParts__

#include "BasicMCFCOMImplementations.h"

#include "I3DShScene.h"
#include "MCClassArray.h"

extern const MCGUID CLSID_ShaderPopupPart;
extern const MCGUID CLSID_ShaderChooserPart;

extern const MCGUID CLSID_MeshPopupPart;
extern const MCGUID CLSID_MeshChooserPart;

/////////////////////////////////////////////////////////////////////////
//
// Shader popup

// ShaderChooserPart (this part countains a shaderPopupPart)
class ShaderChooserPart : public TBasicPart
{
public:
	ShaderChooserPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	// Read the custom tokens
	virtual boolean	MCCOMAPI ReadAttribute(int32 inKeyword, TMCiostream& inStream);

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	void BuildPopup();

	uint32	fParam1ID; // PMap: index in the popup
	uint32	fParam2ID; // PMap: shader name
	TMCCountedPtr<I3DShScene>		fScene;
};

// Rebuild the shader popup when needed
class ShaderPopupPart : public TBasicPart
{
public:
	ShaderPopupPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	// Read the custom tokens
	virtual boolean	MCCOMAPI ReadAttribute(int32 inKeyword, TMCiostream& inStream);

	virtual TMFEventResult	MCCOMAPI SelfMouseDown(const TMCPoint& inWhere, const TMCPlatformEvent& inEvent);

protected:
	void BuildPopup();

	int32	fType;

	TMCCountedPtr<I3DShScene>		fScene;
};


/////////////////////////////////////////////////////////////////////////
//
// Mesh popup

// MeshChooserPart (this part countains a meshPopupPart)
class MeshChooserPart : public TBasicPart
{
public:
	MeshChooserPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

//	virtual boolean	MCCOMAPI ReadAttribute(int32 inKeyword, TMCiostream& inStream);

	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:
	void BuildPopup();

	uint32	fParam1ID; // PMap: index 0 or 1
	uint32	fParam2ID; // PMap: tree element name
	TMCCountedPtr<I3DShScene>		fScene;
};

// Rebuild the mesh popup when needed
class MeshPopupPart : public TBasicPart
{
public:
	MeshPopupPart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

//	virtual boolean	MCCOMAPI ReadAttribute(int32 inKeyword, TMCiostream& inStream);

	virtual TMFEventResult	MCCOMAPI SelfMouseDown(const TMCPoint& inWhere, const TMCPlatformEvent& inEvent);

protected:
	void BuildPopup();

	int32	fType;

	TMCCountedPtr<I3DShScene>		fScene;
};
















#endif // __PopupParts__

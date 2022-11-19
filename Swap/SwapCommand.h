/****************************************************************************************************

		SwapCommand.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/9/2004

****************************************************************************************************/

#ifndef __SwapCommand__
#define __SwapCommand__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "SwapDef.h"

#include "Basic3DCOMImplementations.h"

#include "I3DShScene.h"

#include "ISceneSelection.h"
#include "IChangeManagement.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "MCCountedPtrArray.h"
#include "MCCountedPtr.h"
#include "IShComponent.h" // for IShParameterComponent


extern const MCGUID CLSID_SwapCommand;


struct SwapPMap
{
	SwapPMap();

	// Swap params
	int32 fType;
	TMCDynamicString fObjectName;
	TMCDynamicString fShaderName;
	int32 fTreePermanentID;
	TMCCountedPtr<IShParameterComponent>	fCameraComponent;
	TMCCountedPtr<IShParameterComponent>	fLightComponent;
	boolean fObjectFitIn;
	boolean fTreeFitIn;
};

struct SwapData;

class SwapCommand : public TBasicSceneCommand//, public TBasicWireframe
{
public :  
	SwapCommand();
	virtual ~SwapCommand();

	STANDARD_RELEASE;

	// IExDataExchanger methods :
	virtual MCCOMErr MCCOMAPI QueryInterface			(const MCIID& riid, void** ppvObj);
	virtual void*	 MCCOMAPI GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(SwapPMap);}
	virtual MCCOMErr MCCOMAPI ExtensionDataChanged		();
	virtual int16	 MCCOMAPI GetResID					(){ return 280; }

	virtual void	 MCCOMAPI GetMenuCallBack			(ISelfPrepareMenuCallBack** callBack);
	virtual MCCOMErr MCCOMAPI Init						(ISceneDocument* sceneDocument);
	virtual MCCOMErr MCCOMAPI Prepare					();

	virtual boolean  MCCOMAPI CanUndo					(){return true;}
	virtual boolean  MCCOMAPI Do						();
	virtual boolean  MCCOMAPI Undo				();
	virtual boolean  MCCOMAPI Redo				();

private :

	void Validate();

	void Replace();
	void SetShaders(); // for the shader case

	void SwapTree(I3DShTreeElement* tree, const SwapData& data );
	void UnlinkTree(I3DShTreeElement* tree);

	SwapPMap	fPMap;

	boolean			fIsValid;

	// Progress bar
	TMCCountedPtr<IMCUnknown> fProgressKey; // Progress bar key

	TMCCountedPtr<ISceneDocument>	fSceneDocument;
	TMCCountedPtr<I3DShScene>		fScene;
	TMCCountedPtr<ISceneSelection>	fSelection;
	TMCCountedPtr<IChangeChannel>	fSelectionChannel;
	TMCCountedPtr<IChangeChannel>	fHierarchyChannel;
	TMCCountedPtr<IChangeChannel>	fTreePropertyChannel;

	// Undo/redo
	TMCCountedPtr<ISceneSelection>	fCloneSelection;
	TMCArray<RelinkTreeElementInfo>	fUndoRelinkInfo;
	TMCArray<RelinkTreeElementInfo>	fRedoRelinkInfo;
	TMCCountedPtrArray<I3DShTreeElement> fNewTrees;		// array of the new instances
	TMCCountedPtrArray<I3DShMasterShader> fUndoShaders;
};

#endif

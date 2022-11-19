/****************************************************************************************************

		MBuildingProperties.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#ifndef __MBuildingProperties__
#define __MBuildingProperties__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Copyright.h"
#include "BasicPropertiesClient.h"

#include "BuildingModeler.h"
#include "BasicModule.h"
#include "BasicPropertiesClient.h"

void FillInChildrenPopup(IMFPart*	inPart, const TMCString& objName, 
						 BuildingModeler* modeler, const int32 listID);

class BuildingPropertiesClient : public TBasicPropertiesClient
{
protected:

	BuildingPropertiesClient	(	IPropertiesModule* 			inPropertiesModule, 
									BuildingModeler* 			inModeler);
	virtual ~BuildingPropertiesClient();
	
public:

	static void MCCOMAPI Create	(	IPropertiesModule*			propertiesModule,
									BuildingModeler*			inModeler,
									BuildingPropertiesClient**	client);

	STANDARD_RELEASE;

	// IChangeListener methods
	void 		MCCOMAPI 	DataChanged			(IChangeChannel* 	channel, 
												 IDType 			changeKind, 
												 IMCUnknown* 		changedData);
	// IPropertiesClient methods
	void		MCCOMAPI	GetSelection		(ISceneSelection**	outSelection);
	void		MCCOMAPI	GetSelectionChannel	(IChangeChannel**	outChannel);
	IDType		MCCOMAPI	GetControllingModuleID() { return 'BuiM'; }
#if (VERSIONNUMBER >= 0x040000)
	ResourceID	MCCOMAPI	GetPropResID		();
	void		MCCOMAPI	LoadPageData		(IMFPart* 			inTopPart);
	boolean		MCCOMAPI	HandlePageHit		(IMFPart*			inTopPart,
												 int32				inMessage,
												 IMFResponder*		inResponder,
												 void*				inData);
#else
	ResourceID	MCCOMAPI	GetPropResID		(ISceneSelection* 	inSelection);
	void		MCCOMAPI	LoadPageData		(IMFPart* 			inTopPart,
												 ISceneSelection*	inSelection);
	boolean		MCCOMAPI	HandlePageHit		(IMFPart*			inTopPart,
												 ISceneSelection*	inSelection,
												 int32				inMessage,
												 IMFResponder*		inResponder,
												 void*				inData);
#endif
	
		
	void		MCCOMAPI	UnLoadProperties	(IMFSplitBarPart*	inPropertiesSplitBar);
	void		MCCOMAPI	PreLoadProperties	(IMFSplitBarPart*	inPropertiesSplitBar);


	void		MCCOMAPI	GetExtraPart		(IMFPart**			outExtraPart);
	void		MCCOMAPI	GetExtraTabs		(TMCClassArray<TPropertyTab>& extraTabs);
	void		MCCOMAPI	ExtraTabsLoaded		(IMFPart*			inHostPart);

	virtual void			GetPreferedSplit	(int32& outPreferedSize, boolean& outForTopPart);
	virtual void			StorePreferedSplit	(int32 inPreferedSize);

	void					InvalidateDomainList(){fDomainPopupValid=false;}

protected:
	void	LoadCurrentToolInfos(IMFPart* tabPart, PrimitiveStatus* status);
	void	LoadDimensionProperties(IMFPart* inPart, PrimitiveStatus* status);
	void	LoadShadingProperties(IMFPart* inPart, PrimitiveStatus* status);
	void	LoadLevelProperties(IMFPart* inPart, PrimitiveStatus* status);

	void	LoadDefaultLevelProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultRoofProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultWallProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultRoomProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultWindowProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultDoorProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultStairwayProperties(IMFPart* inPart, const BuildingPrimData& data);
	void	LoadDefaultWorkingBoxProperties(IMFPart* inPart, const BuildingPrimData& data);

	void	LoadDomainList(IMFPart* inPart, BuildingPrim* buildingPrimitive);
	void	LoadBackdropData(IMFPart* inPart, BuildingPrim* buildingPrimitive);

	void	FillInActiveLevelPopup(IMFPart*	inPart);
	
	void	DisplayPositionInPart(const TVector2& position, IMFPart* inPart, const int32 xId, const int32 yId );
	void	DisplayValueInPart(const real32 value, IMFPart* inParentPart, const int32 partID );
	void	DisplayValueInPart(const int32 value, IMFPart* inParentPart, const int32 partID );

	void	EditSelectionPosition(IMFPart* part, int32 toolID, PrimitiveStatus* status);

	TMCCountedPtr<BuildingModeler>		fBuildingModeler;

	// Extra part ( countains tool options )
	TMCCountedPtr<IMFPart> fExtraPart;

	// Shading domains popup
	boolean fDomainPopupValid;

	// channels
	TMCCountedPtr<IChangeChannel>		fImmediateUpdateChannel;

	// Global parts
	int32					fSelectedCell; // Selected domain
	TMCArray<int32>			fDomainList; // list of shading domains
	TMCCountedPtr<IMFPart>	fDomainListPart;
	TMCCountedPtr<IMFPart>	fDomainNamePart;

	// Chil popup menu
	int32 fObjectCount;
	int32 fGroupCount;
};

#endif

#endif // !NETWORK_RENDERING_VERSION

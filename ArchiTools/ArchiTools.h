/****************************************************************************************************

		ArchiTools.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#ifndef __Building__
#define __Building__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCColorRGBA.h"
#include "Vector3.h"
#include "MCString.h"
#include "MCException.h"

// Serial number
boolean IsSerialValid();
boolean CanCreateLevel(int32 curLevelCount);
boolean CanCreatePoint(int32 curPointCount);

// Exception
void HandleException(TMCException* exception, const TMCString&);

// constantes
static const real32 kDefaultLevelHeight = -1;
static const real32 kDefaultRoofHeight = -1;
static const real32 kDefaultThickness = -1;
static const int32 kAllLevels = -1;
static const int32 kLastLevel = -2;
static const int32 kGroundLevel = -3;
static const int32 kFirstLevel = -4;
static const int32 kMultipleDomains = -1;
static const int32 kNoDomains = -2;
static const int32 kBasicDomainsCount = 8;
static const int32 kMultipleValues = -1;
static const int32 kNoValue = -2;
static const real32 kNoVecField = 2147483648.0f;
static const real32 kMultiVecField = -2147483648.0f;
static const TVector3 kNoVector(kNoVecField,kNoVecField,kNoVecField); // Just define a vector that we're not going to use
static const TVector3 kMultiVector(kMultiVecField,kMultiVecField,kMultiVecField); // Just define a vector that we're not going to use
static const TMCString15 kNoName = kNullString;
static const TMCString15 kMultiName("#Multi Names#");

// Dialog constantes
static const int32 kAskDomainName = 5300;
static const int32 kAskValue = 5310;
static const int32 kAskObject = 5320;
static const int32 kAskName = 5330;
static const int32 kAskDomain = 5340;
static const int32 kAskCurve = 5350;

// Action constantes
static const int32 kDefaultActionNumber = 1007;

static const int32 kOffsetAction = 4310;
static const int32 kRotateAction = 4311;
static const int32 kScaleAction = 4312;
static const int32 kOffsetObjectAction = 4320;
static const int32 kRotateObjectAction = 4321;
static const int32 kScaleObjectAction = 4322;
static const int32 kDeleteAction = 4330;
static const int32 kActiveLevelAction = 5000; // reserved from 5000 to 5999
static const int32 kActiveLevelActionLimit = 5999; // reserved from 5000 to 5999
static const int32 kChildAction = 6000; // reserved from 6000 to 6999
static const int32 kChildActionLimit = 6999; // reserved from 6000 to 6999

static const int32 kShadingDomainID0 = 9000; // 9000 to 9099 (the popup menu can contain up to 100 items)
static const int32 kShadingDomainID1 = 9100; // 9100 to 9199 (the popup menu can contain up to 100 items)
static const int32 kShadingDomainID2 = 9200; // 9200 to 9299 (the popup menu can contain up to 100 items)
static const int32 kShadingDomainID3 = 9300; // 9300 to 9399 (the popup menu can contain up to 100 items)
static const int32 kCreateShadingDomain0 = 9400;
static const int32 kCreateShadingDomain1 = 9401;
static const int32 kCreateShadingDomain2 = 9402;
static const int32 kCreateShadingDomain3 = 9403;
static const int32 kDeleteShadingDomain = 9420;

/*
enum EFacetMeshType
{
	eShellMesh,
	eShellMeshNoTop,
	eCachedMesh, // Should be: flat, smooth, with or without UV
	eCachedMeshNoTop, // Used when 1 level is displayed in the 3D view
	eCachedFacetedMesh,
	eCachedFacetedMeshNoTop
};
*/
inline void CLEAR_FLAG(int32& flags, const int32 bit) {flags&=~bit;}
inline void	SET_FLAG(int32& flags, const int32 bit) {flags|=bit;}
inline boolean FLAG(const int32 flags, const int32 bit) {return ((flags&bit) != 0 );}

enum EFacetMeshFlags
{
	eShellMesh	= 0x00000001,	// For the Shell
	eNoTop		= 0x00000002,	// Can be use for the Shell or the modeler
	eFaceted	= 0x00000004,	// Only for the modeler
	e2DMesh		= 0x00000008,	// Only for the modeler
};

enum EPickedType
{
	eNothingPicked,
	eBuildingPicked, // <=> BBox ?
//		eFullWallPicked,
	eWallPicked,
	eWallObjectPicked,
	eRoomFloorPicked,
	eRoomCeilingPicked,
	eRoomObjectPicked,
	eBuildingRoof,
	eEdgePicked,
	ePointPicked,
	eRoofPointPicked,
	eProfilePointPicked,
	eWallHolePointPicked,
	eRoomHolePointPicked,
	eWallHandlePointPicked,
	eLevelUpPicked, // The half upper part of the level
	eLevelDownPicked // The half lower part of the level
};

enum EDimensionType
{
	eFloorThickness,
	eCeilingThickness,
	eWallThickness,
	eWallHeight,
	eWallOffset,
	eWallSegments,
	eCrenelHeight,
	eCrenelWidth,
	eCrenelSpacing,
	eCrenelOffset,
	eCrenelShape,
	eRoofMax,
	eRoofMin,
};

enum EFlagType
{
	eNoCeilingFlag,
	eWallExtraHeightFlag,
	eAutoFlipObjFlag,
};

enum EOptionMode
{
	eRegularMode,
	eOption1Mode,
};

enum ERoofProfileID
{
	eCustomProfile,
	eShape0, // Basic one, no profile
	eShape1,
	eShape2,
	eShape3,
	eShape4,
	eShape5,
	eShape6,
	eShape7,
	eShape8,
	eShape9,
	eUnknownProfileType = 0xEEEEEEEE
};

// The list of predefine shading domains
enum EShadingDomains
{
	eOutsideWallDomain = 0x00000000,
	eInsideWallDomain = 0x00000001,
	eInsideFloorDomain = 0x00000002,
	eInsideCeilingDomain = 0x00000003,
	eOutsideBotRoofDomain = 0x00000004,
	eOutsideMidRoofDomain = 0x00000005,
	eOutsideTopRoofDomain = 0x00000006,
	eInsideRoofDomain = 0x00000007,
	eCustomDomain = 0x00000008, // 8 and over: custom domain
};

// Object type (for construction)
enum EObjectType
{
	eWindow,
	eNarrowWindow,
	ePanoramicWindow,
	eArrowWindow,
	eCircle16Window,
	e2Circle16Window,
	e4Circle16LWindow,
	e4Circle16RWindow,
	eDoor,
	eDoubleDoor,
	eArrowDoor,
	e2Circle16Door,
	e4Circle16LDoor,
	e4Circle16RDoor,
	eSquareStairway,
	eLargeStairway,
	eWideStairway,
	eCircle16Stairway,
	eUnknownObjectType = 0xEEEEEEEE
};

// During a Copy/Paste inside the modeler, we first record the ID during the Copy, then
// make a clone during the Paste
enum ECloneChildrenMode
{
	eCloneChild,
	eNoChild,
};

enum EShowHideOption
{
	eShowAll,
	eShowAllWalls,
	eShowAllRooms,
	eShowAllRoofs,
	eShowAllHolePoints,
	eHideAll,
	eHideAllWalls,
	eHideAllRooms,
	eHideAllRoofs,
	eHideAllHolePoints,
	eHideSelection,
	eUnknownShowHideOption = 0xEEEEEEEE
};

// Part IDs
static const IDType kExtraPart = 7300;
// static const IDType kAssembleRoomPart = 408; see kAssembleRoomResID
static const IDType kToolTabPartID = 'Tabs';
enum EToolTabID
{
	kDefaultTab=1,
	kMoveTab,
	kScaleTab,
	kRotateTab,
	kCameraTab,
};

static const IDType eNoOption = 'NoPt';
static const IDType eOption0 = 'Opt0';
static const IDType eOption1 = 'Opt1';
static const IDType eOption2 = 'Opt2';
static const IDType eOption3 = 'Opt3';
static const IDType eOption4 = 'Opt4';
static const IDType eOption5 = 'Opt5';
static const IDType eOption6 = 'Opt6';
static const IDType eOption7 = 'Opt7';
static const IDType eOption8 = 'Opt8';
static const IDType eOption9 = 'Opt9';
static const IDType eOption10 = 'Op10';

static const IDType eName = 'Name';
static const IDType eValue = 'Valu';

static const IDType eMoveX = 'MovX';
static const IDType eMoveY = 'MovY';
static const IDType eOffsetX = 'OffX';
static const IDType eOffsetY = 'OffY';
static const IDType eScaleX = 'ScaX';
static const IDType eScaleY = 'ScaY';
static const IDType eScaleZ = 'ScaZ';
static const IDType eGlobalScale = 'ScaG';
static const IDType eSizeX = 'SizX';
static const IDType eSizeY = 'SizY';
static const IDType eSizeZ = 'SizZ';
static const IDType eRotateAxis = 'axis';
static const IDType eRotateAngle = 'Angl';
static const IDType eXAxis = 'XAXI'; // Keep this ID, it's the one used by the axisChooser
static const IDType eYAxis = 'YAXI'; // Keep this ID, it's the one used by the axisChooser
static const IDType eZAxis = 'ZAXI'; // Keep this ID, it's the one used by the axisChooser

static const IDType e1Pos = 'Pos1';
static const IDType e2Pos = 'Pos2';
static const IDType eXPos2 = 'XPo2';
static const IDType eYPos2 = 'YPo2';
static const IDType eXPos1 = 'XPo1';
static const IDType eYPos1 = 'YPo1';
static const IDType eXPos0 = 'XPo0';
static const IDType eYPos0 = 'YPo0';

static const IDType eObjectPos = 'ObjP';
static const IDType eWallObjectPos = 'WaOP';
static const IDType eRoomObjectPos = 'RoOP';
static const IDType eXDim = 'XDim';
static const IDType eYDim = 'YDim';
static const IDType eCenterPos = 'Cent';
static const IDType eXCenter = 'XCen';
static const IDType eYCenter = 'YCen';

static const IDType eHolePointpartID = 'Hole';
static const uint32 eHolePointpartRes = 3100; // to be able to create it
static const IDType eSmoothBox = 'Smoo';

static const IDType eSelectionNamepart = 'SNaP';
static const IDType eSelectionName = 'SNam';

static const IDType eWallData = 'WaDa';
static const IDType eRoofData = 'RoDa';
static const IDType eRoomData = 'RmDa';

static const IDType eWallGable = 'WaGa';
static const IDType eCrenelData = 'CrDa';

static const IDType eNode0 = 'nod0';
static const IDType eNode1 = 'nod1';
static const IDType eNode2 = 'nod2';
static const IDType eNode3 = 'nod3';

static const IDType ePopup0 = 'pop0';
static const IDType ePopup1 = 'pop1';
static const IDType ePopup2 = 'pop2';
static const IDType ePopup3 = 'pop3';

static const IDType eText0 = 'txt0';
static const IDType eText1 = 'txt1';
static const IDType eText2 = 'txt2';
static const IDType eText3 = 'txt3';

static const IDType eFloorAutoThickness = 'FTAu';
static const IDType eFloorCustomThickness = 'FTCu';
static const IDType eCeilingAutoThickness = 'CTAu';
static const IDType eCeilingCustomThickness = 'CTCu';
static const IDType eDrawCeiling = 'DrCe';
static const IDType eWallAutoHeight = 'WaAu';
static const IDType eWallCustomHeight = 'WaCu';
static const IDType eWallAutoThickness = 'WaAT';
static const IDType eWallCustomThickness = 'WaCT';
static const IDType eWallOption = 'WalO';
static const IDType eWallArcOffset = 'WaOf'; // for curved walls
static const IDType eWallArcSegments = 'WaSg'; // for curved walls
static const IDType eWallCrenelWidth = 'CrWi';
static const IDType eWallCrenelHeight= 'CrHe';
static const IDType eWallCrenelSpacing = 'CrSp';
static const IDType eWallCrenelOffset = 'CrOf';
static const IDType eWallCrenelShape = 'CrSh';
static const IDType eRoofAutoHeight = 'RoAu';
static const IDType eRoofCustomHeight = 'RoCu';
static const IDType eRoofAutoBase = 'RoAB';
static const IDType eRoofCustomBase = 'RoCB';
static const IDType eRoofTopShape = 'TopS';
static const IDType eRoofBottomShape = 'BotS';
static const IDType eRoofTopInside = 'TopI';
static const IDType eRoofBottomInside = 'BotI';
static const IDType eLevelAutoHeight = 'LeAu';
static const IDType eLevelCustomHeight = 'LeCu';
//static const IDType eActiveLevel = 'ActL';
static const IDType eShowActiveLevel = 'ShAO'; // Assemble Room
static const IDType eShowLevelUp = 'ShUp'; // Assemble Room
static const IDType eShowLevelDown = 'ShDn'; // Assemble Room
static const IDType eDisplayActiveLevel = 'DiAL';
static const IDType eDisplayLevelUp = 'DiUp';
static const IDType eDisplayLevelDown = 'DiDn';
static const IDType eDelLevelPartID = 'DelL';
static const IDType eLevelName = 'LNam';

//static const IDType eShowActiveLevel = 'ShLe';
static const IDType eChildrenList = 'Chil';
static const IDType eMenuChildrenList = 'Chld';
static const IDType eAutoFlip = 'AuFl';
static const IDType eChildPlacement = 'PTyp';
static const IDType ePlacementOffsetX = 'POfX';
static const IDType ePlacementOffsetY = 'POfY';
static const IDType ePlacementOffsetZ = 'POfZ';
static const IDType ePlacementScaleX = 'PScX';
static const IDType ePlacementScaleY = 'PScY';
static const IDType ePlacementScaleZ = 'PScZ';
static const IDType ePlacementRotateX = 'PRoX';
static const IDType ePlacementRotateY = 'PRoY';
static const IDType ePlacementRotateZ = 'PRoZ';
static const IDType eNameList = 'NaLi';
static const IDType eDomainList = 'DoLi';

static const IDType eDLevelHeight = 'DLeH';
static const IDType eDRoofMin = 'DRMi';
static const IDType eDRoofMax = 'DRMa';
static const IDType eDWallThickness = 'DWaT';
static const IDType eDFloorThickness = 'DFlT';
static const IDType eDCeilingThickness = 'DCeT';
static const IDType eDWindowHeight = 'DWiH';
static const IDType eDWindowLength = 'DWiL';
static const IDType eDWindowAltitude = 'DWiA';
static const IDType eDDoorHeight = 'DDoH';
static const IDType eDDoorLength = 'DDoL';
static const IDType eDStairwayWidth = 'DStW';
static const IDType eDStairwayLength = 'DStL';

static const IDType eDGridSpacing = 'DSpa';
static const IDType eDWBSizeX = 'DWBX';
static const IDType eDWBSizeY = 'DWBY';
static const IDType eDWBSizeZ = 'DWBZ';
static const IDType eDSnapPrecision = 'DSnP';
static const IDType eDConstrainAngle = 'DCsA';
static const IDType eDSnap = 'DSna';
static const IDType eDDimPrefs = 'Pre1';
static const IDType eDWBPrefs = 'Pre2';

static const int32 ePickWall = 0x00000001;
static const int32 ePickRoom = 0x00000002;
static const int32 ePickEdge = 0x00000004;
static const int32 ePickPoint = 0x00000008;
static const int32 ePickLevel = 0x00000010;
static const int32 ePickRoof = 0x00000020;
static const int32 ePickProfilePoint = 0x00000040;

static const int32 ePick3D = ePickWall+ePickRoom+ePickPoint+ePickRoof+ePickProfilePoint;
static const int32 ePick2D = ePickEdge+ePickRoom+ePickPoint;

// Global Tab
static const int32 eAddDomainPartID = 'AddD';	// Add Button
static const int32 eDelDomainPartID = 'DelD';	// Del Button
static const int32 eNameDomainPartID = 'DNam';	// Name edit text
static const int32 eUVDomainListPartID = 'DomL';	//the Shading Domain List part
static const int32 eUVDomainScrollPartID = 'DomP';	//the Shading Domain part

static const int32 eImageFrontBackPartID = 'imFB';	// file name for the backdrop
static const int32 eEnableFrontBackPartID = 'enFB';	// Enable checkbox
static const int32 eImageLeftRightPartID = 'imLR';	// file name for the backdrop
static const int32 eEnableLeftRightPartID = 'enLR';	// Enable checkbox
static const int32 eImageTopBottomPartID = 'imTB';	// file name for the backdrop
static const int32 eEnableTopBottomPartID = 'enTB';	// Enable checkbox

// Default color
static const uint8 gRedD=230;
static const uint8 gGreenD=240;
static const uint8 gBlueD=250;
/*static const TMCColorRGBA8 gDefCol(gRedD,gGreenD,gBlueD,255);

static const uint8 gRedBD=115;
static const uint8 gGreenBD=120;
static const uint8 gBlueBD=125;
static const TMCColorRGBA8 gDefBCol(gRedBD,gGreenBD,gBlueBD,255);
*/
// Select color
static const uint8 gRedS=230;
static const uint8 gGreenS=160;
static const uint8 gBlueS=140;
/*static const TMCColorRGBA8 gSelCol(gRedS,gGreenS,gBlueS,255);

static const uint8 gRedBS=115;
static const uint8 gGreenBS=80;
static const uint8 gBlueBS=70;
static const TMCColorRGBA8 gSelBCol(gRedBS,gGreenBS,gBlueBS,255);
*/
// Target color
static const uint8 gRedT=200;
static const uint8 gGreenT=200;
static const uint8 gBlueT=140;
/*static const TMCColorRGBA8 gTarCol(gRedT,gGreenT,gBlueT,255);

static const uint8 gRedBT=100;
static const uint8 gGreenBT=100;
static const uint8 gBlueBT=70;
static const TMCColorRGBA8 gTarBCol(gRedBT,gGreenBT,gBlueBT,255);
*/
// Freeze color
static const uint8 gRedF=180;
static const uint8 gGreenF=180;
static const uint8 gBlueF=240;
/*static const TMCColorRGBA8 gFreCol(gRedF,gGreenF,gBlueF,255);

static const uint8 gRedBF=100;
static const uint8 gGreenBF=100;
static const uint8 gBlueBF=140;
static const TMCColorRGBA8 gFreBCol(gRedBF,gGreenBF,gBlueBF,255);
*/
// Helper color
static const uint8 gRedH=180;
static const uint8 gGreenH=240;
static const uint8 gBlueH=250;
/*static const TMCColorRGBA8 gHelCol(gRedH,gGreenH,gBlueH,255);

static const uint8 gRedBH=90;
static const uint8 gGreenBH=120;
static const uint8 gBlueBH=125;
static const TMCColorRGBA8 gHelBCol(gRedBH,gGreenBH,gBlueBH,255);
*/
// Snaped color
static const uint8 gRedSn=255;
static const uint8 gGreenSn=110;
static const uint8 gBlueSn=110;
/*static const TMCColorRGBA8 gSnaCol(gRedSn,gGreenSn,gBlueSn,255);

static const uint8 gRedBSn=250;
static const uint8 gGreenBSn=60;
static const uint8 gBlueBSn=65;
static const TMCColorRGBA8 gSnaBCol(gRedBSn,gGreenBSn,gBlueBSn,255);
*/




#endif

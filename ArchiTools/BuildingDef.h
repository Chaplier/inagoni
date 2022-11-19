/****************************************************************************************************

		BuildingDef.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#ifndef __BuildingDef__
#define __BuildingDef__

// Network Rendering tag
#define NETWORK_RENDERING_VERSION 0

// File version tag
#define FILE_VERSION_NUMBER 1.3f

// We define the CLSID

#define R_CLSID_BuildingPrim 0xc18ae9ff, 0x2dd2, 0x40fa, 0xbd, 0x3a, 0x79, 0xb4, 0x49, 0xdc, 0xc9, 0xb3
#define R_CLSID_HousePrim 0x623c4977, 0xb69a, 0x4b75, 0x91, 0x22, 0x60, 0xa4, 0xa4, 0x4a, 0x89, 0x7
#define R_CLSID_BuildingModeler 0x76abb5c2, 0x7f39, 0x4c33, 0xb5, 0xb, 0xd8, 0x4, 0x75, 0x8b, 0x14, 0x6f
#define R_CLSID_BuildingPanePart 0x7d308711, 0xf16, 0x48a1, 0xa6, 0x7b, 0x9, 0x3a, 0x36, 0x5c, 0x5a, 0xd8
#define R_CLSID_BuildingDropCandidate 0xcf11a01e, 0x19bc, 0x4687, 0xb7, 0xeb, 0x34, 0x79, 0xc6, 0x23, 0xeb, 0x0
#define R_CLSID_BuildingDropArea 0x96005a5b, 0x3a1f, 0x44c5, 0xb9, 0x2d, 0x62, 0x71, 0xc6, 0x2a, 0xa9, 0x92
#define R_CLSID_BuildingRoofProfileComp 0x551d7090, 0xdfd, 0x4b4b, 0xa8, 0xa6, 0x9, 0xe4, 0xa7, 0x74, 0x74, 0xa3
#define R_CLSID_BuildingClip 0xe3ede890, 0x4d6, 0x429e, 0x81, 0x6f, 0xd2, 0x27, 0x2b, 0xfd, 0x6e, 0x57
#define R_CLSID_AssembleRoomPart 0xc23da01f, 0xaa4, 0x45f3, 0x92, 0x67, 0xa7, 0x8, 0xf6, 0xa2, 0x3, 0x7a


// Define some constantes

#define kBuildingPrimitiveResID				187
#define kHousePrimitiveResID				188
#define kBuildingModelerResID				197
#define kBuildingModelerPanePartResID		207
#define kBuildingPropertiesResID			7217
#define kBuildingDropCandidateResID			227
#define kBuildingDropAreaResID				237
#define kBuildingRoofProfileDataComp		247

#define kAssembleRoomResID					408//407

#define kAssembleRoomPartExtID				417

#define kBuildingMenuBar					1007
#define kBuildingToolBar					8007
#define kBuildingPrefs						177
#define kBuildingModelerWindow				1407

#define kPrimitiveStrings					147
#define kModelerStrings						157
#define kShadingDomainsStrings				158
#define kAlertStrings						159

// Menu IDs : do not use IDs between 128 and 255, they are reserved for Carrara
#define kCreateMenu							107
#define kInsertLevelSubmenu					120		// 208
#define kInsertLevelSubmenuStr				"\0x78"	// "\0xD0"
#define kCreatetRoofSubmenu					121		// 209: doesn't work, maybe used by something else
#define kCreatetRoofSubmenuStr				"\0x79"	// "\0xD1": doesn't work, maybe used by something else
#define kViewMenu							117
//#define kShowSubmenu						218 // The shortcut didn't work with the sub menu
//#define kShowSubmenuStr						"\0xDA"
//#define kHideSubmenu						219
//#define kHideSubmenuStr						"\0xDB"
#define kSelectionMenu						127
#define kSelectSubmenu						122		
#define kSelectSubmenuStr					"\0x7A"	
#define kDeselectSubmenu					123		
#define kDeselectSubmenuStr					"\0x7B"	
#define kReplaceSubmenu						124		
#define kReplaceSubmenuStr					"\0x7C"	

// Menu action items
#define kInsertWindowMenuAction				2007
#define kInsertDoorMenuAction				2008
#define kInsertStairwayMenuAction			2017
#define kLevelTopMenuAction					2027
#define kLevelBottomMenuAction				2028
#define kLevelOnActiveMenuAction			2029
#define kRoofProfileMenuAction				2033
#define kCreateRoomMenuAction				2037
#define kCreateRoof1MenuAction				2038
#define kCreateRoof2MenuAction				2039
#define kCreateRoof3MenuAction				2040
#define kCreateRoof4MenuAction				2041
#define kCreateRoof5MenuAction				2042
#define kCreateRoof6MenuAction				2043
#define kCreateRoof7MenuAction				2044
#define kCreateRoof8MenuAction				2045
#define kCreateRoof9MenuAction				2046
#define kCreateRoof10MenuAction				2077
#define kAttachObjectMenuAction				2047
#define kPlaceObjectChildMenuAction			2048
#define kHideSelectionMenuAction			2057
#define kHideAllMenuAction					2058
#define kHideWallsMenuAction				2059
#define kHideRoomsMenuAction				2060
#define kShowAllMenuAction					2061
#define kShowWallsMenuAction				2062
#define kShowRoomsMenuAction				2063
#define kInvertSelectionMenuAction			2064
#define kHideRoofsMenuAction				2065
#define kShowRoofsMenuAction				2066
#define kShowActiveLevel					2067
#define kLevelHeightMenuAction				2117
#define kWallThicknessMenuAction			2118
#define kWallHeightMenuAction				2119
#define kFloorThicknessMenuAction			2120
#define kCeilingThicknessMenuAction			2121
#define kRoofHeightMenuAction				2122
#define kRoofBaseMenuAction					2123
#define kSplitMenuAction					2124
#define kChildMenuAction					2125
#define kDrawCeilingMenuAction				2126
#define kMergeMenuAction					2127
#define kNameMenuAction						2128
#define kSelectByName						2129
#define kDeselectByName						2130
#define kSelectByShadingDomain				2131
#define kDeselectByShadingDomain			2132
#define kRebuildMenuAction					2133
#define kDuplicateOverAction				2134
#define kDuplicateUnderAction				2135
#define kDetachMenuAction					2136
#define kMoveOverMenuAction					2137
#define kMoveUnderMenuAction				2138
#define kHolesEditionOnOff					2139
#define kSmoothSelection					2140
#define kImportCurveMenuAction				2141
#define kWallArcOffsetMenuAction			2142
#define kWallArcSegmentsMenuAction			2143
#define kReplaceBySimpleWall				2144
#define kReplaceByWallWithCrenel			2145
// System (missing from SDK
#define	kaConvertToOtherModeler		(k3DEditorHostActionsIDBase +  600)	//19600

// Tool items
#define kBuildWallTool						3001
#define kInsertWindowTool					3007
#define kInsertNarrowWindowTool				3008
#define kInsertPanoWindowTool				3009
#define kInsertCircleWindowTool				3010
#define kInsert2CircleWindowTool			3011
#define kInsert4CircleLWindowTool			3012
#define kInsert4CircleRWindowTool			3013
#define kInsertArrowWindowTool				3014

#define kInsertDoorTool						3017
#define kInsertDoubleDoorTool				3018
#define kInsert2CircleDoorTool				3019
#define kInsert4CircleLDoorTool				3020
#define kInsert4CircleRDoorTool				3021
#define kInsertArrowDoorTool				3022

#define kInsertSquareStairwayTool			3027
#define kInsertLargeStairwayTool			3028
#define kInsertWideStairwayTool				3029
#define kInsertCircleStairwayTool			3030

#define kInsertShellLevelTool				3037
#define kInsertDuplicateLevelTool			3038
#define kInsertEmptyLevelTool				3039

#define kDeleteTool							3047

#define kBuildWallWithCrenel1Tool			3051
#define kBuildWallWithCrenel2Tool			3052
#define kBuildWallWithCrenel3Tool			3053
#define kBuildWallWithCrenel4Tool			3054
#define kBuildWallWithCrenel5Tool			3055

// These should be in the SDK
#define kMenuFile 2
#define kMenuEdit 3
#define kMenuWindow 8
#define kMenuWeb 10

// Cursor ID
#define kDeleteCursor						1600
#define kMoveCursor							1604
#define kAddCursor							1613
#define kCantAddCursor						1615
#define kCantSnapCursor						1623
#define kSelectCursor						1630
#define kCantSelectCursor					1635

#endif



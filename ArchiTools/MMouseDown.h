/****************************************************************************************************

		MMouseDown.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	4/2/2004

****************************************************************************************************/

#if !NETWORK_RENDERING_VERSION

#ifndef __MMouseDown__
#define __MMouseDown__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BuildingDef.h"
#include "BuildingPrimitiveData.h"

#include "MCBasicTypes.h"
#include "Vector2.h"
#include "ArchiTools.h"
#include "MCArray.h"

class BuildingModeler;
class BuildingPanePart;

class TMCPoint;
class TMCPlatformEvent;
class TMCString;
struct I3DShObject;
class CommonPoint;

enum EPlacementType;

struct MouseDown
{
	// Use for Move, Scale and Rotate tools
	static boolean			SelectTool(	BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										const TMCPlatformEvent& event, 
										BuildingPanePart* pane,
										const int32 currentTool);

	// Use to build wall point by point
	static boolean			BuildWallTool(	BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										BuildingPanePart* pane,
										const int32 currentTool);
	// Use to insert wall or room objects
	static boolean			InsertObjectTool(	BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										BuildingPanePart* pane,
										const int32 currentTool);
	// Use to insert a new level
	static boolean			InsertLevelTool(BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										BuildingPanePart* pane,
										const int32 currentTool);

	static boolean			DeleteTool(BuildingModeler* modeler,  
										const TMCPoint& mousePos, 
										BuildingPanePart* pane,
										const int32 currentTool);
};

struct MenuAction
{
	static boolean			CopyCut(		BuildingModeler* modeler, 
											const int32 menuAction );
	static boolean			Paste( BuildingModeler* modeler );
	static boolean			Duplicate(	BuildingModeler* modeler, 
										const int32 actionNumber );
	static boolean			CutAndPaste(	BuildingModeler* modeler, 
										const int32 actionNumber );
	static boolean			SetShadingDomain(	BuildingModeler* modeler, 
											const int32 menuAction );
	static boolean			ShadingDomain(	BuildingModeler* modeler, 
											const int32 menuAction,
											const int32 domainID,
											const int32 subPartID,
											const TMCString& domainName );
	static boolean			DelShadingDomain(	BuildingModeler* modeler, 
												const int32 domainID );
	static boolean			MovePoint(		BuildingModeler* modeler, 
											const TVector2& offset,
											CommonPoint* point);
	static boolean			MoveSelection(	BuildingModeler* modeler,  
											const TVector2& offset );
	static boolean			MoveObject(	BuildingModeler* modeler,  
										const TVector2& offset );
	static boolean			ScaleSelection(	BuildingModeler* modeler,  
											const TVector2& offset );
	static boolean			ScaleObject(BuildingModeler* modeler,  
										const TVector2& scale );
	static boolean			RotateSelection(BuildingModeler* modeler,  
											const TVector2& cosSin );
	static boolean			InsertObject(	BuildingModeler* modeler,  
											const int32 currentTool);
	static boolean			InsertLevel(BuildingModeler* modeler,
										const int32 currentTool);
	static boolean			DeleteSelection(BuildingModeler* modeler);
	static boolean			DeleteLevel(BuildingModeler* modeler,
										const int32 level);
	static boolean			CreateRoom(BuildingModeler* modeler);
	static boolean			CreateRoof(BuildingModeler* modeler, const int32 actionNumber);
	static boolean			Split( BuildingModeler* modeler );
	static boolean			Merge( BuildingModeler* modeler, const boolean inOne );
	static boolean			SetLevelHeight(	BuildingModeler* modeler, 
											const TMCArray<int32>& levels, 
											const real32 height);
	static boolean			SetDimension(BuildingModeler*	modeler, 
										const real32		thickness,
										const EDimensionType	type);

	static boolean			SetUInt32Value(BuildingModeler*	modeler, 
										const uint32		value,
										const EDimensionType	type);

	static boolean			SetSelectionFlag(BuildingModeler*	modeler, 
										const boolean			flagValue,
										const EFlagType			flagType);

	static boolean			SmoothSelection(BuildingModeler*	modeler,
										const boolean			value);

	static boolean			SetSelectionName(BuildingModeler*	modeler, 
											const TMCString&	name);

	static boolean			SetLevelName(	BuildingModeler*	modeler, 
											const int32			level, 
											const TMCString&	name);

	static boolean			SetRoofProfile(	BuildingModeler*	modeler, 
											const ERoofProfileID profile,
											const boolean		onTop,
											const boolean		inside);

	static boolean			SetObjectInstance(BuildingModeler*	modeler,
											TMCString& objectName, const boolean isObject ); // can be either an object or a group
	static boolean			SetObjectPlacementType(BuildingModeler* modeler,
											EPlacementType placement );

	static boolean			SetObjectPlacement(BuildingModeler* modeler,
												const real32 value,
												const int32 id);

	static boolean			ShowHide(	BuildingModeler* modeler,
										const int32 currentTool);
	static boolean			ShowActiveLevel(	BuildingModeler* modeler,
										const int32 level);
	static boolean			HoleEditionOnOff(	BuildingModeler* modeler);
	static boolean			SetDefaultSetting(	BuildingModeler* modeler,
											const real32 value,
											const int32 id);
	static boolean			InvertSelection( BuildingModeler* modeler );
	static boolean			SelectDeselectBy( BuildingModeler* modeler, const int32 actionNumber );
	static boolean			SelectAll( BuildingModeler* modeler );
	static boolean			FlipSelection( BuildingModeler* modeler );

	static boolean			ImportCurve( BuildingModeler* modeler );

	static boolean			ReplaceWall(BuildingModeler* modeler, const int32 actionNumber);
};

#endif

#endif // !NETWORK_RENDERING_VERSION

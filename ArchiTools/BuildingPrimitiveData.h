/****************************************************************************************************

		BuildingPrimitiveData.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/30/2004

****************************************************************************************************/

#ifndef __BuildingPrimitiveData__
#define __BuildingPrimitiveData__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedPtr.h"
#include "MCClassArray.h"
#include "Vector2.h"
#include "TBBox.h"
#include "MCColorRGBA.h"
struct IMFResponder; // missing from I3DEditorHostPart.h
struct IMCGraphicContext; // missing from I3DEditorHostPart.h
class TMCRect; // missing from I3DEditorHostPart.h
#include "MCArray.h" // missing from I3DEditorHostPart.h
#include "I3DEditorHostPart.h"
#include "ArchiTools.h"
#include "PNames.h"

#include "POutlinePoint.h"

class Level;
class Wall;
class Room;
class Roof;
class VPoint;
class CommonPoint;
class RoofPoint;
class SubObject;

enum EUVMappingMethod
{
	eProportional,
	eUnfold
};

enum EUVArea
{
	eWallLeft,
	eWallRight,
	eWallBetween,
	eCeiling,
	eFloor,
	eLevelBetween,
	eRoofTop,
	eRoofOut,
	eRoofBottom,
	eRoofIn,
	eRoofFlat,	// Flat portion of the roof at its top
	eRoofSection // vertical section at the roof extremities
};

enum EPlacementType
{
	eFitIn,
	eSmaller,
	eSlightlySmaller,
	eBigger,
	eSlightlyBigger,
	eCustom,
	eFreePlacement,
	eMultiPlacement,	// Usefull for the multi selection
	eNoPlacement,		// Usefull for the multi selection
};

struct CrenelData
{
	real32 mDepth;
	real32 mWidth;
	real32 mSpacing;
	real32 mOffset;
	int32 mShape; // ECrenelShape
};

struct UVMaker
{
public:
	UVMaker() : mMethod(eProportional),
				mDefaultUVLength(108) // mDefaultUVLength=10; change the unit from 1 inch to 3 feet

	{}

	TVector2		ComputeUV(const TVector2& pos) const { return pos/mDefaultUVLength; }

	EUVMappingMethod mMethod;

	// Data use for all UV methods
	real32 mDefaultUVLength;
};


struct PrimitiveStatus
{
	void AddObj(SubObject* obj);

	boolean fHasSelection;

	boolean fCouldSplit; // true if 2 following profile points, 2 following section points or a wall are selected

	int32	fLevelCount;
	int32	fWallCount;
	int32	fRoomCount;
	int32	fRoofCount;
	int32	fWallObjectCount;
	int32	fRoomObjectCount;
//	int32	fRoofObjectCount;
	int32	fPointCount;

	int32	fPartialySelectedLevelCount;

	int32	fSelectedLevelCount;
	int32	fSelectedWallGlobalCount;
	int32	fSelectedSimpleWallCount;
	int32	fSelectedWallWithCrenelCount;
	int32	fSelectedRoomCount;
	int32	fSelectedRoofCount;
	int32	fSelectedWallObjectCount;
	int32	fSelectedRoomObjectCount;
//	int32	fSelectedRoofObjectCount;
	int32	fSelectedPointCount;
	int32	fSelectedRoofPointCount;
	int32	fSelectedProfilePointCount;
	int32	fSelectedWallHolePointCount;
	int32	fSelectedRoomHolePointCount;
	int32	fSelectedCommonPointCount;	// == fSelectedPointCount+fSelectedRoofPointCount+fSelectedProfilePointCount
										// + fSelectedWallHolePointCount + fSelectedRoomHolePointCount

	TVector2	fFirstSelectedPos;
	TVector2	fSecondSelectedPos;

	// Shading Domains

	int32	fDomainRoomFloor;
	int32	fDomainRoomCeiling;
	int32	fDomainRoomWalls;

	int32	fDomainWallLeft;
	int32	fDomainWallRight;
	int32	fDomainWallBetween;

	int32	fDomainRoofOutTop;
	int32	fDomainRoofOutMid;
	int32	fDomainRoofOutBot;
	int32	fDomainRoofInside;

	// Multi selection data
	real32	fWallThickness;
	real32	fWallHeight;
	int32	fWallExtraHeight; // 0: false, 1: true, kMultipleValues: multi
	real32	fWallOffset;	// can be >0 or <0. If kMultiVecField: different values
	int32	fWallSegments; // 1 or more. If ==kMultipleValues: differents values
	CrenelData mCrenel;
	real32	fFloorThickness;
	real32	fCeilingThickness;
	boolean fNoCeiling;
	boolean fAutoFlip; // auto flip object children
	real32	fRoofMax;
	real32	fRoofMin;
	real32	f2DObjWidth;
	real32	f2DObjHeight;
	TVector2	f2DObjCenter;
	EPlacementType fObjPlacement;
	TVector3	fObjOffset;
	TVector3	fObjScale;
	TVector3	fObjRotate;
	TMCDynamicString	fSceneObjectName; // Child
	TMCDynamicString	fWallSelectionName;
	TMCDynamicString	fRoomSelectionName;
	TMCDynamicString	fRoofSelectionName;
	TMCDynamicString	fPointSelectionName;
	TMCDynamicString	fObjectSelectionName;
	int32	fIsSmoothed;

	TBBox3D 	fSelectionBBox; // Approximate bbox of the selection
};

struct BuildingPrimData
{
public:
	BuildingPrimData();

	// Default values
	const inline real32	GetDefaultWallThickness()const{return fDefaultWallThickness;}
	const inline real32	GetDefaultWallHeight()const{return fDefaultLevelHeight;}
	const inline real32	GetDefaultFloorThickness()const{return fDefaultFloorThickness;}
	const inline real32	GetDefaultCeilingThickness()const{return fDefaultCeilingThickness;}
	const inline real32	GetDefaultLevelHeight()const{return fDefaultLevelHeight;}
	const inline real32	GetDefaultRoofMin()const{return fDefaultRoofMin;}
	const inline real32	GetDefaultRoofMax()const{return fDefaultRoofMax;}
	const inline real32	GetDefaultFloorToCeiling()const{return fDefaultLevelHeight-(fDefaultFloorThickness+fDefaultCeilingThickness);}
	const inline real32	GetDefaultCeilingToFloor()const{return (fDefaultFloorThickness+fDefaultCeilingThickness);}
	const inline real32	GetDefaultWindowHeight() const {return fDefaultWindowHeight;}
	const inline real32	GetDefaultWindowLength() const {return fDefaultWindowLength;}
	const inline real32	GetDefaultWindowAltitude() const {return fDefaultWindowAltitude;}
	const inline real32	GetDefaultDoorHeight() const {return fDefaultDoorHeight;}
	const inline real32	GetDefaultDoorLength() const {return fDefaultDoorLength;}
	const inline real32	GetDefaultStairwayWidth() const {return fDefaultStairwayWidth;}
	const inline real32	GetDefaultStairwayLength() const {return fDefaultStairwayLength;}
	// Grid (save in the primitive too)
	const inline real32	GetDefaultGridSpacing() const {return fDefaultGridSpacing;}
	const inline real32	GetDefaultWBSizeX() const {return fDefaultWBSizeX;}
	const inline real32	GetDefaultWBSizeY() const {return fDefaultWBSizeY;}
	const inline real32	GetDefaultWBSizeZ() const {return fDefaultWBSizeZ;}
	const inline real32	GetDefaultSnapPrecision() const {return fDefaultSnapPrecision;}
	const inline real32	GetDefaultConstrainAngle() const {return fDefaultConstrainAngle;}
	const inline boolean	GetDefaultSnap() const {return fDefaultSnap;}
	const inline boolean	GetIsNew() const {return fIsNew;}
	const inline real32	GetGeneralSize() const {return fSceneMagnitude;}

	TMCCountedPtrArray<OutlinePoint> GetDefaultPolyline(EObjectType type);

	void				SetDefaultLevelHeight( const real32 value, const boolean check = true );
	void				SetDefaultRoofMin( const real32 value );
	void				SetDefaultRoofMax( const real32 value );
	void				SetDefaultWallThickness( const real32 value );
	void				SetDefaultFloorThickness( const real32 value, const boolean check = true );
	void				SetDefaultCeilingThickness( const real32 value, const boolean check = true );
	void				SetDefaultWindowHeight( const real32 value, const boolean check = true );
	void				SetDefaultWindowLength( const real32 value );
	void				SetDefaultWindowAltitude( const real32 value, const boolean check = true );
	void				SetDefaultDoorHeight( const real32 value, const boolean check = true );
	void				SetDefaultDoorLength( const real32 value );
	void				SetDefaultStairwayWidth( const real32 value );
	void				SetDefaultStairwayLength( const real32 value );
	void				SetDefaultGridSpacing( const real32 value ){fDefaultGridSpacing = value;}
	void				SetDefaultWBSizeX( const real32 value ){fDefaultWBSizeX = value;}
	void				SetDefaultWBSizeY( const real32 value ){fDefaultWBSizeY = value;}
	void				SetDefaultWBSizeZ( const real32 value ){fDefaultWBSizeZ = value;}
	void				SetDefaultSnapPrecision( const real32 value ){fDefaultSnapPrecision = value;}
	void				SetDefaultConstrainAngle( const real32 value ){fDefaultConstrainAngle = value;}
	void				SetDefaultSnap( const boolean value ){fDefaultSnap = value;}
	void				SetIsNew( const boolean value ){fIsNew = value;}
	void				SetGeneralSize(real32 value) {fSceneMagnitude = value;}

	// Status flag
	const inline boolean	IsStatusValid()const{return fStatusValid;}
	inline void			InvalidateStatus() {fStatusValid=false;}
	inline void			SetStatusIsValid() {fStatusValid=true;}

	// UV method
	const UVMaker&		UVData() const { return mUVData; }
	UVMaker&			UVData() { return mUVData; }

	// View settings
	inline void			SetPaneConfig( const EPaneConfig c){fConfig=c;}
	inline EPaneConfig	GetPaneConfig() const {return fConfig;}

	// Display setting for the Assemble Room (different to the one in the modeling room who need to 
	// post action because of the selction that can be modified)
	void				AssembleRoomShowLevel(const int32 level){fARShowLevel = level;}
	int32				AssembleRoomShowLevel() const {return fARShowLevel;}
	void				AssembleRoomShowAll(const boolean all){fARShowAll = all;}
	boolean				AssembleRoomShowAll() const {return fARShowAll;}

	// Backdrop settings
	inline void			SetFBEnable( const boolean enable ){fFBEnable=enable;}
	inline boolean		GetFBEnable() const {return fFBEnable;}
	inline void			SetLREnable( const boolean enable ){fLREnable=enable;}
	inline boolean		GetLREnable() const {return fLREnable;}
	inline void			SetTBEnable( const boolean enable ){fTBEnable=enable;}
	inline boolean		GetTBEnable() const {return fTBEnable;}
	inline void			SetFBBackdrop( const TMCString& str){fFBBackdrop=str;}
	inline const TMCString&	GetFBBackdrop() const {return fFBBackdrop;}
	inline void			SetLRBackdrop( const TMCString& str){fLRBackdrop=str;}
	inline const TMCString&	GetLRBackdrop() const {return fLRBackdrop;}
	inline void			SetTBBackdrop( const TMCString& str){fTBBackdrop=str;}
	inline const TMCString&	GetTBBackdrop() const {return fTBBackdrop;}

	// Holes edition settings
	inline void			SetHoleEditEnable( const boolean enable ){fHoleEditEnable=enable;}
	inline boolean		GetHoleEditEnable() const {return fHoleEditEnable;}

	inline BuildingPrimData&	operator= (const BuildingPrimData& fromData);

	void Validate();

protected:

	// PMap start here

	real32 fDefaultLevelHeight;
	real32 fDefaultRoofMin;
	real32 fDefaultRoofMax;
	real32 fDefaultWallThickness;
	real32 fDefaultFloorThickness;
	real32 fDefaultCeilingThickness;

	real32 fDefaultWindowHeight;
	real32 fDefaultWindowLength;
	real32 fDefaultWindowAltitude;

	real32 fDefaultDoorHeight;
	real32 fDefaultDoorLength;

	real32 fDefaultStairwayWidth;
	real32 fDefaultStairwayLength;

	// Grid pmap
	real32 fDefaultGridSpacing;
	real32 fDefaultWBSizeX;
	real32 fDefaultWBSizeY;
	real32 fDefaultWBSizeZ;
	real32 fDefaultSnapPrecision;
	real32 fDefaultConstrainAngle;
	boolean fDefaultSnap;

	boolean fIsNew; // A flag to clearly separate the prefs from the prim

	// View settings: more usefull when they're local to the primitive
	// Save the interactive view settings
	EPaneConfig	fConfig;

	// Assemble Room display settings
	boolean fARShowAll;
	int32	fARShowLevel;

	// Backdrop
	boolean	fFBEnable;
	boolean	fLREnable;
	boolean	fTBEnable;
	TMCDynamicString fFBBackdrop; // Front Back
	TMCDynamicString fLRBackdrop; // Left Right
	TMCDynamicString fTBBackdrop; // Top Bottom

	// Holes edition
	boolean fHoleEditEnable;

	// PMap end here

	real32 fSceneMagnitude;

	UVMaker mUVData;

	boolean fDefaultValid;

	// Here so everybody has the ability to invalidate the status
	boolean	fStatusValid;

	void AddPoint(TMCCountedPtrArray<OutlinePoint>& inArray, const TVector2& pos );
	void InitOutlineData(TMCCountedPtrArray<OutlinePoint>& outline);
	TMCCountedPtrArray<OutlinePoint> fDefaultWindow;
	TMCCountedPtrArray<OutlinePoint> fDefaultDoor;
	TMCCountedPtrArray<OutlinePoint> fDefaultStairway;
	TMCCountedPtrArray<OutlinePoint> fDefaultCircle;

public:
	// Color (not part of the pmap: they come from the prefs)
	TMCColorRGBA8 fDefCol;
	TMCColorRGBA8 fObjCol; // a slightly different color from def for the objects
	TMCColorRGBA8 fSelCol;
	TMCColorRGBA8 fTarCol;
	TMCColorRGBA8 fFreCol;
	TMCColorRGBA8 fHelCol;
	TMCColorRGBA8 fSnaCol;
	TMCColorRGBA8 fBRoCol; // Color used to represent the botom roof points
	TMCColorRGBA8 fTRoCol; // Color used to represent the top roof points
	TMCColorRGBA8 fFloCol; // Floor color for the 2D view

	// List of the names used by the objects
	NameChainedList		fDictionary;
};


inline BuildingPrimData&	BuildingPrimData::operator= (const BuildingPrimData& fromData)
{
	// Copy the PMap params
	fDefaultLevelHeight = fromData.fDefaultLevelHeight;
	fDefaultRoofMin = fromData.fDefaultRoofMin;
	fDefaultRoofMax = fromData.fDefaultRoofMax;
	fDefaultWallThickness = fromData.fDefaultWallThickness;
	fDefaultFloorThickness = fromData.fDefaultFloorThickness;
	fDefaultCeilingThickness = fromData.fDefaultCeilingThickness;

	fDefaultWindowHeight = fromData.fDefaultWindowHeight;
	fDefaultWindowLength = fromData.fDefaultWindowLength;
	fDefaultWindowAltitude = fromData.fDefaultWindowAltitude;

	fDefaultDoorHeight = fromData.fDefaultDoorHeight;
	fDefaultDoorLength = fromData.fDefaultDoorLength;

	fDefaultStairwayWidth = fromData.fDefaultStairwayWidth;
	fDefaultStairwayLength = fromData.fDefaultStairwayLength;

	fDefaultGridSpacing = fromData.fDefaultGridSpacing;
	fDefaultWBSizeX = fromData.fDefaultWBSizeX;
	fDefaultWBSizeY = fromData.fDefaultWBSizeY;
	fDefaultWBSizeZ = fromData.fDefaultWBSizeZ;
	fDefaultSnapPrecision = fromData.fDefaultSnapPrecision;
	fDefaultConstrainAngle = fromData.fDefaultConstrainAngle;
	fDefaultSnap = fromData.fDefaultSnap;
	fIsNew = fromData.fIsNew;

	fConfig = fromData.fConfig;

	fARShowAll = fromData.fARShowAll;
	fARShowLevel = fromData.fARShowLevel;
	// PMap end here

	mUVData = fromData.mUVData;

	// Invalidate
	fDefaultValid = false;
	fStatusValid = false;

	return *this;
}


#endif

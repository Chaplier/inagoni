/****************************************************************************************************

		BuildingPrim.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/22/2004

****************************************************************************************************/

#ifndef __BuildingPrim__
#define __BuildingPrim__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"
#include "BuildingDef.h"
#include "ArchiTools.h"
#include "BasicPrimitive.h"
#include "PublicUtilities.h"
#include "IShTokenStream.h"

#include "BuildingPrimitiveData.h"
#include "PLevel.h"
#include "Utils.h"
#include "PNames.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "RealQuat.h"
#endif
#include "I3DShFacetMesh.h"

#include "IMFExPart.h"
class Quotation;

extern const MCGUID CLSID_BuildingPrim;
extern const MCGUID CLSID_HousePrim;

struct PosAndObj
{
	PosAndObj() : mPosition(TVector3::kZero), mPoint(NULL){}
	PosAndObj(const TVector3& pos, OutlinePoint* pt) : mPosition(pos), mPoint(pt){}
	TVector3 mPosition;
	TMCCountedPtr<OutlinePoint> mPoint;
};

class BuildingPrim :	public TBasicPrimitive, 
						public IExStreamIO//, 
					//	public I3DExAnimated 
{
public:
	BuildingPrim();
//	BuildingPrim(int32 whichOne);
	virtual ~BuildingPrim();
  
	STANDARD_RELEASE;
  
	// IMCUnknown methods
	MCCOMErr		MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	uint32			MCCOMAPI AddRef(){ return TBasicPrimitive::AddRef(); }

	// IExDataExchanger methods :
	void			MCCOMAPI Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter);
	void*			MCCOMAPI GetExtensionDataBuffer();
	virtual int32	MCCOMAPI GetParamsBufferSize() const {return sizeof(BuildingPrimData);}
	MCCOMErr		MCCOMAPI ExtensionDataChanged();
	MCCOMErr		MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);
	short			MCCOMAPI GetResID();
  
	// I3DExGeometricPrimitive methods
	void			MCCOMAPI GetBoundingBox(TBBox3D& bbox);
	MCCOMErr		MCCOMAPI GetNbrLOD(short &nbrLod);
	MCCOMErr		MCCOMAPI GetLOD(short lodIndex,real &lod);
	MCCOMErr		MCCOMAPI GetFacetMesh(uint32 lodIndex, FacetMesh** outMesh);

	boolean			MCCOMAPI CanBeSplit(){ return false;}
//	MCCOMErr		MCCOMAPI SplitPrimitive(TMCCountedPtrArray<I3DExGeometricPrimitive>& subParts, TMCArray<TTransform3D>& subPartPositions);
	boolean			MCCOMAPI AutoSwitchToModeler() const;

	// Shading Calls
	uint32			MCCOMAPI GetUVSpaceCount();
	MCCOMErr		MCCOMAPI GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);


	// IExStreamIO methods
	MCCOMErr MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	MCCOMErr MCCOMAPI Write(IShTokenStream* stream);
	MCCOMErr MCCOMAPI FinishRead		(IStreamContext* streamContext);
 
	MCCOMErr ReadContent(IShTokenStream* stream);
	MCCOMErr WriteContent(IShTokenStream* stream);

	// I3DExAnimated methods
//	MCCOMErr MCCOMAPI RegisterParams();
//	MCCOMErr MCCOMAPI InvalidateCaches(long itsID);
//	MCCOMErr MCCOMAPI CopyTimeData(IMCUnknown *dest);

	MCCOMErr			GetFacetMesh(uint32 lodIndex, FacetMesh** outMesh, int32 meshFlags);
	void				GetOther2DMeshes(	TBBox3D&		bBox,
											TSegmentMesh&	wallSegmentMesh, 
											TPointMesh&		wallPointMesh,
											TSegmentMesh&	objectSegmentMesh, 
											TPointMesh&		objectHandleMesh,
											const int32		inLevel );
	void				GetOther3DMeshes(	TBBox3D&		bBox,
											TSegmentMesh&	segmentMesh, 
											TPointMesh&		pointMesh,
											const int32		inLevel );
	void				GetQuotation( Quotation& quotation, const int32 level, const EUnits unit );

	void				GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection);

	// Picking
	EPickedType			Pick(	CommonBase **object,
								int32 flags, 
								FatRay& fatRay, 
								TVector3& hitPosition,
								const int32 inLevel,
								const boolean inPlanView);

	// Does a picking with the flat squares representing the walls
	Wall*				BasicPickBestWall(const TVector3& origin,
										  const TVector3& direction,
										  TVector3& hitPosition,
										  const int32 inLevel); 
	Wall*				ProjectOnNearestWall(TVector3& pos);
	// Does a picking with a polygon at mid level in the room (at handle height)
	Room*				BasicPickBestRoom(const TVector3& origin,
										  const TVector3& direction,
										  TVector3& hitPosition,
										  const int32 inLevel); 
	Room*				GetNearestRoom(const TVector3& pos);

	// To offset an object with the possibility to jump to another parent
	void				OffsetWallObjJump(const TUnitQuaternion& rot, const TVector3& offset, const real32 originDist, const int32 inLevel, const boolean constrainDir, const boolean constrainDist);
	void				OffsetRoomObjJump(const TVector2& projOffset, const int32 inLevel);
	// To offset an object but staying inside their parent
	void				OffsetObj(const TVector2& projOffset, const int32 inLevel);
	void				ScaleObj(const TVector2& scale, const int32 inLevel);
	void				RotateObj(const TVector2& cosSin, const int32 inLevel);

	void				UpdateFacetMeshColors( FacetMesh* facetMesh, int32 flags );
	void				BuildExtendedSelection();
	void				CheckExtendedSelection(const int32 inLevel);
	void				InvalidateSelection(const boolean andBBox, const int32 inLevel);
	void				InvalidateExtendedSelection(const int32 inLevel);
	void				InvalidateObjectSelection(const int32 inLevel);
	void				InvalidateAll(const int32 inLevel);
	void				InvalidateStatus(){fData.InvalidateStatus();}

	inline void			InvalidateBBox(){fBBoxValid=false;}

	void				SetSelection(const boolean select);
	void				InitMarqueeSelection(const int32 inLevel);
	void				SetMarquee(const TMCArray<Plane> &rayPlanes, const int32 marqueeMode,const int32 inLevel);
	void				ShowHide(const EShowHideOption option);

	// Data access
	inline int32		GetLevelCount(){return fLevelArray.GetElemCount();}
	inline Level*		GetLevel( int32 index ){return fLevelArray[index];}
	void				AddLevelToArray(Level* level, const int32 atLevel);
	void				RemoveLevelFromArray(const int32 index, const int32 count);
	const int32			GetGroundLevelIndex() const;
	Level*				GetGroundLevel() const;
	void				SetGroundLevelIndex(const int32 index);
	// Distance from the ground level to the last one ( the floor of the last one)
	const real32		GetAltitude(const int32 level) const; 
	// Merged the passed level with the one having the same index + levelOffset
	void				MergeLevel( Level* level, const int32 levelOffset );

	Wall*				GetFirstSelectedWall( const int32 inLevel );
	Room*				GetFirstSelectedRoom( const int32 inLevel );
	Roof*				GetFirstSelectedRoof( const int32 inLevel );
	CommonPoint*		GetFirstSelectedPoint( const int32 inLevel );
	CommonPoint*		GetSecondSelectedPoint( const int32 inLevel );
	CommonPoint*		GetSelectedPoint( const int32 iPoint, const int32 inLevel );

	// Modeler display
	boolean				ShowAll() const {return fShowAll;}
	int32				ActiveLevel() const {return fActiveLevel;}
	void				ActiveLevel(const int32 l);
	void				ShowAll(const boolean s);
//	int32				GetShownLevel()const {return fShowLevel;}
//	void				ShowLevel(const int32 level);

	// Assemble Room display
	int32				ShellMeshLevel() const {return fData.AssembleRoomShowAll()?kAllLevels:AssembleRoomShowLevel();}
	void				AssembleRoomShowLevel(const int32 level);
	int32				AssembleRoomShowLevel() const {return fData.AssembleRoomShowLevel();}
	void				AssembleRoomShowAll(const boolean all){fData.AssembleRoomShowAll(all);}
	boolean				AssembleRoomShowAll() const {return fData.AssembleRoomShowAll();}

	void				ClearPointFlag(const int32 flag, const int32 inLevel);
	void				ClearWallFlag(const int32 flag, const int32 inLevel);
	void				ClearRoomFlag(const int32 flag, const int32 inLevel);

	VPoint*				GetPointHelper(const int32 inLevel);

	PrimitiveStatus*	GetStatus();
	const BuildingPrimData&	Data() const {return fData;}
	BuildingPrimData&	GetData() {return fData;}

	// Modification
	void				OffsetSelection(const TVector2& offset, const int32 inLevel);
	void				ScaleSelection(const TVector2& scale, const TVector2& center, const EOptionMode mode, const int32 inLevel);
	void				RotateSelection(const TVector2& cosSin, const TVector2& center, const int32 inLevel);
	void				SetSelectionPos( const TMCClassArray<TVector2>& positions, const int32 inLevel );
	void				SetSelectionObjPos( const TMCClassArray<TVector2>& objPpositions, const int32 inLevel );
	void				InsertNewLevel(const int32 inWhere, const boolean over, const int32 type);
	void				SetWallHeight(const real32 height, TMCArray<real32>& undoData, const int32 inLevel);
	void				SetWallThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel);
	void				SetRoofMax(const real32 max, TMCArray<real32>& undoData, const int32 inLevel);
	void				SetRoofMin(const real32 min, TMCArray<real32>& undoData, const int32 inLevel);
	void				SetNoCeiling(const boolean noCeiling, TMCArray<boolean>& undoData, const int32 inLevel);
	void				SetWallExtraHeight(const boolean extraHeight, TMCArray<boolean>& undoData, const int32 inLevel);
	void				SetAutoFlipObjects(const boolean extraHeight, TMCArray<boolean>& undoData, const int32 inLevel);
	void				SetRoofProfile(const ERoofProfileID profile, const boolean onTop, const boolean inside, const int32 inLevel);
	void				SetFloorThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel);
	void				SetCeilingThickness(const real32 thickness, TMCArray<real32>& undoData, const int32 inLevel);
	void				Split(const int32 inLevel);
	void				Merge(const int32 inLevel);
	void				SetSelectionName(const TMCString& name, TMCClassArray<TMCDynamicString>& undoData, const int32 inLevel);
	void				Smooth(const boolean smooth, const int32 inLevel);
	void				SetWallOffset(real32 offset, boolean computeSegCount, const int32 inLevel);

	VPoint*				MakePoint( const TVector2& pos, const int32 inLevel );

	Wall*				MakeWall( VPoint* point1, VPoint* point2, EWallType type, const int32 inLevel);

	WallSubObject*		MakeWallSubObject( Wall* onWall, const EObjectType objectKind );
	RoomSubObject*		MakeRoomSubObject( Room* inRoom, const EObjectType objectKind );

	boolean				BuildRoomFromSelection(const int32 inLevel);
	Roof*				BuildRoof(const ERoofType roofType);

	void				DeleteSelection(const int32 inLevel);
	void				InvertSelection(const int32 inLevel);
	void				SelectByName(const TMCString& name, const boolean select, const int32 inLevel);
	void				SelectByDomain(const int32 domain, const boolean select, const int32 inLevel);
	void				FlipSelection(const TVector2& center, const int32 axis, const int32 inLevel);

	// Constrains
	boolean				SnapPosWithAxis(TVector2& pos, const TVector2& axis, const TVector2& preferedProjDir,const int32 inLevel);

	void				SetSelectionHelper( const boolean set, const int32 inLevel );
	void				CheckSelectionConsistency( const int32 inLevel );

	void				SetAllLevelDistanceToGround();

	// Shading domain
	// Set the shading domain of the current selection
	void				SetShadingDomain(const int32 domainID, const int32 selectionSubPart);
	// Return the ID of the new domain
	int32				AddShadingDomain(const TMCString& domainName);
	void				DelShadingDomain(const int32 domainID, int32 replaceByID);
	UVSpaceInfo&		GetUVSpace(uint32 uvSpaceID){ return fShadingDomains[uvSpaceID]; }

	void				GetInstances( TMCCountedPtrArray<I3DShInstance>& instances );

	void				AttachObjectToSelection(I3DShObject* sceneObject, const int32 inLevel, TMCClassArray<TMCDynamicString>&	undoData);
	void				SetPlacementType(const EPlacementType placement, const int32 inLevel);
	void				SetPlacement(const real32 value, const int32 id, const int32 inLevel);

	void				GetScene(I3DShScene** scene);

	//	tree elem access
	I3DShTreeElement*	GetPrivateGroup( I3DShTreeElement* buildingTreeElement );
	const TTreeIdPath	GetPrivateGroupIdPath() const { return fPrivateGroupTreePath; }
	I3DShTreeElement*	CreatePrivateGroup( I3DShTreeElement* buildingTreeElement );

	inline const int32		LevelCount(const int32 inLevel) const;
	inline const int32		StartLevel(const int32 inLevel) const;

protected:
	Wall*			PickedBestWall( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, const int32 inLevel, Level** levelPicked);
	Wall*			PickedBestEdge( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, const int32 inLevel);
	Wall*			PickedBestWallHandle( const FatRay& fatRay, TVector3& hitPosition, real32& minAlpha, const int32 inLevel);
	Room*			PickedBestRoom( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, const int32 inLevel, Level** levelPicked);
	CommonPoint*	PickedBestPoint( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, EPickedType& pickedType, const int32 inLevel);
	CommonPoint*	PickedBestProfilePoint( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, EPickedType& pickedType, const int32 inLevel);
	CommonPoint*	PickedBestHolePoint(const FatRay& fatRay, TVector3& hitPosition, real32& minAlpha, EPickedType& pickedType, const int32 inLevel);
	Roof*			PickedBestRoof( const FatRay& fatRay, TVector3& hitPosition, real32 &minalpha, const int32 inLevel, Level** levelPicked);
//	SubObject*		PickedBestObject( const FatRay& fatRay, TVector3& hitPosition, real32& minAlpha, EPickedType& pickedType, const int32 inLevel);
	OutlinePoint*	PickedBestHandle( const FatRay& fatRay, TVector3& hitPosition, real32& minAlpha, const int32 inLevel, TMCClassArray<PosAndObj>& handleList);
	boolean			PickTriangles(	const TMCClassArray<Triangle>& triangles,
								const TMCClassArray<Vertex>& vertices,
								real32 &alpha,const FatRay& fatRay, TVector3& hitPosition);

	void				Init();
	void				DefaultShadingDomainList();

	const TMCColorRGBA8&	GetColor(CommonBase* obj);
	const TMCColorRGBA8&	GetColor(RoofPoint* roofPoint, bool zone);

	void				AddRoofTo2DMesh(Roof*			roof, 
									   TPointMesh&		pointMesh,
									   TSegmentMesh&	segmentMesh, 
									   real32			zOffset);
	void				AddWallTo2DMesh(Wall* wall,
										TPointMesh&		geomPointMesh,
										TSegmentMesh&	segmentMesh,
										float zUp);
	void				Add2DObjectToMesh( SubObject* object,
											 TSegmentMesh&	objectSegmentMesh,
											 TPointMesh&	objectHandleMesh,
											 const boolean	useFreezeColor,
											 TMCClassArray<PosAndObj>& handlesCache);
	void				AddObjectToMesh( SubObject* object,
											 TSegmentMesh&	objectSegmentMesh,
											 TPointMesh&	objectHandleMesh,
											 const boolean	useFreezeColor,
											 TMCClassArray<PosAndObj>& handlesCache,
											 const boolean oneHandle);
	void				AddEditableObjectToMesh(SubObject*		object,
											TSegmentMesh&	objectSegmentMesh,
											TPointMesh&		objectHandleMesh,
											const boolean	useFreezeColor,
											 TMCClassArray<PosAndObj>& handlesCache);

	inline boolean		AddPointSelection( const CommonPoint* point, int32& count );

	inline const int32		LevelIndex(const int32 levelID) const;
	inline const boolean	GetActualLevel( int32& useLevel, const int32 inLevel) const;

	TMCString255			ReadRelativePathName( TMCiostream& iostream, const TMCString& knownName);

	TBBox3D 	fBBox;    // BoundingBox
	boolean		fBBoxValid;

	int16		fResourceID;

	TTreeIdPath	fPrivateGroupTreePath;

	// Modeler view settings (part of the primitive to be include in the undo)
	boolean		fShowAll;
	int32		fActiveLevel;
//	int32		fShowLevel; // kAllLevels if all are visible

	// The levels
	int32						fGroundLevelIndex;
	TMCCountedPtrArray<Level>	fLevelArray;
	
	// Shading domain list: might move into the global data
	TMCClassArray<UVSpaceInfo>	fShadingDomains;

	// Data global to the primitive. (primitive pmap)
	BuildingPrimData	fData;

	// Selection status
	PrimitiveStatus		fStatus;

	// Cached data: the handles
	// In a 3D view
	TMCClassArray<PosAndObj> mWallObjectHandles3D;
	TMCClassArray<PosAndObj> mRoomObjectHandles3D;
	// In the plan view
	TMCClassArray<PosAndObj> mWallObjectHandles2D; 
	TMCClassArray<PosAndObj> mRoomObjectHandles2D;
};

inline const int32	BuildingPrim::LevelCount(const int32 inLevel) const 
{ 
	if(inLevel==kAllLevels||inLevel==kLastLevel)
		return fLevelArray.GetElemCount();
	else if(inLevel==kFirstLevel)
		return 1;
	else 
		return inLevel+1; 
}

inline const int32	BuildingPrim::StartLevel(const int32 inLevel) const 
{ 
	if(inLevel==kAllLevels || inLevel==kFirstLevel)
		return 0;
	else if(inLevel==kLastLevel)
		return fLevelArray.GetElemCount()-1;
	else 
		return inLevel; 
}

inline const int32	BuildingPrim::LevelIndex(const int32 levelID) const 
{ 
	if(levelID==kAllLevels||levelID==kLastLevel)
		return fLevelArray.GetElemCount()-1;
	else if(levelID==kFirstLevel)
		return 0;
	else return
		levelID; 
}

inline const boolean	BuildingPrim::GetActualLevel( int32& useLevel, const int32 inLevel) const
{
	if(!fShowAll) // don't display top
	{
		if(inLevel!=kAllLevels && inLevel!=fActiveLevel)
			return false;
	}
	useLevel = (fShowAll?inLevel:fActiveLevel); 

	return true;
}


#endif

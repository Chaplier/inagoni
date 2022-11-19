/****************************************************************************************************

		PLevel.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/29/2004

****************************************************************************************************/

#ifndef __PLevel__
#define __PLevel__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedPtr.h"
#include "TBBox.h"

#include "PCommonBase.h"
#include "PPlan.h"
#include "BuildingPrimitiveData.h"
#include "BasicRenderables.h"
#include "I3DShFacetMesh.h"

struct IShTokenStream;
class BuildingPrim;

enum ELevelFlags
{
	eShowLevelAlone = 0x00010000,
};


// Positive value are kept for duplicate i level
const int32 eNewEmptyLevel = -1;
const int32 eNewDuplicateUnder = -2;
const int32 eNewDuplicateShellUnder = -3;

enum EWallType
{
	eBasic,
	eWithCrenel1,
	eWithCrenel2,
	eWithCrenel3,
	eWithCrenel4,
	eWithCrenel5
};

class Level :	public CommonBase
{
public:

	static void			CreateLevel(Level **newLevel, BuildingPrim* prim, Level* levelUnder, Level* levelOver);
	void				DeleteLevel();
	void				ReleaseReference();

	void				Clone(Level** newLevel, BuildingPrim* inPrimitive, const ECloneChildrenMode cloneMode);

	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	inline void			SetLevelOver(Level* level){fLevelOver = level;}
	inline void			SetLevelUnder(Level* level){fLevelUnder = level;}

	inline Level*		GetLevelOver(){return fLevelOver;}
	inline Level*		GetLevelUnder(){return fLevelUnder;}

	void				SetDistanceToGround(const real32 dist);
	const inline real32	GetDistanceToGround()const{return fDistanceToGround;}

	const inline real32	GetLevelHeight() const { return(fLevelHeight==kDefaultLevelHeight?fData->GetDefaultLevelHeight():fLevelHeight);}
	void				SetLevelHeight(const real32 h);

	void				SetDefaultName();

	void				SetSelection(const boolean select);
	void				SelectIfPossible();
	void				ShowHide(const EShowHideOption option);

	void				GetLevelFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlags);

	VPoint*				MakePoint( const TVector2& pos );

	Wall*				MakeWall( VPoint* point1, VPoint* point2, EWallType type, const boolean tryBuildRoom );
	Wall*				MakeWall( const TVector2& pos1, const TVector2& pos2 );

	Room*				MakeRoom( TMCCountedPtrArray<VPoint>& path, const boolean turnleft );

	Roof*				MakeRoof( const TMCCountedPtrArray<VPoint>& area, const ERoofType roofType );

	void				InvalidateTessellation();

	// Return the bounding box of the level. If exact is true, it will use
	// the facetisation to determine it, otherwise it will use the plan (wall 
	// without thickness )
	void				GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection);

	const inline int32	GetLevelIndex()const{return fLevelIndex;}
	inline void			SetLevelIndex(const int32 index){fLevelIndex=index;}

	inline int32		GetRoomCount(){return fLevelPlan.GetRoomCount();}
	inline int32		GetWallCount(){return fLevelPlan.GetWallCount();}
	inline int32		GetPointCount(){return fLevelPlan.GetPointCount();}
	inline int32		GetRoofCount(){return fLevelPlan.GetRoofCount();}

	inline Room*		GetRoom(const int32 i){return fLevelPlan.GetRoom(i);}
	inline Wall*		GetWall(const int32 i){return fLevelPlan.GetWall(i);}
	inline VPoint*		GetPoint(const int32 i){return fLevelPlan.GetPoint(i);}
	inline Roof*		GetRoof(const int32 i){return fLevelPlan.GetRoof(i);}

	inline Plan&		LevelPlan(){return fLevelPlan;}

	inline BuildingPrimData*	GetPrimitiveData() const {return fData;}
	inline BuildingPrim*		GetPrimitiveNoAddRef() const {return fBuildingPrimitive;}

	void				BuildPossibleRooms(Wall* onWall, const boolean onLeft=true, const boolean onRight=true);

	void				GetInstances( TMCCountedPtrArray<I3DShInstance>& instances );

	boolean				IsEmpty(){return (fLevelPlan.GetPointCount()==0&&fLevelPlan.GetRoofCount()==0);}

protected:

	Level(BuildingPrim* prim, Level* levelUnder, Level* levelOver);
	virtual ~Level();

	Plan	fLevelPlan;

	TMCCountedPtr<Level>	fLevelUnder;
	TMCCountedPtr<Level>	fLevelOver;

	BuildingPrimData* fData;
//	TMCCountedPtr<BuildingPrim> fBuildingPrimitive;
	BuildingPrim* fBuildingPrimitive; // Can't count this one: it's the parent


	real32	fDistanceToGround;
	real32	fLevelHeight;
	int32	fLevelIndex;
};







#endif

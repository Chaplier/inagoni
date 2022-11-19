/****************************************************************************************************

		PRoof.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/17/2004

****************************************************************************************************/

#ifndef __PRoof__
#define __PRoof__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "PCommonBase.h"
#include "MCCountedPtr.h"

#include "PRoofProfile.h"
#include "PRoofZone.h"

#include "ArchiTools.h"
#include "PVertex.h"
#include "I3dShFacetMesh.h"
#include "BuildingModelerEnums.h"

struct BuildingPrimData;
struct IShTokenStream;
class Level;
class RoofZone;


class Roof : public CommonBase
{
public:

	static void			CreateRoof(Roof **newRoof, BuildingPrimData* data, Level* inLevel);
	void				DeleteRoof();

	void				Clone(Roof** newRoof, Level* inLevel);

	MCCOMErr			Write(IShTokenStream* stream);
	MCCOMErr			Read(IShTokenStream* stream); 

	inline void			SetArea(const TMCCountedPtrArray<VPoint>& area, const ERoofType roofType) 
							{fZone.SetArea(area,this,roofType);}

	bool				GetRoofFacetMesh(FacetMesh** outMesh, uint32 lodindex, int32 meshFlag);

	void				GetBoundingBox(TBBox3D& bbox, const boolean exact, const boolean onSelection);

	real32				GetRoofMin() const { return (fRoofMin==kDefaultLevelHeight?fData->GetDefaultRoofMin():fRoofMin);}
	void				SetRoofMin(const real32 min);
	real32				GetRoofMax() const { return (fRoofMax==kDefaultRoofHeight?fData->GetDefaultRoofMax():fRoofMax);}
	void				SetRoofMax(const real32 max);
	real32				GetMinToGround() const;
	real32				GetMaxToGround() const;

	TMCClassArray<Triangle>& Triangles(){ValidateTessellation(); return fTriangleArray;}
	TMCClassArray<Vertex>& Vertices(){ValidateTessellation(); return fVertexArray;}

	inline int32		GetRoofZoneSectionCount() const {return fZone.GetSectionCount();}
	inline ZoneSection&	GetRoofZoneSection(const int32 iPt) {return fZone.GetZoneSection(iPt);}
	inline boolean			RemoveRoofZoneSection(const int32 iPt);
	inline void			InsertRoofZoneSection(const int32 i, const ZoneSection& newSection) {fZone.GetZoneArray().InsertElem(i, newSection);}

	void				RemoveProfilePointReferences(ProfilePoint* point);

	virtual void		SetSelection(const boolean select);
	void				SelectIfPossible();
	void				OffsetSelection(const TVector2& offset);
	void				ScaleSelection(const TVector2& scale, const TVector2& center, const EOptionMode mode);
	void				RotateSelection(const TVector2& cosSin, const TVector2& center);

	void				ShowRoof();
	void				HideRoof();

	// Profile access
	inline TMCCountedPtrArray<ProfilePoint>& GetBotProfile() const { return fProfile->GetBotProfile(); }
	inline int32		GetBotProfilePointCount() const { return fProfile->GetBotProfilePointCount(); }
	const TVector2&		GetBotProfilePos(const int32 i) const {return fProfile->GetBotProfilePos(i);}
	ProfilePoint*		GetBotProfilePoint(const int32 i) const {return fProfile->GetBotProfilePoint(i);}

	inline TMCCountedPtrArray<ProfilePoint>& GetTopProfile() const { return fProfile->GetTopProfile(); }
	inline int32		GetTopProfilePointCount() const { return fProfile->GetTopProfilePointCount(); }
	const TVector2&		GetTopProfilePos(const int32 i) const {return fProfile->GetTopProfilePos(i);}
	ProfilePoint*		GetTopProfilePoint(const int32 i) const {return fProfile->GetTopProfilePoint(i);}

	inline TMCCountedPtrArray<ProfilePoint>& GetBotInside() const { return fProfile->GetBotInside(); }
	inline int32		GetBotInsidePointCount() const { return fProfile->GetBotInsidePointCount(); }
	const TVector2&		GetBotInsidePos(const int32 i) const {return fProfile->GetBotInsidePos(i);}
	ProfilePoint*		GetBotInsidePoint(const int32 i) const {return fProfile->GetBotInsidePoint(i);}

	inline TMCCountedPtrArray<ProfilePoint>& GetTopInside() const { return fProfile->GetTopInside(); }
	inline int32		GetTopInsidePointCount() const { return fProfile->GetTopInsidePointCount(); }
	const TVector2&		GetTopInsidePos(const int32 i) const {return fProfile->GetTopInsidePos(i);}
	ProfilePoint*		GetTopInsidePoint(const int32 i) const {return fProfile->GetTopInsidePoint(i);}

	inline int32		GetTotalProfilePointCount() const { return	fProfile->GetBotProfilePointCount()+
																	fProfile->GetTopProfilePointCount()+
																	fProfile->GetBotInsidePointCount()+
																	fProfile->GetTopInsidePointCount(); }

	void				SetTopProfile(const ERoofProfileID profile);
	void				SetBotProfile(const ERoofProfileID profile);
	void				SetTopInside(const ERoofProfileID profile);
	void				SetBotInside(const ERoofProfileID profile);

	Level*				GetLevel() const {return fLevel;}
	void				SetLevel(Level* level);

	BuildingPrimData*	GetData() const {return fData;}
	void				SetData(BuildingPrimData* data);

	// Shading Domains
	void				SetOutTopDomain( const int32 id ){ fOutTopDomain=id; }
	void				SetOutMidDomain( const int32 id ){ fOutMidDomain=id; }
	void				SetOutBotDomain( const int32 id ){ fOutBotDomain=id; }
	void				SetInsideDomain( const int32 id ){ fInsideDomain=id; }

	int32				GetOutTopDomain() const { return fOutTopDomain; }
	int32				GetOutMidDomain() const { return fOutMidDomain; }
	int32				GetOutBotDomain() const { return fOutBotDomain; }
	int32				GetInsideDomain() const { return fInsideDomain; }

	real32				GetVerticalBase(const TVector3& curZonePos, const TVector3& nextZonePos,
						  TVector3& direction, TVector3& inNormal,
						  const TVector3& curSpinePos, const TVector3& nextSpinePos);

	// Return the triangles used for the boolean operation on the walls and rooms
	void				GetInsideTriangleVertices(TMCArray<TriangleVertices>& triangles);
	
	// Data use to unfold
	struct UnfoldUV
	{
		UnfoldUV() : mOffsetU(0), mOffsetV(0) {}
		real32	mOffsetU; // offset in U, between the sides of a roof
		real32	mOffsetV; // offset in V between the roofs
	};
	Roof::UnfoldUV& UnfoldUVData() { return mUnfoldUVData; }

protected:

	Roof(BuildingPrimData* data, Level* inLevel);
	virtual ~Roof();

	TVector2 ComputeUV(const TVector2& pos);

	void BuildRectangle(const TVector3& pos0,
						const TVector3& pos1,
						const TVector3& pos2,
						const TVector3& pos3,
						const TVector3& normal,
						const TVector2& UV0,
						const TVector2& UV1,
						const TVector2& UV2,
						const TVector2& UV3,
						TMCClassArray<Triangle>& triangles);
	void GetRectangleUV(const TVector3& curBotProj0, 
					   const TVector3& curBotProj1, 
					   const TVector3& curTopProj0, 
					   const TVector3& curTopProj1,
					   real32& cummulatedVOffset,
					   TVector2& UV0,
					   TVector2& UV1,
					   TVector2& UV2,
					   TVector2& UV3, 
					   EUVArea area );

	// Helper method
	boolean BuildNextRectangle( const TVector2& next,
					    const TVector3& curBottomNormal,
					    const TVector3& curBottomDir,
					    const TVector3& botPrevPlaneNormal,
					    const TVector3& botNextPlaneNormal,
						const TVector3& curZonePos,
						const TVector3& nextZonePos,
						const EUVArea shadingArea,
						TVector3& curBotProj0,
						TVector3& curBotProj1,
						real32& cummulatedVOffset,
						real32& uDist0,
						real32& uDist1,
						TMCClassArray<Triangle>& triangles,
						const boolean flip);

	void ValidateTessellation();

	BuildingPrimData*	fData;

	TMCCountedPtr<RoofProfile> fProfile;
	RoofZone			fZone;

	TMCCountedPtr<Level>	fLevel;

	real32				fRoofMin;	// Starting altitude
	real32				fRoofMax;	// Ending altitude

	// Shading Domains
	int32				fOutTopDomain;
	int32				fOutMidDomain;
	int32				fOutBotDomain;
	int32				fInsideDomain;
	
	// UV
	UnfoldUV	mUnfoldUVData;

	// tessellation data
	TMCClassArray<Triangle>	fTriangleArray;
	TMCArray<int32>			fTriangleDomain;
	TMCClassArray<Vertex>	fVertexArray;
	int32					fOutsideOffset; // triangles from 0 to fOutsideOffset are inside the roof,										// from fOutsideOffset to the end are outside
};

inline boolean Roof::RemoveRoofZoneSection(const int32 iPt)
{
	fZone.GetZoneArray().RemoveElem(iPt,1);

	if(fZone.GetSectionCount() < 3)
	{
		DeleteRoof();
		return false;
	}
	else
	{
		ClearFlag(eRoofTessellated);
		return true;
	}
}









#endif

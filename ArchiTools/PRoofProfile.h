/****************************************************************************************************

		PRoofProfile.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	5/16/2004

****************************************************************************************************/

#ifndef __PRoofProfile__
#define __PRoofProfile__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCBasicTypes.h"
#include "MCCountedObject.h"
#include "MCCompObj.h"
#include "BasicCOMImplementations.h"
#include "MCClassArray.h"
#include "MCCountedPtrArray.h"
#include "PCommonBase.h"
#include "Vector2.h"
#include "Vector3.h"
struct IShTokenStream;

class Roof;

///////////////////////////////////////////////////////////////////////////
//
//	ProfilePoint
//
enum EProfilePointFlags
{
	eTopProfileFlag = 0x00010000,
	eOutProfileFlag = 0x00020000,
};

class ProfilePoint :	public CommonPoint
{
public:
//	ProfilePoint():CommonPoint(TVector2::kZero){}

	static void			CreateProfilePoint(ProfilePoint **point, BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean onSpine, const boolean outside);

	void				DeleteProfilePoint();

	void				Clone(ProfilePoint** newPoint, Roof* onRoof);

	void				SetSelection(const boolean select);

	inline virtual void	InvalidateTessellation(const boolean extraInvalidation=false);

	Roof*				GetRoof()const{return fRoof;}

protected:

	ProfilePoint(BuildingPrimData* data, const TVector2& pos, Roof* onRoof, const boolean topProfile, const boolean outside);
	virtual ~ProfilePoint();

	TMCCountedPtr<Roof> fRoof;
};


///////////////////////////////////////////////////////////////////////////
//
//	RoofProfile
//
// A profile can countain 0 pos. In that case, we just use the
// fZonePos and fSpinePos of the RoofZone to define the roof
class RoofProfile : public TMCCountedObject
{
public:
	static void			CreateProfile(RoofProfile **profile, Roof* onRoof, const int32 type);

	inline int32		GetBotProfilePointCount() const { return fBotProfilePoints.GetElemCount(); }
	const TVector2&		GetBotProfilePos(const int32 i) const {return fBotProfilePoints[i]->Position();}
	ProfilePoint*		GetBotProfilePoint(const int32 i) const {return fBotProfilePoints[i];}
	inline int32		GetTopProfilePointCount() const { return fTopProfilePoints.GetElemCount(); }
	const TVector2&		GetTopProfilePos(const int32 i) const {return fTopProfilePoints[i]->Position();}
	ProfilePoint*		GetTopProfilePoint(const int32 i) const {return fTopProfilePoints[i];}

	inline int32		GetBotInsidePointCount() const { return fBotInsidePoints.GetElemCount(); }
	const TVector2&		GetBotInsidePos(const int32 i) const {return fBotInsidePoints[i]->Position();}
	ProfilePoint*		GetBotInsidePoint(const int32 i) const {return fBotInsidePoints[i];}
	inline int32		GetTopInsidePointCount() const { return fTopInsidePoints.GetElemCount(); }
	const TVector2&		GetTopInsidePos(const int32 i) const {return fTopInsidePoints[i]->Position();}
	ProfilePoint*		GetTopInsidePoint(const int32 i) const {return fTopInsidePoints[i];}

	inline TMCCountedPtrArray<ProfilePoint>& GetBotProfile(){return fBotProfilePoints;}
	inline TMCCountedPtrArray<ProfilePoint>& GetTopProfile(){return fTopProfilePoints;}
	inline TMCCountedPtrArray<ProfilePoint>& GetBotInside(){return fBotInsidePoints;}
	inline TMCCountedPtrArray<ProfilePoint>& GetTopInside(){return fTopInsidePoints;}

	inline void			ClearData(){fBotProfilePoints.ArrayFree();
									fTopProfilePoints.ArrayFree();
									fBotInsidePoints.ArrayFree();
									fTopInsidePoints.ArrayFree();}

	void				Copy(RoofProfile* fromProfile, Roof* roof);

	MCCOMErr			Read(IShTokenStream* stream, Roof* roof);

protected:
	RoofProfile(Roof* roof, const int32 type);

	MCCOMErr			ReadArray(IShTokenStream* stream, TMCCountedPtrArray<ProfilePoint>& array, 
									Roof* roof, const boolean onTop, const boolean outside);

		// Profile: start from the bottom to the top.
	// The first and the last will be fixed to the zone (first == fZonePos, last = fSpinePos)
	// If the extrusion is not regular, the segment beetween fElasticIndex
	// and fElasticIndex+1 will be distorted
	TMCCountedPtrArray<ProfilePoint> fBotProfilePoints; // from 0 = fZonePos
	TMCCountedPtrArray<ProfilePoint> fTopProfilePoints; // from 0 = fSpinePos

	// First and last points are the same as in the profile.
	// Used when the roof has vertical areas => draw a wall using this delimitation
	TMCCountedPtrArray<ProfilePoint> fBotInsidePoints;
	TMCCountedPtrArray<ProfilePoint> fTopInsidePoints;
};

#endif

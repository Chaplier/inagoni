/****************************************************************************************************

		ReplicateCommand.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/13/2004

****************************************************************************************************/

#ifndef __ReplicateCommand__
#define __ReplicateCommand__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "InstanciatorDef.h"
#include "ShaderFactory.h"
#include "Basic3DCOMImplementations.h"
//#include "BitField.h"
//#include "I3DShFacetMesh.h"
#include "I3DShScene.h"
//#include "I3DExModifier.h"
#include "ISceneSelection.h"
#include "IChangeManagement.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "MCCountedPtrArray.h"
#include "MCCountedPtr.h"
#include "IShComponent.h" // for IShParameterComponent
//#include "IShRasterLayer.h"
//#include "IShChannel.h"
#include "InstanciatorEnum.h"
#include "Transforms.h"
//#include "BasicWireframe.h"
#include "ShaderTypes.h"
#include "MCRandom.h"


extern const MCGUID CLSID_ReplicateCommand;
extern const MCGUID CLSID_ReplicateData;

// Noise offset values
const real32 kPosXScale = 1.17f;
const real32 kPosYScale = 2.63f;
const real32 kPosZScale = -.89f;
const TVector3 kPosXOffset(.33f,-.6f,3.45f);
const TVector3 kPosYOffset(-.5f,-4.12f,1.03f);
const TVector3 kPosZOffset(1.9f,4.1f,2.21f);

const real32 kScaUScale = .97f;
const TVector3 kScaUOffset(2.61f,1.03f,-.47f);

const real32 kScaXScale = -1.65f;
const real32 kScaYScale = 1.3f;
const real32 kScaZScale = -.76f;
const TVector3 kScaXOffset(1.56f,2.92f,1.7f);
const TVector3 kScaYOffset(2.41f,-1.42f,-1.08f);
const TVector3 kScaZOffset(-.91f,1.58f,.78f);

const real32 kRotXScale = -2.01f;
const real32 kRotYScale = -1.34f;
const real32 kRotZScale = 1.54f;
const TVector3 kRotXOffset(1.23f,-1.11f,1.71f);
const TVector3 kRotYOffset(-.89f,-1.81f,.96f);
const TVector3 kRotZOffset(2.83f,2.17f,1.26f);

const real32 kShadScale = 1.12f;
const TVector3 kShadOffset(2.72f,-2.56f,1.98f);

const int32 kNoShader = -1;

const TVector2 kDefaultMinMax(0,1);

enum EPerturbationFlag
{
	ePosX = 0x00000001,
	ePosY = 0x00000002,
	ePosZ = 0x00000004,
	eScaX = 0x00000008,
	eScaY = 0x00000010,
	eScaZ = 0x00000020,
	eRotX = 0x00000040,
	eRotY = 0x00000080,
	eRotZ = 0x00000100,
	eScaU = 0x00000200,
	eShad = 0x00000400,
	ePerturbAll = ePosX+ePosY+ePosZ+eScaX+eScaY+eScaZ+eRotX+eRotY+eRotZ+eScaU+eShad,
	ePerturbXY = ePosX+ePosY,
	ePerturbAllButXY = ePosZ+eScaX+eScaY+eScaZ+eRotX+eRotY+eRotZ+eScaU+eShad,
};

struct ReplicatePMap
{
	ReplicatePMap();

	ReplicatePMap& operator=(const ReplicatePMap& pmap);

	// Replicate params

	EReplicateMode	fReplicateMode;

	// Cube
	int32		fXCount; // >=1
	real32		fXSpacing; // space between 2 bbox
	int32		fYCount; // >=1
	real32		fYSpacing; // space between 2 bbox
	int32		fZCount; // >=1
	real32		fZSpacing; // space between 2 bbox
	// Cylinder
	int32		fRCount; // >=1, elements arround
	real32		fRadius;
	int32		fCylZCount; // >=1
	real32		fCylZSpacing; // space between 2 bbox
	real32		fAngle;
	boolean		fTorque;
	// Surface
	int32		fTreePermanentID;
	real32		fSurfaceXDensity; 
	real32		fSurfaceYDensity; 
	boolean		fAlignNormal;
	boolean		fCubeMap;
	real32		fRepartitionMin; 
	real32		fRepartitionMax; 

	// Randomize params
	// Position
	real32		fPerturpPosX;
	real32		fPerturpPosY;
	real32		fPerturpPosZ;
	real32		fPerturpScaU;
	real32		fPerturpScaX;
	real32		fPerturpScaY;
	real32		fPerturpScaZ;
	real32		fPerturpRotX;
	real32		fPerturpRotY;
	real32		fPerturpRotZ;
	real32		fPerturpShading;
	TMCColorRGBA fPerturbColor;

	int32		fMaxNewShaders;

	int32		fShaderPosX;
	int32		fShaderPosY;
	int32		fShaderPosZ;
	int32		fShaderScaU;
	int32		fShaderScaX;
	int32		fShaderScaY;
	int32		fShaderScaZ;
	int32		fShaderRotX;
	int32		fShaderRotY;
	int32		fShaderRotZ;
	int32		fShaderShaM;
	int32		fShaderShading;
//	TMCCountedPtr<IShParameterComponent>	fSubShaderComponent;
};

struct CacheData
{
	CacheData();

	inline void		Invalidate(){fValid=false;}
	real32			GetValue(const TVector3& pointIn);
	const  TVector2& GetMinMax();

	TMCCountedPtr<I3DShShader>	fShader;
	int32						fShaderIndex; // not necessaty, but it's isier to compare
	ShadingIn			fShadingIn;
	real32				fCachedValue;
	boolean				fValid;

	boolean				fMinMaxValid;
	TVector2			fMinMax;

	real32				fInvMaxDist; // to rescale the UV on a 0-1 space
	TVector2			fOrigin;
};

enum EProjection
{
	eXDir,
	eMinusXDir,
	eYDir,
	eMinusYDir,
	eZDir,
	eMinusZDir,
};

class ReplicateCommand : public TBasicSceneCommand//, public TBasicWireframe
{
public :  
	ReplicateCommand();
	virtual ~ReplicateCommand();

	STANDARD_RELEASE;

	// IExDataExchanger methods :
	virtual MCCOMErr MCCOMAPI QueryInterface			(const MCIID& riid, void** ppvObj);
	virtual uint32 	 MCCOMAPI AddRef					(){ return TBasicSceneCommand::AddRef(); }
	virtual void*	 MCCOMAPI GetExtensionDataBuffer	(){ return &fPMap; }
	virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(ReplicatePMap);}
	virtual MCCOMErr MCCOMAPI ExtensionDataChanged		();
	virtual int16	 MCCOMAPI GetResID					(){ return 750; }
	
	virtual MCCOMErr MCCOMAPI HandleEvent				(MessageID message, IMFResponder* source, void* data);

	virtual void	 MCCOMAPI GetMenuCallBack			(ISelfPrepareMenuCallBack** callBack);
	virtual MCCOMErr MCCOMAPI Init						(ISceneDocument* sceneDocument);
	virtual MCCOMErr MCCOMAPI Prepare					();

	virtual boolean  MCCOMAPI CanUndo					(){return true;}
	virtual boolean  MCCOMAPI Do						();
	virtual boolean  MCCOMAPI Undo				();
	virtual boolean  MCCOMAPI Redo				();

	// TBasicWireFrame methods
/*	virtual MCCOMErr MCCOMAPI TrackWireFrame	(I3DShWireFrame* wireFrame,int16 proj,I3DShTreeElement* tree, 
												 int16 handle,const TRACKINFO& startinfo,const TRACKINFO& previnfo, 
												 const TRACKINFO& nextinfo){return MC_S_OK;}
//	virtual MCCOMErr MCCOMAPI DataToWireFrame	(I3DShWireFrame* wireFrame,int16 proj,I3DShTreeElement* tree);
//	virtual MCCOMErr MCCOMAPI WireFrameToData	(I3DShWireFrame* wireFrame,int16 proj,I3DShTreeElement* tree);
//	virtual MCCOMErr MCCOMAPI GetWireFrameBBox	(TBBox3D* outBBox, int16 proj, I3DShTreeElement* tree);

	virtual MCCOMErr MCCOMAPI Activate			(){return MC_S_OK;}
	virtual MCCOMErr MCCOMAPI Deactivate		(){return MC_S_OK;}
	virtual boolean	 MCCOMAPI HandlesTool		(int16 inTool){return false;}
*/
private :

	void Validate();

	void CubeReplicate();
	void CylinderReplicate();
	void SurfaceReplicate();
	void SurfaceReplicate(const EProjection projType, const boolean toCenter);

	bool CloneTree(I3DShTreeElement* iTree, I3DShTreeElement** newTree);

	// return a value between -1 and 1
	real32 RandomValue(){return ((real32)MCRandom()/(real32)kINT32_MAX);}

	boolean HitPos( RayHitParameters& hitParams,
			   I3DShInstance* instance, TVector3& localHitPos );

	void OffsetAnimation(I3DShTreeElement* tree, const TVector3& offset);

	ReplicatePMap	fPMap;

	boolean			fIsValid;

	boolean			fUseRepartition; // for the surface case
	EShaderOutput	fImplementedOutput;
	TMCCountedPtr<I3DShShader>		fRepartitionShader;

	int32			fPerturbationFlag;

	// Perturbation
	int32	AddShader(const int32 shaderID);

	inline void	PerturbPos(TVector3& point, const real32 f, const int32 xyz);
	inline void	PerturbScaU(real32& uniScale, const TVector3& point, const real32 f);
	inline void	PerturbScaX(TVector3& scaling, const TVector3& point, const real32 f);
	inline void	PerturbScaY(TVector3& scaling, const TVector3& point, const real32 f);
	inline void	PerturbScaZ(TVector3& scaling, const TVector3& point, const real32 f);
	inline void	PerturbRotX(real32& rotation, const TVector3& point, const real32 f);
	inline void	PerturbRotY(real32& rotation, const TVector3& point, const real32 f);
	inline void	PerturbRotZ(real32& rotation, const TVector3& point, const real32 f);
	inline real32	PertubShadingValue(const TVector3& point);
	inline const TVector2& GetMinMax();

	inline void	ApplyPosNoise(TVector3& point, const real32 f, const int32 xyz){point[xyz] += f*RandomValue();}//fNoise.GetValueLinear(kPosXOffset + kPosXScale*point);}
	inline void	ApplyScaUNoise(real32& scaling, const TVector3& point, const real32 f){scaling *= (1+f*RandomValue());}//fNoise.GetValueLinear(kScaUOffset + kScaUScale*point));}
	inline void	ApplyScaXNoise(TVector3& scaling, const TVector3& point, const real32 f){scaling.x *= (1+f*RandomValue());}//fNoise.GetValueLinear(kScaXOffset + kScaXScale*point));}
	inline void	ApplyScaYNoise(TVector3& scaling, const TVector3& point, const real32 f){scaling.y *= (1+f*RandomValue());}//fNoise.GetValueLinear(kScaYOffset + kScaYScale*point));}
	inline void	ApplyScaZNoise(TVector3& scaling, const TVector3& point, const real32 f){scaling.z *= (1+f*RandomValue());}//fNoise.GetValueLinear(kScaZOffset + kScaZScale*point));}
	inline void	ApplyRotXNoise(real32& rotation, const TVector3& point, const real32 f){rotation += (PI*f*RandomValue());}//fNoise.GetValueLinear(kRotXOffset + kRotXScale*point));}
	inline void	ApplyRotYNoise(real32& rotation, const TVector3& point, const real32 f){rotation += (PI*f*RandomValue());}//fNoise.GetValueLinear(kRotYOffset + kRotYScale*point));}
	inline void	ApplyRotZNoise(real32& rotation, const TVector3& point, const real32 f){rotation += (PI*f*RandomValue());}//fNoise.GetValueLinear(kRotZOffset + kRotZScale*point));}
	inline real32	ApplyShadingNoise(const TVector3& point){return fPMap.fPerturpShading*(.5+.5*RandomValue());}//fNoise.GetValueLinear(kShadOffset + kShadScale*point));}

	boolean ApplyTransformNoise(const TVector3& point, 
		TTreeTransform& treeTransform, 
		const real32 perturbationFactor,
		const int32 perturbationFlag=ePerturbAll);

	void NoiseLightColor(I3DShInstance* lightInstance,
					const TVector3& point,
					const real32 noiseValue, 
					const int32 colorParamID,
					TMCColorRGB& originalLightColor,
					const int32 instanceIndex);
	int32	GetColorParamID(I3DShInstance* lightInstance, TMCColorRGB& color);

	void NoiseMasterShader(I3DShInstance* instance, 
						 I3DShMasterShader* instanceMasterShader, 
						 const real32 noiseValue, 
						 int32& nameCounter);
	I3DShMasterShader* GetOtherShader();

	boolean IsInRepartitionArea(ShadingIn& shadingIn);

	void InvalidateCache();

	TMCCountedPtr<ISceneDocument>	fSceneDocument;
	TMCCountedPtr<I3DShScene>		fScene;
	TMCCountedPtr<ISceneSelection>	fSelection;
	TMCCountedPtr<IChangeChannel>	fSelectionChannel;
	TMCCountedPtr<IChangeChannel>	fTreePropertyChannel;

	// Perturbation data
	TMCArray<int32> fCacheID;	// kNoShader or an index for the fCacheArray
								// we store them like this so if the same shader
								// is used at several places (like pos X, Y and Z),
								// we will only compute it once per point.
	TMCClassArray<CacheData> fCacheArray; // this contains the shaders. Before adding a new
								// shader to the list be sure that it's not already here, 
								// we need to keep this list as short as possible

	// New master data
	TMCClassArray<ShaderFactory> fShaderFactories;

	// Progress bar
	TMCCountedPtr<IMCUnknown> fProgressKey; // Progress bar key

	// Undo/Redo data
	TMCCountedPtr<ISceneSelection>			fCloneSelection;		// undo info
	TMCCountedPtrArray<I3DShTreeElement>	fNewTrees;		// array of the new instances
	TMCCountedPtrArray<I3DShMasterShader>	fNewShaders;	// array of the new shaders
	TMCCountedPtr<I3DShTreeElement>			fNewGroup; // the new trees are placed in a group
	RelinkTreeElementInfo					fRelinkGroupInfo;
	TMCArray<RelinkTreeElementInfo>			fRelinkInfo;	// redo info
};

	
inline void	ReplicateCommand::PerturbPos(TVector3& point, const real32 f, const int32 xyz)
{
	const int32 cacheID = fCacheID[xyz];
	if(cacheID==kNoShader) ApplyPosNoise(point, f, xyz); // Apply the default noise
	else point[xyz] += f*fCacheArray[cacheID].GetValue(point);
}

inline void	ReplicateCommand::PerturbScaU(real32& scaling, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[9];
	if(cacheID==kNoShader) ApplyScaUNoise(scaling, point, f); // Apply the default noise
	else scaling *= (1+fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbScaX(TVector3& scaling, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[3];
	if(cacheID==kNoShader) ApplyScaXNoise(scaling, point, f); // Apply the default noise
	else scaling.x *= (1+fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbScaY(TVector3& scaling, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[4];
	if(cacheID==kNoShader) ApplyScaYNoise(scaling, point, f); // Apply the default noise
	else scaling.y *= (1+fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbScaZ(TVector3& scaling, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[5];
	if(cacheID==kNoShader) ApplyScaZNoise(scaling, point, f); // Apply the default noise
	else scaling.z *= (1+fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbRotX(real32& rotation, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[6];
	if(cacheID==kNoShader) ApplyRotXNoise(rotation, point, f); // Apply the default noise
	else rotation += (PI*fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbRotY(real32& rotation, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[7];
	if(cacheID==kNoShader) ApplyRotYNoise(rotation, point, f); // Apply the default noise
	else rotation += (PI*fCacheArray[cacheID].GetValue(point));
}

inline void	ReplicateCommand::PerturbRotZ(real32& rotation, const TVector3& point, const real32 f)
{
	const int32 cacheID = fCacheID[8];
	if(cacheID==kNoShader) ApplyRotZNoise(rotation, point, f); // Apply the default noise
	else rotation += (PI*fCacheArray[cacheID].GetValue(point));
}

// return a value between 0 and 1
inline real32	ReplicateCommand::PertubShadingValue(const TVector3& point)
{
	const int32 cacheID = fCacheID[10];
	if(cacheID==kNoShader) return ApplyShadingNoise(point); // Apply the default noise
	else return .5*(1 + fCacheArray[cacheID].GetValue(point));
}

inline const TVector2& ReplicateCommand::GetMinMax()
{
	const int32 cacheID = fCacheID[10];
	if(cacheID==kNoShader) return kDefaultMinMax; // Our default noise min/max
	else return fCacheArray[cacheID].GetMinMax();// evaluate the min max on this shader
}

//////////////////////////////////////////////////////////////////////////////
//
// Handle the exact same data, to be able to load and save them
//
#include "BasicDataComponent.h"

class ReplicateSaveData :  public TBasicDataComponent
{
public:
	ReplicateSaveData();
	virtual ~ReplicateSaveData();

	STANDARD_RELEASE;

	// IExDataExchanger methods :
	virtual MCCOMErr MCCOMAPI QueryInterface			(const MCIID& riid, void** ppvObj);
	virtual uint32 	 MCCOMAPI AddRef					(){ return TBasicDataComponent::AddRef(); }
	virtual void*    MCCOMAPI GetExtensionDataBuffer(){return (void*)&fPMap;}
	virtual int32	 MCCOMAPI GetParamsBufferSize() const {return sizeof(ReplicatePMap);}

	void SetFromReplicateCommand(ReplicateCommand& command);
	void SetToReplicateCommand(ReplicateCommand& command);

protected:
	ReplicatePMap	fPMap;

};

#endif

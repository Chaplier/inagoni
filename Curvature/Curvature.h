/************************************************************************************************

	Curvature.h

	Copyright (c)2005 Alan Stafford All rights reserved.

	Author: Alan

	Date: 6/10/05


************************************************************************************************/

#ifndef __Curvature__
#define __Curvature__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "copyright.h"

#include "CurvatureDef.h"

#include "BasicShader.h"
#include "I3DShShader.h"
#include "IShComponent.h"
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShUtilities.h" //C5
#include "COMSafeUtilities.h" //C5
#include "I3DShInstance.h" //C5
#endif
#include "PublicUtilities.h"
#include "COMUtilities.h"

#include "I3DShRenderable.h"
#include "BasicRenderFeature.h"
#include "BasicFinalRenderer.h"

#include "I3DShTreeElement.h"
#include "I3DShScene.h"

#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"
#endif
#include "I3dShModifier.h"
#include "BasicModifiers.h"

#include "I3dShFacetMesh.h"
#include "IMFPart.h"
#include "MCRandom.h"

#include "I3DShObject.h"


// Define the CurvatureShader CLSID ( see the CurvatureDef.h file to get R_CLSID_Curvature value )


extern const MCGUID CLSID_Curvature;

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

class CurvatureMesh :public TMCSMPCountedObject 
{
	// A representation for triangular curvature polygon data using corresponding arrays of data.  

public:
	static	void	Create(CurvatureMesh** outMesh)
	{ 
		TMCCountedCreateHelper<CurvatureMesh> result(outMesh);
		result= new CurvatureMesh;
	}
	
//	void	Clone (CurvatureMesh** clone);
protected:
	CurvatureMesh()		{}
	~CurvatureMesh()	{}

public:
	// get counts of the elements
	inline uint32 Principal1Nbr() const  
		{ return Principal1s.GetElemCount(); }
	inline uint32 Principal2Nbr() const  
		{ return Principal2s.GetElemCount(); }
	inline uint32 CornerAreaNbr() const	
		{ return CornerAreas.GetElemCount();   }
	inline uint32 PointAreaNbr() const	
		{ return PointAreas.GetElemCount();   }
	inline uint32 k1Nbr() const	
		{ return k1s.GetElemCount();   }
	inline uint32 k2Nbr() const	
		{ return k2s.GetElemCount();   }
	inline void SetVerticesCount( const uint32 numVertices ) 
		{ 	//Vertices.SetElemCount(numVertices);
			Normals.SetElemCount(numVertices);
			Principal1s.SetElemCount(numVertices); 
			Principal2s.SetElemCount(numVertices); 
			PointAreas.SetElemCount(numVertices); 
			k1s.SetElemCount(numVertices); 
			k2s.SetElemCount(numVertices); 
			k12s.SetElemCount(numVertices); 
			
		}
	inline void SetFacetsCount( const uint32 numFacets )   
		{ 	CornerAreas.SetElemCount(numFacets);
		}



	//TMCArray<TVector3>	 Vertices;     		// Vertices
	TMCArray<TVector3>	 Normals;     		// Normals
	TMCArray<TVector3>	 Principal1s;     	// k1 Principal Vector
	TMCArray<TVector3>	 Principal2s;     	// k2 Principal Vector
	TMCArray<real[3]> 	 CornerAreas; 		// Corner Areas
	TMCArray<real32>	 PointAreas; 		// Point Area
	TMCArray<real32> 	 k1s;
	TMCArray<real32>     k2s;
	TMCArray<real32>     k12s;

};


struct PointData 
{
TVector3			BaryCoord;
uint32				Facet;
uint32				P1;
TVector3			P1Vertex;
TVector3			P1Normal;
TVector3			P1k1v;
TVector3			P1k2v;
real32				P1k1;
real32				P1k2;
real32				P1K;
real32				P1H;
uint32				P2;
TVector3			P2Vertex;
TVector3			P2Normal;
TVector3			P2k1v;
TVector3			P2k2v;
real32				P2k1;
real32				P2k2;
real32				P2K;
real32				P2H;
uint32				P3;
TVector3			P3Vertex;
TVector3			P3Normal;
TVector3			P3k1v;
TVector3			P3k2v;
real32				P3k1;
real32				P3k2;
real32				P3K;
real32				P3H;
real32				K;
real32				H;
real32				k1;
real32				k2;
};


struct CurvaturePMap_declaration
{
	CurvaturePMap_declaration();

	bool		gaussian_curvature;
	bool		mean_curvature;
	bool		k1_principal_curvature;
	bool		k2_principal_curvature;
	bool		ShowPrincipal;
	bool		mixed_curvature; //x,y system with axes square of mean curvature and sign of gauss times square root of gauss.
	real32		max_pos;
	real32		max_pos_per;
	bool		PosSoftClip;
	real32		max_neg;
	real32		max_neg_per;
	bool		NegSoftClip;
	bool		Classify;
	bool		ReverseConvex;
	bool		Definition;
	bool		RedOn;
	bool		GreenOn;
	bool		BlueOn;
	bool		RedGray;
	bool		GreenGray;
	bool		BlueGray;
	bool		SqRoot;
	bool		Enable;//Fudge to avoid the initial preview problem with terrains.
	bool		Normals;
	bool		Seams;
	bool		Auto;
};


class Curvature : public TBasicShader 
{
public :
	Curvature();
	~Curvature();
	STANDARD_RELEASE;

	virtual boolean			MCCOMAPI    IsEqualTo				(I3DExShader* aShader);  
	virtual void*			MCCOMAPI    GetExtensionDataBuffer	();
	virtual MCCOMErr		MCCOMAPI	ExtensionDataChanged	();
	virtual MCCOMErr		MCCOMAPI	GetShadingFlags			(ShadingFlags& theFlags);
	virtual EShaderOutput	MCCOMAPI	GetImplementedOutput	();
	virtual int32			MCCOMAPI	GetParamsBufferSize() const {return sizeof(CurvaturePMap_declaration);}

#if (VERSIONNUMBER == 0x010000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x020000)
	virtual MCCOMErr		MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER == 0x030000)
	virtual real 			MCCOMAPI    GetColor		(TMCColorRGB& result,boolean& fullArea, ShadingIn& shadingIn);
#elif (VERSIONNUMBER >= 0x040000)
	virtual real 			MCCOMAPI    GetColor		(TMCColorRGBA& result,boolean& fullArea, ShadingIn& shadingIn);
	virtual boolean			MCCOMAPI    WantsTransform	();  
#endif

	virtual MCCOMErr		MCCOMAPI	HandleEvent(MessageID message, IMFResponder* source, void* data);

	CurvaturePMap_declaration	CurvaturePMap;

protected:


	TMCColorRGBA HLS2RGBnoA (real32 Hue , real32 Saturation , real32 Value);
	boolean HitPos( RayHitParameters& hitParams,I3DShInstance* instance, PointData& P, TVector3 Point, TVector3 Normal);
	void CreateTypedComponent(IDType familyID, IDType classID, const MCIID&riid, void** ppvObj);
	void Init();

	bool mUpdateNeeded;

	TMCCountedPtr<I3DShTreeElement> tree;
	TMCString255 TreeName;
//	TMCCountedPtr<I3DShScene> scene;
	TTreeTransform mat;
	TBBox3D MeshBox;
	TBBox3D bbox;
	RayHitParameters hitParams;
	TMCCountedPtr<FacetMesh> amesh;
	TMCCountedPtr<CurvatureMesh> cmesh;
//	TMCCriticalSection* HitPosCS;
//	TMCCriticalSection* GetColorCS;
	PointData P;
	real32 minK;
	real32 maxK;
	real32 minH;
	real32 maxH;
	real32 mink1;
	real32 maxk1;
	real32 mink2;
	real32 maxk2;
	real32 mincurve;
	real32 maxcurve;
	real32 minscale;
	real32 maxscale;
	real32 Castu;
	real32 Castv;
	real32 Castw;
	uint32 CurrentFacet;
	bool CastSucceded;
	real32 xfactor;
	real32 yfactor;
	real32 zfactor;
	real32 maxcurvature;
	real32 mincurvature;


private :

};
                           
#endif // __Curvature__

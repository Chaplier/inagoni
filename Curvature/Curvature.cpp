/************************************************************************************************

	Curvature.cpp

	Copyright (c)2005 Alan Stafford All rights reserved.

	Author: Alan Stafford

	Date: 06/12/05


************************************************************************************************/
//#include <iostream> 
//#include <ctime> 
//#include <cstdlib>

//using namespace std;

#include "Curvature.h"
 
#if (VERSIONNUMBER >= 0x050000) // From Carrara 5 build
#include "IShPartUtilities.h"
#include "IMFPart.h"
#include "IMFSliderPart.h"
#include "PMapTypes.h"
#include "MFAttributes.h"
#endif

#include "ComMessages.h"

// Serial number
#include "SerialNumber.h"

#include "CurvatureMacros.h"

boolean gSerialNumberValid = false;
SerialNumber* gSerial = NULL;

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Curvature(R_CLSID_Curvature);
#else
const MCGUID CLSID_Curvature={R_CLSID_Curvature};
#endif


void Extension3DInit(IMCUnknown* utilities)
{		

}

void Extension3DCleanup()
{
	// Perform any clean-up here
	if(gSerial)
	{
		delete gSerial;
		gSerial = NULL;
	}
}

CurvaturePMap_declaration::CurvaturePMap_declaration()
{
	gaussian_curvature=false;
	mean_curvature = false;
	k1_principal_curvature = false;
	k2_principal_curvature = false;
	mixed_curvature = false;
	max_pos =  1.0;
	max_pos_per =  1.0;
	max_neg =  1.0;
	max_neg_per =  1.0;
	Definition =  false;
	ReverseConvex =  false;
	Classify=true;
	ShowPrincipal=false;
	RedOn =  true;
	GreenOn =  true;
	BlueOn =  true;
	RedGray =  false;
	GreenGray =  false;
	BlueGray =  false;
	SqRoot =  false;
	NegSoftClip =  false;
	PosSoftClip =  false;
	Enable =  false;
	Normals =  false;
	Seams =  false;
	Auto =  true;
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId)
{														
	TBasicUnknown* res = NULL;


	if (classId == CLSID_Curvature) res = new Curvature;
	
	return res;
}


MCCOMErr Curvature::ExtensionDataChanged()
{
	mUpdateNeeded= true;
	return MC_S_OK;

}


Curvature::Curvature()		// Initialize the public data
{
	Init();
}
	
Curvature::~Curvature()
{
}

void Curvature::Init()
{
	mUpdateNeeded = true;

	maxcurvature = 0;
	mincurvature = 0;
	minK = 0;
	maxK = 0;
	minH = 0;
	maxH = 0;
	mink1 = 0;
	maxk1 = 0;
	mink2 = 0;
	maxk2 = 0;
	mincurve = 0;
	maxcurve = 0;
	minscale = 0;
	maxscale = 0;
	Castu = 0;
	Castv = 0;
	Castw = 0;
	CurrentFacet = 0;
	CastSucceded =false;
	xfactor = 1;
	yfactor = 1;
	zfactor = 1;
}

void* Curvature::GetExtensionDataBuffer()
{
	return ((void*) &(CurvaturePMap));
}

boolean Curvature::IsEqualTo(I3DExShader* aShader)		// Compare two Curvature shaders
{
	bool equality = true;	
	if 	(	CurvaturePMap.gaussian_curvature != ((Curvature*)aShader)->CurvaturePMap.gaussian_curvature ||
			CurvaturePMap.mean_curvature != ((Curvature*)aShader)->CurvaturePMap.mean_curvature ||
			CurvaturePMap.k1_principal_curvature != ((Curvature*)aShader)->CurvaturePMap.k1_principal_curvature ||
			CurvaturePMap.k2_principal_curvature != ((Curvature*)aShader)->CurvaturePMap.k2_principal_curvature ||
			CurvaturePMap.ShowPrincipal != ((Curvature*)aShader)->CurvaturePMap.ShowPrincipal ||
			CurvaturePMap.mixed_curvature!= ((Curvature*)aShader)->CurvaturePMap.mixed_curvature ||
			CurvaturePMap.max_pos!= ((Curvature*)aShader)->CurvaturePMap.max_pos ||
			CurvaturePMap.max_pos_per!= ((Curvature*)aShader)->CurvaturePMap.max_pos_per ||
			CurvaturePMap.PosSoftClip != ((Curvature*)aShader)->CurvaturePMap.PosSoftClip ||
			CurvaturePMap.max_neg != ((Curvature*)aShader)->CurvaturePMap.max_neg ||
			CurvaturePMap.max_neg_per != ((Curvature*)aShader)->CurvaturePMap.max_neg_per ||
			CurvaturePMap.NegSoftClip != ((Curvature*)aShader)->CurvaturePMap.NegSoftClip ||
			CurvaturePMap.Classify != ((Curvature*)aShader)->CurvaturePMap.Classify ||
			CurvaturePMap.ReverseConvex != ((Curvature*)aShader)->CurvaturePMap.ReverseConvex ||
			CurvaturePMap.Definition != ((Curvature*)aShader)->CurvaturePMap.Definition ||
			CurvaturePMap.RedOn != ((Curvature*)aShader)->CurvaturePMap.RedOn ||
			CurvaturePMap.GreenOn != ((Curvature*)aShader)->CurvaturePMap.GreenOn ||
			CurvaturePMap.BlueOn != ((Curvature*)aShader)->CurvaturePMap.BlueOn ||
			CurvaturePMap.RedGray != ((Curvature*)aShader)->CurvaturePMap.RedGray ||
			CurvaturePMap.GreenGray != ((Curvature*)aShader)->CurvaturePMap.GreenGray ||
			CurvaturePMap.BlueGray != ((Curvature*)aShader)->CurvaturePMap.BlueGray ||
			CurvaturePMap.SqRoot != ((Curvature*)aShader)->CurvaturePMap.SqRoot ||
			CurvaturePMap.Enable != ((Curvature*)aShader)->CurvaturePMap.Enable ||
			CurvaturePMap.Normals != ((Curvature*)aShader)->CurvaturePMap.Normals ||
			CurvaturePMap.Seams != ((Curvature*)aShader)->CurvaturePMap.Seams ||
			CurvaturePMap.Auto != ((Curvature*)aShader)->CurvaturePMap.Auto 
			)
			{
			equality = false;
			};
	return equality;
		  
}

MCCOMErr Curvature::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsPointLoc				=true;				// Needs surface point in Local Coordinates
	theFlags.fNeedsNormalLoc				=true;			// Needs surface Normal in Local Coordinates
	theFlags.fConstantChannelsMask 		= kNoChannel;		// use these enum: 	kColorChannel,kSpecularityChannel,kShininessChannel,kNormalChannel...	

	return MC_S_OK;
}

EShaderOutput Curvature::GetImplementedOutput()
{
	return EShaderOutput(kUsesGetColor);	
}

#if (VERSIONNUMBER >= 0x040000)
boolean Curvature::WantsTransform()
{
	return false;
}
#endif


#if (VERSIONNUMBER == 0x010000)
MCCOMErr Curvature::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x020000)
MCCOMErr Curvature::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER == 0x030000)
real Curvature::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shadingIn)
#elif (VERSIONNUMBER >= 0x040000)
real Curvature::GetColor(TMCColorRGBA& result,boolean& fullArea,ShadingIn& shadingIn)
#endif
{
	if( mUpdateNeeded )
	{	// Some parameters were changed, reset and recompute some data
		Init();

		mUpdateNeeded = false;
	}

ThrowIfNil(shadingIn.fInstance);
//GetColorCS= NULL; Tried this but system crashes after 4 renders.
//GetColorCS=NewCS();
//CWhileInCS cs(GetColorCS);
	CastSucceded=false;
	RayHit3D* shadingInCast; //See if we can get information about the ray hit by casting shadingIn as RayHit3D. This is really bad.
	long shadingInAddress = (long)&shadingIn; // accepted (not ISO C)
	shadingInCast=(RayHit3D*)shadingInAddress;
ThrowIfNil(shadingInCast);
	if(MCVerify(shadingInCast) )
	{
	if(	(shadingIn.fPointLoc.x != 0.0f && shadingIn.fPointLoc.y != 0.0f && shadingIn.fPointLoc.z != 0.0f ) &&
		fabs(shadingInCast->fBaryCoord[0]+shadingInCast->fBaryCoord[1]+shadingInCast->fBaryCoord[2]-1.0f)<0.000001 && 
	   	fabs(pow(shadingIn.fSurfaceNormalLoc[0],2)+pow(shadingIn.fSurfaceNormalLoc[1],2)+pow(shadingIn.fSurfaceNormalLoc[2],2)-1.0f)<0.000001) 
	{
	CurrentFacet=shadingInCast->fFacetIndex;
	CastSucceded=true;
	Castu=shadingInCast->fBaryCoord[0];
	Castv=shadingInCast->fBaryCoord[1];
	Castw=shadingInCast->fBaryCoord[2];
	}
	else
	{
	CastSucceded=false;
	Castu=shadingInCast->fBaryCoord[0];
	Castv=shadingInCast->fBaryCoord[1];
	Castw=shadingInCast->fBaryCoord[2];
	
	}
	};
bool gotHit=	false;
#define third     0.333333333333

	float SphereDiameter=0.0;
	TMCString255 PreviewName="Universe.Sphere";
	//Add a modifier to the Scene for the Object if there is none. This makes curvature work with objects that don't have calcinfo data.
	if (MCVerify(shadingIn.fInstance)&&(shadingIn.fInstance->GetInstanceKind()==I3DShInstance::kPrimitiveInstance))
	{	uint32 mod=-1;
		if (shadingIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree) == MC_S_OK)		
			{
			if (MCVerify(tree))
			{   tree->GetFullName(TreeName);
				bbox.Init();
				tree->GetBoundingBoxWithoutLightsAndCameras(bbox);
				tree->GetGlobalTreeTransform(mat);
				xfactor=(mat.GetUniformScaling()*mat.GetXYZScaling().x);
				yfactor=(mat.GetUniformScaling()*mat.GetXYZScaling().y);
				zfactor=(mat.GetUniformScaling()*mat.GetXYZScaling().z);
				mod=tree->GetModifiersCount();
				if(TreeName!=PreviewName)
				{
				if(mod==0)
					{ 	TMCCountedPtr<I3DShModifier> modifier;
						CreateTypedComponent('modi', 'idsc', IID_I3DShModifier, (void**)&modifier);//Choose a scale modifier 100% scale.
						tree->InsertModifier (modifier, mod+1, kNoAnim);
					};
				}
				else
				{
				if(mod==0 )
					{if(bbox.GetWidth()==6.0f && bbox.GetHeight()==6.0f && bbox.GetDepth()==6.0f){SphereDiameter=6.0f;};};
				};
			};
		};
	};
				
	// Prepare the Ray
	TVector3 RayOriginLocal=shadingIn.fPointLoc;
	TVector3 RayDirectionLocal=shadingIn.fSurfaceNormalLoc;RayDirectionLocal.Normalize();
	Ray3D localRay(RayOriginLocal,RayDirectionLocal);
	localRay.fIsSelectionRay = true;
	RayHit3D localHit;
#if (VERSIONNUMBER >= 0x040000 )
	TMCColorRGBA Gray(0.5,0.5,0.5,1.0);
	hitParams=RayHitParameters(&localHit, &localRay);
	result=TMCColorRGBA::kBlackFullAlpha;
	result=Gray;
	#else
	hitParams.Init(&localHit, &localRay);
	TMCColorRGB Gray(0.5,0.5,0.5);
	result=Gray;
#endif
	hitParams.tmin = -1.00;
	hitParams.tmax = +1.00;

if((TreeName==PreviewName && SphereDiameter==6.) || CurvaturePMap.Enable==false) 
	{gotHit=true;}
	else
	{gotHit=HitPos(hitParams,shadingIn.fInstance,P,shadingIn.fPointLoc,shadingIn.fSurfaceNormalLoc);};
if(gotHit)
{
	
uint32 ocount=0;
real32 gauss=0.0;
real32 Gauss=0.0;
real32 mean	=0.0;
real32 Mean	=0.0;
real32 k1	=0.0;
real32 K1	=0.0;
real32 k2	=0.0;
real32 K2	=0.0;
	
if((TreeName==PreviewName && SphereDiameter==6.) || CurvaturePMap.Enable==false) //The Shader previews are always made on Universe Sphere diameter 6.0
	{Gauss=1/9.;Mean=-1/3.;K1=-1/3.;K2=-1/3.;gotHit=true;}
	else
	{
	Gauss=P.K;Mean=P.H;K1=P.k1;K2=P.k2;
	};

real32 curvature;
if(!CurvaturePMap.Classify)
	{
	real32 curv=0.0;
	if( CurvaturePMap.gaussian_curvature		) {gauss=1.0;ocount++;}; 
	if( CurvaturePMap.mean_curvature    		) {mean =1.0;ocount++;}; 
	if( CurvaturePMap.k1_principal_curvature    ) {k1   =1.0;ocount++;}; 
	if( CurvaturePMap.k2_principal_curvature    ) {k2   =1.0;ocount++;}; 
	if( CurvaturePMap.mixed_curvature    		) {curv =1.0;ocount++;}; 

	real32 cC=sqrt(0.5*(K1*K1+K2*K2));

	curvature=(gauss*Gauss+Mean*mean+k1*K1+k2*K2+curv*cC)/ocount;
	};
	
if(CurvaturePMap.Auto)
		{
		real32 maxcount=0.0;
		real32 mincount=0.0;
		real32 maxgaussc=0.0;
		real32 maxmeanc=0.0;
		real32 maxk1c=0.0;
		real32 maxk2c=0.0;
		real32 maxcurvc=0.0;
		real32 mingaussc=0.0;
		real32 minmeanc=0.0;
		real32 mink1c=0.0;
		real32 mink2c=0.0;
		real32 mincurvc=0.0;
		if( CurvaturePMap.gaussian_curvature		) {maxgaussc  =1.0;maxcount++;mingaussc=1.0;mincount++;}; 
		if( CurvaturePMap.mean_curvature    		) {maxmeanc   =1.0;maxcount++;minmeanc =1.0;mincount++;}; 
		if( CurvaturePMap.k1_principal_curvature    ) {maxk1c     =1.0;maxcount++;mink1c   =1.0;mincount++;}; 
		if( CurvaturePMap.k2_principal_curvature    ) {maxk2c     =1.0;maxcount++;mink2c   =1.0;mincount++;}; 
		if( CurvaturePMap.mixed_curvature    		) {maxcurvc   =1.0;maxcount++;mincurvc =1.0;mincount++;}; 
		maxcurvature=(maxgaussc*maxK+maxH*maxmeanc+maxk1c*maxk1+maxk2c*maxk2+maxcurvc*maxcurve)/maxcount;
		mincurvature=(mingaussc*minK+minH*minmeanc+mink1c*mink1+mink2c*maxk2+mincurvc*mincurve)/mincount;
		if(CurvaturePMap.SqRoot)
			{
			if(maxcurvature!=0.0f)
			{
			maxcurvature=(sqrt(maxcurvature*maxcurvature)/maxcurvature)*sqrt(fabs(maxcurvature));
			}
			if(mincurvature!=0.0f)
			{
			mincurvature=(sqrt(mincurvature*mincurvature)/mincurvature)*sqrt(fabs(mincurvature));
			};
			};
		if(CurvaturePMap.Classify)
			{
			maxcurvature=sqrt(maxH*maxH+fabs(maxK));
			mincurvature=sqrt(minH*minH+fabs(minK));
			}
		if(CurvaturePMap.Auto==true )
			{
				real32 limit=max(fabs(maxcurvature),fabs(mincurvature));
				if(limit==0.0 || limit==2.24237252e-12){;}else{
					// Can't do that: the ParameterMap is modified but the application doesn't know about it.
					// A parameter modification should be done with IShMinimalParameterComponent interface.
					// But this will in return call ExtensionDataChanged() => mUpdateNeeded will be set to true.
					// So there's a risk of an infinite loop.
					CurvaturePMap.max_pos=fabs(maxcurvature);
					CurvaturePMap.max_neg=fabs(mincurvature);
				};
			}
		};

if(CurvaturePMap.SqRoot)
	{curvature=(sqrt(curvature*curvature)/curvature)*sqrt(fabs(curvature));};

	
if(!CurvaturePMap.PosSoftClip)
	{if ( curvature>= 0.0  && !CurvaturePMap.Classify) {result.R= curvature/CurvaturePMap.max_pos;result.G= 0.0;result.B=0.0;};}
	else
	{if ( curvature>= 0.0  && !CurvaturePMap.Classify) {real32 x=fabs((curvature/CurvaturePMap.max_pos));result.R= erfp(x);result.G= 0.0;result.B=0.0;};};
if(!CurvaturePMap.NegSoftClip)
	{if ( curvature<  0.0  && !CurvaturePMap.Classify) {result.B= curvature/-CurvaturePMap.max_neg;result.R= 0.0;result.G=0.0;};}
	else
	{if ( curvature<  0.0  && !CurvaturePMap.Classify) {real32 x=fabs((curvature/CurvaturePMap.max_neg));result.B= erfp(x);result.R= 0.0;result.G=0.0;};};

if(CurvaturePMap.Classify)
	{
	float signGauss=0.0;
	if(Gauss>0.0){signGauss=1.0;}
		else
		{if(Gauss<0.0){signGauss=-1.0;};};
	float ShapeAngle=RadToDeg(atan2(signGauss*sqrt(fabs(Gauss)),Mean));
	float ShapeScale;
	if(!CurvaturePMap.PosSoftClip )
		{ShapeScale=(     sqrt(Mean*Mean + (fabs(Gauss))))/CurvaturePMap.max_pos;}
	if(CurvaturePMap.PosSoftClip)
		{ShapeScale=erfp((sqrt(Mean*Mean + fabs(Gauss)))/CurvaturePMap.max_pos);};
	if(CurvaturePMap.ReverseConvex)
		{ShapeAngle=210.0+(180-ShapeAngle);
		if((ShapeAngle > 210.0) && (ShapeAngle < 300.0) ) 
			{ShapeAngle=(2/3.*(ShapeAngle-210))+210.0;}
			else
			{if((ShapeAngle < 390.0) && (ShapeAngle > 300.0) )
				{ShapeAngle=(390.0-(2/3.*(390.0-ShapeAngle)));};
			};
		}
		else
		{ShapeAngle=210.0+ShapeAngle;
		if((ShapeAngle > 210.0) && (ShapeAngle < 300.0) ) 
			{ShapeAngle=2/3.*(ShapeAngle-210)+210.0;}
			else
			{if((ShapeAngle < 390.0) && (ShapeAngle > 300.0) )
				 {ShapeAngle=390.0-2/3.*(390.0-ShapeAngle);};
			};
		};
		result=HLS2RGBnoA(ShapeAngle,1.0,ShapeScale);
		result.R=result.R;
		result.G=result.G;
		result.B=result.B;
	};

if(CurvaturePMap.ShowPrincipal ) //Just a toy will not be in released version.
	{
		real32 P1P2=(P.P1Vertex-P.P2Vertex).GetMagnitude();
		real32 P2P3=(P.P2Vertex-P.P3Vertex).GetMagnitude();
		real32 P1P3=(P.P1Vertex-P.P3Vertex).GetMagnitude();
		TVector3 UnitProjPTangentPlaneP1;
		real32 w1=(shadingIn.fPointLoc-P.P1Vertex)*P.P1Normal;
		UnitProjPTangentPlaneP1=P.P1Vertex-((shadingIn.fPointLoc-w1*P.P1Normal));
		real32 lPP1=UnitProjPTangentPlaneP1.GetMagnitude();
		UnitProjPTangentPlaneP1.Normalize();
		TVector3 UnitProjPTangentPlaneP2;
		real32 w2=(shadingIn.fPointLoc-P.P2Vertex)*P.P2Normal;
		UnitProjPTangentPlaneP2=P.P2Vertex-((shadingIn.fPointLoc-w2*P.P2Normal));
		real32 lPP2=UnitProjPTangentPlaneP2.GetMagnitude();
		UnitProjPTangentPlaneP2.Normalize();
		TVector3 UnitProjPTangentPlaneP3;
		real32 w3=(shadingIn.fPointLoc-P.P3Vertex)*P.P3Normal;
		UnitProjPTangentPlaneP3=P.P3Vertex-((shadingIn.fPointLoc-w3*P.P3Normal));
		real32 lPP3=UnitProjPTangentPlaneP3.GetMagnitude();
		UnitProjPTangentPlaneP3.Normalize();
		real32 lPP1Princip1=(UnitProjPTangentPlaneP1*P.P1k1v);
		real32 lPP2Princip1=(UnitProjPTangentPlaneP2*P.P2k1v);
		real32 lPP3Princip1=(UnitProjPTangentPlaneP3*P.P3k1v);
		real32 lPP1Princip2=(UnitProjPTangentPlaneP1*P.P1k2v);
		real32 lPP2Princip2=(UnitProjPTangentPlaneP2*P.P2k2v);
		real32 lPP3Princip2=(UnitProjPTangentPlaneP3*P.P3k2v);
		real32 ave=third*(lPP1+lPP2+lPP3);
		const real32 Parallel=1.0f;const real32 dp=4.0*0.000152f;
		if((lPP1Princip1>(Parallel-dp) && lPP1Princip1<(Parallel+dp)) && lPP1/ave<0.8f)
			{result.R=1.0f;result.G=1.0f;result.B=1.0f;};
		if((lPP2Princip1>(Parallel-dp) && lPP2Princip1<(Parallel+dp)) && lPP2/ave<0.8f)
			{result.R=1.0f;result.G=1.0f;result.B=1.0f;};
		if((lPP3Princip1>(Parallel-dp) && lPP3Princip1<(Parallel+dp)) && lPP3/ave<0.8f)
			{result.R=1.0f;result.G=1.0f;result.B=1.0f;};
		if((lPP1Princip2>(Parallel-dp) && lPP1Princip2<(Parallel+dp)) && lPP1/ave<0.8f)
			{result.R=0.25f;result.G=0.25f;result.B=0.25f;};
		if((lPP2Princip2>(Parallel-dp) && lPP2Princip2<(Parallel+dp)) && lPP2/ave<0.8f)
			{result.R=0.25f;result.G=0.25f;result.B=0.25f;};
		if((lPP3Princip2>(Parallel-dp) && lPP3Princip2<(Parallel+dp)) && lPP3/ave<0.8f)
			{result.R=0.25f;result.G=0.25f;result.B=0.25f;};
		if(lPP1/ave<0.1f)
			{result.R=0.75;result.G=0.75;result.B=0.75;}
		if(lPP2/ave<0.1f)
			{result.R=0.75;result.G=0.75;result.B=0.75;}
		if(lPP3/ave<0.1f)
			{result.R=0.75;result.G=0.75;result.B=0.75;}
	};
	
	
if(!CurvaturePMap.RedOn)	{result.R=0.0;};
if(!CurvaturePMap.GreenOn)	{result.G=0.0;};
if(!CurvaturePMap.BlueOn)	{result.B=0.0;};
TMCColorRGBA Accum;
Accum=result;
if(CurvaturePMap.RedGray)			{Accum.G+=result.R;Accum.B+=result.R;};
if(CurvaturePMap.GreenGray)			{Accum.R+=result.G;Accum.B+=result.G;};
if(CurvaturePMap.BlueGray)			{Accum.R+=result.B;Accum.G+=result.B;};
if(CurvaturePMap.RedGray||CurvaturePMap.GreenGray||CurvaturePMap.BlueGray){result=Accum;};

result.R = (result.R >= 1.0) ? 1.0 : result.R;
result.G = (result.G >= 1.0) ? 1.0 : result.G;
result.B = (result.B >= 1.0) ? 1.0 : result.B;
result.R = (result.R <= 0.0) ? 0.0 : result.R;
result.G = (result.G <= 0.0) ? 0.0 : result.G;
result.B = (result.B <= 0.0) ? 0.0 : result.B;

}
else
{
result=Gray;	
}
if (!gSerialNumberValid ) {result=Gray;};

 
#if (VERSIONNUMBER == 0x040000 )
result.A=1.0;
#endif
#if (VERSIONNUMBER == 0x010000)
return MC_S_OK;
#elif (VERSIONNUMBER == 0x020000)
return MC_S_OK;
#elif (VERSIONNUMBER >= 0x030000)
return 0.0f;
#endif
}


MCCOMErr Curvature::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;
	TMCCountedPtr<IMFPart> parent_Part;
	TMCCountedPtr<IMFPart> text_Part;
//	TMCCountedPtr<IMFPart> preset_Part;
//	TMCCountedPtr<IMFSliderPart> CMAX_Slider_Part;
//	TMCCountedPtr<IMFSliderPart> CMIN_Slider_Part;
//	TMCCountedPtr<IMFPart> CMAX_Slider_Part;
//	TMCCountedPtr<IMFPart> CMIN_Slider_Part;
//	TMCCountedPtr<TPartFindingInfo> Parts;
	
	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();
	parent_Part = sourcePart->FindParentPartByID('NTOP');
	
/*	if(sourceID=='SMAX' && message ==5)
	{	sourcePart->SetAttribute( TMFBooleanAttribute(kCustomDisplayRange_Token, true) );
		sourcePart->SetAttribute( TMFReal32Attribute (kSnpValue_Token,			 0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValue_Token,			 0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValue_Token,			  100.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValueDisplayed_Token,	   0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValueDisplayed_Token,	    100.0) );
		Curvature::ExtensionDataChanged();
	};
	
	
	if(sourceID=='SMIN' && message ==5)
	{	sourcePart->SetAttribute( TMFBooleanAttribute(kCustomDisplayRange_Token, true) );
		sourcePart->SetAttribute( TMFReal32Attribute (kSnpValue_Token,			 0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValue_Token,			 0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValue_Token,			  100.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMinValueDisplayed_Token,	   0.0) );
		sourcePart->SetAttribute( TMFReal32Attribute (kMaxValueDisplayed_Token,	    100.0) );
		Curvature::ExtensionDataChanged();
	};
*/	if (message == 5) { 
		//slider is a multiplier on CurvaturePMap.max_pos  .
		if (sourceID == 'SMAX') {
			CurvaturePMap.max_pos = CurvaturePMap.max_pos_per * CurvaturePMap.max_pos;

			text_Part = parent_Part->FindChildPartByID('CMAX');
			text_Part->SetValue(&CurvaturePMap.max_pos, kReal32ValueType, 1, 0);
			text_Part->ProcessUpdatesPart();
			Curvature::ExtensionDataChanged();
		}
		if (sourceID == 'SMIN') {
			CurvaturePMap.max_neg = CurvaturePMap.max_neg_per * CurvaturePMap.max_neg;

			text_Part = parent_Part->FindChildPartByID('CMIN');
			text_Part->SetValue(&CurvaturePMap.max_neg, kReal32ValueType, 1, 0);
			text_Part->ProcessUpdatesPart();		
			Curvature::ExtensionDataChanged();
		}
	}
		
if (message == kMsg_CUIP_ComponentAttached)
{
//Put the serial check here
	if(!gSerialNumberValid)
	{
		gSerial = new SerialNumber;
		gSerialNumberValid = gSerial->CheckSerial();
		CurvaturePMap.Enable =  false;
		return MC_S_OK;
	}

}
return MC_S_FALSE;
}

boolean Curvature::HitPos( RayHitParameters& hitParams , I3DShInstance* instance , PointData& P , TVector3 Point , TVector3 Normal )
{
//HitPosCS= NULL;
//HitPosCS=NewCS();
//CWhileInCS cs(HitPosCS);
if (MCVerify(instance))
{
int32 ObjectType=instance->GetInstanceKind();
if(ObjectType==I3DShInstance::kPrimitiveInstance)
{
bool try1=false;
bool try2=false;
bool try3=false;
TMCCountedPtr<FacetMesh> facetmesh;
#if (VERSIONNUMBER <= 0x040000) // From Carrara 5 build
instance->GetFMesh(0.0,&facetmesh);
if(MCVerify(facetmesh))
#else
if(facetmesh=instance->GetRenderingFacetMesh())
#endif
{
	
uint32 nv =facetmesh->fVertices.GetElemCount();
uint32 nf =facetmesh->fFacets.GetElemCount();
if(!MCVerify(amesh))
    {

	FacetMeshAccumulator accumulator;
    if(!CurvaturePMap.Seams){nf=1;};
    accumulator.PrepareAccumulation(nf);
    for(int i=0;i<nf;i++)
       {  
	   ThrowIfNil(facetmesh);
       Triangle  TriM;
	   TVertex3D v1;
	   TVertex3D v2;
	   TVertex3D v3;
	   if(MCVerify(facetmesh))
       		{
	   		facetmesh->fFacets.GetElem(i,TriM);
       		facetmesh->fVertices.GetElem(TriM.pt1,v1.fVertex);
       		facetmesh->fVertices.GetElem(TriM.pt2,v2.fVertex);
       		facetmesh->fVertices.GetElem(TriM.pt3,v3.fVertex);
       		facetmesh->fNormals.GetElem(TriM.pt1,v1.fNormal);
       		facetmesh->fNormals.GetElem(TriM.pt2,v2.fNormal);
       		facetmesh->fNormals.GetElem(TriM.pt3,v3.fNormal);
	    	v1.fUV=TVector2::kZero;
	    	v2.fUV=TVector2::kZero;
	    	v3.fUV=TVector2::kZero;
 	   		TFacet3D TriF;
       		TriF.fVertices[0]=v1;
       		TriF.fVertices[1]=v2;
       		TriF.fVertices[2]=v3;
	   		TriF.fUVSpace=0;       
	   		TriF.fReserved=0;       
	   		accumulator.AccumulateFacet(&TriF);
       		};
       }

	accumulator.MakeFacetMesh(&amesh);
	
	

	   ThrowIfNil(amesh);
	   ThrowIfNil(facetmesh);

	if(MCVerify(facetmesh) && !CurvaturePMap.Seams)
//		{facetmesh->Clone(&amesh);};
		{
		ThrowIfNil(facetmesh);
		if(CurvaturePMap.Normals)
		{
		facetmesh->Clone(&amesh);
		ThrowIfNoMem(amesh);
		ThrowIfNil(amesh);
		}
		else
		{amesh=facetmesh;};
		};

	ThrowIfNil(amesh);
	if(MCVerify(amesh))
		{

		ThrowIfNil(amesh);
		nv = amesh->fVertices.GetElemCount();
		nf = amesh->fFacets.GetElemCount();

		if(!MCVerify(cmesh))
			{
			CurvatureMesh::Create(&cmesh);
			};
		ThrowIfNoMem(cmesh);
		cmesh->SetVerticesCount(nv);
		ThrowIfNoMem(cmesh);
		cmesh->SetFacetsCount(nf);
		ThrowIfNoMem(cmesh);
		if(CurvaturePMap.Normals)
			{
			{COMPUTEPOINTNORMALS;};
			}
			else
			{
			{COPYNORMALS;};
			};
		{COMPUTEPOINTAREAS;};
		{COMPUTECURVATURES;};
		{COMPUTEMINMAXCURVATURES;};
		};
	};
	if(!CastSucceded)
		{
		ThrowIfNil(amesh);
		hitParams.hit->fFacetMesh=amesh;
		try1=instance->RayHit(hitParams);}
		else
		{try1=true;};
	if(!try1 ) //Getting desperate but surprising how effective randomly perturbing the direction is.
		{
		real32 randomX = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
		real32 randomY = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
		real32 randomZ = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
		hitParams.ray->fDirection.x+=randomX/100.0;
		hitParams.ray->fDirection.y+=randomY/100.0;
		hitParams.ray->fDirection.z+=randomZ/100.0;
		hitParams.ray->fDirection.Normalize();
		try2=instance->RayHit(hitParams);
		uint32 tcount=0; 
		uint32 tmax=1;
		if(!try2)
		{
			while((try3==false) && tcount < tmax) //Getting desperate but surprising how effective randomly perturbing the direction and the point is.
			{	tcount++;
				real32 randomX = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				real32 randomY = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				real32 randomZ = 2*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				real32 randomDX = 2/10000.0f*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				real32 randomDY = 2/10000.0f*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				real32 randomDZ = 2/10000.0f*(FixedToReal(MCRandom()&0xFFFF)-0.5f);
				hitParams.ray->fDirection.x+=randomX;
				hitParams.ray->fDirection.y-=randomY;
				hitParams.ray->fDirection.z+=randomZ;
				hitParams.ray->fOrigin.x+=randomDX;
				hitParams.ray->fOrigin.y+=randomDY;
				hitParams.ray->fOrigin.z+=randomDZ;
				hitParams.ray->fDirection.Normalize();
				try3=instance->RayHit(hitParams);
			};
		};
	};
	if( try1 || try2 || try3 )
	{
		if(!CastSucceded)
		{	if(hitParams.hit->fCalcInfo)
			{
			ThrowIfNil(hitParams.hit->fFacetMesh);
			hitParams.hit->fShouldSetNormalDerivative=false;
			hitParams.hit->fCalcInfo(*hitParams.hit);
			};
		}
		if(!CastSucceded)
		{
		P.BaryCoord.x=hitParams.hit->fBaryCoord[0];
		P.BaryCoord.y=hitParams.hit->fBaryCoord[1];
		P.BaryCoord.z=hitParams.hit->fBaryCoord[2];
		P.Facet=hitParams.hit->fFacetIndex;
		CurrentFacet=P.Facet;
		}
		else
		{
		P.BaryCoord.x=Castu;
		P.BaryCoord.y=Castv;
		P.BaryCoord.z=Castw;
		P.Facet=CurrentFacet;
		};
		uint32 Facets=amesh->fFacets.GetElemCount();
		uint32 index=P.Facet;
		if (index<Facets && amesh->fFacets.IsValid() && amesh->fVertices.IsValid() && amesh->fNormals.IsValid() ) 
			{
			ThrowIfNil(amesh);
			Triangle Tri;
			
			amesh->fFacets.GetElem(index,Tri);
			P.P1=Tri.pt1;
			P.P2=Tri.pt2;
			P.P3=Tri.pt3;

			};

		if(MCVerify(cmesh))
		{
		ThrowIfNil(cmesh);
		uint32 check=cmesh->k1Nbr();
		if(P.P1<check && P.P2<check && P.P3<check)
		{
		real32	u=P.BaryCoord.x;
		real32	v=P.BaryCoord.y;
		real32	w=P.BaryCoord.z;
		P.P1Vertex=amesh->fVertices[P.P1];
		P.P1Vertex.x*=xfactor;		
		P.P1Vertex.y*=yfactor;		
		P.P1Vertex.z*=zfactor;		
		P.P1Normal=cmesh->Normals[P.P1];		
		P.P1k1=cmesh->k1s[P.P1];
		P.P1k2=cmesh->k2s[P.P1];
		P.P1k1v=cmesh->Principal1s[P.P1];
		P.P1k2v=cmesh->Principal2s[P.P1];
		P.P1K=P.P1k1*P.P1k2;
		P.P1H=0.5f*(P.P1k1+P.P1k2);

		P.P2Vertex=amesh->fVertices[P.P2];
		P.P2Vertex.x*=xfactor;		
		P.P2Vertex.y*=yfactor;		
		P.P2Vertex.z*=zfactor;		
		P.P2Normal=cmesh->Normals[P.P2];		
		P.P2k1=cmesh->k1s[P.P2];
		P.P2k2=cmesh->k2s[P.P2];
		P.P2k1v=cmesh->Principal1s[P.P2];
		P.P2k2v=cmesh->Principal2s[P.P2];
		P.P2K=P.P2k1*P.P2k2;
		P.P2H=0.5f*(P.P2k1+P.P2k2);

		P.P3Vertex=amesh->fVertices[P.P3];
		P.P3Vertex.x*=xfactor;		
		P.P3Vertex.y*=yfactor;		
		P.P3Vertex.z*=zfactor;		
		P.P3Normal=cmesh->Normals[P.P3];		
		P.P3k1=cmesh->k1s[P.P3];
		P.P3k2=cmesh->k2s[P.P3];
		P.P3k1v=cmesh->Principal1s[P.P3];
		P.P3k2v=cmesh->Principal2s[P.P3];
		P.P3K=P.P3k1*P.P3k2;
		P.P3H=0.5f*(P.P3k1+P.P3k2);

		P.K=u*P.P1K+v*P.P2K+w*P.P3K;
		P.H=u*P.P1H+v*P.P2H+w*P.P3H;
		P.k1=u*P.P1k1+v*P.P2k1+w*P.P3k1;
		P.k2=u*P.P1k2+v*P.P2k2+w*P.P3k2;
		}
		else
		{
		return false;
		};
		};
	return true;
	}
}
}
}
	return false;
}



TMCColorRGBA Curvature::HLS2RGBnoA (real32 Hue , real32 Saturation , real32 Value)
{//Assumes Hue is value produced by atan2 converted to degrees;
TMCColorRGBA result=TMCColorRGBA::kBlackFullAlpha;
real32 f,p,q,t;
if ( Saturation<=0.0f )
	{result.red   = Value;result.green = Value;result.blue  = Value;
	}
	else 
	{ Hue=fmod(float(Hue),360.0f);
      Hue /= 60.0;
      int i =int(floor(Hue));
      f = Hue - i;
      p = Value * (1.0 -  Saturation     );
      q = Value * (1.0 - (Saturation * f));
      t = Value * (1.0 - (Saturation * (1.0 - f)));
	  switch (i) 
	  { case 0 :result.red   = Value;result.green = t;result.blue  = p;break;
        case 1 :result.red   = q;result.green = Value;result.blue  = p;break;
        case 2 :result.red   = p; result.green = Value;result.blue = t;break;
        case 3 :result.red   = p;result.green = q;result.blue  = Value;break;
        case 4 :result.red   = t;result.green = p;result.blue  = Value;break;
        default:result.red   = Value;result.green = p;result.blue  = q;break;
      }
     }
   return result;
};


void Curvature::CreateTypedComponent(IDType familyID, IDType classID, const MCIID&riid, void** ppvObj)
 {		TMCCountedPtr<IShComponent>	 component;
		gComponentUtilities->CreateComponent(familyID, classID, &component);
		if (component) component->QueryInterface(riid, ppvObj);
 }







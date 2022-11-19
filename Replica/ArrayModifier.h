/****************************************************************************************************

		ArrayModifier.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	12/10/2006

****************************************************************************************************/

#ifndef __ArrayModifier__
#define __ArrayModifier__

#include "BasicModifiers.h"   
#include "BasicWireFrameSet.h"   
#include "BasicRenderables.h"   
#include "I3dExModifier.h"
#include "I3dWireFrame.h"
#include "PublicUtilities.h"
#include "MCString.h"
#include "PublicUtilities.h"
#include "Id.h"
#include "IShTokenStream.h"

extern const MCGUID CLSID_ArrayModifier;


static const int32 kTargetObject = 'Targ';

struct ArrayModifierPMap
{
	ArrayModifierPMap();
	int32				fCount;
	TMCDynamicString	fTargetName;
	bool				fDisplayBBoxOnly;
	int32				fTransform; // 'Opt1': transform3D, 'Opt2': target

	real32 fGlobalScale;
	real32 fXScale;
	real32 fYScale;
	real32 fZScale;
	real32 fXOffset;
	real32 fYOffset;
	real32 fZOffset;
	real32 fXRotation;
	real32 fYRotation;
	real32 fZRotation;

	TBBox3D				fBoundingBox;  // Bounding Box

	void Get3DTransform(TTransform3D& transform);
};

class ArrayModifier : public TBasicModifier/*, public TBasicWireframeSet*/, public IExStreamIO 
{
public:
	ArrayModifier();
	virtual ~ArrayModifier();

	// IUnknown Interface :
	STANDARD_RELEASE;
	
  	// I3DExDataExchanger methods :
	virtual MCCOMErr		MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	virtual uint32 			MCCOMAPI AddRef() { return TBasicModifier::AddRef(); }
	virtual void*			MCCOMAPI GetExtensionDataBuffer(){return &fPMap;}
	virtual int32			MCCOMAPI GetParamsBufferSize() const {return sizeof(ArrayModifierPMap);}
	virtual MCCOMErr		MCCOMAPI ExtensionDataChanged();
	virtual int16			MCCOMAPI GetResID();
 
	virtual void			MCCOMAPI Clone				(IExDataExchanger**,IMCUnknown* pUnkOuter);
	virtual MCCOMErr		MCCOMAPI CopyComponentExtraData (IExDataExchanger* dest);

	virtual MCCOMErr		MCCOMAPI HandleEvent		(int32 message,
															 IMFResponder* source,
															 void* data);
 
	// I3DExDeformer methods :
	//Tree Modifiers
	virtual MCCOMErr		MCCOMAPI Apply(I3DShTreeElement* tree);
	//Deform Modifiers		
	virtual void			MCCOMAPI SetBoundingBox		(const TBBox3D& bbox);
	virtual MCCOMErr		MCCOMAPI DeformPoint		(const TVector3& point, TVector3& result);
	virtual MCCOMErr		MCCOMAPI DeformFacetMesh	(real lod,FacetMesh* in, FacetMesh** outMesh);
	virtual MCCOMErr		MCCOMAPI DeformBBox			(const TBBox3D& in, TBBox3D& out);
	virtual TModifierFlags	MCCOMAPI GetModifierFlags	()	{ return fFlags; }
	virtual void			MCCOMAPI BeginRendering();
	virtual void			MCCOMAPI EndRendering();

	// IExStreamIO methods
	virtual MCCOMErr		MCCOMAPI Read				(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData); // readUnknown should be called by extention if unknown keyword is encountered (instead of calling SkipoTokenData)
	virtual MCCOMErr		MCCOMAPI Write				(IShTokenStream* stream);
	virtual MCCOMErr		MCCOMAPI FinishRead			(IStreamContext* streamContext){return MC_S_OK;}

	//I3DExWireFrameSet methods
//	virtual MCCOMErr		MCCOMAPI DataToWireFrame	(I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree);
//	virtual void			MCCOMAPI AddRenderables		(TMCPtrArray<I3DShRenderable>& renderables);

protected:
	boolean Preprocess();
	void ReplicateFacetMesh(FacetMesh* modelIn, FacetMesh* outMesh, const int32 level);
	void TransformFacetMesh(FacetMesh* mesh);
	void DeformBBoxInMesh(const TBBox3D& in, FacetMesh* outMesh);

	void AddBoxToWireFrame(I3DShWireFrameSet* wireFrame, TBBox3D& bbox, uint32 level );
	
	boolean fIsValid;
	boolean fIsRendering;
	boolean fUseTarget;

	TTransform3D fTransform;

	ArrayModifierPMap fPMap;
	TTreeIdPath fTargetObject;

	TMCPtrArray<I3DShRenderable> fRenderable;
};

#endif
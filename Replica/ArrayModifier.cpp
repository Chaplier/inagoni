/****************************************************************************************************

		ArrayModifier.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#include "ArrayModifier.h"

#include "Copyright.h"
#include "Replica.h"
#include "InstanciatorDef.h"
#include "I3DShTreeElement.h"
#include "ISceneDocument.h"
#include "COM3DUtilities.h"
#include "I3DShUtilities.h"
#include "I3DShScene.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "MCCountedPtrHelper.h"
#include "MFPartMessages.h"
#include "MiscComUtilsImpl.h"
#include "Transforms.h"
#include "IShComponent.h"
#include "I3DShRenderFeature.h"

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_ArrayModifier(R_CLSID_ArrayModifier);
#else
const MCGUID CLSID_ArrayModifier={R_CLSID_ArrayModifier};
#endif

ArrayModifierPMap::ArrayModifierPMap()
{
	fCount = 5;
	fDisplayBBoxOnly = false;

	fTransform = 'Opt1';

	fGlobalScale = 1;
	fXScale = 1;
	fYScale = 1;
	fZScale = 1;
	fXOffset = 2;
	fYOffset = 0;
	fZOffset = 0;
	fXRotation = 0;
	fYRotation = 0;
	fZRotation = 0;
}

void ArrayModifierPMap::Get3DTransform(TTransform3D& transform)
{
	// Translation
	transform.fTranslation.x = fXOffset;
	transform.fTranslation.y = fYOffset;
	transform.fTranslation.z = fZOffset;
	// Scaling
	transform.fRotationAndScale = TMatrix33::kIdentity;
	transform.fRotationAndScale[0][0]=fXScale;
	transform.fRotationAndScale[1][1]=fYScale;
	transform.fRotationAndScale[2][2]=fZScale;
	// rotation
	real32 phyRad = DegToRad(fZRotation);
	real32 thetaRad= DegToRad(fYRotation);
	real32 psyRad =DegToRad(fXRotation);
	TMatrix33 rotation;
	real64 sinphy = RealSin(phyRad);
	real64 cosphy = RealCos(phyRad);
	real64 sintheta = RealSin(thetaRad);
	real64 costheta = RealCos(thetaRad);
	real64 sinpsy = RealSin(psyRad);
	real64 cospsy = RealCos(psyRad);
	rotation.SetColumn( 0, TVector3(	(cosphy * costheta), (sinphy * costheta), (-sintheta)));
	rotation.SetColumn( 1, TVector3(	(cosphy * sintheta * sinpsy - sinphy * cospsy), 
										(sinphy * sintheta * sinpsy + cosphy * cospsy), 
										(costheta * sinpsy)));
	rotation.SetColumn( 2, TVector3(	(cosphy * sintheta * cospsy + sinphy * sinpsy), 
										(sinphy * sintheta * cospsy - cosphy * sinpsy),
										(costheta * cospsy)));

	transform.fRotationAndScale = transform.fRotationAndScale*rotation;
	transform.fRotationAndScale = (fGlobalScale)*transform.fRotationAndScale;
}


ArrayModifier::ArrayModifier()
{
	fIsValid = false;
	fIsRendering = false;

	fTargetObject = TTreeIdPath::InvalidPath();

	fFlags.SetMasked(TModifierFlags::kDeformModifier);
	fFlags.SetMasked(TModifierFlags::kTreeModifier); // to access the tree transform
}
  
ArrayModifier::~ArrayModifier()
{
}

MCCOMErr ArrayModifier::QueryInterface(const MCIID &riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, CLSID_ArrayModifier))
	{
		TMCCountedGetHelper<ArrayModifier> result(ppvObj);
		result = this;
		return MC_S_OK;
	}
	
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = this;
		return MC_S_OK;
	}


//	if ( (TBasicWireframeSet::QueryInterface(riid, ppvObj) == MC_S_OK) && (*ppvObj) )
//	{
//		return MC_S_OK;
//	}
	return TBasicModifier::QueryInterface(riid, ppvObj);
}

int16 ArrayModifier::GetResID()
{
	return 752;		// This is the view ID in the resource file
}

MCCOMErr ArrayModifier::ExtensionDataChanged()
{
	fIsValid = false;

	return MC_S_OK;
}

MCCOMErr ArrayModifier::Apply(I3DShTreeElement* tree)
{
	if(fUseTarget)
	{
		TMCCountedPtr<I3DShScene> scene;

		// Get the scene
		tree->GetScene(&scene);
		if (!scene)
			return MC_S_OK; // abort any modifications if you can not get the scene

		if( fTargetObject == TTreeIdPath::InvalidPath() )
		{	// Check if we can't find it using the name
			if(fPMap.fTargetName.Length())
			{
				TMCCountedPtr<I3DShTreeElement> tree;
				scene->GetTreeElementByName(&tree, fPMap.fTargetName);

				if (tree)
				{
					fTargetObject = TTreeIdPath::Root() + scene->GetTreePermanentID(tree);

					// Set the name, so the user can see it
					tree->GetName(fPMap.fTargetName);
				}
			}
		}

		TMCCountedPtr<I3DShTreeElement> treeTarget;

		// Search tree element by path
		scene->GetTreeByIDPath(&treeTarget, fTargetObject);
		if (!treeTarget)
			return MC_S_OK; // abort because object 1 not found
		
		TTransform3D targetTransform;
		treeTarget->GetGlobalTransform3D(targetTransform);

		// Set the new position of tree
		TTransform3D treeTransform;
		tree->GetGlobalTransform3D(treeTransform);

		// Compute the relative transform
		TTransform3D invTargetTransform;
		treeTransform.GetInverse(invTargetTransform);


		TTransform3D newTransform = targetTransform*invTargetTransform;
		if( fTransform != newTransform )
		{
			fTransform = newTransform;
			// Invalidate the deformer

			TMCCountedPtr<IShParameterComponent> paramComp;
			this->QueryInterface(IID_IShParameterComponent,(void**)&paramComp);
			paramComp->Inval(false, false);
		}
	}
	return MC_S_OK;
}

void ArrayModifier::SetBoundingBox(const TBBox3D& bbox)
{
	fPMap.fBoundingBox = bbox;
}

MCCOMErr ArrayModifier::DeformPoint(const TVector3& point, TVector3& result)
{
	return MC_E_NOTIMPL;
}

MCCOMErr ArrayModifier::DeformBBox(const TBBox3D& in, TBBox3D& out)
{
	// Preprocess the data and check if there's a deformation
	if( !Preprocess() )
	{	// No deformation: return the same box
		out= in;
		return MC_S_OK;
	}

	TMCCountedPtr<FacetMesh> outMesh;
	FacetMesh::Create( &outMesh );

	DeformBBoxInMesh(in, outMesh);
	
	outMesh->CalcBBox(out);

	if( out.Valid() )
		return MC_S_OK;
	else
		return MC_S_FALSE;
}

void ArrayModifier::DeformBBoxInMesh(const TBBox3D& bBox, FacetMesh* outMesh)
{
	TMCCountedPtr<FacetMesh> boxMesh;
	FacetMesh::Create( &boxMesh );

	for(int16 iCorner=0 ; iCorner<8 ; iCorner++)
	{
		TVector3 cornerPoint;
		bBox.GetPoint(iCorner,cornerPoint);
		// 3 points at each corner for the 3 normals
		boxMesh->fVertices.AddElem(cornerPoint);
		boxMesh->fVertices.AddElem(cornerPoint);
		boxMesh->fVertices.AddElem(cornerPoint);

		boxMesh->fuv.AddElem(TVector2::kZero);
		boxMesh->fuv.AddElem(TVector2::kZero);
		boxMesh->fuv.AddElem(TVector2::kZero);
	}
	// Corner 0
	boxMesh->fNormals.AddElem(-TVector3::kUnitZ);
	boxMesh->fNormals.AddElem(-TVector3::kUnitX);
	boxMesh->fNormals.AddElem(-TVector3::kUnitY);
	// Corner 1
	boxMesh->fNormals.AddElem( TVector3::kUnitZ);
	boxMesh->fNormals.AddElem(-TVector3::kUnitX);
	boxMesh->fNormals.AddElem(-TVector3::kUnitY);
	// Corner 2
	boxMesh->fNormals.AddElem(-TVector3::kUnitZ);
	boxMesh->fNormals.AddElem(-TVector3::kUnitX);
	boxMesh->fNormals.AddElem( TVector3::kUnitY);
	// Corner 3
	boxMesh->fNormals.AddElem( TVector3::kUnitZ);
	boxMesh->fNormals.AddElem(-TVector3::kUnitX);
	boxMesh->fNormals.AddElem( TVector3::kUnitY);
	// Corner 4
	boxMesh->fNormals.AddElem(-TVector3::kUnitZ);
	boxMesh->fNormals.AddElem( TVector3::kUnitX);
	boxMesh->fNormals.AddElem(-TVector3::kUnitY);
	// Corner 1
	boxMesh->fNormals.AddElem( TVector3::kUnitZ);
	boxMesh->fNormals.AddElem( TVector3::kUnitX);
	boxMesh->fNormals.AddElem(-TVector3::kUnitY);
	// Corner 2
	boxMesh->fNormals.AddElem(-TVector3::kUnitZ);
	boxMesh->fNormals.AddElem( TVector3::kUnitX);
	boxMesh->fNormals.AddElem( TVector3::kUnitY);
	// Corner 3
	boxMesh->fNormals.AddElem( TVector3::kUnitZ);
	boxMesh->fNormals.AddElem( TVector3::kUnitX);
	boxMesh->fNormals.AddElem( TVector3::kUnitY);

	// Bottom triangles
	boxMesh->fFacets.AddElem(Triangle(0,12,18));
	boxMesh->fFacets.AddElem(Triangle(0,18,6));

	// Top triangles
	boxMesh->fFacets.AddElem(Triangle(3,15,21));
	boxMesh->fFacets.AddElem(Triangle(3,21,9));

	// back triangles
	boxMesh->fFacets.AddElem(Triangle(1,4,10));
	boxMesh->fFacets.AddElem(Triangle(1,10,7));
	
	// front triangles
	boxMesh->fFacets.AddElem(Triangle(13,16,22));
	boxMesh->fFacets.AddElem(Triangle(13,22,19));
	
	// left triangles
	boxMesh->fFacets.AddElem(Triangle(2,5,17));
	boxMesh->fFacets.AddElem(Triangle(2,17,14));
	
	// right triangles
	boxMesh->fFacets.AddElem(Triangle(8,11,23));
	boxMesh->fFacets.AddElem(Triangle(8,23,20));
	
	
	for(int32 iTgle=0 ; iTgle<12 ; iTgle++)
		boxMesh->fUVSpaceID.AddElem(0);

	/* Doesn't work: segment are not displayed
	for(int16 iCorner=0 ; iCorner<8 ; iCorner++)
	{
		TVector3 cornerPoint;
		bBox.GetPoint(iCorner,cornerPoint);
		boxMesh->fVertices.AddElem(cornerPoint);
		boxMesh->fNormals.AddElem(TVector3::kUnitZ);
		boxMesh->fuv.AddElem(TVector2::kZero);
	}

	TFacetEdgeFlags edgeFlag;
	edgeFlag.SetHidden(false);

	for(uint32 iIndex=0 ; iIndex<4 ; iIndex++)
	{
		uint32 nextIndex = (iIndex+1)%4;

		{	// bottom segment
			TIndex2 indexPair(iIndex,nextIndex);
			boxMesh->fEdgeList.fVertexIndices.AddElem(indexPair);
			boxMesh->fEdgeList.fEdgeFlags.AddElem(edgeFlag);
		}
		{	// top segment
			TIndex2 indexPair(iIndex+4,nextIndex+4);
			boxMesh->fEdgeList.fVertexIndices.AddElem(indexPair);
			boxMesh->fEdgeList.fEdgeFlags.AddElem(edgeFlag);
		}
		{	// between segment
			TIndex2 indexPair(iIndex,iIndex+4);
			boxMesh->fEdgeList.fVertexIndices.AddElem(indexPair);
			boxMesh->fEdgeList.fEdgeFlags.AddElem(edgeFlag);
		}
	}*/

	// replicate the box
	ReplicateFacetMesh(boxMesh, outMesh, fPMap.fCount);
}

MCCOMErr ArrayModifier::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh)
{
	// Preprocess the data and check if there's a deformation
	if( !Preprocess() )
	{	// No deformation: return the same mesh
		TMCCountedGetHelper<FacetMesh> result(outMesh);
		result= in;
		return MC_S_OK;
	}

	// Clone the orignal one
	in->Clone(outMesh);

	if(fPMap.fDisplayBBoxOnly)
	{	// we only draw the bbox
		TBBox3D bBox;
		in->CalcBBox(bBox);

		DeformBBoxInMesh(bBox, *outMesh);
	}
	else
	{
		ReplicateFacetMesh(in, *outMesh, fPMap.fCount);
	}

	return MC_S_OK;
}

boolean ArrayModifier::Preprocess()
{
	if(!fIsValid)
	{
		fUseTarget = (fPMap.fTransform=='Opt2');

		if( !fUseTarget )
		{
			fPMap.Get3DTransform(fTransform);
		}
		else
		{
		// 	fPMap.Set3DTransform(fTransform);
		}

		fIsValid = true;
	}

	if( fTransform.fRotationAndScale==TMatrix33::kIdentity && fTransform.fTranslation==TVector3::kZero )
		return false;

	return true;
}

void ArrayModifier::BeginRendering()
{
	fIsRendering = true;
}

void ArrayModifier::EndRendering()
{
	fIsRendering = false;
}

void ArrayModifier::ReplicateFacetMesh(FacetMesh* modelIn, FacetMesh* outMesh, const int32 level)
{
	if(level<=0)
		return;

	// Replicate the modelIn, using the transform, and add it to the out facet mesh

	TMCCountedPtr<FacetMesh> replicatedMesh;

	modelIn->Clone(&replicatedMesh);

	TransformFacetMesh(replicatedMesh);

	outMesh->Append(*replicatedMesh);

	int nextLevel = level-1;
	if(!IsSerialValid())
	{	// Limit the number of objects in demo mode.
		if(level>4)
		{
			nextLevel = 3;
		}
	}


	// Add the next replication
	ReplicateFacetMesh(replicatedMesh, outMesh, nextLevel);
}

void ArrayModifier::TransformFacetMesh(FacetMesh* mesh)
{
	TVector3 translate = TVector3::kUnitX;
 
	// The vertices
	const int32 vtxCount = mesh->VerticesNbr();
	for (int32 iVtx=0; iVtx<vtxCount; iVtx++)
	{
		mesh->fVertices[iVtx] = fTransform.TransformPoint(mesh->fVertices[iVtx]);
	}
	// The normals
	const int32 nmlCount = mesh->VerticesNbr();
	for (int32 iNml=0; iNml<nmlCount; iNml++)
	{
		mesh->fNormals[iNml] = fTransform.TransformVector(mesh->fNormals[iNml]);
	}
}

/* error with the bbox rotation, and maybe not the right approch because
	the bbox disappear when the object is not selected anymore

void ArrayModifier::AddBoxToWireFrame(I3DShWireFrameSet* wireFrame, TBBox3D& bbox, uint32 level )
{
//	TWFPointList& pointList = wireFrame->GetPointList(level);
	TWFConnectionList&	connectionList = wireFrame->GetConnectionList(level);
TSegmentMesh segmentMesh;
	// The points
	connectionList.AllocatePoints(8);
	for(int16 iCorner=0 ; iCorner<8 ; iCorner++)
	{
		// Need to flip the 3rd and the 4th point to get them in the right order
		uint32 bboxIndex = iCorner;
		if(iCorner==2) bboxIndex=3;
		else if(iCorner==3) bboxIndex=2;
		else if(iCorner==6) bboxIndex=7;
		else if(iCorner==7) bboxIndex=6;

		bbox.GetPoint(bboxIndex,connectionList.GetPointRef(iCorner));
segmentMesh.fVertex.AddElem(connectionList.GetPointRef(iCorner));
	}

	// The segments
	connectionList.PreAllocateConnections(12);
	for(uint32 iIndex=0 ; iIndex<4 ; iIndex++)
	{
		uint32 nextIndex = (iIndex+1)%4;
		
		{	// bottom segment
			TWFConnection connection(iIndex, nextIndex);
			connectionList.AddConnection(connection);
TIndex2 index2(iIndex, nextIndex);
segmentMesh.fSegmentIndices.AddElem(index2);
		}
		{	// top segment
			TWFConnection connection(iIndex+4, nextIndex+4);
			connectionList.AddConnection(connection);
TIndex2 index2(iIndex+4, nextIndex+4);
segmentMesh.fSegmentIndices.AddElem(index2);
		}
		{	// between segment
			TWFConnection connection(iIndex, iIndex+4);
			connectionList.AddConnection(connection);
TIndex2 index2(iIndex, iIndex+4);
segmentMesh.fSegmentIndices.AddElem(index2);
		}
	}

// TEST: list of renderable
TMCColorRGB color(.7f,.3f,.1f);
TMCCountedPtr<TRenderableHelper> newRenderable;
TRenderableFlags flags;
flags.SetMasked(TRenderableFlags::kInterfaceShapeMask);
TSegmentMeshRenderable::Create(&newRenderable, 
								color, 
								bbox,
								flags,
								segmentMesh);
	
fRenderable.AddElem(newRenderable);

	if(level>0)
	{
		TBBox3D transformedbbox(fTransform.TransformPoint( bbox.fMin ), 
								fTransform.TransformPoint( bbox.fMax ));

		AddBoxToWireFrame(wireFrame, transformedbbox, --level);
	}
	
}

MCCOMErr ArrayModifier::DataToWireFrame (I3DShWireFrameSet* wireFrame, int16 proj, I3DShTreeElement* tree)
{
	const uint32 bboxCount = fPMap.fCount + 1; // +1 for the bbox of the original mesh

	wireFrame->AllocateLists(bboxCount, bboxCount);
fRenderable.SetElemCount(0);		
	AddBoxToWireFrame(wireFrame, fPMap.fBoundingBox, fPMap.fCount);

	wireFrame->AddRenderables(fRenderable);
	return MC_S_OK;
}

void ArrayModifier::AddRenderables(TMCPtrArray<I3DShRenderable>& renderables)
{
	renderables = fRenderable;
}
*/
//------------------------------------------------------
MCCOMErr ArrayModifier::HandleEvent(int32 message, IMFResponder* source, void* data)
{
	switch (message)
	{
	case EMFPartMessage::kMsg_PartValueChanged:
		{
			if (source)
			{
				TMCCountedPtr<IMFPart> sourcePart;
				source->QueryInterface(IID_IMFPart,(void**)&sourcePart);
				ThrowIfNil(sourcePart);

				IDType partID = sourcePart->GetIMFPartID();

				switch (partID)
				{
				case kTargetObject:
					TMCDynamicString title, explanation;
					{
						CWhileInCompResFile myRes(kRID_SceneCommandFamilyID, 'RepC');

						gResourceUtilities->GetIndString(title, kStrings, 7);
						gResourceUtilities->GetIndString(explanation, kStrings, 8);
					}

					// get the active scene
					TMCPtr<ISceneDocument>	sceneDoc;
					sceneDoc = gShell3DUtilities->GetLastActiveSceneDoc();
					MCVerify(sceneDoc);
					TMCPtr<I3DShScene> scene;
					scene = sceneDoc->GetScene();
					MCVerify(scene);
					
					TMCCountedPtr<I3DShTreeElement> tree;
					scene->GetTreeByIDPath(&tree, fTargetObject);

					if (gShell3DUtilities->ChooseTreeDialog(&tree, title, explanation, nil, tree))
					{
						fTargetObject = TTreeIdPath::Root() + scene->GetTreePermanentID(tree);

						// Set the name, so the user can see it
						tree->GetName(fPMap.fTargetName);
					}

					break;
				}
			}
		}
	}
	return MC_S_OK;
}


//------------------------------------------------------
void ArrayModifier::Clone(IExDataExchanger** res,IMCUnknown* pUnkOuter)
{
	TMCCountedCreateHelper<IExDataExchanger> result(res);
	ArrayModifier* clone = new ArrayModifier();
	result = (IExDataExchanger*)clone;

	if (clone)
	{
	    clone->fPMap=fPMap; // copy the ArrayModifierData
		clone->fTargetObject = fTargetObject;
	}
	clone->SetControllingUnknown(pUnkOuter);
} 

//------------------------------------------------------
MCCOMErr ArrayModifier::CopyComponentExtraData (IExDataExchanger* dest)
{
	TMCCountedPtr<ArrayModifier> destArrayModifier;
	dest->QueryInterface(CLSID_ArrayModifier, (void**)&destArrayModifier);
	if (destArrayModifier)
	{
		destArrayModifier->fTargetObject = fTargetObject;
	}
	return MC_S_OK;
}

// IExStreamIO methods
//ERROR: this is never called
MCCOMErr ArrayModifier::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	int8 token[256];
	MCCOMErr result = MC_S_OK;

	result = stream->GetBegin();
	if (result) return result;

	result=stream->GetNextToken(token);
	if (result) return result;

	TMCiostream& ioStream = stream->GetStream();

	fTargetObject = TTreeIdPath::InvalidPath();

	while (!stream->IsEndToken(token))
	{
		int32 keyword;
		stream->CompactAttribute(token,&keyword);

		switch (keyword)
		{
		case kTargetObject:
			fTargetObject = TTreeIdPath::Read(ioStream);
			break;
		default:
			readUnknown(keyword, stream, privData);
			break;
		};
		result=stream->GetNextToken(token);
		if (result)
		{
			return result;
		}
	}

	return MC_S_OK;	
}

MCCOMErr ArrayModifier::Write (IShTokenStream* stream)
{
	TMCiostream& ioStream = stream->GetStream();

	if (fTargetObject.isValid())
		fTargetObject.Write(ioStream, kTargetObject);

	return MC_S_OK;
}


/****************************************************************************************************

		MBuildingAction.cpp
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	3/24/2004

****************************************************************************************************/
#include "BuildingDef.h"
#if !NETWORK_RENDERING_VERSION

#include "MBuildingAction.h"
#include "COMSafeUtilities.h"

ModelerAction::ModelerAction(BuildingModeler *modeler) 
{
	fBuildingModeler = modeler; 
	modeler->GetBuildingPrimitive(&fBuildingPrimitive);
	fInvalidateStatus = true;
	fRefreshGeometry = false;
}

ModelerAction::~ModelerAction() 
{
}
	
MCCOMErr ModelerAction::Do()
{
	try
	{
		if (CanUndo()) 
		{
			// Selection has changed?
			fBuildingModeler->InvalidateMeshesAttributes(true);

			if(fInvalidateStatus)
				fBuildingPrimitive->InvalidateStatus();

			// Primitive changed?
			if(fRefreshGeometry)
				fBuildingModeler->InvalidatePrimitive();

			if(!fBuildingModeler->IsFaceless())
			{
				// Tell the modeler?
				fBuildingModeler->ImmediateUpdate(fRefreshGeometry,true);
			}
		}
	}
	catch(TMCException& exception)
	{
		HandleException(&exception, TMCDynamicString("ModelerAction::Do"));
	}
	catch(...)
	{
		HandleException(NULL, TMCDynamicString("ModelerAction::Do"));
	}

	return MC_S_OK;
}

MCCOMErr ModelerAction::Undo()
{
	if (CanUndo()) 
	{
		// Similar updates as in Do()
		ModelerAction::Do();
		return MC_S_OK;
	}
	return MC_E_NOTIMPL;
}

MCCOMErr ModelerAction::Redo()
{
	if (CanUndo()) 
	{
		// Similar updates as in Do()
		ModelerAction::Do();
		return MC_S_OK;
	}
	return MC_E_NOTIMPL;
}
/*
MCCOMErr ModelerAction::GetName(TMCString& name)
{
	name= TMCString31("No Name");
	return MC_S_OK;
}
*/

//////////////////////////////////////////////////////////////////////
//
ModelerMouseAction::ModelerMouseAction(BuildingModeler *modeler, BuildingPanePart* pane) 
{
	fBuildingModeler = modeler; 
	modeler->GetBuildingPrimitive(&fBuildingPrimitive);
	fPanePart = pane;
	fRefreshGeometry = false;
	fInvalidateStatus = true;
}

ModelerMouseAction::~ModelerMouseAction() 
{
}
	
MCCOMErr ModelerMouseAction::Do()
{
	if (CanUndo()) 
	{
		// Selection has changed?
		fBuildingModeler->InvalidateMeshesAttributes(true);

		if(fInvalidateStatus)
			fBuildingPrimitive->InvalidateStatus();

		// Primitive changed?
		if(fRefreshGeometry)
			fBuildingModeler->InvalidatePrimitive();

		boolean refresh = true;
		if(refresh)
		{
			// Tell the modeler?
			fBuildingModeler->ImmediateUpdate(fRefreshGeometry,refresh);
		}
	}

	return MC_S_OK;
}

MCCOMErr ModelerMouseAction::Undo()
{
	if (CanUndo()) 
	{
		// Similar updates as in Do()
		ModelerMouseAction::Do();
		return MC_S_OK;
	}
	return MC_E_NOTIMPL;
}

MCCOMErr ModelerMouseAction::Redo()
{
	if (CanUndo()) 
	{
		// Similar updates as in Do()
		ModelerMouseAction::Do();
		return MC_S_OK;
	}
	return MC_E_NOTIMPL;
}

MCCOMErr ModelerMouseAction::GetName(TMCString& name)
{
	name= TMCString31("No Name");
	return MC_S_OK;
}


//////////////////////////////////////////////////////////////////////
//
BuildingRecorder::BuildingRecorder()
{
}

void BuildingRecorder::SaveBuilding( BuildingPrim* building )
{
	WriteInBuffer( building, &mSavedBuilding );
}

void BuildingRecorder::WriteInBuffer(BuildingPrim* building, IShTokenStream** buffer )
{
	TMCstrstream* stream = new TMCstrstream(kPlatformDependent);// memory buffer stream
	gFileUtilities->CreateTokenStream(buffer,stream,true); // true : the stream will be deleted by mSavedBuilding
	(*buffer)->InitTokenMgr();
	building->WriteContent( *buffer );
}

void BuildingRecorder::SwapBuilding( BuildingPrim* building )
{
	TMCCountedPtr<IShTokenStream> tmpBuildingPtr;
	WriteInBuffer( building, &tmpBuildingPtr );

	mSavedBuilding->InitTokenMgr();
	TMCiostream& stream = mSavedBuilding->GetStream() ;
	stream.seek(0);

	building->ReadContent(mSavedBuilding);
	building->FinishRead(mSavedBuilding->GetStreamContext());

	mSavedBuilding = tmpBuildingPtr;
}

#endif // !NETWORK_RENDERING_VERSION

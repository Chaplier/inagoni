/****************************************************************************************************

		TypePart.h
		Copyright: (c) 2004 My Sister Is Not A Boy. All rights reserved.

		Author:	Julien
		Date:	10/14/2004

****************************************************************************************************/

#ifndef __TypePart__
#define __TypePart__

#if CP_PRAGMA_ONCE
#pragma once
#endif


#include "BasicMCFCOMImplementations.h"
#include "InstanciatorEnum.h"
#include "MCClassArray.h"

extern const MCGUID CLSID_TypePart;

class TypePart : public TBasicPart
{
public:
	TypePart();
	STANDARD_RELEASE;

	virtual void	MCCOMAPI SelfPrepareToDestroy();
	virtual MCErr	MCCOMAPI QueryInterface	(const MCIID& riid, void** ppvObj);
	virtual void	MCCOMAPI FinishCreateFromResource();

	virtual void	MCCOMAPI SelfDraw(IMCGraphicContext* graphicContext, const TMCRect& inZone);
	virtual	boolean	MCCOMAPI Receive (int32 message, IMFResponder* source, void* data);

protected:

	void	BuildTreePopupMenu();
	void	DisplayOption( const int32 option );
	void	SetPreset(EReplicateMode mode, const int32 option);
	void	SetTreeItem(const int32 item);

	TMCArray<int32> fTreePermanentID;
	boolean			fIsInit;
	int32			fEraseRectCount;

	TMCClassArray<TMCRect> fModeRects;
};









#endif

/*! \file kbool/src/instonly.cpp
    \author Probably Klaas Holwerda
 
    Copyright: 2001-2004 (C) Probably Klaas Holwerda
 
    Licence: see kboollicense.txt 
 
    RCS-ID: $Id: instonly.cpp,v 1.2 2006/11/05 14:59:31 titato Exp $
*/

#ifdef __GNUG__
#pragma option -Jgd

#include "_dl_itr.h"
#include "node.h"
#include "record.h"
#include "link.h"
#include "_lnk_itr.h"
#include "scanbeam.h"
#include "graph.h"
#include "graphlst.h"
//#include "kbool/misc.h"

template class DL_Node<void *>;
template class DL_Iter<void *>;
template class DL_List<void *>;

template class DL_Node<int>;
template class DL_Iter<int>;
template class DL_List<int>;

template class TDLI<Node>;
template class TDLI<LPoint>;
template class TDLI<Record>;
template class TDLI<KBoolLink>;
template class TDLI<Graph>;

#endif

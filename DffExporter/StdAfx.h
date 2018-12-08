// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//


#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <iterator>
#include <bitset>
#include <regex>
#include <memory>
//#include <maxscript.h>
#include <impexp.h>
#include <iparamb2.h>
#include <point3.h>
#include <matrix3.h>
#include <triobj.h>
#include <dummy.h>
#include <MeshNormalSpec.h>
#include <stdmat.h>
#include <simpobj.h>
#include <control.h>
#include <units.h>
#include <impapi.h>
#include <meshadj.h>
#include <meshdlib.h>
#include <bitmap.h>
#include <IPathConfigMgr.h>
#include <modstack.h>
#include <iskin.h>
#include <ISceneEventManager.h>
#include <maxapi.h>
//#include <polyobj.h>
//#include <mnmesh.h>
//#include <iepoly.h>
//Tristrip library MACROS and includes
//#define RELEASE(x)		{ if (x != NULL) delete x;		x = NULL; }
//#define RELEASEARRAY(x)	{ if (x != NULL) delete []x;	x = NULL; }

#include "NvTriStrip.h"
#include "NvTriStripObjects.h"
#include "VertexCache.h"
#include "ZappyCommon.h"
//#include "CustomArray.h"
//#include "Adjacency.h"
//#include "RevisitedRadix.h"
//#include "Striper.h"
//end Tristrip library macros and includes
#define GTA_MultiMat Class_ID(0x744721ba, 0x31ec52e0)
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//#define LOGFILE
// !defined(AFX_STDAFX_H__AFD56823_0B43_11D4_8B0F_0050BAC83302__INCLUDED_)
namespace std {
#ifdef _UNICODE
typedef wstring tstring;
typedef wstringstream tstringstream;
#define Tstrlen wcslen
#else
typedef string tstring;
typedef stringstream tstringstream;
#define Tstrlen strlen
#endif
}
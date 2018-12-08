#include "StdAfx.h"
#include "DffExporter.h"
#include "DffExpClassDesc.h"

//Dll handle
HINSTANCE hDllInstance = 0;

//==========================================================================
// Exported DLL functions

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) { 
    // NOTE: Do not call managed code from here.
    // You should do any initialization in LibInitialize
	hDllInstance = hinstDLL; 
	switch(fdwReason) { 
		case DLL_PROCESS_ATTACH: break; 
		case DLL_THREAD_ATTACH: break; 
		case DLL_THREAD_DETACH: break; 
		case DLL_PROCESS_DETACH: break; 
	} 
	return(TRUE); 
} 
__declspec( dllexport ) const TCHAR * LibDescription() 
{ 
	return TEXT("DFF Exporter \xA9 2011 by Seggaeman"); 
} 
__declspec( dllexport ) int LibNumberClasses() 
{ 
	return 1;  
} 
__declspec( dllexport ) ClassDesc2* LibClassDesc(int i) 
{ 
	return ::DffExporterClassDesc::GetClassDescInstance();
} 
__declspec( dllexport ) ULONG LibVersion() 
{ 
	return PLUGIN_VERSION; 
} 
__declspec( dllexport ) ULONG CanAutoDefer() 
{ 
	return 1; 
} 
__declspec( dllexport ) ULONG LibInitialize() 
{ 
    // Note: called after the DLL is loaded (i.e. DllMain is called with DLL_PROCESS_ATTACH)
    //TODO("Add any library initialization here");
    return 1;
}
__declspec( dllexport ) ULONG LibShutdown() 
{ 
    // Note: called before the DLL is unloaded (i.e. DllMain is called with DLL_PROCESS_DETACH)
    //TODO("Add any library finalization here");
    return 1;
}

//==========================================
//Class descriptor

int DffExporterClassDesc::IsPublic() 
{ 
	return 1; 
}

void* DffExporterClassDesc::Create(BOOL loading) 
{ 
	return new DffExporter(); 
}

const MCHAR* DffExporterClassDesc::ClassName()
{
	return _M("DFF_Export");
}

::SClass_ID DffExporterClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}

::Class_ID DffExporterClassDesc::ClassID()
{
	return Class_ID(0x1B924159, 0x63662D11);
}

const MCHAR* DffExporterClassDesc::Category()
{
	return _M("Game file exporter");
}

const MCHAR* DffExporterClassDesc::InternalName()
{
	return _M("DffExporter");
}

HINSTANCE DffExporterClassDesc::HInstance()
{
	return hDllInstance;
}

ClassDesc2* DffExporterClassDesc::GetClassDescInstance()
{
	static DffExporterClassDesc desc;
	return &desc;
}
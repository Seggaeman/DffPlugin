#include "StdAfx.h"
#include "DffExpClassDesc.h"
#include "DffExporter.h"

//=========================================================
//DffExporter constructor, destructor and compulsory functions (excluding DoExport)
//TODO("Define a parameter block descriptor if you want");

//=========================================================================
// UtilityPlugin overrides

ClassDesc2* DffExporter::GetClassDesc()
{
    return DffExporterClassDesc::GetClassDescInstance();
}
//============================================================
// Constructor/destructor

DffExporter::DffExporter()
    : SceneExport(), pedNode(nullptr)
{         
    //TODO("Initialize any member fields");
	this->expData[(UINT)enms::expDatIDs::morphPlg] = false;
	this->expData[(UINT)enms::expDatIDs::meshExt]= true;
	this->expData[(UINT)enms::expDatIDs::skyMipMap]= false;
}

DffExporter::~DffExporter()
{ 
    //TODO("Free any memory allocated here");
}

//=========================================================================
// UtilityPlugin<SceneExport> overrides 

unsigned int DffExporter::Version ()
{
    //TODO("Return the version number, where 3.01 is counted as 301");
    return 100;
}

int DffExporter::ExtCount()
{
    //TODO("return the number of extensions");
    return 1; 
}

const MCHAR* DffExporter::Ext(int n)
{
    //TODO("return the nth file extensionm, without leading dot");
    return _M("dff");
}

const MCHAR* DffExporter::LongDesc()
{
    //TODO("Return a long description of the file type");
    return TEXT("Grand Theft Auto DFF");
}

const MCHAR* DffExporter::ShortDesc()
{
    //TODO("Return a short description of the file type");
    return TEXT("Grand Theft Auto DFF");
}

const MCHAR* DffExporter::AuthorName()
{
    //TODO("Return the author name");
    return TEXT("Seggaeman");
}

const MCHAR* DffExporter::CopyrightMessage()
{
    //TODO("Return the copyright message");
    return TEXT("(c) 2011 by Seggaeman");
}

void DffExporter::ShowAbout(HWND hWnd)
{
    //TODO("Display an about box");
}

const MCHAR* DffExporter::OtherMessage1()
{
    //TODO("Return additional string to be displayed in the UI");
    return TEXT("");
}

const MCHAR* DffExporter::OtherMessage2()
{
    //TODO("Return additional string to be displayed in the UI");
    return TEXT("");
}

BOOL DffExporter::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}






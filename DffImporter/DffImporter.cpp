/**********************************************************************
 *<
	FILE: DffImporter.cpp

	DESCRIPTION:  GTA III/Vice City/San Andreas .dff importer

	AUTHOR: seggaeman

	CONTRIBUTORS: DexX (dff format information), The Hero (dff format information), REspawn (dff format information), Kam (GTA_Material.ms), gtamodding.com

 *>	Copyright (c) 2011, All Rights Reserved.
 **********************************************************************/

#include "DffImporter.h"
using namespace extensions;

HINSTANCE hDllInstance= 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) { 
    // NOTE: Do not call managed code from here.
    // You should do any initialization in LibInitialize
	switch(fdwReason) { 
		case DLL_PROCESS_ATTACH:
		{
			hDllInstance= hinstDLL;
			DisableThreadLibraryCalls(hDllInstance);
			break;
		}
		case DLL_THREAD_ATTACH: break; 
		case DLL_THREAD_DETACH: break; 
		case DLL_PROCESS_DETACH: break; 
	} 
	return(TRUE); 
}

//==================================================//
//Descriptor
//=================================================//

class DffImporterClassDesc
    : public ClassDesc2 
{
public:
    //---------------------------------------
    // ClassDesc2 overrides 

	virtual int IsPublic() {return 1;}
	virtual void* Create(BOOL loading = FALSE) {return new DffImporter;}
	virtual const TCHAR* ClassName()	{return TEXT("DFF_Import");}
	virtual SClass_ID SuperClassID()	{return SCENE_IMPORT_CLASS_ID;}
	virtual Class_ID ClassID()			{return Class_ID(0x4F52272A, 0x2CDF01E5);}
	virtual const TCHAR* Category()	{return	TEXT("Game model file importer");}
	virtual const TCHAR* GetInternalName()	{return TEXT("DFF Importer plugin");}
	virtual HINSTANCE HInstance()	{return hDllInstance; }
	static ClassDesc2* GetClassDescInstance()
	{
		static DffImporterClassDesc desc;
		return &desc;
	}
};

//==========================================================================
// Exported DLL functions

__declspec( dllexport ) const TCHAR* LibDescription() 
{ 
	return _T("DFF importer plugin (c) 2011 by seggaeman");
} 
__declspec( dllexport ) int LibNumberClasses() 
{ 
	return 1;
} 
__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{ 
    static DffImporterClassDesc classdesc; 
    return &classdesc;
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

//TODO("Define a parameter block descriptor if you want");


 //============================================================
 // Constructor/destructor

DffImporter::DffImporter() : SceneImport()
{         
	//TODO("Initialize any member fields");
}

DffImporter::~DffImporter()
{ 
	//TODO("Free any memory allocated here");
}

//=========================================================================
// UtilityPlugin<SceneImport> overrides 

int DffImporter::ExtCount()
{
    //TODO("return the number of extensions");
    return 1; 
}

const TCHAR* DffImporter::Ext(int n)
{
    //TODO("return the nth file extensionm, without leading dot");
    return _T("dff");
}
    
const TCHAR* DffImporter::LongDesc()
{
    //TODO("Return a long description of the file type");
	return _T("GTA 3 series DFF files");
}
    
const TCHAR* DffImporter::ShortDesc()
{
    //TODO("Return a short description of the file type");
    return _T("Grand Theft Auto DFF");
}

const TCHAR* DffImporter::AuthorName()
{
    //TODO("Return the author name");
    return _T("seggaeman");
}
    
const TCHAR* DffImporter::CopyrightMessage()
{
    //TODO("Return the copyright message");
    return _T("(c) 2011 by seggaeman");
}
    
const TCHAR* DffImporter::OtherMessage1()
{
    //TODO("Return additional string to be displayed in the UI");
    return _T("");
}
    
const TCHAR* DffImporter::OtherMessage2()
{
    //TODO("Return additional string to be displayed in the UI");
    return _T("");
}

unsigned int DffImporter::Version ()
{
	//TODO("Return the version number, where 3.01 is counted as 301")
    return 100;
}

void DffImporter::ShowAbout(HWND hWnd)
{
    //TODO("Display an about box");
}

int DffImporter::ZoomExtents()
{
	return ZOOMEXT_YES;
}
//================

ClassDesc2* DffImporter::GetClassDesc()
{
	return DffImporterClassDesc::GetClassDescInstance();
}

/*
static BOOL CALLBACK ToolDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffImporter* leImporteur= (DffImporter*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
switch(Message)
{
	case WM_INITDIALOG:

		leImporteur = (DffImporter*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)leImporteur);
		SetFocus(hwnd);
	break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
				//MessageBox(NULL, leImporteur->GetClassDesc()->ClassName(), "Caption", MB_OK);
			break;


			case IDCANCEL:
				DestroyWindow(hwnd);
				//leImporteur->leWnd= 0;
			break;
		}
	break;

	default:
		return FALSE;
	break;
	}
	return TRUE;
}
*/

static DWORD WINAPI fn(LPVOID arg)
{
	DffImporter* local= (DffImporter*)arg;

	int temp= 100.0f*(float)local->bufIndex/(float)local->fileSize;
	local->dffInterface->ProgressUpdate(temp, TRUE);
	return 0;
}
/*
static INT_PTR CALLBACK laProcedure(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffImporter* leImporteur= (DffImporter*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	switch(Message)
	{
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)lParam);
		break;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
					MessageBox(leImporteur->dffInterface->GetMAXHWnd(), "You clicked ok", leImporteur->AuthorName(), MB_OK);
					EndDialog(hWnd, IDOK);
				break;
				
				case IDCANCEL:
					MessageBox(leImporteur->dffInterface->GetMAXHWnd(), "You cancelled", leImporteur->CopyrightMessage(), MB_OK);
					EndDialog(hWnd, IDCANCEL);
				default:
					break;
			}
		}
		break;

		default:
			return FALSE;		
	}
	return TRUE;
}
*/
//imports all clumps; dialog too buggy and wasn't implemented.
int DffImporter::DoImport(const TCHAR* name, ImpInterface *iImpInterface, Interface *iInterface, BOOL suppressPrompts)
{

	//this->logfile.open(TEXT("C:\\users\\packard-bell\\desktop\\ImportLog.txt"), std::ios::beg| std::ios::out | std::ios::trunc);
	//Check if scripts are present.
	std::tstring startupScriptsDir = IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_STARTUPSCRIPTS_DIR), userStartupScriptsDir = IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_USER_STARTUPSCRIPTS_DIR);
	//MessageBox(iInterface->GetMAXHWnd(), userStartupScriptsDir.c_str(), startupScriptsDir.c_str(), MB_OK);
	WIN32_FIND_DATA fData;
	HANDLE hScriptHandles[] = { FindFirstFile((startupScriptsDir + TEXT("\\GTA_Material.ms")).c_str(), &fData), FindFirstFile((userStartupScriptsDir + TEXT("\\GTA_Material.ms")).c_str(), &fData),
						FindFirstFile((startupScriptsDir + TEXT("\\GTAMultiMaterial.ms")).c_str(), &fData), FindFirstFile((userStartupScriptsDir + TEXT("\\GTAMultiMaterial.ms")).c_str(), &fData) };
	
	if ((hScriptHandles[0] == INVALID_HANDLE_VALUE && hScriptHandles[1] == INVALID_HANDLE_VALUE) || (hScriptHandles[2] == INVALID_HANDLE_VALUE && hScriptHandles[3] == INVALID_HANDLE_VALUE))
	{
		MessageBox(iInterface->GetMAXHWnd(), TEXT("GTA_Material.ms and/or GTAMultiMaterial.ms startup scripts not found"), TEXT("Import cancelled"), MB_OK);
		return IMPEXP_CANCEL;
	}

	this->buffer= 0; this->bufIndex = 0; this->fileSize= 0;	this->IsCharDff= false; this->OkToContinue= true; this->noIssues= true;
	this->dffImpInterface= iImpInterface;
	this->dffInterface= iInterface;
	std::fstream dffReader(name, std::ios::in | std::ios::binary | std::ios::ate);

	this->fileSize= dffReader.tellg();

	this->buffer = new char[this->fileSize];
	dffReader.seekg(0, std::ios::beg);
	dffReader.read(buffer, this->fileSize);
	dffReader.close();

	dffInterface->ProgressStart(TEXT("Importing DFF"), TRUE, fn, (LPVOID)this);
	while(this->OkToContinue)
	{
		this->atomList =0; this->frameList= 0; this->frameNames= 0; this->eMeshList= 0; this->leClump= 0; this->maxObjectList= 0;
		this->frameInfo =0;
		extensions::GTAHeader* theHeader= parseFile();
		if (this->noIssues && (*theHeader == secIDs::CLUMP_ID || *theHeader == secIDs::UVANIM_DIC))
			if(this->IsCharDff)
				this->buildSceneChar();
			else
				this->buildScene();
		else
			break;
	}
	delete[] buffer;
	dffInterface->ProgressEnd();

	//this->logfile.close();
	
	return IMPEXP_SUCCESS;
	
	/*
	::PolyObject* lePolyObject= (PolyObject*)iInterface->CreateInstance(SClass_ID(GEOMOBJECT_CLASS_ID), EPOLYOBJ_CLASS_ID);
	lePolyObject->mm.NewVert(Point3(0,0,0));
	lePolyObject->mm.NewVert(Point3(5.5f,0.0f,0.0f));
	lePolyObject->mm.NewVert(Point3(0.0f,6.49f,0.0f));
	lePolyObject->mm.NewVert(Point3(5.5f,6.49f,0.0f));
	lePolyObject->mm.NewQuad(0,1,2,3,7,2);
	INode* lePolyNode= iInterface->CreateObjectNode(lePolyObject);
	lePolyNode->SetName("lePolyNode");
	lePolyNode->SetUserPropString("Prop", "data");
	::MSTR theData;
	lePolyNode->GetUserPropString("Prop", theData);
	MessageBox(NULL, theData, "Prop", MB_OK);
	lePolyNode->SetUserPropBuffer(MSTR((char*)NULL));
	return IMPEXP_SUCCESS;
	*/
}

 //=========================================================================
 // UtilityPlugin overrides


inline extensions::GTAHeader* DffImporter::readHeader()
{
	return this->readBuffer<extensions::GTAHeader*>(sizeof(GTAHeader));
}

extensions::GTAHeader* DffImporter::parseFile()
{
	extensions::GTAHeader* leHeader= readHeader();

	switch(leHeader->getIdentifier())
	{
		case secIDs::UVANIM_DIC:
			//skip UV anim dictionary for now
			this->readUVAnims();
			parseFile();
		break;

		case secIDs::CLUMP_ID:
		{
			UINT temp= bufIndex;
			//MessageBox(NULL, "You are in clump", NULL, MB_OK);
			//this->logfile << "CLUMP at " << bufIndex-12 << std::endl;
			this->leClump= readBuffer<extensions::ClumpStruct*>(sizeof(extensions::ClumpStruct));
			//version specific: GTAIII does not use lights and cameras
			if (leHeader->fileVersion == GTA_IIIA || leHeader->fileVersion == GTA_IIIB || leHeader->fileVersion == GTA_IIIC)
			{
				//this->logfile << "GTA 3 .dff, version= " << leHeader->fileVersion;
				bufIndex -= 8;
			}
			//end version specific
			//this->logfile << "Clump struct read, current position= " << bufIndex << std::endl;
			readFrameList();
			readGeometryList();
			if (leClump->structHeader.fileVersion != GTA_IIIA && leClump->structHeader.fileVersion != GTA_IIIB && leClump->structHeader.fileVersion != GTA_IIIC)
				for (int idx =0; idx < leClump->lightCount; ++idx)
					readLights();

			if (leClump->structHeader.fileVersion != GTA_IIIA && leClump->structHeader.fileVersion != GTA_IIIB && leClump->structHeader.fileVersion != GTA_IIIC)
				for (int idx =0; idx < leClump->cameraCount; ++idx)
					readCameras();

			bufIndex= temp + leHeader->size;
			if (bufIndex >= (this->fileSize-12))		//some extra protection because a header is 12 bytes
				this->OkToContinue= false;

		}
		break;

		default:
			//this->logfile << "Unknown section ID= " << leHeader->identifier << ", found at " << bufIndex-12 << std::endl;
			bufIndex -= 12;
		break;
	}
	return leHeader;
}

void DffImporter::readFrameList()
{
	if (!(*readHeader() == secIDs::FRAME_LIST))
	{	
		//this->logfile << "Frame list not found at " << bufIndex-12 << std::endl;
		this->noIssues= false;
		return;
	}

	//this->logfile << "FRAME_LIST at " << bufIndex-12 << std::endl;
	this->frameInfo= readBuffer<extensions::FrameStructStart*>(sizeof(extensions::FrameStructStart));
	//this->logfile << "Frame count = " << this->frameInfo->frameCount << std::endl;
	this->frameList= readBuffer<extensions::GTAFrame*>(sizeof(extensions::GTAFrame)*(this->frameInfo->frameCount));
	this->maxObjectList = new extensions::MaxObject[this->frameInfo->frameCount];
	this->frameNames= new char*[this->frameInfo->frameCount];

	//this->logfile << "frameList and frameNames buffer pointers set; current file position= " << bufIndex << std::endl;
	for (int i= 0; i < this->frameInfo->frameCount; ++i)
	{	
		this->frameList[i].unknown = 0xffffffff;		//I'm using the 'unknown' parameter as a flag to distinguish between dummies, meshes, lights and cameras
		parseFrameListExtension(i);
	}
	return;
}

void DffImporter::readGeometryList()
{
	extensions::GTAHeader* theHeader= readHeader();
	if (!(*theHeader == secIDs::GEOMETRY_LIST))
	{
		//this->logfile << "Geometry list not found at " << bufIndex-12 << std::endl;
		return;
	}

	//this->logfile << "Geometry list, ID= " << theHeader->identifier << " at " << bufIndex-12 << std::endl;

	extensions::GeoListStruct* gListStruct= readBuffer<extensions::GeoListStruct*>(sizeof(extensions::GeoListStruct));
	this->eMeshList= new extensions::MaxGeometry[gListStruct->gCount];
	for (int idx= 0; idx < gListStruct->gCount; ++idx)
		this->readGeometry(idx);

	this->atomList= new extensions::AtomicEntryStruct*[this->leClump->objectCount];
	for (int idx =0; idx < leClump->objectCount; ++idx)
		this->readAtomic(idx);
}
///<summary>Reads geometry section</summary>
///<param name="gIndex">The geometry index</param>
void DffImporter::readGeometry(const UINT& gIndex)
{
	fn(this);
	//Generate the multimaterial
	Mtl* gtaMultiMtl = this->eMeshList[gIndex].material = static_cast<Mtl*>(dffInterface->CreateInstance(SClass_ID(MATERIAL_CLASS_ID), GTA_MULTI_MATERIAL_CLASS_ID));

	extensions::GTAHeader* theHeader= readHeader();
	//this->logfile << "Geometry, ID= " << theHeader->identifier << " ,index = " << gIndex << "at " << bufIndex-12 << std::endl;

	//flags: 1= TSTRIP, 2= VTRANS, 4= TEX1, 8= VCOLOR, 16= NORMS, 32= VLIGHT, 64= MMC, 128= TEX2
	extensions::GeoParams* meshParams= readBuffer<extensions::GeoParams*>(sizeof(extensions::GeoParams));
	//configure Triangle list/strip dropdown box. For some reason listbox items start at index 1.
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::FaceType), 0, (meshParams->flags & 1)+1);
	//configure UV Map 1 checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::UVMap1), 0, (meshParams->flags & 4));
	//configure vertex colors checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::VColors), 0, (meshParams->flags & 8));
	//configure normals checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::Normals), 0, (meshParams->flags & 16));
	//configure dynamic lighting checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::DynLight), 0, (meshParams->flags & 32));
	//configure modulate material color checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::MMC), 0, (meshParams->flags & 64));
	//configure UV Map 2 checkbox
	gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::UVMap2), 0, (meshParams->flags & 128));

	this->eMeshList[gIndex].maxMesh= (TriObject*)dffInterface->CreateInstance(SClass_ID(GEOMOBJECT_CLASS_ID), Class_ID(EDITTRIOBJ_CLASS_ID,0));

	::Mesh& thisMesh= eMeshList[gIndex].maxMesh->mesh;
	thisMesh.setNumVerts(meshParams->vertexCount, FALSE, TRUE);

	thisMesh.setNumFaces(meshParams->faceCount, FALSE, TRUE);
	//this->logfile << "Created geometry " << gIndex << "; vertex count= " << meshParams->vertexCount << "; face count= " << meshParams->faceCount << std::endl;
	//this->logfile << "flags = " << meshParams->flags << "; UV maps = " << meshParams->numUVs;

	//Set GTA3/GTAVC_A version specific ambient, diffuse and specular data
	if (meshParams->structHeader.fileVersion== GTA_IIIA || meshParams->structHeader.fileVersion == GTA_IIIB || 
		meshParams->structHeader.fileVersion == GTA_IIIC || meshParams->structHeader.fileVersion == GTA_VCA)
	{
		//this->logfile << "Skipping 12 bytes of colour data in geometry " << gIndex;
		gtaMultiMtl->GetParamBlock(0)->SetValue(static_cast<ParamID>(GTAMultiMaterialConstants::Amb), 0, *this->readBuffer<float*>(sizeof(float)));
		gtaMultiMtl->GetParamBlock(0)->SetValue((ParamID)GTAMultiMaterialConstants::Diff, 0, *this->readBuffer<float*>(sizeof(float)));
		gtaMultiMtl->GetParamBlock(0)->SetValue((ParamID)GTAMultiMaterialConstants::Spec, 0, *this->readBuffer<float*>(sizeof(float)));
	}

	//check for vertex colours RGBA
	if(meshParams->flags & 8)
	{
		//MessageBox(NULL, TEXT("Vertex colors"), TEXT("Caption"), MB_OK);
		extensions::VertexColors* vColors= readBuffer<extensions::VertexColors*>(sizeof(extensions::VertexColors)*meshParams->vertexCount);
		thisMesh.setMapSupport(-2, 1);		//activate default alpha channel (-2)
		thisMesh.setMapSupport(0,1);		//activate default vertex color channel (0).
		thisMesh.setNumMapVerts(-2, thisMesh.numVerts, 0);
		//thisMesh.setNumMapFaces(-2, thisMesh.numFaces, 0, 0);		//not necessary
		thisMesh.setNumMapVerts(0, thisMesh.numVerts, 0);
		//thisMesh.setNumMapFaces(0, thisMesh.numFaces, 0, 0);		//not necessary
		for (int ind= 0; ind < thisMesh.numVerts; ++ind)
		{
			//set vertex colors (RGB)
			thisMesh.mapVerts(0)[ind].x= vColors[ind].red/255.0f;
			thisMesh.mapVerts(0)[ind].y= vColors[ind].blue/255.0f;
			thisMesh.mapVerts(0)[ind].z= vColors[ind].green/255.0f;
			//vertex alpha, only x coordinate used
			thisMesh.mapVerts(-2)[ind].x= vColors[ind].alpha/255.0f;
			thisMesh.mapVerts(-2)[ind].y= vColors[ind].alpha == 255 ? 1.0f : 0.0f;
			thisMesh.mapVerts(-2)[ind].z= vColors[ind].alpha == 255 ? 1.0f : 0.0f;
		}
	}

	//read UV verts
	extensions::UVData* meshUVs= readBuffer<extensions::UVData*>(sizeof(extensions::UVData)*meshParams->numUVs*meshParams->vertexCount);
	int generalUVIndex= 0;
	for (int ind1= 1; ind1 <= meshParams->numUVs; ++ind1)
	{
		//MessageBox(NULL, TEXT("uv map"), TEXT("Caption"), MB_OK);
		thisMesh.setMapSupport(ind1, true);
		thisMesh.setNumMapVerts(ind1, thisMesh.numVerts, 0);
		//thisMesh.setNumMapFaces(ind1, thisMesh.numFaces,0,0);			//not necessary
		for (int ind2=0; ind2 < thisMesh.numVerts; ++ind2)
		{
			thisMesh.mapVerts(ind1)[ind2].x= meshUVs[generalUVIndex].Udat;
			thisMesh.mapVerts(ind1)[ind2].y= 1.0f - meshUVs[generalUVIndex].Vdat;
			thisMesh.mapVerts(ind1)[ind2].z= 0.0f;
			++generalUVIndex;
		}
	}

	//now reading face info in BinMeshPlg (but reading this section is faster)
	//use this section as reference for setting face indices, if total face count <= 0x10000
	extensions::FaceBAFC* faceInfo= readBuffer<extensions::FaceBAFC*>(sizeof(extensions::FaceBAFC)*meshParams->faceCount);
	
	if (meshParams->faceCount <= 0x10000)
	{
		this->faceRefInfoMap.clear();
		for (UINT i=0; i < thisMesh.numFaces; ++i)
		{
			thisMesh.faces[i].setVerts(faceInfo[i].aDat, faceInfo[i].bDat, faceInfo[i].cDat);
			//thisMesh.faces[i].setMatID(faceInfo[i].fDat); material ID is being set in BinMeshPLG.
			//populate face reference indices map.
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].aDat, faceInfo[i].bDat, faceInfo[i].cDat),i));
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].aDat, faceInfo[i].cDat, faceInfo[i].bDat), i));
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].bDat, faceInfo[i].aDat, faceInfo[i].cDat), i));
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].bDat, faceInfo[i].cDat, faceInfo[i].aDat), i));
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].cDat, faceInfo[i].aDat, faceInfo[i].bDat), i));
			this->faceRefInfoMap.insert(std::make_pair(std::make_tuple(faceInfo[i].cDat, faceInfo[i].bDat, faceInfo[i].aDat), i));
			thisMesh.faces[i].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
			thisMesh.faces[i].setSmGroup(1);
			this->setTextureFaces(thisMesh, i, meshParams);
		}
	}
	//skip bounding information
	bufIndex += 24;

	//check for vertex translation info
	//if ((flags & 2) == 2) //this flag is sometimes 0 in VC and GTAIII even though translation info is present

	//MessageBox(NULL, this->ToString<UINT>(this->bufIndex).c_str(), TEXT("Reading vertices"), MB_OK);
	extensions::Vector3* vertexList= readBuffer<extensions::Vector3*>(sizeof(extensions::Vector3)*meshParams->vertexCount);
	//MessageBox(NULL, this->ToString<UINT>(this->bufIndex).c_str(), TEXT("Reading vertices"), MB_OK);
	for (int ind= 0; ind < thisMesh.numVerts; ++ind)
		thisMesh.setVert(ind, vertexList[ind].x, vertexList[ind].y, vertexList[ind].z);

	//MessageBox(NULL, this->ToString<UINT>(this->bufIndex).c_str(), TEXT("Reading normals"), MB_OK);
	//save possible address of normals data, in case present
	if ((meshParams->flags & 16))
		vertexList= readBuffer<extensions::Vector3*>(sizeof(extensions::Vector3)*meshParams->vertexCount);
	//MessageBox(NULL, this->ToString<UINT>(this->bufIndex).c_str(), TEXT("Reading normals"), MB_OK);

	//skip material list

	parseMaterialList(gIndex);
	//thisMesh.InvalidateGeomCache();
	//Read Bin Mesh PLG, Native Data PLG (on Xbox and PS2), Skin PLG Mesh Extension, Night Vertex Colors, Morph PLG and 2dfx

	this->readGeometryExtension(thisMesh, meshParams, gIndex);

	//this->logfile << "Revised faces" << thisMesh.numFaces << std::endl;
	if ((meshParams->flags & 16))
	{
		thisMesh.SpecifyNormals();
		MeshNormalSpec* nSpec= thisMesh.GetSpecifiedNormals();
		nSpec->BuildNormals();
		nSpec->SetFlag(MESH_NORMAL_MODIFIER_SUPPORT, true);
		nSpec->MakeNormalsExplicit(false, 0, true);
		for (int ind= 0; ind < thisMesh.numFaces; ++ind)
		{
			for (int ind1= 0; ind1 <3; ++ind1)
			{
				int vIndex= thisMesh.faces[ind].getVert(ind1);
				int normID= nSpec->Face(ind).GetNormalID(ind1);
				//nSpec->Normal(normID).x= *(normalPoints+vIndex*3);
				//nSpec->Normal(normID).y= *(normalPoints+vIndex*3+1);
				//nSpec->Normal(normID).z= *(normalPoints+vIndex*3+2);
				nSpec->Normal(normID).Set(vertexList[vIndex].x, vertexList[vIndex].y, vertexList[vIndex].z);
			}
		}

	}
	
	thisMesh.DeleteIsoVerts();
	thisMesh.DeleteIsoMapVerts();
	thisMesh.BuildStripsAndEdges();
	//this->logfile << "You are here" << std::endl;
}

void DffImporter::buildScene()
{
	for (int i=0; i < this->frameInfo->frameCount; ++i)
	{
		switch(this->frameList[i].unknown)
		{
			case 0xffffffff:
			{
				DummyObject* leDummy= (DummyObject*)dffInterface->CreateInstance(SClass_ID(HELPER_CLASS_ID), Class_ID(DUMMY_CLASS_ID,0));
				this->maxObjectList[i].theNode= dffInterface->CreateObjectNode(leDummy);
				this->maxObjectList[i].theNode->Scale(0, frameList[i].transMatrix, Point3(10,10,10), 1, 0, PIV_OBJECT_ONLY, 1);
			}
			break;
			case 0xfffffffe:
			{
				LightObject* leLight;
				if (maxObjectList[i].leGlobeStruct->lightType == 0x80)
					leLight= (LightObject*) dffInterface->CreateInstance(SClass_ID(LIGHT_CLASS_ID), Class_ID(OMNI_LIGHT_CLASS_ID, 0));
				else
					leLight= (LightObject*) dffInterface->CreateInstance(SClass_ID(LIGHT_CLASS_ID), Class_ID(FSPOT_LIGHT_CLASS_ID, 0));
				leLight->SetAtten(0, LIGHT_ATTEN_END, maxObjectList[i].leGlobeStruct->attenuation);
				Point3 temp;
				temp.x= maxObjectList[i].leGlobeStruct->RGBcolor[0];//*255.0f;
				temp.y= maxObjectList[i].leGlobeStruct->RGBcolor[1];//*255.0f;
				temp.z= maxObjectList[i].leGlobeStruct->RGBcolor[2];//*255.0f;
				leLight->SetRGBColor(0, temp);
				//leLight->SetFallsize(0, maxObjectList[i].leGlobeStruct->FallSize);		//this parameter is not the fallsize
				leLight->Enable(1);
				leLight->SetConeDisplay(0, 1);
				this->maxObjectList[i].theNode= dffInterface->CreateObjectNode(leLight);
			}
			break;
			case 0xfffffffd:
			break;
			default:
			{
				maxObjectList[i].theNode= dffInterface->CreateObjectNode(this->eMeshList[frameList[i].unknown].maxMesh);
				maxObjectList[i].theNode->SetMtl(this->eMeshList[frameList[i].unknown].material);
			}
			break;
		}

		maxObjectList[i].theNode->SetNodeTM(0, Matrix3(frameList[i].transMatrix));
#ifdef UNICODE
		std::unique_ptr<TCHAR> wbuffer(new TCHAR[strlen(this->frameNames[i])+1]);
		mbstowcs_s(nullptr, wbuffer.get(), strlen(this->frameNames[i])+1, this->frameNames[i], strlen(this->frameNames[i]));
		this->maxObjectList[i].theNode->SetName(wbuffer.get());
#else
		maxObjectList[i].theNode->SetName(this->frameNames[i]);
#endif
		delete[] frameNames[i];
		this->addHAnimProps(i);
	}
	delete[] frameNames;
	frameNames= NULL;

	for (int i= 0; i < this->frameInfo->frameCount; ++i)
		if (frameList[i].parent < this->frameInfo->frameCount)
			maxObjectList[frameList[i].parent].theNode->AttachChild(maxObjectList[i].theNode, 0);

	delete[] maxObjectList;	maxObjectList= NULL;
	delete[] eMeshList; eMeshList= NULL;
	delete[] atomList; atomList= NULL;

	this->HAnimIDFrameKeys.clear();
	this->HAnimFrameIDKeys.clear();
	//this->HAnimList.clear();
	//MessageBox(NULL, "complete", NULL, MB_OK);
}

void DffImporter::parseFrameListExtension(const int& frameIndex)
{
	fn(this);
	extensions::GTAHeader* extHeader= this->readHeader();
	UINT temp= bufIndex;
	while(bufIndex < temp+extHeader->size)
	{
		extensions::GTAHeader* leHeader= this->readHeader();
		switch(leHeader->identifier)
		{
			case secIDs::HANIM_PLG:
			//skip this
				this->readHAnimPLG(frameIndex);
			break;

			case secIDs::FRAME:
				this->frameNames[frameIndex]= new char[leHeader->size+1];
				strncpy(frameNames[frameIndex], (const char*)(buffer+bufIndex), leHeader->size);
				frameNames[frameIndex][leHeader->size]= '\0';
				bufIndex += leHeader->size;		//important
				//this->logfile << "frame name= " << frameNames[frameIndex] << ", file position= " << bufIndex << std::endl;
			break;

			default:
				bufIndex -= 12;
			break;
		}
	}
	if (extHeader->size == 0)
	{
		frameNames[frameIndex]= new char[7];
		strcpy(frameNames[frameIndex], "NoName");
	}
}

void DffImporter::parseMaterialList(const int& geoIndex)
{
	extensions::GTAHeader* leHeader= readHeader();
	if (leHeader->getIdentifier() != secIDs::MATERIAL_LIST)
	{
		//MessageBox(this->dffInterface->GetMAXHWnd(), this->ToString<UINT>(leHeader->identifier).c_str(), TEXT("Caption"), MB_OK);
		return;
	}

	//this->logfile << "Reading material list in geometry " << geoIndex << " at " << bufIndex-12 << std::endl;
	//frameStructStart used here since the data is represented similarly.
	extensions::FrameStructStart* materialListStruct= readBuffer<extensions::FrameStructStart*>(sizeof(extensions::FrameStructStart));
	bufIndex += materialListStruct->frameCount*4;

	//Attempt to create GTAMultiMaterial. Will fail if GTAMultiMaterial.ms script hasn't been loaded.
	/*this->eMeshList[geoIndex].material = static_cast<Mtl*>(this->dffInterface->CreateInstance(SClass_ID(MATERIAL_CLASS_ID), GTA_MULTI_MATERIAL_CLASS_ID));
	//A custom parameter(index 16) was being used to set submaterial count in the script. Have now figured how to access embedded3ds MultiMaterial directly.
	if (this->eMeshList[geoIndex].material != nullptr)
	{
		MultiMtl* internalMultimat = (MultiMtl*)this->eMeshList[0].material->GetReference(0);
		internalMultimat->SetNumSubMtls(materialListStruct->frameCount);
	}
	else
	{
		this->eMeshList[geoIndex].material = (Mtl*)dffInterface->CreateInstance(SClass_ID(MATERIAL_CLASS_ID), Class_ID(MULTI_CLASS_ID, 0));
		reinterpret_cast<MultiMtl*>(this->eMeshList[geoIndex].material)->SetNumSubMtls(materialListStruct->frameCount);
	}*/
	//GTAMultiMaterial already generated in DffImporter::ReadGeometry(const UINT&)
	MultiMtl* internalMultimat = static_cast<MultiMtl*>(this->eMeshList[geoIndex].material->GetReference(0));
	internalMultimat->SetNumSubMtls(materialListStruct->frameCount);

	for (int i= 0; i < materialListStruct->frameCount; ++i)
		this->readMaterial(i, eMeshList[geoIndex].material);
}

void DffImporter::readMaterial(const int& matID, Mtl* multiMat)
{
	fn(this);
	//this->logfile << "Reading material "<< matID << "at " << bufIndex << std::endl;
	::Mtl* gtaMat= (Mtl*)dffInterface->CreateInstance(SClass_ID(MATERIAL_CLASS_ID), Class_ID(GTAMAT_A, GTAMAT_B));
	gtaMat->SetMtlFlag(MTL_DISPLAY_ENABLE_FLAGS);
	extensions::GTAHeader* leHeader= readHeader();
	extensions::MaterialStruct* matStruct= readBuffer<extensions::MaterialStruct*>(sizeof(extensions::MaterialStruct));
	//set color and alpha
	//gtaMat->GetParamBlock(0)->SetValue(3, 0, ::Color(matStruct->matColors.red/255.0f, matStruct->matColors.blue/255.0f, matStruct->matColors.green/255.0f), 0);
	//gtaMat->GetParamBlock(0)->SetValue(6, 0, (int)matStruct->matColors.alpha, 0);
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::color, 0, Color(matStruct->matColors.red / 255.0f, matStruct->matColors.blue / 255.0f, matStruct->matColors.green / 255.0f));
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::alpha, 0, (int)matStruct->matColors.alpha, 0);
	//ambient, specular and diffuse
	/*gtaMat->GetParamBlock(0)->SetValue(0,0, matStruct->ambient, 0);
	gtaMat->GetParamBlock(0)->SetValue(1,0, matStruct->specular, 0);
	gtaMat->GetParamBlock(0)->SetValue(2,0, matStruct->diffuse, 0);*/
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::amb, 0, matStruct->ambient, 0);
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::spc, 0, matStruct->specular, 0);
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::dif, 0, matStruct->diffuse, 0);
	//set unknown parameters.
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::UInteger1, 0, this->ToString<UINT>(matStruct->unknown1,true).c_str(),0);
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::UInteger2, 0, this->ToString<UINT>(matStruct->unknown2,true).c_str(), 0);
	for (int ind= 0; ind < matStruct->textureCount; ++ind)
		this->readTextures(gtaMat);

	this->readMatExtension(gtaMat);
	multiMat->SetSubMtl(matID, gtaMat);
}

void DffImporter::readTextures(Mtl* gtaMat)
{
	fn(this);
	//this->logfile << "Reading material texture at " << bufIndex << std::endl;
	readHeader();		//read (and skip) texture section header
	extensions::TextureSectionStruct* tStruct= readBuffer<extensions::TextureSectionStruct*>(sizeof(extensions::TextureSectionStruct));

	//read diffuse texture
	extensions::GTAHeader* texHeader= readHeader();
	//::BitmapTex* diffTex= (BitmapTex*)dffInterface->CreateInstance(SClass_ID(TEXMAP_CLASS_ID), Class_ID(BMTEX_CLASS_ID,0));
	BitmapTex* diffTex= ::NewDefaultBitmapTex();
#ifdef UNICODE
	std::unique_ptr<TCHAR> wString(new TCHAR[strlen(buffer+bufIndex)+1]);
	mbstowcs_s(nullptr, wString.get(), strlen(buffer+bufIndex)+1, buffer+bufIndex, strlen(buffer+bufIndex));
	diffTex->SetName(wString.get());
#else
	diffTex->SetName(buffer+bufIndex);
#endif
	bufIndex += texHeader->size;

	
	//apply textures if present
	this->setTextureMap(diffTex);

	//set tiling and mirror data from filter flags
	switch (tStruct->filterFlags[0]) //texture filtering: --byte; 1 None/Point, 2 Summed Area/Linear, 6 Pyramidal/Anisotropic
	{				
		case 1:
			diffTex->SetFilterType(FILTER_NADA);
		break;
		case 2:
			diffTex->SetFilterType(FILTER_SAT);
		break;
		default:
			diffTex->SetFilterType(FILTER_PYR);
		break;
	}

	if (tStruct->filterFlags[1] < 0x10)
		tStruct->filterFlags[1] += 0x10;

	diffTex->GetUVGen()->SetFlag(U_WRAP, (tStruct->filterFlags[1]  & 0x01));		
	diffTex->GetUVGen()->SetFlag(U_MIRROR, (tStruct->filterFlags[1] & 0x02));
	diffTex->GetUVGen()->SetFlag(V_WRAP, (tStruct->filterFlags[1]  & 0x10));
	diffTex->GetUVGen()->SetFlag(V_MIRROR, (tStruct->filterFlags[1] & 0x20));

	//gtaMat->GetParamBlock(0)->SetValue(4,0, diffTex,0);
	gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::colormap, 0, diffTex);
	//read alpha texture and apply if name string is not "\0"
	texHeader= readHeader();		//alpha texture header
	if (buffer[bufIndex] != '\0')
	{
		//::BitmapTex* alphaTex= (BitmapTex*)dffInterface->CreateInstance(SClass_ID(TEXMAP_CLASS_ID), Class_ID(BMTEX_CLASS_ID,0));
		BitmapTex* alphaTex= NewDefaultBitmapTex();
#ifdef UNICODE
		std::unique_ptr<TCHAR> wString(new TCHAR[strlen(buffer+bufIndex)+1]);
		mbstowcs_s(nullptr, wString.get(), strlen(buffer + bufIndex) + 1, buffer + bufIndex, strlen(buffer + bufIndex));
		alphaTex->SetName(wString.get());
#else
		alphaTex->SetName(buffer + bufIndex);
#endif
		//apply textures if present
		this->setTextureMap(alphaTex);

		//set texture filter type
		alphaTex->SetFilterType(diffTex->GetFilterType());

		//set wrap / mirror flags
		alphaTex->GetUVGen()->SetFlag(U_WRAP, (tStruct->filterFlags[1] & 0x01));
		alphaTex->GetUVGen()->SetFlag(U_MIRROR, (tStruct->filterFlags[1] & 0x02));
		alphaTex->GetUVGen()->SetFlag(V_WRAP, (tStruct->filterFlags[1] & 0x10));
		alphaTex->GetUVGen()->SetFlag(V_MIRROR, (tStruct->filterFlags[1] & 0x20));

		//gtaMat->GetParamBlock(0)->SetValue(7, 0, alphaTex,0);		//apply alpha texture
		gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::alphamap, 0, alphaTex, 0);
		//gtaMat->GetParamBlock(0)->SetValue(8, 0, TRUE, 0);			//texture active by default
	}
	else
		//gtaMat->GetParamBlock(0)->SetValue(8, 0, FALSE, 0);		//deactivate texture
		gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::use_alphamap,0, FALSE, 0);
	//move pointer to end of alpha texture section
	bufIndex += texHeader->size;

	//skip extension if present
	texHeader= readHeader();
	if(*texHeader == secIDs::EXTENSION)
		bufIndex += texHeader->size;
	else
		bufIndex -= 4;
}

void DffImporter::readMatExtension(Mtl* gtaMat)
{
	fn(this);
	extensions::GTAHeader* matExtHeader= readHeader();

	if (!(*matExtHeader == secIDs::EXTENSION))
	{
		//this->logfile << "Material extension header not found at "  << bufIndex -12 << std::endl;
		return;
	}
	//this->logfile << "Reading material extension at " << bufIndex-12 << std::endl;
	UINT temp= bufIndex + matExtHeader->size;
	while (bufIndex < temp)
	{
		extensions::GTAHeader* leHeader= readHeader();
		UINT secEnd= bufIndex + leHeader->size;
		switch (leHeader->identifier)
		{
			case MATERIAL_EFFECTS_PLG:
			{
				//Material effects PLG can be of variable size
				//set flag for easy export afterwards
				//gtaMat->GetParamBlock(0)->SetValue(19, 0, TRUE, 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::use_RF, 0, TRUE, 0);
				extensions::MatEffectsPlgStart* mEffectsPlg= readBuffer<extensions::MatEffectsPlgStart*>(sizeof(extensions::MatEffectsPlgStart));
				if (mEffectsPlg->unknown2 == 2)
				{
					//gtaMat->GetParamBlock(0)->SetValue(9, 0, (*readBuffer<float*>(4))*100.0f, 0);	//reflection value
					//gtaMat->GetParamBlock(0)->SetValue(11, 0, TRUE, 0);		//use reflection map
					gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::Reflection, 0, (*readBuffer<float*>(4))*100.0f, 0);	//reflection value
					gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::use_reflectionmap, 0, TRUE, 0);		//use reflection map
					bufIndex +=4; //skip unknown (= 0)
					//revision March 8 2012. Zmodeler2 treats this as texture count instead of an ON/OFF switch
					if (*readBuffer<UINT*>(4) == 0)	//if texture switch equals 0 trash 00 00 00 00 data comes next.
						bufIndex = secEnd;
				}
				else
					bufIndex = secEnd;

				if( *readHeader() == secIDs::TEXTURE)
				{
					extensions::TextureSectionStruct* tStruct= readBuffer<extensions::TextureSectionStruct*>(sizeof(extensions::TextureSectionStruct));

					//read diffuse texture name
					extensions::GTAHeader* texHeader= readHeader();
					//::BitmapTex* mdiffTex= (BitmapTex*)dffInterface->CreateInstance(SClass_ID(TEXMAP_CLASS_ID), Class_ID(BMTEX_CLASS_ID,0));
					BitmapTex* mdiffTex= NewDefaultBitmapTex();
#ifdef UNICODE
					std::unique_ptr<TCHAR> wString(new TCHAR[strlen(buffer+bufIndex)+1]);
					mbstowcs_s(nullptr, wString.get(), strlen(buffer+bufIndex)+1, buffer+bufIndex, strlen(buffer+bufIndex));
					mdiffTex->SetName(wString.get());
#else
					mdiffTex->SetName(buffer+bufIndex);
#endif
					bufIndex += texHeader->size;
					//apply textures if present
					this->setTextureMap(mdiffTex);

					//skip alpha texture
					extensions::GTAHeader* mAlphaHeader= readHeader();
					if (*mAlphaHeader == secIDs::STRING)
						bufIndex += mAlphaHeader->size;
					else
						bufIndex -= 12;

					//set tiling and mirror data from filter flags
					switch (tStruct->filterFlags[0])				//texture filtering: --byte; 1 None/Point, 2 Summed Area/Linear, 6 Pyramidal/Anisotropic
					{
					case 1:
						mdiffTex->SetFilterType(FILTER_NADA);
					break;
					case 2:
						mdiffTex->SetFilterType(FILTER_SAT);
					break;
					default:
						mdiffTex->SetFilterType(FILTER_PYR);
					}
					if (tStruct->filterFlags[1] < 0x10)
						tStruct->filterFlags[1] += 0x10;
					mdiffTex->GetUVGen()->SetFlag(U_WRAP, (tStruct->filterFlags[1] & 0x01));
					mdiffTex->GetUVGen()->SetFlag(U_MIRROR, (tStruct->filterFlags[1] & 0x02));
					mdiffTex->GetUVGen()->SetFlag(V_WRAP, (tStruct->filterFlags[1] & 0x01));
					mdiffTex->GetUVGen()->SetFlag(V_MIRROR, (tStruct->filterFlags[1] & 0x02));

					//gtaMat->GetParamBlock(0)->SetValue(10,0, mdiffTex,0);	//reflection map
					gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::reflectionmap, 0, mdiffTex, 0);
					//extensions::GTAHeader* extHeader= readHeader();
					//if (*extHeader == secIDs::EXTENSION)
						//bufIndex += extHeader->size;
					//else
						//bufIndex -= 12;

					//bufIndex += 4;			//for some reason trash data 00 00 00 00 present
					bufIndex = secEnd;	//we are done (no need to read empty extension)
				}

				else
					bufIndex -= 12;
			}
			break;

			case secIDs::REFLECTION_MATERIAL:
			{
				//this->logfile << "Reading reflection material" << std::endl;
				//REFLECTION_MATERIAL equivalent to SA specular in Kam's script. Set on flag for easy export afterwards
				//gtaMat->GetParamBlock(0)->SetValue(20, 0, TRUE, 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::use_SAS, 0, TRUE, 0);
				extensions::ReflectionMaterial* rMatData= readBuffer<extensions::ReflectionMaterial*>(sizeof(extensions::ReflectionMaterial));
				/*gtaMat->GetParamBlock(0)->SetValue(12, 0, Color(rMatData->specColor.x, rMatData->specColor.y, rMatData->specColor.z), 0);
				gtaMat->GetParamBlock(0)->SetValue(15, 0, (int)(255.0f*rMatData->specAlpha), 0);
				gtaMat->GetParamBlock(0)->SetValue(17, 0, rMatData->specRefBlend, 0);*/
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::specular, 0, Color(rMatData->specColor.x, rMatData->specColor.y, rMatData->specColor.z), 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::spec_alpha, 0, (int)(255.0f*rMatData->specAlpha), 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::blend, 0, rMatData->specRefBlend, 0);
			}
			break;

			case secIDs::SPECULAR_MATERIAL:
			{
				//set flag for easy export afterwards
				//gtaMat->GetParamBlock(0)->SetValue(21, 0, TRUE, 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::use_SI, 0, TRUE, 0);
				extensions::SpecularInfo* leSpec= readBuffer<extensions::SpecularInfo*>(sizeof(extensions::SpecularInfo));
				//gtaMat->GetParamBlock(0)->SetValue(16, 0, 100.f*leSpec->glossiness, 0);
				gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::spec_power, 0, 100.f*leSpec->glossiness, 0);
				//BitmapTex* mdiffTex= (BitmapTex*)dffInterface->CreateInstance(SClass_ID(TEXMAP_CLASS_ID), Class_ID(BMTEX_CLASS_ID, 0));
				if (leSpec->specTexture[0] != '\0')
				{
					BitmapTex* mdiffTex= NewDefaultBitmapTex();
#ifdef UNICODE
					std::unique_ptr<TCHAR> wString(new TCHAR[24]);
					mbstowcs_s(nullptr, wString.get(), 24, leSpec->specTexture, 23);
					mdiffTex->SetName(wString.get());
#else
					mdiffTex->SetName(leSpec->specTexture);
#endif
					//set textures if present
					this->setTextureMap(mdiffTex);
					//gtaMat->GetParamBlock(0)->SetValue(13, 0, mdiffTex, 0);
					gtaMat->GetParamBlock(0)->SetValue((ParamID)GTAMaterialConstants::specularmap, 0, mdiffTex, 0);
				}
			}
			break;
			
			case secIDs::UV_ANIM_PLG:
			{
				//this->logfile << "Reading UV anim PLG" << std::endl;
				//skip this
				//bufIndex += *convBytes<UINT*>(4)+4;
				//this section is assumed to always be 60 bytes in size (comprising 12 byte UV_ANIM_PLG header and 48 byte struct)
				//struct contains an integer/unsigned integer(1) followed by 32 byte char string padded with zeros.
				//signed/unsiged integer is always 1 in Deniska's script
				//skip UV_ANIM_PLG size info, RW version, struct header, size, RW version and signed/unsigned integer
				//anims (in ascending order): U offset, V offset, U tiling, V tiling, U angle, V angle, W angle, Blur, Noise amount, Noise size
				//, Noise levels, Phase, Blur offset
				extensions::UVAnimPLGStruct* UVstruct= readBuffer<extensions::UVAnimPLGStruct*>(sizeof(extensions::UVAnimPLGStruct));
				//for (int ind= 0; ind < diffTex->GetUVGen()->SubAnim(0)->NumSubs(); ++ind)
					//MessageBox(NULL, diffTex->GetUVGen()->SubAnim(0)->SubAnimName(ind), "Caption", MB_OK);
				std::map<std::string, std::vector<extensions::UVAnimKey*>>::iterator leIterator= this->UVAnimList.find(UVstruct->AnimMaterial);
				if (leIterator == this->UVAnimList.end())
					break;

				Control* UOffsetCtrl= ::NewDefaultFloatController();
				Control* VOffsetCtrl= ::NewDefaultFloatController();
				Control* UTileCtrl= ::NewDefaultFloatController();
				Control* VTileCtrl= ::NewDefaultFloatController();
				::BitmapTex* diffTex;	// =(BitmapTex*)dffInterface->CreateInstance(SClass_ID(TEXMAP_CLASS_ID), Class_ID(BMTEX_CLASS_ID,0));
				if(!gtaMat->GetParamBlock(0)->GetValue((ParamID)GTAMaterialConstants::colormap, 0, (Texmap*&)diffTex, FOREVER, 0))
				{
					//this->logfile << "Unable to retrieve diffuse texture in UV anim PLG" << std::endl;
					break;
				}
				diffTex->GetUVGen()->SubAnim(0)->AssignController(UOffsetCtrl, 0);
				diffTex->GetUVGen()->SubAnim(0)->AssignController(VOffsetCtrl, 1);
				diffTex->GetUVGen()->SubAnim(0)->AssignController(UTileCtrl, 2);
				diffTex->GetUVGen()->SubAnim(0)->AssignController(VTileCtrl, 3);
				::SuspendAnimate();
				::AnimateOn();
				for(std::vector<extensions::UVAnimKey*>::iterator iter = leIterator->second.begin(); iter < leIterator->second.end(); ++iter)
				{
					float UOffset= -1.0f * (*iter)->negUOffset;
					UOffsetCtrl->SetValue((*iter)->timeOver4800*4800.0f, (void*)&UOffset, 1, CTRL_ABSOLUTE);
					VOffsetCtrl->SetValue((*iter)->timeOver4800*4800.0f, (void*)&(*iter)->VOffset, 1, CTRL_ABSOLUTE);
					UTileCtrl->SetValue((*iter)->timeOver4800*4800.0f, (void*)&(*iter)->UTiling, 1, CTRL_ABSOLUTE);
					VTileCtrl->SetValue((*iter)->timeOver4800*4800.0f, (void*)&(*iter)->VTiling, 1, CTRL_ABSOLUTE);
				}
				::ResumeAnimate();

			}
			break;

			default:
				bufIndex -= 12;
			break;
		}
	}

}

void DffImporter::readUVAnims()
{
	fn(this);
	//this->logfile << "Reading UV anim dictionary at " << bufIndex -12 << std::endl;
	extensions::FrameStructStart* UVanimDicStart= readBuffer<extensions::FrameStructStart*>(sizeof(extensions::FrameStructStart));
	for (int i= 0; i < UVanimDicStart->frameCount; ++i)
	{
		extensions::AnimAnimationStart* animStart= readBuffer<extensions::AnimAnimationStart*>(sizeof(extensions::AnimAnimationStart));
		extensions::UVAnimKey* theKeys= readBuffer<extensions::UVAnimKey*>(sizeof(extensions::UVAnimKey)*animStart->numKeys);
		//this->logfile << "You are now at " << bufIndex << std::endl;
		std::string temp= animStart->AnimMaterial;
		std::pair<std::map<std::string, std::vector<extensions::UVAnimKey*>>::iterator, bool> returnVal 
					= this->UVAnimList.insert(std::make_pair(temp, std::vector<extensions::UVAnimKey*>(animStart->numKeys)));
		for (int j=0; j < animStart->numKeys; ++j)
			returnVal.first->second[j]= (theKeys+j);
	}
}
void DffImporter::readGeometryExtension(Mesh& theMesh, const extensions::GeoParams* meshParams, const UINT& gIndex)
{
	fn(this);
	extensions::GTAHeader* extHeader= readHeader();
	//this->logfile << "Reading geometry extension, ID=  "<< extHeader->identifier << " at " << bufIndex-12 << std::endl;


	UINT temp= bufIndex + extHeader->size;
	while (bufIndex < temp)
	{
		extensions::GTAHeader* leHeader= readHeader();
		switch (leHeader->identifier)
		{
			case secIDs::BIN_MESH_PLG:
				/*if (meshParams->faceCount <= 0x10000)
					bufIndex += leHeader->size;
				else*/
				this->readBinMesh(theMesh, leHeader->size, meshParams);
			break;
			
			case secIDs::NATIVE_DATA_PLG:
				bufIndex += leHeader->size;
			break;

			case secIDs::SKIN_PLG:
				this->IsCharDff= true;
				this->readSkin(gIndex, meshParams, leHeader);
				//bufIndex += leHeader->size;
			break;

			case secIDs::MESH_EXTENSION:
				bufIndex += leHeader->size;
			break;

			case secIDs::NIGHT_VERTEX_COLORS:
				this->readNVC(gIndex);
			break;

			case secIDs::MORPH_PLG:
				bufIndex += leHeader->size;
			break;

			case secIDs::TWO_DFX_PLG:
				bufIndex += leHeader->size;
			break;
			default:
				//this->logfile << "Unknown section " << leHeader->identifier << " in geometry extension at " << bufIndex-12;
			break;
		}	
	}
}

void DffImporter::readBinMesh(Mesh& theMesh, const UINT& sectionSize, const extensions::GeoParams* meshParams)
{
	//this->logfile << "Reading BinMeshPlg" << std::endl;
	UINT BinMeshEnd= bufIndex + sectionSize;
	extensions::BinMeshPLGTotal* theTotal= readBuffer<extensions::BinMeshPLGTotal*>(sizeof(extensions::BinMeshPLGTotal));
	int ind= 0;

	if (meshParams->faceCount > 0x10000)
	{
		if (theTotal->isTriStrip)
		{
			theMesh.setNumFaces(theTotal->tIndexCount - 2, FALSE, TRUE);
			for (int splitIndex = 0; splitIndex < theTotal->tSplitCount; ++splitIndex)
			{
				//get subindex count and material index
				extensions::BinMeshPLGSub* theSub = readBuffer<extensions::BinMeshPLGSub*>(sizeof(extensions::BinMeshPLGSub));
				for (int idx = 0; idx < (theSub->indexCount) - 2; ++idx)
				{
					extensions::FaceIndexStruct* testFaceIndices = readBuffer<extensions::FaceIndexStruct*>(4);
					if (testFaceIndices->x ^ testFaceIndices->y && testFaceIndices->x ^ testFaceIndices->z && testFaceIndices->y ^ testFaceIndices->z)
					{
						this->setFaceInfo(theMesh, ind, meshParams, theSub, testFaceIndices);
						this->setTextureFaces(theMesh, ind, meshParams);
						++ind;
					}
				}
				bufIndex += 8; //because we are stopping 8 bytes before final index in split
			}
			//theMesh.RemoveDegenerateFaces(); //inverting order causes crashes!!!
			//theMesh.RemoveIllegalFaces();
			//set the correct face count
			theMesh.setNumFaces(ind, TRUE, TRUE);
		}
		else
		{
			theMesh.setNumFaces(theTotal->tIndexCount / 3, FALSE, TRUE);
			for (int splitIndex = 0; splitIndex < theTotal->tSplitCount; ++splitIndex)
			{
				//get subindex count and material index
				extensions::BinMeshPLGSub* theSub = readBuffer<extensions::BinMeshPLGSub*>(sizeof(extensions::BinMeshPLGSub));
				for (int idx = 0; idx < theSub->indexCount; idx += 3)
				{
					extensions::FaceIndexStruct* testFaceIndices = readBuffer<extensions::FaceIndexStruct*>(12);
					this->setFaceInfo(theMesh, ind, meshParams, theSub, testFaceIndices);
					this->setTextureFaces(theMesh, ind, meshParams);
					++ind;
				}
			}
		}
	}
	else
	{
		if (theTotal->isTriStrip)
		{
			for (int splitIndex = 0; splitIndex < theTotal->tSplitCount; ++splitIndex)
			{
				//get subindex count and material index
				extensions::BinMeshPLGSub* theSub = readBuffer<extensions::BinMeshPLGSub*>(sizeof(extensions::BinMeshPLGSub));
				for (int idx = 0; idx < (theSub->indexCount) - 2; ++idx)
				{
					extensions::FaceIndexStruct* testFaceIndices = readBuffer<extensions::FaceIndexStruct*>(4);
					if (testFaceIndices->x ^ testFaceIndices->y && testFaceIndices->x ^ testFaceIndices->z && testFaceIndices->y ^ testFaceIndices->z)
					{
						theMesh.faces[this->faceRefInfoMap[std::make_tuple(testFaceIndices->x, testFaceIndices->y, testFaceIndices->z)]].setMatID(theSub->materialID);
						++ind;
					}
				}
				bufIndex += 8; //because we are stopping 8 bytes before final index in split
			}
		}
		else
		{
			for (int splitIndex = 0; splitIndex < theTotal->tSplitCount; ++splitIndex)
			{
				//get subindex count and material index
				extensions::BinMeshPLGSub* theSub = readBuffer<extensions::BinMeshPLGSub*>(sizeof(extensions::BinMeshPLGSub));
				for (int idx = 0; idx < theSub->indexCount; idx += 3)
				{
					extensions::FaceIndexStruct* testFaceIndices = readBuffer<extensions::FaceIndexStruct*>(12);
					theMesh.faces[this->faceRefInfoMap[std::make_tuple(testFaceIndices->x, testFaceIndices->y, testFaceIndices->z)]].setMatID(theSub->materialID);
					++ind;
				}
			}
		}
	}
	bufIndex = BinMeshEnd;
}

inline void DffImporter::setFaceInfo(Mesh& theMesh, const UINT& faceIndex, const extensions::GeoParams* meshParams, const extensions::BinMeshPLGSub* theSub, const extensions::FaceIndexStruct* faceIndices)
{
	//this->logfile << "setting face info" << std::endl;
	//if (faceIndex >= theMesh.numFaces)
		//return;
	theMesh.faces[faceIndex].setVerts(faceIndices->x, faceIndices->y, faceIndices->z);
	theMesh.faces[faceIndex].setMatID(theSub->materialID);
	theMesh.faces[faceIndex].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
	theMesh.faces[faceIndex].setSmGroup(1);

}

inline void DffImporter::setTextureFaces(Mesh& theMesh, const UINT& faceIndex, const extensions::GeoParams* meshParams)
{
	//vertex color faces
	if ((meshParams->flags & 8))
	{
		theMesh.mapFaces(-2)[faceIndex].setTVerts(theMesh.faces[faceIndex].getVert(0), theMesh.faces[faceIndex].getVert(1), theMesh.faces[faceIndex].getVert(2));
		theMesh.mapFaces(0)[faceIndex].setTVerts(theMesh.faces[faceIndex].getVert(0), theMesh.faces[faceIndex].getVert(1), theMesh.faces[faceIndex].getVert(2));
	}
			
	//UV faces
	for (int ind3=1; ind3 <= meshParams->numUVs; ++ind3)
		theMesh.mapFaces(ind3)[faceIndex].setTVerts(theMesh.faces[faceIndex].getVert(0), theMesh.faces[faceIndex].getVert(1), theMesh.faces[faceIndex].getVert(2));
}

void DffImporter::readAtomic(const INT& objectIndex)
{
	fn(this);
	extensions::GTAHeader* theHeader= readHeader();
	//this->logfile << "Reading atomic section, header ID= " << theHeader->identifier << " at " << bufIndex - 12 << std::endl;

	this->atomList[objectIndex]= readBuffer<extensions::AtomicEntryStruct*>(sizeof(extensions::AtomicEntryStruct));
	this->frameList[this->atomList[objectIndex]->frameIndex].unknown= this->atomList[objectIndex]->geoIndex;
	Mtl* multiMat = this->eMeshList[this->atomList[objectIndex]->geoIndex].material;

	theHeader= readHeader();
	if (*theHeader == secIDs::EXTENSION)
	{
		UINT secEnd = bufIndex + theHeader->size;
		while (bufIndex < secEnd)
		{
			theHeader = this->readHeader();
			switch (theHeader->identifier)
			{
				case secIDs::PIPELINE_SET:
				{
					int dropdownIndex = 1;
					switch (*this->readBuffer<GTAPipelineSet*>(sizeof(GTAPipelineSet)))
					{
						case GTAPipelineSet::ReflectiveMaterial:
							dropdownIndex = 2;
						break;
						case GTAPipelineSet::NightVertexColors:
							dropdownIndex = 3;
						break;
						case GTAPipelineSet::Vehicle:
							dropdownIndex = 4;
						break;
						default:
						break;
					}
					multiMat->GetParamBlock(0)->SetValue((ParamID)GTAMultiMaterialConstants::RenderPipe, dffInterface->GetTime(), dropdownIndex);
				}
				break;

				default:
					bufIndex += theHeader->size;
				break;
			}
		}
		bufIndex = secEnd;
	}
	else
		bufIndex -= 12;
}

void DffImporter::readLights()
{
	fn(this);
	extensions::FrameStructStart* LightIdxStruct= readBuffer<extensions::FrameStructStart*>(sizeof(extensions::FrameStructStart));
	extensions::GTAHeader* theHeader= this->readHeader();
	//this->logfile << "Light section header " << theHeader->identifier << " found at " << bufIndex -12 << std::endl;

	this->maxObjectList[LightIdxStruct->frameCount].leGlobeStruct= readBuffer<extensions::LightStruct*>(sizeof(extensions::LightStruct));
	//read "LightIdxStruct->frameCount" as the index of the light object
	this->frameList[LightIdxStruct->frameCount].unknown= 0xfffffffe;	//flag for light object
	theHeader= this->readHeader();
	if(*theHeader == secIDs::EXTENSION)
		bufIndex += theHeader->size;
	else
		bufIndex -= 12;
}

void DffImporter::readCameras()
{
	extensions::GTAHeader* leHeader= readHeader();
	bufIndex += leHeader->size;
	leHeader= readHeader();
	bufIndex += leHeader->size;
}

void DffImporter::readNVC(const UINT& geoIndex)
{
	//this->logfile << "Reading NVC" << std::endl;
	Mesh& theMesh = this->eMeshList[geoIndex].maxMesh->mesh;
	Mtl* multimat = this->eMeshList[geoIndex].material;
	//tick NVC usage checkbox.
	multimat->GetParamBlock(0)->SetValue((ParamID)GTAMultiMaterialConstants::NVC, 0, TRUE);
	//assign unknown value.
	multimat->GetParamBlock(0)->SetValue((ParamID)GTAMultiMaterialConstants::NVCHeader, 0, this->ToString<UINT>(*this->readBuffer<UINT*>(sizeof(UINT)),true).c_str());

	theMesh.setMapSupport(-1, 1);		//activate extra map channel -1 (night vertex colors)
	theMesh.setMapSupport(3, 1);		//activate extra map channel 3 (night vertex alpha)
	theMesh.setNumMapVerts(-1, theMesh.numVerts, 0);
	//theMesh.setNumMapFaces(3, theMesh.numFaces, 0, 0);		//not necessary
	theMesh.setNumMapVerts(3, theMesh.numVerts, 0);
	//theMesh.setNumMapFaces(4, theMesh.numFaces, 0, 0);		//not necessary
	extensions::VertexColors* NVcolors= readBuffer<extensions::VertexColors*>(sizeof(extensions::VertexColors)*theMesh.numVerts);
	for (int ind= 0; ind < theMesh.numVerts; ++ind)
	{
		//set night vertex colors (RGB)
		theMesh.mapVerts(-1)[ind].x= NVcolors[ind].red/255.0f;
		theMesh.mapVerts(-1)[ind].y= NVcolors[ind].blue/255.0f;
		theMesh.mapVerts(-1)[ind].z= NVcolors[ind].green/255.0f;
		//night vertex alpha, only x coordinate used
		theMesh.mapVerts(3)[ind].x= NVcolors[ind].alpha/255.0f;
		theMesh.mapVerts(3)[ind].y= NVcolors[ind].alpha == 255 ? 1.0f: 0.0f;
		theMesh.mapVerts(3)[ind].z= NVcolors[ind].alpha == 255 ? 1.0f: 0.0f;
	}
	//Build night vertex faces
	for (int ind= 0; ind < theMesh.numFaces; ++ind)
	{
		//for (int ind1=3; ind1 <= 4; ++ind1)
		theMesh.mapFaces(-1)[ind].setTVerts(theMesh.faces[ind].getVert(0), theMesh.faces[ind].getVert(1), theMesh.faces[ind].getVert(2));
		theMesh.mapFaces(3)[ind].setTVerts(theMesh.faces[ind].getVert(0), theMesh.faces[ind].getVert(1), theMesh.faces[ind].getVert(2));
	}
}

void DffImporter::buildSceneChar()
{
	//summary: Ignore atomic list entries, every entry in framelist considered a dummy node.
	//Any mesh objects found assigned default names and occupy same position in hierarchy as equivalent bone

	for (int i=0; i < this->frameInfo->frameCount; ++i)
	{
		DummyObject* leDummy= (DummyObject*)dffInterface->CreateInstance(SClass_ID(HELPER_CLASS_ID), Class_ID(DUMMY_CLASS_ID,0));
		this->maxObjectList[i].theNode= dffInterface->CreateObjectNode(leDummy);

		//you'd expect to have only one item in SkinData.

		this->maxObjectList[i].theNode->SetNodeTM(this->dffInterface->GetTime(), this->getBoneTransform(i));
		this->maxObjectList[i].theNode->Scale(0, this->maxObjectList[i].theNode->GetNodeTM(this->dffInterface->GetTime()), Point3(10,10,10), 1, 0, PIV_OBJECT_ONLY, 1);
#ifdef UNICODE
		std::unique_ptr<TCHAR> wString(new TCHAR[strlen(frameNames[i]+1)]);
		mbstowcs_s(nullptr, wString.get(), strlen(frameNames[i])+1, frameNames[i], strlen(frameNames[i]));
		this->maxObjectList[i].theNode->SetName(wString.get());
#else
		maxObjectList[i].theNode->SetName(this->frameNames[i]);
#endif
		maxObjectList[i].theNode->ShowBone(2); //0: Bones are not drawn; 1: Bones are drawn; 2: Only bones are shown
		delete[] frameNames[i];
	}
	delete[] frameNames;

	//Matrix3 temp= this->maxObjectList[0].theNode->GetNodeTM(0);
	//temp.RotateX(HALFPI);
	//this->maxObjectList[0].theNode->SetNodeTM(0, temp);
	//if the keepTM argument in AttachChild is 0, the new transform matrix of the node= original transform matrix * transform matrix of parent
	//create the geometry
	for (int i= 0; i < this->frameInfo->frameCount; ++i)
	{
		if (frameList[i].parent < this->frameInfo->frameCount)
			maxObjectList[frameList[i].parent].theNode->AttachChild(maxObjectList[i].theNode, 1);
	}
	for (int i=0; i < this->leClump->objectCount; ++i)
	{
		INode* GeomNode;
		GeomNode= this->dffInterface->CreateObjectNode(this->eMeshList[this->atomList[i]->geoIndex].maxMesh);
		if (this->frameInfo->frameCount > this->atomList[i]->frameIndex && this->maxObjectList[0].theNode)
		{
			GeomNode->SetNodeTM(0, Matrix3(this->frameList[this->atomList[i]->frameIndex].transMatrix));
			this->maxObjectList[0].theNode->SetNodeTM(0, Matrix3(this->frameList[this->atomList[i]->frameIndex].transMatrix));
			GeomNode->SetMtl(this->eMeshList[this->atomList[i]->geoIndex].material);
			this->addSkinModifier(GeomNode, i);
			this->maxObjectList[0].theNode->Delete(dffInterface->GetTime(), TRUE);
			this->maxObjectList[0].theNode = nullptr;
		}
		else
			GeomNode->SetNodeTM(dffInterface->GetTime(), Matrix3(TRUE));
	}

	delete[] maxObjectList;	maxObjectList= NULL;
	delete[] eMeshList; eMeshList= NULL;
	delete[] atomList; atomList= NULL;

	this->HAnimIDFrameKeys.clear();
	this->HAnimFrameIDKeys.clear();
}
void DffImporter::addHAnimProps(const DWORD& frameIndex)
{
	auto end= this->HAnimDataMappings + this->HAnimFrameIDKeys.size();
	auto iter= std::find_if(this->HAnimDataMappings, end, [&] (const extensions::HAnimPLGItem& theItem)->bool {
		return theItem.boneID == this->HAnimFrameIDKeys[frameIndex]; });

	if (iter >= end)
		return;

	this->maxObjectList[frameIndex].theNode->SetUserPropInt(TEXT("BoneId"), iter->boneID);
	this->maxObjectList[frameIndex].theNode->SetUserPropInt(TEXT("BoneType"), iter->boneType);
	this->maxObjectList[frameIndex].theNode->SetUserPropInt(TEXT("BoneIndex"), iter->boneNo);
	this->maxObjectList[frameIndex].theNode->SetUserPropString(TEXT("FrameName"), this->maxObjectList[frameIndex].theNode->NodeName());
}
void DffImporter::addSkinModifier(INode* leNode, const UINT& geoIndex)
{
	::Modifier* PedSkin= (Modifier*)this->dffInterface->CreateInstance(SClass_ID(OSM_CLASS_ID), SKIN_CLASSID);
	::ISkinImportData* laPeau= (ISkinImportData*)PedSkin->GetInterface(I_SKINIMPORTDATA);
	::ISkin* theISkin= (::ISkin*)PedSkin->GetInterface(I_SKIN);
	::ISkin2* theISkin2= (::ISkin2*)PedSkin->GetInterface(I_SKIN2);
	::GetCOREInterface12()->AddModifier(*leNode, *PedSkin);
	ObjectState theObj= leNode->EvalWorldState(0);

	Tab<::INode*> boneNodes;
	Tab<float> vtxWeights;
	boneNodes.SetCount(4, 1);
	vtxWeights.SetCount(4, 1);
	//select the node
	theHold.Begin();
	dffInterface->SelectNode(leNode, 0);
	theHold.Accept(TEXT("Selection"));
	dffInterface->SetCommandPanelTaskMode(TASK_MODE_MODIFY);
	::ExecuteMAXScriptScript(TEXT("leSkin= modPanel.GetCurrentObject()"));
	for (int i= 0; i < this->HAnimFrameIDKeys.size(); ++i)
	{
		auto iter= std::find_if(this->HAnimDataMappings, this->HAnimDataMappings+this->HAnimFrameIDKeys.size(), 
			[&](const extensions::HAnimPLGItem& theItem)->bool {
				return (theItem.boneNo == i); });

		std::map<int, DWORD>::iterator Itr= this->HAnimIDFrameKeys.find(iter->boneID);
		this->maxObjectList[Itr->second].theNode->SetUserPropInt(TEXT("BoneId"), iter->boneID);
		this->maxObjectList[Itr->second].theNode->SetUserPropInt(TEXT("BoneType"), iter->boneType);
		this->maxObjectList[Itr->second].theNode->SetUserPropInt(TEXT("BoneIndex"), iter->boneNo);
		this->maxObjectList[Itr->second].theNode->SetUserPropString(TEXT("FrameName"), this->maxObjectList[Itr->second].theNode->NodeName());
		this->maxObjectList[Itr->second].theNode->SetUserPropInt(TEXT("SPunknown"), this->SkinData.skStart->spIndicesCount);
		laPeau->AddBoneEx(this->maxObjectList[Itr->second].theNode, false);
		for (int jj= 1; jj <= 2; ++jj)
		{
			std::tstring temp= TEXT("skinops.SetStartPoint leSkin ") + ToString<int>(i+1) + TEXT(" [0,0,0]");
			::ExecuteMAXScriptScript(const_cast<TCHAR*>(temp.c_str()));	
			temp= TEXT("skinops.SetEndPoint leSkin ") + ToString<int>(i+1) + TEXT(" [0,0,0]");
			::ExecuteMAXScriptScript(const_cast<TCHAR*>(temp.c_str()));
			temp= TEXT("skinops.SetInnerRadius leSkin ") + ToString<int>(i+1) + TEXT(" ") + ToString<int>(jj) + TEXT(" 0");
			::ExecuteMAXScriptScript(const_cast<TCHAR*>(temp.c_str()));
			temp= TEXT("skinops.SetOuterRadius leSkin ") + ToString<int>(i+1) + TEXT(" ") + ToString<int>(jj) + TEXT(" 0");
			::ExecuteMAXScriptScript(const_cast<TCHAR*>(temp.c_str()));
		}
	}
	//equivalent to an update for nodes. Required otherwise vertex weights won't be added.
	leNode->EvalWorldState(dffInterface->GetTime());
	for (int i= 0; i < this->eMeshList[geoIndex].maxMesh->GetMesh().numVerts; ++i)
	{
		for (int jk= 0; jk < 4; ++jk)
		{		
			boneNodes[jk]= theISkin->GetBone(this->SkinData.skBVIndices[i].boneVertex[jk]);
			vtxWeights[jk]= this->SkinData.skVWeights[i].weight[jk];
		}
		laPeau->AddWeights(leNode, i, boneNodes, vtxWeights);
	}
}


inline void DffImporter::setTextureMap(::BitmapTex* theTex)
{
	
	std::tstring sExt[]= {TEXT(".TGA"), TEXT(".DDS"), TEXT(".PNG"), TEXT(".GIF"), TEXT(".JPG"), TEXT(".BMP")};
	TCHAR longFileName[MAX_PATH];
	std::tstring shortFileName;
	for (int i=0; i<6; ++i)
	{
		shortFileName= theTex->GetName().data() + sExt[i];
		if (::BMMGetFullFilename(shortFileName.c_str(), longFileName))
		{	
			theTex->SetMapName(longFileName, false);
			return;
		}
	}
	shortFileName= theTex->GetName().data()+sExt[0];
	theTex->SetMapName(shortFileName.c_str());
}

//The bone number is an index into the inverse bone matrix array. Implies non-root bone info is important
void DffImporter::readHAnimPLG(const UINT& frameIndex)	
{
	extensions::HAnimPLGStart* temp= this->readBuffer<extensions::HAnimPLGStart*>(sizeof(extensions::HAnimPLGStart));
	//no need to recalculate transforms from IBM.
	//this->HAnimList.push_back(temp);		//not using this
	this->HAnimIDFrameKeys.insert(std::make_pair(temp->boneID, frameIndex));
	this->HAnimFrameIDKeys.insert(std::make_pair(frameIndex, temp->boneID));

	if (! temp->boneCount)
		return;

	//skip 8 bytes of unknown data
	bufIndex += 8;
	this->HAnimDataMappings= readBuffer<extensions::HAnimPLGItem*>(sizeof(extensions::HAnimPLGItem)*temp->boneCount);
}

void DffImporter::readSkin(const UINT& gIndex, const extensions::GeoParams* meshParams, const extensions::GTAHeader* skinHeader)
{
	this->SkinData.skStart= readBuffer<extensions::SkinPlgStart*>(sizeof(extensions::SkinPlgStart));
	//calculate the number of bone vertex and vertex weight indices (the total is this figure x 2)
	//let this number equal k
	//since each BVI set takes up 4 bytes and each VW set 16 bytes,
	// 4k + 16k + sizeof(extensions::SkinPlgStart) + theSkin.skStart->spIndicesCount + theSkin.skStart->boneCount*sizeof(extensions::inverseBoneMatrix) = skinHeader->size;
	//Discovery: it seems that index set count in RW analyze is incorrect. The count is actually equal to the number of mesh vertices.

	this->SkinData.specialIndices= readBuffer<BYTE*>(SkinData.skStart->spIndicesCount);
	this->SkinData.skBVIndices= readBuffer<extensions::BoneVertexIndices*>(sizeof(extensions::BoneVertexIndices)*meshParams->vertexCount);
	this->SkinData.skVWeights= readBuffer<extensions::VertexWeights*>(sizeof(extensions::VertexWeights)*meshParams->vertexCount);
	
	if (this->leClump->structHeader.fileVersion == GTA_SA)
	{
		this->SkinData.ibm= readBuffer<extensions::InverseBoneMatrix*>(sizeof(extensions::InverseBoneMatrix)*SkinData.skStart->boneCount);
	}
	else
	{
		this->SkinData.ibmEx= readBuffer<extensions::InverseBoneMatrixEx*>(sizeof(extensions::InverseBoneMatrixEx)*SkinData.skStart->boneCount);
	}
	//skip 3 unknown floats in gta SA peds.
	if(meshParams->structHeader.fileVersion == GTA_SA)
		bufIndex += 12;
}


inline Matrix3 DffImporter::getBoneTransform(const DWORD& frameIndex)
{
	//check if there is a bone at this frame index. There shouldn't be one if it's the first frame
	std::map<DWORD,int>::iterator frameItr= this->HAnimFrameIDKeys.find(frameIndex);

	if (frameItr == this->HAnimFrameIDKeys.end())
		return Matrix3(TRUE);

	//get IBM array index= the bone number
	extensions::HAnimPLGItem* theItem= std::find_if(this->HAnimDataMappings, this->HAnimDataMappings+this->HAnimFrameIDKeys.size(), [&](const extensions::HAnimPLGItem& leItem)->bool {
		return leItem.boneID == frameItr->second; });
	Matrix3 ipMatrix;
	if (this->leClump->structHeader.fileVersion == GTA_SA)
	{
		ipMatrix.SetRow(0, this->SkinData.ibm[theItem->boneNo].matrixRow1);
		ipMatrix.SetRow(1, this->SkinData.ibm[theItem->boneNo].matrixRow2);
		ipMatrix.SetRow(2, this->SkinData.ibm[theItem->boneNo].matrixRow3);
		ipMatrix.SetRow(3, this->SkinData.ibm[theItem->boneNo].matrixRow4);
	}
	else
	{
		ipMatrix.SetRow(0, this->SkinData.ibmEx[theItem->boneNo].matrixRow1);
		ipMatrix.SetRow(1, this->SkinData.ibmEx[theItem->boneNo].matrixRow2);
		ipMatrix.SetRow(2, this->SkinData.ibmEx[theItem->boneNo].matrixRow3);
		ipMatrix.SetRow(3, this->SkinData.ibmEx[theItem->boneNo].matrixRow4);
	}
#ifdef LOGFILE
	this->logFile << "inverted matrix" << std::endl << frameIndex << std::endl << ipMatrix.GetRow(0)[0] << ", " << ipMatrix.GetRow(0)[1] << ", " << ipMatrix.GetRow(0)[2] << std::endl
				  << ipMatrix.GetRow(1)[0] << ", " << ipMatrix.GetRow(1)[1] << ", " << ipMatrix.GetRow(1)[2] << std::endl
				  << ipMatrix.GetRow(2)[0] << ", " << ipMatrix.GetRow(2)[1] << ", " << ipMatrix.GetRow(2)[2] << std::endl
				  << ipMatrix.GetRow(3)[0] << ", " << ipMatrix.GetRow(3)[1] << ", " << ipMatrix.GetRow(3)[2] << std::endl;
#endif
	ipMatrix.Invert();

	return ipMatrix;
}

#include "DffExporter.h"
#include "IndependentFunctions.h"
#include "SceneEnumProc.h"
#include "miniball.h"
#include "resource.h"

extern INT_PTR CALLBACK toolDlgProc(HWND, UINT, WPARAM, LPARAM);
extern DWORD WINAPI progressFn(LPVOID);

int DffExporter::DoExport (const MCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options)
{
#ifdef LOGFILE
	this->logFile.open("C:\\Users\\Packard-Bell\\Desktop\\ExportLog.txt", std::ios::out | std::ios::trunc | std::ios::beg);
#endif
    //TODO("Perform the export");
	this->iInterface= i;
	this->eiInterface= ei;
	this->expData[(UINT)enms::expDatIDs::expSelected]= options & SCENE_EXPORT_SELECTED;
	//set configuration file name
	this->fNameStrings.CFGfile= ::IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_PLUGCFG_DIR);
	if (this->fNameStrings.CFGfile[this->fNameStrings.CFGfile.length()-1] != TEXT('\\'))
		this->fNameStrings.CFGfile += TEXT('\\');

	this->fNameStrings.CFGfile += extensions::GetRscString(IDS_CONFIGFILE);
	
	if (suppressPrompts)
	{
		MessageBox(this->iInterface->GetMAXHWnd(), TEXT("Noprompt export not supported"), TEXT("Information"), MB_OK);
	}
	else
	{
		this->fNameStrings.filename= name;	//store the filename
		//this->enumProc= ::extensions::SceneEnumProc(iInterface, eiInterface, options); possible later use
		if(DialogBoxParam(hDllInstance, MAKEINTRESOURCE(IDD_CONTAINER_DIALOG), i->GetMAXHWnd(), toolDlgProc, (LPARAM)this) != IDOK)
			return IMPEXP_CANCEL;

		iInterface->SetQuietMode(TRUE);
		iInterface->DisableSceneRedraw();
		this->WriteFile();
		iInterface->SetQuietMode(FALSE);
		iInterface->EnableSceneRedraw();
		iInterface->ForceCompleteRedraw();
	}
	leStream.close();
#ifdef LOGFILE
	this->logFile.close();
#endif
    return IMPEXP_SUCCESS;
}

/*
inline void DffExporter::DateTimeStamp()
{
	leStream.WriteHeader(enms::secIDs::ZMOD_LOCK, this->fileVersion);
	::SYSTEMTIME timeStruct;
	::GetSystemTime(&timeStruct);
	leStream << TEXT("Exported ") << timeStruct.wMonth << TEXT("/") << timeStruct.wDay << TEXT("/") << timeStruct.wYear << TEXT(" at") << timeStruct.wHour << TEXT(":")
			 << timeStruct.wMinute << TEXT(" with DffExporter (c) 2011-2012 by Seggaeman");
	leStream.FlushSectionSize();
}
*/

void DffExporter::VExpMultiple()
{
	this->buildClumpList();
	//get the path
	extensions::GetShortFilename(this->fNameStrings.filename, &this->fNameStrings.filename);
	std::size_t length= this->fNameStrings.filename.length();

	for (auto itr= this->exportClumps.begin(); itr < this->exportClumps.end(); ++itr)
	{
		this->animatedMats.clear();
		this->fNameStrings.filename= this->fNameStrings.filename.substr(0, length);

		this->fNameStrings.filename +=  itr->orderedFrames[0]->GetName() + std::tstring(TEXT(".dff"));
		TCHAR longColName[MAX_PATH];

		//search collision
		this->fNameStrings.colString = itr->orderedFrames[0]->GetName() + std::tstring(TEXT(".col"));

		if (this->expData[enms::expDatIDs::colAutoseek] && ::BMMGetFullFilename(fNameStrings.colString.c_str(), longColName))
			this->fNameStrings.colString= longColName;
		else
			this->fNameStrings.colString= TEXT("");

		leStream.open(this->fNameStrings.filename, std::ios::out | std::ios::in | std::ios::beg | std::ios::binary | std::ios::trunc);
		WriteClump(itr);
		this->UVAnimUpdate();
	}
}

void DffExporter::UVAnimUpdate()
{
	if (! this->animatedMats.size())
	{
		leStream.close();
		return;		//if there are no UV anims job's done.
	}

	extensions::GetShortFilename(this->fNameStrings.filename, &this->fNameStrings.UVanimFile);
	this->fNameStrings.UVanimFile += TEXT("UVanim.anime");

	this->UVAnimStream.open(this->fNameStrings.UVanimFile, std::ios::out | std::ios::in | std::ios::binary | std::ios::trunc);

	this->WriteUVAnimDic();
	this->UpdateFile();
	this->UVAnimStream.close();
}
/*
// Utility function to give us the TriObject
TriObject* DffExporter::GetTriObjectFromNode(INode *node, TimeValue t,  int &deleteIt)
{
	deleteIt = FALSE;    
	Object *obj = node->EvalWorldState(t).obj;    
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{        
		TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));                
		// Note that the TriObject should only be deleted        
		// if the pointer to it is not equal to the object        
		// pointer that called ConvertToType()        
		if (obj != tri) 
			deleteIt = TRUE;             
		return tri;        
	}        
	else 
	{             
		return NULL;        
	}
}
*/
void DffExporter::SplitBySmoothGroup(Mesh& triMesh)
{
	auto leLambda= [](const DWORD& SmGroup)-> DWORD
	{
		for (int i= 0; i <= 31; ++i)
		{
			if (DWORD SmoothingID= SmGroup & (DWORD)( 0x01 << i ))
				return SmoothingID;
		}
		return 0;
	};

	::MeshDelta leDelta;
	std::map<DWORD, ::BitArray> meshSplits;
	//Categorize faces by smooth group
	//bitarray does not size itself automatically
	for (int i=0; i < triMesh.numFaces; ++i)
	{
		DWORD SmID= leLambda(triMesh.faces[i].smGroup);
		if (!meshSplits.count(SmID))
			meshSplits.insert(std::make_pair(SmID, ::BitArray(triMesh.numFaces)));
		meshSplits[SmID].Set(i);
	}

	for (std::map<DWORD, ::BitArray>::iterator itr= meshSplits.begin(); itr != meshSplits.end(); ++itr)
	{
		leDelta.Detach(triMesh, NULL, itr->second, TRUE, TRUE, TRUE);
		leDelta.Apply(triMesh);	//must "Apply" after every modification else it will be discarded
	}

	//break up faces assigned SmoothingID/SmoothGroup 0
	if (meshSplits.count(0))
	{
		::BitArray vtxSet(triMesh.numVerts);
		//Get the vertices
		for (int i= 0; i < meshSplits[0].GetSize(); ++i)
		{
			if (meshSplits[0][i])
			{
				for (int j= 0; j < 3; ++j)
					vtxSet.Set(triMesh.faces[i].v[j]);
			}
		}
		triMesh.BreakVerts(vtxSet);
	}
}

//identifies vertices correctly but splitting all faces referencing does not always give correspondence between vert and map vert count. Must rebuild mesh using texture data
/*
void DffExporter::BreakVertices(Mesh& theMesh)
{
	std::map<DWORD, extensions::vertexInfo> vertexMap;
	//std::unordered_set<DWORD> flaggedVertices;
	::BitArray flaggedVertices(theMesh.numVerts);
	std::vector<int> supportedMaps;

	for (int idx= -2; idx < 4; ++idx)
		if (theMesh.mapSupport(idx))
			supportedMaps.push_back(idx);

	for (int i= 0; i < theMesh.numFaces; ++i)
	{
		for (int ind= 0; ind < 3; ++ind)
		{
			vertexMap[theMesh.faces[i].v[ind]].faceRefs.insert(extensions::faceRef(i, ind));

			for (std::vector<int>::iterator itr= supportedMaps.begin(); itr < supportedMaps.end(); ++itr)
			{
				vertexMap[theMesh.faces[i].v[ind]].mapVertices[*itr].insert(theMesh.mapFaces(*itr)[i].t[ind]);
				if (vertexMap[theMesh.faces[i].v[ind]].mapVertices[*itr].size() > 1)
					//flaggedVertices.insert(theMesh.faces[i].v[ind]);
					flaggedVertices.Set(theMesh.faces[i].v[ind], 1);
			}
		}
	}
	MessageBox(NULL, extensions::ToString<int>(flaggedVertices.NumberSet()).c_str(), "number set", MB_OK);
	theMesh.BreakVerts(flaggedVertices);
}
*/
inline bool DffExporter::IsChannelFlagged(const int& channel)
{
	switch (channel)
	{
		case -2:
			return this->expData[(UINT)enms::expDatIDs::dayVColors];
		break;

		case -1:
			return this->expData[(UINT)enms::expDatIDs::nightVColors];
		break;

		case 0:
			return this->expData[(UINT)enms::expDatIDs::dayVColors];
		break;

		case 1:
			return this->expData[(UINT)enms::expDatIDs::UVmap1];
		break;

		case 2:
			return this->expData[(UINT)enms::expDatIDs::UVmap2];
		break;

		case 3:
			return this->expData[(UINT)enms::expDatIDs::nightVColors];
		break;

		default:
			return false;
	}
	return false;
}

void DffExporter::buildClumpList(std::unordered_set<INode*>* nodeSet)
{
	for (int i= 0; i < iInterface->GetRootNode()->NumChildren(); ++i)
	{
		INode* topNode= iInterface->GetRootNode()->GetChildNode(i);
		
		if ((nodeSet && nodeSet->find(topNode) == nodeSet->end())		//used in ped export, checks if the node is a member of the set of bones passed as argument
			|| (this->expData[(UINT)enms::expDatIDs::expSelected] && ! topNode->Selected())
			|| (this->expData[(UINT)enms::expDatIDs::incFrozen] == false && topNode->IsFrozen())
			|| (this->expData[(UINT)enms::expDatIDs::incHidden] == false && topNode->IsHidden()))			//verify state of selected, frozen and hidden flags
			continue;

		extensions::clumpNodeList temp;
		::ObjectState os= topNode->EvalWorldState(this->iInterface->GetTime(), TRUE);

		switch (os.obj->SuperClassID())
		{
			case GEOMOBJECT_CLASS_ID:
				//since any kind of object can be used as bone don't add it to the list of geometry when exporting peds
				if (os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) && ! expData[(UINT)enms::expDatIDs::expPreset])	//check if it can be cast to a TriObject
					temp.orderedGeometry.insert(temp.frameNodes.size());		//insert the frame index

				temp.frameNodes.insert(std::make_pair(topNode, temp.frameNodes.size()));
				temp.orderedFrames.insert(std::make_pair(temp.orderedFrames.size(), topNode));
			break;

			case LIGHT_CLASS_ID:
				if (this->fileVersion == enms::gameConstants::GTA_IIIA || this->fileVersion == enms::gameConstants::GTA_IIIB || this->fileVersion == enms::gameConstants::GTA_IIIC
					||(!this->expData[(UINT)enms::expDatIDs::lights]))
					break;
				if (! expData[(UINT)enms::expDatIDs::expPreset])
					temp.orderedLights.insert(temp.frameNodes.size());
				temp.frameNodes.insert(std::make_pair(topNode, temp.frameNodes.size()));
				temp.orderedFrames.insert(std::make_pair(temp.orderedFrames.size(), topNode));
			break;

			default:
				temp.frameNodes.insert(std::make_pair(topNode, temp.frameNodes.size()));
				temp.orderedFrames.insert(std::make_pair(temp.orderedFrames.size(), topNode));
			break;
		}

		FilterClumpNodes(topNode, temp);
		this->exportClumps.push_back(temp);
	}
	if(! this->exportClumps.size())
	{
		MessageBox(this->iInterface->GetMAXHWnd(), TEXT("No clumps found"), TEXT("Caption"), MB_OK);
	}
}


void DffExporter::FilterClumpNodes(INode*& leNode, extensions::clumpNodeList& theList)
{
	for (int i=0; i < leNode->NumberOfChildren(); ++i)
	{
		INode* theChild= leNode->GetChildNode(i);
		if ((this->expData[(UINT)enms::expDatIDs::expSelected] && ! theChild->Selected())
			|| (this->expData[(UINT)enms::expDatIDs::incFrozen] == false && theChild->IsFrozen())
			|| (this->expData[(UINT)enms::expDatIDs::incHidden] == false && theChild->IsHidden()))			//verify state of selected, frozen and hidden flags
			continue;

		//theList.allNodes.insert(theChild);
		::ObjectState os= theChild->EvalWorldState(this->iInterface->GetTime(), TRUE);

		switch(os.obj->SuperClassID())
		{
			case GEOMOBJECT_CLASS_ID:
				if (os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0)))
					theList.orderedGeometry.insert(theList.frameNodes.size());

				theList.frameNodes.insert(std::make_pair(theChild, theList.frameNodes.size()));
				theList.orderedFrames.insert(std::make_pair(theList.orderedFrames.size(), theChild));
			break;

			case LIGHT_CLASS_ID:
				if (this->fileVersion == enms::gameConstants::GTA_IIIA || this->fileVersion == enms::gameConstants::GTA_IIIB || this->fileVersion == enms::gameConstants::GTA_IIIC
					|| (!this->expData[(UINT)enms::expDatIDs::lights]))
					break;
				theList.orderedLights.insert(theList.frameNodes.size());
				theList.frameNodes.insert(std::make_pair(theChild, theList.frameNodes.size()));
				theList.orderedFrames.insert(std::make_pair(theList.orderedFrames.size(), theChild));
			break;

			default:
				theList.frameNodes.insert(std::make_pair(theChild, theList.frameNodes.size()));
				theList.orderedFrames.insert(std::make_pair(theList.orderedFrames.size(), theChild));
			break;
		}
		FilterClumpNodes(theChild, theList);
	}
}

inline void DffExporter::WriteFile()
{
	//in case a scene file has been specified
	INodeTab theNodes;
	if (this->expData[enms::expDatIDs::source])
	{
		//no need to use callback since existing nodes are deleted.
		//extensions::CNECCallback leCallback;
		//SceneEventNamespace::CallbackKey theKey= ::GetISceneEventManager()->RegisterCallback(&leCallback);
		//this->iInterface->MergeFromFile(this->fNameStrings.MAXfile.c_str(), TRUE, FALSE, FALSE, MERGE_DUPS_RENAME, NULL, MERGE_DUP_MTL_RENAME, MERGE_REPARENT_ALWAYS);
		::GetCOREInterface8()->LoadFromFile(this->fNameStrings.MAXfile.c_str(), 
								::Interface8::LoadFromFileFlags::kRefreshViewports | ::Interface8::LoadFromFileFlags::kSetCurrentFilePath | ::Interface8::LoadFromFileFlags::kSuppressPrompts);
		extensions::SceneEnumProc leProc(this->eiInterface, &theNodes);
		//::GetISceneEventManager()->UnRegisterCallback(theKey);
	}

	switch (this->fCreation)
	{	
		case enms::fileCreation::app: 
			leStream.open(this->fNameStrings.filename, std::ios::out | std::ios::in | std::ios::ate | std::ios::binary);
			break;
		case enms::fileCreation::cnew:
			leStream.open(this->fNameStrings.filename, std::ios::out | std::ios::in | std::ios::beg | std::ios::binary | std::ios::trunc);
			break;
		default:
			if (this->expData[enms::expDatIDs::expPreset])
				this->PExpMultiple();
			else
				this->VExpMultiple();
			return;
	}

	if (this->expData[enms::expDatIDs::expPreset])
	{
		if (this->expData[enms::expDatIDs::source])
			this->PExpSingleMAX();
		else
			this->GenPedDFF();
	}
	else
	{
		this->buildClumpList();
		for (auto itr= this->exportClumps.begin(); itr < this->exportClumps.end(); ++itr)
			WriteClump(itr);
		this->UVAnimUpdate();
	}

	//delete nodes loaded from the external file
	::GetCOREInterface9()->DeleteNodes(theNodes);
	iInterface->GetMaterialLibrary().DeleteAll();
}

/*
inline void DffExporter::LockFile()	//uses Zmod1 lock
{
	leStream.GenericWrite<UINT>(secIDs::ZMOD_LOCK);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<unsigned __int64>(0x352694931c153445LL);
	leStream.FlushSectionSize();
}
*/

void DffExporter::WriteClump(const std::vector<extensions::clumpNodeList>::iterator& itr)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": Writing clump" << std::endl;
#endif
	//atomic section shift
	DWORD shift = 0;
	//write clump header
	leStream.GenericWrite<UINT>(enms::secIDs::CLUMP_ID);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write clump struct
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<UINT>((*itr).orderedGeometry.size());
	//GTA 3 does not use lights and cameras
	if (this->fileVersion != enms::gameConstants::GTA_IIIA && this->fileVersion != enms::gameConstants::GTA_IIIB && this->fileVersion != enms::gameConstants::GTA_IIIC)
	{
		leStream.GenericWrite<UINT>((*itr).orderedLights.size());
		leStream.GenericWrite<UINT>(0);			//cameras not supported
	}
	leStream.FlushSectionSize();

	if (this->expData[enms::expDatIDs::incHAnimPLG])
	{
		shift = 1;
		this->WritePedFrames(itr, false);
	}
	else
	{
		this->WriteFrameList(itr);
	}
	this->WriteGeometryList(itr);
	this->WriteAtomic(itr,shift);
	this->WriteLights(itr);
	//write collision, if specified
	leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
	leStream.GenericWrite<UINT>(0,true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	this->WriteCollision();
	leStream.FlushSectionSize();
	leStream.FlushSectionSize();
}

void DffExporter::WriteAtomic(const std::vector<extensions::clumpNodeList>::iterator& itr, const DWORD& shift)
{
	for(std::set<DWORD>::iterator nodeItr= itr->orderedGeometry.begin(); nodeItr != itr->orderedGeometry.end(); ++nodeItr)
	{
#ifdef LOGFILE
		logFile << clumpIdx << ": Writing atomic" << std::endl;
#endif
		//write atomic section header
		leStream.GenericWrite<UINT>(enms::secIDs::ATOMIC);
		leStream.GenericWrite<UINT>(0,true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		//write atomic struct
		leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
		leStream.GenericWrite<UINT>(0,true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		//get the frame index
		//std::vector<INode*>::iterator frameItr= std::find(itr->frameNodes.begin(), itr->frameNodes.end(), *nodeItr);
		//write the frame index
		leStream.GenericWrite<UINT>((*nodeItr) + shift);
		//write the geometry index
		leStream.GenericWrite<UINT>(std::distance(itr->orderedGeometry.begin(), nodeItr));
		//write unknown 1 (5)
		leStream.GenericWrite<UINT>(5);
		leStream.GenericWrite<UINT>(0);
		//write struct size
		leStream.FlushSectionSize();
		//write extension
		leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		if (itr->orderedFrames[*nodeItr]->GetNodeLong())
		{
			this->WriteAtomicRTR(itr->orderedFrames[*nodeItr]);
			this->WriteAtomicMatEffects(itr->orderedFrames[*nodeItr]);
		}
		this->WritePipeline();
		leStream.FlushSectionSize();
		leStream.FlushSectionSize();
	}
}

inline void DffExporter::WriteAtomicMatEffects(INode* leNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": Writing atomic material effects" << std::endl;
#endif
	leStream.GenericWrite<UINT>(enms::secIDs::MATERIAL_EFFECTS_PLG);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<UINT>(1);
	leStream.FlushSectionSize();
}
inline void DffExporter::WriteAtomicRTR(INode* leNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": Writing atomic right to render" << std::endl;
#endif
	leStream.GenericWrite<UINT>(enms::secIDs::RIGHT_TO_RENDER);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<UINT>(0x120);
	leStream.GenericWrite<UINT>(0);
	leStream.FlushSectionSize();
}

inline void DffExporter::WritePipeline()
{
	if (this->pipeOptions)
	{
#ifdef LOGFILE
		logFile << clumpIdx << ": Writing pipeline" << std::endl;
#endif
		leStream.GenericWrite<UINT>(enms::secIDs::PIPELINE);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		leStream.GenericWrite<DWORD>(this->pipeOptions);
		leStream.FlushSectionSize();
	}
}

inline void DffExporter::WriteLights(const std::vector<extensions::clumpNodeList>::iterator& itr)		//export lights
{
	for (std::set<DWORD>::iterator lightItr= itr->orderedLights.begin(); lightItr != itr->orderedLights.end(); ++lightItr)
	{
#ifdef LOGFILE
		logFile << clumpIdx << ": Writing lights" << std::endl;
#endif
		//write struct containing the light object's frame index
		leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		leStream.GenericWrite<DWORD>(*lightItr);
		leStream.FlushSectionSize();

		//write light section
		leStream.GenericWrite<UINT>(enms::secIDs::LIGHT);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		
		//write light struct
		leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
		leStream.GenericWrite<UINT>(0,true);
		leStream.GenericWrite<UINT>(this->fileVersion);

		//acquire the light
		BOOL deleteIt;	int index;
		::GenLight* theLight;
		UINT lightTypes[]= {OMNI_LIGHT_CLASS_ID, SPOT_LIGHT_CLASS_ID, DIR_LIGHT_CLASS_ID, FSPOT_LIGHT_CLASS_ID, TDIR_LIGHT_CLASS_ID};

		for (index= 0; index < 5; ++index)
		{
			theLight= extensions::GetObjectFromNode<GenLight>(itr->orderedFrames[*lightItr], iInterface->GetTime(), lightTypes[index], deleteIt);
			if (theLight)
				break;
		}

		//if acquired successfully write associated parameters (else write defaults)
		if (theLight)
		{
			leStream.GenericWrite<float>(theLight->GetAtten(iInterface->GetTime(), ATTEN_END));
			leStream.GenericWrite<float>(theLight->GetRGBColor(iInterface->GetTime()).x);
			leStream.GenericWrite<float>(theLight->GetRGBColor(iInterface->GetTime()).y);
			leStream.GenericWrite<float>(theLight->GetRGBColor(iInterface->GetTime()).z);
			leStream.GenericWrite<float>(0.0f);	//parameter is not fallsize //leStream.GenericWrite<float>(theLight->GetFallsize(iInterface->GetTime()));
			leStream.GenericWrite<USHORT>(3);
			if (index)
				leStream.GenericWrite<USHORT>(0x81);
			else
				leStream.GenericWrite<USHORT>(0x80);	//if it's an omnilight index will be zero
		}
		else	//write defaults
		{
			for (index= 0; index <5; ++index)
				leStream.GenericWrite<float>(0.0f);
			leStream.GenericWrite<USHORT>(3);
			leStream.GenericWrite<USHORT>(0x81);
		}
		
		if(deleteIt)
			theLight->DeleteThis();

		//write light struct section sizes
		leStream.FlushSectionSize();

		//write empty extension
		leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		leStream.FlushSectionSize();
		//section size of light container
		leStream.FlushSectionSize();
	}
}

void DffExporter::WriteFrameList(const std::vector<extensions::clumpNodeList>::iterator& itr)
{
	//Write frame header
	leStream.GenericWrite<UINT>(enms::secIDs::FRAME_LIST);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write frame struct
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write frame count
	leStream.GenericWrite<UINT>(itr->frameNodes.size());

	for(std::map<DWORD, INode*>::iterator frameItr= itr->orderedFrames.begin(); frameItr != itr->orderedFrames.end(); ++ frameItr)
	{
#ifdef LOGFILE
			logFile << clumpIdx << ": Writing frames" << std::endl;
#endif
		::Matrix3 parentMatrix= (frameItr->second)->GetParentNode()->GetNodeTM(iInterface->GetTime());
		parentMatrix.Invert();
		::Matrix3 exportTM= (frameItr->second)->GetNodeTM(0)*parentMatrix;
		exportTM.Orthogonalize();
		exportTM.NoScale();
		std::map<INode*, DWORD>::iterator parentItr= itr->frameNodes.find(frameItr->second->GetParentNode());

		//if root node write 0xffffffff for parent and an integer value of 131075
		if (parentItr == itr->frameNodes.end())
		{
			if (expData[enms::expDatIDs::forceOrigin])
				exportTM.SetRow(3, Point3(0,0,0));

			//write rotation and translation
			for (int i= 0; i < 4; ++i)
				for (int j= 0; j< 3; ++j)
					leStream.GenericWrite<float>(exportTM.GetRow(i)[j]);
			leStream.GenericWrite<UINT>(0xffffffff);
			leStream.GenericWrite<INT>(131075);
		}
		//else write index of parent and 3
		else
		{
			//write rotation and translation
			for (int i= 0; i < 4; ++i)
				for (int j= 0; j< 3; ++j)
					leStream.GenericWrite<float>(exportTM.GetRow(i)[j]);
			leStream.GenericWrite<DWORD>(parentItr->second);
			leStream.GenericWrite<INT>(3);
		}
	}
	//write struct size
	leStream.FlushSectionSize();

	//write extension sections and frame names
	for(std::map<DWORD, INode*>::iterator frameItr= itr->orderedFrames.begin(); frameItr != itr->orderedFrames.end(); ++ frameItr)
	{
#ifdef LOGFILE
			logFile << clumpIdx << ": Writing frame extension" << std::endl;
#endif
		//write extension
		leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		//write frame section
		leStream.GenericWrite<UINT>(enms::secIDs::FRAME);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		//write frame data
		leStream.writeANSIString((frameItr->second)->GetName());
		//write frame and extension size
		leStream.FlushSectionSize();
		leStream.FlushSectionSize();
	}
	//write framelist section size
	leStream.FlushSectionSize();
}
void DffExporter::WriteGeometryList(const std::vector<extensions::clumpNodeList>::iterator& itr)
{
	//use MeshDelta class to detach faces if required
	//write geometry list header
	leStream.GenericWrite<UINT>(enms::secIDs::GEOMETRY_LIST);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write struct containing geometry count
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<UINT>(itr->orderedGeometry.size());
	leStream.FlushSectionSize();
	this->progressPercent= 0;
	this->iInterface->ProgressStart(TEXT("Exporting geometry"), TRUE, ::progressFn, this);
	for (std::set<DWORD>::iterator gNodeItr= itr->orderedGeometry.begin(); gNodeItr != itr->orderedGeometry.end(); ++gNodeItr)
	{
#ifdef LOGFILE
			logFile << clumpIdx << ": Writing geometry" << std::endl;
#endif
		this->progressPercent= 100.0f*(float)std::distance(itr->orderedGeometry.begin(), gNodeItr)/(float)itr->orderedGeometry.size();
		progressFn(this);
		this->ProcessGeometry(itr->orderedFrames[*gNodeItr]);
	}
	iInterface->ProgressEnd();
	leStream.FlushSectionSize();
}

void DffExporter::RebuildMesh(::Mesh& triMesh)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": Rebuilding mesh" << std::endl;
#endif
	int priorityChannel= 50; //the channel with the highest number of mapvertices, and on which the rebuilt mesh will be based.
	std::unordered_set<int> sChannels;	//any other channels besides the one with the highest map vertex count
	//If none of the map channels have a vertex count higher than the mesh vertex count, there is no need to rebuild the mesh. 
	//priorityChannel initialized to 50 for verifying this possibility 
	//iterate over the set of possible channels: 1 (UV map 1) and 2 (UV map 2)
	//Face indices apparently not important for -1 (night vertex colors), 0 (day vertex colors), -2 (day alpha) and 3 (night alpha).Use only vertex info

	this->condensedIDs.clear();			//clear material ID set

	//check for presence of specified normals
	for (int i= 1; i <= 2; ++i)
	{
		if (this->IsChannelFlagged(i) && triMesh.mapSupport(i))
		{
			sChannels.insert(i);
			if (priorityChannel == 50)
			{
				priorityChannel = i;
			}
			else
				if (triMesh.getNumMapVerts(i) > triMesh.getNumMapVerts(priorityChannel))
					priorityChannel = i;
		}
		else
			triMesh.setMapSupport(i, FALSE);
	}


	if (priorityChannel == 50)
	{
		triMesh.buildNormals();
		::MeshNormalSpec* nSpec= triMesh.GetSpecifiedNormals();
		if (nSpec && nSpec->GetFlag(MESH_NORMAL_NORMALS_BUILT))
		{
			//MessageBox(NULL, "You are here 1", "Caption", MB_OK);
			MeshNormalSpec* nSpec= triMesh.GetSpecifiedNormals();
			for (int i=0; i < triMesh.numFaces; ++i)
			{
				for (int j= 0; j <3 ; ++j)
					triMesh.setNormal(triMesh.faces[i].v[j], nSpec->GetNormal(i,j));
			}
		}
		if (this->expData[(UINT)enms::expDatIDs::dayVColors] && triMesh.mapSupport(0))	//just in case vertex colors are supported
		{
			triMesh.setMapSupport(-2, TRUE);						//activate alpha channel
			triMesh.setNumMapVerts(0, triMesh.numVerts, TRUE);		//ensure that color verts and mesh verts counts correspond.
			triMesh.setNumMapVerts(-2, triMesh.numVerts, TRUE);
		}
		else
		{
			triMesh.setMapSupport(0, FALSE);
			triMesh.setMapSupport(-2, FALSE);		//deactivate this channel in case it's supported
		}

		if (expData[(UINT)enms::expDatIDs::nightVColors] && triMesh.mapSupport(-1))		//night vertex colors
		{
			triMesh.setMapSupport(3, TRUE);		//activate nihgt alpha channel
			triMesh.setNumMapVerts(-1, triMesh.numVerts, TRUE);		//ensure that night verts and mesh verts counts correspond.
			triMesh.setNumMapVerts(3, triMesh.numVerts, TRUE);
		}
		else
		{
			triMesh.setMapSupport(-1, FALSE);
			triMesh.setMapSupport(3, FALSE);
		}

		this->condensedIDs.insert(0);		//insert a single material ID
		//MessageBox(NULL, extensions::ToString<std::size_t>(condensedIDs.size()).c_str(), TEXT("Priority channel = 50"), MB_OK);
		return;
	}

	//remove priority channel from the unordered_set of channels
	sChannels.erase(priorityChannel);
	//make a copy of the mesh
	::Mesh localCopy(triMesh);
	//localCopy.DeleteIsoVerts();		//important but taken care of in DffExporter::WriteGeometry
	//localCopy.DeleteIsoMapVerts();
	triMesh.FreeAll();
	triMesh.setNumVerts(localCopy.getNumMapVerts(priorityChannel));
	triMesh.setNumFaces(localCopy.numFaces);
	triMesh.setMapSupport(priorityChannel, TRUE);
	triMesh.setNumMapVerts(priorityChannel, localCopy.getNumMapVerts(priorityChannel));
	//apparently there can be map support for some channels even if not requested for
	//day vertex colors
	if (this->expData[(UINT)enms::expDatIDs::dayVColors] && localCopy.mapSupport(0))
	{
		triMesh.setMapSupport(0);
		triMesh.setMapSupport(-2);
		triMesh.setNumMapVerts(0, localCopy.getNumMapVerts(priorityChannel));
		triMesh.setNumMapVerts(-2, localCopy.getNumMapVerts(priorityChannel));
	}
	else
	{
		triMesh.setMapSupport(0, FALSE);
		triMesh.setMapSupport(-2, FALSE);
	}
	//night vertex colors
	if (this->expData[(UINT)enms::expDatIDs::nightVColors] && localCopy.mapSupport(-1))
	{
		triMesh.setMapSupport(-1);
		triMesh.setMapSupport(3);
		triMesh.setNumMapVerts(-1, localCopy.getNumMapVerts(priorityChannel));
		triMesh.setNumMapVerts(3, localCopy.getNumMapVerts(priorityChannel));
	}
	else
	{
		triMesh.setMapSupport(-1, FALSE);
		triMesh.setMapSupport(3, FALSE);
	}
	for (std::unordered_set<int>::iterator itr = sChannels.begin(); itr != sChannels.end(); ++itr)
	{
		triMesh.setMapSupport(*itr, TRUE);
		triMesh.setNumMapVerts(*itr, localCopy.getNumMapVerts(priorityChannel));
	}

	for (int fIndex= 0; fIndex < triMesh.numFaces; ++fIndex)
	{
		for (int fCorner= 0; fCorner < 3; ++fCorner)
		{
			//set mesh vertices and faces
			triMesh.setVert(localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.verts[localCopy.faces[fIndex].v[fCorner]]);
			triMesh.faces[fIndex].v[fCorner]= localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner];
			triMesh.setMapVert(priorityChannel, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(priorityChannel)[localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner]]);
			triMesh.mapFaces(priorityChannel)[fIndex].t[fCorner]= localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner];
			for (std::unordered_set<int>::iterator itr = sChannels.begin(); itr != sChannels.end(); ++itr)
			{
				triMesh.setMapVert(*itr, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(*itr)[localCopy.mapFaces(*itr)[fIndex].t[fCorner]]);
				triMesh.mapFaces(*itr)[fIndex].t[fCorner]= localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner];
			}
			if (triMesh.mapSupport(0))
			{
				triMesh.setMapVert(0, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(0)[localCopy.mapFaces(0)[fIndex].t[fCorner]]);
				if (localCopy.mapSupport(-2))
					triMesh.setMapVert(-2, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(-2)[localCopy.mapFaces(-2)[fIndex].t[fCorner]]);
				else
					triMesh.setMapVert(-2, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], UVVert(1.0f,0.0f,0.0f));
			}
			if (triMesh.mapSupport(-1))
			{
				triMesh.setMapVert(-1, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(-1)[localCopy.mapFaces(-1)[fIndex].t[fCorner]]);
				if (localCopy.mapSupport(3))
					triMesh.setMapVert(3, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], localCopy.mapVerts(3)[localCopy.mapFaces(3)[fIndex].t[fCorner]]);
				else
					triMesh.setMapVert(3, localCopy.mapFaces(priorityChannel)[fIndex].t[fCorner], UVVert(1.0f,0.0f,0.0f));
			}
		}
		//triMesh.faces[fIndex].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		//triMesh.faces[fIndex].setSmGroup(1);
		triMesh.faces[fIndex].setMatID(localCopy.getFaceMtlIndex(fIndex));
		this->condensedIDs.insert(localCopy.getFaceMtlIndex(fIndex));
	}

	localCopy.buildNormals();
	triMesh.buildNormals();
	MeshNormalSpec* nSpec= localCopy.GetSpecifiedNormals();
	//nSpec->BuildNormals();
	if (nSpec && nSpec->GetFlag(MESH_NORMAL_NORMALS_BUILT))
	{		
		for (int fIndex =0; fIndex < triMesh.numFaces; ++ fIndex)
		{
			for (int fCorner= 0; fCorner < 3; ++ fCorner)
			{
				triMesh.setNormal(triMesh.faces[fIndex].v[fCorner], nSpec->GetNormal(fIndex, fCorner));
			}
		}
	}
	else
		for (int fIndex= 0; fIndex < triMesh.numFaces; ++fIndex)
		{
			for (int fCorner= 0; fCorner < 3; ++ fCorner)
			{
				//triMesh.setNormal(triMesh.faces[fIndex].v[fCorner], localCopy.getNormal(localCopy.faces[fIndex].v[fCorner]));
				triMesh.setNormal(triMesh.faces[fIndex].v[fCorner], localCopy.getNormal(localCopy.faces[fIndex].v[fCorner]));
			}
		}

	//MessageBox(NULL, extensions::ToString<std::size_t>(condensedIDs.size()).c_str(), TEXT("Caption"), MB_OK);

}

void DffExporter::OverrideExpOptions(Mtl* leMateriel)
{
	//reset export options to what's specified in the dialog
	auto leLambda= [&]()->void 
	{
		this->expData= expDataBackup;
		this->pipeOptions= pipeOptionsBackup;
		this->geoLighting= geoLightingBackup;
		this->nvcHeader = nvcHeaderBackup;
	};

	if (!leMateriel || leMateriel->ClassID() != GTA_MultiMat || this->expData[enms::expDatIDs::overrideGTAMultimaterial])
	{
		leLambda();
		return;
	}


	union {
		int ovrrDialog, boolOptions, faceType, rdPipe;
		float lighting;
	} vals;

	//verify override checkbox -> Parameter removed. Now options in dialog override those in the GTA Multimaterial, in case the "Override GTA multimaterial" checkbox is ticked.
	/*leMateriel->GetParamBlock(0)->GetValue(0, this->iInterface->GetTime(), vals.ovrrDialog, FOREVER);
	if (! vals.ovrrDialog)
	{
		leLambda();
		return;
	}*/

	//face type
	leMateriel->GetParamBlock(0)->GetValue(0, this->iInterface->GetTime(), vals.faceType, FOREVER);

	switch(vals.faceType)
	{
		case 1:
			this->expData[(UINT)enms::expDatIDs::tStrip]= false;
		break;

		case 2:
			this->expData[(UINT)enms::expDatIDs::tStrip]= true;
			this->expData[(UINT)enms::expDatIDs::useZappy]= false;
		break;

		default:
			this->expData[(UINT)enms::expDatIDs::tStrip]= true;
			this->expData[(UINT)enms::expDatIDs::useZappy]= true;
		break;
	}
	

	//render pipeline
	leMateriel->GetParamBlock(0)->GetValue(1, this->iInterface->GetTime(), vals.rdPipe, FOREVER);
	switch(vals.rdPipe)
	{
		case 1:
			this->pipeOptions= enms::renderPipeline::NOTHING;
		break;

		case 2:
			this->pipeOptions= enms::renderPipeline::REFLECTIVE_BUILDING;
		break;

		case 3:
			this->pipeOptions= enms::renderPipeline::NIGHT_VERTEX;
		break;

		default:
			this->pipeOptions= enms::renderPipeline::VEHICLE;
		break;
	}

	//mesh extension to mmc
	for (int i= 2; i <= 11; ++i)
	{
		leMateriel->GetParamBlock(0)->GetValue(i, this->iInterface->GetTime(), vals.boolOptions, FOREVER);
		this->expData[i]= vals.boolOptions ? true : false;
	}
	//night vertex colors header.;
	TCHAR* buffer;
	leMateriel->GetParamBlock(0)->GetValue(12, this->iInterface->GetTime(), (const TCHAR*&)buffer, FOREVER);
	this->nvcHeader = _tcstoul(buffer,nullptr,16);

	//ambient to specular
	for (int i= 13; i <= 15; ++i)
	{
		leMateriel->GetParamBlock(0)->GetValue(i, this->iInterface->GetTime(), vals.lighting, FOREVER);
		this->geoLighting[i-13]= vals.lighting/100.0; // i-13= 0, 1, 2
	}

}

void DffExporter::ProcessGeometry(INode* theNode)
{
	BOOL deleteIt= false;
	TriObject* theTri= extensions::GetObjectFromNode<TriObject>(theNode, iInterface->GetTime(), TRIOBJ_CLASS_ID, deleteIt);
	::Mesh theMesh= theTri->mesh;

	ProcessGeometry2(theNode, theMesh);
	WriteGeometry(theNode, theMesh);

	if (deleteIt)
		theTri->DeleteThis();
}

void DffExporter::ProcessGeometry2(INode* theNode, Mesh& theMesh)
{
	//Override export options in dialog if specified & if that's not the case revert values back (in case they were overriden previously)
	this->OverrideExpOptions(theNode->GetMtl());
	theMesh.RemoveDegenerateFaces(); 
	theMesh.RemoveIllegalFaces();
	this->SplitBySmoothGroup(theMesh);

	theMesh.DeleteIsoVerts();
	theMesh.DeleteIsoMapVerts();
	this->RebuildMesh(theMesh);

	//erase all condensedIDs except the first if the TriObject hasn't been assigned a multimaterial
	::Mtl* nodeMtl= theNode->GetMtl();

	//MessageBox(NULL, extensions::ToString<std::size_t>(condensedIDs.size()).c_str(), TEXT("Caption"), MB_OK);
	//now also considering GTA Multimaterial
	if (! nodeMtl || (nodeMtl->ClassID() != MULTI_MATERIAL_CLASS_ID && nodeMtl->ClassID() != GTA_MultiMat))
		condensedIDs.erase(++condensedIDs.begin(), condensedIDs.end());

}

void DffExporter::WriteGeometry(INode* theNode, Mesh& theMesh)
{
	//write section identifier, size and RW version
	leStream.GenericWrite<UINT>(enms::secIDs::GEOMETRY);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//write struct section info
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//flags: 1= TSTRIP, 2= VTRANS, 4= TEX1, 8= VCOLOR, 16= NORMS, 32= VLIGHT, 64= MMC, 128= TEX2
	BYTE flags;
	flags = this->expData[(UINT)enms::expDatIDs::tStrip] + 2;
	//UV map 1 support
	if (theMesh.mapSupport(1))		//exportData flags are a global switch; do not use for testing here
		flags += 4;
	//verify vertex color support
	if (theMesh.mapSupport(0))
		flags += 8;

	if (this->expData[(UINT)enms::expDatIDs::normals])
		flags += 16;

	if (this->expData[(UINT)enms::expDatIDs::dynLighting])
		flags += 32;

	if (this->expData[(UINT)enms::expDatIDs::MMC])
		flags += 64;

	if (theMesh.mapSupport(2))
		flags += 128;

	leStream.GenericWrite<BYTE>(flags);
	leStream.GenericWrite<BYTE>(0);	//unknown
	leStream.GenericWrite<short>(theMesh.mapSupport(1)+theMesh.mapSupport(2));	//number of UV maps
	//write face count
	leStream.GenericWrite<DWORD>(theMesh.numFaces);
	//write vertex count
	leStream.GenericWrite<DWORD>(theMesh.numVerts);
	//write frame count; always one
	leStream.GenericWrite<DWORD>(1);
	//version specific: write ambient, diffuse and specular values
	if (this->fileVersion == enms::gameConstants::GTA_IIIA || this->fileVersion== enms::gameConstants::GTA_IIIB 
		|| this->fileVersion == enms::gameConstants::GTA_IIIC || this->fileVersion == enms::gameConstants::GTA_VCA)
	{
		leStream.GenericWrite<float>(this->geoLighting.ambient);
		leStream.GenericWrite<float>(this->geoLighting.diffuse);
		leStream.GenericWrite<float>(this->geoLighting.specular);
	}

	//write vertex colors
	if (theMesh.mapSupport(0))
	{
		//MessageBox(NULL, "in vertex colors", extensions::ToString<int>(theMesh.getNumMapVerts(0)).c_str(), MB_OK);
		for (int i= 0; i < theMesh.getNumMapVerts(0); ++i)
		{
			for (int j= 0; j < 3; ++j)
				leStream.GenericWrite<BYTE>(BYTE(theMesh.mapVerts(0)[i][j]*255.0f));

			leStream.GenericWrite<BYTE>(BYTE(theMesh.mapVerts(-2)[i].x * 255.0f));
		}
	}

	//write UV coordinates
	if (theMesh.mapSupport(1))		//UV map 1
	{
		//MessageBox(NULL, extensions::ToString<int>(theMesh.getNumMapVerts(1)).c_str(), "map 1 verts", MB_OK);
		for (int i = 0; i < theMesh.getNumMapVerts(1); ++i)
		{
			leStream.GenericWrite<float>(theMesh.mapVerts(1)[i].x);
			leStream.GenericWrite<float>(1.0f - theMesh.mapVerts(1)[i].y);
		}
	}

	if (theMesh.mapSupport(2))		//UV map 2
	{
		for (int i = 0; i < theMesh.getNumMapVerts(2); ++i)
		{
			leStream.GenericWrite<float>(theMesh.mapVerts(2)[i].x);
			leStream.GenericWrite<float>(1.0f - theMesh.mapVerts(2)[i].y);
		}
	}

	//Write face info (BAFC)
	for (int i= 0; i < theMesh.numFaces; ++i)
	{
		leStream.GenericWrite<USHORT>(theMesh.faces[i].v[1]);
		leStream.GenericWrite<USHORT>(theMesh.faces[i].v[0]);
		auto itr= this->condensedIDs.find(theMesh.faces[i].getMatID());

		if (itr == this->condensedIDs.end())
			leStream.GenericWrite<USHORT>(0);
		else
			leStream.GenericWrite<USHORT>(std::distance(this->condensedIDs.begin(), itr));

		leStream.GenericWrite<USHORT>(theMesh.faces[i].v[2]);
	}

	//write bounding sphere, vertex and normal flags (both 1/0 DWORDs)
	//get scale
	Matrix3 objOffsetMx= (theNode)->GetNodeTM(iInterface->GetTime());
	objOffsetMx.Orthogonalize();
	objOffsetMx.NoScale();
	Point3 scale;		//the node scale which was set to 1 in the frames section.
	//also multiply by object offset scale
	for (int i= 0; i < 3; ++i)
		scale[i]= (theNode)->GetNodeTM(0).GetRow(i)[i]/objOffsetMx.GetRow(i)[i];
	
	//build object transform matrix
	theNode->GetObjOffsetRot().MakeMatrix(objOffsetMx);
	Matrix3 objOffsetTrans(TRUE);
	objOffsetTrans.SetTrans(theNode->GetObjOffsetPos());
	Matrix3 objOffsetScale(TRUE);
	objOffsetScale.SetScale(theNode->GetObjOffsetScale().s);
	objOffsetMx= objOffsetScale*(objOffsetMx*objOffsetTrans);	//parentheses not necessary


/*deprecated
	theMesh.buildBoundingBox();
	Box3 theBox = theMesh.getBoundingBox(NULL);
	Point3 boxCenter= (theBox.Min()+0.5*(-theBox.Min()+theBox.Max()))*objOffsetMx*scale;	//left to right associavity; parentheses for separating objOffsetMx and scale not necessary
	leStream.GenericWrite<float>(boxCenter.x);
	leStream.GenericWrite<float>(boxCenter.y);
	leStream.GenericWrite<float>(boxCenter.z);
	leStream.GenericWrite<float>(0.5*(((theBox.Max()-theBox.Min())*objOffsetMx*scale).FLength()));
	leStream.GenericWrite<DWORD>(1);
	leStream.GenericWrite<DWORD>((DWORD)this->expData[(UINT)enms::expDatIDs::normals]);
end deprecated */


	//miniball lambda function
	auto WriteMiniball= [&](const std::streamoff& reqPos, miniball::Miniball<3>& theBall)->void {
		std::streamoff currentPos= this->leStream.tellp();	//save position
		this->leStream.seekp(reqPos, std::ios::beg);
		theBall.build();	//build the ball
		//write center
		for (auto i= 0; i < 3; ++i)
			leStream.GenericWrite<float>(theBall.center()[i]);
		//write radius
		leStream.GenericWrite<float>(sqrt(theBall.squared_radius()));
		leStream.GenericWrite<DWORD>(1);
		leStream.GenericWrite<DWORD>(this->expData[(UINT)enms::expDatIDs::normals]);
		this->leStream.seekp(currentPos, std::ios::beg);		//go back to end of vertex section
	};
//new code that uses "miniball" algorithms by Dr Bernd Gaertner, ETH Zuerich
	//store file position.
	//write 24 arbitrary bytes
	//Write vertices and check-in each into the miniball.
	//must take object offset and scale into consideration while writing vertex coordinates and normals
	std::streamoff backupPos= this->leStream.tellp();
	leStream.GenericWrite<extensions::geomBounding>(extensions::geomBounding());	//24 arbitrary bytes
	miniball::Miniball<3> laBalle;

	for (int i=0; i< theMesh.numVerts; ++i)
	{
		Point3 ScaledVert= theMesh.verts[i]*objOffsetMx*scale;	//again parentheses not necessary
		miniball::Point<3> thePoint;		//miniball point
		for(int j=0; j<3 ; ++j)
		{
			thePoint[j]= ScaledVert[j];
			leStream.GenericWrite<float>(ScaledVert[j]);
		}
		laBalle.check_in(thePoint);
	}
	WriteMiniball(backupPos, laBalle);
	objOffsetMx.SetRow(3, Point3(0,0,0));	//zero the translation portion of the matrix

	//Write normals
	if (this->expData[(UINT)enms::expDatIDs::normals])
	{
		//MessageBox(NULL, extensions::ToString<int>(theMesh.normalCount).c_str(), TEXT("Normals"), MB_OK);
/* Deprecated since there can be more than one normal per vertex
		for (int i= 0; i < theMesh.normalCount; ++i)
		{
			Point3 scaledNormal= (theMesh.getNormal(i)*objOffsetMx).FNormalize();
			for (int j=0; j<3; ++j)
				leStream.GenericWrite<float>(scaledNormal[j]);
		}
*/
		for (auto vIndex=0; vIndex < theMesh.numVerts; ++vIndex)
		{
			Point3 AvgNormal(0,0,0);
			auto NormalCount= theMesh.getRVert(vIndex).rFlags & NORCT_MASK;
			if (NormalCount > 1)
			{
				for (auto normIndex=0; normIndex < NormalCount; ++normIndex)
				{
						AvgNormal += theMesh.getRVert(vIndex).ern[normIndex].getNormal();
				}

				AvgNormal /= NormalCount;
			}
			else
				AvgNormal= theMesh.getRVert(vIndex).rn.getNormal();

			AvgNormal= (AvgNormal*objOffsetMx).FNormalize();

			for (auto coord =0; coord < 3; ++ coord)
				leStream.GenericWrite<float>(AvgNormal[coord]);
		}
	}

	//write struct section size
	leStream.FlushSectionSize();
	//write material list
	this->WriteMaterialList(theNode);
	//Write mesh extension
	this->WriteGeomExtension(theMesh);

	//write geometry section size
	leStream.FlushSectionSize();
#ifdef LOGFILE
	logFile << clumpIdx << ": Geometry written" << std::endl;
#endif
}

inline void DffExporter::WriteMaterialList(INode* leNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing materials list" << std::endl;
#endif
	//node long pointer used as flag to indicate presence/absence of Material Effects PLG. Required since info to that effect must be added in associated atomic section
	leNode->SetNodeLong(0);
	//Write material list header
	leStream.GenericWrite<UINT>(enms::secIDs::MATERIAL_LIST);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write struct header
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//write number of materials (at least one)
	leStream.GenericWrite<DWORD>(this->condensedIDs.size());
	// write 0xffffffff reserved values
	for (int i=0; i < this->condensedIDs.size(); ++i)
		leStream.GenericWrite<DWORD>(0xffffffff);
	//write struct section size
	leStream.FlushSectionSize();

	Mtl* theMaterial= leNode->GetMtl();
	if (!theMaterial)
	{
		for (auto itr= this->condensedIDs.begin(); itr != condensedIDs.end(); ++itr)
			this->WriteGTAMaterial(NULL, leNode);
	}

	else if (theMaterial->ClassID() == MULTI_MATERIAL_CLASS_ID || theMaterial->ClassID() == GTA_MultiMat)
	{
		for (auto itr= this->condensedIDs.begin(); itr != this->condensedIDs.end(); ++itr)
		{
			::Mtl* subMat= theMaterial->GetSubMtl(*itr);
			//sub materials can have arbitrary IDs; not always in range 0 <= materialID < Submaterial count
			if (subMat && subMat->ClassID() == Class_ID(enms::gameConstants::GTA_MATA, enms::gameConstants::GTA_MATB))
				this->WriteGTAMaterial(subMat, leNode);
			else //else write the submaterial at index 0. GetSubMtl(int) will evaluate to NULL if it isn't present.
				this->WriteGTAMaterial(theMaterial->GetSubMtl(0), leNode);
		}
	}

	else if (theMaterial->ClassID() == Class_ID(enms::gameConstants::GTA_MATA, enms::gameConstants::GTA_MATB))
	{
		for (auto itr= this->condensedIDs.begin(); itr != this->condensedIDs.end(); ++itr)
			this->WriteGTAMaterial(theMaterial, leNode);
	}

	else
	{
		for (auto itr= this->condensedIDs.begin(); itr != condensedIDs.end(); ++itr)
			this->WriteGTAMaterial(NULL, leNode);
	}

	//write material list size
	leStream.FlushSectionSize();
}

//Kam uses ambient, specular, diffuse (in this order)
void DffExporter::WriteGTAMaterial(Mtl* material, INode* theNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing material" << std::endl;
#endif
	union {
		int alpha;
		float ambient;
		float specular;
		float diffuse;
		int UVAnimExp;
	};
	//write material header
	leStream.GenericWrite<UINT>(enms::secIDs::MATERIAL);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write struct header
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	if (!material)
	{
		//write info
		leStream.GenericWrite<DWORD>(0);		//unknown integer
		leStream.GenericWrite<DWORD>(0xffffffff);	//RGBA color (all 0xff)
		leStream.GenericWrite<UINT>(406430684);		//unknown integer
		leStream.GenericWrite<UINT>(0);		//no textures
		for (int i=0; i < 3; ++i)
			leStream.GenericWrite<float>(1.0f);		//ambient, diffuse and specular (all 1)
		leStream.FlushSectionSize();			//write struct section size
	}
	else
	{
		//write info
		//leStream.GenericWrite<DWORD>(0);
		//write uknown DWORD 1.
		TCHAR* uknPtr;
		material->GetParamBlock(0)->GetValue(24, iInterface->GetTime(), (const TCHAR*&)uknPtr, FOREVER);
		leStream.GenericWrite<ULONG>(_tcstoul(uknPtr, nullptr, 16));
		//Get RGB colors
		::Color matColor;
		material->GetParamBlock(0)->GetValue(3, iInterface->GetTime(), matColor, FOREVER, 0);
		for (int i= 0; i < 3; ++i)
			leStream.GenericWrite<BYTE>(matColor[i] * 255.0f);
		//Get alpha
		//int alpha;
		material->GetParamBlock(0)->GetValue(6, iInterface->GetTime(), alpha, FOREVER, 0);
		leStream.GenericWrite<BYTE>(alpha);
		//leStream.GenericWrite<UINT>(406430684);		//unknown integer
		material->GetParamBlock(0)->GetValue(25, iInterface->GetTime(), (const TCHAR*&)uknPtr, FOREVER);
		leStream.GenericWrite<ULONG>(_tcstoul(uknPtr, nullptr, 16));
		//verify presence of diffuse or alpha textures (paramIDs 4 and 7 respectively. Use colormap= 5, use alphamap= 8)
		::BitmapTex* diffTex= NULL; ::BitmapTex* alphaTex= NULL; int useDiffuse= 0; int useAlpha= 0;
		material->GetParamBlock(0)->GetValue(4, iInterface->GetTime(), (::Texmap*&)diffTex, FOREVER);
		material->GetParamBlock(0)->GetValue(7, iInterface->GetTime(), (::Texmap*&)alphaTex, FOREVER);
		material->GetParamBlock(0)->GetValue(5, iInterface->GetTime(), useDiffuse, FOREVER);
		material->GetParamBlock(0)->GetValue(8, iInterface->GetTime(), useAlpha, FOREVER);
		//MessageBox(NULL, extensions::ToString<BitmapTex*>(diffTex).c_str(), "caption", MB_OK);
		//MessageBox(NULL, extensions::ToString<BitmapTex*>(alphaTex).c_str(), "caption", MB_OK);
		if ((diffTex  || alphaTex ) && (useDiffuse || useAlpha))
			leStream.GenericWrite<UINT>(1);
		else
			leStream.GenericWrite<UINT>(0);
		//float ambSpecDiff;
		material->GetParamBlock(0)->GetValue(0, iInterface->GetTime(), ambient, FOREVER);	//ambient
		leStream.GenericWrite<float>(ambient);
		material->GetParamBlock(0)->GetValue(1, iInterface->GetTime(), specular, FOREVER);	//specular
		leStream.GenericWrite<float>(specular);
		material->GetParamBlock(0)->GetValue(2, iInterface->GetTime(), diffuse, FOREVER);	//diffuse
		leStream.GenericWrite<float>(diffuse);
		leStream.FlushSectionSize();			//write struct section size

		//verify if UV anim export is activated and bitmap present
		UVAnimExp= 0;
		material->GetParamBlock(0)->GetValue(22, iInterface->GetTime(), UVAnimExp, FOREVER);	//UV anim checkbox

		if (diffTex && useDiffuse && UVAnimExp)
			this->expData[(UINT)enms::expDatIDs::UVanims]= true;
		else
			this->expData[(UINT)enms::expDatIDs::UVanims]= false;

		//write material textures
		if ((diffTex  || alphaTex ) && (useDiffuse || useAlpha))
			this->WriteMatTextures(diffTex, alphaTex, useDiffuse, useAlpha);
	}
	//write material extension
	this->WriteMaterialExtension(material, theNode);
	leStream.FlushSectionSize();			//write material section size
}

void DffExporter::WriteMatTextures(::BitmapTex* diffTex, ::BitmapTex* alphaTex, bool useDiffuse, bool useAlpha)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing textures" << std::endl;
#endif
	//write header
	leStream.GenericWrite<UINT>(enms::secIDs::TEXTURE);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//Write struct
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0,true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	
	::BitmapTex* defaultTex= diffTex ? diffTex : alphaTex;

	switch(defaultTex->GetFilterType())
	{
		case FILTER_NADA:
			leStream.GenericWrite<BYTE>(1);
		break;

		case FILTER_SAT:
			leStream.GenericWrite<BYTE>(2);
		break;

		default:
			leStream.GenericWrite<BYTE>(6);
		break;
	}
	BYTE flags= 0;
	if(defaultTex->GetUVGen()->GetFlag(U_WRAP))
		flags += 0x01;
	else
		flags += 0x02;

	if (defaultTex->GetUVGen()->GetFlag(V_WRAP))
		flags += 0x00;/*flags += 0x10; flag is zero in default R* models*/
	else
		flags += 0x20;

	leStream.GenericWrite<BYTE>(flags);
	leStream.GenericWrite<USHORT>(1);	//write unknown (always 1)
	//write struct size
	leStream.FlushSectionSize();

	//write diffuse texture string header
	leStream.GenericWrite<UINT>(enms::secIDs::STRING);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	if (diffTex && useDiffuse)
	{
		TCHAR path[MAX_PATH], shortName[50], extension[10];
		::BMMSplitFilename(diffTex->GetMapName(), path, shortName, extension);
		leStream.WritePaddedString(shortName);
	}
	else
		leStream.WritePaddedString(TEXT(""));
	leStream.FlushSectionSize();

	//write alpha texture string header
	leStream.GenericWrite<UINT>(enms::secIDs::STRING);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	if (alphaTex && useAlpha)
	{
		TCHAR path[MAX_PATH], shortName[100], extension[10];
		::BMMSplitFilename(alphaTex->GetMapName(), path, shortName, extension);
		leStream.WritePaddedString(shortName);
	}
	else
		leStream.WritePaddedString(TEXT(""));
	leStream.FlushSectionSize();

	//write an empty extension
	leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.FlushSectionSize();

	//write texture section size
	leStream.FlushSectionSize();
}

//Material effects PLG= Reflection image;	Reflection material= SA specular; Specular material= Specular image 
void DffExporter::WriteMaterialExtension(Mtl* leMateriel, INode* theNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing material extension" << std::endl;
#endif
	leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	if (leMateriel)
	{
		this->WriteMaterialEffects(leMateriel, theNode);
		this->WriteReflection(leMateriel);
		this->WriteSpecular(leMateriel);

		if (this->expData[(UINT)enms::expDatIDs::UVanims])
		{
			this->animatedMats.insert(leMateriel);
			this->WriteUVAnimPlg(leMateriel);
		}
	}
	leStream.FlushSectionSize();
}

void DffExporter::WriteUVAnimPlg(Mtl* leMateriel)
{
	leStream.GenericWrite<UINT>(enms::secIDs::UV_ANIM_PLG);	//UV anim PLG section ID
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//UV anim PLG info is embedded in a struct
	leStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write unknown (1)
	leStream.GenericWrite<UINT>(1);
	leStream.WritePaddedString(std::tstring(leMateriel->GetName()), 32);
	leStream.FlushSectionSize();
	leStream.FlushSectionSize();
}

void DffExporter::WriteUVAnimDic()
{
	UVAnimStream.GenericWrite<UINT>(enms::secIDs::UVANIM_DIC);
	UVAnimStream.GenericWrite<UINT>(0, true);
	UVAnimStream.GenericWrite<UINT>(this->fileVersion);

	//write struct
	UVAnimStream.GenericWrite<UINT>(enms::secIDs::STRUCT);
	UVAnimStream.GenericWrite<UINT>(0, true);
	UVAnimStream.GenericWrite<UINT>(this->fileVersion);
	UVAnimStream.GenericWrite<UINT>(this->animatedMats.size());
	UVAnimStream.FlushSectionSize();
	for (std::unordered_set<Mtl*>::iterator itr = this->animatedMats.begin(); itr != this->animatedMats.end(); ++itr)
		WriteAnimAnimation(*itr);
	UVAnimStream.FlushSectionSize();
}

void DffExporter::WriteAnimAnimation(Mtl* leMateriel)
{
	//retrieve the key info for this material
	this->UVAnimKeys.clear();
	//get the diffuse bitmap
	::BitmapTex* diffTex;
	leMateriel->GetParamBlock(0)->GetValue(4, iInterface->GetTime(), (::Texmap*&)diffTex, FOREVER);
	::StdUVGen* leUVGen= diffTex->GetUVGen();
	//must insert start and end
	this->RetrieveAnimInfo(::GetAnimStart(), leUVGen);
	this->RetrieveAnimInfo(::GetAnimEnd(), leUVGen);
	
	//Access keys like this: <pointer to StdUVGen instance>->SubAnim(0)->SubAnim(k), where 0 <= k && k < 4
	//0= UOffset, 1= VOffset, 2= UTile, 3= VTile
	//Add keys
	for (int i= 0; i < 4; ++i)
		for (int j=0; j < leUVGen->SubAnim(0)->SubAnim(i)->NumKeys(); ++j)
		{
			::TimeValue leTemps= leUVGen->SubAnim(0)->SubAnim(i)->GetKeyTime(j);
			RetrieveAnimInfo(leTemps, leUVGen);
		}
	
	//MessageBox(NULL, extensions::ToString<int>(UVAnimKeys.size()).c_str(), "caption", MB_OK);
	//Write the section
	UVAnimStream.GenericWrite<UINT>(enms::secIDs::ANIM_ANIMATION);
	UVAnimStream.GenericWrite<UINT>(0, true);
	UVAnimStream.GenericWrite<UINT>(this->fileVersion);
	UVAnimStream.GenericWrite<UINT>(256);		//constant 256
	UVAnimStream.GenericWrite<UINT>(449);		//constant 449
	UVAnimStream.GenericWrite<UINT>(UVAnimKeys.size());	//number of keys
	UVAnimStream.GenericWrite<UINT>(0);	//unknown1
	UVAnimStream.GenericWrite<float>(UVAnimKeys.rbegin()->second.timeOver4800);	// final key time/4800.0f
	UVAnimStream.GenericWrite<UINT>(0);	//unknown2
	UVAnimStream.WritePaddedString(std::tstring(leMateriel->GetName()), 64);		//material name

	//evaluate (index-1) member and write keys. Deniska uses (i-2) since array indices start from 1 in MAXScript
	for (std::map<::TimeValue, extensions::UVanimKey>::iterator itr= this->UVAnimKeys.begin(); itr != this->UVAnimKeys.end(); ++itr)
	{
		itr->second.iMinus1= std::distance(this->UVAnimKeys.begin(), itr) - 1;
		UVAnimStream.GenericWrite<extensions::UVanimKey>(itr->second);
	}

	UVAnimStream.FlushSectionSize();
}

inline void DffExporter::RetrieveAnimInfo(::TimeValue theTime, ::StdUVGen* leUVGen)
{
	this->UVAnimKeys[theTime].negUOffset= - (leUVGen->GetUOffs(theTime));
	this->UVAnimKeys[theTime].VOffset= leUVGen->GetVOffs(theTime);
	this->UVAnimKeys[theTime].UTiling= leUVGen->GetUScl(theTime);
	this->UVAnimKeys[theTime].VTiling= leUVGen->GetVScl(theTime);
	UVAnimKeys[theTime].timeOver4800= ((float)theTime)/4800.0;
	UVAnimKeys[theTime].trash1= 0; UVAnimKeys[theTime].trash2= 0;
}

void DffExporter::WriteMaterialEffects(Mtl* leMateriel, INode* leNode)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing material effects" << std::endl;
#endif
	union	//anonymous union
	{	
		int use_RI;
		int matEffectsOverride;
		float refAmount;
		int useTex;
	};

	leMateriel->GetParamBlock(0)->GetValue(19, iInterface->GetTime(), use_RI, FOREVER);
	if (!use_RI)
		return;

	//verify mat effects override
	leMateriel->GetParamBlock(0)->GetValue(23, iInterface->GetTime(), matEffectsOverride, FOREVER);

	//set node long pointer
	leNode->SetNodeLong(1);
	//write header
	leStream.GenericWrite<UINT>(enms::secIDs::MATERIAL_EFFECTS_PLG);
	leStream.GenericWrite<UINT>(0,true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	if (matEffectsOverride)
	{
		leStream.GenericWrite<UINT>(5);
		leStream.GenericWrite<UINT>(5);
		leStream.GenericWrite<UINT>(0);
		leStream.FlushSectionSize();
		return;
	}

	leStream.GenericWrite<UINT>(2);		//write two unknowns
	leStream.GenericWrite<UINT>(2);

	leMateriel->GetParamBlock(0)->GetValue(9, iInterface->GetTime(), refAmount, FOREVER);
	leStream.GenericWrite<float>(refAmount/100.0f);		//write reflection value
	leStream.GenericWrite<UINT>(0);		//write unknown
	::BitmapTex* matEffectsTex;
	//retrieve reflection texture map, if present
	leMateriel->GetParamBlock(0)->GetValue(10, iInterface->GetTime(), (::Texmap*&)matEffectsTex, FOREVER,0);
	useTex= 0;
	leMateriel->GetParamBlock(0)->GetValue(11, iInterface->GetTime(), useTex, FOREVER,0);

	if (matEffectsTex && useTex)
	{
		leStream.GenericWrite<UINT>(1);		//set texture ON/OFF switch
		this->WriteMatTextures(matEffectsTex, NULL, true, false);
	}
	else
		leStream.GenericWrite<UINT>(0);

	//write junk 00 00 00 00 data
	leStream.GenericWrite<UINT>(0);
	leStream.FlushSectionSize();
}

void DffExporter::WriteReflection(Mtl* leMateriel)	//aka SA specular
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing reflection" << std::endl;
#endif
	int use_SAS= 0;
	leMateriel->GetParamBlock(0)->GetValue(20, iInterface->GetTime(), use_SAS, FOREVER);
	if (!use_SAS )//seems like SA specular is also available in vice city :/
		return;

	//write header
	leStream.GenericWrite<UINT>(enms::secIDs::REFLECTION_MATERIAL);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//get specular color
	::Color specColor;
	leMateriel->GetParamBlock(0)->GetValue(12, iInterface->GetTime(), specColor, FOREVER);
	//write color
	for (int i=0; i<3; ++i)
		leStream.GenericWrite<float>(specColor[i]);

	//get specular alpha
	int specAlpha;
	leMateriel->GetParamBlock(0)->GetValue(15, iInterface->GetTime(), specAlpha, FOREVER);
	leStream.GenericWrite<float>(specAlpha/255.0f);
	
	//get specRefBlend
	float specRefBlend;
	leMateriel->GetParamBlock(0)->GetValue(17, iInterface->GetTime(), specRefBlend, FOREVER);
	leStream.GenericWrite<float>(specRefBlend);

	//write unknown(0)
	leStream.GenericWrite<DWORD>(0);
	//write section size
	leStream.FlushSectionSize();
}

void DffExporter::WriteSpecular(Mtl* leMateriel)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing specular" << std::endl;
#endif
	int use_SI= 0;
	leMateriel->GetParamBlock(0)->GetValue(21, iInterface->GetTime(), use_SI, FOREVER);
	if (!use_SI)
		return;

	//write header
	leStream.GenericWrite<UINT>(enms::secIDs::SPECULAR_MATERIAL);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//get glossiness
	float glossiness;
	leMateriel->GetParamBlock(0)->GetValue(16, iInterface->GetTime(), glossiness, FOREVER);
	leStream.GenericWrite<float>(glossiness/100.0f);
	::BitmapTex* specTex= NULL;
	leMateriel->GetParamBlock(0)->GetValue(13, iInterface->GetTime(), (Texmap*&)specTex, FOREVER);
	int useSpecularMap= 0;
	leMateriel->GetParamBlock(0)->GetValue(14, iInterface->GetTime(), useSpecularMap,FOREVER);
	if (specTex && useSpecularMap)
	{
		TCHAR path[MAX_PATH], shortName[100], extension[10];
		::BMMSplitFilename(specTex->GetMapName(),path, shortName, extension);
		leStream.writeANSIString(shortName, false, 24);
	}
	else
		leStream.writeANSIString(TEXT(""), false, 24);

	leStream.FlushSectionSize();
}

inline void DffExporter::WriteGeomExtension(::Mesh& theMesh)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing geometry extension" << std::endl;
#endif
	//MessageBox(NULL, "in geom extension", "caption", MB_OK);
	//Write geometry extension
	leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	//write constituent sections
	this->WriteBinMeshPLG(theMesh);

	if (this->expData[(UINT)enms::expDatIDs::expPreset])
		this->GenSkinPLG();

	this->WriteMeshExtension();
	this->WriteMorphPLG();
	this->WriteNVC(theMesh);
	//write geometry extension section size
	leStream.FlushSectionSize();
}

inline void DffExporter::WriteMorphPLG()
{
	if (! this->expData[(UINT)enms::expDatIDs::morphPlg])
		return;

	leStream.GenericWrite<UINT>(enms::secIDs::MORPH_PLG);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<UINT>(0);
	leStream.FlushSectionSize();
}

inline void DffExporter::WriteBinMeshPLG(::Mesh& theMesh)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing BinMesh PLG" << std::endl;
#endif
	//MessageBox(NULL, "in bin mesh", "caption", MB_OK);
	//split the mesh
	std::map<DWORD, std::vector<UINT>> triLists;
	this->SplitFaceList(theMesh, triLists);

	//Write BinMeshPLG section
	leStream.GenericWrite<UINT>(enms::secIDs::BIN_MESH_PLG);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	if (this->expData[(UINT)enms::expDatIDs::tStrip])
		this->WriteTriStrip(theMesh, triLists);
	else
		this->WriteTriList(theMesh, triLists);
	//write binmeshplg section size
	leStream.FlushSectionSize();
}

inline void DffExporter::WriteTriStrip(::Mesh& theMesh, std::map<DWORD, std::vector<UINT>>& triLists)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing tristrip" << std::endl;
#endif
	std::map<DWORD, std::vector<UINT>> triStrips;
	DWORD idCount= 0;

	if(this->expData[(UINT)enms::expDatIDs::useZappy])
		idCount = this->GenerateZappyStrips(triLists, triStrips);
	else
		idCount = this->GenerateNVStrips(triLists, triStrips);


	if (!idCount)
	{
		this->WriteTriList(theMesh, triLists);
		return;
	}
	//write split type
	leStream.GenericWrite<UINT>(1);
	//write numberof material IDs
	leStream.GenericWrite<UINT>(triLists.size());
	//write total index count
	leStream.GenericWrite<DWORD>(idCount);
	for (std::map<DWORD, std::vector<UINT>>::iterator fItr= triStrips.begin() ; fItr != triStrips.end(); ++fItr)
	{
		//write index count for each split
		leStream.GenericWrite<DWORD>(fItr->second.size());
		//write materialID
		leStream.GenericWrite<DWORD>(fItr->first);
		//write indices
		leStream.write(reinterpret_cast<char*>(&(fItr->second[0])), sizeof(UINT)*fItr->second.size());
	}
}

inline void DffExporter::WriteTriList(::Mesh& theMesh, std::map<DWORD, std::vector<UINT>>& triLists)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing trilist" << std::endl;
#endif
	//write split type
	leStream.GenericWrite<UINT>(0);
	//write number of material IDs
	leStream.GenericWrite<UINT>(triLists.size());
	//write total index count
	leStream.GenericWrite<DWORD>(theMesh.numFaces*3);
	for (std::map<DWORD, std::vector<UINT>>::iterator fItr = triLists.begin(); fItr != triLists.end(); ++fItr)
	{
		//write index count for each list
		leStream.GenericWrite<DWORD>(fItr->second.size());
		//write material ID
		leStream.GenericWrite<DWORD>(fItr->first);
		//write indices
		leStream.write(reinterpret_cast<char*>(&(fItr->second[0])), sizeof(UINT)*fItr->second.size());
	}
}
inline void DffExporter::WriteMeshExtension()
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing mesh extension" << std::endl;
#endif
	//MessageBox(NULL, "You are in mesh extension", "Caption", MB_OK);
	//write mesh extension section
	if (! this->expData[(UINT)enms::expDatIDs::meshExt])
		return;

	leStream.GenericWrite<UINT>(enms::secIDs::MESH_EXTENSION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	leStream.GenericWrite<DWORD>(0);
	//write mesh extension section size
	leStream.FlushSectionSize();
}

void DffExporter::WriteNVC(::Mesh& theMesh)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing night vertex colors" << std::endl;
#endif
	//verify NVC option and map channel -1
	if(!this->expData[(UINT)enms::expDatIDs::nightVColors] ||  !theMesh.mapSupport(-1))
		return;

	//write section header
	leStream.GenericWrite<UINT>(enms::secIDs::NIGHT_VERTEX_COLORS);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	//write unknown 4 byte value. Using same figure as in Rockstar's freightcrane1.dff
	//leStream.GenericWrite<DWORD>(0x0e872b00);
	leStream.GenericWrite<DWORD>(this->nvcHeader);	//now writing a custom value for the header.
	for (int i= 0; i < theMesh.numVerts; ++i)
	{
		//Write night RGB
		for (int j= 0; j< 3; ++j)
			leStream.GenericWrite<BYTE>(theMesh.mapVerts(-1)[i][j] * 255.0f);

		//Write night alpha
		leStream.GenericWrite<BYTE>(theMesh.mapVerts(3)[i].x * 255.0f);
	}
	//write section size
	leStream.FlushSectionSize();
}

inline void DffExporter::SplitFaceList(::Mesh& leMesh, std::map<DWORD, std::vector<UINT>>& faceSplits)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": splitting face list" << std::endl;
#endif
	//MessageBox(NULL, extensions::ToString<int>(leMesh.numFaces).c_str(), "caption", MB_OK);
	WORD cndsID;
	std::set<WORD>::iterator itr;

	for (int i= 0; i < leMesh.numFaces; ++i)
	{
		itr= condensedIDs.find(leMesh.faces[i].getMatID());
		cndsID= itr == condensedIDs.end() ? 0 : std::distance(condensedIDs.begin(), itr);
		for (int j=0; j<3; ++j)
			faceSplits[cndsID].push_back(leMesh.faces[i].v[j]);
	}
	//MessageBox(NULL, "done", "done", MB_OK);
}

DWORD DffExporter::GenerateNVStrips(std::map<DWORD, std::vector<UINT>>& triLists, std::map<DWORD, std::vector<UINT>>& triStrips)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": generating NVDIA strips" << std::endl;
#endif
	bool stripsOk= false;
	DWORD result= 0;
	for (std::map<DWORD, std::vector<UINT>>::iterator listItr= triLists.begin(); listItr!= triLists.end(); ++listItr)
	{
		//use either of the strippers, NOT BOTH!
		//NVTriStrip stripper
		UINT numGroups= 1;
		::PrimitiveGroup** theGroup = new PrimitiveGroup*[1];		//must be an array of pointers to PrimitiveGroup
		//if true is used for last parameter, stripping will fail in a mesh with inverted faces. Typically the case when importing a stripped model and re-exporting it
		stripsOk= ::GenerateStrips(&listItr->second[0], listItr->second.size(), theGroup, &numGroups, false);
#ifdef LOGFILE
	logFile << clumpIdx << ": NVDIA strips generated" << std::endl;
#endif
		if (!stripsOk)
		{	
			delete[] theGroup;
			return 0;
		}
		//itr->second= std::vector<UINT>(theGroup->indices, theGroup->indices+theGroup->numIndices);
#ifdef LOGFILE
		logFile << clumpIdx << ": number of strip indices= " << theGroup[0]->numIndices << std::endl;
#endif
		triStrips[listItr->first]= std::vector<UINT>(theGroup[0]->indices, theGroup[0]->indices+theGroup[0]->numIndices);
#ifdef LOGFILE
		logFile << clumpIdx << ": strip indices copied successfully" << std::endl;
#endif
		result += theGroup[0]->numIndices;
		delete[] theGroup;
#ifdef LOGFILE
		logFile << clumpIdx << ": Primitive group deleted" << std::endl;
#endif
	}
	return result;
}

DWORD DffExporter::GenerateZappyStrips(std::map<DWORD, std::vector<UINT>>& triLists, std::map<DWORD, std::vector<UINT>>& triStrips)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": generating ZAPPY strips" << std::endl;
#endif
	bool stripsOk= false;
	DWORD result= 0;
	for (std::map<DWORD, std::vector<UINT>>::iterator listItr= triLists.begin(); listItr!= triLists.end(); ++listItr)
	{
		::STRIPERCREATE stripIni;
		stripIni.DFaces= &listItr->second[0];
		stripIni.NbFaces= listItr->second.size()/3;
		stripIni.AskForWords= false;
		stripIni.ConnectAllStrips= true;
		stripIni.OneSided= false;
		stripIni.SGIAlgorithm= true;
		stripIni.WFaces= NULL;

		::Striper theStrip;
		stripsOk= theStrip.Init(stripIni);

		if (!stripsOk)
			return 0;

		::STRIPERRESULT theStripResult;
		theStripResult.AskForWords= false;
		stripsOk= theStrip.Compute(theStripResult);

		if (! stripsOk)
			return 0;

		triStrips[listItr->first]= std::vector<UINT>((UINT*)theStripResult.StripRuns, (UINT*)theStripResult.StripRuns+theStripResult.StripLengths[0]);
		result += theStripResult.StripLengths[0];
	}
	return result;
}

void DffExporter::WriteCollision()
{
#ifdef LOGFILE
	logFile << clumpIdx << ": writing collision" << std::endl;
#endif
	//verify if file has been specified
	if (this->fNameStrings.colString == TEXT(""))
		return;

	//write collision header
	leStream.GenericWrite<UINT>(enms::secIDs::COLLISION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);

	std::fstream colStream(fNameStrings.colString, std::ios::in | std::ios::binary | std::ios::beg);

	//read file in approximately 100kb chunks
	colStream.seekg(0, std::ios::end);
	std::streamsize fileSize= colStream.tellg();
	colStream.seekg(0, std::ios::beg);

	std::streamsize num100kChunks= fileSize / 100000;
	std::streamsize remainder= fileSize % 100000;

	char* buffer= NULL;
	for (std::streamsize i =0; i < num100kChunks; ++i)
	{
		buffer= new char[100000];
		colStream.read(buffer, 100000);
		leStream.write(buffer, 100000);
		delete[] buffer;
	}

	buffer= new char[remainder];
	colStream.read(buffer, remainder);
	leStream.write(buffer, remainder);
	delete[] buffer;
	colStream.close();
	//write size of section
	leStream.FlushSectionSize();
}

void DffExporter::UpdateFile()	//append clumps to UVanim file and rename it (after deleting the file associated with leStream)
{
#ifdef LOGFILE
	logFile << clumpIdx << ": appending UVanims" << std::endl;
#endif
	//read leStream in 100 000 byte chunks
	leStream.seekg(0, std::ios::end);
	std::streamsize fileSize= leStream.tellg();
	leStream.seekg(0, std::ios::beg);
	UVAnimStream.seekp(0, std::ios::end);

	std::streamsize num100kChunks= fileSize / 100000;
	std::streamsize remainder= fileSize % 100000;

	char* buffer= NULL;
	for (std::streamsize i= 0; i < num100kChunks; ++i)
	{
		buffer= new char[100000];
		leStream.read(buffer, 100000);
		UVAnimStream.write(buffer, 100000);
		delete[] buffer;
	}

	buffer= new char[remainder];
	leStream.read(buffer, remainder);
	UVAnimStream.write(buffer, remainder);
	delete[] buffer;

	//close the filestreams
	leStream.close();
	UVAnimStream.close();

	DeleteFile(this->fNameStrings.filename.c_str());	//delete file
	MoveFile(this->fNameStrings.UVanimFile.c_str(), this->fNameStrings.filename.c_str());
}
//seems to work
/*
DWORD DffExporter::GenerateZappyStripsExt(::Mesh& theMesh, std::map<DWORD, std::map<DWORD, std::vector<UINT>>>& faceSplits)
{
	//set the ok flag to true
	bool stripsOk= false;
	//Build adjacent edge list
	::AdjEdgeList ae(theMesh);
	//Build adjacent face list
	::AdjFaceList af(theMesh, ae);
	//Build element list
	::FaceElementList fElem(theMesh, af);
	DWORD result= 0;

	for (int i= 0; i< theMesh.numFaces; ++i)
	{
		for (int j= 0; j< 3; ++j)
			faceSplits[theMesh.faces[i].getMatID()][fElem.elem[i]].push_back(theMesh.faces[i].v[j]);
	}

	//stich tristrips together and copy each to the first map element associated with each material ID
	for (std::map<DWORD, std::map<DWORD, std::vector<UINT>>>::iterator matIDItr= faceSplits.begin(); matIDItr != faceSplits.end(); ++matIDItr)
	{
		//get first strip
		std::map<DWORD, std::vector<UINT>>::iterator elemItr= matIDItr->second.begin();

		::STRIPERCREATE stripIni;
		stripIni.DFaces= &elemItr->second[0];
		stripIni.NbFaces= (elemItr->second.size()/3);
		stripIni.AskForWords= false;
		stripIni.ConnectAllStrips= true;
		stripIni.OneSided= false;
		stripIni.SGIAlgorithm= true;
		stripIni.WFaces= NULL;

		::Striper theStrip;
		stripsOk= theStrip.Init(stripIni);
		if (! stripsOk)
			return 0;
		::STRIPERRESULT theStripResult;
		theStripResult.AskForWords= false;
		stripsOk= theStrip.Compute(theStripResult);
		if (!stripsOk)
			return 0;
		elemItr->second= std::vector<UINT>((UINT*)theStripResult.StripRuns, (UINT*)theStripResult.StripRuns+theStripResult.StripLengths[0]);
		//increment iterator
		++elemItr;
		for (; elemItr != matIDItr->second.end(); ++elemItr)
		{
			//generate strip for elemItr
			stripIni.DFaces= &elemItr->second[0];
			stripIni.NbFaces= (elemItr->second.size()/3);
			stripIni.AskForWords= false;
			stripIni.ConnectAllStrips= true;
			stripIni.OneSided= false;
			stripIni.SGIAlgorithm= true;
			stripIni.WFaces= NULL;

			theStrip.Init(stripIni);
			theStripResult.AskForWords= false;
			stripsOk= theStrip.Compute(theStripResult);
			if(!stripsOk)
				return 0;
			//duplicate last index
			matIDItr->second.begin()->second.push_back(matIDItr->second.begin()->second.back());
			//insert the first index of the newly computed strip at the end
			matIDItr->second.begin()->second.push_back(*((UINT*)theStripResult.StripRuns));
			//copy the new strip
			std::copy((UINT*)theStripResult.StripRuns, (UINT*)theStripResult.StripRuns+theStripResult.StripLengths[0], std::back_inserter(matIDItr->second.begin()->second));
		}
		result += matIDItr->second.begin()->second.size();
	}
	return result;
}
*/
/*
void DffExporter::GetAdjacencyFromMesh()
{
	INode *sNode = this->iInterface->GetSelNode(0);    
	if(!sNode) 
		return;    
	BOOL deleteTri = FALSE;    
	TriObject *pTri = GetTriObjectFromNode(sNode,iInterface->GetTime(), deleteTri);
	Mesh& _mesh= pTri->mesh;

	std::vector<UINT> faceIndices;
	for (int i=0; i < _mesh.numFaces; ++i)
	{
		for (int j=0; j < 3; ++j)
			faceIndices.push_back(_mesh.faces[i].getVert(j));
	}

	::STRIPERCREATE stripIni;
	stripIni.DFaces= &faceIndices[0];
	stripIni.NbFaces= (faceIndices.size()/3);
	stripIni.AskForWords= false;
	stripIni.ConnectAllStrips= true;
	stripIni.OneSided= false;
	stripIni.SGIAlgorithm= false;
	stripIni.WFaces= NULL;

	::Striper theStrip;
	theStrip.Init(stripIni);
	::STRIPERRESULT theStripResult;
	theStripResult.AskForWords= false;
	theStrip.Compute(theStripResult);
	extensions::gtaHeader theHeader(secIDs::BIN_MESH_PLG, 0, this->fileVersion);
	leStream.write(reinterpret_cast<char*>(&theHeader), sizeof(extensions::gtaHeader));
	UINT binMeshStart= leStream.tellp();
	//place split type, split count and index total in theHeader
	theHeader.identifier= 0x01; theHeader.size= theStripResult.NbStrips; theHeader.fileVersion= theStripResult.StripLengths[0];
	leStream.write(reinterpret_cast<char*>(&theHeader), sizeof(extensions::gtaHeader));
	//write number of indices for this material ID
	leStream.write(reinterpret_cast<char*>(theStripResult.StripLengths), sizeof(UINT));
	//write the material ID
	UINT temp= 0;
	leStream.write(reinterpret_cast<char*>(&temp), sizeof(UINT));
	//write tristrip indices
	leStream.write(reinterpret_cast<char*>(theStripResult.StripRuns), sizeof(UINT)*theStripResult.StripLengths[0]);

	UINT finalAddr= leStream.tellp();
	leStream.seekp(binMeshStart-8, std::ios::beg);
	this->writeVal<UINT>(finalAddr-binMeshStart, sizeof(UINT));
	leStream.seekp(0, std::ios::end);
	if (deleteTri)
		pTri->DeleteThis();
}
*/
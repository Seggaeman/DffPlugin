#include "IndependentFunctions.h"
#include "DffExporter.h"
#include "SkinmeshCallback.h"
#include "resource.h"

void DffExporter::GenPedDFF()
{
	//get the set of bones
	IDerivedObject* dervObjPtr= reinterpret_cast<IDerivedObject*>(this->pedNode->GetObjectRef());
	Modifier* leModif= dervObjPtr->GetModifier(dervObjPtr->NumModifiers()-1);
	std::unordered_set<INode*> nodeSet;
	ISkin* theSkin= (ISkin*)leModif->GetInterface(I_SKIN);
	
	for (int i= 0; i < theSkin->GetNumBones(); ++i)
		nodeSet.insert(theSkin->GetBone(i));

	this->buildClumpList(&nodeSet);

	//write clumps
	for (std::vector<extensions::clumpNodeList>::iterator itr = this->exportClumps.begin(); itr != this->exportClumps.end(); ++itr )
	{
		if(itr->frameNodes.size() < 2)
		{
			MessageBox(this->iInterface->GetMAXHWnd(), (this->fNameStrings.filename+TEXT(": Clump ")+extensions::ToString<std::size_t>(std::distance(this->exportClumps.begin(), itr))+TEXT(" did not export.\nThere should be at least 2 bones.")).c_str(), TEXT("Export failed"), MB_ICONINFORMATION);
			continue;
		}

		leStream.WriteHeader(enms::secIDs::CLUMP_ID, this->fileVersion);	//clump header
		leStream.WriteHeader(enms::secIDs::STRUCT, this->fileVersion);	//clump struct
		leStream.GenericWrite<UINT>(1);

		//GTA 3 does not use lights and cameras
		if (this->fileVersion != enms::gameConstants::GTA_IIIA && this->fileVersion != enms::gameConstants::GTA_IIIB && this->fileVersion != enms::gameConstants::GTA_IIIC)
		{
			leStream.GenericWrite<UINT>(0);
			leStream.GenericWrite<UINT>(0);			//cameras not supported
		}
		leStream.FlushSectionSize();		//clump struct size
		this->WritePedFrames(itr);

		//write geometry list
		leStream.WriteHeader(enms::secIDs::GEOMETRY_LIST, this->fileVersion);
		
		//struct
		leStream.WriteHeader(enms::secIDs::STRUCT, this->fileVersion);
		leStream.GenericWrite<UINT>(1); //geometry count
		leStream.FlushSectionSize();

		//geometry
		this->ProcessGeometry();
		leStream.FlushSectionSize();		//geometry list size

		//write atomic
		leStream.WriteHeader(enms::secIDs::ATOMIC, this->fileVersion);

		//atomic struct
		leStream.WriteHeader(enms::secIDs::STRUCT, this->fileVersion);
		leStream.GenericWrite<UINT>(2);	//frame index
		leStream.GenericWrite<UINT>(0);	//geometry index
		leStream.GenericWrite<UINT>(5);	//unknown 1
		leStream.GenericWrite<UINT>(0);	//unknown 2
		leStream.FlushSectionSize();	//struct size

		//write extension
		leStream.WriteHeader(enms::secIDs::EXTENSION,this->fileVersion);

		//right to render
		leStream.WriteHeader(enms::secIDs::RIGHT_TO_RENDER, this->fileVersion);
		leStream.GenericWrite<UINT64>(0x100000116);

		leStream.FlushSectionSize();	//RTR size
		leStream.FlushSectionSize();	//extension size
		leStream.FlushSectionSize();	//atomic size

		//empty extension
		leStream.WriteHeader(enms::secIDs::EXTENSION, this->fileVersion);

		leStream.FlushSectionSize();		//extension size
		leStream.FlushSectionSize();		//clump size
	}
}

void DffExporter::PExpSingleMAX()
{
	extensions::SkinmeshCallback leCallback(this);
	for (auto i= 0; i < this->iInterface->GetRootNode()->NumChildren(); ++i)
	{
		if (leCallback.filter(this->iInterface->GetRootNode()->GetChildNode(i)))
		{
			this->pedNode= this->iInterface->GetRootNode()->GetChildNode(i);
			leStream.open(this->fNameStrings.filename, std::ios::out | std::ios::in | std::ios::beg | std::ios::binary | std::ios::trunc);
			this->GenPedDFF();
			leStream.close();
			break;
		}
	}
}

void DffExporter::PExpMultiple()
{
	extensions::SkinmeshCallback leCallback(this);
	//get the path
	extensions::GetShortFilename(this->fNameStrings.filename, &this->fNameStrings.filename);
	std::size_t length= this->fNameStrings.filename.length();
	
	for (auto i= 0; i < this->iInterface->GetRootNode()->NumChildren(); ++i)
	{
		if (leCallback.filter(this->iInterface->GetRootNode()->GetChildNode(i)))
		{
			this->pedNode= this->iInterface->GetRootNode()->GetChildNode(i);
			this->fNameStrings.filename= this->fNameStrings.filename.substr(0, length);
			this->fNameStrings.filename +=  this->pedNode->GetName() + std::tstring(TEXT(".dff"));
			leStream.open(this->fNameStrings.filename, std::ios::out | std::ios::in | std::ios::beg | std::ios::binary | std::ios::trunc);
			this->GenPedDFF();
			leStream.close();
		}
	}
}

void DffExporter::GenSkinPLG()
{
	//MessageBox(NULL, extensions::ToString<LONG_PTR>(pedNode->GetNodeLong()).c_str(), TEXT("Caption"), MB_OK);
	IDerivedObject* dervObjPtr= reinterpret_cast<IDerivedObject*>(this->pedNode->GetObjectRef());
	Modifier* leModif= dervObjPtr->GetModifier(dervObjPtr->NumModifiers()-1);
	ISkin* theSkin= (ISkin*)leModif->GetInterface(I_SKIN);
	ISkinContextData* skData= theSkin->GetContextInterface(this->pedNode);

	//write header
	leStream.WriteHeader(enms::secIDs::SKIN_PLG, this->fileVersion);
	leStream.GenericWrite<BYTE>(theSkin->GetNumBones());
	leStream.GenericWrite<BYTE>(26);	//special indices count
	leStream.GenericWrite<BYTE>(3);
	leStream.GenericWrite<BYTE>(0);
	
	//write special indices
	for (BYTE i=0; i < 26; ++i)
		leStream.GenericWrite<BYTE>(i);

	//acquire vertex specific bone indices and weights
	std::vector<std::set<std::pair<BYTE, float>, extensions::greater>> boneInfo;

	//MessageBox(NULL, extensions::ToString<std::size_t>(skData->GetNumPoints()).c_str(), "num points", MB_OK);
	for (DWORD i= 0; i < skData->GetNumPoints(); ++i)
	{
		boneInfo.push_back(this->GetAvgBoneWeights(i, skData));
	}
	//MessageBox(NULL, extensions::ToString<std::size_t>(boneInfo.size()).c_str(), "num points", MB_OK);
	//write bone indices
	for (auto itr= boneInfo.begin(); itr < boneInfo.end(); ++itr)
	{
		for (auto setItr= itr->begin(); setItr != itr->end(); ++setItr)
		{
			leStream.GenericWrite<BYTE>(setItr->first);
		}


		for (int i= itr->size(); i < 4; ++i)
			leStream.GenericWrite<BYTE>(0);
	}

	//write bone weights
	for (auto itr= boneInfo.begin(); itr < boneInfo.end(); ++itr)
	{
		for (auto setItr= itr->begin(); setItr != itr->end(); ++setItr)
		{
			leStream.GenericWrite<float>(setItr->second);
		}


		for (int i= itr->size(); i < 4; ++i)
			leStream.GenericWrite<float>(0.0f);
	}

	//write inverse bone matrices
	Matrix3 skinMatrix= pedNode->GetNodeTM(iInterface->GetTime());
	skinMatrix.Invert();
	for (int i= 0; i < theSkin->GetNumBones(); ++i)
	{
		Matrix3 theMatrix= theSkin->GetBone(i)->GetNodeTM(iInterface->GetTime());
		theMatrix *= skinMatrix;
		theMatrix.Invert();
		theMatrix.Orthogonalize();
		theMatrix.NoScale();
		if (this->fileVersion != enms::gameConstants::GTA_SA)
			leStream.GenericWrite<DWORD>(66);
		for (int i= 0; i < 4; ++i)
		{
			for (int j= 0; j<3; ++j)
				leStream.GenericWrite<float>(theMatrix.GetRow(i)[j]);

			leStream.GenericWrite<DWORD>(0);
		}
	}

	//write 3 unknown floats in GTA SA peds
	if (this->fileVersion == enms::gameConstants::GTA_SA)
	{
		for (auto i= 0; i < 3; ++i)
			leStream.GenericWrite<float>(666.0f);
	}

	leStream.FlushSectionSize();
}

std::set<std::pair<BYTE, float>, extensions::greater> DffExporter::GetAvgBoneWeights(std::size_t vertID, ISkinContextData* skinCtx)
{
	std::set<std::pair<BYTE, float>, extensions::greater> boneList;
	for (int boneVIdx =0; boneVIdx < skinCtx->GetNumAssignedBones(vertID); ++boneVIdx)
	{
		std::pair<BYTE, float> thePair(skinCtx->GetAssignedBone(vertID, boneVIdx), skinCtx->GetBoneWeight(vertID, boneVIdx));

		boneList.insert(thePair);
	}

	//remove extra bones, average their weights (in case there are more than 4) and add them to bones 0-3.
	union {
		float wTotal;
		float wAvg;
	} vals;

	vals.wTotal= 0.0f;
	DWORD numExtras= boneList.size()-4;

	while (boneList.size() > 4)
	{
		auto endItr= boneList.rbegin();
		vals.wTotal += endItr->second;
		boneList.erase(*endItr);
	}

	vals.wAvg= numExtras ? vals.wTotal/numExtras : 0.0f;

	for (auto startItr= boneList.begin(); startItr != boneList.end(); ++startItr)
	{
		std::pair<BYTE, float> thePair(startItr->first, startItr->second+vals.wAvg);
		boneList.erase(startItr);
		boneList.insert(thePair);
	}
	return boneList;
}

void DffExporter::WritePedFrames(const std::vector<extensions::clumpNodeList>::iterator& itr, const bool& peds)
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
	leStream.GenericWrite<UINT>(itr->frameNodes.size()+1);

	//write frame having default parameters i.e no parent and identity matrix
	Matrix3 defMatrix(TRUE);
	for (int i =0 ; i<4; ++i)
		for (int j= 0; j < 3 ; ++j)
			leStream.GenericWrite<float>(defMatrix.GetRow(i)[j]);

	leStream.GenericWrite<UINT>(0xffffffff);
	leStream.GenericWrite<DWORD>(131075);

	for(std::map<DWORD, INode*>::iterator frameItr= itr->orderedFrames.begin(); frameItr != itr->orderedFrames.end(); ++ frameItr)
	{
#ifdef LOGFILE
			logFile << clumpIdx << ": Writing frames" << std::endl;
#endif
		Matrix3 parentMatrix, exportTM;
		if (peds && std::distance(itr->orderedFrames.begin(), frameItr) == 1)
		{
			exportTM= pedNode->GetNodeTM(iInterface->GetTime());
		}
		else
		{
			parentMatrix= (frameItr->second)->GetParentNode()->GetNodeTM(iInterface->GetTime());
			parentMatrix.Invert();
			exportTM= (frameItr->second)->GetNodeTM(0)*parentMatrix;
		}

		exportTM.Orthogonalize();
		exportTM.NoScale();
		//write rotation and translation
		for (int i= 0; i < 4; ++i)
			for (int j= 0; j< 3; ++j)
				leStream.GenericWrite<float>(exportTM.GetRow(i)[j]);
#ifdef LOGFILE
	Matrix3 skinMatrix= (frameItr->second)->GetNodeTM(0);
	this->logFile << std::endl << std::distance(itr->orderedFrames.begin(), frameItr)+1 << std::endl   << skinMatrix.GetRow(0)[0] << ", " << skinMatrix.GetRow(0)[1] << ", " << skinMatrix.GetRow(0)[2] << std::endl
				  << skinMatrix.GetRow(1)[0] << ", " << skinMatrix.GetRow(1)[1] << ", " << skinMatrix.GetRow(1)[2] << std::endl
				  << skinMatrix.GetRow(2)[0] << ", " << skinMatrix.GetRow(2)[1] << ", " << skinMatrix.GetRow(2)[2] << std::endl
				  << skinMatrix.GetRow(3)[0] << ", " << skinMatrix.GetRow(3)[1] << ", " << skinMatrix.GetRow(3)[2] << std::endl;
#endif
		std::map<INode*, DWORD>::iterator parentItr= itr->frameNodes.find(frameItr->second->GetParentNode());
		//if root node write 0 for parent and an integer value of 3
		if (parentItr == itr->frameNodes.end())
		{
			leStream.GenericWrite<UINT>(0);
		}
		//else write index of (parent+1) and 3 (because all actual parents are offset by 1)
		else
		{
			leStream.GenericWrite<DWORD>(parentItr->second+1);
		}
		leStream.GenericWrite<INT>(3);
	}
	//write struct size
	leStream.FlushSectionSize();
	//write extension containing specified root node name
	leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
	leStream.GenericWrite<UINT>(0, true);
	leStream.GenericWrite<UINT>(this->fileVersion);
	if (fNameStrings.rootNode != TEXT(""))
	{
		this->leStream.GenericWrite<UINT>(enms::secIDs::FRAME);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		leStream.writeANSIString(fNameStrings.rootNode);
		leStream.FlushSectionSize();
	}
	leStream.FlushSectionSize();

	//write extension sections, HAnimPLG and frame names
	for(std::map<DWORD, INode*>::iterator frameItr= itr->orderedFrames.begin(); frameItr != itr->orderedFrames.end(); ++ frameItr)
	{
#ifdef LOGFILE
			logFile << clumpIdx << ": Writing frame extension" << std::endl;
#endif
		//write extension
		leStream.GenericWrite<UINT>(enms::secIDs::EXTENSION);
		leStream.GenericWrite<UINT>(0, true);
		leStream.GenericWrite<UINT>(this->fileVersion);
		//write HAnimPlg
		this->WriteHAnimPLG(itr, frameItr);
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

void DffExporter::WriteHAnimPLG(const std::vector<extensions::clumpNodeList>::iterator& itr, std::map<DWORD, INode*>::iterator& frameItr)
{
	union
	{
		DWORD BoneID;
		DWORD BoneNo;
		DWORD BoneType;
	} boneDat;

	leStream.WriteHeader(enms::secIDs::HANIM_PLG, this->fileVersion);
	leStream.GenericWrite<UINT>(256);
	if(frameItr->second->GetUserPropInt(extensions::GetRscString(IDS_BONEID).c_str(), (int&)boneDat.BoneID))
		leStream.GenericWrite<DWORD>(boneDat.BoneID);
	else
		leStream.GenericWrite<DWORD>(std::distance(itr->orderedFrames.begin(), frameItr));

	if (frameItr->first)
	{
		leStream.GenericWrite<UINT>(0);
	}
	else
	{
		leStream.GenericWrite<UINT>(itr->frameNodes.size());
		leStream.GenericWrite<UINT>(0);
		leStream.GenericWrite<UINT>(36);
		for (auto itr1= itr->orderedFrames.begin(); itr1 != itr->orderedFrames.end(); ++itr1)
		{
			if(itr1->second->GetUserPropInt(extensions::GetRscString(IDS_BONEID).c_str(), (int&)boneDat.BoneID))
				leStream.GenericWrite<DWORD>(boneDat.BoneID);
			else
				leStream.GenericWrite<DWORD>(std::distance(itr->orderedFrames.begin(), itr1));

			if(itr1->second->GetUserPropInt(extensions::GetRscString(IDS_BONEINDEX).c_str(), (int&)boneDat.BoneNo))
				leStream.GenericWrite<DWORD>(boneDat.BoneNo);
			else
				leStream.GenericWrite<DWORD>(std::distance(itr->orderedFrames.begin(), itr1));

			if(itr1->second->GetUserPropInt(extensions::GetRscString(IDS_BONETYPE).c_str(), (int&)boneDat.BoneType))
				leStream.GenericWrite<DWORD>(boneDat.BoneType);
			else
				leStream.GenericWrite<DWORD>((DWORD)enms::boneTypes::Unknown);
		}
	}
	leStream.FlushSectionSize();
}

void DffExporter::ProcessGeometry()
{
	IDerivedObject* theDervd= reinterpret_cast<IDerivedObject*>(this->pedNode->GetObjectRef());
	Object* objBackup= theDervd->GetObjRef();
	IDerivedObject* newDervd= CreateDerivedObject(objBackup);	//Creaing a temporary reference to objBackup so that it doesn't get deleted after reference count drops to zero
	TriObject* newTri= (TriObject*) objBackup->ConvertToType(iInterface->GetTime(), Class_ID(EDITTRIOBJ_CLASS_ID,0));
	Mesh meshBackup= newTri->mesh;
	theDervd->ReferenceObject(newTri);
	ProcessGeometry2(pedNode, newTri->mesh);
	newTri->NotifyDependents(FOREVER, PART_GEOM|PART_TOPO, REFMSG_CHANGE);
	pedNode->EvalWorldState(iInterface->GetTime());		//very important. Updates status of skin modifier
	WriteGeometry(pedNode, newTri->mesh);
	if (objBackup != newTri)
	{
		theDervd->ReferenceObject(objBackup);
		///newTri->DeleteThis();	Has already been deleted
	}
	else
	{
		newTri->mesh= meshBackup;
		newTri->NotifyDependents(FOREVER, PART_GEOM|PART_TOPO, REFMSG_CHANGE);		
	}
	newDervd->DeleteThis();
	//objBackup->DeleteThis(); crash at this point perhaps because reference counting has automatically deleted it
}
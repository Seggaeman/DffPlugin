#pragma once
#include "StdAfx.h"
#include "DffDatatypes.h"
#include "DffStream.h"
//#include "resource.h"
//============================================================
// Class descriptor declaration
#ifdef MAX_RECENT
	#define PLUGIN_VERSION VERSION_3DSMAX
#else
	#define PLUGIN_VERSION ((MAX_RELEASE_R12<<16)+(MAX_API_NUM_R120<<8)+MAX_SDK_REV)
#endif

//============================================================
// The plug-in definition



class DffExporter 
    : public SceneExport
{
    //============================================================
    // Fields

    // Use member fields only when data is not managed by the parameter block
    // or the reference manager. 
    //TODO("Add member fields");

public:
	extensions::DffStream leStream, UVAnimStream;
#ifdef LOGFILE
	DWORD clumpIdx;
	std::fstream logFile;
#endif
	int progressPercent;
	::Interface* iInterface;
	::ExpInterface* eiInterface;
	DWORD pipeOptions, pipeOptionsBackup;
	DWORD nvcHeader, nvcHeaderBackup;
	std::bitset<23> expData, expDataBackup;	//exportPreset: vehicle= false; ped= true
	extensions::GeoLighting geoLighting, geoLightingBackup;
	INode* pedNode;
	std::vector<extensions::clumpNodeList> exportClumps;
	UINT fileVersion;
	enms::fileCreation fCreation;

	struct {
		std::tstring CFGfile;		//configuration file
		std::tstring filename;		//generated file name(s)
		std::tstring colString;		//collision file name(s)
		std::tstring UVanimFile;	//temporary UV anim file(s)
		std::tstring MAXfile;		//3ds max scene file
		std::tstring rootNode;
	} fNameStrings;

	std::set<WORD> condensedIDs;	//set of IDs actually used in the material. Use only ID 0 if the TriObject hasn't been assigned a multimaterial
	std::unordered_set<::Mtl*> animatedMats;		//set of UV animated materials
	std::map<::TimeValue, extensions::UVanimKey> UVAnimKeys;		//animation keys
	std::unordered_set<INode*>* MAXFileNodes;
	HWND hWndArray[4];	//main dialog, file creation, geometry options and miscellaneous

	//Dialog functions
	//=======================
	inline void _stdcall InitCombobox();
	inline void __stdcall SetSpinner(const DWORD& which, const DWORD& buddyBox, const float& value);
	inline float __stdcall GetSpinnerVal(const DWORD& which);
	inline int _stdcall AddRollup(IRollupWindow* theWindow, const DWORD& dialogID, DLGPROC rollupProc, const std::tstring& caption, const LPARAM& custPointer= NULL, const DWORD& rollupFlags= 0);
	inline void _stdcall ControlsUpdate();
	inline void SetSource();
	inline void SetFileVersion();
	inline void SetCreationMode();
	inline void SetExpPreset();
	inline void SetMiscellanenous();
	inline void SetFaceType();
	inline void SetAddOptions();
	inline void SetData();
	inline void SetVerSpecific();
	inline void WriteINIFile();
	inline bool ExportCheck();
	
	//TriObject* GetTriObjectFromNode(INode *node, TimeValue t,  int &deleteIt);
	void SplitBySmoothGroup(::Mesh& triMesh);
	void RebuildMesh(::Mesh& triMesh);	//returns normals
	inline void SplitFaceList(::Mesh& theMesh, std::map<DWORD, std::vector<UINT>>& faceSplits);
	inline void WriteTriList(::Mesh& theMesh, std::map<DWORD, std::vector<UINT>>& triLists);
	inline void WriteTriStrip(::Mesh& theMesh, std::map<DWORD, std::vector<UINT>>& triLists);
	//DWORD GenerateZappyStripsExt(::Mesh& theMesh, std::map<DWORD, std::map<DWORD, std::vector<UINT>>>& faceSplits);	//uses material ID and element as keys
	DWORD GenerateNVStrips(std::map<DWORD, std::vector<UINT>>& triLists, std::map<DWORD, std::vector<UINT>>& triStrips);
	DWORD GenerateZappyStrips(std::map<DWORD, std::vector<UINT>>& triLists, std::map<DWORD, std::vector<UINT>>& triStrips);
	inline void WriteMaterialList(INode* leNode);
	//DWORD DffExporter::GenerateStrips(std::map<DWORD, std::vector<UINT>>& faceSplits); //returns total indices
	inline void WriteGeomExtension(::Mesh& theMesh);
	inline void WriteBinMeshPLG(::Mesh& theMesh);
	inline void WriteMeshExtension();
	void WriteNVC(::Mesh& theMesh);
	inline bool IsChannelFlagged(const int& channel);
	//void BreakVertices(::Mesh& theMesh);
	void buildClumpList(std::unordered_set<INode*>* nodeSet= nullptr);
	void FilterClumpNodes(INode*& leNode, extensions::clumpNodeList& theList);
	inline void WriteFile();
	//inline void LockFile();
	void UpdateFile();
	void WriteUVAnimDic();
	void WriteAnimAnimation(Mtl* leMateriel);
	inline void RetrieveAnimInfo(::TimeValue theTime, ::StdUVGen* leUVGen);
	void WriteClump(const std::vector<extensions::clumpNodeList>::iterator& itr);
	void WriteFrameList(const std::vector<extensions::clumpNodeList>::iterator& itr);
	void WriteGeometryList(const std::vector<extensions::clumpNodeList>::iterator& itr);
	void WriteGeometry(INode* theNode, Mesh& theMesh);
	void WriteGTAMaterial(Mtl* material, INode* theNode);	//Use material= NULL to write a default material;
	void WriteMatTextures(::BitmapTex* diffTex, ::BitmapTex* alphaTex, bool useDiffuse, bool useAlpha);
	inline void WriteMaterialExtension(Mtl* material, INode* theNode);
	void WriteMaterialEffects(Mtl* material, INode* theNode);
	void WriteReflection(Mtl* material);
	void WriteSpecular(Mtl* material);
	void WriteUVAnimPlg(Mtl* leMateriel);
	void WriteAtomic(const std::vector<extensions::clumpNodeList>::iterator& itr, const DWORD& shift);
	inline void WriteAtomicMatEffects(INode* theNode);
	inline void WriteAtomicRTR(INode* theNode);
	inline void WritePipeline();
	void WriteCollision();
	inline void WriteLights(const std::vector<extensions::clumpNodeList>::iterator& itr);
	void OverrideExpOptions(Mtl* theMaterial);
	inline void WriteMorphPLG();
	void GenPedDFF();
	void WritePedFrames(const std::vector<extensions::clumpNodeList>::iterator& itr, const bool& peds=true);
	void GenSkinPLG();
	void WriteHAnimPLG(const std::vector<extensions::clumpNodeList>::iterator& itr, std::map<DWORD, INode*>::iterator& frameItr);
	inline void ProcessGeometry(INode* theNode);	//default (vehicles and map objects)
	inline void ProcessGeometry();		//peds only
	inline void ProcessGeometry2(INode* leNode, Mesh& leMesh);
	std::set<std::pair<BYTE, float>, extensions::greater> GetAvgBoneWeights(std::size_t vertID, ISkinContextData* skinCtx);
	void VExpMultiple();
	void PExpMultiple();
	void PExpSingleMAX();
	void UVAnimUpdate();
	//inline void DateTimeStamp();
	//void GetAdjacencyFromMesh();

	//Maxscript command line
	void RegexParse(const MCHAR* ipString);

	DffExporter();
	~DffExporter();
	virtual ClassDesc2* GetClassDesc();
	virtual unsigned int Version();
	virtual int ExtCount();
	virtual const MCHAR* Ext(int n);
	virtual const MCHAR* LongDesc();
	virtual const MCHAR* ShortDesc(); 
	virtual const MCHAR* AuthorName();
	virtual const MCHAR* CopyrightMessage();
	virtual void ShowAbout(HWND hWnd);
	virtual const MCHAR* OtherMessage1();
	virtual const MCHAR* OtherMessage2();
	virtual int DoExport (const MCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
	virtual BOOL SupportsOptions(int ext, DWORD options);
};

//======================================================================
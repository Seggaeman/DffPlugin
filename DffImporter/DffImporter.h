/**********************************************************************
 *<
	FILE: DffImporter.h

	DESCRIPTION:  GTA III/Vice City/San Andreas .dff importer

	AUTHOR: seggaeman

	CONTRIBUTORS: DexX (dff format information), The Hero(dff format information), REspawn (dff format information), Kam (GTA_Material.ms), gtamodding.com

 *>	Copyright (c) 2011, All Rights Reserved.
 **********************************************************************/


//#define EDIT_NORMALS_CLASS_ID Class_ID(0x4aa52ae3, 0x35ca1cde)
//from the looks of it definition and declaration of the class descriptor shouldn't be separated.

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <maxscript\maxscript.h>
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
#include <iskin.h>
#include <iSkinPose.h>
#include <modstack.h>
//#include <IPathConfigMgr.h>
//#include <IFileResolutionManager.h>
#include <bitmap.h>
#include <pbbitmap.h>
#include <AssetManagement\AssetUser.h>
#include <IPathConfigMgr.h>
//#include <IPathConfigMgr.h>
//#include <Shlwapi.h>
//#include <AssetManagement\iassetmanager.h>
//#include <polyobj.h>
//#include <mnmesh.h>
//#include "resource.h"
///Compile with "No character set" to prevent linker errors. TCHAR* is equivalent to CHAR*
//#define VERSION_3DSMAX ((MAX_RELEASE<<16)+(MAX_API_NUM<<8)+MAX_SDK_REV)
//#define MAX_2005 ((MAX_RELEASE_R7<<16)+(MAX_API_NUM_R70<<8)+MAX_SDK_REV)		//3ds max 7
//#define MAX_2006 ((MAX_RELEASE_R8<<16)+(MAX_API_NUM_R80<<8)+MAX_SDK_REV)		//3ds max 8
//#define MAX_2007 ((MAX_RELEASE_R9<<16)+(MAX_API_NUM_R90<<8)+MAX_SDK_REV)		//3ds max 9
//#define MAX_2008 ((MAX_RELEASE_R10<<16)+(MAX_API_NUM_R100<<8)+MAX_SDK_REV)
//define MAX_2009 ((MAX_RELEASE_R11<<16)+(MAX_API_NUM_R110<<8)+MAX_SDK_REV)
#ifdef MAX_RECENT
	#define PLUGIN_VERSION	VERSION_3DSMAX
#else
	#define PLUGIN_VERSION ((MAX_RELEASE_R12<<16)+(MAX_API_NUM_R120<<8)+MAX_SDK_REV)
#endif
//#define MAX_2011 ((MAX_RELEASE_R13<<16)+(MAX_API_NUM_R130<<8)+MAX_SDK_REV)
//#define LOGFILE
//game version
#define GTA_IIIA 0x0003ffff //GTA III?
#define GTA_IIIB 0x0800ffff //GTA III
#define GTA_IIIC 0x00000310 //GTA III
#define GTA_VCA 0x0c02ffff //GTA VC
#define GTA_VCB 0x1003ffff //GTA VC
#define GTA_SA 0x1803ffff //GTA SA
#define GTAMAT_A 0x48238272 
#define GTAMAT_B 0x48206285
#define GTA_MULTI_MATERIAL_CLASS_ID Class_ID(0x744721ba, 0x31ec52e0)
//RW section IDs
/*
#define CLUMP_ID 0x10
#define HANIM_PLG 0x11e
#define FRAME 0x253f2fe
#define GEOMETRY_LIST= 0x1a
#define EXTENSION= 0x03
#define MATERIAL= 0x07
#define MATERIAL_LIST= 0x08
#define TEXTURE= 0x06
#define STRUCT= 0x01
#define MATERIAL_EFFECTS_PLG= 0x120
#define REFLECTION_MATERIAL= 0x253f2fc
#define SPECULAR_MATERIAL= 0x253f2f6
*/

namespace extensions
{

#pragma pack(1)
	struct GTAHeader
	{
		UINT identifier;
		UINT size;
		UINT fileVersion;

		bool checkVersion()
		{
			return (this->fileVersion == GTA_IIIA || this->fileVersion == GTA_IIIB || this->fileVersion == GTA_IIIC
				|| this->fileVersion == GTA_VCA || this->fileVersion == GTA_VCB || this->fileVersion == GTA_SA);
		}

		bool operator == (const UINT& secID)
		{
			return (this->identifier == secID && this->checkVersion());
		}

		bool operator == (const GTAHeader& leHeader)
		{
			bool checkFileVersion2 = (leHeader.fileVersion == this->fileVersion);
			return (this->identifier == leHeader.identifier && checkVersion() && checkFileVersion2);
		}

		//Supplies additional security by also testing the file version. If file version doesn't corresond to #defines, returns 0
		UINT getIdentifier()
		{
			if (checkVersion())
				return this->identifier;
			else
				return 0;
		}
	};

	struct FrameStructStart
	{
		GTAHeader structHeader;
		UINT frameCount;
	};

	struct ClumpStruct
	{
		GTAHeader structHeader;
		UINT objectCount;
		UINT lightCount;
		UINT cameraCount;
	};

	struct VertexColors
	{
		BYTE red;
		BYTE blue;
		BYTE green;
		BYTE alpha;
	};

	struct FaceBAFC
	{
		USHORT bDat;
		USHORT aDat;
		USHORT fDat; //material ID
		USHORT cDat;
	};

	struct UVData
	{
		float Udat;
		float Vdat;
	};

	struct GeoParams
	{
		GTAHeader structHeader;
		BYTE flags;		//flags and unused correspond to upper and lower bytes if flags is read as USHORT.
		BYTE unused1;
		BYTE numUVs;
		BYTE unused2;
		UINT faceCount;
		UINT vertexCount;
		UINT frameCount;
	};

	struct BSphereXYZR
	{
		float X, Y, Z, R;
	};

	struct Vector3
	{
		float x, y, z;
	};

	struct BinMeshPLGTotal
	{
		UINT isTriStrip;
		UINT tSplitCount;
		UINT tIndexCount;
	};

	struct BinMeshPLGSub
	{
		UINT indexCount;
		UINT materialID;
	};

	struct GTAFrame
	{
		float transMatrix[4][3];
		unsigned int parent;
		UINT unknown;
	};

	struct AtomicEntryStruct
	{
		GTAHeader atomicStructHeader;
		UINT frameIndex;
		UINT geoIndex;
		UINT unknown1;
		UINT unknown2;
	};

	struct UVAnimKey
	{
		float timeOver4800;
		DWORD trash1;
		float UTiling;
		float VTiling;
		DWORD trash2;
		float negUOffset;
		float VOffset;
		DWORD iMinus2;
	};

	struct GeoListStruct
	{
		GTAHeader structHeader;
		UINT gCount;
	};

	struct MaterialStruct
	{
		GTAHeader structHeader;
		UINT unknown1;
		extensions::VertexColors matColors;
		UINT unknown2;
		UINT textureCount;
		float ambient;
		float specular;
		float diffuse;
	};

	struct TextureSectionStruct
	{
		extensions::GTAHeader structHeader;
		BYTE filterFlags[2];
		USHORT unknown;
	};

	struct LightStruct
	{
		GTAHeader structHeader;
		float attenuation;
		float RGBcolor[3];
		float FallSize;		//not FallSize.
		USHORT unknown;		//always 0x3?
		USHORT lightType;	//0x80 for omnilights, 0x81 otherwise according to Kam
	};

	struct MatEffectsPlgStart
	{
		UINT unknown1;
		UINT unknown2;
	};

	//specular parameters in reflection material; sort of a contradiction but that's how this info is read in Kam's scripts
	struct ReflectionMaterial
	{
		Vector3 specColor;
		float specAlpha;
		float specRefBlend;
		UINT unknown;
	};

	struct SpecularInfo
	{
		float glossiness;
		char specTexture[24];
	};

	struct UVAnimPLGStruct
	{
		GTAHeader structHeader;
		UINT unknown;
		char AnimMaterial[32];
	};

	struct AnimAnimationStart
	{
		GTAHeader animHeader;
		UINT const256;
		UINT const449;
		UINT numKeys;
		UINT unknown1;
		float numKeysAsFloatOver4800;
		UINT unknown2;
		char AnimMaterial[64];
	};

	struct FaceIndexStruct
	{
		DWORD x;
		DWORD y;
		DWORD z;

		FaceIndexStruct(const DWORD& idX, const DWORD& idY, const DWORD& idZ) : x(idX), y(idY), z(idZ)
		{
		}

		FaceIndexStruct();
	};

	struct HAnimPLGStart
	{
		DWORD const256;
		int boneID;
		DWORD boneCount;
	};

	struct HAnimPLGItem
	{
		DWORD boneID;
		DWORD boneNo;
		DWORD boneType;

		//comparison function for boneID. Works but deprecated (i.e unused)
		//bool operator == (const DWORD& boneIdentifier) const
		//{
		//return this->boneID == boneIdentifier;
		//}
	};

	struct SkinPlgStart
	{
		BYTE boneCount;
		BYTE spIndicesCount;
		BYTE unknown1;
		BYTE unknown2;
	};

	struct BoneVertexIndices
	{
		BYTE boneVertex[4];
	};

	struct VertexWeights
	{
		float weight[4];
	};

	struct InverseBoneMatrix
	{
		float matrixRow1[3];
		DWORD padding1;
		float matrixRow2[3];
		DWORD padding2;
		float matrixRow3[3];
		DWORD padding3;
		float matrixRow4[3];
		DWORD padding4;
	};

	struct InverseBoneMatrixEx
	{
		DWORD trash;
		float matrixRow1[3];
		DWORD padding1;
		float matrixRow2[3];
		DWORD padding2;
		float matrixRow3[3];
		DWORD padding3;
		float matrixRow4[3];
		DWORD padding4;
	};
#pragma pack()

	struct MaxGeometry
	{
		::TriObject* maxMesh;
		Mtl* material;
	};

	struct MaxObject
	{
		::INode* theNode;
		LightStruct* leGlobeStruct;
	};

	struct SkinInfo
	{
		extensions::SkinPlgStart* skStart;
		BYTE* specialIndices;
		extensions::BoneVertexIndices* skBVIndices;
		extensions::VertexWeights* skVWeights;
		extensions::InverseBoneMatrix* ibm;
		extensions::InverseBoneMatrixEx* ibmEx;
	};

	enum class GTAMaterialConstants : ParamID
	{
		amb, spc, dif, color, colormap, use_colormap, alpha, alphamap, use_alphamap, Reflection, reflectionmap, use_reflectionmap, specular, specularmap, use_specularmap, spec_alpha, spec_power, blend, colhprIdx,
		use_RF, use_SAS, use_SI, UVanimexp, OverrideMatEffects, UInteger1, UInteger2
	};

	enum class GTAMultiMaterialConstants : ParamID
	{
		FaceType, RenderPipe, MeshExt, SkyMipMap, MorphPLG, NVC, UVMap1, UVMap2, VColors, Normals, DynLight, MMC, NVCHeader, Amb, Diff, Spec
	};

	enum class GTAPipelineSet : UINT
	{
		ReflectiveMaterial = 0x53f20098, Vehicle = 0x53F2009A, NightVertexColors = 0x53F2009C
	};
}
namespace std
{
#ifdef UNICODE
	typedef std::wstring tstring;
	typedef std::wstringstream tstringstream;
#else
	typedef std::string tstring;
	typedef std::stringstream tstringstream;
#endif
}
//============================================================
// The plug-in definition

class DffImporter 
    : public SceneImport
{

    //============================================================
    // Fields

    // Use member fields only when data is not managed by the parameter block
    // or the reference manager. 
    //TODO("Add member fields");
public:
	enum secIDs: UINT {ATOMIC= 0x14, CLUMP_ID= 0x10, HANIM_PLG= 0x11e, FRAME= 0x253f2fe, FRAME_LIST= 0x0E, GEOMETRY_LIST= 0x1a, EXTENSION= 0x03, MATERIAL= 0x07, MATERIAL_LIST= 0x08
					   ,TEXTURE= 0x06, GEOMETRY= 0x0F, STRUCT= 0x01, MATERIAL_EFFECTS_PLG= 0x120, REFLECTION_MATERIAL= 0x253f2fc,SPECULAR_MATERIAL= 0x253f2f6 
					   ,BIN_MESH_PLG= 0x50e, NATIVE_DATA_PLG= 0x0510, SKIN_PLG= 0x0116, MESH_EXTENSION= 0x0253F2FD, NIGHT_VERTEX_COLORS= 0x0253F2F9
					   ,MORPH_PLG= 0x0105, TWO_DFX_PLG= 0x0253F2F8, UVANIM_DIC= 0x2B, ANIM_ANIMATION= 0x1B, UV_ANIM_PLG= 0x135, RIGHT_TO_RENDER= 0x1f, PIPELINE_SET = 0x0253f2f3
					   ,STRING= 0x02, LIGHT= 0x12};
	enum boneTypes: UINT {DEFORMABLE= 0x0, NUB_BONE= 0x01, UNKNOWN= 0x02, RIGID= 0x03};
	extensions::ClumpStruct* leClump;					//object, light, and camera counts
	extensions::FrameStructStart* frameInfo;	//number of frames
	extensions::GeoListStruct* geoCount;
	extensions::HAnimPLGItem* HAnimDataMappings;		//pointer to ped HAnimPLG info

	char* buffer;
	unsigned int bufIndex;
	std::streamsize fileSize;
	bool IsCharDff, OkToContinue, noIssues;
	std::map<std::string, std::vector<extensions::UVAnimKey*>> UVAnimList;
	extensions::SkinInfo SkinData;	//to store skin data. Assuming a single mesh per ped model.
	std::map<int, DWORD> HAnimIDFrameKeys; //Bone ID and frame No pair
	std::map<DWORD, int> HAnimFrameIDKeys; //same info with data and key reversed.

	ImpInterface* dffImpInterface;			//private pointer to import interface
	Interface* dffInterface;				//private pointer to interface
	extensions::GTAFrame* frameList;		//better results when this is used as a private member
	char** frameNames;
	std::map<std::tuple<USHORT,USHORT,USHORT>, USHORT> faceRefInfoMap;		//use as reference for setting face indices (provided face count in object is <= 0x10000)
	extensions::MaxGeometry* eMeshList;		//must be a container of TriObject pointers
	extensions::AtomicEntryStruct** atomList;
	extensions::MaxObject* maxObjectList;
	//std::fstream logfile;

	template <typename var> inline var readNumeric()
	{
		var output;
		dffReader.read(reinterpret_cast<char*>(&output), sizeof(var));
		return output;
	}

	template <typename var> inline var readBuffer(const unsigned int& inc)
	{
		var temp= reinterpret_cast<var>(buffer+bufIndex);
		bufIndex += inc;
		return temp;
	}

	/*
	template <typename var> inline var readBuffer()
	{
		var temp= reinterpret_cast<var>(buffer[bufIndex]);
		bufIndex += sizeof(var);
		return temp;
	}
	*/

	template<typename var> inline const std::tstring ToString(const var& input,bool hexRepresentation = false)
	{
		std::tstringstream temp;
		if (hexRepresentation)
			temp << std::hex;

		temp << input;
		return temp.str();
	}

public:
	DffImporter();
	~DffImporter();
	virtual int ExtCount();
	virtual const TCHAR* Ext(int);
	virtual const TCHAR* LongDesc();
	virtual const TCHAR* ShortDesc();
	virtual const TCHAR* AuthorName();
	virtual const TCHAR* CopyrightMessage();
	virtual const TCHAR* OtherMessage1();
	virtual const TCHAR* OtherMessage2();
	virtual unsigned int Version();
	virtual void ShowAbout(HWND);
	virtual int ZoomExtents();
	virtual int DoImport(const TCHAR*, ImpInterface*, Interface*, BOOL);
	virtual ClassDesc2* GetClassDesc();

private:
	inline extensions::GTAHeader* readHeader();
	extensions::GTAHeader* parseFile(); //optional index for sections embedded in  extensions e.g. frame names
	void readFrameList();
	void readGeometryList();
	void parseFrameListExtension(const int& frameIndex);
	void parseMaterialList(const int& geoIndex);
	void readGeometryExtension(::Mesh& theMesh,const extensions::GeoParams* meshParams, const UINT& geoIndex);
	void readGeometry(const UINT& gIndex);
	void readBinMesh(::Mesh& theMesh, const UINT& sectionSize, const extensions::GeoParams* meshParams);
	inline void setFaceInfo(::Mesh& theMesh, const UINT& faceIndex, const extensions::GeoParams* meshParams, const extensions::BinMeshPLGSub* theSub, const extensions::FaceIndexStruct* faceIndices);
	void readAtomic(const INT& objectIndex);
	void buildScene();
	void readLights();
	void readCameras();
	void readNVC(const UINT& geoIndex);
	void readMaterial(const int& matID, Mtl* const multiMat);
	void readTextures(::Mtl* theMaterial);
	void readMatExtension(::Mtl* theMaterial);
	void readUVAnims();
	void buildSceneChar();
	void addSkinModifier(INode* leNode, const UINT& geoIndex);
	inline void setTextureFaces(::Mesh& theMesh, const UINT& faceIndex, const extensions::GeoParams* meshParams);
	inline void setTextureMap(::BitmapTex* theTex);
	inline void addHAnimProps(const DWORD& frameIndex);
	void readHAnimPLG(const UINT& frameIndex);
	void readSkin(const UINT& geoIndex, const extensions::GeoParams* meshParams, const extensions::GTAHeader* skinHeader);
	inline Matrix3 getBoneTransform(const DWORD& index);
};

//======================================================================

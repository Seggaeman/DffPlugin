/**********************************************************************
 *<
	FILE: extensions.h

	DESCRIPTION:  GTA San Andreas .dff importer

	AUTHOR: seggaeman

	CONTRIBUTORS: DexX (dff format information), Kam (GTA_Material.ms), REspawn (dff format information)

 *>	Copyright (c) 2011, All Rights Reserved.
 **********************************************************************/
#pragma once


#include "StdAfx.h"

namespace enms
{
enum fileCreation: BYTE {app= 0, cnew= 1, mult= 2};
enum rollupPage : BYTE {MainWindow= 0, FileCreation= 1, GeometryOptions= 2, Miscellaneous= 3};
enum secIDs: UINT {ATOMIC= 0x14, CLUMP_ID= 0x10, HANIM_PLG= 0x11e, FRAME= 0x253f2fe, FRAME_LIST= 0x0E, GEOMETRY_LIST= 0x1a, EXTENSION= 0x03, MATERIAL= 0x07, MATERIAL_LIST= 0x08
					,TEXTURE= 0x06, GEOMETRY= 0x0F, STRUCT= 0x01, MATERIAL_EFFECTS_PLG= 0x120, REFLECTION_MATERIAL= 0x253f2fc,SPECULAR_MATERIAL= 0x253f2f6 
					,BIN_MESH_PLG= 0x50e, NATIVE_DATA_PLG= 0x0510, SKIN_PLG= 0x0116, MESH_EXTENSION= 0x0253F2FD, NIGHT_VERTEX_COLORS= 0x0253F2F9
					,MORPH_PLG= 0x0105, TWO_DFX_PLG= 0x0253F2F8, UVANIM_DIC= 0x2B, ANIM_ANIMATION= 0x1B, UV_ANIM_PLG= 0x135, RIGHT_TO_RENDER= 0x1f
					,STRING= 0x02, LIGHT= 0x12, COLLISION= 0x253f2fa, PIPELINE= 0x253f2f3, ZMOD_LOCK= 0xf21e};
enum renderPipeline: DWORD {NOTHING= 0, REFLECTIVE_BUILDING= 0x53f20098, NIGHT_VERTEX= 0x53F2009C, VEHICLE= 0x53F2009A};
enum gameConstants: UINT { GTA_IIIA= 0x3ffff, GTA_IIIB= 0x800ffff, GTA_IIIC= 0x310, GTA_VCA= 0xc02ffff, GTA_VCB= 0x1003ffff, GTA_SA= 0x1803ffff, 
							GTA_MATA= 0x48238272 , GTA_MATB= 0x48206285};
enum expDatIDs: UINT {tStrip= 0, useZappy= 1, meshExt= 2, skyMipMap= 3, morphPlg= 4, nightVColors= 5, UVmap1= 6, UVmap2= 7, 
						   dayVColors= 8, normals= 9, dynLighting= 10, MMC= 11, lights= 12, colAutoseek = 13, expSelected= 14, incFrozen= 15, incHidden= 16, 
						   expPreset = 17, source = 18, incHAnimPLG = 19, UVanims = 20, forceOrigin = 21, overrideGTAMultimaterial = 22 };
enum boneTypes: BYTE {Deformable= 0, NubBone= 1,  Unknown= 2, Rigid = 3}; 
}

namespace extensions
{
#pragma pack(1)
struct gtaHeader
{
	UINT identifier;
	UINT size;
	UINT fileVersion;

	inline bool checkVersion()
	{
		return (this->fileVersion== enms::gameConstants::GTA_IIIA || this->fileVersion == enms::gameConstants::GTA_IIIB || this->fileVersion == enms::gameConstants::GTA_IIIC 
			|| this->fileVersion == enms::gameConstants::GTA_VCA ||this->fileVersion == enms::gameConstants::GTA_VCB	|| this->fileVersion== enms::gameConstants::GTA_SA); 
	}

	inline bool operator == (const UINT& secID)
	{
		return (this->identifier == secID && this->checkVersion());
	}

	inline bool operator == (const gtaHeader& leHeader)
	{
		bool checkFileVersion2 = (leHeader.fileVersion== this->fileVersion); 
		return (this->identifier == leHeader.identifier && checkVersion() && checkFileVersion2);
	}

	//Supplies additional security by also testing the file version. If file version doesn't corresond to #defines, returns 0
	inline UINT getIdentifier()
	{
		if (checkVersion())
			return this->identifier;
		else
			return 0;
	}

	inline gtaHeader() { }

	inline gtaHeader(UINT id, UINT sz, UINT fileV)
	{
		this->identifier= id; this->size= sz; this->fileVersion= fileV;
	}
};

struct frameStructStart
{
	gtaHeader structHeader;
	UINT frameCount;
};

struct clumpStruct
{
	gtaHeader structHeader;
	UINT objectCount;
	UINT lightCount;
	UINT cameraCount;
};

struct vertexColors
{
	BYTE red;
	BYTE blue;
	BYTE green;
	BYTE alpha;
};

struct faceBAFC
{
	USHORT bDat;
	USHORT aDat;
	USHORT fDat; //material ID
	USHORT cDat;
};

struct uvData
{
	float Udat;
	float Vdat;
};

struct geoParams
{
	gtaHeader structHeader;
	BYTE flags;		//flags and unused correspond to upper and lower bytes if flags is read as USHORT.
	BYTE unused1;
	BYTE numUVs;
	BYTE unused2;
	UINT faceCount;
	UINT vertexCount;
	UINT frameCount;
};

struct geomBounding
{
	float X,Y,Z,R;
	DWORD unknown;
	DWORD hasNormals;
};

struct vector3
{
	float x,y,z;
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

struct gtaFrame
{
	float transMatrix[4][3];
	unsigned int parent;
	UINT unknown;
};

struct atomicEntryStruct
{
	gtaHeader atomicStructHeader;
	UINT frameIndex;
	UINT geoIndex;
	UINT unknown1;
	UINT unknown2;
};

struct UVanimKey
{
	float timeOver4800;
	DWORD trash1;
	float UTiling;
	float VTiling;
	DWORD trash2;
	float negUOffset;
	float VOffset;
	DWORD iMinus1;
};

struct geoListStruct
{
	gtaHeader structHeader;
	UINT gCount;
};

struct materialStruct
{
	gtaHeader structHeader;
	UINT unknown1;
	extensions::vertexColors matColors;
	UINT unknown2;
	UINT textureCount;
	float ambient;
	float diffuse;
	float specular;
};

struct textureSectionStruct
{
	extensions::gtaHeader structHeader;
	BYTE filterFlags[2];
	USHORT unknown;
};

struct lightStruct
{
	gtaHeader structHeader;
	float attenuation;
	float RGBcolor[3];
	float FallSize;
	float unknown;
};

struct matEffectsPlgStart
{
	UINT unknown1;
	UINT unknown2;
};

//specular parameters in reflection material; sort of a contradiction but that's how this info is read in Kam's scripts
struct reflectionMaterial
{
	vector3 specColor;
	float specAlpha;
	float specRefBlend;
	UINT unknown;
};

struct specularInfo
{
	float glossiness;
	char specTexture[24];
};

struct UVanimPLGStruct
{
	gtaHeader structHeader;
	UINT unknown;
	char AnimMaterial[32];
};

struct AnimAnimationStart
{
	gtaHeader animHeader;
	UINT const256;
	UINT const449;
	UINT numKeys;
	UINT unknown1;
	float numKeysAsfloatOver4800;
	UINT unknown2;
	char AnimMaterial[64];
};

struct GeoLighting
{
	float ambient;
	float diffuse;
	float specular;

	float& operator[] (const int& index)
	{
		switch (index)
		{
			case 0:
				return ambient;
			break;

			case 1:
				return this->diffuse;
			break;

			default:
				return this->specular;
			break;
		}
	}
};

struct clumpNodeList
{
	//std::unordered_set<INode*> allNodes;
	std::map<INode*, DWORD> frameNodes;		//because frames must be in hierarchichal order :(
	std::map<DWORD, INode*> orderedFrames;
	std::set<DWORD> orderedGeometry;
	std::set<DWORD> orderedLights;
};

struct inverseBoneMatrix
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

struct faceRef
{
	DWORD faceIndex;
	BYTE vertexID;	//0, 1 or 2


	inline faceRef() {}

	inline faceRef(const DWORD& fID, const BYTE& vertID) : faceIndex(fID), vertexID(vertID)
	{

	}
};

struct faceRefEq
{
	inline bool operator() (const faceRef& one, const faceRef& two) const
	{
		return ((one.faceIndex == two.faceIndex) && (one.vertexID == two.vertexID));
	}
};

struct faceRefHash
{
	inline std::size_t operator () (const faceRef& candidate) const
	{
		//return std::unordered_set<DWORD>().hash_function()(candidate.faceIndex*3+candidate.vertexID);
		return std::hash<DWORD>()(candidate.faceIndex*3+candidate.vertexID);
	}
};


struct vertexInfo
{
	std::unordered_set<faceRef, extensions::faceRefHash, extensions::faceRefEq> faceRefs;	//the faces referencing this vertex
	std::map<int, std::unordered_set<DWORD>> mapVertices;		//the equivalent map vertices. Map channel number is used as key
};

struct greater : std::binary_function <std::pair<BYTE,float>,std::pair<BYTE,float>,bool> 
{
	bool operator() (const std::pair<BYTE,float>& x, const std::pair<BYTE,float>& y) const
    {
		return x.second > y.second;
	}
};

#pragma pack()
}
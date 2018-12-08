#pragma once

#include "StdAfx.h"
//=====================================================================
namespace extensions
{
class SceneEnumProc : public ::ITreeEnumProc
{
	public:
		::INodeTab* theNodes;
	
	inline SceneEnumProc() : theNodes(nullptr)
	{

	}

	inline SceneEnumProc(::ExpInterface* eiIn, INodeTab* input)
	{
		theNodes= input;
		eiIn->theScene->EnumTree(this);
	}

	inline int SceneEnumProc::callback(INode* node)
	{
		theNodes->AppendNode(node);
		return TREE_CONTINUE;
	}
};
}
#pragma once

#include "StdAfx.h"
#include "DffExporter.h"

namespace extensions {
class SkinmeshCallback : public HitByNameDlgCallback
{
private:
	std::vector<INode*> bones;
	DffExporter* leExporteur;

public:
	SkinmeshCallback(DffExporter*);
	~SkinmeshCallback();

#ifdef MAX_2013
	virtual const MCHAR* dialogTitle();
	virtual const MCHAR* buttonText();
#else
	virtual MCHAR* dialogTitle();
	virtual MCHAR* buttonText();
#endif
	virtual BOOL singleSelect();
	virtual BOOL useFilter();
	virtual int filter(INode* node);
	virtual BOOL useProc();
	virtual void proc(INodeTab& nodeTab);
	virtual BOOL doCustomHilite();
	virtual BOOL doHilite(INode* node);
	virtual BOOL showHiddenAndFrozen();
};
}
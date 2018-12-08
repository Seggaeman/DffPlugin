#include "StdAfx.h"
#include "resource.h"
#include "SkinmeshCallback.h"

extensions::SkinmeshCallback::SkinmeshCallback(DffExporter* theExporter) : leExporteur(theExporter)
{

}

extensions::SkinmeshCallback::~SkinmeshCallback()
{

}

#ifdef MAX_2013
const
#endif
MCHAR* extensions::SkinmeshCallback::dialogTitle()
{
	return TEXT("Select skin mesh");
}

#ifdef MAX_2013
const
#endif
MCHAR* extensions::SkinmeshCallback::buttonText()
{
	return TEXT("Select");
}

BOOL extensions::SkinmeshCallback::singleSelect()
{
	return TRUE;
}

BOOL extensions::SkinmeshCallback::useFilter()
{
	return TRUE;
}

int extensions::SkinmeshCallback::filter(INode* node)
{
	auto checkBones= [] (ISkin* leSkin)->UINT {
		UINT unlinkedNodeCount= 0;
		
		for (auto i= 0; i < leSkin->GetNumBones(); ++i)
		{
			if (leSkin->GetBone(i)->GetParentNode()->IsRootNode())
				unlinkedNodeCount++;
		}
		return unlinkedNodeCount;
	};

	Object* objPtr= node->GetObjectRef();
	if ((leExporteur->expData[(UINT)enms::expDatIDs::expSelected] && ! node->Selected())
		|| (SendMessage(GetDlgItem(leExporteur->hWndArray[enms::rollupPage::FileCreation], IDC_INCFROZEN), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED && node->IsFrozen())
		|| (SendMessage(GetDlgItem(leExporteur->hWndArray[enms::rollupPage::FileCreation], IDC_INCHIDDEN), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED && node->IsHidden()) )
	{
		return FALSE;
	}

	else if (objPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* dervObjPtr= reinterpret_cast<IDerivedObject*>(objPtr);
		Modifier* leModif= dervObjPtr->GetModifier(dervObjPtr->NumModifiers()-1);
		if (leModif->ClassID() == SKIN_CLASSID)
		{
			ISkin* leSkin= (ISkin*)leModif->GetInterface(I_SKIN);

			if (checkBones(leSkin) == 1 && leSkin->GetNumBones() >= 2)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL extensions::SkinmeshCallback::useProc()
{
	return TRUE;
}

void extensions::SkinmeshCallback::proc(INodeTab& nodeTab)
{
	leExporteur->pedNode= nodeTab[0];
	//MessageBox(leExporteur->iInterface->GetMAXHWnd(), TEXT("test"), TEXT("test"), MB_OK);
	SendMessage(GetDlgItem(leExporteur->hWndArray[enms::rollupPage::FileCreation], IDC_SKINMESH), WM_SETTEXT, NULL, (LPARAM) nodeTab[0]->GetName());
}

BOOL extensions::SkinmeshCallback::doCustomHilite()
{
	return FALSE;
}

BOOL extensions::SkinmeshCallback::doHilite(INode* node)
{
	return FALSE;
}


BOOL extensions::SkinmeshCallback::showHiddenAndFrozen()
{
	return TRUE;
}
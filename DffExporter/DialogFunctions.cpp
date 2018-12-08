#include "StdAfx.h"
#include "DffExporter.h"
#include "resource.h"
#include "IndependentFunctions.h"
#include "SkinmeshCallback.h"
//****************************************************************************************************//
//****//Dialog functions******************************************************************************//
//****************************************************************************************************//

extern HINSTANCE hDllInstance;
INT_PTR CALLBACK FileDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeomDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MiscDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
//export check
bool DffExporter::ExportCheck()
{
	bool status= true;

	if (this->expData[enms::expDatIDs::source])
	{
		if (this->fNameStrings.MAXfile == TEXT(""))
		{
			MessageBox(iInterface->GetMAXHWnd(), TEXT("Scene file not specified"), TEXT("Issues"), MB_ICONINFORMATION);
			status= false;
		}
		else if(iInterface->GetRootNode()->NumberOfChildren() && MessageBox(iInterface->GetMAXHWnd(), TEXT("This operation will delete existing scene objects!\nClick CANCEL to abort."), 
							TEXT("Warning"), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
			status= false;
	}
	else if (this->fCreation != enms::fileCreation::mult && this->expData[enms::expDatIDs::expPreset] == true && this->pedNode == nullptr)
	{
		MessageBox(iInterface->GetMAXHWnd(), TEXT("Skin mesh not specified"), TEXT("Issues"), MB_ICONINFORMATION);
		status= false;
	}
	return status;
}

INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_INITDIALOG:
		{
			wchar_t buffer[1000];
			LoadStringW(hDllInstance, IDS_ABOUT, buffer, 1000);
			SendMessageW(GetDlgItem(hWnd, IDC_EDIT1), WM_SETTEXT, NULL, (LPARAM)buffer);
		}
		break;

		default:
			return FALSE;
		break;
	}
	return TRUE;
}
//Dialog procedure
INT_PTR CALLBACK toolDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffExporter* leExporteur= (DffExporter*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (Message)
	{
		case WM_INITDIALOG:
		{
			leExporteur = (DffExporter*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)leExporteur);
			SetFocus(hWnd); // For some reason this was necessary.  DS-3/4/96
			leExporteur->hWndArray[enms::MainWindow]= hWnd;

			//add rollups
			IRollupWindow* leWindow= ::GetIRollup(GetDlgItem(hWnd, IDC_ROLLUPS));

			leExporteur->AddRollup(leWindow, IDD_FILECREATION, FileDlgProc, extensions::GetRscString(IDS_FILE).c_str(), lParam);
			leExporteur->AddRollup(leWindow, IDD_GEOMOPTIONS, GeomDlgProc, extensions::GetRscString(IDS_GEOMETRY).c_str(), lParam, APPENDROLL_CLOSED);
			leExporteur->AddRollup(leWindow, IDD_MISC, MiscDlgProc, extensions::GetRscString(IDS_MISC).c_str(), lParam, APPENDROLL_CLOSED);
			leExporteur->AddRollup(leWindow, IDD_ABOUT, AboutDlgProc, TEXT("About"), NULL, APPENDROLL_CLOSED);
			leExporteur->InitCombobox();

			//Add the rollup pages before calling this
			leWindow->Show();
			leWindow->SetBorderless(TRUE);

			leExporteur->SetSource();
			leExporteur->SetFileVersion();
			leExporteur->SetCreationMode();
			leExporteur->SetExpPreset();
			leExporteur->SetVerSpecific();
			leExporteur->SetAddOptions();
			leExporteur->SetFaceType();
			leExporteur->SetData();
			leExporteur->SetMiscellanenous();
			leExporteur->ControlsUpdate();
			::ReleaseIRollup(leWindow);	//finish everything before calling this
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					leExporteur->WriteINIFile();
					if (! leExporteur->ExportCheck() )
						break;
					EndDialog(hWnd, IDOK);
				}
				break;

				case IDCANCEL:
					EndDialog(hWnd, IDCANCEL);
				break;
			}
		break;

		default:
			return FALSE;
		break;
	}
	return TRUE;
}

INT_PTR CALLBACK FileDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffExporter* leExporteur= (DffExporter*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (Message)
	{
		case WM_INITDIALOG:
			leExporteur= (DffExporter*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			SetFocus(hWnd); // For some reason this was necessary.  DS-3/4/96
			leExporteur->hWndArray[enms::rollupPage::FileCreation]= hWnd;

			//set text
			SendMessage(GetDlgItem(hWnd, IDC_FILENAME), WM_SETTEXT, NULL, (LPARAM)extensions::GetShortFilename(leExporteur->fNameStrings.filename).c_str());
		break;

		case WM_COMMAND:
			switch (wParam)
			{
				case IDC_VERSIONS | (CBN_SELCHANGE << 16):
					UpdateWindow(hWnd);	//repaint necessary
				break;

				//if any of these was selected update UI
				case (IDC_SCENE | (BN_CLICKED << 16)):
				case (IDC_MAXFILE | (BN_CLICKED << 16)):
				case (IDC_CREATENEW | (BN_CLICKED << 16)):
				case (IDC_APPEND | (BN_CLICKED << 16)):
				case (IDC_MULTIFILE | (BN_CLICKED << 16)):
				case (IDC_VEHICLE | (BN_CLICKED << 16 )): 
				case (IDC_PED | (BN_CLICKED << 16)):
				case (IDC_INCHANIM | (BN_CLICKED << 16)):
				case (IDC_INCFROZEN | (BN_CLICKED << 16)):
				case (IDC_INCHIDDEN | (BN_CLICKED << 16)):
					leExporteur->ControlsUpdate();
				break;

				case (IDC_SELECTMAX | (BN_CLICKED << 16)):
				{
					SendMessage(GetDlgItem(hWnd, IDC_SELECTMAX), WM_SETTEXT, NULL, (LPARAM)TEXT("Select .max file"));
					leExporteur->fNameStrings.MAXfile= TEXT("");
					MSTR szFilename, path= ::IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_SCENE_DIR);

					::FilterList lefilter;
					lefilter.Append(TEXT("Scene file(*.max)"));
					lefilter.Append(TEXT("*.max"));
					if (::GetCOREInterface8()->DoMaxOpenDialog(hWnd, TEXT("3ds max scene file"), szFilename, path, lefilter))
					{
						leExporteur->fNameStrings.MAXfile= szFilename;
						SendMessage(GetDlgItem(hWnd, IDC_SELECTMAX), WM_SETTEXT, NULL, (LPARAM)extensions::GetShortFilename(szFilename).c_str());
					}
				}
				break;

				case (IDC_COLFILE | (BN_CLICKED << 16)):
				{
					SendMessage(GetDlgItem(hWnd, IDC_COLFILE), WM_SETTEXT, NULL, (LPARAM)TEXT("Collision file"));
					leExporteur->fNameStrings.colString= TEXT("");
					MSTR szFilename, path= ::IPathConfigMgr::GetPathConfigMgr()->GetDir(APP_SCENE_DIR);

					::FilterList lefilter;
					lefilter.Append(TEXT("Collision file(*.col)"));
					lefilter.Append(TEXT("*.col"));
					if (::GetCOREInterface8()->DoMaxOpenDialog(hWnd, TEXT("3ds max scene file"), szFilename, path, lefilter))
					{
						leExporteur->fNameStrings.colString= szFilename;
						SendMessage(GetDlgItem(hWnd, IDC_COLFILE), WM_SETTEXT, NULL, (LPARAM)extensions::GetShortFilename(szFilename).c_str());
					}
				}
				break;

				case (IDC_SKINMESH | (BN_CLICKED << 16)):
				{
					extensions::SkinmeshCallback leCallback(leExporteur);
					if (!leExporteur->iInterface->DoHitByNameDialog(&leCallback))
					{
						leExporteur->pedNode= nullptr;
						SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)TEXT("Skin mesh"));
					}
				}
				break;

				default:

				break;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK GeomDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffExporter* leExporteur= (DffExporter*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (Message)
	{
		case WM_INITDIALOG:
			leExporteur= (DffExporter*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			SetFocus(hWnd); // For some reason this was necessary.  DS-3/4/96
			leExporteur->hWndArray[enms::rollupPage::GeometryOptions]= hWnd;
		break;

		default:
			return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK MiscDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	DffExporter* leExporteur= (DffExporter*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (Message)
	{
		case WM_INITDIALOG:
			leExporteur= (DffExporter*)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
			SetFocus(hWnd); // For some reason this was necessary.  DS-3/4/96
			leExporteur->hWndArray[enms::rollupPage::Miscellaneous]= hWnd;
		break;

		default:
			return FALSE;
	}
	return TRUE;
}

DWORD WINAPI progressFn(LPVOID arg)
{
	DffExporter* local= (DffExporter*)arg;

	local->iInterface->ProgressUpdate(local->progressPercent, TRUE);
	return 0;
}

//****************************************************************************************************//
//****//Dialog functions******************************************************************************//
//****************************************************************************************************//
void DffExporter::ControlsUpdate()
{
	const HWND& hParentF= this->hWndArray[enms::rollupPage::FileCreation];
	const HWND& hParentG= this->hWndArray[enms::rollupPage::GeometryOptions];
	//select .max file
	if (SendMessage(GetDlgItem(hParentF, IDC_MAXFILE), BM_GETCHECK, NULL, NULL) == BST_CHECKED)
	{
		ShowWindow(GetDlgItem(hParentF, IDC_SELECTMAX), SW_SHOW);
	}
	else
		ShowWindow(GetDlgItem(hParentF, IDC_SELECTMAX), SW_HIDE);

	//autoseek collisions + filename
	if (SendMessage(GetDlgItem(hParentF, IDC_MULTIFILE), BM_GETCHECK, NULL, NULL) == BST_CHECKED)
	{
		ShowWindow(GetDlgItem(hParentF, IDC_AUTOSEEK), SW_SHOW);
		ShowWindow(GetDlgItem(hParentF, IDC_FILENAME), SW_HIDE);
	}
	else
	{
		ShowWindow(GetDlgItem(hParentF, IDC_AUTOSEEK), SW_HIDE);
		ShowWindow(GetDlgItem(hParentF, IDC_FILENAME), SW_SHOW);
	}
	
	//vehicle / ped export options
	if (SendMessage(GetDlgItem(hParentF, IDC_VEHICLE), BM_GETCHECK, NULL, NULL) == BST_CHECKED)
	{
		//show include HAnim checkbox
		ShowWindow(GetDlgItem(hParentF, IDC_INCHANIM), SW_SHOW);
		ShowWindow(GetDlgItem(hParentF, IDC_SKINMESH), SW_HIDE);
		ShowWindow(GetDlgItem(hParentG, IDC_FORCEORIGIN), SW_SHOW);
		//ShowWindow(GetDlgItem(hParentF, IDC_INCFROZEN), SW_SHOW);
		//ShowWindow(GetDlgItem(hParentF, IDC_INCHIDDEN), SW_SHOW);
		if (SendMessage(GetDlgItem(hParentF, IDC_INCHANIM), BM_GETCHECK, NULL, NULL) == BST_CHECKED)
		{
			ShowWindow(GetDlgItem(hParentF, IDC_RNODELABEL), SW_SHOW);
			ShowWindow(GetDlgItem(hParentF, IDC_ROOTNODENAME), SW_SHOW);
		}
		else
		{
			ShowWindow(GetDlgItem(hParentF, IDC_RNODELABEL), SW_HIDE);
			ShowWindow(GetDlgItem(hParentF, IDC_ROOTNODENAME), SW_HIDE);
		}
		if (SendMessage(GetDlgItem(hParentF, IDC_MULTIFILE), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED)
		{
			ShowWindow(GetDlgItem(hParentF, IDC_COLFILE), SW_SHOW);
		}
		else
			ShowWindow(GetDlgItem(hParentF, IDC_COLFILE), SW_HIDE);
	}
	else //peds
	{
		ShowWindow(GetDlgItem(hParentF, IDC_COLFILE), SW_HIDE);
		ShowWindow(GetDlgItem(hParentF, IDC_INCHANIM), SW_HIDE);
		ShowWindow(GetDlgItem(hParentF, IDC_RNODELABEL), SW_SHOW);
		ShowWindow(GetDlgItem(hParentF, IDC_ROOTNODENAME), SW_SHOW);
		ShowWindow(GetDlgItem(hParentG, IDC_FORCEORIGIN), SW_HIDE);
		//ShowWindow(GetDlgItem(hParentF, IDC_INCFROZEN), SW_HIDE);
		//ShowWindow(GetDlgItem(hParentF, IDC_INCHIDDEN), SW_HIDE);
		if(SendMessage(GetDlgItem(hParentF, IDC_MAXFILE), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED 
			&& SendMessage(GetDlgItem(hParentF, IDC_MULTIFILE), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED)
		{
			ShowWindow(GetDlgItem(hParentF, IDC_SKINMESH), SW_SHOW);
		}
		else
		{
			ShowWindow(GetDlgItem(hParentF, IDC_SKINMESH), SW_HIDE);
		}

		//clear skin mesh selection if the mesh in question is frozen or hidden & this doesn't match the checkbox conditions
		if(this->pedNode != nullptr && ((SendMessage(GetDlgItem(hParentF, IDC_INCFROZEN), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED && this->pedNode->IsFrozen())
			|| (SendMessage(GetDlgItem(hParentF, IDC_INCHIDDEN), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED &&  this->pedNode->IsHidden()) ) )
		{
			this->pedNode= nullptr;
			SendMessage(GetDlgItem(hParentF, IDC_SKINMESH), WM_SETTEXT, NULL, (LPARAM)TEXT("Skin mesh"));
		}
	}
//update controls
	UpdateWindow(GetDlgItem(hParentF, IDC_SELECTMAX));
	UpdateWindow(GetDlgItem(hParentF, IDC_AUTOSEEK));
	UpdateWindow(GetDlgItem(hParentF, IDC_COLFILE));
	UpdateWindow(GetDlgItem(hParentF, IDC_SKINMESH));
	UpdateWindow(GetDlgItem(hParentF, IDC_INCHANIM));
	UpdateWindow(GetDlgItem(hParentF, IDC_RNODELABEL));
	UpdateWindow(GetDlgItem(hParentF, IDC_ROOTNODENAME));
	UpdateWindow(GetDlgItem(hParentG, IDC_FORCEORIGIN));
	//UpdateWindow(GetDlgItem(hParentF, IDC_INCFROZEN));
	//UpdateWindow(GetDlgItem(hParentF, IDC_INCHIDDEN));
}

inline int _stdcall DffExporter::AddRollup(IRollupWindow* theWindow, const DWORD& dialogID, DLGPROC rollupProc, const std::tstring& caption, const LPARAM& custPointer, const DWORD& rollupFlags)
{
	::HRSRC hRsrc= FindResource(hDllInstance, MAKEINTRESOURCE(dialogID), MAKEINTRESOURCE(RT_DIALOG));
	::HGLOBAL hTemplate= LoadResource(hDllInstance, hRsrc);
	::DLGTEMPLATE* pTemplate= (DLGTEMPLATE*)LockResource(hTemplate);
	int RollupID= theWindow->AppendRollup(hDllInstance, pTemplate, rollupProc, caption.c_str(), custPointer, rollupFlags);
	UnlockResource(pTemplate);
	FreeResource(hTemplate);
	return RollupID;
}

inline void _stdcall DffExporter::InitCombobox()
{
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTA3A).c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTA3B).c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTA3C).c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTAVCA).c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTAVCB).c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_ADDSTRING, NULL, (LPARAM)extensions::GetRscString(IDS_GTASA).c_str());
}

void __stdcall DffExporter::SetSpinner(const DWORD& which, const DWORD& buddyBox, const float& value)
{
	ISpinnerControl* leSpinner= ::GetISpinner(::GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], which));
	leSpinner->SetAutoScale(TRUE);
	leSpinner->SetLimits(0.0f, 100.0f, TRUE);
	leSpinner->SetValue(value, TRUE);
	leSpinner->LinkToEdit(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], buddyBox), EDITTYPE_POS_FLOAT);
	::ReleaseISpinner(leSpinner);
}

float __stdcall DffExporter::GetSpinnerVal(const DWORD& which)
{
	ISpinnerControl* leSpinner= ::GetISpinner(::GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], which));
	float result= leSpinner->GetFVal();
	::ReleaseISpinner(leSpinner);
	return result;
}

using extensions::GetRscString;

inline void DffExporter::SetSource()
{
	LRESULT value= GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_SOURCE).c_str(), 0, this->fNameStrings.CFGfile.c_str());

	if (value)
	{
		//this->expData[enms::expDatIDs::source] = true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_MAXFILE), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[enms::expDatIDs::source]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_SCENE), BM_SETCHECK, BST_CHECKED, NULL);
	}
}
inline void DffExporter::SetFileVersion()
{
	LRESULT value= ::GetPrivateProfileInt(extensions::GetRscString(IDS_FILE).c_str(), extensions::GetRscString(IDS_FILEV).c_str(), 5, this->fNameStrings.CFGfile.c_str());
	value > 5 ? 5: value;

	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_SETCURSEL, (WPARAM)value, NULL);
/*
	switch (value)
	{
		case 0:
			this->fileVersion= enms::gameConstants::GTA_IIIA;
		break;
				
		case 1:
			this->fileVersion= enms::gameConstants::GTA_IIIB;
		break;

		case 2:
			this->fileVersion= enms::gameConstants::GTA_IIIC;
		break;

		case 3:
			this->fileVersion= enms::gameConstants::GTA_VCA;
		break;

		case 4:
			this->fileVersion= enms::gameConstants::GTA_VCB;
		break;

		default:
			this->fileVersion= enms::gameConstants::GTA_SA;
		break;
	}*/
}

inline void DffExporter::SetCreationMode()
{
	LRESULT value= ::GetPrivateProfileInt(extensions::GetRscString(IDS_FILE).c_str(), GetRscString(IDS_OUTPUT).c_str(), 0, this->fNameStrings.CFGfile.c_str());

	switch(value)
	{
		case 0:
			//this->fCreation= enms::fileCreation::cnew;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_CREATENEW), BM_SETCHECK, BST_CHECKED, NULL);
			break;

		case 1:
			//this->fCreation= enms::fileCreation::app;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_APPEND), BM_SETCHECK, BST_CHECKED, NULL);
			break;

		default:
			//this->fCreation= enms::fileCreation::mult;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_MULTIFILE), BM_SETCHECK, BST_CHECKED, NULL);
			break;
	}

	//autoseek collisions checkbox
	value= ::GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_AUTOCOL).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[enms::expDatIDs::colAutoseek]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_AUTOSEEK), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[enms::expDatIDs::colAutoseek]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_AUTOSEEK), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

}


inline void DffExporter::SetExpPreset()
{
	LRESULT value= GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_EXPPRESET).c_str(), 0, this->fNameStrings.CFGfile.c_str());

	if(value)
	{
		//this->expData[(UINT)enms::expDatIDs::expPreset]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_PED), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::expPreset]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VEHICLE), BM_SETCHECK, BST_CHECKED, NULL);
	}

	//include hanims checkbox
	value= GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCHANIM).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHANIM), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHANIM), BM_SETCHECK, BST_UNCHECKED, NULL);
	}
}

inline void DffExporter::SetVerSpecific()
{
	TCHAR valString[10];
	float value;

	GetPrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_AMBIENT).c_str(), TEXT("100.0"), valString, 10, this->fNameStrings.CFGfile.c_str());
	value= _ttof(valString);
	this->SetSpinner(IDC_SPINAMB, IDC_EDITAMB, value); 

	GetPrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_DIFFUSE).c_str(), TEXT("100.0"), valString, 10, this->fNameStrings.CFGfile.c_str());
	value= _ttof(valString);
	this->SetSpinner(IDC_SPINDIFF, IDC_EDITDIFF, value);

	GetPrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_SPECULAR).c_str(), TEXT("100.0"), valString, 10, this->fNameStrings.CFGfile.c_str());
	value= _ttof(valString);
	this->SetSpinner(IDC_SPINSPEC, IDC_EDITSPEC, value);
}

inline void  DffExporter::SetAddOptions()
{
	LRESULT value= ::GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCFROZEN).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::incFrozen]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCFROZEN), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCFROZEN), BM_SETCHECK, BST_UNCHECKED, NULL);
		//this->expData[(UINT)enms::expDatIDs::incFrozen]= false;
	}
	value= ::GetPrivateProfileInt(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCHIDDEN).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::incHidden]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHIDDEN), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::incHidden]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHIDDEN), BM_SETCHECK, BST_UNCHECKED, NULL);
	}
}

inline void DffExporter::SetFaceType()
{
	LRESULT value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_FACETYPE).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	switch (value)
	{
		case 0:
			//this->expData[(UINT)enms::expDatIDs::tStrip]= false;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_TLIST), BM_SETCHECK, BST_CHECKED, NULL);
		break;

		case 1:
			//this->expData[(UINT)enms::expDatIDs::tStrip]= true;
			//this->expData[(UINT)enms::expDatIDs::useZappy]= false;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_TNVIDIA), BM_SETCHECK, BST_CHECKED, NULL);
		break;

		default:
			//this->expData[(UINT)enms::expDatIDs::tStrip]= true;
			//this->expData[(UINT)enms::expDatIDs::useZappy]= true;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_TZAPPY), BM_SETCHECK, BST_CHECKED, NULL);
		break;
	}
}

inline void DffExporter::SetData()
{
	LRESULT value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_MMC).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	TCHAR valString[11];
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::MMC]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_MMC), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::MMC]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_MMC), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_UVMAP1).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::MMC]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP1), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::UVmap1]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP1), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_UVMAP2).c_str(), 0, this->fNameStrings.CFGfile.c_str());

	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::UVmap2]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP2), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::UVmap2]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP2), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_VCOLORS).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::dayVColors]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_VCOLORS), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::dayVColors]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_VCOLORS), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_DYNLIGHT).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::dynLighting]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_DYNLIGHTING), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::dynLighting]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_DYNLIGHTING), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NORMALS).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::normals]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NORMALS), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::normals]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NORMALS), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NCOLORS).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::nightVColors]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NVCOLORS), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::nightVColors]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NVCOLORS), BM_SETCHECK, BST_UNCHECKED, NULL);
	}
	//value = ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NVCHEADER).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	GetPrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NVCHEADER).c_str(), TEXT("0x00"), valString, 11, this->fNameStrings.CFGfile.c_str());
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_EDIT_NVCHEADER), WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(valString));

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_LIGHTS).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::lights]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_LIGHTS), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::lights]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_LIGHTS), BM_SETCHECK, BST_UNCHECKED, NULL);
	}

	value= ::GetPrivateProfileInt(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_FORCEORIGIN).c_str(), 0, this->fNameStrings.CFGfile.c_str());
	if (value)
	{
		//this->expData[(UINT)enms::expDatIDs::forceOrigin]= true;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_FORCEORIGIN), BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		//this->expData[(UINT)enms::expDatIDs::forceOrigin]= false;
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_FORCEORIGIN), BM_SETCHECK, BST_UNCHECKED, NULL);
	}
}

inline void  DffExporter::SetMiscellanenous()
{
	//set render pipeline.
	LRESULT value= ::GetPrivateProfileInt(GetRscString(IDS_MISC).c_str(), GetRscString(IDS_RPIPELINE).c_str(), 0, this->fNameStrings.CFGfile.c_str());

	switch (value)
	{
		case 0:
			//this->pipeOptions= enms::renderPipeline::NOTHING;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_NOTHING), BM_SETCHECK, BST_CHECKED, NULL);
		break;

		case 1:
			//this->pipeOptions= enms::renderPipeline::REFLECTIVE_BUILDING;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_REFBDG), BM_SETCHECK, BST_CHECKED, NULL);
		break;

		case 2:
			//this->pipeOptions= enms::renderPipeline::NIGHT_VERTEX;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_NVC), BM_SETCHECK, BST_CHECKED, NULL);
		break;

		default:
			//this->pipeOptions= enms::renderPipeline::VEHICLE;
			SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_VEH), BM_SETCHECK, BST_CHECKED, NULL);
		break;
	}
	//set state of GTA Multimaterial override checkbox.
	value = GetPrivateProfileInt(GetRscString(IDS_MISC).c_str(), GetRscString(IDS_OVRR_GTA_MULTIMAT).c_str(), 1, this->fNameStrings.CFGfile.c_str());
	if (value)
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_OVRR_GTA_MULTIMAT), BM_SETCHECK, BST_CHECKED, NULL);
	else
		SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_OVRR_GTA_MULTIMAT), BM_SETCHECK, BST_UNCHECKED, NULL);
}

inline void DffExporter::WriteINIFile()
{
//================================================================//
//file panel
	LRESULT value;
	MCHAR valString[2];

	//Get file version
	value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VERSIONS), CB_GETCURSEL, NULL, NULL);

	switch (value)
	{
		case 0:
			this->fileVersion= enms::gameConstants::GTA_IIIA;
		break;

		case 1:
			this->fileVersion= enms::gameConstants::GTA_IIIB;
		break;

		case 2:
			this->fileVersion= enms::gameConstants::GTA_IIIC;
		break;

		case 3:
			this->fileVersion= enms::gameConstants::GTA_VCA;
		break;

		case 4:
			this->fileVersion= enms::gameConstants::GTA_VCB;
		break;

		default:
			this->fileVersion= enms::gameConstants::GTA_SA;
		break;
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_FILEV).c_str(), extensions::ToString<UINT>(value).c_str(), this->fNameStrings.CFGfile.c_str());
	//source (scene or max file)

	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_SCENE), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->expData[enms::expDatIDs::source] = false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[enms::expDatIDs::source] = true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_SOURCE).c_str(), valString, this->fNameStrings.CFGfile.c_str());


	//append / create new
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_CREATENEW), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->fCreation= enms::fileCreation::cnew;
		_tcscpy(valString, TEXT("0"));
	}

	else if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_APPEND), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->fCreation= enms::fileCreation::app;
		_tcscpy(valString, TEXT("1"));
	}

	else
	{
		this->fCreation= enms::fileCreation::mult;
		_tcscpy(valString, TEXT("2"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_OUTPUT).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//ped complete.
	if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_VEHICLE), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::expPreset]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::expPreset]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_EXPPRESET).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//inclusion of HAnimPLG in vehicles or map objects
	if (SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHANIM), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED)
	{
		this->expData[enms::expDatIDs::incHAnimPLG] = false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[enms::expDatIDs::incHAnimPLG]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCHANIM).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Autoseek collisions
	if (SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_AUTOSEEK), BM_GETCHECK,NULL,NULL) == BST_UNCHECKED)
	{
		this->expData[enms::expDatIDs::colAutoseek]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[enms::expDatIDs::colAutoseek]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_AUTOCOL).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//frozen
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCFROZEN), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::incFrozen]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::incFrozen]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCFROZEN).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//hidden
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_INCHIDDEN), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::incHidden]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::incHidden]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_FILE).c_str(), GetRscString(IDS_INCHIDDEN).c_str(), valString, this->fNameStrings.CFGfile.c_str());

//==========================================================================================================================================================//
//Geometry panel

	//face type
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_TLIST), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::tStrip]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_TNVIDIA), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::tStrip]= true;
		this->expData[(UINT)enms::expDatIDs::useZappy]= false;
		_tcscpy(valString, TEXT("1"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::tStrip]= true;
		this->expData[(UINT)enms::expDatIDs::useZappy]= true;
		_tcscpy(valString, TEXT("2"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_FACETYPE).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//MMC
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_MMC), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::MMC]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::MMC]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_MMC).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//UV map 1
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP1), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::UVmap1]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::UVmap1]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_UVMAP1).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//UV map 2
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_UVMAP2), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::UVmap2]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::UVmap2]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_UVMAP2).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Vertex colors
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_VCOLORS), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::dayVColors]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::dayVColors]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_VCOLORS).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Dynamic lighting
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_DYNLIGHTING), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::dynLighting]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::dynLighting]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_DYNLIGHT).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Normals
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NORMALS), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::normals]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::normals]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NORMALS).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Night vertex colors
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_NVCOLORS), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::nightVColors]= false;
		_tcscpy(valString, TEXT("0"));
	}

	else
	{
		this->expData[(UINT)enms::expDatIDs::nightVColors]= true;
		_tcscpy(valString, TEXT("1"));
	}
	//get supplied nvcheader
	value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_EDIT_NVCHEADER), WM_GETTEXTLENGTH, 0, 0);
	std::unique_ptr<TCHAR[]> buffer(new TCHAR[value + 1]);
	//get the text.
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_EDIT_NVCHEADER), WM_GETTEXT, value + 1, reinterpret_cast<LPARAM>(buffer.get()));
	this->nvcHeader = _tcstoul(buffer.get(), nullptr, 16);

	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NCOLORS).c_str(), valString, this->fNameStrings.CFGfile.c_str());
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_NVCHEADER).c_str(), buffer.get(), this->fNameStrings.CFGfile.c_str());
	//Lights
	if (SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_LIGHTS), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::lights]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::lights]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_LIGHTS).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//force to origin
	if(SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::GeometryOptions], IDC_FORCEORIGIN), BM_GETCHECK, NULL, NULL) == BST_UNCHECKED)
	{
		this->expData[(UINT)enms::expDatIDs::forceOrigin]= false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[(UINT)enms::expDatIDs::forceOrigin]= true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_FORCEORIGIN).c_str(), valString, this->fNameStrings.CFGfile.c_str());	

	//ambient, diffuse and specular
	this->geoLighting.ambient =  this->GetSpinnerVal(IDC_SPINAMB);
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_AMBIENT).c_str(), extensions::ToString<float>(this->geoLighting.ambient).c_str(), this->fNameStrings.CFGfile.c_str());

	this->geoLighting.diffuse =  this->GetSpinnerVal(IDC_SPINDIFF);
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_DIFFUSE).c_str(), extensions::ToString<float>(this->geoLighting.diffuse).c_str(), this->fNameStrings.CFGfile.c_str());

	this->geoLighting.specular =  this->GetSpinnerVal(IDC_SPINSPEC);
	WritePrivateProfileString(GetRscString(IDS_GEOMETRY).c_str(), GetRscString(IDS_SPECULAR).c_str(), extensions::ToString<float>(this->geoLighting.specular).c_str(), this->fNameStrings.CFGfile.c_str());

	for (int i= 0; i <3; ++i)
		this->geoLighting[i] /= 100.0f;

//==========================================================================================//
//Misc panel
	//Render pipeline
	if ((value= SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_NOTHING), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->pipeOptions= enms::renderPipeline::NOTHING;
		_tcscpy(valString, TEXT("0"));
	}

	else if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_REFBDG), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->pipeOptions= enms::renderPipeline::REFLECTIVE_BUILDING;
		_tcscpy(valString, TEXT("1"));
	}

	else if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_RENDER_NVC), BM_GETCHECK, NULL, NULL)) == BST_CHECKED)
	{
		this->pipeOptions= enms::renderPipeline::NIGHT_VERTEX;
		_tcscpy(valString, TEXT("2"));
	}

	else
	{
		this->pipeOptions= enms::renderPipeline::VEHICLE;
		_tcscpy(valString, TEXT("3"));
	}
	WritePrivateProfileString(GetRscString(IDS_MISC).c_str(), GetRscString(IDS_RPIPELINE).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//Override multimaterial
	if ((value = SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::Miscellaneous], IDC_OVRR_GTA_MULTIMAT), BM_GETCHECK, NULL, NULL)) == BST_UNCHECKED)
	{
		this->expData[enms::expDatIDs::overrideGTAMultimaterial] = false;
		_tcscpy(valString, TEXT("0"));
	}
	else
	{
		this->expData[enms::expDatIDs::overrideGTAMultimaterial] = true;
		_tcscpy(valString, TEXT("1"));
	}
	WritePrivateProfileString(GetRscString(IDS_MISC).c_str(), GetRscString(IDS_OVRR_GTA_MULTIMAT).c_str(), valString, this->fNameStrings.CFGfile.c_str());

	//store backups
	this->expDataBackup= this->expData;
	this->geoLightingBackup= this->geoLighting;
	this->pipeOptionsBackup= this->pipeOptions;
	this->nvcHeaderBackup = this->nvcHeader;
	//store root node name
	SendMessage(GetDlgItem(this->hWndArray[enms::rollupPage::FileCreation], IDC_ROOTNODENAME), WM_GETTEXT, 50, (LPARAM)this->fNameStrings.rootNode.c_str());
	*std::remove(this->fNameStrings.rootNode.begin(), fNameStrings.rootNode.end(), TEXT(' '))= TEXT('\0');
}

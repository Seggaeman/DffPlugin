#pragma once

#include "StdAfx.h"
#include "resource.h"
extern HINSTANCE hDllInstance;

namespace extensions {
inline std::tstring GetShortFilename(const TCHAR* LongFilename, std::tstring* path= NULL)
{
	int index;
	for (index= Tstrlen(LongFilename); index >= 0; --index)
	{
		if (LongFilename[index] == TEXT('\\') )
			break;
	}
	if (path)
		*path= (LongFilename+index);
	return std::tstring(LongFilename+index+1);
}

inline std::tstring GetShortFilename(const std::tstring& LongFilename, std::tstring* path= NULL)
{
	int index;
	for (index= LongFilename.length()-1; index >= 0; --index)
	{
		if (LongFilename[index] == TEXT('\\') )
			break;
	}
	if (path)
		*path= LongFilename.substr(0, index+1);
	return LongFilename.substr(index+1);
}

template<typename var> inline const std::tstring ToString(const var& input)
{
	std::tstringstream temp;
	temp << input;
	return temp.str();
}

inline std::tstring GetRscString(const UINT& uID)
{
	//static TCHAR buffer[50];
	LPWSTR buffer = nullptr;
	int bufferLength = LoadStringW(hDllInstance, uID, reinterpret_cast<LPWSTR>(&buffer), 0);
#ifdef UNICODE
	return buffer;
#else
	//MessageBox(NULL, ToString<int>(bufferLength).c_str(), TEXT("Caption"), MB_OK);
	LPSTR newBuffer = new CHAR[bufferLength+1];
	newBuffer[bufferLength] = TEXT('\0');
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer, -1, newBuffer, (bufferLength)*sizeof(CHAR), NULL, NULL);
	//MessageBox(NULL, ToString<DWORD>(GetLastError()).c_str(), TEXT("Caption"), MB_OK);
	std::string retVal(newBuffer);
	delete[] newBuffer;
	return retVal;
#endif
}

inline void EatSpaces(MCHAR* ipCArray)
{
	std::size_t baseCopyIdx= 0;
	std::for_each(ipCArray, ipCArray+_tcslen(ipCArray), [&](MCHAR& targetChr)->void {
		if (targetChr != TEXT(' '))
		{
			ipCArray[baseCopyIdx]= targetChr;
			++baseCopyIdx;
		}
	});
	ipCArray[baseCopyIdx]= TEXT('\0');
}

inline ::OPENFILENAME CreateOpenFileStruct(TCHAR* szFileName, const TCHAR* filter, const TCHAR* defExt, HWND hWnd)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize= sizeof(ofn);
	ofn.hwndOwner= hWnd;
	ofn.lpstrFilter= filter;
	ofn.lpstrFile= szFileName;
	ofn.nMaxFile= MAX_PATH;
	ofn.Flags= OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt= defExt;
	return ofn;
}

template<typename CustomObj>inline CustomObj* GetObjectFromNode(INode* leNode, TimeValue t, const UINT& classIDA, int& deleteIt)
{
	deleteIt = FALSE;    
	Object *obj = leNode->EvalWorldState(t).obj;    
	if (obj->CanConvertToType(Class_ID(classIDA, 0))) 
	{        
		CustomObj* custObj = (CustomObj *) obj->ConvertToType(t, Class_ID(classIDA, 0));                
		// Note that the TriObject should only be deleted        
		// if the pointer to it is not equal to the object        
		// pointer that called ConvertToType()        
		if (obj != custObj) 
			deleteIt = TRUE;             
		return custObj;        
	}        
	else 
	{             
		return NULL;        
	}
}


}
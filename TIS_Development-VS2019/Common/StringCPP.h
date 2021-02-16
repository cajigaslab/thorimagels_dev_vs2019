#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <strsafe.h>

static std::wstring GetDLLName(HINSTANCE hInst)
{
	WCHAR vPath[_MAX_PATH]; 
	if(0 >= GetModuleFileName(hInst, vPath, _MAX_PATH))
		return L"";

	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	WCHAR fname[_MAX_FNAME];
	WCHAR ext[_MAX_EXT];

	_wsplitpath_s(vPath,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);

	return std::wstring(fname);
}

static std::string WStringToString(std::wstring ws)
{	
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, newsize, ws.c_str(), _TRUNCATE);

	std::string str(nstring);
	return str;
}

static std::wstring StringToWString(std::string s)
{
	size_t origsize = strlen(s.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t  convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, newsize, s.c_str(), _TRUNCATE);

	std::wstring ws(wcstring);
	return ws;
}

///<summary> return string from provided string vector, must followed by FreeCharVec </summary>
static const char ** ConvertStrVec(std::vector<std::string> StrVec)
{
	char ** charArr = new char*[StrVec.size()];
	for(size_t i = 0; i < StrVec.size(); i++)
	{
		charArr[i] = new char[StrVec[i].size() + 1];
		strcpy_s(charArr[i], StrVec[i].size() + 1, StrVec[i].c_str());
	}
	return (const char **)charArr;
}

static void FreeCharVec(const char ** charArr,size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		delete [] charArr[i];
	}
	delete [] charArr;
}

///<summary> return directory from provided file name with path </summary>
static std::wstring GetDir(std::wstring fileNameWithPath)
{
	wchar_t outPath[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	_wsplitpath_s(fileNameWithPath.c_str(),drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
	StringCbPrintfW(outPath,_MAX_PATH,L"%s%s",drive,dir);

	return std::wstring(outPath);
}
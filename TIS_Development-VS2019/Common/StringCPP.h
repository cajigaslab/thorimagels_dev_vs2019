#include <string>
#include <windows.h>

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
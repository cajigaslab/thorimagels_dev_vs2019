#pragma once

#include <vector>
#include <string>
#include <io.h>
#include <windows.h>
#include <winnls.h>
#include <cstdarg>
#include <strsafe.h>
using namespace std;

static inline void Split(string& s, string& delim, vector< string >* ret)
{
	int last = 0;
	int index = (int)s.find_first_of(delim, last);
	while (index != string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		if (last >= s.length())
			break;
		else
			index = (int)s.find_first_of(delim, last);
	}
	if (index > last)
	{
		ret->push_back(s.substr(last, index - last));
	}
	else if (index == -1)
	{
		ret->push_back(s.substr(last));
	}
}

static inline string format(const char* fmt, ...)
{
#define FORMAT_MSG_BUFFER_SIZE (2048)
	char szBuffer[FORMAT_MSG_BUFFER_SIZE + 1] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsnprintf(szBuffer, FORMAT_MSG_BUFFER_SIZE, fmt, args);
	va_end(args);
	std::string strRet = szBuffer;
	return strRet;
}

static inline wchar_t* CharToWchar(const char* ch)
{
	const size_t len = strlen(ch) + 1;
	wchar_t* wch = new wchar_t[len];
	size_t retLen;
	mbstowcs_s(&retLen, wch, len, ch, len);
	return wch;
}

static inline string WCharToString(const wchar_t* wch)
{
	DWORD iTextLen = WideCharToMultiByte(CP_ACP, 0, wch, -1, NULL, 0, NULL, NULL);
	char* pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (static_cast<unsigned long long>(iTextLen) + 1) * sizeof(char));

	WideCharToMultiByte(CP_ACP, 0, wch, -1, pElementText, iTextLen, NULL, NULL);
	string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

static inline string ConvertWStringToString(wstring ws)
{
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	return str;
}
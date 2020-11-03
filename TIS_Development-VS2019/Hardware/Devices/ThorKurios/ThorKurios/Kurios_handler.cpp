#include "stdafx.h"
#include <string>
#include "Kurios_handler.h"

kurios_handler::kurios_handler()
{
	_nPortHandle = -1;
	_commandPhrases[KURIOS_SET_WAVELENGTH] = "WL=%d\r";
	_commandPhrases[KURIOS_SET_BANDWIDTHMODE] = "BW=%d\r";
	_commandPhrases[KURIOS_SET_CONTROLMODE] = "OM=%d\r";
	_commandPhrases[KURIOS_SET_SETSEQUENCE] = "SS=%s\r";
	_commandPhrases[KURIOS_SET_INSERTSEQUENCE] = "IS=%s\r";
	_commandPhrases[KURIOS_SET_DELETESEQUENCE] = "DS=%d\r";
	_commandPhrases[KURIOS_SET_TRIGGEROUTSIGNALMODE] = "TO=%d\r";
	_commandPhrases[KURIOS_SET_FORCETRIGGER] = "ET=%d\r";
	_commandPhrases[KURIOS_SET_TRIGGEROUTTIMEMODE] = "CL=1 20140610\rVA=%d\rCL=0 20140610\r";
	_commandPhrases[KURIOS_SET_SWITCHDELAY] = "TT=%d\r";

	_commandPhrases[KURIOS_GET_WAVELENGTH] = "WL?\r";
	_commandPhrases[KURIOS_GET_TEMPERATURESTATUS] = "ST?\r";
	_commandPhrases[KURIOS_GET_TEMPERATURE] = "TP?\r";
	_commandPhrases[KURIOS_GET_BANDWIDTHMODE] = "BW?\r";
	_commandPhrases[KURIOS_GET_CONTROLMODE] = "OM?\r";
	_commandPhrases[KURIOS_GET_ALLSEQUENCES] = "SS?\r";
	_commandPhrases[KURIOS_GET_WAVELENGTHRANGE] = "SP?\r";
	_commandPhrases[KURIOS_GET_FASTSWITCHINGDATA] = "SD?\r";
	_commandPhrases[KURIOS_GET_AVAILABLEBWMODE] = "OH?\r";
	_commandPhrases[KURIOS_GET_TRIGGEROUTSIGNALMODE] = "TO?\r";
	_commandPhrases[KURIOS_GET_TRIGGEROUTTIMEMODE] = "VA?\r";
	_commandPhrases[KURIOS_GET_SWITCHDELAY] = "TT?\r";
}

long kurios_handler::init()
{
	HMODULE dll_handle = 0;
	#ifdef _WIN64
	dll_handle = LoadLibrary(TEXT("uart_library_ftdi64.dll"));
	#else
	dll_handle = LoadLibrary(TEXT("uart_library_ftdi32.dll"));
    #endif
	
	if (dll_handle)
	{
		fnUART_LIBRARY.open = (fnUART_open)GetProcAddress(dll_handle, ("fnUART_LIBRARY_open"));
		if (!fnUART_LIBRARY.open) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.isOpen = (fnUART_isOpen)GetProcAddress(dll_handle, ("fnUART_LIBRARY_isOpen"));
		if (!fnUART_LIBRARY.isOpen) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.list = (fnUART_list)GetProcAddress(dll_handle, ("fnUART_LIBRARY_list"));
		if (!fnUART_LIBRARY.list) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.close = (fnUART_close)GetProcAddress(dll_handle, ("fnUART_LIBRARY_close"));
		if (!fnUART_LIBRARY.close) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.write = (fnUART_write)GetProcAddress(dll_handle, ("fnUART_LIBRARY_write"));
		if (!fnUART_LIBRARY.write) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.read = (fnUART_read)GetProcAddress(dll_handle, ("fnUART_LIBRARY_read"));
		if (!fnUART_LIBRARY.read) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.set = (fnUART_set)GetProcAddress(dll_handle, ("fnUART_LIBRARY_Set"));
		if (!fnUART_LIBRARY.write) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.get = (fnUART_get)GetProcAddress(dll_handle, ("fnUART_LIBRARY_Get"));
		if (!fnUART_LIBRARY.read) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.timeout = (fnUART_timeout)GetProcAddress(dll_handle, ("fnUART_LIBRARY_timeout"));
		if (!fnUART_LIBRARY.timeout) return ERR_LOADDLLFUNCTIONFAILED;
		fnUART_LIBRARY.Purge = (fnUART_Purge)GetProcAddress(dll_handle, ("fnUART_LIBRARY_Purge"));
		if (!fnUART_LIBRARY.Purge) return ERR_LOADDLLFUNCTIONFAILED;
		return SUCCESS;
	}
	else return ERR_LOADDLLFAILED;
}

kurios_handler::~kurios_handler()
{
	Close();
}

long kurios_handler::List(char *nPort, long var)
{
	return fnUART_LIBRARY.list(nPort, var);
}

long kurios_handler::Open(char *serialNo, long nbaud, long timeout)
{
	_nPortHandle = fnUART_LIBRARY.open(serialNo, nbaud, timeout);
	if (_nPortHandle < 0)
	{
		return ERR_DEVICENOTCONNECT;
	}
	else
	{
		return SUCCESS;
	}
}

long kurios_handler::Close()
{
	if (_nPortHandle < 0)
	{
		return ERR_DEVICENOTCONNECT;
	}
	else
	{
		fnUART_LIBRARY.close(_nPortHandle);
		_nPortHandle = -1;
		return SUCCESS;
	}
}

long kurios_handler::SetValue(KURIOS_PARAMS_TYPE type, char* value)
{
	if (_nPortHandle < 0) return ERR_DEVICENOTCONNECT;
	if (type != KURIOS_SET_SETSEQUENCE&&
		type != KURIOS_SET_INSERTSEQUENCE)
		return ERR_FUNCTIONNOTEXIST;
	char c[MSG_SIZE] = { 0 };
	sprintf_s(c, _commandPhrases[type], value);
	return fnUART_LIBRARY.set(_nPortHandle, c, 0);
}

long kurios_handler::SetValue(KURIOS_PARAMS_TYPE type, long value)
{
	if (_nPortHandle < 0) return ERR_DEVICENOTCONNECT;
	if (type != KURIOS_SET_WAVELENGTH&&
		type != KURIOS_SET_BANDWIDTHMODE&&
		type != KURIOS_SET_CONTROLMODE&&
		type != KURIOS_SET_DELETESEQUENCE&&
		type != KURIOS_SET_TRIGGEROUTSIGNALMODE&&
		type != KURIOS_SET_TRIGGEROUTTIMEMODE&&
		type != KURIOS_SET_FORCETRIGGER&&
		type != KURIOS_SET_SWITCHDELAY)
		return ERR_FUNCTIONNOTEXIST;
	char c[MSG_SIZE] = { 0 };
	sprintf_s(c, _commandPhrases[type], value);
	return fnUART_LIBRARY.set(_nPortHandle, c, 0);
}

long kurios_handler::GetValue(KURIOS_PARAMS_TYPE type, char*& value)
{
	if (_nPortHandle < 0) return ERR_DEVICENOTCONNECT;
	if (type != KURIOS_GET_WAVELENGTHRANGE&&
		type != KURIOS_GET_ALLSEQUENCES&&
		type != KURIOS_GET_FASTSWITCHINGDATA)
		return ERR_FUNCTIONNOTEXIST;
	char c[MSG_SIZE] = { 0 };
	sprintf_s(c, _commandPhrases[type]);
	return fnUART_LIBRARY.get(_nPortHandle, c, value);
}

long kurios_handler::GetValue(KURIOS_PARAMS_TYPE type, double & value)
{
	if (_nPortHandle < 0) return ERR_DEVICENOTCONNECT;
	if (type != KURIOS_GET_TEMPERATURE)
		return ERR_FUNCTIONNOTEXIST;
	char c[MSG_SIZE] = { 0 };
	char d[MSG_SIZE] = { 0 };
	sprintf_s(c, _commandPhrases[type]);
	if(fnUART_LIBRARY.get(_nPortHandle, c, d) < 0) return ERR;
	if (ParseString(d) < 0) return ERR_GETSTRINGWRONG;
	value = atof(d);
	return SUCCESS;
}

long kurios_handler::GetValue(KURIOS_PARAMS_TYPE type, long & value)
{
	if (_nPortHandle < 0) return ERR_DEVICENOTCONNECT;
	if (type != KURIOS_GET_WAVELENGTH&&
		type != KURIOS_GET_AVAILABLEBWMODE&&
		type != KURIOS_GET_TEMPERATURESTATUS&&
		type != KURIOS_GET_BANDWIDTHMODE&&
		type != KURIOS_GET_CONTROLMODE&&
		type != KURIOS_GET_TRIGGEROUTSIGNALMODE&&
		type != KURIOS_GET_TRIGGEROUTTIMEMODE&&
		type != KURIOS_GET_SWITCHDELAY)
		return ERR_FUNCTIONNOTEXIST;
	char c[MSG_SIZE] = { 0 };
	char d[MSG_SIZE] = { 0 };
	sprintf_s(c, _commandPhrases[type]);
	if(fnUART_LIBRARY.get(_nPortHandle, c, d) < 0) return ERR;
	if (ParseString(d) < 0) return ERR_GETSTRINGWRONG;
	value = atoi(d);
	return SUCCESS;
}

long kurios_handler::GetWavelengthRange(long &max, long &min)
{
	min = 0;
	max = 0;
	char *str = new char[MSG_SIZE];
	if (GetValue(KURIOS_GET_WAVELENGTHRANGE, str) < 0)
		return ERR;
	string ss(str);
	vector<string> results;
	string delim = "\r";
	Split(ss, delim, &results);
	if (results.size()<2)
		return ERR;
	delim = "=";
	vector<string> maxs;
	Split(results[0], delim, &maxs);
	if (maxs.size()<2)
		return ERR;
	if (maxs[0].compare("WLmax") == 0)
		max = atoi(maxs[1].c_str());
	else
		return ERR;
	vector<string> mins;
	Split(results[1], delim, &mins);
	if (mins.size()<2)
		return ERR;
	if (mins[0].compare("WLmin") == 0)
		min = atoi(mins[1].c_str());
	else
		return ERR;
	return SUCCESS;
}

long kurios_handler::GetAvailableBandwidthMode(long &max, long &min)
{
	min = 0;
	max = 0;
	long type = 0;
	if (GetValue(KURIOS_GET_AVAILABLEBWMODE, type) < 0)
		return ERR;
	switch (type & 0xF)
	{
	case BLACKMODE_BIT | WIDEMODE_BIT:
		min = WIDEMODE_BIT;
		max = WIDEMODE_BIT;
		break;
	case BLACKMODE_BIT | MEDIEMMODE_BIT:
		min = MEDIEMMODE_BIT;
		max = MEDIEMMODE_BIT;
		break;
	case BLACKMODE_BIT | NARROWMODE_BIT:
		min = NARROWMODE_BIT;
		max = NARROWMODE_BIT;
		break;
	case BLACKMODE_BIT | WIDEMODE_BIT | MEDIEMMODE_BIT | NARROWMODE_BIT:
		min = WIDEMODE_BIT;
		max = NARROWMODE_BIT;
		break;
	default:
		break;
	}
	return SUCCESS;
}

void kurios_handler::Split(string& s, string& delim, vector<string>* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}

long kurios_handler::ParseString(char* str)
{
	string s(str);
	vector<string> results;
	string delim = "=>";

	Split(s, delim, &results);
	size_t size = results.size();
	if (size >= 2)
	{
		string valueStr = results.at(1);
		size_t len = valueStr.length();
		memset(str, '/0', sizeof(str));
		valueStr.copy(str, len, 0);
		return SUCCESS;
	}
	return ERR;
}

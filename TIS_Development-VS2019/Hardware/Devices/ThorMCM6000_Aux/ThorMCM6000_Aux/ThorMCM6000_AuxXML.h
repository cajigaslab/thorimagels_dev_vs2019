#pragma once
#include "stdafx.h"

class ThorMCM6000_AuxXML
{
private:
	ticpp::Document* _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];
	long SetAttribute(string tagName, string attribute, string attributeValue);
	long OpenConfigFile();
	long SaveConfigFile();

public:
	enum { NUM_CONNECTION_ATTRIBUTES = 3 };
	static const char* const CONNECTION;
	static const char* const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	static const char* const TIMEOUT;
	enum { NUM_TIMEOUT_ATTRIBUTES = 1 };
	static const char* const TIMEOUT_ATTR[NUM_TIMEOUT_ATTRIBUTES];

	ThorMCM6000_AuxXML();
	~ThorMCM6000_AuxXML();

	long GetConnection(long& portID, long& baudRate, string& serialNumber);
	long SetPortID(const string signature, const long portID);
	long GetTimeOut(long& timeOutTime);
};
#pragma once

class ThorBCMXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long SetAttribute(string tagName, string attribute, string attributeValue);
	long OpenConfigFile();
	long SaveConfigFile();
public:
	ThorBCMXML();
	~ThorBCMXML();

	enum {NUM_CONNECTION_ATTRIBUTES = 3};
	static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];
	long GetConnection(const string signature, long &portID,long &baudRate, string &serialNumber);
	long SetPortID(const string signature, const long portID);	
};
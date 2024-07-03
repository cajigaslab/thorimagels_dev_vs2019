#pragma once
class ThorPreludeXML
{

private:
	static const char* const CONNECTION;
	enum { NUM_CONNECTION_ATTRIBUTES = 2 };
	static const char* const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	long OpenConfigFile();
	long SaveConfigFile();
	ticpp::Document* _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];

public:

public:
	ThorPreludeXML(void);
	~ThorPreludeXML(void);
	long GetConnection(long& portID, long& baudRate);
};


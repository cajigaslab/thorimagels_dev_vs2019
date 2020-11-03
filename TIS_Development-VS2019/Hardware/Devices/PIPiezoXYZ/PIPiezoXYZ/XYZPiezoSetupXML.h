#pragma once

class XYZPiezoXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const CONVERSION;
	enum {NUM_CONVERSION_ATTRIBUTES = 7};
	static const char * const CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 6};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	XYZPiezoXML();
	~XYZPiezoXML();

	long GetConversion(double &volt2mm, double &xPos_min, double &xPos_max, double &yPos_min, double &yPos_max, double &zPos_min,double &zPos_max);
	long GetIO(string &piezoXLine, string &XanalogLine, string &piezoYLine, string &YanalogLine,string &piezoZLine, string &ZanalogLine);
	long OpenConfigFile();
	long SaveConfigFile();
};
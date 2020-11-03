#pragma once

class ThorTiberiusXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	 
	 static const char * const CONNECTION;
	 enum {NUM_CONNECTION_ATTRIBUTES = 1};
	 static const char * const CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES];

	 static const char * const WAVELENGTH;
	 enum {NUM_WAVELENGTH_ATTRIBUTES = 2};
	 static const char * const WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES];


	ThorTiberiusXML();
	~ThorTiberiusXML();

	long GetConnection(long &portID);
	long GetWavelength(long &wavelengthMin,  long &wavelengthMax);

	long OpenConfigFile(string path);

};
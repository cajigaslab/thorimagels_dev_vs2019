#pragma once

class ThorChrolisXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long OpenConfigFile();

	static const char * const NOMINAL_WAVELENGTHS;
	enum {NUM_NORMINAL_WAVELENGTHS_ATTRIBUTES = 6};
	static const char * const NOMINAL_WAVELENGTHS_ATTR[NUM_NORMINAL_WAVELENGTHS_ATTRIBUTES];

public:	 
	ThorChrolisXML();
	~ThorChrolisXML();
	long GetNominalWavelengths(long& nominalWavelengthLED1, long& nominalWavelengthLED2, long& nominalWavelengthLED3, long& nominalWavelengthLED4, long& nominalWavelengthLED5, long& nominalWavelengthLED6);
};
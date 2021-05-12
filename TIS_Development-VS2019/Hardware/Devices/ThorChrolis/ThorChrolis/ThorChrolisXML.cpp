#include "stdafx.h"
#include "ThorChrolisXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="ThorChrolisXML"/> class.
/// </summary>
ThorChrolisXML::ThorChrolisXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorChrolisXML"/> class.
/// </summary>
ThorChrolisXML::~ThorChrolisXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorChrolisXML::NOMINAL_WAVELENGTHS = "NominalWavelenghts";

const char * const ThorChrolisXML::NOMINAL_WAVELENGTHS_ATTR[NUM_NORMINAL_WAVELENGTHS_ATTRIBUTES] = {"LED1", "LED2", "LED3", "LED4", "LED5", "LED6"};

/// <summary>
/// Gets the nominal wavelengths from ThorChrolisSettings.xml.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorChrolisXML::GetNominalWavelengths(long& nominalWavelengthLED1, long& nominalWavelengthLED2, long& nominalWavelengthLED3, long& nominalWavelengthLED4, long& nominalWavelengthLED5, long& nominalWavelengthLED6)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator< ticpp::Element > child(configObj, NOMINAL_WAVELENGTHS);

	for ( child = child.begin( configObj ); child != child.end(); child++)
	{
		for(long attCount = 0; attCount<NUM_NORMINAL_WAVELENGTHS_ATTRIBUTES; attCount++)
		{
			string str;
			child->GetAttribute(NOMINAL_WAVELENGTHS_ATTR[attCount], &str);
			stringstream ss(str);
			switch(attCount)
			{
			case 0:
				{
					ss>>nominalWavelengthLED1;
				}
				break;
			case 1:
				{
					ss>>nominalWavelengthLED2;
				}
				break;
			case 2:
				{
					ss>>nominalWavelengthLED3;
				}
				break;
			case 3:
				{
					ss>>nominalWavelengthLED4;
				}
				break;
			case 4:
				{
					ss>>nominalWavelengthLED5;
				}
				break;
			case 5:
				{
					ss>>nominalWavelengthLED6;
				}
				break;
			}
		}
	}		
	return FALSE;
}


/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ThorChrolisXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorChrolisSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());

	if(_xmlObj == NULL) 
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}

#include "stdafx.h"
#include "ThorPMT2100XML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="ThorPMT2100XML"/> class.
/// </summary>
ThorPMT2100XML::ThorPMT2100XML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorPMT2100XML"/> class.
/// </summary>
ThorPMT2100XML::~ThorPMT2100XML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorPMT2100XML::CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES] = {"serialNumber"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorPMT2100XML::GetDeviceAddress(const string signature, string &deviceAddress)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

	for ( child = child.begin( configObj ); child != child.end(); child++)
	{
		for(long attCount = 0; attCount<NUM_SETTINGS_ATTRIBUTES; attCount++)
		{
			string str;
			child->GetAttribute(CONNECTION_ATTR[attCount], &str);

			switch(attCount)
			{
			case 0:
				{
					deviceAddress = str;
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
long ThorPMT2100XML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorPMT2100Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj == NULL) 
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}

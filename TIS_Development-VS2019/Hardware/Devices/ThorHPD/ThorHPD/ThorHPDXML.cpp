#include "stdafx.h"
#include "ThorHPDXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="ThorHPDXML"/> class.
/// </summary>
ThorHPDXML::ThorHPDXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorHPDXML"/> class.
/// </summary>
ThorHPDXML::~ThorHPDXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorHPDXML::CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES] = {"portID","baudRate","serialNumber", "type"};

/// <summary>
/// Gets the connection info.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorHPDXML::GetDeviceConnectionInfo(const string signature, long &portID, long &baudRate, string &serialNumber, long &detectorType)
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
			stringstream ss(str);
			switch(attCount)
			{
			case 0:
				{
					 ss>>portID;
				}
				break;
			case 1:
				{
					ss>>baudRate;
				}
				break;
			case 2:
				{
					serialNumber = str;
				}
				break;
			case 3:
				{
					 ss>>detectorType;
				}
				break;
			}
		}
	}		
	return FALSE;
}


const char * const ThorHPDXML::GAIN = "DetectorGain";

const char * const ThorHPDXML::GAIN_ATTR[NUM_GAIN_ATTRIBUTES] = {"gain1", "gain2","gain3","gain4"};

/// <summary>
/// Gets the default gain for each detector.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorHPDXML::GetDetectorDefaultGain(long &gain1, long &gain2, long &gain3, long &gain4)
{	
	try
	{				
		OpenConfigFile();
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, GAIN);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_GAIN_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(GAIN_ATTR[attCount], &str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>gain1;
					}
					break;
				case 1:
					{
						ss>>gain2;
					}
					break;
				case 2:
					{
						ss>>gain3;
					}
					break;
				case 3:
					{
						ss>>gain4;
					}
					break;
				}
			}
		}	
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Gets the default gain for each detector.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorHPDXML::SetDetectorDefaultGain(long gain1, long gain2, long gain3, long gain4)
{	
	try
	{
		long ret = FALSE;

		OpenConfigFile();

		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return ret;
		}
		else
		{
			string str;
			stringstream ss;

			ss << gain1;
			ss << endl;
			ss << gain2;
			ss << endl;
			ss << gain3;
			ss << endl;
			ss << gain4;
			ss << endl;
			long index;

			for(index=0; index<NUM_GAIN_ATTRIBUTES; index++)
			{
				getline(ss,str);

				// iterate over to get the particular tag element specified as a parameter(tagName)
				ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(GAIN), GAIN);
				//get the attribute value for the specified attribute name
				child->SetAttribute(GAIN_ATTR[index], str);
			}
		}

		SaveConfigFile();
	}
	catch(ticpp::Exception ex)
	{
		return FALSE;
	}

	return TRUE;
}


/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ThorHPDXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorHPDSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	if(_xmlObj == NULL) 
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}

long ThorHPDXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();	
	}			

	return TRUE;
}

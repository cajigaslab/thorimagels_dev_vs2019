#include "stdafx.h"
#include "ThorObjectiveChangerXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorLSKGRXML"/> class.
/// </summary>
ThorObjectiveChangerXML::ThorObjectiveChangerXML()
{
	_xmlObj = nullptr;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorLSKGRXML"/> class.
/// </summary>
ThorObjectiveChangerXML::~ThorObjectiveChangerXML()
{
	if(nullptr != _xmlObj)
	{
		delete _xmlObj;
	}
}


const char * const ThorObjectiveChangerXML::CONNECTION = "Connection";

const char * const ThorObjectiveChangerXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate", "serialNumber", "homed"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="serialNumber">The Serial Number.</param>
/// <returns>long.</returns>
long ThorObjectiveChangerXML::GetConnection(long &portID,long &baudRate, std::wstring &serialNumber, long &homed)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator<ticpp::Element> child(configObj, CONNECTION);

	for (child = child.begin(configObj); child != child.end(); child++)
	{
		for(long attCount = 0; attCount < NUM_CONNECTION_ATTRIBUTES; attCount++)
		{
			std::string str;
			child->GetAttribute(CONNECTION_ATTR[attCount], &str);

			switch(attCount)
			{
			case 0:
				{
					std::stringstream ss(str);
					ss>>portID;
				}
				break;

			case 1:
				{
					std::stringstream ss(str);
					ss>>baudRate;
				}
				break;
			case 2:
				{
					std::wstring temp(str.length(),L' ');
					copy(str.begin(), str.end(), temp.begin());
					serialNumber = temp;
				}
				break;
			case 3:
				{
					std::stringstream ss(str);
					ss>>homed;
				}
				break;
			}
		}
	}		  

	return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorObjectiveChangerXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorObjectiveChangerSettings.xml");		

	std::string s = ConvertWStringToString(_currentPathAndFile);

	if(nullptr != _xmlObj)
	{
		return TRUE;
	}

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	

	return TRUE;
}

long ThorObjectiveChangerXML::UpdateHomedFlagToSettings(long &homed)
{
	long ret = FALSE;
	OpenConfigFile();
	if(nullptr != _xmlObj)
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) return FALSE;

		ticpp::Iterator<ticpp::Element> child(configObj, CONNECTION);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			child->SetAttribute("homed", homed);
		}
		ret = TRUE;
	}
	return ret;
}

/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorObjectiveChangerXML::SaveConfigFile()
{
	if(nullptr != _xmlObj)
	{
		_xmlObj->SaveFile();
	}	

	return TRUE;
}
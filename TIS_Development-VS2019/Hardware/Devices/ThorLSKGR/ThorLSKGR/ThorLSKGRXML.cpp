#include "stdafx.h"
#include "ThorLSKGRXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorLSKGRXML"/> class.
/// </summary>
ThorLSKGRXML::ThorLSKGRXML()
{
	_xmlObj = nullptr;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorLSKGRXML"/> class.
/// </summary>
ThorLSKGRXML::~ThorLSKGRXML()
{
	if(nullptr != _xmlObj)
	{
		delete _xmlObj;
	}
}


const char * const ThorLSKGRXML::CONNECTION = "Connection";

const char * const ThorLSKGRXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorLSKGRXML::GetConnection(long &portID,long &baudRate)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if (nullptr == configObj)
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,CONNECTION);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_CONNECTION_ATTRIBUTES; attCount++)
			{
				std::string str;
				child->GetAttribute(CONNECTION_ATTR[attCount],&str);
				switch(attCount)
				{
				case 0: portID = stoi(str); break;
				case 1: baudRate = stoi(str); break;				  
				}
			}
		}		  
	}	
	return TRUE;
}


const char * const ThorLSKGRXML::CONFIGURATION = "Configuration";

const char * const ThorLSKGRXML::CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES] = {"rsInitMode"};

// get resonance scanner initial mode from .xml settings file
/// <summary>
/// Gets the configuration.
/// </summary>
/// <param name="rsInitMode">The resonance scanner initialize mode. 0 - Resonance scanner 'toggle' mode.  1- Resonance Scanner 'always on' mode</param>
/// <returns>long.</returns>
long ThorLSKGRXML::GetConfiguration(long &rsInitMode)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

    if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,CONFIGURATION);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
		  	std::string str;
			child->GetAttribute(CONFIGURATION_ATTR[0],&str);	
			rsInitMode = stoi(str);	// 0 off; 1 on	  
            return TRUE;  
		}		  
	}	
	return FALSE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGRXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorLSKGRSettings.xml");		

	std::string s = ConvertWStringToString(_currentPathAndFile);

	if(nullptr != _xmlObj)
	{
		return TRUE;
	}

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	

	return TRUE;
}

/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorLSKGRXML::SaveConfigFile()
{
	if(nullptr != _xmlObj)
	{
		_xmlObj->SaveFile();
	}	

	return TRUE;
}
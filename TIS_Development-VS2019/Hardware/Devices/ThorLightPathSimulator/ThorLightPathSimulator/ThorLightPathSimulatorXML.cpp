#include "stdafx.h"
#include "ThorLightPathSimulatorXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorLightPathSimulatorXML"/> class.
/// </summary>
ThorLightPathSimulatorXML::ThorLightPathSimulatorXML()
{
	_currentPathAndFile[0] = NULL;
	_xmlObj = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorLightPathSimulatorXML"/> class.
/// </summary>
ThorLightPathSimulatorXML::~ThorLightPathSimulatorXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorLightPathSimulatorXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES]  = {"portID","baudRate","serialNumber"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="signature">The signature.</param>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="serialNumber">The serial number.</param>
/// <returns>long.</returns>
long ThorLightPathSimulatorXML::GetConnection(const string signature, long &portID,long &baudRate, string &serialNumber)
{	
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL ) return FALSE;

	ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

	for ( child = child.begin( configObj ); child != child.end(); child++)
	{
		for(long attCount = 0; attCount<NUM_CONNECTION_ATTRIBUTES; attCount++)
		{
			string str;
			child->GetAttribute(CONNECTION_ATTR[attCount], &str);
			
			switch(attCount)
			{
			case 0:
				{
					stringstream ss(str);
					ss>>portID;
				}
				break;

			case 1:
				{
					stringstream ss(str);
					ss>>baudRate;
				}
				break;
			case 2:
				{
					serialNumber = str;
				}
				break;
			}
		}
	}		  

	return TRUE;
}

/// <summary>
/// Sets the port identifier.
/// </summary>
/// <param name="signature">The signature.</param>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorLightPathSimulatorXML::SetPortID(const string signature, const long portID)
{
	string str;
	stringstream ss;

	ss << portID;
	getline(ss,str);
	if(FALSE == SetAttribute(signature.c_str(), CONNECTION_ATTR[0], str))
	{
		return FALSE;
	}

	SaveConfigFile();
	return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulatorXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorLightPathSimulatorSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	if(_xmlObj == NULL) 
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}

/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorLightPathSimulatorXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	

	return TRUE;
}

/// <summary>
/// Sets the attribute.
/// </summary>
/// <param name="tagName">Name of the tag.</param>
/// <param name="attribute">The attribute.</param>
/// <param name="attributeValue">The attribute value.</param>
/// <returns>long.</returns>
long ThorLightPathSimulatorXML::SetAttribute(string tagName, string attribute, string attributeValue)
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
		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
		return TRUE;
	}
}

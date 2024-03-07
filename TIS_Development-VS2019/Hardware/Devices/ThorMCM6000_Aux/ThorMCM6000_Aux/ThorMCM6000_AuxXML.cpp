#include "stdafx.h"
#include "ThorMCM6000_AuxXML.h"

/// <summary>
/// Initializes a new instance of the <see cref="ThorDDR05XML"/> class.
/// </summary>
ThorMCM6000_AuxXML::ThorMCM6000_AuxXML()
{
	_currentPathAndFile[0] = NULL;
	_xmlObj = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorDDR05XML"/> class.
/// </summary>
ThorMCM6000_AuxXML::~ThorMCM6000_AuxXML()
{
	delete _xmlObj;
	_xmlObj = NULL;
}

const char* const ThorMCM6000_AuxXML::CONNECTION = "Connection";
const char* const ThorMCM6000_AuxXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = { "portID", "baudRate", "serialNumber" };
/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="serialNumber">The serial number. If 'NA' is specified for the serial number the device does not check for a matching serial number</param>
/// <returns>long.</returns>
long ThorMCM6000_AuxXML::GetConnection(long& portID, long& baudRate, string& serialNumber)
{
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator<ticpp::Element> child(configObj, CONNECTION);

	for (child = child.begin(configObj); child != child.end(); child++)
	{
		for (long attCount = 0; attCount < NUM_CONNECTION_ATTRIBUTES; attCount++)
		{
			string str;
			child->GetAttribute(CONNECTION_ATTR[attCount], &str);

			switch (attCount)
			{
			case 0:
			{
				stringstream ss(str);
				ss >> portID;
			}
			break;

			case 1:
			{
				stringstream ss(str);
				ss >> baudRate;
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
long ThorMCM6000_AuxXML::SetPortID(const string signature, const long portID)
{
	string str;
	stringstream ss;

	ss << portID;
	getline(ss, str);
	if (FALSE == SetAttribute(signature.c_str(), CONNECTION_ATTR[0], str))
	{
		return FALSE;
	}

	SaveConfigFile();
	return TRUE;
}

const char* const ThorMCM6000_AuxXML::TIMEOUT = "TimeOut";
const char* const ThorMCM6000_AuxXML::TIMEOUT_ATTR[NUM_TIMEOUT_ATTRIBUTES] = { "timeMS" };
/// <summary>
/// Gets the time out.
/// </summary>
/// <param name="timeOutTime">The max time out to wait for the device to reach position.</param>
/// <returns>long.</returns>
long ThorMCM6000_AuxXML::GetTimeOut(long& timeOutTime)
{
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator< ticpp::Element > child(configObj, TIMEOUT);

	for (child = child.begin(configObj); child != child.end(); child++)
	{
		for (long attCount = 0; attCount < NUM_TIMEOUT_ATTRIBUTES; attCount++)
		{
			string str;
			child->GetAttribute(TIMEOUT_ATTR[attCount], &str);
			stringstream ss(str);
			switch (attCount)
			{
			case 0:
			{
				ss >> timeOutTime;
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
long ThorMCM6000_AuxXML::OpenConfigFile()
{
	wsprintf(_currentPathAndFile, L"ThorMCM6000_AuxSettings.xml");

	string s = ConvertWStringToString(_currentPathAndFile);

	if (_xmlObj == NULL)
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
long ThorMCM6000_AuxXML::SaveConfigFile()
{

	if (_xmlObj != NULL)
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
long ThorMCM6000_AuxXML::SetAttribute(string tagName, string attribute, string attributeValue)
{
	long ret = FALSE;
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj != NULL)
	{
		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(tagName), tagName);
		//get the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
		ret = TRUE;
	}
	return ret;
}
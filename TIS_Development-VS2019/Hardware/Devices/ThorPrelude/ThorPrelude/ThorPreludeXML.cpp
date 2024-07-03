#include "stdafx.h"
#include "ThorPreludeXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorPreludeXML"/> class.
/// </summary>
ThorPreludeXML::ThorPreludeXML()
{
	_xmlObj = nullptr;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorPreludeXML"/> class.
/// </summary>
ThorPreludeXML::~ThorPreludeXML()
{
	if (nullptr != _xmlObj)
	{
		delete _xmlObj;
	}
}


const char* const ThorPreludeXML::CONNECTION = "Connection";

const char* const ThorPreludeXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = { "portID","baudRate" };

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorPreludeXML::GetConnection(long& portID, long& baudRate)
{
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (nullptr == configObj)
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj, CONNECTION);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			for (long attCount = 0; attCount < NUM_CONNECTION_ATTRIBUTES; attCount++)
			{
				std::string str;
				child->GetAttribute(CONNECTION_ATTR[attCount], &str);
				switch (attCount)
				{
				case 0: portID = stoi(str); break;
				case 1: baudRate = stoi(str); break;
				}
			}
		}
	}
	return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorPreludeXML::OpenConfigFile()
{
	wsprintf(_currentPathAndFile, L"ThorPreludeSettings.xml");

	std::string s = ConvertWStringToString(_currentPathAndFile);

	if (nullptr != _xmlObj)
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
long ThorPreludeXML::SaveConfigFile()
{
	if (nullptr != _xmlObj)
	{
		_xmlObj->SaveFile();
	}

	return TRUE;
}
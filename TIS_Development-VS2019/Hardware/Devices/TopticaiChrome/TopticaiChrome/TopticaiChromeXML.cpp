#include "stdafx.h"
#include "TopticaiChromeXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="TopticaiChromeXML"/> class.
/// </summary>
TopticaiChromeXML::TopticaiChromeXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
	_errMsg[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="TopticaiChromeXML"/> class.
/// </summary>
TopticaiChromeXML::~TopticaiChromeXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char* const TopticaiChromeXML::CONNECTION = "Connection";

const char* const TopticaiChromeXML::CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES] = { "portID","baudRate" };

/// <summary>
/// Gets the connection info.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long TopticaiChromeXML::GetDeviceConnectionInfo(long& portID, long& baudRate)
{
	try
	{
		OpenConfigFile();
		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) 
		{
			return FALSE;
		}

		ticpp::Iterator< ticpp::Element > child(configObj, CONNECTION);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			for (long attCount = 0; attCount < NUM_SETTINGS_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(CONNECTION_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
				{
					ss >> portID;
				}
				break;
				case 1:
				{
					ss >> baudRate;
				}
				break;
				}
			}
		}
	}
	catch (ticpp::Exception ex)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"TopticaiChromeXML: GetDeviceConnectionInfo exception thrown, couldn't open TopticaiChromeSettings.xml");
		logDll->TLTraceEvent(ERROR_EVENT, 1, _errMsg);
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long TopticaiChromeXML::OpenConfigFile()
{
	wsprintf(_currentPathAndFile, L"TopticaiChromeSettings.xml");

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
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long TopticaiChromeXML::SaveConfigFile()
{
	if (_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}

	return TRUE;
}

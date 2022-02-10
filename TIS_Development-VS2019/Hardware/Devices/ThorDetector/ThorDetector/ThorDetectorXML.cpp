#include "stdafx.h"
#include "ThorDetectorXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="ThorDetectorXML"/> class.
/// </summary>
ThorDetectorXML::ThorDetectorXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorDetectorXML"/> class.
/// </summary>
ThorDetectorXML::~ThorDetectorXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char* const ThorDetectorXML::CONNECTION_ATTR[NUM_SETTINGS_ATTRIBUTES] = { "portID","baudRate","serialNumber", "type" };

/// <summary>
/// Gets the connection info.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorDetectorXML::GetDeviceConnectionInfo(const string signature, long& portID, long& baudRate, string& serialNumber, long& detectorType)
{
	try
	{
		OpenConfigFile();
		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

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
				case 2:
				{
					serialNumber = str;
				}
				break;
				case 3:
				{
					ss >> detectorType;
				}
				break;
				}
			}
		}
	}
	catch (ticpp::Exception ex)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorDetectorXML: GetDeviceConnectionInfo exception thrown, couldn't open ThorDetectorSettings.xml");
		return FALSE;
	}
	return TRUE;
}

/// <summary>
/// Save the detector's serial number to ThorDetectorSettings.xml
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorDetectorXML::SetSerialNumber(const string signature, string serialNumber)
{
	long ret = FALSE;
	try
	{
		OpenConfigFile();
		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			child->SetAttribute(CONNECTION_ATTR[2], serialNumber);
		}
		ret = SaveConfigFile();
	}
	catch (ticpp::Exception ex)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorDetectorXML: SetSerialNumber exception thrown, couldn't open or save to ThorDetectorSettings.xml");
		return FALSE;
	}

	return ret;
}

/// <summary>
/// Save the detector type to ThorDetectorSettings.xml
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorDetectorXML::SetDetectorType(const string signature, string detectorType)
{
	long ret = FALSE;
	try
	{
		OpenConfigFile();
		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL) return FALSE;

		ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			child->SetAttribute(CONNECTION_ATTR[3], detectorType);
		}
		ret = SaveConfigFile();
	}
	catch (ticpp::Exception ex)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorDetectorXML: SetDetectorType exception thrown, couldn't open or save to ThorDetectorSettings.xml");
		return FALSE;
	}

	return ret;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ThorDetectorXML::OpenConfigFile()
{
	wsprintf(_currentPathAndFile, L"ThorDetectorSettings.xml");

	string s = ConvertWStringToString(_currentPathAndFile);

	if (_xmlObj == NULL)
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}

long ThorDetectorXML::SaveConfigFile()
{
	if (_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}

	return TRUE;
}

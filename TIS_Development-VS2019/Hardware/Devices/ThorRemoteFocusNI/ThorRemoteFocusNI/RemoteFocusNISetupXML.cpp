#include "stdafx.h"
#include "RemoteFocusNISetupXML.h"

RemoteFocusNIXML::RemoteFocusNIXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

RemoteFocusNIXML::~RemoteFocusNIXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char* const RemoteFocusNIXML::DMA = "Dma";

const char* const RemoteFocusNIXML::DMA_ATTR[NUM_DMA_ATTRIBUTES] = { "rfActiveLoadMS", "preLoadCount" };

long RemoteFocusNIXML::GetDMA(long& activeLoadMS, long& preLoadCount)
{
	try
	{
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, DMA);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_DMA_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(DMA_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0: ss >> activeLoadMS; break;
					case 1: ss >> preLoadCount; break;
					}
				}
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const RemoteFocusNIXML::IO = "IO";

const char* const RemoteFocusNIXML::IO_ATTR[NUM_IO_ATTRIBUTES] = { "devCard","analogLine","triggerLine" };

long RemoteFocusNIXML::GetIO(string& devCard, string& analogLine, string& triggerLine)
{
	try
	{
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, IO);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_IO_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(IO_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0: ss >> devCard; break;
					case 1: ss >> analogLine; break;
					case 2: ss >> triggerLine; break;
					}
				}
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

long RemoteFocusNIXML::OpenConfigFile()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	wsprintf(_currentPathAndFile, L"ThorRemoteFocusNISettings.xml");

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	return TRUE;
}

long RemoteFocusNIXML::SaveConfigFile()
{
	if (_xmlObj != NULL)
	{
		wsprintf(_currentPathAndFile, L"ThorRemoteFocusNISettings.xml");
		wstring ws = _currentPathAndFile;
		string s = ConvertWStringToString(ws);

		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;
}


long RemoteFocusNIXML::ReadPositionVoltages(long& numberOfPlanes, vector<double>* posVoltages)
{
	wstring docFolder = ResourceManager::getInstance()->GetMyDocumentsThorImageFolder();
	wstring ws = docFolder + L"Application Settings\\RemoteFocusPositionValues.xml";
	string s = ConvertWStringToString(ws);

	try
	{
		ticpp::Document* xmlDoc = new ticpp::Document(s);
		xmlDoc->LoadFile();

		ticpp::Element* configObj = xmlDoc->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, "NumberOfPlanes");

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				string str;
				child->GetAttribute("value", &str);
				stringstream ss(str);
				ss >> numberOfPlanes;
			}

			ticpp::Iterator< ticpp::Element > secChild(configObj, "PlaneVoltage");

			for (secChild = secChild.begin(configObj); secChild != secChild.end(); secChild++)
			{
				for (long attCount = 1; attCount <= numberOfPlanes; attCount++)
				{
					string str;
					string attrName = "plane" + to_string(attCount);
					secChild->GetAttribute(attrName, &str);
					stringstream ss(str);
					double val = 0; 
					ss >> val;
					posVoltages->push_back(val);
				}
			}
		}
	}
	catch (exception ex)
	{
		static wchar_t errMsg[MAX_PATH];
		wstring msg = L"Could not read values from " + ws;
		StringCbPrintfW(errMsg, MAX_PATH, msg.c_str());
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}

	return TRUE;
}
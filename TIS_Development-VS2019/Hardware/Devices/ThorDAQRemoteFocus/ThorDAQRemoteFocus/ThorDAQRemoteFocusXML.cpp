#include "stdafx.h"
#include "ThorDAQRemoteFocusXML.h"

ThorDAQRemoteFocusXML::ThorDAQRemoteFocusXML()
{

}

ThorDAQRemoteFocusXML::~ThorDAQRemoteFocusXML()
{

}

long ThorDAQRemoteFocusXML::ReadPositionVoltages(long& numberOfPlanes, vector<double>* posVoltages)
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
		//LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}

	return TRUE;
}
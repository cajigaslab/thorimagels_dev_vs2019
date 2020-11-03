#include "stdafx.h"
#include "ThorConfocalGalvoSimulatorXML.h"
#include "Strsafe.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorConfocalGalvoSimulatorXML::ThorConfocalGalvoSimulatorXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorConfocalGalvoSimulatorXML::~ThorConfocalGalvoSimulatorXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorConfocalGalvoSimulatorXML::CONNECTION = "Connection";

const char * const ThorConfocalGalvoSimulatorXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"path", "imageUpdateIntervalMS"};

long ThorConfocalGalvoSimulatorXML::GetConnection(string& path, long& imageUpdateIntervalMS)
{
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThorConfocalGalvoSimulatorSettings.xml");

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	OpenConfigFile(s);
	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"configObj == NULL");
#endif
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj, CONNECTION);

		for (child = child.begin(configObj); child != child.end(); child++)
		{
			string str;
			for (long attCount = 0; attCount < NUM_CONNECTION_ATTRIBUTES; attCount++)
			{
				child->GetAttribute(CONNECTION_ATTR[attCount], &str);
				stringstream ss(str);
				switch (attCount)
				{
				case 0:
				{
					path = str;
				}
				break;
				case 1:
				{
					ss >> imageUpdateIntervalMS;
				}
				break;

				}
			}
		}
		return TRUE;
	}
}

long ThorConfocalGalvoSimulatorXML::GetTilesDimension(string& path, long& row, long& col, long& zSteps)
{
	string expF = path + "\\Experiment.xml";
	OpenConfigFile(expF);
	ticpp::Element *configObj = _xmlObj->FirstChildElement("ThorImageExperiment");

	if ( configObj == NULL )
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
#endif
		return FALSE;
	}
	else
	{
		string nam, isE, subR, subC, zSte;
		ticpp::Iterator< ticpp::Element > child0(configObj, "ZStage");
		child0.begin(configObj)->GetAttribute("steps", &zSte);

		ticpp::Element *SampleObj = configObj->FirstChildElement("Sample");
		ticpp::Element *WellsObj = SampleObj->FirstChildElement("Wells");
		ticpp::Iterator< ticpp::Element > child(WellsObj, "SubImages");

		for(child = child.begin(WellsObj); child != child.end(); child++)
		{
			child->GetAttribute("name", &nam);
			child->GetAttribute("isEnabled", &isE);
			child->GetAttribute("subRows", &subR);
			child->GetAttribute("subColumns", &subC);			 
		}

		if(nam.compare("Tiles") == 0 && isE.compare("True") == 0)
		{
			stringstream ssR(subR);
			ssR >> row;
			stringstream ssC(subC);
			ssC >> col;
			stringstream ssZ(zSte);
			ssZ >> zSteps;
		}

		return TRUE; 	  
	}	
}

long ThorConfocalGalvoSimulatorXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	

	return TRUE;
}

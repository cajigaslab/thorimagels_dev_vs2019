#include "stdafx.h"
#include "CoherentChameleonSetupXML.h"
#include "Strsafe.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

CoherentChameleonXML::CoherentChameleonXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

CoherentChameleonXML::~CoherentChameleonXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const CoherentChameleonXML::CONNECTION = "Connection";

const char * const CoherentChameleonXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","secondLaserLine","laserMin","laserMax"};

long CoherentChameleonXML::GetConnection(long &portID, long &secondLaserLine, long &laserMin, long &laserMax)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"CoherentChameleonSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	OpenConfigFile(s);
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
#ifdef LOGGING_ENABLED
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
#endif
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,CONNECTION);


		for (child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_CONNECTION_ATTRIBUTES; attCount++)
			{
				string str;		

				try
				{
					switch(attCount)
					{
					case 0:
						{
							child->GetAttribute(CONNECTION_ATTR[attCount],&str);	
							stringstream ss(str);
							ss>>portID;		  
						}
						break;
					case 1:
						{
							child->GetAttribute(CONNECTION_ATTR[attCount],&str);	
							stringstream ss(str);
							ss>>secondLaserLine;		  
						}
						break;
					case 2:
						{
							child->GetAttribute(CONNECTION_ATTR[attCount],&str);	
							stringstream ss(str);
							ss>>laserMin;		  
						}
						break;
					case 3:
						{
							child->GetAttribute(CONNECTION_ATTR[attCount],&str);	
							stringstream ss(str);
							ss>>laserMax;		  
						}
						break;
					}
				}
				catch(...)
				{

				}
			}  
		}		  
		return TRUE;
	}	
	return FALSE;
}


long CoherentChameleonXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	

	return TRUE;
}

#include "stdafx.h"
#include "OTMLaserSetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

OTMLaserXML::OTMLaserXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

OTMLaserXML::~OTMLaserXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const OTMLaserXML::CONNECTION = "Connection";

const char * const OTMLaserXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

long OTMLaserXML::GetConnection(long &portID)
{	
	OpenConfigFile();
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

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
		  	  string str;		

			  child->GetAttribute(CONNECTION_ATTR[0],&str);	
			  stringstream ss(str);
			  ss>>portID;		  
              return TRUE;  
		  }		  
	  }	
	  return FALSE;
}

const char * const OTMLaserXML::LASERCONFIG = "LaserConfig";

const char * const OTMLaserXML::LASERCONFIG_ATTR[NUM_LASERCONFIG_ATTRIBUTES] = {"RS232"};

/// <summary>
/// Gets the laser configuration.
/// </summary>
/// <param name="RS232">The maximum mm.</param>
/// <returns>long.</returns>
long OTMLaserXML::GetLaserConfig(long &IsRS232)
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
		ticpp::Iterator< ticpp::Element > child(configObj,LASERCONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_LASERCONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(LASERCONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>IsRS232;
					}
					break;

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
long OTMLaserXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"OTMLaserSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

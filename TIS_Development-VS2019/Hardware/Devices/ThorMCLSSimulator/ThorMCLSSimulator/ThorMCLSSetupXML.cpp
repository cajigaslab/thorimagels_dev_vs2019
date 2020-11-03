#include "stdafx.h"
#include "ThorMCLSSetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorMCLSSimulatorXML::ThorMCLSSimulatorXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

ThorMCLSSimulatorXML::~ThorMCLSSimulatorXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorMCLSSimulatorXML::CONNECTION = "Connection";

const char * const ThorMCLSSimulatorXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

long ThorMCLSSimulatorXML::GetConnection(long &portID)
{	
	wsprintf(_currentPathAndFile,L"ThorMCLSSimulatorSettings.xml");		

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


long ThorMCLSSimulatorXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

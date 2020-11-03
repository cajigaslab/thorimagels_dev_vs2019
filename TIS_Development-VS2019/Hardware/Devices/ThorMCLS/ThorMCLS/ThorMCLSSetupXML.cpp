#include "stdafx.h"
#include "ThorMCLSSetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorMCLSXML::ThorMCLSXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

ThorMCLSXML::~ThorMCLSXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorMCLSXML::CONNECTION = "Connection";

const char * const ThorMCLSXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

long ThorMCLSXML::GetConnection(long &portID)
{	

	wsprintf(_currentPathAndFile,L"ThorMCLSSettings.xml");		

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


long ThorMCLSXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

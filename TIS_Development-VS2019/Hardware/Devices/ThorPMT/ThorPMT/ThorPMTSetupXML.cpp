#include "stdafx.h"
#include "ThorPMTSetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorPMTXML::ThorPMTXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorPMTXML::~ThorPMTXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorPMTXML::CONNECTION = "Connection";

const char * const ThorPMTXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

long ThorPMTXML::GetConnection(long &portID)
{	
	wsprintf(_currentPathAndFile,L"ThorPMTSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

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

const char * const ThorPMTXML::CONFIGURATION = "Configuration";

const char * const ThorPMTXML::CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES] = {"rsInitMode"};

// get resonance scanner initial mode from .xml settings file
long ThorPMTXML::GetConfiguration(long &rsInitMode)
{	
	wsprintf(_currentPathAndFile,L"ThorPMTSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

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
		  ticpp::Iterator< ticpp::Element > child(configObj,CONFIGURATION);

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
		  	  string str;		

			  child->GetAttribute(CONFIGURATION_ATTR[0],&str);	
			  stringstream ss(str);
			  ss>>rsInitMode;	// 0 off; 1 on	  
              return TRUE;  
		  }		  
	  }	
	  return FALSE;
}

long ThorPMTXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

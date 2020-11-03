#include "stdafx.h"
#include "ThorPMT2SetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorPMT2XML::ThorPMT2XML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorPMT2XML::~ThorPMT2XML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorPMT2XML::CONNECTION = "Connection";

const char * const ThorPMT2XML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

long ThorPMT2XML::GetConnection(long &portID)
{	
	wsprintf(_currentPathAndFile,L"ThorPMT2Settings.xml");		

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


long ThorPMT2XML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

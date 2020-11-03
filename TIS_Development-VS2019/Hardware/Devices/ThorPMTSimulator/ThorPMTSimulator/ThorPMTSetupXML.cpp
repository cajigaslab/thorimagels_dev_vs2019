#include "stdafx.h"
#include "ThorPMTSetupXML.h"



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

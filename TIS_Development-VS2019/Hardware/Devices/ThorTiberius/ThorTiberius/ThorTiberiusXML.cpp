#include "stdafx.h"
#include "ThorTiberiusXML.h"
#include "Strsafe.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

ThorTiberiusXML::ThorTiberiusXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorTiberiusXML::~ThorTiberiusXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorTiberiusXML::CONNECTION = "Connection";

const char * const ThorTiberiusXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

const char * const ThorTiberiusXML::WAVELENGTH = "Wavelength";

const char * const ThorTiberiusXML::WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES] = {"wavelengthMin","wavelengthMax"};




long ThorTiberiusXML::GetConnection(long &portID)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorTiberiusSettings.xml");		

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


long ThorTiberiusXML::GetWavelength(long &wavelengthMin,  long &wavelengthMax)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if(configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,WAVELENGTH);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount=0; attCount<NUM_WAVELENGTH_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(WAVELENGTH_ATTR[attCount],&str);	
				stringstream ss(str);
				switch(attCount)
				{
					case 0: ss>>wavelengthMin; break;
					case 1: ss>>wavelengthMax; break;
				

				}
			}
		}
	}
	return TRUE;
}




long ThorTiberiusXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

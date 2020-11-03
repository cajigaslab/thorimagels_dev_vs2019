#include "stdafx.h"
#include "PowerControlXML.h"


PowerControlXML::PowerControlXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

PowerControlXML::~PowerControlXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const PowerControlXML::CONNECTION = "Connection";

const char * const PowerControlXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

long PowerControlXML::GetConnection(long &portID,long &baudRate, long &address)
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
		  ticpp::Iterator< ticpp::Element > child(configObj,CONNECTION);

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
			  for(long attCount = 0; attCount<NUM_CONNECTION_ATTRIBUTES; attCount++)
			  {

				  string str;
				  child->GetAttribute(CONNECTION_ATTR[attCount],&str);
				  stringstream ss(str);
				  switch(attCount)
				  {
				  case 0:
					  {
						  ss>>portID;
					  }
					  break;

				  case 1:
					  {

						  ss>>baudRate;
					  }
					  break;

				  case 2:
					  {
						  ss>>address;
					  }
					  break;
				  }
			  }

		  }		  
	  }	
	  return TRUE;
}

const char * const PowerControlXML::PowerLevel = "PowerLevel";

const char * const PowerControlXML::POWERLEVEL_ATTR[NUM_POWERLEVEL_ATTRIBUTES] = {"value"};

long PowerControlXML::GetPowerLevel(long &value)
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
		  ticpp::Iterator< ticpp::Element > child(configObj,PowerLevel);

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
			  for(long attCount = 0; attCount<NUM_POWERLEVEL_ATTRIBUTES; attCount++)
			  {

				  string str;
				  child->GetAttribute(POWERLEVEL_ATTR[attCount],&str);
				  stringstream ss(str);
				  switch(attCount)
				  {
				  case 0:
					  {
						  ss>>value;
					  }
					  break;
				  }
			  }

		  }		  
	  }	
	  return TRUE;
}
long PowerControlXML::SetPowerLevel(long value)
{
	OpenConfigFile();
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	long curID = 0;
      if ( configObj == NULL )
	  {
		  return FALSE;
	  }
	  else
	  {
		  ticpp::Iterator< ticpp::Element > child(configObj,PowerLevel);

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
			  for(long attCount = 0; attCount<NUM_POWERLEVEL_ATTRIBUTES; attCount++)
			  {
				  stringstream ss;
				  switch(attCount)
				  {
				  case 0:
					  {
						  ss<<value;
						  child->SetAttribute(POWERLEVEL_ATTR[attCount],ss.str());
					  }
					  break;
				  }
			  }
		  }		  
	  }	
	  SaveConfigFile();
	  return TRUE;
}

long PowerControlXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"PowerControlSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj != NULL)
	{
		return TRUE;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

long PowerControlXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}

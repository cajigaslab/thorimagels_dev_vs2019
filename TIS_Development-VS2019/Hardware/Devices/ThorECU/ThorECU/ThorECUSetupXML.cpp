#include "stdafx.h"
#include "ThorECUSetupXML.h"


#ifdef LOGGING_ENABLED
extern auto_ptr<LogDll> logDll;
#endif 

/// <summary>
/// Initializes a new instance of the <see cref="ThorECUXML"/> class.
/// </summary>
ThorECUXML::ThorECUXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorECUXML"/> class.
/// </summary>
ThorECUXML::~ThorECUXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorECUXML::CONNECTION = "Connection";

const char * const ThorECUXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorECUXML::GetConnection(long &portID)
{	
	wsprintf(_currentPathAndFile,L"ThorECUSettings.xml");		

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

const char * const ThorECUXML::CONFIGURATION = "Configuration";

const char * const ThorECUXML::CONFIGURATION_ATTR[NUM_CONFIGURATION_ATTRIBUTES] = {"rsInitMode"};

// get resonance scanner initial mode from .xml settings file
/// <summary>
/// Gets the configuration.
/// </summary>
/// <param name="rsInitMode">The resonance scanner initialize mode. 0 - Resonance scanner 'toggle' mode.  1- Resonance Scanner 'always on' mode</param>
/// <returns>long.</returns>
long ThorECUXML::GetConfiguration(long &rsInitMode)
{	
	wsprintf(_currentPathAndFile,L"ThorECUSettings.xml");		

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



/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ThorECUXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

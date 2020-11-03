#include "stdafx.h"
#include "MLSStageSetupXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="MLSStageXML"/> class.
/// </summary>
MLSStageXML::MLSStageXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="MLSStageXML"/> class.
/// </summary>
MLSStageXML::~MLSStageXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const MLSStageXML::CONNECTION = "Connection";

const char * const MLSStageXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long MLSStageXML::GetConnection(long &portID,long &baudRate, long &address)
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


/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long MLSStageXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorMLSStageSettings.xml");		

	string s = ConvertWStringToString(_currentPathAndFile);

	if(_xmlObj != NULL)
	{
		return TRUE;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long MLSStageXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}


const char * const MLSStageXML::XAXISCONFIG = "XAxisConfig";

const char * const MLSStageXML::XAXISCONFIG_ATTR[NUM_XAXISCONFIG_ATTRIBUTES] = {"invert"};

/// <summary>
/// Gets the x axis configuration.
/// </summary>
/// <param name="invert">The reverse config.</param>
/// <returns>long.</returns>
long MLSStageXML:: GetXAxisConfig (bool &invert)
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
		ticpp::Iterator< ticpp::Element > child(configObj,XAXISCONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_XAXISCONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(XAXISCONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>invert;
					}
					break;
				}
			}
		}
	}	
	return TRUE;
}

const char * const MLSStageXML::YAXISCONFIG = "YAxisConfig";

const char * const MLSStageXML::YAXISCONFIG_ATTR[NUM_YAXISCONFIG_ATTRIBUTES] = {"invert"};
/// <summary>
/// Gets the x axis configuration.
/// </summary>
/// <param name="invert">The reverse config.</param>
/// <returns>long.</returns>
long MLSStageXML:: GetYAxisConfig (bool &invert)
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
		ticpp::Iterator< ticpp::Element > child(configObj,YAXISCONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_YAXISCONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(YAXISCONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>invert;
					}
					break;
				}
			}
		}
	}	
	return TRUE;
}
#include "stdafx.h"
#include "ThorBScopeXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorBScopeXML"/> class.
/// </summary>
ThorBScopeXML::ThorBScopeXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorBScopeXML"/> class.
/// </summary>
ThorBScopeXML::~ThorBScopeXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorBScopeXML::CONNECTION = "Connection";

const char * const ThorBScopeXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorBScopeXML::GetConnection(long &portID,long &baudRate, long &address)
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

const char * const ThorBScopeXML::RRANGECONFIG = "RRangeConfig";

const char * const ThorBScopeXML::RRANGECONFIG_ATTR[NUM_RRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM"};

/// <summary>
/// Gets the r range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <returns>long.</returns>
long ThorBScopeXML::GetRRangeConfig(double &minMM, double &maxMM)
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
		ticpp::Iterator< ticpp::Element > child(configObj,RRANGECONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_RRANGECONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(RRANGECONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>minMM;
					}
					break;

				case 1:
					{

						ss>>maxMM;
					}
					break;
				}
			}
		}		  
	}	
	  return TRUE;
}

const char * const ThorBScopeXML::XRANGECONFIG = "XRangeConfig";

const char * const ThorBScopeXML::XRANGECONFIG_ATTR[NUM_XRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the x range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorBScopeXML::GetXRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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
		ticpp::Iterator< ticpp::Element > child(configObj,XRANGECONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_XRANGECONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(XRANGECONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>minMM;
					}
					break;

				case 1:
					{

						ss>>maxMM;
					}
					break;

				case 2:
					{
						ss>>threshold;
					}
					break;
				case 3:
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

const char * const ThorBScopeXML::YRANGECONFIG = "YRangeConfig";

const char * const ThorBScopeXML::YRANGECONFIG_ATTR[NUM_YRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the y range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorBScopeXML::GetYRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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
		ticpp::Iterator< ticpp::Element > child(configObj,YRANGECONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_YRANGECONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(YRANGECONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>minMM;
					}
					break;

				case 1:
					{

						ss>>maxMM;
					}
					break;

				case 2:
					{
						ss>>threshold;
					}
					break;
				case 3:
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




const char * const ThorBScopeXML::ZRANGECONFIG = "ZRangeConfig";

const char * const ThorBScopeXML::ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the z range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorBScopeXML::GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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
		ticpp::Iterator< ticpp::Element > child(configObj,ZRANGECONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_ZRANGECONFIG_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(ZRANGECONFIG_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>minMM;
					}
					break;

				case 1:
					{

						ss>>maxMM;
					}
					break;

				case 2:
					{
						ss>>threshold;
					}
					break;
				case 3:
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

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorBScopeXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorBScopeSettings.xml");		

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
long ThorBScopeXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}

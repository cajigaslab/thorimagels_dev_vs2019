#include "stdafx.h"
#include "ThorMTS25XML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorMTS25XML"/> class.
/// </summary>
ThorMTS25XML::ThorMTS25XML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorMTS25XML"/> class.
/// </summary>
ThorMTS25XML::~ThorMTS25XML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorMTS25XML::CONNECTION = "Connection";

const char * const ThorMTS25XML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorMTS25XML::GetConnection(long &portID,long &baudRate, long &address)
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




const char * const ThorMTS25XML::ZRANGECONFIG = "ZRangeConfig";

const char * const ThorMTS25XML::ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert","home"};

/// <summary>
/// Gets the z range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorMTS25XML::GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert, bool &home)
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
				case 4:
					{
						ss>>home;
					}
					break;
				}
			}
		}		  
	}	
	  return TRUE;
}


const char * const ThorMTS25XML::SLEEPAFTERMOVE = "SleepAfterMove";

const char * const ThorMTS25XML::SLEEPAFTERMOVE_ATTR[NUM_SLEEPAFTERMOVE_ATTRIBUTES] = {"time_milliseconds"};

/// <summary>
/// Gets the Sleep time after a move is complete
/// </summary>
/// <param name="time_milliseconds">Time in milliseconds.</param>
/// <returns>long.</returns>
long ThorMTS25XML::GetSleepAfterMove(long &time_milliseconds)
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
		ticpp::Iterator< ticpp::Element > child(configObj,SLEEPAFTERMOVE);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_SLEEPAFTERMOVE_ATTRIBUTES; attCount++)
			{

				string str;
				child->GetAttribute(SLEEPAFTERMOVE_ATTR[attCount],&str);
				stringstream ss(str);
				switch(attCount)
				{
				case 0:
					{
						ss>>time_milliseconds;
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
long ThorMTS25XML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorMTS25Settings.xml");		

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
long ThorMTS25XML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}

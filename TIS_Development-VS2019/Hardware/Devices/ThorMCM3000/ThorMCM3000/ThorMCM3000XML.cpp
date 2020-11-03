#include "stdafx.h"
#include "ThorMCM3000XML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ThorMCM3000XML"/> class.
/// </summary>
ThorMCM3000XML::ThorMCM3000XML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorMCM3000XML"/> class.
/// </summary>
ThorMCM3000XML::~ThorMCM3000XML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorMCM3000XML::CONNECTION = "Connection";

const char * const ThorMCM3000XML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetConnection(long &portID,long &baudRate, long &address)
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

const char * const ThorMCM3000XML::STAGEAXISCONFIG = "StageAxisConfig";

const char * const ThorMCM3000XML::STAGEAXISCONFIG_ATTR[NUM_STAGEAXISCONFIG_ATTRIBUTES] = {"Stage1","Stage2","Stage3"};

/// <summary>
/// Gets the xyz stage axes.
/// </summary>
/// <param name="axis0">The axis0.</param>
/// <param name="axis1">The axis1.</param>
/// <param name="axis2">The axis2.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetXYZStageAxes(long &axis0,long &axis1, long &axis2)
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
		ticpp::Iterator< ticpp::Element > child(configObj,STAGEAXISCONFIG);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_STAGEAXISCONFIG_ATTRIBUTES; attCount++)
			{
				string str;
				child->GetAttribute(STAGEAXISCONFIG_ATTR[attCount],&str);
				stringstream ss(str);				  
				switch(attCount)
				{
				case 0:
					{
						if(str == "x" || str == "X")
						{
							axis0 = IDevice::STAGE_X;
						}
						else if(str == "y" || str == "Y")
						{
							axis0 = IDevice::STAGE_Y;
						}
						else if(str == "z" || str == "Z")
						{
							axis0 = IDevice::STAGE_Z;
						}
					}
					break;

				case 1:
					{
						if(str == "x" || str == "X")
						{
							axis1 = IDevice::STAGE_X;
						}
							else if(str == "y" || str == "Y")
						{
							axis1 = IDevice::STAGE_Y;
						}
							else if(str == "z" || str == "Z")
						{
							axis1 = IDevice::STAGE_Z;
						}
					}
					break;
				case 2:
					{
						if(str == "x" || str == "X")
						{
							axis2 = IDevice::STAGE_X;
						}
						else if(str == "y" || str == "Y")
						{
							axis2 = IDevice::STAGE_Y;
						}
						else if(str == "z" || str == "Z")
						{
							axis2 = IDevice::STAGE_Z;
						}
					}
					break;
				}
			}
		}	  
	}	
	return TRUE;
}

const char * const ThorMCM3000XML::XRANGECONFIG = "XRangeConfig";

const char * const ThorMCM3000XML::XRANGECONFIG_ATTR[NUM_XRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the x range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetXRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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

const char * const ThorMCM3000XML::YRANGECONFIG = "YRangeConfig";

const char * const ThorMCM3000XML::YRANGECONFIG_ATTR[NUM_YRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the y range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetYRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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




const char * const ThorMCM3000XML::ZRANGECONFIG = "ZRangeConfig";

const char * const ThorMCM3000XML::ZRANGECONFIG_ATTR[NUM_ZRANGECONFIG_ATTRIBUTES] = {"minMM","maxMM", "threshold", "invert"};

/// <summary>
/// Gets the z range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetZRangeConfig(double &minMM, double &maxMM, double &threshold, bool &invert)
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


const char * const ThorMCM3000XML::SLEEPAFTERMOVE = "SleepAfterMove";

const char * const ThorMCM3000XML::SLEEPAFTERMOVE_ATTR[NUM_SLEEPAFTERMOVE_ATTRIBUTES] = {"time_milliseconds"};

/// <summary>
/// Gets the Sleep time after a move is complete
/// </summary>
/// <param name="time_milliseconds">Time in milliseconds.</param>
/// <returns>long.</returns>
long ThorMCM3000XML::GetSleepAfterMove(long &time_milliseconds)
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
long ThorMCM3000XML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorMCM3000Settings.xml");		

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
long ThorMCM3000XML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}

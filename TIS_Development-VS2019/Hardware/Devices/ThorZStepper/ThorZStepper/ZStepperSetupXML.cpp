#include "stdafx.h"
#include "ZStepperSetupXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="ZStepperXML"/> class.
/// </summary>
ZStepperXML::ZStepperXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

/// <summary>
/// Finalizes an instance of the <see cref="ZStepperXML"/> class.
/// </summary>
ZStepperXML::~ZStepperXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ZStepperXML::CONNECTION = "Connection";

const char * const ZStepperXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ZStepperXML::GetConnection(long &portID,long &baudRate, long &address)
{	
	wsprintf(_currentPathAndFile,L"ZStepperSettings.xml");		

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

const char * const ZStepperXML::STEPCONFIG = "StepConfig";

const char * const ZStepperXML::STEPCONFIG_ATTR[NUM_STEPCONFIG_ATTRIBUTES] = {"microstep","prescaler","stepToEncoder","mmPerRot"};

/// <summary>
/// Gets the step configuration.
/// </summary>
/// <param name="microstep">The microstep.</param>
/// <param name="prescaler">The prescaler.</param>
/// <param name="stepToEncoder">The step to encoder.</param>
/// <param name="mmPerRot">The mm per rot.</param>
/// <returns>long.</returns>
long ZStepperXML::GetStepConfig(long &microstep,double &prescaler, double &stepToEncoder, double &mmPerRot)
{	
	wsprintf(_currentPathAndFile,L"ZStepperSettings.xml");		

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
		  ticpp::Iterator< ticpp::Element > child(configObj,STEPCONFIG);

		  for ( child = child.begin( configObj ); child != child.end(); child++)
		  {
			  for(long attCount = 0; attCount<NUM_STEPCONFIG_ATTRIBUTES; attCount++)
			  {

				  string str;
				  child->GetAttribute(STEPCONFIG_ATTR[attCount],&str);
				  stringstream ss(str);
				  switch(attCount)
				  {
				  case 0:
					  {
						  ss>>microstep;
					  }
					  break;

				  case 1:
					  {

						  ss>>prescaler;
					  }
					  break;

				  case 2:
					  {
						  ss>>stepToEncoder;
					  }
					  break;
				  case 3:
					  {
						  ss>>mmPerRot;
					  }
					  break;
				  }
			  }

		  }		  
	  }	
	  return TRUE;
}


const char * const ZStepperXML::RANGECONFIG = "RangeConfig";

const char * const ZStepperXML::RANGECONFIG_ATTR[NUM_RANGECONFIG_ATTRIBUTES] = {"minMM","maxMM","threshold","invert"};

/// <summary>
/// Gets the range configuration.
/// </summary>
/// <param name="minMM">The minimum mm.</param>
/// <param name="maxMM">The maximum mm.</param>
/// <param name="threshold">The threshold.</param>
/// <param name="invert">Invert the direction of motion</param>
/// <returns>long.</returns>
long ZStepperXML::GetRangeConfig(double &minMM, double &maxMM, double &threshold, long &invert)
{	
	wsprintf(_currentPathAndFile,L"ZStepperSettings.xml");		

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
		  try
		  {
			  ticpp::Iterator< ticpp::Element > child(configObj,RANGECONFIG);

			  for ( child = child.begin( configObj ); child != child.end(); child++)
			  {
				  for(long attCount = 0; attCount<NUM_RANGECONFIG_ATTRIBUTES; attCount++)
				  {

					  string str;
					  child->GetAttribute(RANGECONFIG_ATTR[attCount],&str);
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
		  catch(ticpp::Exception ex)
		  {
			//wchar_t errMsg[MSG_SIZE];
			//StringCbPrintfW(errMsg,MSG_SIZE,L"GetRangeConfig ticpp exception");
			//ThorLSMCam::getInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		  }
	  }	
	  return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ZStepperXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	
	
	return TRUE;
}

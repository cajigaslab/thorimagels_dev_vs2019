#include "stdafx.h"
#include "PinholeStepperSetupXML.h"


/// <summary>
/// Initializes a new instance of the <see cref="PinholeStepperXML"/> class.
/// </summary>
PinholeStepperXML::PinholeStepperXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

/// <summary>
/// Finalizes an instance of the <see cref="PinholeStepperXML" /> class.
/// </summary>
PinholeStepperXML::~PinholeStepperXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const PinholeStepperXML::CONNECTION = "Connection";

const char * const PinholeStepperXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate","address"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long PinholeStepperXML::GetConnection(long &portID,long &baudRate, long &address)
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

const char * const PinholeStepperXML::PINHOLELOC = "PinholeLoc";

const char * const PinholeStepperXML::PINHOLELOC_ATTR[NUM_PINHOLELOC_ATTRIBUTES] = {"value"};

/// <summary>
/// Gets the pinhole location.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="value">The value.</param>
/// <returns>long.</returns>
long PinholeStepperXML::GetPinholeLocation(long id, long &value)
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
		  ticpp::Iterator< ticpp::Element > child(configObj,PINHOLELOC);
		 
		  for ( child = child.begin( configObj ); child != child.end(); child++,curID++)
		  {
			  if(curID == id)
			  {
				  for(long attCount = 0; attCount<NUM_PINHOLELOC_ATTRIBUTES; attCount++)
				  {

					  string str;
					  child->GetAttribute(PINHOLELOC_ATTR[attCount],&str);
					  stringstream ss(str);
					  switch(attCount)
					  {
					  case 0:
						  {
							  ss>>value;
							  return TRUE;
						  }
						  break;
					  }
				  }
			  }

		  }		  
	  }	
	  return FALSE;
}

/// <summary>
/// Sets the pinhole location.
/// </summary>
/// <param name="id">The identifier.</param>
/// <param name="value">The value.</param>
/// <returns>long.</returns>
long PinholeStepperXML::SetPinholeLocation(long id, long value)
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
		  ticpp::Iterator< ticpp::Element > child(configObj,PINHOLELOC);
		 
		  for ( child = child.begin( configObj ); child != child.end(); child++,curID++)
		  {
			  if(curID == id)
			  {
				  for(long attCount = 0; attCount<NUM_PINHOLELOC_ATTRIBUTES; attCount++)
				  {
					  stringstream ss;
					  ss << value;
					  child->SetAttribute(PINHOLELOC_ATTR[attCount],ss.str());
				  }
			  }

		  }		  
	  }	
	  return FALSE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <returns>long.</returns>
long PinholeStepperXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorPinholeStepperSettings.xml");		

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

/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long PinholeStepperXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	
	
	return TRUE;
}

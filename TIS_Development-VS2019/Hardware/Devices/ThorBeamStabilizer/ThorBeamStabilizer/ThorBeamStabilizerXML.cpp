#include "stdafx.h"
#include "ThorBeamStabilizerXML.h"


ThorBeamStabilizerXML::ThorBeamStabilizerXML() :
	_xmlObj(nullptr)
{
	_currentPathAndFile[0] = NULL;
}


ThorBeamStabilizerXML::~ThorBeamStabilizerXML()
{
	if(nullptr != _xmlObj)
	{
		delete _xmlObj;
	}
}


const char * const ThorBeamStabilizerXML::CONNECTION = "PiezoActuatorsConnection";

const char * const ThorBeamStabilizerXML::CONNECTION_ATTR[NUM_CONNECTION_ATTRIBUTES] = {"portID","baudRate"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <param name="baudRate">The baud rate.</param>
/// <param name="address">The address.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::GetPiezoActuatorsConnection(long &portID,long &baudRate)
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

				std::string str;
				child->GetAttribute(CONNECTION_ATTR[attCount],&str);
				switch(attCount)
				{
				case 0: portID = std::stoi(str); break;
				case 1: baudRate = std::stoi(str); break;
				}
			}
		}		  
	}	

	return TRUE;
}


const char* const ThorBeamStabilizerXML::ACTUATORS_POSITION = "PiezoActuatorPositions";

const char* const ThorBeamStabilizerXML::ACTUATORS_POSITION_ATTR[NUM_ACTUATORS_POSITION_ATTRIBUTES] = {"piezo1", "piezo2", "piezo3", "piezo4"};

/// <summary>
/// Gets the position for each of the four piezos.
/// </summary>
/// <param name="piezo1">position for actuator 1.</param>
/// <param name="piezo2">position for actuator 2.</param>
/// <param name="piezo3">position for actuator 3.</param>
/// <param name="piezo4">position for actuator 4.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::GetPiezoActuatorPositions(long &piezo1, long &piezo2, long &piezo3, long &piezo4)
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
		ticpp::Iterator< ticpp::Element > child(configObj, ACTUATORS_POSITION);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_ACTUATORS_POSITION_ATTRIBUTES; attCount++)
			{

				std::string str;
				child->GetAttribute(ACTUATORS_POSITION_ATTR[attCount],&str);
				switch(attCount)
				{
				case 0: piezo1 = std::stoi(str); break;
				case 1: piezo2 = std::stoi(str); break;
				case 2: piezo3 = std::stoi(str); break;
				case 3: piezo4 = std::stoi(str); break;
				}
			}
		}		  
	}	

	return TRUE;
}


/// <summary>
/// Sets the position for each of the four piezos.
/// </summary>
/// <param name="piezo1">position for actuator 1.</param>
/// <param name="piezo2">position for actuator 2.</param>
/// <param name="piezo3">position for actuator 3.</param>
/// <param name="piezo4">position for actuator 4.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::SetPiezoActuatorPositions(long piezo1, long piezo2, long piezo3, long piezo4)
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
		std::string str;
		std::stringstream ss;

		ss << piezo1;
		ss << std::endl;
		ss << piezo2;
		ss << std::endl;
		ss << piezo3;
		ss << std::endl;
		ss << piezo4;
		ss << std::endl;

		for(long index=0; index<NUM_ACTUATORS_POSITION_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(ACTUATORS_POSITION), ACTUATORS_POSITION);
			//get the attribute value for the specified attribute name
			child->SetAttribute(ACTUATORS_POSITION_ATTR[index], str);
		}	  
	}	

	return TRUE;
}



const char * const ThorBeamStabilizerXML::BEAM_PROFILER_ATTR[NUM_BEAM_PROFILE_SETTINGS_ATTRIBUTES] = {"serialNumber"};

/// <summary>
/// Gets the connection.
/// </summary>
/// <param name="portID">The port identifier.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::GetBeamProfilerSerialNumber(const std::string signature, std::string &deviceAddress)
{	
	OpenConfigFile();

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL) return FALSE;

	ticpp::Iterator< ticpp::Element > child(configObj, signature.c_str());

	for ( child = child.begin( configObj ); child != child.end(); child++)
	{
		for(long attCount = 0; attCount<NUM_BEAM_PROFILE_SETTINGS_ATTRIBUTES; attCount++)
		{
			std::string str;
			child->GetAttribute(BEAM_PROFILER_ATTR[attCount], &str);

			switch(attCount)
			{
			case 0: deviceAddress = str; break;
			}
		}
	}		
	return FALSE;
}

const char* const ThorBeamStabilizerXML::CONTROL_SETTINGS = "ControlSettings";

const char* const ThorBeamStabilizerXML::CONTROL_SETTINGS_ATTR[NUM_CONTROL_SETTINGS_ATTRIBUTES] = {"piezo1Orientation", "piezo2Orientation", "piezo3Orientation", "piezo4Orientation", "deadband", "p1Term", "p2Term", "maxExposureTime", "minExposureTime", "clipLevel", "alignTimeOutSec"};

/// <summary>
/// Gets the position for each of the four piezos.
/// </summary>
/// <param name="piezo1">position for actuator 1.</param>
/// <param name="piezo2">position for actuator 2.</param>
/// <param name="piezo3">position for actuator 3.</param>
/// <param name="piezo4">position for actuator 4.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::GetControlSettings(long &piezo1Orientation, long &piezo2Orientation, long &piezo3Orientation, long &piezo4Orientation, long &deadband, double &p1Term, double &p2Term, double &maxExposureTime, double &minExposureTime, double &clipLevel, long &alignTimeOutSec)
{	
	OpenConfigFile();

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( NULL == configObj )
	{
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj, CONTROL_SETTINGS);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_CONTROL_SETTINGS_ATTRIBUTES; attCount++)
			{

				std::string str;
				child->GetAttribute(CONTROL_SETTINGS_ATTR[attCount],&str);
				switch(attCount)
				{					
				case 0: piezo1Orientation = (str.size() && '-' == str[0]) ? -1 : 1; break;
				case 1: piezo2Orientation = (str.size() && '-' == str[0]) ? -1 : 1; break;
				case 2: piezo3Orientation = (str.size() && '-' == str[0]) ? -1 : 1; break;
				case 3: piezo4Orientation = (str.size() && '-' == str[0]) ? -1 : 1; break;
				case 4: deadband = std::stoi(str); break;
				case 5: p1Term = std::stod(str); break;
				case 6: p2Term = std::stod(str); break;
				case 7: maxExposureTime = std::stoi(str); break;
				case 8: minExposureTime = std::stoi(str); break;
				case 9: clipLevel = std::stod(str); break;
				case 10: alignTimeOutSec = std::stoi(str); break;
				}
			}
		}		  
	}	

	return TRUE;
}


const char* const ThorBeamStabilizerXML::FACTORY_ACTUATORS_POSITION = "FactoryPiezoActuatorPositions";

const char* const ThorBeamStabilizerXML::FACTORY_ACTUATORS_POSITION_ATTR[NUM_FACTORY_ACTUATORS_POSITION_ATTRIBUTES] = {"piezo1", "piezo2", "piezo3", "piezo4","piezoStepLimit"};

/// <summary>
/// Gets the position for each of the four piezos.
/// </summary>
/// <param name="piezo1">position for actuator 1.</param>
/// <param name="piezo2">position for actuator 2.</param>
/// <param name="piezo3">position for actuator 3.</param>
/// <param name="piezo4">position for actuator 4.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::GetFactoryPiezoActuatorPositions(long &piezo1, long &piezo2, long &piezo3, long &piezo4, long &piezoStepLimit)
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
		ticpp::Iterator< ticpp::Element > child(configObj, FACTORY_ACTUATORS_POSITION);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			for(long attCount = 0; attCount<NUM_FACTORY_ACTUATORS_POSITION_ATTRIBUTES; attCount++)
			{

				std::string str;
				child->GetAttribute(FACTORY_ACTUATORS_POSITION_ATTR[attCount],&str);
				switch(attCount)
				{
				case 0: piezo1 = std::stoi(str); break;
				case 1: piezo2 = std::stoi(str); break;
				case 2: piezo3 = std::stoi(str); break;
				case 3: piezo4 = std::stoi(str); break;
				case 4: piezoStepLimit = std::stoi(str); break;
				}
			}
		}		  
	}	

	return TRUE;
}





/// <summary>
/// Saves the configuration file.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::SaveConfigFile()
{	

	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}	

	return TRUE;
}

/// <summary>
/// Opens the configuration file.
/// </summary>
/// <param name="path">The path.</param>
/// <returns>long.</returns>
long ThorBeamStabilizerXML::OpenConfigFile()
{	
	wsprintf(_currentPathAndFile,L"ThorBeamStabilizerSettings.xml");		

	std::string s = ConvertWStringToString(_currentPathAndFile);

	if(nullptr == _xmlObj) 
	{
		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();
	}

	return TRUE;
}
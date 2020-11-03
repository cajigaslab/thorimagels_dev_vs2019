#include "stdafx.h"
#include "ElectroPhysSetupXML.h"


ElectroPhysXML::ElectroPhysXML()
{
	_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;
}

ElectroPhysXML::~ElectroPhysXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}


const char * const ElectroPhysXML::IO = "IO";

const char * const ElectroPhysXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"devName","digitalPort"};

long ElectroPhysXML::GetIO(std::string &devName, std::string &digitalPort)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,IO);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_IO_ATTRIBUTES; attCount++)
				{
					std::string str;		
					child->GetAttribute(IO_ATTR[attCount],&str);	
					if(std::all_of(str.begin(), str.end(), isspace)) str.clear();
					switch(attCount)
					{
					case 0: devName = str; break;
					case 1: digitalPort = str; break;
					}
				}
			}		  
		}	
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

const char * const ElectroPhysXML::DIG_OUTPUT = "Output";

const char * const ElectroPhysXML::DIG_OUTPUT_ATTR[NUM_DIG_OUTPUT_ATTRIBUTES] = {"port1","port2","port3","port4","port5","port6","port7","port8"};

long ElectroPhysXML::GetDigOutput(std::string &port1, std::string &port2, std::string &port3, std::string &port4, std::string &port5, std::string &port6, std::string &port7, std::string &port8)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,DIG_OUTPUT);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_DIG_OUTPUT_ATTRIBUTES; attCount++)
				{
					std::string str;		
					child->GetAttribute(DIG_OUTPUT_ATTR[attCount],&str);	
					std::stringstream ss(str);
					switch(attCount)
					{
					case 0: port1 = str; break;
					case 1: port2 = str; break;
					case 2: port3 = str; break;
					case 3: port4 = str; break;
					case 4: port5 = str; break;
					case 5: port6 = str; break;
					case 6: port7 = str; break;
					case 7: port8 = str; break;
					}
				}
			}		  
		}	
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

const char * const ElectroPhysXML::FREQPROBE = "FrequencyProbe";

const char * const ElectroPhysXML::FREQPROBE_ATTR[NUM_FREQPROBE_ATTRIBUTES] = {"probeIntervalSec", "averageCount", "counterLine", "measureLine"};

long ElectroPhysXML::GetFrequencyProbe(double &probeIntervalSec, long &averageCount, std::string &counterLine, std::string &measureLine)

{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,FREQPROBE);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_FREQPROBE_ATTRIBUTES; attCount++)
				{
					std::string str = "";		
					child->GetAttribute(FREQPROBE_ATTR[attCount],&str);
					std::stringstream ss(str);
					switch(attCount)
					{
					case 0:
						ss>>probeIntervalSec;
						break;
					case 1:
						ss>>averageCount;
						break;
					case 2:
						counterLine = str;
						break;
					case 3:
						measureLine = str;
						break;
					default:
						break;
					}
				}
			}		  
		}	
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

const char * const ElectroPhysXML::TRIGGER = "TriggerConfiguration";

const char * const ElectroPhysXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"activeLoadCount", "counterOutput", "outputLine", "edgeOutput", "bufferOutput", "customInput"};

long ElectroPhysXML::GetTriggerConfig(long &activeLoadCount, std::vector<std::string> *triggerConfig)
{
	try
	{
		triggerConfig->clear();

		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,TRIGGER);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<NUM_TRIGGER_ATTRIBUTES; attCount++)
				{
					std::string str = "";		
					child->GetAttribute(TRIGGER_ATTR[attCount],&str);
					std::stringstream ss(str);
					switch(attCount)
					{
					case 0:
						ss>>activeLoadCount;
						break;
					default:
						triggerConfig->push_back(str); 
						break;
					}
				}
			}		  
		}	
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

long ElectroPhysXML::OpenConfigFile()
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	wsprintf(_currentPathAndFile,L"ThorElectroPhysSettings.xml");		

	std::string s = ConvertWStringToString(_currentPathAndFile);

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	return TRUE;
}

long ElectroPhysXML::SaveConfigFile()
{	
	if(_xmlObj != NULL)
	{
		wsprintf(_currentPathAndFile,L"ThorElectroPhysSettings.xml");		
		std::string s = ConvertWStringToString(_currentPathAndFile);
		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;

}

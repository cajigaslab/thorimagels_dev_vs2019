#include "stdafx.h"
#include "ElectroPhysSetupXML.h"
#include "..\..\..\..\Common\stringCPP.h"
#include "..\..\..\..\Tools\tinyxml2\include\tinyxml2.h"

using namespace tinyxml2;

std::unique_ptr<tinyxml2::XMLDocument> _xmlDoc;
char xmlValue[_MAX_PATH];

ElectroPhysXML::ElectroPhysXML()
{
	_currentPathAndFile = "ThorElectroPhysSettings.xml";
}

ElectroPhysXML::~ElectroPhysXML()
{
	_xmlDoc.release();
}

const char* const ElectroPhysXML::IO = "IO";

const char* const ElectroPhysXML::IO_ATTR[NUM_IO_ATTRIBUTES] = { "devName","digitalPort" };

long ElectroPhysXML::GetIO(std::string& devName, std::string& digitalPort)
{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* xElement = root->FirstChildElement(IO);
		if (xElement == NULL)	return FALSE;

		for (int i = 0; i < NUM_IO_ATTRIBUTES; i++)
		{
			const char* att = xElement->Attribute(IO_ATTR[i]);
			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			std::string str(xmlValue);
			switch (i)
			{
			case 0:	devName = str; break;
			case 1:	digitalPort = str; break;
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const ElectroPhysXML::DIG_OUTPUT = "Output";

const char* const ElectroPhysXML::DIG_OUTPUT_ATTR[NUM_DIG_OUTPUT_ATTRIBUTES] = { "port1","port2","port3","port4","port5","port6","port7","port8" };

long ElectroPhysXML::GetDigOutput(std::string& port1, std::string& port2, std::string& port3, std::string& port4, std::string& port5, std::string& port6, std::string& port7, std::string& port8)
{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* xElement = root->FirstChildElement(DIG_OUTPUT);
		if (xElement == NULL)	return FALSE;

		for (int i = 0; i < NUM_DIG_OUTPUT_ATTRIBUTES; i++)
		{
			const char* att = xElement->Attribute(DIG_OUTPUT_ATTR[i]);
			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			std::string str(xmlValue);
			switch (i)
			{
			case 0:	port1 = str; break;
			case 1:	port2 = str; break;
			case 2:	port3 = str; break;
			case 3:	port4 = str; break;
			case 4:	port5 = str; break;
			case 5:	port6 = str; break;
			case 6:	port7 = str; break;
			case 7:	port8 = str; break;
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const ElectroPhysXML::FREQPROBE = "FrequencyProbe";

const char* const ElectroPhysXML::FREQPROBE_ATTR[NUM_FREQPROBE_ATTRIBUTES] = { "probeIntervalSec", "averageCount", "counterLine", "measureLine" };

long ElectroPhysXML::GetFrequencyProbe(double& probeIntervalSec, long& averageCount, std::string& counterLine, std::string& measureLine)

{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* xElement = root->FirstChildElement(FREQPROBE);
		if (xElement == NULL)	return FALSE;

		for (int i = 0; i < NUM_FREQPROBE_ATTRIBUTES; i++)
		{
			const char* att = xElement->Attribute(FREQPROBE_ATTR[i]);
			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			std::stringstream ss(xmlValue);
			switch (i)
			{
			case 0:
				ss >> probeIntervalSec;
				break;
			case 1:
				ss >> averageCount;
				break;
			case 2:
				counterLine = std::string(xmlValue);
				break;
			case 3:
				measureLine = std::string(xmlValue);
				break;
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const ElectroPhysXML::GENERAL = "GeneralSettings";

const char* const ElectroPhysXML::GENERAL_ATTR[NUM_GENERAL_ATTRIBUTES] = { "activeLoadCount", "activeUnitMS", "voltageMin", "voltageMax", "analogResponseType", "parkAnalogLineAtLastVoltage" };

const char* const ElectroPhysXML::TRIGGER = "TriggerConfiguration";

const char* const ElectroPhysXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = { "digitalCounter", "digitalOutput", "analogCounter", "analogOutput", "edgeOutput", "bufferOutput", "customInput" };

long ElectroPhysXML::GetGeneralSettings(long& activeLoadCount, double& activeUnitMS, double& voltageMin, double& voltageMax, long& responseType, long& parkAnalogLineAtLastVoltage)
{
	try
	{
		const char* att;
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* generalElement = root->FirstChildElement(GENERAL);
		XMLElement* triggerElement = root->FirstChildElement(TRIGGER);
		if (triggerElement == NULL)	return FALSE;

		att = triggerElement->Attribute("activeLoadCount");
		if (NULL == generalElement || NULL != att)
		{
			//keep backward compatible
			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			std::stringstream ss(xmlValue);
			ss >> activeLoadCount;
			return SetGeneralSettings(activeLoadCount, activeUnitMS, voltageMin, voltageMax, responseType, parkAnalogLineAtLastVoltage);
		}

		for (int i = 0; i < NUM_GENERAL_ATTRIBUTES; i++)
		{
			att = generalElement->Attribute(GENERAL_ATTR[i]);
			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			std::stringstream ss(xmlValue);
			switch (i)
			{
			case 0:
				ss >> activeLoadCount;
				break;
			case 1:
				ss >> activeUnitMS;
				break;
			case 2:
				ss >> voltageMin;
				break;
			case 3:
				ss >> voltageMax;
				break;
			case 4:
				ss >> responseType;
				break;
			case 5:
				ss >> parkAnalogLineAtLastVoltage;
				break;
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

long ElectroPhysXML::GetTriggerConfig(std::vector<std::string>* triggerConfig)
{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* triggerElement = root->FirstChildElement(TRIGGER);
		if (triggerElement == NULL)	return FALSE;

		triggerConfig->clear();

		for (int i = 0; i < NUM_TRIGGER_ATTRIBUTES; i++)
		{
			const char* att = triggerElement->Attribute(TRIGGER_ATTR[i]);
			//keep backward compatible
			att = (0 == i && NULL == att) ? triggerElement->Attribute("counterOutput") : att;
			att = (1 == i && NULL == att) ? triggerElement->Attribute("outputLine") : att;

			strcpy_s(xmlValue, (NULL == att) ? "" : att);
			triggerConfig->push_back(xmlValue);
		}

		if (NULL != triggerElement->Attribute("activeLoadCount") ||
			NULL != triggerElement->Attribute("counterOutput") ||
			NULL != triggerElement->Attribute("outputLine"))
		{
			SetTriggerConfig(*triggerConfig);
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}


long ElectroPhysXML::OpenConfigFile()
{
	_xmlDoc.reset(new tinyxml2::XMLDocument());
	return (XML_SUCCESS == _xmlDoc.get()->LoadFile(_currentPathAndFile.c_str())) ? TRUE : FALSE;
}

//******************************************		Private Functions		******************************************//

long ElectroPhysXML::SetGeneralSettings(long activeLoadCount, double activeUnitMS, double voltageMin, double voltageMax, long responseType, long parkAnalogLineAtLastVoltage)
{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* xElement = root->FirstChildElement(GENERAL);
		if (xElement == NULL)
		{
			XMLElement* tElement = root->FirstChildElement(TRIGGER);
			xElement = _xmlDoc.get()->NewElement(GENERAL);
			root->InsertAfterChild(tElement, xElement);
			xElement = root->FirstChildElement(GENERAL);
		}

		std::string attr[NUM_GENERAL_ATTRIBUTES] = { std::to_string(activeLoadCount), std::to_string(activeUnitMS), std::to_string(voltageMin), std::to_string(voltageMax), std::to_string(responseType), std::to_string(parkAnalogLineAtLastVoltage) };

		for (int i = 0; i < NUM_GENERAL_ATTRIBUTES; i++)
		{
			xElement->SetAttribute(GENERAL_ATTR[i], attr[i].c_str());
		}
		SaveConfigFile();
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

long ElectroPhysXML::SetTriggerConfig(std::vector<std::string> triggerConfig)
{
	try
	{
		XMLElement* root = _xmlDoc.get()->RootElement();
		if (root == NULL) return FALSE;

		XMLElement* xElement = root->FirstChildElement(TRIGGER);
		if (xElement == NULL)	return FALSE;

		//rebuild node to maintain attribute order
		xElement->DeleteAttribute("activeLoadCount");
		xElement->DeleteAttribute("counterOutput");
		xElement->DeleteAttribute("outputLine");
		for (int i = 0; i < NUM_TRIGGER_ATTRIBUTES; i++)
		{
			xElement->DeleteAttribute(TRIGGER_ATTR[i]);
		}

		for (int i = 0; i < triggerConfig.size(); i++)
		{
			xElement->SetAttribute(TRIGGER_ATTR[i], triggerConfig[i].c_str());
		}
		SaveConfigFile();
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

long ElectroPhysXML::SaveConfigFile()
{
	return (XML_SUCCESS == _xmlDoc.get()->SaveFile(_currentPathAndFile.c_str())) ? TRUE : FALSE;
}

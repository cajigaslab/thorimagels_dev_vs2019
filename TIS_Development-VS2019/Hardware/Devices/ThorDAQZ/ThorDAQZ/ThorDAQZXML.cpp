#include "stdafx.h"
#include "ThorDAQZXML.h"

ThorDAQZXML::ThorDAQZXML()
{
	_xmlObj = NULL;
	_currentPathAndFile = "ThorDAQZSettings.xml";
}

ThorDAQZXML::~ThorDAQZXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char* const ThorDAQZXML::CONVERSION = "Conversion";

const char* const ThorDAQZXML::CONVERSION_ATTR[NUM_CONVERSION_ATTRIBUTES] = { "volts2mm","offsetmm","mm_min", "mm_max" };

long ThorDAQZXML::GetConversion(double& volt2mm, double& mmoffset_0v, double& mm_min, double& mm_max)
{	
	try
	{
		if (_xmlObj != NULL)
		{
			delete _xmlObj;
		}

		_xmlObj = new ticpp::Document(_currentPathAndFile);

		_xmlObj->LoadFile();

		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, CONVERSION);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_CONVERSION_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CONVERSION_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0: ss >> volt2mm; break;
					case 1: ss >> mmoffset_0v; break;
					case 2: ss >> mm_min; break;
					case 3: ss >> mm_max; break;
					}
				}
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const ThorDAQZXML::DMA = "Dma";

const char* const ThorDAQZXML::DMA_ATTR[NUM_DMA_ATTRIBUTES] = { "piezoActiveLoadMS", "preLoadCount" };

long ThorDAQZXML::GetDMA(long& activeLoadMS, long& preLoadCount)
{
	try
	{
		_xmlObj = new ticpp::Document(_currentPathAndFile);

		_xmlObj->LoadFile();

		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, DMA);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_DMA_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(DMA_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0: ss >> activeLoadMS; break;
					case 1: ss >> preLoadCount; break;
					}
				}
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

const char* const ThorDAQZXML::SETTLING_CONFIG = "SettlingConfig";

const char* const ThorDAQZXML::SETTLING_CONFIG_ATTR [NUM_SETTLING_CONFIG_ATTRIBUTES] = { "settlingTimeTypicalMove_sec", "typicalMove_mm" };

long ThorDAQZXML::GetsettlingConfig(double& settlingTimeTypicalMove_sec, double& typicalMove_mm)
{
	try
	{
		_xmlObj = new ticpp::Document(_currentPathAndFile);

		_xmlObj->LoadFile();

		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, SETTLING_CONFIG);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_SETTLING_CONFIG_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(SETTLING_CONFIG_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0: ss >> settlingTimeTypicalMove_sec; break;
					case 1: ss >> typicalMove_mm; break;
					}
				}
			}
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}
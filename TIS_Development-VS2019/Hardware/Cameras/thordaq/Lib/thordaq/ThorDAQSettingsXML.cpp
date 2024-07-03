#include "stdafx.h"
#include "ThorDAQSettingsXML.h"
#include "thordaq.h"

ThorDAQSettingsXML::ThorDAQSettingsXML()
{
	_xmlObj = NULL;
}

ThorDAQSettingsXML::~ThorDAQSettingsXML()
{
	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char* const ThorDAQSettingsXML::DAC_PARKING_POSITIONS = "DACParkingLastPositions";

long ThorDAQSettingsXML::GetDACLastParkingPositions(std::map<UINT, USHORT>& dacParkingPositions)
{
	string s = "ThorDAQSettings.xml";

	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		try
		{
			ticpp::Iterator< ticpp::Element > child(configObj, DAC_PARKING_POSITIONS);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (UINT8 attIdx = 0; attIdx < DAC_ANALOG_CHANNEL_COUNT; attIdx++)
				{
					string str;
					string attribute = "DAC" + to_string(attIdx);
					child->GetAttribute(attribute, &str);
					stringstream ss(str);
					long val;

					if (str.empty() || std::all_of(str.begin(), str.end(), [](char c) {return std::isspace(c); }))
					{
						val = 0;
					}
					else
					{
						ss >> val;
					}

					dacParkingPositions[attIdx] = (USHORT)(val < 0 ? 0 : val);
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			return FALSE;
		}
	}
	return TRUE;
}

long ThorDAQSettingsXML::SetDACLastParkingPositions(std::map<UINT, USHORT> dacParkingPositions)
{
	string s = "ThorDAQSettings.xml";

	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{

		string str;
		stringstream ss;

		for (UINT i = 0; i < DAC_ANALOG_CHANNEL_COUNT; ++i)
		{
			if (dacParkingPositions.find(i) != dacParkingPositions.end())
			{
				ss << dacParkingPositions[i];
				ss << endl;
				getline(ss, str);

				// iterate over to get the particular tag element specified as a parameter(tagName)
				ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(DAC_PARKING_POSITIONS), DAC_PARKING_POSITIONS);
				//get the attribute value for the specified attribute name

				string attribute = "DAC" + to_string(i);

				child->SetAttribute(attribute, str);
			}
		}
	}

	if (_xmlObj != NULL)
	{
		_xmlObj->SaveFile();
	}

	return TRUE;
}

const char* const ThorDAQSettingsXML::LASTHWCONNECTIONTIME = "HWConnectionInfo";
const char* const ThorDAQSettingsXML::LASTHWCONNECTIONTIME_ATTR[NUM_LASTHWCONNECTIONTIME_ATTRIBUTES] = { "lastConnectionTime" };
long ThorDAQSettingsXML::GetLastHWConnectionTime(INT64& time)
{
	string s = "ThorDAQSettings.xml";

	if (_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		return FALSE;
	}
	else
	{
		try
		{
			ticpp::Iterator< ticpp::Element > child(configObj, LASTHWCONNECTIONTIME);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_LASTHWCONNECTIONTIME_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(LASTHWCONNECTIONTIME_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0:
					{
						ss >> time;
					}
					break;
					}
				}

			}
		}
		catch (ticpp::Exception ex)
		{
			return FALSE;
		}
	}

	return TRUE;

}

long ThorDAQSettingsXML::SetLastHWConnectionTime(INT64 time)
{
	try
	{
		string s = "ThorDAQSettings.xml";

		if (_xmlObj != NULL)
		{
			delete _xmlObj;
		}

		_xmlObj = new ticpp::Document(s);
		_xmlObj->LoadFile();

		// make sure the top level root element exist
		ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

		if (configObj == NULL)
		{
			return FALSE;
		}
		else
		{

			string str;
			stringstream ss;

			ss << time;
			ss << endl;

			long index;

			for (index = 0; index < NUM_LASTHWCONNECTIONTIME_ATTRIBUTES; index++)
			{
				getline(ss, str);

				// iterate over to get the particular tag element specified as a parameter(tagName)
				ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(LASTHWCONNECTIONTIME), LASTHWCONNECTIONTIME);
				//get the attribute value for the specified attribute name
				child->SetAttribute(LASTHWCONNECTIONTIME_ATTR[index], str);
			}
		}

		if (_xmlObj != NULL)
		{
			_xmlObj->SaveFile();
		}
	}
	catch (ticpp::Exception ex)
	{
		return FALSE;
	}

	return TRUE;
}
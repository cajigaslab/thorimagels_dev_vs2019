#include "stdafx.h"
#include "Strsafe.h"
#include "stdlib.h"
#include "ThorStim.h"
#include "ThorStimSetupXML.h"


ThorStimXML::ThorStimXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorStimXML::~ThorStimXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

wchar_t ThorStimXML::_libName[_MAX_PATH];

const char * const ThorStimXML::CONFIGURES = "Configures";

const char * const ThorStimXML::CONFIGURES_ATTR[NUM_CONFIGURES_ATTRIBUTES] = {"driverType", "activeLoadMS", "activeLoadCount", "sampleRateKHz", "riseTimeUS"};

long ThorStimXML::GetConfigures(long &driverType, long &activeLoadMS, long &activeLoadCount, long &sampleRateKHz, double &riseTimeUS)
{	
	if (NULL == _xmlObj)
		return FALSE;

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
			ticpp::Iterator< ticpp::Element > child(configObj,CONFIGURES);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_CONFIGURES_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CONFIGURES_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>driverType;
						}
						break;
					case 1:
						{
							ss>>activeLoadMS;
						}
						break;
					case 2:
						{
							ss>>activeLoadCount;
						}
						break;
					case 3:
						{
							ss>>sampleRateKHz;
						}
						break;
					case 4:
						{
							ss>>riseTimeUS;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorStimXML GetConfigures failed");
			LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThorStimXML::MODULATION = "Modulations";

const char * const ThorStimXML::MODULATION_ATTR[NUM_MODULATION_ATTRIBUTES] = {"counter", "triggerIn",
	"line1", "lineMinVoltage1", "lineMaxVoltage1",
	"line2", "lineMinVoltage2", "lineMaxVoltage2",
	"line3", "lineMinVoltage3", "lineMaxVoltage3",
	"line4", "lineMinVoltage4", "lineMaxVoltage4"};

long ThorStimXML::GetModulations(string &counter, string &triggerIn, 
								 string &line1, double &lineMinVoltage1, double &lineMaxVoltage1,
								 string &line2, double &lineMinVoltage2, double &lineMaxVoltage2,
								 string &line3, double &lineMinVoltage3, double &lineMaxVoltage3,
								 string &line4, double &lineMinVoltage4, double &lineMaxVoltage4)
{	
	if (NULL == _xmlObj)
		return FALSE;

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
			ticpp::Iterator< ticpp::Element > child(configObj,MODULATION);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_MODULATION_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(MODULATION_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							counter = str;
						}
						break;
					case 1:
						{
							triggerIn = str;
						}
						break;
					case 2:
						{
							line1 = ('/' == str[0]) ? str.substr(1,str.length()-1) : str;
						}
						break;
					case 3:
						{
							if (!line1.empty())
								ss>>lineMinVoltage1;
							else
								lineMinVoltage1 = -1;
						}
						break;
					case 4:
						{
							if (!line1.empty())
								ss>>lineMaxVoltage1;
							else
								lineMaxVoltage1 = -1;
						}
						break;
					case 5:
						{
							line2 = ('/' == str[0]) ? str.substr(1,str.length()-1) : str;
						}
						break;
					case 6:
						{
							if (!line2.empty())
							{
								//no gap of line is allowed
								if (line1.empty())
									return FALSE;
								ss>>lineMinVoltage2;
							}
							else
								lineMinVoltage2 = -1;
						}
						break;
					case 7:
						{
							if (!line2.empty())
							{
								if (line1.empty())
									return FALSE;
								ss>>lineMaxVoltage2;
							}
							else
								lineMaxVoltage2 = -1;
						}
						break;
					case 8:
						{
							line3 = ('/' == str[0]) ? str.substr(1,str.length()-1) : str;
						}
						break;
					case 9:
						{
							if (!line3.empty())
							{
								if (line1.empty() || line2.empty())
									return FALSE;
								ss>>lineMinVoltage3;
							}
							else
								lineMinVoltage3 = -1;
						}
						break;
					case 10:
						{
							if (!line3.empty())
							{
								if (line1.empty() || line2.empty())
									return FALSE;
								ss>>lineMaxVoltage3;
							}
							else
								lineMaxVoltage3 = -1;
						}
						break;
					case 11:
						{
							line4 = ('/' == str[0]) ? str.substr(1,str.length()-1) : str;
						}
						break;
					case 12:
						{
							if (!line4.empty())
							{
								if (line1.empty() || line2.empty() || line3.empty())
									return FALSE;
								ss>>lineMinVoltage4;
							}
							else
								lineMinVoltage4 = -1;
						}
						break;
					case 13:
						{
							if (!line4.empty())
							{
								if (line1.empty() || line2.empty() || line3.empty())
									return FALSE;
								ss>>lineMaxVoltage4;
							}
							else
								lineMaxVoltage4 = -1;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorStimXML GetModulations failed");
			LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

long ThorStimXML::SetModulations(double lineMinVoltage1, double lineMaxVoltage1,double lineMinVoltage2, double lineMaxVoltage2,
								 double lineMinVoltage3, double lineMaxVoltage3,double lineMinVoltage4, double lineMaxVoltage4)
{
	long ret = FALSE;

	OpenConfigFile();

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return ret;
	}
	else
	{
		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(MODULATION), MODULATION);
		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				string str, strValue;
				attribute->GetName(&str);
				stringstream ss;			

				if(0 == str.compare(MODULATION_ATTR[3]))
				{
					ss.clear();
					ss << lineMinVoltage1;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[4]))
				{
					ss.clear();
					ss << lineMaxVoltage1;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[6]))
				{
					ss.clear();
					ss << lineMinVoltage2;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[7]))
				{
					ss.clear();
					ss << lineMaxVoltage2;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[9]))
				{
					ss.clear();
					ss << lineMinVoltage3;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[10]))
				{
					ss.clear();
					ss << lineMaxVoltage3;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[12]))
				{
					ss.clear();
					ss << lineMinVoltage4;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
				if(0 == str.compare(MODULATION_ATTR[13]))
				{
					ss.clear();
					ss << lineMaxVoltage4;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}
			}
		}
	}
	SaveConfigFile();
	return TRUE;
}
const char * const ThorStimXML::WAVEFORM = "Waveform";

const char * const ThorStimXML::WAVEFORM_ATTR[NUM_WAVEFORM_ATTRIBUTES] = {"pockelDigOutput","completeOutput","cycleOutput","iterationOutput", "patternOutput", "patternComplete","activeOutput","epochOutput","cycleInverse"};

long ThorStimXML::GetWaveform(std::vector<std::string> *digiLines)
{	
	if (NULL == _xmlObj)
		return FALSE;

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
			digiLines->clear();

			ticpp::Iterator< ticpp::Element > child(configObj,WAVEFORM);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_WAVEFORM_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(WAVEFORM_ATTR[attCount],&str);
					digiLines->push_back(('/' == str[0]) ? str.substr(1, str.length()-1) : str);
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorStimXML GetWaveform failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorStimXML::OpenConfigFile()
{
	try
	{
		SAFE_DELETE_PTR(_xmlObj);
		StringCbPrintfW(_libName,_MAX_PATH,GetDLLName(ThorStim::getInstance()->hDLLInstance).c_str());
		wstring settingName = GetDLLName(ThorStim::getInstance()->hDLLInstance) + L"Settings.xml";
		_xmlObj = new ticpp::Document(WStringToString(settingName));
		_xmlObj->LoadFile();	
	}
	catch(...)
	{
		SAFE_DELETE_PTR(_xmlObj);
		return FALSE;
	}
	return TRUE;
}

long ThorStimXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();	
	}			

	return TRUE;
}
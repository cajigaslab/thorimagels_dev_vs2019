#include "stdafx.h"
#include "RealTimeDataXML.h"
#include "Strsafe.h"

///************************************	 XML Loader		************************************///
void XMLFile::GetPathFilename(wchar_t* pathfile)
{ 
	StringCbPrintfW(pathfile,_MAX_PATH,_currentPathAndFile);
}

void XMLFile::SetPathFilename(const wchar_t* pathfile)
{
	StringCbPrintfW(_currentPathAndFile,_MAX_PATH,pathfile);
}

///************************************	 RealTimeData XML	************************************///

RealTimeDataXML::RealTimeDataXML()
{
	_xmlObj = NULL;
	_boardElement = _spectralDomainElement = NULL;
	SetPathFilename(L"ThorRealTimeDataSettings.xml");	/*..\\..\\..\\..\\Hardware\\Devices\\ThorRealTimeData\\*/
}

RealTimeDataXML::RealTimeDataXML(wchar_t* pathfile)
{
	_xmlObj = NULL;
	_boardElement = _spectralDomainElement = NULL;
	SetPathFilename(pathfile);
}

RealTimeDataXML::~RealTimeDataXML()
{
	SAFE_DELETE_ARRAY(_xmlObj);
}

const char * const RealTimeDataXML::USERSETTINGS = "UserSettings";

const char * const RealTimeDataXML::DAQDEVICE = "DaqDevices";

const char * const RealTimeDataXML::BOARDTYPE = "AcquireBoard";

const char * const RealTimeDataXML::BOARDTYPE_ATTR[NUM_BOARDTYPE_ATTRIBUTES] = {"type","devID","active","totalAI","totalDI","duration","StimulusLimit","hwTrigMode","hwTrigChannel","hwTrigType"};

long RealTimeDataXML::GetBoard(BoardInfo &board)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(true);
		ticpp::Node* nodeObj = configObj->LastChild(DAQDEVICE,false);
		configObj = nodeObj->ToElement();

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,BOARDTYPE);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<5; attCount++)	//NUM_BOARDTYPE_ATTRIBUTES
				{
					std::string str;		
					child->GetAttribute(BOARDTYPE_ATTR[attCount],&str);	
					std::stringstream ss(str);

					switch(attCount)
					{
					case 0: board.name = str; break;
					case 1: board.devID = str; break;
					case 2: ss>>board.active; break;
					case 3: ss>>board.totalAI; break;
					case 4: ss>>board.totalDI; break;
					}
				}
				if(board.active == 1)
				{
					return TRUE;
				}
				else
				{
					continue;
				}				
			}
			return FALSE;
		}
	}
	catch(...)
	{
		return FALSE;
	}
}

ticpp::Element* RealTimeDataXML::GetBoard()
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(true);
		ticpp::Node* nodeObj = configObj->LastChild(DAQDEVICE,false);
		configObj = nodeObj->ToElement();

		if ( configObj == NULL )
		{
			return NULL;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,BOARDTYPE);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount<5; attCount++)	//NUM_BOARDTYPE_ATTRIBUTES
				{
					std::string str;		
					child->GetAttribute(BOARDTYPE_ATTR[attCount],&str);	
					std::stringstream ss(str);

					switch(attCount)
					{
					case 2: 
						if(0 == str.compare("1"))
						{
							configObj = child.Get();
							return configObj;
						}
						else
						{
							continue;
						}	
						break;
					}
				}
			}
			return NULL;
		}
	}
	catch(...)
	{
		return NULL;
	}
}

const char * const RealTimeDataXML::CHANNELTYPE = "DataChannel";

const char * const RealTimeDataXML::VIRTUALCHANNELTYPE = "VirtualChannel";

const char * const RealTimeDataXML::SPECCHANNELTYPE = "SpectralChannel";

const char * const RealTimeDataXML::CHANNELTYPE_ATTR[NUM_CHANNELTYPE_ATTRIBUTES] = {"enable","alias","signalType","group","physicalChannel","sample","Stimulus"};  //"aiTrigger"

const char * const RealTimeDataXML::SAMPLERATE = "SampleRate";

const char * const RealTimeDataXML::SAMPLERATE_ATTR[NUM_SAMPLERATE_ATTRIBUTES] = {"name","enable","rate"};

const char * const RealTimeDataXML::DISPLAY = "Display";

const char * const RealTimeDataXML::DISPLAY_ATTR[NUM_DISPLAY_ATTRIBUTES] = {"resolution","enable","value"};

long RealTimeDataXML::GetMode(Mode* mode)
{
	try
	{
		_boardElement = GetBoard();

		if(NULL == _boardElement)
			return FALSE;

		for(long attCount=0; attCount<NUM_BOARDTYPE_ATTRIBUTES; attCount++)
		{
			std::string str;		
			_boardElement->GetAttribute(BOARDTYPE_ATTR[attCount],&str);	
			std::stringstream ss(str);

			switch(attCount)
			{
			case 5: ss>>mode->duration; break;
			case 6: ss>>mode->StimulusLimit; break;
			case 7: ss>>mode->hwTrigMode; break;
			case 8: mode->hwTrigChannel = str; break;
			case 9: ss>>mode->hwTrigType; break;
			}
		}

		ticpp::Element *child1 = _boardElement->FirstChildElement(SAMPLERATE, true);
		if(NULL != child1)
		{	
			ticpp::Iterator< ticpp::Element > child2(child1,SAMPLERATE);

			for ( child2 = child2.Get(); child2 != child2.end(); child2++)
			{
				bool enabled = false;
				for(long count1=0; count1<NUM_SAMPLERATE_ATTRIBUTES; count1++)
				{
					std::string str;		
					child2->GetAttribute(SAMPLERATE_ATTR[count1],&str);
					std::stringstream ss(str);
					switch(count1)
					{								
					case 0: break;
					case 1: 
						if(str == "0")
						{
							enabled = false;
							continue;
						}
						else
						{
							enabled = true;
						}
						break;
					case 2: 
						if(enabled)
						{
							ss>>mode->sampleRate; 
						}
						break;
					}
				}
			}
		}

		child1 = _boardElement->FirstChildElement(DISPLAY, true);			
		if (NULL != child1)
		{
			ticpp::Iterator< ticpp::Element > child2(child1,DISPLAY);

			for ( child2 = child2.Get(); child2 != child2.end(); child2++)
			{
				bool enabled = false;
				for(long count2=0; count2<NUM_DISPLAY_ATTRIBUTES; count2++)
				{
					std::string str;		
					child2->GetAttribute(DISPLAY_ATTR[count2],&str);
					std::stringstream ss(str);
					switch(count2)
					{								
					case 0: break;
					case 1: 
						if(str == "0")
						{
							enabled = false;
							continue;
						}
						else
						{
							enabled = true;
						}
						break;
					case 2: 
						if(enabled)
						{
							ss>>mode->interleave; 
						}
						break;
					}
				}
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

long RealTimeDataXML::GetDataChannel(std::vector<Channels>& channel)
{
	try
	{
		if(NULL == _boardElement)
			return FALSE;

		ticpp::Element *child1 = _boardElement->FirstChildElement(CHANNELTYPE, false);
		if(child1 != NULL)
		{						
			ticpp::Iterator< ticpp::Element > child2(child1,CHANNELTYPE);

			channel.clear();

			for ( child2 = child2.Get(); child2 != child2.end(); child2++)
			{	
				bool skipChannel = false;
				Channels localch={"",0,"","",0,0};						

				for(long count2=0; count2<NUM_CHANNELTYPE_ATTRIBUTES; count2++)
				{
					std::string str;		
					child2->GetAttribute(CHANNELTYPE_ATTR[count2],&str);
					std::stringstream ss(str);

					switch(count2)
					{
					case 0: 
						if(str == "0")
						{
							skipChannel = true;
						}
						break;
					case 1: localch.alias = str; break;
					case 2: ss>>localch.signalType; break;
					case 3: localch.type = str; break;
					case 4: localch.lineId = str; break;
					case 5: ss>>(localch.sample); break;
						//case 6: ss>>(localch.aiTrigger); break;
					case 6: ss>>(localch.Stimulus); break;
					}
					if(skipChannel)
					{
						break;
					}
				}
				if(!skipChannel)
				{
					channel.push_back(localch);
				}
			}					
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

long RealTimeDataXML::PopulateVirtualChannels(ticpp::Iterator< ticpp::Element > itElement, std::vector<Channels>& channel)
{
	if(NULL == itElement.Get())
		return FALSE;

	channel.clear();

	for ( itElement = itElement.Get(); itElement != itElement.end(); itElement++)
	{	
		bool skipChannel = false;
		Channels localch={"",0,"","",0,0};						

		for(long count2=0; count2<NUM_VIRTUALCHANNELTYPE_ATTRIBUTES; count2++)
		{
			std::string str;		
			itElement->GetAttribute(CHANNELTYPE_ATTR[count2],&str);
			std::stringstream ss(str);

			switch(count2)
			{
			case 0: 
				if(str == "0")
				{
					skipChannel = true;
				}
				break;
			case 1: localch.alias = str; break;
			case 2: ss>>localch.signalType; break;
			case 3: localch.type = str; break;
			case 4: localch.lineId = str; break;
			}
			if(skipChannel)
			{
				break;
			}
		}
		if(!skipChannel)
		{
			channel.push_back(localch);
		}
	}
	return TRUE;
}

long RealTimeDataXML::GetVirtualChannel(std::vector<Channels>& channel)
{
	try
	{
		if(NULL == _boardElement)
			return FALSE;
		ticpp::Element * child1 = _boardElement->FirstChildElement(VIRTUALCHANNELTYPE,true);
		if(NULL == child1)
			return FALSE;

		ticpp::Iterator< ticpp::Element > child2(child1,VIRTUALCHANNELTYPE);
		return PopulateVirtualChannels(child2, channel);
	}
	catch(...)
	{
		return FALSE;
	}
}

const char * const RealTimeDataXML::BLEACHPARAM = "Bleach";

const char * const RealTimeDataXML::BLEACHPARAM_ATTR[NUM_BLEACHPARAM_ATTRIBUTES] = {"hwTrigLine","shutterLine","bleachLine","outputLine","pmtCloseTime","bleachTime","bleachIdleTime","bleachIteration","outDelayTime","bleachTrigMode", "cycle", "interval"};

long RealTimeDataXML::GetAsyncMode(AsyncParams* aparam)
{
	try
	{
		if ( NULL == _boardElement )
			return FALSE;

		ticpp::Element *child1 = _boardElement->FirstChildElement(false);
		if(NULL != child1)
		{				
			//set bleach:												
			child1 = child1->NextSiblingElement(BLEACHPARAM,true);
			ticpp::Iterator< ticpp::Element > child2(child1,BLEACHPARAM);
			for(long count2=0; count2<NUM_BLEACHPARAM_ATTRIBUTES; count2++)
			{
				std::string str;		
				child2->GetAttribute(BLEACHPARAM_ATTR[count2],&str);
				std::stringstream ss(str);

				switch(count2)
				{
				case 0: aparam->bleach.hwTrigLine = str; break;
				case 1: aparam->bleach.shutterLine = str; break;
				case 2: aparam->bleach.bleachLine = str; break;
				case 3: aparam->bleach.outputLine = str; break;
				case 4: ss>>aparam->bleach.pmtCloseTime; break;
				case 5: ss>>aparam->bleach.bleachTime; break;	
				case 6: ss>>aparam->bleach.bleachIdleTime; break;
				case 7: ss>>aparam->bleach.bleachIteration; break;
				case 8: ss>>aparam->bleach.outDelayTime; break;
				case 9: ss>>aparam->bleach.hwTrigMode; break;
				case 10: ss>>aparam->bleach.cycle; break;
				case 11: ss>>aparam->bleach.interval; break;
				}
			}							
		}
		return TRUE;

	}
	catch(...)
	{
		return FALSE;
	}
}

const char * const RealTimeDataXML::INVERTPARAM = "Invert";

const char * const RealTimeDataXML::INVERTPARAM_ATTR[NUM_INVERTPARAM_ATTRIBUTES] = {"inputLine","outputLine"};

long RealTimeDataXML::GetInvert(Invert* iparam)
{
	try
	{
		if ( NULL == _boardElement )
			return FALSE;

		ticpp::Element *child1 = _boardElement->FirstChildElement(false);
		if(NULL != child1)
		{
			child1 = child1->NextSiblingElement(INVERTPARAM,true);
			ticpp::Iterator< ticpp::Element > child2(child1,INVERTPARAM);
			for(long count2=0; count2<NUM_INVERTPARAM_ATTRIBUTES; count2++)
			{
				std::string str;		
				child2->GetAttribute(INVERTPARAM_ATTR[count2],&str);
				std::stringstream ss(str);

				switch(count2)
				{
				case 0: iparam->inputLine = str; break;
				case 1: iparam->outputLine = str; break;
				}
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

const char * const RealTimeDataXML::SPECTRALDOMAIN = "SpectralDomain";

const char * const RealTimeDataXML::SPECTRALDOMAIN_ATTR[NUM_SPECTRALDOMAIN_ATTRIBUTES] = {"liveSampleSec","freqMin","freqMax","sampleMinSec","sampleMaxSec","freqAvgNum","freqAvgMode","blockNum"}; 

long RealTimeDataXML::GetSpectralDomain(Spectrum &sDomain)
{
	try
	{
		//initialize given param in case failed
		sDomain.liveSampleSec = sDomain.freqMin = sDomain.freqMax = sDomain.sampleMinSec = 0;
		sDomain.sampleMaxSec = sDomain.freqAvgNum = sDomain.freqAvgMode = sDomain.blockNum = 0;

		if(NULL == _boardElement)
		{
			_boardElement = GetBoard();
		}

		_spectralDomainElement = _boardElement->FirstChildElement(SPECTRALDOMAIN,true);

		ticpp::Iterator< ticpp::Element > child2(_spectralDomainElement,SPECTRALDOMAIN);
		for ( child2 = child2.Get(); child2 != child2.end(); child2++)
		{	
			for(long count2=0; count2<NUM_SPECTRALDOMAIN_ATTRIBUTES; count2++)
			{
				std::string str;		
				child2->GetAttribute(SPECTRALDOMAIN_ATTR[count2],&str);
				std::stringstream ss(str);

				switch(count2)
				{
				case 0: ss>>sDomain.liveSampleSec; break;
				case 1: ss>>sDomain.freqMin; break;
				case 2: ss>>sDomain.freqMax; break;
				case 3: ss>>sDomain.sampleMinSec; break;
				case 4: ss>>sDomain.sampleMaxSec; break;
				case 5: ss>>sDomain.freqAvgNum; break;
				case 6: ss>>sDomain.freqAvgMode; break;
				case 7: ss>>sDomain.blockNum; break;
				}
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

long RealTimeDataXML::GetSpecChannel(std::vector<Channels>& channel)
{
	try
	{
		if(NULL == _spectralDomainElement)
			return FALSE;

		ticpp::Element * child1 = _spectralDomainElement->FirstChildElement(SPECCHANNELTYPE,true);

		ticpp::Iterator< ticpp::Element > child2(child1,SPECCHANNELTYPE);
		return PopulateVirtualChannels(child2, channel);
	}
	catch(...)
	{
		return FALSE;
	}
}

long RealTimeDataXML::GetSpecVirtualChannel(std::vector<Channels>& channel)
{
	try
	{
		if(NULL == _spectralDomainElement)
			return FALSE;

		ticpp::Element *child1 = _spectralDomainElement->FirstChildElement(VIRTUALCHANNELTYPE,true);

		ticpp::Iterator< ticpp::Element > child2(child1,VIRTUALCHANNELTYPE);
		return PopulateVirtualChannels(child2, channel);
	}
	catch(...)
	{
		return FALSE;
	}
}

const char * const RealTimeDataXML::SAVEPARAM = "Save";

const char * const RealTimeDataXML::SAVEPARAM_ATTR[NUM_SAVEPARAM_ATTRIBUTES] = {"CallbackSec"};

long RealTimeDataXML::GetSave(double &callbackSec)
{
	try
	{
		long active = 0;
		ticpp::Element *configObj = _xmlObj->FirstChildElement(true);
		ticpp::Node* nodeObj = configObj->LastChild(USERSETTINGS,false);
		configObj = nodeObj->ToElement();

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj,SAVEPARAM);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{				
				std::string str;		
				child->GetAttribute(SAVEPARAM_ATTR[0],&str,true);	
				std::stringstream ss(str);
				ss>>callbackSec;
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

const char * const RealTimeDataXML::VARIABLES = "Variables";

const char * const RealTimeDataXML::VARIABLESETTINGS = "Var";

const char * const RealTimeDataXML::VARIABLE_ATTR[NUM_VARIABLE_ATTRIBUTES] = {"ID","Value","Name"};

long RealTimeDataXML::GetVariables(std::map<int, GlobalVar>& vars)
{
	try
	{
		vars.clear();
		ticpp::Element *configObj = _xmlObj->FirstChildElement(true);
		ticpp::Node* nodeObj = configObj->LastChild(VARIABLES,true);
		configObj = nodeObj->ToElement();

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, VARIABLESETTINGS);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				GlobalVar var;
				for(long attCount = 0; attCount < NUM_VARIABLE_ATTRIBUTES; attCount++)
				{
					std::string str;		
					child->GetAttribute(VARIABLE_ATTR[attCount],&str);	
					std::stringstream ss(str);

					switch(attCount)
					{
					case 0: ss>>var.id; break;
					case 1: ss>>var.value; break;
					case 2: var.name = str; break;
					}
				}
				vars.insert(std::make_pair(var.id, var));
			}
			return TRUE;
		}
	}
	catch(...)
	{
		return FALSE;
	}
}

void RealTimeDataXML::SetVariables(std::map<int, GlobalVar>& vars)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(true);
		ticpp::Node* nodeObj = configObj->LastChild(VARIABLES,false);
		configObj = nodeObj->ToElement();

		if ( configObj == NULL )
		{
			return;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, VARIABLESETTINGS);
			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				std::string str;		
				child->GetAttribute(VARIABLE_ATTR[0],&str);	

				child->SetAttribute(VARIABLE_ATTR[0], vars[std::stoi(str)].id);
				child->SetAttribute(VARIABLE_ATTR[1], vars[std::stoi(str)].value);
				child->SetAttribute(VARIABLE_ATTR[2], vars[std::stoi(str)].name);
			}
		}
		SaveConfigFile();
	}
	catch(...)
	{
	}
}

const char * const RealTimeDataXML::FILEPATH = "FilePath";

const char * const RealTimeDataXML::FILEPATH_ATTR[NUM_FILEPATH_ATTRIBUTES] = {"folder","episode"};

long RealTimeDataXML::GetFilePath(FilePathParam* filepath)
{
	try
	{
		filepath->settingPath = filepath->episodePath = L"";

		_boardElement = GetBoard();

		ticpp::Element *child1 = _boardElement->FirstChildElement(FILEPATH, true);
		if(NULL != child1)
		{
			ticpp::Iterator< ticpp::Element > child2(child1,FILEPATH);
			std::string folder;
			for(long count = 0; count < NUM_FILEPATH_ATTRIBUTES; count++)
			{
				std::string str;		
				child2->GetAttribute(FILEPATH_ATTR[count],&str);

				switch(count)
				{
				case 0: 
					folder = str;
					filepath->settingPath = StringToWString(folder + "\\" + SETTINGS_FILE_NAME); 
					break;
				case 1: 
					if (std::string::npos != str.find(".h5"))
						str = str.substr(0, str.find(".h5"));
					filepath->episodePath = StringToWString(folder + "\\" + str + ".h5"); 
					break;
				}
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

long RealTimeDataXML::OpenConfigFile(bool reload)
{	
	try
	{
		if((reload) && (_xmlObj != NULL))
		{
			SAFE_DELETE_ARRAY(_xmlObj);
		}
		if(NULL == _xmlObj)
		{
			//wsprintf(_currentPathAndFile,L"ThorRealTimeDataSettings.xml");		
			std::wstring ws = _currentPathAndFile;
			std::string s(ws.begin(), ws.end());
			s.assign(ws.begin(), ws.end());

			_xmlObj = new ticpp::Document(s);
			_xmlObj->LoadFile();
			_boardElement = _spectralDomainElement = NULL;
		}
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

long RealTimeDataXML::SaveConfigFile()
{	
	if(_xmlObj != NULL)
	{
		std::wstring ws = _currentPathAndFile;
		std::string s(ws.begin(), ws.end());
		s.assign(ws.begin(), ws.end());
		_xmlObj->SaveFile(s);
		return TRUE;
	}
	return FALSE;

}

///************************************	 OTM DATA XML	************************************///

OTMDataXML::OTMDataXML()
{
	_xmlObj = NULL;
	SetPathFilename(L"OTMSettings.xml");
}

OTMDataXML::OTMDataXML(const wchar_t* pathfile)
{
	if(NULL == pathfile)
	{
		SetPathFilename(L"OTMSettings.xml");
	}
	else
	{
		SetPathFilename(pathfile);
	}
	_xmlObj = NULL;
}

OTMDataXML::~OTMDataXML()
{
	SAFE_DELETE_ARRAY(_xmlObj);
}

const char * const OTMDataXML::PARAMETERSETTINGS = "Parameters";

const char * const OTMDataXML::PARAMETER_ATTR[NUM_PARAMETER_ATTRIBUTES] = {"IsCurveFit","FitFreqMin","FitFreqMax","Temperature","Radius","Viscosity","GammaTheory","DiffTheory","FreqBlock","Beta2FreqMin","Beta2FreqMax"};

long OTMDataXML::GetParameter(OTMParam* otmParam)
{
	try
	{
		//initialize given param in case failed
		otmParam->isCurveFit = FALSE;
		otmParam->fitFreqMin = 0;
		otmParam->fitFreqMax = 1;
		otmParam->temperature = 298.15-CELSIUS_TO_KELVIN;
		otmParam->radius = 1;
		otmParam->viscosity = 0.00089;
		otmParam->gammaTheory = 0.0167761047701695;
		otmParam->diffTheory = 0.2454;
		otmParam->freqBlock = 1;
		otmParam->beta2FreqMin = 0;
		otmParam->beta2FreqMax = 1;

		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, PARAMETERSETTINGS);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount < NUM_PARAMETER_ATTRIBUTES; attCount++)
				{
					std::string str;		
					child->GetAttribute(PARAMETER_ATTR[attCount],&str);	
					std::stringstream ss(str);

					switch(attCount)
					{
					case 0: otmParam->isCurveFit = (0 == str.compare("False")) ? FALSE : TRUE; break;
					case 1: ss>>otmParam->fitFreqMin; break;
					case 2: ss>>otmParam->fitFreqMax; break;
					case 3: ss>>otmParam->temperature; break;
					case 4: ss>>otmParam->radius; break;
					case 5: ss>>otmParam->viscosity; break;
					case 6: ss>>otmParam->gammaTheory; break;
					case 7: ss>>otmParam->diffTheory; break;
					case 8: ss>>otmParam->freqBlock; break;
					case 9: ss>>otmParam->beta2FreqMin; break;
					case 10: ss>>otmParam->beta2FreqMax; break;
					}
				}
			}
			return TRUE;
		}
	}
	catch(...)
	{
		return FALSE;
	}
}

void OTMDataXML::SetParameter(OTMParam* otmParam)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return;
	}
	else
	{
		std::string str;
		std::stringstream ss;

		ss << otmParam->fitFreqMin;
		ss << std::endl;
		ss << otmParam->fitFreqMax;
		ss << std::endl;
		ss << otmParam->temperature;
		ss << std::endl;
		ss << otmParam->radius;
		ss << std::endl;
		ss << otmParam->viscosity;
		ss << std::endl;
		ss << otmParam->gammaTheory;
		ss << std::endl;
		ss << otmParam->diffTheory;
		ss << std::endl;
		ss << otmParam->freqBlock;
		ss << std::endl;
		ss << otmParam->beta2FreqMin;
		ss << std::endl;
		ss << otmParam->beta2FreqMax;
		ss << std::endl;

		long index;

		for(index = 0; index < NUM_PARAMETER_ATTRIBUTES; index++)
		{
			if(0 == index)
				str = (0 == otmParam->isCurveFit) ? "False" : "True";
			else
				getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(PARAMETERSETTINGS), PARAMETERSETTINGS);
			//get the attribute value for the specified attribute name
			child->SetAttribute(PARAMETER_ATTR[index], str);
		}
	}
	SaveConfigFile(TRUE);
}

const char * const OTMDataXML::FITTINGSETTINGS = "Fittings";

const char * const OTMDataXML::FITTING_ATTR[NUM_FITTING_ATTRIBUTES] = {"DiffX1","DiffY1","DiffX2","DiffY2","CornerX","CornerY","ChiX","ChiY","GammaX","GammaY","KappaX","KappaY","BetaX1","BetaY1","BetaX2","BetaY2"};

long OTMDataXML::GetFittings(OTMFit* otmFit)
{
	try
	{
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		if ( configObj == NULL )
		{
			//initialize otmFit params before return:
			otmFit->diffXY1[0] = otmFit->diffXY1[1] = 0.0;
			otmFit->diffXY2[0] = otmFit->diffXY2[1] = 0.0;
			otmFit->cornerXY[0] = otmFit->cornerXY[1] = 0.0;
			otmFit->chiXY[0] = otmFit->chiXY[1] = 0.0;
			otmFit->gammaXY[0] = otmFit->gammaXY[1] = 0.0;
			otmFit->kappaXY[0] = otmFit->kappaXY[1] = 0.0;
			otmFit->betaXY1[0] = otmFit->betaXY1[1] = 0.0;
			otmFit->betaXY2[0] = otmFit->betaXY2[1] = 0.0;
			return FALSE;
		}
		else
		{
			ticpp::Iterator< ticpp::Element > child(configObj, FITTINGSETTINGS);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount=0; attCount< NUM_FITTING_ATTRIBUTES; attCount++)
				{
					std::string str;		
					child->GetAttribute(FITTING_ATTR[attCount],&str);	
					std::stringstream ss(str);

					switch(attCount)
					{
					case 0: ss>>otmFit->diffXY1[0]; break;
					case 1: ss>>otmFit->diffXY1[1]; break;
					case 2: ss>>otmFit->diffXY2[0]; break;
					case 3: ss>>otmFit->diffXY2[1]; break;
					case 4: ss>>otmFit->cornerXY[0]; break;
					case 5: ss>>otmFit->cornerXY[1]; break;
					case 6: ss>>otmFit->chiXY[0]; break;
					case 7: ss>>otmFit->chiXY[1]; break;
					case 8: ss>>otmFit->gammaXY[0]; break;
					case 9: ss>>otmFit->gammaXY[1]; break;
					case 10: ss>>otmFit->kappaXY[0]; break;
					case 11: ss>>otmFit->kappaXY[1]; break;
					case 12: ss>>otmFit->betaXY1[0]; break;
					case 13: ss>>otmFit->betaXY1[1]; break;
					case 14: ss>>otmFit->betaXY2[0]; break;
					case 15: ss>>otmFit->betaXY2[1]; break;
					}
				}
			}
			return TRUE;
		}
	}
	catch(...)
	{
		return FALSE;
	}
}

void OTMDataXML::SetFittings(OTMFit* otmFit)
{
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return;
	}
	else
	{
		std::string str;
		std::stringstream ss;

		ss << otmFit->diffXY1[0];
		ss << std::endl;
		ss << otmFit->diffXY1[1];
		ss << std::endl;
		ss << otmFit->diffXY2[0];
		ss << std::endl;
		ss << otmFit->diffXY2[1];
		ss << std::endl;
		ss << otmFit->cornerXY[0];
		ss << std::endl;
		ss << otmFit->cornerXY[1];
		ss << std::endl;
		ss << otmFit->chiXY[0];
		ss << std::endl;
		ss << otmFit->chiXY[1];
		ss << std::endl;
		ss << otmFit->gammaXY[0];
		ss << std::endl;
		ss << otmFit->gammaXY[1];
		ss << std::endl;
		ss << otmFit->kappaXY[0];
		ss << std::endl;
		ss << otmFit->kappaXY[1];
		ss << std::endl;
		ss << otmFit->betaXY1[0];
		ss << std::endl;
		ss << otmFit->betaXY1[1];
		ss << std::endl;
		ss << otmFit->betaXY2[0];
		ss << std::endl;
		ss << otmFit->betaXY2[1];
		ss << std::endl;

		long index;

		for(index=0; index<NUM_FITTING_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(FITTINGSETTINGS), FITTINGSETTINGS);
			//get the attribute value for the specified attribute name
			child->SetAttribute(FITTING_ATTR[index], str);
		}
	}
	SaveConfigFile(TRUE);
}

long OTMDataXML::OpenConfigFile(bool reload)
{	
	try
	{
		if((reload) && (_xmlObj != NULL))
		{
			SAFE_DELETE_ARRAY(_xmlObj);
		}
		if(NULL == _xmlObj)
		{
			std::wstring ws = _currentPathAndFile;
			std::string s(ws.begin(), ws.end());
			s.assign(ws.begin(), ws.end());

			_xmlObj = new ticpp::Document(s);
			_xmlObj->LoadFile();
		}
	}
	catch(...)
	{
		return FALSE;
	}
	return TRUE;
}

void OTMDataXML::SaveConfigFile(long saveGlobal)
{	
	if(_xmlObj != NULL)
	{
		wchar_t baseFile[MAX_PATH];
		StringCbPrintfW(baseFile,MAX_PATH,L"OTMSettings.xml");		

		std::wstring wStrVec[2];
		wStrVec[0] = std::wstring(_currentPathAndFile);
		wStrVec[1] = std::wstring(baseFile);

		for (int i = 0; i < 2; i++)
		{
			std::string s(wStrVec[i].begin(), wStrVec[i].end());
			_xmlObj->SaveFile(s);
			if((FALSE == saveGlobal) && (0 == i))
				return;
		}
	}
}

#include "stdafx.h"
#include "Strsafe.h"
#include "stdlib.h"
#include "ThorGGNI.h"
#include "ThorGGNISetupXML.h"


ThorGGNIXML::ThorGGNIXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorGGNIXML::~ThorGGNIXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorGGNIXML::CONFIG = "Configuration";

const char * const ThorGGNIXML::CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES] = {"field2Theta","pockelsParkAtMinimum","galvoParkAtStart","fieldSizeMin","fieldSizeMax","pockelsTurnAroundBlank","analogCh1","analogCh2","analogCh1FeedbackRatio","analogCh2FeedbackRatio","analogXYmode1","minGalvoFreqHz1","analogCh3","analogCh4"};

long ThorGGNIXML::GetConfiguration(double &field2Theta, long &pockelsParkAtMinimum, long &galvoParkAtStart, long &fieldSizeMin, long &fieldSizeMax, long &pockelsTurnAroundBlank, string &analogCh1, string &analogCh2, double &analogCh1FeedbackRatio, double &analogCh2FeedbackRatio, long &analogXYmode1, double&minGalvoFreqHz1, string &analogCh3, string &analogCh4)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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
			ticpp::Iterator< ticpp::Element > child(configObj,CONFIG);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_CONFIG_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CONFIG_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>field2Theta;
						}
						break;
					case 1:
						{
							ss>>pockelsParkAtMinimum;
						}
						break;
					case 2:
						{
							ss>>galvoParkAtStart;
						}
						break;
					case 3:
						{
							ss>>fieldSizeMin;
						}
						break;
					case 4:
						{
							ss>>fieldSizeMax;
						}
						break;
					case 5:
						{
							ss>>pockelsTurnAroundBlank;
						}
						break;
					case 6:
						{
							analogCh1 = str;
						}
						break;
					case 7:
						{
							analogCh2 = str;
						}
						break;
					case 8:
						{
							ss>>analogCh1FeedbackRatio;
						}
						break;
					case 9:
						{
							ss>>analogCh2FeedbackRatio;
						}
						break;
					case 10:
						{
							ss>>analogXYmode1;
						}
						break;
					case 11:
						{
							ss>>minGalvoFreqHz1;
						}
						break;
					case 12:
						{
							analogCh3 = str;
						}
						break;
					case 13:
						{
							analogCh4 = str;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetConfiguration failed");
			LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThorGGNIXML::DMA = "Dma";

const char * const ThorGGNIXML::DMA_ATTR[NUM_DMA_ATTRIBUTES] = {"bufferCount", "activeLoadCount", "imageActiveLoadMS", "imageActiveLoadCount"};

long ThorGGNIXML::GetDMA(long &bufferCount, long &activeLoadCount, long &imageActiveLoadMS, long &imageActiveLoadCount)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile,MAX_PATH, settingName.c_str());		

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
			ticpp::Iterator< ticpp::Element > child(configObj,DMA);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_DMA_ATTRIBUTES; attCount++)
				{


					string str;
					child->GetAttribute(DMA_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>bufferCount;
						}
						break;
					case 1:
						{
							ss>>activeLoadCount;
						}
						break;
					case 2:
						{
							ss>>imageActiveLoadMS;
						}
						break;
					case 3:
						{
							ss>>imageActiveLoadCount;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetDMA failed");
			LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThorGGNIXML::IO = "IO";

const char * const ThorGGNIXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"pockelsCounterInternal","pockelsCounterOutput","pockelsTriggerIn","pockelsVoltageSlopeThreshold",
	"pockelsLine","pockelsPowerInputLine","pockelsScanVoltageStart","pockelsScanVoltageStop","pockelsMinVoltage","pockelsMaxVoltage",
	"pockelsLine1","pockelsPowerInputLine1","pockelsScanVoltageStart1","pockelsScanVoltageStop1",
	"pockelsLine2","pockelsPowerInputLine2","pockelsScanVoltageStart2","pockelsScanVoltageStop2",
	"pockelsLine3","pockelsPowerInputLine3","pockelsScanVoltageStart3","pockelsScanVoltageStop3", "pockelsReferenceLine",
	"pockelsResponseType", "pockelsResponseType1","pockelsResponseType2","pockelsResponseType3"};

long ThorGGNIXML::GetIO(string &pockelsCounterInternal, string &pockelsCounterOutput, string &pockelsTriggerIn, double &pockelsVoltageSlopeThreshold, 
						string &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, double &pockelsMinVoltage, double &pockelsMaxVoltage,
						string &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1,
						string &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2,
						string &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, string &pockelsReferenceLine,
						long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2, long &pockelsResponseType3)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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
			ticpp::Iterator< ticpp::Element > child(configObj,IO);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_IO_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(IO_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							pockelsCounterInternal = str;
						}
						break;
					case 1:
						{
							pockelsCounterOutput = str;
						}
						break;
					case 2:
						{
							pockelsTriggerIn = str;
						}
						break;
					case 3:
						{
							ss>>pockelsVoltageSlopeThreshold;
						}
						break;
					case 4:
						{
							pockelsLine0 = str;
						}
						break;
					case 5:
						{
							pockelsPowerInputLine0 = str;
						}
						break;
					case 6:
						{
							ss>>pockelsScanVoltageStart0;
						}
						break;
					case 7:
						{
							ss>>pockelsScanVoltageStop0;
						}
						break;
					case 8:
						{
							ss>>pockelsMinVoltage;
						}
						break;
					case 9:
						{
							ss>>pockelsMaxVoltage;
						}
						break;
					case 10:
						{
							pockelsLine1 = str;
						}
						break;
					case 11:
						{
							pockelsPowerInputLine1 = str;
						}
						break;
					case 12:
						{
							ss>>pockelsScanVoltageStart1;
						}
						break;
					case 13:
						{
							ss>>pockelsScanVoltageStop1;
						}
						break;
					case 14:
						{
							pockelsLine2 = str;
						}
						break;
					case 15:
						{
							pockelsPowerInputLine2 = str;
						}
						break;
					case 16:
						{
							ss>>pockelsScanVoltageStart2;
						}
						break;
					case 17:
						{
							ss>>pockelsScanVoltageStop2;
						}
						break;
					case 18:
						{
							pockelsLine3 = str;
						}
						break;
					case 19:
						{
							pockelsPowerInputLine3 = str;
						}
						break;
					case 20:
						{
							ss>>pockelsScanVoltageStart3;
						}
						break;
					case 21:
						{
							ss>>pockelsScanVoltageStop3;
						}
						break;
					case 22:
						{
							pockelsReferenceLine = str;
						}
						break;
					case 23:
						{
							ss>>pockelsResponseType0;
						}
						break;
					case 24:
						{
							ss>>pockelsResponseType1;
						}
						break;
					case 25:
						{
							ss>>pockelsResponseType2;
						}
						break;
					case 26:
						{
							ss>>pockelsResponseType3;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetIO failed");
			LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

long ThorGGNIXML::SetIO(double pockelsMinVoltage,double pockelsMaxVoltage)
{
	long ret = FALSE;

	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	OpenConfigFile(s);
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return ret;
	}
	else
	{
		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(IO), IO);
		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			ticpp::Iterator< ticpp::Attribute > attribute;
			for(attribute = attribute.begin(child.Get()); attribute != attribute.end(); attribute++)
			{
				string str, strValue;
				attribute->GetName(&str);
				stringstream ss;			

				if(0 == str.compare(IO_ATTR[8]))
				{
					ss.clear();
					ss << pockelsMinVoltage;
					ss << endl;
					getline(ss,strValue);
					attribute->SetValue(strValue);
				}

				if(0 == str.compare(IO_ATTR[9]))
				{
					ss.clear();
					ss << pockelsMaxVoltage;
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

const char * const ThorGGNIXML::CALIBRATION = "Calibration";

const char * const ThorGGNIXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"fieldSizeCalibration","flipVerticalScan","fineOffsetX","fineOffsetY","fineScaleX","fineScaleY","oneXFieldSize","maxGalvoOpticalAngle","minSignalInputVoltage","maxSignalInputVoltage","galvoRetraceTimeUS","pockelsPhaseAdjustUS"};

long ThorGGNIXML::GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &fineScaleX, double &fineScaleY, long &oneXFieldSize, double &maxGalvoOpticalAngle, double &minSignalInputVoltage, double &maxSignalInputVoltage, double &galvoRetraceTimeUS, double &pockelsPhaseAdjustUS)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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
			ticpp::Iterator< ticpp::Element > child(configObj,CALIBRATION);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_CALIBRATION_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CALIBRATION_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>fieldSizeCalibration;
						}
						break;
					case 1:
						{
							ss>>flipVerticalScan;
						}
						break;
					case 2:
						{
							ss>>fineOffsetX;
						}
						break;
					case 3:
						{
							ss>>fineOffsetY;
						}
						break;
					case 4:
						{
							ss>>fineScaleX;
						}
						break;
					case 5:
						{
							ss>>fineScaleY;
						}
						break;
					case 6:
						{
							ss>>oneXFieldSize;
						}
						break;
					case 7:
						{
							ss>>maxGalvoOpticalAngle;
						}
					case 8:
						{
							ss>>minSignalInputVoltage;
						}
						break;
					case 9:
						{
							ss>>maxSignalInputVoltage;
						}
						break;
					case 10:
						{
							ss>>galvoRetraceTimeUS;
						}
						break;
					case 11:
						{
							ss>>pockelsPhaseAdjustUS;
						}
						break;
					}
				}
			}	    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetCalibration failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorGGNIXML::SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double fineScaleX, double fineScaleY, long oneXFieldSize, double maxGalvoOpticalAngle, double minSignalInputVoltage, double maxSignalInputVoltage, double galvoRetraceTimeMS, double pockelsPhaseAdjustUS)
{	
	long ret = TRUE;

	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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

		string str;
		stringstream ss;

		ss << fieldSizeCalibration;
		ss << endl;
		ss << flipVerticalScan;
		ss << endl;
		ss << fineOffsetX;
		ss << endl;
		ss << fineOffsetY;
		ss << endl;
		ss << fineScaleX;
		ss << endl;
		ss << fineScaleY;
		ss << endl;
		ss << oneXFieldSize;
		ss << endl;
		ss << maxGalvoOpticalAngle;
		ss << endl;
		ss << minSignalInputVoltage;
		ss << endl;
		ss << maxSignalInputVoltage;
		ss << endl;
		ss<< galvoRetraceTimeMS;
		ss << endl;
		ss<< pockelsPhaseAdjustUS;
		ss << endl;
		long index;

		for(index=0; index<NUM_CALIBRATION_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CALIBRATION), CALIBRATION);
			//get the attribute value for the specified attribute name
			child->SetAttribute(CALIBRATION_ATTR[index], str);
		}
	}
	SaveConfigFile();
	return ret;
}

const char * const ThorGGNIXML::POLARITY = "Polarity";

const char * const ThorGGNIXML::POLARITY_ATTR[NUM_POLARITY_ATTRIBUTES] = {"chanAPol","chanBPol","chanCPol","chanDPol"};

long ThorGGNIXML::GetPolarity(long &chanAPol, long &chanBPol, long &chanCPol, long &chanDPol)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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

			ticpp::Iterator< ticpp::Element > child(configObj,POLARITY);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_POLARITY_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(POLARITY_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>chanAPol;
						}
						break;
					case 1:
						{
							ss>>chanBPol;
						}
						break;
					case 2:
						{
							ss>>chanCPol;
						}
						break;
					case 3:
						{
							ss>>chanDPol;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetPolarity failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThorGGNIXML::TRIGGER = "Trigger";

const char * const ThorGGNIXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"waitTime", "bufferReadyLine","captureActiveLine","captureActiveInvert","stimulationShutter", "shutterPreIdleMS", "shutterPostIdleMS"};

long ThorGGNIXML::GetTrigger(long &waitTime, string &bufferReadyOutput, string &captureActiveOutput, long &captureActiveOutputInvert, string &stimulationShutter, long &shutterPreIdleMS, long &shutterPostIdleMS)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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
			ticpp::Iterator< ticpp::Element > child(configObj,TRIGGER);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_TRIGGER_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(TRIGGER_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						ss>>waitTime;
						break;
					case 1:
						bufferReadyOutput = str;
						break;
					case 2:
						captureActiveOutput = str;
						break;
					case 3:
						ss>>captureActiveOutputInvert;
						break;
					case 4:
						stimulationShutter = str;
						break;
					case 5:
						ss>>shutterPreIdleMS;
						break;
					case 6:
						ss>>shutterPostIdleMS;
						break;
					}
				}
			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetTrigger failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThorGGNIXML::WAVEFORM = "Waveform";

const char * const ThorGGNIXML::WAVEFORM_ATTR[NUM_WAVEFORM_ATTRIBUTES] = {"pockelDigOutput","completeOutput","cycleOutput","iterationOutput", "patternOutput", "patternComplete","activeOutput","epochOutput","cycleInverse"};

long ThorGGNIXML::GetWaveform(string &pockelDigOutput, string &completeOutput, string &cycleOutput, string &iterationOutput, string &patternOutput, string &patternCompleteOutput, string &activeOutput, string &epochOutput, string &cycleInverse)
{	
	wstring settingName = GetDLLName(ThorLSMCam::getInstance()->hDLLInstance) + L"Settings.xml";
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, settingName.c_str());		

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

			ticpp::Iterator< ticpp::Element > child(configObj,WAVEFORM);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_WAVEFORM_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(WAVEFORM_ATTR[attCount],&str);
					switch(attCount)
					{
					case 0:
						{
							pockelDigOutput = str;
						}
						break;
					case 1:
						{
							completeOutput = str;
						}
						break;
					case 2:
						{
							cycleOutput = str;
						}
						break;
					case 3:
						{
							iterationOutput = str;
						}
						break;
					case 4:
						{
							patternOutput = str;
						}
						break;
					case 5:
						{
							patternCompleteOutput = str;
						}
						break;
					case 6:
						{
							activeOutput = str;
						}
						break;
					case 7:
						{
							epochOutput = str;
						}
						break;
					case 8:
						{
							cycleInverse = str;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[_MAX_PATH];
			StringCbPrintfW(errMsg,_MAX_PATH,L"ThorGGNIXML GetWaveform failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorGGNIXML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	

	return TRUE;
}

long ThorGGNIXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();	
	}			

	return TRUE;
}
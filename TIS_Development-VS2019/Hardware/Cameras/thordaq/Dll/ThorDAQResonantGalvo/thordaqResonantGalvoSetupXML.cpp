#include "stdafx.h"
#include "thordaqResonantGalvo.h"
#include "thordaqResonantGalvoSetupXML.h"

ThordaqResonantGalvoXML::ThordaqResonantGalvoXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThordaqResonantGalvoXML::~ThordaqResonantGalvoXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThordaqResonantGalvoXML::CONFIG = "Configuration";

const char * const ThordaqResonantGalvoXML::CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES] = {"field2Theta","crsFrequency","saveCrsFrequencyToLog","pockelsParkAtMinimum", "fieldSizeMin", "fieldSizeMax", "pockelsTurnAroundBlank", "pockelsFlybackBlank", "RGGMode", "rotationPosition", "preMoveGalvoToStartPosition", "waveformUpdateRateSPS", "scannerType"};

long ThordaqResonantGalvoXML::GetConfiguration(double& field2Theta, double& crsFrequency, long& saveCrsFrequencyToLog, long& pockelsParkAtMinimum, long& fieldSizeMin, long& fieldSizeMax, long& pockelsTurnAroundBlank, long& pockelsFlybackBlank, long& RGGMode, long& rotationPosition, long& preMoveGalvotoStart, double& waveformUpdateRateSPS, long& scannerType)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();	

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
							ss>>crsFrequency;
						}
						break;
					case 2:
						{
							ss >> saveCrsFrequencyToLog;
						}
						break;
					case 3:
						{
							ss>>pockelsParkAtMinimum;
						}
						break;
					case 4:
						{
							ss>>fieldSizeMin;
						}
						break;
					case 5:
						{
							ss>>fieldSizeMax;
						}
						break;
					case 6:
						{
							ss>>pockelsTurnAroundBlank;
						}
						break;
					case 7:
					{
						ss >> pockelsFlybackBlank;
					}
					break;
					case 8:
						{
							ss>>RGGMode;
						}
						break;
					case 9:
						{
							ss>>rotationPosition;
						}
						break;
					case 10:
						{
							ss >> preMoveGalvotoStart;
						}
						break;
					case 11:
						{
							ss >> waveformUpdateRateSPS;
						}
						break;
					case 12:
						{
							ss >> scannerType;
						}
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThordaqResonantGalvoSettings GetConfiguration failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThordaqResonantGalvoXML::CALIBRATION = "Calibration";

const char * const ThordaqResonantGalvoXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"fieldSizeCalibration","oneXFieldSize","pockelsPhaseAdjustMicroSec","pockelsPhaseAdjustMicroSec1","pockelsPhaseAdjustMicroSec2","pockelsPhaseAdjustMicroSec3","pockelsMaskPhaseShiftPercent", "preImagingCalibrationCycles", "imagingRampExtensionCycles", "minFlybackCyclesFactor", 
																							"pockelsMinVoltage", "pockelsMaxVoltage", "pockelsMinVoltage1", "pockelsMaxVoltage1", "pockelsMinVoltage2", "pockelsMaxVoltage2","pockelsMinVoltage3", "pockelsMaxVoltage3", 
																							"scanLensFocalLength", "xHalfAngleMaxDeg", "yHalfAngleMaxDeg", "galvoTiltAngleDeg" };

long ThordaqResonantGalvoXML::GetCalibration(double& fieldSizeCalibration, long& oneXFieldSize, double(&pockelsDelayUS)[MAX_POCKELS_CELL_COUNT], double& pockelsMaskPhaseShiftPercent, long& preImagingCalibrationCycles, long& imagingRampExtensionCycles, double& minFlybackCyclesFactor, double(&pockelsMinVoltage)[MAX_POCKELS_CELL_COUNT], double(&pockelsMaxVoltage)[MAX_POCKELS_CELL_COUNT], double& scanLensFocalLength, double& xHalfAngleMax, double& yHalfAngleMax, double& galvoTiltAngle)
{
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThordaqResonantGalvoSettings.xml");

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

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
			ticpp::Iterator< ticpp::Element > child(configObj, CALIBRATION);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_CALIBRATION_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CALIBRATION_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0:
					{
						ss >> fieldSizeCalibration;
					}
					break;
					case 1:
					{
						ss >> oneXFieldSize;
					}
					break;
					case 2:
					{
						ss >> pockelsDelayUS[0];
					}
					break;
					case 3:
					{
						ss >> pockelsDelayUS[1];
					}
					break;
					case 4:
					{
						ss >> pockelsDelayUS[2];
					}
					break;
					case 5:
					{
						ss >> pockelsDelayUS[3];
					}
					break;
					case 6:
					{
						ss >> pockelsMaskPhaseShiftPercent;
					}
					break;
					case 7:
					{
						ss >> preImagingCalibrationCycles;
					}
					break;
					case 8:
					{
						ss >> imagingRampExtensionCycles;
					}
					break;
					case 9:
					{
						ss >> minFlybackCyclesFactor;
					}
					break;
					case 10:
					{
						ss >> pockelsMinVoltage[0];
					}
					break;
					case 11:
					{
						ss >> pockelsMaxVoltage[0];
					}
					break;
					case 12:
					{
						ss >> pockelsMinVoltage[1];
					}
					break;
					case 13:
					{
						ss >> pockelsMaxVoltage[1];
					}
					break;
					case 14:
					{
						ss >> pockelsMinVoltage[2];
					}
					break;
					case 15:
					{
						ss >> pockelsMaxVoltage[2];
					}
					break;
					case 16:
					{
						ss >> pockelsMinVoltage[3];
					}
					break;
					case 17:
					{
						ss >> pockelsMaxVoltage[3];
					}
					break;
					case 18:
					{
						ss >> scanLensFocalLength;
					}
					break;
					case 19:
					{
						ss >> xHalfAngleMax;
					}
					break;
					case 20:
					{
						ss >> yHalfAngleMax;
					}
					break;
					case 21:
					{
						ss >> galvoTiltAngle;
					}
					break;
					}
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"ThordaqResonantGalvoSettings GetCalibration failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThordaqResonantGalvoXML::IO = "IO";

const char * const ThordaqResonantGalvoXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"pockelsVoltageSlopeThreshold", "pockelsLine","pockelsPowerInputLine","pockelsScanVoltageStart","pockelsScanVoltageStop","pockelsLine1","pockelsPowerInputLine1","pockelsScanVoltageStart1","pockelsScanVoltageStop1","pockelsLine2","pockelsPowerInputLine2","pockelsScanVoltageStart2","pockelsScanVoltageStop2","pockelsLine3","pockelsPowerInputLine3","pockelsScanVoltageStart3","pockelsScanVoltageStop3","pockelsResponseType","pockelsResponseType1","pockelsResponseType2","pockelsResponseType3"};

long ThordaqResonantGalvoXML::GetIO(double &pockelsVoltageSlopeThreshold, string &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, string &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, string &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2, string &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2, long &pockelsResponseType3)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

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
							ss>>pockelsVoltageSlopeThreshold;
						}
						break;
					case 1:
						{
							pockelsLine0 = str;
						}
						break;
					case 2:
						{
							pockelsPowerInputLine0 = str;
						}
						break;
					case 3:
						{
							ss>>pockelsScanVoltageStart0;
						}
						break;
					case 4:
						{
							ss>>pockelsScanVoltageStop0;
						}
						break;
					case 5:
						{
							pockelsLine1  = str;
						}
						break;
					case 6:
						{
							pockelsPowerInputLine1 = str;
						}
						break;
					case 7:
						{
							ss>>pockelsScanVoltageStart1;
						}
						break;
					case 8:
						{
							ss>>pockelsScanVoltageStop1;
						}
						break;
					case 9:
						{
							pockelsLine2 = str;
						}
						break;
					case 10:
						{
							pockelsPowerInputLine2 = str;
						}
						break;
					case 11:
						{
							ss>>pockelsScanVoltageStart2;
						}
						break;
					case 12:
						{
							ss>>pockelsScanVoltageStop2;
						}
						break;
					case 13:
						{
							pockelsLine3 = str;
						}
						break;
					case 14:
						{
							pockelsPowerInputLine3 = str;
						}
						break;
					case 15:
						{
							ss>>pockelsScanVoltageStart3;
						}
						break;
					case 16:
						{
							ss>>pockelsScanVoltageStop3;
						}
						break;
					case 17:
						{
							ss>>pockelsResponseType0;
						}
						break;
					case 18:
						{
							ss>>pockelsResponseType1;
						}
						break;
					case 19:
						{
							ss>>pockelsResponseType2;
						}
						break;
					case 20:
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
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"GetIO failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThordaqResonantGalvoXML::TRIGGER = "Trigger";

const char * const ThordaqResonantGalvoXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"waitTime"};

long ThordaqResonantGalvoXML::GetTrigger(long &waitTime)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

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
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorConfocalXML GetTrigger failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}
	return TRUE;
}

const char * const ThordaqResonantGalvoXML::HARDWARE_TEST_MODE = "HardwareTestMode";

const char * const ThordaqResonantGalvoXML::HARDWARE_TEST_MODE_ATTR[NUM_HARDWARE_TEST_MODE_ATTRIBUTES] = {"enable"};

long ThordaqResonantGalvoXML::GetHardwareTestMode(long &enable)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s = ConvertWStringToString(ws);

	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
			
	_xmlObj = new ticpp::Document(s);
	_xmlObj->LoadFile();

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

			ticpp::Iterator< ticpp::Element > child(configObj,HARDWARE_TEST_MODE);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_HARDWARE_TEST_MODE_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(HARDWARE_TEST_MODE_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						ss>>enable;
						break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorConfocalXML GetHardwareTestMode failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThordaqResonantGalvoXML::SetCalibration(double pockelsDelayUS[MAX_POCKELS_CELL_COUNT], double pockelsMaskPhaseShiftPercent, long preImagingCalibrationCycles, long imagingRampExtensionCycles, double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT], double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT], double xHalfAngleMax, double yHalfAngleMax, double galvoTiltAngle)
{
	try
	{
		StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThordaqResonantGalvoSettings.xml");

		wstring ws = _currentPathAndFile;
		string s = ConvertWStringToString(ws);

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
			for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
			{
				ss << pockelsDelayUS[i];
				ss << endl;
			}
			long index;
			const long firstPockelsDelayAttrIndex = 2;

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CALIBRATION), CALIBRATION);

			for (index = 0; index < MAX_POCKELS_CELL_COUNT; index++)
			{
				getline(ss, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index + firstPockelsDelayAttrIndex], str);
			}

			//get the attribute value for the specified attribute name
			child->SetAttribute("pockelsMaskPhaseShiftPercent", to_string(pockelsMaskPhaseShiftPercent));
			child->SetAttribute("preImagingCalibrationCycles", to_string(preImagingCalibrationCycles));
			child->SetAttribute("imagingRampExtensionCycles", to_string(imagingRampExtensionCycles));
			child->SetAttribute("xHalfAngleMaxDeg", to_string(xHalfAngleMax));
			child->SetAttribute("yHalfAngleMaxDeg", to_string(yHalfAngleMax));
			child->SetAttribute("galvoTiltAngleDeg", to_string(galvoTiltAngle));
			stringstream ss2;
			for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
			{
				ss2 << pockelsMinVoltage[i];
				ss2 << endl;
				ss2 << pockelsMaxVoltage[i];
				ss2 << endl;
			}
			const long minMaxPockelsVoltageIndex = 10;
			for (index = 0; index < MAX_POCKELS_CELL_COUNT; index+=2)
			{
				getline(ss2, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index + minMaxPockelsVoltageIndex], str);

				getline(ss2, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index + 1 + minMaxPockelsVoltageIndex], str);
			}
			_xmlObj->SaveFile();			
		}
	}
	catch (ticpp::Exception ex)
	{
		return FALSE;
	}

	return TRUE;
}
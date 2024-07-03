#include "stdafx.h"
#include "thorDAQGalvoGalvo.h"
#include "ThorDAQGalvoGalvoSetupXML.h"

ThorGalvoGalvoXML::ThorGalvoGalvoXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorGalvoGalvoXML::~ThorGalvoGalvoXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorGalvoGalvoXML::CONFIG = "Configuration";

const char* const ThorGalvoGalvoXML::CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES] = { "field2Theta","pockelsParkAtMinimum", "pockelsDelayUS", "pockelsDelayUS1", "pockelsDelayUS2", "pockelsDelayUS3", "galvoParkAtStart", "useExternalBoxFrequency3P","fieldSizeMin","fieldSizeMax","pockelsTurnAroundBlank", "pockelsFlybackBlank", "sumPulsesPerPixel", "scannerType", "maxScannerSampleRate", "enableGalvoXPark", "limitGalvoSpeed", "multiplaneBlankLines", "enableMultiplaneBlankLinesInLiveModeOnly", "maxAngularVelocityRadPerSec", "maxAngularAccelerationRadPerSecSq"};

long ThorGalvoGalvoXML::GetConfiguration(double& field2Theta, long& pockelsParkAtMinimum, double(&pockelsDelayUS)[MAX_POCKELS_CELL_COUNT], long& galvoParkAtStart, long& useExternalBoxFrequency3P, long& fieldSizeMin, long& fieldSizeMax, long& pockelsTurnAroundBlank, long& pockelsFlybackBlank, long& sumPulsesPerPixel, long& scannerType, double& maxScannerSampleRate, long& enableGalvoXPark, long& limitGalvoSpeed, long& multiplaneBlankLines, long& multiplaneBlankLinesInLiveModeOnly, double& maxAngularVelocityRadPerSec, double& maxAngularAccelerationRadPerSecSq)
{
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThorDAQGalvoGalvoSettings.xml");

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
			ticpp::Iterator< ticpp::Element > child(configObj, CONFIG);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_CONFIG_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(CONFIG_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0:
					{
						ss >> field2Theta;
					}
					break;
					case 1:
					{
						ss >> pockelsParkAtMinimum;
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
						ss >> galvoParkAtStart;
					}
					break;
					case 7:
					{
						ss >> useExternalBoxFrequency3P;
					}
					break;
					case 8:
					{
						ss >> fieldSizeMin;
					}
					break;
					case 9:
					{
						ss >> fieldSizeMax;
					}
					break;
					case 10:
					{
						ss >> pockelsTurnAroundBlank;
					}
					break;
					case 11:
					{
						ss >> pockelsFlybackBlank;
					}
					break;
					case 12:
					{
						ss >> sumPulsesPerPixel;
					}
					break;
					case 13:
					{
						ss >> scannerType;
					}
					break;
					case 14:
					{
						ss >> maxScannerSampleRate;
					}
					break;
					case 15:
					{
						ss >> enableGalvoXPark;
					}
					break;
					case 16:
					{
						ss >> limitGalvoSpeed;
					}
					break;
					case 17:
					{
						ss >> multiplaneBlankLines;
					}
					break;
					case 18:
					{
						ss >> multiplaneBlankLinesInLiveModeOnly;
					}
					break;
					case 19:
					{
						ss >> maxAngularVelocityRadPerSec;
					}
					break;
					case 20:
					{
						ss >> maxAngularAccelerationRadPerSecSq;
					}
					break;
					}
				}

			}
		}
		catch (ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvoSettings GetConfiguration failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
		}
	}
	return TRUE;
}

const char* const ThorGalvoGalvoXML::RGGCONFIG = "Configuration";

const char* const ThorGalvoGalvoXML::RGGCONFIG_ATTR[NUM_RGGCONFIG_ATTRIBUTES] = { "RGGMode" };

long ThorGalvoGalvoXML::GetRGGConfiguration(long& rggMode)
{
	string s = "ThordaqResonantGalvoSettings.xml";

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
			ticpp::Iterator< ticpp::Element > child(configObj, RGGCONFIG);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_RGGCONFIG_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(RGGCONFIG_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0:
					{
						ss >> rggMode;
					}
					break;
					}
				}

			}
		}
		catch (ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvoSettings GetConfiguration failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
		}
	}
	return TRUE;
}


long ThorGalvoGalvoXML::SetConfiguration(double pockelsDelayUS[MAX_POCKELS_CELL_COUNT])
{
	try
	{
		StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThorDAQGalvoGalvoSettings.xml");

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
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CONFIG), CONFIG);

			for (index = 0; index < MAX_POCKELS_CELL_COUNT; index++)
			{
				getline(ss, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CONFIG_ATTR[index + firstPockelsDelayAttrIndex], str);
			}
		}

		SaveConfigFile();
	}
	catch (ticpp::Exception ex)
	{
		return FALSE;
	}

	return TRUE;
}

const char * const ThorGalvoGalvoXML::CALIBRATION = "Calibration";

const char * const ThorGalvoGalvoXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"fieldSizeCalibration","flipVerticalScan","fineOffsetX","fineOffsetY","fineScaleX","fineScaleY","oneXFieldSize",
												"pockelsMinVoltage", "pockelsMaxVoltage", "pockelsMinVoltage1", "pockelsMaxVoltage1", "pockelsMinVoltage2", "pockelsMaxVoltage2","pockelsMinVoltage3", "pockelsMaxVoltage3" };

long ThorGalvoGalvoXML::GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &fineScaleX, double &fineScaleY, long &oneXFieldSize, 
										double(&pockelsMinVoltage)[MAX_POCKELS_CELL_COUNT], double(&pockelsMaxVoltage)[MAX_POCKELS_CELL_COUNT])
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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
						  ss >> pockelsMinVoltage[0];
					  }
					  break;
					  case 8:
					  {
						  ss >> pockelsMaxVoltage[0];
					  }
					  break;
					  case 9:
					  {
						  ss >> pockelsMinVoltage[1];
					  }
					  break;
					  case 10:
					  {
						  ss >> pockelsMaxVoltage[1];
					  }
					  break;
					  case 11:
					  {
						  ss >> pockelsMinVoltage[2];
					  }
					  break;
					  case 12:
					  {
						  ss >> pockelsMaxVoltage[2];
					  }
					  break;
					  case 13:
					  {
						  ss >> pockelsMinVoltage[3];
					  }
					  break;
					  case 14:
					  {
						  ss >> pockelsMaxVoltage[3];
					  }
					  break;
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvoSettings GetCalibration failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorGalvoGalvoXML::SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double fineScaleX, double fineScaleY, long oneXFieldSize, double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT], double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT])
{	
	try
	{
		StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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

			long index;
			const long NUM_POCKELS_MIN_MAX_ATTRIBUTE = 8;
			
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CALIBRATION), CALIBRATION);

			for (index = 0; index < NUM_CALIBRATION_ATTRIBUTES - NUM_POCKELS_MIN_MAX_ATTRIBUTE; index++)
			{
				getline(ss, str);


				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index], str);
			}

			stringstream ss2;
			for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
			{
				ss2 << pockelsMinVoltage[i];
				ss2 << endl;
				ss2 << pockelsMaxVoltage[i];
				ss2 << endl;
			}
			const long minMaxPockelsVoltageIndex = index;
			for (index = 0; index < MAX_POCKELS_CELL_COUNT; index++)
			{
				getline(ss2, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index*2 + minMaxPockelsVoltageIndex], str);

				getline(ss2, str);

				//get the attribute value for the specified attribute name
				child->SetAttribute(CALIBRATION_ATTR[index*2 + 1 + minMaxPockelsVoltageIndex], str);
			}
		}

		SaveConfigFile();
	}
	catch(ticpp::Exception ex)
	{
		return FALSE;
	}

	return TRUE;
}

const char * const ThorGalvoGalvoXML::Stream = "Stream";

const char * const ThorGalvoGalvoXML::STREAM_ATTR[NUM_STREAM_ATTRIBUTES] = {"SampleRate"};

long ThorGalvoGalvoXML::GetStreamConfiguration(long &samplerate)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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
			  ticpp::Iterator< ticpp::Element > child(configObj,Stream);

			  for ( child = child.begin( configObj ); child != child.end(); child++)
			  {
				  for(long attCount = 0; attCount<NUM_STREAM_ATTRIBUTES; attCount++)
				  {
					  string str;
					  child->GetAttribute(STREAM_ATTR[attCount],&str);
					  stringstream ss(str);
					  switch(attCount)
					  {
					  case 0:
						  {
							  ss>>samplerate;
						  }
						  break;
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvoSettings GetStream failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char* const ThorGalvoGalvoXML::STIM = "Stim";

const char* const ThorGalvoGalvoXML::STIM_ATTR[NUM_STIM_ATTRIBUTES] = { "activeLoadCount" };

long ThorGalvoGalvoXML::GetStimConfiguration(long& activeLoadCount)
{
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThorDAQGalvoGalvoSettings.xml");

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
			ticpp::Iterator< ticpp::Element > child(configObj, STIM);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_STIM_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(STIM_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
						case 0:
						{
							ss >> activeLoadCount;
						}
						break;
					}
				}

			}
		}
		catch (ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvoSettings GetStimConfiguration failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThorGalvoGalvoXML::IO = "IO";

const char * const ThorGalvoGalvoXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"pockelsVoltageSlopeThreshold", 
	"pockelsLine","pockelsPowerInputLine","pockelsScanVoltageStart","pockelsScanVoltageStop",
	"pockelsLine1","pockelsPowerInputLine1","pockelsScanVoltageStart1","pockelsScanVoltageStop1",
	"pockelsLine2","pockelsPowerInputLine2","pockelsScanVoltageStart2","pockelsScanVoltageStop2",
	"pockelsLine3","pockelsPowerInputLine3","pockelsScanVoltageStart3","pockelsScanVoltageStop3",
	"pockelsResponseType","pockelsResponseType1","pockelsResponseType2","pockelsResponseType3"};

long ThorGalvoGalvoXML::GetIO(double& pockelsVoltageSlopeThreshold, long& pockelsLine0, string& pockelsPowerInputLine0, double& pockelsScanVoltageStart0, double& pockelsScanVoltageStop0, long& pockelsLine1, string& pockelsPowerInputLine1, double& pockelsScanVoltageStart1, double& pockelsScanVoltageStop1, long& pockelsLine2, string& pockelsPowerInputLine2, double& pockelsScanVoltageStart2, double& pockelsScanVoltageStop2, long& pockelsLine3, string& pockelsPowerInputLine3, double& pockelsScanVoltageStart3, double& pockelsScanVoltageStop3, long& pockelsResponseType0, long& pockelsResponseType1, long& pockelsResponseType2, long& pockelsResponseType3)
{
	StringCbPrintfW(_currentPathAndFile, MAX_PATH, L"ThorDAQGalvoGalvoSettings.xml");

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
			ticpp::Iterator< ticpp::Element > child(configObj, IO);

			for (child = child.begin(configObj); child != child.end(); child++)
			{
				for (long attCount = 0; attCount < NUM_IO_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(IO_ATTR[attCount], &str);
					stringstream ss(str);
					switch (attCount)
					{
					case 0:
					{
						ss >> pockelsVoltageSlopeThreshold;
					}
					break;
					case 1:
					{
						//set to 0 in case str is an empty string
						pockelsLine0 = 0;
						ss >> pockelsLine0;
					}
					break;
					case 2:
					{
						pockelsPowerInputLine0 = str;
					}
					break;
					case 3:
					{
						ss >> pockelsScanVoltageStart0;
					}
					break;
					case 4:
					{
						ss >> pockelsScanVoltageStop0;
					}
					break;

					case 5:
					{
						//set to 0 in case str is an empty string
						pockelsLine1 = 0;
						ss >> pockelsLine1;
					}
					break;
					case 6:
					{
						pockelsPowerInputLine1 = str;
					}
					break;
					case 7:
					{
						ss >> pockelsScanVoltageStart1;
					}
					break;
					case 8:
					{
						ss >> pockelsScanVoltageStop1;
					}
					break;
					case 9:
					{
						//set to 0 in case str is an empty string
						pockelsLine2 = 0;
						ss >> pockelsLine2;
					}
					break;
					case 10:
					{
						pockelsPowerInputLine2 = str;
					}
					break;
					case 11:
					{
						ss >> pockelsScanVoltageStart2;
					}
					break;
					case 12:
					{
						ss >> pockelsScanVoltageStop2;
					}
					break;
					case 13:
					{
						//set to 0 in case str is an empty string
						pockelsLine3 = 0;
						ss >> pockelsLine3;
					}
					break;
					case 14:
					{
						pockelsPowerInputLine3 = str;
					}
					break;
					case 15:
					{
						ss >> pockelsScanVoltageStart3;
					}
					break;
					case 16:
					{
						ss >> pockelsScanVoltageStop3;
					}
					break;
					case 17:
					{
						ss >> pockelsResponseType0;
					}
					break;
					case 18:
					{
						ss >> pockelsResponseType1;
					}
					break;
					case 19:
					{
						ss >> pockelsResponseType2;
					}
					break;
					case 20:
					{
						ss >> pockelsResponseType3;
					}
					break;
					}
				}
			}
		}
		catch (ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"GetIO failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
		}
	}
	return TRUE;
}

const char * const ThorGalvoGalvoXML::TRIGGER = "Trigger";

const char * const ThorGalvoGalvoXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"waitTime"};

long ThorGalvoGalvoXML::GetTrigger(long &waitTime)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}
	return TRUE;
}


const char * const ThorGalvoGalvoXML::FIR = "FIR";

const char * const ThorGalvoGalvoXML::FIR_ATTR[NUM_FIR_ATTRIBUTES] = {
	"FIR1Tap1", "FIR1Tap2","FIR1Tap3","FIR1Tap4","FIR1Tap5", "FIR1Tap6","FIR1Tap7","FIR1Tap8",
	"FIR1Tap9", "FIR1Tap10","FIR1Tap11","FIR1Tap12","FIR1Tap13", "FIR1Tap14","FIR1Tap15","FIR1Tap16",
	"FIR2Tap1", "FIR2Tap2","FIR2Tap3","FIR2Tap4","FIR2Tap5", "FIR2Tap6","FIR2Tap7","FIR2Tap8",
	"FIR2Tap9", "FIR2Tap10","FIR2Tap11","FIR2Tap12","FIR2Tap13", "FIR2Tap14","FIR2Tap15","FIR2Tap16"};

long ThorGalvoGalvoXML::GetFIR(double (&firFilter)[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT])
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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
			ticpp::Iterator< ticpp::Element > child(configObj,FIR);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				long attCount = 0;

				for (long p = 0; p < FIR_FILTER_COUNT; ++p)
				{
					for (long f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
					{
						string str;
						child->GetAttribute(FIR_ATTR[attCount],&str);
						stringstream ss(str);
						attCount++;
						double value = 0;
						ss>>value;
						for (long c = 0; c < MAX_CHANNEL_COUNT; ++c)
						{
							firFilter[p][c][f] = value;
						}
					}
				}
			}		  
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"GetFIR failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		}
	}

	return TRUE;
}


long ThorGalvoGalvoXML::SetFIR(double firFilter[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT])
{	
	try
	{
		StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDAQGalvoGalvoSettings.xml");		

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

			string str;
			stringstream ss;
			long attCount = 0;
			for (long p = 0; p < FIR_FILTER_COUNT; ++p)
			{
				for (long f = 0; f < FIR_FILTER_TAP_COUNT; ++f)
				{
					ss << firFilter[p][0][f];
					ss << endl;
				}
			}		

			long index;

			for(index=0; index<NUM_FIR_ATTRIBUTES; index++)
			{
				getline(ss,str);

				// iterate over to get the particular tag element specified as a parameter(tagName)
				ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(FIR), FIR);
				//get the attribute value for the specified attribute name
				child->SetAttribute(FIR_ATTR[index], str);
			}
		}

		SaveConfigFile();
	}
	catch(ticpp::Exception ex)
	{

		return FALSE;
	}

	return TRUE;
}

long ThorGalvoGalvoXML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();	
	}			

	return TRUE;
}
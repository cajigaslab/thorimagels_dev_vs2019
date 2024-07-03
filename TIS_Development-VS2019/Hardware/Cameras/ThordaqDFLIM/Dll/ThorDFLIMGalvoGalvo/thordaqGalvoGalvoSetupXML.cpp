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

const char * const ThorGalvoGalvoXML::CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES] = {"field2Theta","pockelsParkAtMinimum","pockelsDelayUS","acquisitionMode", "maxAngularVelocityRadPerSec", "maxAngularAccelerationRadPerSecSq" };

long ThorGalvoGalvoXML::GetConfiguration(double &field2Theta, long &pockelsParkAtMinimum, long &pockelsDelayUS, long &acquisitionMode, double& maxAngularVelocityRadPerSec, double& maxAngularAccelerationRadPerSecSq)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
							ss>>pockelsParkAtMinimum;
						}
						break;
					case 2:
						{
							ss>>pockelsDelayUS;
						}
						break;
					case 3:
						{
							ss>>acquisitionMode;
						}
						break;
					case 4:
					{
						ss >> maxAngularVelocityRadPerSec;
					}
					break;
					case 5:
					{
						ss >> maxAngularAccelerationRadPerSecSq;
					}
					break;
					}
				}

			}		  
		}
		catch(ticpp::Exception ex)
		{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetConfiguration failed");
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThorGalvoGalvoXML::CALIBRATION = "Calibration";

const char * const ThorGalvoGalvoXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"fieldSizeCalibration","flipVerticalScan","fineOffsetX","fineOffsetY","fineScaleX","fineScaleY","oneXFieldSize"};

long ThorGalvoGalvoXML::GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &fineScaleX, double &fineScaleY, long &oneXFieldSize)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetCalibration failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorGalvoGalvoXML::SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double fineScaleX, double fineScaleY, long oneXFieldSize)
{	
	long ret = FALSE;

	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
		return ret;
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

		for(index=0; index<NUM_CALIBRATION_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CALIBRATION), CALIBRATION);
			//get the attribute value for the specified attribute name
			child->SetAttribute(CALIBRATION_ATTR[index], str);
		}
	}
	//SaveConfigFile();
	return ret;
}

const char * const ThorGalvoGalvoXML::Stream = "Stream";

const char * const ThorGalvoGalvoXML::STREAM_ATTR[NUM_STREAM_ATTRIBUTES] = {"SampleRate","FIRFilter1","FIRFilter2","DCOffset1","DCOffset2"};

long ThorGalvoGalvoXML::GetStreamConfiguration(long &samplerate, long &FIRFilter1, long &FIRFilter2, double &DCOffset1, double &DCOffset2)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
					  case 1:
						  {
							  ss>>FIRFilter1;
						  }
						  break;
					  case 2:
						  {
							  ss>>FIRFilter2;
						  }
						  break;
					  case 3:
						  {
							  ss>>DCOffset1;
						  }
						  break;
					  case 4:
						  {
							  ss>>DCOffset2;
						  }
						  break;
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetStream failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
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
	"pockelsReferenceLine","pockelsResponseType","pockelsResponseType1","pockelsResponseType2","pockelsResponseType3"};

long ThorGalvoGalvoXML::GetIO(double &pockelsVoltageSlopeThreshold, long &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, long &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, long &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2,long &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, string &pockelsReferenceLine, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2,long &pockelsResponseType3)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
							ss>>pockelsLine0;
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
							ss>>pockelsLine1;
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
							ss>>pockelsLine2;
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
							ss>>pockelsLine3;
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
							pockelsReferenceLine = str;
						}
						break;	
					case 18:
						{
							ss>>pockelsResponseType0;
						}
						break;
					case 19:
						{
							ss>>pockelsResponseType1;
						}
						break;
					case 20:
						{
							ss>>pockelsResponseType2;
						}
						break;
					case 21:
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
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
		}
	}	
	return TRUE;
}

const char * const ThorGalvoGalvoXML::TRIGGER = "Trigger";

const char * const ThorGalvoGalvoXML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"waitTime"};

long ThorGalvoGalvoXML::GetTrigger(long &waitTime)
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
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetTrigger failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThorGalvoGalvoXML::CHANNEL_SETTINGS[C] = {"Ch0Settings", "Ch1Settings", "Ch2Settings", "Ch3Settings"};

const char * const ThorGalvoGalvoXML::CHANNEL_SETTINGS_ATTR[NUM_CHANNEL_SETTINGS_ATTRIBUTES] = {"fineShiftA","fineShiftB","coarseShiftA","coarseShiftB","enableIntAdj","threshold","baselineTolerance","maxLevel0","maxLevel1"};

long  ThorGalvoGalvoXML::GetChannelSettings(long (&fineShiftA)[C],long (&fineShiftB)[C],long (&coarseShiftA)[C],long (&coarseShiftB)[C],long (&enableIntAdj)[C],long (&threshold)[C],
		long (&baselineTolerance)[C],long (&maxLevel0)[C],long (&maxLevel1)[C])
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
			for (int i = 0; i < C; ++i)
			{
				ticpp::Iterator< ticpp::Element > child(configObj,CHANNEL_SETTINGS[i]);

				for ( child = child.begin( configObj ); child != child.end(); child++)
				{
					for(long attCount = 0; attCount<NUM_CHANNEL_SETTINGS_ATTRIBUTES; attCount++)
					{
						string str;
						child->GetAttribute(CHANNEL_SETTINGS_ATTR[attCount],&str);
						stringstream ss(str);
						switch(attCount)
						{
						case 0:
							{
								ss>>fineShiftA[i];
							}
							break;
						case 1:
							{
								ss>>fineShiftB[i];
							}
							break;
						case 2:
							{
								ss>>coarseShiftA[i];
							}
							break;
						case 3:
							{
								ss>>coarseShiftB[i];
							}
							break;
						case 4:
							{
								ss>>enableIntAdj[i];
							}
							break;
						case 5:
							{
								ss>>threshold[i];
							}
							break;
						case 6:
							{
								ss>>baselineTolerance[i];
							}
							break;
						case 7:
							{
								ss>>maxLevel0[i];
							}
							break;
						case 8:
							{
								ss>>maxLevel1[i];
							}
							break;
						}
					}
				}
			}		    
		}

		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetChannelSettings failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}
	return TRUE;
}

long ThorGalvoGalvoXML::SetChannelSettings(long (fineShiftA)[C],long (fineShiftB)[C],long (coarseShiftA)[C],long (coarseShiftB)[C],long (enableIntAdj)[C],long (threshold)[C],
		long (baselineTolerance)[C],long (maxLevel0)[C],long (maxLevel1)[C])
{	
	try
	{
		long ret = FALSE;

		StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
			return ret;
		}
		else
		{
			for (int i = 0; i < C; ++i)
			{
				string str;
				stringstream ss;

				ss << fineShiftA[i];
				ss << endl;
				ss << fineShiftB[i];
				ss << endl;
				ss << coarseShiftA[i];
				ss << endl;
				ss << coarseShiftB[i];
				ss << endl;
				ss << enableIntAdj[i];
				ss << endl;
				ss << threshold[i];
				ss << endl;
				ss << baselineTolerance[i];
				ss << endl;
				ss << maxLevel0[i];
				ss << endl;
				ss << maxLevel1[i];
				ss << endl;

				long index;

				for(index=0; index<NUM_CHANNEL_SETTINGS_ATTRIBUTES; index++)
				{
					getline(ss,str);

					// iterate over to get the particular tag element specified as a parameter(tagName)
					ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(CHANNEL_SETTINGS[i]), CHANNEL_SETTINGS[i]);
					//get the attribute value for the specified attribute name
					child->SetAttribute(CHANNEL_SETTINGS_ATTR[index], str);
				}
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


const char * const ThorGalvoGalvoXML::FRONTEND = "FrontEndTuning";

const char * const ThorGalvoGalvoXML::FRONTEND_ATTR[NUM_FRONTEND_ATTRIBUTES] = {"syncIDelay","resyncDelay","forceSyncEveryLine"};

long ThorGalvoGalvoXML::GetFrontEndTuning(long &syncDelay, long &resyncDelay, long &forceSyncEveryLine)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
			  ticpp::Iterator< ticpp::Element > child(configObj,FRONTEND);

			  for ( child = child.begin( configObj ); child != child.end(); child++)
			  {
				  for(long attCount = 0; attCount<NUM_FRONTEND_ATTRIBUTES; attCount++)
				  {
					  string str;
					  child->GetAttribute(FRONTEND_ATTR[attCount],&str);
					  stringstream ss(str);
					  switch(attCount)
					  {
					  case 0:
						  {
							  ss>>syncDelay;
						  }
						  break;
					  case 1:
						  {
							  ss>>resyncDelay;
						  }
						  break;
					  case 2:
						  {
							  ss>>forceSyncEveryLine;
						  }
						  break;
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDFLIMGalvoGalvoSettings GetStream failed");
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorGalvoGalvoXML::SetFrontEndTuning(long syncDelay, long resyncDelay, long forceSyncEveryLine)
{
	try
	{
		long ret = FALSE;

		StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorDFLIMGalvoGalvoSettings.xml");		

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
			return ret;
		}
		else
		{

			string str;
			stringstream ss;

			ss << syncDelay;
			ss << endl;
			ss << resyncDelay;
			ss << endl;
			ss << forceSyncEveryLine;
			ss << endl;

			long index;

			for(index=0; index<NUM_FRONTEND_ATTRIBUTES; index++)
			{
				getline(ss,str);

				// iterate over to get the particular tag element specified as a parameter(tagName)
				ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(FRONTEND), FRONTEND);
				//get the attribute value for the specified attribute name
				child->SetAttribute(FRONTEND_ATTR[index], str);
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
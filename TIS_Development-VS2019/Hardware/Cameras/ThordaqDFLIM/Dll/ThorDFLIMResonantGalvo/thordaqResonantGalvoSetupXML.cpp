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

const char * const ThordaqResonantGalvoXML::CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES] = {"field2Theta","crsFrequency","pockelsParkAtMinimum"};

long ThordaqResonantGalvoXML::GetConfiguration(double &field2Theta,double &crsFrequency, long &pockelsParkAtMinimum)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
							ss>>pockelsParkAtMinimum;
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

const char * const ThordaqResonantGalvoXML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"fieldSizeCalibration","oneXFieldSize","pockelsPhaseAdjustMicroSec","pockelsMaskPhaseShiftPercent"};

long ThordaqResonantGalvoXML::GetCalibration(double &fieldSizeCalibration,long &oneXFieldSize,double &pockelsPhaseAdjustMicroSec, double &pockelsMaskPhaseShiftPercent)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
							  ss>>oneXFieldSize;
						  }
						  break;
					  case 2:
						  {
							  ss>>pockelsPhaseAdjustMicroSec;
						  }
						  break;
					  case 3:
						  {
							  ss>>pockelsMaskPhaseShiftPercent;
						  }
						  break;
					  }
				  }

			  }		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThordaqResonantGalvoSettings GetCalibration failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThordaqResonantGalvoXML::Stream = "Stream";

const char * const ThordaqResonantGalvoXML::STREAM_ATTR[NUM_STREAM_ATTRIBUTES] = {"DCOffset1","DCOffset2"};

long ThordaqResonantGalvoXML::GetStreamConfiguration(double &DCOffset1, double &DCOffset2)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
							  ss>>DCOffset1;
						  }
						  break;
					  case 1:
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
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThordaqResonantGalvoSettings GetStream failed");
			CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThordaqResonantGalvoXML::IO = "IO";

const char * const ThordaqResonantGalvoXML::IO_ATTR[NUM_IO_ATTRIBUTES] = {"pockelsVoltageSlopeThreshold", "pockelDigOutput", "pockelsLine","pockelsPowerInputLine","pockelsScanVoltageStart","pockelsScanVoltageStop","pockelsLine1","pockelsPowerInputLine1","pockelsScanVoltageStart1","pockelsScanVoltageStop1","pockelsLine2","pockelsPowerInputLine2","pockelsScanVoltageStart2","pockelsScanVoltageStop2","pockelsResponseType","pockelsResponseType1","pockelsResponseType2"};

long ThordaqResonantGalvoXML::GetIO(double &pockelsVoltageSlopeThreshold, string &pockelDigOutput, string &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, string &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, string &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThordaqResonantGalvoSettings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
							pockelDigOutput = str;
						}
						break;
					case 2:
						{
							pockelsLine0 = str;
						}
						break;
					case 3:
						{
							pockelsPowerInputLine0 = str;
						}
						break;
					case 4:
						{
							ss>>pockelsScanVoltageStart0;
						}
						break;
					case 5:
						{
							ss>>pockelsScanVoltageStop0;
						}
						break;

					case 6:
						{
							pockelsLine1  = str;
						}
						break;
					case 7:
						{
							pockelsPowerInputLine1 = str;
						}
						break;
					case 8:
						{
							ss>>pockelsScanVoltageStart1;
						}
						break;
					case 9:
						{
							ss>>pockelsScanVoltageStop1;
						}
						break;
					case 10:
						{
							pockelsLine2 = str;
						}
						break;
					case 11:
						{
							pockelsPowerInputLine2 = str;
						}
						break;
					case 12:
						{
							ss>>pockelsScanVoltageStart2;
						}
						break;
					case 13:
						{
							ss>>pockelsScanVoltageStop2;
						}
						break;
						
					case 14:
						{
							ss>>pockelsResponseType0;
						}
						break;
					case 15:
						{
							ss>>pockelsResponseType1;
						}
						break;
					case 16:
						{
							ss>>pockelsResponseType2;
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
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
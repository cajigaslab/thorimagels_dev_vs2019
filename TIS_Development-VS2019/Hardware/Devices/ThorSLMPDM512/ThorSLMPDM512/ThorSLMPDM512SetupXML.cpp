#include "stdafx.h"
#include "Strsafe.h"
#include "ThorSLMPDM512.h"
#include "ThorSLMPDM512SetupXML.h"

ThorSLMPDM512XML::ThorSLMPDM512XML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
}

ThorSLMPDM512XML::~ThorSLMPDM512XML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}
}

const char * const ThorSLMPDM512XML::POSTTRANSFORM = "PostCalibration";

const char * const ThorSLMPDM512XML::POSTTRANSFORM_ATTR[NUM_POSTTRANSFORM_ATTRIBUTES] = {"verticalFlip","rotateAngle","scaleX","scaleY","offsetX","offsetY"};

long ThorSLMPDM512XML::GetPostTransform(long &verticalFlip, double &rotateAngle, double &scaleX, double &scaleY, long &offsetX, long &offsetY)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
			ticpp::Iterator< ticpp::Element > child(configObj,POSTTRANSFORM);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_POSTTRANSFORM_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(POSTTRANSFORM_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							ss>>verticalFlip;
						}
						break;
					case 1:
						{
							ss>>rotateAngle;
						}
						break;
					case 2:
						{
							ss>>scaleX;
						}
						break;
					case 3:
						{
							ss>>scaleY;
						}
						break;
					case 4:
						{
							ss>>offsetX;
						}
						break;
					case 5:
						{
							ss>>offsetY;
						}
						break;
					}
				}

			}		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorSLMPDM512XML GetPostTransform failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorSLMPDM512XML::SetPostTransform(long verticalFlip, double rotateAngle, double scaleX, double scaleY, long offsetX, long offsetY)
{
	long ret = TRUE;

	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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

		ss << verticalFlip;
		ss << endl;
		ss << rotateAngle;
		ss << endl;
		ss << scaleX;
		ss << endl;
		ss << scaleY;
		ss << endl;
		ss << offsetX;
		ss << endl;
		ss << offsetY;
		ss << endl;

		long index;

		for(index=0; index<NUM_POSTTRANSFORM_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(POSTTRANSFORM), POSTTRANSFORM);
			//get the attribute value for the specified attribute name
			child->SetAttribute(POSTTRANSFORM_ATTR[index], str);
		}
	}
	SaveConfigFile();
	return ret;}

const char * const ThorSLMPDM512XML::CALIBRATION = "Calibration";
const char * const ThorSLMPDM512XML::CALIBRATION2 = "Calibration2";

const char * const ThorSLMPDM512XML::CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES] = {"wavelengthNM","coeff1","coeff2","coeff3","coeff4","coeff5","coeff6", "coeff7", "coeff8"};

long ThorSLMPDM512XML::GetCalibration(int id, double& wavelengthNM, double &coeff1, double &coeff2, double &coeff3, double &coeff4, double &coeff5, double &coeff6, double &coeff7, double &coeff8)
{	
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
			ticpp::Iterator< ticpp::Element > child(configObj, 1 == id ? CALIBRATION : CALIBRATION2);

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
							ss>> wavelengthNM;
						}
						break;
					case 1:
						{
							ss>>coeff1;
						}
						break;
					case 2:
						{
							ss>>coeff2;
						}
						break;
					case 3:
						{
							ss>>coeff3;
						}
						break;
					case 4:
						{
							ss>>coeff4;
						}
						break;
					case 5:
						{
							ss>>coeff5;
						}
						break;
					case 6:
						{
							ss>>coeff6;
						}
						break;
					case 7:
						{
							ss>>coeff7;
						}
						break;
					case 8:
						{
							ss>>coeff8;
						}
						break;
					}
				}

			}		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorSLMPDM512XML GetCalibration failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorSLMPDM512XML::SetCalibration(int id, double coeff1, double coeff2, double coeff3, double coeff4, double coeff5, double coeff6, double coeff7, double coeff8)
{	
	long ret = TRUE;

	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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

		ss << coeff1;
		ss << endl;
		ss << coeff2;
		ss << endl;
		ss << coeff3;
		ss << endl;
		ss << coeff4;
		ss << endl;
		ss << coeff5;
		ss << endl;
		ss << coeff6;
		ss << endl;
		ss << coeff7;
		ss << endl;
		ss << coeff8;
		ss << endl;

		long index;

		for(index=1; index<NUM_CALIBRATION_ATTRIBUTES; index++)
		{
			getline(ss,str);

			// iterate over to get the particular tag element specified as a parameter(tagName)
			ticpp::Iterator<ticpp::Element> child(configObj->FirstChildElement(1 == id ? CALIBRATION : CALIBRATION2), 1 == id ? CALIBRATION : CALIBRATION2);
			//get the attribute value for the specified attribute name
			child->SetAttribute(CALIBRATION_ATTR[index], str);
		}
	}
	SaveConfigFile();
	return ret;
}

const char * const ThorSLMPDM512XML::SPEC = "Spec";

const char * const ThorSLMPDM512XML::SPEC_ATTR[NUM_SPEC_ATTRIBUTES] = {"Name","dmdMode","overDrive","transientFrames","pixelUM","pixelXmin","pixelXmax","pixelYmin","pixelYmax","LUT","overDriveLUT","waveFront"};

long ThorSLMPDM512XML::GetSpec(string &name, long &dmdMode, long &overDrive, unsigned int &transientFrames, long &pixelUM, long &pixelXmin, long &pixelXmax, long &pixelYmin, long &pixelYmax, string &lut, string &odLUT, string &wavefront)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
			ticpp::Iterator< ticpp::Element > child(configObj,SPEC);

			for ( child = child.begin( configObj ); child != child.end(); child++)
			{
				for(long attCount = 0; attCount<NUM_SPEC_ATTRIBUTES; attCount++)
				{
					string str;
					child->GetAttribute(SPEC_ATTR[attCount],&str);
					stringstream ss(str);
					switch(attCount)
					{
					case 0:
						{
							name = str;
						}
						break;
					case 1:
						{
							ss>>dmdMode;
						}
						break;
					case 2:
						{
							ss>>overDrive;
						}
						break;
					case 3:
						{
							ss>>transientFrames;
						}
						break;
					case 4:
						{
							ss>>pixelUM;
						}
						break;						
					case 5:
						{
							ss>>pixelXmin;
						}
						break;
					case 6:
						{
							ss>>pixelXmax;
						}
						break;
					case 7:
						{
							ss>>pixelYmin;
						}
						break;
					case 8:
						{
							ss>>pixelYmax;
						}
						break;
					case 9:
						{
							lut = str;
						}
						break;
					case 10:
						{
							odLUT = str;
						}
						break;
					case 11:
						{
							wavefront = str;
						}
						break;
					}
				}

			}		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorSLMPDM512XML GetSpec failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

const char * const ThorSLMPDM512XML::TRIGGER = "Trigger";

const char * const ThorSLMPDM512XML::TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES] = {"counterLine","triggerLine"};

long ThorSLMPDM512XML::GetTrigger(string &counterLine, string &triggerInput)
{
	StringCbPrintfW(_currentPathAndFile,MAX_PATH,L"ThorSLMPDM512Settings.xml");		

	wstring ws = _currentPathAndFile;
	string s(ws.begin(), ws.end());
	s.assign(ws.begin(), ws.end());

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
						{
							counterLine = str;
						}
						break;
					case 1:
						{
							triggerInput = str;
						}
						break;
					}
				}

			}		    
		}
		catch(ticpp::Exception ex)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"ThorSLMPDM512XML GetTrigger failed");
			LogMessage(errMsg,VERBOSE_EVENT);
			return FALSE;
		}
	}

	return TRUE;
}

long ThorSLMPDM512XML::OpenConfigFile(string path)
{	
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}

	_xmlObj = new ticpp::Document(path);
	_xmlObj->LoadFile();	

	return TRUE;
}

long ThorSLMPDM512XML::SaveConfigFile()
{
	if(_xmlObj != NULL)
	{
		_xmlObj->SaveFile();	
	}			

	return TRUE;
}
#include "HardwareSetupXML.h"
#include <shlobj.h>
#include "Strsafe.h"
#include <exception>
#include "..\StringCpp.h"
#include "..\Camera.h"
#include "..\Device.h"
#include "..\ResourceManager\ResourceManager\ResourceManager.h"

#define MSG_LENGTH 256
wchar_t messageHard[MSG_LENGTH];
HardwareSetupXML::HardwareSetupXML()
{
	_xmlObj = NULL;
	_currentPathAndFile;
	_filePathChanged = TRUE;
	_fastLoad = FALSE;
}

HardwareSetupXML::~HardwareSetupXML()
{
	if(_xmlObj != NULL)
	{
		delete _xmlObj;
		_xmlObj = NULL;
	}
}

const char * const HardwareSetupXML::ROOT_TAG = "HardwareSettings";

const char * const HardwareSetupXML::WAVELENGTHS = "HardwareSettings";
const char * const HardwareSetupXML::WAVELENGTH = "Wavelength";
const char * const HardwareSetupXML::WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES] = {"name","wavelength","ex","em","dic","fluor","color","bp","wp"};
long HardwareSetupXML::GetWavelength(string name, double &ex, double &em, double &dic, long &fluor, string &color, long &bp, long &wp)
{	
	if ( _xmlObj == NULL )
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML GetWavelength _xmlObj is null");
		return FALSE;
	}

	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,WAVELENGTH);

		for ( child = child.begin( configObj ); child != child.end(); child++)
		{
			string str;			  
			string waveName;	

			GetAttribute(child, WAVELENGTH, WAVELENGTH_ATTR[0],str);

			stringstream ss(str);
			waveName = ss.str();		  

			if (waveName == name)
			{		
				string strr;
				long attCount;
				for(attCount = 0; attCount<NUM_WAVELENGTH_ATTRIBUTES; attCount++)
				{							  
					GetAttribute(child, WAVELENGTH, WAVELENGTH_ATTR[attCount],strr);	
					stringstream sss(strr);

					switch(attCount)
					{
					case 2:
						sss>>ex;						 
						break;
					case 3:
						sss>>em;						 
						break;
					case 4:
						sss>>dic;					
						break;
					case 5:
						sss>>fluor;					
						break;
					case 6:
						sss>>color;
						break;
					case 7:
						sss>>bp;
						break;
					case 8:
						sss>>wp;
						break;
					}
				}
				return TRUE;
			}			 			  
		}		  
	}	
	return FALSE;
}

long  HardwareSetupXML::GetWavelengthIndex(string name, long &index)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,WAVELENGTH);
		long i=0;


		for ( child = child.begin( configObj ); child != child.end(); child++,i++)
		{
			string str;			  
			string waveName;	

			GetAttribute(child, WAVELENGTH, WAVELENGTH_ATTR[0], str);	
			stringstream ss(str);
			waveName = ss.str();		  

			if (waveName == name)
			{	
				index=i;
				return TRUE;
			}			 			  
		}		  
	}	
	return FALSE;
}

long HardwareSetupXML::GetWavelengthName(long index,string &name)
{	
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Iterator< ticpp::Element > child(configObj,WAVELENGTH);

		long count = 0;
		for ( child = child.begin( configObj ); child != child.end(); child++, count++)
		{		  
			if(count == index)
			{
				string str;			  

				GetAttribute(child, WAVELENGTH, WAVELENGTH_ATTR[0], str);
				stringstream ss(str);
				name = ss.str();

				return TRUE;
			}
		}
	}

	return FALSE;
}

long HardwareSetupXML::GetCurrentPath()
{
	wstring tempPath = ResourceManager::getInstance()->GetHardwareSettingsFilePathAndName();

	wchar_t newFile[_MAX_PATH];	
	StringCbPrintfW(newFile,_MAX_PATH, tempPath.c_str());
	_filePathChanged = (lstrcmpW(_currentPathAndFile,newFile) == 0) ? FALSE : TRUE;	
	StringCbPrintfW(_currentPathAndFile,_MAX_PATH, tempPath.c_str());
	if(GetFileAttributes(_currentPathAndFile) == INVALID_FILE_ATTRIBUTES)
	{	
		return FALSE;
	}

	return TRUE;
}

const char * const HardwareSetupXML::OBJECTIVES = "Objectives";
const char * const HardwareSetupXML::OBJECTIVE = "Objective";
const char * const HardwareSetupXML::OBJECTIVE_ATTR[NUM_OBJECTIVE_ATTRIBUTES] = {"name","mag","na","afScanStartMM","afFocusOffsetMM","afAdaptiveOffsetMM","beamExp","beamWavelength","beamExp2","beamWavelength2","turretPosition","zAxisToEscape","zAxisEscapeDistance","fineAfPercentDecrease"};
long HardwareSetupXML::GetMagInfoFromMagVal(double mag, string &name, long &position, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double &finePercentage)
{	
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{   
		ticpp::Node *objectivesObj = configObj->FirstChild(OBJECTIVES,false);

		ticpp::Iterator< ticpp::Element > child(objectivesObj->FirstChildElement(false),OBJECTIVE);

		long index=1;
		for ( child = child.begin( objectivesObj ); child != child.end(); child++,index++)
		{
			string str;			  
			double magVal;	
			//	  string strr;
			long attCount;

			for(attCount = 0; attCount<NUM_OBJECTIVE_ATTRIBUTES; attCount++)
			{

				try
				{
					GetAttribute(child, OBJECTIVE, OBJECTIVE_ATTR[attCount], str);	
					stringstream sss(str);

					switch(attCount)
					{
					case 0:	
						name = str;
						break;
					case 1:
						sss>>magVal;
						break;
					case 2:
						sss>>numAper;
					case 3:
						sss>>afScanStart;					
						break;
					case 4:
						sss>>afFocusOffset;					
						break;
					case 5:
						sss>>afAdaptiveOffset;					
						break;
					case 6:
						sss>>beamExpPos;
						break;
					case 7:
						sss>>beamExpWavelength;
						break;
					case 8:
						sss>>beamExpPos2;
						break;
					case 9:
						sss>>beamExpWavelength2;
						break;
					case 10:
						sss>>turretPosition;
						break;
					case 11:
						sss>>zAxisToEscape;
						break;
					case 12:
						sss>>zAxisEscapeDistance;
						break;
					case 13:
						sss >> finePercentage;
						break;
					}
				}
				catch(...)
				{
				}
			}

			const double MAG_PRECISION = .001;
			if ((magVal >= mag) && (magVal < (mag + MAG_PRECISION)))
			{
				position = index;			
				return TRUE;
			}			 			  
		}		  
	}	

	position = 1;
	return FALSE;
}


long HardwareSetupXML::GetMagInfoFromPosition(long position,  string &name, double &mag, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double &finePercentage)
{	
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{   
		ticpp::Node *objectivesObj = configObj->FirstChild(OBJECTIVES,false);

		ticpp::Iterator< ticpp::Element > child(objectivesObj->FirstChildElement(false),OBJECTIVE);

		long index=1;
		for ( child = child.begin( objectivesObj ); child != child.end(); child++,index++)
		{
			if(index == position)
			{
				string str;	
				//string strr;
				long attCount;

				for(attCount = 0; attCount<NUM_OBJECTIVE_ATTRIBUTES; attCount++)
				{
					try
					{
						GetAttribute(child, OBJECTIVE, OBJECTIVE_ATTR[attCount], str);
						stringstream sss(str);

						switch(attCount)
						{
						case 0:	
							name = str;
							break;
						case 1:
							sss>>mag;
							break;
						case 2:
							sss>>numAper;
							break;
						case 3:
							sss>>afScanStart;					
							break;
						case 4:
							sss>>afFocusOffset;					
							break;
						case 5:
							sss>>afAdaptiveOffset;					
							break;
						case 6:
							sss>>beamExpPos;
							break;
						case 7:
							sss>>beamExpWavelength;
							break;
						case 8:
							sss>>beamExpPos2;
							break;
						case 9:
							sss>>beamExpWavelength2;
							break;
						case 10:
							sss>>turretPosition;
							break;
						case 11:
							sss>>zAxisToEscape;
							break;
						case 12:
							sss>>zAxisEscapeDistance;
							break;
						case 13:
							sss >> finePercentage;
							break;
						}
					}
					catch(...)
					{
					}
				}
				return TRUE;
			}
		}		  
	}	

	return FALSE;
}

long HardwareSetupXML::GetMagInfoFromName(string name,double &mag, long &position, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double &finePercentage)
{	
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{   
		ticpp::Node *objectivesObj = configObj->FirstChild(OBJECTIVES,false);

		ticpp::Iterator< ticpp::Element > child(objectivesObj->FirstChildElement(false),OBJECTIVE);

		long index=1;
		for ( child = child.begin( objectivesObj ); child != child.end(); child++,index++)
		{
			string str;		
			//	  string strr;
			string strTemp;
			long attCount;

			for(attCount = 0; attCount<NUM_OBJECTIVE_ATTRIBUTES; attCount++)
			{
				try
				{
					GetAttribute(child, OBJECTIVE, OBJECTIVE_ATTR[attCount], str);	
					stringstream sss(str);

					switch(attCount)
					{
					case 0:	
						strTemp = str;
						break;
					case 1:
						sss>>mag;
						break;
					case 2:
						sss>>numAper;
					case 3:
						sss>>afScanStart;					
						break;
					case 4:
						sss>>afFocusOffset;					
						break;
					case 5:
						sss>>afAdaptiveOffset;					
						break;
					case 6:
						sss>>beamExpPos;
						break;
					case 7:
						sss>>beamExpWavelength;
						break;
					case 8:
						sss>>beamExpPos2;
						break;
					case 9:
						sss>>beamExpWavelength2;
						break;
					case 10:
						sss>>turretPosition;
						break;
					case 11:
						sss>>zAxisToEscape;
						break;
					case 12:
						sss>>zAxisEscapeDistance;
						break;
					case 13:
						sss >> finePercentage;
						break;
					}
				}
				catch(ticpp::Exception ex)
				{
				}
			}

			if(0 == strTemp.compare(name))
			{
				position = index;			
				return TRUE;
			}			 			  
		}		  
	}	

	position = 1;
	return FALSE;
}

const char * const HardwareSetupXML::COLORCHANNEL_ATTR[NUM_COLORCHANNEL_ATTRIBUTES] = {"name"};
const char * const HardwareSetupXML::COLORCHANNELS = "ColorChannels";
const char * const HardwareSetupXML::COLORCHANNELRED = "Red";
const char * const HardwareSetupXML::COLORCHANNELGREEN = "Green";
const char * const HardwareSetupXML::COLORCHANNELBLUE = "Blue";
const char * const HardwareSetupXML::COLORCHANNELCYAN = "Cyan";
const char * const HardwareSetupXML::COLORCHANNELMAGENTA = "Magenta";
const char * const HardwareSetupXML::COLORCHANNELYELLOW = "Yellow";
const char * const HardwareSetupXML::COLORCHANNELGRAY = "Gray";
long HardwareSetupXML::GetColorChannels(string &nameRed, string &nameGreen,string &nameBlue, string &nameCyan, string &nameMagenta, string &nameYellow,string &nameGray)
{	
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Element *parent = configObj->FirstChildElement(COLORCHANNELS,false);

		ticpp::Element  *child;

		for ( child = parent->FirstChildElement(false); child ;child = child->NextSiblingElement(false))
		{
			string str;			  
			string waveName;	

			str = child->Value();			
			GetAttribute(child, str, COLORCHANNEL_ATTR[0], str);	
			stringstream sss(str);

			if (COLORCHANNELRED == str)
				sss>>nameRed;
			if (COLORCHANNELGREEN == str)
				sss>>nameGreen;
			if (COLORCHANNELBLUE == str)
				sss>>nameBlue;
			if (COLORCHANNELCYAN == str)
				sss>>nameCyan;	
			if (COLORCHANNELMAGENTA == str)
				sss>>nameMagenta;					
			if (COLORCHANNELYELLOW == str)
				sss>>nameYellow;							  
			if (COLORCHANNELGRAY == str)
				sss>>nameGray;				  
		}	

		return TRUE;
	}	

	return FALSE;
}

long HardwareSetupXML::OpenConfigFile(string path)
{	
	if(!_fastLoad)
	{
			//reloading 
			if(_xmlObj != NULL)
			{
				delete _xmlObj;
				_xmlObj = NULL;
			}

			_xmlObj = new ticpp::Document(path);
			_xmlObj->LoadFile();	
	}
	else
	{
		//only load if the xml file has not been loaded
		if((_filePathChanged)&&(_xmlObj != NULL))
		{
			delete _xmlObj;
			_xmlObj = NULL;
		}

		if(_xmlObj == NULL)		
		{
			_xmlObj = new ticpp::Document(path);
			_xmlObj->LoadFile();	
		}			
	}
	return TRUE;
}

const char * const HardwareSetupXML::STREAMING = "Streaming";
const char * const HardwareSetupXML::STREAMING_ATTR[NUM_STREAMING_ATTRIBUTES] = {"path", "previewRate", "alwaysSaveImagesOnStop"};
long HardwareSetupXML::GetStreaming(wstring &path, double &previewRate, long & alwaysSaveImagesWhenStopped)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Element *child = configObj->FirstChildElement(STREAMING,false);

		string str;	
		wstring str2;
		long attCount;
		for(attCount = 0; attCount<NUM_STREAMING_ATTRIBUTES; attCount++)
		{
			str.clear();
			GetAttribute(child, STREAMING, STREAMING_ATTR[attCount], str);	
			stringstream ss(str);

			switch(attCount)
			{
			case 0:
				str2 = wstring(str.length(), L' '); // Make room for characters
				// Copy string to wstring.
				std::copy(str.begin(), str.end(), str2.begin());
				path = str2;	
				break;
			case 1:
				ss>>previewRate;
				break;
			case 2:
				ss >> alwaysSaveImagesWhenStopped;
				break;
			}
		}
		return TRUE;
	}	

	return FALSE;	
}

const char * const HardwareSetupXML::LSM = "LSM";
const char * const HardwareSetupXML::LSM_ATTR[NUM_LSM_ATTRIBUTES] = {"fieldSizeCalibration"};
long HardwareSetupXML::GetLSM(double &fieldSizeCalibration)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"configObj == NULL");
		return FALSE;
	}
	else
	{
		ticpp::Element *child = configObj->FirstChildElement(LSM,false);

		if(child)
		{
			string str;			  

			GetAttribute(child, LSM, LSM_ATTR[0], str);
			stringstream ss(str);
			ss>>fieldSizeCalibration;		  	  
			return TRUE;
		}

	}	
	return FALSE;	
}

const char * const HardwareSetupXML::CAMERA_TAG = "Camera";
const char * const HardwareSetupXML::IMAGEDETECTORS_TAG = "ImageDetectors";
long HardwareSetupXML::GetActivationCameraID(long &id)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActivationCameraID _xmlObj is null");
		return FALSE;
	}

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		ticpp::Element* parent = configObj->FirstChildElement(IMAGEDETECTORS_TAG); 

		if(parent->NoChildren())
			return FALSE;

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement(LSM_TAG), LSM_TAG);

		//find the last child element tag
		for( ; child != child.end(); child++)
		{
			string activationStr;			
			GetAttribute(child, LSM_TAG, HARDWARE_ATTR[ACTIVATION_ATTR], activationStr);			
			long activation = atol(activationStr.c_str());

			if(activation == 1)
			{
				string idStr;
				GetAttribute(child, LSM_TAG, HARDWARE_ATTR[ID_ATTR], idStr);
				id = atol(idStr.c_str());				
				return TRUE;
			}
		}
	}
	catch(ticpp::Exception ex)
	{
		const char* msg = ex.what();
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActivationCameraID an exception has occurred ");
		return FALSE;
	}

	return FALSE;
}

const char * const HardwareSetupXML::LSM_TAG = "LSM";
const char * const HardwareSetupXML::DEVICES_TAG = "Devices";
const char * const HardwareSetupXML::CONTROL_UNIT_TAG = "ControlUnit";
const char * const HardwareSetupXML::PMT1_TAG = "PMT1";
const char * const HardwareSetupXML::PMT2_TAG = "PMT2";
const char * const HardwareSetupXML::PMT3_TAG = "PMT3";
const char * const HardwareSetupXML::PMT4_TAG = "PMT4";
const char * const HardwareSetupXML::STAGE_Z_TAG = "ZStage";
const char * const HardwareSetupXML::STAGE_Z2_TAG = "ZStage2";
const char * const HardwareSetupXML::BEAM_EXPANDER_TAG = "BeamExpander";
const char * const HardwareSetupXML::MCLS_TAG = "MCLS";
const char * const HardwareSetupXML::PINHOLE_WHEEL_TAG = "PinholeWheel";
const char * const HardwareSetupXML::POWER_REG_TAG = "PowerRegulator";
const char * const HardwareSetupXML::STAGE_XY_TAG = "XYStage";
const char * const HardwareSetupXML::SHUTTER_TAG = "Shutter";
const char * const HardwareSetupXML::LIGHT_PATH_TAG = "LightPath";
const char * const HardwareSetupXML::SPECTRUM_FILTER_TAG = "SpectrumFilter";
const char * const HardwareSetupXML::TURRET_TAG = "Turret";
const char * const HardwareSetupXML::SLM_TAG = "SLM";
const char * const HardwareSetupXML::PMT_SWITCH_TAG = "PMTSwitch";
const char * const HardwareSetupXML::BEAM_STABILIZER_TAG = "BeamStabilizer";
const char * const HardwareSetupXML::POWER_REG2_TAG = "PowerRegulator2";
const char * const HardwareSetupXML::EPI_TURRET_TAG = "EpiTurret";
const char * const HardwareSetupXML::LAMP_TAG = "LAMP";
const char * const HardwareSetupXML::HARDWARE_ATTR[NUM_HARDWARE_ATTRIBUTES] = {"dllName", "active", "id", "cameraName", "serialNumber", "activation"};
//get the active hardware id in its map
long HardwareSetupXML::GetActiveHardwareID(string tagParent, string tagName, long &id)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActiveHardwareID _xmlObj is null");
		return FALSE;
	}

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		ticpp::Element* parent = configObj->FirstChildElement(tagParent); 

		if(parent->NoChildren())
			return FALSE;

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement(tagName), tagName);

		//find the last child element tag
		for( ; child != child.end(); child++)
		{
			//long active = 0;
			//child->GetAttribute(HARDWARE_ATTR[ACTIVE_ATTR], &active, false);
			string activeStr;
			GetAttribute(child, tagName, HARDWARE_ATTR[ACTIVE_ATTR], activeStr);
			long active = atol(activeStr.c_str());

			if(active == 1)
			{
				//child->GetAttribute(HARDWARE_ATTR[ID_ATTR], &id);
				string idStr;
				GetAttribute(child, tagName, HARDWARE_ATTR[ID_ATTR], idStr);
				id = atol(idStr.c_str());
				return TRUE;
			}
		}
	}
	catch(ticpp::Exception ex)
	{
		const char* msg = ex.what();
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActiveHardwareID an exception has occurred ");
		return FALSE;
	}

	return FALSE;
}


long HardwareSetupXML::GetActiveHardwareDllName(string tagParent, string tagName, string &dllName)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActiveHardwareID _xmlObj is null");
		return FALSE;
	}

	try
	{
		// make sure the top level root element exist
		ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

		ticpp::Element* parent = configObj->FirstChildElement(tagParent); 

		if(parent->NoChildren())
			return FALSE;

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement(tagName), tagName);

		//find the last child element tag
		for( ; child != child.end(); child++)
		{
			string activeStr;
			GetAttribute(child, tagName, HARDWARE_ATTR[ACTIVE_ATTR], activeStr);
			long active = atol(activeStr.c_str());

			if(1 == active)
			{
				GetAttribute(child, tagName, HARDWARE_ATTR[DLLNAME_ATTR], dllName);
				return TRUE;
			}
		}
	}
	catch(ticpp::Exception ex)
	{
		const char* msg = ex.what();
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetActiveHardwareDllName an exception has occurred ");
		return FALSE;
	}

	return FALSE;
}

//------------------------------------------ help functions ------------------------------------------//
long HardwareSetupXML::GetAttribute(ticpp::Iterator<ticpp::Element> child, string tagName, string attribute, string &attributeValue)
{
	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  GetAttribute _xmlObj is null");
		return FALSE;
	}
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
			child->GetAttribute(attribute, &attributeValue);
		}
		catch(ticpp::Exception ex)
		{
			char buf[512];
			WCHAR wbuf[512];
			const char* msg = ex.what();
			StringCbPrintfA(buf,512, "%s=> Tag: %s Attribute: %s", msg, tagName.c_str(), attribute.c_str());
			MultiByteToWideChar(0, 0, buf, -1, wbuf, 512);  
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, wbuf);
		}

		return TRUE;
	}
}

long HardwareSetupXML::SetAttribute(string tagParent, string tagName, string attribute, string attributeValue)
{
	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  SetAttribute _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(tagParent);

		// iterate over to get the particular tag element specified as a parameter(tagName)
		ticpp::Iterator<ticpp::Element> child(parent->FirstChildElement(tagName), tagName);

		//find the last child element tag
		for( ; child->NextSiblingElement(false) != child.end(); child++);

		//set the attribute value for the specified attribute name
		child->SetAttribute(attribute, attributeValue);
		return TRUE;
	}
}

long HardwareSetupXML::CreateTag(string tagParent, string tag)
{
	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  CreateTag _xmlObj is null");
		return FALSE;
	}
	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		//get the attribute value for the specified attribute name
		ticpp::Element * element = new ticpp::Element(tag);

		string str(ROOT_TAG);

		if(tagParent == str)
		{
			configObj->LinkEndChild(element);
		}
		else
		{
			ticpp::Element * parentElement = configObj->FirstChildElement(tagParent,false);

			parentElement->LinkEndChild(element);
		}
		return TRUE;
	}
}

long HardwareSetupXML::Save()
{
	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"HardwareSetupXML  Save _xmlObj is null");
		return FALSE;
	}

	_xmlObj->SaveFile(WStringToString(_currentPathAndFile));

	return TRUE;
}

//remove this tag and all its children
long HardwareSetupXML::RemoveNonRootTag(string tagName)
{
	if(_xmlObj == NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  RemoveWavelength _xmlObj is null");
		return FALSE;
	}

	// make sure the top level root element exist
	ticpp::Element *configObj = _xmlObj->FirstChildElement(false);

	if ( configObj == NULL )
	{
		return FALSE;
	}
	else
	{
		ticpp::Element* parent = configObj->FirstChildElement(tagName, false);

		if(parent == NULL)
			return FALSE;

		parent->Clear(); //remove all children below this element

		configObj->RemoveChild(parent);
	}

	return TRUE;
}

long HardwareSetupXML::PersistHardwareSetup(multimap<long, vector<wstring>>& devInfoMap, multimap<long, vector<wstring>> camInfoMap)
{
	if(FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT,1,L"Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	RemoveNonRootTag(IMAGEDETECTORS_TAG);
	RemoveNonRootTag(DEVICES_TAG);

	CreateTag(ROOT_TAG, IMAGEDETECTORS_TAG);
	CreateTag(ROOT_TAG, DEVICES_TAG);

	multimap<long, vector<wstring>>::iterator it;

	//persist camera information to HardwareSettigns.xml
	for(it = camInfoMap.begin(); it != camInfoMap.end(); it++)
	{
		string parentTag = IMAGEDETECTORS_TAG;
		string childTag;

		switch((*it).first)
		{
		case ICamera::CCD:
			{
				childTag = CAMERA_TAG;
			}
			break;
		case ICamera::LSM:
			{
				childTag = LSM_TAG;
			}
			break;
		}

		CreateTag(parentTag, childTag);

		for(int i=0; i < NUM_HARDWARE_ATTRIBUTES; i++)
		{
			string strAttrValue = ConvertWStringToString((*it).second.at(i));

			//save the attribute value to xml only if it is not empty
			if(!strAttrValue.empty())
			{
				SetAttribute(parentTag, childTag, HARDWARE_ATTR[i], strAttrValue);
			}
		}
	}

	//persist device information to HardwareSettings.xml
	for(it = devInfoMap.begin(); it != devInfoMap.end(); it++)
	{
		string parentTag = DEVICES_TAG;
		string childTag;

		switch((*it).first)
		{
		case IDevice::STAGE_X | IDevice::STAGE_Y:
			{
				childTag = STAGE_XY_TAG;
			}
			break;
		case IDevice::STAGE_Z:
			{
				childTag = STAGE_Z_TAG;
			}
			break;
		case IDevice::STAGE_Z2:
			{
				childTag = STAGE_Z2_TAG;
			}
			break;
		case IDevice::CONTROL_UNIT:
			{
				childTag = CONTROL_UNIT_TAG;
			}
			break;
		case IDevice::PMT1:
			{
				childTag = PMT1_TAG;
			}
			break;
		case IDevice::PMT2:
			{
				childTag = PMT2_TAG;
			}
			break;
		case IDevice::PMT3:
			{
				childTag = PMT3_TAG;
			}
			break;
		case IDevice::PMT4:
			{
				childTag = PMT4_TAG;
			}
			break;
		case IDevice::BEAM_EXPANDER:
			{
				childTag = BEAM_EXPANDER_TAG;
			}
			break;
		case IDevice::POWER_REG:
			{
				childTag = POWER_REG_TAG;
			}
			break;
		case IDevice::PINHOLE_WHEEL:
			{
				childTag = PINHOLE_WHEEL_TAG;
			}
			break;
		case IDevice::LASER1:
			{
				childTag = MCLS_TAG;
			}
			break;
		case IDevice::SHUTTER:
			{
				childTag = SHUTTER_TAG;
			}
			break;
		case IDevice::LIGHT_PATH:
			{
				childTag = LIGHT_PATH_TAG;
			}
			break;
		case IDevice::SPECTRUM_FILTER:
			{
				childTag = SPECTRUM_FILTER_TAG;
			}
			break;
		case IDevice::TURRET:
			{
				childTag = TURRET_TAG;
			}
			break;
		case IDevice::SLM:
			{
				childTag = SLM_TAG;
			}
			break;
		case IDevice::PMT_SWITCH:
			{
				childTag = PMT_SWITCH_TAG;
			}
			break;
		case IDevice::BEAM_STABILIZER:
			{
				childTag = BEAM_STABILIZER_TAG;
			}
			break;
		case IDevice::POWER_REG2:
			{
				childTag = POWER_REG2_TAG;
			}
			break;
		case IDevice::FILTER_WHEEL_DIC:
			{
				childTag = EPI_TURRET_TAG;
			}
			break;
		case IDevice::LAMP:
			{
				childTag = LAMP_TAG;
			}
			break;
		default:
			childTag.clear();
			break;
		}

		if(childTag.empty())
			continue;

		CreateTag(parentTag, childTag);

		for(int i=0; i< NUM_DEVICE_ATTRIBUTES; i++)
		{
			string strAttrValue = ConvertWStringToString((*it).second.at(i));
			SetAttribute(parentTag, childTag, HARDWARE_ATTR[i], strAttrValue);
		}
	}

	Save();
	return TRUE;
}

long HardwareSetupXML::SetFastLoad(long val)
{
	_fastLoad = val;

	return TRUE;
}

const char* const HardwareSetupXML::INVERTED = "Inverted";
const char* const HardwareSetupXML::INVERTED_ATTR[NUM_INVERTED_ATTRIBUTES] = { "SafetyInterlockCheck" };
long HardwareSetupXML::GetInvertedSettings(long& safetyInterlockCheckEnabled)
{
	if (FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetInvertedSettings -> Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetInvertedSettings -> configObj == NULL, top level root element of HardwareSettings.xml doesn't exist.");
		return FALSE;
	}
	else
	{
		ticpp::Element* child = configObj->FirstChildElement(INVERTED, false);
		if (NULL == child)
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetInvertedSettings -> child == NULL, Inverted tag doesn't exist in HardwareSettings.xml.");
			return FALSE;
		}

		string str;
		wstring str2;
		for (long attCount = 0; attCount < NUM_INVERTED_ATTRIBUTES; attCount++)
		{
			str.clear();
			GetAttribute(child, INVERTED, INVERTED_ATTR[attCount], str);
			stringstream ss(str);
			switch (attCount)
			{
			case 0:
				ss >> safetyInterlockCheckEnabled;
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}

const char* const HardwareSetupXML::SHUTTER = "Shutter";
const char* const HardwareSetupXML::SHUTTER_ATTR[NUM_SHUTTER_ATTRIBUTES] = { "OpenShutterInSimulusMode" };
long HardwareSetupXML::GetShutterOptions(long& openShutterInSimulusMode)
{
	if (FALSE == GetCurrentPath())
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetShutterOptions -> Failed to open HardwareSetttings.xml file");
		return FALSE;
	}

	OpenConfigFile(WStringToString(_currentPathAndFile));

	// make sure the top level root element exist
	ticpp::Element* configObj = _xmlObj->FirstChildElement(false);

	if (configObj == NULL)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetShutterOptions -> configObj == NULL, top level root element of HardwareSettings.xml doesn't exist.");
		return FALSE;
	}
	else
	{
		ticpp::Element* child = configObj->FirstChildElement(SHUTTER, false);
		if (NULL == child)
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1, L"HardwareSetupXML GetShutterOptions -> child == NULL, Shutter tag doesn't exist in HardwareSettings.xml.");
			return FALSE;
		}

		string str;
		wstring str2;
		for (long attCount = 0; attCount < NUM_SHUTTER_ATTRIBUTES; attCount++)
		{
			str.clear();
			GetAttribute(child, SHUTTER, SHUTTER_ATTR[attCount], str);
			stringstream ss(str);
			switch (attCount)
			{
			case 0:
				ss >> openShutterInSimulusMode;
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}
#pragma once

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif

#include <string>
#include <map>
#include "..\..\Tools\ticpp\ticpp.h"
#include "..\..\Tools\ticpp\tinyxml.h"
#include "..\..\Tools\ticpp\ticpprc.h"
#include "..\Log.h"
#include "..\..\Common\THORSHAREDTYPESCPP.H"
using namespace std;

extern std::auto_ptr<LogDll> logDll;
extern wchar_t message[256];

class HardwareSetupXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long _filePathChanged;
	long _fastLoad;

public:
	static const char * const ROOT_TAG;
	 
	 static const char * const WAVELENGTHS;
	 static const char * const WAVELENGTH;
	 static const char * const OBJECTIVES;
	 static const char * const OBJECTIVE;
	 static const char * const COLORCHANNELS;
	 static const char * const COLORCHANNELRED;
	 static const char * const COLORCHANNELGREEN;
	 static const char * const COLORCHANNELBLUE;
	 static const char * const COLORCHANNELCYAN;
	 static const char * const COLORCHANNELMAGENTA;
	 static const char * const COLORCHANNELYELLOW;
	 static const char * const COLORCHANNELGRAY;	
	 static const char * const STREAMING;
	 static const char * const LSM;
	 static const char * const DEVICES_TAG;
	 static const char * const IMAGEDETECTORS_TAG;
	 static const char * const LSM_TAG;
	 static const char * const CAMERA_TAG;
	 static const char * const CONTROL_UNIT_TAG;
	 static const char * const PMT1_TAG;
	 static const char * const PMT2_TAG;
	 static const char * const PMT3_TAG;
	 static const char * const PMT4_TAG;
	 static const char * const STAGE_Z_TAG;
	 static const char * const STAGE_Z2_TAG;
	 static const char * const STAGE_XY_TAG;
	 static const char * const PINHOLE_WHEEL_TAG;
	 static const char * const MCLS_TAG;
	 static const char * const BEAM_EXPANDER_TAG;
	 static const char * const POWER_REG_TAG;
	 static const char * const POWER_REG2_TAG;
	 static const char * const EPI_TURRET_TAG;
	 static const char * const SHUTTER_TAG;
	 static const char * const LIGHT_PATH_TAG;
	 static const char * const SPECTRUM_FILTER_TAG;
	 static const char * const TURRET_TAG;
	 static const char * const SLM_TAG;
	 static const char * const PMT_SWITCH_TAG;
	 static const char * const BEAM_STABILIZER_TAG;
	 static const char * const LAMP_TAG;
	 static const char* const INVERTED;
	 enum {NUM_WAVELENGTH_ATTRIBUTES = 9};
	 enum {NUM_OBJECTIVE_ATTRIBUTES = 14};
	 enum {NUM_COLORCHANNEL_ATTRIBUTES = 1};
	 enum {NUM_STREAMING_ATTRIBUTES = 2};
	 enum {NUM_LSM_ATTRIBUTES = 1};
	 enum {NUM_HARDWARE_ATTRIBUTES = 6};
	 enum { NUM_INVERTED_ATTRIBUTES = 1 };
	 enum {NUM_DEVICE_ATTRIBUTES = 3};
	 enum 
	 {
		 DLLNAME_ATTR = 0, 
		 ACTIVE_ATTR, 
		 ID_ATTR,
		 NAME_ATTR, 
		 SERIAL_NUMBER_ATTR,
		 ACTIVATION_ATTR,
	 };
	 static const char * const WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES];
	 static const char * const OBJECTIVE_ATTR[NUM_OBJECTIVE_ATTRIBUTES];
	 static const char * const COLORCHANNEL_ATTR[NUM_COLORCHANNEL_ATTRIBUTES];
	 static const char * const STREAMING_ATTR[NUM_STREAMING_ATTRIBUTES];
	 static const char * const LSM_ATTR[NUM_LSM_ATTRIBUTES];
	 static const char * const HARDWARE_ATTR[NUM_HARDWARE_ATTRIBUTES];
	 static const char* const INVERTED_ATTR[NUM_INVERTED_ATTRIBUTES];

	HardwareSetupXML();
	~HardwareSetupXML();

	long GetWavelength(string name, double &ex, double &em, double &dic,long &fluor, string &color, long &bp, long &wp);///wavelength details
	long GetWavelengthIndex(string name, long &index);
	long GetWavelengthName(long index, string &name);
	long GetMagInfoFromMagVal(double mag, string &name, long &position, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double& finePercentage);///magnification details from magnification value
	long GetMagInfoFromPosition(long position,  string &name, double &mag, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double& finePercentage);//magnification details from position index
	long GetMagInfoFromName(string name, double &mag, long &position, double &numAper, double &afScanStart,double &afFocusOffset, double &afAdaptiveOffset,long &beamExpPos,long &beamExpWavelength,long &beamExpPos2, long &beamExpWavelength2,long &turretPosition,long &zAxisToEscape, double &zAxisEscapeDistance, double &finePercentage);
	long GetColorChannels(string &nameRed, string &nameGreen,string &nameBlue,string &nameCyan, string &nameMagenta, string &nameYellow,string &nameGray);///color channel assignment details
	long GetStreaming(wstring &path, double &previewRate);
	long GetLSM(double &fieldSizeCalibration);
	long OpenConfigFile(string path);

	long GetCurrentPath();

	long GetAttribute(ticpp::Iterator<ticpp::Element> child, string tagName, string attribute, string &attributeValue);	// arguments: xmlChild, tagName, attributeName, attributrReturn;
	long SetAttribute(string tagParent, string tagName, string attribute, string attributeValue);
	long CreateTag(string parentTag, string tag);
	long Save();
	long PersistHardwareSetup(multimap<long, vector<wstring>>& devNameMap, multimap<long, vector<wstring>> cameraNameMap);
	long RemoveNonRootTag(string tagName);
	long GetActiveHardwareID(string tagParent, string tagName, long &id);
	long GetActiveHardwareDllName(string tagParent, string tagName, string &dllName);
	long GetActivationCameraID(long &id);
	long SetFastLoad(long val);///<if fast load is enabled the OpenConfigFile function will be bypassed if the file was previously loaded
	long GetInvertedSettings(long& safetyInterlockCheckEnabled);
};
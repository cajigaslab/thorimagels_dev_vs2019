#pragma once
#include ".\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <regex>
#include <windows.h>
#include "EnumString.h"

using namespace std;

#define HALF_CIRCLE					180.0
#define PI							3.1415926535897932384626433832795
#define MAX_AO_VOLTAGE				10.0
#define MIN_AO_VOLTAGE				-10.0
#define MAX_CHANNEL_COUNT			4
#define MAX_IMAGE_SIZE				2147483648							//2GB
#define BUFFER_LENGTH				255
#define MAX_DIGITAL_SWITCHES		8
#define US_TO_S						1000000.0 //microseconds to seconds conversion

#define SAFE_MEMCPY(x,y,z) if((x != NULL) && (z != NULL) && (0 < y)) { memcpy_s(x,y,z,y); }
#define SAFE_DELETE_MEMORY(x) if (NULL != x) { free((void*)x); x = NULL; }
#define SAFE_DELETE_ARRAY(x) if (x != NULL) { delete[] x; x = NULL; }
#define SAFE_DELETE_HANDLE(x) if(NULL != x) { CloseHandle(x); x = NULL;}
#define SAFE_DELETE_PTR(x) if (x != NULL) { delete x; x = NULL; }

// *********************************************************************** //
// Enum Strings: allow convert enum in SharedEnums to string or vice versa //
// *********************************************************************** //
Begin_Enum_String(BLEACHSCAN_DIGITAL_LINENAME)
{
	Enum_String(DUMMY);
	Enum_String(POCKEL_DIG);
	Enum_String(ACTIVE_ENVELOPE);
	Enum_String(CYCLE_COMPLETE);
	Enum_String(CYCLE_ENVELOPE);
	Enum_String(ITERATION_ENVELOPE);
	Enum_String(PATTERN_TRIGGER);
	Enum_String(PATTERN_COMPLETE);
	Enum_String(EPOCH_ENVELOPE);
	Enum_String(CYCLE_COMPLEMENTARY);
	Enum_String(POCKEL_DIG_1);
	Enum_String(POCKEL_DIG_2);
	Enum_String(POCKEL_DIG_3);
	Enum_String(DIGITAL_LINENAME_LAST);
}End_Enum_String;

// *************************************** //
// shared template structs & functions     //
// *************************************** //

template < class T > inline long CheckNewValue(T& val, T newVal) { if (val != newVal) { val = newVal; return TRUE; } else { return FALSE; } }

extern "C" _declspec(dllexport) typedef struct GGalvoWaveformParams
{
	unsigned long long ClockRate;
	unsigned long long analogXYSize;
	unsigned long long analogPockelSize;
	unsigned long long digitalSize;
	unsigned long long analogZSize;
	double stepVolt;
	unsigned char pockelsCount;
	unsigned char driverType;
	double* GalvoWaveformXY;
	double* GalvoWaveformPockel;
	unsigned char* DigBufWaveform;
	double* PiezoWaveformZ;
	long digitalLineCnt;
	long Scanmode;
	long Triggermode;
	long CycleNum;
	HANDLE bufferHandle;
	long lastLoaded;
	long PreCapStatus;											//sync above order with the one defined at C#
	unsigned long long unitSize[SignalType::SIGNALTYPE_LAST];	//unit data length for each SignalType, only used at C++
	wchar_t WaveFileName[_MAX_PATH];
}GalvoWaveformParams;

extern "C" _declspec(dllexport) typedef struct ThorDAQGGWaveformParams
{
	unsigned long long ClockRate;
	unsigned long long analogXYSize;
	unsigned long long analogPockelSize;
	unsigned long long digitalSize;
	double stepVolt;
	unsigned char pockelsCount;
	unsigned char driverType;
	unsigned short* GalvoWaveformX;
	unsigned short* GalvoWaveformY;
	unsigned short* GalvoWaveformPockel;
	unsigned short* DigBufWaveform;
	unsigned short GalvoWaveformXOffset;
	unsigned short GalvoWaveformYOffset;
	unsigned short GalvoWaveformPoceklsOffset;
	long digitalLineCnt;
	long Scanmode;
	long Triggermode;
	long CycleNum;
	HANDLE bufferHandle;
	long lastLoaded;
	long PreCapStatus;						//sync above order with the one defined at C#
	unsigned long long unitSize[3];			//unit data length for each SignalType, only used at C++
	wchar_t WaveFileName[_MAX_PATH];
}ThorDAQGGWaveformParams;

extern "C" _declspec(dllexport) typedef struct EPhysTriggerStruct
{
	long	configured;
	long	enable;
	long	mode;
	double	startIdleMS;
	double	durationMS;
	double	idleMS;
	double	minIdleMS;
	long	iterations;
	long	startEdge;
	long	repeats;
	long	framePerZSlice;
	long    clockRateHz;
	long    outputType;
	wchar_t triggerLine[EPHYS_ARRAY_SIZE];
	int		stepEdge[EPHYS_ARRAY_SIZE];
	double  voltageRange[2];
	double  powerPercent[EPHYS_ARRAY_SIZE];
	long	responseType;
}EPhysTrigStruct;

extern "C" _declspec(dllexport) typedef struct FrameInfoStruct
{
	long	 imageWidth;
	long	 imageHeight;
	long	 channels;
	long	 fullFrame;
	long	 isMROI;
	long	 scanAreaID;
	long	 scanAreaIndex;
	long	 totalScanAreas;
	long	 isNewMROIFrame;
	long	 fullImageWidth;
	long     fullImageHeight;
	long     topInFullImage;
	long     leftInFullImage;
	long	 mROIStripeFieldSize;
	long	 bufferType;
	unsigned long long copySize;
	long	 numberOfPlanes;
	long	 polarImageType;
	long	 totalSequences;
	long	 sequenceIndex;
	long	 sequenceSelectedChannels;
	long     pixelAspectRatioYScale;
}FrameInfo;

extern "C" _declspec(dllexport) typedef struct CaptureNotificationStruct
{
	long isAsyncParamUpdate;
}CaptureNotification;

struct WaveformGenParams
{
	double	galvoRetraceTime;
	double	dwellTime;
	long	PixelX;
	long	PixelY;
	long	areaMode;
	long	fieldSize;
	double	fieldScaleFineX;
	double	fieldScaleFineY;
	long	offsetX;
	long	offsetY;
	double	fineOffset[2];
	long	numFrame;
	double	scaleYScan;
	long	verticalScanDirection;
	double  scanAreaAngle;
	long	pockelsLineEnable[MAX_GG_POCKELS_CELL_COUNT];
	double	pockelsPower[MAX_GG_POCKELS_CELL_COUNT];
	double	pockelsIdlePower[MAX_GG_POCKELS_CELL_COUNT];
	double  pockelsMaxPower[MAX_GG_POCKELS_CELL_COUNT];
	double  pockelsLineBlankingPercentage[MAX_GG_POCKELS_CELL_COUNT];
	long	flybackCycles;
	long	minLowPoints;
	long	clockRatePockels;
	double  field2Volts;
	long	digLineSelect;
	long	pockelsTurnAroundBlank;
	long	pockelsFlybackBlank;
	long	scanMode;
	double	yAmplitudeScaler;
	long	galvoEnable;
	long	useReferenceForPockelsOutput;
	long	pockelsReferenceRequirementsMet;
	long	scanAreaIndex;
	long	scanAreaCount;
	long	interleaveScan;

};

extern "C" _declspec(dllexport) typedef struct ThorDAQZWaveformParams
{
	double updateRate;
	long framesPerWaveform;
	long samplesPerFrame;
	long waveformLength;
	long waveformChannel;
	double parkPosition;
	double offsetPosition;
	USHORT* waveform;
}ThorDAQZWaveformParams;

static double CalculateMinimumDwellTime(double fieldSize, double fineFieldSizeScale, long pixelX, long turnAroundTimeUS, double field2Theta, long maxGalvoOpticalAngle, double maxAngularVelocityRadSecSq, double maxAngularAccelerationRadSecSq, double minDwellTimeUS, double maxDwellTimeUS, double dwellTimeIncrementUS)
{
	//peak velocity = amplitude * 2π * Freq
	//peak acceleration = amplitude * (2π * Freq)^2

	/*
		converion(V / s to rad / s)
		escale = 20; %% volts peak to peak full scale
		mscale = 15; %% mechanical degrees full scale
		volts2deg = mscale / escale; volts to degrees
	*/

	double accelerationMinDwellUS = minDwellTimeUS - dwellTimeIncrementUS; //us
	double peakAccelerationDegPerSecSq = 0;
	const double escale = 20;// volts peak - peak full scale
	const double mscale = 15;// mechanical degrees full scale
	const double volts2deg = mscale / escale;// volts to Degrees;

	const double linearPortionAmplitudeDeg = fieldSize * fineFieldSizeScale * field2Theta * volts2deg;

	const double freq = US_TO_SEC / (2 * turnAroundTimeUS);
	do
	{
		accelerationMinDwellUS += dwellTimeIncrementUS;
		if (accelerationMinDwellUS > maxDwellTimeUS)
		{
			return accelerationMinDwellUS;
		}
		double linearPortionTimeUS = accelerationMinDwellUS * pixelX;

		double accelerationPortionAmplitudeDeg = linearPortionAmplitudeDeg * ((double)turnAroundTimeUS / linearPortionTimeUS) / PI;

		peakAccelerationDegPerSecSq = accelerationPortionAmplitudeDeg * pow((2 * PI * freq), 2);

	} while (peakAccelerationDegPerSecSq > (maxAngularAccelerationRadSecSq * 180 / PI));

	double velocityMinDwellUS = dwellTimeIncrementUS; //us

	double  peakVelocityDegPerSec = 0;

	do
	{
		velocityMinDwellUS += dwellTimeIncrementUS;
		if (velocityMinDwellUS > maxDwellTimeUS)
		{
			return accelerationMinDwellUS;
		}
		double linearPortionTimeUS = velocityMinDwellUS * pixelX;
		peakVelocityDegPerSec = (linearPortionAmplitudeDeg / (linearPortionTimeUS / US_TO_S)) / PI / 2;

	} while (peakVelocityDegPerSec > (maxAngularVelocityRadSecSq * 180 / PI));

	return velocityMinDwellUS > accelerationMinDwellUS ? velocityMinDwellUS : accelerationMinDwellUS;
}

static long CalculateMaxFieldSize(double dwellTime, long pixelX, long turnAroundTimeUS, double field2Theta, long maxGalvoOpticalAngle, double maxAngularVelocity, double maxAngularAcceleration)
{
	long maxFieldSize = static_cast<long>((dwellTime * PI * maxGalvoOpticalAngle * pixelX) / (field2Theta * ((4.0 * ((double)turnAroundTimeUS / 2.0)) + (dwellTime * pixelX * PI))));
	return maxFieldSize;
}

static string removeSpaces(string& str)
{
	int count = 0;
	for (int i = 0; str[i]; i++)
		if (str[i] != ' ')
			str[count++] = str[i];
	str[count] = '\0';
	return str;
}

static string ConvertWStringToString(wstring ws)
{
	size_t origsize = wcslen(ws.c_str()) + 1;
	const size_t newsize = _MAX_PATH;
	size_t convertedChars = 0;
	char nstring[newsize];
	wcstombs_s(&convertedChars, nstring, origsize, ws.c_str(), _TRUNCATE);

	string str(nstring);

	return str;
}

static vector<string> FindSerialNumbersInRegistry(string VID, string PID)
{
	wchar_t data[BUFFER_LENGTH];
	HKEY hk;
	DWORD count = 0;
	DWORD sz = sizeof(DWORD);
	vector<string> serialNumbers;
	string dataString;
	string format = "VID_" + VID + ".*&.*PID_" + PID + "\\\\";
	regex reg(format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\servies\usbser as hk
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\usbser", 0, KEY_READ, &hk))
	{
		// No usbSer device is connected, return empty list (size=0).
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected usbser devices 
	if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No usbSer device is connected, return empty list (size=0).
		return serialNumbers;
	}
	// Iterate through the parameters, one for each connected device
	for (DWORD i = 0; i < count; i++)
	{
		DWORD cbData = BUFFER_LENGTH;
		wchar_t usbConnectedIndex[BUFFER_LENGTH];
		// Use i as the parameter name, the parameter name for each device is their index number
		swprintf_s(usbConnectedIndex, BUFFER_LENGTH, L"%d", i);
		// Get the data associated with each device connected inside the Enum key
		if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", usbConnectedIndex, RRF_RT_REG_SZ, NULL, (LPBYTE)data, &cbData))
			continue;
		// Convert the returned wstring data to string for regex search
		dataString = ConvertWStringToString(data);
		// search for the regular expression VID/PID in 'data' using this format 'VID_####&PID_####\'
		if (regex_search(dataString.c_str(), matchedResult, reg))
		{
			// If regex search matched, store the substring after the regex format. This is the 17-digit serial number of the device.
			serialNumbers.push_back(matchedResult[0].second);
		}
	}
	return serialNumbers;
}

static wstring FindCOMPortInRegistry(string VID, string PID, string serialNum)
{
	HKEY hk;
	HKEY hSubKey;
	HKEY deviceSubKey;
	HKEY parameterSubKey;
	wstring comPort = L"";
	wstring path;
	string deviceNames;
	string devicesPidVidString;
	string format = "VID_" + VID + ".*&.*PID_" + PID;
	regex reg(format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\Enum as hk
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &hk) != ERROR_SUCCESS)
		return comPort;
	// For every subfolder in the key generate a new key (SubKeyName) and open the subfolder
	for (DWORD i = 0; ; i++)
	{
		DWORD cName = BUFFER_LENGTH;
		wchar_t SubKeyName[BUFFER_LENGTH];
		// Get the name of the ith subkey
		if (ERROR_SUCCESS != RegEnumKeyEx(hk, i, SubKeyName, &cName, NULL, NULL, NULL, NULL))
			break;
		// Set the path to the subkey and open it in hsubkey
		path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName);
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hSubKey))
			break;
		// For every subfolder in the key generate a new key (deviceSubKey) and open the subfolder
		for (DWORD j = 0; ; j++)
		{
			DWORD dName = BUFFER_LENGTH;
			wchar_t devicesPidVid[BUFFER_LENGTH];
			// Get the name of the jth subkey and save it in devicesPidVid
			if (ERROR_SUCCESS != RegEnumKeyEx(hSubKey, j, devicesPidVid, &dName, NULL, NULL, NULL, NULL))
				break;
			// Convert the key name to string for RegEx search
			devicesPidVidString = ConvertWStringToString(devicesPidVid);
			// Compare the key name to the passed VID/PID using this format VID_####&PID_####
			if (regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID folder look at their key name and compare it to the passed serial number, 
				// if it matches grab the COM port number from the PortName field
				for (DWORD k = 0; ; k++)
				{
					DWORD pName = BUFFER_LENGTH;
					wchar_t deviceHID[BUFFER_LENGTH];
					// Get the name of the kth subkey and store it in DeviceHID
					if (ERROR_SUCCESS != RegEnumKeyEx(deviceSubKey, k, deviceHID, &pName, NULL, NULL, NULL, NULL))
						break;
					// Convert the name of the subkey to a string and compare it to the passed serialNum
					deviceNames = ConvertWStringToString(deviceHID);
					if (0 == deviceNames.compare(serialNum))
					{
						DWORD cbData = BUFFER_LENGTH;
						wchar_t value[BUFFER_LENGTH];
						// Set the path to the subkey with the matching serial number and open it in parameterSubKey
						path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid) + L"\\" + wstring(deviceHID);
						if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &parameterSubKey))
							continue;
						// Get the value from the PortName parameter
						if (ERROR_SUCCESS != RegGetValue(parameterSubKey, L"Device Parameters", L"PortName", RRF_RT_REG_SZ, NULL, (LPBYTE)value, &cbData))
							continue;
						comPort = wstring(value);
					}
				}
			}
		}
	}
	return comPort;
}


#pragma once
#include ".\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"
#include <iostream>
#include <string>
#include <windows.h>
#include "EnumString.h"

using namespace std;

#define HALF_CIRCLE					180.0
#define PI							3.1415926535897932384626433832795
#define MAX_AO_VOLTAGE				10.0
#define MIN_AO_VOLTAGE				-10.0
#define MAX_CHANNEL_COUNT			4
#define MAX_IMAGE_SIZE				2147483648							//2GB

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
	wchar_t triggerLine[_MAX_PATH];
	int		stepEdge[_MAX_PATH];
}EPhysTrigStruct;

extern "C" _declspec(dllexport) typedef struct FrameInfoStruct
{
	long	 imageWidth;
	long	 imageHeight;
	long	 channels;
	long	 fullFrame;
	long	 scanAreaID;
	long	 bufferType;
	unsigned long long copySize;
	long numberOfPlanes;
}FrameInfo;

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
	long	scanMode;
	double	yAmplitudeScaler;
	long	galvoEnable;
	long	useReferenceForPockelsOutput;
	long	pockelsReferenceRequirementsMet;
	long	scanAreaIndex;
	long	scanAreaCount;
	long	interleaveScan;

};

static double CalculateMinimumDwellTime(double fieldSize, long pixelX, long turnAroundTimeUS, double field2Theta, long maxGalvoOpticalAngle)
{
	double minDwell = (4.0 * ((double)turnAroundTimeUS / 2.0) * fieldSize * field2Theta) / (PI * pixelX * (maxGalvoOpticalAngle - fieldSize * field2Theta));
	return minDwell;
}

static long CalculateMinimumFieldSize(double dwellTime, long pixelX, long turnAroundTimeUS, double field2Theta, long maxGalvoOpticalAngle)
{
	long minFieldSize = static_cast<long>((dwellTime * PI * maxGalvoOpticalAngle * pixelX) / (field2Theta * ((4.0 * ((double)turnAroundTimeUS / 2.0)) + (dwellTime * pixelX * PI))));
	return minFieldSize;
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

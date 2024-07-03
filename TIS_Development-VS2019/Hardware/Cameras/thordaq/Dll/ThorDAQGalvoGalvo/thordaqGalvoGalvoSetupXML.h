#pragma once
#include "thordaqcmd.h"
class ThorGalvoGalvoXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long SaveConfigFile();
public:
	static const char * const CONFIG;
	enum {NUM_CONFIG_ATTRIBUTES = 21};
	static const char * const CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES];

	static const char* const RGGCONFIG;
	enum { NUM_RGGCONFIG_ATTRIBUTES = 1 };
	static const char* const RGGCONFIG_ATTR[NUM_RGGCONFIG_ATTRIBUTES];

	static const char* const STIM;
	enum { NUM_STIM_ATTRIBUTES = 1 };
	static const char* const STIM_ATTR[NUM_STIM_ATTRIBUTES];

	static const char * const CALIBRATION;
	enum {NUM_CALIBRATION_ATTRIBUTES = 15};
	static const char * const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char * const Stream;
	enum {NUM_STREAM_ATTRIBUTES = 1};
	static const char * const STREAM_ATTR[NUM_STREAM_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 21};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 1};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char * const FIR;
	enum {NUM_FIR_ATTRIBUTES = 32};
	static const char * const FIR_ATTR[NUM_FIR_ATTRIBUTES];

public:
	ThorGalvoGalvoXML();
	~ThorGalvoGalvoXML();

	long GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &findScaleX, double &fineScaleY,long &oneXFieldSize, double(&pockelsMinVoltage)[MAX_POCKELS_CELL_COUNT], double(&pockelsMaxVoltage)[MAX_POCKELS_CELL_COUNT]);
	long SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double findScaleX, double fineScaleY, long oneXFieldSize, double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT], double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT]);

	long GetConfiguration(double &field2Theta, long &pockelsParkAtMinimum, double (&pockelsDelayUS)[MAX_POCKELS_CELL_COUNT], long &galvoParkAtStart,long &useExternalBoxFrequency3P, long &fieldSizeMin, long &fieldSizeMax, long &pockelsTurnAroundBlank, long &pockelsFlybackBlank, long &sumPulsesPerPixel, long &scannerType, double &maxScannerSampleRate, long &enableGalvoXPark, long& limitGalvoSpeed, long& multiplaneBlankLines, long& multiplaneBlankLinesInLiveModeOnly, double& maxAngularVelocityRadPerSec, double& maxAngularAccelerationRadPerSecSq);
	long SetConfiguration(double pockelsDelayUS[MAX_POCKELS_CELL_COUNT]);

	long GetRGGConfiguration(long& rggMode);

	long GetStreamConfiguration(long &samplerate);

	long GetStimConfiguration(long& activeLoadCount);

	long GetIO(double &pockelsVoltageSlopeThreshold, long &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, long &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, long &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2,long &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2,long &pockelsResponseType3);
	long GetTrigger(long& waitTime);

	long GetFIR(double (&firFilter)[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT]);
	long SetFIR(double firFilter[FIR_FILTER_COUNT][MAX_CHANNEL_COUNT][FIR_FILTER_TAP_COUNT]);
};
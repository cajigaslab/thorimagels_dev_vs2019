#pragma once
#ifndef CONST_H
#include "const.h"
#endif

#define C MAX_CHANNEL_COUNT
class ThorGalvoGalvoXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	
	long SaveConfigFile();
public:
	static const char * const CONFIG;
	enum {NUM_CONFIG_ATTRIBUTES = 4};
	static const char * const CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES];

	static const char * const CALIBRATION;
	enum {NUM_CALIBRATION_ATTRIBUTES = 7};
	static const char * const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char * const Stream;
	enum {NUM_STREAM_ATTRIBUTES = 5};
	static const char * const STREAM_ATTR[NUM_STREAM_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 22};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 1};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char * const CHANNEL_SETTINGS[C];
	enum {NUM_CHANNEL_SETTINGS_ATTRIBUTES = 9};
	static const char * const CHANNEL_SETTINGS_ATTR[NUM_CHANNEL_SETTINGS_ATTRIBUTES];

	static const char * const FRONTEND;
	enum {NUM_FRONTEND_ATTRIBUTES = 3};
	static const char * const FRONTEND_ATTR[NUM_FRONTEND_ATTRIBUTES];

public:
	ThorGalvoGalvoXML();
	~ThorGalvoGalvoXML();

	long GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &findScaleX, double &fineScaleY,long &oneXFieldSize);
	long SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double findScaleX, double fineScaleY, long oneXFieldSize);
   
	//enableIntAdj="1" threshold="22" baselineTolerance="13" maxLevel0="60" maxLevel1="134"
	long GetChannelSettings(long (&fineShiftA)[C],long (&fineShiftB)[C],long (&coarseShiftA)[C],long (&coarseShiftB)[C],long (&enableIntAdj)[C],long (&threshold)[C],
		long (&baselineTolerance)[C],long (&maxLevel0)[C],long (&maxLevel1)[C]);
	long SetChannelSettings(long (fineShiftA)[C],long (fineShiftB)[C],long (coarseShiftA)[C],long (coarseShiftB)[C],long (enableIntAdj)[C],long (threshold)[C],
		long (baselineTolerance)[C],long (maxLevel0)[C],long (maxLevel1)[C]);

	long GetFrontEndTuning(long &syncDelay, long &resyncDelay, long &forceSyncEveryLine);
	long SetFrontEndTuning(long syncDelay, long resyncDelay, long forceSyncEveryLine);

	long GetConfiguration(double &field2Theta, long &pockelsParkAtMinimum, long &pockelsDelayUS, long &acquisitionMode);

	long GetStreamConfiguration(long &samplerate, long &FIRFilter1, long &FIRFilter2, double &DCOffset1, double &DCOffset2);

	long GetIO(double &pockelsVoltageSlopeThreshold, long &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, long &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, long &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2,long &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, string &pockelsReferenceLine, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2,long &pockelsResponseType3);

	long GetTrigger(long &waitTime);
};
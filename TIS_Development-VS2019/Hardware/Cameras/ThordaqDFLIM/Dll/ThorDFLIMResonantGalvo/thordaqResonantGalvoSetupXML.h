#pragma once

class ThordaqResonantGalvoXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:
	static const char * const CONFIG;
	enum {NUM_CONFIG_ATTRIBUTES = 3};
	static const char * const CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES];

	static const char * const CALIBRATION;
	enum {NUM_CALIBRATION_ATTRIBUTES = 4};
	static const char * const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char * const Stream;
	enum {NUM_STREAM_ATTRIBUTES = 2};
	static const char * const STREAM_ATTR[NUM_STREAM_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 17};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 1};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];
public:
	ThordaqResonantGalvoXML();
	~ThordaqResonantGalvoXML();

	long GetCalibration(double &fieldSizeCalibration,long &oneXFieldSize,double &pockelsPhaseAdjustMicroSec, double &pockelsMaskPhaseShiftPercent);
 
	long GetIO(double &pockelsVoltageSlopeThreshold, string &pockelDigOutput, string &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, string &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1, string &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2, long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2);

	long GetConfiguration(double &field2Theta,double &crsFrequency, long &pockelsParkAtMinimum);

	long GetStreamConfiguration(double &DCOffset1, double &DCOffset2);

	long GetTrigger(long &waitTime);
};
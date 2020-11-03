#pragma once

class ThorGGNIXML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const CONFIG;
	enum {NUM_CONFIG_ATTRIBUTES = 14};
	static const char * const CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES];

	static const char * const DMA;
	enum {NUM_DMA_ATTRIBUTES = 4};
	static const char * const DMA_ATTR[NUM_DMA_ATTRIBUTES];

	static const char * const IO;
	enum {NUM_IO_ATTRIBUTES = 27};
	static const char * const IO_ATTR[NUM_IO_ATTRIBUTES];

	static const char * const CALIBRATION;
	enum {NUM_CALIBRATION_ATTRIBUTES = 12};
	static const char * const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char * const POLARITY;
	enum {NUM_POLARITY_ATTRIBUTES = 4};
	static const char * const POLARITY_ATTR[NUM_POLARITY_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 7};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char * const WAVEFORM;
	enum {NUM_WAVEFORM_ATTRIBUTES = 9};
	static const char * const WAVEFORM_ATTR[NUM_WAVEFORM_ATTRIBUTES];

	ThorGGNIXML();
	~ThorGGNIXML();

	long GetConfiguration(double &field2Theta, long &pockelsParkAtMinimum, long &galvoParkAtStart, long &fieldSizeMin, long &fieldSizeMax, long &pockelsTurnAroundBlank, string &analogCh1, string &analogCh2, double &analogCh1FeedbackRatio, double &analogCh2FeedbackRatio, long &analogXYmode1, double &minGalvoFreqHz1, string &analogCh3, string &analogCh4);
	long GetDMA(long &bufferSize, long &activeLoadCount, long &imageActiveLoadMS, long &imageActiveLoadCount);
	long GetIO(string &pockelsCounterInternal, string &pockelsCounterOutput, string &pockelsTriggerIn, double &pockelsVoltageSlopeThreshold, 
		string &pockelsLine0, string &pockelsPowerInputLine0, double &pockelsScanVoltageStart0, double &pockelsScanVoltageStop0, double &pockelsMinVoltage, double &pockelsMaxVoltage,
		string &pockelsLine1, string &pockelsPowerInputLine1, double &pockelsScanVoltageStart1, double &pockelsScanVoltageStop1,
		string &pockelsLine2, string &pockelsPowerInputLine2, double &pockelsScanVoltageStart2, double &pockelsScanVoltageStop2,
		string &pockelsLine3, string &pockelsPowerInputLine3, double &pockelsScanVoltageStart3, double &pockelsScanVoltageStop3, string &pockelsReferenceLine,
		long &pockelsResponseType0, long &pockelsResponseType1, long &pockelsResponseType2, long &pockelsResponseType3);
	long SetIO(double pockelsMinVoltage,double pockelsMaxVoltage);

	long GetCalibration(double &fieldSizeCalibration,long &flipVerticalScan, double &fineOffsetX, double &fineOffsetY, double &findScaleX, double &fineScaleY,long &oneXFieldSize, double &maxGalvoOpticalAngle, double &minSignalInputVoltage, double &maxSignalInputVoltage, double &galvoRetraceTimeUS, double &pockelsPhaseAdjustUS);
	long SetCalibration(double fieldSizeCalibration,long flipVerticalScan, double fineOffsetX, double fineOffsetY, double findScaleX, double fineScaleY, long oneXFieldSize, double maxGalvoOpticalAngle, double minSignalInputVoltage, double maxSignalInputVoltage, double galvoRetraceTimeUS, double pockelsPhaseAdjustUS);

	long GetPolarity(long &chanAPol, long &chanBPol, long &chanCPol, long &chanDPol);
	long GetTrigger(long &waitTime, string &bufferReadyOutput, string &captureActiveOutput, long &captureActiveOutputInvert, string &stimulationShutter, long &shutterPreIdleMS, long &shutterPostIdleMS);
	long GetWaveform(string &PockelDigOutput, string &CompleteOutput, string &cycleOutput, string &iterationOutput, string &patternOutput, string &patternCompleteOutput, string &activeOutput, string &epochOutput, string &cycleInverse);

	long OpenConfigFile(string path);
	long SaveConfigFile();

};
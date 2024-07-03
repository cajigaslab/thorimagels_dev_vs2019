#pragma once

class ThordaqResonantGalvoXML
{
private:
	ticpp::Document* _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];

public:
	static const char* const CONFIG;
	enum { NUM_CONFIG_ATTRIBUTES = 13 };
	static const char* const CONFIG_ATTR[NUM_CONFIG_ATTRIBUTES];

	static const char* const CALIBRATION;
	enum { NUM_CALIBRATION_ATTRIBUTES = 22 };
	static const char* const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char* const IO;
	enum { NUM_IO_ATTRIBUTES = 21 };
	static const char* const IO_ATTR[NUM_IO_ATTRIBUTES];

	static const char* const TRIGGER;
	enum { NUM_TRIGGER_ATTRIBUTES = 1 };
	static const char* const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char* const HARDWARE_TEST_MODE;
	enum { NUM_HARDWARE_TEST_MODE_ATTRIBUTES = 1 };
	static const char* const HARDWARE_TEST_MODE_ATTR[NUM_HARDWARE_TEST_MODE_ATTRIBUTES];
public:
	ThordaqResonantGalvoXML();
	~ThordaqResonantGalvoXML();

	long GetCalibration(double& fieldSizeCalibration, long& oneXFieldSize, double(&pockelsDelayUS)[MAX_POCKELS_CELL_COUNT], double& pockelsMaskPhaseShiftPercent, long& preImagingCalibrationCycles, long &imagingRampExtensionCycles, double& minFlybackCyclesFactor, double(&pockelsMinVoltage)[MAX_POCKELS_CELL_COUNT], double(&pockelsMaxVoltage)[MAX_POCKELS_CELL_COUNT], double &scanLensFocalLength, double &xHalfAngleMax, double &yHalfAngleMax, double &galvoTiltAngle);

	long GetIO(double& pockelsVoltageSlopeThreshold, string& pockelsLine0, string& pockelsPowerInputLine0, double& pockelsScanVoltageStart0, double& pockelsScanVoltageStop0, string& pockelsLine1, string& pockelsPowerInputLine1, double& pockelsScanVoltageStart1, double& pockelsScanVoltageStop1, string& pockelsLine2, string& pockelsPowerInputLine2, double& pockelsScanVoltageStart2, double& pockelsScanVoltageStop2, string& pockelsLine3, string& pockelsPowerInputLine3, double& pockelsScanVoltageStart3, double& pockelsScanVoltageStop3, long& pockelsResponseType0, long& pockelsResponseType1, long& pockelsResponseType2, long& pockelsResponseType3);

	long GetConfiguration(double& field2Theta, double& crsFrequency, long& saveCrsFrequencyToLog, long& pockelsParkAtMinimum, long& fieldSizeMin, long& fieldSizeMax, long& pockelsTurnAroundBlank, long& pockelsFlybackBlank, long& RGGMode, long& rotationPosition, long& preMoveGalvotoStart, double& waveformUpdateRateSPS, long& scannerType);

	long GetHardwareTestMode(long& enable);

	long GetTrigger(long& waitTime);

	long SetCalibration(double pockelsDelayUS[MAX_POCKELS_CELL_COUNT], double pockelsMaskPhaseShiftPercent, long preImagingCalibrationCycles, long imagingRampExtensionCycles, double pockelsMinVoltage[MAX_POCKELS_CELL_COUNT], double pockelsMaxVoltage[MAX_POCKELS_CELL_COUNT], double xHalfAngleMax, double yHalfAngleMax, double galvoTiltAngle);
};
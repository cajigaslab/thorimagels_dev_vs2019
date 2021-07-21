#pragma once

class ThorSLMPDM512XML
{
private:
	ticpp::Document * _xmlObj;
	wchar_t _currentPathAndFile[_MAX_PATH];	

public:

	static const char * const POSTTRANSFORM;
	static const char * const POSTTRANSFORM2;
	enum {NUM_POSTTRANSFORM_ATTRIBUTES = 6};
	static const char * const POSTTRANSFORM_ATTR[NUM_POSTTRANSFORM_ATTRIBUTES];

	static const char * const CALIBRATION;
	static const char * const CALIBRATION2;
	enum {NUM_CALIBRATION_ATTRIBUTES = 10};
	static const char * const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char * const SPEC;
	enum {NUM_SPEC_ATTRIBUTES = 12};
	static const char * const SPEC_ATTR[NUM_SPEC_ATTRIBUTES];

	static const char * const TRIGGER;
	enum {NUM_TRIGGER_ATTRIBUTES = 2};
	static const char * const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	ThorSLMPDM512XML();
	~ThorSLMPDM512XML();

	long GetPostTransform(int id, long &verticalFlip, double &rotateAngle, double &scaleX, double &scaleY, long &offsetX, long &offsetY);
	long SetPostTransform(int id, long verticalFlip, double rotateAngle, double scaleX, double scaleY, long offsetX, long offsetY);

	long GetCalibration(int id, double& wavelengthNM, long &phaseMax, double &affineCoeff1, double &affineCoeff2, double &affineCoeff3, double &affineCoeff4, double &affineCoeff5, double &affineCoeff6, double &affineCoeff7, double &affineCoeff8);
	long SetCalibration(int id, double affineCoeff1, double affineCoeff2, double affineCoeff3, double affineCoeff4, double affineCoeff5, double affineCoeff6, double affineCoeff7, double affineCoeff8);

	long GetSpec(string &name, long &dmdMode, long &overDrive, unsigned int &transientFrames, double &flatDiagRatio, double&flatPowerMinPercent, double&flatPowerMaxPercent, long &pixelX, long &pixelY,  string &lut, string &odLUT, string &wavefront);
	long GetTrigger(string &counterLine, string &triggerInput);

	long OpenConfigFile(string path);
	long SaveConfigFile();

};
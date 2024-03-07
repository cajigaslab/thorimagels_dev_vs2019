#pragma once

class ThorSLMXML
{
private:
	std::unique_ptr<ticpp::Document> _xmlObj;
	std::string _currentPathAndFile;
	bool _isLoad;
	wchar_t _errMsg[MSG_SIZE];

public:

	static const char* const POSTTRANSFORM;
	static const char* const POSTTRANSFORM2;
	enum { NUM_POSTTRANSFORM_ATTRIBUTES = 6 };
	static const char* const POSTTRANSFORM_ATTR[NUM_POSTTRANSFORM_ATTRIBUTES];

	static const char* const CALIBRATION;
	static const char* const CALIBRATION2;
	const int CALIBRATION_OFFSET_INDEX = 2;
	enum { NUM_CALIBRATION_ATTRIBUTES = 10 };
	static const char* const CALIBRATION_ATTR[NUM_CALIBRATION_ATTRIBUTES];

	static const char* const CALIBRATION3D;
	static const char* const CALIBRATION3D2;
	const int CALIBRATION3D_OFFSET_INDEX = 2;
	enum { NUM_CALIBRATION3D_ATTRIBUTES = 18 };
	static const char* const CALIBRATION3D_ATTR[NUM_CALIBRATION3D_ATTRIBUTES];

	static const char* const SPEC;
	enum { NUM_SPEC_ATTRIBUTES = 13 };
	static const char* const SPEC_ATTR[NUM_SPEC_ATTRIBUTES];

	static const char* const BLANK;
	enum { NUM_BLANK_ATTRIBUTES = 3 };
	static const char* const BLANK_ATTR[NUM_BLANK_ATTRIBUTES];

	static const char* const TRIGGER;
	enum { NUM_TRIGGER_ATTRIBUTES = 2 };
	static const char* const TRIGGER_ATTR[NUM_TRIGGER_ATTRIBUTES];

	static const char* const WINDVI;
	enum { NUM_WINDVI_ATTRIBUTES = 1 };
	static const char* const WINDVI_ATTR[NUM_WINDVI_ATTRIBUTES];

	ThorSLMXML();
	~ThorSLMXML();

	long GetPostTransform(int id, long& verticalFlip, double& rotateAngle, double& scaleX, double& scaleY, long& offsetX, long& offsetY);
	long SetPostTransform(int id, long verticalFlip, double rotateAngle, double scaleX, double scaleY, long offsetX, long offsetY);

	long GetCalibration(int id, double& wavelengthNM, long& phaseMax, double& affineCoeff1, double& affineCoeff2, double& affineCoeff3, double& affineCoeff4, double& affineCoeff5, double& affineCoeff6, double& affineCoeff7, double& affineCoeff8);
	long SetCalibration(int id, double affineCoeff1, double affineCoeff2, double affineCoeff3, double affineCoeff4, double affineCoeff5, double affineCoeff6, double affineCoeff7, double affineCoeff8);
	long GetCalibration3D(int id, double& wavelengthNM, long& phaseMax, double* affineCoeffs, long size);
	long SetCalibration3D(int id, double* affineCoeffs, long size);
	long SetCalibrationAttribute(int mode, int lamdaID, int attrID, double val);

	long GetSpec(string& name, long& dmdMode, long& overDrive, unsigned int& transientFrames, double& pitchUM, double& flatDiagRatio, double& flatPowerMinPercent, double& flatPowerMaxPercent, long& pixelX, long& pixelY, string& lut, string& odLUT, string& wavefront);
	long GetBlank(long& dualPatternShiftPx, long& persistHologramZone1, long& persistHologramZone2);

	long GetTrigger(string& counterLine, string& triggerInput);

	long GetWinDVI(std::wstring& monitorID);

	long GetLastErrorMsg(wchar_t* msg, long size);
	long OpenConfigFile(long forceReload = FALSE);
	long SaveConfigFile();

private:
	void UpdateNode(ticpp::Iterator<ticpp::Element> iElement, const char* const attributes[], std::vector<string> attriValues);
};
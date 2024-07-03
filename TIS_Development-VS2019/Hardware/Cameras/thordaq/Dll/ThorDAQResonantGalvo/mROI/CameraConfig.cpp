#include "stdafx.h"
#include "CameraConfig.h"
#include "..\..\..\..\..\Tools\tinyxml2\include\tinyxml2.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <Shlobj.h>
#include "..\..\..\..\..\Common\ScanManager\scan.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace tinyxml2;

bool CreateFolder(string path)
{
	return CreateDirectoryA(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError();
}
string GetConfigureFilePath()
{
	string configureFilePath = "ThorMesoScanSettings.xml";
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(configureFilePath.c_str()))
		return configureFilePath;

	char buffer[_MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, buffer)))
	{
		string folder = string(buffer) + "\\Thorlabs\\MesoscopeSys\\";
		configureFilePath = folder + "ThorMesoScanSettings.xml";
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(configureFilePath.c_str()))
		{
			string fd = string(buffer) + "\\Thorlabs";
			if (CreateFolder(fd) && CreateFolder(fd + "\\MesoscopeSys"))
			{
				bool exit = false;
				if (!CopyFileA(".\\Modules_Native\\Cameras\\ThorMesoScanSettings.xml",
					configureFilePath.c_str(), exit))
				{
					throw exception("Copy MesoScanSettings Failed!");
				}
			}
			else
			{
				throw exception("Create dictionary for MesoScanSettings Failed!");
			}
		}
	}
	return configureFilePath;
}

CameraConfig::CameraConfig() : 
	PockelDutyCycle(0.8),
	PockelAllowMin(0),
	PockelAllowMax(1),
	PockelMinPercent(0),
	PockelInMax(0.85),
	PockelInMin(0),
	ResonanceFrequency(12001),
	SamplesPerLine(180),
	// FIELD to voltage
	GForX2(-2),
	HForX2(12),
	MinPosX(0),
	MaxPosX(5000),
	MinPosXVoltage(9),
	MaxPosXVoltage(3),
	MinPosY(0),
	MaxPosY(5000),
	MinPosYVoltage(4),
	MaxPosYVoltage(-4),
	MinPosZ(0),
	MaxPosZ(1000),
	MinPosZVoltage(-2.1),
	MaxPosZVoltage(2.1),
	GYFeedbackRatio(0.5),
	GX1FeedbackRatio(0.25),
	GX2FeedbackRatio(0.5),
	VCFeedbackRatio(0.4088),
	GYFeedbackOffset(0.0),
	GX1FeedbackOffset(0.0),
	GX2FeedbackOffset(0.0),
	VCFeedbackOffset(0.0034),
	DelayTimeGy(0.0),
	DelayTimeGx1(0.0),
	DelayTimeGx2(0.0),
	DelayTimeVC(0.0),
	DelayTimePC(0),
	GXExtendTimeStart(0.0),
	GXExtendTimeEnd(0.0),
	GYExtendTimeStart(0.0),
	GYExtendTimeEnd(0.0),
	VCExtendTimeStart(0.0),
	VCExtendTimeEnd(0.0),
	VCSkipLines(54),
	CenterShiftX(0),
	CenterShiftY(0),
	VCPointsPerLine(1),
	GalvoEnable(1),
	ScanMode(ScanMode::TWO_WAY_SCAN),
	AreaMode(ICamera::LSMAreaMode::SQUARE),
	FieldSize(DEFAULT_FIELDSIZE),
	AverageMode(AverageMode::NO_AVERAGE),
	AverageNum(1),
	OneXFieldSize(DEFAULT_FIELDSIZE),
	FieldSizeCalibration(MaxPosX/MAX_FIELDSIZE),
	FrameCount(0)
{
	memset(FieldOffset, 0x0, 2*sizeof(double));
	InitParameterRange();
	LoadFromXMLFile();
}

CameraConfig::CameraConfig(CameraConfig* cfg)
{
	InitParameterRange();
	PockelDutyCycle = cfg->PockelDutyCycle;
	PockelAllowMin = cfg->PockelAllowMin;
	PockelAllowMax = cfg->PockelAllowMax;
	PockelMinPercent = cfg->PockelMinPercent;
	PockelInMax = cfg->PockelInMax;
	PockelInMin = cfg->PockelInMin;
	ResonanceFrequency = cfg->ResonanceFrequency;
	SamplesPerLine = cfg->SamplesPerLine;
	GForX2 = cfg->GForX2;
	HForX2 = cfg->HForX2;
	MinPosX = cfg->MinPosX;
	MaxPosX = cfg->MaxPosX;
	MinPosXVoltage = cfg->MinPosXVoltage;
	MaxPosXVoltage = cfg->MaxPosXVoltage;
	MinPosY = cfg->MinPosY;
	MaxPosY = cfg->MaxPosY;
	MinPosYVoltage = cfg->MinPosYVoltage;
	MaxPosYVoltage = cfg->MaxPosYVoltage;
	MinPosZ = cfg->MinPosZ;
	MaxPosZ = cfg->MaxPosZ;
	MinPosZVoltage = cfg->MinPosZVoltage;
	MaxPosZVoltage = cfg->MaxPosZVoltage;
	GYFeedbackRatio = cfg->GYFeedbackRatio;
	GX1FeedbackRatio = cfg->GX1FeedbackRatio;
	GX2FeedbackRatio = cfg->GX2FeedbackRatio;
	VCFeedbackRatio = cfg->VCFeedbackRatio;
	GYFeedbackOffset = cfg->GYFeedbackOffset;
	GX1FeedbackOffset = cfg->GX1FeedbackOffset;
	GX2FeedbackOffset = cfg->GX2FeedbackOffset;
	VCFeedbackOffset = cfg->VCFeedbackOffset;
	DelayTimeGy = cfg->DelayTimeGy;
	DelayTimeGx1 = cfg->DelayTimeGx1;
	DelayTimeGx2 = cfg->DelayTimeGx2;
	DelayTimeVC = cfg->DelayTimeVC;
	DelayTimePC = cfg->DelayTimePC;
	GXExtendTimeStart = cfg->GXExtendTimeStart;
	GXExtendTimeEnd = cfg->GXExtendTimeEnd;
	GYExtendTimeStart = cfg->GYExtendTimeStart;
	GYExtendTimeEnd = cfg->GYExtendTimeEnd;
	VCExtendTimeStart = cfg->VCExtendTimeStart;
	VCExtendTimeEnd = cfg->VCExtendTimeEnd;
	VCSkipLines = cfg->VCSkipLines;
	CenterShiftX = cfg->CenterShiftX;
	CenterShiftY = cfg->CenterShiftY;
	VCPointsPerLine = cfg->VCPointsPerLine;
	MaxVelocityX = cfg->MaxVelocityX;
	MaxVelocityY = cfg->MaxVelocityY;
	MaxVelocityVC = cfg->MaxVelocityVC;
	MaxOvershootX = cfg->MaxOvershootX;
	MaxOvershootY = cfg->MaxOvershootY;
	MaxOvershootVC = cfg->MaxOvershootVC;
	GalvoEnable = cfg->GalvoEnable;
	ScanMode = cfg->ScanMode;
	AreaMode = cfg->AreaMode;
	FieldSize = cfg->FieldSize;
	AverageMode = cfg->AverageMode;
	AverageNum = cfg->AverageNum;
	OneXFieldSize = cfg->OneXFieldSize;
	FieldSizeCalibration = cfg->FieldSizeCalibration;
	memcpy_s(FieldOffset, 2*sizeof(double), cfg->FieldOffset, 2*sizeof(double));

	list<PointMap<double>>::iterator it = cfg->_resonantAmplitudeToVoltage.begin();
	for (; it != cfg->_resonantAmplitudeToVoltage.end(); ++it)
	{
		_resonantAmplitudeToVoltage.push_back(PointMap<double>(it->XValue, it->YValue));
	}

	it = cfg->_resonantTwoWayAlignment.begin();
	for (; it != cfg->_resonantTwoWayAlignment.end(); ++it)
	{
		_resonantTwoWayAlignment.push_back(PointMap<double>(it->XValue, it->YValue));
	}
	CalculateFieldToVoltage();
}

void CameraConfig::LoadFromXMLFile()
{
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(GetConfigureFilePath().c_str()) != 0)
	{
		throw exception("Load MesoScanSettings Failed!");
	}
	XMLElement* root = doc.RootElement();
	XMLElement* niSettingNode = root->FirstChildElement("NISetting");
	if (niSettingNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}
	XMLElement* clockNode = niSettingNode->FirstChildElement("Clock");
	if (clockNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}

	XMLElement* waveformNode = root->FirstChildElement("Waveforms");
	if (waveformNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}

	ResonanceFrequency = waveformNode->DoubleAttribute("ResonanceFrequency");
	SamplesPerLine = waveformNode->UnsignedAttribute("SamplesPerLine");

	XMLElement* galvoXWaveformNode = waveformNode->FirstChildElement("GalvoXWaveform");
	if (galvoXWaveformNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}
	MinPosX = galvoXWaveformNode->DoubleAttribute("MinPos");
	MaxPosX = galvoXWaveformNode->DoubleAttribute("MaxPos");
	MinPosXVoltage = galvoXWaveformNode->DoubleAttribute("MinPosVoltage");
	MaxPosXVoltage = galvoXWaveformNode->DoubleAttribute("MaxPosVoltage");
	DelayTimeGx1 = galvoXWaveformNode->DoubleAttribute("DelayTimeGx1");
	DelayTimeGx2 = galvoXWaveformNode->DoubleAttribute("DelayTimeGx2");
	GForX2 = galvoXWaveformNode->DoubleAttribute("GForX2");
	HForX2 = galvoXWaveformNode->DoubleAttribute("HForX2");
	GXExtendTimeStart = galvoXWaveformNode->DoubleAttribute("ExtendTimeStart");
	GXExtendTimeEnd = galvoXWaveformNode->DoubleAttribute("ExtendTimeEnd");
	MaxVelocityX = galvoXWaveformNode->DoubleAttribute("MaxVelocity");
	MaxOvershootX = galvoXWaveformNode->DoubleAttribute("MaxOvershoot");

	XMLElement* galvoYWaveformNode = waveformNode->FirstChildElement("GalvoYWaveform");
	if (galvoYWaveformNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}
	MinPosY = galvoYWaveformNode->DoubleAttribute("MinPos");
	MaxPosY = galvoYWaveformNode->DoubleAttribute("MaxPos");
	MinPosYVoltage = galvoYWaveformNode->DoubleAttribute("MinPosVoltage");
	MaxPosYVoltage = galvoYWaveformNode->DoubleAttribute("MaxPosVoltage");
	DelayTimeGy = galvoYWaveformNode->DoubleAttribute("DelayTime");
	GYExtendTimeStart = galvoYWaveformNode->DoubleAttribute("ExtendTimeStart");
	GYExtendTimeEnd = galvoYWaveformNode->DoubleAttribute("ExtendTimeEnd");
	MaxVelocityY = galvoYWaveformNode->DoubleAttribute("MaxVelocity");
	MaxOvershootY = galvoYWaveformNode->DoubleAttribute("MaxOvershoot");

	XMLElement* pokelsCellWaveformNode = waveformNode->FirstChildElement("PokelsCellWaveform");
	if (pokelsCellWaveformNode == NULL)
	{
		throw exception("Bad Format in ThorMesoScanSettings.xml");
	}
	DelayTimePC = pokelsCellWaveformNode->DoubleAttribute("DelayTime");
	PockelDutyCycle = pokelsCellWaveformNode->DoubleAttribute("PockelDutyCycle");
	PockelAllowMin = pokelsCellWaveformNode->DoubleAttribute("PockelAllowMin");
	PockelAllowMax = pokelsCellWaveformNode->DoubleAttribute("PockelAllowMax");
	PockelMinPercent = pokelsCellWaveformNode->DoubleAttribute("PockelMinPercent");
	PockelInMax = pokelsCellWaveformNode->DoubleAttribute("PockelInMax");
	PockelInMin = pokelsCellWaveformNode->DoubleAttribute("PockelInMin");
	XMLElement* voiceCoilWaveformNode = waveformNode->FirstChildElement("VoiceCoilWaveform");
	if (voiceCoilWaveformNode == NULL)
	{
		throw new exception("Bad Format in ThorMesoScanSettings.xml");
	}
	MinPosZ = voiceCoilWaveformNode->DoubleAttribute("MinPos");
	MaxPosZ = voiceCoilWaveformNode->DoubleAttribute("MaxPos");
	MinPosZVoltage = voiceCoilWaveformNode->DoubleAttribute("MinPosVoltage");
	MaxPosZVoltage = voiceCoilWaveformNode->DoubleAttribute("MaxPosVoltage");
	DelayTimeVC = voiceCoilWaveformNode->DoubleAttribute("DelayTime");
	VCExtendTimeStart = voiceCoilWaveformNode->DoubleAttribute("ExtendTimeStart");
	VCExtendTimeEnd = voiceCoilWaveformNode->DoubleAttribute("ExtendTimeEnd");
	VCSkipLines = voiceCoilWaveformNode->UnsignedAttribute("SkipLines");
	MaxVelocityVC = voiceCoilWaveformNode->DoubleAttribute("MaxVelocity");
	MaxOvershootVC = voiceCoilWaveformNode->DoubleAttribute("MaxOvershoot");
	CenterShiftX = voiceCoilWaveformNode->DoubleAttribute("CenterShiftX");
	CenterShiftY = voiceCoilWaveformNode->DoubleAttribute("CenterShiftY");
	CurveParamA = voiceCoilWaveformNode->DoubleAttribute("CurveParamA");
	CurveParamB = voiceCoilWaveformNode->DoubleAttribute("CurveParamB");

	XMLElement* resonanceNode = root->FirstChildElement("Resonance");
	if (resonanceNode == NULL)
	{
		throw new exception("Bad Format in ThorMesoScanSettings.xml");
	}
	const char *ResAO = resonanceNode->Attribute("RESONANT_AO_CHANNEL");
	const char *ResCtr = resonanceNode->Attribute("RESONANT_CTR");

	auto zoomPoint = resonanceNode->FirstChildElement("ZoomCalibration")->FirstChildElement("Point");
	while (zoomPoint != NULL)
	{
		auto zP = PointMap<double>(zoomPoint->DoubleAttribute("Amplitude"), zoomPoint->DoubleAttribute("Voltage"));
		_resonantAmplitudeToVoltage.push_back(zP);
		zoomPoint = zoomPoint->NextSiblingElement();
	}

	auto twoWayAlignPoint = resonanceNode->FirstChildElement("TowWayAlignment")->FirstChildElement("Point");
	while (twoWayAlignPoint != NULL)
	{
		auto tP = PointMap<double>(twoWayAlignPoint->DoubleAttribute("Amplitude"), twoWayAlignPoint->DoubleAttribute("ShiftPoints"));
		_resonantTwoWayAlignment.push_back(tP);
		twoWayAlignPoint = twoWayAlignPoint->NextSiblingElement();
	}

	XMLElement* feedbackNode = root->FirstChildElement("FeedBack");
	if (feedbackNode == NULL)
	{
		throw new exception("Bad Format in ThorMesoScanSettings.xml");
	}
	const char *GYVCFeedback = feedbackNode->Attribute("GY_VOICECOIL_FEEDBACK_CHANNEL");
	const char *GX1X2Feedback = feedbackNode->Attribute("GX1_GX2_FEEDBACK_CHANNEL");
	GYFeedbackRatio = feedbackNode->DoubleAttribute("GYFeedbackRatio");
	GX1FeedbackRatio = feedbackNode->DoubleAttribute("GX1FeedbackRatio");
	GX2FeedbackRatio = feedbackNode->DoubleAttribute("GX2FeedbackRatio");
	VCFeedbackRatio = feedbackNode->DoubleAttribute("VCFeedbackRatio");
	GYFeedbackOffset = feedbackNode->DoubleAttribute("GYFeedbackOffset");
	GX1FeedbackOffset = feedbackNode->DoubleAttribute("GX1FeedbackOffset");
	GX2FeedbackOffset = feedbackNode->DoubleAttribute("GX2FeedbackOffset");
	VCFeedbackOffset = feedbackNode->DoubleAttribute("VCFeedbackOffset");


	XMLElement* alazarNode = root->FirstChildElement("Alazar");
	if (alazarNode == NULL)
	{
		throw new exception("Bad Format in ThorMesoScanSettings.xml");
	}

	CalculateFieldToVoltage();

}


void CameraConfig::InitParameterRange()
{
	_doubleparameters[ICamera::PARAM_MESO_GX1_FEEDBACK_RATIO] = ParameterValue<double>(0, 1, &GX1FeedbackRatio);
	_doubleparameters[ICamera::PARAM_MESO_GX1_FEEDBACK_OFFSET] = ParameterValue<double>(0, 1, &GX1FeedbackOffset);
	_doubleparameters[ICamera::PARAM_MESO_GX2_FEEDBACK_RATIO] = ParameterValue<double>(0, 1, &GX2FeedbackRatio);
	_doubleparameters[ICamera::PARAM_MESO_GX2_FEEDBACK_OFFSET] = ParameterValue<double>(0, 1, &GX2FeedbackOffset);
	_doubleparameters[ICamera::PARAM_MESO_GY_FEEDBACK_RATIO] = ParameterValue<double>(0, 1, &GYFeedbackRatio);
	_doubleparameters[ICamera::PARAM_MESO_GY_FEEDBACK_OFFSET] = ParameterValue<double>(0, 1,&GYFeedbackOffset);
	_doubleparameters[ICamera::PARAM_MESO_VOICECOIL_FEEDBACK_RATIO] = ParameterValue<double>(0, 1, &VCFeedbackRatio);
	_doubleparameters[ICamera::PARAM_MESO_VOICECOIL_FEEDBACK_OFFSET] = ParameterValue<double>(0, 1, &VCFeedbackOffset);
	_doubleparameters[ICamera::PARAM_MESO_POS_MIN_X_VOLTAGE] = ParameterValue<double>(-9.6, GALVO_MAX_VOL, &MinPosXVoltage);
	_doubleparameters[ICamera::PARAM_MESO_POS_MIN_Y_VOLTAGE] = ParameterValue<double>(GALVO_MIN_VOL, GALVO_MAX_VOL, &MinPosYVoltage);
	_doubleparameters[ICamera::PARAM_MESO_POS_MIN_Z_VOLTAGE] = ParameterValue<double>(GALVO_MIN_VOL, GALVO_MAX_VOL, &MinPosZVoltage);
	_doubleparameters[ICamera::PARAM_MESO_POS_MAX_X_VOLTAGE] = ParameterValue<double>(-9.6, GALVO_MAX_VOL, &MaxPosXVoltage);
	_doubleparameters[ICamera::PARAM_MESO_POS_MAX_Y_VOLTAGE] = ParameterValue<double>(GALVO_MIN_VOL, GALVO_MAX_VOL, &MaxPosYVoltage);
	_doubleparameters[ICamera::PARAM_MESO_POS_MAX_Z_VOLTAGE] = ParameterValue<double>(GALVO_MIN_VOL, GALVO_MAX_VOL, &MaxPosZVoltage);

	_doubleparameters[ICamera::PARAM_MESO_MAXPOS_X] = ParameterValue<double>(600, 6300, &MaxPosX);
	_doubleparameters[ICamera::PARAM_MESO_MAXPOS_Y] = ParameterValue<double>(600, 6300, &MaxPosY);
	_doubleparameters[ICamera::PARAM_MESO_MAXPOS_Z] = ParameterValue<double>(1000, 2000, &MaxPosZ);

	_doubleparameters[ICamera::PARAM_MESO_LAG_GALVO_Y] = ParameterValue<double>(0, 1.000, &DelayTimeGy);
	_doubleparameters[ICamera::PARAM_MESO_DAMPING_GALVO_Y] = ParameterValue<double>(0.25, 2.0, &MaxOvershootY);
	_doubleparameters[ICamera::PARAM_MESO_MAXVELOCITY_GALVO_Y] = ParameterValue<double>(2000, 4000, &MaxVelocityY);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_START_Y] = ParameterValue<double>(0, 0, &GYExtendTimeStart);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_END_Y] = ParameterValue<double>(0, 0, &GYExtendTimeEnd);

	_doubleparameters[ICamera::PARAM_MESO_LAG_GALVO_X1] = ParameterValue<double>(0, 1.000, &DelayTimeGx1);
	_doubleparameters[ICamera::PARAM_MESO_LAG_GALVO_X2] = ParameterValue<double>(0, 1.000, &DelayTimeGx2);
	_doubleparameters[ICamera::PARAM_MESO_DAMPING_GALVO_X] = ParameterValue<double>(0.25, 2.0, &MaxOvershootX);
	_doubleparameters[ICamera::PARAM_MESO_MAXVELOCITY_GALVO_X] = ParameterValue<double>(1500, 4000, &MaxVelocityX);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_START_X] = ParameterValue<double>(0, 0.200, &GXExtendTimeStart);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_END_X] = ParameterValue<double>(0, 0.200,&GXExtendTimeEnd);

	_doubleparameters[ICamera::PARAM_MESO_LAG_VOICECOIL] = ParameterValue<double>(0, 3.000, &DelayTimeVC);
	_doubleparameters[ICamera::PARAM_MESO_DAMPING_VOICECOIL] = ParameterValue<double>(0.25, 2, &MaxOvershootVC);
	_doubleparameters[ICamera::PARAM_MESO_MAXVELOCITY_VOICECOIL] = ParameterValue<double>(1500, 4000, &MaxVelocityVC);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_START_VOICECOIL] = ParameterValue<double>(0, 2.000, &VCExtendTimeStart);
	_doubleparameters[ICamera::PARAM_MESO_EXTENDTIME_END_VOICECOIL] = ParameterValue<double>(0, 2.000, &VCExtendTimeEnd);

	_doubleparameters[ICamera::PARAM_MESO_TWOWAY_ALIGNMENT_SHIFT] = ParameterValue<double>(-200, 200, NULL);
	_doubleparameters[ICamera::PARAM_MESO_GY_FIELD_TO_VOLTAGE] = ParameterValue<double>(-0.004, 0.004, &F2VGy);
	_doubleparameters[ICamera::PARAM_MESO_RESONANT_FIELD_TO_VOLTAGE] = ParameterValue<double>(0, 0.0100, NULL);

	_doubleparameters[ICamera::PARAM_MESO_GX_FIELD_TO_VOLTAGE] = ParameterValue<double>(-0.0032, 0.0032, &F2VGx1);
	_doubleparameters[ICamera::PARAM_MESO_VOICECOIL_FIELD_TO_VOLTAGE] = ParameterValue<double>(-0.005, 0.005, &F2VZ);
	_doubleparameters[ICamera::PARAM_MESO_DUTYCYCLE] = ParameterValue<double>(0, 1, &PockelDutyCycle);
	_doubleparameters[ICamera::PARAM_MESO_POCKEL_ALLOW_MIN] = ParameterValue<double>(POCKEL_MIN_VOL, POCKEL_MAX_VOL, &PockelAllowMin);
	_doubleparameters[ICamera::PARAM_MESO_POCKEL_ALLOW_MAX] = ParameterValue<double>(POCKEL_MIN_VOL, POCKEL_MAX_VOL, &PockelAllowMax);
	_doubleparameters[ICamera::PARAM_LSM_POCKELS_MAX_VOLTAGE_0] = ParameterValue<double>(POCKEL_MIN_VOL, POCKEL_MAX_VOL, &PockelInMax);
	_doubleparameters[ICamera::PARAM_LSM_POCKELS_MIN_VOLTAGE_0] = ParameterValue<double>(POCKEL_MIN_VOL, POCKEL_MAX_VOL, &PockelInMin);
	_doubleparameters[ICamera::PARAM_MESO_POCKEL_MIN_PERCENT] = ParameterValue<double>(0, 100, &PockelMinPercent);

	_doubleparameters[ICamera::PARAM_MESO_G_FOR_X2] = ParameterValue<double>(-20, 20, &GForX2);
	_doubleparameters[ICamera::PARAM_MESO_H_FOR_X2] = ParameterValue<double>(-20, 20, &HForX2);

	// z = a*(x-shiftx)*(x-shiftx) + b*(y-shifty)*(y-shifty)  the range of x is 0~2.5 ; the range of z is (10.0 - MaxPosZVoltage)/F2VZ
	// default set F2VZ = 0.0036; MaxPosZ = 6.34; so the rang of a,b is about 80
	_doubleparameters[ICamera::PARAM_MESO_CURVE_PARAME_A] = ParameterValue<double>(0, 50, &CurveParamA);
	_doubleparameters[ICamera::PARAM_MESO_CURVE_PARAME_B] = ParameterValue<double>(0, 50, &CurveParamB);

	_doubleparameters[ICamera::PARAM_MESO_CENTER_SHIFT_X_VOICECOIL] = ParameterValue<double>(-500, 500, &CenterShiftX);
	_doubleparameters[ICamera::PARAM_MESO_CENTER_SHIFT_Y_VOICECOIL] = ParameterValue<double>(-500, 500, &CenterShiftY);
	_uintParameters[ICamera::PARAM_MESO_POINTS_PER_LINE_VOICECOIL] = ParameterValue<uint32_t>(1, 10, &VCPointsPerLine);



	//===	additional parameters	 ===//

	_uintParameters[ICamera::PARAM_LSM_GALVO_ENABLE] = ParameterValue<uint32_t>(0, 1, &GalvoEnable);
	_uintParameters[ICamera::PARAM_LSM_SCANMODE] = ParameterValue<uint32_t>(ScanMode::TWO_WAY_SCAN, ScanMode::SCANMODE_LAST, &ScanMode);
	_uintParameters[ICamera::PARAM_LSM_AREAMODE] = ParameterValue<uint32_t>(ICamera::LSMAreaMode::FIRST_AREA_MODE, ICamera::LSMAreaMode::LAST_AREA_MODE, &AreaMode);
	_uintParameters[ICamera::PARAM_LSM_FIELD_SIZE] = ParameterValue<uint32_t>(MIN_FIELDSIZE, MAX_FIELDSIZE, &FieldSize);
	_uintParameters[ICamera::PARAM_LSM_AVERAGEMODE] = ParameterValue<uint32_t>(0, 1, &AverageMode);
	_uintParameters[ICamera::PARAM_LSM_AVERAGENUM] = ParameterValue<uint32_t>(1, INT_MAX, &AverageNum);
	_uintParameters[ICamera::PARAM_LSM_1X_FIELD_SIZE] = ParameterValue<uint32_t>(MIN_FIELDSIZE, MAX_FIELDSIZE, &OneXFieldSize);
	_uintParameters[ICamera::PARAM_MULTI_FRAME_COUNT] = ParameterValue<uint32_t>(0, INT_MAX, &FrameCount);

	_doubleparameters[ICamera::PARAM_LSM_FIELD_SIZE_CALIBRATION] = ParameterValue<double>(1.0, 1000.0, &FieldSizeCalibration);
	_doubleparameters[ICamera::PARAM_LSM_OFFSET_X] = ParameterValue<double>(-(MAX_FIELDSIZE - MIN_FIELDSIZE) / 2, (MAX_FIELDSIZE - MIN_FIELDSIZE) / 2, &FieldOffset[0]);
	_doubleparameters[ICamera::PARAM_LSM_OFFSET_Y] = ParameterValue<double>(-(MAX_FIELDSIZE - MIN_FIELDSIZE) / 2, (MAX_FIELDSIZE - MIN_FIELDSIZE) / 2, &FieldOffset[1]);

}

long CameraConfig::GetParameterRange(ICamera::Params parameter, uint32_t &min, uint32_t &max, uint32_t &value)
{
	auto pos = _uintParameters.find(parameter);
	if (pos != _uintParameters.end())
	{
		auto v = pos->second;
		min = v.MinValue;
		max = v.MaxValue;
		if (v.ActualValue != NULL)
		{
			value = *(v.ActualValue);
			return TRUE;
		}
	}
	return FALSE;
}

long CameraConfig::GetParameterRange(ICamera::Params parameter, double &min, double &max, double &value)
{
	auto pos = _doubleparameters.find(parameter);
	if (pos != _doubleparameters.end())
	{
		auto v = pos->second;
		min = v.MinValue;
		max = v.MaxValue;
		if (v.ActualValue != NULL)
		{
			value = *(v.ActualValue);
		}
		return TRUE;
	}
	return FALSE;
}

long CameraConfig::GetParameter(ICamera::Params parameter, uint32_t &value)
{
	auto pos = _uintParameters.find(parameter);
	if (pos != _uintParameters.end())
	{
		auto v = pos->second;
		if (v.ActualValue != NULL)
		{
			value = *(v.ActualValue);
			return TRUE;
		}
	}
	return FALSE;
}

long CameraConfig::GetParameter(ICamera::Params parameter, double &value)
{
	auto pos = _doubleparameters.find(parameter);
	if (pos != _doubleparameters.end())
	{
		auto v = pos->second;
		if (v.ActualValue != NULL)
		{
			value = *(v.ActualValue);
			return TRUE;
		}
	}
	return FALSE;
}

long CameraConfig::SetParameter(ICamera::Params parameter, uint32_t value)
{
	auto pos = _uintParameters.find(parameter);
	if (pos != _uintParameters.end())
	{
		auto v = pos->second;
		if ((value <= v.MaxValue) && (value >= v.MinValue) && (v.ActualValue != NULL))
		{
			*(v.ActualValue) = value;
			return TRUE;
		}
	}

	return FALSE;
}

long CameraConfig::SetParameter(ICamera::Params parameter, double value)
{
	auto pos = _doubleparameters.find(parameter);
	if (pos != _doubleparameters.end())
	{
		auto v = pos->second;
		if ((value <= v.MaxValue) && (value >= v.MinValue) && (v.ActualValue!= NULL))
		{
			*(v.ActualValue) = value;
			return TRUE;
		}
	}
	return FALSE;
}

long CameraConfig::GetResonantVoltage(double amplitude, double &voltage)
{
	PointMap<double>* min = NULL;
	PointMap<double>* max = NULL;
	for (list<PointMap<double>>::iterator it = _resonantAmplitudeToVoltage.begin(); it != _resonantAmplitudeToVoltage.end(); ++it)
	{
		if (it->XValue <= amplitude)
		{
			min = &(PointMap<double>)(*it);
		}
		else if(it->XValue >= amplitude)
		{
			max = &(PointMap<double>)(*it);
			break;
		}
	}
	if ((NULL == min) || (NULL == max))
	{
		return FALSE;
	}

	voltage = ((max->YValue - min->YValue) / (max->XValue - min->XValue))*(amplitude - min->XValue) + min->YValue;

	return TRUE;
}

long CameraConfig::SetReosnantVoltage(double amplitude, double voltage)
{
	if (amplitude < _resonantAmplitudeToVoltage.front().XValue || voltage > _resonantAmplitudeToVoltage.back().YValue)
		return FALSE;
	list<PointMap<double>>::iterator it = _resonantAmplitudeToVoltage.begin();
	for (; it != _resonantAmplitudeToVoltage.end(); ++it)
	{
		if (abs(it->XValue - amplitude) < 0.001)
		{
			it->YValue = voltage;
			return TRUE;
		}
		else if (it->XValue > amplitude)
		{
			break;
		}
	}
	if (it != _resonantAmplitudeToVoltage.end())
	{
		auto newA2V = PointMap<double>(amplitude, voltage);
		_resonantAmplitudeToVoltage.insert(it, newA2V);
		return TRUE;
	}
	return FALSE;
}

long CameraConfig::GetTwoWayAlignmentPoint(double amplitude, double &point)
{
	PointMap<double>* min = NULL;
	PointMap<double>* max = NULL;
	for (list<PointMap<double>>::iterator it = _resonantTwoWayAlignment.begin(); it != _resonantTwoWayAlignment.end(); ++it)
	{
		if (it->XValue <= amplitude)
		{
			min = &(PointMap<double>)(*it);
		}
		else if (it->XValue >= amplitude)
		{
			max = &(PointMap<double>)(*it);
			break;
		}
	}
	if ((NULL == min) || (NULL == max))
	{
		return FALSE;
	}

	point = ((max->YValue - min->YValue) / (max->XValue - min->XValue))*(amplitude - min->XValue) + min->YValue;

	return TRUE;
}

long CameraConfig::SetTwoWayAlignmentPoint(double amplitude, double point)
{
	if (amplitude < _resonantTwoWayAlignment.front().XValue)
		return FALSE;
	list<PointMap<double>>::iterator it = _resonantTwoWayAlignment.begin();
	for (; it != _resonantTwoWayAlignment.end(); ++it)
	{
		if (abs(it->XValue - amplitude) < 0.001)
		{
			it->YValue = point;
			return TRUE;
		}
		else if (it->XValue > amplitude)
		{
			break;
		}
	}
	if (it != _resonantTwoWayAlignment.end())
	{
		auto ashiftP = PointMap<double>(amplitude, point);
		_resonantTwoWayAlignment.insert(it, ashiftP);
		return TRUE;
	}
	return FALSE;
}

bool CameraConfig::IsUIntValue(ICamera::Params parameter)
{
	if ((parameter == ICamera::PARAM_MESO_POINTS_PER_LINE_VOICECOIL) ||
		(parameter == ICamera::PARAM_LSM_DMA_BUFFER_COUNT) || (parameter == ICamera::PARAM_LSM_GALVO_ENABLE) ||
		(parameter == ICamera::PARAM_LSM_SCANMODE) || (parameter == ICamera::PARAM_LSM_AREAMODE) || 
		(parameter == ICamera::PARAM_LSM_FIELD_SIZE) || (parameter == ICamera::PARAM_LSM_CHANNEL) || 
		(parameter == ICamera::PARAM_LSM_AVERAGEMODE) || (parameter == ICamera::PARAM_LSM_AVERAGENUM) || 
		(parameter == ICamera::PARAM_LSM_1X_FIELD_SIZE) || (parameter == ICamera::PARAM_MULTI_FRAME_COUNT) || 
		(parameter == ICamera::PARAM_TRIGGER_MODE) || (parameter == ICamera::PARAM_RAW_SAVE_ENABLED_CHANNELS_ONLY) ||
		(parameter == ICamera::PARAM_DROPPED_FRAMES)
		)
	{
		return true;
	}
	return false;
}

double CameraConfig::GetPockelPowerVoltage(double percentage)
{
	double para = PockelInMin;
	if (percentage > PockelMinPercent)
	{
		double tempVolt = (percentage - PockelMinPercent) / (1 - PockelMinPercent / 100);
		para = asin(pow(tempVolt / 100, 1.0 / 2.2)) / (M_PI*0.5)*(PockelInMax - PockelInMin) + PockelInMin;
		if (para > PockelInMax) para = PockelInMax;
		else if (para < PockelInMin) para = PockelInMin;
	}
	return para;
}

CameraConfig::~CameraConfig()
{

}



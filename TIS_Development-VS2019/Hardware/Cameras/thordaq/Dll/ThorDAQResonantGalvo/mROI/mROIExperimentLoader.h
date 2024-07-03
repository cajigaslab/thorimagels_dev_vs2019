#pragma once
#include <memory>
#include <vector>
#include "..\..\..\..\..\Common\camera.h"
#include "..\..\..\..\..\Common\ScanManager\scan.h"
#include "ScanmROI.h"
#include <functional>
#define CHANNEL_COUNT		4
#define CHECK_PFUNC(x, y) if(NULL != x) { x->y; }

#define CHECK_PFUNC_OUTPUT(x, y, z)	if(NULL != x) {	z = x->y; }

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

enum TASK_ID { ZSTAGE = 0, POCKELS1 = 0, GY = 1, GX1 = 2, POCKELS2 = 3, FRAMETRG = 0, GX2 = 0, POCKELS3 = 1, POCKELS4 = 2 };

enum GALVO_PARK_TYPE { PARK_AT_CENTER = 0, PARK_NEAR_START = 1, PARK_AT_EXIT = 2 };


typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int frameWidth;
	unsigned int frameHeight;

} FrameROI;

typedef struct {
	unsigned short ScanID;
	unsigned short ScanAreaID;
	unsigned short StreamID;
	unsigned short TimeID;
	unsigned short ZID;
	unsigned short ChannelID;
} ChanBufferInfo;

typedef struct {
	unsigned int stripIndex;
	unsigned int stripCount;
	double frameRate;
	double scanTime;
} CaptureINFO;

enum ImageCompleteStates
{
	SCAN_ERROR = 0,
	SCAN_COMPLETE = 1,
	SCAN_ABORT = 2,
	SCAN_IMAGE = 4,
	SCAN_IMAGE_STRIPEEND = 5,
	SCAN_BUSY = 6
};
typedef std::function<void(ImageCompleteStates, ChanBufferInfo, FrameROI, unsigned short*, unsigned int)> ImageCompleteCallback;

using namespace std;

class PowerMask
{
public:
	uint8_t* data;

	uint32_t LineCount;
	uint32_t YSize;
	double XPhysicalSize;
	double XPos;

	vector<ROI*> ROIs;
	bool isUsed;
	~PowerMask()
	{
		if (data != NULL)
			delete data;

		for (size_t i = 0; i < ROIs.size(); i++)
		{
			if (NULL != ROIs.at(i))
			{
				delete ROIs.at(i);
				ROIs.at(i) = NULL;
			}
		}
		ROIs.clear();
	}
};


class StripInfo
{
public:
	StripInfo()
	{
		PowerMask = NULL;
	}
	~StripInfo()
	{
		ROIPower.clear();
	}
	static void DeleteStripLoop(StripInfo* strip)
	{
		StripInfo* current = strip;
		while (current != NULL)
		{
			bool isLast = current->IsEnd;
			delete current;
		}
	}
	StripInfo(const StripInfo& source)
	{
		XPosResonMid = source.XPosResonMid;
		XPos = source.XPos;
		YPos = source.YPos;
		ZPos = source.ZPos;
		XPhysicalSize = source.XPhysicalSize;
		YPhysicalSize = source.YPhysicalSize;
		ZPhysicalSize = source.ZPhysicalSize;
		XSize = source.XSize;
		YSize = source.YSize;
		IncludeSignal = source.IncludeSignal;
		SkipSignal = source.SkipSignal;
		Power = source.Power;
		ROIPower = source.ROIPower;
		ScanMode = source.ScanMode;
		PockelsVolt = source.PockelsVolt;
		FrameROI = source.FrameROI;
		PowerMask = source.PowerMask;
		IsEnd = source.IsEnd;
		IsStart = source.IsStart;
		IsFrameEnd = source.IsFrameEnd;
		IsFrameStart = source.IsFrameStart;
		ScanAreaID = source.ScanAreaID;
		ActiveScanAreasIndex = source.ActiveScanAreasIndex;
		flyToNextStripeSkipLines = source.flyToNextStripeSkipLines;
		FullFOVPhysicalSizeUM = source.FullFOVPhysicalSizeUM;
		StripeFieldSize = source.StripeFieldSize;
	}
	double XPosResonMid;
	double XPos;
	double YPos;
	double ZPos;
	double XPhysicalSize;
	double YPhysicalSize;
	double ZPhysicalSize;
	unsigned int XSize;
	unsigned int YSize;
	unsigned int IncludeSignal;  //
	unsigned int SkipSignal;	//
	unsigned int flyToNextStripeSkipLines;
	vector<double> Power;
	vector<pair<ROI*, double>> ROIPower;
	ScanMode ScanMode;
	double PockelsVolt;
	FrameROI FrameROI;
	PowerMask* PowerMask;
	double FullFOVPhysicalSizeUM;
	bool IsStart;
	bool IsEnd;
	bool IsFrameEnd;
	bool IsFrameStart;
	double XPixelSize;
	double YPixelSize;
	unsigned int ScanAreaID;
	unsigned int ActiveScanAreasIndex;
	unsigned short StripeFieldSize;
	bool IsFullFOV;
};

class WaveformParams
{
public:
	double F2TGx1;
	double F2TGx2;
	double F2TGy;
	int SamplesPerLine;
	double ResFreq;
	double PockelDutyCycle;
	double PockelMinPercent;
	double PockelInMax;
	double PockelInMin;
	double PockelSetValue;
	double PockelGetValue;
	double PhysicalStripeWidth;
	unsigned int StripWidth;
	double CurveParameterA;
	double CurveParameterB;
	double CenterShiftX;
	double CenterShiftY;
	unsigned int VCPointsPerLine;
	double MaxVelocityX;
	double MaxOvershootX;
	double MaxVelocityY;
	double MaxOvershootY;
	double MaxVelocityVC;
	double MaxOvershootVC;
	double FieldWidth;
	double FieldHeight;
	unsigned short NumberOfAverageFrame;
	double DelayTimeGy;
	double DelayTimeGx1;
	double DelayTimeGx2;
	double DelayTimeVC;
	double DelayTimePC;

	double ZeroPointVoltGX1;
	double ZeroPointVoltGY;
	double ZeroPointVoltVoiceCoil;
	double MaxPointVoltVoiceCoil;
	double GX2Scale;
	double GX2Shift;
	double VoicecoilZToVolts;
	double GXExtendTimeStart;
	double GXExtendTimeEnd;
	double GYExtendTimeStart;
	double GYExtendTimeEnd;
	double VCExtendTimeStart;
	double VCExtendTimeEnd;
	unsigned int VCSkipLines;
	bool IsEnableCurveCorrection;
	vector<pair<uint16_t, double>> powerBoxs;
	int TwoWayOffset;
};

class mROIExperimentLoader
{
public:
	mROIExperimentLoader() : ExpPath(L"") {};
	~mROIExperimentLoader()
	{
		ReleaseScans();
	}

	std::wstring ExpPath;
	std::vector<Scan*> Scans;

	void ReleaseScans()
	{
		if (0 < Scans.size())
		{
			for (size_t i = 0; i < Scans.size(); i++)
			{
				delete Scans.at(i);
			}
			Scans.clear();
		}
	}

	long LoadExperimentXML();

};
#pragma once

#include <string>
#include <vector>

enum ResUnit
{
	None = 0,
	Inch = 1,
	Centimeter = 2,
	Millimetre = 3,
	Micron = 4,
	Nanometer = 5,
	Picometer = 6
};

struct SampleRegion
{
	uint16_t WellSampleID;
	uint16_t RegionID;
	double PositionX;
	double PositionY;
	double PositionZ;
	double SizeX;
	double SizeY;
	double SizeZ;
	ResUnit PhysicalSizeXUnit;
	ResUnit PhysicalSizeYUnit;
	ResUnit PhysicalSizeZUnit;
};

extern "C" _declspec(dllexport) typedef struct ScanRegion
{
	 uint8_t ScanID;
	uint16_t RegionID;
	uint32_t SizeX;
	uint32_t SizeY;
	uint32_t SizeZ;
	uint32_t SizeT;
	uint32_t SizeS;
	  size_t BufferSize;
}ScanRegionStruct;

using namespace std;

class IExperiment
{	
public:
	enum SampleType
	{
		WELLTYPE_6 = 0,
		WELLTYPE_24 = 1,
		WELLTYPE_96 = 2,
		WELLTYPE_384 = 3,
		WELLTYPE_1536 = 4,
		WELLTYPE_SLIDE = 5
	};

	struct SubImage
	{
		int wellID;
		bool isEnable;
		int subRows;
		int subColumns;
		double transOffsetXMM;
		double transOffsetYMM;
		double transOffsetZMM;
		double overlapX;
		double overlapY;
		double scanAreaWidth;
		double scanAreaHeight;
	};

	struct Wavelength
	{
		string name;
		long exposureTimeMS;
	};

	struct Pockels
	{
		long type;
		double start;
		double stop;
		string path;
	};

	struct SequenceStep
	{
		long PMT1Gain;
		long PMT1Enable;
		long PMT2Gain;
		long PMT2Enable;
		long PMT3Gain;
		long PMT3Enable;
		long PMT4Gain;
		long PMT4Enable;
		long LSMChannel;
		long MultiphotonPos;
		double MCLSPower1;
		long MCLSEnable1;
		double MCLSPower2;
		long MCLSEnable2;
		double MCLSPower3;
		long MCLSEnable3;
		double MCLSPower4;
		long MCLSEnable4;
		long PinholePos;
		long LightPathGGEnable;
		long LightPathGREnable;
		long LightPathCamEnable;
		long InvertedLightPathPosition;
		vector<Wavelength> Wavelength;
		vector<Pockels> Pockels;
	};	

	struct ZScanParams
	{
		long scanID;
		long regionID;
		long hyperspectral;
		long totalT;
		long zFramesPerVolume;
		double zStartPos;
		double zStopPos;
		double zStepSizeMM;
	};

	enum CaptureModes
	{
		Z_AND_T = 0,
		STREAMING = 1,
		//TDI = 2,
		BLEACHING = 3,
		HYPERSPECTRAL = 4
	};

	virtual long GetPathAndName(wstring &pathAndName)=0;///Experiment file path

	virtual long GetName(string &name)=0;///Experiment Name
	virtual long SetName(string name)=0; 

	virtual long GetDate(string &name)=0;///Creation Date
	virtual long SetDate(string name)=0;

	virtual long GetUser(string &name)=0;///User name that created the experiment
	virtual long SetUser(string name)=0; 

	virtual long GetComputer(string &name)=0;///computer name the experiment was created on
	virtual long SetComputer(string name)=0;

	virtual long GetSoftware(double &version)=0;///software version the experiment was created on
	virtual long SetSoftware(double version)=0;

	virtual long GetModality(long& cameraType,string &modality,long& lsmType)=0;

	virtual long GetImageArea(long cameraType,long lsmtype,long &pixelX,long &pixelY,double &pixelSizeUM)=0;
	virtual long GetCamera(string &name,long &width,long &height,double &pixelSizeUM,double &exposureTimeMS, long &gain, long &blackLevel, long &lightMode, long &left, long &top, long &right, long &bottom, long &binningX, long &binningY, long &tapsIndex, long &tapsBalance, long &readOutSpeedIndex, long &averageMode, long &averageNum, long &verticalFlip, long &horizontalFlip, long &imageAngle)=0;///camera information
	virtual long SetCamera(string name, long width, long height, double pixelSizeUM, long binning, long gain, long lightMode)=0;

	virtual long GetLSM(long &areaMode, double &areaAngle, long &scanMode,long &interleave,long &pixelX,long &pixelY,long &channel, long &fieldSize, long &offsetX, long &offsetY,
		long &averageMode, long &averageNum, long &clockSource, long &inputRange1, long &inputRange2, long &twoWayAlignment, long &extClockRate, double &dwellTime, 
		long &flybackCycles, long &inputRange3, long &inputRange4, long &minimizeFlybackCycles, long &polarity1, long &polarity2, long &polarity3, long &polarity4,
		long &verticalFlip, long &horizontalFlip, double &crsFrequencyHz, long &timeBasedLineScan, long &timeBasedLSTimeMS)=0;
	virtual long SetLSM(long areaMode, double areaAngle,long scanMode,long interleave,long pixelX,long pixelY,long channel, long fieldSize, long offsetX, long offsetY,
		long averageMode, long averageNum, long clockSource, long inputRange1, long inputRange2, long twoWayAlignment, long extClockRate,double dwellTime,long flybackCycles, long inputRange3, long inputRange4, long minimizeFlybackCycles, long polarity1, long polarity2, long polarity3, long polarity4, long verticalFlip, long horizontalFlip)=0;

	virtual long GetMCLS(long &enable1, double &power1,long &enable2, double &power2,long &enable3, double &power3,long &enable4, double &power4)=0;
	virtual long SetMCLS(long enable1, double power1,long enable2, double power2,long enable3, double power3,long enable4, double power4)=0;

	virtual long GetPinholeWheel(long &positiion)=0;///pinhole position value
	virtual long SetPinholeWheel(long position)=0;

	virtual long GetRaw(long &onlyEnabledChannels)=0;///flag to acquire enabled channel only for saving space and expediting exp

	virtual long GetStreaming(long& enable, long& frames, long& rawData, long& triggerMode, long& displayImage, long& storageMode, long& zFastEnable, long& zFastMode, long& flybackFrames, long& flybackLines, double& flybackTimeAdjustMS, double& volumeTimeAdjustMS, double& stepTimeAdjustMS, long& stimulusTriggering, long& dmaFrames, long& stimulusMaxFrames, long& previewIndex, long& useReferenceVoltageForPockels, long& displayRollingAveragePreview) = 0;
	virtual long SetStreaming(long enable,long frames, long rawData, long triggerMode, long displayImage, long storageMode, long zFastEnable, long flybackFrames, long stimulusTriggering,long dmaFrames, long stimulusMaxFrames, long previewIndex, long useReferenceVoltageForPockels)=0;

	virtual long GetPMT(long &enableA, long &gainA, long &bandwidthA, double &offsetA, long &enableB, long &bandwidthB, long &gainB, double &offsetB, long &enableC, long &gainC, long &bandwidthC, double &offsetC, long &enableD, long &gainD, long &bandwidthD, double &offsetD)=0;
	virtual long SetPMT(long enableA, long gainA, long bandwidthA, double offsetA, long enableB, long gainB, long bandwidthB, double offsetB, long enableC, long gainC, long bandwidthC, double offsetC, long enableD, long gainD, long bandwidthD, double offsetD)=0;

	virtual long GetMagnification(double &mag,string &name)=0;///magnification value
	virtual long SetMagnification(double mag,string name)=0;

	virtual long GetNumberOfWavelengths()=0;///number of wavelengths imaged in the experiment
	virtual long GetChannelEnableSet()=0;///number of enabled wavelengths in the experiment, in bitmask: 1:ChA, 2:ChB, 4:ChC, 8:ChD
	virtual long GetWavelength(long index,string &name,double &exposureTimeMS)=0;///wavelength details
	virtual long SetWavelength(long index,string name,double exposureTimeMS)=0;
	virtual long AddWavelength(string name,double exposureTimeMS)=0;
	virtual long RemoveWavelength(string name)=0;
	virtual long RemoveAllWavelengths()=0;

	virtual long GetZStage(string &name, long &enable, long &steps, double &stepSize, double &startPos, long &zStreamFrames, long &zStreamMode)=0; //for z stream
	virtual long SetZStage(string name,  long enable, long steps, double stepSize, double startPos, long zStreamFrames, long zStreamMode)=0;

	virtual long GetTimelapse( long &timepoints,double &intervalSec,long &triggerMode)=0;///Timelapse details
	virtual long SetTimelapse( long timepoints, double intervalSec, long triggerMode)=0;

	virtual long GetTimelapseTOffset(long &t)=0;///Offset for timelapse t;
	virtual long SetTimelapseTOffset(long t)=0;

	//Sample Well dimension details
	virtual long GetSampleInfo(string &name, double &width, double &height, long &row, long &column, double &diameter, double &centerToCenterX, double &centerToCenterY, double &topLeftCenterOffsetX, double &topLeftCenterOffsetY, string &WellShape, double &WellWidth, double &WellHeight, double &initialStageLocationX, double &initialStageLocationY )=0;
	virtual long GetSample(double &offsetX, double &offsetY, double &offsetZ, long &tiltAdjustment, double &fPt1X, double &fPt1Y, double &fPt1Z, double &fPt2X, double &fPt2Y, double &fPt2Z, double &fPt3X, double &fPt3Y, double &fPt3Z )=0;///Sample details
	virtual long SetSample(SampleType type, double offsetX, double offsetY, long tiltAdjustment, double fPt1X, double fPt1Y, double fPt1Z, double fPt2X, double fPt2Y, double fPt2Z, double fPt3X, double fPt3Y, double fPt3Z)=0;

	virtual long GetWells(long &startRow, long &startColumn, long &rows, long &columns, double &wellOffSetXMM, double &wellOffsetYMM)=0;///Well information for sample type "plate"
	virtual long SetWells(long startRow, long startColumn, long rows, long columns, double wellOffSetXMM, double wellOffsetYMM)=0;

	virtual long GetSubImages( vector<SubImage>& subImages, long cameraType, long lsmType)=0;///mosaic details at each well
	virtual long SetSubImages( long subRows, long subColumns, double subOffsetXMM, double subOffsetYMM,double transOffsetXMM, double transOffsetYMM)=0;

	virtual long GetComments(string &name)=0;///user provided comments
	virtual long SetComments(string name)=0;

	virtual long GetAutoFocus(long &type, long &repeat, double &expTimeMS, double &stepSizeUM, double &startPosMM, double &stopPosMM)=0;
	virtual long SetAutoFocus(long type, long repeat, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM)=0;

	virtual long GetPower(long &enable, long &type, double &start, double &stop, string &path, double &offset, long index)=0;
	virtual long SetPower(long enable, long type, double start, double stop, string path, double offset, long index)=0;

	virtual long GetPockels(long index, long &type, double &start, double &stop, string &path, long &blankPercent)=0;
	virtual long SetPockels(long index, long type, double start, double stop, string path, long blankPercent)=0;	

	virtual long GetPockelsMask(long &enable, long &invert, string &path) = 0;
	virtual long SetPockelsMask(long enable, long invert, string path) = 0;

	virtual long GetImageCorrection(long &enablePincushion, double &pinCoeff1, double &pinCoeff2, double &pinCoeff3, long &enableBackgroundSubtraction, string &pathBackgroundSubtraction, long &enableFlatField, string &pathFlatField)=0;
	virtual long SetImageCorrection(long enablePincushion, double pinCoeff1, double pinCoeff2, double pinCoeff3, long enableBackgroundSubtraction, string pathBackgroundSubtraction, long enableFlatField, string pathFlatField)=0;

	virtual long GetMultiPhotonLaser(long &enable, long &positiion, long &seqEnable, long &seqPos1, long &seqPos2)=0;
	virtual long SetMultiPhotonLaser(long enable, long position, long seqEnable, long seqPos1, long seqPos2)=0;

	virtual long GetPhotoBleachingAttr(char* attrName, double& val)=0;
	virtual long GetPhotobleaching(long &enable, long &laserPositiion, long &durationMS, double &powerPosition, long &width, long &height, long &offsetX, long &offsetY, long &bleachingFrames, long &fieldSize, long &bleachTrigger, long &preBleachingFrames, double &preBleachingInterval, long &preBleachingStream, long &postBleachingFrames1, double &postBleachingInterval1, long &postBleachingStream1, long &postBleachingFrames2, double &postBleachingInterval2, long &postBleachingStream2, long &powerEnable, long &laserEnable, long &bleachQuery, long &bleachPostTrigger, long &enableSimultaneous, long &pmt1EnableDuringBleach, long &pmt2EnableDuringBleach, long &pmt3EnableDuringBleach, long &pmt4EnableDuringBleach )=0;
	virtual long SetPhotobleaching(long enable, long laserPosition, long durationMS, double powerPosition, long width, long height, long offsetX, long offsetY, long bleachingFrames, long fieldSize, long bleachTrigger, long preBleachingFrames, double preBleachingInterval, long preBleachingStream, long postBleachingFrames1, double postBleachingInterval1, long postBleachingStream1, long postBleachingFrames2, double postBleachingInterval2, long postBleachingStream2, long powerEnable, long laserEnable, long bleachQuery, long bleachPostTrigger, long enableSimultaneous, long pmt1EnableDuringBleach, long pmt2EnableDuringBleach, long pmt3EnableDuringBleach, long pmt4EnableDuringBleach)=0;

	virtual long GetCaptureMode(long &mode)=0;///capture mode selection
	virtual long SetCaptureMode(long mode)=0;

	virtual long GetLightPath(long &galvoGalvo, long &galvoRes, long &camera, long &invertedLightPathPos)=0;
	virtual long SetLightPath(long galvoGalvo, long galvoRes, long camera, long invertedLightPathPos)=0;

	virtual long GetCaptureSequence(long &enable)=0;
	virtual long GetSequenceSteps(vector<SequenceStep> &captureSequenceSettings)=0;

	virtual long GetSpectralFilter(long &startWavelength, long &stopWavelength, long &stepsSizeWavelength, long &bandwidthMode, string &path)=0;
	virtual long SetSpectralFilter(long startWavelength, long stopWavelength, long stepsSizeWavelength, long bandwidthMode, string path)=0;

	virtual long GetLampLED(long& led1enable, double& led1power, long& led2enable, double& led2power, long& led3enable, double& led3power, long& led4enable, double& led4power, long& led5enable, double& led5power, long& led6enable, double& led6power) = 0;

	virtual long GetSampleRegions(long &viewMode, vector<SampleRegion> &sregions)=0;
	virtual long GetScanRegions(long &viewMode, vector<ScanRegion> &sregions)=0;
	virtual long GetScanAttribute(long viewMode, string attribute, double &value)=0;
	virtual long SetAllScanAreas(long viewMode, long enable)=0;

	virtual void Update()=0;
};
#pragma once

class ExperimentXML : public IExperiment
{
private:
	auto_ptr<ticpp::Document> _xmlObj;
	string _currentPathAndFile;
public:
	static const char * const ROOT_TAG;
	static const char * const ZFILEPOSTAG;

	static const char * const NAME;
	enum {NUM_NAME_ATTRIBUTES = 1};
	static const char * const NAME_ATTR[NUM_NAME_ATTRIBUTES];

	static const char * const DATE;
	enum {NUM_DATE_ATTRIBUTES = 1};
	static const char * const DATE_ATTR[NUM_DATE_ATTRIBUTES];

	static const char * const USER;
	enum {NUM_USER_ATTRIBUTES = 1};
	static const char * const USER_ATTR[NUM_USER_ATTRIBUTES];

	static const char * const COMPUTER;
	enum {NUM_COMPUTER_ATTRIBUTES = 1};
	static const char * const COMPUTER_ATTR[NUM_COMPUTER_ATTRIBUTES];

	static const char * const SOFTWARE;
	enum {NUM_SOFTWARE_ATTRIBUTES = 1};
	static const char * const SOFTWARE_ATTR[NUM_SOFTWARE_ATTRIBUTES];

	static const char * const MODALITY;
	enum {NUM_MODALITY_ATTRIBUTES = 3};
	static const char * const MODALITY_ATTR[NUM_MODALITY_ATTRIBUTES];

	static const char * const CAMERA;
	enum {NUM_CAMERA_ATTRIBUTES = 22};
	static const char * const CAMERA_ATTR[NUM_CAMERA_ATTRIBUTES];

	static const char * const LSM;
	enum {NUM_LSM_ATTRIBUTES = 33};
	static const char * const LSM_ATTR[NUM_LSM_ATTRIBUTES];

	static const char * const RAW;
	enum {NUM_RAW_ATTRIBUTES = 1};
	static const char * const RAW_ATTR[NUM_RAW_ATTRIBUTES];

	static const char * const STREAMING;
	enum {NUM_STREAMING_ATTRIBUTES = 19};
	static const char * const STREAMING_ATTR[NUM_STREAMING_ATTRIBUTES];

	static const char * const PMT;
	enum {NUM_PMT_ATTRIBUTES = 16};
	static const char * const PMT_ATTR[NUM_PMT_ATTRIBUTES];

	static const char * const MCLS;
	enum {NUM_MCLS_ATTRIBUTES = 8};
	static const char * const MCLS_ATTR[NUM_MCLS_ATTRIBUTES];

	static const char * const MAGNIFICATION;
	enum {NUM_MAGNIFICATION_ATTRIBUTES = 2};
	static const char * const MAGNIFICATION_ATTR[NUM_MAGNIFICATION_ATTRIBUTES];

	static const char * const PINHOLEWHEEL;
	enum {NUM_PINHOLEWHEEL_ATTRIBUTES = 1};
	static const char * const PINHOLEWHEEL_ATTR[NUM_PINHOLEWHEEL_ATTRIBUTES];

	static const char * const WAVELENGTHS;
	static const char * const WAVELENGTH;
	enum {NUM_WAVELENGTH_ATTRIBUTES = 2};
	static const char * const WAVELENGTH_ATTR[NUM_WAVELENGTH_ATTRIBUTES];

	static const char * const ZSTAGE;
	enum {NUM_ZSTAGE_ATTRIBUTES = 7};
	static const char * const ZSTAGE_ATTR[NUM_ZSTAGE_ATTRIBUTES]; 

	static const char * const ZSTREAM;	
	enum {NUM_ZSTREAM_ATTRIBUTES = 2};
	static const char * const ZSTREAM_ATTR[NUM_ZSTREAM_ATTRIBUTES]; 

	static const char * const TIMELAPSE;
	enum {NUM_TIMELAPSE_ATTRIBUTES = 3};
	static const char * const TIMELAPSE_ATTR[NUM_TIMELAPSE_ATTRIBUTES];

	enum {NUM_TIMELAPSE_T_OFFSET_ATTRIBUTES = 1};
	static const char * const TIMELAPSE_T_OFFSET_ATTR[NUM_TIMELAPSE_T_OFFSET_ATTRIBUTES]; 

	static const char * const SAMPLE;
	enum {NUM_SAMPLE_ATTRIBUTES = 13};
	static const char * const SAMPLE_ATTR[NUM_SAMPLE_ATTRIBUTES]; 
	enum {NUM_SAMPLEINFO_ATTRIBUTES = 15};
	static const char * const SAMPLEINFO_ATTR[NUM_SAMPLEINFO_ATTRIBUTES];

	static const char * const WELLS;
	enum {NUM_WELLS_ATTRIBUTES = 6};
	static const char * const WELLS_ATTR[NUM_WELLS_ATTRIBUTES];

	static const char * const SUBIMAGES;
	enum {NUM_SUBIMAGES_ATTRIBUTES = 8};
	static const char * const SUBIMAGES_ATTR[NUM_SUBIMAGES_ATTRIBUTES];

	static const char * const COMMENTS;
	enum {NUM_COMMENTS_ATTRIBUTES = 1};
	static const char * const COMMENTS_ATTR[NUM_COMMENTS_ATTRIBUTES];

	static const char * const AUTOFOCUS;
	enum {NUM_AUTOFOCUS_ATTRIBUTES = 6};
	static const char * const AUTOFOCUS_ATTR[NUM_AUTOFOCUS_ATTRIBUTES];

	static const char * const POWER;
	static const char * const POWER2;
	enum {NUM_POWER_ATTRIBUTES = 6};
	static const char * const POWER_ATTR[NUM_POWER_ATTRIBUTES];

	static const char * const POCKELS;
	enum {NUM_POCKELS_ATTRIBUTES = 5};
	static const char * const POCKELS_ATTR[NUM_POCKELS_ATTRIBUTES];
	enum {NUM_POCKELSMASK_ATTRIBUTES = 3};
	static const char * const POCKELSMASK_ATTR[NUM_POCKELSMASK_ATTRIBUTES];

	static const char * const IMAGECORRECTION;
	enum {NUM_IMAGECORRECTION_ATTRIBUTES = 8};
	static const char * const IMAGECORRECTION_ATTR[NUM_IMAGECORRECTION_ATTRIBUTES];

	static const char * const MULTIPHOTONLASER;
	enum {NUM_MULTIPHOTONLASER_ATTRIBUTES = 5};
	static const char * const MULTIPHOTONLASER_ATTR[NUM_MULTIPHOTONLASER_ATTRIBUTES];

	static const char * const PHOTOBLEACHING;
	enum {NUM_PHOTOBLEACHING_ATTRIBUTES = 29};
	static const char * const PHOTOBLEACHING_ATTR[NUM_PHOTOBLEACHING_ATTRIBUTES];

	static const char * const CAPTUREMODE;
	enum {NUM_CAPTUREMODE_ATTRIBUTES = 1};
	static const char * const CAPTUREMODE_ATTR[NUM_CAPTUREMODE_ATTRIBUTES];

	static const char* const LAMP;
	enum { NUM_LAMP_ATTRIBUTES = 19 };
	static const char* const LAMP_ATTR[NUM_LAMP_ATTRIBUTES];

	static const char * const LIGHTPATH;
	enum {NUM_LIGHTPATH_ATTRIBUTES = 5};
	static const char * const LIGHTPATH_ATTR[NUM_LIGHTPATH_ATTRIBUTES];

	static const char * const CAPTURESEQUENCE;
	enum {NUM_CAPTURESEQUENCE_ATTRIBUTES = 1};
	static const char * const CAPTURESEQUENCE_ATTR[NUM_CAPTURESEQUENCE_ATTRIBUTES];

	static const char * const SEQUENCESTEP;
	enum {NUM_SEQUENCESTEP_PMT_ATTRIBUTES = 8};
	static const char * const SEQUENCESTEP_PMT_ATTR[NUM_SEQUENCESTEP_PMT_ATTRIBUTES];

	enum {NUM_SEQUENCESTEP_LSM_ATTRIBUTES = 1};
	static const char * const SEQUENCESTEP_LSM_ATTR[NUM_SEQUENCESTEP_LSM_ATTRIBUTES];

	enum {NUM_SEQUENCESTEP_MULTIPHOTON_ATTRIBUTES = 1};
	static const char * const SEQUENCESTEP_MULTIPHOTON_ATTR[NUM_SEQUENCESTEP_MULTIPHOTON_ATTRIBUTES];

	enum {NUM_SEQUENCESTEP_MCLS_ATTRIBUTES = 8};
	static const char * const SEQUENCESTEP_MCLS_ATTR[NUM_SEQUENCESTEP_MCLS_ATTRIBUTES];

	enum {NUM_SEQUENCESTEP_PINHOLE_ATTRIBUTES = 1};
	static const char * const SEQUENCESTEP_PINHOLE_ATTR[NUM_SEQUENCESTEP_PINHOLE_ATTRIBUTES];

	static const char * const SEQUENCESTEP_WAVELENGTHS;
	enum {NUM_SEQUENCESTEP_WAVELENGTH_ATTRIBUTES = 2};
	static const char * const SEQUENCESTEP_WAVELENGTH_ATTR[NUM_SEQUENCESTEP_WAVELENGTH_ATTRIBUTES];

	static const char * const SPECTRALFILTER;
	enum {NUM_SPECTRALFILTER_ATTRIBUTES = 5};
	static const char * const SPECTRALFILTER_ATTR[NUM_SPECTRALFILTER_ATTRIBUTES];

	static const char* const EPITURRET;
	enum { NUM_EPITURRET_ATTRIBUTES = 2 };
	static const char* const EPITURRET_ATTR[NUM_EPITURRET_ATTRIBUTES];

	enum {NUM_TEMPLATE_LEVELS = 4};
	static const char * const TEMPLATE_NODES[NUM_TEMPLATE_LEVELS];

	ExperimentXML();
	~ExperimentXML();

	wstring GetPathAndName();///Experiment file path

	long GetName(string &name);///Experiment Name
	long SetName(string name); 

	long GetDate(string &name);///Creation Date
	long SetDate(string name);

	long GetUser(string &name);///User name that created the experiment
	long SetUser(string name); 

	long GetComputer(string &name);///computer name the experiment was created on
	long SetComputer(string name);

	long GetSoftware(double &version);///software version the experiment was created on
	long SetSoftware(double version);

	long GetModality(long& cameraType,string &modality,long& lsmType);

	long GetImageArea(long cameraType,long lsmtype,long &pixelX,long &pixelY,double &pixelSizeUM);
	long GetCamera(string &name,long &width,long &height,double &pixelSizeUM,double &exposureTimeMS, long &gain, long &blackLevel, long &lightMode, long &left, long &top, long &right, long &bottom, long &binningX, long &binningY, long &tapsIndex, long &tapsBalance, long &readOutSpeedIndex, long &averageMode, long &averageNum, long &verticalFlip, long &horizontalFlip, long &imageAngle);///camera information
	long SetCamera(string name, long width, long height, double pixelSizeUM, long binning, long gain, long lightMode);

	long GetLSM(long& areaMode, double& areaAngle, long& scanMode, long& interleave, long& pixelX, long& pixelY, long& channel, long& fieldSize, long& offsetX, long& offsetY,
		long& averageMode, long& averageNum, long& clockSource, long& inputRange1, long& inputRange2, long& twoWayAlignment, long& extClockRate, double& dwellTime,
		long& flybackCycles, long& inputRange3, long& inputRange4, long& minimizeFlybackCycles, long& polarity1, long& polarity2, long& polarity3, long& polarity4,
		long& verticalFlip, long& horizontalFlip, double& crsFrequencyHz, long& timeBasedLineScan, long& timeBasedLSTimeMS, long& threePhotonEnable, long& numberOfPlanes);
	long SetLSM(long areaMode, double areaAngle,long scanMode,long interleave,long pixelX,long pixelY,long channel, long fieldSize, long offsetX, long offsetY,
		long averageMode, long averageNum, long clockSource, long inputRange1, long inputRange2, long twoWayAlignment, long extClockRate,double dwellTime, long flybackCycles, long inputRange3, long inputRange4, long minimizeFlybackCycles, long polarity1, long polarity2, long polarity3, long polarity4, long verticalFlip, long horizontalFlip);

	long GetMCLS(long &enable1, double &power1,long &enable2, double &power2,long &enable3, double &power3,long &enable4, double &power4);
	long SetMCLS(long enable1, double power1,long enable2, double power2,long enable3, double power3,long enable4, double power4);

	long GetPMT(long &enableA, long &gainA, long &bandwidthA, double &offsetA, long &enableB, long &bandwidthB, long &gainB, double &offsetB, long &enableC, long &gainC, long &bandwidthC, double &offsetC, long &enableD, long &gainD, long &bandwidthD, double &offsetD);
	long SetPMT(long enableA, long gainA, long bandwidthA, double offsetA, long enableB, long gainB, long bandwidthB, double offsetB, long enableC, long gainC, long bandwidthC, double offsetC, long enableD, long gainD, long bandwidthD, double offsetD);

	long GetPinholeWheel(long &positiion);///pinhole position value
	long SetPinholeWheel(long position);

	long GetRaw(long &onlyEnabledChannels);///flag to acquire enabled channel only for saving space and expediting exp

	long GetStreaming(long& enable, long& frames, long& rawData, long& triggerMode, long& displayImage, long& storageMode, long& zFastEnable, long& zFastMode, long& flybackFrames, long& flybackLines, double& flybackTimeAdjustMS, double& volumeTimeAdjustMS, double& stepTimeAdjustMS, long& previewIndex, long& stimulusTriggering, long& dmaFrames, long& stimulusMaxFrames, long& useReferenceVoltageForPockels, long& displayRollingAveragePreview);
	long SetStreaming(long enable,long frames, long rawData, long triggerMode, long displayImage, long storageMode, long zFastEnable, long flybackFrames, long stimulusTriggering,long dmaFrames, long stimulusMaxFrames, long previewIndex, long useReferenceVoltageForPockels);

	long GetMagnification(double &mag, string &name);///magnification value
	long SetMagnification(double mag, string name);

	long GetNumberOfWavelengths();///number of wavelengths imaged in the experiment
	long GetChannelEnableSet();///number of enabled wavelengths in the experiment, in bitmask: 1:ChA, 2:ChB, 4:ChC, 8:ChD
	long GetWavelength(long index,string &name,double &exposureTimeMS);///wavelength details
	long SetWavelength(long index,string name,double exposureTimeMS);
	long AddWavelength(string name,double exposureTimeMS);
	long RemoveWavelength(string name);
	long RemoveAllWavelengths();

	long GetZStage(string &name, long &enable, long &steps, double &stepSize, double &startPos, long &zStreamFrames, long &zStreamMode);	// zStreamFrames: number of frames per Z stack stream 
	long SetZStage(string name, long enable, long steps, double stepSize, double startPos, long zStreamFrames, long zStreamMode);

	long GetTimelapse(long &timepoints,double &intervalSec,long &triggerMode);///Timelapse details
	long SetTimelapse(long timepoints, double intervalSec, long triggerMode);
	long GetTimelapseTOffset(long &t);///Offset for timelapse t;
	long SetTimelapseTOffset(long t);

	//Sample Well dimension details
	long GetSampleInfo(string &name, double &width, double &height, long &row, long &column, double &diameter, double &centerToCenterX, double &centerToCenterY, double &topLeftCenterOffsetX, double &topLeftCenterOffsetY, string &WellShape, double &WellWidth, double &WellHeight, double &initialStageLocationX, double &initialStageLocationY );
	long GetSample(double &offsetX, double &offsetY, double &offsetZ, long &tiltAdjustment, double &fPt1X, double &fPt1Y, double &fPt1Z, double &fPt2X, double &fPt2Y, double &fPt2Z, double &fPt3X, double &fPt3Y, double &fPt3Z );///Sample details
	long SetSample(SampleType type, double offsetX, double offsetY, long tiltAdjustment, double fPt1X, double fPt1Y, double fPt1Z, double fPt2X, double fPt2Y, double fPt2Z, double fPt3X, double fPt3Y, double fPt3Z);

	long GetWells(long &startRow, long &startColumn, long &rows, long &columns, double &wellOffSetXMM, double &wellOffsetYMM);///Well information for sample type "plate"
	long SetWells(long startRow, long startColumn, long rows, long columns, double wellOffSetXMM, double wellOffsetYMM);

	long GetSubImages(vector<SubImage>& subImages, long cameraType, long lsmType);///mosaic details at each well
	long SetSubImages(long subRows,long subColumns, double subOffsetXMM, double subOffsetYMM, double transOffsetXMM, double transOffsetYMM);

	long GetComments(string &name);///user provided comments
	long SetComments(string name);

	long GetAutoFocus(long &type, long &repeat, double &expTimeMS, double &stepSizeUM, double &startPosMM, double &stopPosMM);
	long SetAutoFocus(long type, long repeat, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM);

	long GetPower(long &enable, long &type, double &start, double &stop, string &path, double &zeroOffset, long index);
	long SetPower(long enable, long type, double start, double stop, string path, double zeroOffset, long index);

	long GetPockels(long index, long &type, double &start, double &stop, string &path, long &blankPercent);
	long SetPockels(long index, long type, double start, double stop, string path, long blankPercent);

	long GetPockelsMask(long &enable, long &invert, string &path);
	long SetPockelsMask(long enable, long invert, string path);

	long GetImageCorrection(long &enablePincushion, double &pinCoeff1, double &pinCoeff2, double &pinCoeff3, long &enableBackgroundSubtration, string &pathBackgroundSubtration, long &enableFlatField, string &pathFlatField);
	long SetImageCorrection(long enablePincushion, double pinCoeff1, double pinCoeff2, double pinCoeff3, long enableBackgroundSubtration, string pathBackgroundSubtration, long enableFlatField, string pathFlatField);

	long GetMultiPhotonLaser(long &enable, long &positiion, long &seqEnable, long &seqPos1, long &seqPos2);
	long SetMultiPhotonLaser(long enable, long position, long seqEnable, long seqPos1, long seqPos2);

	long GetPhotoBleachingAttr(char* attrName, double& val);

	long GetPhotobleaching(long &enable, long &laserPositiion, long &durationMS, double &powerPosition, long &width, long &height, long &offsetX, long &offsetY, long &bleachingFrames, long &fieldSize, long &bleachTrigger, long &preBleachingFrames, double &preBleachingInterval, long &preBleachingStream, long &postBleachingFrames1, double &postBleachingInterval1, long &postBleachingStream1, long &postBleachingFrames2, double &postBleachingInterval2, long &postBleachingStream2, long &powerEnable, long &laserEnable, long &bleachQuery,long &bleachpostTrigger, long &enableSimultaneous, long &pmt1EnableDuringBleach, long &pmt2EnableDuringBleach, long &pmt3EnableDuringBleach, long &pmt4EnableDuringBleach);
	long SetPhotobleaching(long enable, long laserPosition, long durationMS, double powerPosition, long width, long height, long offsetX, long offsetY, long bleachingFrames, long fieldSize, long bleachTrigger, long preBleachingFrames, double preBleachingInterval, long preBleachingStream, long postBleachingFrames1, double postBleachingInterval1, long postBleachingStream1, long postBleachingFrames2, double postBleachingInterval2, long postBleachingStream2, long powerEnable, long laserEnable, long bleachQuery,long bleachpostTrigger, long enableSimultaneous, long pmt1EnableDuringBleach, long pmt2EnableDuringBleach, long pmt3EnableDuringBleach, long pmt4EnableDuringBleach);

	long GetCaptureMode(long &mode);
	long SetCaptureMode(long mode);

	long GetLightPath(long& galvoGalvo, long& galvoRes, long& camera, long& invertedLightPathPos, long& ndd);
	long SetLightPath(long galvoGalvo, long galvoRes, long camera, long invertedLightPathPos, long ndd);

	long GetCaptureSequence(long &enable);
	long GetSequenceSteps(vector<SequenceStep> &captureSequenceSettings);

	long GetSpectralFilter(long &startWavelength, long &stopWavelength, long &stepsSizeWavelength, long &bandwidthMode, string &path);
	long SetSpectralFilter(long startWavelength, long stopWavelength, long stepsSizeWavelength, long bandwidthMode, string path);

	long GetEpiTurret(long& position, string& name);
	long SetEpiTurret(long position, string name);

	long GetLampLED(long& led1enable, double& led1power, long& led2enable, double& led2power, long& led3enable, double& led3power, long& led4enable, double& led4power, long& led5enable, double& led5power, long& led6enable, double& led6power, double& mainPower);

	long GetSampleRegions(long &viewMode, vector<SampleRegion> &sregions);
	long GetScanRegions(long &viewMode, vector<ScanRegion> &sregions);
	long GetScanAttribute(long viewMode, string attribute, double &value);
	long SetAllScanAreas(long viewMode, long enable);

	long GetZPosList(vector<double>& vec);
	long GetZFileInfo(int& enable, double& scale);
	long GetZ2LockInfo(int& lockEnable, int& mirrorEnable);

	void Update();

	long GetAttribute(string tagName, string attribute, string &attributeValue);
	long GetAttributeWithChild(ticpp::Iterator<ticpp::Element> child, string tagName, string attribute, string &attributeValue);
	long SetAttribute(string tagName, string attribute, string attributeValue);
	long SetAttribute(string tagParent, string tagName, string attribute, string attributeValue);
	long CreateConfigFile(string path);
	long OpenConfigFile(string path);
	long CreateTag(string parentTag, string tag);
	long GetTagCount(string tagParent, string tagName, long &count);
	long GetElementList(string tagParent, string tagName, vector<double>& vect);
	long Save();
	string GetPath();
};

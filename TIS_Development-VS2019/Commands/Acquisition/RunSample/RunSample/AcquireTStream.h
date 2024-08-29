#pragma once

#include "..\..\..\..\Common\Sample\Sample\Acquire.h"
#include "AcquireSaveInfo.h"
#include "..\..\..\Common\ThorSharedTypesCPP.h"
#include "..\..\..\Common\BinaryImageDataUtilities\RawFile.h"
#include <map>

struct AVPARAMS
{
	long id;
	int avNum;
	long size;
	unsigned short* pMem;
	unsigned long* pSum;
};

enum TempFileHandle
{	
	RESIZE_SINGLE_STIMULUS,
	COMBINE_TWO_STIMULUS,
	CONCATENATE_PREVIOUS_STIMULUS,	
	RENAME_SINGLE_TIS,
	COMBINE_ALL_TIS_TEMP,
	DELETE_ALL_TEMP,
	DELETE_RAW,
	DELETE_TIF,
	DELETE_LEFT_FILETYPE
};

struct FileNameStruc
{
	std::wstring fileName;
	long index;
};

struct ZRangeInfo
{
	long zFramesPerVolume;
	double zStartPosUM;
	double zStopPosUM;
	double zStepSizeUM;
};

class AcquireTStream : public IAcquire
{
	Publisher* publisher;
public:
	AcquireTStream(IExperiment *,wstring path);

	void SetPublisher(Publisher*) override;

	virtual long Execute(long index, long subWell);//Synchrnous acquisition of data
	virtual long Execute(long index, long subWell, long zFrame, long tFrame);//

	virtual long CallCaptureComplete(long captureComplete);
	virtual long CallCaptureImage(long index);
	virtual long CallSaveImage(long index, BOOL isImageUpdate);
	virtual long CallCaptureSubImage(long index);
	virtual long CallSaveSubImage(long index);
	virtual long PreCaptureEventCheck(long &status);
	virtual long StopCaptureEventCheck(long &status);
	virtual long CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5);
	virtual long CallSaveTImage(long index);
	virtual long CallSequenceStepCurrent(long index);
	virtual long CallStartProgressBar(long index, long resetTotalCount = 0);


	static HANDLE hEvent;
	static HANDLE hEventZ;
	static HANDLE hEventAbort;
	static HANDLE hEventSave;
	static HANDLE hEventUnlock;
	static HANDLE hEventCalc;
	static HANDLE hEventMain;

	static LONGLONG _startTime;
	static unsigned long _saveIndex;
	static volatile unsigned long _saveThreadCount;
	static volatile unsigned long _saveThreadCountFinished;
	static BOOL _saveThreadActive;

	class HeaderInfo
	{
	public:
		HeaderInfo();
		HeaderInfo(string headerFilename);
		~HeaderInfo();

		bool parseHeaderInfo(string headerFilename);

		string Description;
		string SensorType;
		string FileType;
		int HeaderOffset;
		int Samples;   // Columns
		int Lines;     // Rows
		int bands;

		int DataType;
		// The type of data representation :
		// 1 = Byte : 8 - bit unsigned integer
		// 2 = Integer : 16 - bit signed integer
		// 3 = Long : 32 - bit signed integer
		// 4 = Floating - point : 32 - bit single - precision
		// 5 = Double - precision : 64 - bit double - precision floating - point
		// 6 = Complex : Real - imaginary pair of single - precision floating - point
		// 9 = Double - precision complex : Real - imaginary pair of double precision floating - point
		// 12 = Unsigned integer : 16 - bit
		// 13 = Unsigned long integer : 32 - bit
		// 14 = 64 - bit long integer(signed)
		// 15 = 64 - bit unsigned long integer(unsigned)
		string Interleave;

		// BSQ, BIL, or BIP
		int ByteOrder;

		// 0 = Little Endian, 1 = Big Endian
		vector<float>* WaveLength;

	private:
		static string trim(string& s);
		void FillProps(string sKey, string sVal);
		void FillWavelength(string wl) const;
		void Init();
	};

	class SaveParams
	{
	public:
		SaveParams() :
			_pHeaderInfo(nullptr)
		{
			_zInfo.zStartPosUM = _zInfo.zStopPosUM = _zInfo.zStepSizeUM = 0;
			_zInfo.zFramesPerVolume = 0;
			imageMethod = 0;
			pMemoryBuffer = NULL;
		}

		SaveParams(SaveParams &src) :
			_pHeaderInfo(nullptr)
		{
			this->path = src.path;
			for(long i=0; i<4; i++)
				this->wavelengthName[i] = src.wavelengthName[i];

			this->index = src.index;
			this->subWell = src.subWell;

			for(long i=0; i<3; i++)
			{
				this->red[i];
				this->green[i];
				this->blue[i];
				this->bp[i];
				this->wp[i];
			}

			this->colorChannels = src.colorChannels;
			this->bufferChannels = src.bufferChannels;
			this->imageIDsMap = src.imageIDsMap;
			this->regionImageIDsMap = src.regionImageIDsMap;
			this->dimensionsMap = src.dimensionsMap;
			this->regionDimensionsMap = src.regionDimensionsMap;
			this->regionMap = src.regionMap;
			this->previewRegionID = src.previewRegionID;
			this->umPerPixel = src.umPerPixel;
			this->firstFrameIndex = src.firstFrameIndex;
			this->numFramesToSave = src.numFramesToSave;
			this->deleteFileOnThreadCompletion = src.deleteFileOnThreadCompletion;
			this->fastFlybackFrames = src.fastFlybackFrames;
			this->avgFrames = src.avgFrames;
			this->useVirtualMemory = src.useVirtualMemory;
			this->lsmChannels = src.lsmChannels;
			this->storageMode = src.storageMode;
			this->hyperSpectralWavelengths = src.hyperSpectralWavelengths;
			this->doCompression = src.doCompression;
			this->fileType = src.fileType;
			this->_zInfo = src._zInfo;
			this->imageMethod = src.imageMethod;
		}

		~SaveParams()
		{
		}

		wstring path;
		string wavelengthName[4];
		long index;
		long subWell;

		long red[3];
		long green[3];
		long blue[3];
		long bp[3];
		long wp[3];

		long colorChannels;
		map<long,long> bufferChannels;				//map of scan area index to buffer channels, default:(0, bufferChannel)
		long lsmChannels;							///bit value of all selected channels
		map<long,long> imageIDsMap;					//map of DFlim BufferType to image index
		map<long, Dimensions> dimensionsMap;		//map of DFlim BufferType to dimension
		map<long,long> regionImageIDsMap;			//map of scan area index to image index
		map<long, Dimensions> regionDimensionsMap;	//map of scan area index to dimension
		map<long,ScanRegion> regionMap;				//map of scan area index (0) to collection of image width, height, (fast)Z frames, (total)T frames
		long previewRegionID;
		double umPerPixel;
		long firstFrameIndex;
		long numFramesToSave;
		long deleteFileOnThreadCompletion;
		long fastFlybackFrames;
		long avgFrames;
		bool useVirtualMemory;
		long storageMode;
		long hyperSpectralWavelengths;
		long imageMethod;							//Regular(0), Dflim(1)
		long fileType;
		char * pMemoryBuffer;
		long doCompression;
		HeaderInfo* _pHeaderInfo;					//used in Hyperspectral capture
		ZRangeInfo _zInfo;
		size_t photonsBufferTotalSize;
	};

	static AcquireSaveInfo* _acquireSaveInfo;

	static string uUIDSetup(SaveParams *sp, long bufferChannels, long timePoints, long zstageSteps, long index, long subWell);
	static long HandleStimStreamRawFile(SaveParams *sp, long ID, std::wstring lhsFilename=std::wstring(), std::wstring rhsFilename=std::wstring(), bool rawContainsDisabledChannels=false);
	static void ReAscendFileNameVec(std::vector<FileNameStruc> &nameStrucVec, std::wstring targetStr, std::wstring fileType);
	static bool SortFileNameStrucByIndex(const FileNameStruc &lhsNameStruc, const FileNameStruc &rhsNameStruc) {return lhsNameStruc.index < rhsNameStruc.index; }

private:
	static vector<pair<long,long> > getActiveChannels(SaveParams& sp);
	static vector<int> getEnabledChannelIndices(SaveParams& sp);
	long CreateSaveThread(SaveParams& sp, char* pMem);
	void ResumeSaveThread(int threadPriority);

	long SetPMT();	
	void SaveData(SaveParams *sp, long imageMode, bool stopCheckAllowed, bool rawContainsDisabledChannels, long subwells);
	long ScannerEnable(long enable);	
	long SaveTimingToExperimentFolder();
	bool saveHeaderHdr(wstring filename, HeaderInfo& hinfo);
	void SavePreviewImage(SaveParams *sp, long tFrame, char * pBuffer, long saveEnabledChannelsOnly);
	void SaveFastZPreviewImageWithoutOME(SaveParams *sp, char *buffer, long bufferSizeBytes);
	long SetupImageData(wstring streamPath, ICamera *pCamera, long averageMode, SaveParams* sp, long zFastEnableGUI, long bufferChannels, double pixelDwellTime, long avgFrames, Dimensions &baseDimensions);
	void SetupSaveParams(long index, long subWell, long streamFrames, double exposureTimeMS, long width, long height, long numberOfPlanes, double umPerPixel, long zFastEnable, long zStageSteps, long zFlybackFrames,long bufferChannels, long lsmChannels, long storageMode, long hyperSpectralWavelengths, long rawData, long previewID, SaveParams *sp);
	void SaveImagesPostCapture(long index, long subWell, long streamFrames, SaveParams *sp, bool rawContainsDisabledChannels, long subwell);
	long SetupZStage(int setupMode, ICamera *pCamera, ZRangeInfo* zRange);
	//void ExtractBufferDetails(ICamera *pCamera,double magnification, double fieldSizeCalibration, long channel,long pixelX,long pixelY, long left, long top, long right, long bottom, long binningX, long binningY, ICamera::CameraType &cameraType, double &umPerPixel, long &bufferChannels);
	long PreCaptureAutoFocus(long index, long subWell, double afStartPos, double afAdaptiveOffset);
	long GetStimulusActive(void);
	void ControlShutterInStream(long zFastEnable);	
	shared_ptr<InternallyStoredImage <unsigned short> > createFullChannelCopy(char* bufferWithOnlyEnabledChannels, SaveParams& sp);
	long SetGetCameraSettings(ICamera* pCamera, long& channel, long& bufferChannels, long& width, long& height, long& avgMode, long& avgNum, double& umPerPixel, long& fieldSize, double& dwellTime, long& numberOfPlanes);;
	long SetCameraTriggerMode(ICamera* pCamera);
	long PrepareAndStartAcquisition(ICamera* pCamera, long& dmaFrames, SaveParams& sp, long zFastEnableGUI, long cameraType, long stimulusTriggering, long captureMode);
	void UnlockImages(map<long, long>& idsMap, long unlockFrameID);
	void DestroyImages(map<long, long>& idsMap);
	void FailedAcquisition(ICamera* pCamera, SaveParams& sp, long zFastEnableGUI);
	long BreakOutWaitCameraStatus(ICamera* pCamera, SaveParams& sp, long& status, double& droppedFrameCnt, long totalFrame, long zFastEnableGUI, long saveEnabledChannelsOnly, long captureMode);
	long CalculateRollingAverage(USHORT* historyBuffer, USHORT* newBuffer, long averageNum, long frameLength);
	long CalculateRollingAverageAndSumDFLIM(USHORT* averageIntensityBuffer, USHORT* sumSinglePhotonBuffer, ULONG32* sumArrivalTimeSumBuffer, ULONG32* sumHistogramBuffer, USHORT* newIntensityBuffer, USHORT* newSinglePhotonBuffer, ULONG32* newArrivalTimeSumBuffer, ULONG32* newHistogramBuffer, long averageNum, long frameLength);

	static BOOL _evenOdd;
	static double _lastGoodFocusPosition;
	static 	vector<long> savedFrameNumVec;
	const static int MAX_CHANNELS = 4;

	IExperiment * _pExp;
	double _adaptiveOffset;
	long _repeat;
	long _counter;
	long _tFrame;
	double _zstageStepSize;
	wstring _path;
	DWORD _lastImageUpdateTime;
	vector<HANDLE> _threadHandles;
	long _hwStimulusActive;
	long _swStimulusActive;
	long _messageID;
	long _triggerMode;
	char* _pMemoryBuffer;
};

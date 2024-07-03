#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include "..\..\thread.h"
#include "..\..\ThorSharedTypesCPP.h" 
#include "..\..\..\Commands\ImageAnalysis\ThorImageProcess\ThorImageProcess\ImageProcessDll.h"
#include "..\..\..\Commands\ImageAnalysis\LineProfile\LineProfile\LineProfile.h"
#include "..\..\RoiDataStore\RoiDataStore\ROIDataStoreDll.h"
#include "..\..\PDLL\pdll.h" 

#if defined(STATS_MANAGER)
#define DllExport_StatsManager __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExport_StatsManager __declspec(dllimport)
#endif

#pragma warning( push )
#pragma warning( disable : 4251 )

typedef void (_cdecl *pushImageProcessDataCallBack)(unsigned short* buffer, unsigned short* rawBuffer, long &width, long &height, long &channels, long &numOfROIs);
typedef void (_cdecl *pushLineProfileCallBack)(double* resultBuffer, long length, long realLength, long channelEnableBinary, long numChannel);
typedef void (_cdecl *pushDFLIMROIHistogramsCallBack)(UINT32* roiHistograms, UINT32 length, UINT32 rois, UINT32 tNum, long channelEnableBinary, UINT32 numChannel);

typedef struct
{
	long width;
	long height;
}Size;

typedef struct
{
	unsigned short *srcBuffer;
	unsigned short *contourBuffer;
	Size RIPsize;
}RIPBuffer;//ROI Image Process Buffer

typedef enum {
	NONE_FLAG,
	OVERLAY_FLAG,
	CONTOUR_FLAG,
	BOTH_FLAG,
}ROISourceFlags;

enum Imageconnectedness{
	Connected_8,
	Connected_4,
};

struct ROIStatistics
{
	//int numROI;
	long *boundRect;
	double *mean;
	double *stdDev;
	double *min;
	double *max;
	double *tbar;
	ROIStatistics(int numROI){
		boundRect = new long[(size_t)numROI*4];
		mean = new double[numROI];
		stdDev = new double[numROI];
		min = new double[numROI];
		max = new double[numROI];
		memset(boundRect,0,(sizeof(long) * numROI*4));
		memset(mean,0,numROI*sizeof(double));
		memset(stdDev,0,numROI*sizeof(double));
		memset(min,0,numROI*sizeof(double));
		memset(max,0,numROI*sizeof(double));
		memset(tbar,0,numROI*sizeof(double));
	};
	ROIStatistics(){
		boundRect = NULL;
		mean = NULL;
		stdDev = NULL;
		min = NULL;
		max = NULL;
		tbar = NULL;
	};
	~ROIStatistics(){
		SAFE_DELETE_ARRAY(boundRect);
		SAFE_DELETE_ARRAY(mean);
		SAFE_DELETE_ARRAY(stdDev);
		SAFE_DELETE_ARRAY(min);
		SAFE_DELETE_ARRAY(max);
		SAFE_DELETE_ARRAY(tbar);
	}
};

class DllExport_StatsManager StatsManager
{
private:
	wstring _appDir;
	wstring _appSettingsDir;
	wstring _captureTemplateDir;
	wstring _zStackCacheDir;

	StatsManager();

	static bool _instanceFlag;
	static bool _setupFlag;
	static auto_ptr<StatsManager> _single;

	HANDLE hThread;	
	BOOL _isThreadActive;
	BOOL _isImageThreadActive;

public:

	~StatsManager();
	static StatsManager* getInstance();	

	//StatsManager();
	static const long MAX_ROI_COUNT = 65535;
	static const long DFLIM_HISTOGRAM_BINS = 256;
	static const long DFLIM_HISTOGRAM_T = 241;//256;//241;
	static const long MAX_STATS_COUNT = 5;	// 5: min max mean stddev tBar
	static const long DFLIM_STATS_COUNT = 1; // 5: min max mean stddev tBar
	static const long MAX_STATSNAME_COUNT = 40;	
	static const size_t MAX_BOUNDRY_COUNT = 4;	
	static const long PAGE_GRANULARITY = 4096;
	static const long STATSLIST_LENGTH = MAX_STATS_COUNT * MAX_CHANNEL_COUNT * MAX_ROI_COUNT * (1+MAX_BOUNDRY_COUNT);	
	static const long STATSLIST_LENGTH_VALLOC = (static_cast<long>(STATSLIST_LENGTH/PAGE_GRANULARITY) + 1) * PAGE_GRANULARITY;

	static long _roiBoundryInfo[MAX_ROI_COUNT*4];	//4: left top width height
	static double _meanVal[MAX_ROI_COUNT];
	static double _stdDevVal[MAX_ROI_COUNT];
	static double _minVal[MAX_ROI_COUNT];
	static double _maxVal[MAX_ROI_COUNT];
	static double _tbarVal[MAX_ROI_COUNT];
	static bool _channelEnable[MAX_CHANNEL_COUNT];
	static long _channelEnableBinary;

	static long _imgWidth;
	static long _imgHeight;
	static BufferType _bufferType;
	static long _mskWidth;
	static long _mskHeight;
	static long _ChannelIdx;
	static long _numChannel;
	static long _numROI;
	static long _includeRegularStats;

	static long _imgBufferLength;
	static long _dflimHistogramLength;
	static long _singleActiveChannelIdx; // channelIdx when there is only one channel enbled
	static long _numOfImageProcessROI;
	static vector<long> _activeROIIdx;

	static map<short, vector<long>> _mskBrief;
	static unsigned short * _pMask;
	static unsigned short * _pData;
	static unsigned short * _pDataDFLIMSinglePhoton;
	static UINT32 * _pDataDFLIMArrivalTimeSum;
	static UINT32 * _pDataDFLIMHistogram;
	static char * _pDataDFLIMPhotonList;
	static unsigned short * _pDataForLine;
	static UINT32 ** _dFLIMXYT;

	static vector<long> _polylinePX;
	static vector<long> _polylinePY;
	static long _lineProfileImgWidth;
	static long _lineProfileImgHeight;
	static long _lineProfileLineWidth;
	static long _lineProfileNumChannel;
	static long _includeLineProfileInStatsCalc;
	static long _lineProfileOn;	
	static long _lineProfilechannelEnableBinary;
	static long _lineIsActive;
	static long _lineProfileImgBufferSize;
	static bool _lineProfileChannelEnable[MAX_CHANNEL_COUNT];
	static long _newImageForLineProfile;
	static size_t _dFLIMPhotonListLength;
	static size_t _dFLIMXYTLength;
	static HANDLE _hThread;
	static HANDLE _hImageThread;
	static HANDLE _hLineProfileThread;

	static long _flagContour;
	static long _ImageProcessWidth;
	static long _ImageProcessHeight;
	static const long MIN_INDEX_ROI_CONTOUR = 1600;//The max number of ROI in OverlayManager is 1600
	static long _ImageProcessChannelSelectedIndex;
	static BOOL _ImageProcessDataReady;
	static RIPBuffer _ripBuffer;
	static ROISourceFlags _roi_source_flag;
	static long UpdateMaskNumROI(long numOfROIContours, vector<long>& roi);
	long _dsType;
	static long _maxRoiNum;
	static long _minSnr;
	static bool _minAreaFilterActive;
	static long _minAreaFilterValue;

	static char *_statsOption[MAX_STATS_COUNT];
	static char *StatsManager::_roiBoundryInfoOption[MAX_BOUNDRY_COUNT];

	static char** _statsNameList;
	static char* _statsNameVirtualBlock;
	static double _statsList[STATSLIST_LENGTH];	// (1+MAX_BOUNDRY_COUNT): stats + boundry info
	static double _tZero[MAX_CHANNEL_COUNT];
	static CritSect _critSect;
	long ComputeStats(unsigned short *data, FrameInfoStruct frameInfo, long channelEnable, long includeLineProfile, long includeRegularStats, long enabledChannelsOnly = FALSE);
	long ComputeContours(unsigned short *data, long width, long height, long channelEnable, long channelSelected);
	long CopyStatsImageDataToLineImageData();
	long SetStatsMask(unsigned short *mask, long width, long height);
	long SetLineProfileLine(long* pX, long* pY, long numberOfPoints, long activeLine);
	long SetLineProfileLineWidth(long lineWidth);
	long SetTZero(double tZero, long channelIndex);
	long IsStatsComplete();
	long StatsManager::CreateStatsManagerROIDS(long dsType, char* pathAndName);
	long StatsManager::Init();
	long StatsManager::TearDown();
	long StatsManager::GetMaskNumROI(unsigned short* mask, long width, long height);
	void StatsManager::GetChannelEnable(long channelEnable);
	static void StatsManager::BuildStatsNameList(long channelIdx, long numROI, long listOffset, vector<long> activeROIIdx, BufferType bufferType);
	static void StatsManager::BuildStatsList(long listOffset, long channelIdx, long numROI, long numChannel, BufferType bufferType, double *min, double *max, double *mean, double *stddev, double *tbar);
	static long StatsManager::GetActiveChannelIdx(long channelIdx);
	static long StatsManager::GetLineProfileData(double* &resultBuffer, long &length, long& realLineLength, long &channelEnableBinary, long &nChannel);
	void SetupBuffer(long width, long height);

	static long MergeStats(const long num_ovly, const long num_cntr);
	long Compute2DROIStats(unsigned short * imgData, 
		unsigned short * dFLIMSinglePhotonData, 
		UINT32 * dFLIMArrivalTimeSumData,
		UINT32 * dFLIMHistogram,				
		void* pMskBrief,
		long imgWidth, 
		long imgHeight,
		BufferType bufferType,
		long channel,
		double *meanVal,
		double *minVal,
		double *maxVal,
		double *stdDevVal,
		double *tbarVal);

	long ComputeDFLIMROIHistograms(UINT32 * dFLIMHistogram,
		void* pMskBrief,	
		long numChannel,
		bool channelEnable[MAX_CHANNEL_COUNT],
		UINT32* &roiHistograms,
		UINT32 &rois,
		UINT32 &length
		);

	// image dimension pointer
	void SetImgWidth(long imgWidth);
	void SetImgHeight(long imgHeight);

	long GetImgWidth();
	long GetImgHeight();

	static unsigned short * _ImageProcessData;
	long GetNumROI();

	// data/mask pointer
	unsigned short* GetDataPtr();
	unsigned short* GetMaskPtr();
	void SetDataPtr(unsigned short* dataPtr);
	void SetMaskPtr(unsigned short* maskPtr);

	// stats pointer
	double* GetMinPtr();
	void SetMinPtr(double* minVal);
	double* GetMaxPtr();
	void SetMaxPtr(double* maxVal);
	double* GetMeanPtr();
	void SetMeanPtr(double* meanVal);
	double* GetStdDevPtr();
	void SetStdDevPtr(double* tbarVal);
	double* GetTbarPtr();
	void SetTbarPtr(double* tbarVal);

	long CreateDFLIMXYTObject(char* photonList, size_t photonListSize, long imageWidth, long imageHeight, long numChannel, bool channelEnable[MAX_CHANNEL_COUNT]);

	long GetThreadBusyStatus();
	void SetThreadBusyStatus(long isThreadActive);

	long GetLineProfileThreadBusyStatus();
	void SetLineProdileThreadBusyStatus(long isThreadActive);

	void SetImageThreadBusyStatus(long isThreadActive);
	long GetImageThreadBusyStatus();
	static pushImageProcessDataCallBack _pushImageProcessDataFuncPtr;
	static pushLineProfileCallBack _pushLineProfileFuncPtr;
	static pushDFLIMROIHistogramsCallBack _pushDFLIMROIHistogramsFuncPtr;
	static long InitCallBack(pushImageProcessDataCallBack pushFuncPtr);
	static long InitCallBackLineProfilePush(pushLineProfileCallBack pushLineProfileFuncPtr);
	static long InitCallBackDFLIMROIHistogramsPush(pushDFLIMROIHistogramsCallBack pushDFLIMROIHistogramsFuncPtr);
};

#pragma warning( push )
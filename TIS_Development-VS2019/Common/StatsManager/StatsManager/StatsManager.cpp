
#include "stdafx.h"
#include "StatsManager.h"
#include <iterator>
#define VIRTUAL_ALLOC
#define BITDEPTH 16

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
auto_ptr<ROIDataStoreDll> roiDataStoreDll(new ROIDataStoreDll(L"ROIDataStore.dll"));
auto_ptr<ImageProcessDll> imgProDll(new ImageProcessDll(L"ThorImageProcess.dll"));
auto_ptr<StatsManager> StatsManager::_single(NULL);
wchar_t message[256];

StatsManager::StatsManager()
{
	Init();
}

typedef void (_cdecl *statsPrototype)(long chan, long &numROI, long * min, long *max, double *mean, double *stdDev);

void (*statsFunctionPointer)(long chan, long &numROI, long * min, long *max, double *mean, double *stdDev) = NULL;

template<class T>
int SaveBuffer(char* fileName, T buffer, int width, int height);

bool StatsManager::_instanceFlag = false;

bool StatsManager::_setupFlag = false;

long StatsManager::_roiBoundryInfo[MAX_ROI_COUNT*4]; 
double StatsManager::_meanVal[MAX_ROI_COUNT];
double StatsManager::_stdDevVal[MAX_ROI_COUNT];
double StatsManager::_minVal[MAX_ROI_COUNT];
double StatsManager::_maxVal[MAX_ROI_COUNT];
double StatsManager::_tbarVal[MAX_ROI_COUNT];
bool StatsManager::_channelEnable[MAX_CHANNEL_COUNT];
long StatsManager::_channelEnableBinary = 0;
long StatsManager::_imgWidth;
long StatsManager::_imgHeight;
BufferType StatsManager::_bufferType;
long StatsManager::_mskWidth;
long StatsManager::_mskHeight;
long StatsManager::_ChannelIdx;
long StatsManager::_numChannel;
long StatsManager::_numROI;
long StatsManager::_imgBufferLength;
long StatsManager::_dflimHistogramLength;
long StatsManager::_singleActiveChannelIdx;
long StatsManager::_includeRegularStats = FALSE;

long StatsManager::_flagContour;
HANDLE StatsManager::_hThread = NULL;
HANDLE StatsManager::_hImageThread = NULL;
vector<long> StatsManager::_activeROIIdx;
long StatsManager::_numOfImageProcessROI;
long StatsManager::_ImageProcessChannelSelectedIndex;
BOOL StatsManager::_ImageProcessDataReady;
RIPBuffer StatsManager::_ripBuffer;
ROISourceFlags StatsManager::_roi_source_flag;
long StatsManager::_maxRoiNum;
long StatsManager::_minSnr;
bool StatsManager::_minAreaFilterActive;
long StatsManager::_minAreaFilterValue;

unsigned short * StatsManager::_pMask;
unsigned short * StatsManager::_pData = NULL;
unsigned short * StatsManager::_pDataDFLIMSinglePhoton = NULL;
UINT32 * StatsManager::_pDataDFLIMArrivalTimeSum = NULL;
UINT32 * StatsManager::_pDataDFLIMHistogram = NULL;
char * StatsManager::_pDataDFLIMPhotonList = NULL;
UINT32 ** StatsManager::_dFLIMXYT = NULL;
unsigned short * StatsManager::_ImageProcessData;
long StatsManager::_ImageProcessWidth;
long StatsManager::_ImageProcessHeight;

char *StatsManager::_statsOption[MAX_STATS_COUNT] = {"min", "max", "mean", "stddev", "tbar"};
char *StatsManager::_roiBoundryInfoOption[MAX_BOUNDRY_COUNT] = {"left", "top", "width", "height"};

char **StatsManager::_statsNameList;
char *StatsManager::_statsNameVirtualBlock;
double StatsManager::_statsList[STATSLIST_LENGTH];

pushImageProcessDataCallBack StatsManager::_pushImageProcessDataFuncPtr;
pushLineProfileCallBack StatsManager::_pushLineProfileFuncPtr;
pushDFLIMROIHistogramsCallBack StatsManager::_pushDFLIMROIHistogramsFuncPtr;

long StatsManager::_lineProfileImgHeight = 0;
long StatsManager::_lineProfileImgWidth = 0;
long StatsManager::_lineProfileLineWidth = 1;
long StatsManager::_lineProfileNumChannel = 0;
long StatsManager::_lineP1X = 0;
long StatsManager::_lineP1Y = 0;
long StatsManager::_lineP2X = 0;
long StatsManager::_lineP2Y = 0;
long StatsManager::_includeLineProfileInStatsCalc = FALSE;
long StatsManager::_lineIsActive = FALSE;
long StatsManager::_lineProfileImgBufferSize = 0;
long StatsManager::_lineProfilechannelEnableBinary = 0;
bool StatsManager::_lineProfileChannelEnable[MAX_CHANNEL_COUNT];
unsigned short* StatsManager::_pDataForLine = NULL;
long StatsManager::_newImageForLineProfile = TRUE;
size_t StatsManager::_dFLIMPhotonListLength = 0;
size_t StatsManager::_dFLIMXYTLength = 0;

double StatsManager::_tZero[MAX_CHANNEL_COUNT];

map<short, vector<long>> StatsManager::_mskBrief;
CritSect StatsManager::_critSect;

StatsManager::~StatsManager()
{
	TearDown();
}

long StatsManager::CreateStatsManagerROIDS(long dsType, char* pathAndName)
{
	long ret = TRUE;
	_dsType = dsType;
	roiDataStoreDll->CreateROIDataStore(dsType, pathAndName);
	return ret;
}

long StatsManager::Init()
{
	//Lock lock(critSect);

	SetThreadBusyStatus(FALSE);
	SetDataPtr(NULL);
	SetMaskPtr(NULL);
	hThread = NULL;
	_numROI = 0;
	_numOfImageProcessROI = 0;
	_dsType = -1;
	_imgBufferLength = 0;
	_dflimHistogramLength = 0;
	_statsNameList = NULL;
	_statsNameVirtualBlock = NULL;
	_isImageThreadActive = FALSE;
	_flagContour = FALSE;
	_pushImageProcessDataFuncPtr = NULL;
	_pushLineProfileFuncPtr = NULL;
	_pushDFLIMROIHistogramsFuncPtr = NULL;
	_ImageProcessWidth = 0;
	_ImageProcessHeight = 0;
	_ImageProcessChannelSelectedIndex = 0;
	_ImageProcessDataReady = FALSE;
	_ripBuffer.contourBuffer = NULL;
	_ripBuffer.srcBuffer = NULL;
	_ripBuffer.RIPsize.width = 0;
	_ripBuffer.RIPsize.height = 0;
	_roi_source_flag = NONE_FLAG;
	_maxRoiNum = 500;
	_minSnr = 15;
	_minAreaFilterActive = false;
	_minAreaFilterValue = 0;
	memset(_tZero, 0, sizeof(double)*MAX_CHANNEL_COUNT);
	return TRUE;
}

long StatsManager::InitCallBack(pushImageProcessDataCallBack pushFuncPtr)
{
	_pushImageProcessDataFuncPtr = pushFuncPtr;

	if(_pushImageProcessDataFuncPtr != NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"InitCallBack");
	}
	return TRUE;
}

long StatsManager::InitCallBackLineProfilePush(pushLineProfileCallBack pushLineProfileFuncPtr)
{
	_pushLineProfileFuncPtr = pushLineProfileFuncPtr;
	if(_pushLineProfileFuncPtr != NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"InitCallBackLineProfilePush");
	}
	else
	{
		//clear history when null callback
		_lineProfileImgWidth = _lineProfileImgHeight = 0;
	}
	return TRUE;
}

long StatsManager::InitCallBackDFLIMROIHistogramsPush(pushDFLIMROIHistogramsCallBack pushDFLIMROIHistogramsFuncPtr)
{
	_pushDFLIMROIHistogramsFuncPtr = pushDFLIMROIHistogramsFuncPtr;

	if(_pushDFLIMROIHistogramsFuncPtr != NULL)
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"InitCallBackDFLIMROIHistogramsPush");
	}
	return TRUE;
}

long StatsManager::TearDown()
{
	if (NULL != _pMask)
	{
		delete[] _pMask;
		_pMask = NULL;
	}

	if (NULL != _pData)
	{
		delete[] _pData;
	}

	if(NULL != _hThread)
	{
		CloseHandle(_hThread);
		_hThread = NULL;
	}

	if (NULL != _ImageProcessData)
	{
		delete[] _ImageProcessData;
	}

	if (NULL != _ripBuffer.contourBuffer)
	{
		delete[] _ripBuffer.contourBuffer;
	}

	if (NULL != _ripBuffer.srcBuffer)
	{
		delete[] _ripBuffer.srcBuffer;
	}

	if(NULL != _hImageThread)
	{
		CloseHandle(_hImageThread);
		_hImageThread = NULL;
	}

	if (NULL != _statsNameList)
	{

#ifdef VIRTUAL_ALLOC
		//VirtualFree(_statsNameVirtualBlock,MAX_STATSNAME_COUNT*STATSLIST_LENGTH,MEM_DECOMMIT);
		VirtualFree(_statsNameVirtualBlock,0,MEM_RELEASE);
		_statsNameVirtualBlock = NULL;
#endif
		for (long n = 0; n < STATSLIST_LENGTH; n++ )
		{
			if (NULL != _statsNameList[n])
			{
#ifdef VIRTUAL_ALLOC

#else
				delete[] _statsNameList[n];
#endif
				_statsNameList[n] = NULL;
			}
		}

#ifdef VIRTUAL_ALLOC
		//VirtualFree(_statsNameList,STATSLIST_LENGTH,MEM_DECOMMIT);
		VirtualFree(_statsNameList,0,MEM_RELEASE);
#else
		delete[] _statsNameList;
#endif

		_statsNameList = NULL;
	}

	_instanceFlag = false;

	return TRUE;
}

StatsManager* StatsManager::getInstance()
{
	if(! _instanceFlag)
	{
		try
		{
			_single.reset(new StatsManager());
			_instanceFlag = true;
			wsprintf(message,L"StatsManager Created");
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
	}
	return _single.get();
}

const long MSG_SIZE = 256;
wchar_t statsManagerMsg[MSG_SIZE];

long InitStatsCallBack(statsPrototype dm) 
{
	//this function is called by the IDataStore that will receive the 
	//stats results

	statsFunctionPointer = dm;

	if(statsFunctionPointer != NULL)
	{
		//unable to assign Stats callback
	}

	return TRUE;
}

long StatsManager::IsStatsComplete()
	//long IsStatsComplete()
{
	//This function is to only be used when you need to do
	//the computation in synchrnous mode	
	if(GetThreadBusyStatus())
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/* 
list ordering:
ex: roi 1, 2, and 3, channel a and c
naming statsname_channelidx_roiidx

min_1_1
min_1_2
min_1_3
max_1_1
max_1_2
max_1_3
mean_1_1
mean_1_2
mean_1_3
stddev_1_1
stddev_1_2
stddev_1_3
tbar_1_1
tbar_1_2
tbar_1_3

min_2_1
min_2_2
min_2_3
max_2_1
max_2_2
max_2_3
mean_2_1
mean_2_2
mean_2_3
stddev_2_1
stddev_2_2
stddev_2_3
tbar_1_1
tbar_2_2
tbar_3_3
*/

// building stats list, all values in list are numerical
void StatsManager::BuildStatsList(long listOffset, long channelIdx, long numROI, long numChannel, BufferType bufferType, double *min, double *max, double *mean, double *stddev, double *tbar)
{	
	long offset;
	switch (bufferType)
	{
	case BufferType::INTENSITY:
	case BufferType::DFLIM_DIAGNOSTIC:
		{
			offset = listOffset * numROI * (MAX_STATS_COUNT - DFLIM_STATS_COUNT + MAX_BOUNDRY_COUNT);
			memcpy_s(_statsList+offset, sizeof(double)*numROI, min, sizeof(double)*numROI);
			memcpy_s(_statsList+offset+numROI, sizeof(double)*numROI, max,  sizeof(double)*numROI);
			memcpy_s(_statsList+offset+2*numROI, sizeof(double)*numROI, mean, sizeof(double)*numROI);
			memcpy_s(_statsList+offset+3*numROI, sizeof(double)*numROI, stddev, sizeof(double)*numROI);		
			offset += (MAX_STATS_COUNT - DFLIM_STATS_COUNT)*numROI;
		}
		break;
	case BufferType::DFLIM_ALL:
	case BufferType::DFLIM_IMAGE:
		{
			offset = listOffset * numROI * (MAX_STATS_COUNT + MAX_BOUNDRY_COUNT);
			memcpy_s(_statsList+offset, sizeof(double)*numROI, min, sizeof(double)*numROI);
			memcpy_s(_statsList+offset+numROI, sizeof(double)*numROI, max,  sizeof(double)*numROI);
			memcpy_s(_statsList+offset+2*numROI, sizeof(double)*numROI, mean, sizeof(double)*numROI);
			memcpy_s(_statsList+offset+3*numROI, sizeof(double)*numROI, stddev, sizeof(double)*numROI);
			memcpy_s(_statsList+offset+4*numROI, sizeof(double)*numROI, tbar, sizeof(double)*numROI);
			offset += MAX_STATS_COUNT*numROI;
		}
		break;
	default:
		return;
	}

	long copyLength = MAX_BOUNDRY_COUNT * numChannel * numROI;

	double* statList = _statsList + offset;
	
	std::copy(_roiBoundryInfo, _roiBoundryInfo +  MAX_BOUNDRY_COUNT * numChannel * numROI, stdext::checked_array_iterator<double *>(statList, copyLength));

}

// width, height: width and height of image/mask; all entries in the list are string/char* based
void StatsManager::BuildStatsNameList(long channelIdx, long numROI, long listOffset, vector<long> activeROIIdx, BufferType bufferType)
{
	long maxStatsCount = 0;
	long maxStatsNameCount = 0;
	switch (bufferType)
	{
	case BufferType::INTENSITY:
	case BufferType::DFLIM_DIAGNOSTIC:
		{
			maxStatsCount = MAX_STATS_COUNT - DFLIM_STATS_COUNT;
			maxStatsNameCount = MAX_STATSNAME_COUNT - DFLIM_STATS_COUNT * MAX_CHANNEL_COUNT;
		}
		break;
	case BufferType::DFLIM_ALL:
	case BufferType::DFLIM_IMAGE:
		{
			maxStatsCount =  MAX_STATS_COUNT;
			maxStatsNameCount = MAX_STATSNAME_COUNT;
		}
		break;
	default:
		return;
	}

	char* statsName = new char[maxStatsNameCount];
	long offset = listOffset * numROI * (maxStatsCount + MAX_BOUNDRY_COUNT);

	for (long m = 0; m < maxStatsCount; m++)
	{
		for (long n = 0; n < activeROIIdx.size(); n++)
		{			
			sprintf_s(statsName, maxStatsNameCount, "%s_%d_%d", _statsOption[m], channelIdx+1, activeROIIdx[n]);			
			strcpy_s(_statsNameList[offset+m*numROI+n], maxStatsNameCount, statsName);			
		}
	}

	offset += maxStatsCount*numROI;

	for (long m = 0; m < maxStatsCount; m++)
	{
		for (long n = 0; n < activeROIIdx.size(); n++)
		{
			sprintf_s(statsName, maxStatsNameCount, "%s_%d_%d", _roiBoundryInfoOption[m], channelIdx+1, activeROIIdx[n]);				
			strcpy_s(_statsNameList[offset++], maxStatsNameCount, statsName);	
		}
	}

	delete[] statsName;
	statsName = NULL;
}

long StatsManager::GetActiveChannelIdx(long channelIdx)
{
	long activeChannelIdx = 0;

	for (long n = 0; n < channelIdx; n++)
	{
		if (_channelEnable[n])
		{
			activeChannelIdx++;
		}
	}

	return activeChannelIdx;
}


// find num of roi of a given mask; computation complexity O(n^2)
long StatsManager::GetMaskNumROI(unsigned short* mask, long width, long height)
{
	// width, height: width and height of image/mask
	long nROI = 0;
	unsigned short* p = mask;
	bool* found = new bool[MAX_ROI_COUNT]();
	memset(found,0,sizeof(bool)*MAX_ROI_COUNT);

	Lock lock(_critSect);
	_activeROIIdx.clear();
	_mskBrief.clear();
	for (long n = 0; n < height; n++)
	{
		for (long m = 0; m < width; m++)
		{
			if (*p>0)
			{	
				_mskBrief[*p].push_back(n * width + m);
				if(found[*p]==false)
				{
					found[*p] = true;
					_activeROIIdx.push_back(*p);	// get active roi index; ex. 2,4,5
					nROI++;
				}				
			}
			p++;
		}
	}

	int k = 0, s = static_cast<int>(_mskBrief.size());
	for(auto& i : _mskBrief)
	{
		_roiBoundryInfo[k] = width;
		_roiBoundryInfo[k + s] = height;
		_roiBoundryInfo[k + 2 * s] = 0;
		_roiBoundryInfo[k + 3 * s] = 0;
		for(int j = 0; j < i.second.size(); j++)
		{
			long tx = i.second[j] % width;
			long ty = (long)(i.second[j] / width);
			if(_roiBoundryInfo[k] > tx) _roiBoundryInfo[k] = tx;
			if(_roiBoundryInfo[k + s] > ty) _roiBoundryInfo[k + s] = ty;
			if(_roiBoundryInfo[k + 2 * s] < tx) _roiBoundryInfo[k + 2 * s] = tx;
			if(_roiBoundryInfo[k + 3 * s] < ty) _roiBoundryInfo[k + 3 * s] = ty;
		}
		_roiBoundryInfo[k + 2 * s] = _roiBoundryInfo[k + 2 * s] - _roiBoundryInfo[k] + 1;
		_roiBoundryInfo[k + 3 * s] = _roiBoundryInfo[k + 3 * s] - _roiBoundryInfo[k + s] + 1;
		k++;
	}

	sort(_activeROIIdx.begin(), _activeROIIdx.end());

	delete[] found;

	return nROI;
}

long StatsManager::UpdateMaskNumROI(long numOfROIContours, vector<long>& roi)
{
	long nROI = _numROI;
	roi.resize(nROI); // initiate with original size
	if (MIN_INDEX_ROI_CONTOUR > (USHRT_MAX -  numOfROIContours))//bounday detection
	{
		return nROI;
	}
	for (int i = MIN_INDEX_ROI_CONTOUR; i < MIN_INDEX_ROI_CONTOUR+numOfROIContours; i++)//add the index of ROIs from contours
	{
		roi.push_back(i);
	}
	return (nROI + numOfROIContours); // return the total nums of ROIs
}

long StatsManager::Compute2DROIStats(unsigned short * imgData, 
									 unsigned short * flimSinglePhotonData, 
									 UINT32 * flimArrivalTimeSumData,
									 UINT32 * flimHistogramData,
									 void* pMskBrief,
									 long imgWidth, 
									 long imgHeight,
									 BufferType bufferType,
									 long channelIdx,
									 double *meanVal,
									 double *minVal,
									 double *maxVal,
									 double *stdDevVal,
									 double *tbarVal)
{
	if (NULL == imgData || NULL == pMskBrief)
	{
		return FALSE;
	}

	if ((BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType) && 
		(NULL == flimSinglePhotonData || NULL == flimArrivalTimeSumData || NULL == flimHistogramData))
	{
		return FALSE;
	}

	Lock lock(_critSect);
	map<short, vector<long>>* pMsk = &StatsManager::_mskBrief;
	double binDuration = 0;
	if ((BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType) && flimHistogramData[254] != 0)
	{
		binDuration = 5.0 * static_cast<double>(flimHistogramData[255]) / 128.0 / static_cast<double>(flimHistogramData[254]);
	}	
	
	int k = 0;	
	for(map<short, vector<long>>::iterator i = pMsk->begin(); i != pMsk->end(); i++)
	{
		double min = 0xFFFF;
		double max = 0;
		double mean = 0;
		double stdDev = 0;
		double N = 0;
		double T = 0;
		int j;
		for(j = 0; j < i->second.size(); j++)
		{
			double oldM = mean;
			mean += (imgData[(i->second)[j]] - mean) / (j + 1);
			stdDev += (imgData[(i->second)[j]] - mean) * (imgData[(i->second)[j]] - oldM);
			if(imgData[(i->second)[j]] > max) max = imgData[(i->second)[j]];
			if(imgData[(i->second)[j]] < min) min = imgData[(i->second)[j]];
			if (BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType)
			{
				N += flimSinglePhotonData[(i->second)[j]];
				T += flimArrivalTimeSumData[(i->second)[j]];
			}
		}
		minVal[k] = min;
		maxVal[k] = max;
		meanVal[k] = mean;
		stdDevVal[k] = sqrt(stdDev / (j - 1));
		
		if (N != 0)
		{
			tbarVal[k] = binDuration*T / N - _tZero[channelIdx];
		}
		else
		{
			tbarVal[k] = 0;
		}

		k++;
	}

	return TRUE;
}

long StatsManager::MergeStats(const long num_ovly, const long num_cntr)
{
	if(num_cntr)
	{
		long s = num_cntr * sizeof(double);
		double* tmp = (double*)malloc(s);

		//merge into mean
		memcpy_s(tmp, s, _meanVal + StatsManager::MIN_INDEX_ROI_CONTOUR, s);
		memcpy_s(_meanVal + num_ovly, s, tmp, s);

		//merge into min
		memcpy_s(tmp, s, _minVal + StatsManager::MIN_INDEX_ROI_CONTOUR, s);
		memcpy_s(_minVal + num_ovly,  s, tmp, s);

		//merge into max
		memcpy_s(tmp, s, _minVal + StatsManager::MIN_INDEX_ROI_CONTOUR, s);
		memcpy_s(_maxVal + num_ovly, s, tmp, s);
		//merge into stdDev
		memcpy_s(tmp, s, _stdDevVal + StatsManager::MIN_INDEX_ROI_CONTOUR, s);
		memcpy_s(_stdDevVal + num_ovly, s, tmp, s);
	}

	return num_ovly + num_cntr;
}

UINT StatsThreadProc( LPVOID pParam )
{	
	StatsManager* paramPtr = (StatsManager*)pParam;
	const ROISourceFlags roi_source_flag = StatsManager::_roi_source_flag;//0 NONE; 1 OVERLAY; 2 CONTOUR; 3 BOTH
	if (roi_source_flag & ROISourceFlags::CONTOUR_FLAG)
	{
		while(StatsManager::_ImageProcessDataReady == FALSE){
		}
	}

	const long num_roi_overlay = StatsManager::_numROI;
	const long num_roi_contour = StatsManager::_numOfImageProcessROI;
	long num_total_roi = num_roi_contour + num_roi_overlay;
	if (num_total_roi == 0)//if the number of ROIs == 0 then send NULL for data and 0 for length
	{
		roiDataStoreDll->LoadROIData( NULL, NULL, 0, 0); 
	}
	else
	{
		const long numChannel = StatsManager::_numChannel;
		const long imgWidth   = StatsManager::_imgWidth;
		const long imgHeight  = StatsManager::_imgHeight;
		const BufferType bufferType  = StatsManager::_bufferType;
		if (TRUE == StatsManager::_includeLineProfileInStatsCalc && NULL != (*(StatsManager::_pushLineProfileFuncPtr)))
		{			
			if (TRUE == StatsManager::_lineIsActive)
			{
				Lock lock(StatsManager::_critSect);

				//Always copy the data to the LineImageBuffer
				paramPtr->CopyStatsImageDataToLineImageData();

				long chanEnableBin = 0;
				double* lineProfileBuffer = NULL;
				long lineProfileLength = 0;				
				long nChannel = 0;

				StatsManager::GetLineProfileData(lineProfileBuffer,lineProfileLength, chanEnableBin, nChannel);
				if (NULL != lineProfileBuffer)
					(*(StatsManager::_pushLineProfileFuncPtr))(lineProfileBuffer,lineProfileLength,chanEnableBin, nChannel);			
				SAFE_DELETE_MEMORY(lineProfileBuffer);
			}
		}

		if (TRUE == StatsManager::_includeRegularStats)
		{
			vector<long> activeROIIdx;

			if (0 < StatsManager::_activeROIIdx.size())
			{
				Lock lock(StatsManager::_critSect);
				activeROIIdx = StatsManager::_activeROIIdx;// Active ROI index array
			}
			else
			{
				StatsManager::_roi_source_flag = ROISourceFlags::NONE_FLAG;
				StatsManager::_ImageProcessDataReady = FALSE;
				((StatsManager*)pParam)->SetThreadBusyStatus(FALSE);
				return 0;
			}

			long listOffset = 0;
			long channelIdx = 0; 

			USHORT * pData =  NULL;
			USHORT * pFLIMSinglePhotonData = NULL;
			UINT32 * pDataFLIMArrivalTimeSum = NULL;
			UINT32 * pDataFLIMHistogram= NULL;

			ROIStatistics *roi_contour = NULL; 
			bool bOverlay = num_roi_overlay && ( roi_source_flag & ROISourceFlags::OVERLAY_FLAG);
			bool bContour = num_roi_contour && (roi_source_flag & ROISourceFlags::CONTOUR_FLAG);

			if (bContour)
			{
				roi_contour = new ROIStatistics(num_roi_contour);
			}

			if (BufferType::DFLIM_ALL == bufferType)
			{					
				paramPtr->CreateDFLIMXYTObject(StatsManager::_pDataDFLIMPhotonList, StatsManager::_dFLIMPhotonListLength, StatsManager::_imgWidth, StatsManager::_imgHeight, StatsManager::_numChannel, StatsManager::_channelEnable);
					
				UINT32* roiHistograms = NULL;
				UINT32 roiNum = 0;
				UINT32 length = 0;
				paramPtr->ComputeDFLIMROIHistograms(StatsManager::_pDataDFLIMHistogram, &StatsManager::_mskBrief, StatsManager::_numChannel, StatsManager::_channelEnable, roiHistograms, roiNum, length);
				if (NULL != StatsManager::_pushDFLIMROIHistogramsFuncPtr && NULL != roiHistograms)
				{
					(*(StatsManager::_pushDFLIMROIHistogramsFuncPtr))(roiHistograms, length, StatsManager::_numROI, StatsManager::DFLIM_HISTOGRAM_BINS, StatsManager::_channelEnableBinary, StatsManager::_numChannel);
							
					size_t roiNum = StatsManager::_mskBrief.size();

					SAFE_DELETE_ARRAY(roiHistograms);
				}						
			}

			if (numChannel > 1)
			{	
				listOffset = 0;
				for( long n = 0; n < MAX_CHANNEL_COUNT; n++)
				{
					if (StatsManager::_channelEnable[n])	
					{		
						channelIdx = n;

						pData = StatsManager::_pData + n*imgWidth*imgHeight;
						if (BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType)
						{
							pFLIMSinglePhotonData = StatsManager::_pDataDFLIMSinglePhoton + n*imgWidth*imgHeight;
							pDataFLIMArrivalTimeSum = StatsManager::_pDataDFLIMArrivalTimeSum + n*imgWidth*imgHeight;
							pDataFLIMHistogram = StatsManager::_pDataDFLIMHistogram + n*StatsManager::DFLIM_HISTOGRAM_BINS;
						}
						if (NULL != pData)
						{
							if (bOverlay)
							{
								paramPtr->Compute2DROIStats(pData,
									pFLIMSinglePhotonData,
									pDataFLIMArrivalTimeSum,
									pDataFLIMHistogram,
									&StatsManager::_mskBrief,
									imgWidth, 
									imgHeight,
									bufferType,
									n,
									StatsManager::_meanVal,									
									StatsManager::_minVal, 
									StatsManager::_maxVal, 
									StatsManager::_stdDevVal,
									StatsManager::_tbarVal);
							}
							if (bContour)
							{
								StatsManager::UpdateMaskNumROI(num_roi_contour,activeROIIdx);
								paramPtr->Compute2DROIStats(pData,
									pFLIMSinglePhotonData,
									pDataFLIMArrivalTimeSum,
									pDataFLIMHistogram,
									/*StatsManager::_ripBuffer.contourBuffer,*/
									NULL,
									imgWidth, 
									imgHeight,
									bufferType,
									n,
									/*roi_contour->boundRect,*/
									StatsManager::_meanVal + paramPtr->MIN_INDEX_ROI_CONTOUR,
									StatsManager::_minVal + paramPtr->MIN_INDEX_ROI_CONTOUR,
									StatsManager::_maxVal + paramPtr->MIN_INDEX_ROI_CONTOUR,
									StatsManager::_stdDevVal + paramPtr->MIN_INDEX_ROI_CONTOUR,
									StatsManager::_tbarVal + paramPtr->MIN_INDEX_ROI_CONTOUR);
							}
							num_total_roi = StatsManager::MergeStats(num_roi_overlay,num_roi_contour);
							StatsManager::BuildStatsNameList(channelIdx, num_total_roi, listOffset, activeROIIdx, bufferType);				

							StatsManager::BuildStatsList( listOffset,
								channelIdx,
								num_total_roi,
								numChannel,
								bufferType,
								StatsManager::_minVal, 
								StatsManager::_maxVal, 
								StatsManager::_meanVal, 
								StatsManager::_stdDevVal,
								StatsManager::_tbarVal);

							listOffset++;
						}
					}
				}
			}
			else if(numChannel == 1)
			{
				channelIdx = StatsManager::_singleActiveChannelIdx;
				listOffset = 0;
				pData = StatsManager::_pData;
				if (BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType)
				{
					pFLIMSinglePhotonData = StatsManager::_pDataDFLIMSinglePhoton;
					pDataFLIMArrivalTimeSum = StatsManager::_pDataDFLIMArrivalTimeSum;
					pDataFLIMHistogram = StatsManager::_pDataDFLIMHistogram;			
				}
				if (NULL != pData)
				{
					if (bOverlay)
					{
						paramPtr->Compute2DROIStats(pData,
							pFLIMSinglePhotonData,
							pDataFLIMArrivalTimeSum,
							pDataFLIMHistogram,
							&StatsManager::_mskBrief,
							imgWidth, 
							imgHeight,
							bufferType,
							channelIdx,
							StatsManager::_meanVal,
							StatsManager::_minVal, 
							StatsManager::_maxVal, 							 
							StatsManager::_stdDevVal,
							StatsManager::_tbarVal);
					}
					if (bContour)
					{
						StatsManager::UpdateMaskNumROI(num_roi_contour,activeROIIdx);
						paramPtr->Compute2DROIStats(pData,
							pFLIMSinglePhotonData,
							pDataFLIMArrivalTimeSum,
							pDataFLIMHistogram,
							/*StatsManager::_ripBuffer.contourBuffer,*/
							NULL,
							imgWidth, 
							imgHeight,
							bufferType,
							channelIdx,
							/*roi_contour->boundRect,*/
							roi_contour->mean,
							roi_contour->min,
							roi_contour->max,
							roi_contour->stdDev,
							roi_contour->tbar);
					}

					num_total_roi = StatsManager::MergeStats(num_roi_overlay, num_roi_contour);

					StatsManager::BuildStatsNameList(channelIdx, num_total_roi, listOffset, activeROIIdx, bufferType);	// data buffer offset is 0;

					StatsManager::BuildStatsList( listOffset,
						channelIdx,
						num_total_roi,
						numChannel,
						bufferType,
						StatsManager::_minVal, 
						StatsManager::_maxVal, 
						StatsManager::_meanVal, 
						StatsManager::_stdDevVal,
						StatsManager::_tbarVal);
				}
			}

			if (bContour)
			{
				delete roi_contour;
			}
			long maxStatsCount= BufferType::DFLIM_IMAGE == bufferType || BufferType::DFLIM_ALL == bufferType ? StatsManager::MAX_STATS_COUNT : StatsManager::MAX_STATS_COUNT - StatsManager::DFLIM_STATS_COUNT;

			const long L = numChannel*(num_total_roi)*(maxStatsCount + StatsManager::MAX_BOUNDRY_COUNT);
			roiDataStoreDll->LoadROIData( StatsManager::_statsNameList, StatsManager::_statsList, L, 0);			
		}
	}
	StatsManager::_roi_source_flag = ROISourceFlags::NONE_FLAG;
	StatsManager::_ImageProcessDataReady = FALSE;

	((StatsManager*)pParam)->SetThreadBusyStatus(FALSE);
	return 0;
}

UINT LineProfileThreadProc( LPVOID pParam )
{
	if (NULL != (*(StatsManager::_pushLineProfileFuncPtr)))
	{
		Lock lock(StatsManager::_critSect);
		if (NULL == StatsManager::_pDataForLine)
		{
			StatsManager::getInstance()->CopyStatsImageDataToLineImageData();
		}

		double* lineProfileBuffer = NULL;
		long lineProfileLength = 0;
		long channelEnableBinary = 0;
		long numChannel = 0;

		StatsManager::GetLineProfileData(lineProfileBuffer,lineProfileLength,channelEnableBinary,numChannel);
		if (NULL != lineProfileBuffer)
			(*(StatsManager::_pushLineProfileFuncPtr))(lineProfileBuffer,lineProfileLength,channelEnableBinary,numChannel);
		SAFE_DELETE_MEMORY(lineProfileBuffer);
	}
	return 0;
}

long ImageProcess(USHORT* srcImagePtr, USHORT width, USHORT height, USHORT& roiCount, USHORT* contourImagePtr)
{
	long ret = FALSE;

	//Get minimal Label Index of Auto Tracking ROI
	long minLabel = StatsManager::MIN_INDEX_ROI_CONTOUR;
	//Morphology
	{
		ret = imgProDll->MorphImage(srcImagePtr, width, height, MORPH_OPEN);
		if (ret == FALSE)
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1,L"Morph Processing failure");
			return FALSE;
		}

		ret = imgProDll->MorphImage(srcImagePtr, width, height, MORPH_CLOSE);
		if (ret == FALSE)
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1,L"Morph Processing failure");
			return FALSE;
		}
	}	

	//Binarize
	if (FALSE == imgProDll->BinarizeImage(srcImagePtr, width, height, BI_OTSU, StatsManager::_minSnr))
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1,L"Binarize Processing failure");
		return FALSE;
	}	

	long minArea = 9;
	if (StatsManager::_minAreaFilterActive)
	{
		minArea = max(minArea, StatsManager::_minAreaFilterValue);
	}
	roiCount = static_cast<USHORT>(StatsManager::_maxRoiNum);
	// label and get contour
	if (FALSE == imgProDll->LableImage(srcImagePtr, contourImagePtr, width, height, roiCount, static_cast<USHORT>(minArea), static_cast<USHORT>(minLabel)))
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1,L"label and contour Processing failure");
		return FALSE;
	}
	return TRUE;
}

///Image Process
UINT ImageProcessThreadProc( LPVOID pParam)
{
	long numChannel = StatsManager::_numChannel;
	USHORT imgWidth = static_cast<USHORT>(StatsManager::_ImageProcessWidth);
	USHORT imgHeight = static_cast<USHORT>(StatsManager::_ImageProcessHeight);
	USHORT roiCount = 0;
	USHORT * contourImg = new USHORT[imgWidth * imgHeight];
	memset(contourImg,0,imgWidth*imgHeight*sizeof(USHORT));
	//Image Process
	long result = ImageProcess((USHORT*)StatsManager::_ImageProcessData, imgWidth, imgHeight, roiCount, (USHORT*)contourImg);
	if (result == TRUE)
	{
		StatsManager::_numOfImageProcessROI = static_cast<long>(roiCount);
		//call back to C#
		(*(StatsManager::_pushImageProcessDataFuncPtr))(contourImg,StatsManager::_ImageProcessData,StatsManager::_ImageProcessWidth,StatsManager::_ImageProcessHeight,numChannel,StatsManager::_numOfImageProcessROI);
		//assign inter-thread buffer
		SAFE_DELETE_ARRAY(StatsManager::_ripBuffer.contourBuffer);
		StatsManager::_ripBuffer.contourBuffer = new USHORT[imgWidth*imgHeight];
		memcpy_s(StatsManager::_ripBuffer.contourBuffer,imgWidth*imgHeight*sizeof(USHORT),(USHORT*)StatsManager::_ImageProcessData,imgWidth*imgHeight*sizeof(USHORT));
		StatsManager::_ripBuffer.RIPsize.height = imgHeight;
		StatsManager::_ripBuffer.RIPsize.width = imgWidth;
		StatsManager::_ImageProcessDataReady = TRUE;//W & R
	}else
	{
		StatsManager::_ImageProcessDataReady = FALSE;//W & R
	}
	//release
	SAFE_DELETE_ARRAY(contourImg);
	SAFE_DELETE_HANDLE(StatsManager::_hImageThread);
	((StatsManager*)pParam)->SetImageThreadBusyStatus( FALSE );
	return 0;
}

void StatsManager::GetChannelEnable(long channelEnable) 
{
	std::bitset<MAX_CHANNEL_COUNT> channel_enable (channelEnable);
	_numChannel = static_cast<long>(channel_enable.count());
	for (int i = 0; i < channel_enable.size(); i++)
	{
		_channelEnable[i] = channel_enable.test(i);
		if (_numChannel == 1 && _channelEnable[i])
		{
			_singleActiveChannelIdx = i;
		}
	}
}

/**
This function ComputeContours.

@param data Pointer to the source image.
@param width Width of source image in pixels.
@param height Height of source image in pixels.
@param channelEnable 4bits representd the status of channel. e.g. 1111 means all channels are enabled
@param channelSelected channel is selected to display contour.
@return .
*/
long StatsManager::ComputeContours(unsigned short *data, long width, long height, long channelEnable,long channelSelected)
{
	long ret = FALSE;
	if (GetImageThreadBusyStatus())
	{
		return FALSE;
	} 
	_ImageProcessChannelSelectedIndex = channelSelected;

	GetChannelEnable(channelEnable);

	if (_numChannel > 0 && _ImageProcessChannelSelectedIndex > 0) //
	{
		//allocate memory for the buffer if needed
		long bufferSize = width*height;
		if (_ImageProcessData == NULL)
		{
			_ImageProcessData = new USHORT[bufferSize];
		}
		else if (_ImageProcessHeight*_ImageProcessWidth != bufferSize)
		{
			SAFE_DELETE_ARRAY(_ImageProcessData);
			_ImageProcessData = new USHORT[bufferSize];
		}
		_ImageProcessHeight = height;
		_ImageProcessWidth = width;
		//contour only does the single channel
		if ( 1 == _numChannel)
		{
			memcpy_s(_ImageProcessData, sizeof(unsigned short)*bufferSize, data, sizeof(unsigned short)*bufferSize);
		}
		else
		{
			memcpy_s(_ImageProcessData, sizeof(unsigned short)*bufferSize, data+bufferSize*(_ImageProcessChannelSelectedIndex-1), sizeof(unsigned short)*bufferSize); //copy the specific channel data
		}
		DWORD dwThreadId;
		SAFE_DELETE_HANDLE(_hImageThread);
		SetImageThreadBusyStatus(TRUE);
		_hImageThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) ImageProcessThreadProc, this, 0, &dwThreadId );
		if (_hImageThread != NULL)
		{
			ret = TRUE;
		}
	}else
	{
		_numOfImageProcessROI = 0;
	}
	return ret;
}

//model will call this function
//DllExportStatsManager ComputeStats(unsigned short *data, long numChannels, long width, long height)
long StatsManager::ComputeStats(unsigned short *data, FrameInfoStruct frameInfo, long channelEnable, long includeLineProfile, long includeRegularStats, long enabledChannelsOnly)
{
	long ret = TRUE;	
	
	if(GetThreadBusyStatus())
	{
		return FALSE;
	}

	GetChannelEnable(channelEnable);
	if (_numChannel <= 0)
	{
		return FALSE;
	}
	_includeRegularStats = includeRegularStats;
	_channelEnableBinary = channelEnable;
	_imgWidth = frameInfo.imageWidth;

	long planes = frameInfo.numberOfPlanes > 1 ? frameInfo.numberOfPlanes : 1;

	_imgHeight = frameInfo.imageHeight * planes;
	_bufferType =  (BufferType)frameInfo.bufferType;
	_includeLineProfileInStatsCalc = includeLineProfile;

	if (_imgWidth == _mskWidth && _imgHeight == _mskHeight)
	{
		_roi_source_flag = OVERLAY_FLAG;
	}
	if ( _imgWidth == _ripBuffer.RIPsize.width && _imgHeight == _ripBuffer.RIPsize.height && _ImageProcessChannelSelectedIndex)
	{
		_roi_source_flag = (ROISourceFlags)(_roi_source_flag + CONTOUR_FLAG);
	}
	// no need to do anything if no roi defined or no channel acquiring unless we only want the line profile
	if (_roi_source_flag || (TRUE == includeLineProfile && FALSE == includeRegularStats))	
	{
		if (TRUE == includeRegularStats)
		{
			if (NULL == _statsNameList)
			{
#ifdef VIRTUAL_ALLOC
				_statsNameList = reinterpret_cast<char**>(VirtualAlloc(NULL, STATSLIST_LENGTH_VALLOC*sizeof(char*),MEM_COMMIT,PAGE_READWRITE));	
#else
				_statsNameList = new char*[STATSLIST_LENGTH];
#endif

#ifdef VIRTUAL_ALLOC			
				SIZE_T blockSize = (static_cast<long long>(MAX_STATSNAME_COUNT*STATSLIST_LENGTH*sizeof(char*)/PAGE_GRANULARITY) + 1) * PAGE_GRANULARITY;
				_statsNameVirtualBlock = reinterpret_cast<char*>(VirtualAlloc(NULL, blockSize,MEM_COMMIT,PAGE_READWRITE));
#endif
				if((_statsNameList != NULL) && (_statsNameVirtualBlock != NULL))
				{
					for (long n = 0; n < STATSLIST_LENGTH; n++ )
					{		
#ifdef VIRTUAL_ALLOC
						_statsNameList[n] = _statsNameVirtualBlock + n*MAX_STATSNAME_COUNT;
#else
						_statsNameList[n] = new char[MAX_STATSNAME_COUNT];
#endif
					}
				}
				else
				{
					VirtualFree(_statsNameVirtualBlock,0,MEM_RELEASE);
					_statsNameVirtualBlock = NULL;
					VirtualFree(_statsNameList,0,MEM_RELEASE);
					_statsNameList = NULL;
					return FALSE;
				}
			}
		}

		long bufferLength = ( 1 == _numChannel) ? _imgWidth * _imgHeight : _imgWidth * _imgHeight * MAX_CHANNEL_COUNT;
		long dflimHistogramLength = ( 1 == _numChannel) ? DFLIM_HISTOGRAM_BINS : DFLIM_HISTOGRAM_BINS * MAX_CHANNEL_COUNT;

		switch (_bufferType)
		{
		case BufferType::DFLIM_ALL:
		case BufferType::DFLIM_IMAGE:
			{
				if (NULL == _pData)//allocate memory for the incoming buffer if needed
				{
					_pData = new unsigned short[bufferLength];
				}
				else if (bufferLength != _imgBufferLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pData);
					_pData = new unsigned short[bufferLength];						
				}					

				if (NULL == _pDataDFLIMSinglePhoton)//allocate memory for the incoming buffer if needed
				{
					_pDataDFLIMSinglePhoton = new unsigned short[bufferLength];
				}
				else if (bufferLength != _imgBufferLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pDataDFLIMSinglePhoton);
					_pDataDFLIMSinglePhoton = new unsigned short[bufferLength];						
				}

				if (NULL == _pDataDFLIMArrivalTimeSum)//allocate memory for the incoming buffer if needed
				{
					_pDataDFLIMArrivalTimeSum = new UINT32[bufferLength];
				}
				else if (bufferLength != _imgBufferLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pDataDFLIMArrivalTimeSum);
					_pDataDFLIMArrivalTimeSum = new UINT32[bufferLength];
				}

				if (NULL == _pDataDFLIMHistogram)//allocate memory for the incoming buffer if needed
				{
					_pDataDFLIMHistogram = new UINT32[dflimHistogramLength];
				}
				else if (dflimHistogramLength != _dflimHistogramLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pDataDFLIMHistogram);
					_pDataDFLIMHistogram = new UINT32[dflimHistogramLength];
					_dflimHistogramLength = dflimHistogramLength;
				}
			}
			break;
		case BufferType::INTENSITY:
		case BufferType::DFLIM_DIAGNOSTIC:
			{
				if (NULL == _pData)//allocate memory for the incoming buffer if needed
				{
					_pData = new unsigned short[bufferLength];
				}
				else if (bufferLength != _imgBufferLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pData);
					_pData = new unsigned short[bufferLength];						
				}
			}
			break;
		default:
			return FALSE;
		}

		_imgBufferLength = bufferLength;

		if ( 1 == _numChannel)
		{
			switch (_bufferType)
			{
			case BufferType::DFLIM_ALL:
			case BufferType::DFLIM_IMAGE:
				{				
					char* dataBytes = (char*)data;

					memcpy_s(_pDataDFLIMHistogram, sizeof(UINT32)*dflimHistogramLength, dataBytes, sizeof(UINT32)*dflimHistogramLength);
					long copiedBytes = sizeof(UINT32)*dflimHistogramLength;

					memcpy_s(_pData, sizeof(unsigned short)*bufferLength, dataBytes + copiedBytes, sizeof(unsigned short)*bufferLength);
					copiedBytes +=sizeof(unsigned short)*bufferLength;

					memcpy_s(_pDataDFLIMSinglePhoton, sizeof(unsigned short)*bufferLength, dataBytes + copiedBytes, sizeof(unsigned short)*bufferLength);
					copiedBytes +=sizeof(unsigned short)*bufferLength;

					memcpy_s(_pDataDFLIMArrivalTimeSum, sizeof(UINT32)*bufferLength, dataBytes + copiedBytes, sizeof(UINT32)*bufferLength);
					copiedBytes +=sizeof(UINT32)*bufferLength;
				}
				break;
			case BufferType::INTENSITY:
			case BufferType::DFLIM_DIAGNOSTIC:
				{
					memcpy_s(_pData, sizeof(unsigned short)*bufferLength, data, sizeof(unsigned short)*bufferLength);					
				}
				break;
			default:
				return FALSE;
			}			
		}
		else
		{
			if (enabledChannelsOnly && _numChannel < MAX_CHANNEL_COUNT)
			{
				switch (_bufferType)
				{
				case BufferType::DFLIM_ALL:
				case BufferType::DFLIM_IMAGE:
					{				
						char* dataBytes = (char*)data;
						long copiedBytes = 0;
						int ch = 0;
						for (int ic = 0; ic < MAX_CHANNEL_COUNT; ic++)
						{
							if ((_channelEnable[ic]) && (ch < _numChannel))
							{
								memcpy_s(_pDataDFLIMHistogram + ic*DFLIM_HISTOGRAM_BINS, sizeof(UINT32)*DFLIM_HISTOGRAM_BINS, dataBytes + sizeof(UINT32)*DFLIM_HISTOGRAM_BINS, sizeof(UINT32)*DFLIM_HISTOGRAM_BINS);
								copiedBytes += sizeof(UINT32)*DFLIM_HISTOGRAM_BINS;
							}
							ch++;
						}
						ch = 0;
						for (int ic = 0; ic < MAX_CHANNEL_COUNT; ic++)
						{
							if ((_channelEnable[ic]) && (ch < _numChannel))
							{
								
								memcpy_s(_pData + ic*_imgWidth*_imgHeight, sizeof(unsigned short)*_imgWidth*_imgHeight, dataBytes + copiedBytes, sizeof(unsigned short)*_imgWidth*_imgHeight);
								copiedBytes += sizeof(unsigned short)*_imgWidth*_imgHeight;
							}
							ch++;
						}
						ch = 0;
						for (int ic = 0; ic < MAX_CHANNEL_COUNT; ic++)
						{
							if ((_channelEnable[ic]) && (ch < _numChannel))
							{	
								memcpy_s(_pDataDFLIMSinglePhoton + ic*_imgWidth*_imgHeight, sizeof(unsigned short)*_imgWidth*_imgHeight, dataBytes + copiedBytes, sizeof(unsigned short)*_imgWidth*_imgHeight);
								copiedBytes += sizeof(unsigned short)*_imgWidth*_imgHeight;
							}
							ch++;
						}
						ch = 0;
						for (int ic = 0; ic < MAX_CHANNEL_COUNT; ic++)
						{
							if ((_channelEnable[ic]) && (ch < _numChannel))
							{		
								memcpy_s(_pDataDFLIMArrivalTimeSum + ic*_imgWidth*_imgHeight, sizeof(UINT32)*_imgWidth*_imgHeight, dataBytes + copiedBytes, sizeof(UINT32)*_imgWidth*_imgHeight);
								copiedBytes += sizeof(UINT32)*_imgWidth*_imgHeight;
								
								ch++;
							}
						}
					}
					break;
				case BufferType::INTENSITY:
				case BufferType::DFLIM_DIAGNOSTIC:
					{
						int ch = 0;
						for (int ic = 0; ic < MAX_CHANNEL_COUNT; ic++)
						{
							if ((_channelEnable[ic]) && (ch < _numChannel))
							{
								memcpy_s(_pData + ic*_imgWidth*_imgHeight, sizeof(unsigned short)*_imgWidth*_imgHeight, data + ch*_imgWidth*_imgHeight, sizeof(unsigned short)*_imgWidth*_imgHeight);							
								ch++;
							}
						}				
					}
					break;
				default:
					return FALSE;
				}				
			}
			else
			{
				switch (_bufferType)
				{
				case BufferType::DFLIM_ALL:
				case BufferType::DFLIM_IMAGE:
					{				
						char* dataBytes = (char*)data;

						memcpy_s(_pDataDFLIMHistogram, sizeof(UINT32)*dflimHistogramLength, dataBytes, sizeof(UINT32)*dflimHistogramLength);
						long copiedBytes = sizeof(UINT32)*dflimHistogramLength;

						memcpy_s(_pData, sizeof(unsigned short)*bufferLength, dataBytes + copiedBytes, sizeof(unsigned short)*bufferLength);
						copiedBytes +=sizeof(unsigned short)*bufferLength;

						memcpy_s(_pDataDFLIMSinglePhoton, sizeof(unsigned short)*bufferLength, dataBytes + copiedBytes, sizeof(unsigned short)*bufferLength);
						copiedBytes +=sizeof(unsigned short)*bufferLength;

						memcpy_s(_pDataDFLIMArrivalTimeSum, sizeof(UINT32)*bufferLength, dataBytes + copiedBytes, sizeof(UINT32)*bufferLength);
						copiedBytes +=sizeof(UINT32)*bufferLength;
					}
					break;
				case INTENSITY:
				case DFLIM_DIAGNOSTIC:
					{
						memcpy_s(_pData, sizeof(unsigned short)*bufferLength, data, sizeof(unsigned short)*bufferLength);			
					}
					break;
				default:
					return FALSE;
				}		
			}
		}

		if (BufferType::DFLIM_ALL == _bufferType)
		{
			long channels = enabledChannelsOnly || 1 == _numChannel ? _numChannel : MAX_CHANNEL_COUNT;

			INT64 dFLIMHistogramBufferSize = channels * DFLIM_HISTOGRAM_BINS * sizeof(UINT32);
			INT64 imageBufferSize = channels * _imgWidth * _imgHeight * sizeof(unsigned short);
			INT64 dFLIMSinglePhotonBufferSize = channels * _imgWidth * _imgHeight * sizeof(unsigned short);
			INT64 dFLIMArrivalTimeSumBufferSize = channels * _imgWidth * _imgHeight * sizeof(UINT32);
			
			INT64 dFLIMImageSize = dFLIMHistogramBufferSize + imageBufferSize + dFLIMSinglePhotonBufferSize + dFLIMArrivalTimeSumBufferSize;
			INT64 dFLIMPhotonListSize = frameInfo.copySize - dFLIMImageSize;

			if (dFLIMPhotonListSize > 0 )
			{
				char* dataBytes = (char*)data;
				if (NULL == _pDataDFLIMPhotonList)//allocate memory for the incoming buffer if needed
				{
					_pDataDFLIMPhotonList = new char[dFLIMPhotonListSize];
				}
				else if (dFLIMPhotonListSize != _dFLIMPhotonListLength)//Clear the memory if the the buffer size has changed
				{
					SAFE_DELETE_ARRAY(_pDataDFLIMPhotonList);
					_pDataDFLIMPhotonList = new char[dFLIMPhotonListSize];						
				}					

				_dFLIMPhotonListLength = dFLIMPhotonListSize;
				memcpy_s(_pDataDFLIMPhotonList, dFLIMPhotonListSize, dataBytes + dFLIMImageSize, dFLIMPhotonListSize);
			}
		}

		_newImageForLineProfile = TRUE;
		if (_imgWidth == _mskWidth && _imgHeight == _mskHeight)
		{	
			// do stats calculation using imagestats
			if(_hThread != NULL)
			{
				CloseHandle(_hThread);
				_hThread = NULL;
			}
			DWORD dwThreadId;
			SetThreadBusyStatus(TRUE);
			_hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) StatsThreadProc, this, 0, &dwThreadId );
			if (_hThread == NULL)
			{
				ret = FALSE;
				SetThreadBusyStatus(FALSE);
			}
		}
	}
	return ret;
}

#include <fstream> 

template<class T>
int SaveBuffer(char* fileName, T buffer, int width, int height)
{
	std::ofstream outfile(fileName, std::ofstream::binary);

	int size = width* height;

	// allocate memory for file content

	// write to outfile
	outfile.write ((char*)buffer,size);

	// release dynamically-allocated memory

	outfile.close();

	return 0;
}

//model will call this function
long StatsManager::SetStatsMask(unsigned short* mask, long mskWidth, long mskHeight)
{
	Lock lock(_critSect);
	long ret = TRUE;	

	//Clear the memory if the mask dimensions have changed
	if((NULL != _pMask) && (_mskWidth != mskWidth || _mskHeight != mskHeight))
	{
		delete[] _pMask;
		_pMask = NULL;
	}

	//allocate memory for the incoming mask if needed	
	if (NULL == _pMask)
	{
		_pMask = new unsigned short[mskWidth*mskHeight];
	}

	_mskWidth = mskWidth;
	_mskHeight = mskHeight;

	//memcpy the mask to the new buffer
	if(NULL == mask)
	{
		return TRUE;
	}
	memcpy_s(_pMask,mskWidth*mskHeight*sizeof(unsigned short),mask,mskWidth*mskHeight*sizeof(unsigned short));

	////***Uncomment for Debugging//
	//fileName = "D:\\pMask.csv";
	//SaveBuffer(fileName, _pMask,  width,  height);
	////***Uncomment for Debugging//

	_numROI = GetMaskNumROI(mask,mskWidth,mskHeight);
	return ret;
}

void StatsManager::SetImgWidth(long imgWidth)
{
	_imgWidth = imgWidth;
}

void StatsManager::SetImgHeight(long imgHeight)
{
	_imgHeight = imgHeight;
}

long StatsManager::GetImgWidth()
{
	return _imgWidth;
}

long StatsManager::GetImgHeight()
{
	return _imgHeight;
}

unsigned short* StatsManager::GetDataPtr()
{
	return _pData;
}

unsigned short* StatsManager::GetMaskPtr()
{
	return _pMask;
}

void StatsManager::SetDataPtr(unsigned short* dataPtr)
{
	_pData = dataPtr;
}

void StatsManager::SetMaskPtr(unsigned short* maskPtr)
{
	_pMask = maskPtr;
}

void StatsManager::SetThreadBusyStatus(long isThreadActive)
{
	_isThreadActive = isThreadActive;
}

long StatsManager::GetThreadBusyStatus()
{
	return _isThreadActive;
}

long StatsManager::GetNumROI()
{
	return (_numROI + _numOfImageProcessROI);
}

void StatsManager::SetImageThreadBusyStatus(long isThreadActive)
{
	_isImageThreadActive = isThreadActive;
}

long StatsManager::GetImageThreadBusyStatus()
{
	return _isImageThreadActive;
}

long StatsManager::SetLineProfileLine(long p1X, long p1Y, long p2X, long p2Y, long lineIsActive)
{	
	long ret = TRUE;
	_lineP1X = p1X;
	_lineP1Y = p1Y;
	_lineP2X = p2X;
	_lineP2Y = p2Y;

	if (TRUE == lineIsActive)
	{
		// do stats calculation using imagestats
		DWORD dwThreadId;

		CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) LineProfileThreadProc, 0, 0, &dwThreadId );
	}

	if (FALSE == lineIsActive &&  _lineIsActive != lineIsActive &&
		NULL != (*(StatsManager::_pushLineProfileFuncPtr)))
	{
		(*(StatsManager::_pushLineProfileFuncPtr))(NULL,0,0, 0);
	}

	//let the higher level function know that there is no line profile
	_lineIsActive = lineIsActive;

	return ret;
}

long StatsManager::SetLineProfileLineWidth(long lineWidth)
{
	long ret = TRUE;
	if (_lineProfileLineWidth != lineWidth)
	{
		_lineProfileLineWidth = lineWidth;
		if (TRUE == _lineIsActive)
		{
			// do stats calculation using imagestats
			DWORD dwThreadId;

			CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) LineProfileThreadProc, 0, 0, &dwThreadId );
		}
	}
	return ret;
}


long StatsManager::SetTZero(double tZero, long channelIndex)
{
	if (channelIndex < 0 || channelIndex >= MAX_CHANNEL_COUNT)
	{
		return FALSE;
	}
	_tZero[channelIndex] = tZero;
	return true;
}

long StatsManager::CopyStatsImageDataToLineImageData()
{
	if (NULL != _pData && TRUE == _newImageForLineProfile)
	{		
		_lineProfileImgWidth = _imgWidth;
		_lineProfileImgHeight = _imgHeight;
		_lineProfileNumChannel = _numChannel;
		_lineProfilechannelEnableBinary = _channelEnableBinary;
		memcpy_s(_lineProfileChannelEnable, sizeof(bool)*MAX_CHANNEL_COUNT,StatsManager::_channelEnable, sizeof(bool)*MAX_CHANNEL_COUNT);
		if ( 0 == _numChannel)
		{
			_pDataForLine = NULL;
		}
		else if ( 1 == _numChannel)
		{
			const long bufferSize = _imgWidth*_imgHeight;

			if (_imgBufferLength != bufferSize)
			{
				return FALSE;
			}

			if (NULL == _pDataForLine)//allocate memory for the incoming buffer if needed
			{
				_pDataForLine = new unsigned short[bufferSize];
				_lineProfileImgBufferSize = bufferSize;
			}else if (bufferSize != _lineProfileImgBufferSize)//Clear the memory if the the buffer size has changed
			{
				SAFE_DELETE_ARRAY(_pDataForLine);
				_pDataForLine = new unsigned short[bufferSize];
				_lineProfileImgBufferSize = bufferSize;
			}

			memcpy_s(_pDataForLine, sizeof(unsigned short)*bufferSize, _pData, sizeof(unsigned short)*bufferSize);
			
		}
		else
		{
			const long bufferSize = _imgWidth*_imgHeight*MAX_CHANNEL_COUNT;

			if (_imgBufferLength != bufferSize)
			{
				return FALSE;
			}

			if (NULL == _pDataForLine)//allocate memory for the incoming buffer if needed
			{
				_pDataForLine = new unsigned short[bufferSize];
				_lineProfileImgBufferSize = bufferSize;
			}else if (bufferSize != _lineProfileImgBufferSize)//Clear the memory if the the buffer size has changed
			{
				SAFE_DELETE_ARRAY(_pDataForLine);
				_pDataForLine = new unsigned short[bufferSize];
				_lineProfileImgBufferSize = bufferSize;
			}
			
			memcpy_s(_pDataForLine, sizeof(unsigned short)*bufferSize, _pData, sizeof(unsigned short)*bufferSize);
			
		}
		_newImageForLineProfile = FALSE;
	}
	return TRUE;
}

long StatsManager::GetLineProfileData(double* &resultBuffer, long &length, long &chanEnableBin, long &nChannel)
{
	unsigned short* data = _pDataForLine;
	if(NULL == data)
	{
		return FALSE;
	}

	long ret = TRUE;

	const long width = StatsManager::_lineProfileImgWidth;
	const long height = StatsManager::_lineProfileImgHeight;

	long p1x = StatsManager::_lineP1X;
	long p1y = StatsManager::_lineP1Y;
	long p2x = StatsManager::_lineP2X;
	long p2y = StatsManager::_lineP2Y;

	//if both points are outside of the image then return FALSE
	if ((p1x >= width && p2x >= width) || (p1y >= height && p2y >= height))
	{
		return FALSE;
	}

	//If any part of the line is outside of the image, truncate it to to avoid fake data or crashes
	if (p1x >= width || p2x >= width || p1y >= height || p2y >= height )
	{
		double m = 0; 
		if (p2x == p1x)
		{
			m = INFINITE;
		}
		else
		{
			m = static_cast<double>(p2y - p1y)/static_cast<double>(p2x - p1x);
		}

		double b = p1y - m * p1x;
		if (p1x >= width)
		{
			p1x = width - 1;
			p1y = static_cast<long>(m * p1x + b + .5);
		}
		if (p2x >= width)
		{
			p2x = width - 1;
			p2y = static_cast<long>(m * p2x + b + .5);
		}

		if (p1y >= height)
		{
			p1y = height - 1;
			if (INFINITE == m)
			{
				p1x = p2x;
			}
			else
			{
				p1x = static_cast<long>((p1y - b)/m + 0.5);
			}
		}
		if (p2y >= height)
		{
			p2y = height - 1;
			if (INFINITE == m)
			{
				p2x = p1x;
			}
			else
			{
				p2x = static_cast<long>((p2y - b)/m + 0.5);
			}
		}
	}

	const long numChannel = StatsManager::_lineProfileNumChannel;
	const long lineWidth = StatsManager::_lineProfileLineWidth;
	const long channelEnableBinary = StatsManager::_lineProfilechannelEnableBinary;

	bool channelEnable[MAX_CHANNEL_COUNT] = {false};
	memcpy_s(channelEnable, sizeof(bool)*MAX_CHANNEL_COUNT,StatsManager::_lineProfileChannelEnable, sizeof(bool)*MAX_CHANNEL_COUNT);

	double** sumResults = (double**) malloc(numChannel*sizeof(double*));

	long totalStats = 0;
	long part1 = (p1x - p2x) * (p1x - p2x);
	long part2 = (p1y - p2y) * (p1y - p2y);
	long lineLength = (long)sqrt(part1 + part2);

	long j = 0;
	for (long i = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		if (true == channelEnable[i] && j <=numChannel)
		{
			long channelOffset = (numChannel > 1) ? i: 0;
			long offset = channelOffset*width*height*(BITDEPTH/8);
			unsigned char* results = (unsigned char*)malloc(lineLength*lineWidth*sizeof(unsigned char*));
			long arraylenght = LineProfile( ((unsigned char*)(data)) + offset,
				width,
				height,
				BITDEPTH,
				p1x,
				p1y,
				p2x,
				p2y,
				lineWidth,
				results);
			long dispLen = arraylenght / lineWidth;
			sumResults[j] = (double*) malloc(dispLen*sizeof(double));
			totalStats+=dispLen;

			//Sum the data and average to include the different lineWidths on the data
			for (long k = 0; k < dispLen; k++)
			{
				double sumVal = 0;
				for (long l = 0; l < lineWidth; l++)
				{
					long index = k + l * (dispLen);
					sumVal += ((unsigned short*)(results))[index];
				}
				sumResults[j][k] = sumVal / lineWidth;
			}
			j++;
		}
	}
	resultBuffer = (double*)malloc(totalStats*sizeof(double*));
	for (long i = 0; i < numChannel; i++)
	{
		memcpy_s(resultBuffer + i*totalStats/numChannel, sizeof(double)*totalStats/numChannel, sumResults[i], sizeof(double)*totalStats/numChannel);
	}
	length = totalStats;
	nChannel = numChannel;
	chanEnableBin = channelEnableBinary;
	return ret;
}

long StatsManager::CreateDFLIMXYTObject(char* photonList, size_t photonListSize, long imageWidth, long imageHeight, long numChannel, bool channelEnable[MAX_CHANNEL_COUNT])
{
	UINT32 iLine = 0;
    UINT32 iPixel = 0;
    UINT32 iFrame = 0;
	UINT32 dim_t = DFLIM_HISTOGRAM_T;
    UINT32 dim_lines = imageHeight;
    UINT32 dim_pixels = imageWidth; 
	UINT32 elements = dim_t * dim_lines * dim_pixels;
	UINT32 frameNum = 0;

	const UCHAR PHOTON_VAL_NEWPIXEL = 255;
	const UCHAR PHOTON_VAL_NEWLINE = 254;
	const UCHAR PHOTON_VAL_NEWFRAME = 253;

	if (elements != _dFLIMXYTLength)
	{
		if (NULL != _dFLIMXYT)
		{
			for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
			{
				SAFE_DELETE_ARRAY(_dFLIMXYT[i]);
			}
			SAFE_DELETE_ARRAY(_dFLIMXYT);
		}
		_dFLIMXYT = new UINT32*[MAX_CHANNEL_COUNT];
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			_dFLIMXYT[c] = new UINT32[elements]();
		}
		_dFLIMXYTLength = elements;
	}
	else
	{
		for (int c = 0; c < MAX_CHANNEL_COUNT; ++c)
		{
			memset(_dFLIMXYT[c], 0,elements*sizeof(UINT32));
		}		
	}

	int j = 0;
	int photonListOffset = 0;
	for (int c = 0; c < MAX_CHANNEL_COUNT; c++)
	{
		if (true == channelEnable[c] && j <=numChannel)
		{			
			for (int k = photonListOffset; k < photonListSize; ++k) 
			{
				UCHAR b = static_cast<UCHAR>(photonList[k]);
				switch(b) 
				{
					case PHOTON_VAL_NEWPIXEL:
						iPixel = iPixel + 1;
						break;
					case PHOTON_VAL_NEWLINE:
						iLine = iLine + 1;						
						iPixel = 0;						
						break;
					case PHOTON_VAL_NEWFRAME:
						////TODO: need to add this for image review when processing averaged frames
						//iFrame = iFrame + 1;
						//if (iFrame <= frameNum) {
						//	frameidx[iFrame] = k+1;  // keep the frame index (at the i+1'th loc)
						//}
						//iPixel = 0;
						//iLine = 0;
						break;
					default :						
						//alternative ix calculation
						//UINT32 ix = (iPixel * dim_lines + iLine) * dim_t + (int) b;
						
						UINT32 ix = (iPixel + dim_pixels * iLine) * dim_t + (int) b;
						if (ix < elements) // error check!
						{  
							_dFLIMXYT[c][ix] = _dFLIMXYT[c][ix]+1;
						}
						break;
				}
				if (iLine == dim_lines)
				{
					iLine = 0;
					photonListOffset = k + 1;
					break;
				}
			}
			++j;
		}
	}
	return TRUE;
}


long StatsManager::ComputeDFLIMROIHistograms(UINT32 * dFLIMHistogram, 
											 void* pMskBrief,	
											 long numChannel,
											 bool channelEnable[MAX_CHANNEL_COUNT],
											 UINT32* &flimROIHistograms,
											 UINT32 &rois,
											 UINT32 &length)
{
	Lock lock(_critSect);

	if (NULL == dFLIMHistogram || NULL == _dFLIMXYT || NULL == pMskBrief)
	{
		return FALSE;
	}

	map<short, vector<long>>* pMsk = &StatsManager::_mskBrief;
	size_t roiNum = pMsk->size();

	rois = static_cast<UINT32>(roiNum);
	length = static_cast<UINT32>(numChannel*(roiNum + 1)*DFLIM_HISTOGRAM_BINS);
	flimROIHistograms = new UINT32[length]();
	long j = 0;
	for( long c = 0; c < MAX_CHANNEL_COUNT; c++)
	{	
		if (StatsManager::_channelEnable[c])	
		{	
			UINT32* channelHistogram = numChannel > 1 ? dFLIMHistogram + c * DFLIM_HISTOGRAM_BINS : dFLIMHistogram;
			memcpy_s(flimROIHistograms + j*(roiNum + 1)*DFLIM_HISTOGRAM_BINS, sizeof(UINT32)*DFLIM_HISTOGRAM_BINS, channelHistogram, sizeof(UINT32)*DFLIM_HISTOGRAM_BINS);
			int k = 1;	
			for(map<short, vector<long>>::iterator i = pMsk->begin(); i != pMsk->end(); i++)
			{
				for (int t = 0; t < DFLIM_HISTOGRAM_T; ++t)
				{
					UINT32 h = 0;
					for(int m = 0; m < i->second.size(); m++)
					{
						h += _dFLIMXYT[c][(i->second)[m]*DFLIM_HISTOGRAM_T + t];
					}

					flimROIHistograms[j*(roiNum+1)*DFLIM_HISTOGRAM_BINS + k*DFLIM_HISTOGRAM_BINS + t] = h;
				}
				flimROIHistograms[j*(roiNum+1)*DFLIM_HISTOGRAM_BINS + k*DFLIM_HISTOGRAM_BINS + 254] = channelHistogram[254];
				flimROIHistograms[j*(roiNum+1)*DFLIM_HISTOGRAM_BINS + k*DFLIM_HISTOGRAM_BINS + 255] = channelHistogram[255];
				++k;
			}

			++j;
		}
	}

	return TRUE;
}


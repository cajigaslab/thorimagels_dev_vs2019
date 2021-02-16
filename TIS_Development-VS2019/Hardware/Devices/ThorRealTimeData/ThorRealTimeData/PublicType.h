#pragma once

#include <string>
#include <vector>
#include <map>
#include <regex>
#include "..\..\..\..\GUI\Controls\RealTimeLineChart\RealTimeLineChart\PublicEnum.cs"

//***********************************************************************//
//*** Define enum & typedef in this header file					  ***//
//***********************************************************************//

#define SAFE_MEMCPY(x,y,z) if((x != NULL) && (z != NULL) && (0 < y)) { memcpy_s(x,y,z,y); }
#define SAFE_DELETE_MEMORY(x) if (NULL != x) { free((void*)x); x = NULL; }
#define SAFE_DELETE_ARRAY(x) if (x != NULL) { delete[] x; x = NULL; }
#define SAFE_DELETE_HANDLE(x) if(NULL != x) { CloseHandle(x); x = NULL;}

typedef std::chrono::high_resolution_clock Clock;
using	std::chrono::milliseconds;
using	std::chrono::duration_cast;

enum BoardStyle
{
	PCI,
	USB
};

typedef struct BoardType
{
	std::string name;
	std::string devID;
	long active;
	long totalAI;
	long totalDI;
	BoardStyle bStyle;
}BoardInfo;

typedef struct channelSpec
{	
	std::string alias;
	int signalType;
	std::string type;
	std::string lineId;
	long sample;
	//long aiTrigger;
	long Stimulus;
}Channels;

typedef struct AcquireMode
{
	//int buffer;
	double sampleRate;
	double duration;
	double StimulusLimit;
	long hwTrigMode;
	std::string hwTrigChannel;
	long interleave;
	long hwTrigType;
}Mode;

typedef struct Bleach
{
	std::string hwTrigLine;
	std::string shutterLine;
	std::string bleachLine;
	std::string outputLine;
	double pmtCloseTime;
	double bleachTime;
	double bleachIdleTime;
	long bleachIteration;
	double outDelayTime;
	long hwTrigMode;
	long cycle;
	double interval;
}BleachParam;

typedef struct Invert
{
	std::string inputLine;
	std::string outputLine;
}InvertParam;

typedef struct FielPath
{
	std::wstring settingPath;
	std::wstring episodePath;
}FilePathParam;

typedef struct Async
{
	BoardInfo board;
	BleachParam bleach;
	bool outBuffered;
	unsigned long long arrayLength;
	unsigned long long callbackLength;
	unsigned char* arrayPtr;
}AsyncParams;

typedef struct Spectrum
{
	double liveSampleSec;
	double freqMin;
	double freqMax;
	double sampleMinSec;
	double sampleMaxSec;
	long   freqAvgNum;
	long   freqAvgMode;
	long   blockNum;
}SpectrumParams;

typedef struct VirtualVarStruct
{
	std::string			name;
	SignalType			sType;
	int					offset;
	double				pValue;
	VirtualVarStruct(std::string &str):name(str) {}
	bool operator==(const VirtualVarStruct &str) const { return (0 == str.name.compare(this->name)); }
}VirtualVariable;

typedef struct OTMParamStruct
{
	long isCurveFit;
	double fitFreqMin;
	double fitFreqMax;
	double temperature;
	double radius;
	double viscosity;
	double gammaTheory;
	double diffTheory;
	int freqBlock;
	double beta2FreqMin;
	double beta2FreqMax;
}OTMParam;

typedef struct OTMFitStruct
{
	double diffXY1[2];		//[um^2], based on betaXY1
	double diffXY2[2];		//[um^2], based on betaXY2
	double cornerXY[2];		//[Hz]
	double chiXY[2];
	double gammaXY[2];		//[kg/s]
	double kappaXY[2];		//[pN/nm]
	double betaXY1[2];		//[V/nm]
	double betaXY2[2];		//[V/nm]
}OTMFit;

typedef struct GlobalVarStruct
{
	long id;
	double value;
	std::string name;
}GlobalVar;

class RunTimeException
{
	std::wstring exceptMessage;
	signed long error;

public:
	RunTimeException(std::wstring str,signed long err){exceptMessage = str; error = err;}
	RunTimeException(std::wstring str){exceptMessage = str;}
	//~RunTimeException();
	std::wstring GetExceptionMessage(){return exceptMessage;}
	signed long GetErrorCode(){return error;}
};

#ifdef LOGGING_ENABLED
extern std::auto_ptr<LogDll> logDll;
#endif

extern wchar_t message[MSG_LENGTH];

//error message log
static void LogMessage(wchar_t *logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

class ChannelCenter
{
private:
	static bool _instanceFlag;
	static std::unique_ptr<ChannelCenter> _single;

	long _isLoaded;
	std::map<std::string, std::vector<std::string>> _fileDatasetMap;

	ChannelCenter();

	//clear all enable channel vectors
	void ClearAll();

	//initial loading of episide and settings
	long InitialEpisode();

	//parse for variable from source
	void ChannelCenter::YieldVariable(std::string sourceName, std::vector<VirtualVariable>& vars, std::vector<std::string> enabledLines, SignalType sType);

public:
	~ChannelCenter();
	static ChannelCenter* getInstance();

	std::vector<std::string> _enabledAI;
	std::vector<std::string> _enabledDI;
	std::vector<std::string> _enabledCI;
	std::vector<std::string> _enabledFI;
	std::vector<std::string> _enabledVT;
	std::vector<std::string> _enabledVF;

	std::vector<Channels> _dataChannel;
	std::vector<Channels> _virChannel;
	std::vector<Channels> _specChannel;
	std::vector<Channels> _specVirChannel;

	std::map<int, GlobalVar> _globalVar;	//global variables from VariableSettings

	std::vector<double> _timeSec;
	std::vector<double> _freqHz;
	long _freqRangeIdx[4];					//[CurveFitMin, CurveFitMax, beta2FitMin, beta2FitMax]
	BoardInfo _board;
	Mode _mode;
	SpectrumParams _specParam;
	OTMParam _otmParam;
	OTMFit _otmFit;
	AsyncParams _asyncParam;
	InvertParam _invert;
	FilePathParam _filePath;
	double _threadTime;

	long _isLive;
	long _isLoading;
	long _stopLoading;
	static std::wstring _lastError;
	wchar_t _episodefile[_MAX_PATH];

	//screen for enabled channels
	void EnabledChannels(std::vector<Channels> channelVec);

	std::wstring GetLastError() { return _lastError;	}

	std::wstring GetEpisodeName() { return std::wstring(_episodefile); }

	//parse for variables from source
	std::vector<VirtualVariable> ChannelCenter::YieldVariables(std::string sourceName);

	//load time domain data from episode 
	long LoadEpisode();

	//load data without time info from episode for all enabled channels, file must be opened before this function
	long LoadEpisodeDataOnly(void* file, CompoundData* cData, unsigned long long dOffset, unsigned long long dLength, unsigned long long start, unsigned long long length);

	//load episode group and dataset names
	long LoadEpisodeGroupDatasetNames(const wchar_t* episodeName, std::vector<std::string> &groupNameVec, std::map<std::string, std::vector<std::string>> &datasetMap);

	//load info from settings xml file, allow force reload
	long LoadXML(long reload = TRUE, const wchar_t* targetXML = NULL);

	//reload global variables
	void ReloadGlobalVariables();

	//save spectral and spectral virtual data
	long SaveSpectral(FreqCompDataStruct* fData);

	//create spectral nodes by providing file pointer
	long SetupFileH5(unsigned long long length);

	//carry out spectral analysis, should be invoked after loading of time domain data
	long SpectralAnalysis();

	//verify consistency between settings and file
	long VerifyFileChannels(std::map<std::string, std::vector<std::string>> fileNameMap);

	//persist global OTM settings
	void UpdateOTM();

};

template <typename T>								//output = vector1 + vector2
std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B)
{
	std::vector<T> result;
	result.reserve( A.size() + B.size() );          // preallocate memory
	result.insert( result.end(), A.begin(), A.end() );
	result.insert( result.end(), B.begin(), B.end() );
	return result;
}

template <typename T>								//output += vector1
std::vector<T> &operator+=(std::vector<T> &A, const std::vector<T> &B)
{
	A.reserve( A.size() + B.size() );               // preallocate memory without erasing original
	A.insert( A.end(), B.begin(), B.end() );
	return A;                                
}

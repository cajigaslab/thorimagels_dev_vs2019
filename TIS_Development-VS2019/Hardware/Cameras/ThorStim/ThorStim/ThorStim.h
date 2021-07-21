
#pragma once
#include "..\..\..\..\Common\BlockRingBuffer.h"

#define MAX_DIG_PORT_OUTPUT		8
#define DEFAULT_DO_SAMPLE_RATE	20000	//[Hz]

extern char logMsg[_MAX_PATH];	//used for logging, use message for GetLastErrorMsg instead

extern std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder;
extern std::unique_ptr<BoardInfoNI> boardInfoNI;

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct _ParamsPty
	{
		double pockelsMaxVoltage[MAX_GG_POCKELS_CELL_COUNT];
		double pockelsMinVoltage[MAX_GG_POCKELS_CELL_COUNT];
	}ParamsPty, *pParamsPty;

	class ThorStim : ICamera
	{
	private:
		ThorStim();
	public:
		static HMODULE hDLLInstance;

		static ThorStim* getInstance();
		~ThorStim();

		///<function implementation
		long FindCameras(long &cameraCount); ///<Search system to find daq board
		long SelectCamera(const long camera);
		long TeardownCamera(); ///<close handles for daq board
		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); ///<get the information for each parameter
		long SetParam(const long paramID, const double param);///<set parameter value
		long GetParam(const long paramID, double &param);///<get the parameter value
		long PreflightAcquisition(char * pDataBuffer);///<Setup for , should be called before each experiment, or whenever trigger mode has been changed
		long SetupAcquisition(char * pDataBuffer);
		long StartAcquisition(char * pDataBuffer);///<Start an experiment
		long StatusAcquisition(long &status);///<Status of a stimulation 
		long StatusAcquisitionEx(long &status,long &indexOfLastCompletedFrame);
		long CopyAcquisition(char * pDataBuffer, void* frameInfo);
		long PostflightAcquisition(char * pDataBuffer);
		long GetLastErrorMsg(wchar_t * msg, long size);
		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long GetParamBuffer(const long paramID, char * pBuffer, long size);
		long SetParamBuffer(const long paramID, char * pBuffer, long size);

	private:
		const static long RATE6321 = 250000;

		///<private funcs
		long CloseNITasks();
		long CheckConfigNI();
		long TryTaskMasterClockwCounter(int counterID);
		static long TryFrameTriggerInputwCounter(char counterID);
		static long SyncCustomWaveformOnOff(bool32 start);
		static long SetFrameInTriggerableTask(TaskHandle taskHandle, long armStart);
		static long TryWriteTaskMasterPockelWaveform(long checkStop);
		static long TryWriteTaskMasterLineWaveform(long checkStop);
		static long WaveformModeFinished(void);

		///<generic funcs
		long SetupProtocol();
		long PostflightProtocol();
		static long TryBuildTaskMaster(void);

		///<NI Task Master functions - finite tasks with contineous clock
		long SetupTaskMasterClock(void);
		static long SetupTaskMasterPockel(void);
		static long SetupTaskMasterDigital(void);
		static long SetupFrameTriggerInput(void);
		static int32 CVICALLBACK HWTriggerCallback(TaskHandle taskHandle, int32 signalID, void *callbackData);
		static int32 CVICALLBACK CycleDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
		static int32 CVICALLBACK EveryNDigitalOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
		static int32 CVICALLBACK EveryNPockelOutCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

		///<NI actions
		long MovePockelsToPowerLevel(long index);
		long MovePockelsToParkPosition(void);

	private:

		//*********************************************************//
		//***	gemeral members									***//
		//*********************************************************//

		static bool _instanceFlag;
		static auto_ptr<ThorStim> _single;

		//*********************************************************//
		//***	analog outputs for optical power modulation		***//
		//*********************************************************//

		///<General members
		ParamsPty _paramsPty;
		static string _counter;
		static string _triggerIn;
		long _numCam;
		long _numDigiLines;
		static long _driverType;
		static long _activeLoadMS;///<user defined unit load callback time
		long _activeLoadCount;///<user defined block ring buffer count

		///<Get/Set params
		static long  _sampleRateHz;
		static long _frameCount;
		static long _triggerMode;
		static std::wstring _waveformPathName; ///<waveform file's path and name

		///<Handles
		static HANDLE _hStopAcquisition; ///<event to stop frame acquisition
		static HANDLE _hThreadStopped; ///<Signals if the acquisition thread has stopped
		static HANDLE _hHardwareTriggerInEvent;	///<Signals if the trigger input is received

		///<Tasks
		static TaskHandle _taskHandleAOPockels; ///<pockels control
		static TaskHandle _taskHandleDO; ///<digital lines control
		static TaskHandle _taskHandleDI; ///<trigger input
		static TaskHandle _taskHandleCO0; ///<counter task

		string _pockelsLine[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsMaxVoltage[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsMinVoltage[MAX_GG_POCKELS_CELL_COUNT];
		double _pockelsPowerLevel[MAX_GG_POCKELS_CELL_COUNT];
		long _pockelsResponseType[MAX_GG_POCKELS_CELL_COUNT]; ///<power fit method:[0]sinusoidal [1]linear

		static int _digiBleachSelect;///<active digital lines bitwise selection [N,...,0]: [1]dummy,[2]pockelsDig,[4]complete,[8]cycle,[16]iteration,[32]pattern,[64]patternComplete...
		static int _numPockelsLines;///<user selected pockels line counts
		static std::string _pockelsLineStr;///<complete string for active pockels lines
		static std::string _digiLineStr;///<complete string for active digital lines
		std::vector<std::string> _digiLines;///<digital output line of waveform signal, exclude first dummy line trigger

		///<Task members
		static long _waveformTaskStatus; ///<task status of the waveform mode
		static long _finishedCycleCnt; ///<current finished cycle number for waveform mode
		static long _triggeredCycleCnt; ///<HW triggered cycle number for waveform mode, increment at start of current cycle
		static long _cycleDoneLength; ///<data length before cycle complete
		static long _dLengthPerAOCallback[SignalType::SIGNALTYPE_LAST]; ///<data count for analog out everyN callback
		static uint64_t _totalLength[SignalType::SIGNALTYPE_LAST]; ///<total data length for signals, including all frames consisting of Line BW and Frm BW
		static uint64_t _currentIndex[(int)(SignalType::SIGNALTYPE_LAST)]; ///<current copied index in waveform
		static long _precaptureStatus; ///<pre-capture status for waveform active load
		static GGalvoWaveformParams _gGalvoWaveParams;

	};

#ifdef __cplusplus
}
#endif

#pragma once
#include "stdafx.h"
#include "CameraConfig.h"
#include "DAQBoard.h"
#include "AlazarBoard.h"
#include "Types.h"
#include "WaveformManagerBase.h"
#include <queue>
#include <mutex>


#ifdef __cplusplus
extern "C"
{
#endif
	class ThorLSMCam :public ICamera
	{
	private:
		DAQBoard* _hDAQController;
		AlazarBoard* _hAlazarBoard;
		WaveformManagerBase* _hMesoScanWaveform;
		Scan* _preScan;
		std::unique_ptr<CameraConfig> _pCameraConfig;
		bool _alazarHasError;
		static std::once_flag _onceFlag;
		static unique_ptr<ThorLSMCam> _single;
		bool _isPockelsCalibrationRunning;

		//***	additional members	***//

		const int MAX_PIXEL;
		const int MIN_PIXEL;
		const int DEFAULT_PIXEL;
		const double DEFAULT_RES_VOLT;
		const uInt8 LOW_UINT8;
		const uInt8 HIGH_UINT8;
		const int MIN_INPUTRANGE;
		const int MAX_INPUTRANGE;
		const int DEFAULT_INPUTRANGE;


		ImageCompleteCallback _imageCompleteCallback;
		static CRITICAL_SECTION _accessSection; ///<critical section control for setup and start acquisition
		static HANDLE _hHardwareTriggerInEvent; ///<event set after HW trigger input
		std::unique_ptr<LoadMeso> _expLoader;///<experiment loader to create scan info from file
		std::timed_mutex _dmaMutex;///<Mutex to claim the exclusive access to the buffer
		int _acquireStatus;///<status to reflect current camera acquisition
		DMABufferInfo _bufferInfo;///<info of each dma buffer, used to determine recreation of dma
		unsigned short* _pFrmDllBuffer[MAX_DMABUFNUM];///<dma buffers in scan area index order
		map<long, long long> _bufferOrder;///<map of dma buffer index and completed frame index
		long long _indexOfLastCompletedFrame;///<counter for tracking the sequence index of the current frame (exclude scan area)
		long long _indexOfLastCopiedFrame;///<counter for tracking the index of the frames being copied out to the user (exclude scan area)
		long _bufCompleteID; ///<DMA index of current buffer in acquire
		long _pixelX; ///<pixel size X
		long _pixelY; ///<pixel size Y
		long _scanMode; ///<scan mode
		long _triggerMode; ///<trigger mode
		vector<Channel> _channels; ///<channels' info
		double* _tmpBuffer; ///<buffer for moving galvo tasks
		double _pockelsPowerLevel[MAX_GG_POCKELS_CELL_COUNT];	//power level percentange for pockels cell
		long _rsInitMode; ///<keep resonance scanner on
		long _inputRangeChannel[4];///<The digitizer input channel measurement range, see the enumeration of "inputrange"

	public:
		ThorLSMCam();
		~ThorLSMCam();
		static ThorLSMCam* getInstance();
		long GetLastErrorMsg(wchar_t * msg, long size);
		void SetStatusHandle(HANDLE handle);

		long FindCameras(long &cameraCount); 
		long SelectCamera(const long camera);
		long TeardownCamera();

		long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault); 
		long SetParam(const long paramID, const double param);
		long GetParam(const long paramID, double &param);
		long PreflightAcquisition(char * pDataBuffer);
		long SetupAcquisition(char * pDataBuffer);
		long StartAcquisition(char * pDataBuffer);
		long StatusAcquisition(long &status);
		long StatusAcquisitionEx(long &status, long &indexOfLastCompletedFrame);
		long CopyAcquisition(char * pDataBuffer, void* frameInfo);
		long PostflightAcquisition(char * pDataBuffer);

		long SetParamString(const long paramID, wchar_t * str);
		long GetParamString(const long paramID, wchar_t * str, long size);
		long GetParamBuffer(const long paramID, char * pBuffer, long size);
		long SetParamBuffer(const long paramID, char * pBuffer, long size);
		long InitCallBack(ImageCompleteCallback imageCompleteCallback);
		long GetMapParam(const long paramID, double inputValue, double &param);
		long SetMapParam(const long paramID, double inputValue, double param);
		long SaveConfigration();
		long GetDeviceConfigration(const long paramID, void** param);
		long GetCaptureTime(Scan scans[], const uint8_t scanSize, long& timeMillisecond);

		//fill partial DMA frame buffer when callback
		void ImageComplete(ImageCompleteStates state, ChanBufferInfo info, FrameROI roi, unsigned short* buffer, unsigned int size);
	private:
		void StripInfoChanged(StripInfo* stripInfoList);

		long IsHardwareReady();
		long SetBdDMA();  //Configure Alazar board
		long SetDAQBoard();  //Generate the DAQ tasks
		long SetWaveform(); //According the settings to generate waveform builder parameters
		long StartWaveform();
		long ReadPosition();
		long SetParameters();
		long StartScan();
		long StopScan();
		void CloseLSMCam();

		long MoveLightToPosition(GALVO_PARK_TYPE type);

		//***	additional private functions	***//

		long CheckHardware();
		long ResetDMABuffers(DMABufferInfo* dmaInfo, long ResetDMABuffers);
		long SetupParamAndWaveforms();
	};
#ifdef __cplusplus
}
#endif

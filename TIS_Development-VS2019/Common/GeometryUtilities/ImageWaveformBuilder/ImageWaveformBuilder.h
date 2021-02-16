#pragma once

#include "GeometryUtilitiesCpp.h"


#define SLOW_GALVO_RATE			1000.0			//Galvo: 1000Hz
#define INV_DIRECTION			-1
#define GALVO_LINEAR_DWELLTIME	50				//[us]
#define THRESHOLD_ANGLE			90				//[degree]
//#define FRAMETRIGGER_LOW_TIMEPOINTS 15

#ifdef LOGGING_ENABLED
extern std::auto_ptr<LogDll> logDll;
#endif
extern wchar_t message[_MAX_PATH];

/// Other than Square or Rectangle, ImageWaveformBuilder will handle other shapes' imaging waveform:
/// Polyline, ...
class ImageWaveformBuilder
{
private:

	static bool _instanceFlag;
	static std::unique_ptr<ImageWaveformBuilder> _single;
	static unsigned long _countPerCallback[SignalType::SIGNALTYPE_LAST]; ///<number of count per callback in active loading
	static uint64_t _countTotal[MAX_MULTI_AREA_SCAN_COUNT][SignalType::SIGNALTYPE_LAST][3]; ///<[0]:travel to start, [1]:waveform body (/file), [2]:idle patch for callback
	static uint64_t _countIndex[MAX_MULTI_AREA_SCAN_COUNT][(int)(SignalType::SIGNALTYPE_LAST)]; ///<index to reflect current location in waveform
	static WaveformGenParams _wParams;
	static GGalvoWaveformParams _gParams[MAX_MULTI_AREA_SCAN_COUNT]; ///< for area scan waveform
	static GGalvoWaveformParams _gWaveXY[MAX_MULTI_AREA_SCAN_COUNT]; ///< for initial travel in active load
	static ThorDAQGGWaveformParams _gThorDAQParams[MAX_MULTI_AREA_SCAN_COUNT]; ///< for area scan waveform
	static ThorDAQGGWaveformParams _gThorDAQWaveXY[MAX_MULTI_AREA_SCAN_COUNT]; ///< for initial travel in active load
	static BlockRingBuffer* _bRingBuffer[(int)(SignalType::SIGNALTYPE_LAST)]; ///<block ring buffers for image active loading
	static long _scanAreaId; ///<index of current scan area
	LineSegVec _lineSegs;
	int _daqType;
	int _daqSampleSize;
	double _deltaX_Volt;
	std::wstring _waveformFileName; ///<current waveform file
	int _digitalLineSelection; ///<user selected digital lines, in order: [1]dummy,[2]pockelsDig,[4]complete,[8]cycle,[16]iteration,[32]pattern,[64]patternComplete
	unsigned long _overallLines, _backwardLines, _forwardLines; //back+forward=Overall lines in a frame
	long _frameDataLength; ///<The total data sample length per frame, including Line BW and Frm BW		
	long _galvoDataLength; ///<The total galvo data sample length per frame, including Line BW and Frm BW		
	long _galvoSamplesPadding; ///<number of AO updates samples padded to the active (effective linear) samples
	long _galvoSamplesEffective; ///<number of AO updates samples, active (effective linear)
	long _galvoSamplesPerLine; ///<number of AO updates sample per line for the waveform of the galvo
	long _galvoFwdSamplesPerLine; ///<FwdLineSplLength + BwdLineSplLength = LineSplLength
	long _galvoBwdSamplesPerLine; ///<FwdLineSplLength + BwdLineSplLength = LineSplLength
	long _galvoDataForward;
	long _galvoDataBack;
	double _galvoXOffset;
	double _galvoYOffset;
	double _fieldX_volt;
	double _fieldY_volt;
	double _frameTime; ///<time to finish one frame
	long _PixelX; ///<lateral pixel counts, different in polyline mode
	long _lineFlybackLength; ///<number of line flyback count
	long _pockelsSamplesEffective; ///<number of pockels active samples
	long _lineFactor; ///<Two-Way:(2), One-Way:(1)
	double* _pGalvoWaveformX; ///<galvo X waveform
	double* _pGalvoWaveformY; ///<galvo Y waveform
	double* _outputXY; ///<galvo X & Y waveform for output on moving to start or others

	//private functions:
	ImageWaveformBuilder(int type);
	long SetupParamsForArea();

	//private functions: [for polyline]
	double AngleInTwoVectors(double Vx1, double Vy1, double Vx2, double Vy2);
	long CalculatePolyLinePixelX();
	std::vector<double> ConvertPixelToVolt(std::vector<long> vertices,double offsetVolt,double deltaX_volt);

	//private func: [for active load]
	long BuildTravelToStart(double* powerIdle, double* srcDstVxy, uint64_t& outCount);
	long BuildTravelToStartImagePos(long sAreaId, double stepVolts, double * srcVxy);
	uint64_t GetTotalCount(long& countPerCallback, SignalType sType, uint64_t stage1, uint64_t stage2);
	void InitializeParams();
	long ResetGGalvoWaveformParam(GGalvoWaveformParams* params, long* unitSize, long numPockels, long digitalLineCnt);
	long ResetThorDAQGGWaveformParam(ThorDAQGGWaveformParams* params, long* unitSize, long numPockels, long digitalLineCnt);
	long BuildGGFrameWaveformXY(void);
	long BuildGGInterleaveScanXY(void);
	long BuildGGLinePockels(void);
	long BuildGGFrameDigiLines(void);
	long ProcGGFrameWaveformXY(void);

public:
	~ImageWaveformBuilder()	{ ResetGGalvoWaveformParams(); _lineSegs.clear(); _instanceFlag = false; }
	static ImageWaveformBuilder* getInstance(long type);
	unsigned long GetForwardLines(){ return _forwardLines; }
	unsigned long GetOverallLines(){ return _overallLines; }
	long GetFrameDataLength(){ return _frameDataLength; }
	long GetLineDataLength(){ return _galvoSamplesPerLine; }
	long GetSamplesPadding(){ return _galvoSamplesPadding; }
	double GetFrameTime(){ return _frameTime; }
	long GetPockelsSamplesEffective(){ return _pockelsSamplesEffective; }
	long GetGGalvoWaveformParams(void* params);
	long GetGGalvoWaveformParams(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow);
	long GetThorDAQGGWaveformParams(SignalType sType, void* params, long preCaptureStatus, uint64_t& indexNow);
	long GetGGalvoWaveformStartLoc(const wchar_t* waveformFileName, double* startXY, long& clockRate);
	long GetThorDAQGGWaveformStartLoc(const wchar_t* waveformFileName, unsigned short* startXY, long& clockRate);
	static unsigned char GetPockelsCount();
	double* GetTravelWaveform(double stepSize, long outputInterleave, double* posFromXYToXY, long& count);
	void SetWaveformGenParams(void* params);
	void ResetGGalvoWaveformParams();
	long VerifyPolyLine(std::vector<long> Ptx, std::vector<long> Pty, long fieldSize, double field2Volts, double fieldScaleFineX, double fieldScaleFineY, long PixelY, long &PixelX);
	long BuildPolyLine();

	//[for active load]
	static void BufferAvailableCallbackFunc(long sType, long bufSpaceNum);
	long BuildSpiral(long count); ///<test function for active load to move galvo in spiral
	void ConnectBufferCallback(SignalType sType, BlockRingBuffer* brBuf); ///<register to buffer available callback
	uint64_t GetCounter(SignalType sType);
	long BuildImageWaveform(double* startXY, long* countPerCallback, uint64_t* total, wstring outPath); ///<build image waveform and save if outPath configured
	long BuildImageWaveformFromStart(long rebuild, double stepVolts, double * currentVxy, long* countPerCallback, uint64_t* total); ///<rebuild image waveform from current position
	uint64_t RebuildWaveformFromFile(const wchar_t* waveformFileName, double * currentVxy, int digLineSelection, long* countPerCallback); ///<rebuild waveform from current position to complete waveform file, return total counts
	uint64_t RebuildThorDAQWaveformFromFile(const wchar_t* waveformFileName, unsigned short * currentVxy, int digLineSelection, long* countPerCallback); ///<rebuild waveform from current position to complete waveform file, return total counts
	void ResetCounter(){ for (int j=0;j < MAX_MULTI_AREA_SCAN_COUNT; j++) { for (int i=0;i < (int)(SignalType::SIGNALTYPE_LAST); i++) {_countIndex[j][i] = 0;}}} ///<reset counter index for potential re-execute of the same waveform
	void CloseWaveformFile();
};
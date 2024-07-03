#pragma once
#include "stdafx.h"
#include "Types.h"
#include "Includes\NIDAQmx.h"

#define BUFFER_READ_LENGTH (75000)
#define START_XPOS (0.0)

class MesoScanWaveform;
typedef struct WaveformGenerateParams
{
	int pockelsPointCount;
	double dutyCycle;
	double physicalStripeWidth;
	unsigned int stripWidth;
	double yPixelSize;
	double pokelsVolt;
	double zOffset;
	double zPixelSize;
	bool isTwoWay;
}WaveformGenerateParams;

class WaveformBuffer
{
private:
	double*	waveformBufferData;
	long waveformBufferLength;
	long waveformBufferIndex;
	double*	waveformBufferDataNew;
	long waveformBufferLengthNew;
	MesoScanWaveform* pCBObj;
	long lastGetBufLength;
	bool newBufferNeeded;
	void(MesoScanWaveform::*NewBufferCallback)(WaveformBuffer *);	//Callback when new buffer is needed to generate
	void(MesoScanWaveform::*RemoveBufferCallback)(double* buffer);	//Callback when an old buffer output is ended and can be released
public:
	WaveformBuffer();
	~WaveformBuffer();
	void RegisterCallback(MesoScanWaveform* pObj, void(MesoScanWaveform::*newCallback)(WaveformBuffer *), void(MesoScanWaveform::*removeCallback)(double* buffer));
	bool InitBuffer(double* buffer, long length);	//Initialize with the first part of waveform
	bool AddBuffer(double* buffer, long length);	//Add new waveform buffer to the queue
	bool GetData(double* dataBuffer, long dataBufferLength, long &outputLength);	//Read waveform data from buffer
	void ResetBuffer();		//Reset buffer queue when the output is stopped
};

class MesoScanWaveform
{
	enum BufferType
	{
		GALVOBUFFER,
		POKELSBUFFER,
		VOICECOILBUFFER,
		GALVOX1BUFFER,
		GALVOX2BUFFER
	};
private:
	vector<ScanROI> _rois;
	WaveformGenerateParams _params;

	HANDLE _threadHandle;
	DWORD _threadID;
	HANDLE _hStartWriteBuffer;
	HANDLE _hEndWriteBuffer;
	HANDLE _hWritingBuffer;

	vector<ScanROI>::iterator _currentROI;
	long _currentStripeIdx;
	long _currentZIdx;
	vector<ScanROI>::iterator _nextROI;
	long _nextStripeIdx;
	long _nextZIdx;
	long _waveformBufferWriteLength;
	bool _endflag;

	double _f2TGy;
	double _f2TGx1;
	double _f2TGx2;
	double _theta2Volts;

	float64*	_galvoWaveformMemory1;
	float64*	_pokelsWaveformMemory1;
	float64*	_voiceCoilWaveformMemory1;
	float64*	_galvoX1WaveformMemory1;
	float64*	_galvoX2WaveformMemory1;
	bool	_galvoWaveformMemory1Available;
	bool	_pokelsWaveformMemory1Available;
	bool	_voiceCoilWaveformMemory1Available;
	bool	_galvoX1WaveformMemory1Available;
	bool	_galvoX2WaveformMemory1Available;
	float64*	_galvoWaveformMemory2;
	float64*	_pokelsWaveformMemory2;
	float64*	_voiceCoilWaveformMemory2;
	float64*	_galvoX1WaveformMemory2;
	float64*	_galvoX2WaveformMemory2;
	bool	_galvoWaveformMemory2Available;
	bool	_pokelsWaveformMemory2Available;
	bool	_voiceCoilWaveformMemory2Available;
	bool	_galvoX1WaveformMemory2Available;
	bool	_galvoX2WaveformMemory2Available;

	float64*	_galvoWaveformData;
	float64*	_pokelsWaveformData;
	float64*	_voiceCoilWaveformData;
	float64*	_galvoX1WaveformData;
	float64*	_galvoX2WaveformData;

	WaveformBuffer _galvoBufferQueue;
	WaveformBuffer _pokelsBufferQueue;
	WaveformBuffer _voicecoilBufferQueue;
	WaveformBuffer _galvoX1BufferQueue;
	WaveformBuffer _galvoX2BufferQueue;

	bool	_galvoCallBackFlag;
	bool	_pokelsCallBackFlag;
	bool	_voicecoilCallBackFlag;
	bool	_galvoX1CallBackFlag;
	bool	_galvoX2CallBackFlag;

	double* _voiceCoilTable;
	int _voiceCoilTableWidth;
	int _voiceCoilTableHeight;

	double* _galvoVoiceCoilWaveform;
	double* _galvoXWaveform;
	double* _pockelWaveform;
	StripInformation _stripInformation;

private:
	double GalvoX1FieldToVolts(double size) { return size * _f2TGx1 * _theta2Volts; }
	double GalvoX2FieldToVolts(double size) { return size * _f2TGx2 * _theta2Volts; }
	double GalvoYFieldToVolts(double size) { return size * _f2TGy * _theta2Volts; }
	int GetZCount(vector<ScanROI>::iterator pROI){return (int)ceil(pROI->ZHeight / _params.zPixelSize);}
	int GetStripeCount(vector<ScanROI>::iterator pROI){ return (int)ceil(pROI->XWidth / _params.physicalStripeWidth); }
	void SetLength(long length){_waveformBufferWriteLength = length;};
	void GetStripeList(vector<ScanROI>::iterator pROI1, long zIdx1, long stripeIdx1, long length, vector<ScanROI>::iterator &pROI2, long &zIdx2, long &stripeIdx2, vector<Stripe>& stripeList, Stripe* &pNextStripe, long &lengthOut);
	long GetStripeLength(vector<ScanROI>::iterator pROI){ return _params.pockelsPointCount * LineCount(pROI);}
	int LineCount(vector<ScanROI>::iterator pROI){return _params.isTwoWay ? (int)ceil(pROI->YHeight / _params.yPixelSize) : 2*(int)ceil(pROI->YHeight / _params.yPixelSize);}
	long GetBufferLength(){ return _waveformBufferWriteLength;}
	double GetStripeShift() { return GalvoX1FieldToVolts(_params.physicalStripeWidth * (1 - _params.dutyCycle) / _params.dutyCycle); }

	double* SelectBuffer(BufferType type);

	void GalvoWaveformNewCallback(WaveformBuffer* waveformBuffer);
	void PokelsWaveformNewCallback(WaveformBuffer* waveformBuffer);
	void VoiceCoilWaveformNewCallback(WaveformBuffer* waveformBuffer);
	void GalvoX1WaveformNewCallback(WaveformBuffer* waveformBuffer);
	void GalvoX2WaveformNewCallback(WaveformBuffer* waveformBuffer);

	void CheckForWaveFormUpdate(BufferType type);

	void GalvoWaveformRemoveCallback(double* buffer);
	void PokelsWaveformRemoveCallback(double* buffer);
	void VoiceCoilWaveformRemoveCallback(double* buffer);
	void GalvoX1WaveformRemoveCallback(double* buffer);
	void GalvoX2WaveformRemoveCallback(double* buffer);

	void InitializeVoiceCoilWaveform();

	long GetBlankLengthByLine(double xOffset, double yOffset, double zOffset);
	long GetBlankLengthByPoint(double xOffset, double yOffset, double zOffset);
	long GetCurrentStripeBlankLengthByLine(vector<ScanROI>::iterator pROI, long zIdx, long stripeIdx);
	long GetCurrentStripeBlankLengthByPoint(vector<ScanROI>::iterator pROI, long zIdx, long stripeIdx);

	static void threadWriteBuffer(LPVOID lParam);

	void SetWaveformParam();
	void SetROIs(vector<ScanROI>& roisIn);
	bool ReadGalvoVoiceCoilBuffer(double* data, long channelLength, long &outputLength);
	bool ReadGalvoXBuffer(double* data, long channelLength, long &realLength);
	void ReleaseBuffer();

public:
	MesoScanWaveform();
	long InitHardwareInfo(double f2TGy, double f2TGx1, double f2TFx2, double gRfrequence, double waveformSampleClock);
	~MesoScanWaveform();

	double* GetGalvoVoiceCoilWaveForm(long length, long &lengthOut, bool& isContinue);
	double* GetGalvoXWaveForm(long length, long &lengthOut, bool& isContinue);
	double* GetPockelWaveForm();

	long GenerateWaveForms(WaveformParams* wP);
	long RestartGeneration();

	void GetStartPos(double &x1, double &x2, double &y);

	long ClearFrameBuffers();

	StripInformation GetStripInformation();

};


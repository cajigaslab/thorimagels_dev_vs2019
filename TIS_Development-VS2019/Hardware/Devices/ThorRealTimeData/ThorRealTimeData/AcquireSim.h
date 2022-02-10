#pragma once
#include "stdafx.h"
#include "AcquireData.h"
#include "RealTimeDataXML.h"

//**************************************************************************************//
//*** class to implement ThorSync Simulator.										 ***//
//*** All functions should be as close as NI did. 									 ***//
//*** Start(), Stop(): start or stop acquisition, will save to file in SaveThread	 ***//
//*** if file name is defined, otherwise, will only update display buffer for GUI.	 ***//
//*** GetAcquiring() get to know the status of acquisition.			 				 ***//
//**************************************************************************************//

#define T_FREQ		1.0
#define AMPLITUDE	2.0

using namespace std;

typedef enum SampleModeEnum
{
	SimFinite,
	SimContinuous
}SampleMode;

typedef struct SimData
{
	int nAI;
	int nDI;
	int nCI;
	int nVI;
	unsigned long * gCtr;
	double * aiData;
	unsigned char* diData;
	double *aiSpectrum;
	unsigned long * diSpectrum;
	unsigned long * ciData;
	double * viData;
}SimDataStruct;

class AcquireSim : public IAcquireRealTimeData
{	
private:	//members:
	std::vector<std::string> _ailineName;
	std::vector<std::string> _dilineName;

public:		//members
	static StimulusSaveStruct* simStimulusParams;
	static std::vector<std::string> counterNames;
	static signed long sampleMode;
	static long interleave;
	static unsigned long long _samplesPerCallback;
	static unsigned long long _displayPerCallback;
	static CompoundData* _displayCompData;
	static double _threadTime;
	static BOOL _isAcquiring;
	static BOOL _isAsyncAcquiring;
	static BOOL _isSaving;

	static UINT_PTR _hTimer;
	static volatile size_t _saveThreadCnt;
	static volatile size_t _saveThreadFinishedCnt;
	static volatile size_t _totalNumThreads;
	static SimDataStruct * _simData;
	static std::list<HANDLE> _hSaveThreads;
	static HANDLE _hProcessSaveThread;
	static BOOL _inSaveThread;
	static unsigned long gCtrOverflowCnt;

public:		//functions:
	AcquireSim();
	~AcquireSim();
	static void TimerCallback(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
	static long StaticStop();

	//virtual functions' implementation:
	virtual long Enter();
	virtual long Exit();
	virtual long SetupChannels();
	virtual long SetupFileIO();
	virtual long Start();
	virtual long Pause();
	virtual long Restart();
	virtual long Stop();
	virtual long GetAcquiring();
	virtual long GetAsyncAcquiring();
	virtual long GetSaving();
	virtual long SetSaving(long toSave);
	virtual long InitCallbacks(SpectralUpdateCallback su, DataUpdateCallback du);
	virtual long CopyStructData(void* ptr);
	virtual long Status();
	virtual long LoadXML();
	virtual long StartAsync();
	virtual long StopAsync();
	virtual std::wstring GetLastError();


private:	//functions:
	void DeleteCriticalSections();	
	void InitializeCriticalSections();
	void ResetParams();
	void ResetTimingParams();
	long SetupHDF5File();
};

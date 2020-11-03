#pragma once

//***********************************************************************//
//*** Abstract class for simutaneously acquire of multiple channels   ***//
//*** including input/output of Analog/Digital/counter channel types, ***//
//*** allow setting of sampleRate, sampleMode (finite/Contineous),	  ***//
//*** and Hardware trigger  ...etc.								      ***//
//***********************************************************************//

class IAcquireRealTimeData
{
public:
	virtual long Enter() = 0;
	virtual long Exit() = 0;
	virtual long SetupChannels() = 0;
	virtual long SetupFileIO() = 0;
	virtual long Start() = 0;
	virtual long Pause() = 0;
	virtual long Restart() = 0;
	virtual long Stop() = 0;
	virtual long GetAcquiring() = 0;
	virtual long GetAsyncAcquiring() = 0;
	virtual long GetSaving() = 0;
	virtual long SetSaving(long toSave) = 0;
	virtual long InitCallbacks(SpectralUpdateCallback su, DataUpdateCallback du) = 0;
	virtual long CopyStructData(void* ptr) = 0;
	virtual long Status() = 0;
	virtual long LoadXML() = 0;
	virtual long StartAsync() = 0;
	virtual long StopAsync() = 0;
	virtual std::wstring GetLastError() = 0;
};

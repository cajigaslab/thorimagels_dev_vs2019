#pragma once

#ifdef _WIN32
#include <io.h> 
#define access    _access_s
#else
#include <unistd.h>
#endif

static bool FileExists(const std::string& Filename)
{
	return access(Filename.c_str(), 0) == 0;
}

#ifdef LOGGING_ENABLED
extern std::auto_ptr<LogDll> logDll;
#endif
extern wchar_t message[_MAX_PATH];

/// Other than Square or Rectangle, ImageWaveformBuilder will handle other shapes' imaging waveform:
/// Polyline, ...
class WaveformSaver : IWaveformSaver
{
private:

	//members
	static bool _instanceFlag;
	static std::unique_ptr<WaveformSaver> _single;
	static HANDLE _hInitialized; ///<Signals for waveform being pushed to buffer

	GGalvoWaveformParams _gParams;

	//functions
	WaveformSaver();


public:
	~WaveformSaver() { _instanceFlag = false; }
	static WaveformSaver* getInstance();

	virtual long SaveData(wstring outPath, SignalType stype, void* gparam, unsigned long long length);

};
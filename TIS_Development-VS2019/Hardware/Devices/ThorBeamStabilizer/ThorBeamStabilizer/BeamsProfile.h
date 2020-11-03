#pragma once
#include <visa.h>
#include <Windows.h>

class BeamsProfile
{
public:

	BeamsProfile();
	~BeamsProfile();

	bool Connect(std::string serialA, std::string serialB);
	bool Disconnect();
	bool GetData(const long deviceIdx, double &diameter, double &centerX, double &centerY, double &saturation, double &exposureTime);
	void SetClipLevel(double clipLevel);
	std::string GetSerialNumberBPA();
	std::string GetSerialNumberBPB();

	enum
	{
		BEAM_PROFILE_NUM = 2
	};

private:
	std::array<std::string, BEAM_PROFILE_NUM>_deviceSN;
	std::array<std::string, BEAM_PROFILE_NUM> _deviceAddress;
	std::array<ViSession, BEAM_PROFILE_NUM> _viSession;
	CritSect _critSect;
	double _clipLevel;
	bool _connectionStablished;

	const std::wstring RESOURCE_NAME_STRUCTURE;
};


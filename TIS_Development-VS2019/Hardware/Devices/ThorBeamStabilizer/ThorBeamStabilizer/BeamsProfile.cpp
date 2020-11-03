#include "stdafx.h"
#include "BeamsProfile.h"
#include "Strsafe.h"

#define ADDRESS_LEN 256
#define PRECISION_MODE 0
#define PIXEL_SIZE 6.45f

std::unique_ptr<TLBC1Dll> tlbc1Dll(new TLBC1Dll(L"TLBC1_64.dll"));

BeamsProfile::BeamsProfile() :
	RESOURCE_NAME_STRUCTURE(L"USB::0x1313::0x8012::%S::RAW"),
	_clipLevel(0.3),
	_connectionStablished(false),
	_viSession({})
{
}

BeamsProfile::~BeamsProfile()
{
}

bool BeamsProfile::Connect(std::string serialA, std::string serialB)
{
	Lock lock(_critSect);
	if (_connectionStablished) return true;
	ViStatus err = VI_SUCCESS;	///< get the error code returned by the driver functions
	ViUInt32 deviceCnt = 0;				///< number of connected devices
	ViChar* rscPtr = VI_NULL;				///< pointer to resource string

	// get the number of connected devices
	if(0 != tlbc1Dll->TLBC1_get_device_count(VI_NULL, &deviceCnt))
	{
		//if there is an error return false
		return false;
	}

	//at least 2 beam profilers need to be connected for proper functionality
	//if there are less than to devices connected, return false
	if (2 > deviceCnt) 
	{
		return false;
	}

	bool ret = true;

	_deviceSN[0] = serialA;
	_deviceSN[1] = serialB;

	std::string* detectedAddress = new std::string[deviceCnt];
	std::string* detectedSN = new std::string[deviceCnt];
	long* availableDev = new long[deviceCnt];

	//
	for(unsigned long i = 0; i < deviceCnt; ++i)
	{
		ViChar serial_number[ADDRESS_LEN];
		ViBoolean available;			
		ViChar resource_name[ADDRESS_LEN];
		err = tlbc1Dll->TLBC1_get_device_information(VI_NULL,				// vi (not used)
			i,						// device_index
			VI_NULL,				// manufacturer
			VI_NULL,				// model_name
			serial_number,			// serial_number
			&available,				// device_available
			resource_name);			// name
		availableDev[i] = available;
		if(available)
		{
			detectedSN[i] = serial_number;
			detectedAddress[i] = resource_name;			
		}
	}

	//build the devices address
	for (long i = 0; i < BEAM_PROFILE_NUM; ++i)
	{
		//use the address structure to complete the address with the serial number
		TCHAR wsAddress[ADDRESS_LEN];		
		StringCbPrintf(wsAddress, ADDRESS_LEN, RESOURCE_NAME_STRUCTURE.c_str(), _deviceSN[i].c_str());
		_deviceAddress[i] = ConvertWStringToString(wsAddress);

		//compare the detected device serial number to the serial number in the settings file
		//if the serial number is repeated then set the detectedDeviceStatus for the
		//corresponding detected device
		for (unsigned long j = 0; j < deviceCnt; ++j)
		{
			if (_deviceAddress[i] == detectedAddress[j] || detectedSN[j] == _deviceSN[i])
			{
				availableDev[j] = FALSE;
			}
		}
	}

	for(unsigned long i = 0; i < BEAM_PROFILE_NUM; ++i)
	{
		//if the serial number is "NA" try to fill up the address with one of the
		//detected devices. This will only succed if there are extra detected devices
		//not found in the settings file.
		if ("NA" == _deviceSN[i]) 
		{
			for (unsigned long j = 0; j < deviceCnt; ++j)
			{
				if (TRUE == availableDev[j])
				{
					availableDev[j] = FALSE;
					_deviceSN[i] = detectedSN[j];
					_deviceAddress[i] = detectedAddress[j];
					break;
				}
			}
		}
	}

	for (long i = 0; i < BEAM_PROFILE_NUM; ++i)
	{
		ViChar resource_name[ADDRESS_LEN];
		memcpy(resource_name, _deviceAddress[i].c_str(), ADDRESS_LEN);

		rscPtr = resource_name;
		err = tlbc1Dll->TLBC1_init(rscPtr, VI_TRUE, VI_FALSE, &_viSession[i]);

		//return false if there is any error connecting
		if (err != VI_SUCCESS) ret = false;

		err = tlbc1Dll->TLBC1_set_clip_level(_viSession[i], _clipLevel);
		err = tlbc1Dll->TLBC1_set_precision_mode(_viSession[i], PRECISION_MODE);
		err = tlbc1Dll->TLBC1_set_auto_exposure(_viSession[i], TRUE);
	}

	//delete allocated memory
	delete[] detectedAddress;
	delete[] detectedSN;
	delete[] availableDev;

	_connectionStablished = ret;

	return ret;
}

bool BeamsProfile::Disconnect()
{
	Lock lock(_critSect);
	for (long i = 0; i < BEAM_PROFILE_NUM; ++i)
	{
		tlbc1Dll->TLBC1_close(_viSession[i]);
	}
	_connectionStablished = false;
	return true;
}

bool BeamsProfile::GetData(const long deviceIdx, double &diameter, double &centerX, double &centerY, double &saturation, double &exposureTime)
{
	Lock lock(_critSect);
	//ensure the devices are connected and the index is correct
	if (!_connectionStablished || BEAM_PROFILE_NUM <= deviceIdx || 0 > deviceIdx)
	{
		return false;
	}

	ViStatus err = VI_SUCCESS;	///< get the error code returned by the driver functions

	//retrieve calculations from beam profiler
	TLBC1_Calculations calculations;
	err = tlbc1Dll->TLBC1_get_scan_data(_viSession[deviceIdx], &calculations);

	//check for error before using calculation values
	if (err != VI_SUCCESS) return false;

	//set the data values
	diameter = (double)calculations.ellipseDiaMean * PIXEL_SIZE;
	centerX = ((double)calculations.centroidPositionX - calculations.imageWidth / 2) * PIXEL_SIZE;
	centerY = ((double)calculations.imageHeight / 2 - calculations.centroidPositionY) * PIXEL_SIZE;
	saturation = calculations.saturation;

	//retrieve the exposure time
	double expTime;
	err = tlbc1Dll->TLBC1_get_exposure_time(_viSession[deviceIdx], &expTime);

	//check for error before using exposure time value
	if (err != VI_SUCCESS) return false;

	exposureTime = expTime;

	return true;
}

void BeamsProfile::SetClipLevel(double clipLevel)
{
	_clipLevel = clipLevel;
}

std::string BeamsProfile::GetSerialNumberBPA()
{
	return _deviceSN[0];
}

std::string BeamsProfile::GetSerialNumberBPB()
{
	return _deviceSN[1];
}

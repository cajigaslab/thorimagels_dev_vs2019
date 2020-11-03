// ThorBeamStabilizer.cpp : Defines the exported functions for the DLL application.
//

//$T indentinput.cpp GC 1.140 05/18/10 14:20:27

// ThorBeamStabilizer.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "ThorBeamStabilizer.h"
#include "ThorBeamStabilizerXML.h"


#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif 

/// <summary>
/// The message
/// </summary>
wchar_t message[MSG_SIZE];
long APT_MAX_STEP_MOVE = 2000;


/// <summary>
/// Prevents a default instance of the <see cref="ThorBeamStabilizer"/> class from being created.
/// </summary>
ThorBeamStabilizer::ThorBeamStabilizer() :
	_beamsProfile(nullptr),
	_piezoActuators(nullptr),
	_connectionStablished(false),
	_stabilizeBeam(false),
	_stabilizeBeam_C(false),
	_stabilizeBeam_B(false),
	_factoryPosBeam(false),
	_factoryPosBeam_C(false),
	_factoryPosBeam_B(false),
	_deadBand(30),
	_maxExposureTime(100.0),
	_minExposureTime(1.0),
	_alignTimeoutSec(30),
	_stabilizingThreadActive(false),
	_errMsg(),
	_pTerm({}),
	_piezoActuatorStart({}),
	_piezoOrientation({})
{
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorBeamStabilizer"/> class.
/// </summary>
ThorBeamStabilizer::~ThorBeamStabilizer()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
}

bool ThorBeamStabilizer:: _instanceFlag = false;

std::unique_ptr<ThorBeamStabilizer> ThorBeamStabilizer::_single(new ThorBeamStabilizer());
HANDLE ThorBeamStabilizer::_hThread = NULL;

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorBeamStabilizer *.</returns>
ThorBeamStabilizer *ThorBeamStabilizer::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorBeamStabilizer());
		_instanceFlag = true;

		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::FindDevices(long &deviceCount)
{
	_connectionStablished = false;
	long portId, baudRate;
	std::string bpsnA, bpsnB;

	try
	{
		std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());

		//retrieve the serial device port Id and baud rate from the settings xml.
		//these will be used to connect to the device
		pSetup->GetPiezoActuatorsConnection(portId, baudRate);

		//retrieve the serial device serial numbers from the settings xml.
		//the serial numbers will be used to connect to the Beam profilers	
		pSetup->GetBeamProfilerSerialNumber("BeamProfilerA", bpsnA);
		pSetup->GetBeamProfilerSerialNumber("BeamProfilerB", bpsnB);

	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBeamStabilizerSettings.xml file");
		deviceCount = 0;
		return false;
	}

	if (nullptr == _beamsProfile)
	{
		_beamsProfile = new BeamsProfile();
	}

	if (nullptr == _piezoActuators)
	{
		_piezoActuators = new PiezoInertiaActuators();
	}

	if (!_beamsProfile->Connect(bpsnA, bpsnB)) 
	{
		deviceCount = 0;
		return false;
	}

	if (!_piezoActuators->Connect(portId, baudRate))
	{
		deviceCount = 0;
		return false;
	}

	deviceCount = 1;
	_connectionStablished = true;

	return true;
}

/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::SelectDevice(const long device)
{
	long ret = true;

	if (nullptr == _beamsProfile || nullptr == _piezoActuators || !_connectionStablished)
	{
		ret = false;
	}

	_piezoActuators->StartStatusUpdates();

	for (long i = 0; i < PiezoInertiaActuators::PIEZO_ACTUATORS_NUM; ++i)
	{
		_piezoActuators->GetPiezoPosition(i, _piezoActuatorStart[i]);
	}

	long piezoSettingsPositons[PiezoInertiaActuators::PIEZO_ACTUATORS_NUM];

	try
	{
		std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());

		pSetup->GetPiezoActuatorPositions(piezoSettingsPositons[0], piezoSettingsPositons[1], piezoSettingsPositons[2], piezoSettingsPositons[3]);
		double clipLevel = 0.3;
		pSetup->GetControlSettings(_piezoOrientation[0], _piezoOrientation[1], _piezoOrientation[2], _piezoOrientation[3], _deadBand, _pTerm[0], _pTerm[1], _maxExposureTime, _minExposureTime, clipLevel, _alignTimeoutSec);
		_beamsProfile->SetClipLevel(clipLevel);

		long factoryPiezoPosition[4] = {0,0,0,0};
		long factoryPiezoStepLimit = 0;

		pSetup->GetFactoryPiezoActuatorPositions(factoryPiezoPosition[0], factoryPiezoPosition[1], factoryPiezoPosition[2], factoryPiezoPosition[3], factoryPiezoStepLimit);
		_piezoActuators->SetPiezoStepLimit(factoryPiezoStepLimit);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorBeamStabilizerSettings.xml file");
		return false;
	}

	for (long i = 0; i < PiezoInertiaActuators::PIEZO_ACTUATORS_NUM; ++i)
	{			
		_piezoActuators->SetPiezoPosition(i,piezoSettingsPositons[i]);
		_piezoActuatorStart[i] = piezoSettingsPositons[i];
	}

	return ret;
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizer::TeardownDevice()
{
	_beamsProfile->Disconnect();

	_piezoActuators->StopStatusUpdates();
	_piezoActuators->Disconnect();

	_connectionStablished = false;

	return true;
}

/// <summary>
/// Gets the parameter information.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramType">Type of the parameter.</param>
/// <param name="paramAvailable">The parameter available.</param>
/// <param name="paramReadOnly">The parameter read only.</param>
/// <param name="paramMin">The parameter minimum.</param>
/// <param name="paramMax">The parameter maximum.</param>
/// <param name="paramDefault">The parameter default.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::GetParamInfo
	(
	const long	paramID,
	long		&paramType,
	long		&paramAvailable,
	long		&paramReadOnly,
	double		&paramMin,
	double		&paramMax,
	double		&paramDefault
	)
{
	long ret = true;
	switch (paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:		
		paramType		= IDevice::TYPE_LONG;
		paramAvailable	= true;
		paramMin		= static_cast<double>(BEAM_STABILIZER);
		paramMax		= static_cast<double>(BEAM_STABILIZER);
		paramDefault	= static_cast<double>(BEAM_STABILIZER);
		paramReadOnly	= true;
		break;

	case IDevice::PARAM_BEAM_STABILIZER_BPA_SN:
		paramType		= IDevice::TYPE_STRING;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= 0;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;

	case IDevice::PARAM_BEAM_STABILIZER_BPB_SN:
		paramType		= IDevice::TYPE_STRING;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= 0;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_REALIGNBEAM:
		paramType		= IDevice::TYPE_BOOL;
		paramAvailable	= true;
		paramMin		= false;
		paramMax		= true;
		paramDefault	= false;
		paramReadOnly	= false;
		break;
	case IDevice::PARAM_CONNECTION_STATUS:
		paramType		= IDevice::TYPE_BOOL;
		paramAvailable	= true;
		paramMin		= (double)ConnectionStatusType::CONNECTION_WARMING_UP;
		paramMax		= (double)ConnectionStatusType::CONNECTION_ERROR_STATE;
		paramDefault	= (double)ConnectionStatusType::CONNECTION_READY;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_CENTER_X:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= _deadBand;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_CENTER_Y:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= _deadBand;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_EXPOSURE:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= _minExposureTime;
		paramMax		= _maxExposureTime;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_CENTER_X:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= _deadBand;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_CENTER_Y:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= _deadBand;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_EXPOSURE:
		paramType		= IDevice::TYPE_DOUBLE;
		paramAvailable	= true;
		paramMin		= _minExposureTime;
		paramMax		= _maxExposureTime;
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO1_POS:
		paramType		= IDevice::TYPE_LONG;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= (nullptr == _piezoActuators)? 0 : _piezoActuators->GetPiezoStepLimit();
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO2_POS:
		paramType		= IDevice::TYPE_LONG;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= (nullptr == _piezoActuators)? 0 : _piezoActuators->GetPiezoStepLimit();
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO3_POS:
		paramType		= IDevice::TYPE_LONG;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= (nullptr == _piezoActuators)? 0 : _piezoActuators->GetPiezoStepLimit();
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO4_POS:
		paramType		= IDevice::TYPE_LONG;
		paramAvailable	= true;
		paramMin		= 0;
		paramMax		= (nullptr == _piezoActuators)? 0 : _piezoActuators->GetPiezoStepLimit();
		paramDefault	= 0;
		paramReadOnly	= true;
		break;
	case IDevice::PARAM_BEAM_STABILIZER_FACTORY_RESET_PIEZOS:
		paramType		= IDevice::TYPE_BOOL;
		paramAvailable	= true;
		paramMin		= false;
		paramMax		= true;
		paramDefault	= false;
		paramReadOnly	= false;
		break;
	default:
		paramAvailable = false;
		ret = false;
		break;
	}

	return ret;	
}

/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::SetParam(const long paramID, const double param)
{
	long ret = true;
	switch (paramID)
	{
	case IDevice::PARAM_BEAM_STABILIZER_REALIGNBEAM:
		_stabilizeBeam = true;
		break;

	case IDevice::PARAM_BEAM_STABILIZER_FACTORY_RESET_PIEZOS:
		_factoryPosBeam = true;
		break;
	default:
		ret = false;
		break;
	}
	return ret;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::GetParam(const long paramID, double &param)
{
	long ret = false;
	switch(paramID)
	{
	case IDevice::PARAM_DEVICE_TYPE:
		{
			param =  static_cast<double>(BEAM_STABILIZER);
			ret = true;
		}
		break;	
	case IDevice::PARAM_BEAM_STABILIZER_REALIGNBEAM:
		{
			param = false;
			ret = true;
		}
		break;
	case IDevice::PARAM_CONNECTION_STATUS:
		{
			param = (_connectionStablished) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			ret = true;
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_CENTER_X:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 0;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = centroidX;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_CENTER_Y:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 0;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = centroidY;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPA_EXPOSURE:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 0;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = exposureTime;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_CENTER_X:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 1;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = centroidX;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_CENTER_Y:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 1;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = centroidY;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_EXPOSURE:
		if (nullptr !=_beamsProfile)
		{
			const long BP_INDEX = 1;
			double diameter, centroidX, centroidY, saturation, exposureTime;		 
			if (_beamsProfile->GetData(BP_INDEX, diameter, centroidX, centroidY, saturation, exposureTime))
			{
				param = exposureTime;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO1_POS:
		if (nullptr !=_beamsProfile)
		{
			const long PIEZO_INDEX = 0;
			long piezoPos = 0;		 
			if (_piezoActuators->GetPiezoPosition(PIEZO_INDEX, piezoPos))
			{
				param = piezoPos;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO2_POS:
		if (nullptr !=_beamsProfile)
		{
			const long PIEZO_INDEX = 1;
			long piezoPos = 0;		 
			if (_piezoActuators->GetPiezoPosition(PIEZO_INDEX, piezoPos))
			{
				param = piezoPos;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO3_POS:
		if (nullptr !=_beamsProfile)
		{
			const long PIEZO_INDEX = 2;
			long piezoPos = 0;		 
			if (_piezoActuators->GetPiezoPosition(PIEZO_INDEX, piezoPos))
			{
				param = piezoPos;
				ret = true;
			}
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_PIEZO4_POS:
		if (nullptr !=_beamsProfile)
		{
			const long PIEZO_INDEX = 3;
			long piezoPos = 0;		 
			if (_piezoActuators->GetPiezoPosition(PIEZO_INDEX, piezoPos))
			{
				param = piezoPos;
				ret = true;
			}
		}
		break;		
	case IDevice::PARAM_BEAM_STABILIZER_FACTORY_RESET_PIEZOS:
		{
			if(nullptr !=_beamsProfile)
			{
				param = false;
				ret = true;
			}
		}		
		break;
	}
	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = true;

	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = true;

	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::SetParamString(const long paramID, wchar_t* str)
{
	long ret = true;

	return ret;
}

std::wstring utf8toUtf16(const std::string & str)
{
	if (str.empty())
		return std::wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw std::runtime_error("Failed converting UTF-8 string to UTF-16");

	std::vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0, 
		str.data(), static_cast<long>(str.size()), &buffer[0], static_cast<long>(buffer.size()));
	if (charsConverted == 0)
		throw std::runtime_error("Failed converting UTF-8 string to UTF-16");

	return std::wstring(&buffer[0], charsConverted);
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = false;
	switch (paramID)
	{
	case IDevice::PARAM_BEAM_STABILIZER_BPA_SN:
		if (nullptr != _beamsProfile)
		{			
			std::string sn = _beamsProfile->GetSerialNumberBPA();
			std::wstring wsTemp = utf8toUtf16(sn);
			wcscpy_s(str,size, wsTemp.c_str());
			ret = true;
		}
		break;
	case IDevice::PARAM_BEAM_STABILIZER_BPB_SN:
		if (nullptr != _beamsProfile)
		{			
			std::string sn = _beamsProfile->GetSerialNumberBPB();
			std::wstring wsTemp = utf8toUtf16(sn);
			wcscpy_s(str,size, wsTemp.c_str());
			ret = true;
		}
		break;
	}

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizer::PreflightPosition()
{
	return true;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizer::SetupPosition()
{
	if (_stabilizeBeam != _stabilizeBeam_C && true == _stabilizeBeam)
	{
		_stabilizeBeam_B = true;
	}

	if (_factoryPosBeam != _factoryPosBeam_C && true == _factoryPosBeam)
	{
		_factoryPosBeam_B = true;
	}
	return true;
}


/// <summary>
/// Aligns the beam
/// </summary>
/// <returns>UINT.</returns>
UINT AlignBeamThreadProc( LPVOID pParam )
{
	bool ret = true;

	//**Get data and objects from ThorBeamStabilizer into this thread
	std::array<double, BeamsProfile::BEAM_PROFILE_NUM> pTerm = ThorBeamStabilizer::getInstance()->_pTerm;
	BeamsProfile* beamsProfile = ThorBeamStabilizer::getInstance()->_beamsProfile;
	PiezoInertiaActuators* piezoActuators = ThorBeamStabilizer::getInstance()->_piezoActuators;
	const std::array<double, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> PID_P = { pTerm[0], pTerm[0], pTerm[1], pTerm[1] };
	double maxExpTime = ThorBeamStabilizer::getInstance()->_maxExposureTime;
	double deadBand = ThorBeamStabilizer::getInstance()->_deadBand;
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> piezoOrientation = ThorBeamStabilizer::getInstance()->_piezoOrientation;
	long alignTimeOutSec = ThorBeamStabilizer::getInstance()->_alignTimeoutSec;
	//**end get data

	//retrieve the piezo positions
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> piezoPosition = {0, 0, 0, 0};
	for (long i = 0; i < piezoPosition.size(); ++i)
	{
		piezoActuators->GetPiezoPosition(i, piezoPosition[i]);
	}

	//Stop Piezo position updates to avoid any comm errors
	piezoActuators->StopStatusUpdates();

	std::clock_t start = std::clock();

	do
	{
		Sleep(110);
		std::array<double, BeamsProfile::BEAM_PROFILE_NUM> diameter = {0.0, 0.0};
		std::array<double, BeamsProfile::BEAM_PROFILE_NUM> centroidX = {0.0, 0.0};
		std::array<double, BeamsProfile::BEAM_PROFILE_NUM> centroidY = {0.0, 0.0};
		std::array<double, BeamsProfile::BEAM_PROFILE_NUM> saturation = {0.0, 0.0};
		std::array<double, BeamsProfile::BEAM_PROFILE_NUM> exposureTime = {0.0, 0.0};

		//retrieve the beam data from both beam profilers
		for (long i = 0; i < BeamsProfile::BEAM_PROFILE_NUM; ++i)
		{
			beamsProfile->GetData(i, diameter[i], centroidX[i], centroidY[i], saturation[i], exposureTime[i]);
		}

		//Ensure the exposure time is within range
		if (exposureTime[0] > maxExpTime || exposureTime[1] > maxExpTime)
		{
			ret = false;
			break;
		}

		std::array<double, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> centroid = { centroidX[0], centroidY[0], centroidX[1], centroidY[1] };
		std::array<double, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> centroidAbs = { std::abs(centroidX[0]), std::abs(centroidY[0]), std::abs(centroidX[1]), std::abs(centroidY[1]) };                      

		centroid[0] = max(-2.0*deadBand,min(2*deadBand,centroid[0]));
		double max = centroidAbs[0];
		long maxIndex = 0;

		//second detector must be adjusted first
		if(centroidAbs[2] > deadBand)
		{
			maxIndex = 2;
			max = centroidAbs[2];

			centroid[2] = max(-2.0*deadBand,min(2*deadBand,centroid[2]));
		}
		else if(centroidAbs[3] > deadBand)
		{
			maxIndex = 3;
			max = centroidAbs[3];
			centroid[3] = max(-2.0*deadBand,min(2*deadBand,centroid[3]));
		}
		else if (centroidAbs[1] > deadBand)
		{
			maxIndex = 1;
			max = centroidAbs[1];
			centroid[1] = max(-2.0*deadBand,min(2*deadBand,centroid[1]));
		}

		//Determines if the misalignment is greater than the deadband and checks absolute position of the actuators

		if(false == ThorBeamStabilizer::getInstance()->CheckPiezoLimits(piezoPosition))
		{
			MessageBox(NULL, L"A piezo mirror is outside of the step limit for the device. You will need to factory reset the device and try again. If the issue continues please contact Thorlabs support for realignment",L"Beam Stabilizer",MB_OK); 
			break;
		}
		else if (centroidAbs[maxIndex] > deadBand)
		{
			//Computes PID terms for the axis in question
			double pTerm = centroid[maxIndex] * PID_P[maxIndex] * piezoOrientation[maxIndex];
			double offsetPosition;
			if (abs(pTerm) <= APT_MAX_STEP_MOVE)
			{
				offsetPosition = -pTerm;
			}
			else
			{
				offsetPosition = (pTerm < 0)? (-APT_MAX_STEP_MOVE*-1): -APT_MAX_STEP_MOVE;
			}
			long newPiezoPosition = (long)((double)piezoPosition[maxIndex] + offsetPosition);

			std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> newPiezoPositions = {piezoPosition[0], piezoPosition[1], piezoPosition[2], piezoPosition[3]};

			newPiezoPositions[maxIndex] += static_cast<long>(offsetPosition);

			if(false == ThorBeamStabilizer::getInstance()->CheckPiezoLimits(newPiezoPositions))
			{
				wchar_t wtext[300];
				char buff[300];
				sprintf_s(buff, "A piezo mirror is trying to move outside of the step limit for the device (P1:%ld P2:%ld P3:%ld P4:%ld). You will need to factory reset the device and try again. If the issue continues please contact Thorlabs support for realignment.", newPiezoPositions[0], newPiezoPositions[1], newPiezoPositions[2], newPiezoPositions[3]);
				size_t retVal = 0;
				mbstowcs_s(&retVal, wtext, strlen(buff)+1, buff, _TRUNCATE);
				MessageBox(NULL, wtext,L"Beam Stabilizer",MB_OK); 
				break;
			}

			piezoActuators->JogMove(maxIndex, (long)offsetPosition);
			long sleep = min(max(abs((long)offsetPosition)*2, 500), 5000);
			Sleep(sleep);

			piezoPosition[maxIndex] = newPiezoPosition;
		}
		//If all axis are within the deadband, alignment is complete
		else if (centroidAbs[0] < deadBand && centroidAbs[1] < deadBand && centroidAbs[2] < deadBand && centroidAbs[3] < deadBand)
		{		
			break;
		}

		double duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

		//if the system is not aligned within the TIMOUT time, break from the loop
		//don't allow an infinite loop
		if (duration > alignTimeOutSec)
		{
			break;
		}

	} while(true);

	try
	{
		//store the piezo locations after every routine in case a of an unexpected software
		//shutdown
		std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());

		//save the current piezo positions
		pSetup->SetPiezoActuatorPositions(piezoPosition[0], piezoPosition[1], piezoPosition[2], piezoPosition[3]);
		pSetup->SaveConfigFile();
	}
	catch(...)
	{
		return false;
	}	

	//restart position updates for the piezos
	piezoActuators->StartStatusUpdates();

	CloseHandle(ThorBeamStabilizer::_hThread);
	ThorBeamStabilizer::_hThread = NULL;
	ThorBeamStabilizer::getInstance()->_stabilizingThreadActive = false;

	return 0;
}

/// <summary>
/// Move the piezos to their original position
/// </summary>
/// <returns>UINT.</returns>
UINT MoveToFactoryThreadProc( LPVOID pParam )
{
	bool ret = true;

	//**Get data and objects from ThorBeamStabilizer into this thread
	PiezoInertiaActuators* piezoActuators = ThorBeamStabilizer::getInstance()->_piezoActuators;
	long movingDistance = 0, sleep = 0;
	//**end get data

	//retrieve the piezo positions
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> piezoPosition = {0, 0, 0, 0};
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> factoryPosition = {0, 0, 0, 0};

	for (long i = 0; i < piezoPosition.size(); ++i)
	{
		piezoActuators->GetPiezoPosition(i, piezoPosition[i]);
	}

	try
	{
		//retrieve the piezo locations From
		//shutdown
		std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());

		long piezoStepLimit = 0;
		//save the current piezo positions
		pSetup->GetFactoryPiezoActuatorPositions(factoryPosition[0], factoryPosition[1], factoryPosition[2], factoryPosition[3],piezoStepLimit);
	}
	catch(...)
	{
		return false;
	}	
	//Stop Piezo position updates to avoid any comm errors
	piezoActuators->StopStatusUpdates();

	if(false == ThorBeamStabilizer::getInstance()->CheckPiezoLimits(piezoPosition))
	{
		MessageBox(NULL, L"A piezo mirror is outside of the step limit for the device. You will need to factory reset the device and try again. If the issue continues please contact Thorlabs support for realignment",L"Beam Stabilizer",MB_OK); 
		return false;
	}

	for (long i = 0; i < piezoPosition.size(); ++i)
	{
		movingDistance = (long) factoryPosition[i] - piezoPosition[i];
		while (0 != movingDistance)
		{
			if(APT_MAX_STEP_MOVE < abs(movingDistance))
			{
				movingDistance = (movingDistance < 0)? -APT_MAX_STEP_MOVE: APT_MAX_STEP_MOVE;
			}
			piezoActuators->JogMove(i, movingDistance); 
			sleep = min(max(abs(movingDistance) * 2, 500), 5000);
			Sleep(sleep);
			piezoPosition[i] += movingDistance;
			piezoActuators->SetPiezoPosition(i, piezoPosition[i]);
			movingDistance = (long) factoryPosition[i] - piezoPosition[i];
		}
	}
	try
	{
		std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());
		//save the current piezo positions
		pSetup->SetPiezoActuatorPositions(piezoPosition[0], piezoPosition[1], piezoPosition[2], piezoPosition[3]);
		pSetup->SaveConfigFile();
	}
	catch(...)
	{
		return false;
	}	

	//restart position updates for the piezos
	piezoActuators->StartStatusUpdates();

	CloseHandle(ThorBeamStabilizer::_hThread);
	ThorBeamStabilizer::_hThread = NULL;
	ThorBeamStabilizer::getInstance()->_stabilizingThreadActive = false;

	return 0;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizer::StartPosition()
{
	long ret = true;

	if (_stabilizeBeam_B)
	{
		if (false == _stabilizingThreadActive)
		{
			_stabilizingThreadActive = true;
			DWORD dwThreadId;
			_hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) AlignBeamThreadProc, this, 0, &dwThreadId );
			if (_hThread == NULL)
			{
				ret = FALSE;
			}
		}

		_stabilizeBeam = _stabilizeBeam_C = _stabilizeBeam_B = false;
	}

	if(_factoryPosBeam_B)
	{
		PiezoInertiaActuators* piezoActuators = ThorBeamStabilizer::getInstance()->_piezoActuators;

		//Stop Piezo position updates to avoid any comm errors
		piezoActuators->StopStatusUpdates();

		long factoryPiezoPosition[4] = {0,0,0,0};
		long piezoStepLimit = 0;
		try
		{
			std::unique_ptr<ThorBeamStabilizerXML> pSetup( new ThorBeamStabilizerXML());

			//get the factory piezo positions
			pSetup->GetFactoryPiezoActuatorPositions(factoryPiezoPosition[0], factoryPiezoPosition[1], factoryPiezoPosition[2], factoryPiezoPosition[3], piezoStepLimit);
		}
		catch(...)
		{
			ret = false;
		}	

		DWORD dwThreadId;
		_hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) MoveToFactoryThreadProc, this, 0, &dwThreadId );
		if (_hThread == NULL)
		{
			ret = FALSE;
		}

		//restart position updates for the piezos
		piezoActuators->StartStatusUpdates();

		_factoryPosBeam = _factoryPosBeam_C = _factoryPosBeam_B = false;

		ret = true;
	}	

	return ret;
}

/// <summary>
/// Statuses the position.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::StatusPosition(long &status)
{
	long ret = true;
	status = IDevice::STATUS_READY;
	return ret;
}

/// <summary>
/// Reads the position.
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::ReadPosition(DeviceType deviceType, double &pos)
{
	return true;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorBeamStabilizer::PostflightPosition()
{
	return true;
}


/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorBeamStabilizer::GetLastErrorMsg(wchar_t *msg, long size)
{	
	wcsncpy_s(msg,MSG_SIZE,_errMsg,MSG_SIZE);
	return true;
}

/// <summary>
/// Checks if the each one of the piezo is within its step limit
/// </summary>
/// <param name="piezoPos">The correct piezo position in steps.</param>
bool ThorBeamStabilizer::CheckPiezoLimits(std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> piezoPos)
{
	for (long i = 0; i < piezoPos.size(); ++i)
	{
		if (abs(piezoPos[i]) >= _piezoActuators->GetPiezoStepLimit())
		{
			return false;
		}
	}

	return true;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorBeamStabilizer::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}
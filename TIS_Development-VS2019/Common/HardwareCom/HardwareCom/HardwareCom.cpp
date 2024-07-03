#include "stdafx.h"
#include "HardwareCom.h"
#include "HardwareComFunctions.h"

//TODO: Complete camera com implematation

const long MSG_SIZE = 256;

/// <summary>
/// The message
/// </summary>
wchar_t message[MSG_SIZE];

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
auto_ptr<CommandDll> shwDll(new CommandDll(L".\\Modules_Native\\SelectHardware.dll"));
bool HardwareCom::_instanceFlag = false;
auto_ptr<HardwareCom> HardwareCom::_single(NULL);
HANDLE hStopStatusCheck = CreateEvent(0, TRUE, FALSE, 0);	//manual reset
const long MAX_WAIT_TIME = 60000;	//[msec]

struct StatusDeviceProcParams
{
	IDevice *pDevice;
	HANDLE *pEventHandle;
};

/// <summary>
/// Status thread
/// </summary>
/// <param name="pParam">pointer to the parameter used inside the thread.</param>
UINT StatusDeviceThreadProc( LPVOID pParam )
{
	long status = IDevice::STATUS_BUSY;
	StatusDeviceProcParams * pStatus = (StatusDeviceProcParams*)pParam;

	while((status == IDevice::STATUS_BUSY) && (WAIT_OBJECT_0 != WaitForSingleObject(hStopStatusCheck, 0)))
	{
		if(NULL == pStatus->pDevice)
		{
			break;
		}
		if(FALSE == pStatus->pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent(*pStatus->pEventHandle);

	return 0;
}

/// <summary>
/// Prevents a default instance of the <see cref="HardwareCom"/> class from being created.
/// </summary>
HardwareCom::HardwareCom()
{

}

/// <summary>
/// Finalizes an instance of the <see cref="HardwareCom"/> class.
/// </summary>
HardwareCom::~HardwareCom()
{
	_instanceFlag = false;
	message[0] = 0;
}

/// <summary>
/// Gets a singleton instance of HardwareCom.
/// </summary>
/// <returns>HardwareCom*.</returns>
HardwareCom* HardwareCom::getInstance()
{
	if(! _instanceFlag)
	{
		StringCbPrintfW(message,MSG_SIZE,L"Creating HardwareCom Singleton");
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);

		_single.reset(new HardwareCom());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}

/// <summary>
/// Execute the device task
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="pEvent">Pointer to the command handle.</param>
/// <param name="maxWaitTime">Max wait length in msec.</param>
/// <param name="exeOrWait">execute only or wait for status.</param>
/// <returns>long.</returns>
long HardwareCom::ExecuteDevice(IDevice *pDevice, long maxWaitTime, long exeOrWait)
{
	long ret = TRUE;
	HANDLE hEvent = NULL;

	switch ((IDevice::DeviceSetParamType)exeOrWait)
	{
	case IDevice::DeviceSetParamType::EXECUTION_NO_WAIT:	//execute without waiting for status
	case IDevice::DeviceSetParamType::EXECUTION_WAIT:		//execute and wait for status
		pDevice->PreflightPosition();
		pDevice->SetupPosition();
		ret = pDevice->StartPosition();

		if((long)IDevice::DeviceSetParamType::EXECUTION_WAIT == exeOrWait)
		{
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			ret = StatusHandlerDevice(pDevice, &hEvent, maxWaitTime);
		}

		pDevice->PostflightPosition();
		break;
	default:
		break;
	}

	if(NULL != hEvent)
	{
		CloseHandle(hEvent);
		hEvent = NULL;
	}
	return ret;
}

/// <summary>
/// Gets a parameter availablity corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterAvailable(IDevice *pDevice,long paramID)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pDevice is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pDevice->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		ret = paramAvailable;
	}
	return ret;
}

/// <summary>
/// Gets a parameter readonly flag corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterReadOnly(IDevice *pDevice,long paramID)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pDevice is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pDevice->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		ret = paramReadOnly;
	}
	return ret;
}


/// <summary>
/// Gets a parameter corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The parameter value.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterValue(IDevice* pDevice,long paramID, double &val)
{
	long ret = FALSE;
	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		double dVal=0;
		ret = pDevice->GetParam(paramID,dVal);
		val = dVal;
	}
	return ret;
}

/// <summary>
/// Gets a parameter corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="buf">The parameter buffer.</param>
/// <param name="len">The parameter buffer length.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterBuffer(IDevice* pDevice,long paramID, char* buf, long len)
{
	long ret = FALSE;
	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		ret = pDevice->GetParamBuffer(paramID, buf, len);
	}
	return ret;
}

/// <summary>
/// Gets a string corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The string buffer.</param>
/// <param name="size">The string length.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterString(IDevice *pDevice,long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		ret = pDevice->GetParamString(paramID, str, size);
	}
	return ret;
}

/// <summary>
/// Gets a parameter's value range corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="valMin">The min value the parameter can be set to.</param>
/// <param name="valMax">The max value the parameter can be set to.</param>
/// <param name="valMax">The default value of the parameter.</param>
/// <returns>long.</returns>
long HardwareCom::GetDeviceParameterValueRange(IDevice * pDevice,long paramID, double &valMin, double &valMax, double &valDefault)
{
	long ret = FALSE;

	if(NULL == pDevice)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pDevice->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(TRUE == paramAvailable)
		{
			valMin = paramMin;
			valMax = paramMax;
			valDefault = paramDefault;
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}
	}

	return ret;
}

/// <summary>
/// Sets a parameter corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The parameter value.</param>
/// <param name="exeOrWait">Flag for execution or waiting until the command is completed.</param>
/// <param name="hEvent">Command handle.</param>
/// <param name="waitTime">Max wait length in msec in case bwait is set to true.</param>
/// <returns>long.</returns>
long HardwareCom::SetDeviceParameterValue(IDevice *pDevice,long paramID, double val, long exeOrWait, long waitTime)
{
	long ret = FALSE;
	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		ret = pDevice->SetParam(paramID, val);

		ret = ExecuteDevice(pDevice, waitTime, exeOrWait);
	}
	return ret;
}

/// <summary>
/// Sets a parameter corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="buf">The parameter buffer.</param>
/// <param name="len">The length of buffer.</param>
/// <param name="bWait">Flag for execution or waiting until the command is completed.</param>
/// <param name="hEvent">Command handle.</param>
/// <param name="waitTime">Max wait length in msec in case bwait is set to true.</param>
/// <returns>long.</returns>
long HardwareCom::SetDeviceParameterBuffer(IDevice *pDevice,long paramID, char* buf, long len, long exeOrWait, long waitTime)
{
	long ret = FALSE;

	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		ret = pDevice->SetParamBuffer(paramID, buf, len);

		ret = ExecuteDevice(pDevice, waitTime, exeOrWait);
	}
	return ret;
}

/// <summary>
/// Sets a string corresponding to the paramId for the entered device.
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="bWait">Flag for execution or waiting until the command is completed.</param>
/// <param name="hEvent">Command handle.</param>
/// <param name="waitTime">Max wait length in msec in case bwait is set to true.</param>
/// <returns>long.</returns>
long HardwareCom::SetDeviceParameterString(IDevice *pDevice, long paramID, wchar_t* str, long exeOrWait, long waitTime)
{
	long ret = FALSE;

	if((NULL != pDevice) && GetDeviceParameterAvailable(pDevice, paramID))
	{
		ret = pDevice->SetParamString(paramID, str);

		ret = ExecuteDevice(pDevice, waitTime, exeOrWait);
	}
	return ret;
}

/// <summary>
/// Handles the status of the device
/// </summary>
/// <param name="pDevice">A pointer to the device.</param>
/// <param name="pEvent">Pointer to the command handle.</param>
/// <param name="maxWaitTime">Max wait length in msec.</param>
/// <returns>long.</returns>
long HardwareCom::StatusHandlerDevice(IDevice * pDevice, HANDLE *pEvent, long maxWaitTime)
{
	long ret = TRUE;

	if(NULL == pEvent)
	{
		*pEvent = CreateEvent(0, FALSE, FALSE, 0);
	}

	DWORD dwThread;

	StatusDeviceProcParams statusParams;

	statusParams.pDevice = pDevice;
	statusParams.pEventHandle = pEvent;
	ResetEvent(hStopStatusCheck);

	HANDLE hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)StatusDeviceThreadProc, &statusParams, 0, &dwThread );

	while (WAIT_OBJECT_0 != WaitForSingleObject(*pEvent, maxWaitTime))
	{
		//inform status thread to stop:
		SetEvent(hStopStatusCheck);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"StatusHandlerDevice  StatusThreadProc  failed");
		ret = FALSE;
	}

	CloseHandle(hThread);
	hThread = NULL;
	CloseHandle(*pEvent);
	*pEvent = NULL;
	return ret;
}

/// <summary>
/// Gets the device corresponding to the deviceSelection
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <returns>IDevice*.</returns>
IDevice* HardwareCom::GetDevice(long deviceSelection)
{
	SelectHardwareCustomParams shwcp;

	shwDll->SetupCommand();

	shwDll->GetCustomParamsBinary((LPSTR)&shwcp);

	//order of the switch statement items matches the order of the enum definition
	//and the order of the select hardware data structure
	switch (deviceSelection)
	{
	case SelectedHardware::SELECTED_XYSTAGE:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeXYStage);
	case SelectedHardware::SELECTED_ZSTAGE:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeZStage);
	case SelectedHardware::SELECTED_ZSTAGE2:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeZStage2);
	case SelectedHardware::SELECTED_BEAMSTABILIZER:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeBeamStabilizer);
	case SelectedHardware::SELECTED_EMISSION:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeEmission);
	case SelectedHardware::SELECTED_DICHROIC:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeDichroic);
	case SelectedHardware::SELECTED_SHUTTER1:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeShutter1);
	case SelectedHardware::SELECTED_TURRET:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeTurret);
	case SelectedHardware::SELECTED_BFLAMP:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeBFLamp);
	case SelectedHardware::SELECTED_AUTOFOCUS:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeAutoFocus);
	case SelectedHardware::SELECTED_CONTROLUNIT:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeControlUnit);
	case SelectedHardware::SELECTED_PMT1:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePMT1);
	case SelectedHardware::SELECTED_PMT2:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePMT2);
	case SelectedHardware::SELECTED_PMT3:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePMT3);
	case SelectedHardware::SELECTED_PMT4:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePMT4);
	case SelectedHardware::SELECTED_POWERREGULATOR:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePowerRegulator);
	case SelectedHardware::SELECTED_POWERREGULATOR2:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePowerRegulator2);
	case SelectedHardware::SELECTED_BEAMEXPANDER:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeBeamExpander);
	case SelectedHardware::SELECTED_PINHOLEWHEEL:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePinholeWheel);
	case SelectedHardware::SELECTED_LASER1:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeLaser1);
	case SelectedHardware::SELECTED_LASER2:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeLaser2);
	case SelectedHardware::SELECTED_LASER3:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeLaser3);
	case SelectedHardware::SELECTED_LASER4:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeLaser4);
	case SelectedHardware::SELECTED_SLM:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeSLM);
	case SelectedHardware::SELECTED_RSTAGE:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeRStage);		
	case SelectedHardware::SELECTED_PMTSWITCH:
		return DeviceManager::getInstance()->GetDevice(shwcp.activePMTSwitch);
	case SelectedHardware::SELECTED_EPHYS:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeEphys);
	case SelectedHardware::SELECTED_LIGHTPATH:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeLightPath);	
	case SelectedHardware::SELECTED_SPECTRUMFILTER:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeSpectrumFilter);
	case SelectedHardware::SELECTED_EPITURRET:
		return DeviceManager::getInstance()->GetDevice(shwcp.activeDichroic);
	default:
		return NULL;
	}
}

/// <summary>
/// Gets the camera corresponding to the cameraSelection
/// </summary>
/// <param name="cameraSelection">Camera selection enum value.</param>
/// <returns>ICamera*.</returns>
ICamera* HardwareCom::GetCamera(long cameraSelection)
{
	SelectHardwareCustomParams shwcp;

	shwDll->SetupCommand();

	shwDll->GetCustomParamsBinary((LPSTR)&shwcp);

	switch (cameraSelection)
	{
	case SelectedHardware::SELECTED_BLEACHINGSCANNER:
		return CameraManager::getInstance()->GetCamera(shwcp.activeBleachingScanner);
	case SelectedHardware::SELECTED_CAMERA1:
		return CameraManager::getInstance()->GetCamera(shwcp.activeCamera1);
	default:
		return NULL;
	}
}

long HardwareCom::GetDeviceSelectedIndex(long deviceSelection)
{
	SelectHardwareCustomParams shwcp;

	shwDll->SetupCommand();

	shwDll->GetCustomParamsBinary((LPSTR)&shwcp);

	switch (deviceSelection)
	{
	case SelectedHardware::SELECTED_XYSTAGE:
		return shwcp.activeXYStage;
	case SelectedHardware::SELECTED_ZSTAGE:
		return shwcp.activeZStage;
	case SelectedHardware::SELECTED_ZSTAGE2:
		return shwcp.activeZStage2;
	case SelectedHardware::SELECTED_BEAMSTABILIZER:
		return shwcp.activeBeamStabilizer;
	case SelectedHardware::SELECTED_EMISSION:
		return shwcp.activeEmission;
	case SelectedHardware::SELECTED_DICHROIC:
		return shwcp.activeDichroic;
	case SelectedHardware::SELECTED_SHUTTER1:
		return shwcp.activeShutter1;
	case SelectedHardware::SELECTED_TURRET:
		return shwcp.activeTurret;
	case SelectedHardware::SELECTED_BFLAMP:
		return shwcp.activeBFLamp;
	case SelectedHardware::SELECTED_AUTOFOCUS:
		return shwcp.activeAutoFocus;
	case SelectedHardware::SELECTED_CONTROLUNIT:
		return shwcp.activeControlUnit;
	case SelectedHardware::SELECTED_PMT1:
		return shwcp.activePMT1;
	case SelectedHardware::SELECTED_PMT2:
		return shwcp.activePMT2;
	case SelectedHardware::SELECTED_PMT3:
		return shwcp.activePMT3;
	case SelectedHardware::SELECTED_PMT4:
		return shwcp.activePMT4;
	case SelectedHardware::SELECTED_POWERREGULATOR:
		return shwcp.activePowerRegulator;
	case SelectedHardware::SELECTED_BEAMEXPANDER:
		return shwcp.activeBeamExpander;
	case SelectedHardware::SELECTED_PINHOLEWHEEL:
		return shwcp.activePinholeWheel;
	case SelectedHardware::SELECTED_LASER1:
		return shwcp.activeLaser1;
	case SelectedHardware::SELECTED_LASER2:
		return shwcp.activeLaser2;
	case SelectedHardware::SELECTED_LASER3:
		return shwcp.activeLaser3;
	case SelectedHardware::SELECTED_LASER4:
		return shwcp.activeLaser4;
	case SelectedHardware::SELECTED_SLM:
		return shwcp.activeSLM;
	case SelectedHardware::SELECTED_RSTAGE:
		return shwcp.activeRStage;		
	case SelectedHardware::SELECTED_PMTSWITCH:
		return shwcp.activePMTSwitch;
	case SelectedHardware::SELECTED_EPHYS:
		return shwcp.activeEphys;
	case SelectedHardware::SELECTED_LIGHTPATH:
		return shwcp.activeLightPath;	
	case SelectedHardware::SELECTED_SPECTRUMFILTER:
		return shwcp.activeSpectrumFilter;
	case SelectedHardware::SELECTED_POWERREGULATOR2:
		return shwcp.activePowerRegulator2;
	case SelectedHardware::SELECTED_EPITURRET:
		return shwcp.activeDichroic;
	default:
		return NULL;
	}
}

long HardwareCom::GetCameraSelectedIndex(long cameraSelection)
{
	SelectHardwareCustomParams shwcp;

	shwDll->SetupCommand();

	shwDll->GetCustomParamsBinary((LPSTR)&shwcp);

	switch (cameraSelection)
	{
	case SelectedHardware::SELECTED_BLEACHINGSCANNER:
		return shwcp.activeBleachingScanner;
	case SelectedHardware::SELECTED_CAMERA1:
		return shwcp.activeCamera1;
	default:
		return NULL;
	}
}

/// <summary>
/// Check availablity of the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceParamAvailable(long deviceSelection, long paramId)
{
	IDevice* pDevice = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->GetDeviceParameterAvailable(pDevice, paramId);
}

/// <summary>
/// Check availablity of the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceParamReadOnly(long deviceSelection, long paramId)
{
	IDevice* pDevice = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->GetDeviceParameterReadOnly(pDevice, paramId);
}

/// <summary>
/// Gets the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <returns>ICamera*.</returns>
DllExportFunc GetDeviceParamDouble(long deviceSelection, long paramId, double &param)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->GetDeviceParameterValue(device,paramId,param);
}

/// <summary>
/// Sets the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <param name="bWait">Flag for waiting until the command is completed.</param>
/// <returns>ICamera*.</returns>
DllExportFunc SetDeviceParamDouble(long deviceSelection, long paramId, double param, long exeOrWait)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->SetDeviceParameterValue(device,paramId,param,exeOrWait,MAX_WAIT_TIME);
}

/// <summary>
/// Gets the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <returns>ICamera*.</returns>
DllExportFunc GetDeviceParamLong(long deviceSelection, long paramId, long &param)
{	
	long ret = 0;
	double paramVal = 0;

	ret = GetDeviceParamDouble(deviceSelection,paramId, paramVal);
	param = static_cast<long>(paramVal);

	return ret;
}

/// <summary>
/// Sets the device parameter corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <param name="bWait">Flag for waiting until the command is completed.</param>
/// <returns>ICamera*.</returns>
DllExportFunc SetDeviceParamLong(long deviceSelection, long paramId, long param, long bWait)
{	
	long ret = FALSE;

	return SetDeviceParamDouble(deviceSelection, paramId, static_cast<long>(param), bWait);
}


/// <summary>
/// Gets the device parameter range for the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <param name="bWait">Flag for waiting until the command is completed.</param>
/// <returns>ICamera*.</returns>
DllExportFunc GetDeviceParamRangeDouble(long deviceSelection, long paramId,  double &valMin, double &valMax, double &valDefault)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);

	if(NULL == device)
	{
		return FALSE;
	}

	return HardwareCom::getInstance()->GetDeviceParameterValueRange(device, paramId, valMin, valMax, valDefault);
}

/// <summary>
/// Gets the device parameter range for the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="param">The parameter value.</param>
/// <param name="bWait">Flag for waiting until the command is completed.</param>
/// <returns>ICamera*.</returns>
DllExportFunc GetDeviceParamRangeLong(long deviceSelection, long paramId,  long &valMin, long &valMax, long &valDefault)
{	
	long ret = FALSE;
	double dMin=0;
	double dMax=0;
	double dDefault=0;

	ret = GetDeviceParamRangeDouble(deviceSelection, paramId,  dMin, dMax, dDefault);

	valMin = static_cast<long>(dMin);
	valMax = static_cast<long>(dMax);
	valDefault = static_cast<long>(dDefault);

	return ret;
}

/// <summary>
/// Gets the camera parameter available flag.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramAvailable">The parameter available flag.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamAvailable(long cameraSelection, long paramID)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}

	long paramType;
	long paramReadOnly;
	long paramAvailable;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		ret = paramAvailable;
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Gets the camera parameter available flag.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramAvailable">The parameter available flag.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamReadOnly(long cameraSelection, long paramID)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}

	long paramType;
	long paramReadOnly;
	long paramAvailable;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		ret = paramReadOnly;
	}
	else
	{
		ret = FALSE;
	}
	return ret;
}

/// <summary>
/// Gets the camera parameter value double.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The value.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamDouble(long cameraSelection, long paramID, double &val)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}

	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			pCamera->GetParam(paramID,val);

			if((val >= paramMin)||(val <= paramMax))
			{
				ret = TRUE;
			}
			else
			{
				StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: %d value %f out of range %f to %f (%hs)", __FUNCTION__, __LINE__,val,paramID,paramMin,paramMax,__FILE__);
				logDll->TLTraceEvent(ERROR_EVENT,1,message);
			}
		}
		else
		{
			StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: %d not available. (%hs)", __FUNCTION__, __LINE__,paramID,__FILE__);
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		}
	}
	return ret;
}


/// <summary>
/// Gets the camera parameter value range double.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="valMin">The value minimum.</param>
/// <param name="valMax">The value maximum.</param>
/// <param name="valDefault">The value default.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamRangeDouble(long cameraSelection, long paramID, double &valMin, double &valMax, double &valDefault)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		valMin = paramMin;
		valMax = paramMax;
		valDefault = paramDefault;
		ret = TRUE;
	}
	return ret;
}


/// <summary>
/// Gets the camera parameter value range long.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="valMin">The value minimum.</param>
/// <param name="valMax">The value maximum.</param>
/// <param name="valDefault">The value default.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamRangeLong(long cameraSelection, long paramID, long &valMin, long &valMax, long &valDefault)
{
	long ret = FALSE;
	double paramMin;
	double paramMax;
	double paramDefault;

	ret = GetCameraParamRangeDouble(cameraSelection, paramID, paramMin, paramMax, paramDefault);

	valMin = static_cast<long>(paramMin);
	valMax = static_cast<long>(paramMax);
	valDefault = static_cast<long>(paramDefault);

	return ret;
}


/// <summary>
/// Sets the camera parameter value double.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The value.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc SetCameraParamDouble(long cameraSelection, long paramID, double val)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault))
	{
		if(paramAvailable)
		{
			//range check before set parameter to camera
			double setVal = max(paramMin, min(val, paramMax));
			ret = pCamera->SetParam(paramID,setVal);
		}
		else
		{
			StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: %d paramAvailable is false. (%hs)", __FUNCTION__, __LINE__, paramID,__FILE__);
			logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
			return ret;
		}
	}
	else
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera->GetParamInfo %d failed. (%hs)", __FUNCTION__, __LINE__, paramID,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	return ret;
}

/// <summary>
/// Sets the camera parameter value long.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The value.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc SetCameraParamLong(long cameraSelection, long paramID, long val)
{
	return SetCameraParamDouble(cameraSelection, paramID, static_cast<double>(val));
}

/// <summary>
/// Sets the camera parameter buffer.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">The p buffer.</param>
/// <param name="size">The size.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc SetCameraParamBuffer(long cameraSelection, long paramID, char * pBuffer, long size)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			ret = pCamera->SetParamBuffer(paramID,pBuffer,size);
		}
	}
	return ret;
}

/// <summary>
/// Gets the camera parameter buffer.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">The p buffer.</param>
/// <param name="size">The size.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamBuffer(long cameraSelection, long paramID, char * pBuffer, long size)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			ret = pCamera->GetParamBuffer(paramID,pBuffer,size);
		}
	}
	return ret;
}

/// <summary>
/// Sets the camera parameter buffer.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The string.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc SetCameraParamString(long cameraSelection, long paramID, wchar_t * str)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			ret = pCamera->SetParamString(paramID, str);
		}
	}
	return ret;
}

/// <summary>
/// Gets the camera parameter string.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The string.</param>
/// <param name="size">The size.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamString(long cameraSelection, long paramID, wchar_t * str, long size)
{
	ICamera* pCamera = HardwareCom::getInstance()->GetCamera(cameraSelection);
	long ret = FALSE;

	if(NULL == pCamera)
	{
		StringCbPrintfW(message,MSG_SIZE,L"%hs@%u: pCamera is NULL. (%hs)", __FUNCTION__, __LINE__,__FILE__);
		logDll->TLTraceEvent(INFORMATION_EVENT,1,message);
		return ret;
	}
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;
	long pMin = -1, pMax=-1;

	if(pCamera->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		if(paramAvailable)
		{
			ret = pCamera->GetParamString(paramID,str,size);
		}
	}
	return ret;
}

/// <summary>
/// Gets the camera parameter value long.
/// </summary>
/// <param name="cameraSelection">The camera selection.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="val">The value.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraParamLong(long cameraSelection, long paramID, long &val)
{
	long ret = FALSE;

	double dVal = 0;

	ret = GetCameraParamDouble(cameraSelection, paramID, dVal);

	val = static_cast<long>(dVal);

	return ret;
}


DllExportDevFunc GetDevice(long selectedDevice)
{
	IDevice * pDevice = HardwareCom::getInstance()->GetDevice(selectedDevice);

	if(NULL == pDevice)
	{
		return FALSE;
	}

	return pDevice;
}

/// <summary>
/// Gets the camera.
/// </summary>
/// <param name="selectedCamera">The selected camera.</param>
/// <param name="pCamera">The p camera.</param>
/// <returns>DllExportFunc.</returns>
DllExportCameraFunc GetCamera(long selectedCamera)
{
	ICamera * pCamera = HardwareCom::getInstance()->GetCamera(selectedCamera);

	if(NULL == pCamera)
	{
		return FALSE;
	}

	return pCamera;
}

/// <summary>
/// Gets the index of the device selected.
/// </summary>
/// <param name="selectedDevice">The selected device.</param>
/// <param name="selectedIndex">Index of the selected.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetDeviceSelectedIndex(long selectedDevice, long &selectedIndex)
{	
	selectedIndex = HardwareCom::getInstance()->GetDeviceSelectedIndex(selectedDevice);

	return TRUE;
}

/// <summary>
/// Gets the index of the camera selected.
/// </summary>
/// <param name="selectedCamera">The selected camera.</param>
/// <param name="selectedIndex">Index of the selected.</param>
/// <returns>DllExportFunc.</returns>
DllExportFunc GetCameraSelectedIndex(long selectedCamera, long &selectedIndex)
{	
	selectedIndex = HardwareCom::getInstance()->GetCameraSelectedIndex(selectedCamera);

	return TRUE;
}

/// <summary>
/// Sets the device parameter buffer corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="buf">The parameter buffer.</param>
/// <param name="len">The length of parameter buffer.</param>
/// <param name="bWait">Flag for waiting until the command is completed.</param>
/// <returns>ICamera*.</returns>
DllExportFunc SetDeviceParamBuffer(long deviceSelection, long paramId, char* buf, long len, long exeOrWait)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->SetDeviceParameterBuffer(device, paramId, buf, len, exeOrWait, MAX_WAIT_TIME);
}

/// <summary>
/// Gets the device parameter buffer corresponding to the device selection and parameter ID
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramId">The parameter identifier.</param>
/// <param name="buf">The parameter buffer.</param>
/// <param name="len">The length of parameter buffer.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceParamBuffer(long deviceSelection, long paramId, char* buf, long len)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->GetDeviceParameterBuffer(device, paramId, buf, len);
}

/// <summary>
/// Set the device string corresponding to the device selection
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The wide string to set.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc SetDeviceParamString(long deviceSelection, long paramID, wchar_t * str, long exeOrWait)
{
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->SetDeviceParameterString(device, paramID, str, exeOrWait, MAX_WAIT_TIME);
}

/// <summary>
/// Get the device string corresponding to the device selection
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The wide string to get.</param>
/// <param name="size">The size of the wide string.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceParamString(long deviceSelection, long paramID, wchar_t * str, long size)
{
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	return HardwareCom::getInstance()->GetDeviceParameterString(device, paramID, str, size);
}

/// <summary>
/// Gets the device status corresponding to the device selection
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="param">The status.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceStatus(long deviceSelection, long &status)
{	
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);

	if(NULL == device)
	{
		return FALSE;
	}

	return device->StatusPosition(status);
}

/// <summary>
/// Gets the device error message corresponding to the device selection
/// </summary>
/// <param name="deviceSelection">Device selection enum value.</param>
/// <param name="errorMessage">The error message.</param>
/// <returns>TRUE/FALSE</returns>
DllExportFunc GetDeviceErrorMessage(long deviceSelection, wchar_t* errorMessage, long size)
{
	IDevice* device = HardwareCom::getInstance()->GetDevice(deviceSelection);
	if(NULL == device)
	{
		return FALSE;
	}
	return device->GetLastErrorMsg(errorMessage, size);
}

DllExportFunc PreflightCamera(long cameraSelection)
{
	ICamera* camera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	if (NULL == camera)
	{
		return FALSE;
	}

	return camera->PreflightAcquisition(NULL);
}

DllExportFunc SetupCamera(long cameraSelection)
{
	ICamera* camera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	if (NULL == camera)
	{
		return FALSE;
	}

	return camera->SetupAcquisition(NULL);
}

DllExportFunc PostflightCamera(long cameraSelection)
{
	ICamera* camera = HardwareCom::getInstance()->GetCamera(cameraSelection);

	if (NULL == camera)
	{
		return FALSE;
	}

	return camera->PostflightAcquisition(NULL);
}

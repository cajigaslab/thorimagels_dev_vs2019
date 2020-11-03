// SelectHardware.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SelectHardware.h"
#include "HardwareSetupXML.h"
#include <math.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define MAX_CAM_NAME_LEN		64

typedef void (_cdecl *myPrototype)(int * val);

void (*myFunctionPointer)(int * val) = NULL;

DWORD dwThreadId = NULL;
HANDLE hThread = NULL;
HANDLE hStatusEvent;
BOOL stopRun = FALSE;
BOOL activeRun = FALSE;
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
wchar_t message[256];

SelectHardware::SelectHardware()
{
	//private constructor

	params.version = 1.0;
	params.showDialog = FALSE;

	memset(&paramsCustom,0,sizeof paramsCustom);	

}

GUID SelectHardware::guid = { 0xbed71593, 0xce3d, 0x4909, { 0x9d, 0xe5, 0x63, 0xce, 0x3a, 0x5, 0x28, 0x9b } };

bool SelectHardware::instanceFlag = true;

bool SelectHardware::_setupFlag = false;

auto_ptr<SelectHardware> SelectHardware::_single(new SelectHardware());

SelectHardware* SelectHardware::getInstance()
{
	if(! instanceFlag)
	{
		wsprintf(message,L"Creating SelectHardware Singleton");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

		_single.reset(new SelectHardware());

		instanceFlag = true;
	}
	return _single.get();
}

long SelectHardware::GetCommandGUID(GUID *guidRet)
{
	memcpy(guidRet,&guid,sizeof GUID);

	return TRUE;
}

long SelectHardware::GetParamInfo(const long paramID, long &paramType,long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			paramType = ICommand::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = 1.0;
			paramMax = 1.0;
			paramDefault = 1.0;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			paramType = ICommand::TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = FALSE;
			paramMin = 0;
			paramMax = 1.0;
			paramDefault = 1.0;
		}
		break;
	default :
		{
			ret = FALSE;
		}
	}

	return ret;
}

long SelectHardware::SetParam(const long paramID, const double param)
{
	long ret = FALSE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			ret = FALSE;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			if((param >= ICommand::PARAM_SHOWDIALOG_MIN) && (param<=ICommand::PARAM_SHOWDIALOG_MAX))
			{
				params.showDialog = static_cast<long>(param);
				ret = TRUE;
			}
		}
		break;
	default :
		{
		}
	}

	return ret;
}

long SelectHardware::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_VERSION:
		{
			param = params.version;
		}
		break;
	case PARAM_SHOWDIALOG:
		{
			param = params.showDialog;
		}
		break;
	default :
		{
			ret = FALSE;
		}
	}

	return ret;
}

long SelectHardware::SetCustomParamsBinary(const char *buf)
{
	memcpy(&paramsCustom,buf,sizeof (paramsCustom));

	return TRUE;
}

long SelectHardware::GetCustomParamsBinary(char *buf)
{
	memcpy(buf,&paramsCustom,sizeof (paramsCustom));

	return TRUE;
}

long SelectHardware::SaveCustomParamsXML(void *fileHandle)
{
	return FALSE;
}

long SelectHardware::LoadCustomParamXML(void *fileHandle)
{
	//read the setting file again
	if(NULL == fileHandle)
	{
		ReadHardwareSetupXML();
	}
	return FALSE;
}

long SelectHardware::Execute()
{
	return TRUE;
}

long SelectHardware::Status(long &status)
{
	status = ICommand::STATUS_READY;

	return TRUE;
}

long SelectHardware::InitializeCustomParameters()
{	
	long i;
	long id=0;
	long numCameras=0;
	long numDevices=0;

	wchar_t appDir[_MAX_PATH];

	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	wstringstream ss;

	//retrieve the application directory
	GetModuleFileNameW(NULL,appDir,_MAX_PATH);

	if(wcsstr(appDir,L"QTAgent.exe"))
	{
		//this is being executed in a unit test
		//choose a different directory for locating dlls
		GetCurrentDirectoryW(_MAX_PATH,appDir);
		ss << appDir << "\\Modules_Native\\*.dll";
	}
	else
	{
		_wsplitpath_s(appDir,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
		ss << drive << dir << "Modules_Native\\*.dll";
	}	

	wsprintf(message,L"SelectHardware current directory %s",ss.str().c_str());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	if(FALSE == CameraManager::getInstance()->FindCameras(ss.str().c_str(),numCameras))
	{		
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SelectHardware CameraManager::FindCameras failed");
	}

	wsprintf(message,L"SelectHardware number of potential active cameras %d",numCameras);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	cameras.clear();
	cameraInfos.clear();

	std::multiset<ICamera*> cameraSet;

	//TODO retreive the cameras from a camera setup command
	for(i=0; i<numCameras; i++)
	{
		if(CameraManager::getInstance()->GetCameraId(i,id)==TRUE)
		{
			ICamera* camera = NULL;

			camera = CameraManager::getInstance()->GetCamera(id);

			wstring camName = CameraManager::getInstance()->GetCameraDllName(id);

			if(NULL != camera)
			{
				int count = static_cast<int>(cameraSet.count(camera));

				//select next camera if the previous one had been selected
				if(camera->SelectCamera(count) == TRUE)
				{
					//**TODO**
					//add checks for camera parameter equality
					// the assumption is that thee cameras have the same xy dimensions and bit depth

					//only add to the list if the camera is available
					cameras.push_back(camera);
					CameraInfo* camInfo = new CameraInfo();
					camInfo->dllName = camName;
					camInfo->active = L"0";
					camInfo->camID = id;
					wchar_t temp[MAX_CAM_NAME_LEN];
					memset(temp, 0, MAX_CAM_NAME_LEN);
					if(camera->GetParamString(ICamera::PARAM_DETECTOR_NAME, temp, MAX_CAM_NAME_LEN))
					{
						camInfo->cameraName = wstring(temp);
					}
					else
					{
						camInfo->cameraName = camInfo->dllName;
					}
					memset(temp, 0, MAX_CAM_NAME_LEN);
					if(camera->GetParamString(ICamera::PARAM_DETECTOR_SERIAL_NUMBER, temp, MAX_CAM_NAME_LEN))
					{
						camInfo->serialNumber = L"-" + wstring(temp);
					}
					else
					{
						camInfo->serialNumber = L"";
					}

					cameraInfos.push_back(camInfo);

					cameraSet.insert(camera);
				}
			}
		}
	}

	itCam = cameras.begin();
	itCamInfo = cameraInfos.begin();

	wsprintf(message,L"SelectHardware number of cameras loaded %d",cameras.size());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	if(FALSE == DeviceManager::getInstance()->FindDevices(ss.str().c_str(),numDevices))
	{
		logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SelectHardware DeviceManager::FindDevices failed");
	}

	wsprintf(message,L"SelectHardware number of potential devices %d",numDevices);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	devices.clear();
	deviceInfos.clear();
	deviceInfoMap.clear();

	//TODO retreive the devices from a camera setup command
	for(i=0; i<numDevices; i++)
	{
		if(DeviceManager::getInstance()->GetDeviceId(i,id)==TRUE)
		{
			IDevice* device;

			device = DeviceManager::getInstance()->GetDevice(id);

			wstring devName = DeviceManager::getInstance()->GetDeviceDllName(id);

			long j=0;
			long devicesInDll=device->FindDevices(devicesInDll);

			wsprintf(message,L"SelectHardware devices in %s dll %d",devName.c_str(), devicesInDll);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			for(j=0; j<devicesInDll; j++)
			{
				if(device->SelectDevice(j))
				{
					//**TODO**
					//check that the device is a shutter or xy stage
					devices.push_back(device);	

					DeviceInfo* devInfo = new DeviceInfo();
					devInfo->dllName = devName;
					devInfo->active = L"0";
					devInfo->devID = id;
					deviceInfos.push_back(devInfo);
				}
			}				
		}
	}

	wsprintf(message,L"SelectHardware number of devices loaded %d",devices.size());
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	itDev = devices.begin();
	itDevInfo = deviceInfos.begin();

	ReadHardwareSetupXML();

	//after the tables with active devices have been generated and mapped with all connected devices
	//select the active devices and persist them into the HW settings file
	UpdateAndPersistCurrentDevices();

	return TRUE;
}

long SelectHardware::DisconnectHardware()
{	
	if(DeviceManager::getInstance()->ReleaseDevices()!=TRUE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"SelectHardware DeviceManager::ReleaseDevices failed");
	}

	if(CameraManager::getInstance()->ReleaseCameras()!=TRUE)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT,1,L"SelectHardware CameraManager::ReleaseCameras failed");
	}

	return TRUE;
}

long SelectHardware::SetupCommand()
{

	if(FALSE == _setupFlag)
	{	
		//load the camera and hardward list once
		_setupFlag = TRUE;

		InitializeCustomParameters();

		return TRUE;
	}
	else
	{
		return TRUE;
	}
}

long SelectHardware::TeardownCommand()
{
	if(TRUE == _setupFlag)
	{
		//disconnect all the hardware before exiting the application
		DisconnectHardware();
		_setupFlag = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


long SelectHardware::Stop()
{
	return TRUE;
}


long SelectHardware::GetDeviceID(long deviceType)
{
	long ret = 0;
	switch(deviceType)
	{
	case IDevice::CONTROL_UNIT: ret = paramsCustom.activeControlUnit;break;
	case IDevice::PMT1:	ret = paramsCustom.activePMT1;break;
	case IDevice::PMT2:	ret = paramsCustom.activePMT2;break;
	case IDevice::PMT3:	ret = paramsCustom.activePMT3;break;
	case IDevice::PMT4:	ret = paramsCustom.activePMT4;break;
	case IDevice::POWER_REG:ret = paramsCustom.activePowerRegulator;break;
	case IDevice::BEAM_EXPANDER:ret = paramsCustom.activeBeamExpander;break;
	case IDevice::LASER1:ret = paramsCustom.activeLaser1;break;
	case IDevice::LASER2:ret = paramsCustom.activeLaser2;break;
	case IDevice::LASER3:ret = paramsCustom.activeLaser3;break;
	case IDevice::LASER4:ret = paramsCustom.activeLaser4;break;
	case IDevice::PINHOLE_WHEEL:ret = paramsCustom.activePinholeWheel;break;
	case IDevice::STAGE_Z:ret = paramsCustom.activeZStage;break;
	case IDevice::STAGE_Z2:ret = paramsCustom.activeZStage2;break;
	case IDevice::STAGE_X:
	case IDevice::STAGE_Y:ret = paramsCustom.activeXYStage;break;
	case IDevice::AUTOFOCUS:ret = paramsCustom.activeAutoFocus;break;
		//case IDevice::FILTER_WHEEL_EX:ret = paramsCustom.activeExcitation;break;
	case IDevice::FILTER_WHEEL_EM:ret = paramsCustom.activeEmission;break;
	case IDevice::FILTER_WHEEL_DIC:ret = paramsCustom.activeDichroic;break;
	case IDevice::SHUTTER:ret = paramsCustom.activeShutter1;break;
	case IDevice::TURRET:ret = paramsCustom.activeTurret;break;
	case IDevice::LAMP:ret = paramsCustom.activeBFLamp;break;
	case IDevice::SLM:ret = paramsCustom.activeSLM;break;
	case IDevice::STAGE_R:ret = paramsCustom.activeRStage;break;
	case IDevice::PMT_SWITCH:ret = paramsCustom.activePMTSwitch;break;
	case IDevice::EPHYS:ret = paramsCustom.activeEphys;break;
	case IDevice::LIGHT_PATH:ret = paramsCustom.activeLightPath;break;
	case IDevice::SPECTRUM_FILTER: ret = paramsCustom.activeSpectrumFilter;break;
	case IDevice::BEAM_STABILIZER: ret = paramsCustom.activeBeamStabilizer;break;
	case IDevice::POWER_REG2: ret = paramsCustom.activePowerRegulator2;break;
	}

	return ret;
}

long SelectHardware::GetDeviceID(string dllName)
{
	for(long i=0; i<static_cast<long>(devices.size()); i++)
	{
		//assume the device_type parameter always returns a read only long
		double type;
		if(itDev[i]->GetParam(IDevice::PARAM_DEVICE_TYPE,type)==TRUE)
		{
			long lDevType = static_cast<long>(type);
			const wstring devDllName = wstring(dllName.begin(), dllName.end());
			if(devDllName == itDevInfo[i]->dllName)
			{
				return itDevInfo[i]->devID;
			}
		}
	}
	return 0;
}

long SelectHardware::SetDetectorName(long detectorID, wchar_t* name)
{
	try
	{
		ICamera* camera = NULL;

		camera = CameraManager::getInstance()->GetCamera(detectorID);

		long paramType;
		long paramAvailable;
		long paramReadOnly;
		double dummy;

		//change the detector name only if it is supported and not read only
		if(camera->GetParamInfo(ICamera::PARAM_DETECTOR_NAME, paramType, paramAvailable, paramReadOnly, dummy, dummy, dummy))
		{
			if(paramAvailable && (!paramReadOnly))
			{
				if(camera->SetParamString(ICamera::PARAM_DETECTOR_NAME, name))
				{
					for(int i=0; i<static_cast<int>(cameras.size()); i++)
					{
						if(itCamInfo[i]->camID == detectorID)
						{
							itCamInfo[i]->cameraName = wstring(name);
						}
					}
					return TRUE;
				}
			}
		}
	}
	catch(...)
	{
		wsprintf(message,L"SelectHardware SetDetectorName failed. ");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
	}


	return FALSE;
}

long SelectHardware::SetDeviceID(long deviceType, long deviceID)
{
	bool bDevExists = (1 == DeviceExists(deviceType, deviceID));

	long ret = TRUE;

	switch(deviceType)
	{
	case IDevice::CONTROL_UNIT:
		{
			paramsCustom.activeControlUnit = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active CONTROL_UNIT %d",paramsCustom.activeControlUnit);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PMT1:	
		{
			paramsCustom.activePMT1 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PMT1 %d",paramsCustom.activePMT1);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PMT2:	
		{
			paramsCustom.activePMT2 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PMT2 %d",paramsCustom.activePMT2);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PMT3:	
		{
			paramsCustom.activePMT3 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PMT3 %d",paramsCustom.activePMT3);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PMT4:	
		{
			paramsCustom.activePMT4 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PMT4 %d",paramsCustom.activePMT4);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::POWER_REG:
		{
			paramsCustom.activePowerRegulator = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PowerRegulator %d",paramsCustom.activePowerRegulator);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::BEAM_EXPANDER:
		{
			paramsCustom.activeBeamExpander = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active BeamExpander %d",paramsCustom.activeBeamExpander);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LASER1:
		{
			paramsCustom.activeLaser1 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Laser1 %d",paramsCustom.activeLaser1);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LASER2:
		{
			paramsCustom.activeLaser2 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Laser2 %d",paramsCustom.activeLaser2);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LASER3:
		{
			paramsCustom.activeLaser3 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Laser3 %d",paramsCustom.activeLaser3);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LASER4:
		{
			paramsCustom.activeLaser4 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Laser4 %d",paramsCustom.activeLaser4);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PINHOLE_WHEEL:
		{
			paramsCustom.activePinholeWheel = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PinholeWheel %d",paramsCustom.activePinholeWheel);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::STAGE_Z:
		{
			paramsCustom.activeZStage = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active ZStage %d",paramsCustom.activeZStage);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::STAGE_Z2:
		{
			paramsCustom.activeZStage2 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active ZStage2 %d",paramsCustom.activeZStage2);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

			//assume the r stage is on the same selected device as Z stage2
			paramsCustom.activeRStage = (bDevExists == true ? deviceID : 0); 
			wsprintf(message,L"SelectHardware SetDeviceID active Stage R %d",paramsCustom.activeRStage);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);		
		}
		break;
	case IDevice::STAGE_X:
	case IDevice::STAGE_Y: 
	case IDevice::STAGE_X | IDevice::STAGE_Y:
		{
			paramsCustom.activeXYStage = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active XYStage %d",paramsCustom.activeXYStage);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::AUTOFOCUS:
		{
			paramsCustom.activeAutoFocus = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active AutoFocus %d",paramsCustom.activeAutoFocus);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
		//case IDevice::FILTER_WHEEL_EX:
		//	{
		//		paramsCustom.activeExcitation = (bDevExists == true ? deviceID : 0);
		//		wsprintf(message,L"SelectHardware SetDeviceID active Excitation %d",paramsCustom.activeExcitation);
		//		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		//	}
		//	break;
	case IDevice::FILTER_WHEEL_EM:
		{
			paramsCustom.activeEmission = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Emission %d",paramsCustom.activeEmission);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::FILTER_WHEEL_DIC:
		{
			paramsCustom.activeDichroic = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Dichroic %d",paramsCustom.activeDichroic);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::SHUTTER:
		{
			paramsCustom.activeShutter1 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Shutter1 %d",paramsCustom.activeShutter1);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::TURRET:
		{
			paramsCustom.activeTurret = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Turret %d",paramsCustom.activeTurret);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LAMP:
		{
			paramsCustom.activeBFLamp = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active BFLamp %d",paramsCustom.activeBFLamp);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::SLM:
		{
			paramsCustom.activeSLM = (bDevExists == true ? deviceID : 0); 
			wsprintf(message,L"SelectHardware SetDeviceID active SLM %d",paramsCustom.activeSLM);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::PMT_SWITCH:
		{
			paramsCustom.activePMTSwitch = (bDevExists == true ? deviceID : 0); 
			wsprintf(message,L"SelectHardware SetDeviceID active PMT Switch %d",paramsCustom.activePMTSwitch);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::EPHYS:
		{
			paramsCustom.activeEphys = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Ephys %d",paramsCustom.activeEphys);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::LIGHT_PATH:
		{
			paramsCustom.activeLightPath = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Light Path %d",paramsCustom.activeLightPath);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::SPECTRUM_FILTER:
		{
			paramsCustom.activeSpectrumFilter = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Spectrum Filter %d", paramsCustom.activeSpectrumFilter);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	case IDevice::BEAM_STABILIZER:
		{
			paramsCustom.activeBeamStabilizer = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active Beam Stabilizer %d", paramsCustom.activeBeamStabilizer);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;		
	case IDevice::POWER_REG2:
		{
			paramsCustom.activePowerRegulator2 = (bDevExists == true ? deviceID : 0);
			wsprintf(message,L"SelectHardware SetDeviceID active PowerRegulator2 %d",paramsCustom.activePowerRegulator2);
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
		}
		break;
	default:
		ret = FALSE;
	}
	return ret;
}

///<summary>postflight selected camera and bleach scanner to park position</summary>
long SelectHardware::DeslectCameras()
{
	long ret = FALSE;
	long parkAtParking = 1;

	for(long i=0; i<static_cast<long>(cameras.size()); i++)
	{
		if ((paramsCustom.activeCamera1 == itCamInfo[i]->camID) || (paramsCustom.activeBleachingScanner == itCamInfo[i]->camID))
		{
			itCam[i]->PostflightAcquisition((char*)&parkAtParking);
			ret = TRUE;
		}
	}
	return FALSE;
}

long SelectHardware::GetCameraID()
{
	return paramsCustom.activeCamera1;
}

long SelectHardware::GetBleachID()
{
	return paramsCustom.activeBleachingScanner;
}

long SelectHardware::SetCameraID(long camID)
{
	if(CameraExists(ICamera::LSM, camID) || CameraExists(ICamera::CCD, camID))
	{
		paramsCustom.activeCamera1 = camID;
		return TRUE;
	}
	return FALSE;
}

long SelectHardware::GetCameraType(long camID, long &type)
{
	long ret = FALSE;
	type = ICamera::LSM;

	for(unsigned long i=0; i<cameras.size(); i++)
	{
		double dType;
		//assume the device_type parameter always returns a read only long
		if(itCam[i]->GetParam(ICamera::PARAM_CAMERA_TYPE,dType)==TRUE)
		{
			if(itCamInfo[i]->camID == camID)
			{
				type = static_cast<long>(dType);
				ret = TRUE;
			}
		}
	}

	return ret;
}

long SelectHardware::LoadDevice(unsigned long devType, long devIndex)
{
	if(devIndex >= static_cast<long>(deviceInfoMap.count(devType)))
		return FALSE;

	pair<multimap<long, vector<wstring>>::iterator, multimap<long, vector<wstring>>::iterator> pIter;
	pIter = deviceInfoMap.equal_range(devType);

	int i = 0;
	//check if device info already exist in map
	for(multimap<long, vector<wstring>>::iterator it = pIter.first; it != pIter.second; it++, i++)
	{
		if(devIndex == i)
		{
			SetDeviceID(devType, _wtol(it->second.at(2).c_str()));
			it->second.at(1) = L"1";	//set the active attribute in XML
		}
		else
		{
			it->second.at(1) = L"0";	//set the active attribute in XML
		}
	}

	PersistHardwareSetup();

	return TRUE;
}

//translate combobox index to camera ids
long SelectHardware::LoadCamera(long cameraIndex)
{
	if(cameraIndex >= static_cast<long>(cameraInfos.size()))
		return FALSE;

	CameraInfoIterator it = cameraInfos.begin();
	int i = 0;
	while((i < cameraIndex) && (it != cameraInfos.end()))
	{
		it ++;
		i ++;
	}	
	SelectActiveCamera((*it)->camID);

	if(ICamera::LSM == (*it)->cameraType)
	{
		//if there is a valid switch
		//set the switch to ensure signal is going to 
		//the approriate digitizer for the laser scanner
		if(paramsCustom.activePMTSwitch > 0)
		{
			double lsmType;

			itCam[i]->GetParam(ICamera::PARAM_LSM_TYPE,lsmType);
			switch(static_cast<long>(lsmType))
			{
			case ICamera::GALVO_RESONANCE:itDev[paramsCustom.activePMTSwitch-1]->SetParam(IDevice::PARAM_PMT_SWITCH_POS, 0);
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SelectHardware Setting switch 0");break;
			case ICamera::GALVO_GALVO:itDev[paramsCustom.activePMTSwitch-1]->SetParam(IDevice::PARAM_PMT_SWITCH_POS, 1);
				logDll->TLTraceEvent(VERBOSE_EVENT,1,L"SelectHardware Setting switch 1");break;
			}
			itDev[paramsCustom.activePMTSwitch-1]->PreflightPosition();
			itDev[paramsCustom.activePMTSwitch-1]->SetupPosition();
			itDev[paramsCustom.activePMTSwitch-1]->StartPosition();
			itDev[paramsCustom.activePMTSwitch-1]->PostflightPosition();
		}
	}

	return TRUE;
}

long SelectHardware::SetActiveBleachingScanner(long scannerID)
{
	long scannerIndex = scannerID - 1;
	if(scannerIndex >= static_cast<long>(cameraInfos.size()))
		return FALSE;

	wsprintf(message,L"SelectHardware SetActiveBleachingScanner %d",scannerID);
	logDll->TLTraceEvent(VERBOSE_EVENT,1,message);

	paramsCustom.activeBleachingScanner = scannerID;

	//only one camera can be active bleaching scanner:
	for(int i=0;i<static_cast<int>(cameraInfos.size());i++)
	{
		itCamInfo[i]->activation = L"0";
	}
	itCamInfo[scannerIndex]->activation = L"1";

	PersistCameraInfosToMap();

	return TRUE;
}

long SelectHardware::AddDeviceInfoToMap(long devType, wstring wsDevName, wstring wsActive, wstring wsId)
{
	pair<multimap<long, vector<wstring>>::iterator, multimap<long, vector<wstring>>::iterator> pIter;
	pIter = deviceInfoMap.equal_range(devType);

	//check if device info already exist in map
	for(multimap<long, vector<wstring>>::iterator it = pIter.first; it != pIter.second; ++it)
	{
		if((devType == (*it).first) && (wsId.compare((*it).second.at(2)) == 0)) //deviceInfo already in map
		{
			(*it).second.at(0) = wsDevName;
			(*it).second.at(1) = wsActive;
			return FALSE;
		}
	}

	vector<wstring> devInfo (3);
	devInfo.at(0) = wsDevName;
	devInfo.at(1) = wsActive;
	devInfo.at(2) = wsId;

	deviceInfoMap.insert(pair<long, vector<wstring>>(devType, devInfo));

	return TRUE;
}

long SelectHardware::PersistCameraInfosToMap()
{
	//clear the map so not polluted by previous set-up
	cameraInfoMap.clear();

	CameraInfoIterator it;
	for(it = cameraInfos.begin(); it != cameraInfos.end(); it++)
	{
		vector<wstring> vCamInfo;

		vCamInfo.push_back((*it)->dllName);

		vCamInfo.push_back((*it)->active);

		std::wstringstream wss;
		wss<<(*it)->camID;
		vCamInfo.push_back(wss.str());

		vCamInfo.push_back((*it)->cameraName);

		vCamInfo.push_back((*it)->serialNumber);

		vCamInfo.push_back((*it)->activation);

		cameraInfoMap.insert(pair<long, vector<wstring>>((*it)->cameraType, vCamInfo));
	}

	return TRUE;
}


long SelectHardware::SelectFirstActiveDevice(long devType)
{
	long ret = FALSE;

	//check if a device is available
	if(devices.size() == 0)
	{
		return ret;
	}

	for(unsigned long i=0; i<devices.size(); i++)
	{
		//assume the device_type parameter always returns a read only long
		double type;
		if(itDev[i]->GetParam(IDevice::PARAM_DEVICE_TYPE,type)==TRUE)
		{
			long lDevType = static_cast<long>(type);

			if(lDevType & devType)
			{
				//select first device sharing this type
				if(ret == FALSE)
				{
					SetDeviceID(devType, itDevInfo[i]->devID);
					itDevInfo[i]->active = L"1";
				}
				else
				{
					itDevInfo[i]->active = L"0";
				}

				wchar_t tempID[20];
				_ltow_s(itDevInfo[i]->devID, tempID,20, 10);
				AddDeviceInfoToMap(devType, itDevInfo[i]->dllName, itDevInfo[i]->active, wstring(tempID));

				ret = TRUE;
			}
		}
	}

	return ret;
}

long SelectHardware::SelectActiveDevice(unsigned long devType, long devID)
{
	long ret = FALSE;

	//check if a device is available
	if(devices.size() == 0)
	{
		return ret;
	}

	for(unsigned long i=0; i<devices.size(); i++)
	{
		//assume the device_type parameter always returns a read only long
		double type;
		if(itDev[i]->GetParam(IDevice::PARAM_DEVICE_TYPE,type)==TRUE)
		{
			long lDevType = static_cast<long>(type);

			if(lDevType & devType)
			{
				if(devID == itDevInfo[i]->devID)
				{
					SetDeviceID(devType, devID);
					itDevInfo[i]->active = L"1";
					ret = TRUE;
				}
				else
				{
					itDevInfo[i]->active = L"0";
				}

				wchar_t tempID[20];
				_ltow_s(itDevInfo[i]->devID, tempID,20, 10);
				AddDeviceInfoToMap(devType, itDevInfo[i]->dllName, itDevInfo[i]->active, wstring(tempID));
			}
		}
	}

	if(ret == FALSE) //no matched device found
	{
		LoadDevice(devType, 0); //select the first active device
	}

	return ret;
}

long SelectHardware::SelectCustomCamerasAndDevices(SelectHardwareCustomParams params,bool initialize)
{
	long ret = TRUE;

	if(initialize)
	{
		ZeroMemory(&params, sizeof(SelectHardwareCustomParams));
	}
	//cameras:
	if(0==cameras.size())
	{
		paramsCustom.activeCamera1=0;
	}
	else
	{
		if((params.activeCamera1 == 0) || (params.activeCamera1 > static_cast<long>(cameras.size())))
		{
			SelectActiveCamera(1); //select the first camera by default
		}
		else if(params.activeCamera1 != paramsCustom.activeCamera1)
		{
			SelectActiveCamera(params.activeCamera1); //select the camera specified by user
		}

		if((params.activeBleachingScanner == 0) || (params.activeBleachingScanner > static_cast<long>(cameras.size())))
		{
			//no default bleach scanner
		}
		else
		{
			SetActiveBleachingScanner(params.activeBleachingScanner); //select the bleaching scanner
		}
	}

	//devices:
	if(0==devices.size())
	{
	}
	else
	{
		if(params.activeXYStage == 0)
		{
			SelectFirstActiveDevice(IDevice::STAGE_X | IDevice::STAGE_Y);
		}
		else if(params.activeXYStage != paramsCustom.activeXYStage)
		{
			SelectActiveDevice(IDevice::STAGE_X | IDevice::STAGE_Y, params.activeXYStage);
		}

		if(params.activeZStage == 0)
		{
			SelectFirstActiveDevice(IDevice::STAGE_Z);
		}
		else if(params.activeZStage != paramsCustom.activeZStage)
		{
			SelectActiveDevice(IDevice::STAGE_Z, params.activeZStage);
		}

		if(params.activeZStage2 == 0)
		{
			SelectFirstActiveDevice(IDevice::STAGE_Z2);
		}
		else if(params.activeZStage2 != paramsCustom.activeZStage2)
		{
			SelectActiveDevice(IDevice::STAGE_Z2, params.activeZStage2);
		}

		if(params.activeShutter1 == 0)
		{
			SelectFirstActiveDevice(IDevice::SHUTTER);
		}
		else if(params.activeShutter1 != paramsCustom.activeShutter1)
		{
			SelectActiveDevice(IDevice::SHUTTER, params.activeShutter1);
		}

		//if(params.activeExcitation == 0)
		//{
		//	SelectFirstActiveDevice(IDevice::FILTER_WHEEL_EX);
		//}
		//else if(params.activeExcitation != paramsCustom.activeExcitation)
		//{
		//	SelectActiveDevice(IDevice::FILTER_WHEEL_EX, params.activeExcitation);
		//}

		if(params.activeEmission == 0)
		{
			SelectFirstActiveDevice(IDevice::FILTER_WHEEL_EM);
		}
		else if(params.activeEmission != paramsCustom.activeEmission)
		{
			SelectActiveDevice(IDevice::FILTER_WHEEL_EM, params.activeEmission);
		}

		if(params.activeDichroic == 0)
		{
			SelectFirstActiveDevice(IDevice::FILTER_WHEEL_DIC);
		}
		else if(params.activeDichroic != paramsCustom.activeDichroic)
		{
			SelectActiveDevice(IDevice::FILTER_WHEEL_DIC, params.activeDichroic);
		}

		if(params.activeTurret == 0)
		{
			SelectFirstActiveDevice(IDevice::TURRET);
		}
		else if(params.activeTurret != paramsCustom.activeTurret)
		{
			SelectActiveDevice(IDevice::TURRET, params.activeTurret);
		}

		if(params.activeBFLamp == 0)
		{
			SelectFirstActiveDevice(IDevice::LAMP);
		}
		else if(params.activeBFLamp != paramsCustom.activeBFLamp)
		{
			SelectActiveDevice(IDevice::LAMP, params.activeBFLamp);
		}

		if(params.activeAutoFocus == 0)
		{
			SelectFirstActiveDevice(IDevice::AUTOFOCUS);
		}
		else if(params.activeAutoFocus != paramsCustom.activeAutoFocus)
		{
			SelectActiveDevice(IDevice::AUTOFOCUS, params.activeAutoFocus);
		}

		if(params.activeControlUnit == 0)
		{
			SelectFirstActiveDevice(IDevice::CONTROL_UNIT);
		}
		else if(params.activeControlUnit != paramsCustom.activeControlUnit)
		{
			SelectActiveDevice(IDevice::CONTROL_UNIT, params.activeControlUnit);
		}

		if(params.activePMT1 == 0)
		{
			SelectFirstActiveDevice(IDevice::PMT1);
		}
		else if(params.activePMT1 != paramsCustom.activePMT1)
		{
			SelectActiveDevice(IDevice::PMT1, params.activePMT1);
		}

		if(params.activePMT2 == 0)
		{
			SelectFirstActiveDevice(IDevice::PMT2);
		}
		else if(params.activePMT2 != paramsCustom.activePMT2)
		{
			SelectActiveDevice(IDevice::PMT2, params.activePMT2);
		}

		if(params.activePMT3 == 0)
		{
			SelectFirstActiveDevice(IDevice::PMT3);
		}
		else if(params.activePMT3 != paramsCustom.activePMT3)
		{
			SelectActiveDevice(IDevice::PMT3, params.activePMT3);
		}

		if(params.activePMT4 == 0)
		{
			SelectFirstActiveDevice(IDevice::PMT4);
		}
		else if(params.activePMT4 != paramsCustom.activePMT4)
		{
			SelectActiveDevice(IDevice::PMT4, params.activePMT4);
		}

		if(params.activePowerRegulator == 0)
		{
			SelectFirstActiveDevice(IDevice::POWER_REG);
		}
		else if(params.activePowerRegulator != paramsCustom.activePowerRegulator)
		{
			SelectActiveDevice(IDevice::POWER_REG, params.activePowerRegulator);
		}

		if(params.activeBeamExpander == 0)
		{
			SelectFirstActiveDevice(IDevice::BEAM_EXPANDER);
		}
		else if(params.activeBeamExpander != paramsCustom.activeBeamExpander)
		{
			SelectActiveDevice(IDevice::BEAM_EXPANDER, params.activeBeamExpander);
		}

		if(params.activeLaser1 == 0)
		{
			SelectFirstActiveDevice(IDevice::LASER1);	
		}
		else if(params.activeLaser1 != paramsCustom.activeLaser1)
		{
			SelectActiveDevice(IDevice::LASER1, params.activeLaser1);
		}

		if(params.activeLaser2 == 0)
		{
			SelectFirstActiveDevice(IDevice::LASER2);	
		}
		else if(params.activeLaser2 != paramsCustom.activeLaser2)
		{
			SelectActiveDevice(IDevice::LASER2, params.activeLaser2);
		}

		if(params.activeLaser3 == 0)
		{
			SelectFirstActiveDevice(IDevice::LASER3);	
		}
		else if(params.activeLaser3 != paramsCustom.activeLaser3)
		{
			SelectActiveDevice(IDevice::LASER3, params.activeLaser3);
		}

		if(params.activeLaser4 == 0)
		{
			SelectFirstActiveDevice(IDevice::LASER4);	
		}
		else if(params.activeLaser4 != paramsCustom.activeLaser4)
		{
			SelectActiveDevice(IDevice::LASER4, params.activeLaser4);
		}

		if(params.activePinholeWheel == 0)
		{
			SelectFirstActiveDevice(IDevice::PINHOLE_WHEEL);
		}
		else if(params.activePinholeWheel != paramsCustom.activePinholeWheel)
		{
			SelectActiveDevice(IDevice::PINHOLE_WHEEL, params.activePinholeWheel);
		}

		if(params.activeSLM == 0)
		{
			SelectFirstActiveDevice(IDevice::SLM);
		}
		else if(params.activeSLM != paramsCustom.activeSLM)
		{
			SelectActiveDevice(IDevice::SLM, params.activeSLM);
		}

		if(params.activeRStage == 0)
		{
			SelectFirstActiveDevice(IDevice::STAGE_R);
		}
		else if(params.activeRStage != paramsCustom.activeRStage)
		{
			SelectActiveDevice(IDevice::STAGE_R, params.activeRStage);
		}

		if(params.activePMTSwitch == 0)
		{
			SelectFirstActiveDevice(IDevice::PMT_SWITCH);
		}
		else if(params.activePMTSwitch != paramsCustom.activePMTSwitch)
		{
			SelectActiveDevice(IDevice::PMT_SWITCH, params.activePMTSwitch);		
		}

		if(params.activeEphys == 0)
		{
			SelectFirstActiveDevice(IDevice::EPHYS);
		}
		else if(params.activeEphys != paramsCustom.activeEphys)
		{
			SelectActiveDevice(IDevice::EPHYS, params.activeEphys);
		}

		if(params.activeLightPath == 0)
		{
			SelectFirstActiveDevice(IDevice::LIGHT_PATH);
		}
		else if(params.activeLightPath != paramsCustom.activeLightPath)
		{
			SelectActiveDevice(IDevice::LIGHT_PATH, params.activeLightPath);
		}

		if(params.activeSpectrumFilter == 0)
		{
			SelectFirstActiveDevice(IDevice::SPECTRUM_FILTER);
		}
		else if(params.activeSpectrumFilter != paramsCustom.activeSpectrumFilter)
		{
			SelectActiveDevice(IDevice::SPECTRUM_FILTER, params.activeSpectrumFilter);
		}

		if(params.activeBeamStabilizer == 0)
		{
			SelectFirstActiveDevice(IDevice::BEAM_STABILIZER);
		}
		else if(params.activeBeamStabilizer != paramsCustom.activeBeamStabilizer)
		{
			SelectActiveDevice(IDevice::BEAM_STABILIZER, params.activeBeamStabilizer);
		}

		if(params.activePowerRegulator2 == 0)
		{
			SelectFirstActiveDevice(IDevice::POWER_REG2);
		}
		else if(params.activePowerRegulator2 != paramsCustom.activePowerRegulator2)
		{
			SelectActiveDevice(IDevice::POWER_REG2, params.activePowerRegulator2);
		}
	}

	return ret;
}

long SelectHardware::DeviceExists(long devType, long devID)
{
	for(long i=0; i<static_cast<long>(devices.size()); i++)
	{
		//assume the device_type parameter always returns a read only long
		double type;
		if(itDev[i]->GetParam(IDevice::PARAM_DEVICE_TYPE,type)==TRUE)
		{
			long lDevType = static_cast<long>(type);

			if((lDevType & devType) && (itDevInfo[i]->devID == devID))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

long SelectHardware::CameraExists(long camType, long camID)
{
	for(long i=0; i<static_cast<long>(cameras.size()); i++)
	{
		//assume the device_type parameter always returns a read only long
		double type;
		if(itCam[i]->GetParam(ICamera::PARAM_CAMERA_TYPE,type)==TRUE)
		{
			long lCamType = static_cast<long>(type);
			long lCamId = itCamInfo[i]->camID;

			if((lCamType == camType) && (lCamId == camID))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

long SelectHardware::PersistHardwareSetup()
{
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);
	pHardware->PersistHardwareSetup(deviceInfoMap, cameraInfoMap);
	return TRUE;
}

//select the active devices and persist into the HW settings XML file
long SelectHardware::UpdateAndPersistCurrentDevices()
{
	SelectHardwareCustomParams paramsSelected = paramsCustom;

	//update with new devices if exist:
	SelectCustomCamerasAndDevices(paramsCustom, true);

	//reset active devices for user:
	SelectCustomCamerasAndDevices(paramsSelected, false);

	PersistHardwareSetup();

	return TRUE;
}

long SelectHardware::ReadHardwareSetupXML()
{
	auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML);

	long camID = 0;
	if(pHardware->GetActiveHardwareID(pHardware->IMAGEDETECTORS_TAG, pHardware->LSM_TAG, camID) == FALSE)
	{
		pHardware->GetActiveHardwareID(pHardware->IMAGEDETECTORS_TAG, pHardware->CAMERA_TAG, camID);
	}
	SetCameraID(camID);

	long activationCamID = 0;
	if(pHardware->GetActivationCameraID(activationCamID))
	{
		paramsCustom.activeBleachingScanner = activationCamID;
	}

	//For all devices use the index of the currently connected devices to set the active device index
	//Look for the device with the matching dllName to get the index
	string devDllName = "";
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->CONTROL_UNIT_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::CONTROL_UNIT, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PMT1_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PMT1, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PMT2_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PMT2, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PMT3_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PMT3, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PMT4_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PMT4, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->SHUTTER_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::SHUTTER, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->STAGE_Z_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::STAGE_Z, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->STAGE_Z2_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::STAGE_Z2, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->POWER_REG_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::POWER_REG, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->BEAM_EXPANDER_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::BEAM_EXPANDER, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->MCLS_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::LASER1, devID);
		SetDeviceID(IDevice::LASER2, devID);
		SetDeviceID(IDevice::LASER3, devID);
		SetDeviceID(IDevice::LASER4, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PINHOLE_WHEEL_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PINHOLE_WHEEL, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->STAGE_XY_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::STAGE_X | IDevice::STAGE_Y, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->LIGHT_PATH_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::LIGHT_PATH, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->SPECTRUM_FILTER_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::SPECTRUM_FILTER, devID);
	}	
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->TURRET_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::TURRET, devID);
	}		
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->SLM_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::SLM, devID);
	}		
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->PMT_SWITCH_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::PMT_SWITCH, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->BEAM_STABILIZER_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::BEAM_STABILIZER, devID);
	}	
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->POWER_REG2_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::POWER_REG2, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->EPI_TURRET_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::FILTER_WHEEL_DIC, devID);
	}
	if(pHardware->GetActiveHardwareDllName(pHardware->DEVICES_TAG, pHardware->LAMP_TAG, devDllName))
	{
		long devID = GetDeviceID(devDllName);
		SetDeviceID(IDevice::LAMP, devID);
	}

	return TRUE;
}

long SelectHardware::SelectActiveCamera(long id)
{
	long ret = FALSE;

	if(cameras.size() == 0)
		return FALSE;

	paramsCustom.activeCamera1 = 1; //first camera selected by default

	std::multiset<ICamera*> cameraSet;

	for(long i=0; i<static_cast<long>(cameras.size()); i++)
	{
		//assume the camera_type parameter always returns a read only long
		double type;
		if(itCam[i]->GetParam(ICamera::PARAM_CAMERA_TYPE,type)==TRUE)
		{
			itCamInfo[i]->cameraType = static_cast<long>(type);

			if((id == itCamInfo[i]->camID) && (ret == FALSE))
			{
				int count = static_cast<int>(cameraSet.count(itCam[i]));
				itCam[i]->SelectCamera(count);

				paramsCustom.activeCamera1 = id;
				itCamInfo[i]->active = L"1";
				ret = TRUE;
			}
			else
			{
				itCamInfo[i]->active = L"0";
			}

			cameraSet.insert(itCam[i]);
		}
	}

	PersistCameraInfosToMap();

	return ret;
}

long SelectHardware::GetCameraParameter(long camID, long paramID, double &param)
{
	long ret = FALSE;

	if((cameras.size() <= 0) || (1 > camID))
	{
		return ret;
	}

	//assume the device_type parameter always returns a read only long
	if(itCam[camID-1]->GetParam(paramID,param)==TRUE)
	{
		ret = TRUE;
	}	

	return ret;
}

long SelectHardware::GetDeviceParamInfo(const long deviceID, const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = FALSE;

	if(1 > deviceID)
	{
		return ret;
	}

	//assume the device_type parameter always returns a read only long
	if(itDev[deviceID-1]->GetParamInfo(paramID,paramType,paramAvailable,paramReadOnly,paramMin,paramMax,paramDefault))
	{
		ret = TRUE;
	}	

	return ret;
}

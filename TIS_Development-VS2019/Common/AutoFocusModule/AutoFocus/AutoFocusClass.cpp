// ThorLoggingClass.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <codecvt>
#include "AutoFocus.h"
#include "AutoFocusClass.h"
#include "AutoFocusHW.h"
#include "AutoFocusHWandImage.h"
#include "AutoFocusNone.h"
#include "AutoFocusImage.h"

auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));

double IAutoFocus::_lastGoodFocusPosition = 0.0;
auto_ptr<AutoFocusHW> _afHW(new AutoFocusHW());
auto_ptr<AutoFocusHWandImage> _afHWImage(new AutoFocusHWandImage());
auto_ptr<AutoFocusImage> _afImage(new AutoFocusImage());
auto_ptr<AutoFocusNone> _afNone(new AutoFocusNone());
HANDLE IAutoFocus::hEventZ = NULL;
double IAutoFocus::_adaptiveOffset = 0;
long IAutoFocus::_selectedAutoFocusType = AutoFocusTypes::AF_NONE;
long IAutoFocus::_autoFocusRunning = FALSE;

UINT StatusZThreadProc2(LPVOID pParam)
{
	long status = IDevice::STATUS_BUSY;

	IDevice* pDevice = (IDevice*)pParam;

	while (status == IDevice::STATUS_BUSY)
	{
		if (FALSE == pDevice->StatusPosition(status))
		{
			break;
		}
	}

	SetEvent(IAutoFocus::hEventZ);

	return 0;
}

IAutoFocus::IAutoFocus()
{
}

IAutoFocus::~IAutoFocus()
{
}

bool IAutoFocus::instanceFlag = false;

IAutoFocus* IAutoFocus::single = NULL;

CritSect IAutoFocus::critSect;

void IAutoFocus::cleanup(void)
{
	Lock lock(critSect);

	if(single != NULL)
	{
		delete single;
	}
}

IAutoFocus* IAutoFocus::getInstance()
{
	Lock lock(critSect);

	if(! instanceFlag)
	{
		
		try
		{
			single = new IAutoFocus();
		}
		catch(...)
		{
			//critically low on resources
			//do not proceed with application
			throw;
		}
		instanceFlag = true;
		atexit(cleanup);
		return single;
	}
	else
	{
		return single;
	}
}

//Methods will be overrode by the subclasess of IAutoFocus
long IAutoFocus::Execute(long index, IDevice* pAutoFocus, BOOL& bFound)
{
	return FALSE;
}

long IAutoFocus::WillAFExecuteNextIteration()
{
	return FALSE;
}

long IAutoFocus::SetupAF(long afType, long repeat, double afFocusOffset, double expTimeMS, double stepSizeUM, double startPosMM, double stopPosMM, long binning, double finePercentage, long enableGUIUpdate)
{
	int result = EXIT_SUCCESS;
	try
	{
		/*		Old setup, this part should be done in the project calling the AutoFocus module
		//reading from the experiment setup XML files
		IExperiment* exp;

		wstring tempPath = ResourceManager::getInstance()->GetActiveSettingsFilePathAndName();

		exp = ExperimentManager::getInstance()->GetExperiment(tempPath);

		//Get filter parameters from hardware setup.xml
		auto_ptr<HardwareSetupXML> pHardware(new HardwareSetupXML());

		string objName;
		double magnification;
		long position = 0;
		double numAperture;
		double afStartPos = 0;
		double afFocusOffset = 0;
		double afAdaptiveOffset = 0;
		long beamExpPos = 0;
		long beamExpWavelength = 0;
		long beamExpPos2 = 0;
		long beamExpWavelength2 = 0;
		long turretPosition = 0;
		long zAxisToEscape = 0;
		double zAxisEscapeDistance = 0;

		//make sure the file only gets loaded once
		pHardware->SetFastLoad(TRUE);

		pHardware->GetMagInfoFromName(objName, magnification, position, numAperture, afStartPos, afFocusOffset, afAdaptiveOffset, beamExpPos, beamExpWavelength, beamExpPos2, beamExpWavelength2, turretPosition, zAxisToEscape, zAxisEscapeDistance);

		_adaptiveOffset = afAdaptiveOffset;

		long type, repeat;
		double expTimeMS, stepSizeUM, startPosMM, stopPosMM;
		long binning = 1;

		//retrieve the autofocus parameters
		/*exp->GetAutoFocus(type, repeat, expTimeMS, stepSizeUM, startPosMM, stopPosMM);*/

		/*auto_ptr<AutoFocusHW> afHW;
		auto_ptr<AutoFocusHWandImage> afHWImage;
		auto_ptr<AutoFocusImage> afImage;
		auto_ptr<AutoFocusNone> afNone;*/

		//Determine if we are capturing the first image for the experiment. If so make sure an autofocus is executed if enabled.
		//After the first iteration the Z position will overlap with the XY motion
		/*if (afType != IAutoFocus::AF_NONE)
		{
			_lastGoodFocusPosition = afStartPos + _adaptiveOffset;
			if (FALSE == SetAFStartZPosition(afStartPos, TRUE, FALSE))
			{
				return FALSE;
			}
		}
		*/

		ICamera* pCamera = NULL;
		IDevice* pZStage = NULL;

		pCamera = GetCamera(SelectedHardware::SELECTED_CAMERA1);
		pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

		//assign the autofocus type
		switch (afType)
		{
		case IAutoFocus::AF_HARDWARE://hardware only
		{
			_afHW.reset(new AutoFocusHW());
			_afHW->SetupParameters(repeat, afFocusOffset);
		}
		break;

		case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
		{
			_afHWImage.reset(new AutoFocusHWandImage());
			_afHWImage->SetupParameters(pCamera, pZStage, repeat, afFocusOffset, expTimeMS, stepSizeUM, startPosMM, stopPosMM, binning, finePercentage, enableGUIUpdate);
		}
		break;
		case IAutoFocus::AF_IMAGE://image
		{
			_afImage.reset(new AutoFocusImage());
			_afImage->SetupParameters(pCamera, pZStage, repeat, afFocusOffset, expTimeMS, stepSizeUM, startPosMM, stopPosMM, binning, finePercentage, enableGUIUpdate);
		}
		break;
		case IAutoFocus::AF_NONE:
		default:
		{
			_afNone.reset(new AutoFocusNone());
			_afNone->SetupParameters(repeat);
		}
		}
	}

	catch (std::exception&)
	{
		result = EXIT_FAILURE;
	}
	return result;
}

//Manually stop autofocus before moving to a Z Position. Return TRUE(1) if autofocus was stopped within 30ms
long IAutoFocus::StopAF()
{
	int result = TRUE;
	if (_selectedAutoFocusType != IAutoFocus::AF_NONE)
	{
		switch (_selectedAutoFocusType)
		{
		case IAutoFocus::AF_HARDWARE://hardware only
		{
			_afHW.get()->SetStopFlag(TRUE);
		}
		break;

		case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
		{
			_afHWImage.get()->SetStopFlag(TRUE);
		}
		break;
		case IAutoFocus::AF_IMAGE://image
		{
			_afImage.get()->SetStopFlag(TRUE);
		}
		break;
		}
	}

	Sleep(30);

	return _autoFocusRunning;
}

long IAutoFocus::RunAF(long magnification, long afType, BOOL& bFound)
{
	int result = TRUE;
	try
	{
		IDevice* pAutoFocusDevice = NULL;

		pAutoFocusDevice = GetDevice(SelectedHardware::SELECTED_AUTOFOCUS);

		if (NULL == pAutoFocusDevice)
		{
			logDll->TLTraceEvent(ERROR_EVENT, 1, L"AutoFocus Execute could not create autofocus device");
			return result;
		}

		BOOL afFound = FALSE;
		_selectedAutoFocusType = afType;

		if (afType != IAutoFocus::AF_NONE)
		{
			long res = FALSE;
			_autoFocusRunning = TRUE;
			switch (afType)
			{
			case IAutoFocus::AF_HARDWARE://hardware only
			{
				_afHW.get()->SetStopFlag(FALSE);
				res = _afHW.get()->Execute(magnification, pAutoFocusDevice, afFound);
				/*if (TRUE == _afHW->WillAFExecuteNextIteration())
				{
					////move to an offset of of the start location	
					if (FALSE == SetAFStartZPosition(afStartPos, FALSE, afFound))
					{
						return FALSE;
					}
				}*/
			}
			break;

			case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
			{
				_afHWImage.get()->SetStopFlag(FALSE);
				res = AutoFocusAndRetry(magnification, pAutoFocusDevice, _afHWImage.get(), afFound);
				/*if (TRUE == _afHWImage->WillAFExecuteNextIteration())
				{
					////move to an offset of of the start location	
					if (FALSE == SetAFStartZPosition(afStartPos, FALSE, afFound))
					{
						return FALSE;
					}
				}*/
			}
			break;
			case IAutoFocus::AF_IMAGE://image
			{
				_afImage.get()->SetStopFlag(FALSE);
				res = AutoFocusAndRetry(magnification, pAutoFocusDevice, _afImage.get(), afFound);
				/*if (TRUE == _afImage->WillAFExecuteNextIteration())
				{
					////move to an offset of of the start location	
					if (FALSE == SetAFStartZPosition(afStartPos, FALSE, afFound))
					{
						return FALSE;
					}
				}*/
			}
			break;
			}
			_autoFocusRunning = FALSE;
			if (FALSE == res)
			{
				logDll->TLTraceEvent(ERROR_EVENT, 1, L"RunAF AutoFocusAndRetry failed");
				return result;
			}
		}
	}
	catch (std::exception&)
	{
		result = FALSE;
	}

	return result;
}


// Returns a flag that marks whether the autofoucs loop of repeats is still running
long IAutoFocus::AFExecuteNextIteration(long afType)
{
	long result = FALSE;

	//assign the autofocus type
	switch (afType)
	{
	case IAutoFocus::AF_HARDWARE://hardware only
	{
		result = _afHW->WillAFExecuteNextIteration();
	}
	break;
	case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
	{
		result = _afHWImage->WillAFExecuteNextIteration();
	}
	break;
	case IAutoFocus::AF_IMAGE://image
	{
		result = _afImage->WillAFExecuteNextIteration();
	}
	break;
	case IAutoFocus::AF_NONE:
	default:
	{
		result = _afNone->WillAFExecuteNextIteration();
	}
	}

	return result;
}

long IAutoFocus::AutoFocusAndRetry(long index, IDevice* pAutoFocusDevice, IAutoFocus* autofocusType, BOOL& afFound)
{
	IDevice* pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if (NULL == pZStage)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AutoFocusAndRetry Execute could not create z stage");
		return FALSE;
	}

	afFound = FALSE;

	autofocusType->Execute(index, pAutoFocusDevice, afFound);

	/*	This might be used later for tiling keep the logic for now
	double val;

	pZStage->GetParam(IDevice::PARAM_Z_POS, val);

	const long RETRIES = 1;
	const double RESULT_LOCATION_THREASHOLD_MM = .10;//must be within 50um of the previous location
	const long SIG_DIGITS_MULTIPLIER = 1000;
	long count = 0;
	double lower = 0;
	double upper = 0;

	do
	{
		lower = _lastGoodFocusPosition - RESULT_LOCATION_THREASHOLD_MM;
		upper = _lastGoodFocusPosition + RESULT_LOCATION_THREASHOLD_MM;

		//check if the focus was found or if the result location is within the threshold
		//if not perform retires for the count of retries specified
		if ((val < lower) || (val > upper) || (FALSE == afFound))
		{

			StringCbPrintfW(msg, MSG_LENGTH, L"AFAndRetry z position val %d.%03d outside lower %d.%03d upper %d.%03d", (int)val, (int)((val - static_cast<long>(val)) * SIG_DIGITS_MULTIPLIER), (int)lower, (int)((lower - static_cast<long>(lower)) * SIG_DIGITS_MULTIPLIER), (int)upper, (int)((upper - static_cast<long>(upper)) * SIG_DIGITS_MULTIPLIER));
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, msg);

			autofocusType->Execute(index, pAutoFocusDevice, afFound);
			pZStage->GetParam(IDevice::PARAM_Z_POS, val);
		}
		else
		{
			StringCbPrintfW(msg, MSG_LENGTH, L"AFAndRetry passed z position %d.%03d", (int)val, (int)((val - static_cast<long>(val)) * SIG_DIGITS_MULTIPLIER));
			logDll->TLTraceEvent(VERBOSE_EVENT, 1, msg);
			break;
		}
		count++;
	} while (count <= RETRIES);

	if ((val < lower) || (val > upper) || (FALSE == afFound))
	{
		pZStage->SetParam(IDevice::PARAM_Z_POS, _lastGoodFocusPosition);

		pZStage->PreflightPosition();

		pZStage->SetupPosition();

		pZStage->StartPosition();

		//don't wait for the z to finish its motion will overlap with the next XY movement
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StatusZThreadProc2, pZStage, 0, &dwThread);

		const long MAX_Z_WAIT_TIME = 5000;

		DWORD dwWait = WaitForSingleObject(hEventZ, MAX_Z_WAIT_TIME);

		if (dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AcquireSingle Execute Z failed");
			//return FALSE;
		}

		CloseHandle(hThread);
		CloseHandle(hEventZ);
		pZStage->PostflightPosition();
	}
	*/

	return TRUE;
}

long IAutoFocus::SetAFStartZPosition(double afStartPos, BOOL bWait, BOOL afFound)
{
	IDevice* pZStage = NULL;

	pZStage = GetDevice(SelectedHardware::SELECTED_ZSTAGE);

	if (NULL == pZStage)
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AutoFocus Execute could not create z stage");
		return FALSE;
	}

	//if found use a relative offset from the current position
	if (afFound)
	{
		double pos;
		pZStage->GetParam(IDevice::PARAM_Z_POS_CURRENT, pos);

		_lastGoodFocusPosition = pos;

		pos -= _adaptiveOffset;

		pZStage->SetParam(IDevice::PARAM_Z_POS, pos);

		StringCbPrintfW(msg, MSG_LENGTH, L"SetAutoFocusStartZPosition new af start position %d.%d", (int)pos, (int)((pos - static_cast<long>(pos)) * 1000));
	}
	else
	{
		double pos = _lastGoodFocusPosition;

		//modify the last good focus position each pass to ensure its unique
		_lastGoodFocusPosition = _lastGoodFocusPosition - .001;

		pos -= _adaptiveOffset;

		pZStage->SetParam(IDevice::PARAM_Z_POS, pos);

		StringCbPrintfW(msg, MSG_LENGTH, L"SetAutoFocusStartZPosition new af start position %d.%d", (int)pos, (int)((pos - static_cast<long>(pos)) * 1000));
	}

	pZStage->PreflightPosition();

	pZStage->SetupPosition();

	pZStage->StartPosition();

	if (TRUE == bWait)
	{
		//don't wait for the z to finish its motion will overlap with the next XY movement
		hEventZ = CreateEvent(0, FALSE, FALSE, 0);

		DWORD dwThread;

		HANDLE hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StatusZThreadProc2, pZStage, 0, &dwThread);

		const long MAX_Z_WAIT_TIME = 5000;

		DWORD dwWait = WaitForSingleObject(hEventZ, MAX_Z_WAIT_TIME);

		if (dwWait != WAIT_OBJECT_0)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"AutoFocus Execute Z failed");
			//return FALSE;
		}

		CloseHandle(hThread);
		CloseHandle(hEventZ);
	}
	pZStage->PostflightPosition();

	return TRUE;
}

long IAutoFocus::GetAFStatusAndImage(char* imageBuffer, long &imageAvailable, long &afRunning, long& frameNumber, long& currentRepeat, long& status, long& zSteps, long& currentZIndex)
{
	int result = TRUE;
	try
	{
		if (_selectedAutoFocusType != IAutoFocus::AF_NONE)
		{
			switch (_selectedAutoFocusType)
			{
			case IAutoFocus::AF_HARDWARE://hardware only
			{
				//HW for AutoFocus not implemented yet. It won't require this, since everything will be executed in the auto focus hardware dll.
			}
			break;

			case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
			{
				long imageReady = _afHWImage.get()->GetImageBuffer(imageBuffer, frameNumber, currentRepeat, status, zSteps, currentZIndex);
				imageAvailable = imageReady;
				afRunning = _autoFocusRunning;
			}
			break;
			case IAutoFocus::AF_IMAGE://image
			{
				long imageReady = _afImage.get()->GetImageBuffer(imageBuffer, frameNumber, currentRepeat, status, zSteps, currentZIndex);
				imageAvailable = imageReady;
				afRunning = _autoFocusRunning;
			}
			break;
			}
		}
	}
	catch (std::exception&)
	{
		result = FALSE;
	}

	return result;
}

//Return 1 for auto focus running and 0 for not running
long IAutoFocus::GetAFRunning()
{
	return _autoFocusRunning;
}

long IAutoFocus::GetAFStatus(long& currentStatus, long& bestContrastScore, double& bestZPosition, double& nextZPosition, long& currentRepeatIndex)
{
	int result = TRUE;
	try
	{
		if (_selectedAutoFocusType != IAutoFocus::AF_NONE)
		{
			switch (_selectedAutoFocusType)
			{
			case IAutoFocus::AF_HARDWARE://hardware only
			{
				//HW for AutoFocus not implemented yet
			}
			break;

			case IAutoFocus::AF_HARDWARE_IMAGE://hardware + image
			{
				_afHWImage.get()->GetStatus(currentStatus, bestContrastScore, bestZPosition, nextZPosition, currentRepeatIndex);
			}
			break;
			case IAutoFocus::AF_IMAGE://image
			{
				_afImage.get()->GetStatus(currentStatus, bestContrastScore, bestZPosition, nextZPosition, currentRepeatIndex);
			}
			break;
			}
		}
	}
	catch (std::exception&)
	{
		result = FALSE;
	}

	return result;
}


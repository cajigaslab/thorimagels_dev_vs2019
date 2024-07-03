#include "stdafx.h"
#include "AcquireHyperspectral.h"
#include "AcquireFactory.h"
#include "RunSample.h"
#include <iostream>
#include <fstream>
#include <iostream>
#include <cmath>
#include <climits>

#define MSG_SIWlE 256
#define WL_MAX_COUNT 310

AcquireHyperspectral::AcquireHyperspectral(IExperiment* pExperiment,wstring path)
{
	_pExp = pExperiment;
	_zFrame = 1;
	_tFrame = 1;
	_path = path;
}

long AcquireHyperspectral::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireHyperspectral::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireHyperspectral::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireHyperspectral::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireHyperspectral::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireHyperspectral::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireHyperspectral::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;

}

long AcquireHyperspectral::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireHyperspectral::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireHyperspectral::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireHyperspectral::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireHyperspectral::CallInformMessage(wchar_t* message)
{
	InformMessage(message);
	return TRUE;
}

long AcquireHyperspectral::CallNotifySavedFileIPC(wchar_t* message)
{
	NotifySavedFileIPC(message);
	return TRUE;
}

long AcquireHyperspectral::CallAutoFocusStatus(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat)
{
	AutoFocusStatus(isRunning, bestScore, bestZPos, nextZPos, currRepeat);
	return TRUE;
}

long AcquireHyperspectral::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_tFrame = tFrame;
	_zFrame = zFrame;
	return Execute(index, subWell);
}

double GetCustomExposureValue(double wlStart, double wlStop, double wlPos, string path)
{	
	double result = 0;

	double percentOfWlRange = WL_MAX_COUNT * abs(wlPos - wlStart);
	if (wlStart != wlStop)
	{
		percentOfWlRange /= abs(wlStop - wlStart);
	}

	double lastWlPercent = -1;
	double lastExposure = 0;

	std::ifstream infile(path);

	double pos,exposure;
	char c;

	while((infile >> pos >> c >> exposure) && (c == ','))
	{
		//piecewise interpolate
		if((percentOfWlRange > lastWlPercent)&&(percentOfWlRange <= pos))
		{
			if(lastWlPercent < 0)
			{
				//first point do not need interpolation
				result = exposure;
			}
			else
			{
				double slope = (exposure - lastExposure)/(pos - lastWlPercent);

				result = lastExposure + slope * (percentOfWlRange-lastWlPercent);
			}
			break;
		}

		lastWlPercent = pos;
		lastExposure = exposure;
	}

	return result;
}

long AcquireHyperspectral::Execute(long index, long subWell)
{	
	if(NULL == GetDevice(SelectedHardware::SELECTED_SPECTRUMFILTER))
	{
		StringCbPrintfW(message,MSG_LENGTH,L"RunSample Execute Hyperspectral acquisition failed, there is no selected Spectrum Filter");
		logDll->TLTraceEvent(ERROR_EVENT,1,message);
		return FALSE;
	}
	long wavelengthStart, wavelengthStop, stepSize, bandwidthMode;
	string sequencePath;
	_pExp->GetSpectralFilter(wavelengthStart, wavelengthStop, stepSize, bandwidthMode, sequencePath);

	long steps = abs(wavelengthStop - wavelengthStart) / stepSize + 1;

	//set the Kurios control mode to MANUAL
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_CONTROLMODE, IDevice::KuriosControlMode::MANUAL, false);

	//delete any previous sequence
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_DELETESEQUENCE, FALSE, false);
	Sleep(10);
	//set the sequence step by step

	string* sequence = new string[steps];

	long sign = ((wavelengthStop - wavelengthStart) & 0x80000000) ? -1 : 1;
	long wlMin = (wavelengthStart < wavelengthStop) ? wavelengthStart : wavelengthStop;
	long wlMax = (wavelengthStop > wavelengthStart) ? wavelengthStop : wavelengthStart;

	//set sequence in kurios
	for (int i = 0; i < steps; i++)
	{
		long pos = min(wavelengthStart + sign * i * stepSize, wlMax);
		//round to 2 decimals
		float exp =  static_cast<float>((static_cast<long>(floor(GetCustomExposureValue(wavelengthStart, wavelengthStop, pos, sequencePath) * 100 + 0.5))) / 100.0);
		//long exp =  static_cast<long>(ceil(GetCustomExposureValue(wavelengthStart, wavelengthStop, pos, sequencePath)));
		sequence[i] = std::to_string(i+1) + ' ' + std::to_string(pos) + ' ' + std::to_string(exp) + ' ' + std::to_string(bandwidthMode);
		SetDeviceParamBuffer(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_SETSEQUENCE, (char*)sequence[i].c_str(), static_cast<long>(sequence[i].size()), false);
		Sleep(10);
	}


	//set the trigger signal mode to NON_FLIPPED
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_TRIGGEROUTSIGNALMODE, IDevice::KuriosTriggerOutSignalMode::NON_FLIPPED, false);
	Sleep(10);

	//set the trigger time mode to ENABLE_BULB
	//SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_TRIGGEROUTTIMEMODE, IDevice::KuriosTriggerOutTimeMode::ENABLE_BULB, false);
	//Sleep(10);

	//set the Kurios control mode to SEQUENCE_EXT
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_CONTROLMODE, IDevice::KuriosControlMode::SEQUENCE_EXT, false);
	Sleep(10);

	/********** FOR DEBUGGING **********/
	//std::ofstream outfile ("c:/Temp/sequence.txt");
	//for(int i = 0; i < steps ; ++i) 
	//{
	//	outfile << sequence[i] << endl; // I also tried replacing endl with a "\n"
	//}
	//outfile.close();

	//create a stream acquisition and execute it
	auto_ptr<IAcquire> acq(NULL);
	AcquireFactory factory;
	acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_STREAM,NULL,_pExp,_path));
	acq->Execute(index,subWell,_zFrame,_tFrame);

	//when the experiment is done, then set the curios back to MANUAL mode
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_CONTROLMODE, IDevice::KuriosControlMode::MANUAL, false);
	Sleep(10);
	//change the trigger out mode to DISABLE_BULB
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_TRIGGEROUTTIMEMODE, IDevice::KuriosTriggerOutTimeMode::DISABLE_BULB, false);

	// This sequence of changing the wavelength to start + 1 and then back 
	// to start is to force the trigger line on the kurios to go down.
	if(wavelengthStart <= wavelengthStop)
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_WAVELENGTH, wavelengthStart+1, false);
	}
	else
	{
		SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_WAVELENGTH, wavelengthStart-1, false);
	}	
	Sleep(100);
	SetDeviceParamLong(SelectedHardware::SELECTED_SPECTRUMFILTER, IDevice::PARAM_KURIOS_WAVELENGTH, wavelengthStart, false);
	Sleep(10);
	return TRUE;
}

long AcquireHyperspectral::ZStreamExecute(long index, long subWell, ICamera* pCamera, long zstageSteps, long timePoints, long undefinedVar)
{
	return FALSE;
}


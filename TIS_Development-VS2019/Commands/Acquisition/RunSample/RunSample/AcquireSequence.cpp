#include "stdafx.h"
#include "AcquireSequence.h"
#include "AcquireFactory.h"
#include "RunSample.h"

extern auto_ptr<CommandDll> shwDll;

AcquireSequence::AcquireSequence(IExperiment *pExperiment, wstring path)
{
	_pExp = pExperiment;
	_counter = 0;
	_zFrame = 1;
	_tFrame = 1;
	_path = path;
}

long AcquireSequence::CallCaptureComplete(long captureComplete)
{
	return TRUE;
}

long AcquireSequence::CallCaptureImage(long index)
{
	CaptureImage(index);
	return TRUE;
}

long AcquireSequence::CallSaveImage(long index, BOOL isImageUpdate)
{
	SaveImage(index, isImageUpdate);
	return TRUE;
}

long AcquireSequence::CallCaptureSubImage(long index)
{
	CaptureSubImage(index);
	return TRUE;
}

long AcquireSequence::CallSaveSubImage(long index)
{
	SaveSubImage(index);
	return TRUE;
}

long AcquireSequence::PreCaptureEventCheck(long &status)
{
	return TRUE;
}

long AcquireSequence::StopCaptureEventCheck(long &status)
{
	StopCapture(status);
	return TRUE;

}

long AcquireSequence::CallSaveZImage(long index, double power0, double power1, double power2, double power3,double power4, double power5)
{
	SaveZImage(index, power0, power1, power2, power3, power4, power5);
	return TRUE;
}

long AcquireSequence::CallSaveTImage(long index)
{
	SaveTImage(index);
	return TRUE;
}

long AcquireSequence::CallSequenceStepCurrent(long index)
{
	SequenceStepCurrent(index);
	return TRUE;
}

long AcquireSequence::CallStartProgressBar(long index, long resetTotalCount)
{
	StartProgressBar(index, resetTotalCount);
	return TRUE;
}

long AcquireSequence::CallInformMessage(wchar_t* message)
{
	InformMessage(message);
	return TRUE;
}

long AcquireSequence::CallNotifySavedFileIPC(wchar_t* message)
{
	NotifySavedFileIPC(message);
	return TRUE;
}

long AcquireSequence::CallAutoFocusStatus(long isRunning, long bestScore, double bestZPos, double nextZPos, long currRepeat)
{
	AutoFocusStatus(isRunning, bestScore, bestZPos, nextZPos, currRepeat);
	return TRUE;
}

long AcquireSequence::Execute(long index, long subWell, long zFrame, long tFrame)
{
	_tFrame = tFrame;
	_zFrame = zFrame;
	return Execute(index, subWell);
}

long AcquireSequence::Execute(long index, long subWell)
{	
	long channelOrderEnable = FALSE;
	_pExp->GetCaptureSequence(channelOrderEnable);

	//Get the Capture Sequence Settings
	vector<IExperiment::SequenceStep> channelOrder;
	_pExp->GetSequenceSteps(channelOrder);	

	//Get all the LSM Settings
	long areaMode,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles,polarity[4], verticalFlip, horizontalFlip;
	double areaAngle,dwellTime,crsFrequencyHz = 0;
	long timeBasedLineScan = FALSE;
	long timeBasedLineScanMS = 0;
	long threePhotonEnable = FALSE;
	long numberOfPlanes = 1;
	_pExp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLineScanMS, threePhotonEnable, numberOfPlanes);

	//Get all the PMT Settings
	long enableA,bandwidthA,enableB,bandwidthB,enableC,bandwidthC,enableD,bandwidthD;
	double gainA,offsetA,gainB,offsetB,gainC,offsetC,gainD,offsetD;
	_pExp->GetPMT(enableA,gainA,bandwidthA,offsetA,enableB,gainB,bandwidthB,offsetB,enableC,gainC,bandwidthC,offsetC,enableD,gainD,bandwidthD,offsetD);

	//Get all the Multiphoton Settings
	long multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2;

	_pExp->GetMultiPhotonLaser(multiphotonEnable,multiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2);

	auto_ptr<IAcquire> acq(NULL);

	long captureMode;
	_pExp->GetCaptureMode(captureMode);

	string zstageName;
	long zStreamFrames,zStreamMode,zstageSteps,zEnable;
	double zstageStepSize, zStartPos;
	_pExp->GetZStage(zstageName,zEnable,zstageSteps,zstageStepSize,zStartPos,zStreamFrames,zStreamMode);

	double intervalSec;
	long timePoints,triggerModeTimelapse;
	_pExp->GetTimelapse(timePoints,intervalSec,triggerModeTimelapse);

	double activeCamera;

	GetCameraParamDouble(SelectedHardware::SELECTED_CAMERA1,ICamera::PARAM_CAMERA_TYPE,activeCamera);

	if (0 < channelOrder.size() && TRUE == channelOrderEnable)
	{	
		for (long i = 0; i < channelOrder.size(); i++)
		{
			//replace the capture settings with the settings found in the channel Step Settings.
			//Notice that the channel Step settings don't replace every single setting on some of the devices (i.e. it only replaces the channel setting on LSM)
			_pExp->SetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channelOrder[i].LSMChannel,fieldSize,offsetX,offsetY,averageMode,averageNum,clockSource,inputRange1,inputRange2,twoWayAlignment,extClockRate,dwellTime,flybackCycles,inputRange3,inputRange4,minimizeFlybackCycles, polarity[0],polarity[1],polarity[2],polarity[3], verticalFlip, horizontalFlip);
			_pExp->SetPMT(channelOrder[i].PMT1Enable,channelOrder[i].PMT1Gain,bandwidthA,offsetA,channelOrder[i].PMT2Enable,channelOrder[i].PMT2Gain,bandwidthB,offsetB,channelOrder[i].PMT3Enable,channelOrder[i].PMT3Gain,bandwidthC,offsetC,channelOrder[i].PMT4Enable,channelOrder[i].PMT4Gain,bandwidthD,offsetD);
			_pExp->SetMCLS(channelOrder[i].MCLSEnable1,channelOrder[i].MCLSPower1,channelOrder[i].MCLSEnable2,channelOrder[i].MCLSPower2,channelOrder[i].MCLSEnable3,channelOrder[i].MCLSPower3,channelOrder[i].MCLSEnable4,channelOrder[i].MCLSPower4, channelOrder[i].LaserTTL, channelOrder[i].LaserAnalog, channelOrder[i].Wavelength1, channelOrder[i].Wavelength2, channelOrder[i].Wavelength3, channelOrder[i].Wavelength4);
			_pExp->SetMultiPhotonLaser(multiphotonEnable,channelOrder[i].MultiphotonPos,multiphotonSeqEnable,multiphotonSeqPos1,multiphotonSeqPos2);
			_pExp->SetPinholeWheel(channelOrder[i].PinholePos);
			_pExp->SetLightPath(channelOrder[i].LightPathGGEnable, channelOrder[i].LightPathGREnable, channelOrder[i].LightPathCamEnable, channelOrder[i].InvertedLightPathPosition, channelOrder[i].LightPathNDDPosition);

			//continue in software mode after trigger first:
			if((1 == triggerModeTimelapse) && (0 < i))
			{
				triggerModeTimelapse = 0;
				_pExp->SetTimelapse(timePoints, intervalSec, triggerModeTimelapse);
			}

			//before adding the wavelengths for the channel Step, first remove all of them
			//then only add the wavelengths that are in the current channel Step
			_pExp->RemoveAllWavelengths();
			for (long j = 0; j < channelOrder[i].Wavelength.size(); j++)
			{
				_pExp->AddWavelength(channelOrder[i].Wavelength[j].name, channelOrder[i].Wavelength[j].exposureTimeMS);
			}

			//Retrieve the pockels settings from the settings file
			//and replace with the settings found in the channel step
			//do this for all three pockels
			long type, blankPercent;
			double start,stop;
			string path;
			for (long j = 0; j < channelOrder[i].Pockels.size(); j++)
			{
				_pExp->GetPockels(j, type, start, stop, path, blankPercent);
				_pExp->SetPockels(j, channelOrder[i].Pockels[j].type, channelOrder[i].Pockels[j].start, channelOrder[i].Pockels[j].stop, channelOrder[i].Pockels[j].path, blankPercent);
			}

			//Set the acquire mode
			SetAcquire(captureMode, timePoints, zStreamMode, zstageSteps, static_cast<long>(activeCamera), static_cast<long>(channelOrder[i].Wavelength.size()), acq);

			//perform pre capture protocols
			PreCaptureProtocol(_pExp);

			//Let the observer know the current index channel Step
			CallSequenceStepCurrent(i);

			//Execute the Acquisition
			if(acq->Execute(index,subWell,_zFrame,_tFrame) != TRUE)
			{
				logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireSequence Acquire execute failed");
				return FALSE;
			}	
		}
	}
	else
	{
		long wavelengths = _pExp->GetNumberOfWavelengths();	
		SetAcquire(captureMode, timePoints, zStreamMode, zstageSteps, static_cast<long>(activeCamera), wavelengths, acq);
		//perform pre capture protocols
		PreCaptureProtocol(_pExp);

		//Execute the Acquisition
		if(acq->Execute(index,subWell,_zFrame,_tFrame) != TRUE)
		{
			logDll->TLTraceEvent(INFORMATION_EVENT,1,L"AcquireSequence Acquire execute failed");
			return FALSE;
		}	
	}
	return TRUE;
}

void AcquireSequence::SetAcquire(long captureMode, size_t timePoints, long zStreamMode, long zStageSteps, long activeCamera, long nWavelengths, auto_ptr<IAcquire> &acq)
{
	AcquireFactory factory;

	switch(captureMode)
	{
	case IExperiment::STREAMING:
		{
			acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_STREAM,NULL,_pExp,_path));
		}
		break;
	case IExperiment::BLEACHING:
		{	
			acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_BLEACHING,NULL,_pExp,_path));
		}
		break;
	case IExperiment::HYPERSPECTRAL:
		{
			acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_HYPERSPECTRAL,NULL,_pExp,_path));
		}
		break;
	case IExperiment::Z_AND_T:
	default:
		{
		//:TODO: This will always go onto the first if statement because timepoints is always at least 1. This means
		//AcquireZStack is not used at all.
			if((timePoints >= 1)||(1 == zStreamMode))
			{
				acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_SERIES,NULL,_pExp,_path));
			}
			else if(zStageSteps > 1)
			{
				acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_Z_STACK,NULL,_pExp,_path));
			}
			else if(nWavelengths > 1)
			{
				if(ICamera::CCD == activeCamera)
				{
					acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_MULTI_WAVELENGTH,NULL,_pExp,_path));
				}
				else
				{
					acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_SERIES,NULL,_pExp,_path));
				}
			}
			else
			{		
				acq.reset(factory.getAcquireInstance(AcquireFactory::ACQ_T_SERIES,NULL,_pExp,_path));
			}
		}
		break;
	}
}

#include "stdafx.h"
#include "AutoExposureImpl.h"

using namespace std;

unique_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
mutex AutoExposure::_initLock;
AutoExposure* AutoExposure::_singleton = nullptr;

Dimensions AutoExposure::GetImageDimensions(ICamera* camera)
{
	// grab parameters from camera

	double left, right, top, bottom, binX, binY, val, width, height;
	long cameraType = ICamera::CameraType::LSM, numberOfChannels = 1, channel = 1;

	camera->GetParam(ICamera::PARAM_CAMERA_TYPE, val);
	cameraType = static_cast<long>(val);

	if (ICamera::CameraType::CCD == cameraType)
	{
		camera->GetParam(ICamera::PARAM_CAPTURE_REGION_LEFT, left);
		camera->GetParam(ICamera::PARAM_CAPTURE_REGION_RIGHT, right);
		camera->GetParam(ICamera::PARAM_CAPTURE_REGION_TOP, top);
		camera->GetParam(ICamera::PARAM_CAPTURE_REGION_BOTTOM, bottom);
		camera->GetParam(ICamera::PARAM_BINNING_X, binX);
		camera->GetParam(ICamera::PARAM_BINNING_Y, binY);
		camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_WIDTH, width);
		camera->GetParam(ICamera::PARAM_CAMERA_IMAGE_HEIGHT, height);
		// TODO: possible to remake image when it comes in based on image info...?
		double colorImageType;
		camera->GetParam(ICamera::PARAM_CAMERA_COLOR_IMAGE_TYPE, colorImageType);
		double cameraSensorType;
		camera->GetParam(ICamera::PARAM_CAMERA_SENSOR_TYPE, cameraSensorType);
		if (colorImageType != 0 && cameraSensorType != 0)
		{
			numberOfChannels = 3;
		}
		else
		{
			numberOfChannels = 1;
		}
	}
	else
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"Unsupported CameraType requested of Autoexposure");
		return Dimensions();
	}

	Dimensions d;
	d.c = numberOfChannels;
	d.dType = INT_16BIT;
	d.imageBufferType = 0;
	d.m = 1;
	d.mType = CONTIGUOUS_CHANNEL;//DETACHED_CHANNEL;
	d.t = 1;
	d.x = (long)width;
	d.y = (long)height;
	d.z = 1;

	return d;
}

bool AutoExposure::CaptureSnapshot(ICamera* camera, char* buffer, FrameInfo &frameInfo)
{

	if (FALSE == camera->PreflightAcquisition(buffer))
	{
		return false;
	}

	if (FALSE == camera->SetupAcquisition(buffer))
	{
		return false;
	}

	if (FALSE == camera->StartAcquisition(buffer))
	{
		return false;
	}

	while (FALSE == camera->CopyAcquisition(buffer, &frameInfo))
	{
		this_thread::sleep_for(chrono::milliseconds(10));
		if (_isStopFlag)
		{
			return false;
		}
	}

	if (FALSE == camera->PostflightAcquisition(NULL))
	{
		return false;
	}

	frameInfo.fullFrame = 1; // TODO: is this a bool or not? some places like ImageViewMVM.Models.ImageViewMBase::Create24BitPixelDataByteRawAndComposite() are checking for == 1.

	return true;
}

void AutoExposure::FillHistogram(unsigned short* imageData, int bitDepth, const FrameInfo &frameInfo, vector<int>& histogram)
{
	int numBins = (1 << bitDepth);

	if (histogram.size() != numBins)
	{
		histogram.resize(numBins);
	}

	memset(histogram.data(), 0, numBins * sizeof(int)); // clear histogram

	// TODO: quick and dirty histogram, could make one quicker with IPP
	// TODO: multiple plane ?
	// assuming Planar pixel layout (each channel is stored contiguously in full, rather than interlaced)
	int totalNumPixels = frameInfo.imageWidth * frameInfo.imageHeight * frameInfo.channels;
	const int oneChannelSize = frameInfo.imageWidth * frameInfo.imageHeight;

	for (int row = 0; row < frameInfo.imageHeight; ++row)
	{
		int rowOffset = row * frameInfo.imageWidth;
		for (int col = 0; col < frameInfo.imageWidth; ++col)
		{
			double tempValue = 0.0;
			for (int chan = 0; chan < frameInfo.channels; ++chan)
			{
				tempValue += imageData[(oneChannelSize * chan) + (rowOffset)+col];
			}
			tempValue /= frameInfo.channels;
			++histogram[(unsigned short)tempValue];
		}
	}
}

AutoExposure* AutoExposure::GetInstance()
{
	lock_guard<mutex> guard(_initLock);

	if (!_singleton)
	{
		_singleton = new AutoExposure();
	}

	return _singleton;
}

void AutoExposure::Start(ICamera *camera)
{
	_isStopFlag = false;

	if (_isRunning)
	{
		return;
	}

	if (nullptr == camera)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"Null camera handle was sent to Autoexposure module");
		return;
	}

	_isRunning = true;
	_isStable = false;

	// started running -> post update
	double startingExposure = 0.0;
	double startingGain = 0.0;
	camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, startingExposure);
	camera->GetParam(ICamera::PARAM_GAIN, startingGain);
	for (auto& callback : _updateCallbacks)
	{
		callback(_isStable, _isRunning, startingExposure, startingGain);
	}

	double oldTrigMode;
	camera->GetParam(ICamera::PARAM_TRIGGER_MODE, oldTrigMode);
	camera->SetParam(ICamera::PARAM_TRIGGER_MODE, ICamera::SW_SINGLE_FRAME);

	try
	{
		Dimensions imageDimensions = GetImageDimensions(camera);
		
		// fill out params
		AutoExposureParams autoExpParams;

		double bitDepth;
		camera->GetParam(ICamera::PARAM_BITS_PER_PIXEL, bitDepth);
		autoExpParams.BitDepth = static_cast<int>(bitDepth);
		
		// exposure
		long expParamType;
		long expParamAvailable; 
		long expParamReadOnly;
		double expParamMin;
		double expParamMax;
		double expParamDefault;
		camera->GetParamInfo(ICamera::PARAM_EXPOSURE_TIME_MS, expParamType, expParamAvailable, expParamReadOnly, expParamMin, expParamMax, expParamDefault);
		autoExpParams.ExposureTimeMax_ms = expParamMax;
		autoExpParams.ExposureTimeMin_ms = expParamMin;
	
		// gain
		long gainParamType;
		long gainParamAvailable;
		long gainParamReadOnly;
		double gainParamMin;
		double gainParamMax;
		double gainParamDefault;
		camera->GetParamInfo(ICamera::PARAM_GAIN, gainParamType, gainParamAvailable, gainParamReadOnly, gainParamMin, gainParamMax, gainParamDefault);
		autoExpParams.GainMax_dB = gainParamMax;
		autoExpParams.GainMin_dB = gainParamMin;
		autoExpParams.IsGainAdjustable = gainParamAvailable;

		long imageId;
		if (ImageManager::getInstance()->CreateImage(imageId, imageDimensions) == FALSE)
		{
			throw runtime_error("Autoexposure module could not create memory buffer");
		}
		char* imageBuffer = ImageManager::getInstance()->GetImagePtr(imageId, 0, 0, 0, 0);
		_currentImageBuffer = imageBuffer;
		_currentImageFrameNumber = -1;

		vector<int> histogram(1 << static_cast<int>(bitDepth));
		auto waitUntilTime = chrono::system_clock::now();



		// AE acquisition loop
		while (!_isStopFlag)
		{
			// wait for timer
			while (chrono::system_clock::now() < waitUntilTime)
			{
				this_thread::sleep_for(chrono::milliseconds(10));
			}

			// read the current exposure / gain
			camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, autoExpParams.ExposureTime_ms);
			if (autoExpParams.IsGainAdjustable)
			{
				camera->GetParam(ICamera::PARAM_GAIN, autoExpParams.Gain_dB);
			}

			// get an image
			if (false == CaptureSnapshot(camera, imageBuffer, _currentFrameInfo))
			{
				continue;
			}
			else
			{
				_currentImageBufferSize = _currentFrameInfo.imageHeight * _currentFrameInfo.imageWidth * _currentFrameInfo.channels * sizeof(unsigned short);
				++_currentImageFrameNumber;
			}
			autoExpParams.ImageHeight = _currentFrameInfo.imageHeight;
			autoExpParams.ImageWidth = _currentFrameInfo.imageWidth;

			autoExpParams.TargetPercent = _targetPercent;

			// get histogram
			FillHistogram((unsigned short*)imageBuffer, static_cast<int>(bitDepth), _currentFrameInfo, histogram);

			// run the AE algorithm on it
			AutoExposureResults results = AutoExposure::GetAutoExposure(histogram, autoExpParams);

			//set new exposure
			static const int maxExposureForAutoExposure_ms = 3000; // this is the limit we will allow AE to push exposure
			double targetExposure_ms = min(results.TargetExposureTime_ms, maxExposureForAutoExposure_ms);

			bool isExposureTimeSettled = abs(targetExposure_ms - autoExpParams.ExposureTime_ms) <= 0.01;
			bool isGainSettled = !autoExpParams.IsGainAdjustable || abs(results.TargetGain_dB - autoExpParams.Gain_dB) <= 0.01;

			if (!isExposureTimeSettled)
			{
				long result = camera->SetParam(ICamera::PARAM_EXPOSURE_TIME_MS, results.TargetExposureTime_ms);
				if (FALSE == result)
				{
					logDll->TLTraceEvent(ERROR_EVENT, 1, L"Autoexposure was unable to set Exposure time");
				}
			}
			if (!isGainSettled)
			{
				long result = camera->SetParam(ICamera::PARAM_GAIN, results.TargetGain_dB);
				if (FALSE == result)
				{
					logDll->TLTraceEvent(ERROR_EVENT, 1, L"Autoexposure was unable to set Gain");
				}
			}

			bool isStable = isExposureTimeSettled && isGainSettled;

			if (!isStable || isStable != _isStable)
			{
				// stability changed -> post update
				_isStable = isStable;

				for (auto& callback : _updateCallbacks)
				{
					callback(_isStable, _isRunning, results.TargetExposureTime_ms, results.TargetGain_dB);
				}
			}

			double frameTime_ms = 0.0;
			camera->GetParam(ICamera::PARAM_CAMERA_FRAME_TIME, frameTime_ms);

			int margin_us = 20000;
			waitUntilTime = chrono::system_clock::now() + chrono::microseconds((int) (frameTime_ms * 1000) + margin_us);
		}
	}
	catch (exception e)
	{
		const char* message = e.what();
		int len = (int)strlen(message);
		std::vector<wchar_t> wMessage(len);
		swprintf(wMessage.data(), len, L"%hs", message);
		logDll->TLTraceEvent(ERROR_EVENT, 1, wMessage.data());
	}
	_isRunning = false;
	camera->SetParam(ICamera::PARAM_TRIGGER_MODE, oldTrigMode);

	// stopped running -> post update
	double finalExposure = 0.0;
	double finalGain = 0.0;
	camera->GetParam(ICamera::PARAM_EXPOSURE_TIME_MS, finalExposure);
	camera->GetParam(ICamera::PARAM_GAIN, finalGain);
	for (auto& callback : _updateCallbacks)
	{
		callback(_isStable, _isRunning, finalExposure, finalGain);
	}
}

void AutoExposure::Stop()
{
	_isStopFlag = true;
}

bool AutoExposure::IsStable()
{
	return _isStable;
}

bool AutoExposure::IsRunning()
{
	return _isRunning;
}

bool AutoExposure::GetAutoExposureImage(char* imageBuffer, FrameInfoStruct &frameInfo, long imageBufferSize, long& frameNumber)
{
	if (false == _isRunning)
	{
		return false;
	}

	if (nullptr == imageBuffer)
	{
		return false;
	}

	if (nullptr == _currentImageBuffer)
	{
		return false;
	}

	if (-1 == _currentImageFrameNumber)
	{
		return false;
	}

	memcpy(imageBuffer, _currentImageBuffer, min(imageBufferSize, _currentImageBufferSize));
	frameNumber = _currentImageFrameNumber;
	frameInfo = _currentFrameInfo;
	return true;
}

void AutoExposure::RegisterUpdateCallback(UPDATE_CALLBACK callback)
{
	for (auto itr = _updateCallbacks.cbegin(); itr != _updateCallbacks.cend(); ++itr)
	{
		if (*itr == callback)
		{
			// don't allow duplicate callbacks
			return;
		}
	}
	_updateCallbacks.push_back(callback);
}

void AutoExposure::UnregisterUpdateCallback(UPDATE_CALLBACK callback)
{
	for (auto itr = _updateCallbacks.cbegin(); itr != _updateCallbacks.cend(); ++itr)
	{
		if (*itr == callback)
		{
			_updateCallbacks.erase(itr);
			return;
		}
	}
}

void AutoExposure::SetTargetPercent(double percent)
{
	_targetPercent = min(1.0, max(0.0, percent));
}

double AutoExposure::GetTargetPercent()
{
	return _targetPercent;
}

AutoExposureResults AutoExposure::GetAutoExposure(const std::vector<int>& histogram, const AutoExposureParams &autoExposureParams)
{
	// todo: There is an issue where a current exposure of 0.000 will always return a target exposure of 0.000
	auto exposureTime_ms = max(autoExposureParams.ExposureTime_ms, 0.001);
	auto maximumIntensityBasedOnBitDepth = (1 << autoExposureParams.BitDepth) - 1;

	if (histogram.size() < maximumIntensityBasedOnBitDepth)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"Autoexposure algorithm was sent a Histogram with too few bin values for the given bit depth");
	}

	// The target exposure specified by ming is 20% below max, presumably because it was assumed that some population above the 90th percentile would be at or near saturation.
	//auto targetIntensity = (int)(0.8 * maximumIntensityBasedOnBitDepth); // WHERE DO WE WANT TO PUSH THOSE VALUES? LET'S SAY 1033 VALUE IS WHERE WE HAVE 96% OF PIXELS BELOW IT.
	auto targetIntensity = (int)(autoExposureParams.TargetPercent * maximumIntensityBasedOnBitDepth); 

	//auto totalNumberOfPixels = autoExposureParams.RoiWidth_pixels * autoExposureParams.RoiHeight_pixels; //roiWidth_pixels * roiHeight_pixels; // TODO: channels
	auto totalNumberOfPixels = autoExposureParams.ImageWidth * autoExposureParams.ImageHeight; //roiWidth_pixels * roiHeight_pixels; // TODO: channels

	// TODO: assuming histogram is averaged, in the case of multi-channel data

	// Determine if the image is heavily saturated by looking only at the upper Nth percentile, where N is defined by targetSaturationFactor
	int numberOfSaturatedPixels = 0;
	// Define the effective saturation level, slightly less than max.
	int targetSaturationIntensity = (int)(maximumIntensityBasedOnBitDepth * 0.98);

	// start accumulating the histogram values starting from the maximum intensity and decrementing to the target saturation intensity MP
	for (int intensity = maximumIntensityBasedOnBitDepth; intensity >= targetSaturationIntensity; intensity--)
	{
		numberOfSaturatedPixels += histogram[intensity];
	}

	// TODO: rename: allowableNumSaturatedPixels
	auto numberOfPixelsThatCanBeSaturated = 0.25 * totalNumberOfPixels;

	// Special case to speed up the algorithm:
	// If the number of saturated pixels exceeds the allowed
	// fraction of the total, there are too many. So set exposure
	// to 100ms and gain to minimum.
	if (numberOfSaturatedPixels > numberOfPixelsThatCanBeSaturated)
	{
		AutoExposureResults results;
		const double scaleFactorForQuickExposureJump = 0.2;//0.05;
		results.TargetExposureTime_ms = autoExposureParams.ExposureTime_ms * scaleFactorForQuickExposureJump;
		results.TargetGain_dB = autoExposureParams.GainMin_dB;
		results.NumSaturatedPixels = numberOfSaturatedPixels;
		return results;
		//autoExposureResultParameters.isQuickExposureJump = true;
	}

	//else there are not too many saturated pixels, and we can
	//proceed with creating the cumulative histogram, starting
	//from the minimum value find the index at which the cumulative
	//number of pixels equals the desired percentile of the total. That
	//value will be considered the current intensity.
	//initialize the variable that will be used to hold the current intensity of the Nth percentile of all pixels
	int calculatedThresholdIntensity = 0;

	//the image intensity statistic to be used to establish aims is a percentile. The intensity of the Nth percentile is considered the image intensity. MP
	// Determines how far up the histogram we want to go before we stop, allowing us to throw away the upper values. It is the point at which the cumulative histogram contains 96% of the pixels. This avoids outliers/hot pixels. In other words, at what point do I have enough pixels to represent the overall image "intensity."
	int numberOfPixelsUpToOutliersThreshold = (int) (0.96 * totalNumberOfPixels);

	int numberOfPixelsSoFar = 0;
	for (int intensity = 0; intensity < histogram.size(); intensity++)
	{
		// TODO: summed channels messes this up
		numberOfPixelsSoFar += histogram[intensity];
		if (numberOfPixelsSoFar > numberOfPixelsUpToOutliersThreshold)
		{
			calculatedThresholdIntensity = intensity;
			break;
		}
	}
	
	// If the image is almost black, let's not guess what to do.
	if (calculatedThresholdIntensity == 0)
	{
		AutoExposureResults results;
		results.TargetExposureTime_ms = autoExposureParams.ExposureTime_ms;
		results.TargetGain_dB = autoExposureParams.Gain_dB;
		results.NumSaturatedPixels = numberOfSaturatedPixels;
		return results;
		//autoExposureResultParameters.isImageNearlyBlack = true;
	}

	// Multiplier for scaling from the calculated intensity to the desired
	double intensityRatioTargetToCalculated = targetIntensity / (double)calculatedThresholdIntensity;
	double intensityTolerance = targetIntensity * 0.02;

	if (abs(targetIntensity - calculatedThresholdIntensity) <= intensityTolerance)
	{
		AutoExposureResults results;
		results.TargetExposureTime_ms = autoExposureParams.ExposureTime_ms;
		results.TargetGain_dB = autoExposureParams.Gain_dB;
		results.NumSaturatedPixels = numberOfSaturatedPixels;
		return results;
		// autoExposureResultParameters.isIntensityBelowTolerance = true;
	}

	//finds the difference, since these values are related to dB, and difference in log space is a ratio in linear space, which is to be computed
	double currentGainOffsetFromMinimum_dB = autoExposureParams.Gain_dB - autoExposureParams.GainMin_dB;

	// Convert dB to linear space so that gain can be compared to exposure in the effect on the image intensity.
	double currentGainRatio = pow(10, currentGainOffsetFromMinimum_dB / 20);
	// autoExposureResultParameters.currentGainRatio = currentGainRatio;
                
	double minimumExposureTime_ms_notZero = max(autoExposureParams.ExposureTimeMin_ms, 0.001);

	// since we'll be extrapolating exposures and gains we must account for their offsets (the minimum)
	//MP: next calculate the exposure ratio
	double currentExposureTimeRatio = exposureTime_ms / minimumExposureTime_ms_notZero;

	double totalTargetRatio = autoExposureParams.IsGainAdjustable ? intensityRatioTargetToCalculated * currentGainRatio * currentExposureTimeRatio : intensityRatioTargetToCalculated * currentExposureTimeRatio;

	double maxAvailableExposureTimeRatio = autoExposureParams.ExposureTimeMax_ms / minimumExposureTime_ms_notZero;

	double targetExposureTime_ms;
	double targetGain_dB;

	// Priorities: minimize gain -> use exposure time to compensate for image intensity until it reaches maximum allowed by the algorithm (not necessarily the maximum possible)
	if (totalTargetRatio < maxAvailableExposureTimeRatio)
	{
		targetGain_dB = autoExposureParams.GainMin_dB;
		// This multiplies back out the minimum exposure time that was used in the ratio.
		targetExposureTime_ms = minimumExposureTime_ms_notZero * totalTargetRatio;
	}
	else
	{
		targetExposureTime_ms = autoExposureParams.ExposureTimeMax_ms;
		double finalRatio = totalTargetRatio / maxAvailableExposureTimeRatio;
		targetGain_dB = 20.0 * log10(finalRatio);
	}

	return AutoExposureResults(targetExposureTime_ms, targetGain_dB, numberOfSaturatedPixels);
}

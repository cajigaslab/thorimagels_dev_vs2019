#include "stdafx.h"
#include "thordaqResonantGalvo.h"

ThorDAQZWaveformParams CThordaqResonantGalvo::_fastZWaveformParams = ThorDAQZWaveformParams();
long CThordaqResonantGalvo::_useBuiltZWaveform = FALSE;
std::unique_ptr<ImageDistortionCorrectionDll> ImageDistortionCorrection(new ImageDistortionCorrectionDll(L".\\ThorImageProcess.dll"));
/************************************************************************************************
* @fn	long CThordaqResonantGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
*
* @brief	Configure thordaq settings
* @param [in,out]	pImgAcqPty	  	Identifier of Image Acquisition Struct.
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
{
	int32 error = 0, retVal = 0;
	long ret = TRUE;

	//only sleep if in preview mode where settings can quickly change
	if (pImgAcqPty->triggerMode == ICamera::SW_FREE_RUN_MODE)
	{
		//TODO: need to remove this
		Sleep(200); //Time added to help freq stablize when rebuilding waveforms.
	}
	//Initiate the Struct
	pImgAcqPty->rx = 0;
	pImgAcqPty->ry = 0;
	for (auto& channel : _daqAcqCfg.dacCtrl)
	{
		SAFE_DELETE_ARRAY(channel.second.waveformBuffer);
	}

	for (auto& channel : _daqAcqCfg.dacCtrl2)
	{
		SAFE_DELETE_ARRAY(channel.second.waveformBuffer);
	}

	_daqAcqCfg = IMAGING_CONFIGURATION_STRUCT();

	if (_current_resonant_scanner_frequency == 0)
	{
		_current_resonant_scanner_frequency = _crsFrequencyHighPrecision;
	}

	// set the resonant scanner frequency
	_daqAcqCfg.imageCtrl.clockRate = DEFAULT_INTERNALCLOCKRATE;
	if (pImgAcqPty->clockSource == INTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::INTERNAL_80MHZ_REF;
		//set the GR clock rate, it will basically tell the board what frequency to expect before we read it. Keep both functions together.
		//ThordaqErrChk (L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex,CLOCK_SOURCE::INTERNAL_80MHZ_REF));
		ThordaqErrChk(L"ThorDAQAPISetGRClockRate", retVal = ThorDAQAPISetGRClockRate(_DAQDeviceIndex, DEFAULT_INTERNALCLOCKRATE, static_cast<ULONG32>(_crsFrequencyHighPrecision)));
	}
	else
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE;
		_daqAcqCfg.imageCtrl.clockRate = pImgAcqPty->clockRateExternal; // need test
	}

	//sleep to give the FPGA enough time to calculate the freq
	Sleep(5);

	_current_resonant_scanner_frequency = _crsFrequencyHighPrecision;

	ThordaqErrChk(L"ThorDAQAPIGetLineTriggerFrequency", retVal = ThorDAQAPIGetLineTriggerFrequency(_DAQDeviceIndex, DEFAULT_INTERNALCLOCKRATE, _current_resonant_scanner_frequency, static_cast<ULONG32>(_crsFrequencyHighPrecision)));
	if (retVal != STATUS_SUCCESSFUL)
	{
		return FALSE;
	}
	if (_saveCrsFrequencyToLog)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ConfigAcqSettings->GetLineTriggerFrequency Samplerate: %ld ,Frequency read: %f, expected frequence: %f", DEFAULT_INTERNALCLOCKRATE, _current_resonant_scanner_frequency, _crsFrequencyHighPrecision);
		CThordaqResonantGalvo::LogMessage(errMsg, ERROR_EVENT);
	}

	//  set the resonant scanner frequency using the one read from the device
	_daqAcqCfg.imageCtrl.clockRate = DEFAULT_INTERNALCLOCKRATE;
	if (pImgAcqPty->clockSource == INTERNAL_CLOCK)
	{
		//set the GR clock rate, it will basically tell the board what frequency to expect before we read it. Keep both functions together.
		//ThordaqErrChk (L"ThorDAQAPISetClockSource", retVal = ThorDAQAPISetClockSource(_DAQDeviceIndex,CLOCK_SOURCE::INTERNAL_80MHZ_REF));
		ThordaqErrChk(L"ThorDAQAPISetGRClockRate", retVal = ThorDAQAPISetGRClockRate(_DAQDeviceIndex, DEFAULT_INTERNALCLOCKRATE, static_cast<ULONG32>(_current_resonant_scanner_frequency)));
	}

	//detemine the X& Y FOV in unit of volt, full swing of waveformBuffer,
	//based on field size and the image pixel aspect ratio
	// voltage required is happend to be the mechanical angle of the mirror 
	double theta = (double)pImgAcqPty->fieldSize * _field2Theta;
	GalvoStruct _galvo_control = GalvoStruct();
	_galvo_control.amplitude = theta * (double)pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle
	if (TRUE == _useZoomArray)
	{
		_galvo_control.amplitude = _galvo_control.amplitude + _galvo_control.amplitude * _zoomArray[pImgAcqPty->fieldSize] / 100.0;
	}
	_galvo_control.amplitude = (pImgAcqPty->yAmplitudeScaler / 100.0) * _galvo_control.amplitude;

	if (FALSE == pImgAcqPty->galvoEnable) //if LineScan is enabled set the amplitude to 0
	{
		_galvo_control.amplitude = 0;
	}
	_galvo_control.offset = (double)pImgAcqPty->offsetY * _field2Theta / 2;
	_galvo_control.park = (double)GALVO_PARK_POSITION;
	_galvo_control.scan_direction = pImgAcqPty->verticalScanDirection == 0 ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;

	// detemine size of image frame
	_scan_info = ScanStruct();
	_scan_info.forward_lines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (pImgAcqPty->pixelY / 2) : pImgAcqPty->pixelY;
	_scan_info.backward_lines = pImgAcqPty->flybackCycle;
	_scan_info.overall_lines = _scan_info.forward_lines + _scan_info.backward_lines;


	//TODO ThorDAQ 2.0: still need to work on internal digital trigger and level trigger modes
	THORDAQ_HW_TRIGGER_MODES triggerMode1 = (_digitalIOSelection[DI_ImagingHardwareTrigger1] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	THORDAQ_HW_TRIGGER_MODES triggerMode2 = (_digitalIOSelection[DI_ImagingHardwareTrigger2] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	//TODO ThorDAQ 2.0: the internal digital trigger works like a level trigger. For this to work with Stim, we would need to park the complete line high until imaging is complete (when N frames are acquired).

	_daqAcqCfg.triggerSettings.hwTrigger1Selection = THORDAQ_TRIGGER_INPUTS::THORDAQ_HW_TRIGGER_1;
	_daqAcqCfg.triggerSettings.hwTrigger2Selection = THORDAQ_TRIGGER_INPUTS::THORDAQ_HW_TRIGGER_2;
	_daqAcqCfg.triggerSettings.hwTrigger1Mode = triggerMode1;
	_daqAcqCfg.triggerSettings.hwTrigger2Mode = triggerMode2;
	_daqAcqCfg.triggerSettings.enableInternalDigitalTrigger = false;
	_daqAcqCfg.triggerSettings.logicOperand = THORDAQ_OR;
	switch (pImgAcqPty->triggerMode)
	{
	case ICamera::SW_FREE_RUN_MODE:
		_daqAcqCfg.imageCtrl.frameCnt = MAX_FRAME_NUM;
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_EACH:
		_daqAcqCfg.imageCtrl.frameCnt = MAX_FRAME_NUM;
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
		break;
	case ICamera::SW_SINGLE_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = 1;
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
		break;
	case ICamera::HW_SINGLE_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = 1;
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
		break;
	case ICamera::SW_MULTI_FRAME:
		_daqAcqCfg.imageCtrl.frameCnt = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_SOFTWARE_START;
		break;
	case ICamera::HW_MULTI_FRAME_TRIGGER_FIRST:
		_daqAcqCfg.imageCtrl.frameCnt = (MAX_FRAME_NUM <= pImgAcqPty->numFrame) ? (MAX_FRAME_NUM) : static_cast<ULONG32>(pImgAcqPty->numFrame);
		_daqAcqCfg.triggerSettings.triggerMode = THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER;
		break;
	}

	double frameRate = 0;
	if (_scan_info.overall_lines != 0)
	{
		double cycles = (_scan_info.overall_lines + pImgAcqPty->preImagingCalibrationCycles + pImgAcqPty->imagingRampExtensionCycles);
		if (pImgAcqPty->imageOnFlyback)
		{
			cycles += pImgAcqPty->imagingRampExtensionCycles;
		}
		frameRate = _current_resonant_scanner_frequency / cycles;
	}

	//alignment settings
	_daqAcqCfg.imageCtrl.system_mode = SYSTEM_MODE::INTERNAL_RESONANT_GALVO;
	_daqAcqCfg.imageCtrl.channel = static_cast<USHORT>(pImgAcqPty->channel);
	pImgAcqPty->rx = _daqAcqCfg.imageCtrl.imgHSize = static_cast<USHORT>(pImgAcqPty->pixelX);
	int skippedTopLines = pImgAcqPty->scanMode == TWO_WAY_SCAN ? pImgAcqPty->preImagingCalibrationCycles * 2 : pImgAcqPty->preImagingCalibrationCycles;
	int skippedBottomLines = ((skippedTopLines % PIXEL_Y_MULTIPLE) == 0) ? 0 : PIXEL_Y_MULTIPLE - skippedTopLines % PIXEL_Y_MULTIPLE;
	int totalSkippedLines = skippedTopLines + skippedBottomLines;
	pImgAcqPty->postImagingCalibrationCycles = pImgAcqPty->scanMode == TWO_WAY_SCAN ? skippedBottomLines / 2 : skippedBottomLines;
	pImgAcqPty->postImagingCalibrationLines = skippedBottomLines;
	pImgAcqPty->preImagingCalibrationLines = skippedTopLines;

	_daqAcqCfg.imageCtrl.imgVSize = static_cast<USHORT>(pImgAcqPty->pixelY + totalSkippedLines);
	pImgAcqPty->ry = pImgAcqPty->pixelY;

	_daqAcqCfg.imageCtrl.scanMode = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? SCAN_MODES::BIDIRECTION_SCAN : SCAN_MODES::UNIDIRECTION_SCAN;
	_daqAcqCfg.imageCtrl.scanDir = (pImgAcqPty->horizontalFlip == FALSE) ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;
	_daqAcqCfg.imageCtrl.alignmentOffset = static_cast<USHORT>(pImgAcqPty->twoWayZonesFine[_fieldSizeMax - pImgAcqPty->fieldSize]);
	_daqAcqCfg.imageCtrl.frameNumPerSec = max(static_cast<ULONG32>(static_cast<ULONG32>(ceil(frameRate))), 1);
	_daqAcqCfg.imageCtrl.frameNumPerTransfer = max(static_cast<ULONG32>(static_cast<ULONG32>(ceil(frameRate / (double)MAX_TRANSFERS_PER_SECOND))), 1); // set frequency of interrupt to 50ms min
	_daqAcqCfg.imageCtrl.frameRate = _frameRate;
	_daqAcqCfg.imageCtrl.numPlanes = 1;
	_daqAcqCfg.imageCtrl.isPreviewImaging = ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode;

	_daqAcqCfg.imageCtrl.digitalLinesConfig = _thorDAQDigitalLinesConfig;

	for (int i = 0; i < 4; ++i)
	{
		_daqAcqCfg.imageCtrl.ADCGain[i] = pImgAcqPty->ADCGain[i];
	}

	if (_daqAcqCfg.imageCtrl.frameCnt <= _daqAcqCfg.imageCtrl.frameNumPerTransfer)
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = _daqAcqCfg.imageCtrl.frameCnt;
	}

	if (TRUE == pImgAcqPty->powerRampEnable) //live mode
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = 1;
	}
	else if (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode) //live mode
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = max(static_cast<ULONG32>(static_cast<ULONG32>(ceil(frameRate / (double)MAX_TRANSFERS_PER_SECOND_LIVE))), 1);
	}

	double flybackCycles = (pImgAcqPty->flybackCycle + pImgAcqPty->imagingRampExtensionCycles);
	if (pImgAcqPty->imageOnFlyback)
	{
		flybackCycles += pImgAcqPty->imagingRampExtensionCycles;
	}

	_daqAcqCfg.resonantGalvoCtrl.flybackTime = 1.0 / _current_resonant_scanner_frequency * flybackCycles;

	_daqAcqCfg.streamingCtrl.fir_filter_enabled = pImgAcqPty->realTimeDataAverage;
	_daqAcqCfg.streamingCtrl.scan_period = _current_resonant_scanner_frequency;

	//for now phaseIncrementMode should always be 2 
	_daqAcqCfg.coherentSamplingCtrl.phaseIncrementMode = 2;//0: disable 1:incremental mode 2: static offset

	//coherent sampling
	if (pImgAcqPty->clockSource == EXTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.threePhotonMode = FALSE;
		_daqAcqCfg.coherentSamplingCtrl.phaseOffset = static_cast<USHORT>(pImgAcqPty->laserCoherentSamplingPhase * 8.0 * 16.0 / 100.0);
		_daqAcqCfg.streamingCtrl.channel_multiplexing_enabled = pImgAcqPty->laserCoherentSamplingEnable;
	}

	switch (pImgAcqPty->dataMapMode)
	{
		int i;
	case ICamera::POLARITY_INDEPENDENT:
	{
		//folding mapping for positive and negative amplifiers
		//datamap should reflect 14bit resolution of the digitizer
		//negative voltage mapping
		for (i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			_datamap[i] = &(_datamapIndependent[0]);
		}
	}
	break;
	case ICamera::POLARITY_POSITIVE:
	{
		//16 bit mapping with most significant data in positive polarity
		//positive voltage mapping
		for (i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			_datamap[i] = &(_datamapPositiveSigned[0]);
		}
	}
	break;
	case ICamera::POLARITY_NEGATIVE:
	{
		//16 bit mapping with most significant data in negative polarity
		//negative voltage mapping
		for (i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			_datamap[i] = &(_datamapNegativeSigned[0]);
		}
	}
	break;
	default:
		for (i = 0; i < MAX_CHANNEL_COUNT; i++)
		{
			_datamap[i] = (0 == pImgAcqPty->channelPolarity[i]) ? &(_datamapNegativeUnsigned[0]) : &(_datamapPositiveUnsigned[0]);
		}
	}

	double dac_rate = 0;
	_scanLine = ScanLineStruct();
	GetDACSamplesPerLine(&_scanLine, dac_rate, 1.0 / _current_resonant_scanner_frequency / 2.0);
	_scan_info.dac_rate = dac_rate;
	_daqAcqCfg.imageCtrl.TwoBankDACDMAPlayback = false;
	_powerRampCurrentIndex = 0;
	_fastZCurrentIndex = 0;
	_daqAcqCfg.imageCtrl.dacSlowMoveToOffset = true; //always use the slow move mode for safety

	//if HW trigger pre move to start position and set preSOFTime to 0 so imaging starts immmidiately after HW trigger comes in
	if (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _daqAcqCfg.triggerSettings.triggerMode ||
		TRUE == _preMoveGalvoToStartPosition)
	{
		_daqAcqCfg.resonantGalvoCtrl.preSOFTime = 0; //0s
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::GR_Y], _daqAcqCfg.dacCtrl[_thordaqAOSelection[AO::GR_Y]].offset_val));
	}
	else if (ScannerType::MEMS == _scannerType)
	{
		_daqAcqCfg.resonantGalvoCtrl.preSOFTime = 0; // no need to wait for slow move to complete for MEMS scanner
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::GR_Y], _daqAcqCfg.dacCtrl[_thordaqAOSelection[AO::GR_Y]].offset_val));
	}
	else
	{
		double basePreSOFTime = 0.0012; // it should take 8us but we are adding a bit more to allow the move to complete smoothly. Artifacts appear at the top of the first image when we don't do this
		_daqAcqCfg.resonantGalvoCtrl.preSOFTime = basePreSOFTime;
	}

	pImgAcqPty->stripeFieldSize = pImgAcqPty->fieldSize;

	if (false == pImgAcqPty->mROIModeEnable)
	{
		_frameRate = frameRate;
		if (TRUE == _hardwareTestModeEnable)
		{
			BuildTestWaveform(&_scan_info, &_scanLine, &_galvo_control);
		}
		else
		{
			_useBuiltZWaveform = SetAndBuildFastZWaveform(&_scan_info, &_scanLine, pImgAcqPty, _frameRate, _daqAcqCfg.dacCtrl, _daqAcqCfg.dacCtrl2, &_fastZWaveformParams);

			if (TRUE == pImgAcqPty->powerRampEnable || TRUE == _useBuiltZWaveform)
			{
				//setup the galvo waveforms for dacCtrl and dacCtrl2 (first 2 frames)
				BuildGalvoWaveform(&_scan_info, &_scanLine, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl, false, true);
				BuildGalvoWaveform(&_scan_info, &_scanLine, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl2, false, true);

				if (pImgAcqPty->powerRampPercentValues.size() > 1 && pImgAcqPty->powerRampNumFrames == pImgAcqPty->powerRampPercentValues.size())
				{
					SetupPowerRampSettings(pImgAcqPty);

					//setup the pockels ramp for dacCtrl and dacCtrl2 (first 2 frames)
					SetPowerAndBuildPockelsPowerRampWaveforms(pImgAcqPty->pockelPty, _daqAcqCfg.dacCtrl);
					++_powerRampCurrentIndex;
					++_twoBankFrameIndex;
					SetPowerAndBuildPockelsPowerRampWaveforms(pImgAcqPty->pockelPty, _daqAcqCfg.dacCtrl2);
					++_powerRampCurrentIndex;
					++_twoBankFrameIndex;
					if (_powerRampCurrentIndex >= pImgAcqPty->powerRampNumFrames + pImgAcqPty->powerRampNumFlybackFrames)
					{
						_powerRampCurrentIndex = 0;
					}
				}
				else if (TRUE == pImgAcqPty->powerRampEnable)
				{
					LogMessage(L"ThordaqGR power ramp not setup correctly", ERROR_EVENT);
					return FALSE;
				}
				else
				{
					if (pImgAcqPty->imageOnFlyback)
					{
						BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl, false, false);
						BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl2, true, false);
					}
					else
					{
						BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl, false, true);
						BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl2, false, true);
					}
				}

				_daqAcqCfg.imageCtrl.TwoBankDACDMAPlayback = true;
				INT32 error = 0, retVal = 0;

				ThordaqErrChk(L"ThorDAQAPIDACBankSwitchingRegisterReadyForNextImageWaveformsEvent", retVal = ThorDAQAPIDACBankSwitchingRegisterReadyForNextImageWaveformsEvent(_DAQDeviceIndex, 0, DACReadyForNextImageWaveformsCallback, NULL));
			}
			else
			{
				if (pImgAcqPty->imageOnFlyback)
				{
					BuildGalvoWaveform(&_scan_info, &_scanLine, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl, false, false);
					BuildGalvoWaveform(&_scan_info, &_scanLine, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl2, true, false);
					BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl, false, false);
					BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl2, true, false);
				}
				else
				{
					BuildGalvoWaveform(&_scan_info, &_scanLine, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl, false, true);
					BuildPockelsWaveform(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl, false, true);
				}
			}

			if (pImgAcqPty->imageOnFlyback)
			{
				_daqAcqCfg.imageCtrl.TwoBankDACDMAPlayback = true;

				_daqAcqCfg.resonantGalvoCtrl.preSOFTime = 0; //0s

				_daqAcqCfg.imageCtrl.frameNumPerTransfer = 1;

				ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::GR_Y], _daqAcqCfg.dacCtrl[_thordaqAOSelection[AO::GR_Y]].offset_val));

			}

			SAFE_DELETE_PTR(_pDataProcessor);
			_pDataProcessor = new DataProcessor((BYTE)pImgAcqPty->channel, _daqAcqCfg.imageCtrl.imgHSize, _daqAcqCfg.imageCtrl.imgVSize, skippedTopLines, skippedBottomLines, _daqAcqCfg.imageCtrl.frameNumPerTransfer, _datamap, pImgAcqPty->imageOnFlyback);

			if (SetupFrameBuffer(pImgAcqPty) != STATUS_SUCCESSFUL)
			{
				ret = FALSE;
				return ret;
			}

			if (_rGGMode)
			{
				double offsetX = pImgAcqPty->offsetX * _field2Theta / 2;
				ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _thordaqAOSelection[AO::GG0_X], offsetX));
			}
		}
		pImgAcqPty->ismROI = false;
	}
	else
	{
		vector<StripInfo*> mROIStripes = vector<StripInfo*>();
		mROIStripesManager* waveformManager = mROIStripesManager::GetInstance();
		waveformManager->GenerateStripList(_mROIScan, mROIStripes);

		if (mROIStripes.size() <= 0)
		{
			LogMessage(L"ThordaqGR no mROI stripes generated", ERROR_EVENT);
			ret = FALSE;
			return ret;
		}

		pImgAcqPty->ismROI = (_mROIScan->Name != "Meso");

		pImgAcqPty->rx = 0;
		pImgAcqPty->ry = 0;
		pImgAcqPty->stripeFieldSize = mROIStripes[0]->StripeFieldSize;
		pImgAcqPty->maxSizeInUMForMaxFieldSize = _mROIScan->MaxSizeInUMForMaxFieldSize;
		double lineCycleTime = 1 / _current_resonant_scanner_frequency;

		if (mROIStripes.size() <= 0)
		{
			_daqAcqCfg.resonantGalvoCtrl.flybackTime = 1.0 / _current_resonant_scanner_frequency * pImgAcqPty->flybackCycle;
		}

		int overallLines = 0;
		int vSize = 0;
		int hSize = 0;
		int imageXPixel = 0;
		int imageYPixel = 0;
		int flybackCycle = 0;

		CalculatemROITotalImagingLinesAndOtherProperties(pImgAcqPty, mROIStripes, overallLines, vSize, hSize, imageXPixel, imageYPixel, flybackCycle);
		if (!pImgAcqPty->ismROI && mROIStripes[0]->Power.size() > 0)
		{
			mROIStripes[0]->Power[0] = pImgAcqPty->pockelPty.pockelsPowerLevel[0];
		}
		pImgAcqPty->doZ = (_thordaqAOSelection[AO::Z] >= 0) && pImgAcqPty->ismROI;

		_daqAcqCfg.resonantGalvoCtrl.flybackTime = 1.0 / _current_resonant_scanner_frequency * flybackCycle;
		_scan_info.backward_lines = flybackCycle;

		_daqAcqCfg.imageCtrl.imgHSize = static_cast<USHORT>(hSize);
		_daqAcqCfg.imageCtrl.imgVSize = static_cast<USHORT>(vSize);
		pImgAcqPty->rx = imageXPixel;
		pImgAcqPty->ry = imageYPixel;

		if (overallLines != 0)
		{
			_frameRate = _current_resonant_scanner_frequency / (_scan_info.backward_lines + overallLines);
		}

		_daqAcqCfg.imageCtrl.frameNumPerSec = max(static_cast<ULONG32>(static_cast<ULONG32>(ceil(_frameRate))), 1);
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = 1; // set frequency of interrupt to 50ms min

		GalvoStruct _galvo_controlX = GalvoStruct();
		_galvo_controlX.amplitude = theta * (double)pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX / 2; // divide by 2 because the mechanical angle is half of the optical angle

		if (TRUE == _useZoomArray)
		{
			_galvo_controlX.amplitude = _galvo_controlX.amplitude + _galvo_controlX.amplitude * _zoomArray[pImgAcqPty->fieldSize] / 100.0;
		}

		_galvo_controlX.amplitude = (pImgAcqPty->yAmplitudeScaler / 100.0) * _galvo_controlX.amplitude;

		if (FALSE == pImgAcqPty->galvoEnable) //if LineScan is enabled set the amplitude to 0
		{
			_galvo_controlX.amplitude = 0;
		}

		_galvo_controlX.offset = (double)pImgAcqPty->offsetX * _field2Theta / 2;
		_galvo_controlX.park = (double)GALVO_PARK_POSITION;
		_galvo_controlX.scan_direction = pImgAcqPty->verticalScanDirection == 0 ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;

		BuildmROIWaveforms(mROIStripes, &_scan_info, &_scanLine, &_galvo_controlX, &_galvo_control, pImgAcqPty, _daqAcqCfg.dacCtrl);

		map<ULONG, mROIImageStruct> imageAreas = map<ULONG, mROIImageStruct>();

		SAFE_DELETE_PTR(_pDataProcessor);

		_pDataProcessor = new mROIDataProcessor((BYTE)pImgAcqPty->channel, _daqAcqCfg.imageCtrl.frameNumPerTransfer, _datamap, mROIStripes, pImgAcqPty->horizontalFlip == TRUE);

		if (SetupFrameBuffermROI(pImgAcqPty, mROIStripes) != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
			return ret;
		}
	}

	_lastAreaCopiedIndex = -1;

	ThordaqErrChk(L"ThorDAQAPISetImagingConfiguration", retVal = ThorDAQAPISetImagingConfiguration(_DAQDeviceIndex, _daqAcqCfg));
	if (retVal != STATUS_SUCCESSFUL)
	{
		//printf("Setup Packet Generator failed, Max Packet Size (%ld) is too large\n", pDgTzrParams.bufferSize);	//MaxPacketSize
		ret = FALSE;
		return ret;
	}

	return ret;
}

void CThordaqResonantGalvo::SetPowerAndBuildPockelsPowerRampWaveforms(PockelPty pockelsSettings, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms)
{
	switch (_imgAcqPty_Pre.powerRampMode)
	{
	case PowerRampMode::POWER_RAMP_MODE_STAIRCASE:
	{
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			pockelsSettings.pockelsPowerLevel[i] = _pockelsResponsePowerLevels[i][_powerRampCurrentIndex];
		}

		bool invert = _imgAcqPty_Pre.imageOnFlyback && (_twoBankFrameIndex % 2 == 1);
		CThordaqResonantGalvo::GetInstance()->BuildPockelsWaveform(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty_Pre, waveforms, invert, !_imgAcqPty_Pre.imageOnFlyback);
		break;
	}
	default:
	{
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			pockelsSettings.pockelsPowerLevel[i] = _pockelsImagePowerRampVector[i][_powerRampCurrentIndex].startPowerLevel;
			pockelsSettings.pockelsPowerLevel2[i] = _pockelsImagePowerRampVector[i][_powerRampCurrentIndex].endPowerLevel;
		}

		bool invert = _imgAcqPty_Pre.imageOnFlyback && (_twoBankFrameIndex % 2 == 1);

		CThordaqResonantGalvo::GetInstance()->BuildPockelsPowerRampWaveform(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty_Pre, waveforms, invert, !_imgAcqPty_Pre.imageOnFlyback);
		break;
	}
	}
}

void CThordaqResonantGalvo::SetupPowerRampSettings(ImgAcqPty* pImgAcqPty)
{
	for (int k = 0; k < MAX_POCKELS_CELL_COUNT; ++k)
	{
		_pockelsResponsePowerLevels[k].clear();
		for (int i = 0; i < pImgAcqPty->powerRampPercentValues.size(); ++i)
		{
			double val = 0.0;
			switch (CThordaqResonantGalvo::GetInstance()->_pockelsResponseType[k])
			{
			case static_cast<long>(PockelsResponseType::SINE_RESPONSE):
			{
				val = acos(1 - static_cast<double>(Constants::AREA_UNDER_CURVE) * pImgAcqPty->powerRampPercentValues[i] / 100.0) / PI;
				break;
			}
			case static_cast<long>(PockelsResponseType::LINEAR_RESPONSE):
			{
				val = pImgAcqPty->powerRampPercentValues[i] / 100.0;
				break;
			}
			default:
			{
				val = 0;
				break;
			}
			}

			_pockelsResponsePowerLevels[k].push_back(val);
		}
	}
	std::vector<double> pockelsPowerLevels[MAX_POCKELS_CELL_COUNT];
	for (int k = 0; k < MAX_POCKELS_CELL_COUNT; ++k)
	{

		int last = pImgAcqPty->powerRampNumFrames + 1;
		pockelsPowerLevels[k].clear();

		for (int i = 0; i < last; ++i)
		{
			double percentOfZRange = min(1.0, (i + 1) / ((double)last));
			double lastZPercent = -1;
			double lastPower = 0;


			double pos, power;

			for (int j = 0; j < _pockelsResponsePowerLevels[k].size(); ++j)
			{
				pos = min(1.0, (j + 1) / ((double)_pockelsResponsePowerLevels[k].size()));

				power = _pockelsResponsePowerLevels[k][j];
				//piecewise interpolate
				if ((percentOfZRange > lastZPercent) && (percentOfZRange <= pos))
				{
					double result;
					if (lastZPercent < 0)
					{
						//first point do not need interpolation
						result = _pockelsResponsePowerLevels[k][j];
					}
					else
					{
						double slope = (power - lastPower) / (pos - lastZPercent);

						result = lastPower + slope * (percentOfZRange - lastZPercent);
					}

					pockelsPowerLevels[k].push_back(result);
					lastZPercent = pos;
					lastPower = power;
					break;
				}

				lastZPercent = pos;
				lastPower = power;
			}
		}
	}

	for (int k = 0; k < MAX_POCKELS_CELL_COUNT; ++k)
	{
		_pockelsImagePowerRampVector[k].clear();

		for (int i = 0; i < pImgAcqPty->powerRampNumFrames; ++i)
		{
			PockelsImagePowerRampStruct str = PockelsImagePowerRampStruct();
			str.startPowerLevel = pockelsPowerLevels[k][i];
			str.endPowerLevel = pockelsPowerLevels[k][i + 1];
			_pockelsImagePowerRampVector[k].push_back(str);
		}
	}
}

long CThordaqResonantGalvo::SetAndBuildFastZWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, ImgAcqPty* pImgAcqPty, double frameRate, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms2, ThorDAQZWaveformParams* zWaveformParams)
{
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	UINT rampExtensionSamples = samples_per_line * 2 * (UINT)(pImgAcqPty->imagingRampExtensionCycles);
	UINT backward_data_num_intra = samples_per_line * 2 * ((UINT)scanInfo->backward_lines) + rampExtensionSamples; //intra
	UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 1 + pImgAcqPty->preImagingCalibrationCycles + pImgAcqPty->postImagingCalibrationCycles);

	if (pImgAcqPty->imageOnFlyback)
	{
		backward_data_num_intra += rampExtensionSamples;
	}

	UINT forward_data_num0 = samples_per_line * 2 * (UINT)(scanInfo->forward_lines);
	UINT forward_data_num = rampExtensionSamples + forward_data_num0;
	if (pImgAcqPty->imageOnFlyback)
	{
		forward_data_num += rampExtensionSamples;
	}

	long useZWave = BuildAndGetFastZWaveform(scanInfo->dac_rate, forward_data_num, backward_data_num, frameRate, zWaveformParams);
	long useRemoteFocusWaveform = GetRemoteFocusFastZWaveform(scanInfo->dac_rate, forward_data_num, backward_data_num, frameRate, zWaveformParams);
	long ret = FALSE;

	_fastZCurrentIndex = 0;
	// Piezo FastZ and Remote Focus FastZ can be used at the same time. If that ever changes then this setup needs to be separated accordingly
	if (TRUE == useZWave || TRUE == useRemoteFocusWaveform)
	{
		DAC_WAVEFORM_CRTL_STRUCT gZCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		gZCtrl.update_rate = scanInfo->dac_rate;
		gZCtrl.flyback_samples = backward_data_num_intra;
		gZCtrl.output_port = zWaveformParams->waveformChannel;
		gZCtrl.waveform_buffer_size = sizeof(USHORT) * (zWaveformParams->samplesPerFrame + DAC_FIFO_DEPTH);
		gZCtrl.waveformBuffer = new USHORT[zWaveformParams->samplesPerFrame + DAC_FIFO_DEPTH];
		gZCtrl.filterInhibit = false;
		gZCtrl.hSync = true;
		gZCtrl.enableEOFFreeze = true;
		gZCtrl.park_val = zWaveformParams->parkPosition;
		gZCtrl.offset_val = zWaveformParams->offsetPosition;
		for (int i = 0; i < zWaveformParams->samplesPerFrame; ++i)
		{
			gZCtrl.waveformBuffer[i] = zWaveformParams->waveform[_fastZCurrentIndex];

			_fastZCurrentIndex++;

			if (_fastZCurrentIndex >= zWaveformParams->waveformLength)
			{
				_fastZCurrentIndex = 0;
			}
		}
		memcpy(gZCtrl.waveformBuffer + zWaveformParams->samplesPerFrame, zWaveformParams->waveform + _fastZCurrentIndex, DAC_FIFO_DEPTH * sizeof(USHORT));

		waveforms[zWaveformParams->waveformChannel] = gZCtrl;

		DAC_WAVEFORM_CRTL_STRUCT gZCtrl2 = DAC_WAVEFORM_CRTL_STRUCT();
		gZCtrl2.update_rate = scanInfo->dac_rate;
		gZCtrl2.flyback_samples = backward_data_num_intra;
		gZCtrl2.output_port = zWaveformParams->waveformChannel;
		gZCtrl2.waveform_buffer_size = sizeof(USHORT) * (zWaveformParams->samplesPerFrame + DAC_FIFO_DEPTH);
		gZCtrl2.waveformBuffer = new USHORT[zWaveformParams->samplesPerFrame + DAC_FIFO_DEPTH];
		gZCtrl2.filterInhibit = false;
		gZCtrl2.hSync = true;
		gZCtrl2.enableEOFFreeze = true;
		gZCtrl2.park_val = zWaveformParams->parkPosition;
		gZCtrl2.offset_val = zWaveformParams->offsetPosition;
		for (int i = 0; i < zWaveformParams->samplesPerFrame; ++i)
		{
			gZCtrl2.waveformBuffer[i] = zWaveformParams->waveform[_fastZCurrentIndex];

			_fastZCurrentIndex++;

			if (_fastZCurrentIndex >= zWaveformParams->waveformLength)
			{
				_fastZCurrentIndex = 0;
			}
		}
		memcpy(gZCtrl2.waveformBuffer + zWaveformParams->samplesPerFrame, zWaveformParams->waveform + _fastZCurrentIndex, DAC_FIFO_DEPTH * sizeof(USHORT));

		waveforms2[zWaveformParams->waveformChannel] = gZCtrl2;

		if (_fastZCurrentIndex >= zWaveformParams->waveformLength)
		{
			_fastZCurrentIndex = 0;
		}
		ret = TRUE;
	}

	return ret;
}

void CTHORDAQCALLBACK CThordaqResonantGalvo::DACReadyForNextImageWaveformsCallback(THORDAQ_STATUS status, void* callbackData)
{
	if (TRUE == _imgAcqPty_Pre.powerRampEnable)
	{
		if (_powerRampCurrentIndex < _imgAcqPty_Pre.powerRampNumFrames)
		{
			CThordaqResonantGalvo::GetInstance()->SetPowerAndBuildPockelsPowerRampWaveforms(_imgAcqPty_Pre.pockelPty, _daqAcqCfg.dacCtrl2);
		}
		else if (_powerRampCurrentIndex < _imgAcqPty_Pre.powerRampNumFrames + _imgAcqPty_Pre.powerRampNumFlybackFrames)
		{
			PockelPty pockelsSettings = _imgAcqPty_Pre.pockelPty;
			for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
			{
				pockelsSettings.pockelsPowerLevel[i] = 0;
			}

			CThordaqResonantGalvo::GetInstance()->BuildPockelsWaveform(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty_Pre, _daqAcqCfg.dacCtrl2, false, !_imgAcqPty_Pre.imageOnFlyback);
		}

		++_powerRampCurrentIndex;

		if (_powerRampCurrentIndex >= _imgAcqPty_Pre.powerRampNumFrames + _imgAcqPty_Pre.powerRampNumFlybackFrames)
		{
			_powerRampCurrentIndex = 0;
		}
	}
	else
	{
		PockelPty pockelsSettings = _imgAcqPty_Pre.pockelPty;
		bool invert = _imgAcqPty_Pre.imageOnFlyback && (_twoBankFrameIndex % 2 == 1);
		CThordaqResonantGalvo::GetInstance()->BuildPockelsWaveform(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty_Pre, _daqAcqCfg.dacCtrl2, invert, !_imgAcqPty_Pre.imageOnFlyback);
	}
	++_twoBankFrameIndex;
	if (_useBuiltZWaveform)
	{
		for (int i = 0; i < _fastZWaveformParams.samplesPerFrame; ++i)
		{
			_daqAcqCfg.dacCtrl2[_fastZWaveformParams.waveformChannel].waveformBuffer[i] = _fastZWaveformParams.waveform[_fastZCurrentIndex];

			++_fastZCurrentIndex;

			if (_fastZCurrentIndex >= _fastZWaveformParams.waveformLength)
			{
				_fastZCurrentIndex = 0;
			}
		}
		memcpy(_daqAcqCfg.dacCtrl2[_fastZWaveformParams.waveformChannel].waveformBuffer + _fastZWaveformParams.samplesPerFrame, _fastZWaveformParams.waveform + _fastZCurrentIndex, DAC_FIFO_DEPTH * sizeof(USHORT));
	}

	ThorDAQAPIDACBankSwitchingLoadNextWaveform(_DAQDeviceIndex, _daqAcqCfg.dacCtrl2);
}

// Convert radians to degrees and vice versa
inline double radians(double deg) { return deg * M_PI / 180.0; }
inline double degrees(double rad) { return rad * 180.0 / M_PI; }

/************************************************************************************************
* @fn	long CThordaqResonantGalvo::SetupFrameBuffer()
*
* @brief	Set up Frame Buffer.
* @return	A long.
**************************************************************************************************/
long CThordaqResonantGalvo::SetupFrameBuffer(ImgAcqPty* pImgAcqPty)
{
	std::bitset<sizeof(size_t)* CHAR_BIT> channel_bitset(pImgAcqPty->channel);
	long channelCount = static_cast<long>(channel_bitset.count());
	//BUFFER #1: 1 second frame buffer (frame rate frames for all channels):
	size_t AllocSize = _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize * _daqAcqCfg.imageCtrl.frameNumPerTransfer * sizeof(USHORT);

	{
		if ((AllocSize == 0) || (AllocSize < 0))
		{
			printf("Invalid Buffer Allocation Size = %zd\n", AllocSize);
			return FALSE;
		}
		for (int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
		{
			UCHAR* ptr = S2MMdmaBuffer.GetChannelStartAddress(i, AllocSize);
			_BufferContiguousArray[i] = ptr;
		}
	}
	//BUFFER #2: history buffer for average (1 frame for all channels):

	{
		for (int i = 0; i < _pHistoryBuf.size(); ++i)
		{
			SAFE_DELETE_PTR(_pHistoryBuf[i]);
		}
		_pHistoryBuf.clear();

		ProcessedFrame* histFrame = new ProcessedFrame((UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, (UINT)channelCount, 0, (UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, 0, 0);
		if (histFrame == NULL)
		{
			return FALSE;
		}
		_pHistoryBuf.push_back(histFrame);
	}

	{
		//BUFFER #3: circular buffer for read (by user) and write (by camera):
		//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
		SAFE_DELETE_PTR(_pFrmBuffer);
		if (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode || ICamera::HW_MULTI_FRAME_TRIGGER_EACH == pImgAcqPty->triggerMode) // continuous scan
		{
			// No need for a big buffer, better to keep it at 1 (transerFrames = 1 in continuous mode)so we know we are grabbing the last frame
			_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->rx, pImgAcqPty->ry, channelCount, sizeof(USHORT), (size_t)_daqAcqCfg.imageCtrl.frameNumPerTransfer * DEFAULT_DMA_BUFFER_NUM);
		}
		else
		{
			size_t dmaBufferCount = (ULONG)pImgAcqPty->dmaBufferCount > _daqAcqCfg.imageCtrl.frameCnt ? _daqAcqCfg.imageCtrl.frameCnt : static_cast<size_t>(pImgAcqPty->dmaBufferCount);
			// Use DMA buffer to setup the size of this circular buffer
			_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->rx, pImgAcqPty->ry, channelCount, sizeof(USHORT), dmaBufferCount);
		}
	}

	{
		for (int i = 0; i < _pTempBuf.size(); ++i)
		{
			SAFE_DELETE_PTR(_pTempBuf[i]);
		}
		_pTempBuf.clear();

		ProcessedFrame* tempFrame = new ProcessedFrame((UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, (UINT)channelCount, 0, (UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, 0, 0);
		if (tempFrame == NULL)
		{
			return FALSE;
		}
		_pTempBuf.push_back(tempFrame);
	}


	{
		for (int i = 0; i < _pTempBufCorrection.size(); ++i)
		{
			SAFE_DELETE_PTR(_pTempBufCorrection[i]);
		}
		_pTempBufCorrection.clear();
		SAFE_DELETE_ARRAY(_pTempDoubleBuffCorrection);

		//if image distortion correction is enabled, create a buffer to store the corrected pixel data
		//precompute the indices for each pixel
		//store the indices in a vector
		//use the indices to access the pixel data in the corrected image
		//also precompute the scaling factors for each pixel
		if (pImgAcqPty->enableImageDistortionCorrection)
		{
			PrepareImageDistortionCorrectionParameters(pImgAcqPty, channelCount);
		}
	}

	return STATUS_SUCCESSFUL;
}

long CThordaqResonantGalvo::BuildGalvoWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrl, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart)
{
	double dac_rate = scanInfo->dac_rate;
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	UINT rampExtensionSamples = samples_per_line * 2 * (UINT)(pImgAcqPty->imagingRampExtensionCycles);
	UINT backward_data_num_intra = samples_per_line * 2 * ((UINT)scanInfo->backward_lines) + rampExtensionSamples; //intra
	if (!flybackToStart)
	{
		backward_data_num_intra += rampExtensionSamples;
	}
	UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 1 + pImgAcqPty->preImagingCalibrationCycles + pImgAcqPty->postImagingCalibrationCycles);

	UINT forward_data_num0 = samples_per_line * 2 * (UINT)(scanInfo->forward_lines);
	UINT forward_data_num = rampExtensionSamples + forward_data_num0;
	if (!flybackToStart)
	{
		forward_data_num += rampExtensionSamples;
	}

	UINT fwdLines = (UINT)scanInfo->forward_lines + (UINT)pImgAcqPty->imagingRampExtensionCycles;
	if (!flybackToStart)
	{
		fwdLines += (UINT)pImgAcqPty->imagingRampExtensionCycles;
	}

	UINT total_samples = forward_data_num + backward_data_num + DAC_FIFO_DEPTH;
	USHORT* pGalvoWaveform = new USHORT[total_samples]();

	double half_P2P_amp_Y = galvoCtrl->amplitude / 2.0 / GALVO_RESOLUTION;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrl->scan_direction) ? 1.0 : -1.0;
	if (invert)
	{
		yDirection = -yDirection;
	}
	double amp_offset_Y = galvoCtrl->offset / GALVO_RESOLUTION + 0x8000;
	double galvoYFwdStep = (galvoCtrl->amplitude / (double)(forward_data_num0) / GALVO_RESOLUTION);
	double offset = 0;// galvoCtrl->offset - yDirection * galvoCtrl->amplitude / 2.0 - yDirection * rampExtensionSteps * galvoYFwdStep * GALVO_RESOLUTION;
	double waveOffset = !invert ? 0 : (offset / GALVO_RESOLUTION + 0x8000) + (galvoCtrl->amplitude * (-1) * yDirection) / GALVO_RESOLUTION + 0x8000 + 2 * rampExtensionSamples * galvoYFwdStep; // Need to check why this works

	//----------------Allocate space for digital waveforms--------------------------
	USHORT* pGalvoWaveformDigitalOutputs;
	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_samples]();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = DAC_WAVEFORM_CRTL_STRUCT();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer = pGalvoWaveformDigitalOutputs;
	}

	memset(pGalvoWaveformDigitalOutputs, 0, total_samples * sizeof(USHORT));

	// For 'invert vertical scan' we multiply the waveformBuffer value by -1.0 same as the NI/Alazar counterpart. The negative value is cast to
	//unsigned short. This will convert it to UMAX SHORT - waveformBuffer value, which works for the vertical invert. This is why we don't check if
	//the waveformBuffer value is below 0
	for (ULONG32 j = 0; j < (ULONG32)forward_data_num; j++)
	{
		*(pGalvoWaveform + j) = static_cast<USHORT>(waveOffset + yDirection * (galvoYFwdStep * (double)(j + 1)));
		*(pGalvoWaveformDigitalOutputs + j) |= FRAME_TRIGGER_OUT_HIGH;
	}

	if (_scannerType == ScannerType::MEMS && pImgAcqPty->imageOnFlyback == false)
	{
		for (ULONG32 j = 0; j < (ULONG32)backward_data_num; j++)
		{
			*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + j)) = 0;
			*(pGalvoWaveformDigitalOutputs + (ULONG32)(samples_per_line * 2 * fwdLines + j)) |= THORDAQ_DO_LOW;
		}
	}
	else
	{
		for (ULONG32 j = 0; j < (ULONG32)backward_data_num; j++)
		{
			if (flybackToStart)
			{
				*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + j)) = static_cast<USHORT>(min(USHRT_MAX, yDirection * (galvoYFwdStep * (double)forward_data_num) * (cos(M_PI * (double)(j + 1) / (double)backward_data_num) / 2.0 + 0.5)));
				*(pGalvoWaveformDigitalOutputs + (ULONG32)(samples_per_line * 2 * fwdLines + j)) |= THORDAQ_DO_LOW;
			}
			else
			{
				*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + j)) = *(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + j - 1));
				*(pGalvoWaveformDigitalOutputs + (ULONG32)(samples_per_line * 2 * fwdLines + j)) |= THORDAQ_DO_LOW;
			}
		}
	}

	for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
	{
		*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + backward_data_num + j)) = *(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * fwdLines + backward_data_num + j - 1));
		*(pGalvoWaveformDigitalOutputs + (ULONG32)(samples_per_line * 2 * fwdLines + backward_data_num + j)) |= THORDAQ_DO_LOW;
	}

	//--------------Analog Y galvo waveformBuffer--------------
	DAC_WAVEFORM_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gYCtrl.park_val = galvoCtrl->park;
	gYCtrl.offset_val = galvoCtrl->offset - yDirection * galvoCtrl->amplitude / 2.0 - yDirection * rampExtensionSamples * galvoYFwdStep * GALVO_RESOLUTION;
	gYCtrl.update_rate = dac_rate;
	gYCtrl.flyback_samples = backward_data_num_intra;
	gYCtrl.output_port = _thordaqAOSelection[AO::GR_Y];
	gYCtrl.waveform_buffer_size = sizeof(USHORT) * (total_samples);
	gYCtrl.waveformBuffer = pGalvoWaveform;
	gYCtrl.filterInhibit = false;
	gYCtrl.hSync = true;
	gYCtrl.enableEOFFreeze = !flybackToStart;

	waveforms[_thordaqAOSelection[AO::GR_Y]] = gYCtrl;

	/*string waveformFile = "Galvo_waveform.txt";
	ofstream myfile (waveformFile);
	if (myfile.is_open())
	{
	for (int i = 0; i < total_samples; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveform+i));
	myfile << "\n";
	}
	myfile.close();
	}*/

	//SAFE_DELETE_ARRAY(pGalvoWaveform);

	return TRUE;
}


long CThordaqResonantGalvo::BuildTestWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrl)
{
	double dac_rate = scanInfo->dac_rate;
	UINT samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	UINT backward_data_num = samples_per_line * 2 * ((UINT)scanInfo->backward_lines - 1);
	UINT forward_data_num = samples_per_line * 2 * (UINT)scanInfo->forward_lines;

	UINT total_samples = forward_data_num + backward_data_num + DAC_FIFO_DEPTH;
	USHORT* pGalvoWaveform = new USHORT[total_samples];

	double half_P2P_amp_Y = galvoCtrl->amplitude / 2.0 / GALVO_RESOLUTION;
	double amp_offset_Y = galvoCtrl->offset / GALVO_RESOLUTION + 0x8000;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrl->scan_direction) ? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrl->amplitude / (double)(forward_data_num) / GALVO_RESOLUTION);

	USHORT waveform_start = max(0, min(USHRT_MAX, static_cast<USHORT>(amp_offset_Y - yDirection * half_P2P_amp_Y)));
	int index = 0;
	for (int i = 0; i < scanInfo->forward_lines; i++)
	{
		for (UINT j = 0; j < 2 * samples_per_line; j++)
		{
			*(pGalvoWaveform + index++) = min(USHRT_MAX, max(0, static_cast<USHORT>(half_P2P_amp_Y - half_P2P_amp_Y * cos((double)j / (2.0 * (double)samples_per_line) * 2.0 * M_PI))));
		}
	}
	for (ULONG32 j = 0; j < (ULONG32)backward_data_num; j++)
	{
		*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + j)) = 0;
	}

	for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
	{
		*(pGalvoWaveform + (ULONG32)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j)) = 0;
	}

	IMAGING_BUFFER_STRUCT DACMemorySetting;
	DACMemorySetting.buffer = (UCHAR*)(pGalvoWaveform + FIFO_DELAY_SAMPLES);
	DACMemorySetting.channel = _thordaqAOSelection[AO::GR_Y];
	DACMemorySetting.length = (total_samples - FIFO_DELAY_SAMPLES) * 2;
	DACMemorySetting.offset = 0;

	for (int i = 0; i < WAVETABLE_CHANNEL_COUNT; i++)
	{
		_daqAcqCfg.dacCtrl[i].park_val = galvoCtrl->park;
		_daqAcqCfg.dacCtrl[i].offset_val = galvoCtrl->offset - yDirection * galvoCtrl->amplitude / 2.0;
		_daqAcqCfg.dacCtrl[i].update_rate = dac_rate;
		_daqAcqCfg.dacCtrl[i].output_port = i;
		_daqAcqCfg.dacCtrl[i].flyback_samples = backward_data_num;
	}


	/*string waveformFile = "Galvo_waveform.txt";
	ofstream myfile (waveformFile);
	if (myfile.is_open())
	{
	for (int i = 0; i < total_samples; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveform+i));
	myfile << "\n";
	}
	myfile.close();
	}*/

	SAFE_DELETE_ARRAY(pGalvoWaveform);
	return TRUE;
}

long CThordaqResonantGalvo::BuildPockelsWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart)
{
	double dac_rate = scanInfo->dac_rate;
	int samples_per_line = 2 * (int)scanLine->samples_idle + scanLine->samples_scan;
	int preImagingCalibrationSamples = samples_per_line * 2 * (int)(pImgAcqPty->preImagingCalibrationCycles);
	int rampExtensionSamples = samples_per_line * 2 * (int)(pImgAcqPty->imagingRampExtensionCycles);
	int postcalibrationSamples = samples_per_line * 2 * (int)pImgAcqPty->postImagingCalibrationCycles;
	int backward_data_num_intra = samples_per_line * 2 * ((int)scanInfo->backward_lines) + rampExtensionSamples; //intra
	if (!flybackToStart)
	{
		backward_data_num_intra += rampExtensionSamples;
	}

	int backward_data_num = samples_per_line * 2 * ((int)scanInfo->backward_lines - 1) + postcalibrationSamples;
	int total_samples = preImagingCalibrationSamples + samples_per_line * 2 * (int)scanInfo->forward_lines + rampExtensionSamples + backward_data_num + DAC_FIFO_DEPTH;
	if (!flybackToStart)
	{
		total_samples += rampExtensionSamples;
	}

	int calibrationSamples = preImagingCalibrationSamples;

	USHORT* pGalvoWaveformDigitalOutputs;
	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_samples];
	}

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	unsigned char* blankingMask = NULL;
	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (pockelPty->pockelsLineBlankingPercentage[i] > 0)
		{
			blankingMask = new unsigned char[width * height];
			break;
		}
	}
	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		unsigned short* roiMask = NULL;
		const long BYTES_PER_PIXEL = 2;
		if (_pockelsMaskSize[i] == width * height * BYTES_PER_PIXEL)
		{
			roiMask = new unsigned short[width * height];
			LSMGRRotationAngle rotation = (LSMGRRotationAngle)_rotationAnglePosition;
			long horizontalFlip = pImgAcqPty->horizontalFlip;
			if (FALSE == horizontalFlip)
			{
				switch ((LSMGRRotationAngle)_rotationAnglePosition)
				{
				case LSMGRRotationAngle::DEG_270: rotation = LSMGRRotationAngle::DEG_90; break;
				case LSMGRRotationAngle::DEG_90: rotation = LSMGRRotationAngle::DEG_270; break;
				default:
					break;
				}
			}

			if (invert)
			{
				switch ((LSMGRRotationAngle)_rotationAnglePosition)
				{
				case LSMGRRotationAngle::DEG_270: rotation = LSMGRRotationAngle::DEG_90; break;
				case LSMGRRotationAngle::DEG_90: rotation = LSMGRRotationAngle::DEG_270; break;
				case LSMGRRotationAngle::DEG_180: rotation = LSMGRRotationAngle::DEG_0; break;
				case LSMGRRotationAngle::DEG_0: rotation = LSMGRRotationAngle::DEG_180; break;
				default:
					break;
				}
				horizontalFlip = horizontalFlip == FALSE ? TRUE : FALSE;
			}
			ManipulateAndCopyImage((unsigned short*)_pPockelsMask[i], roiMask, width, height, 1, 1, TRUE, rotation, horizontalFlip);
		}

		if (_pockelsEnable[i] == FALSE || pockelPty->pockelsPowerLevel[i] == 0)
		{
			SAFE_DELETE_ARRAY(roiMask);
			continue;
		}

		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + i)];

		double pockelsOnVoltage = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[i]) / GALVO_RESOLUTION));

		USHORT* pPockelWaveform;
		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			pPockelWaveform = waveforms[pockelOutputChannel].waveformBuffer;
		}
		else
		{
			pPockelWaveform = new USHORT[total_samples];
		}
		memset(pPockelWaveform, pockels_output_low_ushort, total_samples * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		if (FALSE == _pockelsFlybackBlank)
		{
			for (int i = 0; i < total_samples; ++i)
			{
				pGalvoWaveformDigitalOutputs[i] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelWaveform[i] = pockels_output_high_ushort;
			}
		}

		long usePockelsMask = FALSE;

		//if the mode is enabled, the pointer is valid, and the offset not being applied in X
		if (pockelPty->pockelsMaskEnable[i] && _pPockelsMask[i] && (pImgAcqPty->offsetX == 0))
		{

			//determine if the size of the mask matches the pockels waveformBuffer output
			if (_pockelsMaskSize[i] == width * height * BYTES_PER_PIXEL)
			{
				usePockelsMask = TRUE;
			}
		}

		double blankingPercent = pockelPty->pockelsLineBlankingPercentage[i];
		if (blankingPercent > 0)
		{
			int k = 0;
			for (int j = 0; j < height; ++j)
			{
				for (int m = 0; m < width; ++m)
				{
					if ((int)round(width * blankingPercent) <= m && (int)round(width * (1 - blankingPercent)) > m)
					{
						blankingMask[k] = 1;
					}
					else
					{
						blankingMask[k] = 0;
					}
					++k;
				}
			}
		}

		//Make sure pockels phase adjust is less than the number of blanking samples
		int pockelsPhaseAdjustSamples = static_cast<int>(round(pockelPty->pockelsDelayUS[i] / 1000000.0 * dac_rate));

		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if ((FALSE == _pockelsTurnAroundBlank || pImgAcqPty->scanMode == TWO_WAY_SCAN) && 0 == pockelPty->pockelsLineBlankingPercentage[i] && FALSE == usePockelsMask)
		{
			for (int i = (int)calibrationSamples; i < samples_per_line * 2 * (int)scanInfo->forward_lines + (int)calibrationSamples; i++)
			{
				int sample_index = i + pockelsPhaseAdjustSamples >= 0 ? i + pockelsPhaseAdjustSamples : 0; //pockel cell index ptr
				pGalvoWaveformDigitalOutputs[sample_index] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelWaveform[sample_index] = pockels_output_high_ushort;
			}
		}
		else
		{
			for (int j = 0; j < scanInfo->forward_lines; j++)
			{
				int sample_index = 0;

				//There are 4 blanking regions that must be accomodated
				//1-Beginning of a front line scan
				//2-End of a front line scan to begining Of back line scan
				//3-End of a back line scan
				//4-Backscan

				int beginningOfFrontLineScan = pockelsPhaseAdjustSamples;
				int endOfFrontLineScan = static_cast<int>(samples_per_line);
				int endOfBackLineScan = static_cast<int>(samples_per_line * 2);

				/*StringCbPrintfW(message,MSG_SIZE, L"ThorGR pockels blank percentage spl %d %f phaseAdjustment %f blanking samples %d",samples_per_line, pockelPty->pockelsLineBlankingPercentage[i],_pockelsPhaseAdjustMicroSec, blanking_samples);
				LogMessage(message,ERROR_EVENT);*/

				double const W1 = (width - 1);
				double const PHASE_SHIFT = M_PI * pImgAcqPty->pockelsBlankingPhaseShiftPercent / 100;


				///        |>-----beginningOfFrontLineScan-----------------endOfFrontLineScan--------->-|
				///                                                                                    |
				///        |<-----endOfBackLineScan-----------------------beginingOfBackLineScan-----<-|


				for (; sample_index < beginningOfFrontLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsPhaseAdjustSamples + calibrationSamples > 0 ? pockelsPhaseAdjustSamples + calibrationSamples : calibrationSamples;

					pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
					pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
				}

				for (; sample_index < endOfFrontLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsPhaseAdjustSamples + calibrationSamples > 0 ? pockelsPhaseAdjustSamples + calibrationSamples : calibrationSamples;
					//To account for the sinusoidal movement of the resonant scanner
					//the x offset on the mask is calculated using a cosine and shifting it by 90deg
					//and adding 1 to start at 0 and increase as sample_index increases "(cos(PI + PI * sample_index / (samples) + C1) + 1) / 2.0"
					//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
					long xOffset = 0;
					if (FALSE == pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(floor(W1 * (cos(M_PI + M_PI * (sample_index + sample_offset) / (samples_per_line)+PHASE_SHIFT) + 1) / 2.0 + 0.5));
					}
					else
					{
						//if there is a horizontal flip, then start at the end of the line
						xOffset = static_cast<long>(floor(W1 * (cos(M_PI + M_PI * ((samples_per_line - 1) - (sample_index + sample_offset)) / (samples_per_line - 1) - PHASE_SHIFT) + 1) / 2.0 + 0.5));
					}

					long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? j * (width * 2) + xOffset : j * (width)+xOffset;

					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					if (TRUE == usePockelsMask)
					{
						unsigned short* pRoiMask = roiMask;
						pRoiMask += maskOffset;

						//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
						//then set the pockels to the on voltage
						if ((0 != *pRoiMask && !pockelPty->pockelsMaskInvert[i]) || (0 == *pRoiMask && pockelPty->pockelsMaskInvert[i] && (NULL == blankingMask || 0 != blankingMask[maskOffset])))
						{
							//StringCbPrintfW(message,MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d",maskOffset,*pMask);
							//LogMessage(message,VERBOSE_EVENT);
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
						}
					}
					else if (blankingPercent > 0)
					{
						if (0 != blankingMask[maskOffset])
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
						}
					}
					else
					{
						pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
						pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
					}
				}

				for (; sample_index < endOfBackLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsPhaseAdjustSamples + calibrationSamples > 0 ? pockelsPhaseAdjustSamples + calibrationSamples : calibrationSamples;
					//only enable the backscan if two way mode is enabled
					if (pImgAcqPty->scanMode == TWO_WAY_SCAN)
					{
						//To account for the sinusoidal movement of the resonant scanner
						//the x offset on the mask is calculated using a cosine and shifting it by 90deg
						//and adding 1 to start at 0 and increase as j increases "cos(PI + PI* (j -  (_subtriggerLength/2))/(_subtriggerLength/2) + C1) + 1)/2.0 "
						//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
						//Here, because its the backward scan, we want to start at the end of the x for the line and then move to the left until
						//we reach the begining of the line. Then we go back to the forward scan
						long xOffset = 0;
						if (FALSE == pImgAcqPty->horizontalFlip)
						{
							xOffset = static_cast<long>(ceil(W1 * (cos(M_PI + M_PI * ((sample_index + sample_offset) - samples_per_line) / samples_per_line + PHASE_SHIFT) + 1) / 2.0 - 0.5));
						}
						else
						{
							//if there is a horizontal flip, then start at the end of the line
							xOffset = static_cast<long>(ceil(W1 * (cos(M_PI + M_PI * ((samples_per_line)-((sample_index + sample_offset) - (samples_per_line))) / (samples_per_line)-PHASE_SHIFT) + 1) / 2.0 - 0.5));
						}

						long maskOffset = j * (width * 2) + (width * 2 - xOffset);

						if (maskOffset < 0)
						{
							maskOffset = 0;
						}

						if (TRUE == usePockelsMask)
						{
							unsigned short* pRoiMask = roiMask;
							pRoiMask += maskOffset;

							//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
							//then set the pockels to the on voltage
							if ((*pRoiMask > 0 && !pockelPty->pockelsMaskInvert[i]) || (*pRoiMask <= 0 && pockelPty->pockelsMaskInvert[i] && (NULL == blankingMask || 0 != blankingMask[maskOffset])))
							{
								//StringCbPrintfW(message,MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d",maskOffset,*pMask);
								//LogMessage(message,VERBOSE_EVENT);
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
							}
							else
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
							}
						}
						else if (blankingPercent > 0)
						{
							if (0 != blankingMask[maskOffset])
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
							}
							else
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
							}
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
					}
					else
					{
						pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
						pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
					}
				}

				for (; sample_index < samples_per_line * 2; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsPhaseAdjustSamples + calibrationSamples > 0 ? pockelsPhaseAdjustSamples + calibrationSamples : calibrationSamples;
					//blanking region 4
					pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
					pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
				}
			}
		}

		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if ((FALSE == _pockelsTurnAroundBlank || pImgAcqPty->scanMode == TWO_WAY_SCAN) && 0 == pockelPty->pockelsLineBlankingPercentage[i] && FALSE == usePockelsMask)
		{
			pockelsPhaseAdjustSamples = 0;
		}
		else if (pockelsPhaseAdjustSamples > 0)
		{
			pockelsPhaseAdjustSamples = 0;
		}
		USHORT doLow = THORDAQ_DO_LOW;
		USHORT flybackDigitalPockels = (TRUE == _pockelsFlybackBlank) ? doLow : THORDAQ_DO_POCKELS_DIG_HIGH;
		USHORT flybackPowerPockels = (TRUE == _pockelsFlybackBlank) ? pockels_output_low_ushort : pockels_output_high_ushort;
		//80 moving average filter
		for (int j = 0; j < (int)backward_data_num + pockelsPhaseAdjustSamples; j++)
		{
			//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
			*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) |= flybackDigitalPockels;
			*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) = flybackPowerPockels;
		}

		if (pockelsPhaseAdjustSamples < 0)
		{
			for (int j = (int)backward_data_num + pockelsPhaseAdjustSamples; j < (int)backward_data_num; j++)
			{
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) |= THORDAQ_DO_POCKELS_DIG_HIGH;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) = pockels_output_high_ushort;
			}
		}

		if (pockelsPhaseAdjustSamples >= 0)
		{
			for (int j = 0; j < (int)DAC_FIFO_DEPTH; j++)
			{
				//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) |= flybackDigitalPockels;;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) = flybackPowerPockels;
			}
		}
		else
		{
			for (int j = 0; j < (int)DAC_FIFO_DEPTH; j++)
			{
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) |= THORDAQ_DO_POCKELS_DIG_HIGH;;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) = pockels_output_high_ushort;
			}
		}

		DAC_WAVEFORM_CRTL_STRUCT pockelsCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		pockelsCtrl.park_val = (1 == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsOnVoltage;
		pockelsCtrl.offset_val = pockelPty->pockelsMinVoltage[i];
		pockelsCtrl.update_rate = dac_rate;
		pockelsCtrl.flyback_samples = backward_data_num_intra;
		pockelsCtrl.output_port = (UINT16)pockelOutputChannel;
		pockelsCtrl.waveform_buffer_size = ((size_t)total_samples) * sizeof(USHORT);
		pockelsCtrl.waveformBuffer = pPockelWaveform;//new USHORT[(size_t)total_samples];
		pockelsCtrl.filterInhibit = true;
		pockelsCtrl.hSync = true;
		pockelsCtrl.enableEOFFreeze = false;

		//memcpy(dacCtrl.waveformBuffer, pPockelWaveform, ((size_t)total_samples) * sizeof(USHORT));

		waveforms[pockelOutputChannel] = pockelsCtrl;

		/*	string waveformFile = "pockelsWaveform" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_samples; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}
		*/
		//SAFE_DELETE_ARRAY(pPockelWaveform);

		SAFE_DELETE_ARRAY(roiMask);
	}

	SAFE_DELETE_ARRAY(blankingMask);
	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_samples; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
	d0Ctrl.park_val = 0;
	d0Ctrl.offset_val = 0;
	d0Ctrl.update_rate = dac_rate;
	d0Ctrl.flyback_samples = backward_data_num_intra;
	d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
	d0Ctrl.waveform_buffer_size = ((size_t)total_samples) * sizeof(USHORT);
	d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
	d0Ctrl.filterInhibit = true;
	d0Ctrl.hSync = true;
	d0Ctrl.enableEOFFreeze = false;

	waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;

	return TRUE;
}

long CThordaqResonantGalvo::BuildPockelsPowerRampWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool invert, bool flybackToStart)
{
	double dac_rate = scanInfo->dac_rate;
	int samples_per_line = 2 * scanLine->samples_idle + scanLine->samples_scan;
	int preImagingCalibrationSamples = samples_per_line * 2 * (int)(pImgAcqPty->preImagingCalibrationCycles);
	int rampExtensionSamples = samples_per_line * 2 * (int)(pImgAcqPty->imagingRampExtensionCycles);
	int postcalibrationSamples = samples_per_line * 2 * (int)pImgAcqPty->postImagingCalibrationCycles;
	int backward_data_num_intra = samples_per_line * 2 * ((int)scanInfo->backward_lines); //intra
	if (!flybackToStart)
	{
		backward_data_num_intra += rampExtensionSamples;
	}

	int backward_data_num = samples_per_line * 2 * ((int)scanInfo->backward_lines - 1) + postcalibrationSamples;
	int samples = samples_per_line * 2 * (int)scanInfo->forward_lines;
	int total_samples = preImagingCalibrationSamples + samples + backward_data_num + rampExtensionSamples + DAC_FIFO_DEPTH;
	if (!flybackToStart)
	{
		total_samples += rampExtensionSamples;
	}

	int calibrationSamples = preImagingCalibrationSamples;
	USHORT* pGalvoWaveformDigitalOutputs;
	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_samples];
	}

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	unsigned char* blankingMask = NULL;
	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		if (pockelPty->pockelsLineBlankingPercentage[i] > 0)
		{
			blankingMask = new unsigned char[width * height];
			break;
		}
	}

	for (int i = 0; i < MAX_POCKELS_CELL_COUNT; i++)
	{
		unsigned short* roiMask = NULL;
		const long BYTES_PER_PIXEL = 2;
		if (_pockelsMaskSize[i] == width * height * BYTES_PER_PIXEL)
		{
			roiMask = new unsigned short[width * height];

			long horizontalFlip = pImgAcqPty->horizontalFlip;
			LSMGRRotationAngle rotation = (LSMGRRotationAngle)_rotationAnglePosition;
			if (FALSE == horizontalFlip)
			{
				switch ((LSMGRRotationAngle)_rotationAnglePosition)
				{
				case LSMGRRotationAngle::DEG_270: rotation = LSMGRRotationAngle::DEG_90; break;
				case LSMGRRotationAngle::DEG_90: rotation = LSMGRRotationAngle::DEG_270; break;
				default:
					break;
				}
			}

			if (invert)
			{
				switch ((LSMGRRotationAngle)_rotationAnglePosition)
				{
				case LSMGRRotationAngle::DEG_270: rotation = LSMGRRotationAngle::DEG_90; break;
				case LSMGRRotationAngle::DEG_90: rotation = LSMGRRotationAngle::DEG_270; break;
				case LSMGRRotationAngle::DEG_180: rotation = LSMGRRotationAngle::DEG_0; break;
				case LSMGRRotationAngle::DEG_0: rotation = LSMGRRotationAngle::DEG_180; break;
				default:
					break;
				}

				horizontalFlip = horizontalFlip == FALSE ? TRUE : FALSE;
			}

			ManipulateAndCopyImage((unsigned short*)_pPockelsMask[i], roiMask, width, height, 1, 1, TRUE, rotation, horizontalFlip);
		}

		if (_pockelsEnable[i] == FALSE || pockelPty->pockelsPowerLevel[i] == 0)
		{
			SAFE_DELETE_ARRAY(roiMask);
			continue;
		}

		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + i)];

		double pockelsOnVoltage = pockelPty->pockelsMinVoltage[i] + (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * pockelPty->pockelsPowerLevel[i];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[i]) / GALVO_RESOLUTION));
		USHORT pockels_output_high_start_ushort = pockels_output_high_ushort;
		double step = (pockelPty->pockelsMaxVoltage[i] - pockelPty->pockelsMinVoltage[i]) * (pockelPty->pockelsPowerLevel2[i] - pockelPty->pockelsPowerLevel[i]) / (double)samples;

		USHORT* pPockelWaveform;
		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			pPockelWaveform = waveforms[pockelOutputChannel].waveformBuffer;
		}
		else
		{
			pPockelWaveform = new USHORT[total_samples];
		}
		memset(pPockelWaveform, pockels_output_low_ushort, total_samples * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		if (FALSE == _pockelsFlybackBlank)
		{
			for (int i = 0; i < total_samples; ++i)
			{
				pGalvoWaveformDigitalOutputs[i] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelWaveform[i] = pockels_output_high_ushort;
			}
		}

		long usePockelsMask = FALSE;

		//if the mode is enabled, the pointer is valid, and the offset not being applied in X
		if (pockelPty->pockelsMaskEnable[i] && _pPockelsMask[i] && (pImgAcqPty->offsetX == 0))
		{
			const long BYTES_PER_PIXEL = 2;
			//determine if the size of the mask matches the pockels waveformBuffer output
			if (_pockelsMaskSize[i] == width * height * BYTES_PER_PIXEL)
			{
				usePockelsMask = TRUE;
			}
		}

		double blankingPercent = pockelPty->pockelsLineBlankingPercentage[i];
		if (blankingPercent > 0)
		{
			int k = 0;
			for (int j = 0; j < height; ++j)
			{
				for (int m = 0; m < width; ++m)
				{
					if ((int)round(width * blankingPercent) <= m && (int)round(width * (1 - blankingPercent)) > m)
					{
						blankingMask[k] = 1;
					}
					else
					{
						blankingMask[k] = 0;
					}
					++k;
				}
			}
		}

		int s = 1;
		int pockelsDelaySamples = static_cast<int>(pockelPty->pockelsDelayUS[i] / 1000000.0 * dac_rate);
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if (FALSE == _pockelsTurnAroundBlank && 0 == pockelPty->pockelsLineBlankingPercentage[i] && FALSE == usePockelsMask)
		{
			for (int i = (int)calibrationSamples; i < samples_per_line * 2 * (int)scanInfo->forward_lines + pockelsDelaySamples + (int)calibrationSamples; i++)
			{
				int sample_index = i + pockelsDelaySamples >= 0 ? i + pockelsDelaySamples : 0; //pockel cell index ptr
				pGalvoWaveformDigitalOutputs[sample_index] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelWaveform[sample_index] = pockels_output_high_ushort;
				pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
				++s;
			}
		}
		else
		{
			for (int j = 0; j < scanInfo->forward_lines; j++)
			{
				int sample_index = j > 0 || pockelsDelaySamples > 0 ? pockelsDelaySamples : 0; //pockel cell index ptr
				//There are 4 blanking regions that must be accomodated
				//1-Beginning of a front line scan
				//2-End of a front line scan to begining Of back line scan
				//3-End of a back line scan
				//4-Backscan

				int beginningOfFrontLineScan = pockelsDelaySamples;
				int endOfFrontLineScan = static_cast<int>(samples_per_line);
				int endOfBackLineScan = static_cast<int>(samples_per_line * 2);

				/*StringCbPrintfW(message,MSG_SIZE, L"ThorGR pockels blank percentage spl %d %f phaseAdjustment %f blanking samples %d",samples_per_line, pockelPty->pockelsLineBlankingPercentage[i],_pockelsPhaseAdjustMicroSec, blanking_samples);
				LogMessage(message,ERROR_EVENT);*/

				double const W1 = (width - 1);
				double const PHASE_SHIFT = M_PI * pImgAcqPty->pockelsBlankingPhaseShiftPercent / 100;

				///        |>-----beginningOfFrontLineScan-----------------endOfFrontLineScan--------->-|
				///                                                                                    |
				///        |<-----endOfBackLineScan-----------------------beginingOfBackLineScan-----<-|


				for (; sample_index < beginningOfFrontLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsDelaySamples + calibrationSamples > 0 ? pockelsDelaySamples + calibrationSamples : calibrationSamples;

					pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
					pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				for (; sample_index < endOfFrontLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsDelaySamples + calibrationSamples > 0 ? pockelsDelaySamples + calibrationSamples : calibrationSamples;

					//To account for the sinusoidal movement of the resonant scanner
					//the x offset on the mask is calculated using a cosine and shifting it by 90deg
					//and adding 1 to start at 0 and increase as sample_index increases "(cos(PI + PI * sample_index / (samples) + C1) + 1) / 2.0"
					//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
					long xOffset = 0;
					if (FALSE == pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(floor(W1 * (cos(M_PI + M_PI * (sample_index + sample_offset) / (samples_per_line)+PHASE_SHIFT) + 1) / 2.0 + 0.5));
					}
					else
					{
						//if there is a horizontal flip, then start at the end of the line
						xOffset = static_cast<long>(floor(W1 * (cos(M_PI + M_PI * ((samples_per_line - 1) - (sample_index + sample_offset)) / (samples_per_line - 1) - PHASE_SHIFT) + 1) / 2.0 + 0.5));
					}

					long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? j * (width * 2) + xOffset : j * (width)+xOffset;

					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					if (TRUE == usePockelsMask)
					{
						unsigned short* pRoiMask = roiMask;
						pRoiMask += maskOffset;

						//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
						//then set the pockels to the on voltage
						if ((0 != *pRoiMask && !pockelPty->pockelsMaskInvert[i]) || (0 == *pRoiMask && pockelPty->pockelsMaskInvert[i] && (NULL == blankingMask || 0 != blankingMask[maskOffset])))
						{
							//StringCbPrintfW(message, MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d", maskOffset, *pRoiMask);
							//LogMessage(message, VERBOSE_EVENT);
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
						}
					}
					else if (blankingPercent > 0)
					{
						if (0 != blankingMask[maskOffset])
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
						}
					}
					else
					{
						pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
						pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
					}
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				for (; sample_index < endOfBackLineScan; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsDelaySamples + calibrationSamples > 0 ? pockelsDelaySamples + calibrationSamples : calibrationSamples;

					//only enable the backscan if two way mode is enabled
					if (pImgAcqPty->scanMode == TWO_WAY_SCAN)
					{
						//To account for the sinusoidal movement of the resonant scanner
						//the x offset on the mask is calculated using a cosine and shifting it by 90deg
						//and adding 1 to start at 0 and increase as j increases "cos(PI + PI* (j -  (_subtriggerLength/2))/(_subtriggerLength/2) + C1) + 1)/2.0 "
						//the phase shift "PHASE_SHIFT" is needed to accomodate for the delay in the pockels modulator
						//Here, because its the backward scan, we want to start at the end of the x for the line and then move to the left until
						//we reach the begining of the line. Then we go back to the forward scan
						long xOffset = 0;
						if (FALSE == pImgAcqPty->horizontalFlip)
						{
							xOffset = static_cast<long>(ceil(W1 * (cos(M_PI + M_PI * ((sample_index + sample_offset) - samples_per_line) / samples_per_line + PHASE_SHIFT) + 1) / 2.0 - 0.5));
						}
						else
						{
							//if there is a horizontal flip, then start at the end of the line
							xOffset = static_cast<long>(ceil(W1 * (cos(M_PI + M_PI * ((samples_per_line)-((sample_index + sample_offset) - (samples_per_line))) / (samples_per_line)-PHASE_SHIFT) + 1) / 2.0 - 0.5));
						}

						long maskOffset = j * (width * 2) + (width * 2 - xOffset);

						if (maskOffset < 0)
						{
							maskOffset = 0;
						}

						if (TRUE == usePockelsMask)
						{
							unsigned short* pRoiMask = roiMask;
							pRoiMask += maskOffset;

							//if mask value is greater than 0 and invert is off, or mask value is 0 (or less) and invert is on
							//then set the pockels to the on voltage
							if ((*pRoiMask > 0 && !pockelPty->pockelsMaskInvert[i]) || *pRoiMask <= 0 && pockelPty->pockelsMaskInvert[i])
							{
								//StringCbPrintfW(message, MSG_SIZE, L"ThorLSMCam Mask offset %d mask value %d", maskOffset, *pRoiMask);
								//LogMessage(message, VERBOSE_EVENT);
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
							}
							else
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
							}
						}
						else if (blankingPercent > 0)
						{
							if (0 != blankingMask[maskOffset])
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
							}
							else
							{
								pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
								pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
							}
						}
						else
						{
							pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_POCKELS_DIG_HIGH;
							pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_high_ushort;
						}
					}
					else
					{
						pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
						pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
					}
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				for (; sample_index < samples_per_line * 2; ++sample_index)
				{
					int sample_offset = j * 2 * samples_per_line + sample_index + pockelsDelaySamples + calibrationSamples > 0 ? pockelsDelaySamples + calibrationSamples : calibrationSamples;

					//blanking region 4
					pGalvoWaveformDigitalOutputs[j * 2 * samples_per_line + (sample_index + sample_offset)] |= THORDAQ_DO_LOW;
					pPockelWaveform[j * 2 * samples_per_line + (sample_index + sample_offset)] = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
			}
		}

		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if ((FALSE == _pockelsTurnAroundBlank || pImgAcqPty->scanMode == TWO_WAY_SCAN) && 0 == pockelPty->pockelsLineBlankingPercentage[i] && FALSE == usePockelsMask)
		{
			pockelsDelaySamples = 0;
		}
		else if (pockelsDelaySamples > 0)
		{
			pockelsDelaySamples = 0;
		}

		USHORT doLow = THORDAQ_DO_LOW;
		USHORT flybackDigitalPockels = (TRUE == _pockelsFlybackBlank) ? doLow : THORDAQ_DO_POCKELS_DIG_HIGH;
		USHORT flybackPowerPockels = (TRUE == _pockelsFlybackBlank) ? pockels_output_low_ushort : pockels_output_high_ushort;

		//80 moving average filter
		for (int j = 0; j < (int)backward_data_num + pockelsDelaySamples; j++)
		{
			//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
			*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) |= flybackDigitalPockels;;
			*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + j + calibrationSamples)) = flybackPowerPockels;
		}

		if (pockelsDelaySamples < 0)
		{
			for (int j = (int)backward_data_num + pockelsDelaySamples; j < (int)backward_data_num; j++)
			{
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) |= THORDAQ_DO_POCKELS_DIG_HIGH;;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) = pockels_output_high_ushort;
			}
		}

		if (pockelsDelaySamples >= 0)
		{
			for (int j = 0; j < (int)DAC_FIFO_DEPTH; j++)
			{
				//*(pGalvoWaveformX+(forwardLines*samples_two_lines+ i)) = max(0,min( USHRT_MAX - Waveform_Start[0], static_cast<USHORT>(amp_offset_X - (double)pad_amp * cos(1.5 * M_PI + pre_flayback_step*double(i)) - half_P2P_amp_X) - Waveform_Start[0]));
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) |= flybackDigitalPockels;;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) = flybackPowerPockels;
			}
		}
		else
		{
			for (int j = 0; j < (int)DAC_FIFO_DEPTH; j++)
			{
				*(pGalvoWaveformDigitalOutputs + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) |= THORDAQ_DO_POCKELS_DIG_HIGH;;
				*(pPockelWaveform + (int)(samples_per_line * 2 * scanInfo->forward_lines + backward_data_num + j + calibrationSamples)) = pockels_output_high_ushort;
			}
		}

		DAC_WAVEFORM_CRTL_STRUCT pockelsCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		pockelsCtrl.park_val = (1 == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[i] : pockelsOnVoltage;
		pockelsCtrl.offset_val = pockelPty->pockelsMinVoltage[i];
		pockelsCtrl.update_rate = dac_rate;
		pockelsCtrl.flyback_samples = backward_data_num_intra;
		pockelsCtrl.output_port = (UINT16)pockelOutputChannel;
		pockelsCtrl.waveform_buffer_size = ((size_t)total_samples) * sizeof(USHORT);
		pockelsCtrl.waveformBuffer = pPockelWaveform;//new USHORT[(size_t)total_samples];
		pockelsCtrl.filterInhibit = true;
		pockelsCtrl.hSync = true;
		pockelsCtrl.enableEOFFreeze = false;

		//memcpy(dacCtrl.waveformBuffer, pPockelWaveform, ((size_t)total_samples) * sizeof(USHORT));

		waveforms[pockelOutputChannel] = pockelsCtrl;

		/*	string waveformFile = "pockelsWaveform" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_samples; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}
		*/
		//SAFE_DELETE_ARRAY(pPockelWaveform);
		SAFE_DELETE_ARRAY(roiMask);
	}
	SAFE_DELETE_ARRAY(blankingMask);

	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_samples; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
	d0Ctrl.park_val = 0;
	d0Ctrl.offset_val = 0;
	d0Ctrl.update_rate = dac_rate;
	d0Ctrl.flyback_samples = backward_data_num_intra;
	d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
	d0Ctrl.waveform_buffer_size = ((size_t)total_samples) * sizeof(USHORT);
	d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
	d0Ctrl.filterInhibit = true;
	d0Ctrl.hSync = true;
	d0Ctrl.enableEOFFreeze = false;

	waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;

	return TRUE;
}

/************************************************************************************************
* @fn	UINT CThordaqResonantGalvo::StartFrameAcqProc(LPVOID instance)
*
* @brief	Start  Acquisition Thread.
* @param 	instance	  	Acquisition Thread instance.
* @return	A uint.
**************************************************************************************************/
UINT CThordaqResonantGalvo::StartFrameAcqProc(LPVOID instance)
{
	int				frameWritten = 0;
	THORDAQ_STATUS	status = STATUS_SUCCESSFUL;
	bool            noErrorOnAcquisition = true;
	long long		targetFrames = static_cast<long long>(_daqAcqCfg.imageCtrl.frameCnt);// do later
	std::bitset<sizeof(size_t)* CHAR_BIT> channel_bitset(_daqAcqCfg.imageCtrl.channel);
	ULONG			channelCount = static_cast<ULONG>(channel_bitset.count());
	ULONG frameSizeRead = _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
	int frameSizeReadByte = frameSizeRead * sizeof(USHORT);
	ULONG			transferSize = static_cast<ULONG>(frameSizeReadByte * _daqAcqCfg.imageCtrl.frameNumPerTransfer * MAX_CHANNEL_COUNT);
	UINT64           size;
	long chFrameNum[4] = { 0, 0, 0, 0 };
	int targetFrmNum = 0;
	const double EXTRA_DELAY = 200;
	double  crsMeasuredFrequency = CThordaqResonantGalvo::GetInstance()->_current_resonant_scanner_frequency;
	double regular_timeout = 10.0 * MS_TO_SEC * _daqAcqCfg.imageCtrl.imgVSize / crsMeasuredFrequency * _daqAcqCfg.imageCtrl.frameNumPerSec + EXTRA_DELAY;
	double hardware_trigger_timeout = _triggerWaitTimeout * MS_TO_SEC;
	double timeout = regular_timeout;
	BOOL hardware_timeout_enable = FALSE;
	BOOL is_hardware_captured = FALSE;
	ULONG32 frame_count = 0;
	ULONG transferredFrames = 0;
	BOOL isPartialData = FALSE;
	int32 error = 0, retVal = 0;
	if (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _daqAcqCfg.triggerSettings.triggerMode)
	{
		hardware_timeout_enable = TRUE;
	}
	ThordaqErrChk(L"ThorDAQAPIStartAcquisition", retVal = ThorDAQAPIStartAcquisition(_DAQDeviceIndex));
	ThorDAQAPIProgressiveScan(_DAQDeviceIndex, FALSE);

	ULONG32 bank = 0;
	long long frame_left = targetFrames - 1;
	do
	{
		frame_left = targetFrames - 1 - _indexOfLastCompletedFrame;
		// For debugging, print the index of frame counts
		/*(L"ThorDAQAPIGetTotalFrameCount", status = ThorDAQAPIGetTotalFrameCount(_DAQDeviceIndex,frame_count));
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoRes \nframe_count: %d \n frame_left: %d \n _indexOfLastCompletedFrame: %d, \nregular_timeout: %f", frame_count, frame_left, _indexOfLastCompletedFrame, regular_timeout);
		CThordaqResonantGalvo::GetInstance()->LogMessage(errMsg,ERROR_EVENT);*/
		BOOL isLastTransfer = frame_left <= _daqAcqCfg.imageCtrl.frameNumPerTransfer;
		ULONG framesToTransfer = isLastTransfer ? (ULONG)frame_left : _daqAcqCfg.imageCtrl.frameNumPerTransfer;
		timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;

		//we only need to copy a finite amount of frames so set the size accordingly
		size = isLastTransfer ? (UINT64)frame_left * MAX_CHANNEL_COUNT * frameSizeReadByte : transferSize;

		//get the the frames (framesToTransfer)
		ThordaqErrChk(L"ThorDAQAPIReadFrames", status = ThorDAQAPIReadFrames(_DAQDeviceIndex, &size, _BufferContiguousArray[0], timeout, framesToTransfer, isLastTransfer, isPartialData));
		if (status == STATUS_SUCCESSFUL && size > 0) // read buffer successfully
		{
			if (hardware_timeout_enable) is_hardware_captured = TRUE;

			// Notify WaitForHardwareTrigger the board did receive a hardware trigger 
			SetEvent(_hHardwareTriggerInEvent);
		}
		else if (status == STATUS_READ_BUFFER_TIMEOUT_ERROR)
		{
			noErrorOnAcquisition = false;
			SetEvent(_hTriggerTimeout);
			break;
		}
		else if (status == STATUS_ACQUISITION_ABORTED)
		{
			break;
		}
		else
		{
			noErrorOnAcquisition = false;
			break;
		}

		if ((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			break;
		}

		transferredFrames = framesToTransfer;
		frame_count += transferredFrames;
		if (status == STATUS_SUCCESSFUL && noErrorOnAcquisition)
		{
			hardware_timeout_enable = FALSE; // only used for first frame
			ResetEvent(_hTriggerTimeout);
			auto processedBuf = _pDataProcessor->ProcessBuffer(_BufferContiguousArray, transferredFrames);

			if ((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
			{
				break;
			}

			if (1 < _imgAcqPty_Pre.averageNum && _imgAcqPty_Pre.averageMode == FRM_CUMULATIVE_MOVING && frame_count > 0)
			{
				USHORT* bPtr = NULL; //Identifier pointer for history buffer
				USHORT* tPtr = NULL; //Identifier pointer for captured data

				for (size_t t = 0; t < transferredFrames; ++t)
				{
					vector<ProcessedFrame*> buffermap;
					//write history buffer to circular buffer:
					//when in live mode there is only one image at a time
					vector<vector<ProcessedFrame*>> historyBufVector;
					for (int a = 0; a < _pHistoryBuf.size(); ++a)
					{
						size_t processedFrameLength = _pHistoryBuf[a]->GetDataLengthPerChannel();
						//write to history buffer:
						bPtr = _pHistoryBuf[a]->Data;
						long avg = (long)(frame_count >= (UINT64)_imgAcqPty_Pre.averageNum ? _imgAcqPty_Pre.averageNum : frame_count - (transferredFrames - t - 1));
						double factor1 = (1 / (double)(avg));
						double factor2 = ((double)avg - 1) / (double)(avg);
						tPtr = (USHORT*)processedBuf.at(t)[a]->Data; // Get the pointer
						for (size_t chID = 0; chID < channelCount; ++chID)
						{
							for (size_t p = 0; p < processedFrameLength; ++p)
							{
								size_t index = chID * processedFrameLength + p;
								*(bPtr + index) = static_cast<USHORT>((*(bPtr + index)) * factor2 + (*(tPtr + index)) * factor1);
							}
						}

						buffermap.push_back(_pHistoryBuf[a]);
					}
					historyBufVector.push_back(buffermap);
					_pFrmBuffer->WriteFrames(historyBufVector, 1);
				}
			}
			else
			{
				_pFrmBuffer->WriteFrames(processedBuf, transferredFrames);
			}
			_indexOfLastCompletedFrame += transferredFrames;

		}
	} while ((_indexOfLastCompletedFrame < (targetFrames - 1)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && noErrorOnAcquisition);
	ULONG32 clockStatus = 0;
	//TODO: make sure a similar check is not necessary for the GR
	//ThordaqErrChk(L"ThorDAQAPIGetExternClockStatus", status = ThorDAQAPIGetExternClockStatus(_DAQDeviceIndex, clockStatus));
	//if (!noErrorOnAcquisition && _daqAcqCfg.imageCtrl.threePhotonMode == TRUE
	//	&& status == THORDAQ_STATUS::STATUS_SUCCESSFUL
	//	&& clockStatus == 0)
	//{
	//	MessageBox(NULL, L"Laser SYNC Error", L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
	//}
	//done capture:
	((CThordaqResonantGalvo*)instance)->StopDaqBrd();
	CThordaqResonantGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StartFrameAcqProc thread exiting", VERBOSE_EVENT);

	SetEvent(_hThreadStopped);
	return 0;
}



long CThordaqResonantGalvo::CorrectPreludeImageDistortion(const USHORT* pData, USHORT* pDataDest, long channelNum, long channelEnable, long width, long height)
{
	const long imageSize = width * height;

	for (int c = 0, c2 = 0, k = 0; c < MAX_CHANNEL_COUNT; ++c, ++c2)
	{
		if ((channelEnable & (0x0001 << c)) != 0x0000)
		{
			const unsigned short* pixels = pData + imageSize * c2;

			unsigned short* pixelsDup = pDataDest + imageSize * k;

			ImageDistortionCorrection->CorrectPreludeImageDistortion(pixels, pixelsDup, width, height);
			++k;
		}
	}

	return TRUE;
}

void CThordaqResonantGalvo::PrepareImageDistortionCorrectionParameters(ImgAcqPty* pImgAcqPty, long channelCount)
{
	ProcessedFrame* tempFrame = new ProcessedFrame((UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, (UINT)channelCount, 0, (UINT)pImgAcqPty->rx, (UINT)pImgAcqPty->ry, 0, 0);
	if (tempFrame == NULL)
	{
		return;
	}
	_pTempBufCorrection.push_back(tempFrame);

	int width = pImgAcqPty->rx;
	int height = pImgAcqPty->ry;

	const double fs = _scanLensFocalLength; // Scan Lens focal Length
	const double XAngleMax = pImgAcqPty->ImageDistortionCorrectionCalibrationXAngleMax; // degree optical half angle max
	const double YAngleMax = pImgAcqPty->ImageDistortionCorrectionCalibrationYAngleMax; // degree optical half angle max
	const double AngleIn = pImgAcqPty->ImageDistortionCorrectionCalibrationGalvoTiltAngle; // Galvo Tilt angle
	ImageDistortionCorrection->SetImageDistortionCorrectionParameters(width, height, XAngleMax, YAngleMax, AngleIn, fs);

}
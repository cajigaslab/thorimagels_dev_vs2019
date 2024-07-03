#include "stdafx.h"
#include "ThorDAQGalvoGalvo.h"
#include "thordaqGalvoGalvoSetupXML.h"


FrameCirBuffer* CThorDAQGalvoGalvo::_pFrmBuffer = nullptr;
UCHAR* CThorDAQGalvoGalvo::_pHistoryBuf = nullptr;
UCHAR* CThorDAQGalvoGalvo::_pHistoryProgressiveBuf = nullptr;
UCHAR* CThorDAQGalvoGalvo::_pProgressiveBuf[] = { NULL, NULL, NULL, NULL };
IDataProcessor* CThorDAQGalvoGalvo::_pDataProcessor = nullptr;
UCHAR* CThorDAQGalvoGalvo::_BufferContiguousArray[] = { NULL, NULL, NULL, NULL };
double CThorDAQGalvoGalvo::_frameRate = 1;
long CThorDAQGalvoGalvo::_shiftArray[256];
std::atomic<long long> CThorDAQGalvoGalvo::_index_of_last_written_frame = 0;
std::atomic<long long>  CThorDAQGalvoGalvo::_index_of_last_read_frame = 0;
IMAGING_CONFIGURATION_STRUCT CThorDAQGalvoGalvo::_daqAcqCfg = IMAGING_CONFIGURATION_STRUCT();
long CThorDAQGalvoGalvo::_imageStatus = StatusType::STATUS_BUSY;
UINT64 CThorDAQGalvoGalvo::_powerRampCurrentIndex = 0;
ThorDAQZWaveformParams CThorDAQGalvoGalvo::_fastZWaveformParams = ThorDAQZWaveformParams();
long CThorDAQGalvoGalvo::_useBuiltZWaveform = FALSE;
UINT64 CThorDAQGalvoGalvo::_fastZCurrentIndex = 0;
ScanStruct  CThorDAQGalvoGalvo::_scan_info;
ScanLineStruct  CThorDAQGalvoGalvo::_scanLine;
std::vector<PockelsImagePowerRampStruct> CThorDAQGalvoGalvo::_pockelsImagePowerRampVector[MAX_POCKELS_CELL_COUNT];
std::vector<double> CThorDAQGalvoGalvo::_pockelsResponsePowerLevels[MAX_POCKELS_CELL_COUNT];
std::unique_ptr<ImageWaveformBuilderDLL> ImageWaveformBuilder(new ImageWaveformBuilderDLL(L".\\Modules_Native\\GeometryUtilitiesCPP.dll"));
ThorDAQDMAbuffer S2MMdmaBuffer(MAX_PIXEL_X* MAX_PIXEL_Y * sizeof(USHORT)* MAX_CHANNEL_COUNT* MAX_NUMBER_OF_PLANES);

void CThorDAQGalvoGalvo::SetupDataMaps()
{
	unsigned int i;
	{
		//folding mapping for positive and negative amplifiers
		//datamap should reflect 14bit resolution of the digitizer

		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapIndependent[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (i = 8192; i < 65536; i++)
		{
			_datamapIndependent[i] = 0;
		}
	}

	{
		//16 bit mapping with most significant data in positive polarity
		//positive voltage mapping
		for (i = 0; i < 16384; i++)
		{
			_datamapPositiveSigned[i] = static_cast<USHORT>(i - 8192);
		}
		//positive voltage mapping
		for (i = 16384; i < 65536; i++)
		{
			_datamapPositiveSigned[i] = 8192;
		}
	}

	{
		//16 bit mapping with most significant data in negative polarity
		//negative voltage mapping
		for (i = 0; i < 16384; i++)
		{
			_datamapNegativeSigned[i] = static_cast<USHORT>((16383 - i) - 8192);
		}
		for (i = 16384; i < 65536; i++)
		{
			_datamapNegativeSigned[i] = static_cast<USHORT>(-8192);
		}
	}

	{
		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapPositiveUnsigned[i] = 0;
		}

		//positive voltage mapping
		for (i = 8192; i < 16384; i++)
		{
			_datamapPositiveUnsigned[i] = (i - 8192) * 2;
		}

		//positive voltage mapping
		for (i = 16384; i < 65536; i++)
		{
			_datamapPositiveUnsigned[i] = 16382;
		}
	}

	{
		//negative voltage mapping
		for (i = 0; i < 8192; i++)
		{
			_datamapNegativeUnsigned[i] = (8191 - i) * 2;
		}

		//positive voltage mapping
		for (i = 8192; i < 65536; i++)
		{
			_datamapNegativeUnsigned[i] = 0;
		}
	}
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
*
* @brief	Configure thordaq settings
* @param [in,out]	pImgAcqPty	  	Identifier of Image Acquisition Struct.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::ConfigAcqSettings(ImgAcqPty* pImgAcqPty)
{
	long ret = TRUE;
	int32 error = 0, retVal = 0;

	//only sleep if in preview mode where settings can quickly change
	if (pImgAcqPty->triggerMode == ICamera::SW_FREE_RUN_MODE)
	{
		Sleep(50); //Implemented to stop ThorDAQ GG from Scrolling. Tested that 20ms to be the min allowable delay before scrolling occurs.
	}

	for (auto& waveformCtrl : _daqAcqCfg.dacCtrl)
	{
		SAFE_DELETE_ARRAY(waveformCtrl.second.waveformBuffer);
	}
	_daqAcqCfg.dacCtrl.clear();

	for (auto& waveformCtrl : _daqAcqCfg.dacCtrl2)
	{
		SAFE_DELETE_ARRAY(waveformCtrl.second.waveformBuffer);
	}
	_daqAcqCfg.dacCtrl2.clear();

	//initialize the struct
	_daqAcqCfg = IMAGING_CONFIGURATION_STRUCT();

	double dwell_time = pImgAcqPty->dwellTime / 1000000.0;
	_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::INTERNAL_80MHZ_REF;
	_daqAcqCfg.imageCtrl.clockRate = DEFAULT_INTERNALCLOCKRATE;

	int totalYLines = pImgAcqPty->pixelY;
	int averageLines = 1;
	if (pImgAcqPty->lineAveragingEnable == TRUE)
	{
		totalYLines *= pImgAcqPty->lineAveragingNumber;
		averageLines = pImgAcqPty->lineAveragingNumber;
	}

	// system runs at external clock mode
	if (pImgAcqPty->clockSource != INTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.clock_source = CLOCK_SOURCE::EXTERNAL_CLOCK_SOURCE; // works on external laser sync mode
		// 2P mode // need to add in the future
		if (pImgAcqPty->threePhotonModeEnable)
		{
			ULONG32 clock_rate = 0;
			ULONG32 clock_ref = 0;
			ULONG32 mode = 1; //threePhoton mode

			ULONG32 clockStatus = 0;
			ThordaqErrChk(L"ThorDAQAPIGetExternClockStatus", retVal = ThorDAQAPIGetExternClockStatus(_DAQDeviceIndex, clockStatus));
			// if external clock measure fails or 3P laser signal is not in the range, break.
			if (pImgAcqPty->clockRateExternal <= MIN_3PCLOCKRATE
				|| pImgAcqPty->clockRateExternal > MAX_3PCLOCKRATE
				|| retVal != THORDAQ_STATUS::STATUS_SUCCESSFUL
				|| clockStatus == 0)
			{
				MessageBox(NULL, L"Laser SYNC Error", L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
				return FALSE;
			}

			//for 3P acquisition it is important to measure the external clock right before acquisition.
			if (FALSE == _useExternalBoxFrequency3P) // For debugging purposes, allow user to type in the frequency instead of measuring it.
			{
				ThordaqErrChk(L"ThorDAQAPIMeasureExternClockRate", retVal = ThorDAQAPIMeasureExternClockRate(_DAQDeviceIndex, clock_rate, clock_ref, mode));
				if (retVal == THORDAQ_STATUS::STATUS_SUCCESSFUL)
				{
					pImgAcqPty->clockRateExternal = clock_rate;
					pImgAcqPty->maxSampleRate = static_cast<long>(clock_ref * 2);
					_imgAcqPty.clockRateExternal = pImgAcqPty->clockRateExternal;
					_imgAcqPty.maxSampleRate = pImgAcqPty->maxSampleRate;
					Sleep(100);
				}
			}

			_daqAcqCfg.imageCtrl.clockRate = pImgAcqPty->clockRateExternal;
			_daqAcqCfg.imageCtrl.clockReference = 2 * clock_ref;
			const double CLOCK_PRECISION_200MHZ = 5.0; //5ns
			//convert to nanoseconds	

			double tempDwellns = 1000000.0 / static_cast<double>(_daqAcqCfg.imageCtrl.clockRate - 80);
			_minDwellTime = _dwellTimeStep = tempDwellns;

			if (tempDwellns < _displayedDwellTime)
			{
				_ratio = static_cast<long>(ceil(_displayedDwellTime / tempDwellns - 0.5));
				tempDwellns *= static_cast<double>((double)_ratio + 12.0 / (double)pImgAcqPty->pixelX);
			}
			else
			{
				_ratio = 1;
			}

			if (FALSE == pImgAcqPty->FIR1ManualControlenable)
			{
				if (pImgAcqPty->numberOfPlanes > 1)
				{
					for (int channel = 0; channel < MAX_CHANNEL_COUNT; channel++)
					{
						for (int t = 0; t < FIR_FILTER_TAP_COUNT; t++)
						{
							//for the second FIR filter only turn on 1 tap when in multiplane mode
							if (7 == t)
							{
								pImgAcqPty->FIRFilters[1][channel][t] = 1;
							}
							else
							{
								pImgAcqPty->FIRFilters[1][channel][t] = 0;
							}
							_imgAcqPty.FIRFilters[1][channel][t] = pImgAcqPty->FIRFilters[1][channel][t];
						}
					}
				}
				else
				{
					//Set second FIR filter tap values when 3P is enabled for multiple pulse measurement.
					// When dwell time is increased it should increase the number of pulses it is going to acquire. Current limit is 16 pulses.
					long firTapsToSet = min(_ratio, FIR_FILTER_TAP_COUNT);
					long initialTapIndex = FIR_FILTER_TAP_COUNT / 2;
					long taps;

					for (int channel = 0; channel < MAX_CHANNEL_COUNT; channel++)
					{
						taps = 0;
						for (int i = 0; i < FIR_FILTER_TAP_COUNT; i++)
						{
							// Set the FIR number of enabled taps same as the number of pulses that we want to average or sum. 
							double filterValue = (TRUE == _sumPulsesPerPixel) ? 1.0 : 1.0 / firTapsToSet;
							initialTapIndex = (FIR_FILTER_TAP_COUNT / 2) - (int)ceil(firTapsToSet / 2.0);
							pImgAcqPty->FIRFilters[1][channel][i] = (i >= initialTapIndex && i < initialTapIndex + firTapsToSet) ? filterValue : 0;
							_imgAcqPty.FIRFilters[1][channel][i] = pImgAcqPty->FIRFilters[1][channel][i];
						}
					}
				}
			}

			//convert tempDwellns to seconds
			dwell_time = tempDwellns / 1000000.0;
			_imgAcqPty.dwellTime = pImgAcqPty->dwellTime = tempDwellns;
		}
	}

	bool oneWayLineScan = false;//TODO: test without this and remove if it works well// pImgAcqPty->pixelY == 1 && TWO_WAY_SCAN_MODE != pImgAcqPty->scanMode && ICamera::LSMAreaMode::POLYLINE != pImgAcqPty->areaMode;

	//TODO: test with normal acquisition (setting VSize to 1 instead of 2 for oneway scan)
	//always make line scan two-way backward line will be discarded in postprocessing
	long vSize = totalYLines;// oneWayLineScan ? 2 : static_cast<USHORT>(pImgAcqPty->pixelY);

	_daqAcqCfg.imageCtrl.system_mode = SYSTEM_MODE::INTERNAL_GALVO_GALVO;
	_daqAcqCfg.imageCtrl.channel = static_cast<USHORT>(pImgAcqPty->channel);
	_daqAcqCfg.imageCtrl.imgHSize = static_cast<USHORT>(pImgAcqPty->pixelX);
	_daqAcqCfg.imageCtrl.imgVSize = static_cast<USHORT>(vSize);
	_daqAcqCfg.imageCtrl.scanMode = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? SCAN_MODES::BIDIRECTION_SCAN : SCAN_MODES::UNIDIRECTION_SCAN;
	_daqAcqCfg.imageCtrl.scanDir = (pImgAcqPty->horizontalFlip == FALSE) ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;
	_daqAcqCfg.imageCtrl.defaultMode = FALSE;
	_daqAcqCfg.imageCtrl.isPreviewImaging = ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode;

	_daqAcqCfg.imageCtrl.digitalLinesConfig = _thorDAQImagingDigitalLinesConfig;

	//only allow multiplane and changing downsampling rate for 3photon acquisitions and setup the FIR filters accordingly
	if (TRUE == pImgAcqPty->threePhotonModeEnable)
	{
		_daqAcqCfg.imageCtrl.numPlanes = pImgAcqPty->numberOfPlanes >= 1 ? static_cast<USHORT>(pImgAcqPty->numberOfPlanes) : 1;
		_daqAcqCfg.imageCtrl.ddsEnable = _ddsClockEnable;
		_daqAcqCfg.imageCtrl.enableDownsampleRateChange = pImgAcqPty->enableDownsamplingRateChange;
		_daqAcqCfg.imageCtrl.downSampleRate = pImgAcqPty->threePhotonDownsamplingRate;
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"3P Downsample Parameters Updated in Lower Level ThorDAQGalvoGalvoImage->thordaq  Enable Downsample: %d  DS Rate: %d", _daqAcqCfg.imageCtrl.enableDownsampleRateChange, _daqAcqCfg.imageCtrl.downSampleRate);
		LogMessage(errMsg, VERBOSE_EVENT);
	}
	else
	{
		_daqAcqCfg.imageCtrl.numPlanes = 1;
		_daqAcqCfg.imageCtrl.ddsEnable = false;
		_daqAcqCfg.imageCtrl.enableDownsampleRateChange = 0;
	}

	for (int i = 0; i < 4; ++i)
	{
		_daqAcqCfg.imageCtrl.ADCGain[i] = pImgAcqPty->ADCGain[i];
	}

	//TODO ThorDAQ 2.0: still need to work on internal digital trigger and level trigger modes
	THORDAQ_HW_TRIGGER_MODES triggerMode1 = (_digitalIOSelection[DI_ImagingHardwareTrigger1] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	THORDAQ_HW_TRIGGER_MODES triggerMode2 = (_digitalIOSelection[DI_ImagingHardwareTrigger2] >= 0) ? THORDAQ_HW_TRIGGER_MODES::THORDAQ_HARDWARE_EDGE_TRIGGER_MODE : THORDAQ_HW_TRIGGER_MODES::THORDAQ_NO_TRRIGER;
	//TODO ThorDAQ 2.0: the inter digital trigger works like a level trigger. For this to work with Stim, we would need to park the complete line high until imaging is complete (when N frames are acquired).

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

	//detemine the X& Y FOV in unit of volt, full swing of waveformBuffer,
	//based on field size and the image pixel aspect ratio
	// voltage required is happend to be the mechanical angle of the mirror 
	GalvoStruct galvo_x_control;
	GalvoStruct galvo_y_control;
	double theta = (double)pImgAcqPty->fieldSize * _field2Theta;
	galvo_x_control.amplitude = theta * _theta2Volts * pImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	galvo_y_control.amplitude = theta * _theta2Volts * (double)pImgAcqPty->pixelY / (double)pImgAcqPty->pixelX;
	galvo_y_control.amplitude = (pImgAcqPty->yAmplitudeScaler / 100.0) * galvo_y_control.amplitude * pImgAcqPty->fineFieldSizeScaleY; // Vertical galvo amplitude
	galvo_x_control.offset = (1 * (double)pImgAcqPty->offsetX * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetX; //horizontal galvo offset
	galvo_y_control.offset = ((double)pImgAcqPty->verticalScanDirection * pImgAcqPty->offsetY * _field2Theta) * _theta2Volts + pImgAcqPty->fineOffsetY;// Vertical galvo offset
	galvo_y_control.scan_direction = pImgAcqPty->verticalScanDirection == 1 ? SCAN_DIRECTION::FORWARD_SC : SCAN_DIRECTION::REVERSE_SC;
	pImgAcqPty->flybackCycle = _imgAcqPty.flybackCycle = GetFlybackCycle();

	// Set up DAC settings
	_scan_info = ScanStruct(pImgAcqPty->pixelY, pImgAcqPty->flybackCycle, averageLines);

	_scanLine = ScanLineStruct();
	double linetime = 0;
	double dac_rate = 0;
	bool useFastOneway = false;
	if (pImgAcqPty->threePhotonModeEnable)
	{
		if (FALSE == GetDACSamplesPerLine3P(&_scanLine, pImgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
		{
			return FALSE;
		}
		_imgAcqPty.dwellTime = pImgAcqPty->dwellTime = _daqAcqCfg.galvoGalvoCtrl.dwellTime * US_TO_SEC;
	}
	else
	{
		if (FALSE == GetDACSamplesPerLine(&_scanLine, pImgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
		{
			return FALSE;
		}
	}

	_scan_info.dacRate = dac_rate;
	_scan_info.blockBackwardLine = oneWayLineScan;
	_scan_info.scanMode = pImgAcqPty->scanMode;

	_daqAcqCfg.galvoGalvoCtrl.flybackCycle = pImgAcqPty->flybackCycle;
	_daqAcqCfg.galvoGalvoCtrl.flybackTime = 2 * linetime * _daqAcqCfg.galvoGalvoCtrl.flybackCycle;

	_daqAcqCfg.galvoGalvoCtrl.fastOneWayImaging = _useFastOneway = useFastOneway;

	if (TWO_WAY_SCAN == pImgAcqPty->scanMode || useFastOneway)
	{
		_frameRate = 1.0 / (_scan_info.forward_lines * linetime * _scan_info.average_lines_num + _daqAcqCfg.galvoGalvoCtrl.flybackTime);
	}
	else
	{
		_frameRate = 1.0 / (_scan_info.forward_lines * 2 * linetime * _scan_info.average_lines_num + _daqAcqCfg.galvoGalvoCtrl.flybackTime); // One way is 2 linetimes for each Y pixel
	}

	_daqAcqCfg.imageCtrl.frameRate = _frameRate;
	_daqAcqCfg.imageCtrl.frameNumPerTransfer = max(1, static_cast<ULONG32>(ceil(_frameRate / (double)MAX_TRANSFERS_PER_SECOND)));// set frequency of interrupt to 50ms min//1;//(_frameRate > 1)? static_cast<ULONG>(_frameRate): 1;
	_daqAcqCfg.imageCtrl.frameNumPerSec = max(1, static_cast<ULONG32>(ceil(_frameRate)));
	_daqAcqCfg.imageCtrl.movingAverageFilterEnable = pImgAcqPty->movingAverageFilterEnable == TRUE;
	_daqAcqCfg.imageCtrl.movingAverageMultiplier = pImgAcqPty->movingAverageFilterMultiplier;
	if (_daqAcqCfg.imageCtrl.frameCnt <= _daqAcqCfg.imageCtrl.frameNumPerTransfer)
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = _daqAcqCfg.imageCtrl.frameCnt;
	}

	if (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode) //live mode
	{
		_daqAcqCfg.imageCtrl.frameNumPerTransfer = 1;
	}

	_daqAcqCfg.streamingCtrl.fir_filter_enabled = TRUE;

	if (TRUE == pImgAcqPty->threePhotonModeEnable && FORWARD_SCAN == pImgAcqPty->scanMode && TRUE == pImgAcqPty->acquireDuringTurnAround)
	{
		_daqAcqCfg.imageCtrl.alignmentOffset = 0;
		_daqAcqCfg.galvoGalvoCtrl.sampleOffsetStartLUT3PTI = pImgAcqPty->sampleOffsetStartLUT3PTI;
	}
	else
	{
		//Set the alignmentOffset before building the pockels waveformBuffer, in order to offset the waveformBuffer with the alignment value
		USHORT loadedShiftValue = static_cast<USHORT>((pImgAcqPty->threePhotonModeEnable == TRUE) ? _shiftArray[_ratio - 1] * ALIGNMENT_MULTIPLIER : _shiftArray[static_cast<long>(pImgAcqPty->dwellTime * 5 - 2)] * ALIGNMENT_MULTIPLIER);
		_daqAcqCfg.imageCtrl.alignmentOffset = loadedShiftValue + static_cast<USHORT>(pImgAcqPty->alignmentForField);
		_daqAcqCfg.galvoGalvoCtrl.sampleOffsetStartLUT3PTI = 0;
	}


	if (pImgAcqPty->clockSource == EXTERNAL_CLOCK)
	{
		_daqAcqCfg.imageCtrl.threePhotonMode = pImgAcqPty->threePhotonModeEnable;
		_daqAcqCfg.coherentSamplingCtrl.phaseIncrementMode = 2; //0: disable 1:incremental mode 2: static offset
		_daqAcqCfg.coherentSamplingCtrl.phaseOffset = static_cast<USHORT>(pImgAcqPty->laserCoherentSamplingPhase * 8.0 * 16.0 / 100.0);
		_daqAcqCfg.streamingCtrl.channel_multiplexing_enabled = FALSE;
		_daqAcqCfg.streamingCtrl.fir_filter_enabled = FALSE;
		memcpy(_daqAcqCfg.streamingCtrl.fir_filter, pImgAcqPty->FIRFilters, FIR_FILTER_COUNT * MAX_CHANNEL_COUNT * FIR_FILTER_TAP_COUNT * sizeof(double));
		if (pImgAcqPty->threePhotonModeEnable)
		{
			for (int channel = 0; channel < MAX_CHANNEL_COUNT; channel++)
			{
				_daqAcqCfg.imageCtrl.threePhotonPhaseAlignment[channel] = static_cast<ULONG32>(pImgAcqPty->threePhotonModeAlignmentPhase[channel]);
			}
		}
	}

	_powerRampCurrentIndex = 0;
	_fastZCurrentIndex = 0;
	_daqAcqCfg.imageCtrl.TwoBankDACDMAPlayback = false;

	_useBuiltZWaveform = SetAndBuildFastZWaveform(&_scan_info, &_scanLine, pImgAcqPty, _frameRate, _daqAcqCfg.dacCtrl, _daqAcqCfg.dacCtrl2, &_fastZWaveformParams, useFastOneway); \

		if (TRUE == pImgAcqPty->powerRampEnable || TRUE == _useBuiltZWaveform)
		{
			if (TWO_WAY_SCAN == pImgAcqPty->scanMode || false == useFastOneway)
			{
				//setup the galvo waveforms for dacCtrl and dacCtrl2 (first 2 frames)
				if (!BuildGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl))
				{
					return FALSE;
				}
				if (!BuildGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl2))
				{
					return FALSE;
				}
			}
			else
			{
				//setup the galvo waveforms for dacCtrl and dacCtrl2 (first 2 frames)
				if (!BuildFastOneWayGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl))
				{
					return FALSE;
				}
				if (!BuildFastOneWayGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl2))
				{
					return FALSE;
				}
			}
			if (pImgAcqPty->powerRampPercentValues.size() > 1 && pImgAcqPty->powerRampNumFrames == pImgAcqPty->powerRampPercentValues.size())
			{
				SetupPowerRampSettings(pImgAcqPty);

				//Build pockels waveform before galvo waveform so the digital waveforms are correct
				// 
				//setup the pockels ramp for dacCtrl and dacCtrl2 (first 2 frames)
				SetPowerAndBuildPockelsPowerRampWaveforms(_imgAcqPty.pockelPty, _daqAcqCfg.dacCtrl, true);
				++_powerRampCurrentIndex;
				SetPowerAndBuildPockelsPowerRampWaveforms(_imgAcqPty.pockelPty, _daqAcqCfg.dacCtrl2, true);
				++_powerRampCurrentIndex;
				INT32 error = 0, retVal = 0;
			}
			else if (TRUE == pImgAcqPty->powerRampEnable)
			{
				LogMessage(L"ThordaqGR power ramp not setup correctly", ERROR_EVENT);
				return FALSE;
			}

			ThordaqErrChk(L"ThorDAQAPIDACBankSwitchingRegisterReadyForNextImageWaveformsEvent", retVal = ThorDAQAPIDACBankSwitchingRegisterReadyForNextImageWaveformsEvent(_DAQDeviceIndex, 0, DACReadyForNextImageWaveformsCallback, NULL));

			_daqAcqCfg.imageCtrl.TwoBankDACDMAPlayback = true;
		}
		else
		{
			//Build pockels waveform before galvo waveform so the digital waveforms are correct
			//BuildPockelsControlWaveforms(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, _daqAcqCfg.dacCtrl);
			if (ICamera::LSMAreaMode::POLYLINE == pImgAcqPty->areaMode)
			{
				BuildPockelsControlWaveforms(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl);
				BuildPolylineWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl);
			}
			else
			{
				if (TWO_WAY_SCAN == pImgAcqPty->scanMode || false == useFastOneway)
				{
					BuildPockelsControlWaveforms(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl);

					if (!BuildGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl))
					{
						return FALSE;
					}
				}
				else
				{
					BuildPockelsControlFastOneWayWaveforms(&_scan_info, &_scanLine, &pImgAcqPty->pockelPty, pImgAcqPty, _daqAcqCfg.dacCtrl);

					if (!BuildFastOneWayGalvoWaveforms(&_scan_info, &_scanLine, &galvo_x_control, &galvo_y_control, pImgAcqPty, _daqAcqCfg.dacCtrl))
					{
						return FALSE;
					}
				}
			}
		}

	//if HW trigger pre move to start position and set preSOFTime to turnaroundTime / 2 so imaging starts as fast as possible after HW trigger comes in
	//and move the galvos to the start position
	if (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _daqAcqCfg.triggerSettings.triggerMode || TRUE == _galvoParkAtStart)
	{
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _imagingActiveAOSelection[GG_AO::GG_X], _daqAcqCfg.dacCtrl[_imagingActiveAOSelection[GG_AO::GG_X]].offset_val));
		ThordaqErrChk(L"ThorDAQAPISetDACParkValue", retVal = ThorDAQAPISetDACParkValue(_DAQDeviceIndex, _imagingActiveAOSelection[GG_AO::GG_Y], _daqAcqCfg.dacCtrl[_imagingActiveAOSelection[GG_AO::GG_Y]].offset_val));
		_daqAcqCfg.galvoGalvoCtrl.preSOFTime = _daqAcqCfg.galvoGalvoCtrl.turnaroundTime / 2; //minimum of hald turaround time (intraline delay)
	}
	else
	{
		double basePreSOFTime = FALSE == _limitGalvoSpeed ? 0.0005 : 0.0008; //500us or 800us
		_daqAcqCfg.galvoGalvoCtrl.preSOFTime = basePreSOFTime + _daqAcqCfg.galvoGalvoCtrl.turnaroundTime / 2;
	}
	_daqAcqCfg.imageCtrl.dacSlowMoveToOffset = TRUE == _limitGalvoSpeed;

	ThordaqErrChk(L"ThorDAQAPISetImagingConfiguration", retVal = ThorDAQAPISetImagingConfiguration(_DAQDeviceIndex, _daqAcqCfg));
	if (retVal != STATUS_SUCCESSFUL)
	{
		ret = FALSE;
		return ret;
	}
	else  //Set up Buffer
	{
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

		ULONG rawPixelSize = pImgAcqPty->pixelX * totalYLines;
		SAFE_DELETE_PTR(_pDataProcessor);
		if (_daqAcqCfg.imageCtrl.numPlanes > 1)
		{
			ULONG blankLines = (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode || FALSE == _multiplaneBlankLinesInLiveModeOnly) ? (ULONG)_multiplaneBlankLines : 0;

			_pDataProcessor = new MultiPlaneDataProcessor((BYTE)pImgAcqPty->channel, pImgAcqPty->pixelX, totalYLines, averageLines, _daqAcqCfg.imageCtrl.frameNumPerTransfer, _daqAcqCfg.imageCtrl.numPlanes, _datamap, pImgAcqPty->scanMode, blankLines);
		}
		else
		{
			_pDataProcessor = new DataProcessor((BYTE)pImgAcqPty->channel, pImgAcqPty->pixelX, totalYLines, averageLines, _daqAcqCfg.imageCtrl.frameNumPerTransfer, _datamap);
		}

		if (SetupFrameBuffer(pImgAcqPty) != STATUS_SUCCESSFUL)
		{
			ret = FALSE;
			return ret;
		}
	}

	return ret;
}

/************************************************************************************************
* @fn	long CThorDAQGalvoGalvo::SetupFrameBuffer()
*
* @brief	Set up Frame Buffer.
* @return	A long.
**************************************************************************************************/
long CThorDAQGalvoGalvo::SetupFrameBuffer(ImgAcqPty* pImgAcqPty)
{
	std::bitset<sizeof(size_t)* CHAR_BIT> channel_bitset(pImgAcqPty->channel);
	long channelCount = static_cast<long>(channel_bitset.count());
	bool oneWayLineScan = false;// //TODO: test without this and remove if it works wellpImgAcqPty->pixelY == 1 && TWO_WAY_SCAN_MODE != pImgAcqPty->scanMode;

	size_t AllocSize = sizeof(USHORT) * _daqAcqCfg.imageCtrl.numPlanes * _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize * _daqAcqCfg.imageCtrl.frameNumPerTransfer;

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

	//history buffer for average (1 frame for all channels):
	if (_pHistoryBuf != NULL)
	{
		VirtualFree(_pHistoryBuf, 0, MEM_RELEASE);
	}
	size_t histBufSize = (size_t)pImgAcqPty->numberOfPlanes * MAX_CHANNEL_COUNT * pImgAcqPty->pixelX * pImgAcqPty->pixelY * sizeof(USHORT);
	_pHistoryBuf = (UCHAR*)VirtualAlloc(NULL, histBufSize, MEM_COMMIT, PAGE_READWRITE);
	if (_pHistoryBuf == NULL)
	{
		return FALSE;
	}

	if (_pHistoryProgressiveBuf != NULL)
	{
		VirtualFree(_pHistoryProgressiveBuf, 0, MEM_RELEASE);
	}
	_pHistoryProgressiveBuf = (UCHAR*)VirtualAlloc(NULL, histBufSize, MEM_COMMIT, PAGE_READWRITE);
	if (_pHistoryProgressiveBuf == NULL)
	{
		return FALSE;
	}

	size_t progressiveBufLength = (size_t)_daqAcqCfg.imageCtrl.numPlanes * _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;

	//progressive buffer for keeping the last progressive buffer (1 frame for all channels):
	if (_daqAcqCfg.imageCtrl.frameNumPerSec <= 1)
	{
		for (int i = 0; i < static_cast<int>(MAX_CHANNEL_COUNT); i++)
		{

			SAFE_DELETE_ARRAY(_pProgressiveBuf[i]);
			_pProgressiveBuf[i] = new UCHAR[progressiveBufLength * sizeof(USHORT)]();
		}
	}

	//circular buffer for read (by user) and write (by camera):
	//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
	SAFE_DELETE_PTR(_pFrmBuffer);

	if (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode || ICamera::HW_MULTI_FRAME_TRIGGER_EACH == pImgAcqPty->triggerMode) // continuous scan
	{
		// No need for a big buffer, better to keep it at 1 (transerFrames = 1 in continuous mode)so we know we are grabbing the last frame
		_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY, channelCount, sizeof(USHORT), (size_t)_daqAcqCfg.imageCtrl.frameNumPerTransfer * DEFAULT_DMA_BUFFER_NUM, _daqAcqCfg.imageCtrl.numPlanes);
	}
	else
	{
		size_t dmaBufferCount = (ULONG)pImgAcqPty->dmaBufferCount > _daqAcqCfg.imageCtrl.frameCnt ? _daqAcqCfg.imageCtrl.frameCnt : static_cast<size_t>(pImgAcqPty->dmaBufferCount);
		// Use DMA buffer to setup the size of this circular buffer
		_pFrmBuffer = new FrameCirBuffer(pImgAcqPty->pixelX, pImgAcqPty->pixelY, channelCount, sizeof(USHORT), dmaBufferCount, _daqAcqCfg.imageCtrl.numPlanes);
	}

	SAFE_DELETE_ARRAY(_pTempBuf);
	size_t circularBufferframeSizeByte = sizeof(USHORT) * pImgAcqPty->numberOfPlanes * pImgAcqPty->pixelX * pImgAcqPty->pixelY;
	_pTempBuf = new UCHAR[circularBufferframeSizeByte * MAX_CHANNEL_COUNT];

	return STATUS_SUCCESSFUL;
}


void CThorDAQGalvoGalvo::SetPowerAndBuildPockelsPowerRampWaveforms(PockelPty pockelsSettings, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigitalWaveforms)
{
	switch (_imgAcqPty.powerRampMode)
	{
	case PowerRampMode::POWER_RAMP_MODE_STAIRCASE:
	{
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			pockelsSettings.pockelsPowerLevel[i] = _pockelsResponsePowerLevels[i][_powerRampCurrentIndex];
		}

		if (TWO_WAY_SCAN == _imgAcqPty.scanMode || false == _useFastOneway)
		{
			CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, waveforms, buildDigitalWaveforms);
		}
		else
		{
			CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlFastOneWayWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, waveforms, buildDigitalWaveforms);
		}
		break;
	}
	default:
	{
		for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
		{
			pockelsSettings.pockelsPowerLevel[i] = _pockelsImagePowerRampVector[i][_powerRampCurrentIndex].startPowerLevel;
			pockelsSettings.pockelsPowerLevel2[i] = _pockelsImagePowerRampVector[i][_powerRampCurrentIndex].endPowerLevel;
		}

		if (TWO_WAY_SCAN == _imgAcqPty.scanMode || false == _useFastOneway)
		{
			CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlPowerRampWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, waveforms, buildDigitalWaveforms);
		}
		else
		{
			CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlPowerRampFastOneWayWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, waveforms, buildDigitalWaveforms);
		}
		break;
	}
	}
}

void CThorDAQGalvoGalvo::SetupPowerRampSettings(ImgAcqPty* pImgAcqPty)
{
	for (int k = 0; k < MAX_POCKELS_CELL_COUNT; ++k)
	{
		_pockelsResponsePowerLevels[k].clear();
		for (int i = 0; i < pImgAcqPty->powerRampPercentValues.size(); ++i)
		{
			double val = 0.0;
			switch (CThorDAQGalvoGalvo::GetInstance()->_pockelsResponseType[k])
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

long CThorDAQGalvoGalvo::SetAndBuildFastZWaveform(ScanStruct* scanInfo, ScanLineStruct* scanLine, ImgAcqPty* pImgAcqPty, double frameRate, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms2, ThorDAQZWaveformParams* zWaveformParams, bool useFastOneway)
{
	double dac_rate = scanInfo->dacRate;
	UINT forward_data_num = 0;
	UINT backward_data_num = 0;

	if (TWO_WAY_SCAN == pImgAcqPty->scanMode || false == useFastOneway)
	{
		UINT samples_line = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * scanLine->samples_idle + scanLine->samples_scan : 2 * (2 * scanLine->samples_idle + scanLine->samples_scan);
		ULONG32 fullLine = TWO_WAY_SCAN == pImgAcqPty->scanMode ? samples_line * 2 : samples_line;

		//the number of times each line will be repeated
		UINT linesPerLine = (UINT)scanInfo->average_lines_num;
		UINT half_turnaround_padding = (samples_line - scanLine->samples_scan) / 2; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
		UINT sync_secure = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2 + scanLine->samples_idle;
		forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_line;
		UINT sync_securebw = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2;
		backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(scanInfo->backward_lines) * fullLine - (sync_securebw + FLYBACK_OFFSET); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(fullLine - (sync_secure + FLYBACK_OFFSET));

		backward_data_num = min(MAX_FLYBACK_DAC_SAMPLES, backward_data_num);
	}
	else
	{
		UINT samples_line = 2 * scanLine->samples_idle + scanLine->samples_scan + scanLine->samples_back;
		ULONG32 fullLine = samples_line;
		UINT half_turnaround_padding = scanLine->samples_idle + scanLine->samples_back; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.

		//the number of times each line will be repeated
		UINT linesPerLine = (UINT)scanInfo->average_lines_num;

		UINT sync_secure = scanLine->samples_idle;
		forward_data_num = (ULONG32)scanInfo->forward_lines * samples_line * linesPerLine;
		backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)min(MAX_FLYBACK_DAC_SAMPLES, (scanInfo->backward_lines) * fullLine - (sync_secure)); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback
	}

	long useZWave = BuildAndGetFastZWaveform(scanInfo->dacRate, forward_data_num, backward_data_num, frameRate, zWaveformParams);
	long useRemoteFocusWaveform = GetRemoteFocusFastZWaveform(scanInfo->dacRate, forward_data_num, backward_data_num, frameRate, zWaveformParams);
	long ret = FALSE;

	// Piezo FastZ and Remote Focus FastZ can be used at the same time. If that ever changes then this setup needs to be separated accordingly
	if (TRUE == useZWave || TRUE == useRemoteFocusWaveform)
	{
		DAC_WAVEFORM_CRTL_STRUCT gZCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		gZCtrl.update_rate = scanInfo->dacRate;
		gZCtrl.flyback_samples = backward_data_num;
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
		gZCtrl2.update_rate = scanInfo->dacRate;
		gZCtrl2.flyback_samples = backward_data_num;
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

void CTHORDAQCALLBACK CThorDAQGalvoGalvo::DACReadyForNextImageWaveformsCallback(THORDAQ_STATUS status, void* callbackData)
{
	if (TRUE == _imgAcqPty_Pre.powerRampEnable)
	{
		if (_powerRampCurrentIndex < _imgAcqPty.powerRampNumFrames)
		{
			CThorDAQGalvoGalvo::GetInstance()->SetPowerAndBuildPockelsPowerRampWaveforms(_imgAcqPty.pockelPty, _daqAcqCfg.dacCtrl2, false);
		}
		else if (_powerRampCurrentIndex < _imgAcqPty.powerRampNumFrames + _imgAcqPty.powerRampNumFlybackFrames)
		{
			PockelPty pockelsSettings = _imgAcqPty.pockelPty;
			for (int i = 0; i < MAX_POCKELS_CELL_COUNT; ++i)
			{
				pockelsSettings.pockelsPowerLevel[i] = 0;
			}

			if (TWO_WAY_SCAN == _imgAcqPty.scanMode || false == CThorDAQGalvoGalvo::GetInstance()->_useFastOneway)
			{
				CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, _daqAcqCfg.dacCtrl2, false);
			}
			else
			{
				CThorDAQGalvoGalvo::GetInstance()->BuildPockelsControlFastOneWayWaveforms(&_scan_info, &_scanLine, &pockelsSettings, &_imgAcqPty, _daqAcqCfg.dacCtrl2, false);
			}
		}

		_powerRampCurrentIndex++;

		if (_powerRampCurrentIndex >= _imgAcqPty.powerRampNumFrames + _imgAcqPty.powerRampNumFlybackFrames)
		{
			_powerRampCurrentIndex = 0;
		}
	}

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

/************************************************************************************************
* @fn	UINT CThorDAQGalvoGalvo::StartFrameAcqProc(LPVOID instance)
*
* @brief	Start  Acquisition Thread.
* @param 	instance	  	Acquisition Thread instance.
* @return	A uint.
**************************************************************************************************/
UINT CThorDAQGalvoGalvo::StartFrameAcqProc(LPVOID instance)
{
	THORDAQ_STATUS	status = STATUS_SUCCESSFUL;
	bool            noErrorOnAcquisition = true;
	long long		targetFrames = static_cast<long long>(_daqAcqCfg.imageCtrl.frameCnt);// do later
	std::bitset<sizeof(size_t)* CHAR_BIT> channel_bitset(_daqAcqCfg.imageCtrl.channel);
	ULONG			channelCount = static_cast<ULONG>(channel_bitset.count());
	ULONG			rawframeSize = (ULONG)_daqAcqCfg.imageCtrl.numPlanes * _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
	ULONG			rawframeSizeByte = static_cast<ULONG>(rawframeSize * sizeof(USHORT));
	ULONG			transferSize = static_cast<ULONG>(rawframeSizeByte * _daqAcqCfg.imageCtrl.frameNumPerTransfer * MAX_CHANNEL_COUNT);
	UINT64          size;
	ULONG			transferredFrames = 0;
	double          regular_timeout = 1.0 / _daqAcqCfg.imageCtrl.frameRate * MS_TO_SEC * 10 * _daqAcqCfg.imageCtrl.frameNumPerTransfer * 10;
	double			hardware_trigger_timeout = (double)_triggerWaitTimeout * MS_TO_SEC * 10;
	double			timeout = regular_timeout;
	BOOL			is_hardware_captured = FALSE;
	BOOL			hardware_timeout_enable = (THORDAQ_TRIGGER_MODES::THORDAQ_HARDWARE_TRIGGER == _daqAcqCfg.triggerSettings.triggerMode) ? TRUE : FALSE;
	long long       frame_left = targetFrames - 1; // 0 based
	ULONG32			frame_count = 0;
	const long		PROGRESSIVE_LINES = _daqAcqCfg.imageCtrl.frameRate > 0.7 ? 32 : 16;
	BOOL isPartialData = FALSE;
	int32 error = 0, retVal = 0;
	// START with "global scan" Setting...
	UINT64 tlineGroupStart, tend, tdif, tframeStart;
	UINT64 sofSleepTime = (UINT64)ceil(_daqAcqCfg.galvoGalvoCtrl.preSOFTime * MS_TO_SEC);
	tframeStart = tlineGroupStart = GetTickCount64() + sofSleepTime;
	ThordaqErrChk(L"ThorDAQAPIStartAcquisition", retVal = ThorDAQAPIStartAcquisition(_DAQDeviceIndex));
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StartFrameAcqProc thread starting", VERBOSE_EVENT);

	long multiplier = TWO_WAY_SCAN == _imgAcqPty.scanMode || CThorDAQGalvoGalvo::GetInstance()->_useFastOneway ? 1 : 2;
	const int US_PER_SECOND = 1000000;
	double lineTime = _daqAcqCfg.galvoGalvoCtrl.turnaroundTime + _imgAcqPty.dwellTime / US_PER_SECOND * _daqAcqCfg.imageCtrl.imgHSize;
	long progressiveSleepTime = static_cast<long>(floor(lineTime * PROGRESSIVE_LINES * 1000 * multiplier));
	long flybackSleepTime = static_cast<long>(floor(1000 * _daqAcqCfg.galvoGalvoCtrl.flybackTime));
	long progressiveLineCount = 0;
	long progressiveScan = FALSE;
	CThorDAQGalvoGalvo::GetInstance()->_currentlyImaging = true;
	if (SW_FREE_RUN_MODE == _imgAcqPty_Pre.triggerMode && _frameRate <= PROGRESSIVESCAN_MAX_FRAMERATE)
	{
		progressiveScan = TRUE;
	}

	ThorDAQAPIProgressiveScan(_DAQDeviceIndex, progressiveScan);

	// start the loop which exits on user application (stop acquisition) command
	do
	{
		frame_left = targetFrames - 1 - _index_of_last_written_frame;
		// For debugging, print the index of frame counts
		/*(L"ThorDAQAPIGetTotalFrameCount", status = ThorDAQAPIGetTotalFrameCount(_DAQDeviceIndex,frame_count));
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"ThorDAQGalvoGalvo \nframe_count: %d \n frame_left: %d \n _indexOfLastCompletedFrame: %d, \nregular_timeout: %f", frame_count, frame_left, _indexOfLastCompletedFrame, regular_timeout);
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg,ERROR_EVENT);*/

		if (TRUE == progressiveScan)
		{
			//sleep 40ms for smooth display
			Sleep(40);
			if (0 == progressiveLineCount && frame_count == 0)
			{
				tend = GetTickCount64();
				tdif = tend - tlineGroupStart;
				while (tdif < sofSleepTime)
				{
					Sleep(1);
					tend = GetTickCount64();
					tdif = tend - tlineGroupStart;
				}
				tlineGroupStart = GetTickCount64();
			}
			else if (0 == progressiveLineCount)
			{
				tend = GetTickCount64();
				tdif = tend - tlineGroupStart;
				while (tdif < flybackSleepTime)
				{
					Sleep(1);
					tend = GetTickCount64();
					tdif = tend - tlineGroupStart;
				}
				tlineGroupStart = GetTickCount64();
			}
			tend = GetTickCount64();
			tdif = tend - tlineGroupStart;
			while (tdif < progressiveSleepTime)
			{
				Sleep(1);
				tend = GetTickCount64();
				tdif = tend - tlineGroupStart;
			}

			tlineGroupStart = GetTickCount64();
			double frameTimeMilliSeconds = 1000 / _frameRate;
			double framePercentComplete = (tend - tframeStart) / frameTimeMilliSeconds;
			progressiveLineCount = static_cast<long>(floor(framePercentComplete * _daqAcqCfg.imageCtrl.imgVSize));
		}

		if ((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
		{
			break;
		}

		BOOL isLastTransfer = frame_left <= _daqAcqCfg.imageCtrl.frameNumPerTransfer;
		ULONG framesToTransfer = isLastTransfer ? (ULONG)frame_left : _daqAcqCfg.imageCtrl.frameNumPerTransfer;
		timeout = (hardware_timeout_enable && !is_hardware_captured) ? hardware_trigger_timeout : regular_timeout;

		//we only need to copy a finite amount of frames so set the size accordingly
		size = isLastTransfer ? frame_left * MAX_CHANNEL_COUNT * rawframeSizeByte : transferSize;

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

		if (status == STATUS_SUCCESSFUL && noErrorOnAcquisition)
		{
			hardware_timeout_enable = FALSE; // only used for first frame
			ResetEvent(_hTriggerTimeout);
			vector<UCHAR*>  processedBuf;
			if (TRUE == progressiveScan && isPartialData && progressiveLineCount <= _daqAcqCfg.imageCtrl.imgVSize)
			{
				int progressiveCopySize =  _daqAcqCfg.imageCtrl.imgHSize * progressiveLineCount * sizeof(USHORT);
				for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
				{
					for (int j = 0; j < _daqAcqCfg.imageCtrl.numPlanes; ++j)
					{
						//only copy the portion of the lines completed for each plane
						if (_pProgressiveBuf[i] != NULL)
						{
							int copyOffset =  j* _daqAcqCfg.imageCtrl.imgHSize* _daqAcqCfg.imageCtrl.imgVSize * sizeof(USHORT);
							memcpy(_pProgressiveBuf[i] + copyOffset, _BufferContiguousArray[i] + copyOffset, progressiveCopySize);							
						}
					}					
				}
				processedBuf = _pDataProcessor->ProcessBuffer(_pProgressiveBuf, transferredFrames);				
			}
			else
			{
				frame_count += transferredFrames;
				tframeStart = tlineGroupStart = GetTickCount64();
				processedBuf = _pDataProcessor->ProcessBuffer(_BufferContiguousArray, transferredFrames);

				if (TRUE == progressiveScan)
				{
					progressiveLineCount = 0;
					for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
					{
						memcpy(_pProgressiveBuf[i], _BufferContiguousArray[i], rawframeSizeByte);
					}
				}
			}

			if ((WaitForSingleObject(_hStopAcquisition, 0) == WAIT_OBJECT_0))
			{
				break;
			}

			if (_imgAcqPty_Pre.lineAveragingEnable == FALSE && 1 < _imgAcqPty_Pre.averageNum && _imgAcqPty_Pre.averageMode == FRM_CUMULATIVE_MOVING && frame_count > 0)
			{
				USHORT* bPtr = NULL; //Identifier pointer for history buffer
				USHORT* tPtr = NULL; //Identifier pointer for captured data
				//write to history buffer:
				int averageLimit = 0;
				if (TRUE == progressiveScan && isPartialData && progressiveLineCount <= _daqAcqCfg.imageCtrl.imgVSize)
				{
					averageLimit = _daqAcqCfg.imageCtrl.numPlanes * _daqAcqCfg.imageCtrl.imgHSize * progressiveLineCount;
					size_t histBufSize = (size_t)_daqAcqCfg.imageCtrl.numPlanes * channelCount * _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize * sizeof(USHORT);
					memcpy(_pHistoryProgressiveBuf, _pHistoryBuf, histBufSize);
					bPtr = (USHORT*)_pHistoryProgressiveBuf;
				}
				else
				{
					//write to history buffer:
					bPtr = (USHORT*)_pHistoryBuf;
					averageLimit = (size_t)_daqAcqCfg.imageCtrl.numPlanes * channelCount * _daqAcqCfg.imageCtrl.imgHSize * _daqAcqCfg.imageCtrl.imgVSize;
				}

				for (size_t t = 0; t < transferredFrames; ++t)
				{
					long avg = (long)(frame_count >= (UINT64)_imgAcqPty_Pre.averageNum ? _imgAcqPty_Pre.averageNum : frame_count - (transferredFrames - t - 1));
					double factor1 = (1 / (double)(avg));
					double factor2 = ((double)avg - 1) / (double)(avg);
					tPtr = (USHORT*)processedBuf.at(t); // Get the pointer

					for (size_t chID = 0; chID < channelCount; ++chID)
					{
						for (size_t p = 0; p < rawframeSizeByte; ++p)
						{
							size_t index = chID * rawframeSizeByte + p;
							if (averageLimit > index)
							{
								*(bPtr + index) = static_cast<USHORT>((*(bPtr + index)) * factor2 + (*(tPtr + index)) * factor1);
							}
						}
					}
					//write history buffer to circular buffer:
					//when in live mode there is only one image at a time
					vector<UCHAR*> historyBufVector;
					if (TRUE == progressiveScan && isPartialData && progressiveLineCount <= _daqAcqCfg.imageCtrl.imgVSize)
					{
						historyBufVector.push_back(_pHistoryProgressiveBuf);
					}
					else
					{
						historyBufVector.push_back(_pHistoryBuf);
					}

					_pFrmBuffer->WriteFrames(historyBufVector, 1);
					_imageStatus = StatusType::STATUS_READY;
				}
			}
			else
			{
				_pFrmBuffer->WriteFrames(processedBuf, transferredFrames);
				_imageStatus = isPartialData ? StatusType::STATUS_PARTIAL : StatusType::STATUS_READY;
			}
			_index_of_last_written_frame += transferredFrames;
		}
	} while ((_index_of_last_written_frame < (targetFrames - 1)) && (WaitForSingleObject(_hStopAcquisition, 0) != WAIT_OBJECT_0) && noErrorOnAcquisition);
	ULONG32 clockStatus = 0;
	ThordaqErrChk(L"ThorDAQAPIGetExternClockStatus", status = ThorDAQAPIGetExternClockStatus(_DAQDeviceIndex, clockStatus));
	if (!noErrorOnAcquisition && _daqAcqCfg.imageCtrl.threePhotonMode == TRUE
		&& status == THORDAQ_STATUS::STATUS_SUCCESSFUL
		&& clockStatus == 0)
	{
		MessageBox(NULL, L"Laser SYNC Error", L"Error", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);
	}
	//done capture:
	((CThorDAQGalvoGalvo*)instance)->StopDaqBrd();
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(L"thordaqGalvoGalvo::StartFrameAcqProc thread exiting", VERBOSE_EVENT);
	CThorDAQGalvoGalvo::GetInstance()->_currentlyImaging = false;
	SetEvent(_hThreadStopped);
	return 0;
}

/// <summary> Calculates the minimum value for flyback cycles the current settings can support </summary>
/// <returns> Return 0, when vertical galvo is disabled </returns>
long CThorDAQGalvoGalvo::GetMinFlybackCycle()
{
	//TODO: FLyback cycle == 0 still needs some work. We will leave to be 1 for now, which looks fine.
	// We found some combinations make the Galvos click at flyback of 1, for now the new default will be 2. It still can be 1 in GGSuperUser mode
	//if (ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode || FALSE == _imgAcqPty.galvoEnable)
	//{
	//	return 0;
	//}
	//else
	{
		if (TRUE == _ggSuperUserMode)
		{
			return 1;
		}
		return 1;
	}
}

/// <summary> Sets the current flyback cycle </summary>
/// <param name="flybackCycle"> The new value for flyback cycle </param>
void CThorDAQGalvoGalvo::SetFlybackCycle(long flybackCycle)
{
	if (MAX_FLYBACK_CYCLE < flybackCycle || MAX_FLYBACK_TIME < GetFlybackTime(flybackCycle))
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"PARAM_LSM_FLYBACK_CYCLE %d outside range %d to %d, and above %f seconds", static_cast<long> (flybackCycle), GetMinFlybackCycle(), MAX_FLYBACK_CYCLE, MAX_FLYBACK_TIME);
		LogMessage(_errMsg, ERROR_EVENT);
	}
	else
	{
		long minFlybackCycle = GetMinFlybackCycle();
		_imgAcqPty.flybackCycle = _flybackCycles = (flybackCycle > minFlybackCycle ? flybackCycle : minFlybackCycle);
	}
}

double CThorDAQGalvoGalvo::GetFlybackTime(long flybackCycles)
{
	if (flybackCycles == 0 && (FALSE == _imgAcqPty.galvoEnable || ICamera::LSMAreaMode::POLYLINE == _imgAcqPty.areaMode))
	{
		return LINE_FRAMETRIGGER_LOW_TIMEPOINTS * _imgAcqPty.dwellTime / 1000000.0;
	}
	else
	{
		//double galvoSamplesPaddingTime = (_imgAcqPty.turnAroundTimeUS / 2.0) / 1000000.0;  //use user set amount of time for x galvo retrace, default is 400us
		//double galvoSamplesPerFullLine = 2.0 * _imgAcqPty.pixelX * _imgAcqPty.dwellTime / 1000000.0 + 2.0 * (_imgAcqPty.pixelX - 1) * 17.0 / 200000000.0 + 4 * galvoSamplesPaddingTime;

		bool oneWayLineScan = false;////TODO: test without this and remove if it works well _imgAcqPty.pixelY == 1 && TWO_WAY_SCAN_MODE != _imgAcqPty.scanMode;

		//long vSize = _imgAcqPty.pixelY;// oneWayLineScan ? 2 : static_cast<USHORT>(_imgAcqPty.pixelY);

		//ScanStruct scan_info = ScanStruct(vSize, flybackCycles);

		double dwell_time = _imgAcqPty.dwellTime / 1000000.0;
		double linetime = 0;
		double dac_rate = 0;
		ScanLineStruct scanLine = ScanLineStruct();
		bool useFastOneway = false;
		if (_imgAcqPty.threePhotonModeEnable)
		{
			if (FALSE == GetDACSamplesPerLine3P(&scanLine, &_imgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
			{
				return FALSE;
			}
		}
		else
		{
			if (FALSE == GetDACSamplesPerLine(&scanLine, &_imgAcqPty, dac_rate, dwell_time, linetime, oneWayLineScan, useFastOneway))
			{
				return FALSE;
			}
		}
		double flybackTime = 2 * linetime * flybackCycles;


		return flybackTime;
	}
}

long CThorDAQGalvoGalvo::GetFlybackCycle()
{
	long minFlybackCycle = GetMinFlybackCycle();
	// If the current set flybackCycle is less than the minimum or "Always Use Fastest" is checked, set flybackCycle to the minimum
	if (_imgAcqPty.minimizeFlybackCycles || minFlybackCycle > _flybackCycles)
	{
		return minFlybackCycle;
	}
	else if (GetFlybackTime(_imgAcqPty.flybackCycle) > MAX_FLYBACK_TIME)
	{
		double galvoSamplesPaddingTime = ((_imgAcqPty.turnAroundTimeUS / 2.0) / 1000000.0); //use user set amount of time for x galvo retrace, default is 400us
		double galvoSamplesPerLine = 2.0 * (double)_imgAcqPty.pixelX * _imgAcqPty.dwellTime / 1000000.0 + 2.0 * (_imgAcqPty.pixelX - 1) * 17.0 / 200000000.0 + 4.0 * galvoSamplesPaddingTime;
		_flybackCycles = static_cast<long>(MAX_FLYBACK_TIME / galvoSamplesPerLine);
	}
	return _flybackCycles;
}

int CThorDAQGalvoGalvo::CountChannelBits(long channelSet)
{
	int count = 0;
	while (channelSet)
	{
		channelSet &= (channelSet - 1);
		count++;
	}
	return count;
}

long CThorDAQGalvoGalvo::SetFrameBufferReadyOutput()
{
	// Set Frame Buffer Ready to high then low for one milisecond
	//Toggle the first channel Aux_digital_output_0 by turning on the first bit.
	USHORT auxDigChannel = 0, low = 0, high = 1;
	int32 error = 0, retVal = 0;

	ThordaqErrChk(L"ThorDAQAPIToggleAuxDigitalOutputs", retVal = ThorDAQAPIToggleAuxDigitalOutputs(_DAQDeviceIndex, auxDigChannel, high));
	if (retVal != STATUS_SUCCESSFUL)
	{
		return FALSE;
	}

	Sleep(1);

	ThordaqErrChk(L"ThorDAQAPIToggleAuxDigitalOutputs", retVal = ThorDAQAPIToggleAuxDigitalOutputs(_DAQDeviceIndex, auxDigChannel, low));
	if (retVal != STATUS_SUCCESSFUL)
	{
		return FALSE;
	}

	return TRUE;
}

LONG CThorDAQGalvoGalvo::BuildPockelsControlPowerRampWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWaveforms)
{
	//!hardcode to use the same amount fo GG waveformBuffer for now
	bool blockBackwardLine = scanInfo->blockBackwardLine;
	double dac_rate = scanInfo->dacRate;

	ULONG32 samples_pad = scanLine->samples_idle;
	ULONG32 samples_sweep = scanLine->samples_scan;
	ULONG32 samples_single_line = TWO_WAY_SCAN == scanInfo->scanMode ? samples_pad * 2 + samples_sweep : 2 * (samples_pad * 2 + samples_sweep);
	ULONG32 fullLine = TWO_WAY_SCAN == scanInfo->scanMode ? samples_single_line * 2 : samples_single_line;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;
	UINT sync_secure = TWO_WAY_SCAN == scanInfo->scanMode ? scanLine->samples_idle : samples_single_line / 2 + scanLine->samples_idle;
	UINT half_turnaround_padding = (samples_single_line - scanLine->samples_scan) / 2;  //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_single_line;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)min(MAX_FLYBACK_DAC_SAMPLES, (scanInfo->backward_lines) * fullLine - (sync_secure));
	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = (waveform_dataLength + DAC_FIFO_DEPTH / 2);//Fifo depth: 1024 words (1 word = 32bits)

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	long imageSize = width * height * sizeof(USHORT);

	USHORT* pGalvoWaveformDigitalOutputs = NULL;
	if (buildDigiWaveforms)
	{
		if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
		{
			pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
		}
		else
		{
			pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		}
	}

	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		bool pockelsEnable = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;
		if (pockelsEnable == FALSE || pockelPty->pockelsPowerLevel[pockelsIndex] == 0)
		{
			continue;
		}

		DAC_WAVEFORM_CRTL_STRUCT pockelsCtrl;
		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];

		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			pockelsCtrl = waveforms[pockelOutputChannel];
		}
		else
		{
			pockelsCtrl = DAC_WAVEFORM_CRTL_STRUCT();
			pockelsCtrl.park_val = pockelPty->pockelsMinVoltage[pockelsIndex];
			pockelsCtrl.offset_val = pockelPty->pockelsMinVoltage[pockelsIndex];
			pockelsCtrl.update_rate = dac_rate;
			pockelsCtrl.flyback_samples = backward_data_num;
			pockelsCtrl.output_port = pockelOutputChannel;
			pockelsCtrl.waveform_buffer_size = total_dataLength * sizeof(USHORT);
			pockelsCtrl.waveformBuffer = new USHORT[total_dataLength];
			pockelsCtrl.filterInhibit = true;
			pockelsCtrl.hSync = true;
			pockelsCtrl.enableEOFFreeze = false;
		}

		USHORT* pPockelsWaveform = pockelsCtrl.waveformBuffer;

		double pockelsOnVoltage;
		//the reference output is only used for pockels1, for everything else use the normal settings

		pockelsOnVoltage = pockelPty->pockelsMinVoltage[pockelsIndex] + (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * pockelPty->pockelsPowerLevel[pockelsIndex];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[pockelsIndex]) / GALVO_RESOLUTION));

		USHORT pockels_output_high_start_ushort = pockels_output_high_ushort;
		double step = (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * (pockelPty->pockelsPowerLevel2[pockelsIndex] - pockelPty->pockelsPowerLevel[pockelsIndex]) / (double)waveform_dataLength;

		memset(pPockelsWaveform, pockels_output_low_ushort, total_dataLength * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		ULONG32 samplesPercentBlank = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 lowSamples = samples_pad + samplesPercentBlank;
		ULONG32 endOfForwardLine = samples_pad + samples_sweep - static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 blankSamples = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 effectiveSamples = samples_sweep - 2 * blankSamples;
		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		ULONG32 twowayOffsetSamples = static_cast<ULONG32>(max(0, round(_daqAcqCfg.imageCtrl.alignmentOffset) / static_cast<ULONG32>((double)SYS_CLOCK_FREQ / dac_rate + pockelPty->pockelsDelayUS[pockelsIndex])));

		// set up the waveformBuffer for the horizontal galvo
		// first line
		ULONG32 sample_index = half_turnaround_padding + twowayOffsetSamples;

		if (FALSE == _pockelsFlybackBlank)
		{
			for (UINT i = 0; i < total_dataLength; ++i)
			{
				pGalvoWaveformDigitalOutputs[i] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelsWaveform[i] = pockels_output_high_ushort;
			}
		}

		bool doROIMask = pockelPty->pockelsMaskEnable[pockelsIndex] && _pockelsMaskSize[pockelsIndex] == imageSize && NULL != _pPockelsMask[pockelsIndex];

		int s = sample_index + 1;
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if (FALSE == _pockelsTurnAroundBlank && 0 == pockelPty->pockelsLineBlankingPercentage[pockelsIndex] && !doROIMask)
		{
			ULONG32 frameSamples = forward_data_num - sample_index;
			for (ULONG32 i = 0; i < frameSamples; ++i)
			{
				if (buildDigiWaveforms)
				{
					*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
				}
				*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
				pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
				++s;
			}
		}
		else
		{
			short* roiMask = (short*)_pPockelsMask[pockelsIndex];

			UINT passes = (TWO_WAY_SCAN == scanInfo->scanMode) ? ((UINT)((scanInfo->forward_lines * linesPerLine) / 2)) : ((UINT)(scanInfo->forward_lines * linesPerLine));

			if (blockBackwardLine)
			{
				passes = 1;
			}

			for (UINT j = 0; j < passes; ++j)
			{
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;

					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				UINT lineIndex = j / linesPerLine;

				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					long xOffset;
					if (FALSE != pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
					}
					else
					{
						xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
					}

					long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? lineIndex * (width * 2) + (width * 2 - xOffset - 1) : lineIndex * width + width - xOffset - 1;
					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					long roiMaskValue = 0;
					if (doROIMask)
					{
						roiMaskValue = roiMask[maskOffset];
					}
					if (!doROIMask ||
						(true == doROIMask &&
							(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
								((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
					}
					else
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
				for (ULONG32 i = 0; i < lowSamples * 2; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					if (TWO_WAY_SCAN != scanInfo->scanMode || blockBackwardLine)
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
					else
					{
						long xOffset;
						if (FALSE != pImgAcqPty->horizontalFlip)
						{
							xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
						}
						else
						{
							xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
						}
						long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? lineIndex * (width * 2) + xOffset : lineIndex * width + xOffset;
						long roiMaskValue = 0;
						if (doROIMask)
						{
							roiMaskValue = roiMask[maskOffset];
						}

						if (!doROIMask ||
							(true == doROIMask &&
								(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
									((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
						{
							if (buildDigiWaveforms)
							{
								*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
							}
							*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
						}
						else
						{
							if (buildDigiWaveforms)
							{
								*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
							}
							*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
						}
					}
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
			}
		}

		waveforms[pockelOutputChannel] = pockelsCtrl;
		/*string waveformFile = "waveformBuffer" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_dataLength; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelsWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}*/

	}

	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	if (buildDigiWaveforms)
	{
		DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
		d0Ctrl.park_val = 0;
		d0Ctrl.offset_val = 0;
		d0Ctrl.update_rate = dac_rate;
		d0Ctrl.flyback_samples = backward_data_num;
		d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
		d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
		d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
		d0Ctrl.filterInhibit = true;
		d0Ctrl.hSync = true;
		d0Ctrl.enableEOFFreeze = false;

		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;
	}

	return TRUE;
}

LONG CThorDAQGalvoGalvo::BuildPockelsControlWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWaveforms)
{
	//!hardcode to use the same amount fo GG waveformBuffer for now
	bool blockBackwardLine = scanInfo->blockBackwardLine;
	double dac_rate = scanInfo->dacRate;

	ULONG32 samples_pad = scanLine->samples_idle;
	ULONG32 samples_sweep = scanLine->samples_scan;
	ULONG32 samples_single_line = TWO_WAY_SCAN == scanInfo->scanMode ? samples_pad * 2 + samples_sweep : 2 * (samples_pad * 2 + samples_sweep);
	ULONG32 fullLine = TWO_WAY_SCAN == scanInfo->scanMode ? samples_single_line * 2 : samples_single_line;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;
	UINT sync_secure = TWO_WAY_SCAN == scanInfo->scanMode ? scanLine->samples_idle : samples_single_line / 2 + scanLine->samples_idle;
	UINT half_turnaround_padding = (samples_single_line - scanLine->samples_scan) / 2;  //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_single_line;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(scanInfo->backward_lines) * fullLine - (sync_secure + FLYBACK_OFFSET); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(fullLine - (sync_secure + FLYBACK_OFFSET));
	backward_data_num = min(MAX_FLYBACK_CYCLE, backward_data_num);
	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)

	USHORT* pGalvoWaveformDigitalOutputs = NULL;
	if (buildDigiWaveforms)
	{
		if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
		{
			pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
		}
		else
		{
			pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		}
	}

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	long imageSize = width * height * sizeof(USHORT);

	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		bool pockelsEnable = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;

		if (pockelsEnable == FALSE || pockelPty->pockelsPowerLevel[pockelsIndex] == 0)
		{
			continue;
		}

		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];

		DAC_WAVEFORM_CRTL_STRUCT dacCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		dacCtrl.park_val = pockelPty->pockelsMinVoltage[pockelsIndex];
		dacCtrl.offset_val = pockelPty->pockelsMinVoltage[pockelsIndex];
		dacCtrl.update_rate = dac_rate;
		dacCtrl.flyback_samples = backward_data_num;
		dacCtrl.output_port = pockelOutputChannel;
		dacCtrl.waveform_buffer_size = total_dataLength * sizeof(USHORT);
		dacCtrl.filterInhibit = true;
		dacCtrl.hSync = true;
		dacCtrl.enableEOFFreeze = false;

		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			dacCtrl.waveformBuffer = waveforms[pockelOutputChannel].waveformBuffer;
		}
		else
		{
			dacCtrl.waveformBuffer = new USHORT[total_dataLength];
		}

		USHORT* pPockelsWaveform = dacCtrl.waveformBuffer;

		double pockelsOnVoltage;
		//the reference output is only used for pockels1, for everything else use the normal settings

		pockelsOnVoltage = pockelPty->pockelsMinVoltage[pockelsIndex] + (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * pockelPty->pockelsPowerLevel[pockelsIndex];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[pockelsIndex]) / GALVO_RESOLUTION));
		memset(pPockelsWaveform, pockels_output_low_ushort, total_dataLength * sizeof(USHORT)); //NOTE: Memset can only be used with 0
		ULONG32 samplesPercentBlank = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 lowSamples = samples_pad + samplesPercentBlank;
		ULONG32 endOfForwardLine = samples_pad + samples_sweep - static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 blankSamples = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 effectiveSamples = samples_sweep - 2 * blankSamples;
		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		ULONG32 twowayOffsetSamples = static_cast<ULONG32>(max(0, round(_daqAcqCfg.imageCtrl.alignmentOffset) / static_cast<ULONG32>((double)SYS_CLOCK_FREQ / dac_rate + pockelPty->pockelsDelayUS[pockelsIndex])));

		// set up the waveformBuffer for the horizontal galvo
		// first line
		ULONG32 sample_index = half_turnaround_padding + twowayOffsetSamples;// half_turnaround_padding + lowSamples + twowayOffsetSamples;

		if (FALSE == _pockelsFlybackBlank)
		{
			for (UINT i = 0; i < total_dataLength; ++i)
			{
				pGalvoWaveformDigitalOutputs[i] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelsWaveform[i] = pockels_output_high_ushort;
			}
		}

		bool doROIMask = pockelPty->pockelsMaskEnable[pockelsIndex] && _pockelsMaskSize[pockelsIndex] == imageSize && NULL != _pPockelsMask[pockelsIndex];

		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if (FALSE == _pockelsTurnAroundBlank && 0 == pockelPty->pockelsLineBlankingPercentage[pockelsIndex] && !doROIMask)
		{
			sample_index += lowSamples;
			ULONG32 frameSamples = forward_data_num - sample_index;
			for (ULONG32 i = 0; i < frameSamples; ++i)
			{
				if (buildDigiWaveforms)
				{
					*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
				}
				*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
			}
		}
		else
		{
			short* roiMask = (short*)_pPockelsMask[pockelsIndex];

			UINT passes = (TWO_WAY_SCAN == scanInfo->scanMode) ? ((UINT)((scanInfo->forward_lines * linesPerLine) / 2)) : ((UINT)(scanInfo->forward_lines * linesPerLine));
			if (blockBackwardLine)
			{
				passes = 1;
			}
			
			for (UINT j = 0; j < passes; ++j)
			{
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}

				UINT lineIndex = j / linesPerLine;
				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					long xOffset;
					if (FALSE != pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
					}
					else
					{
						xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
					}
					long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? lineIndex * (width * 2) + (width * 2 - xOffset - 1) : lineIndex * width + width - xOffset - 1;
					
					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					long roiMaskValue = 0;
					if (doROIMask)
					{
						roiMaskValue = roiMask[maskOffset];
					}

					if (!doROIMask ||
						(true == doROIMask &&
							(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
								((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
					}
					else
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
				}
				for (ULONG32 i = 0; i < lowSamples * 2; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}

				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					if (TWO_WAY_SCAN != scanInfo->scanMode || blockBackwardLine)
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
					else
					{
						long xOffset;
						if (FALSE != pImgAcqPty->horizontalFlip)
						{
							xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
						}
						else
						{
							xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
						}
						long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? lineIndex * (width * 2) + xOffset : lineIndex * width + xOffset;
						long roiMaskValue = 0;
						if (doROIMask)
						{
							roiMaskValue = roiMask[maskOffset];
						}

						if (!doROIMask ||
							(true == doROIMask &&
								(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
									((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
						{
							if (buildDigiWaveforms)
							{
								*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
							}
							*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
						}
						else
						{
							if (buildDigiWaveforms)
							{
								*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
							}
							*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
						}
					}
				}
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}
			}
		}

		waveforms[pockelOutputChannel] = dacCtrl;
		/*string waveformFile = "waveformBuffer" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_dataLength; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelsWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}*/

	}

	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	if (buildDigiWaveforms)
	{
		DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
		d0Ctrl.park_val = 0;
		d0Ctrl.offset_val = 0;
		d0Ctrl.update_rate = dac_rate;
		d0Ctrl.flyback_samples = backward_data_num;
		d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
		d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
		d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
		d0Ctrl.filterInhibit = true;

		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;
	}

	return TRUE;
}

LONG CThorDAQGalvoGalvo::BuildPockelsControlPowerRampFastOneWayWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWaveforms)
{
	bool blockBackwardLine = scanInfo->blockBackwardLine;
	double dac_rate = scanInfo->dacRate;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;

	UINT samples_line = 2 * scanLine->samples_idle + scanLine->samples_scan + scanLine->samples_back;
	ULONG32 fullLine = samples_line;
	UINT sync_secure = scanLine->samples_idle;
	UINT half_turnaround_padding = scanLine->samples_idle + scanLine->samples_back; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_line;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)min(MAX_FLYBACK_DAC_SAMPLES, (scanInfo->backward_lines) * fullLine - (sync_secure)); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback
	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	ULONG32 samples_pad = scanLine->samples_idle;
	ULONG32 samples_sweep = scanLine->samples_scan;
	ULONG32 samples_single_line = samples_pad * 2 + samples_sweep + scanLine->samples_back;

	USHORT* pGalvoWaveformDigitalOutputs = NULL;
	if (buildDigiWaveforms)
	{
		if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
		{
			pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
		}
		else
		{
			pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		}
	}

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	long imageSize = width * height * sizeof(USHORT);

	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		ULONG32 twowayOffsetSamples = static_cast<ULONG32>(max(0, round(_daqAcqCfg.imageCtrl.alignmentOffset) / static_cast<ULONG32>((double)SYS_CLOCK_FREQ / dac_rate + pockelPty->pockelsDelayUS[pockelsIndex])));

		bool pockelsEnable = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;
		if (pockelsEnable == FALSE || pockelPty->pockelsPowerLevel[pockelsIndex] == 0)
		{
			continue;
		}

		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];

		DAC_WAVEFORM_CRTL_STRUCT dacCtrl;

		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			dacCtrl = waveforms[pockelOutputChannel];
		}
		else
		{
			dacCtrl = DAC_WAVEFORM_CRTL_STRUCT();
			dacCtrl.park_val = pockelPty->pockelsMinVoltage[pockelsIndex];
			dacCtrl.offset_val = pockelPty->pockelsMinVoltage[pockelsIndex];
			dacCtrl.update_rate = dac_rate;
			dacCtrl.flyback_samples = backward_data_num;
			dacCtrl.output_port = pockelOutputChannel;
			dacCtrl.waveform_buffer_size = total_dataLength * sizeof(USHORT);
			dacCtrl.waveformBuffer = new USHORT[total_dataLength];
			dacCtrl.filterInhibit = true;
			dacCtrl.hSync = true;
		}

		USHORT* pPockelsWaveform = dacCtrl.waveformBuffer;

		double pockelsOnVoltage;
		//the reference output is only used for pockels1, for everything else use the normal settings

		pockelsOnVoltage = pockelPty->pockelsMinVoltage[pockelsIndex] + (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * pockelPty->pockelsPowerLevel[pockelsIndex];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[pockelsIndex]) / GALVO_RESOLUTION));

		USHORT pockels_output_high_start_ushort = pockels_output_high_ushort;
		double step = (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * (pockelPty->pockelsPowerLevel2[pockelsIndex] - pockelPty->pockelsPowerLevel[pockelsIndex]) / (double)waveform_dataLength;

		memset(pPockelsWaveform, pockels_output_low_ushort, total_dataLength * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		ULONG32 samplesPercentBlank = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 lowSamples = samples_pad + samplesPercentBlank;
		ULONG32 endOfForwardLine = samples_pad + samples_sweep - static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 blankSamples = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 effectiveSamples = samples_sweep - 2 * blankSamples;

		// set up the waveformBuffer for the horizontal galvo
		// first line
		ULONG32 sample_index = half_turnaround_padding + twowayOffsetSamples;

		if (FALSE == _pockelsFlybackBlank)
		{
			for (UINT i = 0; i < total_dataLength; ++i)
			{
				pGalvoWaveformDigitalOutputs[i] |= THORDAQ_DO_POCKELS_DIG_HIGH;
				pPockelsWaveform[i] = pockels_output_high_ushort;
			}
		}

		bool doROIMask = pockelPty->pockelsMaskEnable[pockelsIndex] && _pockelsMaskSize[pockelsIndex] == imageSize && NULL != _pPockelsMask[pockelsIndex];

		int s = sample_index + 1;
		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if (FALSE == _pockelsTurnAroundBlank && 0 == pockelPty->pockelsLineBlankingPercentage[pockelsIndex])
		{
			ULONG32 frameSamples = forward_data_num - sample_index;
			for (ULONG32 i = 0; i < frameSamples; ++i)
			{
				if (buildDigiWaveforms)
				{
					*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
				}
				*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
				pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
				++s;
			}
		}
		else
		{
			short* roiMask = (short*)_pPockelsMask[pockelsIndex];

			UINT passes = ((UINT)(scanInfo->forward_lines * linesPerLine));
			if (blockBackwardLine)
			{
				passes = 1;
			}
			for (UINT j = 1; j < passes; ++j)
			{
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}

				UINT lineIndex = j / linesPerLine;

				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					long xOffset;
					if (FALSE != pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
					}
					else
					{
						xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
					}

					long maskOffset = (pImgAcqPty->scanMode == TWO_WAY_SCAN) ? lineIndex * (width * 2) + (width * 2 - xOffset - 1) : lineIndex * width + width - xOffset - 1;
					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					long roiMaskValue = 0;
					if (doROIMask)
					{
						roiMaskValue = roiMask[maskOffset];
					}

					if (!doROIMask ||
						(true == doROIMask &&
							(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
								((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
					}
					else
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
				for (ULONG32 i = 0; i < scanLine->samples_back; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					pockels_output_high_ushort = pockels_output_high_start_ushort + static_cast<USHORT>(round((s * step) / GALVO_RESOLUTION));
					++s;
				}
			}
		}


		waveforms[pockelOutputChannel] = dacCtrl;
		/*string waveformFile = "waveformBuffer" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_dataLength; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelsWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}*/

	}

	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	if (buildDigiWaveforms)
	{
		DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
		d0Ctrl.park_val = 0;
		d0Ctrl.offset_val = 0;
		d0Ctrl.update_rate = dac_rate;
		d0Ctrl.flyback_samples = backward_data_num;
		d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
		d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
		d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
		d0Ctrl.filterInhibit = true;
		d0Ctrl.hSync = true;

		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;
	}

	return TRUE;
}

LONG CThorDAQGalvoGalvo::BuildPockelsControlFastOneWayWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, PockelPty* pockelPty, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms, bool buildDigiWaveforms)
{
	//!hardcode to use the same amount fo GG waveformBuffer for now
	bool blockBackwardLine = scanInfo->blockBackwardLine;
	double dac_rate = scanInfo->dacRate;

	UINT samples_line = 2 * scanLine->samples_idle + scanLine->samples_scan + scanLine->samples_back;
	ULONG32 fullLine = samples_line;

	UINT half_turnaround_padding = scanLine->samples_idle + scanLine->samples_back; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT sync_secure = scanLine->samples_idle;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_line;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)min(MAX_FLYBACK_DAC_SAMPLES, (scanInfo->backward_lines) * fullLine - (sync_secure)); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback
	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	ULONG32 samples_pad = scanLine->samples_idle;
	ULONG32 samples_sweep = scanLine->samples_scan;
	ULONG32 samples_single_line = samples_pad * 2 + samples_sweep + scanLine->samples_back;

	double maxVal = USHRT_MAX;
	double minVal = 0;

	USHORT* pGalvoWaveformDigitalOutputs = NULL;
	if (buildDigiWaveforms)
	{
		if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
		{
			pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
		}
		else
		{
			pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		}
	}

	long width = pImgAcqPty->pixelX;
	long height = pImgAcqPty->pixelY;

	long imageSize = width * height * sizeof(USHORT);

	for (int pockelsIndex = 0; pockelsIndex < MAX_POCKELS_CELL_COUNT; pockelsIndex++)
	{
		bool pockelsEnable = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)] >= 0 ? _pockelsEnable[pockelsIndex] : FALSE;
		if (pockelsEnable == FALSE || pockelPty->pockelsPowerLevel[pockelsIndex] == 0)
		{
			continue;
		}

		// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
		ULONG32 twowayOffsetSamples = static_cast<ULONG32>(max(0, round(_daqAcqCfg.imageCtrl.alignmentOffset) / static_cast<ULONG32>((double)SYS_CLOCK_FREQ / dac_rate + pockelPty->pockelsDelayUS[pockelsIndex])));

		long pockelOutputChannel = _imagingActiveAOSelection[(GG_AO)((int)GG_AO::Pockels0 + pockelsIndex)];

		DAC_WAVEFORM_CRTL_STRUCT dacCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		dacCtrl.park_val = pockelPty->pockelsMinVoltage[pockelsIndex];
		dacCtrl.offset_val = pockelPty->pockelsMinVoltage[pockelsIndex];
		dacCtrl.update_rate = dac_rate;
		dacCtrl.flyback_samples = backward_data_num;
		dacCtrl.output_port = pockelOutputChannel;
		dacCtrl.waveform_buffer_size = total_dataLength * sizeof(USHORT);
		dacCtrl.filterInhibit = true;
		dacCtrl.hSync = true;

		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			dacCtrl.waveformBuffer = waveforms[pockelOutputChannel].waveformBuffer;
		}
		else
		{
			dacCtrl.waveformBuffer = new USHORT[total_dataLength];
		}

		USHORT* pPockelsWaveform = dacCtrl.waveformBuffer;

		double pockelsOnVoltage;
		//the reference output is only used for pockels1, for everything else use the normal settings

		pockelsOnVoltage = pockelPty->pockelsMinVoltage[pockelsIndex] + (pockelPty->pockelsMaxVoltage[pockelsIndex] - pockelPty->pockelsMinVoltage[pockelsIndex]) * pockelPty->pockelsPowerLevel[pockelsIndex];

		USHORT pockels_output_low_ushort = 0;
		USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[pockelsIndex]) / GALVO_RESOLUTION));
		memset(pPockelsWaveform, pockels_output_low_ushort, total_dataLength * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		ULONG32 samplesPercentBlank = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 lowSamples = samples_pad + samplesPercentBlank;
		ULONG32 blankSamples = static_cast<ULONG32>(ceil(pockelPty->pockelsLineBlankingPercentage[pockelsIndex] * samples_sweep - 0.5));
		ULONG32 effectiveSamples = samples_sweep - 2 * blankSamples;

		// set up the waveformBuffer for the horizontal galvo
		// first line
		ULONG32 sample_index = half_turnaround_padding + twowayOffsetSamples;

		bool doROIMask = pockelPty->pockelsMaskEnable[pockelsIndex] && _pockelsMaskSize[pockelsIndex] == imageSize && NULL != _pPockelsMask[pockelsIndex];

		//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
		if (FALSE == _pockelsTurnAroundBlank && 0 == pockelPty->pockelsLineBlankingPercentage[pockelsIndex])
		{
			sample_index += lowSamples;
			ULONG32 frameSamples = forward_data_num - sample_index;
			for (ULONG32 i = 0; i < frameSamples; ++i)
			{
				if (buildDigiWaveforms)
				{
					*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
				}
				*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
			}
		}
		else
		{
			short* roiMask = (short*)_pPockelsMask[pockelsIndex];

			UINT passes = ((UINT)(scanInfo->forward_lines * linesPerLine));
			if (blockBackwardLine)
			{
				passes = 1;
			}
			for (UINT j = 0; j < passes; ++j)
			{
				UINT lineIndex = j / linesPerLine;

				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}

				for (ULONG32 i = 0, lineSampleIndex = samplesPercentBlank; i < effectiveSamples; ++i, ++lineSampleIndex)
				{
					long xOffset;
					if (FALSE != pImgAcqPty->horizontalFlip)
					{
						xOffset = static_cast<long>(round((width - 1) * (double)lineSampleIndex / (double)samples_sweep));
					}
					else
					{
						xOffset = static_cast<long>(round((width - 1) * (1.0 - (double)lineSampleIndex / (double)samples_sweep)));
					}

					long maskOffset = lineIndex * width + width - xOffset - 1;
					if (maskOffset < 0)
					{
						maskOffset = 0;
					}

					long roiMaskValue = 0;
					if (doROIMask)
					{
						roiMaskValue = roiMask[maskOffset];
					}
					if (!doROIMask ||
						(true == doROIMask &&
							(((roiMaskValue > 0) && !pockelPty->pockelsMaskInvert[pockelsIndex]) ||
								((roiMaskValue <= 0) && pockelPty->pockelsMaskInvert[pockelsIndex]))))
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_POCKELS_DIG_HIGH;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_high_ushort;
					}
					else
					{
						if (buildDigiWaveforms)
						{
							*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
						}
						*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
					}
				}
				for (ULONG32 i = 0; i < lowSamples; ++i)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}
				for (ULONG32 i = 0; i < scanLine->samples_back; i++)
				{
					if (buildDigiWaveforms)
					{
						*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
					}
					*(pPockelsWaveform + sample_index++) = pockels_output_low_ushort;
				}
			}
		}

		waveforms[pockelOutputChannel] = dacCtrl;
		/*string waveformFile = "waveformBuffer" + to_string(i + 2)+".txt";
		ofstream myfile (waveformFile);
		if (myfile.is_open())
		{
		for (int i = 0; i < total_dataLength; i++)
		{
		myfile << std::fixed << std::setprecision(8) << (*(pPockelsWaveform+i));
		myfile << "\n";
		}
		myfile.close();
		}*/

	}

	/* Print out pGalvoWaveformDigitalOutputs for debugging
	ofstream myfile ("digitalWaveform.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformDigitalOutputs+i));
	myfile << "\n";
	}
	myfile.close();
	}
	*/

	//--------------Digital waveforms--------------
	// Load now, after the Galvo and  Pockels waveformBuffer's digital part has been added
	if (buildDigiWaveforms)
	{
		DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
		d0Ctrl.park_val = 0;
		d0Ctrl.offset_val = 0;
		d0Ctrl.update_rate = dac_rate;
		d0Ctrl.flyback_samples = backward_data_num;
		d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
		d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
		d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
		d0Ctrl.filterInhibit = true;
		d0Ctrl.hSync = true;
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;
	}

	return TRUE;
}

long WaveformResampler(USHORT* inputArray, size_t inputArrayLength, double inputRate, double outputRate, USHORT*& outputArray, size_t& outputArrayLength)
{
	long ret = FALSE;
	double ratio = outputRate / inputRate;
	//floor this because we can't interpolate beyond the input array.
	outputArrayLength = (size_t)floor((double)inputArrayLength * ratio) - 1;
	//output_array = new int[outputArrayLength];


	USHORT* tempOutputArray = new USHORT[outputArrayLength];
	outputArray = new USHORT[outputArrayLength];
	for (int c = 0; c < outputArrayLength; c++)
	{
		//find the samples from the input array that are on either side of the sample we need
		double ratio_index = c / ratio;
		int floorInt, ceilInt;
		floorInt = (int)floor(ratio_index);
		ceilInt = (int)ceil(ratio_index);
		//calcualte the fractions that come from each of those two samples (linear interpolation)
		double floor_part = inputArray[floorInt] * (1 - (ratio_index - floorInt));
		double ceil_part = inputArray[ceilInt] * (1 - (ceilInt - ratio_index));
		if (floorInt == ceilInt) //if it's exactly on a sample, there is no interpolation
		{
			floor_part = 0;
		}
		tempOutputArray[c] = (int)round(floor_part + ceil_part);
	}

	const size_t FILTER_OFFSET_SAMPLES = 150;

	if (outputArrayLength > FILTER_OFFSET_SAMPLES)
	{
		memcpy(outputArray, tempOutputArray + FILTER_OFFSET_SAMPLES, (outputArrayLength - FILTER_OFFSET_SAMPLES) * sizeof(USHORT));
		memcpy(outputArray + (outputArrayLength - FILTER_OFFSET_SAMPLES), tempOutputArray, FILTER_OFFSET_SAMPLES * sizeof(USHORT));
		ret = TRUE;
	}

	delete[] tempOutputArray;

	return ret;
}

LONG CThorDAQGalvoGalvo::BuildGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms)
{
	double dac_rate = scanInfo->dacRate;
	UINT samples_line = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * scanLine->samples_idle + scanLine->samples_scan : 2 * (2 * scanLine->samples_idle + scanLine->samples_scan);
	ULONG32 fullLine = TWO_WAY_SCAN == pImgAcqPty->scanMode ? samples_line * 2 : samples_line;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;
	UINT half_turnaround_padding = (samples_line - scanLine->samples_scan) / 2; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT sync_secure = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2 + scanLine->samples_idle;
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_line;
	UINT sync_securebw = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(scanInfo->backward_lines) * fullLine - (sync_securebw + FLYBACK_OFFSET); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(fullLine - (sync_secure + FLYBACK_OFFSET));

	backward_data_num = min(MAX_FLYBACK_DAC_SAMPLES, backward_data_num);

	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(fullLine * scanInfo->backward_lines - (sync_secure + FLYBACK_OFFSET));
	flybackSteps = min(MAX_FLYBACK_DAC_SAMPLES, flybackSteps);
	double galvoYBwdStep = galvoCtrlY->amplitude / flybackSteps / GALVO_RESOLUTION;

	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	/// Calculate the waveformBuffer of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion
	double linesToScan = TWO_WAY_SCAN == pImgAcqPty->scanMode ? (scanInfo->forward_lines * linesPerLine) / 2 : scanInfo->forward_lines * linesPerLine;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrlY->scan_direction) ? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrlY->amplitude / linesToScan / GALVO_RESOLUTION);
	double vPadY = galvoYFwdStep / 4.0;
	//double galvoYBwdRage = (galvoCtrlY->amplitude - galvoCtrlY->amplitude / linesToScan /4);
	//double galvoYBwdStep = galvoCtrlY->amplitude / (double)(backward_data_num) / GALVO_RESOLUTION;
	double half_P2P_amp_Y = galvoCtrlY->amplitude / 2.0 / GALVO_RESOLUTION;
	double half_P2P_amp_X = galvoCtrlX->amplitude / 2.0 / GALVO_RESOLUTION;

	//For X Waveform only
	double pad_amp = (galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle / M_PI_2) / GALVO_RESOLUTION;
	double pad_step = M_PI_2 / (double)scanLine->samples_idle;
	double sweep_amp_step = galvoCtrlX->amplitude / (double)scanLine->samples_scan / GALVO_RESOLUTION;

	double waveOffsetx = -(galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle / M_PI_2) - galvoCtrlX->amplitude / 2.0;
	double waveOffsety = pImgAcqPty->galvoEnable ? -yDirection * galvoCtrlY->amplitude / 2.0 : 0;

	double offset_x = galvoCtrlX->offset + waveOffsetx;
	offset_x = offset_x > 0 ? min(MAX_GALVO_VOLTAGE, offset_x) : max(MIN_GALVO_VOLTAGE, offset_x);

	double offset_y = galvoCtrlY->offset + waveOffsety;
	offset_y = offset_y > 0 ? min(MAX_GALVO_VOLTAGE, offset_y) : max(MIN_GALVO_VOLTAGE, offset_y);

	//----------------Allocate space for digital waveforms--------------------------
	USHORT* pGalvoWaveformDigitalOutputs;

	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = DAC_WAVEFORM_CRTL_STRUCT();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer = pGalvoWaveformDigitalOutputs;
	}


	//----------------Build X Waveform----------------
	DAC_WAVEFORM_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_CRTL_STRUCT();


	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_X]) != waveforms.end())
	{
		gXCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_X]].waveformBuffer;
	}
	else
	{
		gXCtrl.waveformBuffer = new USHORT[(total_dataLength)];
	}


	short* pGalvoWaveformX = (short*)gXCtrl.waveformBuffer;
	memset(pGalvoWaveformX, 0, total_dataLength * sizeof(short));

	ULONG32 sample_index = half_turnaround_padding;

	//for (int j = 0; j < (int)scanInfo->average_lines_num; ++j)
	{
		for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
		{
			*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
			*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(pad_amp - pad_amp * cos(pad_step * double(i)))); // pi / 2
		}
		for (ULONG32 i = 0; i < scanLine->samples_scan; ++i)
		{
			*(pGalvoWaveformDigitalOutputs + sample_index) |= LINE_TRIGGER_OUT_HIGH;
			*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(pad_amp + sweep_amp_step * (double)i));
		}
		for (ULONG32 i = 0; i < scanLine->samples_idle * 2; ++i)
		{
			*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
			*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(half_P2P_amp_X * 2 + pad_amp - pad_amp * cos(M_PI_2 + pad_step * double(i))));  // pi /2 - pi 3/2
		}
		for (ULONG32 i = 0; i < scanLine->samples_scan; ++i)
		{
			if (TWO_WAY_SCAN == pImgAcqPty->scanMode)
			{
				*(pGalvoWaveformDigitalOutputs + sample_index) |= LINE_TRIGGER_OUT_HIGH;
			}
			else
			{
				*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
			}
			*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(half_P2P_amp_X * 2 + pad_amp - sweep_amp_step * (double)i));
		}
		for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
		{
			*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
			*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(pad_amp - pad_amp * cos(1.5 * M_PI + pad_step * double(i)))); // pi 3/2 - 2 pi 
		}

		// copy first line's settings to the rest lines
		if (TWO_WAY_SCAN == pImgAcqPty->scanMode)
		{
			for (UINT i = 1; i < ((UINT)((scanInfo->forward_lines * scanInfo->average_lines_num) / 2)); ++i)
			{
				memcpy((pGalvoWaveformDigitalOutputs + (half_turnaround_padding + samples_line * 2 * i)), (pGalvoWaveformDigitalOutputs + half_turnaround_padding), samples_line * 2 * sizeof(short));
				memcpy((pGalvoWaveformX + (half_turnaround_padding + samples_line * 2 * i)), (pGalvoWaveformX + half_turnaround_padding), samples_line * 2 * sizeof(short));
			}
		}
		else
		{
			for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines * linesPerLine)); ++i)
			{
				memcpy((pGalvoWaveformDigitalOutputs + (half_turnaround_padding + samples_line * i)), (pGalvoWaveformDigitalOutputs + half_turnaround_padding), samples_line * sizeof(short));
				memcpy((pGalvoWaveformX + (half_turnaround_padding + samples_line * i)), (pGalvoWaveformX + half_turnaround_padding), samples_line * sizeof(short));
			}
		}
	}

	//----------------Build Y Waveform----------------

	DAC_WAVEFORM_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_CRTL_STRUCT();

	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_Y]) != waveforms.end())
	{
		gYCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]].waveformBuffer;
	}
	else
	{
		gYCtrl.waveformBuffer = new USHORT[total_dataLength];
	}

	short* pGalvoWaveformY = (short*)gYCtrl.waveformBuffer;

	// set up the waveformBuffer for the vertical galvo
	memset(pGalvoWaveformY, 0, total_dataLength * sizeof(USHORT));

	unsigned long k = half_turnaround_padding;
	bool alwaysStep = linesPerLine == 1;
	// For 'invert vertical scan' we multiply the waveformBuffer value by -1.0 same as the NI/Alazar counterpart. The negative value is cast to
	//unsigned short. This will convert it to UMAX SHORT - waveformBuffer value, which works for the vertical invert. This is why we don't check if
	//the waveformBuffer value is below 0
	if (pImgAcqPty->galvoEnable)
	{
		for (unsigned long i = 0; i < (UINT)linesToScan; ++i)
		{
			bool doStepFWD = alwaysStep || (i % linesPerLine) == 0;
			bool doStepBWD = alwaysStep || (i % (linesPerLine - 1)) == 0 && i > 0;
			for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
			{
				if (doStepFWD)
				{
					pGalvoWaveformY[k] = static_cast<short>(round(yDirection * (galvoYFwdStep * (double)i + vPadY * sin((double)j * pad_step))));
					// imaging hasn't started at this point yet when i == 0
					if (0 != i)
					{
						pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
					}
					else
					{
						pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
					}
				}
				else
				{
					pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
					// imaging hasn't started at this point yet when i == 0
					if (0 != i)
					{
						pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
					}
					else
					{
						pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
					}
				}
			}
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
			//the imaging portion is done here for one way mode
			if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
			{
				for (unsigned long j = 0; j < scanLine->samples_idle * 2; ++j)
				{
					if (alwaysStep)
					{
						pGalvoWaveformY[k] = static_cast<short>(round(yDirection * (galvoYFwdStep * (double)i + vPadY * (2 + (sin((double)j * pad_step - M_PI_2))))));
					}
					else
					{
						pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
					}
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
			//the imaging portion is done here for one way mode
			if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
			{
				for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
				{
					pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
			//the imaging portion is done here
			if (i != linesToScan - 1)
			{
				for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
				{
					if (doStepBWD)
					{
						pGalvoWaveformY[k] = static_cast<short>(round(yDirection * (galvoYFwdStep * (double)i + vPadY * (3 + sin((double)j * pad_step)))));
					}
					else
					{
						pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
					}
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
		}

		for (unsigned long i = 0; k < waveform_dataLength; ++i)
		{
			pGalvoWaveformY[k] = static_cast<short>(round(yDirection * (half_P2P_amp_Y * 2 - galvoYBwdStep * (i + 1))));

			if (pGalvoWaveformY[k] == 0)
			{
				break;
			}
			pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
		}
	}
	else
	{
		for (unsigned long i = 0; i < (UINT)round(linesToScan); ++i)
		{
			for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
			{
				// imaging hasn't started at this point yet when i == 0
				if (0 != i)
				{
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
				else
				{
					pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
				}
			}
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
			//the imaging portion is done here for one way mode
			if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
			{
				for (unsigned long j = 0; j < scanLine->samples_idle * 2; ++j)
				{
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
			//the imaging portion is done here for one way mode
			if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
			{
				for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
				{
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
			//the imaging portion is done here
			if (i != linesToScan - 1)
			{
				for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
				{
					pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
				}
			}
		}
	}


	//TODO: flyback should be set already at memeset (total waveformBuffer lenght) to 0. Delete if there is no issue with not having this
	/*if (backward_data_num >= half_turnaround_padding)
	{
	for (ULONG32 i = 0; i < (ULONG32)backward_data_num - half_turnaround_padding; ++i)
	{
	*(pGalvoWaveformX+((UINT)forward_data_num + half_turnaround_padding + i)) = 0;
	}
	}*/

	//----------------Rotate Galvo XY Waveform----------------
	if (0 != pImgAcqPty->scanAreaAngle)
	{
		double angle = (1 == pImgAcqPty->verticalScanDirection) ? pImgAcqPty->scanAreaAngle : -pImgAcqPty->scanAreaAngle;
		double tempX0 = 0, tempY0 = 0, tempX1 = 0, tempY1 = 0;
		short offsetX = 0;
		short offsetY = 0;

		for (unsigned long i = 0; i < static_cast<unsigned long>(total_dataLength); ++i)
		{
			//convert back to double number centered around 0
			tempX0 = pGalvoWaveformX[i] * GALVO_RESOLUTION + waveOffsetx;
			tempY0 = pGalvoWaveformY[i] * GALVO_RESOLUTION + waveOffsety;

			//calculate transform
			tempX1 = tempX0 * cos(angle) - tempY0 * sin(angle);
			tempY1 = tempX0 * sin(angle) + tempY0 * cos(angle);

			pGalvoWaveformX[i] = static_cast<short>(round(tempX1 * VOLT_TO_THORDAQ_VAL)) - offsetX;

			pGalvoWaveformY[i] = static_cast<short>(round(tempY1 * VOLT_TO_THORDAQ_VAL)) - offsetY;

			//get the value of the first sample and make it the offset, then make the first sample 0 so the waveform always at zero
			if (0 == i)
			{
				offsetX = pGalvoWaveformX[i];
				pGalvoWaveformX[i] = 0;

				offsetY = pGalvoWaveformY[i];
				pGalvoWaveformY[i] = 0;
			}
		}

		//update the offset with a transformed offset for the angled X and Y waveforms
		offset_x = gXCtrl.offset_val = offsetX * GALVO_RESOLUTION + galvoCtrlX->offset;
		offset_y = gYCtrl.offset_val = offsetY * GALVO_RESOLUTION + galvoCtrlY->offset;

		if (offset_x > MAX_GALVO_VOLTAGE || offset_x < MIN_GALVO_VOLTAGE || offset_y > MAX_GALVO_VOLTAGE || offset_y < MIN_GALVO_VOLTAGE)
		{
			wchar_t errMsg2[MSG_SIZE];
			StringCbPrintfW(errMsg2, MSG_SIZE, L"ThorDAQGalvoGalvo BuildGalvoWaveforms Offsets out of range, OffsetX: %f OffsetY: %f", offset_x, offset_y);
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg2, ERROR_EVENT);
			return FALSE;
		}
	}

	gXCtrl.park_val = galvoCtrlX->park;
	gXCtrl.offset_val = offset_x;
	gXCtrl.update_rate = dac_rate;
	gXCtrl.flyback_samples = backward_data_num;
	gXCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_X];
	gXCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gXCtrl.filterInhibit = false;
	gXCtrl.hSync = true;

	gYCtrl.park_val = galvoCtrlY->park;
	gYCtrl.offset_val = offset_y;
	gYCtrl.update_rate = dac_rate;
	gYCtrl.flyback_samples = backward_data_num;
	gYCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_Y];
	gYCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gYCtrl.filterInhibit = false;
	gYCtrl.hSync = true;
	//-------------------------------------------------

	////print data for matlab
	/*
	ofstream myfile1 ("waveformY.txt");
	if (myfile1.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile1 << std::fixed << std::setprecision(8) << (*(pGalvoWaveformY+i));
	myfile1 << "\n";
	}
	myfile1.close();
	}

	ofstream myfile ("waveformX.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformX+i));
	myfile << "\n";
	}
	myfile.close();
	}*/

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvo Samples idle: %d \nSamples Scan: %d \n OffsetX: %f \n OffsetY: %f", scanLine->samples_idle, scanLine->samples_scan, offset_x, offset_y);
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);

	//if (1 == _scannerType)
	//{
	//	double ratio = _maxScannerSampleRate / dac_rate;
	//	//floor this because we can't interpolate beyond the input array.
	//	UINT64 flybackSamples = (UINT64)floor((double)backward_data_num * ratio) - 1;
	//	gYCtrl.flyback_samples = flybackSamples;
	//	USHORT* downsampledArrayY;
	//	size_t downSampleArrayLengthY = 0;
	//	WaveformResampler(gYCtrl.waveformBuffer, gYCtrl.waveform_buffer_size / sizeof(USHORT), dac_rate, _maxScannerSampleRate, downsampledArrayY, downSampleArrayLengthY);
	//	delete[] gYCtrl.waveformBuffer;
	//	gYCtrl.waveformBuffer = downsampledArrayY;
	//	gYCtrl.waveform_buffer_size = downSampleArrayLengthY * sizeof(USHORT);
	//	gYCtrl.update_rate = _maxScannerSampleRate;
	//	gYCtrl.enableWaveformFilter = true;

	//	gXCtrl.flyback_samples = flybackSamples;
	//	USHORT* downsampledArrayX;
	//	size_t downSampleArrayLengthX = 0;
	//	WaveformResampler(gXCtrl.waveformBuffer, gXCtrl.waveform_buffer_size / sizeof(USHORT), dac_rate, _maxScannerSampleRate, downsampledArrayX, downSampleArrayLengthX);
	//	delete[] gXCtrl.waveformBuffer;
	//	gXCtrl.waveformBuffer = downsampledArrayX;
	//	gXCtrl.waveform_buffer_size = downSampleArrayLengthX * sizeof(USHORT);
	//	gXCtrl.update_rate = _maxScannerSampleRate;
	//	gXCtrl.enableWaveformFilter = true;
	//}

	//--------------Analog Y galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]] = gYCtrl;

	//--------------Analog X galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_X]] = gXCtrl;


	DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
	d0Ctrl.park_val = 0;
	d0Ctrl.offset_val = 0;
	d0Ctrl.update_rate = dac_rate;
	d0Ctrl.flyback_samples = backward_data_num;
	d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
	d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
	d0Ctrl.filterInhibit = true;

	waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;

	return TRUE;
}

LONG CThorDAQGalvoGalvo::BuildFastOneWayGalvoWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms)
{
	double dac_rate = scanInfo->dacRate;
	UINT samples_line = 2 * scanLine->samples_idle + scanLine->samples_scan + scanLine->samples_back;
	ULONG32 fullLine = samples_line;
	UINT half_turnaround_padding = scanLine->samples_idle + scanLine->samples_back; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;

	UINT sync_secure = scanLine->samples_idle;
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * samples_line * linesPerLine;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)min(MAX_FLYBACK_DAC_SAMPLES, (scanInfo->backward_lines) * fullLine - (sync_secure)); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback
	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)

	/// Calculate the waveformBuffer of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion
	double linesToScan = scanInfo->forward_lines * linesPerLine;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrlY->scan_direction) ? 1.0 : -1.0;
	double galvoYFwdStep = (galvoCtrlY->amplitude / (linesToScan - 1) / GALVO_RESOLUTION);
	double back_step_Y = M_PI / (2.0 * (double)scanLine->samples_idle + (double)scanLine->samples_back - 1);
	double back_step_X = M_PI / ((double)scanLine->samples_back - 1);

	double half_P2P_amp_Y = galvoCtrlY->amplitude / 2.0 / GALVO_RESOLUTION;
	double half_P2P_amp_X = galvoCtrlX->amplitude / 2.0 / GALVO_RESOLUTION;

	//For X Waveform only
	double pad_amp = (galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle / M_PI_2) / GALVO_RESOLUTION;
	double pad_step = M_PI_2 / (double)scanLine->samples_idle;
	double sweep_amp_step = galvoCtrlX->amplitude / (double)scanLine->samples_scan / GALVO_RESOLUTION;

	double waveOffsetx = -(galvoCtrlX->amplitude / (double)scanLine->samples_scan * (double)scanLine->samples_idle / M_PI_2) - galvoCtrlX->amplitude / 2.0;
	double waveOffsety = pImgAcqPty->galvoEnable ? -yDirection * galvoCtrlY->amplitude / 2.0 : 0;

	double offset_x = galvoCtrlX->offset + waveOffsetx;
	offset_x = offset_x > 0 ? min(MAX_GALVO_VOLTAGE, offset_x) : max(MIN_GALVO_VOLTAGE, offset_x);

	double offset_y = galvoCtrlY->offset + waveOffsety;
	offset_y = offset_y > 0 ? min(MAX_GALVO_VOLTAGE, offset_y) : max(MIN_GALVO_VOLTAGE, offset_y);

	bool alwaysStep = linesPerLine == 1;

	//----------------Allocate space for digital waveforms--------------------------
	USHORT* pGalvoWaveformDigitalOutputs;

	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = DAC_WAVEFORM_CRTL_STRUCT();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer = pGalvoWaveformDigitalOutputs;
	}

	//----------------Build X Waveform----------------
	DAC_WAVEFORM_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gXCtrl.park_val = galvoCtrlY->park;
	gXCtrl.offset_val = offset_x;
	gXCtrl.update_rate = dac_rate;
	gXCtrl.flyback_samples = backward_data_num;
	gXCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_X];
	gXCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gXCtrl.filterInhibit = false;
	gXCtrl.hSync = true;

	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_X]) != waveforms.end())
	{
		gXCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_X]].waveformBuffer;
	}
	else
	{
		gXCtrl.waveformBuffer = new USHORT[total_dataLength]();
	}

	short* pGalvoWaveformX = (short*)gXCtrl.waveformBuffer;
	memset(pGalvoWaveformX, 0, total_dataLength * sizeof(short));

	ULONG32 sample_index = half_turnaround_padding;
	for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
	{
		*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
		*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(pad_amp - pad_amp * cos(pad_step * double(i)))); // pi / 2
	}
	for (ULONG32 i = 0; i < scanLine->samples_scan; ++i)
	{
		*(pGalvoWaveformDigitalOutputs + sample_index) |= LINE_TRIGGER_OUT_HIGH;
		*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(pad_amp + sweep_amp_step * (double)i));
	}
	for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
	{
		*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
		*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(half_P2P_amp_X * 2 + pad_amp - pad_amp * cos(M_PI_2 + pad_step * double(i))));  // pi /2 - pi 3/2
	}
	for (ULONG32 i = 0; i < scanLine->samples_back; ++i)
	{
		*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
		*(pGalvoWaveformX + sample_index++) = static_cast<short>(round(half_P2P_amp_X + pad_amp + (half_P2P_amp_X + pad_amp) * cos(back_step_X * double(i))));  // pi /2 - pi 3/2
	}
	//for (ULONG32 i = 0; i < scanLine->samples_idle; ++i)
	//{
	//	*(pGalvoWaveformDigitalOutputs + sample_index) |= THORDAQ_DO_LOW;
	//	*(pGalvoWaveformX + sample_index++) = static_cast<USHORT>(max(minVal, min(maxVal, round(pad_amp - pad_amp * cos(1.5 * M_PI + pad_step * double(i)))))); // pi 3/2 - 2 pi 
	//}

	// copy first line's settings to the rest lines

	for (UINT i = 1; i < linesToScan; ++i)
	{
		memcpy((pGalvoWaveformDigitalOutputs + half_turnaround_padding + samples_line * i), (pGalvoWaveformDigitalOutputs + half_turnaround_padding), samples_line * sizeof(short));
		memcpy((pGalvoWaveformX + half_turnaround_padding + samples_line * i), (pGalvoWaveformX + half_turnaround_padding), samples_line * sizeof(short));
	}

	//----------------Build Y Waveform----------------

	DAC_WAVEFORM_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_CRTL_STRUCT();

	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_Y]) != waveforms.end())
	{
		gYCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]].waveformBuffer;
	}
	else
	{
		gYCtrl.waveformBuffer = new USHORT[total_dataLength];
	}

	short* pGalvoWaveformY = (short*)gYCtrl.waveformBuffer;

	// set up the waveformBuffer for the vertical galvo
	memset(pGalvoWaveformY, 0, total_dataLength * sizeof(short));

	unsigned long k = half_turnaround_padding + scanLine->samples_idle;

	// For 'invert vertical scan' we multiply the waveformBuffer value by -1.0 same as the NI/Alazar counterpart. The negative value is cast to
	//unsigned short. This will convert it to UMAX SHORT - waveformBuffer value, which works for the vertical invert. This is why we don't check if
	//the waveformBuffer value is below 0
	if (pImgAcqPty->galvoEnable)
	{
		for (unsigned long i = 0; i < (linesToScan - 1); ++i)
		{
			bool doStepBWD = alwaysStep || (i % (linesPerLine - 1)) == 0 && i > 0;
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
			for (unsigned long j = 0; j < (2 * scanLine->samples_idle + scanLine->samples_back); j++)
			{
				if (doStepBWD)
				{
					pGalvoWaveformY[k] = static_cast<short>(round(yDirection * (galvoYFwdStep * (double)i + galvoYFwdStep * (0.5 + 0.5 * sin((double)j * back_step_Y - M_PI_2)))));
				}
				else
				{
					pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
				}
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
		}

		//last line
		for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
		{
			pGalvoWaveformY[k] = pGalvoWaveformY[k - 1];
			pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
		}

		for (unsigned long i = 0; i < backward_data_num; ++i)
		{
			pGalvoWaveformY[k] = static_cast<short>((yDirection * (galvoYFwdStep * (scanInfo->forward_lines * linesPerLine - 1) * (cos(M_PI * (double)(i + 1) / (double)(backward_data_num)) / 2.0 + 0.5))));
			pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
		}
	}
	else
	{
		for (unsigned long i = 0; i < linesToScan - 1; ++i)
		{
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
			for (unsigned long j = 0; j < (2 * scanLine->samples_idle + scanLine->samples_back); j++)
			{
				pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
			}
		}
		//last line
		for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
		{
			pGalvoWaveformDigitalOutputs[k++] |= FRAME_TRIGGER_OUT_HIGH;
		}

		for (unsigned long i = 0; i < backward_data_num; ++i)
		{
			pGalvoWaveformDigitalOutputs[k++] |= THORDAQ_DO_LOW;
		}
	}

	//TODO: flyback should be set already at memeset (total waveformBuffer lenght) to 0. Delete if there is no issue with not having this
	/*if (backward_data_num >= half_turnaround_padding)
	{
	for (ULONG32 i = 0; i < (ULONG32)backward_data_num - half_turnaround_padding; ++i)
	{
	*(pGalvoWaveformX+((UINT)forward_data_num + half_turnaround_padding + i)) = 0;
	}
	}*/

	//----------------Rotate Galvo XY Waveform----------------
	if (0 != pImgAcqPty->scanAreaAngle)
	{
		double angle = (1 == pImgAcqPty->verticalScanDirection) ? pImgAcqPty->scanAreaAngle : -pImgAcqPty->scanAreaAngle;
		double tempX0 = 0, tempY0 = 0, tempX1 = 0, tempY1 = 0;
		short offsetX = 0;
		short offsetY = 0;

		for (unsigned long i = 0; i < static_cast<unsigned long>(total_dataLength); ++i)
		{
			//convert back to double number centered around 0
			tempX0 = pGalvoWaveformX[i] * GALVO_RESOLUTION + waveOffsetx;
			tempY0 = pGalvoWaveformY[i] * GALVO_RESOLUTION + waveOffsety;

			//calculate transform
			tempX1 = tempX0 * cos(angle) - tempY0 * sin(angle);
			tempY1 = tempX0 * sin(angle) + tempY0 * cos(angle);

			pGalvoWaveformX[i] = static_cast<short>(round(tempX1 * VOLT_TO_THORDAQ_VAL)) - offsetX;

			pGalvoWaveformY[i] = static_cast<short>(round(tempY1 * VOLT_TO_THORDAQ_VAL)) - offsetY;

			//get the value of the first sample and make it the offset, then make the first sample 0 so the waveform always at zero
			if (0 == i)
			{
				offsetX = pGalvoWaveformX[i];
				pGalvoWaveformX[i] = 0;

				offsetY = pGalvoWaveformY[i];
				pGalvoWaveformY[i] = 0;
			}
		}

		//update the offset with a transformed offset for the angled X and Y waveforms
		offset_x = gXCtrl.offset_val = offsetX * GALVO_RESOLUTION + galvoCtrlX->offset;
		offset_y = gYCtrl.offset_val = offsetY * GALVO_RESOLUTION + galvoCtrlY->offset;

		if (offset_x > MAX_GALVO_VOLTAGE || offset_x < MIN_GALVO_VOLTAGE || offset_y > MAX_GALVO_VOLTAGE || offset_y < MIN_GALVO_VOLTAGE)
		{
			wchar_t errMsg2[MSG_SIZE];
			StringCbPrintfW(errMsg2, MSG_SIZE, L"ThorDAQGalvoGalvo BuildFastOneWayGalvoWaveforms Offsets out of range, OffsetX: %f OffsetY: %f", offset_x, offset_y);
			CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg2, ERROR_EVENT);
			return FALSE;
		}
	}

	gXCtrl.park_val = galvoCtrlX->park;
	gXCtrl.offset_val = offset_x;
	gXCtrl.update_rate = dac_rate;
	gXCtrl.flyback_samples = backward_data_num;
	gXCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_X];
	gXCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gXCtrl.filterInhibit = false;
	gXCtrl.hSync = true;

	gYCtrl.park_val = galvoCtrlY->park;
	gYCtrl.offset_val = offset_y;
	gYCtrl.update_rate = dac_rate;
	gYCtrl.flyback_samples = backward_data_num;
	gYCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_Y];
	gYCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gYCtrl.filterInhibit = false;
	gYCtrl.hSync = true;
	//-------------------------------------------------

	////print data for matlab
	/*
	ofstream myfile1 ("waveformY.txt");
	if (myfile1.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile1 << std::fixed << std::setprecision(8) << (*(pGalvoWaveformY+i));
	myfile1 << "\n";
	}
	myfile1.close();
	}

	ofstream myfile ("waveformX.txt");
	if (myfile.is_open())
	{
	for (UINT i = 0; i < total_dataLength; i++)
	{
	myfile << std::fixed << std::setprecision(8) << (*(pGalvoWaveformX+i));
	myfile << "\n";
	}
	myfile.close();
	}*/

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvo Samples idle: %d \nSamples Scan: %d \n OffsetX: %f \n OffsetY: %f", scanLine->samples_idle, scanLine->samples_scan, offset_x, offset_y);
	CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);

	//if (1 == _scannerType)
	//{
	//	double ratio = _maxScannerSampleRate / dac_rate;
	//	//floor this because we can't interpolate beyond the input array.
	//	UINT64 flybackSamples = (UINT64)floor((double)backward_data_num * ratio) - 1;
	//	gYCtrl.flyback_samples = flybackSamples;
	//	USHORT* downsampledArrayY;
	//	size_t downSampleArrayLengthY = 0;
	//	WaveformResampler(gYCtrl.waveformBuffer, gYCtrl.waveform_buffer_size / sizeof(USHORT), dac_rate, _maxScannerSampleRate, downsampledArrayY, downSampleArrayLengthY);
	//	delete[] gYCtrl.waveformBuffer;
	//	gYCtrl.waveformBuffer = downsampledArrayY;
	//	gYCtrl.waveform_buffer_size = downSampleArrayLengthY * sizeof(USHORT);
	//	gYCtrl.update_rate = _maxScannerSampleRate;
	//	gYCtrl.enableWaveformFilter = true;

	//	gXCtrl.flyback_samples = flybackSamples;
	//	USHORT* downsampledArrayX;
	//	size_t downSampleArrayLengthX = 0;
	//	WaveformResampler(gXCtrl.waveformBuffer, gXCtrl.waveform_buffer_size / sizeof(USHORT), dac_rate, _maxScannerSampleRate, downsampledArrayX, downSampleArrayLengthX);
	//	delete[] gXCtrl.waveformBuffer;
	//	gXCtrl.waveformBuffer = downsampledArrayX;
	//	gXCtrl.waveform_buffer_size = downSampleArrayLengthX * sizeof(USHORT);
	//	gXCtrl.update_rate = _maxScannerSampleRate;
	//	gXCtrl.enableWaveformFilter = true;
	//}

	//--------------Analog Y galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]] = gYCtrl;

	//--------------Analog X galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_X]] = gXCtrl;


	DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
	d0Ctrl.park_val = 0;
	d0Ctrl.offset_val = 0;
	d0Ctrl.update_rate = dac_rate;
	d0Ctrl.flyback_samples = backward_data_num;
	d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
	d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
	d0Ctrl.filterInhibit = true;
	d0Ctrl.hSync = true;

	waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;

	return TRUE;
}

long CThorDAQGalvoGalvo::BuildPolylineWaveforms(ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms)
{
	LineSegVec lineSegments;
	double originalDeltaX_Volt = 0;

	ImageWaveformBuilder->GetPolylineSegments(lineSegments, originalDeltaX_Volt);
	long ret = TRUE;
	if (0 == lineSegments.size())
		return FALSE;

	//the number of times each line will be repeated
	UINT linesPerLine = (UINT)scanInfo->average_lines_num;
	double deltaX_Volt = originalDeltaX_Volt * pImgAcqPty->pixelX / scanLine->samples_scan;
	double dac_rate = scanInfo->dacRate;
	//UINT samples_line = 2 * (2 * scanLine->samples_idle + scanLine->samples_scan);
	UINT samples_line = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * scanLine->samples_idle + scanLine->samples_scan : 2 * (2 * scanLine->samples_idle + scanLine->samples_scan);

	double linesToScan = TWO_WAY_SCAN == pImgAcqPty->scanMode ? (scanInfo->forward_lines * linesPerLine) / 2 : scanInfo->forward_lines * linesPerLine;
	UINT half_turnaround_padding = (samples_line - scanLine->samples_scan) / 2; //This is a padding added to the beginning of each frame, it is supposed to align SOF with the beginning of the waveformBuffer, instead of SOF being at the beginning of scan.
	UINT sync_secure = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2 + scanLine->samples_idle;
	UINT forward_data_num = (ULONG32)scanInfo->forward_lines * linesPerLine * samples_line;
	UINT sync_securebw = TWO_WAY_SCAN == pImgAcqPty->scanMode ? scanLine->samples_idle : samples_line / 2;
	UINT backward_data_num = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(scanInfo->backward_lines) * samples_line - (sync_securebw + FLYBACK_OFFSET); // flyback = flyback lines * samples per line - (initial padding + 16). The padding is part of the flyback for the waveformBuffer playback	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(fullLine - (sync_secure + FLYBACK_OFFSET));
	backward_data_num = min(MAX_FLYBACK_DAC_SAMPLES, backward_data_num);
	double flybackSteps = scanInfo->backward_lines == 0 ? 0 : (ULONG32)(samples_line * scanInfo->backward_lines - (sync_secure + FLYBACK_OFFSET));

	UINT waveform_dataLength = forward_data_num + backward_data_num;		// total samples for entire waveformBuffer including sweep time and flyback time
	UINT total_dataLength = waveform_dataLength + DAC_FIFO_DEPTH / 2;//Fifo depth: 1024 words (1 word = 32bits)
	/// Calculate the waveformBuffer of Y, Y is the slow axis.  Keeps moving at a constant pace to accomplish a sawtooth xy motion

	double minVal = 0;

	double waveOffsetx = 0;
	double waveOffsety = 0;

	double offset_x = galvoCtrlX->offset + waveOffsetx;
	offset_x = offset_x > 0 ? min(MAX_GALVO_VOLTAGE, offset_x) : max(MIN_GALVO_VOLTAGE, offset_x);

	double offset_y = galvoCtrlY->offset + waveOffsety;
	offset_y = offset_y > 0 ? min(MAX_GALVO_VOLTAGE, offset_y) : max(MIN_GALVO_VOLTAGE, offset_y);

	//forward scan:
	double* wFwdGalvoX = new double[total_dataLength]();
	double* wFwdGalvoY = new double[total_dataLength]();

	double padding = deltaX_Volt * (2.0 * scanLine->samples_idle / PI);
	double paddingStepSize = PI / scanLine->samples_idle / 2.0;

	//----------------Allocate space for digital waveforms--------------------------
	USHORT* pGalvoWaveformDigitalOutputs;

	if (waveforms.find(WAVETABLE_CHANNEL::DIG_D8to15) != waveforms.end())
	{
		pGalvoWaveformDigitalOutputs = waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer;
	}
	else
	{
		pGalvoWaveformDigitalOutputs = new USHORT[total_dataLength]();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = DAC_WAVEFORM_CRTL_STRUCT();
		waveforms[WAVETABLE_CHANNEL::DIG_D8to15].waveformBuffer = pGalvoWaveformDigitalOutputs;
	}

	Cartesian2D startPt = Cartesian2D(max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (+padding * lineSegments[0].GetUnitVector(-1).First() + lineSegments[0].GetStartPoint().First()))),
		max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (+padding * lineSegments[0].GetUnitVector(-1).Second() + lineSegments[0].GetStartPoint().Second()))));

	Cartesian2D curPt = Cartesian2D();

	//calculate the first line waveform, the rest are just replica
	//left-right:
	UINT32 k = 0;
	UINT32 q = half_turnaround_padding;
	for (UINT32 i = 0; i < 1; i++)
	{
		//accelerate:
		for (UINT32 j = 0; j < scanLine->samples_idle; j++)
		{
			wFwdGalvoX[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (+padding * lineSegments[0].GetUnitVector(-1).First() * cos(paddingStepSize * static_cast<double>(j)) + lineSegments[0].GetStartPoint().First())));
			wFwdGalvoY[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (+padding * lineSegments[0].GetUnitVector(-1).Second() * cos(paddingStepSize * static_cast<double>(j)) + lineSegments[0].GetStartPoint().Second())));
			pGalvoWaveformDigitalOutputs[q++] |= THORDAQ_DO_LOW;
			++k;
		}
		//linear:
		unsigned long p = 0;
		for (long j = 0; j < static_cast<long>(lineSegments.size()); j++)
		{
			long lineSegStepCnt = static_cast<long>(ceil(lineSegments[j].GetLineLength() / deltaX_Volt));
			long lineSegStepCntFloor = static_cast<long>(floor(lineSegments[j].GetLineLength() / deltaX_Volt));
			for (long m = 0; m < lineSegStepCntFloor; m++)
			{
				wFwdGalvoX[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[j].GetUnitVector(1).First() * (deltaX_Volt * static_cast<double>(m)) + lineSegments[j].GetStartPoint().First())));
				wFwdGalvoY[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[j].GetUnitVector(1).Second() * (deltaX_Volt * static_cast<double>(m)) + lineSegments[j].GetStartPoint().Second())));
				pGalvoWaveformDigitalOutputs[q++] |= LINE_TRIGGER_OUT_HIGH;
				++k;
				p++;
			}
			if (lineSegStepCntFloor != lineSegStepCnt)
			{
				wFwdGalvoX[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[j].GetEndPoint().First())));
				wFwdGalvoY[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[j].GetEndPoint().Second())));
				pGalvoWaveformDigitalOutputs[q++] |= LINE_TRIGGER_OUT_HIGH;
				++k;
				p++;
			}
		}
		//padding for sampling:
		for (UINT32 j = 0; j < (scanLine->samples_scan - static_cast<long>(p)); j++)
		{
			wFwdGalvoX[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[lineSegments.size() - 1].GetUnitVector(1).First() * (deltaX_Volt * static_cast<double>(j)) + lineSegments[lineSegments.size() - 1].GetEndPoint().First())));
			wFwdGalvoY[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (lineSegments[lineSegments.size() - 1].GetUnitVector(1).Second() * (deltaX_Volt * static_cast<double>(j)) + lineSegments[lineSegments.size() - 1].GetEndPoint().Second())));
			pGalvoWaveformDigitalOutputs[q++] |= LINE_TRIGGER_OUT_HIGH;
			++k;
		}
		curPt = Cartesian2D(wFwdGalvoX[k - 1], wFwdGalvoY[k - 1]);
		//decelerate:
		for (UINT32 j = 0; j < scanLine->samples_idle; j++)
		{
			wFwdGalvoX[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (-padding * lineSegments[lineSegments.size() - 1].GetUnitVector(1).First() * cos(PI / 2.0 + paddingStepSize * static_cast<double>(j)) + curPt.First())));
			wFwdGalvoY[k] = max(MIN_AO_VOLTAGE, min(MAX_AO_VOLTAGE, (-padding * lineSegments[lineSegments.size() - 1].GetUnitVector(1).Second() * cos(PI / 2.0 + paddingStepSize * static_cast<double>(j)) + curPt.Second())));
			pGalvoWaveformDigitalOutputs[q++] |= THORDAQ_DO_LOW;
			++k;
		}
	}
	UINT32 fwdSamples = scanLine->samples_scan + 2 * scanLine->samples_idle;
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrlY->scan_direction) ? 1.0 : -1.0;
	///flipscan:
	if (1.0 != yDirection)
	{
		for (UINT32 i = 0; i < fwdSamples; i++)
		{
			double tempY = wFwdGalvoY[i];
			wFwdGalvoY[i] = tempY * yDirection;
		}
	}

	///If the scan area angle is different from 0 do a matrix rotation to the left-right waveform
	///before the right-left or offset is added
	if (0 != pImgAcqPty->scanAreaAngle)
	{
		double angle = (1 == pImgAcqPty->verticalScanDirection) ? pImgAcqPty->scanAreaAngle : -pImgAcqPty->scanAreaAngle;
		for (UINT32 i = 0; i < fwdSamples; i++)
		{
			double tempX = wFwdGalvoX[i];
			double tempY = wFwdGalvoY[i];
			wFwdGalvoX[i] = tempX * cos(angle) - tempY * sin(angle);
			wFwdGalvoY[i] = tempX * sin(angle) + tempY * cos(angle);
		}
	}

	//always one way scan since retrace may not follow trace
	for (UINT32 i = 0; i < fwdSamples; i++)
	{
		wFwdGalvoX[k] = wFwdGalvoX[fwdSamples - 1 - i];
		wFwdGalvoY[k] = wFwdGalvoY[fwdSamples - 1 - i];
		++k;
	}

	if (TWO_WAY_SCAN == pImgAcqPty->scanMode)
	{
		for (UINT i = 1; i < ((UINT)((scanInfo->forward_lines * scanInfo->average_lines_num) / 2)); ++i)
		{
			memcpy((wFwdGalvoX + (size_t)samples_line * i * 2), (wFwdGalvoX), 2 * samples_line * sizeof(double));
			memcpy((wFwdGalvoY + (size_t)samples_line * i * 2), (wFwdGalvoY), 2 * samples_line * sizeof(double));
			memcpy((pGalvoWaveformDigitalOutputs + half_turnaround_padding + (size_t)samples_line * i * 2), (pGalvoWaveformDigitalOutputs + half_turnaround_padding), 2 * samples_line * sizeof(USHORT));
		}
	}
	else
	{
		for (UINT i = 1; i < ((UINT)(scanInfo->forward_lines * linesPerLine)); ++i)
		{
			memcpy((wFwdGalvoX + (size_t)samples_line * i), (wFwdGalvoX), samples_line * sizeof(double));
			memcpy((wFwdGalvoY + (size_t)samples_line * i), (wFwdGalvoY), samples_line * sizeof(double));
			memcpy((pGalvoWaveformDigitalOutputs + half_turnaround_padding + (size_t)samples_line * i), (pGalvoWaveformDigitalOutputs + half_turnaround_padding), samples_line * sizeof(USHORT));
		}
	}

	q = half_turnaround_padding;
	for (unsigned long i = 0; i < (UINT)linesToScan; ++i)
	{
		for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
		{
			// imaging hasn't started at this point yet when i == 0
			if (0 != i)
			{
				pGalvoWaveformDigitalOutputs[q++] |= FRAME_TRIGGER_OUT_HIGH;
			}
			else
			{
				pGalvoWaveformDigitalOutputs[q++] |= THORDAQ_DO_LOW;
			}
		}
		for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
		{
			pGalvoWaveformDigitalOutputs[q++] |= FRAME_TRIGGER_OUT_HIGH;
		}
		//the imaging portion is done here for one way mode
		if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
		{
			for (unsigned long j = 0; j < scanLine->samples_idle * 2; ++j)
			{
				pGalvoWaveformDigitalOutputs[q++] |= FRAME_TRIGGER_OUT_HIGH;
			}
		}
		//the imaging portion is done here for one way mode
		if (i != linesToScan - 1 || TWO_WAY_SCAN == pImgAcqPty->scanMode)
		{
			for (unsigned long j = 0; j < scanLine->samples_scan; ++j)
			{
				pGalvoWaveformDigitalOutputs[q++] |= FRAME_TRIGGER_OUT_HIGH;
			}
		}
		//the imaging portion is done here
		if (i != linesToScan - 1)
		{
			for (unsigned long j = 0; j < scanLine->samples_idle; ++j)
			{
				pGalvoWaveformDigitalOutputs[q++] |= FRAME_TRIGGER_OUT_HIGH;
			}
		}
	}

	//----------------Build X Waveform----------------
	DAC_WAVEFORM_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gXCtrl.park_val = galvoCtrlY->park;
	gXCtrl.offset_val = offset_x;
	gXCtrl.update_rate = dac_rate;
	gXCtrl.flyback_samples = backward_data_num;
	gXCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_X];
	gXCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gXCtrl.filterInhibit = false;
	gXCtrl.hSync = true;

	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_X]) != waveforms.end())
	{
		gXCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_X]].waveformBuffer;
	}
	else
	{
		gXCtrl.waveformBuffer = new USHORT[total_dataLength]();
	}

	USHORT* pGalvoWaveformX = gXCtrl.waveformBuffer;
	memset(pGalvoWaveformX, static_cast<USHORT>(minVal), total_dataLength * sizeof(USHORT));

	DAC_WAVEFORM_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gYCtrl.park_val = galvoCtrlY->park;
	gYCtrl.offset_val = offset_y;
	gYCtrl.update_rate = dac_rate;
	gYCtrl.flyback_samples = backward_data_num;
	gYCtrl.output_port = _imagingActiveAOSelection[GG_AO::GG_Y];
	gYCtrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	gYCtrl.filterInhibit = false;
	gYCtrl.hSync = true;
	if (waveforms.find(_imagingActiveAOSelection[GG_AO::GG_Y]) != waveforms.end())
	{
		gYCtrl.waveformBuffer = waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]].waveformBuffer;
	}
	else
	{
		gYCtrl.waveformBuffer = new USHORT[total_dataLength]();
	}

	USHORT* pGalvoWaveformY = gYCtrl.waveformBuffer;
	double tempX0 = 0, tempY0 = 0, tempX1 = 0, tempY1 = 0;
	USHORT offsetX = 0;
	USHORT offsetY = 0;

	ULONG32 sample_index = half_turnaround_padding;

	USHORT midX = wFwdGalvoX[0] > 0 ? 0x7fff : 0x8000;
	offsetX = static_cast<USHORT>(round(wFwdGalvoX[0] * VOLT_TO_THORDAQ_VAL + midX));

	USHORT midY = wFwdGalvoY[0] > 0 ? 0x7fff : 0x8000;
	offsetY = static_cast<USHORT>(round(wFwdGalvoY[0] * VOLT_TO_THORDAQ_VAL + midY));

	for (UINT32 i = 0; i < sample_index; ++i)
	{
		pGalvoWaveformX[i] = 0;

		USHORT midY = wFwdGalvoY[0] > 0 ? 0x7fff : 0x8000;
		pGalvoWaveformY[i] = 0;
	}

	k = sample_index;
	for (unsigned long i = 0; i < static_cast<unsigned long>(scanInfo->forward_lines * linesPerLine * samples_line); ++i)
	{
		USHORT midX = static_cast<USHORT>(wFwdGalvoX[i] > 0.0 ? 0x7fff : 0x8000);
		pGalvoWaveformX[k] = static_cast<USHORT>(round(wFwdGalvoX[i] * VOLT_TO_THORDAQ_VAL + midX)) - offsetX;

		USHORT midY = static_cast<USHORT>(wFwdGalvoY[i] > 0.0 ? 0x7fff : 0x8000);
		pGalvoWaveformY[k] = static_cast<USHORT>(round(wFwdGalvoY[i] * VOLT_TO_THORDAQ_VAL + midY)) - offsetY;
		++k;
	}

	//update the offset with a transformed offset for the angled X and Y waveforms
	offset_x = gXCtrl.offset_val = offsetX * GALVO_RESOLUTION + MIN_GALVO_VOLTAGE + galvoCtrlX->offset;
	offset_y = gYCtrl.offset_val = offsetY * GALVO_RESOLUTION + MIN_GALVO_VOLTAGE + galvoCtrlY->offset;

	//release mem:
	SAFE_DELETE_ARRAY(wFwdGalvoX);
	SAFE_DELETE_ARRAY(wFwdGalvoY);


	//--------------Analog Y galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_Y]] = gYCtrl;

	//--------------Analog X galvo waveformBuffer--------------
	waveforms[_imagingActiveAOSelection[GG_AO::GG_X]] = gXCtrl;

	DAC_WAVEFORM_CRTL_STRUCT d0Ctrl = DAC_WAVEFORM_CRTL_STRUCT();
	d0Ctrl.park_val = 0;
	d0Ctrl.offset_val = 0;
	d0Ctrl.update_rate = dac_rate;
	d0Ctrl.flyback_samples = backward_data_num;
	d0Ctrl.output_port = WAVETABLE_CHANNEL::DIG_D8to15;
	d0Ctrl.waveform_buffer_size = (total_dataLength) * sizeof(USHORT);
	d0Ctrl.waveformBuffer = pGalvoWaveformDigitalOutputs;
	d0Ctrl.filterInhibit = true;
	d0Ctrl.hSync = true;

	waveforms[WAVETABLE_CHANNEL::DIG_D8to15] = d0Ctrl;

	return ret;
}

long CThorDAQGalvoGalvo::AlignDataLoadFile()
{
	long i;
	char appPath[256]; ///<the char of application path

	char filePath[256]; ///<char of the whole file path
	auto returnval = _getcwd(appPath, 256);
	strcpy_s(filePath, appPath);
	strcat_s(filePath, "\\AlignDataGalvo.txt");
	FILE* AlignDataFile;
	if (fopen_s(&AlignDataFile, filePath, "r") != 0)
	{
		for (i = 0; i < 256; i++)
			_shiftArray[i] = 0;
		return TRUE;
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			if (fscanf_s(AlignDataFile, "%d", &_shiftArray[i]) == EOF)
			{
				for (i; i < 256; i++)
					_shiftArray[i] = 0;
			}
			else if (_shiftArray[i] < MIN_ALIGNMENT)
				_shiftArray[i] = MIN_ALIGNMENT;
			else if (_shiftArray[i] > MAX_ALIGNMENT)
				_shiftArray[i] = MAX_ALIGNMENT;
		}
		fclose(AlignDataFile);
		return TRUE;
	}
}

//BGB / JB version 3
LONG CThorDAQGalvoGalvo::GetDACSamplesPerLine(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double pixel_dwell_time, double& line_time, bool onewayLineScan, bool& useFastOneWayMode)
{
	//To find proper register for dwell time and pixel delay time
	//line time = pixel_density * pixel_dwelltime + (pixel_density - 1) * pixel_delaytime
	//time sequence likes below:
	//            __________   __________         ___________   __________              __________   _______
	//           |       |  | |       |  |       |        |  | |       |  |            |       |  | |       |
	// ----------            --           -------             -            ------------            -         ------------
	//-----------dwell delay  dwell delay.........dwell delay  dwell delay.............dwell delay  dwell------------
	useFastOneWayMode = false;
	double theta = (double)PImgAcqPty->fieldSize * _field2Theta;
	double amp_x = theta * _theta2Volts * PImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	//hard code pixel delay time to be the minimal
	double pixel_delay_time = 1;
	//find proper register for turnaround time and DAC update rate to make integer samples of total line times.
	bool isFound = false;
	int samples_line_time = 0;
	int singleLineTime = 0;

	//calculate the active scan time in a line
	double activeScanTimePerLine = pixel_dwell_time * PImgAcqPty->pixelX;

	const double CLOCK_PRECISION_200MHZ = 5.0; //5ns

	int totalYLines = PImgAcqPty->pixelY;
	if (PImgAcqPty->lineAveragingEnable == TRUE)
	{
		totalYLines *= PImgAcqPty->lineAveragingNumber;
	}

	//calculate the dwelltime used for sampling calculations
	long dwelltime = static_cast<long>(round((activeScanTimePerLine * (double)SYS_CLOCK_FREQ - ((double)pixel_delay_time * 16.0 + 1.0) * ((double)PImgAcqPty->pixelX - 1.0)) / (double)PImgAcqPty->pixelX));

	//int pre_samples_of_sweep = PImgAcqPty->pixelX * dwelltime + static_cast<int>(pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1);

	const int US_PER_SECOND = 1000000;

	int turnaround = static_cast<ULONG32>(round(2 * (((double)PImgAcqPty->turnAroundTimeUS / 2.0 / US_PER_SECOND * (double)SYS_CLOCK_FREQ / 2.0 - 1.0) / 16.0)));//static_cast<int>(ceil(((((double)PImgAcqPty->turnAroundTimeUS / 2.0) * (updateRate + 1.0)) - 1.0) / 16.0)); //Convert from microseconds to clock counts (200MHz)

	double preUpdateRate = (pixel_dwell_time * US_PER_SECOND > 1.0) ? 1.0 / (pixel_dwell_time) : DAC_MAX_UPDATERATE;

	int updateRate = (int)round((double)SYS_CLOCK_FREQ / preUpdateRate - 1);
	int pre_samples_of_sweep = PImgAcqPty->pixelX * dwelltime + static_cast<int>(pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1);

	//Intra_Line_Time = 2 * [Intra_Line_Delay * 16 + 1] / facq_clk
//	(Intra_Line_Time * facq_clk/2 - 1) / 16 = Intra_Line_Delay_count
	for (; updateRate < 65536; updateRate++) // The range of update rate is 3k - 1M
	{
		turnaround = static_cast<ULONG32>(round(2 * (((double)PImgAcqPty->turnAroundTimeUS / 2.0 / US_PER_SECOND * (double)SYS_CLOCK_FREQ / 2.0 - 1.0) / 16.0)));
		for (; turnaround <= 6250; turnaround++)//max is 500us for half turaround time
		{
			double sample = (double)((double)PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16 + 1) * ((double)PImgAcqPty->pixelX - 1) + 2 * ((double)turnaround * 16 + 1)) / (updateRate + 1.0);
			if ((sample - floor(sample)) < 0.2)
			{
				samples_line_time = static_cast<long>(((double)PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16 + 1) * ((double)PImgAcqPty->pixelX - 1) + 2 * ((double)turnaround * 16 + 1)) / (updateRate + 1.0));
				isFound = true;
				break;
			}
		}
		if (isFound)
		{
			break;
		}
	}
	if (!isFound)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvo could not find a matching turnAround or dac rate for the current dwell time / pixel density.");
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}

	dac_rate = (double)SYS_CLOCK_FREQ / (updateRate + 1.0);

	scanLine->samples_scan = (ULONG32)(((double)PImgAcqPty->pixelX * (double)dwelltime + (pixel_delay_time * 16 + 1) * ((double)PImgAcqPty->pixelX - 1.0)) / ((double)updateRate + 1.0));

	if (PImgAcqPty->scanMode != TWO_WAY_SCAN && !onewayLineScan && TRUE == PImgAcqPty->fastOneWayEnable && LSMAreaMode::POLYLINE != PImgAcqPty->areaMode) //one way scan
	{
		double theta = (double)PImgAcqPty->fieldSize * _field2Theta;
		double amplitude = theta * _theta2Volts * PImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
		double turnaroundTime = ((double)(turnaround * 16.0 + 1.0)) / (double)SYS_CLOCK_FREQ;//200us turnaround time
		double galvoRetraceTime = _imgAcqPty.turnAroundTimeUS / 2;

		double paddedAmplitude = (double)PImgAcqPty->fieldSize * galvoRetraceTime * 4 / (PImgAcqPty->pixelX * PImgAcqPty->dwellTime * 2 * M_PI);
		double fieldX_angle = ((double)PImgAcqPty->fieldSize + paddedAmplitude * 2) * _field2Theta;
		//TODO decide if we should use fieldX_angle or amp


		double pad_amp = static_cast<long> (turnaroundTime * dac_rate);// volt changes of turnaround time
		double galvoXFwdStep = amp_x / (double)(scanLine->samples_scan);
		double amp = amp_x + 2 * galvoXFwdStep * pad_amp / M_PI;

		if (TRUE == _limitGalvoSpeed)
		{
			amp = fieldX_angle;
		}
		//double minimal_backwards_sample = ceil(M_PI / asin(min(1.0,0.05 / (2.0 * pad_amp + amp_x))));
		//double minimal_backwards_sample = samples_line_time;
		double minimal_backwards_sample = ceill(amp / 2.5 * M_PI / GALVO_RESOLUTION_MAX * dac_rate);
		long turn_around_time_settings = static_cast<long>(((minimal_backwards_sample * ((double)updateRate + 1.0) / 2.0 - 1.0) / 16.0));
		while (turn_around_time_settings < UINT32_MAX)
		{
			minimal_backwards_sample = 2.0 * (turn_around_time_settings * 16.0 + 1.0) / ((double)updateRate + 1.0);
			//turn_around_time_settings = ((++minimal_backwards_sample * ((double)updateRate + 1.0) / 2.0 - 1.0) / 16.0);
			if (minimal_backwards_sample - floor(minimal_backwards_sample) < 0.1) break;
			turn_around_time_settings++;
		}

		double templinetime1 = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));

		int tempTurnaround = turnaround + turn_around_time_settings;

		double templinetime2 = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)tempTurnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));

		double frametime1 = totalYLines * 2.0 * templinetime1;
		double frametime2 = totalYLines * templinetime2;
		if (frametime2 < frametime1)
		{
			scanLine->samples_back = (UINT32)minimal_backwards_sample;
			turnaround = turnaround + turn_around_time_settings;
			samples_line_time = (int)minimal_backwards_sample + samples_line_time;
			useFastOneWayMode = true;
		}
	}
	// Step3: Setup the DAC register 
	// Setup the update rate for horizontal and vertical galvo

	// round up to nearest even number
	if ((samples_line_time - scanLine->samples_back - scanLine->samples_scan) % 2 != 0)
	{
		scanLine->samples_scan++;
	}

	scanLine->samples_idle = static_cast<ULONG32>((samples_line_time - scanLine->samples_back - scanLine->samples_scan) / 2);// samples of turnaround time

	_daqAcqCfg.galvoGalvoCtrl.dwellTime = (double)dwelltime / (double)SYS_CLOCK_FREQ;
	_daqAcqCfg.galvoGalvoCtrl.pixelDelayCnt = pixel_delay_time;
	_daqAcqCfg.galvoGalvoCtrl.lineTime = line_time = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));
	_daqAcqCfg.galvoGalvoCtrl.pureTurnAroundTime = scanLine->samples_idle / dac_rate;
	if (PImgAcqPty->scanMode == TWO_WAY_SCAN || useFastOneWayMode)
	{
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}
	else
	{
		// For one way, the turn around is acceleration + deceleration + 1 line time (for the retrace)
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = line_time + 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}

	return TRUE;
}

//3P version, it changes the dwell time to include the intra pixel delay. This way it can be on the right frequency of the laser without adding extra pixels
LONG CThorDAQGalvoGalvo::GetDACSamplesPerLine3P(ScanLineStruct* scanLine, ImgAcqPty* PImgAcqPty, double& dac_rate, double dwell_time, double& line_time, bool onewayLineScan, bool& useFastOneWayMode)
{
	//To find proper register for dwell time and pixel delay time
	//line time = pixel_density * pixel_dwelltime + (pixel_density - 1) * pixel_delaytime
	//time sequence likes below:
	//            __________   __________         ___________   __________              __________   _______
	//           |       |  | |       |  |       |        |  | |       |  |            |       |  | |       |
	// ----------            --           -------             -            ------------            -         ------------
	//-----------dwell delay  dwell delay.........dwell delay  dwell delay.............dwell delay  dwell------------
	useFastOneWayMode = false;

	double theta = (double)PImgAcqPty->fieldSize * _field2Theta;
	double amp_x = theta * _theta2Volts * PImgAcqPty->fineFieldSizeScaleX; // horizontal galvo amplitude
	//hard code pixel delay time to be the minimal
	double pixel_delay_time = 1;
	//find proper register for turnaround time and DAC update rate to make integer samples of total line times.
	bool isFound = false;
	int samples_line_time = 0;
	int singleLineTime = 0;

	//Convert from seconds to 200MHz counts, 5ns
	double singlePulseCounts = floor(_minDwellTime / 0.005);
	double tempDwellns = ceil((dwell_time / 0.000000005)); //- 0.5);
	//substract the intra pixel delay time from the dwell time
	long dwelltime = static_cast<long>((tempDwellns)-(pixel_delay_time * 16.0 + 1.0));

	int totalYLines = PImgAcqPty->pixelY;
	if (PImgAcqPty->lineAveragingEnable == TRUE)
	{
		totalYLines *= PImgAcqPty->lineAveragingNumber;
	}

	int pre_samples_of_sweep = PImgAcqPty->pixelX * dwelltime + static_cast<int>(pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1);

	int updateRate = static_cast<ULONG32>(SYS_CLOCK_FREQ / DAC_MAX_UPDATERATE - 1); // DAC_SAMPLE_RATE = SYS_CLK / (MAX_REGISTER_UPDATERATE + 1)
	int turnaround = static_cast<int>(ceil(((((double)PImgAcqPty->turnAroundTimeUS / 2.0) * (updateRate + 1.0)) - 1.0) / 16.0)); //Convert from microseconds to clock counts (200MHz)

	double turnbackTime = 0.0;
	if (PImgAcqPty->scanMode != TWO_WAY_SCAN && !onewayLineScan && TRUE == PImgAcqPty->fastOneWayEnable && FALSE == PImgAcqPty->acquireDuringTurnAround) // one-way scan    turnaround time = turnaround time + turnback time
	{
		double turnaroundTime = PImgAcqPty->turnAroundTimeUS / US_TO_S;
		double sweep_time = pre_samples_of_sweep / (double)SYS_CLOCK_FREQ;
		double pad_amp = (amp_x / sweep_time * (double)turnaroundTime / 2.0 / M_PI); // volt changes of turnaround time
		double amp = 2.0 * pad_amp + amp_x; // max volt when galvo mirror moves forward
		//formula:    v = amp / 2 * cos( pi / T * t) + amp / 2;
		// max speed = amp / 2 * pi / T  = GALVO_RESOLUTION_MAX
		// T = amp * pi / GALVO_RESOLUTION_MAX / 2
		turnbackTime = amp / 2.0 * M_PI / GALVO_RESOLUTION_MAX;
		turnaroundTime = turnaroundTime + turnbackTime;
	}

	for (; updateRate < 65536; updateRate++) // The range of update rate is 3k - 1M
	{
		if (FALSE == PImgAcqPty->acquireDuringTurnAround)
		{
			turnaround = static_cast<int>(ceil(((((double)PImgAcqPty->turnAroundTimeUS / 2.0) * (updateRate + 1.0)) - 1.0) / 16.0)); //Convert from microseconds to clock counts (200MHz)
		}
		else
		{
			double oneLaserPulseTime = 2.0 * US_TO_S / _daqAcqCfg.imageCtrl.clockRate;
			turnaround = static_cast<int>(ceil(((oneLaserPulseTime * (updateRate + 1.0)) - 1.0) / 16.0));
		}
		for (; turnaround <= 6250; turnaround++)//max is 500us for half turaround time
		{
			double sample = (double)(PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16.0 + 1.0) * (PImgAcqPty->pixelX - 1) + 2.0 * ((double)turnaround * 16.0 + 1.0)) / ((double)updateRate + 1.0);
			if ((sample - floor(sample)) < 0.2)
			{
				samples_line_time = static_cast<long>((PImgAcqPty->pixelX * dwelltime + (pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1) + 2 * ((double)turnaround * 16 + 1.0)) / ((double)updateRate + 1.0));
				isFound = true;
				wchar_t errMsg[MSG_SIZE];
				StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvo line time found, Samples LineTime: %d turnaround: %d", samples_line_time, turnaround);
				CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, VERBOSE_EVENT);
				break;
			}
		}
		if (isFound)
		{
			break;
		}
	}

	if (!isFound)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorDAQGalvoGalvo could not find a matching dwell time, turnAround or dac rate for this setup.");
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}

	dac_rate = (double)SYS_CLOCK_FREQ / (updateRate + 1.0);


	if (PImgAcqPty->scanMode != TWO_WAY_SCAN && !onewayLineScan && TRUE == PImgAcqPty->fastOneWayEnable && FALSE == PImgAcqPty->acquireDuringTurnAround && LSMAreaMode::POLYLINE != PImgAcqPty->areaMode) //one way scan
	{
		double turnaroundTime = ((double)(turnaround * 16.0 + 1.0)) / (double)SYS_CLOCK_FREQ;//200us turnaround time
		//double pad_amp = (amp_x / scanLine->samples_scan * (double)turnaroundTime / M_PI); // volt changes of turnaround time
		double pad_amp = static_cast<long> (turnaroundTime * dac_rate);// volt changes of turnaround time
		double galvoXFwdStep = amp_x / (double)(scanLine->samples_scan);
		double amp = amp_x + 2 * galvoXFwdStep * pad_amp / M_PI;

		double galvoRetraceTime = _imgAcqPty.turnAroundTimeUS / 2;
		double paddedAmplitude = (double)PImgAcqPty->fieldSize * galvoRetraceTime * 4 / (PImgAcqPty->pixelX * PImgAcqPty->dwellTime * 2 * M_PI);
		double fieldX_angle = ((double)PImgAcqPty->fieldSize + paddedAmplitude * 2) * _field2Theta;
		if (TRUE == _limitGalvoSpeed)
		{
			amp = fieldX_angle;
		}
		//double minimal_backwards_sample = ceil(M_PI / asin(min(1.0,0.05 / (2.0 * pad_amp + amp_x))));
		//double minimal_backwards_sample = samples_line_time;
		double minimal_backwards_sample = ceill(amp / 2.0 * M_PI / GALVO_RESOLUTION_MAX * dac_rate);
		long turn_around_time_settings = static_cast<long>(((minimal_backwards_sample * ((double)updateRate + 1.0) / 2.0 - 1.0) / 16.0));
		while (turn_around_time_settings < UINT32_MAX)
		{
			minimal_backwards_sample = 2.0 * (turn_around_time_settings * 16.0 + 1.0) / ((double)updateRate + 1.0);
			//turn_around_time_settings = ((++minimal_backwards_sample * ((double)updateRate + 1.0) / 2.0 - 1.0) / 16.0);
			if (minimal_backwards_sample - floor(minimal_backwards_sample) < 0.1) break;
			turn_around_time_settings++;
		}
		double templinetime1 = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));

		int tempTurnaround = turnaround + turn_around_time_settings;

		double templinetime2 = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)tempTurnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));

		double frametime1 = totalYLines * 2.0 * templinetime1;
		double frametime2 = totalYLines * templinetime2;
		if (frametime2 < frametime1)
		{
			scanLine->samples_back = (UINT32)minimal_backwards_sample;
			turnaround = turnaround + turn_around_time_settings;
			samples_line_time = (int)minimal_backwards_sample + samples_line_time;
			useFastOneWayMode = true;
		}
	}

	if (TRUE == PImgAcqPty->acquireDuringTurnAround)
	{
		double turnaroundTime = PImgAcqPty->turnAroundTimeUS / US_TO_S;
		int dacSamplesTurnaround = static_cast<int>(round(dac_rate * turnaroundTime));
		if (dacSamplesTurnaround > samples_line_time)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, _MAX_PATH, L"ThorDAQ 3P Scan timing error scanSamples = %d turnaroundSamples = %d ", samples_line_time, dacSamplesTurnaround);
			MessageBox(NULL, errMsg, L"Not enough time to image during turn around at current pixel X. Try increasing number number of horizontal pixels and try again", MB_OK | MB_SETFOREGROUND | MB_ICONWARNING);

			return FALSE;
		}

		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"intralineDelay %d", turnaround);
		CThorDAQGalvoGalvo::GetInstance()->LogMessage(errMsg, ERROR_EVENT);
		scanLine->samples_back = 0;
		scanLine->samples_scan = samples_line_time - dacSamplesTurnaround;
		scanLine->samples_idle = dacSamplesTurnaround / 2;
	}
	else
	{
		// Step3: Setup the DAC register 
		// Setup the update rate for horizontal and vertical galvo
		scanLine->samples_scan = static_cast<ULONG32>((((double)PImgAcqPty->pixelX * (double)dwelltime + (pixel_delay_time * 16.0 + 1.0) * ((double)PImgAcqPty->pixelX - 1.0)) / ((double)updateRate + 1.0)));
		scanLine->samples_back = static_cast<long>(ceil(turnbackTime * dac_rate - 0.5));
		// round up to nearest even number
		if ((samples_line_time - scanLine->samples_back - scanLine->samples_scan) % 2 != 0)
		{
			scanLine->samples_scan++;
		}
		scanLine->samples_idle = static_cast<ULONG32>((samples_line_time - scanLine->samples_back - scanLine->samples_scan) / 2);// samples of turnaround time
	}

	_daqAcqCfg.galvoGalvoCtrl.dwellTime = (double)dwelltime / (double)SYS_CLOCK_FREQ;
	_daqAcqCfg.galvoGalvoCtrl.pixelDelayCnt = pixel_delay_time;
	_daqAcqCfg.galvoGalvoCtrl.lineTime = line_time = (double)((PImgAcqPty->pixelX * (double)dwelltime / (double)SYS_CLOCK_FREQ) + ((pixel_delay_time * 16 + 1) * (PImgAcqPty->pixelX - 1.0) / (double)SYS_CLOCK_FREQ) + (2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ));
	_daqAcqCfg.galvoGalvoCtrl.pureTurnAroundTime = scanLine->samples_idle / dac_rate;
	if (PImgAcqPty->scanMode == TWO_WAY_SCAN || useFastOneWayMode)
	{
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}
	else
	{
		// For one way, the turn around is acceleration + deceleration + 1 line time (for the retrace)
		_daqAcqCfg.galvoGalvoCtrl.turnaroundTime = line_time + 2.0 * ((double)turnaround * 16.0 + 1.0) / (double)SYS_CLOCK_FREQ;
	}
	return TRUE;
}
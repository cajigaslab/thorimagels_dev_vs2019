#include "stdafx.h"
#include "thordaqResonantGalvo.h"

LONG CThordaqResonantGalvo::BuildmROIWaveforms(vector<StripInfo*> mROIStrips, ScanStruct* scanInfo, ScanLineStruct* scanLine, GalvoStruct* galvoCtrlX, GalvoStruct* galvoCtrlY, ImgAcqPty* pImgAcqPty, std::map<UINT, DAC_WAVEFORM_CRTL_STRUCT>& waveforms)
{
	double yDirection = (SCAN_DIRECTION::FORWARD_SC == galvoCtrlY->scan_direction) ? 1.0 : -1.0;

	double F2VGx = GetField2VoltsX(pImgAcqPty->maxSizeInUMForMaxFieldSize);

	double F2VGy = GetField2VoltsY(pImgAcqPty->maxSizeInUMForMaxFieldSize);

	int imagingLines = 0;
	for (int s = 0; s < mROIStrips.size(); ++s)
	{
		imagingLines += (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (mROIStrips[s]->YSize / 2) : mROIStrips[s]->YSize;
		if (!mROIStrips[s]->IsEnd)
		{
			imagingLines += (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[s]->flyToNextStripeSkipLines / 2 : mROIStrips[s]->flyToNextStripeSkipLines;
		}
	}

	int areaCount = (int)mROIStrips.size();

	double backwardLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[areaCount - 1]->flyToNextStripeSkipLines / 2 : mROIStrips[areaCount - 1]->flyToNextStripeSkipLines;

	double dacRate = scanInfo->dac_rate;
	INT samplesPerLine = scanLine->samples_scan;
	UINT calibrationSamples = samplesPerLine * 2 * (UINT)pImgAcqPty->preImagingCalibrationCycles;
	UINT postcalibrationSamples = samplesPerLine * 2 * (UINT)pImgAcqPty->postImagingCalibrationCycles;
	INT totalBackwardDataNumIntra = 2 * samplesPerLine * ((UINT)backwardLines) + postcalibrationSamples;
	INT totalBackwardDataNum = 2 * samplesPerLine * ((UINT)backwardLines - 2) + postcalibrationSamples; // we do 2 lines less to allow enough time to get to offset before intra delay is done
	INT totalForwardDataNum = 2 * samplesPerLine * (UINT)(imagingLines);
	INT totalSamples = calibrationSamples + totalForwardDataNum + totalBackwardDataNum + DAC_FIFO_DEPTH;

	short* pGalvoYWaveform = new short[totalSamples]();
	short* pGalvoXWaveform = new short[totalSamples]();

	//X
	size_t wOffsetX = 0;
	pGalvoXWaveform[wOffsetX] = 0;
	for (int s = 0; s < mROIStrips.size(); ++s)
	{
		double startVolt = mROIStrips[s]->XPos * F2VGx;
		int frameLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (mROIStrips[s]->YSize / 2) : mROIStrips[s]->YSize;
		int staticSamples = 2 * frameLines * samplesPerLine;

		for (int s = 0; s < staticSamples; ++s)
		{
			if (wOffsetX > 0)
			{
				pGalvoXWaveform[wOffsetX] = pGalvoXWaveform[wOffsetX - 1];
			}
			++wOffsetX;
		}

		if (!mROIStrips[s]->IsEnd && mROIStrips.size() > s + 1)
		{
			double startNextVolt = mROIStrips[s + 1]->XPos * F2VGx;

			double amplitude = startNextVolt - startVolt;
			int skipLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[s]->flyToNextStripeSkipLines / 2 : mROIStrips[s]->flyToNextStripeSkipLines;
			int flySamples = 2 * samplesPerLine * (skipLines - 2);
			double galvoXStep = (amplitude / (double)(flySamples) / GALVO_RESOLUTION);
			double amplitudeCounts = galvoXStep * (double)flySamples;
			short startingPoint = pGalvoXWaveform[wOffsetX - 1];
			if (_scannerType == ScannerType::MEMS)
			{
				for (int j = 0; j < flySamples; ++j)
				{
					pGalvoXWaveform[wOffsetX] = static_cast<short>(min(SHRT_MAX, startNextVolt / GALVO_RESOLUTION));
					++wOffsetX;
				}
			}
			else
			{
				for (int j = 0; j < flySamples; ++j)
				{
					double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flySamples));
					pGalvoXWaveform[wOffsetX] = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;
					++wOffsetX;
				}
			}

			//because we don't do all the skiplines, we actually leave 2 //TODO: better comments this
			for (int j = 0; j < 2 * samplesPerLine * 2; ++j)
			{
				pGalvoXWaveform[wOffsetX] = pGalvoXWaveform[wOffsetX - 1];
				++wOffsetX;
			}
		}
		else if (mROIStrips[s]->IsEnd)
		{
			double startVolt0 = mROIStrips[s]->XPos * F2VGx;
			double endVolt = mROIStrips[0]->XPos * F2VGx;
			double amplitude = endVolt - startVolt;
			int flySamples = totalBackwardDataNum;
			double galvoXStep = (amplitude / (double)(flySamples) / GALVO_RESOLUTION);
			double amplitudeCounts = galvoXStep * (double)flySamples;
			short startingPoint = pGalvoXWaveform[wOffsetX - 1];
			if (_scannerType == ScannerType::MEMS)
			{
				for (int j = 0; j < flySamples; ++j)
				{
					pGalvoXWaveform[wOffsetX] = static_cast<short>(min(SHRT_MAX, endVolt / GALVO_RESOLUTION));
					++wOffsetX;
				}
			}
			else
			{
				for (int j = 0; j < flySamples; ++j)
				{
					double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flySamples));
					pGalvoXWaveform[wOffsetX] = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;
					++wOffsetX;
				}
			}
			//flyback to start of first frame
		}
	}

	for (size_t i = wOffsetX; i < totalSamples; ++i)
	{
		pGalvoXWaveform[wOffsetX] = 0;
		++wOffsetX;
	}


	double offset_x = mROIStrips[0]->XPos * F2VGx;
	offset_x = offset_x > 0 ? min(MAX_GALVO_VOLTAGE, offset_x) : max(MIN_GALVO_VOLTAGE, offset_x);

	double offset_y = yDirection > 0 ? mROIStrips[0]->YPos * F2VGy : (mROIStrips[0]->YPos + mROIStrips[0]->YPhysicalSize) * F2VGy;
	offset_y = offset_y > 0 ? min(MAX_GALVO_VOLTAGE, offset_y) : max(MIN_GALVO_VOLTAGE, offset_y);


	//Y
	size_t wOffsetY = 0;
	pGalvoYWaveform[wOffsetY] = 0;
	for (size_t s = 0; s < mROIStrips.size(); ++s)
	{
		double startVolt = mROIStrips[s]->YPos * F2VGy;
		double endVolt = (mROIStrips[s]->YPos + mROIStrips[s]->YPhysicalSize) * F2VGy;
		if (yDirection < 0)
		{
			endVolt = mROIStrips[s]->YPos * F2VGy;
			startVolt = (mROIStrips[s]->YPos + mROIStrips[s]->YPhysicalSize) * F2VGy;
		}
		int frameLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (mROIStrips[s]->YSize / 2) : mROIStrips[s]->YSize;
		int scanSamples = 2 * frameLines * samplesPerLine;
		double amplitude = endVolt - startVolt;
		double galvoYStep = (amplitude / (double)(scanSamples) / GALVO_RESOLUTION);
		short startingPoint = wOffsetY > 0 ? pGalvoYWaveform[wOffsetY - 1] : 0;

		// For 'invert vertical scan' we multiply the waveformBuffer value by -1.0 same as the NI/Alazar counterpart. The negative value is cast to
		//unsigned short. This will convert it to UMAX SHORT - waveformBuffer value, which works for the vertical invert. This is why we don't check if
		//the waveformBuffer value is below 0
		for (ULONG32 j = 0; j < (ULONG32)scanSamples; j++)
		{
			pGalvoYWaveform[wOffsetY] = static_cast<short>(min(USHRT_MAX, (galvoYStep * (double)(j + 1)))) + startingPoint;
			++wOffsetY;
		}

		if (!mROIStrips[s]->IsEnd && mROIStrips.size() > s + 1)
		{
			double startNextVolt = mROIStrips[s + 1]->YPos * F2VGy;
			if (yDirection < 0)
			{
				startNextVolt = (mROIStrips[s + 1]->YPos + mROIStrips[s + 1]->YPhysicalSize) * F2VGy;
			}
			double amplitude = startNextVolt - endVolt;
			int skipLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[s]->flyToNextStripeSkipLines / 2 : mROIStrips[s]->flyToNextStripeSkipLines;
			int flySamples = 2 * samplesPerLine * (skipLines - 2);
			double galvoYStep = (amplitude / (double)(flySamples) / GALVO_RESOLUTION);
			double amplitudeCounts = galvoYStep * (double)flySamples;
			short startingPoint = pGalvoYWaveform[wOffsetY - 1];
			for (int j = 0; j < flySamples; ++j)
			{
				double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flySamples));
				pGalvoYWaveform[wOffsetY] = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;
				++wOffsetY;
			}
			for (int j = 0; j < 2 * samplesPerLine * 2; ++j)
			{
				pGalvoYWaveform[wOffsetY] = pGalvoYWaveform[wOffsetY - 1];
				++wOffsetY;
			}
			//flyback to start of first frame
		}
		else if (mROIStrips[s]->IsEnd)
		{
			double startVolt0 = 0;
			endVolt = pGalvoYWaveform[wOffsetY - 1] * GALVO_RESOLUTION;

			double amplitude = startVolt0 - endVolt;
			int flySamples = totalBackwardDataNum;
			double galvoYStep = (amplitude / (double)(flySamples) / GALVO_RESOLUTION);
			double amplitudeCounts = galvoYStep * (double)flySamples;
			short startingPoint = pGalvoYWaveform[wOffsetY - 1];
			for (int j = 0; j < flySamples; ++j)
			{
				double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flySamples));
				pGalvoYWaveform[wOffsetY] = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;
				++wOffsetY;
			}
		}
	}


	for (size_t i = wOffsetY; i < totalSamples; ++i)
	{
		pGalvoYWaveform[wOffsetY] = 0;
		++wOffsetY;
	}

	//----------------Rotate Galvo XY Waveform----------------
	if (0 != pImgAcqPty->scanAreaAngle)
	{
		double angle = (1 == pImgAcqPty->verticalScanDirection) ? pImgAcqPty->scanAreaAngle : -pImgAcqPty->scanAreaAngle;
		double tempX0 = 0, tempY0 = 0, tempX1 = 0, tempY1 = 0;
		short offsetX = 0;
		short offsetY = 0;

		for (unsigned long i = 0; i < static_cast<unsigned long>(totalSamples); ++i)
		{
			//convert back to double number centered around 0
			tempX0 = ((int)pGalvoXWaveform[i]) * GALVO_RESOLUTION + offset_x;
			tempY0 = ((int)pGalvoYWaveform[i]) * GALVO_RESOLUTION + offset_y;

			//calculate transform
			tempX1 = tempX0 * std::cos(angle) - tempY0 * std::sin(angle);
			tempY1 = tempX0 * std::sin(angle) + tempY0 * std::cos(angle);

			pGalvoXWaveform[i] = static_cast<short>(floor(tempX1 * VOLT_TO_THORDAQ_VAL)) - offsetX;

			pGalvoYWaveform[i] = static_cast<short>(floor(tempY1 * VOLT_TO_THORDAQ_VAL)) - offsetY;

			//get the value of the first sample and make it the offset, then make the first sample 0 so the waveform always at zero
			if (0 == i)
			{
				offsetX = pGalvoXWaveform[0];
				pGalvoXWaveform[0] = 0;

				offsetY = pGalvoYWaveform[0];
				pGalvoYWaveform[0] = 0;
			}
		}

		//update the offset with a transformed offset for the angled X and Y waveforms
		offset_x = offsetX * GALVO_RESOLUTION;
		offset_y = offsetY * GALVO_RESOLUTION;

		if (offset_x > MAX_GALVO_VOLTAGE || offset_x < MIN_GALVO_VOLTAGE || offset_y > MAX_GALVO_VOLTAGE || offset_y < MIN_GALVO_VOLTAGE)
		{
			wchar_t errMsg2[MSG_SIZE];
			StringCbPrintfW(errMsg2, MSG_SIZE, L"ThorDAQGalvoGalvo BuildGalvoWaveforms Offsets out of range, OffsetX: %f OffsetY: %f", offset_x, offset_y);
			LogMessage(errMsg2, ERROR_EVENT);
			return FALSE;
		}
	}

	offset_x += galvoCtrlX->offset;

	//--------------Analog X galvo waveformBuffer--------------
	DAC_WAVEFORM_CRTL_STRUCT gXCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gXCtrl.park_val = galvoCtrlX->park;
	gXCtrl.offset_val = offset_x;
	gXCtrl.update_rate = dacRate;
	gXCtrl.flyback_samples = totalBackwardDataNumIntra;
	gXCtrl.output_port = _thordaqAOSelection[AO::GG0_X];
	gXCtrl.waveform_buffer_size = sizeof(USHORT) * (totalSamples);
	gXCtrl.waveformBuffer = (USHORT*)pGalvoXWaveform;
	gXCtrl.filterInhibit = false;
	gXCtrl.hSync = true;
	gXCtrl.enableEOFFreeze = false;

	waveforms[_thordaqAOSelection[AO::GG0_X]] = gXCtrl;

	offset_y += galvoCtrlY->offset;

	//--------------Analog Y galvo waveformBuffer--------------
	DAC_WAVEFORM_CRTL_STRUCT gYCtrl = DAC_WAVEFORM_CRTL_STRUCT();
	gYCtrl.park_val = galvoCtrlY->park;
	gYCtrl.offset_val = offset_y;
	gYCtrl.update_rate = dacRate;
	gYCtrl.flyback_samples = totalBackwardDataNumIntra;
	gYCtrl.output_port = _thordaqAOSelection[AO::GR_Y];
	gYCtrl.waveform_buffer_size = sizeof(USHORT) * (totalSamples);
	gYCtrl.waveformBuffer = (USHORT*)pGalvoYWaveform;
	gYCtrl.filterInhibit = false;
	gYCtrl.hSync = true;
	gYCtrl.enableEOFFreeze = false;

	waveforms[_thordaqAOSelection[AO::GR_Y]] = gYCtrl;

	for (int p = 0; p < MAX_POCKELS_CELL_COUNT; p++)
	{
		if (_pockelsEnable[p] == FALSE)
		{
			continue;
		}
		long pockelOutputChannel = _thordaqAOSelection[(AO)((int)AO::GR_P0 + p)];

		auto pockelPty = &(pImgAcqPty->pockelPty);


		USHORT pockels_output_low_ushort = 0;

		USHORT* pPockelWaveform;
		if (waveforms.find(pockelOutputChannel) != waveforms.end())
		{
			pPockelWaveform = waveforms[pockelOutputChannel].waveformBuffer;
		}
		else
		{
			pPockelWaveform = new USHORT[totalSamples];
		}
		memset(pPockelWaveform, pockels_output_low_ushort, totalSamples * sizeof(USHORT)); //NOTE: Memset can only be used with 0

		//Make sure pockels phase adjust is less than the number of blanking samples
		int pockelsPhaseAdjustSamples = static_cast<int>(round(pockelPty->pockelsDelayUS[p] / 1000000.0 * dacRate));

		INT64 wOffsetPockels = 0;
		double pockelsOnVoltage = 0;

		for (int s = 0; s < mROIStrips.size(); ++s)
		{

			if (p >= mROIStrips[s]->Power.size())
			{
				continue;
			}

			pockelsOnVoltage = pockelPty->pockelsMinVoltage[p] + (pockelPty->pockelsMaxVoltage[p] - pockelPty->pockelsMinVoltage[p]) * mROIStrips[s]->Power[p];
			USHORT pockels_output_high_ushort = static_cast<USHORT>(round((pockelsOnVoltage - pockelPty->pockelsMinVoltage[p]) / DAC_RESOLUTION));


			const long BYTES_PER_PIXEL = 2;
			int width = mROIStrips[s]->XSize;


			int forwardLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (mROIStrips[s]->YSize / 2) : mROIStrips[s]->YSize;
			int scanSamples = 2 * forwardLines * samplesPerLine;

			// Use the two way offset value to find the amount of offset samples needed to align the pockels waveformS
			//If TurnAroundBlank is disabled and the blank percentage is 0, don't do any line blanking. Pockels digital output will mimic the state of the last enabled pockels
			if ((FALSE == _pockelsTurnAroundBlank || _imgAcqPty_Pre.scanMode == TWO_WAY_SCAN))
			{
				for (int i = 0; i < samplesPerLine * 2 * forwardLines; i++)
				{
					INT sample_index = wOffsetPockels + i + pockelsPhaseAdjustSamples >= 0 ? i + pockelsPhaseAdjustSamples : 0; //pockel cell index ptr		
					pPockelWaveform[wOffsetPockels + sample_index] = pockels_output_high_ushort;
				}
			}
			else
			{
				for (int l = 0; l < forwardLines; l++)
				{
					INT sample_index = wOffsetPockels <= 0 || pockelsPhaseAdjustSamples > 0 ? 0 : pockelsPhaseAdjustSamples;

					//There are 4 blanking regions that must be accomodated
					//1-Beginning of s front line scan
					//2-End of s front line scan to begining Of back line scan
					//3-End of s back line scan
					//4-Backscan

					INT beginningOfFrontLineScan = pockelsPhaseAdjustSamples;
					INT endOfFrontLineScan = static_cast<INT>(samplesPerLine);
					INT endOfBackLineScan = static_cast<INT>(samplesPerLine * 2);

					/*StringCbPrintfW(message,MSG_SIZE, L"ThorGR pockels blank percentage spl %d %f phaseAdjustment %f blanking samples %d",samples_per_line, pockelPty->pockelsLineBlankingPercentage[p],_pockelsPhaseAdjustMicroSec, blanking_samples);
					LogMessage(message,ERROR_EVENT);*/

					double const W1 = (width - 1);
					double const PHASE_SHIFT = M_PI * pImgAcqPty->pockelsBlankingPhaseShiftPercent / 100;


					///        |>-----beginningOfFrontLineScan-----------------endOfFrontLineScan--------->-|
					///                                                                                    |
					///        |<-----endOfBackLineScan-----------------------beginingOfBackLineScan-----<-|


					for (; sample_index < beginningOfFrontLineScan; ++sample_index)
					{
						INT sample_offset = l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples > 0 ?
							l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples : l * 2 * samplesPerLine + sample_index;

						pPockelWaveform[wOffsetPockels + sample_offset] = pockels_output_low_ushort;
					}

					for (; sample_index < endOfFrontLineScan; ++sample_index)
					{
						INT sample_offset = l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples > 0 ?
							l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples : l * 2 * samplesPerLine + sample_index;
						pPockelWaveform[wOffsetPockels + sample_offset] = pockels_output_high_ushort;
					}

					for (; sample_index < endOfBackLineScan; ++sample_index)
					{
						INT sample_offset = l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples > 0 ?
							l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples : l * 2 * samplesPerLine + sample_index;

						//only enable the backscan if two way mode is enabled
						if (_imgAcqPty_Pre.scanMode == TWO_WAY_SCAN)
						{
							pPockelWaveform[wOffsetPockels + sample_offset] = pockels_output_high_ushort;
						}
						else
						{
							pPockelWaveform[wOffsetPockels + sample_offset] = pockels_output_low_ushort;
						}
					}

					for (; sample_index < samplesPerLine * 2; ++sample_index)
					{
						INT sample_offset = l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples > 0 ?
							l * 2 * samplesPerLine + sample_index + pockelsPhaseAdjustSamples : l * 2 * samplesPerLine + sample_index;

						//blanking region 4
						pPockelWaveform[wOffsetPockels + sample_offset] = pockels_output_low_ushort;
					}
				}
			}

			int skipLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[s]->flyToNextStripeSkipLines / 2 : mROIStrips[s]->flyToNextStripeSkipLines;
			int flySamples = !mROIStrips[s]->IsEnd ? 2 * samplesPerLine * skipLines : totalBackwardDataNum;

			//80 moving average filter
			for (LONG32 j = 0; j < (LONG32)flySamples + pockelsPhaseAdjustSamples; j++)
			{
				*(pPockelWaveform + wOffsetPockels + scanSamples + j) = pockels_output_low_ushort;
			}

			if (pockelsPhaseAdjustSamples < 0)
			{
				for (LONG32 j = pockelsPhaseAdjustSamples; j < 0; j++)
				{
					*(pPockelWaveform + wOffsetPockels + scanSamples + flySamples + j) = pockels_output_high_ushort;
				}
			}

			wOffsetPockels += scanSamples + flySamples;
		}


		for (ULONG32 j = 0; j < (ULONG32)DAC_FIFO_DEPTH; j++)
		{
			*(pPockelWaveform + wOffsetPockels + j) = pockels_output_low_ushort;
		}


		DAC_WAVEFORM_CRTL_STRUCT pockelsCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		pockelsCtrl.park_val = (1 == _pockelsParkAtMinimum) ? pockelPty->pockelsMinVoltage[p] : pockelsOnVoltage;
		pockelsCtrl.offset_val = pockelPty->pockelsMinVoltage[p];
		pockelsCtrl.update_rate = dacRate;
		pockelsCtrl.flyback_samples = totalBackwardDataNumIntra;
		pockelsCtrl.output_port = (UINT16)pockelOutputChannel;
		pockelsCtrl.waveform_buffer_size = ((size_t)totalSamples) * sizeof(USHORT);
		pockelsCtrl.waveformBuffer = pPockelWaveform;
		pockelsCtrl.filterInhibit = true;
		pockelsCtrl.hSync = true;
		pockelsCtrl.enableEOFFreeze = false;

		waveforms[pockelOutputChannel] = pockelsCtrl;
	}


	if (pImgAcqPty->doZ)
	{
		//Z
		short* pZWaveform = new short[totalSamples]();
		size_t wOffsetZ = 0;
		pZWaveform[wOffsetZ] = 0;
		for (int s = 0; s < mROIStrips.size(); ++s)
		{
			double startVolt = (mROIStrips[s]->ZPos - _zOffsetmm) / _zVolts2mm;
			int frameLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? (mROIStrips[s]->YSize / 2) : mROIStrips[s]->YSize;
			int staticSamples = 2 * frameLines * samplesPerLine;

			for (int s = 0; s < staticSamples; ++s)
			{
				if (wOffsetZ > 0)
				{
					pZWaveform[wOffsetZ] = pZWaveform[wOffsetZ - 1];
				}
				++wOffsetZ;
			}

			if (!mROIStrips[s]->IsEnd && mROIStrips.size() > s + 1)
			{
				double startNextVolt = (mROIStrips[s + 1]->ZPos - _zOffsetmm) / _zVolts2mm;

				double amplitude = startNextVolt - startVolt;
				int skipLines = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? mROIStrips[s]->flyToNextStripeSkipLines / 2 : mROIStrips[s]->flyToNextStripeSkipLines;
				int flySamples = 2 * samplesPerLine * (skipLines);
				double ZPos = (amplitude / DAC_RESOLUTION);

				short startingPoint = pZWaveform[wOffsetZ - 1];

				double zPosDif = abs(mROIStrips[s]->ZPos - mROIStrips[s + 1]->ZPos);

				if (zPosDif == 0)
				{
					for (int j = 0; j < flySamples; ++j)
					{
						pZWaveform[wOffsetZ] = pZWaveform[wOffsetZ - 1];
						++wOffsetZ;
					}
				}
				else
				{
					int settlingSteps = (int)ceil(zPosDif / _zTypicalMove_mm);
					int flyingSamples2 = (int)(flySamples / 1.25);

					double ZStep = ((amplitude / (double)(flyingSamples2) / DAC_RESOLUTION));

					double finalZpos = static_cast<short>(min(SHRT_MAX, ZPos)) + startingPoint;
					double amplitudeCounts = ZStep * (double)flyingSamples2;

					for (int j = 0; j < flyingSamples2; ++j)
					{
						double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flyingSamples2));

						double newZPos = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;




						if ((finalZpos < newZPos && ZStep > 0 || finalZpos > newZPos && ZStep < 0) || j == (flyingSamples2 - 1))
						{
							newZPos = finalZpos;
						}

						pZWaveform[wOffsetZ] = (short)newZPos;
						++wOffsetZ;
					}

					for (int j = 0; j < flySamples - flyingSamples2; ++j)
					{
						pZWaveform[wOffsetZ] = pZWaveform[wOffsetZ - 1];
						++wOffsetZ;
					}
				}
			}
			else if (mROIStrips[s]->IsEnd)
			{
				double startVolt0 = (mROIStrips[s]->ZPos - _zOffsetmm) / _zVolts2mm;
				double endVolt = (mROIStrips[0]->ZPos - _zOffsetmm) / _zVolts2mm;
				double amplitude = endVolt - startVolt;
				int flySamples = totalBackwardDataNum;
				double ZPos = (amplitude / DAC_RESOLUTION);
				short startingPoint = pZWaveform[wOffsetZ - 1];

				double zPosDif = abs(mROIStrips[s]->ZPos - mROIStrips[0]->ZPos);
				if (zPosDif == 0)
				{
					for (int j = 0; j < flySamples; ++j)
					{
						pZWaveform[wOffsetZ] = pZWaveform[wOffsetZ - 1];
						++wOffsetZ;
					}
				}
				else
				{
					int flyingSamples2 = (int)(flySamples / 1.25);

					double ZStep = ((amplitude / (double)(flyingSamples2) / GALVO_RESOLUTION));
					///*ZStep = ZStep > 0 ? ceil(ZStep) : floor(ZStep);*/
					double finalZpos = static_cast<short>(min(SHRT_MAX, ZPos)) + startingPoint;
					double amplitudeCounts = ZStep * (double)flyingSamples2;
					//	for (int i = 0; i < flyingSamples2; ++i)
						//{


					for (int j = 0; j < flyingSamples2; ++j)
					{
						double sinE = (sin(-M_PI_2 + M_PI * (double)(j) / (double)flyingSamples2));

						double newZPos = static_cast<short>(min(SHRT_MAX, (amplitudeCounts * sinE / 2 + amplitudeCounts / 2))) + startingPoint;




						if ((finalZpos < newZPos && ZStep > 0 || finalZpos > newZPos && ZStep < 0) || j == (flyingSamples2 - 1))
						{
							newZPos = finalZpos;
						}

						pZWaveform[wOffsetZ] = (short)newZPos;
						++wOffsetZ;
					}

					for (int j = 0; j < flySamples - flyingSamples2; ++j)
					{
						pZWaveform[wOffsetZ] = pZWaveform[wOffsetZ - 1];
						++wOffsetZ;
					}
				}
			}
		}

		for (size_t i = wOffsetZ; i < totalSamples; ++i)
		{
			pZWaveform[wOffsetZ] = 0;
			++wOffsetZ;
		}

		double offset_z = (mROIStrips[0]->ZPos - _zOffsetmm) / _zVolts2mm;

		offset_z = offset_z > 0 ? min(MAX_GALVO_VOLTAGE, offset_z) : max(MIN_GALVO_VOLTAGE, offset_z);

		//--------------Analog Z Piezo waveformBuffer--------------
		DAC_WAVEFORM_CRTL_STRUCT gZCtrl = DAC_WAVEFORM_CRTL_STRUCT();
		gZCtrl.park_val = offset_z;
		gZCtrl.offset_val = offset_z;
		gZCtrl.update_rate = dacRate;
		gZCtrl.flyback_samples = totalBackwardDataNumIntra;
		gZCtrl.output_port = _thordaqAOSelection[AO::Z];
		gZCtrl.waveform_buffer_size = sizeof(USHORT) * (totalSamples);
		gZCtrl.waveformBuffer = (USHORT*)pZWaveform;
		gZCtrl.filterInhibit = false;
		gZCtrl.hSync = true;
		gZCtrl.enableEOFFreeze = false;

		waveforms[_thordaqAOSelection[AO::Z]] = gZCtrl;

	}
	return TRUE;
}

long CThordaqResonantGalvo::SetupFrameBuffermROI(ImgAcqPty* pImgAcqPty, vector<StripInfo*> mROIStrips)
{
	if (mROIStrips.size() <= 0)
	{
		return FALSE;
	}


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
		//BUFFER #3: circular buffer for read (by user) and write (by camera):
		//int channelCount = CountChannelBits(_imgAcqPty.channel); // Do later
		SAFE_DELETE_PTR(_pFrmBuffer);
		if (ICamera::SW_FREE_RUN_MODE == pImgAcqPty->triggerMode || ICamera::HW_MULTI_FRAME_TRIGGER_EACH == pImgAcqPty->triggerMode) // continuous scan
		{
			// No need for s big buffer, better to keep it at 1 (transerFrames = 1 in continuous mode)so we know we are grabbing the last frame
			_pFrmBuffer = new mROICircularBuffer(mROIStrips, channelCount, sizeof(USHORT), (size_t)_daqAcqCfg.imageCtrl.frameNumPerTransfer * DEFAULT_DMA_BUFFER_NUM, _mROIScan->IsmROI);
		}
		else
		{
			size_t dmaBufferCount = (ULONG)pImgAcqPty->dmaBufferCount > _daqAcqCfg.imageCtrl.frameCnt ? _daqAcqCfg.imageCtrl.frameCnt : static_cast<size_t>(pImgAcqPty->dmaBufferCount);
			// Use DMA buffer to setup the size of this circular buffer
			_pFrmBuffer = new mROICircularBuffer(mROIStrips, channelCount, sizeof(USHORT), dmaBufferCount, _mROIScan->IsmROI);
		}
	}

	{
		for (int i = 0; i < _pTempBuf.size(); ++i)
		{
			SAFE_DELETE_PTR(_pTempBuf[i]);
		}
		_pTempBuf.clear();


		for (int i = 0; i < _pTempBufCorrection.size(); ++i)
		{
			SAFE_DELETE_PTR(_pTempBufCorrection[i]);
		}
		_pTempBufCorrection.clear();

		for (int i = 0; i < _pHistoryBuf.size(); ++i)
		{
			SAFE_DELETE_PTR(_pHistoryBuf[i]);
		}
		_pHistoryBuf.clear();

		UINT currentScanAreaID = -1;
		UINT areaWidth = 0;
		UINT areaHeight = 0;
		INT top = 0;
		INT left = 0;
		bool areaChanged = true;
		bool leftSet = false;
		vector<ProcessedFrame*> imageAreasData;
		int horizontalFlipSign = pImgAcqPty->horizontalFlip ? -1 : 1;
		int verticalFlipSign = pImgAcqPty->verticalScanDirection ? -1 : 1;
		int stripes = 1;
		for (int i = 0; i < mROIStrips.size(); ++i)
		{
			if (currentScanAreaID != mROIStrips[i]->ScanAreaID)
			{
				stripes = 0;
				currentScanAreaID = mROIStrips[i]->ScanAreaID;
				for (int j = i; j < mROIStrips.size(); ++j)
				{
					if (currentScanAreaID == mROIStrips[j]->ScanAreaID)
					{
						++stripes;
					}
				}

			}

			currentScanAreaID = mROIStrips[i]->ScanAreaID;
			UINT fullImageWidth = 0;
			UINT fullImageHeight = 0;

			fullImageWidth = (int)floor(mROIStrips[i]->FullFOVPhysicalSizeUM / mROIStrips[i]->XPixelSize);
			fullImageWidth = (fullImageWidth % 2) != 0 ? fullImageWidth + 1 : fullImageWidth;

			fullImageHeight = (int)floor(mROIStrips[i]->FullFOVPhysicalSizeUM / mROIStrips[i]->XPixelSize);
			fullImageHeight = (fullImageHeight % 2) != 0 ? fullImageHeight + 1 : fullImageHeight;

			if ((mROIStrips.size() > i + 1 && currentScanAreaID != mROIStrips[i + 1]->ScanAreaID) ||
				(mROIStrips.size() == i + 1))
			{
				areaWidth += mROIStrips[i]->XSize;
				areaHeight = mROIStrips[i]->YSize;

				if (_mROIScan->IsmROI)
				{
					top = (int)floor((double)fullImageHeight / 2 + verticalFlipSign * mROIStrips[i]->YPos / mROIStrips[i]->YPixelSize);

					if (0 == left && !leftSet)
					{
						left = (int)floor((double)fullImageWidth / 2 + (horizontalFlipSign * mROIStrips[i]->XPos - mROIStrips[i]->XPhysicalSize / 2) / mROIStrips[i]->XPixelSize);
					}
				}
				else
				{
					fullImageWidth = areaWidth;
					fullImageHeight = areaHeight;
				}

				if (pImgAcqPty->verticalScanDirection)
				{
					top -= areaHeight - 1;
				}


				if (pImgAcqPty->horizontalFlip)
				{
					left -= (stripes - 1) * areaWidth / stripes - 1;
				}

				if (top < 0)
				{
					top = 0;
				}

				if (left < 0)
				{
					left = 0;
				}

				ProcessedFrame* pfH = new ProcessedFrame(areaWidth, areaHeight, channelCount, currentScanAreaID, fullImageWidth, fullImageHeight, top, left);
				_pHistoryBuf.push_back(pfH);

				ProcessedFrame* pfT = new ProcessedFrame(areaWidth, areaHeight, channelCount, currentScanAreaID, fullImageWidth, fullImageHeight, top, left);
				_pTempBuf.push_back(pfT);

				if (pImgAcqPty->enableImageDistortionCorrection)
				{
					ProcessedFrame* pfC = new ProcessedFrame(areaWidth, areaHeight, channelCount, currentScanAreaID, fullImageWidth, fullImageHeight, top, left);
					_pTempBufCorrection.push_back(pfC);
				}

				areaWidth = 0;
				areaHeight = 0;
				top = 0;
				left = 0;
				leftSet = false;
			}
			else
			{
				areaWidth += mROIStrips[i]->XSize;

				if (0 == left)
				{
					left = (int)floor((double)fullImageWidth / 2 + (horizontalFlipSign * mROIStrips[i]->XPos - mROIStrips[i]->XPhysicalSize / 2) / mROIStrips[i]->XPixelSize);
					leftSet = true;
				}
			}
		}
	}
	return STATUS_SUCCESSFUL;
}


double CThordaqResonantGalvo::GetField2VoltsX(double maxSizeInUMForMaxFieldSize)
{
	double amplitude = (double)_fieldSizeMax * _field2Theta / 2;

	if (TRUE == _useZoomArray)
	{
		amplitude = amplitude + amplitude * _zoomArray[_fieldSizeMax] / 100.0;
	}

	double maxPosXVoltage = amplitude / 2;
	double minPosXVoltage = -amplitude / 2;
	double F2VGx = (maxPosXVoltage - minPosXVoltage) / (maxSizeInUMForMaxFieldSize);

	return F2VGx;
}

double CThordaqResonantGalvo::GetField2VoltsY(double maxSizeInUMForMaxFieldSize)
{
	double amplitude = (double)_fieldSizeMax * _field2Theta / 2;

	if (TRUE == _useZoomArray)
	{
		amplitude = amplitude + amplitude * _zoomArray[_fieldSizeMax] / 100.0;
	}

	double maxPosYVoltage = amplitude / 2;
	double minPosYVoltage = -amplitude / 2;

	double F2VGy = (maxPosYVoltage - minPosYVoltage) / (maxSizeInUMForMaxFieldSize);

	return F2VGy;
}

long CThordaqResonantGalvo::
CalculatemROITotalImagingLinesAndOtherProperties(ImgAcqPty* pImgAcqPty, vector<StripInfo*> mROIStripes, int& overlLinesOut, int& vSizeOut, int& hSizeOut, int& totalXPixelsOut, int& totalYPixelsOut, int& flybackCycleOut)
{
	if (mROIStripes.size() <= 0)
	{
		LogMessage(L"ThordaqGR no mROI stripes generated", ERROR_EVENT);
		return FALSE;
	}
	USHORT vSize = 0;
	int rx = 0;
	int ry = 0;
	_mROIScan->MaxSizeInUMForMaxFieldSize;
	double lineCycleTime = 1 / _current_resonant_scanner_frequency;

	double F2VGx = GetField2VoltsX(_mROIScan->MaxSizeInUMForMaxFieldSize);

	double F2VGy = GetField2VoltsY(_mROIScan->MaxSizeInUMForMaxFieldSize);

	totalXPixelsOut = 0;
	totalYPixelsOut = 0;
	flybackCycleOut = 0;

	//TODO: vertical flip will cause issue for 2nd and up mROIs not imaging the right area

	for (int i = 0; i < mROIStripes.size(); ++i)
	{
		if (mROIStripes.size() > 1 && i + 1 < mROIStripes.size())
		{
			double startVoltX = mROIStripes[i]->XPos * F2VGx;
			double startNextVoltX = mROIStripes[i + 1]->XPos * F2VGx;
			double amplitudeX = abs(startNextVoltX - startVoltX);

			double endVolt = (mROIStripes[i]->YPos + mROIStripes[i]->YPhysicalSize) * F2VGy;
			double startNextVoltY = mROIStripes[i + 1]->YPos * F2VGy;
			double amplitudeY = abs(startNextVoltY - endVolt);

			double amplitude = amplitudeX > amplitudeY ? amplitudeX : amplitudeY;

			UINT minFlybackCycle = (UINT)GetMinFlybackCycle(amplitude);

			UINT minFlyLines = (UINT)(TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * minFlybackCycle : minFlybackCycle);

			double zDif = abs(mROIStripes[i]->ZPos - mROIStripes[i + 1]->ZPos);
			double minTime = _zTypicalMoveSettlingTime_sec * zDif / _zTypicalMove_mm;

			UINT minZFlyCyles = (UINT)round(minTime / lineCycleTime);

			UINT minZFlyLines = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * minZFlyCyles : minZFlyCyles;

			mROIStripes[i]->flyToNextStripeSkipLines = (UINT)(minZFlyLines > minFlyLines ? minZFlyLines : minFlyLines);
		}
		else if (mROIStripes[i]->IsEnd && mROIStripes.size() > 1)
		{
			double startVoltX = mROIStripes[i]->XPos * F2VGx;
			double startNextVoltX = mROIStripes[0]->XPos * F2VGx;
			double amplitudeX = abs(startNextVoltX - startVoltX);

			double endVolt = (mROIStripes[i]->YPos + mROIStripes[i]->YPhysicalSize) * F2VGy;
			double startNextVoltY = mROIStripes[0]->YPos * F2VGy;
			double amplitudeY = abs(startNextVoltY - endVolt);

			double amplitude = amplitudeX > amplitudeY ? amplitudeX : amplitudeY;

			UINT minFlybackCycle = (UINT)GetMinFlybackCycle(amplitude);

			UINT minFlyLines = (UINT)(TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * minFlybackCycle : minFlybackCycle);

			double zDif = abs(mROIStripes[i]->ZPos - mROIStripes[0]->ZPos);
			double minTime = _zTypicalMoveSettlingTime_sec * zDif / _zTypicalMove_mm;

			UINT minZFlyCyles = (UINT)round(minTime / lineCycleTime) + 2;

			UINT minZFlyLines = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * minZFlyCyles : minZFlyCyles;

			int flybackCycle = (UINT)pImgAcqPty->flybackCycle > minFlyLines ? pImgAcqPty->flybackCycle : (int)minFlyLines;

			mROIStripes[i]->flyToNextStripeSkipLines = (UINT)((int)minZFlyLines > 2 * flybackCycle ? minZFlyLines : 2 * flybackCycle);
		}
		else
		{
			double endVolt = (mROIStripes[i]->YPos + mROIStripes[i]->YPhysicalSize) * F2VGy;
			double startNextVoltY = mROIStripes[i]->YPos * F2VGy;
			double amplitudeY = abs(startNextVoltY - endVolt);
			int minFlybackCycle = GetMinFlybackCycle(amplitudeY);
			int minFlyLines = TWO_WAY_SCAN == pImgAcqPty->scanMode ? 2 * minFlybackCycle : minFlybackCycle;

			mROIStripes[i]->flyToNextStripeSkipLines = (UINT)minFlyLines;
		}

		if (mROIStripes[i]->IsEnd)
		{
			UINT flybackCycle = TWO_WAY_SCAN == pImgAcqPty->scanMode ? mROIStripes[i]->flyToNextStripeSkipLines / 2 : mROIStripes[i]->flyToNextStripeSkipLines;
			flybackCycleOut = flybackCycle;
		}

		totalXPixelsOut += mROIStripes[0]->XSize;
		totalYPixelsOut = mROIStripes[i]->YSize;

		vSize += mROIStripes[i]->YSize;
		if (!mROIStripes[i]->IsEnd)
		{
			vSize += mROIStripes[i]->flyToNextStripeSkipLines;
		}
	}

	hSizeOut = (int)mROIStripes[0]->XSize;
	vSizeOut = vSize;

	overlLinesOut = (TWO_WAY_SCAN == pImgAcqPty->scanMode) ? vSize / 2 : vSize;

	return TRUE;
}


// TODO: Fix X and Y for twoway -- done
// TODO: Get mROI working -- done
// TODO: Get multiple channels working with mROI -- done
// TODO: Update ImageViewVM to be able to display the MROI areas -- done
// TODO: Use Slider instead of combobox to select Stripe Width -- done
// TODO: Get Z working  -- done
// TODO: Fix issue where the very first time imaging stripes gets stopped and started again once -- done
// TODO: Fix having to image in Full FOV before imaging in mROI everytime -- done
// TODO: In MesoMVM set and read the mROIStripeFieldSize instead of the SelectedStripSize -- done
// TODO: Make processing work with multiple ROIs taking into consideration wheather an ROI contains more than one stripe -- done
// TODO: Get Pockels working -- done
// TODO: Fix Full FOV and mROI setup so it uses s calculated field size instead of microns -- done
// TODO: Need to get the Max FieldSize fieldcalibration value, instead of current fieldSize -- done I think
// TODO: Need to find a way of separating full FOV field size from the scanner field size -- done
// 
// TODO: Fix issue where if I have no ROI it sometimes uses the last ROI and doesn't image FULL FOV
// TODO: In ImageViewModel, keep a copy of the data per area so we can adjust histograms after the fact


// TODO: Improve GUI and name everything in the GUI as stripes instead of strips, as Full FOV instead of MESO, and as mROI instead of MICRO
// TODO: Update all the naming of variables, projects, dlls, xml nodes and parameters as follows: stripes instead of strips, as Full FOV instead of MESO, and as mROI instead of MICRO
// TODO: Use fieldsizes and field locations in field size units with decimal places instead of microns, so it works without having to calibrate the galvo voltage to microns relationship, we are already calibrating galvo to fieldsize so we should keep it there.
//			However, we should keep the current parameters in the xml because they do provide important metadata to the customer. We should instead add new parameters using field size units -- done, still using um but calibrated by passing max um for max fieldsize
// TODO: fix regular imaging flyback to use what we have now in mROI
// TODO: Update everything when the Magnitude is updated
// TODO: Move init R scanner checkbox to Scan control
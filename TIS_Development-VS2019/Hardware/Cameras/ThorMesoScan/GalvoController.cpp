#include "GalvoController.h"
#include "stdafx.h"
#include "MesoScanWaveform.h"
#include "DevParamDef.h"

#define ERROR_STR_LEN (2048)

#define GALVO_MOVE_MAX_LENGTH (75000)
#define GALVO_MOVE_CLK (1200000.0)

#define X_MINIMAL_STEP (0.0005)
#define Y_MINIMAL_STEP (0.0005)
#define Z_MINIMAL_STEP (0.0002)

#define BUFFER_COUNT_IN_NI_CARD (8)

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

GalvoController * GalvoController::_pInstance = NULL;

GalvoController::GalvoController(MesoScanWaveform* pMesoScanWaveform) :_hasGalvoController(true)
{
	_pMesoScanWaveform = pMesoScanWaveform;
	_isActive = false;
	_isContinueOutput = false;
	_pInstance = this;
}

long GalvoController::GetTravelLength(double x1Start, double x1End, double x2Start, double x2End, double yStart, double yEnd)
{
	long x1Length = (x1End - x1Start) / X_MINIMAL_STEP;
	long x2Length = (x2End - x2Start) / X_MINIMAL_STEP;
	long yLength = (yEnd - yStart) / Y_MINIMAL_STEP;
	long length = x1Length >= x2Length ? (yLength >= x1Length ? yLength : x1Length) : (yLength >= x2Length ? yLength : x2Length);
	length = length >= 1 ? length : 1;
	return length;
}

long GalvoController::GetCurrentPos(double &x1, double &x2, double &y)
{
#ifdef READ_POS
	int32 retVal = TRUE, error = 0;

	retVal = DAQmxStopTask(_taskHandleAI0);
	retVal = DAQmxClearTask(_taskHandleAI0);

	DAQmxErrChk(DAQmxCreateTask("", &_taskHandleAI0));
	retVal = DAQmxCreateAIVoltageChan(_taskHandleAI0, "Dev1/ai1", "", DAQmx_Val_RSE, MIN_AO_VOLTAGE, MAX_AO_VOLTAGE, DAQmx_Val_Volts, NULL);

	int32 samplesRead;
	DAQmxReadAnalogF64(_taskHandleAI0, 1, 10.0, DAQmx_Val_GroupByChannel, &x1, 2, &samplesRead, NULL);
	x2 = 0.0;
	y = 0.0;
	return retVal;
Error:
	x1 = 0.0;
	x2 = 0.0;
	y = 0.0;
	return retVal;
#else
	x1 = 0.0;
	x2 = 0.0;
	y = 0.0;
	return TRUE;
#endif
}

void GalvoController::CreateGalvoWaveform(double* data, long length, double start, double end)
{
	int cycle;
	double step = (end - start) / length;
	for (cycle = 0; cycle < length; cycle++)
	{
		data[cycle] = start + cycle * step;
	}
}

GalvoController::~GalvoController()
{
}

long GalvoController::StartGG()
{
	return TRUE;
}

long GalvoController::StopGG()
{
	return TRUE;
}

long GalvoController::MoveGGToPos(double x1, double x2)
{
	if (!_hasGalvoController)
	{
		return FALSE;
	}
	int         error = 0;
	_taskHandleAO0 = 0;
	_taskHandleAO1 = 0;
	float64     data0[GALVO_MOVE_MAX_LENGTH];
	float64     data1[GALVO_MOVE_MAX_LENGTH];
	int			length = 0;
	char        errBuff[ERROR_STR_LEN] = { '\0' };
	int32   	written0;

	double x1Start = 0, x2Start = 0, yStart = 0;
	double x1End = 0, x2End = 0, yEnd = 0;
	GetCurrentPos(x1Start, x2Start, yStart);
	_pMesoScanWaveform->GetStartPos(x1End, x2End, yEnd);
	length = GetTravelLength(x1Start, x1End, x2Start, x2End, yStart, yEnd);

	DAQmxErrChk(DAQmxCreateTask("", &_taskHandleAO0));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandleAO0, DEV_GX_OUT_STR, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxSetWriteRegenMode(_taskHandleAO0, DAQmx_Val_DoNotAllowRegen));
	DAQmxErrChk(DAQmxCreateTask("", &_taskHandleAO1));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandleAO1, DEV_GY_OUT_STR, "", VOLT_MIN, VOLT_MAX, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxSetWriteRegenMode(_taskHandleAO1, DAQmx_Val_DoNotAllowRegen));
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO0, "", GALVO_MOVE_CLK, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, length));
	DAQmxErrChk(DAQmxCfgSampClkTiming(_taskHandleAO1, "", GALVO_MOVE_CLK, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, length));

	DAQmxErrChk(DAQmxCfgOutputBuffer(_taskHandleAO0, length));
	DAQmxErrChk(DAQmxCfgOutputBuffer(_taskHandleAO1, length));
	CreateGalvoWaveform(data0, length, x1Start, x1End);
	CreateGalvoWaveform(data0 + length, length, x2Start, x2End);
	CreateGalvoWaveform(data1, length, yStart, yEnd);

	DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandleAO0, length, 0, WRITE_TIMEOUT, DAQmx_Val_GroupByChannel, data0, &written0, NULL));
	DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandleAO1, length, 0, WRITE_TIMEOUT, DAQmx_Val_GroupByChannel, data1, &written0, NULL));

	DAQmxErrChk(DAQmxStartTask(_taskHandleAO0));
	DAQmxErrChk(DAQmxStartTask(_taskHandleAO1));

	DAQmxErrChk(DAQmxWaitUntilTaskDone(_taskHandleAO0, WAIT_TIME));
	DAQmxErrChk(DAQmxWaitUntilTaskDone(_taskHandleAO1, WAIT_TIME));

	long ret = TRUE;
Error:
	if (DAQmxFailed(error))
	{
		DAQmxGetExtendedErrorInfo(errBuff, ERROR_STR_LEN);
		ret = FALSE;
	}
	if (_taskHandleAO0 != 0) {
		DAQmxStopTask(_taskHandleAO0);
		DAQmxClearTask(_taskHandleAO0);
	}
	if (_taskHandleAO1 != 0) {
		DAQmxStopTask(_taskHandleAO1);
		DAQmxClearTask(_taskHandleAO1);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit\n");
	return ret;
}

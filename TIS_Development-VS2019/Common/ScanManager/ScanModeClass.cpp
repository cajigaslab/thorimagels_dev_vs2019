//ScanModeClass.cpp: Defines ScanModeClass functions.
//

#include "ScanModeClass.h"
#include "..\camera.h"
#include "..\ThorSharedTypes\ThorSharedTypes\SharedEnums.cs"

#define FUNC_CHECK_BREAK(retVal, func) { retVal = func; if(0 == retVal) break; }
#define FUNC_CHECK_RETURN(func) { if(0 == func) return 0; }

///	***************************************** <summary> TwoWay Scan Functions </summary>	********************************************** ///

long TwoWayScan::GetParam(BehaviorProp bProp, long& pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		pVal = _active;
		ret = 1;
		break;
	case LINE_FACTOR:
		pVal = _lineFactor;
		ret = 1;
		break;
	case SCAN_MODE:
		pVal = static_cast<long>(_scanMode);
		ret = 1;
		break;
	case AVERAGE_MODE:
		pVal = static_cast<long>(_avgMode);
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		pVal = _procBufLineCount;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		pVal = _interleaveScan;
		ret = 1;
		break;
	}
	return ret;
}

long TwoWayScan::SetParam(BehaviorProp bProp, long pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		_active = pVal;
		ret = 1;
		break;
	case AVERAGE_MODE:
		_avgMode = (AverageMode)pVal;
		ret = 1;
		break;
	case SWITCH_BEHAVIOR:
		_switchBehavior = pVal;
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		_procBufLineCount = pVal;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		_interleaveScan = pVal;
		ret = 1;
		break;
	}
	return ret;
}

long TwoWayScan::ProcessBuffer(long procFrameID, long lineStart, long lineEnd)
{
	//do nothing if not active behavior
	if((!_active) || (_switchBehavior))
		return 0;

	long ret = 1;
	long offsetEvenOdd = (0 == procFrameID % 2) ? BufferProcMode::OFFSET_EVEN : BufferProcMode::OFFSET_ODD;
	offsetEvenOdd = (_interleaveScan) ? offsetEvenOdd : BufferProcMode::OFFSET_NONE;

	//only set process mode once at the beginning of frame
	//when process buffer section-wise
	if((0 == lineEnd) || (0 == lineStart))
	{
		switch (_avgMode)
		{
		case AverageMode::NO_AVERAGE:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_NO_AVG));
			break;
		case AverageMode::FRM_CUMULATIVE_MOVING:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_FRM_CMA));
			break;
		}
	}

	long end = (0 < lineEnd) ? lineEnd : _procBufLineCount;
	for (long i = lineStart; i < end; i++)
	{
		if(_switchBehavior)
			break;

		switch ((BufferProcMode)offsetEvenOdd)
		{
		case BufferProcMode::OFFSET_NONE:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			break;
		case BufferProcMode::OFFSET_EVEN:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			break;
		case BufferProcMode::OFFSET_ODD:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			break;
		}
	}

	//notice done process one frame
	if(!_switchBehavior && (_procBufLineCount == end))
		_lsmActor.SetAction(ActionType::DONE_PROC_ONE_BUFFER);

	return ret;
}

long TwoWayScan::PreflightAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	if (0 == _lsmActor.SetAction(ActionType::INITIAL_PROPERTIES))
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
	{
		_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

		_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

		return _lsmActor.SetAction(ActionType::PREFLIGHT_PROPERTIES);
	}
	return 0;
}

long TwoWayScan::SetupAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_CHECK))
	{
		if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
		{
			_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

			_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

			return _lsmActor.SetAction(ActionType::PERSIST_PROPERTIES);
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

long TwoWayScan::StartAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	return _lsmActor.SetAction(ActionType::START_PROTOCOL);
}

long TwoWayScan::StatusAcquisition(long& status)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::STATUS_PROTOCOL, status);
}

long TwoWayScan::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::COPY_PROTOCOL, pDataBuffer);
}

long TwoWayScan::PostflightAcquisition(char * pDataBuffer)
{
	long val = 0;
	if (NULL != pDataBuffer) 
		memcpy_s((void*)&val, sizeof(long), (void*)pDataBuffer, sizeof(long));

	return _lsmActor.SetActionWithParam(ActionType::POSTFLIGHT_PROTOCOL, (long)val);
}

///	***************************************** <summary> Forward Scan Functions </summary>	********************************************** ///

long ForwardScan::GetParam(BehaviorProp bProp, long& pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		pVal = _active;
		ret = 1;
		break;
	case LINE_FACTOR:
		pVal = _lineFactor;
		ret = 1;
		break;
	case SCAN_MODE:
		pVal = static_cast<long>(_scanMode);
		ret = 1;
		break;
	case AVERAGE_MODE:
		pVal = static_cast<long>(_avgMode);
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		pVal = _procBufLineCount;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		pVal = _interleaveScan;
		ret = 1;
		break;
	}
	return ret;
}

long ForwardScan::SetParam(BehaviorProp bProp, long pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		_active = pVal;
		ret = 1;
		break;
	case AVERAGE_MODE:
		_avgMode = (AverageMode)pVal;
		ret = 1;
		break;
	case SWITCH_BEHAVIOR:
		_switchBehavior = pVal;
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		_procBufLineCount = pVal;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		_interleaveScan = pVal;
		ret = 1;
		break;
	}
	return ret;
}

long ForwardScan::ProcessBuffer(long procFrameID, long lineStart, long lineEnd)
{
	if((!_active) || (_switchBehavior))
		return 0;

	long ret = 1;
	long offsetEvenOdd = (0 == procFrameID % 2) ? BufferProcMode::OFFSET_EVEN : BufferProcMode::OFFSET_ODD;
	offsetEvenOdd = (_interleaveScan) ? offsetEvenOdd : BufferProcMode::OFFSET_NONE;

	//only set process mode once at the beginning of frame
	//when process buffer section-wise
	if((0 == lineEnd) || (0 == lineStart))
	{
		switch (_avgMode)
		{
		case AverageMode::NO_AVERAGE:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_NO_AVG));
			break;
		case AverageMode::FRM_CUMULATIVE_MOVING:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_FRM_CMA));
			break;
		}
	}

	long end = (0 < lineEnd) ? lineEnd : _procBufLineCount;
	for (long i = lineStart; i < end; i++)
	{
		if(_switchBehavior)
			break;

		switch ((BufferProcMode)offsetEvenOdd)
		{
		case BufferProcMode::OFFSET_NONE:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			break;
		case BufferProcMode::OFFSET_EVEN:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			break;
		case BufferProcMode::OFFSET_ODD:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC1));
			break;
		}
	}

	//notice done process one frame
	if(!_switchBehavior && (_procBufLineCount == end))
		_lsmActor.SetAction(ActionType::DONE_PROC_ONE_BUFFER);

	return ret;
}

long ForwardScan::PreflightAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	if (0 == _lsmActor.SetAction(ActionType::INITIAL_PROPERTIES))
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
	{
		_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

		_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

		return _lsmActor.SetAction(ActionType::PREFLIGHT_PROPERTIES);
	}
	return 0;
}

long ForwardScan::SetupAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_CHECK))
	{
		if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
		{
			_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

			_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

			return _lsmActor.SetAction(ActionType::PERSIST_PROPERTIES);
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

long ForwardScan::StartAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	return _lsmActor.SetAction(ActionType::START_PROTOCOL);
}

long ForwardScan::StatusAcquisition(long& status)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::STATUS_PROTOCOL, status);
}

long ForwardScan::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::COPY_PROTOCOL, pDataBuffer);
}

long ForwardScan::PostflightAcquisition(char * pDataBuffer)
{
	long val = 0;
	if (NULL != pDataBuffer) 
		memcpy_s((void*)&val, sizeof(long), (void*)pDataBuffer, sizeof(long));

	return _lsmActor.SetActionWithParam(ActionType::POSTFLIGHT_PROTOCOL, (long)val);
}

///	***************************************** <summary> Backward Scan Functions </summary>	********************************************** ///

long BackwardScan::GetParam(BehaviorProp bProp, long& pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		pVal = _active;
		ret = 1;
		break;
	case LINE_FACTOR:
		pVal = _lineFactor;
		ret = 1;
		break;
	case SCAN_MODE:
		pVal = static_cast<long>(_scanMode);
		ret = 1;
		break;
	case AVERAGE_MODE:
		pVal = static_cast<long>(_avgMode);
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		pVal = _procBufLineCount;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		pVal = _interleaveScan;
		ret = 1;
		break;
	}
	return ret;
}

long BackwardScan::SetParam(BehaviorProp bProp, long pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		_active = pVal;
		ret = 1;
		break;
	case AVERAGE_MODE:
		_avgMode = (AverageMode)pVal;
		ret = 1;
		break;
	case SWITCH_BEHAVIOR:
		_switchBehavior = pVal;
		ret = 1;
		break;
	case PROC_BUF_LINE_COUNT:
		_procBufLineCount = pVal;
		ret = 1;
		break;
	case INTERLEAVE_BUF:
		_interleaveScan = pVal;
		ret = 1;
		break;
	}
	return ret;
}

long BackwardScan::ProcessBuffer(long procFrameID, long lineStart, long lineEnd)
{
	if((!_active) || (_switchBehavior))
		return 0;

	long ret = 1;
	long offsetEvenOdd = (0 == procFrameID % 2) ? BufferProcMode::OFFSET_EVEN : BufferProcMode::OFFSET_ODD;
	offsetEvenOdd = (_interleaveScan) ? offsetEvenOdd : BufferProcMode::OFFSET_NONE;

	//only set process mode once at the beginning of frame
	//when process buffer section-wise
	if((0 == lineEnd) || (0 == lineStart))
	{
		switch (_avgMode)
		{
		case AverageMode::NO_AVERAGE:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_NO_AVG));
			break;
		case AverageMode::FRM_CUMULATIVE_MOVING:
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::PROC_BUFFER_FRM_CMA));
			break;
		}
	}

	long end = (0 < lineEnd) ? lineEnd : _procBufLineCount;
	for (long i = lineStart; i < end; i++)
	{
		if(_switchBehavior)
			break;

		switch ((BufferProcMode)offsetEvenOdd)
		{
		case BufferProcMode::OFFSET_NONE:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			break;
		case BufferProcMode::OFFSET_EVEN:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			break;
		case BufferProcMode::OFFSET_ODD:
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC3));
			FUNC_CHECK_BREAK(ret, _lsmActor.SetAction(ActionType::PROC_BUFFER_FUNC2));
			break;
		}
	}

	//notice done process one frame
	if(!_switchBehavior && (_procBufLineCount == end))
		_lsmActor.SetAction(ActionType::DONE_PROC_ONE_BUFFER);

	return ret;
}

long BackwardScan::PreflightAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	if (0 == _lsmActor.SetAction(ActionType::INITIAL_PROPERTIES))
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
	{
		_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

		_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

		FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

		return _lsmActor.SetAction(ActionType::PREFLIGHT_PROPERTIES);
	}
	return 0;
}

long BackwardScan::SetupAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	if(1 == _lsmActor.SetAction(ActionType::SETUP_CHECK))
	{
		if(1 == _lsmActor.SetAction(ActionType::SETUP_BOARD))
		{
			_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

			_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_CLOCK));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_POCKEL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_DIGITAL));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER));

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_GALVO));

			return _lsmActor.SetAction(ActionType::PERSIST_PROPERTIES);
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

long BackwardScan::StartAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	return _lsmActor.SetAction(ActionType::START_PROTOCOL);
}

long BackwardScan::StatusAcquisition(long& status)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::STATUS_PROTOCOL, status);
}

long BackwardScan::CopyAcquisition(char * pDataBuffer, void* frameInfo)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::COPY_PROTOCOL, pDataBuffer);
}

long BackwardScan::PostflightAcquisition(char * pDataBuffer)
{
	long val = 0;
	if (NULL != pDataBuffer) 
		memcpy_s((void*)&val, sizeof(long), (void*)pDataBuffer, sizeof(long));

	return _lsmActor.SetActionWithParam(ActionType::POSTFLIGHT_PROTOCOL, (long)val);
}

///	***************************************** <summary> Center Scan Functions </summary>	********************************************** ///

long CenterScan::GetParam(BehaviorProp bProp, long& pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		pVal = _active;
		ret = 1;
		break;
	case LINE_FACTOR:
		pVal = static_cast<long>(_lineFactor);
		ret = 1;
		break;
	case SCAN_MODE:
		pVal = static_cast<long>(_scanMode);
		ret = 1;
		break;
	case AVERAGE_MODE:
		pVal = static_cast<long>(_avgMode);
		ret = 1;
		break;
	}
	return ret;
}

long CenterScan::SetParam(BehaviorProp bProp, long pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		_active = pVal;
		ret = 1;
		break;
	case AVERAGE_MODE:
		_avgMode = (AverageMode)pVal;
		ret = 1;
		break;
	}
	return ret;
}

long CenterScan::PreflightAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_CENTER);

	for(long i=0; i<Constants::MAX_GG_POCKELS_CELL_COUNT; i++)
	{
		_lsmActor.SetActionWithParam(ActionType::MOVE_POCKEL_TO_POWER_LEVEL, i);
	}
	return 1;
}

long CenterScan::StatusAcquisition(long &status)
{
	status = ICamera::STATUS_READY;
	return 1;
}

long CenterScan::PostflightAcquisition(char * pDataBuffer)
{
	long val = 0;
	if (NULL != pDataBuffer) 
		memcpy_s((void*)&val, sizeof(long), (void*)pDataBuffer, sizeof(long));

	return _lsmActor.SetActionWithParam(ActionType::POSTFLIGHT_PROTOCOL, (long)val);
}

///	***************************************** <summary> Bleach Scan Functions </summary>	********************************************** ///

long BleachScan::GetParam(BehaviorProp bProp, long& pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		pVal = _active;
		ret = 1;
		break;
	case SCAN_MODE:
		pVal = static_cast<long>(_scanMode);
		ret = 1;
		break;
	case AVERAGE_MODE:
		pVal = static_cast<long>(_avgMode);
		ret = 1;
		break;
	}
	return ret;
}

long BleachScan::SetParam(BehaviorProp bProp, long pVal)
{
	long ret = 0;
	switch (bProp)
	{
	case ACTIVE_BEHAVIOR:
		_active = pVal;
		ret = 1;
		break;
	case AVERAGE_MODE:
		_avgMode = (AverageMode)pVal;
		ret = 1;
		break;
	}
	return ret;
}

long BleachScan::PreflightAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	_lsmActor.SetAction(ActionType::INITIAL_PROPERTIES);
	return 1;
}

long BleachScan::SetupAcquisition(char * pDataBuffer)
{	
	if(!_active)
		return 0;

	long ret = 0;

	if (_lsmActor.SetAction(ActionType::SETUP_CHECK))
	{
		//BleachScan is a Task-Mater mode which only use event of _hStopAcquisition and _hThreadStopped;
		//Two type of tasks: analog (both galvo and pockels) and digital will be controlled by clockRateNI.
		//user need to define: scanmode, triggermode, clock rate.

		try
		{
			_lsmActor.SetAction(ActionType::MOVE_POCKEL_TO_PARK);

			_lsmActor.SetAction(ActionType::MOVE_GALVO_TO_START);

			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::BUILD_TASK_MASTER));

			//the AO Clock output is not started until the Fram Async Acquisition start
			//could be failed when sharing Pockels with G/R at Dev1
			FUNC_CHECK_RETURN(_lsmActor.SetAction(ActionType::SETUP_TASK_MASTER_CLOCK));

			//Setup Galvo waveform into DAQ
			_lsmActor.SetAction(ActionType::SETUP_TASK_MASTER_GALVO);

			//Setup pockel waveform into DAQ
			_lsmActor.SetAction(ActionType::SETUP_TASK_MASTER_POCKEL);

			//setup digital waveforms into DAQ; Setup Triggers: triggered cycle count be handled in CycleDone callback:
			_lsmActor.SetAction(ActionType::SETUP_TASK_MASTER_DIGITAL);

			//write an additional set to avoid underflow, 
			//after start tasks in active mode:
			_lsmActor.SetAction(ActionType::WRITE_TASK_MASTER_GALVO);
			_lsmActor.SetAction(ActionType::WRITE_TASK_MASTER_POCKEL);
			_lsmActor.SetAction(ActionType::WRITE_TASK_MASTER_LINE);

			//arm proper HW trigger mode
			_lsmActor.SetAction(ActionType::SETUP_CLOCK_MASTER_TRIGGER);

			//to de-signal the Stop Acquisition Event,
			//can only be done at the end:
			_lsmActor.SetAction(ActionType::DONE_SETUP);
			ret = 1;
		}
		catch(...)
		{
			_lsmActor.SetActionWithParam(ActionType::STATUS, ICamera::STATUS_BUSY);
			_lsmActor.SetAction(ActionType::STOP_SCAN);
			return 0;
		}
	}
	return ret;
}

long BleachScan::StartAcquisition(char * pDataBuffer)
{
	if(!_active)
		return 0;

	_lsmActor.SetAction(ActionType::READY_TO_START);
	return _lsmActor.SetAction(ActionType::START_SCAN);
}

long BleachScan::StatusAcquisition(long &status)
{
	if(!_active)
		return 0;

	return _lsmActor.GetActionResult(ActionType::STATUS, status);
}

long BleachScan::PostflightAcquisition(char * pDataBuffer)
{
	_lsmActor.SetAction(ActionType::STOP_SCAN);
	long val = 0;
	if (NULL != pDataBuffer) 
		memcpy_s((void*)&val, sizeof(long), (void*)pDataBuffer, sizeof(long));

	return _lsmActor.SetActionWithParam(ActionType::POSTFLIGHT_PROTOCOL, (long)val);
}

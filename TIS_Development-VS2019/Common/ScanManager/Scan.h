//Defines interface functions for LSM scanner
//

#pragma once

enum ScanMode
{
	TWO_WAY_SCAN = 0,
	FORWARD_SCAN = 1,
	BACKWARD_SCAN = 2,
	CENTER = 3,
	BLEACH_SCAN = 4,
	SCANMODE_LAST
};

enum AverageMode
{
	NO_AVERAGE = 0,
	FRM_CUMULATIVE_MOVING,
	FRM_SIMPLE_MOVING,
	LINE_AVERAGE,
	LINE_INTEGRATION,
	PIXEL_AVERAGE,
	PIXEL_INTEGRATION,
	AVERAGE_MODE_LAST
};

enum ActionType
{
	MOVE_GALVO_TO_PARK = 0,			///<Single actions for NI
	MOVE_GALVO_TO_START,
	MOVE_GALVO_TO_CENTER,
	MOVE_POCKEL_TO_PARK,
	MOVE_POCKEL_TO_POWER_LEVEL,
	INITIAL_PROPERTIES,				///<initialize image properties before preflight properties
	PREFLIGHT_PROPERTIES,			///<preflight properties after successful setup board	
	SETUP_CHECK,					///<compare image properties to determine if another setup board is necessary
	SETUP_BOARD,					///<setup digitizer board and build image waveform
	PERSIST_PROPERTIES,				///<update image properties when able to setup boards
	START_PROTOCOL,					
	STATUS_PROTOCOL,
	COPY_PROTOCOL,
	POSTFLIGHT_PROTOCOL,
	SETUP_CLOCK_MASTER_CLOCK,		///<clock master means finite clock with continuous task, used in image scanning
	SETUP_CLOCK_MASTER_GALVO,
	SETUP_CLOCK_MASTER_POCKEL,
	SETUP_CLOCK_MASTER_DIGITAL,
	SETUP_CLOCK_MASTER_TRIGGER,
	SETUP_TASK_MASTER_CLOCK,		///<Task master means finite task with continuous clock, used in bleach scanning
	SETUP_TASK_MASTER_GALVO,
	SETUP_TASK_MASTER_POCKEL,
	SETUP_TASK_MASTER_DIGITAL,
	BUILD_TASK_MASTER,
	WRITE_TASK_MASTER_GALVO,
	WRITE_TASK_MASTER_POCKEL,
	WRITE_TASK_MASTER_LINE,
	DONE_SETUP,
	READY_TO_START,
	STATUS,
	START_SCAN,
	STOP_SCAN,
	PROC_BUFFER_NO_AVG,				///<Process data buffer without average
	PROC_BUFFER_FRM_CMA,			///<Process data buffer with cumulative frame average
	DONE_PROC_ONE_BUFFER,			///<Notice done process data buffer of one frame
	PROC_BUFFER_FUNC1,				///<Process data buffer with function 1
	PROC_BUFFER_FUNC2,
	PROC_BUFFER_FUNC3,
	ACTION_LAST
};

enum BehaviorProp
{
	ACTIVE_BEHAVIOR,				///<behavior is in active flag
	LINE_FACTOR,					///<get line index factor for different scan mode
	SCAN_MODE,
	AVERAGE_MODE,
	SWITCH_BEHAVIOR,				///<signal to switch behavior
	PROC_BUF_LINE_COUNT,			///<forward line count
	INTERLEAVE_BUF,					///<interleave scan
	BEHAVIOR_PROP_LAST
};

class IScan
{
public:

	virtual long PreflightAcquisition(char * pDataBuffer) = 0;
	virtual long SetupAcquisition(char * pDataBuffer) = 0;
	virtual long StartAcquisition(char * pDataBuffer) = 0;
	virtual long StatusAcquisition(long &status) = 0;
	virtual long CopyAcquisition(char * pDataBuffer, void* frameInfo) = 0;
	virtual long PostflightAcquisition(char * pDataBuffer) = 0;
};

class IAverage
{
public:

	virtual long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0) = 0;
};

class IBehavior : public IScan, public IAverage
{
public:

	virtual long GetParam(BehaviorProp bProp, long& pVal) = 0;
	virtual long SetParam(BehaviorProp bProp, long pVal) = 0;
};

class IActionReceiver
{
public:

	virtual long SetAction(ActionType actionType) = 0;
	virtual long SetActionWithParam(ActionType actionType, long paramVal) = 0;
	virtual long GetActionResult(ActionType actionType, long& paramVal) = 0;
	virtual long GetActionResult(ActionType actionType, char* pDataBuffer) = 0;
};

class IAction
{
protected:

	IActionReceiver* _actionReceiver;

public:

	IAction(IActionReceiver* receiver):_actionReceiver(receiver)
	{}

	virtual long SetAction(ActionType actionType) = 0;
	virtual long SetActionWithParam(ActionType actionType, long paramVal) = 0;
	virtual long GetActionResult(ActionType actionType, long& paramVal) = 0;
	virtual long GetActionResult(ActionType actionType, char* pDataBuffer) = 0;
};


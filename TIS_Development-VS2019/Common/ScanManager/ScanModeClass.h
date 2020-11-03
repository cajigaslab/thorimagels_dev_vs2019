#pragma once
#include "Scan.h"

enum BufferProcMode
{
	OFFSET_NONE = 0,
	OFFSET_EVEN,
	OFFSET_ODD
};

///<Concrete LSMActions
class LSMAction : public IAction
{
public:

	LSMAction(IActionReceiver* receiver) : IAction(receiver)
	{}

	long SetAction(ActionType actionType){return _actionReceiver->SetAction(actionType);}
	long SetActionWithParam(ActionType actionType, long paramVal){return _actionReceiver->SetActionWithParam(actionType, paramVal);}
	long GetActionResult(ActionType actionType, long& paramVal){return _actionReceiver->GetActionResult(actionType, paramVal);}
	long GetActionResult(ActionType actionType, char* pDataBuffer){return _actionReceiver->GetActionResult(actionType, pDataBuffer);}

};

///<Scan Mode Class: TwoWay Scan
class TwoWayScan : public IBehavior
{
private:

	ScanMode _scanMode;
	LSMAction _lsmActor;
	AverageMode _avgMode;
	long _active, _lineFactor, _procBufLineCount, _interleaveScan, _switchBehavior;

public:

	TwoWayScan(IActionReceiver* receiver) : _scanMode(ScanMode::TWO_WAY_SCAN), _lsmActor(receiver), _avgMode(AverageMode::NO_AVERAGE), _active(0), _lineFactor(2), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){}
	TwoWayScan(IActionReceiver* receiver, AverageMode aMode) : _scanMode(ScanMode::TWO_WAY_SCAN), _lsmActor(receiver), _avgMode(aMode), _active(0), _lineFactor(2), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){};
	~TwoWayScan();

	long GetParam(BehaviorProp bProp, long& pVal);
	long SetParam(BehaviorProp bProp, long pVal);
	long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0);
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer);
	long StartAcquisition(char * pDataBuffer);
	long StatusAcquisition(long &status);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char * pDataBuffer);
};

///<Scan Mode Class: Forward Scan
class ForwardScan : public IBehavior
{
private:

	ScanMode _scanMode;
	LSMAction _lsmActor;
	AverageMode _avgMode;
	long _active, _lineFactor, _procBufLineCount, _interleaveScan,_switchBehavior;

public:

	ForwardScan(IActionReceiver* receiver) : _scanMode(ScanMode::FORWARD_SCAN), _lsmActor(receiver), _avgMode(AverageMode::NO_AVERAGE), _active(0), _lineFactor(1), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){};
	ForwardScan(IActionReceiver* receiver, AverageMode aMode) : _scanMode(ScanMode::FORWARD_SCAN), _lsmActor(receiver), _avgMode(aMode), _active(0), _lineFactor(1), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){};
	~ForwardScan();

	long GetParam(BehaviorProp bProp, long& pVal);
	long SetParam(BehaviorProp bProp, long pVal);
	long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0);
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer);
	long StartAcquisition(char * pDataBuffer);
	long StatusAcquisition(long &status);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char * pDataBuffer);
};

///<Scan Mode Class: Backward Scan
class BackwardScan : public IBehavior
{
private:

	ScanMode _scanMode;
	LSMAction _lsmActor;
	AverageMode _avgMode;
	long _active,_lineFactor, _procBufLineCount, _interleaveScan, _switchBehavior;

public:

	BackwardScan(IActionReceiver* receiver) : _scanMode(ScanMode::BACKWARD_SCAN), _lsmActor(receiver), _avgMode(AverageMode::NO_AVERAGE), _active(0), _lineFactor(1), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){};
	BackwardScan(IActionReceiver* receiver, AverageMode aMode) : _scanMode(ScanMode::BACKWARD_SCAN), _lsmActor(receiver), _avgMode(aMode), _active(0), _lineFactor(1), _procBufLineCount(0), _interleaveScan(0),_switchBehavior(0){};
	~BackwardScan();

	long GetParam(BehaviorProp bProp, long& pVal);
	long SetParam(BehaviorProp bProp, long pVal);
	long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0);
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer);
	long StartAcquisition(char * pDataBuffer);
	long StatusAcquisition(long &status);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo);
	long PostflightAcquisition(char * pDataBuffer);
};

///<Scan Mode Class: Center Scan
class CenterScan : public IBehavior
{
private:

	ScanMode _scanMode;
	LSMAction _lsmActor;
	AverageMode _avgMode;
	long _active, _lineFactor;

public:

	CenterScan(IActionReceiver* receiver) : _scanMode(ScanMode::CENTER), _lsmActor(receiver), _active(0), _lineFactor(1), _avgMode(AverageMode::NO_AVERAGE){};
	CenterScan(IActionReceiver* receiver, AverageMode aMode) : _scanMode(ScanMode::CENTER), _active(0), _lsmActor(receiver), _lineFactor(1), _avgMode(aMode){};
	~CenterScan();

	long GetParam(BehaviorProp bProp, long& pVal);
	long SetParam(BehaviorProp bProp, long pVal);
	long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0){return 1;}
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer){return 1;}
	long StartAcquisition(char * pDataBuffer){return 1;}
	long StatusAcquisition(long &status);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo){return 1;}
	long PostflightAcquisition(char * pDataBuffer);
};

///<Scan Mode Class: Bleach Scan
class BleachScan : public IBehavior
{
private:

	ScanMode _scanMode;
	LSMAction _lsmActor;
	AverageMode _avgMode;
	long _active;

public:

	BleachScan(IActionReceiver* receiver) : _scanMode(ScanMode::BLEACH_SCAN), _lsmActor(receiver), _avgMode(AverageMode::NO_AVERAGE), _active(0){};
	BleachScan(IActionReceiver* receiver, AverageMode aMode) : _scanMode(ScanMode::BLEACH_SCAN), _lsmActor(receiver), _avgMode(aMode), _active(0){};
	~BleachScan();

	long GetParam(BehaviorProp bProp, long& pVal);
	long SetParam(BehaviorProp bProp, long pVal);
	long ProcessBuffer(long procFrameID, long lineStart = 0, long lineEnd = 0){return 1;}
	long PreflightAcquisition(char * pDataBuffer);
	long SetupAcquisition(char * pDataBuffer);
	long StartAcquisition(char * pDataBuffer);
	long StatusAcquisition(long &status);
	long CopyAcquisition(char * pDataBuffer, void* frameInfo){return 1;}
	long PostflightAcquisition(char * pDataBuffer);
};

///<Behavior Factory for all scanning behaviors
class BehaviorFactory
{
private:

	IBehavior* pBehavior[ScanMode::SCANMODE_LAST];

public:

	BehaviorFactory(IActionReceiver* receiver, AverageMode aMode) {	this->InitializeBehaviors(); this->CreateBehaviors(receiver, aMode); }
	BehaviorFactory(){ this->InitializeBehaviors(); }
	~BehaviorFactory(){ this->ClearBehaviors(); }
	void InitializeBehaviors() { for (int i = 0; i < (int)(ScanMode::SCANMODE_LAST); i++) {	pBehavior[i] = 0; } }
	void ClearBehaviors(){ for (int i = 0; i < (int)(ScanMode::SCANMODE_LAST); i++){ if(pBehavior[i]) { delete(pBehavior[i]); pBehavior[i] = 0; } } }
	ScanMode GetActiveBehavior()
	{ 
		long val = 0;	
		for (int i = 0; i < static_cast<int>(ScanMode::SCANMODE_LAST); i++)
		{	
			pBehavior[i]->GetParam(BehaviorProp::ACTIVE_BEHAVIOR, val);	
			if(1 == val) 
				return static_cast<ScanMode>(i);
		}	
		return ScanMode::SCANMODE_LAST;	
	}
	IBehavior* GetBehaviorInstance(IActionReceiver* receiver, ScanMode sMode) 
	{
		ScanMode curBehavior = GetActiveBehavior();
		if(ScanMode::SCANMODE_LAST != curBehavior)
			pBehavior[(int)curBehavior]->SetParam(BehaviorProp::ACTIVE_BEHAVIOR, 0); 

		pBehavior[(int)sMode]->SetParam(BehaviorProp::ACTIVE_BEHAVIOR, 1); 
		return pBehavior[(int)sMode]; 
	}
	IBehavior* GetBehaviorInstance(IActionReceiver* receiver, ScanMode sMode, AverageMode aMode)
	{
		ScanMode curBehavior = GetActiveBehavior();
		if(ScanMode::SCANMODE_LAST != curBehavior)
			pBehavior[(int)curBehavior]->SetParam(BehaviorProp::ACTIVE_BEHAVIOR, 0); 

		pBehavior[(int)sMode]->SetParam(BehaviorProp::AVERAGE_MODE, aMode);
		pBehavior[(int)sMode]->SetParam(BehaviorProp::ACTIVE_BEHAVIOR, 1);
		return pBehavior[(int)sMode];
	}
	void CreateBehaviors(IActionReceiver* receiver, AverageMode aMode)
	{
		this->ClearBehaviors();

		for (int i = 0; i < static_cast<int>(ScanMode::SCANMODE_LAST); i++)
		{
			switch ((ScanMode)i)
			{
			case ScanMode::TWO_WAY_SCAN:
				pBehavior[i] = new TwoWayScan(receiver, aMode);
				break;
			case ScanMode::FORWARD_SCAN:
				pBehavior[i] = new ForwardScan(receiver, aMode);
				break;
			case ScanMode::BACKWARD_SCAN:
				pBehavior[i] = new BackwardScan(receiver, aMode);
				break;
			case ScanMode::CENTER:
				pBehavior[i] = new CenterScan(receiver, aMode);
				break;
			case ScanMode::BLEACH_SCAN:
				pBehavior[i] = new BleachScan(receiver, aMode);
				break;
			default:
				break;
			}
		}
	}

};

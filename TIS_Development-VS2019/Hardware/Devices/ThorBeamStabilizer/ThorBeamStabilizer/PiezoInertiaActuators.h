#pragma once
#include "Serial.h"

class PiezoInertiaActuators
{
public:
	PiezoInertiaActuators();
	~PiezoInertiaActuators();

	bool Connect(long comPort, long baudRate);
	bool Disconnect();
	bool AbsoluteMove(long channel, long step);	
	bool JogMove(long channel, long step);
	bool StopAll();
	bool GetPiezoPosition(long channel, long &stepPos);
	bool SetPiezoPosition(long channel, long stepPos);
	bool UpdatePiezoPositions();
	bool StartStatusUpdates();
	bool StopStatusUpdates();
	void SetPiezoStepLimit(long val);
	long GetPiezoStepLimit();

	enum
	{
		PIEZO_ACTUATORS_NUM = 4
	};
	
	//**public to be accessed from the thread**//
	static HANDLE _hThread;
	bool _updatePositions;
	//****************************************//

private:	
	bool ChangeChannel(long channel);
	bool SendStartUpdateCommand();
	std::vector<unsigned char> GetBytes(int value);
	CritSect _critSect;
	CSerial _serialPort;
	bool _connectionStablished;	
	std::array<long, PiezoInertiaActuators::PIEZO_ACTUATORS_NUM> _piezoPosition;
	long _piezoStepLimit;
};


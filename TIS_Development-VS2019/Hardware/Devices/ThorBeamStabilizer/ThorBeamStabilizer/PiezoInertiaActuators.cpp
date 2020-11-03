#include "stdafx.h"
#include "PiezoInertiaActuators.h"


PiezoInertiaActuators::PiezoInertiaActuators() :
	_connectionStablished(false),
	_updatePositions(false)
{
	_piezoStepLimit = 0;

	_piezoPosition[0] = 0;
	_piezoPosition[1] = 0;
	_piezoPosition[2] = 0;
	_piezoPosition[3] = 0;
}


PiezoInertiaActuators::~PiezoInertiaActuators()
{
}

HANDLE PiezoInertiaActuators::_hThread = NULL;

/// <summary>
/// Calls the update function
/// </summary>
/// <returns>UINT.</returns>
UINT GetPositionThread( LPVOID pParam )
{
	PiezoInertiaActuators* piezos = (PiezoInertiaActuators*)pParam;
	while(piezos->_updatePositions)
	{
		Sleep(100);
		piezos->UpdatePiezoPositions();		
	}
	CloseHandle(PiezoInertiaActuators::_hThread);
	PiezoInertiaActuators::_hThread = NULL;

	return 0;
}

bool PiezoInertiaActuators::Connect(long comPort, long baudRate)
{
	Lock lock(_critSect);

	//open the comport, keep connection status
	bool ret = _connectionStablished = (TRUE == _serialPort.Open(comPort, baudRate));

	return ret;
}

bool PiezoInertiaActuators::Disconnect()
{
	Lock lock(_critSect);

	_updatePositions = false;
	//close the comport
	bool ret = (TRUE == _serialPort.Close());

	//reset connection status
	_connectionStablished = false;

	return ret;
}

bool PiezoInertiaActuators::AbsoluteMove(long channel, long step)
{
	Lock lock(_critSect);

	//clear the  serial data buffer and read all available to be read for the next status update
	const long CLEAR_LEN = 1000;	
	char clearbuf[CLEAR_LEN];
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);

	long realChannel = static_cast<long>(pow(2,channel));
	ChangeChannel(realChannel);

	std::vector<unsigned char> adjustedChannel = GetBytes(realChannel);
	std::vector<unsigned char> adjustedPosition = GetBytes(step);

	const long LEN = 12;
	const unsigned char commandBytesAbsoluteMove[LEN] = { 0xD4, 0x08, 0x06, 0x00, 0xD0, 0x01, adjustedChannel[0], 0x00, adjustedPosition[0], adjustedPosition[1], adjustedPosition[2], adjustedPosition[3] };

	_serialPort.SendData(commandBytesAbsoluteMove, LEN);

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	//clear the  serial data buffer and read all available to be read for the next status update
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);

	return true;
}

bool PiezoInertiaActuators::JogMove(long channel, long step)
{
	Lock lock(_critSect);
	//clear the  serial data buffer and read all available to be read for the next status update
	const long CLEAR_LEN = 1000;	
	char clearbuf[CLEAR_LEN];
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);

	long realChannel = static_cast<long>(pow(2,channel));
	ChangeChannel(realChannel);

	const long JOG_STEP_RATE = 500; //500 Steps/Sec 
	const long JOG_STEP_ACCN = 1000; //1,000 Steps/Sec/Sec
	std::vector<unsigned char> adjustedChannel = GetBytes(realChannel);
	long steps = std::abs(step);
	std::vector<unsigned char> adjustedSteps = GetBytes(steps);
	std::vector<unsigned char> adjustedJogStepRate = GetBytes(JOG_STEP_RATE);
	std::vector<unsigned char> adjustedJogStepAccn= GetBytes(JOG_STEP_ACCN);
	const long PARAMETERS_LEN = 24;
	const unsigned char commandSetJogParameters[PARAMETERS_LEN] = { 0xC0, 0x08, 0x12, 0x00, 0xD0, 0x01, 0x09, 0x00, adjustedChannel[0], 0x00, 0x02, 0x00,
		adjustedSteps[0], adjustedSteps[1], adjustedSteps[2], adjustedSteps[3], 
		adjustedJogStepRate[0], adjustedJogStepRate[1], adjustedJogStepRate[2], adjustedJogStepRate[3],
		adjustedJogStepAccn[0], adjustedJogStepAccn[1], adjustedJogStepAccn[2], adjustedJogStepAccn[3]};

	_serialPort.SendData(commandSetJogParameters, PARAMETERS_LEN);

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	unsigned char direction;
	if (0 <= step)
	{
		direction = 0x01;

		_piezoPosition[channel] += step;
	}
	else
	{
		direction = 0x02;

		_piezoPosition[channel] += step;
	}

	const long JOG_LEN = 6;
	const unsigned char commandJog[JOG_LEN] = { 0xD9, 0x08, adjustedChannel[0], direction, 0x50, 0x01 };
	_serialPort.SendData(commandJog, JOG_LEN);
	Sleep(50);
	//clear the  serial data buffer and read all available to be read for the next status update
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);
	return true;
}

bool PiezoInertiaActuators::StopAll()
{
	Lock lock(_critSect);

	const long LEN = 6;
	const unsigned char commandBytesStop1[LEN] = { 0xD7, 0x08, 0x01, 0x00, 0x50, 0x01 };
	_serialPort.SendData(commandBytesStop1, LEN);
	Sleep(50);

	const unsigned char commandBytesStop2[LEN] = { 0xD7, 0x08, 0x02, 0x00, 0x50, 0x01 };
	_serialPort.SendData(commandBytesStop2, LEN);
	Sleep(50);

	const unsigned char commandBytesStop3[LEN] = { 0xD7, 0x08, 0x04, 0x00, 0x50, 0x01 };
	_serialPort.SendData(commandBytesStop3, LEN);
	Sleep(50);

	const unsigned char commandBytesStop4[LEN] = { 0xD7, 0x08, 0x08, 0x00, 0x50, 0x01 };
	_serialPort.SendData(commandBytesStop4, LEN);
	Sleep(50);

	return true;
}

bool PiezoInertiaActuators::GetPiezoPosition(long channel, long &stepPos)
{
	stepPos = _piezoPosition[channel];
	return true;
}

bool PiezoInertiaActuators::SetPiezoPosition(long channel, long stepPos)
{
	_piezoPosition[channel] = stepPos;

	return true;
}

/// <summary>
/// Updates the positions
/// </summary>
/// <returns>bool.</returns>
bool PiezoInertiaActuators::UpdatePiezoPositions()
{
	Lock lock(_critSect);

	//commented out the section below. Do not rely on the position coming from the device. This is an open loop system

	//const long HEADER_LEN = 6;
	//unsigned char header[HEADER_LEN];
	//memset(header, 0, sizeof(header));
	//_serialPort.ReadData(header, HEADER_LEN);

	////check if the first 2 bytes of the header == 0x08E1
	//if (225 == header[0] && 8 == header[1])
	//{
	//	const long LEN = 56;
	//	char bytesToRead[LEN];
	//	memset(bytesToRead, 0, sizeof(bytesToRead));

	//	_serialPort.ReadData(bytesToRead, LEN);
	//	if (bytesToRead[42] != 0)
	//	{
	//		//covert to position
	//		_piezoPosition[0] = (bytesToRead[1] | bytesToRead[2] << 8 | bytesToRead[3] << 16 | bytesToRead[4] << 24) / 256;
	//		_piezoPosition[1] = (bytesToRead[15] | bytesToRead[16] << 8 | bytesToRead[17] << 16 | bytesToRead[18] << 24) / 256;
	//		_piezoPosition[2] = (bytesToRead[29] | bytesToRead[30] << 8 | bytesToRead[31] << 16 | bytesToRead[32] << 24) / 256;
	//		_piezoPosition[3] = (bytesToRead[43] | bytesToRead[44] << 8 | bytesToRead[45] << 16 | bytesToRead[46] << 24) / 256;
	//	}
	//}
	//else
	//{
	//	//clear the  serial data buffer and read all available to be read for the next status update
	//	const long LEN = 1000;		
	//	char buf[LEN];
	//	memset(buf, 0, sizeof(buf));
	//	_serialPort.ReadData(buf, LEN);
	//}

	return true;
}

bool PiezoInertiaActuators::SendStartUpdateCommand()
{
	Lock lock(_critSect);

	_updatePositions = true;
	const long LEN = 6;
	const unsigned char commandStartUpdateMSG[LEN] = { 0x11, 0x00, 0x04, 0x01, 0x50, 0x01 };
	_serialPort.SendData(commandStartUpdateMSG, LEN);
	return true;
}

bool PiezoInertiaActuators::StartStatusUpdates()
{

	bool ret = true;
	//if the connection was stablished and the update positions thread is not already running
	//Start the status updates and start the thread to check on these status updates
	//This class was design to only have one place reading the serial port (UpdatePiezoPositions())
	if (!_updatePositions && NULL == _hThread)
	{
		SendStartUpdateCommand();
		DWORD dwThreadId;
		_hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) GetPositionThread, this, 0, &dwThreadId );
		if (_hThread == NULL)
		{
			ret = false;
		}
	}
	return ret;
}

bool PiezoInertiaActuators::StopStatusUpdates()
{
	Lock lock(_critSect);
	const long LEN = 6;
	const unsigned char commandStartUpdateMSG[LEN] = { 0x12, 0x00, 0x04, 0x01, 0x50, 0x01 };
	_serialPort.SendData(commandStartUpdateMSG, LEN);
	_updatePositions = false;
	Sleep(110);
	return true;
}



bool PiezoInertiaActuators::ChangeChannel(long realChannel)
{	
	Lock lock(_critSect);

	//clear the  serial data buffer and read all available to be read for the next status update
	const long CLEAR_LEN = 1000;
	char clearbuf[CLEAR_LEN];
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);


	std::vector<unsigned char> adjustedChannel = GetBytes(realChannel);

	const long LEN = 6;
	const unsigned char commandBytesChangeChannel[LEN] = { 0x10, 0x02, adjustedChannel[0], 0x01, 0x50, 0x01 };

	_serialPort.SendData(commandBytesChangeChannel, LEN);

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	//clear the  serial data buffer and read all available to be read for the next status update
	memset(clearbuf, 0, sizeof(clearbuf));
	_serialPort.ReadData(clearbuf, CLEAR_LEN);

	return true;
}


/// <summary>
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> PiezoInertiaActuators::GetBytes(int value)
{
	std::vector<unsigned char> bytes(sizeof(int));
	std::memcpy(&bytes[0], &value, sizeof(int));
	return bytes;
}


void PiezoInertiaActuators::SetPiezoStepLimit(long val)
{
	_piezoStepLimit = val;
}

long PiezoInertiaActuators::GetPiezoStepLimit()
{
	return _piezoStepLimit;
}
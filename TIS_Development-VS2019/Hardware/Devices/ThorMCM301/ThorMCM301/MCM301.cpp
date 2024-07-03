#include <memory>
#include <iostream>
#include <string>
#include <regex>
#include "Strsafe.h"
#include "include/apt_cmd_library.h"
#include "include/apt_cmd_library_motor.h"
#include "include/uart_library.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include "MCM301.h"
#include "MCM301XML.h"

string THORLABS_VID = "1313";
string THORLABS_MCM_PID = "2016";

HANDLE MCM301::_hGetStatusThread = NULL; // Initialize status thread
int MCM301::_threadDeviceHandler = -1;
long MCM301::_setSlots = 0;
APT* MCM301::_apt = NULL;
Mcm301Params* MCM301::_mcm301Params;
unsigned long MCM301::_responseWaitTime = RESPONSE_WAIT_TIME;
long MCM301::_statusThreadStopped = FALSE;
long MCM301::_stopStatusThread = FALSE;
long MCM301::_settingsFileChanged = FALSE;
CritSect MCM301::_critSect;

MCM301::MCM301()
{
	_deviceHandler = -1;
	_foundUsbSer = FALSE;
	_baudRate = 115200; // 115200;// needs to be set for tablet
	_numberOfSetSlots = 0;
	_mcm301Params = (Mcm301Params*)malloc(sizeof(Mcm301Params));
	_mcm301Params->xPositionCurrent = 0;
	_mcm301Params->yPositionCurrent = 0;
	_mcm301Params->zPositionCurrent = 0;
	_mcm301Params->rPositionCurrent = 0;
	_mcm301Params->condenserPositionCurrent = 0;
	_mcm301Params->xThreshold = 0.4;
	_mcm301Params->yThreshold = 0.4;
	_mcm301Params->zThreshold = 0.4;
	_mcm301Params->rThreshold = 0.4;
	_mcm301Params->condenserThreshold = 0.4;
	_mcm301Params->xInvert = false;
	_mcm301Params->yInvert = false;
	_mcm301Params->zInvert = false;
	_mcm301Params->rInvert = false;
	_mcm301Params->condenserInvert = false;

	_mcm301Params->xPidEnable = false;
	_mcm301Params->yPidEnable = false;
	_mcm301Params->zPidEnable = false;
	_mcm301Params->rPidEnable = false;
	_mcm301Params->condenserPidEnable = false;

	_mcm301Params->x_slot_id = 0;
	_mcm301Params->y_slot_id = 0;
	_mcm301Params->z_slot_id = 0;
	_mcm301Params->r_slot_id = 0;
	_mcm301Params->ze_slot_id = 0;

	_mcm301Params->xPidKickoutEnable = false;
	_mcm301Params->yPidKickoutEnable = false;
	_mcm301Params->zPidKickoutEnable = false;
	_mcm301Params->rPidKickoutEnable = false;
	_mcm301Params->condenserPidKickoutEnable = false;

	_mcm301Params->x_ccw_moving = false;
	_mcm301Params->x_cw_moving = false;
	_mcm301Params->y_ccw_moving = false;
	_mcm301Params->y_cw_moving = false;
	_mcm301Params->z_ccw_moving = false;
	_mcm301Params->z_cw_moving = false;
	_mcm301Params->r_ccw_moving = false;
	_mcm301Params->r_cw_moving = false;
	_mcm301Params->ze_ccw_moving = false;
	_mcm301Params->ze_cw_moving = false;
	_mcm301Params->condenser_ccw_moving = false;
	_mcm301Params->condenser_cw_moving = false;

	_mcm301Params->xConfigured = FALSE;
	_mcm301Params->yConfigured = FALSE;
	_mcm301Params->zConfigured = FALSE;
	_mcm301Params->rConfigured = FALSE;
	_mcm301Params->zeConfigured = FALSE;
	_mcm301Params->condenserConfigured = FALSE;

	_board_type = 0;
	_cardType[0] = CardTypes::NO_CARD_IN_SLOT;
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		_cardType[i] = CardTypes::NO_CARD_IN_SLOT;
		_mcm301Params->slotName[i][0] = '\0';
	}
	_cpldRev = NULL;
	_firmwareRev[0] = NULL;
	_scopeType = ScopeType::UPRIGHT;
	firmwareVersion = NULL;
	firmwareVersionLength = 0;
	serialNumber = NULL;
	serialNumberLength = 0;
}

MCM301::~MCM301()
{
	/*delete firmwareVersion;
	delete serialNumber;
	DeleteCriticalSection(&CriticalSection);*/
}

void MCM301::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}


/************************************************************************************************
* @fn	HANDLE MCM301::GetStatusThread(DWORD &threadID)
*
* @brief	Create a Thread to request and read the status of each board.
* @param 	threadID	  	GetStatus Thread ID.
* @return	Thread Handle.
**************************************************************************************************/
HANDLE MCM301::GetStatusThread(DWORD& threadID)
{
	_threadDeviceHandler = _deviceHandler;
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) & (MCM301::GetStatusAllBoards), (void*)this, 0, &threadID);
	SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
	return handle;
}

// Request the status of every board that was set up. First Sends a request command, waits a certain time, then reads the response.
// The response is parsed through APT.cpp and saved to the data structure mcm301Params. The rest of the code requests the current values (position, 
// status) from mcm301Params.
//:TODO: Check the type of board to know what command to send for the status. Currently it is hard coded to request status of stepper motors
void MCM301::GetStatusAllBoards(LPVOID instance)
{
	long slotNumber = CARD_ID_START_ADDRESS;
	unsigned long errorCounter = 0;
	const long MAX_NUM_ERRORS = 10000; //Do not overfill the logger with errors if device is disconnected
	do
	{
		//Request the status of every board that was set up in ThorMCM301Settings.xml. Wait RESPONSE_WAIT_TIME for the response 
		if (_setSlots & (1 << (slotNumber - CARD_ID_START_ADDRESS)))
		{
			int result = -1;
			if (CardTypes::High_Current_Stepper_Card == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::High_Current_Stepper_Card_HD == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::ST_Invert_Stepper_BISS_type == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::ST_Invert_Stepper_SSI_type == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::MCM_Stepper_Internal_BISS_L6470 == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::MCM_Stepper_Internal_SSI_L6470 == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::MCM_Stepper_LC_HD_DB15 == _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS])
			{
				char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_REQ_STATUSUPDATE & 0xFF), (UCHAR)((MGMSG_MCM_REQ_STATUSUPDATE & 0xFF00) >> 8), 0x00, 0x00, static_cast<char>(slotNumber), HOST_ID };
				result = fnUART_LIBRARY_write(_threadDeviceHandler, bytesToSend, 6);
				if (result >= 0)
				{
					Sleep(_responseWaitTime);

					char* resultArray = new char[BUFFER_LENGTH];
					result = fnUART_LIBRARY_read(_threadDeviceHandler, resultArray, BUFFER_LENGTH);
					if (result >= 0)
					{
						_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->GetStatusAllBoards: Error while reading MGMSG_MCM_REQ_STATUSUPDATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
					delete[] resultArray;
				}
				else
				{
					wchar_t errMsg[MSG_SIZE];
					StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->GetStatusAllBoards: Error while writing MGMSG_MCM_REQ_STATUSUPDATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm301Params->cardType[slotNumber - CARD_ID_START_ADDRESS]);
					LogMessage(errMsg, ERROR_EVENT);
					errorCounter++;
				}
			}
		}
		if (CARD_ID_START_ADDRESS + TOTAL_CARD_SLOTS - 1 == slotNumber) // When reaching the max slot number reset slotNumber to the first slot
		{
			slotNumber = CARD_ID_START_ADDRESS;
		}
		else
		{
			slotNumber++;
		}
		//If the thread is about to be stopped, it needs to make sure it read the 
		// last status request. Otherwise we can have a pending request next time we read status.
		if (TRUE == _stopStatusThread)
		{
			_statusThreadStopped = TRUE;
			return;
		}
		//If the number of errors printed by this thread reaches 10,000 (5 seconds), stop the thread. Otherwise it will easily saturate the log file.
		if (MAX_NUM_ERRORS < errorCounter)
		{
			return;
		}
	} while (true);
}

// Request and Read the board info (firmware version, cpld version, card types) from the device
long MCM301::GetHardwareInfo()
{
	char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_HW_REQ_INFO & 0xFF), (UCHAR)((MGMSG_MCM_HW_REQ_INFO & 0xFF00) >> 8), 0x00, 0x00, MOTHERBOARD_ID, HOST_ID };
	int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->GetHardwareInfo: Error while sending MGMSG_MCM_HW_REQ_INFO command.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	Sleep(_responseWaitTime * 3);

	char* resultArray = new char[BUFFER_LENGTH];
	result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);

	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->GetHardwareInfo: Error while reading MGMSG_MCM_HW_REQ_INFO request.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
	delete[] resultArray;
	return TRUE;
}

// Request and Read the parameters (mm_to_encoder counts, min, max) from each card 
long MCM301::RequestStageParameters()
{
	for (char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_STAGEPARAMS & 0xFF), static_cast<char>((MGMSG_MCM_REQ_STAGEPARAMS & 0xFF00) >> 8), 0x00, 0x00, CARD_ID_START_ADDRESS + i, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestStageParameters: Error while sending MGMSG_MCM_REQ_STAGEPARAMS command.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime * 3);

		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestStageParameters: Error while reading MGMSG_MCM_REQ_STAGEPARAMS request.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
		delete[] resultArray;
	}
	return TRUE;
}

// Request and read the slot title from each card
long MCM301::RequestSlotTitle()
{
	for (char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_SLOT_TITLE & 0xFF), static_cast<char>((MGMSG_MCM_REQ_SLOT_TITLE & 0xFF00) >> 8), i, 0x00, MOTHERBOARD_ID, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestSlotTitle: Error while sending MGMSG_MCM_REQ_SLOT_TITLE command.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime * 3);

		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestSlotTitle: Error while reading MGMSG_MCM_REQ_SLOT_TITLE request.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
		delete[] resultArray;
	}
	return TRUE;
}

// Request and read the plug and play status from each card
long MCM301::RequestPnpStatus()
{
	for (char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char lowerID = static_cast<char>(MGMSG_MCM_REQ_PNPSTATUS & 0xFF);
		char upperID = static_cast<char>((MGMSG_MCM_REQ_PNPSTATUS & 0xFF00) >> 8);
		char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_PNPSTATUS & 0xFF), static_cast<char>((MGMSG_MCM_REQ_PNPSTATUS & 0xFF00) >> 8), i, 0x00, MOTHERBOARD_ID, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestPnpStatus: Error while sending MGMSG_MCM_REQ_PNPSTATUS command.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime * 3);

		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestPnpStatus: Error while reading MGMSG_MCM_REQ_PNPSTATUS request.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
		delete[] resultArray;
	}
	return TRUE;
}

long MCM301::RequestDeviceSerialNo()
{
	for (char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char bytesToSend[6] = { static_cast<char>(MGMSG_REQ_DEVICE & 0xFF), static_cast<char>((MGMSG_REQ_DEVICE & 0xFF00) >> 8), i, 0x00, MOTHERBOARD_ID, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestSlotTitle: Error while sending MGMSG_MCM_REQ_DEVICE_BOARD command.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime * 3);
		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->RequestSlotTitle: Error while reading MGMSG_MCM_GET_DEVICE_BOARD request.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}

		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm301Params);
		delete[] resultArray;
	}
	return TRUE;
}

long MCM301::InitializeParams()
{
	DWORD ThreadForStatus;
	// Need to notify the status thread it is about to be stopped. Make sure it read all
	//pending requests
	if (NULL != _hGetStatusThread)
	{
		clock_t nextUpdateLoop = clock();
		_stopStatusThread = TRUE;
		while (FALSE == _statusThreadStopped && static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < (_numberOfSetSlots * _responseWaitTime))
		{
			//wait until the status thread has stopped
		}
		_statusThreadStopped = FALSE;
		_stopStatusThread = FALSE;
	}

	SAFE_DELETE_HANDLE(_hGetStatusThread);

	if (FALSE == GetHardwareInfo())
	{
		return FALSE;
	}

	if (FALSE == RequestStageParameters())
	{
		return FALSE;
	}

	if (FALSE == RequestSlotTitle())
	{
		return FALSE;
	}

	if (FALSE == RequestPnpStatus())
	{
		return FALSE;
	}

	if (FALSE == RequestDeviceSerialNo())
	{
		return FALSE;
	}
	/*Board Type is a single bit which is 1 for static boards. Needs to be masked*/
	for (int cardIndex = 0; cardIndex < TOTAL_CARD_SLOTS; cardIndex++) {
		_mcm301Params->cardType[cardIndex] = _mcm301Params->cardType[cardIndex] & 31;
	}
	try
	{
		auto_ptr<MCM301XML> pSetup(new MCM301XML());
		//_mcm301Params->x_slot_id = CARD_ID_START_ADDRESS + 0 + 4;
		//_mcm301Params->y_slot_id = CARD_ID_START_ADDRESS + 1 + 4;
		//_mcm301Params->z_slot_id = CARD_ID_START_ADDRESS + 2 + 4;
		if (!pSetup->SaveSerialNoToSettingsFile(_mcm301Params))
			return FALSE;

		if (FALSE == pSetup->VerifySlotCards(_mcm301Params, _scopeType))
		{
			return FALSE;
		}
	}
	catch (...)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorMCM301: Initialize Params -> Unable to locate MCM301 xml file");
		LogMessage(errMsg, ERROR_EVENT);
	}

	//Check if the slot layout in ThorMCM301Settings.xml is configured correctly. If it isn't display an error message and do not connect to the device
	// NOTE: It only checks the type of cards in the board (stepper, slider or Invert Stepper). It doesn't check what motor is controlling, which one is X, Y, Z or R
	// It is possible to only check if the type is correct. For this separate the first && and make the right part of the condition it's own if statement
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		if ((CardTypes::High_Current_Stepper_Card == _mcm301Params->cardType[i] ||
			CardTypes::High_Current_Stepper_Card_HD == _mcm301Params->cardType[i]) &&
			(_mcm301Params->x_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm301Params->y_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm301Params->z_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm301Params->r_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm301Params->ze_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm301Params->condenser_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type HC Stepper Card mismatch. There is a card of type HC Stepper that is not accounted for. Please check ThorMCM301Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: X, Y, Z, R, ZElevator, Condenser";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Card Type Mismatch", MB_OK);
		}
		else if ((CardTypes::ST_Invert_Stepper_BISS_type == _mcm301Params->cardType[i] ||
			CardTypes::ST_Invert_Stepper_SSI_type == _mcm301Params->cardType[i] ||
			CardTypes::MCM_Stepper_Internal_BISS_L6470 == _mcm301Params->cardType[i] ||
			CardTypes::MCM_Stepper_Internal_SSI_L6470 == _mcm301Params->cardType[i]) &&
			(_mcm301Params->z_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type Inverted Stepper mismatch. There is a card of type Inverted Stepper that is not accounted for. Please check ThorMCM301Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: Z, EpiTurret, InvertedLP, AFSwitch";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Card Type Mismatch", MB_OK);
		}
	}

	//Start the status request thread once the hardware info and stage parameters have been queried 
	SAFE_DELETE_HANDLE(_hGetStatusThread);
	_hGetStatusThread = GetStatusThread(ThreadForStatus);

	/* Do not write the Pid control if something fails in the code, this will overwrite the stage configuration. MGMSG_MCM_SET_STAGEPARAMS writes on the flash memory
	Panchy says we shouldn't do this every time, it should only be done through his UI. But he is going to add space in the RAM where we can set the flags safely
	if(IsSlotIdValid(_mcm301Params->x_slot_id))
	{
		ConfigPid(_mcm301Params->x_slot_id, _mcm301Params->xParams, _mcm301Params->xPidEnable);
		ConfigPidKickout(_mcm301Params->x_slot_id, _mcm301Params->xParams, _mcm301Params->xPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm301Params->y_slot_id))
	{
		ConfigPid(_mcm301Params->y_slot_id, _mcm301Params->yParams, _mcm301Params->yPidEnable);
		ConfigPidKickout(_mcm301Params->y_slot_id, _mcm301Params->yParams, _mcm301Params->yPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm301Params->z_slot_id))
	{
		ConfigPid(_mcm301Params->z_slot_id, _mcm301Params->zParams, _mcm301Params->zPidEnable);
		ConfigPidKickout(_mcm301Params->z_slot_id, _mcm301Params->zParams, _mcm301Params->zPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm301Params->r_slot_id))
	{
		ConfigPid(_mcm301Params->r_slot_id, _mcm301Params->rParams, _mcm301Params->rPidEnable);
		ConfigPidKickout(_mcm301Params->r_slot_id, _mcm301Params->rParams, _mcm301Params->rPidKickoutEnable);
	}
	if (IsSlotIdValid(_mcm301Params->condenser_slot_id))
	{
		ConfigPid(_mcm301Params->condenser_slot_id, _mcm301Params->condenserParams, _mcm301Params->condenserPidEnable);
		ConfigPidKickout(_mcm301Params->condenser_slot_id, _mcm301Params->condenserParams, _mcm301Params->condenserPidKickoutEnable);
	}
	*/

	// Panchy mentioned setting the jog parameters a lot might take an effect on the EPROM. 
	// He doesn't recommend doing this. Will comment it for now.
	/*if(IsSlotIdValid(_mcm301Params->x_slot_id))
	SaveJogSize(_mcm301Params->x_slot_id, _mcm301Params->xJogSize);
	if(IsSlotIdValid(_mcm301Params->y_slot_id))
	SaveJogSize(_mcm301Params->y_slot_id, _mcm301Params->yJogSize);
	if(IsSlotIdValid(_mcm301Params->z_slot_id))
	SaveJogSize(_mcm301Params->z_slot_id, _mcm301Params->zJogSize);
	if(IsSlotIdValid(_mcm301Params->r_slot_id))
	SaveJogSize(_mcm301Params->r_slot_id, _mcm301Params->rJogSize);
	if (IsSlotIdValid(_mcm301Params->condenser_slot_id))
		SaveJogSize(_mcm301Params->condenser_slot_id, _mcm301Params->condenserJogSize); */

	return TRUE;
}

long MCM301::ConfigPid(UCHAR slotId, byte* params, bool pidEn)
{
	Lock lock(_critSect);
	char bytesToSend[6 + 96];

	byte head[6] = { (UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF),
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8),
		96,
		0x00,
		(byte)(slotId | 0x80),
		HOST_ID };
	memcpy(bytesToSend, head, sizeof(head));
	if (pidEn)
		params[81] |= (1 << 4);
	else
		params[81] &= (~(1 << 4));

	memcpy(bytesToSend + sizeof(head), params, 96);

	fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));

	return 0;
}

long MCM301::ConfigPidKickout(UCHAR slotId, byte* params, bool pidEn)
{
	Lock lock(_critSect);
	char bytesToSend[6 + 96];

	byte head[6] = { (UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF),
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8),
		96,
		0x00,
		(byte)(slotId | 0x80),
		HOST_ID };
	memcpy(bytesToSend, head, sizeof(head));
	if (pidEn)
		params[81] |= (1 << 5);
	else
		params[81] &= (~(1 << 5));

	memcpy(bytesToSend + sizeof(head), params, 96);

	fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));

	return 0;
}

long MCM301::UpdateDeviceInfo()
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, MOTHERBOARD_ID, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_HW_Req_Info(_deviceHandler, message);
	int ret = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (ret < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->UpdateDeviceInfo: Error while updating device info.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	Sleep(50);
	char* result = new char[BUFFER_LENGTH];

	ret = fnUART_LIBRARY_read(_deviceHandler, result, BUFFER_LENGTH);
	if (ret < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->UpdateDeviceInfo: Error while reading device info.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	wchar_t tFirmwareVersion[] = { (wchar_t)'0' + result[22],'.',(wchar_t)'0' + result[21],'.',(wchar_t)'0' + result[20], '\0' };
	SAFE_DELETE_ARRAY(firmwareVersion);
	firmwareVersionLength = 12;
	firmwareVersion = new wchar_t[firmwareVersionLength];
	wmemcpy_s(firmwareVersion, firmwareVersionLength, tFirmwareVersion, firmwareVersionLength);

	wchar_t tSerialNumer[] = { (wchar_t)result[6],(wchar_t)result[7] ,(wchar_t)result[8] ,(wchar_t)result[9], '\0' };
	SAFE_DELETE_ARRAY(serialNumber);
	serialNumberLength = 10;
	serialNumber = new wchar_t[serialNumberLength];
	wmemcpy_s(serialNumber, serialNumberLength, tSerialNumer, serialNumberLength);

	delete[] result;
	return TRUE;
}

long MCM301::FirmwareVersion(wchar_t* version, int size)
{
	if (firmwareVersion == NULL)
	{
		int ret = UpdateDeviceInfo();
		if (ret == FALSE)
			return FALSE;
	}
	memcpy_s(version, size, firmwareVersion, firmwareVersionLength);
	return TRUE;
}

long MCM301::SerialNumber(wchar_t* number, int size)
{
	if (serialNumber == NULL)
	{
		int ret = UpdateDeviceInfo();
		if (ret == FALSE)
			return FALSE;
	}
	memcpy_s(number, size, serialNumber, serialNumberLength);
	return TRUE;
}

long MCM301::Home(unsigned char slotId)
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_MOT_MoveHome(_deviceHandler, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->Home: Error while sending command fnAPT_DLL_MOT_MoveHome. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::Zero(unsigned char slotId)
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_Set_Enccounter(_deviceHandler, 0, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->Zero: Error while sending command fnAPT_DLL_MOT_Set_Enccounter. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::Stop(unsigned char slotId)
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_MOT_MoveStop(_deviceHandler, 0, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->Stop: Error while sending command fnAPT_DLL_MOT_MoveStop. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::MoveBy(UCHAR slotId, double distance)
{
	Lock lock(_critSect);
	const int SIZE_OF_INT32 = sizeof(INT32);
	INT32 distanceToMove = static_cast<INT32>(distance);
	byte data[SIZE_OF_INT32];
	USHORT cmd = (USHORT)MGMSG_MCM_MOT_MOVE_BY;

	memcpy(data, &distanceToMove, SIZE_OF_INT32);
	byte bytesToSend[12] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
		0x06, 0x00, (byte) (slotId | 0x80), HOST_ID,
		(byte)(slotId & (0x0f - 1)), 0x00,         // Chan Ident
		data[0], data[1], data[2], data[3],    // Move By step size in encoder counts
	};
	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->MoveBy: Error while sending command MGMSG_MCM_MOT_MOVE_BY. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::MoveTo(UCHAR slotId, double distance)
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_MoveAbsolute(_deviceHandler, distance, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->MoveTo: Error while sending command fnAPT_DLL_MOT_MoveAbsolute. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::SaveJogSize(UCHAR slotId, double size)
{
	Lock lock(_critSect);
	byte data[4];
	double val = (size * 1e3) / _mcm301Params->slot_nm_per_count[slotId - CARD_ID_START_ADDRESS];
	int s = (int)(val + 0.5);
	memcpy(data, &s, 4);
	USHORT cmd = (USHORT)MGMSG_MOT_SET_JOGPARAMS;

	byte bytesToSend[28] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
		22, 0x00, slotId | 0x80, HOST_ID,
		(byte)(slotId & (0x0f - 1)), 0,         // Chan Ident
		2, 0,                    // Jog Mode - single step jogging
		data[0], data[1], data[2], data[3],    // Jog Step Size in encoder counts
		0x00, 0x00, 0x00, 0x00,    // Jog Min Velocity (not used)
		0x00, 0x00, 0x00, 0x00,    // Jog Acceleration (not used)
		0x00, 0x00, 0x00, 0x00,    // Jog Max Velocity (not used)
		0x00, 0x00
	};
	long ret = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (ret < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->SaveJogSize: Error while sending command MGMSG_MOT_SET_JOGPARAMS. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return ret;
}

long MCM301::Jog(unsigned char slotId, unsigned char direction)
{
	Lock lock(_critSect);
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_MoveJog(_deviceHandler, direction, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->Jog: Error while sending command fnAPT_DLL_MOT_MoveJog. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM301::MoveToStoredPos(int pos, UCHAR slotId)
{
	Lock lock(_critSect);
	char bytesToSend[6] = { static_cast<char>(MGMSG_SET_GOTO_STORE_POSITION & 0xFF),
		static_cast<char>((MGMSG_SET_GOTO_STORE_POSITION & 0xFF00) >> 8),
		static_cast<char>(slotId - CARD_ID_START_ADDRESS),
		static_cast<char>(pos),
		static_cast<char>(slotId),
		HOST_ID };;
	long ret = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));
	if (ret < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301->MoveToStoredPos: Error while sending command MGMSG_SET_GOTO_STORE_POSITION. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return ret;
}

// -------------------------------Start of stage specific functions----------------------------------------------------------
long MCM301::HomeX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Home X");
	long result = Home(_mcm301Params->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Home X");
	return result;
}

long MCM301::HomeY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Home Y");
	long result = Home(_mcm301Params->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Home Y");
	return result;
}

long MCM301::HomeZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Home Z");
	long result = Home(_mcm301Params->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Home Z");
	return result;
}

long MCM301::HomeR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Home R");
	long result = Home(_mcm301Params->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Home R");
	return result;
}

long MCM301::HomeCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Home Condenser");
	long result = Home(_mcm301Params->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Home Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM301::Home()
//{
//	long resultX = HomeX();
//	long resultY = HomeY();
//	long resultZ = HomeZ();
//	long resultR = HomeR();
//	long resultCondenser = HomeCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM301::ZeroX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Zero X");
	long result = Zero(_mcm301Params->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Zero X");
	return result;
}

long MCM301::ZeroY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Zero Y");
	long result = Zero(_mcm301Params->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Zero Y");
	return result;
}

long MCM301::ZeroZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Zero Z");
	long result = Zero(_mcm301Params->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Zero Z");
	return result;
}

long MCM301::ZeroR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Zero R");
	long result = Zero(_mcm301Params->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Zero R");
	return result;
}

long MCM301::ZeroCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Zero Condenser");
	long result = Zero(_mcm301Params->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Zero Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM301::Zero()
//{
//	long resultX = ZeroX();
//	long resultY = ZeroY();
//	long resultZ = ZeroZ();
//	long resultR = ZeroR();
//	long resultCondenser = ZeroCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM301::StopX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Stop X");
	long result = Stop(_mcm301Params->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Stop X");
	return result;
}

long MCM301::StopY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Stop Y");
	long result = Stop(_mcm301Params->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Stop Y");
	return result;
}

long MCM301::StopZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Stop Z");
	long result = Stop(_mcm301Params->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Stop Z");
	return result;
}

long MCM301::StopR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Stop R");
	long result = Stop(_mcm301Params->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Stop R");
	return result;
}

long MCM301::StopCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 Start -> Stop Condenser");
	long result = Stop(_mcm301Params->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM301 End -> Stop Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM301::Stop()
//{
//	long resultX = StopX();
//	long resultY = StopY();
//	long resultZ = StopZ();
//	long resultR = StopR();
//	long resultCondenser = StopCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM301::MoveXBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move X By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->x_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: X Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm301Params->x_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move X By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveXTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move X To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->x_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: X Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm301Params->x_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move X To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveYBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Y By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->y_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:  Y Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm301Params->y_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Y By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveYTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Y To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->y_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:  Y Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm301Params->y_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Y To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveZBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Z By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->z_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:   Z Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm301Params->z_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Z By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveZTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Z To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->z_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:   Z Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm301Params->z_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Z To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveRBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move R By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->r_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:    R Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm301Params->r_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move R By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveRTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move R To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->r_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:    R Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT);	//Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm301Params->r_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move R To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveCondenserBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Condenser By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->condenser_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:   Condenser Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm301Params->condenser_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Condenser By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::MoveCondenserTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM301 %s -> Move Condenser To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm301Params->slot_nm_per_count[_mcm301Params->condenser_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301:   Condenser Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm301Params->condenser_slot_id, distance);

	wsprintf(logText, L"MCM301 %s -> Move Condenser To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM301::XJogCW()
{
	return Jog(_mcm301Params->x_slot_id, 0);
}

long MCM301::XJogCCW()
{
	return Jog(_mcm301Params->x_slot_id, 1);
}

long MCM301::YJogCW()
{
	return Jog(_mcm301Params->y_slot_id, 0);
}

long MCM301::YJogCCW()
{
	return Jog(_mcm301Params->y_slot_id, 1);
}

long MCM301::ZJogCW()
{
	return Jog(_mcm301Params->z_slot_id, 0);
}

long MCM301::ZJogCCW()
{
	return Jog(_mcm301Params->z_slot_id, 1);
}

long MCM301::RJogCW()
{
	return Jog(_mcm301Params->r_slot_id, 0);
}

long MCM301::RJogCCW()
{
	return Jog(_mcm301Params->r_slot_id, 1);
}

long MCM301::CondenserJogCW()
{
	return Jog(_mcm301Params->condenser_slot_id, 0);
}

long MCM301::CondenserJogCCW()
{
	return Jog(_mcm301Params->condenser_slot_id, 1);
}

long MCM301::GetXPos(double& value)
{
	value = _mcm301Params->xPositionCurrent;
	return TRUE;
}

long MCM301::GetYPos(double& value)
{
	value = _mcm301Params->yPositionCurrent;
	return TRUE;
}

long MCM301::GetZPos(double& value)
{
	value = _mcm301Params->zPositionCurrent;
	return TRUE;
}

long MCM301::GetRPos(double& value)
{
	value = _mcm301Params->rPositionCurrent;
	return TRUE;
}

long MCM301::GetCondenserPos(double& value)
{
	value = _mcm301Params->condenserPositionCurrent;
	return TRUE;
}

long MCM301::GetZElevatorPos(double& value)
{
	value = _mcm301Params->zePositionCurrent;
	return TRUE;
}

bool MCM301::IsXmoving()
{
	return (_mcm301Params->x_ccw_moving || _mcm301Params->x_cw_moving);
}

bool MCM301::IsYmoving()
{
	return (_mcm301Params->y_ccw_moving || _mcm301Params->y_cw_moving);
}

bool MCM301::IsZmoving()
{
	return (_mcm301Params->z_ccw_moving || _mcm301Params->z_cw_moving);
}

bool MCM301::IsRmoving()
{
	return (_mcm301Params->r_ccw_moving || _mcm301Params->r_cw_moving);
}

bool MCM301::IsCondenserMoving()
{
	return (_mcm301Params->condenser_ccw_moving || _mcm301Params->condenser_cw_moving);
}

bool MCM301::IsZEmoving()
{
	return (_mcm301Params->ze_ccw_moving || _mcm301Params->ze_cw_moving);
}

// -------------------------------End of stage specific functions----------------------------------------------------------

long MCM301::StatusPosition(long& status)
{
	Lock lock(_critSect);
	long ret = true;
	clock_t nextUpdateLoop = clock();
	//The status of the board updates every RESPONSE_WAIT_TIME * number of setup boards
	while (static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < (_numberOfSetSlots * _responseWaitTime))
	{
		status = IDevice::STATUS_READY;
		if (IsXmoving() || IsYmoving() || IsZmoving() || IsRmoving() || IsCondenserMoving())
		{
			status = IDevice::STATUS_BUSY;
		}
	}

	return TRUE;
}

long MCM301::Close()
{
	Lock lock(_critSect);

	// Need to notify the status thread it is about to be stopped. Make sure it read all
	//pending requests
	if (NULL != _hGetStatusThread)
	{
		clock_t nextUpdateLoop = clock();
		_stopStatusThread = TRUE;
		while (FALSE == _statusThreadStopped && static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < (_numberOfSetSlots * _responseWaitTime))
		{
			//wait until the status thread has stopped
		}
		_statusThreadStopped = FALSE;
		_stopStatusThread = FALSE;
	}

	SAFE_DELETE_HANDLE(_hGetStatusThread);

	int ret = fnUART_LIBRARY_close(_deviceHandler);

	Sleep(200);	//For Refresh Hardware. Need to wait until the com port is completely closed.
	return (ret == 0) ? TRUE : FALSE;
}

bool MCM301::IsConnected()
{
	return (_deviceHandler >= 0) ? true : false;
}

long MCM301::FindAllDevs(long& devCount)
{
	Lock lock(_critSect);
	long ret = devCount = _foundUsbSer = FALSE, retValue = FALSE;
	try
	{
		auto_ptr<MCM301XML> pSetup(new MCM301XML());
		retValue = pSetup->ReadSettingsFile(_mcm301Params, _scopeType, _portId, _baudRate, _setSlots, _numberOfSetSlots);
	}
	catch (...)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorMCM301: FindAllDevs -> Unable to locate MCM301 xml file");
		LogMessage(errMsg, ERROR_EVENT);
	}
	if (TRUE == retValue)
	{
		//Search the Registry for any connected devices associated with this PID and VID.
		_snList = SerialNumbers(THORLABS_VID, THORLABS_MCM_PID);

		if (0 < _snList.size())
		{
			_foundUsbSer = TRUE;
			devCount = (int)_snList.size();
			ret = TRUE;
		}
		else
		{
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM301: Unable to find any serial device connected in the registry associated with this PID and VID");
		}
	}
	return ret;
}

vector<string> MCM301::SerialNumbers(string VID, string PID)
{
	wchar_t data[BUFFER_LENGTH];
	HKEY hk;
	DWORD count = 0;
	DWORD sz = sizeof(DWORD);
	vector<string> serialNumbers;
	string dataString;
	string format = "VID_" + VID + ".*&.*PID_" + PID + "\\\\";
	regex reg(format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\servies\usbser as hk
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\usbser", 0, KEY_READ, &hk))
	{
		// No usbSer device is connected, return empty list (size=0).
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM301: Could not find registry key SYSTEM\\CurrentControlSet\\services\\usbser");
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected usbser devices 
	if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No usbSer device is connected, return empty list (size=0).
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM301: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\usbser\\Enum doesn't exist");
		return serialNumbers;
	}
	// Iterate through the parameters, one for each connected device
	for (DWORD i = 0; i < count; i++)
	{
		DWORD cbData = BUFFER_LENGTH;
		wchar_t usbConnectedIndex[BUFFER_LENGTH];
		// Use i as the parameter name, the parameter name for each device is their index number
		swprintf_s(usbConnectedIndex, BUFFER_LENGTH, L"%d", i);
		// Get the data associated with each device connected inside the Enum key
		if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", usbConnectedIndex, RRF_RT_REG_SZ, NULL, (LPBYTE)data, &cbData))
			continue;
		// Convert the returned wstring data to string for regex search
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
		dataString = converterX.to_bytes(wstring(data));
		// serach for the regular expression VID/PID in 'data' using this format 'VID_####&PID_####\'
		if (regex_search(dataString.c_str(), matchedResult, reg))
		{
			// If regex search matched, store the substring after the regex format. This is the 17-digit serial number of the device.
			serialNumbers.push_back(matchedResult[0].second);
		}
		else
		{
			wstring messageWstring = L"ThorMCM301: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
			buf.push_back(0);
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, buf.data());
		}
	}
	return serialNumbers;
}

long MCM301::SelectAndConnect(const long& dev)
{
	Lock lock(_critSect);
	long count = 0;
	string port;
	string serialNum;
	wstring settingsSerialNum;
	vector<string>::iterator it;

	if (0 > dev)
	{
		return FALSE;
	}

	if (0 >= _snList.size())
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM301: Select Device, no device was detected.");
		return FALSE;
	}

	// Iterate through the list of serial numbers until the index matches the value passed
	for (it = _snList.begin(); it != _snList.end(); ++it)
	{
		if (count == dev)
		{
			serialNum = it->data();
			//Search the Registry for the COM port associated with the PID&IVID and the selected device serial number.
			if (TRUE == _foundUsbSer)
			{
				port = FindCOMPort(THORLABS_VID, THORLABS_MCM_PID, serialNum);
				_responseWaitTime = RESPONSE_WAIT_TIME;
				/*
				if (port.compare("COM" + _portId)
				{
					wstring messageWstring = L"ThorMCM301: The device port ID mismatched the arranged Port in ThorMCM301Settings file. \n\nConfigure the device port number to be COM" + wstring(_portId.begin(), _portId.end()) + L" in Device Manager.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: COM Port Mismatch", MB_OK);
				}
				*/
				if (atoi(_portId.c_str()) >= 17 && atoi(_portId.c_str()) <= 56)
				{
					wstring messageWstring = L"MCM301 using port " + wstring(_portId.begin(), _portId.end()) + L".\n\nTo avoid conflicts with other devices, consider using a port number less than 17 and greater than 56.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: Recommended Changing COM Port ", MB_OK);

				}
			}
			// port wstring returned should be more than 3 characters
			if (3 >= port.size())
			{
				wstring messageWstring = L"ThorMCM301: Returned port ID doesn't match required format to continue. Format should be: COM##, format returned is: " + wstring(port.begin(), port.end());
				vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
				buf.push_back(0);
				logDll->TLTraceEvent(ERROR_EVENT, 1, buf.data());
				return FALSE;
			}
		}
		count++;
	}
	// Open the serial port after getting the number

	if (_deviceHandler >= 0) fnUART_LIBRARY_close(_deviceHandler);

	_deviceHandler = fnUART_LIBRARY_open((char*)port.c_str(), _baudRate, 3);

	int open = fnUART_LIBRARY_isOpen((char*)port.c_str(), 0);

	if (_deviceHandler < 0 || TRUE != open)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"Connect to MCM301 failed, unable to open port.");
		return FALSE;
	}
	else
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"Connect to MCM301 success");
	}
	fnAPT_DLL_InitDevice(_deviceHandler);
	fnAPT_DLL_SetTSTScalerB(_deviceHandler, 1);

	SAFE_DELETE_PTR(_apt);
	_apt = new APT();

	if (FALSE == InitializeParams())
	{
		return FALSE;
	}

	return TRUE;
}

string MCM301::FindCOMPort(string VID, string PID, string serialNum)
{
	HKEY hk;
	HKEY hSubKey;
	HKEY deviceSubKey;
	HKEY parameterSubKey;
	string comPort = "";
	wstring path;
	string deviceNames;
	string devicesPidVidString;
	string format = "VID_" + VID + ".*&.*PID_" + PID;
	regex reg(format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\Enum as hk
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &hk) != ERROR_SUCCESS)
		return comPort;
	// For every subfolder in the key generate a new key (SubKeyName) and open the subfolder
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	for (DWORD i = 0; ; i++)
	{
		DWORD cName = BUFFER_LENGTH;
		wchar_t SubKeyName[BUFFER_LENGTH];
		// Get the name of the ith subkey
		if (ERROR_SUCCESS != RegEnumKeyEx(hk, i, SubKeyName, &cName, NULL, NULL, NULL, NULL))
			break;
		// Set the path to the subkey and open it in hsubkey
		path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName);
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hSubKey))
			break;
		// For every subfolder in the key generate a new key (deviceSubKey) and open the subfolder
		for (DWORD j = 0; ; j++)
		{
			DWORD dName = BUFFER_LENGTH;
			wchar_t devicesPidVid[BUFFER_LENGTH];
			// Get the name of the jth subkey and save it in devicesPidVid
			if (ERROR_SUCCESS != RegEnumKeyEx(hSubKey, j, devicesPidVid, &dName, NULL, NULL, NULL, NULL))
				break;
			// Convert the key name to string for RegEx search
			devicesPidVidString = converterX.to_bytes(wstring(devicesPidVid));
			// Compare the key name to the passed VID/PID using this format VID_####&PID_####
			if (regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID folder look at their key name and compare it to the passed serial number, 
				// if it matches grab the COM port number from the PortName field
				for (DWORD k = 0; ; k++)
				{
					DWORD pName = BUFFER_LENGTH;
					wchar_t deviceHID[BUFFER_LENGTH];
					// Get the name of the kth subkey and store it in DeviceHID
					if (ERROR_SUCCESS != RegEnumKeyEx(deviceSubKey, k, deviceHID, &pName, NULL, NULL, NULL, NULL))
						break;
					// Convert the name of the subkey to a string and compare it to the passed serialNum
					deviceNames = converterX.to_bytes(wstring(deviceHID));
					if (0 == deviceNames.compare(serialNum))
					{
						DWORD cbData = BUFFER_LENGTH;
						wchar_t value[BUFFER_LENGTH];
						// Set the path to the subkey with the matching serial number and open it in parameterSubKey
						path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid) + L"\\" + wstring(deviceHID);
						if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &parameterSubKey))
							continue;
						// Get the value from the PortName parameter
						if (ERROR_SUCCESS != RegGetValue(parameterSubKey, L"Device Parameters", L"PortName", RRF_RT_REG_SZ, NULL, (LPBYTE)value, &cbData))
							continue;
						comPort = converterX.to_bytes(wstring(value));
					}
				}
			}
		}
	}
	return comPort;
}
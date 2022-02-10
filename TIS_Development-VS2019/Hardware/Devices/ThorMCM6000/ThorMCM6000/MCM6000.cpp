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
#include "MCM6000.h"
#include "MCM6000XML.h"

string THORLABS_VID = "1313";
string THORLABS_MCM_PID = "2003";

string FTDI_VID = "0403";
string FTDI_PID = "6015";

HANDLE MCM6000::_hGetStatusThread = NULL; // Initialize status thread
int MCM6000::_threadDeviceHandler = -1;
long MCM6000::_setSlots = 0;
APT* MCM6000::_apt = NULL;
Mcm6kParams* MCM6000::_mcm6kParams;
unsigned long MCM6000::_responseWaitTime = RESPONSE_WAIT_TIME;
long MCM6000::_statusThreadStopped = FALSE;
long MCM6000::_stopStatusThread = FALSE;
long MCM6000::_settingsFileChanged = FALSE;
CritSect MCM6000::_critSect;

MCM6000::MCM6000()
{
	_deviceHandler = -1;
	_foundUsbSer = FALSE;
	_foundFTDI = FALSE;
	_baudRate = 115200; // 115200;// needs to be set for tablet
	_ftdiBaudRate = 921600; // 460800 500000 921600;
	_numberOfSetSlots = 0;
	_mcm6kParams = (Mcm6kParams*)malloc(sizeof(Mcm6kParams));
	_mcm6kParams->xPositionCurrent = 0;
	_mcm6kParams->yPositionCurrent = 0;
	_mcm6kParams->zPositionCurrent = 0;
	_mcm6kParams->rPositionCurrent = 0;
	_mcm6kParams->condenserPositionCurrent = 0;
	_mcm6kParams->xThreshold = 0.4;
	_mcm6kParams->yThreshold = 0.4;
	_mcm6kParams->zThreshold = 0.4;
	_mcm6kParams->rThreshold = 0.4;
	_mcm6kParams->condenserThreshold = 0.4;
	_mcm6kParams->xInvert = false;
	_mcm6kParams->yInvert = false;
	_mcm6kParams->zInvert = false;
	_mcm6kParams->rInvert = false;
	_mcm6kParams->condenserInvert = false;

	_mcm6kParams->xPidEnable = false;
	_mcm6kParams->yPidEnable = false;
	_mcm6kParams->zPidEnable = false;
	_mcm6kParams->rPidEnable = false;
	_mcm6kParams->condenserPidEnable = false;

	_mcm6kParams->x_slot_id = 0;
	_mcm6kParams->y_slot_id = 0;
	_mcm6kParams->z_slot_id = 0;
	_mcm6kParams->r_slot_id = 0;
	_mcm6kParams->ze_slot_id = 0;
	_mcm6kParams->lp_slot_id = 0;
	_mcm6kParams->et_slot_id = 0;
	_mcm6kParams->inverted_lp_slot_id = 0;
	_mcm6kParams->condenser_slot_id = 0;
	_mcm6kParams->shutter_slot_id = 0;
	_mcm6kParams->piezo_slot_id = 0;
	_mcm6kParams->ndd_slot_id = 0;

	_mcm6kParams->xPidKickoutEnable = false;
	_mcm6kParams->yPidKickoutEnable = false;
	_mcm6kParams->zPidKickoutEnable = false;
	_mcm6kParams->rPidKickoutEnable = false;
	_mcm6kParams->condenserPidKickoutEnable = false;

	_mcm6kParams->x_ccw_moving = false;
	_mcm6kParams->x_cw_moving = false;
	_mcm6kParams->y_ccw_moving = false;
	_mcm6kParams->y_cw_moving = false;
	_mcm6kParams->z_ccw_moving = false;
	_mcm6kParams->z_cw_moving = false;
	_mcm6kParams->r_ccw_moving = false;
	_mcm6kParams->r_cw_moving = false;
	_mcm6kParams->ze_ccw_moving = false;
	_mcm6kParams->ze_cw_moving = false;
	_mcm6kParams->condenser_ccw_moving = false;
	_mcm6kParams->condenser_cw_moving = false;
	_mcm6kParams->epiTurret_ccw_moving = false;
	_mcm6kParams->epiTurret_cw_moving = false;
	_mcm6kParams->lightPath_ccw_moving = false;
	_mcm6kParams->lightPath_cw_moving = false;
	_mcm6kParams->ndd_ccw_moving = false;
	_mcm6kParams->ndd_cw_moving = false;

	_mcm6kParams->xConfigured = FALSE;
	_mcm6kParams->yConfigured = FALSE;
	_mcm6kParams->zConfigured = FALSE;
	_mcm6kParams->rConfigured = FALSE;
	_mcm6kParams->lightPathConfigured = FALSE;
	_mcm6kParams->epiTurretConfigured = FALSE;
	_mcm6kParams->zeConfigured = FALSE;
	_mcm6kParams->condenserConfigured = FALSE;
	_mcm6kParams->piezoConfigured = FALSE;
	_mcm6kParams->nddConfigured = FALSE;
	_mcm6kParams->shutterConfigured = FALSE;

	_mcm6kParams->shuttersPositions[0] = SHUTTER_CLOSED;
	_mcm6kParams->shuttersPositions[1] = SHUTTER_CLOSED;
	_mcm6kParams->shuttersPositions[2] = SHUTTER_CLOSED;
	_mcm6kParams->shuttersPositions[3] = SHUTTER_CLOSED;
	_mcm6kParams->safetyInterlockState = FALSE;

	_mcm6kParams->piezoMode = -1;
	_board_type = 0;
	_cardType[0] = CardTypes::NO_CARD_IN_SLOT;
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		_cardType[i] = CardTypes::NO_CARD_IN_SLOT;
		_mcm6kParams->slotName[i][0] = '\0';
	}
	_cpldRev = NULL;
	_firmwareRev[0] = NULL;
	_ftdiModeEnabled = FALSE;
	_scopeType = ScopeType::UPRIGHT;
	firmwareVersion = NULL;
	firmwareVersionLength = 0;
	serialNumber = NULL;
	serialNumberLength = 0;
}

MCM6000::~MCM6000()
{
	/*delete firmwareVersion;
	delete serialNumber;
	DeleteCriticalSection(&CriticalSection);*/
}

void MCM6000::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}


/************************************************************************************************
* @fn	HANDLE MCM6000::GetStatusThread(DWORD &threadID)
*
* @brief	Create a Thread to request and read the status of each board.
* @param 	threadID	  	GetStatus Thread ID.
* @return	Thread Handle.
**************************************************************************************************/
HANDLE MCM6000::GetStatusThread(DWORD& threadID)
{
	_threadDeviceHandler = _deviceHandler;
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) & (MCM6000::GetStatusAllBoards), (void*)this, 0, &threadID);
	SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
	return handle;
}

// Request the status of every board that was set up. First Sends a request command, waits a certain time, then reads the response.
// The response is parsed through APT.cpp and saved to the data structure Mcm6KParams. The rest of the code requests the current values (position, 
// status) from Mcm6KParams.
//:TODO: Check the type of board to know what command to send for the status. Currently it is hard coded to request status of stepper motors
void MCM6000::GetStatusAllBoards(LPVOID instance)
{
	long slotNumber = CARD_ID_START_ADDRESS;
	unsigned long errorCounter = 0;
	const long MAX_NUM_ERRORS = 10000; //Do not overfill the logger with errors if device is disconnected
	do
	{
		//Request the status of every board that was set up in ThorMCM6000Settings.xml. Wait RESPONSE_WAIT_TIME for the response 
		if (_setSlots & (1 << (slotNumber - CARD_ID_START_ADDRESS)))
		{
			int result = -1;
			if (CardTypes::High_Current_Stepper_Card == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::High_Current_Stepper_Card_HD == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::ST_Invert_Stepper_BISS_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::ST_Invert_Stepper_SSI_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::MCM_Stepper_Internal_BISS_L6470 == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] ||
				CardTypes::MCM_Stepper_Internal_SSI_L6470 == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
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
						_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while reading MGMSG_MCM_REQ_STATUSUPDATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
					delete[] resultArray;
				}
				else
				{
					wchar_t errMsg[MSG_SIZE];
					StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while writing MGMSG_MCM_REQ_STATUSUPDATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
					LogMessage(errMsg, ERROR_EVENT);
					errorCounter++;
				}
			}
			else if (CardTypes::Slider_IO_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
			{
				for (long chan = 0; chan < MAX_MIRRORS_PER_CARD; chan++)
				{
					char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_REQ_MIRROR_STATE & 0xFF), (UCHAR)((MGMSG_MCM_REQ_MIRROR_STATE & 0xFF00) >> 8), static_cast<char>(chan), 0x00, static_cast<char>(slotNumber), HOST_ID };
					result = fnUART_LIBRARY_write(_threadDeviceHandler, bytesToSend, 6);
					if (result >= 0)
					{
						Sleep(RESPONSE_WAIT_TIME); // for mirrors the sleep time is 20ms

						char* resultArray = new char[BUFFER_LENGTH];
						result = fnUART_LIBRARY_read(_threadDeviceHandler, resultArray, BUFFER_LENGTH);
						if (result >= 0)
						{
							_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
						}
						else
						{
							wchar_t errMsg[MSG_SIZE];
							StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while reading MGMSG_MCM_REQ_MIRROR_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
							LogMessage(errMsg, ERROR_EVENT);
							errorCounter++;
						}
						delete[] resultArray;
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while writing MGMSG_MCM_REQ_MIRROR_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
				}
			}
			else if (CardTypes::Shutter_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
			{
				char bytesToSend[6] = { (UCHAR)(MGMSG_MOT_REQ_SOL_STATE & 0xFF), (UCHAR)((MGMSG_MOT_REQ_SOL_STATE & 0xFF00) >> 8), 0x00, 0x00, static_cast<char>(slotNumber), HOST_ID };
				result = fnUART_LIBRARY_write(_threadDeviceHandler, bytesToSend, 6);
				if (result >= 0)
				{
					Sleep(_responseWaitTime);

					char* resultArray = new char[BUFFER_LENGTH];
					result = fnUART_LIBRARY_read(_threadDeviceHandler, resultArray, BUFFER_LENGTH);
					if (result >= 0)
					{
						_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while reading MGMSG_MOT_REQ_SOL_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
					delete[] resultArray;
				}
				else
				{
					wchar_t errMsg[MSG_SIZE];
					StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while writing MGMSG_MOT_REQ_SOL_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
					LogMessage(errMsg, ERROR_EVENT);
					errorCounter++;
				}
			}
			else if (CardTypes::Shutter_4_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] || CardTypes::Shutter_4_type_REV6 == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
			{
				//Check for the state of the safety interlock
				char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_REQ_INTERLOCK_STATE & 0xFF), (UCHAR)((MGMSG_MCM_REQ_INTERLOCK_STATE & 0xFF00) >> 8), 0x00, 0x00, static_cast<char>(slotNumber), HOST_ID };
				result = fnUART_LIBRARY_write(_threadDeviceHandler, bytesToSend, 6);
				if (result >= 0)
				{
					Sleep(_responseWaitTime);

					char* resultArray = new char[BUFFER_LENGTH];
					result = fnUART_LIBRARY_read(_threadDeviceHandler, resultArray, BUFFER_LENGTH);
					if (result >= 0)
					{
						_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while reading MGMSG_MCM_REQ_INTERLOCK_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
					delete[] resultArray;
				}
				else
				{
					wchar_t errMsg[MSG_SIZE];
					StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while writing MGMSG_MCM_REQ_INTERLOCK_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
					LogMessage(errMsg, ERROR_EVENT);
					errorCounter++;
				}
				/*for (int chanId = 0; chanId < 4; chanId++)
				{
					char bytesToSend[6] = { (UCHAR)(MGMSG_MOT_REQ_SOL_STATE & 0xFF), (UCHAR)((MGMSG_MOT_REQ_SOL_STATE & 0xFF00) >> 8), static_cast<char>(chanId), 0x00, static_cast<char>(slotNumber), HOST_ID };
					result = fnUART_LIBRARY_write(_threadDeviceHandler, bytesToSend, 6);
					if (result >= 0)
					{
						Sleep(_responseWaitTime);

						char* resultArray = new char[BUFFER_LENGTH];
						result = fnUART_LIBRARY_read(_threadDeviceHandler, resultArray, BUFFER_LENGTH);
						if (result >= 0)
						{
							_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
						}
						else
						{
							wchar_t errMsg[MSG_SIZE];
							StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while reading MGMSG_MOT_REQ_SOL_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
							LogMessage(errMsg, ERROR_EVENT);
							errorCounter++;
						}
						delete[] resultArray;
					}
					else
					{
						wchar_t errMsg[MSG_SIZE];
						StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetStatusAllBoards: Error while writing MGMSG_MOT_REQ_SOL_STATE command. Slot: %d Card Type: %d", slotNumber - CARD_ID_START_ADDRESS, _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS]);
						LogMessage(errMsg, ERROR_EVENT);
						errorCounter++;
					}
				}*/
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
long MCM6000::GetHardwareInfo()
{
	char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_HW_REQ_INFO & 0xFF), (UCHAR)((MGMSG_MCM_HW_REQ_INFO & 0xFF00) >> 8), 0x00, 0x00, MOTHERBOARD_ID, HOST_ID };
	int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetHardwareInfo: Error while sending MGMSG_MCM_HW_REQ_INFO command.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	Sleep(_responseWaitTime * 3);

	char* resultArray = new char[BUFFER_LENGTH];
	result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);

	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->GetHardwareInfo: Error while reading MGMSG_MCM_HW_REQ_INFO request.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
	delete[] resultArray;
	return TRUE;
}

// Request and Read the parameters (mm_to_encoder counts, min, max) from each card 
long MCM6000::RequestStageParameters()
{
	for (char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_STAGEPARAMS & 0xFF), static_cast<char>((MGMSG_MCM_REQ_STAGEPARAMS & 0xFF00) >> 8), 0x00, 0x00, CARD_ID_START_ADDRESS + i, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->RequestStageParameters: Error while sending MGMSG_MCM_REQ_STAGEPARAMS command.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime * 3);

		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->RequestStageParameters: Error while reading MGMSG_MCM_REQ_STAGEPARAMS request.");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
		delete[] resultArray;
	}
	return TRUE;
}

long MCM6000::InitializeParams()
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

	try
	{
		auto_ptr<MCM6000XML> pSetup(new MCM6000XML());
		if (FALSE == pSetup->SaveSlotNameToSettingsFile(_mcm6kParams, _settingsFileChanged))
		{
			return FALSE;
		}

		//If the settings file has been changed in SaveSlotNameToSettingsFile() read it again for the new slot configuration
		if (TRUE == _settingsFileChanged)
		{
			if (FALSE == pSetup->ReadSettingsFile(_mcm6kParams, _scopeType, _portId, _baudRate, _ftdiPortId, _ftdiBaudRate, _ftdiModeEnabled, _setSlots, _numberOfSetSlots))
			{
				return FALSE;
			}
			_settingsFileChanged = FALSE;
		}

		if (FALSE == pSetup->VerifySlotCards(_mcm6kParams, _scopeType))
		{
			return FALSE;
		}
	}
	catch (...)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorMCM6000: Initialize Params -> Unable to locate MCM6000 xml file");
		LogMessage(errMsg, ERROR_EVENT);
	}

	//Check if the slot layout in ThorMCM6000Settings.xml is configured correctly. If it isn't display an error message and do not connect to the device
	// NOTE: It only checks the type of cards in the board (stepper, slider or Invert Stepper). It doesn't check what motor is controlling, which one is X, Y, Z or R
	// It is possible to only check if the type is correct. For this separate the first && and make the right part of the condition it's own if statement
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		if ((CardTypes::High_Current_Stepper_Card == _mcm6kParams->cardType[i] ||
			CardTypes::High_Current_Stepper_Card_HD == _mcm6kParams->cardType[i]) &&
			(_mcm6kParams->x_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->y_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->z_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->r_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->ze_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->condenser_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type HC Stepper Card mismatch. There is a card of type HC Stepper that is not accounted for. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: X, Y, Z, R, ZElevator, Condenser";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK);
		}
		else if (CardTypes::Slider_IO_type == _mcm6kParams->cardType[i] && _mcm6kParams->lp_slot_id != i + CARD_ID_START_ADDRESS)
		{
			wstring messageWstring = L"Card type Slider IO mismatch. There is a card of type Slider IO that is not accounted for. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card (lightpath): LP";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK);
		}
		else if ((CardTypes::ST_Invert_Stepper_BISS_type == _mcm6kParams->cardType[i] ||
			CardTypes::ST_Invert_Stepper_SSI_type == _mcm6kParams->cardType[i] || 
			CardTypes::MCM_Stepper_Internal_BISS_L6470 == _mcm6kParams->cardType[i] ||
			CardTypes::MCM_Stepper_Internal_SSI_L6470 == _mcm6kParams->cardType[i]) &&
			(_mcm6kParams->et_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->inverted_lp_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->z_slot_id != i + CARD_ID_START_ADDRESS &&
				_mcm6kParams->ndd_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type Inverted Stepper mismatch. There is a card of type Inverted Stepper that is not accounted for. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: Z, EpiTurret, InvertedLP, AFSwitch";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK);
		}
		//Piezo card type needs to be OR'ed with 0x8000 sometimes because the Piezo card might not have an EEPROM 
		if ((CardTypes::Piezo_Type == _mcm6kParams->cardType[i] ||
			(CardTypes::Piezo_Type | STATIC_CARD_SLOT_MASK) == _mcm6kParams->cardType[i]) &&
			(_mcm6kParams->piezo_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type Piezo mismatch. There is a card of type Piezo that is not accounted for. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: Piezo";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK);
		}
		if ((CardTypes::Shutter_type == _mcm6kParams->cardType[i] ||
			CardTypes::Shutter_4_type == _mcm6kParams->cardType[i] ||
			CardTypes::Shutter_4_type_REV6 == _mcm6kParams->cardType[i]) &&
			(_mcm6kParams->shutter_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type Shutter mismatch. There is a card of type Shutter that is not accounted for. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nPossible stage types for this type of card: Shutter";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK);
		}
	}

	//Set the piezo to Analog-In Mode
	if (TRUE == _mcm6kParams->piezoConfigured && Piezo_modes::PZ_ANALOG_INPUT_MODE != _mcm6kParams->piezoMode)
	{
		long piezoMode = -1;
		PiezoSetMode(Piezo_modes::PZ_ANALOG_INPUT_MODE);
		Sleep(RESPONSE_WAIT_TIME);
		PiezoRequestMode(piezoMode);
		//Check if the piezo was correctly set, if it wasn't try it again with a delay in between
		if (Piezo_modes::PZ_ANALOG_INPUT_MODE != piezoMode)
		{
			Sleep(RESPONSE_WAIT_TIME);
			PiezoSetMode(Piezo_modes::PZ_ANALOG_INPUT_MODE);
			Sleep(RESPONSE_WAIT_TIME);
			PiezoRequestMode(piezoMode);
			Sleep(RESPONSE_WAIT_TIME);
			if (Piezo_modes::PZ_ANALOG_INPUT_MODE != _mcm6kParams->piezoMode)
			{
				wchar_t errMsg[MSG_SIZE];
				StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000 error: Piezo mode not set to Analog Input Mode. Current mode is: %d", _mcm6kParams->piezoMode);
				LogMessage(errMsg, ERROR_EVENT);
			}
		}
	}

	//Start the status request thread once the hardware info and stage parameters have been queried 
	SAFE_DELETE_HANDLE(_hGetStatusThread);
	_hGetStatusThread = GetStatusThread(ThreadForStatus);

	/* Do not write the Pid control if something fails in the code, this will overwrite the stage configuration. MGMSG_MCM_SET_STAGEPARAMS writes on the flash memory
	Panchy says we shouldn't do this every time, it should only be done through his UI. But he is going to add space in the RAM where we can set the flags safely
	if(IsSlotIdValid(_mcm6kParams->x_slot_id))
	{
		ConfigPid(_mcm6kParams->x_slot_id, _mcm6kParams->xParams, _mcm6kParams->xPidEnable);
		ConfigPidKickout(_mcm6kParams->x_slot_id, _mcm6kParams->xParams, _mcm6kParams->xPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm6kParams->y_slot_id))
	{
		ConfigPid(_mcm6kParams->y_slot_id, _mcm6kParams->yParams, _mcm6kParams->yPidEnable);
		ConfigPidKickout(_mcm6kParams->y_slot_id, _mcm6kParams->yParams, _mcm6kParams->yPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm6kParams->z_slot_id))
	{
		ConfigPid(_mcm6kParams->z_slot_id, _mcm6kParams->zParams, _mcm6kParams->zPidEnable);
		ConfigPidKickout(_mcm6kParams->z_slot_id, _mcm6kParams->zParams, _mcm6kParams->zPidKickoutEnable);
	}
	if(IsSlotIdValid(_mcm6kParams->r_slot_id))
	{
		ConfigPid(_mcm6kParams->r_slot_id, _mcm6kParams->rParams, _mcm6kParams->rPidEnable);
		ConfigPidKickout(_mcm6kParams->r_slot_id, _mcm6kParams->rParams, _mcm6kParams->rPidKickoutEnable);
	}
	if (IsSlotIdValid(_mcm6kParams->condenser_slot_id))
	{
		ConfigPid(_mcm6kParams->condenser_slot_id, _mcm6kParams->condenserParams, _mcm6kParams->condenserPidEnable);
		ConfigPidKickout(_mcm6kParams->condenser_slot_id, _mcm6kParams->condenserParams, _mcm6kParams->condenserPidKickoutEnable);
	}
	*/

	// Panchy mentioned setting the jog parameters a lot might take an effect on the EPROM. 
	// He doesn't recommend doing this. Will comment it for now.
	/*if(IsSlotIdValid(_mcm6kParams->x_slot_id))
	SaveJogSize(_mcm6kParams->x_slot_id, _mcm6kParams->xJogSize);
	if(IsSlotIdValid(_mcm6kParams->y_slot_id))
	SaveJogSize(_mcm6kParams->y_slot_id, _mcm6kParams->yJogSize);
	if(IsSlotIdValid(_mcm6kParams->z_slot_id))
	SaveJogSize(_mcm6kParams->z_slot_id, _mcm6kParams->zJogSize);
	if(IsSlotIdValid(_mcm6kParams->r_slot_id))
	SaveJogSize(_mcm6kParams->r_slot_id, _mcm6kParams->rJogSize);
	if (IsSlotIdValid(_mcm6kParams->condenser_slot_id))
		SaveJogSize(_mcm6kParams->condenser_slot_id, _mcm6kParams->condenserJogSize); */

	return TRUE;
}

long MCM6000::ConfigPid(UCHAR slotId, byte* params, bool pidEn)
{
	Lock lock(_critSect);
	char bytesToSend[6 + 96];

	byte head[6] = { (UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF),
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8),
		96,
		0x00,
		slotId | 0x80,
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

long MCM6000::ConfigPidKickout(UCHAR slotId, byte* params, bool pidEn)
{
	Lock lock(_critSect);
	char bytesToSend[6 + 96];

	byte head[6] = { (UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF),
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8),
		96,
		0x00,
		slotId | 0x80,
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

long MCM6000::UpdateDeviceInfo()
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->UpdateDeviceInfo: Error while updating device info.");
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	Sleep(50);
	char* result = new char[BUFFER_LENGTH];

	ret = fnUART_LIBRARY_read(_deviceHandler, result, BUFFER_LENGTH);
	if (ret < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->UpdateDeviceInfo: Error while reading device info.");
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

long MCM6000::FirmwareVersion(wchar_t* version, int size)
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

long MCM6000::SerialNumber(wchar_t* number, int size)
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

long MCM6000::Home(unsigned char slotId)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->Home: Error while sending command fnAPT_DLL_MOT_MoveHome. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::Zero(unsigned char slotId)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->Zero: Error while sending command fnAPT_DLL_MOT_Set_Enccounter. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::Stop(unsigned char slotId)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->Stop: Error while sending command fnAPT_DLL_MOT_MoveStop. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::MoveBy(UCHAR slotId, double distance)
{
	Lock lock(_critSect);
	const int SIZE_OF_INT32 = sizeof(INT32);
	INT32 distanceToMove = static_cast<INT32>(distance);
	byte data[SIZE_OF_INT32];
	USHORT cmd = (USHORT)MGMSG_MCM_MOT_MOVE_BY;

	memcpy(data, &distanceToMove, SIZE_OF_INT32);
	byte bytesToSend[12] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
		0x06, 0x00, slotId | 0x80, HOST_ID,
		slotId & 0x0f - 1, 0x00,         // Chan Ident
		data[0], data[1], data[2], data[3],    // Move By step size in encoder counts
	};
	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->MoveBy: Error while sending command MGMSG_MCM_MOT_MOVE_BY. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::MoveTo(UCHAR slotId, double distance)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->MoveTo: Error while sending command fnAPT_DLL_MOT_MoveAbsolute. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::SaveJogSize(UCHAR slotId, double size)
{
	Lock lock(_critSect);
	byte data[4];
	double val = (size * 1e3) / _mcm6kParams->slot_nm_per_count[slotId - CARD_ID_START_ADDRESS];
	int s = (int)(val + 0.5);
	memcpy(data, &s, 4);
	USHORT cmd = (USHORT)MGMSG_MOT_SET_JOGPARAMS;

	byte bytesToSend[28] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
		22, 0x00, slotId | 0x80, HOST_ID,
		slotId & 0x0f - 1, 0,         // Chan Ident
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->SaveJogSize: Error while sending command MGMSG_MOT_SET_JOGPARAMS. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return ret;
}

long MCM6000::Jog(unsigned char slotId, unsigned char direction)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->Jog: Error while sending command fnAPT_DLL_MOT_MoveJog. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::MoveToStoredPos(int pos, UCHAR slotId)
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
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->MoveToStoredPos: Error while sending command MGMSG_SET_GOTO_STORE_POSITION. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return ret;
}

long MCM6000::MoveMirror(unsigned char slotId, long state, long mirrorChannel)
{
	Lock lock(_critSect);
	USHORT cmd = (USHORT)MGMSG_MCM_SET_MIRROR_STATE;
	byte bytesToSend[6] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8), static_cast<char>(mirrorChannel), static_cast<char>(state), slotId, HOST_ID };

	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->MoveMirror: Error while sending command MGMSG_MCM_SET_MIRROR_STATE. SlotId: %d", slotId);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::PiezoSetMode(long mode)
{
	Lock lock(_critSect);
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 PiezoSetMode-> Setting Piezo to Mode: %d", mode);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);

	USHORT cmd = (USHORT)MGMSG_MCM_PIEZO_SET_MODE;
	byte bytesToSend[6] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8), static_cast<char>(mode), 0x00, _mcm6kParams->piezo_slot_id, HOST_ID };

	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->PiezoSetMode: Error while sending command MGMSG_MCM_PIEZO_SET_MODE. SlotId: %d", _mcm6kParams->piezo_slot_id);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	return TRUE;
}

long MCM6000::PiezoRequestMode(long& mode)
{
	Lock lock(_critSect);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 PiezoRequestMode-> Requesting Piezo Mode");

	USHORT cmd = (USHORT)MGMSG_MCM_PIEZO_REQ_MODE;
	byte bytesToSend[6] = { (UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8), 0x00, 0x00, _mcm6kParams->piezo_slot_id, HOST_ID };

	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->PiezoRequestMode: Error while sending command MGMSG_MCM_PIEZO_REQ_MODE. SlotId: %d", _mcm6kParams->piezo_slot_id);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}

	Sleep(RESPONSE_WAIT_TIME);

	char* resultArray = new char[BUFFER_LENGTH];
	result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
	if (result >= 0)
	{
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
	}
	else
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000->PiezoRequestMode: Error while reading command MGMSG_MCM_PIEZO_GET_MODE. SlotId: %d", _mcm6kParams->piezo_slot_id);
		LogMessage(errMsg, ERROR_EVENT);
		return FALSE;
	}
	delete[] resultArray;
	mode = _mcm6kParams->piezoMode;

	return TRUE;
}

// -------------------------------Start of stage specific functions----------------------------------------------------------
long MCM6000::HomeX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM600 Start -> Home X");
	long result = Home(_mcm6kParams->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Home X");
	return result;
}

long MCM6000::HomeY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Home Y");
	long result = Home(_mcm6kParams->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Home Y");
	return result;
}

long MCM6000::HomeZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Home Z");
	long result = Home(_mcm6kParams->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Home Z");
	return result;
}

long MCM6000::HomeR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Home R");
	long result = Home(_mcm6kParams->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Home R");
	return result;
}

long MCM6000::HomeCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Home Condenser");
	long result = Home(_mcm6kParams->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Home Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM6000::Home()
//{
//	long resultX = HomeX();
//	long resultY = HomeY();
//	long resultZ = HomeZ();
//	long resultR = HomeR();
//	long resultCondenser = HomeCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM6000::ZeroX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Zero X");
	long result = Zero(_mcm6kParams->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Zero X");
	return result;
}

long MCM6000::ZeroY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Zero Y");
	long result = Zero(_mcm6kParams->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Zero Y");
	return result;
}

long MCM6000::ZeroZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Zero Z");
	long result = Zero(_mcm6kParams->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Zero Z");
	return result;
}

long MCM6000::ZeroR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Zero R");
	long result = Zero(_mcm6kParams->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Zero R");
	return result;
}

long MCM6000::ZeroCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Zero Condenser");
	long result = Zero(_mcm6kParams->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Zero Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM6000::Zero()
//{
//	long resultX = ZeroX();
//	long resultY = ZeroY();
//	long resultZ = ZeroZ();
//	long resultR = ZeroR();
//	long resultCondenser = ZeroCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM6000::StopX()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Stop X");
	long result = Stop(_mcm6kParams->x_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Stop X");
	return result;
}

long MCM6000::StopY()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Stop Y");
	long result = Stop(_mcm6kParams->y_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Stop Y");
	return result;
}

long MCM6000::StopZ()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Stop Z");
	long result = Stop(_mcm6kParams->z_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Stop Z");
	return result;
}

long MCM6000::StopR()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Stop R");
	long result = Stop(_mcm6kParams->r_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Stop R");
	return result;
}

long MCM6000::StopCondenser()
{
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 Start -> Stop Condenser");
	long result = Stop(_mcm6kParams->condenser_slot_id);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"MCM6000 End -> Stop Condenser");
	return result;
}

// :TODO: Not used yet. Should check if the the slot_id is configured before calling all stages
//long MCM6000::Stop()
//{
//	long resultX = StopX();
//	long resultY = StopY();
//	long resultZ = StopZ();
//	long resultR = StopR();
//	long resultCondenser = StopCondenser();
//	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE || resultCondenser == FALSE) ? FALSE : TRUE;
//}

long MCM6000::MoveXBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move X By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: X Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm6kParams->x_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move X By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveXTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move X To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: X Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->x_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move X To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveYBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Y By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:  Y Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm6kParams->y_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Y By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveYTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Y To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:  Y Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->y_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Y To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveZBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Z By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:   Z Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm6kParams->z_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Z By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveZTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Z To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:   Z Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->z_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Z To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveRBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move R By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->r_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:    R Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm6kParams->r_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move R By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveRTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move R To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->r_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:    R Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT);	//Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->r_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move R To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveCondenserBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Condenser By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->condenser_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:   Condenser Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveBy(_mcm6kParams->condenser_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Condenser By %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveCondenserTo(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move Condenser To %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->condenser_slot_id - CARD_ID_START_ADDRESS];

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000:   Condenser Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg, ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->condenser_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move Condenser To %lf", "End", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	return result;
}

long MCM6000::MoveLpTo(int pos)
{
	return MoveToStoredPos(pos, _mcm6kParams->inverted_lp_slot_id);
}

long MCM6000::MoveEtTo(int pos)
{
	return MoveToStoredPos(pos, _mcm6kParams->et_slot_id);
}

long MCM6000::MoveNDDTo(int pos)
{
	return MoveToStoredPos(pos, _mcm6kParams->ndd_slot_id);
}

long MCM6000::MoveGGLightpath(long state)
{
	return MoveMirror(_mcm6kParams->lp_slot_id, state, GG_MIRROR_CHAN_INDEX);
}

long MCM6000::MoveGRLightpath(long state)
{
	return MoveMirror(_mcm6kParams->lp_slot_id, state, GR_MIRROR_CHAN_INDEX);
}
long MCM6000::MoveCAMLightpath(long state)
{
	return MoveMirror(_mcm6kParams->lp_slot_id, state, CAM_MIRROR_CHAN_INDEX);
}

long MCM6000::XJogCW()
{
	return Jog(_mcm6kParams->x_slot_id, 0);
}

long MCM6000::XJogCCW()
{
	return Jog(_mcm6kParams->x_slot_id, 1);
}

long MCM6000::YJogCW()
{
	return Jog(_mcm6kParams->y_slot_id, 0);
}

long MCM6000::YJogCCW()
{
	return Jog(_mcm6kParams->y_slot_id, 1);
}

long MCM6000::ZJogCW()
{
	return Jog(_mcm6kParams->z_slot_id, 0);
}

long MCM6000::ZJogCCW()
{
	return Jog(_mcm6kParams->z_slot_id, 1);
}

long MCM6000::RJogCW()
{
	return Jog(_mcm6kParams->r_slot_id, 0);
}

long MCM6000::RJogCCW()
{
	return Jog(_mcm6kParams->r_slot_id, 1);
}

long MCM6000::CondenserJogCW()
{
	return Jog(_mcm6kParams->condenser_slot_id, 0);
}

long MCM6000::CondenserJogCCW()
{
	return Jog(_mcm6kParams->condenser_slot_id, 1);
}

long MCM6000::GetXPos(double& value)
{
	value = _mcm6kParams->xPositionCurrent;
	return TRUE;
}

long MCM6000::GetYPos(double& value)
{
	value = _mcm6kParams->yPositionCurrent;
	return TRUE;
}

long MCM6000::GetZPos(double& value)
{
	value = _mcm6kParams->zPositionCurrent;
	return TRUE;
}

long MCM6000::GetRPos(double& value)
{
	value = _mcm6kParams->rPositionCurrent;
	return TRUE;
}

long MCM6000::GetCondenserPos(double& value)
{
	value = _mcm6kParams->condenserPositionCurrent;
	return TRUE;
}

long MCM6000::GetLpPos(long& pos)
{
	pos = _mcm6kParams->invertedLightPathPos;
	return 0;
}

long MCM6000::GetEtPos(long& pos)
{
	pos = _mcm6kParams->epiTurretCurrentPos;
	return 0;
}

long MCM6000::GetZElevatorPos(double& value)
{
	value = _mcm6kParams->zePositionCurrent;
	return TRUE;
}

long MCM6000::GetNDDPos(long& pos)
{
	pos = _mcm6kParams->nddCurrentPos;
	return TRUE;
}

bool MCM6000::IsXmoving()
{
	return (_mcm6kParams->x_ccw_moving || _mcm6kParams->x_cw_moving);
}

bool MCM6000::IsYmoving()
{
	return (_mcm6kParams->y_ccw_moving || _mcm6kParams->y_cw_moving);
}

bool MCM6000::IsZmoving()
{
	return (_mcm6kParams->z_ccw_moving || _mcm6kParams->z_cw_moving);
}

bool MCM6000::IsRmoving()
{
	return (_mcm6kParams->r_ccw_moving || _mcm6kParams->r_cw_moving);
}

bool MCM6000::IsCondenserMoving()
{
	return (_mcm6kParams->condenser_ccw_moving || _mcm6kParams->condenser_cw_moving);
}

bool MCM6000::IsZEmoving()
{
	return (_mcm6kParams->ze_ccw_moving || _mcm6kParams->ze_cw_moving);
}

bool MCM6000::IsLighPathMoving()
{
	return (_mcm6kParams->lightPath_ccw_moving || _mcm6kParams->lightPath_cw_moving);
}

bool MCM6000::IsEpiTurretMoving()
{
	return (_mcm6kParams->epiTurret_ccw_moving || _mcm6kParams->epiTurret_cw_moving);
}

bool MCM6000::IsNDDmoving()
{
	return (_mcm6kParams->ndd_ccw_moving || _mcm6kParams->ndd_cw_moving);
}
// -------------------------------End of stage specific functions----------------------------------------------------------

long MCM6000::StatusPosition(long& status)
{
	Lock lock(_critSect);
	long ret = true;
	clock_t nextUpdateLoop = clock();
	//The status of the board updates every RESPONSE_WAIT_TIME * number of setup boards
	while (static_cast<unsigned long>(abs(nextUpdateLoop - clock()) / (CLOCKS_PER_SEC / 1000)) < (_numberOfSetSlots * _responseWaitTime))
	{
		status = IDevice::STATUS_READY;
		if (IsXmoving() || IsYmoving() || IsZmoving() || IsRmoving() || IsCondenserMoving() || IsLighPathMoving() || IsEpiTurretMoving() || IsNDDmoving())
		{
			status = IDevice::STATUS_BUSY;
		}
	}

	return TRUE;
}

long MCM6000::Close()
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

	//Set the piezo to Stop Mode to disable analog mode. Recommended by Panchy, in case the BNC signal is disconnected we are not sure what the floating signal might do with the Piezo
	if (TRUE == _mcm6kParams->piezoConfigured && Piezo_modes::PZ_STOP_MODE != _mcm6kParams->piezoMode)
	{
		long piezoMode = -1;
		PiezoSetMode(Piezo_modes::PZ_STOP_MODE);
		Sleep(RESPONSE_WAIT_TIME);
		PiezoRequestMode(piezoMode);
		//Check if the piezo was correctly set, if it wasn't try it again with a delay in between
		if (Piezo_modes::PZ_STOP_MODE != piezoMode)
		{
			Sleep(RESPONSE_WAIT_TIME);
			PiezoSetMode(Piezo_modes::PZ_STOP_MODE);
			Sleep(RESPONSE_WAIT_TIME);
			PiezoRequestMode(piezoMode);
			Sleep(RESPONSE_WAIT_TIME);

			if (Piezo_modes::PZ_STOP_MODE != _mcm6kParams->piezoMode)
			{
				wchar_t errMsg[MSG_SIZE];
				StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000 error: Piezo mode not set to Stop Mode. Current mode is: %d", _mcm6kParams->piezoMode);
				LogMessage(errMsg, ERROR_EVENT);
			}
		}
	}

	int ret = fnUART_LIBRARY_close(_deviceHandler);

	Sleep(200);	//For Refresh Hardware. Need to wait until the com port is completely closed.
	return (ret == 0) ? TRUE : FALSE;
}

bool MCM6000::IsConnected()
{
	return (_deviceHandler >= 0) ? true : false;
}

long MCM6000::FindAllDevs(long& devCount)
{
	Lock lock(_critSect);
	long ret = devCount = _foundFTDI = _foundUsbSer = FALSE, retValue = FALSE;
	try
	{
		auto_ptr<MCM6000XML> pSetup(new MCM6000XML());
		retValue = pSetup->ReadSettingsFile(_mcm6kParams, _scopeType, _portId, _baudRate, _ftdiPortId, _ftdiBaudRate, _ftdiModeEnabled, _setSlots, _numberOfSetSlots);
	}
	catch (...)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg, MSG_SIZE, L"ThorMCM6000: FindAllDevs -> Unable to locate MCM6000 xml file");
		LogMessage(errMsg, ERROR_EVENT);
	}
	if (TRUE == retValue)
	{
		if (TRUE == _ftdiModeEnabled)
		{
			//Search the Registry for any connected devices associated with this FTDI PID and VID.
			_snList = SerialNumbersFTDI(FTDI_VID, FTDI_PID);

			if (0 < _snList.size())
			{
				_foundFTDI = TRUE;
				devCount = (int)_snList.size();
				ret = TRUE;
			}
			else
			{
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000: Unable to find any device FTDI connected in the registry associated with this PID and VID");
			}
		}
		else
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
				logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000: Unable to find any serial device connected in the registry associated with this PID and VID");
			}
		}
	}
	return ret;
}

vector<string> MCM6000::SerialNumbers(string VID, string PID)
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
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000: Could not find registry key SYSTEM\\CurrentControlSet\\services\\usbser");
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected usbser devices 
	if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No usbSer device is connected, return empty list (size=0).
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\usbser\\Enum doesn't exist");
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
			wstring messageWstring = L"ThorMCM6000: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
			buf.push_back(0);
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, buf.data());
		}
	}
	return serialNumbers;
}

vector<string> MCM6000::SerialNumbersFTDI(string VID, string PID)
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
	// Select registry key System\CurrentControlSet\servies\FTDIBUS as hk
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\FTDIBUS", 0, KEY_READ, &hk))
	{
		// No FTDIBUS device is connected, return empty list (size=0).
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000-FTDI: Could not find registry key SYSTEM\\CurrentControlSet\\services\\FTDIBUS");
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected FTDIBUS devices 
	if (ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No FTDIBUS device is connected, return empty list (size=0).
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000-FTDI: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\FTDIBUS\\Enum doesn't exist");
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
			wstring messageWstring = L"ThorMCM6000-FTDI: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
			buf.push_back(0);
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, buf.data());
		}
	}
	return serialNumbers;
}

long MCM6000::SelectAndConnect(const long& dev)
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
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"ThorMCM6000: Select Device, no device was detected.");
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

				if (port.compare("COM" + _portId) != 0)
				{
					wstring messageWstring = L"ThorMCM6000: The device port ID mismatched the arranged Port in ThorMCM6000Settings file. \n\nConfigure the device port number to be COM" + wstring(_portId.begin(), _portId.end()) + L" in Device Manager.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: COM Port Mismatch", MB_OK);
				}
			}
			if (TRUE == _foundFTDI)
			{
				port = FindCOMPortFTDI(FTDI_VID, FTDI_PID, serialNum);
				_responseWaitTime = RESPONSE_WAIT_TIME_FTDI;

				if (port.compare("COM" + _ftdiPortId) != 0)
				{
					wstring messageWstring = L"ThorMCM6000 FTDI: The device port ID mismatched the arranged Port for FTDI in ThorMCM6000Settings file. \n\nConfigure the device Thorlabs MCM6000F port number to be COM" + wstring(_ftdiPortId.begin(), _ftdiPortId.end()) + L" in Device Manager.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: COM Port Mismatch", MB_OK);
				}
			}

			// port wstring returned should be more than 3 characters
			if (3 >= port.size())
			{
				wstring messageWstring = L"ThorMCM6000: Returned port ID doesn't match required format to continue. Format should be: COM##, format returned is: " + wstring(port.begin(), port.end());
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

	if (TRUE == _foundFTDI)
	{
		_deviceHandler = fnUART_LIBRARY_open((char*)port.c_str(), _ftdiBaudRate, 3);
	}
	else
	{
		_deviceHandler = fnUART_LIBRARY_open((char*)port.c_str(), _baudRate, 3);
	}

	int open = fnUART_LIBRARY_isOpen((char*)port.c_str(), 0);

	if (_deviceHandler < 0 || TRUE != open)
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"Connect to MCM6000 failed, unable to open port.");
		return FALSE;
	}
	else
	{
		logDll->TLTraceEvent(INFORMATION_EVENT, 1, L"Connect to MCM6000 success");
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

string MCM6000::FindCOMPort(string VID, string PID, string serialNum)
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

string MCM6000::FindCOMPortFTDI(string VID, string PID, string serialNum)
{
	HKEY hk;
	HKEY hSubKey;
	HKEY deviceSubKey;
	HKEY parameterSubKey;
	string comPort = "";
	wstring path;
	string devicesPidVidString;
	string format = "VID_" + VID + "\\+PID_" + PID + "\\+" + serialNum + ".*";
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
			// Compare the key name to the passed VID/PID using this format VID_####+PID_####+serialNum
			if (regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID and serial number
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID and serial number folder grab the COM port number from the PortName field
				for (DWORD k = 0; ; k++)
				{
					DWORD pName = BUFFER_LENGTH;
					wchar_t deviceHID[BUFFER_LENGTH];
					// Get the name of the kth subkey and store it in DeviceHID
					if (ERROR_SUCCESS != RegEnumKeyEx(deviceSubKey, k, deviceHID, &pName, NULL, NULL, NULL, NULL))
						break;
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
	return comPort;
}
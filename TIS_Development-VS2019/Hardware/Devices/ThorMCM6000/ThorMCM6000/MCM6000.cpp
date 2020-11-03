#include "MCM6000.h"
#include "include/apt_cmd_library.h"
#include "include/apt_cmd_library_motor.h"
#include "include/uart_library.h"
#include "..\..\..\..\Common\Device.h"
#include "..\..\..\..\Common\Log.h"
#include <memory>
#include <iostream>
#include <string>
#include <regex>
#include <codecvt>
#include "msxml6.h"
#include "Strsafe.h"

#define BUFFER_LENGTH 255

string THORLABS_VID = "1313";
string THORLABS_MCM_PID = "2003";

string FTDI_VID = "0403";
string FTDI_PID = "6015";

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)\

//std::auto_ptr<LogDll> logDll(new LogDll(L"ThorLogging.dll"));

HANDLE MCM6000::_hGetStatusThread = NULL; // Initialize status thread
int MCM6000::_threadDeviceHandler = -1;
long MCM6000::_setSlots = 0;
APT * MCM6000::_apt;
Mcm6kParams * MCM6000::_mcm6kParams;
unsigned long MCM6000::_responseWaitTime = RESPONSE_WAIT_TIME;
long MCM6000::_statusThreadStopped = FALSE;
long MCM6000::_stopStatusThread = FALSE;

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
	_mcm6kParams->xThreshold = 0.4;
	_mcm6kParams->yThreshold = 0.4;
	_mcm6kParams->zThreshold = 0.4;
	_mcm6kParams->rThreshold = 0.4;
	_mcm6kParams->xInvert = false;
	_mcm6kParams->yInvert = false;
	_mcm6kParams->zInvert = false;
	_mcm6kParams->rInvert = false;

	_mcm6kParams->xPidEnable = false;
	_mcm6kParams->yPidEnable = false;
	_mcm6kParams->zPidEnable = false;
	_mcm6kParams->rPidEnable = false;

	_mcm6kParams->xPidKickoutEnable = false;
	_mcm6kParams->yPidKickoutEnable = false;
	_mcm6kParams->zPidKickoutEnable = false;
	_mcm6kParams->rPidKickoutEnable = false;

	_mcm6kParams->x_ccw_moving = false;
	_mcm6kParams->x_cw_moving = false;
	_mcm6kParams->y_ccw_moving = false;
	_mcm6kParams->y_cw_moving = false;
	_mcm6kParams->z_ccw_moving = false;
	_mcm6kParams->z_cw_moving = false;
	_mcm6kParams->r_ccw_moving = false;
	_mcm6kParams->r_cw_moving = false;

	_mcm6kParams->zeAvailable = FALSE;
	
	CriticalSection = {};
	_board_type = 0;
	_cardType[0] = CardTypes::NO_CARD_IN_SLOT;
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		_cardType[i] = CardTypes::NO_CARD_IN_SLOT;
	}
	_cpldRev = NULL;
	_firmwareRev[0] = NULL;
	_ftdiModeEnabled = FALSE;
	_scopeType = ScopeType::UPRIGHT;
	firmwareVersion = nullptr;
	firmwareVersionLength = 0;
	serialNumber = nullptr;
	serialNumberLength = 0;
}

MCM6000::~MCM6000()
{
	delete firmwareVersion;
	delete serialNumber;
	DeleteCriticalSection(&CriticalSection);
}

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT &Variant)
{
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);
	CHK_ALLOC(bstr);

	V_VT(&Variant)   = VT_BSTR;
	V_BSTR(&Variant) = bstr;

CleanUp:
	return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument **ppDoc)
{
	HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
	if (SUCCEEDED(hr))
	{
		// these methods should not fail so don't inspect result
		(*ppDoc)->put_async(VARIANT_FALSE);  
		(*ppDoc)->put_validateOnParse(VARIANT_FALSE);
		(*ppDoc)->put_resolveExternals(VARIANT_FALSE);
	}
	return hr;
}

void MCM6000::LogMessage(wchar_t *logMsg,long eventLevel)
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
HANDLE MCM6000::GetStatusThread(DWORD &threadID)
{
	_threadDeviceHandler = _deviceHandler;
	HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &(MCM6000::GetStatusAllBoards), (void *) this, 0, &threadID);
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
	do
	{
		//Request the status of every board that was set up in ThorMCM6000Settings.xml. Wait RESPONSE_WAIT_TIME for the response 
		if(_setSlots & (1 << (slotNumber - CARD_ID_START_ADDRESS)))
		{
			int result = -1;
			if (CardTypes::High_Current_Stepper_Card == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] || CardTypes::ST_Invert_Stepper_BISS_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS] || CardTypes::ST_Invert_Stepper_SSI_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
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
				}			
			}
			else if (CardTypes::Slider_IO_type == _mcm6kParams->cardType[slotNumber - CARD_ID_START_ADDRESS])
			{
				for(long chan = 0; chan < MAX_MIRRORS_PER_CARD; chan++)
				{
					char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_MIRROR_STATE & 0xFF), static_cast<char>((MGMSG_MCM_REQ_MIRROR_STATE & 0xFF00) >> 8), static_cast<char>(chan), 0x00, static_cast<char>(slotNumber), HOST_ID };
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
					}		
				}
			}
		}
		if(CARD_ID_START_ADDRESS + TOTAL_CARD_SLOTS - 1 == slotNumber) // When reaching the max slot number reset slotNumber to the first slot
		{
			slotNumber = CARD_ID_START_ADDRESS;
		}
		else
		{
			slotNumber++;
		}
		//If the thread is about to be stopped, it needs to make sure it read the 
		// last status request. Otherwise we can have a pending request next time we read status.
		if(TRUE == _stopStatusThread)
		{
			_statusThreadStopped = TRUE;
			return;		
		}
	}
	while(true);
}

long MCM6000::ReadSettingsFile()
{
	HRESULT hr = CoInitialize(NULL);
	bool initializedCOM = false;
	// Only record that we initialized COM if that is what we really did. An error code of RPC_E_CHANGED_MODE means that the call
	// to CoInitialize failed because COM had already been initialized on another mode - which isn't a fatal condition and so in this
	// case we don't want to call CoUninitialize
	if (FAILED(hr))
	{
		if (hr != RPC_E_CHANGED_MODE)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000: ReadSettingsFile error, could not initialize COM");
			LogMessage(errMsg,ERROR_EVENT);
			return FALSE;
		}
		else
		{
			initializedCOM = true;
		}
	}
	if (SUCCEEDED(hr) || initializedCOM) {
		hr = S_OK;
		IXMLDOMDocument *pXMLDom = NULL;
		IXMLDOMParseError *pXMLErr = NULL;

		BSTR bstrXML = NULL;
		BSTR bstrErr = NULL;
		VARIANT_BOOL varStatus;
		VARIANT varFileName;
		VariantInit(&varFileName);
		VARIANT val;
		VariantInit(&val);
		CHK_HR(CreateAndInitDOM(&pXMLDom));

		// XML file name to load
		CHK_HR(VariantFromString(L"ThorMCM6000Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));
		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM6000Settings/SlotLayout/@slot");
			IXMLDOMNode *pNode = NULL;
			BSTR bstrQuery;
			_mcm6kParams->zeAvailable = FALSE;
			for(int i = 0; i < TOTAL_CARD_SLOTS; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);

				if (pNode)
				{
					pNode->get_nodeValue(&val);

					if(wstring(val.bstrVal).compare(L"X") == 0) _mcm6kParams->x_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"Y") == 0) _mcm6kParams->y_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"Z") == 0) _mcm6kParams->z_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"R") == 0) _mcm6kParams->r_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"LP") == 0) _mcm6kParams->lp_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"ET") == 0) _mcm6kParams->et_slot_id = CARD_ID_START_ADDRESS + i;
					if(wstring(val.bstrVal).compare(L"ILP") == 0) _mcm6kParams->inverted_lp_slot_id = CARD_ID_START_ADDRESS + i;
					if (wstring(val.bstrVal).compare(L"ZE") == 0)
					{
						_mcm6kParams->ze_slot_id = CARD_ID_START_ADDRESS + i;
						_mcm6kParams->zeAvailable = TRUE;
					}
				}
				SysFreeString(bstrQuery);
			}

			//Check if the SlotID is valid and was configured, enabled the corresponding bit on _setSlots
			//:TODO: This should be set when the boards are checked for what type of board they are, instead of doing it the other way around
			_setSlots |= (IsSlotIdValid(_mcm6kParams->x_slot_id)) ? 1 << (_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->y_slot_id)) ? 1 << (_mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->z_slot_id)) ? 1 << (_mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->r_slot_id)) ? 1 << (_mcm6kParams->r_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->ze_slot_id)) ? 1 << (_mcm6kParams->ze_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->lp_slot_id)) ? 1 << (_mcm6kParams->lp_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->et_slot_id)) ? 1 << (_mcm6kParams->et_slot_id - CARD_ID_START_ADDRESS) : _setSlots;
			_setSlots |= (IsSlotIdValid(_mcm6kParams->inverted_lp_slot_id)) ? 1 << (_mcm6kParams->inverted_lp_slot_id - CARD_ID_START_ADDRESS) : _setSlots;

			_numberOfSetSlots = CountSetBits(_setSlots);

			////////////////////////////////////////////////////////////
			// max
			bq = L"/MCM6000Settings/XRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@max";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// min

			bq = L"/MCM6000Settings/XRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@min";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// threshold

			bq = L"/MCM6000Settings/XRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rThreshold = _wtof(val.bstrVal) / 1e3;
			}
			SysFreeString(bstrQuery);

			// invert

			bq = L"/MCM6000Settings/XRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// Move by Threshold
			bq = L"/MCM6000Settings/XRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			// pid
			bq = L"/MCM6000Settings/XRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// PidKickout
			bq = L"/MCM6000Settings/XRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->xPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->yPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->zPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				_mcm6kParams->rPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			///////////////////////////////////////////////////////////////////

			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

			//*TODO* _serialNumber is not currently used. We can use it if there are multiple
			// MCM6000 connected and the user can pick one, but this would need to be discussed.
			bq = L"/MCM6000Settings/DeviceInfo/@serialNumber";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				if(SysStringLen(val.bstrVal) != 0)
					_serialNumber = converterX.to_bytes(wstring(val.bstrVal));
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/DeviceInfo/@baudRate";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				if(t > 0)
					_baudRate = t;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/DeviceInfo/@portId";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				if(SysStringLen(val.bstrVal) != 0)
					_portId = _serialNumber = converterX.to_bytes(wstring(val.bstrVal));;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/DeviceInfo/@scopeType";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				_scopeType = (t == 1) ? INVERTED : UPRIGHT;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/FTDIsettings/@portID";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				if(SysStringLen(val.bstrVal) != 0)
					_ftdiPortId = converterX.to_bytes(wstring(val.bstrVal));;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/FTDIsettings/@baudRate";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				if(t > 0)
					_ftdiBaudRate = t;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/FTDIsettings/@FTDIMode";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if(pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				_ftdiModeEnabled = (t == 1) ? TRUE : FALSE;
			}
			SysFreeString(bstrQuery);
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			return FALSE;
		}

CleanUp:
		SAFE_RELEASE(pXMLDom);
		SAFE_RELEASE(pXMLErr);
		SysFreeString(bstrXML);
		SysFreeString(bstrErr);
		VariantClear(&varFileName);
		CoUninitialize();
	}
	return TRUE;
}

// Request and Read the board info (firmware version, cpld version, card types) from the device
long MCM6000::GetHardwareInfo()
{
	char bytesToSend[6] = { (UCHAR)(MGMSG_MCM_HW_REQ_INFO & 0xFF), (UCHAR)((MGMSG_MCM_HW_REQ_INFO & 0xFF00) >> 8), 0x00, 0x00, MOTHERBOARD_ID, HOST_ID };
	int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000->GetHardwareInfo: Error while sending MGMSG_MCM_HW_REQ_INFO command.");
		LogMessage(errMsg,ERROR_EVENT);
		return FALSE;
	}
	Sleep(_responseWaitTime*3);

	char* resultArray = new char[BUFFER_LENGTH];
	result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);

	if (result <= 0)
	{
		wchar_t errMsg[MSG_SIZE];
		StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000->GetHardwareInfo: Error while reading MGMSG_MCM_HW_REQ_INFO request.");
		LogMessage(errMsg,ERROR_EVENT);
		return FALSE;
	}
	_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
	return TRUE;
}

// Request and Read the parameters (mm_to_encoder counts, min, max) from each card 
long MCM6000::RequestStageParameters()
{
	for(char i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		char bytesToSend[6] = { static_cast<char>(MGMSG_MCM_REQ_STAGEPARAMS & 0xFF), static_cast<char>((MGMSG_MCM_REQ_STAGEPARAMS & 0xFF00) >> 8), 0x00, 0x00, CARD_ID_START_ADDRESS + i, HOST_ID };
		int result = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, 6);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000->RequestStageParameters: Error while sending MGMSG_MCM_REQ_STAGEPARAMS command.");
			LogMessage(errMsg,ERROR_EVENT);
			return FALSE;
		}
		Sleep(_responseWaitTime*3);

		char* resultArray = new char[BUFFER_LENGTH];
		result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
		if (result < 0)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000->RequestStageParameters: Error while reading MGMSG_MCM_REQ_STAGEPARAMS request.");
			LogMessage(errMsg,ERROR_EVENT);
			return FALSE;
		}
		_apt->ParseApt(resultArray, BUFFER_LENGTH, _mcm6kParams);
	}
	return TRUE;
}

long MCM6000::InitializeParams()
{
	DWORD ThreadForStatus;
	// Need to notify the status thread it is about to be stopped. Make sure it read all
	//pending requests
	if(NULL != _hGetStatusThread)
	{
		clock_t nextUpdateLoop = clock();
		_stopStatusThread = TRUE;
		while(FALSE == _statusThreadStopped  && static_cast<unsigned long>(abs(nextUpdateLoop - clock())/(CLOCKS_PER_SEC/1000)) < (_numberOfSetSlots*_responseWaitTime))
		{
			//wait until the status thread has stopped
		}
		_statusThreadStopped = FALSE;
		_stopStatusThread = FALSE;
	}

	SAFE_DELETE_HANDLE(_hGetStatusThread);	

	if(FALSE == GetHardwareInfo())
	{
		return FALSE;	
	}

	//Check if the slot layout in ThorMCM6000Settings.xml is configured correctly. If it isn't display an error message and do not connect to the device
	// NOTE: It only checks the type of cards in the board (stepper, slider or Invert Stepper). It doesn't check what motor is controlling, which one is X, Y, Z or R
	// It is possible to only check if the type is correct. For this separate the first && and make the right part of the condition it's own if statement
	for(int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		if(CardTypes::High_Current_Stepper_Card == _mcm6kParams->cardType[i] &&
			(_mcm6kParams->x_slot_id != i + CARD_ID_START_ADDRESS &&
			_mcm6kParams->y_slot_id != i + CARD_ID_START_ADDRESS &&	
			_mcm6kParams->z_slot_id != i + CARD_ID_START_ADDRESS &&	
			_mcm6kParams->r_slot_id != i + CARD_ID_START_ADDRESS &&	
			_mcm6kParams->ze_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type mismatch. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nCard abbreviations: X, Y, Z, R, LP, ZE, ET, ILP";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK); 
			return FALSE;
		}
		else if( CardTypes::Slider_IO_type == _mcm6kParams->cardType[i] && _mcm6kParams->lp_slot_id != i + CARD_ID_START_ADDRESS)
		{
			wstring messageWstring = L"Card type mismatch. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nCard abbreviations: X, Y, Z, R, LP, ZE, ET, ILP";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK); 
			return FALSE;
		}
		else if ((CardTypes::ST_Invert_Stepper_BISS_type == _mcm6kParams->cardType[i] ||
			CardTypes::ST_Invert_Stepper_SSI_type == _mcm6kParams->cardType[i]) &&
			(_mcm6kParams->et_slot_id != i + CARD_ID_START_ADDRESS &&
			_mcm6kParams->inverted_lp_slot_id != i + CARD_ID_START_ADDRESS &&
			_mcm6kParams->z_slot_id != i + CARD_ID_START_ADDRESS))
		{
			wstring messageWstring = L"Card type mismatch. Please check ThorMCM6000Settings.xml Make sure SlotLayout is configured correctly with the right matching cards. \n\nIf error persists please contact Thorlabs customer support.\nCard abbreviations: X, Y, Z, R, LP, ZE, ET, ILP";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Card Type Mismatch", MB_OK); 
			return FALSE;
		}
	}

	_mcm6kParams->slot_nm_per_count[_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_NM_PER_COUNT;
	_mcm6kParams->slot_nm_per_count[_mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_NM_PER_COUNT;
	_mcm6kParams->slot_nm_per_count[_mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_NM_PER_COUNT;
	_mcm6kParams->slot_counter_per_unit[_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_XY_NM_PER_COUNT;
	_mcm6kParams->slot_counter_per_unit[_mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_XY_NM_PER_COUNT;
	_mcm6kParams->slot_counter_per_unit[_mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS] = DEFAULT_Z_NM_PER_COUNT;

	if(FALSE == RequestStageParameters())
	{
		return FALSE;
	}

	//Start the status request thread once the hardware info and stage parameters have been queried 
	_hGetStatusThread = GetStatusThread(ThreadForStatus);

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

	// Panchy mentioned setting the jog parameters a lot might take an effect on the EPROM. 
	// He doesn't recommend doing this. Will comment it for now.
	/*if(IsSlotIdValid(_mcm6kParams->x_slot_id))
	SaveJogSize(_mcm6kParams->x_slot_id, _mcm6kParams->xJogSize);
	if(IsSlotIdValid(_mcm6kParams->y_slot_id))
	SaveJogSize(_mcm6kParams->y_slot_id, _mcm6kParams->yJogSize);
	if(IsSlotIdValid(_mcm6kParams->z_slot_id))
	SaveJogSize(_mcm6kParams->z_slot_id, _mcm6kParams->zJogSize);
	if(IsSlotIdValid(_mcm6kParams->r_slot_id))
	SaveJogSize(_mcm6kParams->r_slot_id, _mcm6kParams->rJogSize);*/

	return TRUE;
}

bool MCM6000::IsSlotIdValid(UCHAR id)
{
	return (id < CARD_ID_START_ADDRESS + TOTAL_CARD_SLOTS && id >= CARD_ID_START_ADDRESS);
}

long MCM6000::ConfigPid(UCHAR slotId, byte* params, bool pidEn)
{
	char bytesToSend[6 + 96];

	byte head[6] = {(UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF), 
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8), 
		96, 
		0x00, 
		slotId | 0x80, 
		HOST_ID};
	memcpy(bytesToSend, head, sizeof(head));
	if(pidEn)
		params[81] |= (1<<4);
	else
		params[81] &= (~(1<<4));

	memcpy(bytesToSend + sizeof(head), params, 96);

	fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));

	return 0;
}

long MCM6000::ConfigPidKickout(UCHAR slotId, byte* params, bool pidEn)
{
	char bytesToSend[6 + 96];

	byte head[6] = {(UCHAR)(MGMSG_MCM_SET_STAGEPARAMS & 0xFF), 
		(UCHAR)((MGMSG_MCM_SET_STAGEPARAMS & 0xFF00) >> 8), 
		96, 
		0x00, 
		slotId | 0x80, 
		HOST_ID};
	memcpy(bytesToSend, head, sizeof(head));
	if(pidEn)
		params[81] |= (1<<5);
	else
		params[81] &= (~(1<<5));

	memcpy(bytesToSend + sizeof(head), params, 96);

	fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));

	return 0;
}

// This can be used later, if we want to make the reading time more efficient
// we would need to separate the request command from the read response part
// just need to make sure the response is complete, including the extendent data
/*void MCM6000::SerialPort_DataReceived()
{
char* resultArray = new char[BUFFER_LENGTH];
char* completeArray = new char[BUFFER_LENGTH];
char header[6];
char* extData = NULL;
int  extDataSize = 0;
int result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);

if (result < 0)
return;

int counter = 0;
while(resultArray[counter] != '\0')
{
counter++;
} 

memcpy(completeArray, resultArray, counter);

if(counter < 5)
{
RecursiveReadData(completeArray, counter, 5);
}

memcpy(header, completeArray, 6);
//long size = sizeof(resultArray);
if((header[4] & 0x80) != 0) 
{
extDataSize = (USHORT)(header[3] << 8) + (USHORT)header[2];
if(extDataSize > (BUFFER_LENGTH - 6)) return; // error
counter = 6;
while(completeArray[counter] != '\0')
{
counter++;
}
if(counter < extDataSize)
{
RecursiveReadData(completeArray, counter, extDataSize);
}

extData = (char*)malloc(extDataSize * sizeof(char));
memcpy(extData, (completeArray + 6), extDataSize);
}

_apt->ParseApt(completeArray, BUFFER_LENGTH, _mcm6kParams);
}

long MCM6000::RecursiveReadData(char * completeBuf, long initialCounter, long extDataSize)
{
int counter = 0;
char* resultArray = new char[BUFFER_LENGTH];
int result = fnUART_LIBRARY_read(_deviceHandler, resultArray, BUFFER_LENGTH);
while(resultArray[counter] != '\0')
{
counter++;
}
memcpy(completeBuf + initialCounter, resultArray, counter);
if(initialCounter + counter < extDataSize)
{
return RecursiveReadData(completeBuf, initialCounter + counter, extDataSize);
}
else
{
return TRUE;
}
}*/

long MCM6000::UpdateDeviceInfo()
{
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, MOTHERBOARD_ID, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_HW_Req_Info(_deviceHandler, message);
	int ret = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (ret < 0)
		return FALSE;
	Sleep(50);
	char *result = new char[BUFFER_LENGTH];

	ret = fnUART_LIBRARY_read(_deviceHandler, result, BUFFER_LENGTH);
	if (ret < 0)
		return FALSE;
	wchar_t tFirmwareVersion[] = { (wchar_t)'0' + result[22],'.',(wchar_t)'0' + result[21],'.',(wchar_t)'0' + result[20], '\0' };
	firmwareVersionLength = 12;
	firmwareVersion = new wchar_t[firmwareVersionLength];
	wmemcpy_s(firmwareVersion, firmwareVersionLength, tFirmwareVersion, firmwareVersionLength);

	wchar_t tSerialNumer[] = { (wchar_t)result[6],(wchar_t)result[7] ,(wchar_t)result[8] ,(wchar_t)result[9], '\0' };
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
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_MOT_MoveHome(_deviceHandler, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
		return FALSE;
	return TRUE;
}

long MCM6000::Zero(unsigned char slotId)
{
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_Set_Enccounter(_deviceHandler, 0, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
		return FALSE;
	return TRUE;
}

long MCM6000::Stop(unsigned char slotId)
{
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, 0x00);
	auto messageSize = fnAPT_DLL_MOT_MoveStop(_deviceHandler, 0, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
		return FALSE;
	return TRUE;
}

long MCM6000::MoveBy(UCHAR slotId, double distance)
{
	const int SIZE_OF_INT32 = sizeof(INT32);
	INT32 distanceToMove = static_cast<INT32>(distance);
	byte data[SIZE_OF_INT32];
	USHORT cmd = (USHORT)MGMSG_MCM_MOT_MOVE_BY;

	memcpy(data, &distanceToMove, SIZE_OF_INT32);
	byte bytesToSend[12] = {(UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
		0x06, 0x00, slotId | 0x80, HOST_ID,
		slotId & 0x0f - 1, 0x00,         // Chan Ident
		data[0], data[1], data[2], data[3],    // Move By step size in encoder counts
	};
	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
		return FALSE;

	return TRUE;
}

long MCM6000::MoveTo(UCHAR slotId, double distance)
{
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_MoveAbsolute(_deviceHandler, distance, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
		return FALSE;

	return TRUE;
}

long MCM6000::SaveJogSize(UCHAR slotId, double size)
{
	byte data[4];
	double val = (size * 1e3) / _mcm6kParams->slot_nm_per_count[slotId - CARD_ID_START_ADDRESS];
	int s = (int)(val + 0.5);
	memcpy(data, &s, 4);
	USHORT cmd = (USHORT)MGMSG_MOT_SET_JOGPARAMS;

	byte bytesToSend[28] = {(UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8),
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

	return ret;
}

long MCM6000::Jog(unsigned char slotId, unsigned char direction)
{
	unsigned char* message = new unsigned char[BUFFER_LENGTH];
	fnAPT_DLL_SetMsgSrcDest(_deviceHandler, slotId, 0x00, 0x01, (slotId ^ 0x20) - 1);
	auto messageSize = fnAPT_DLL_MOT_MoveJog(_deviceHandler, direction, message);
	int result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(message), messageSize);
	delete[] message;
	if (result < 0)
		return FALSE;
	return TRUE;
}

long MCM6000::MoveToStoredPos(int pos, UCHAR slotId)
{
	char bytesToSend[6] = { static_cast<char>(MGMSG_SET_GOTO_STORE_POSITION & 0xFF),
		static_cast<char>((MGMSG_SET_GOTO_STORE_POSITION & 0xFF00) >> 8),
		static_cast<char>(slotId - CARD_ID_START_ADDRESS),
		static_cast<char>(pos),
		static_cast<char>(slotId),
		HOST_ID};;
	long ret = fnUART_LIBRARY_write(_deviceHandler, bytesToSend, sizeof(bytesToSend));
	return ret;
}

long MCM6000::MoveMirror(unsigned char slotId, long state, long mirrorChannel)
{
	USHORT cmd = (USHORT)MGMSG_MCM_SET_MIRROR_STATE;
	byte bytesToSend[6] = {(UCHAR)(cmd & 0xFF), (UCHAR)((cmd & 0xFF00) >> 8), static_cast<char>(mirrorChannel), static_cast<char>(state), slotId, HOST_ID};
	
	long result = fnUART_LIBRARY_write(_deviceHandler, reinterpret_cast<char*>(bytesToSend), sizeof(bytesToSend));
	if (result < 0)
		return FALSE;

	return TRUE;
}

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

long MCM6000::Home()
{
	long resultX = HomeX();
	long resultY = HomeY();
	long resultZ = HomeZ();
	long resultR = HomeR();
	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE) ? FALSE : TRUE;
}

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

long MCM6000::Zero()
{
	long resultX = ZeroX();
	long resultY = ZeroY();
	long resultZ = ZeroZ();
	long resultR = ZeroR();
	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE) ? FALSE : TRUE;
}

long MCM6000::Stop()
{
	long resultX = StopX();
	long resultY = StopY();
	long resultZ = StopZ();
	long resultR = StopR();
	return (resultX == FALSE || resultY == FALSE || resultZ == FALSE || resultR == FALSE) ? FALSE : TRUE;
}

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

long MCM6000::MoveXBy(double distance)
{
	wchar_t logText[BUFFER_LENGTH];
	wsprintf(logText, L"MCM6000 %s -> Move X By %lf", "Start", distance);
	logDll->TLTraceEvent(INFORMATION_EVENT, 1, logText);
	double distanceNM = distance;

	distance = (UM_TO_MM * distance) / _mcm6kParams->slot_nm_per_count[_mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS];
	distance = round(distance); //Temporary solution, the calculated encoder distance might be below the target number of encoder steps

	wchar_t errMsg[MSG_SIZE];
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000: X Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000: X Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:  Y Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:  Y Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:   Z Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:   Z Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:    R Move By distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT); //Temporary, show in logger whenever a stage is trying to move.

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
	StringCbPrintfW(errMsg,MSG_SIZE,L"MCM6000:    R Move TO distance uM: %.3Lf, Encoder counts: %Lf", distanceNM, distance);
	LogMessage(errMsg,ERROR_EVENT);	//Temporary, show in logger whenever a stage is trying to move.

	long result = MoveTo(_mcm6kParams->r_slot_id, distance);

	wsprintf(logText, L"MCM6000 %s -> Move R To %lf", "End", distance);
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
	return MoveMirror(_mcm6kParams->lp_slot_id, state,CAM_MIRROR_CHAN_INDEX);
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

long MCM6000::StatusPosition(long &status) {
	long ret = true;
	EnterCriticalSection(&CriticalSection);
	clock_t nextUpdateLoop = clock();
	//The status of the board updates every RESPONSE_WAIT_TIME * number of setup boards
	while(static_cast<unsigned long>(abs(nextUpdateLoop - clock())/(CLOCKS_PER_SEC/1000)) < (_numberOfSetSlots*_responseWaitTime))
	{
		status = IDevice::STATUS_READY;
		if(IsXmoving() || IsYmoving() || IsZmoving() || IsRmoving())
		{
			status = IDevice::STATUS_BUSY;
		}
	}
	LeaveCriticalSection(&CriticalSection);

	return TRUE;
}

long MCM6000::Close()
{
	int ret = fnUART_LIBRARY_close(_deviceHandler);

	// Need to notify the status thread it is about to be stopped. Make sure it read all
	//pending requests
	if(NULL != _hGetStatusThread)
	{
		clock_t nextUpdateLoop = clock();
		_stopStatusThread = TRUE;
		while(FALSE == _statusThreadStopped  && static_cast<unsigned long>(abs(nextUpdateLoop - clock())/(CLOCKS_PER_SEC/1000)) < (_numberOfSetSlots*_responseWaitTime))
		{
			//wait until the status thread has stopped
		}
		_statusThreadStopped = FALSE;
		_stopStatusThread = FALSE;
	}

	SAFE_DELETE_HANDLE(_hGetStatusThread);

	Sleep(200);	//For Refresh Hardware. Need to wait until the com port is completely closed.
	return (ret == 0) ? TRUE : FALSE;
}

bool MCM6000::IsConnected()
{
	return (_deviceHandler >= 0) ? true : false;
}

//Using Brian Kernighan’s Algorithm
unsigned long MCM6000::CountSetBits(int value) 
{ 
	int n = value;
	unsigned int count = 0; 
	while (n) 
	{ 
		n &= (n-1) ; 
		count++; 
	} 
	return count; 
} 

long MCM6000::FindAllDevs(long& devCount)
{
	long ret = devCount = _foundFTDI = _foundUsbSer = FALSE;
	if(TRUE == ReadSettingsFile())
	{
		if(TRUE == _ftdiModeEnabled)
		{
			//Search the Registry for any connected devices associated with this FTDI PID and VID.
			_snList = SerialNumbersFTDI(FTDI_VID, FTDI_PID);

			if ( 0 < _snList.size())
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

			if ( 0 < _snList.size())
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
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\servies\usbser as hk
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\usbser", 0, KEY_READ, &hk))
	{
		// No usbSer device is connected, return empty list (size=0).
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorMCM6000: Could not find registry key SYSTEM\\CurrentControlSet\\services\\usbser");
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected usbser devices 
	if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No usbSer device is connected, return empty list (size=0).
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorMCM6000: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\usbser\\Enum doesn't exist");
		return serialNumbers;
	}
	// Iterate through the parameters, one for each connected device
	for(DWORD i = 0; i < count; i++)
	{
		DWORD cbData = BUFFER_LENGTH;
		wchar_t usbConnectedIndex[BUFFER_LENGTH];
		// Use i as the parameter name, the parameter name for each device is their index number
		swprintf_s(usbConnectedIndex, BUFFER_LENGTH, L"%d", i);
		// Get the data associated with each device connected inside the Enum key
		if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", usbConnectedIndex, RRF_RT_REG_SZ, NULL, (LPBYTE)data, &cbData))
			continue;
		// Convert the returned wstring data to string for regex search
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
		dataString = converterX.to_bytes(wstring(data));
		// serach for the regular expression VID/PID in 'data' using this format 'VID_####&PID_####\'
		if(regex_search(dataString.c_str(), matchedResult, reg))
		{
			// If regex search matched, store the substring after the regex format. This is the 17-digit serial number of the device.
			serialNumbers.push_back(matchedResult[0].second);
		}
		else
		{
			wstring messageWstring = L"ThorMCM6000: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
			buf.push_back(0);
			logDll->TLTraceEvent(ERROR_EVENT, 1, buf.data());
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
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\servies\FTDIBUS as hk
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\FTDIBUS", 0, KEY_READ, &hk))
	{
		// No FTDIBUS device is connected, return empty list (size=0).
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorMCM6000-FTDI: Could not find registry key SYSTEM\\CurrentControlSet\\services\\FTDIBUS");
		return serialNumbers;
	}
	// Read parameter Count which is the number of connected FTDIBUS devices 
	if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", L"Count", RRF_RT_REG_DWORD, NULL, (LPBYTE)&count, &sz))
	{
		// No FTDIBUS device is connected, return empty list (size=0).
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorMCM6000-FTDI: No USB serial device is connected, registry key SYSTEM\\CurrentControlSet\\services\\FTDIBUS\\Enum doesn't exist");
		return serialNumbers;
	}
	// Iterate through the parameters, one for each connected device
	for(DWORD i = 0; i < count; i++)
	{
		DWORD cbData = BUFFER_LENGTH;
		wchar_t usbConnectedIndex[BUFFER_LENGTH];
		// Use i as the parameter name, the parameter name for each device is their index number
		swprintf_s(usbConnectedIndex, BUFFER_LENGTH, L"%d", i);
		// Get the data associated with each device connected inside the Enum key
		if(ERROR_SUCCESS != RegGetValue(hk, L"Enum", usbConnectedIndex, RRF_RT_REG_SZ, NULL, (LPBYTE)data, &cbData))
			continue;
		// Convert the returned wstring data to string for regex search
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
		dataString = converterX.to_bytes(wstring(data));
		// serach for the regular expression VID/PID in 'data' using this format 'VID_####&PID_####\'
		if(regex_search(dataString.c_str(), matchedResult, reg))
		{
			// If regex search matched, store the substring after the regex format. This is the 17-digit serial number of the device.
			serialNumbers.push_back(matchedResult[0].second);
		}
		else
		{
			wstring messageWstring = L"ThorMCM6000-FTDI: USB serial device " + wstring(data) + L" doesn't match device PID and VID";
			vector<wchar_t> buf(messageWstring.begin(), messageWstring.end());
			buf.push_back(0);
			logDll->TLTraceEvent(ERROR_EVENT, 1, buf.data());
		}
	}
	return serialNumbers;
}

long MCM6000::SelectAndConnect(const long &dev)
{
	long count = 0;
	string port;
	string serialNum;
	wstring settingsSerialNum;
	vector<string>::iterator it;

	if(0 > dev)
	{
		return FALSE;
	}

	if (0 >= _snList.size())
	{
		logDll->TLTraceEvent(ERROR_EVENT, 1, L"ThorMCM6000: Select Device, no device was detected.");
		return FALSE;
	}

	// Iterate through the list of serial numbers until the index matches the value passed
	for(it = _snList.begin(); it != _snList.end(); ++it)
	{
		if(count == dev)
		{
			serialNum = it->data();
			//Search the Registry for the COM port associated with the PID&IVID and the selected device serial number.
			if(TRUE == _foundUsbSer)
			{
				port = FindCOMPort(THORLABS_VID, THORLABS_MCM_PID, serialNum);
				_responseWaitTime = RESPONSE_WAIT_TIME;

				if(port.compare("COM" + _portId) != 0)
				{
					wstring messageWstring = L"ThorMCM6000: The device port ID mismatched the arranged Port in ThorMCM6000Settings file. \n\nConfigure the device port number to be COM" + wstring(_portId.begin(), _portId.end()) + L" in Device Manager.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: COM Port Mismatch", MB_OK); 
				}
			}
			if(TRUE == _foundFTDI)
			{
				port = FindCOMPortFTDI(FTDI_VID, FTDI_PID, serialNum);
				_responseWaitTime = RESPONSE_WAIT_TIME_FTDI;

				if(port.compare("COM" + _ftdiPortId) != 0)
				{
					wstring messageWstring = L"ThorMCM6000 FTDI: The device port ID mismatched the arranged Port for FTDI in ThorMCM6000Settings file. \n\nConfigure the device Thorlabs MCM6000F port number to be COM" + wstring(_ftdiPortId.begin(), _ftdiPortId.end()) + L" in Device Manager.";
					MessageBox(NULL, messageWstring.c_str(), L"Warning: COM Port Mismatch", MB_OK); 
				}
			}

			// port wstring returned should be more than 3 characters
			if(3 >= port.size())
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

	if(_deviceHandler >= 0) fnUART_LIBRARY_close(_deviceHandler);

	if(TRUE == _foundFTDI)
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

	_apt = new APT();

	if(FALSE == InitializeParams())
	{
		return FALSE;
	}

	InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x00000400);

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
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\Enum as hk
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &hk) != ERROR_SUCCESS)
		return comPort;
	// For every subfolder in the key generate a new key (SubKeyName) and open the subfolder
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	for (DWORD i = 0; ;i++)
	{
		DWORD cName = BUFFER_LENGTH;
		wchar_t SubKeyName[BUFFER_LENGTH];
		// Get the name of the ith subkey
		if (ERROR_SUCCESS != RegEnumKeyEx(hk, i, SubKeyName, &cName, NULL, NULL, NULL, NULL))
			break;
		// Set the path to the subkey and open it in hsubkey
		path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName);
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hSubKey))
			break;
		// For every subfolder in the key generate a new key (deviceSubKey) and open the subfolder
		for(DWORD j = 0; ; j++)
		{		
			DWORD dName = BUFFER_LENGTH;
			wchar_t devicesPidVid[BUFFER_LENGTH];
			// Get the name of the jth subkey and save it in devicesPidVid
			if (ERROR_SUCCESS != RegEnumKeyEx(hSubKey, j, devicesPidVid, &dName, NULL, NULL, NULL, NULL))
				break;
			// Convert the key name to string for RegEx search
			devicesPidVidString = converterX.to_bytes(wstring(devicesPidVid));
			// Compare the key name to the passed VID/PID using this format VID_####&PID_####
			if(regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID folder look at their key name and compare it to the passed serial number, 
				// if it matches grab the COM port number from the PortName field
				for(DWORD k = 0; ; k++)
				{		
					DWORD pName = BUFFER_LENGTH;
					wchar_t deviceHID[BUFFER_LENGTH];
					// Get the name of the kth subkey and store it in DeviceHID
					if (ERROR_SUCCESS != RegEnumKeyEx(deviceSubKey, k, deviceHID, &pName, NULL, NULL, NULL, NULL))
						break;
					// Convert the name of the subkey to a string and compare it to the passed serialNum
					deviceNames = converterX.to_bytes(wstring(deviceHID));
					if(0 == deviceNames.compare(serialNum))
					{
						DWORD cbData = BUFFER_LENGTH;
						wchar_t value[BUFFER_LENGTH];
						// Set the path to the subkey with the matching serial number and open it in parameterSubKey
						path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid) + L"\\" + wstring(deviceHID);
						if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &parameterSubKey))
							continue;
						// Get the value from the PortName parameter
						if(ERROR_SUCCESS != RegGetValue(parameterSubKey, L"Device Parameters", L"PortName", RRF_RT_REG_SZ, NULL, (LPBYTE)value, &cbData))
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
	regex reg (format);
	cmatch matchedResult;
	// Select registry key System\CurrentControlSet\Enum as hk
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &hk) != ERROR_SUCCESS)
		return comPort;
	// For every subfolder in the key generate a new key (SubKeyName) and open the subfolder
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	for (DWORD i = 0; ;i++)
	{
		DWORD cName = BUFFER_LENGTH;
		wchar_t SubKeyName[BUFFER_LENGTH];
		// Get the name of the ith subkey
		if (ERROR_SUCCESS != RegEnumKeyEx(hk, i, SubKeyName, &cName, NULL, NULL, NULL, NULL))
			break;
		// Set the path to the subkey and open it in hsubkey
		path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName);
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hSubKey))
			break;
		// For every subfolder in the key generate a new key (deviceSubKey) and open the subfolder
		for(DWORD j = 0; ; j++)
		{		
			DWORD dName = BUFFER_LENGTH;
			wchar_t devicesPidVid[BUFFER_LENGTH];
			// Get the name of the jth subkey and save it in devicesPidVid
			if (ERROR_SUCCESS != RegEnumKeyEx(hSubKey, j, devicesPidVid, &dName, NULL, NULL, NULL, NULL))
				break;
			// Convert the key name to string for RegEx search
			devicesPidVidString = converterX.to_bytes(wstring(devicesPidVid));
			// Compare the key name to the passed VID/PID using this format VID_####+PID_####+serialNum
			if(regex_search(devicesPidVidString.c_str(), matchedResult, reg))
			{
				// If the VID and PID match, open this subkey with the matching VID/PID and serial number
				path = L"SYSTEM\\CurrentControlSet\\Enum\\" + wstring(SubKeyName) + L"\\" + wstring(devicesPidVid);
				if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &deviceSubKey))
					break;
				// For every subfolder in the matching VID/PID and serial number folder grab the COM port number from the PortName field
				for(DWORD k = 0; ; k++)
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
					if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &parameterSubKey))
						continue;
					// Get the value from the PortName parameter
					if(ERROR_SUCCESS != RegGetValue(parameterSubKey, L"Device Parameters", L"PortName", RRF_RT_REG_SZ, NULL, (LPBYTE)value, &cbData))
						continue;
					comPort = converterX.to_bytes(wstring(value));
				}
			}
		}
	}
	return comPort;
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
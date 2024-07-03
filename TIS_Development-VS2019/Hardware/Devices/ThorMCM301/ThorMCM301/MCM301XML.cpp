#include "MCM301XML.h"

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

/// <summary>
/// Initializes a new instance of the <see cref="MCM301XML"/> class.
/// </summary>
MCM301XML::MCM301XML()
{
	/*_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;*/
}

/// <summary>
/// Finalizes an instance of the <see cref="MCM301XML"/> class.
/// </summary>
MCM301XML::~MCM301XML()
{
	/*if(_xmlObj != NULL)
	{
		delete _xmlObj;
	}*/
}

// Helper function to create a VT_BSTR variant from a null terminated string. 
HRESULT VariantFromString(PCWSTR wszValue, VARIANT& Variant)
{
	HRESULT hr = S_OK;
	BSTR bstr = SysAllocString(wszValue);
	CHK_ALLOC(bstr);

	V_VT(&Variant) = VT_BSTR;
	V_BSTR(&Variant) = bstr;

CleanUp:
	return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument** ppDoc)
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

void MCM301XML::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

//Using Brian Kernighan’s Algorithm
unsigned long MCM301XML::CountSetBits(int value)
{
	int n = value;
	unsigned int count = 0;
	while (n)
	{
		n &= (n - 1);
		count++;
	}
	return count;
}

bool MCM301XML::IsSlotIdValid(UCHAR id)
{
	return (id < CARD_ID_START_ADDRESS + TOTAL_CARD_SLOTS && id >= CARD_ID_START_ADDRESS);
}


long MCM301XML::ReadSettingsFile(Mcm301Params* mcm301Params, ScopeType& scopeType, string& portId, int& baudRate, long& setSlots, unsigned long& numberOfSetSlots)
{
	HRESULT hr = CoInitialize(NULL);
	bool initializedCOM = false;
	scopeType = UPRIGHT;
	/*mcm301Params->x_slot_id = CARD_ID_START_ADDRESS + 0 + 4;
	mcm301Params->y_slot_id = CARD_ID_START_ADDRESS + 1 + 4;
	mcm301Params->z_slot_id = CARD_ID_START_ADDRESS + 2 + 4;*/
/*	mcm301Params->r_slot_id = 0;
	mcm301Params->xConfigured = TRUE;
	mcm301Params->yConfigured = TRUE;
	mcm301Params->zConfigured = TRUE;
	mcm301Params->cardConfigured[0 + 4] = TRUE;
	mcm301Params->cardConfigured[1 + 4] = TRUE;
	mcm301Params->cardConfigured[2 + 4] = TRUE;
	mcm301Params->rConfigured = TRUE;
	mcm301Params->zeConfigured = TRUE;
	mcm301Params->ze_slot_id = 0;
	mcm301Params->condenser_slot_id = 0;
	mcm301Params->xConfigured = FALSE;
	mcm301Params->yConfigured = FALSE;
	mcm301Params->zConfigured = FALSE;
	mcm301Params->rConfigured = FALSE;
	mcm301Params->zeConfigured = FALSE;
	mcm301Params->condenserConfigured = FALSE;*/

	// Only record that we initialized COM if that is what we really did. An error code of RPC_E_CHANGED_MODE means that the call
	// to CoInitialize failed because COM had already been initialized on another mode - which isn't a fatal condition and so in this
	// case we don't want to call CoUninitialize
	if (FAILED(hr))
	{
		if (hr != RPC_E_CHANGED_MODE)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: ReadSettingsFile error, could not initialize COM");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		else
		{
			initializedCOM = true;
		}
	}
	if (SUCCEEDED(hr) || initializedCOM) {
		hr = S_OK;
		IXMLDOMDocument* pXMLDom = NULL;
		IXMLDOMParseError* pXMLErr = NULL;

		BSTR bstrXML = NULL;
		BSTR bstrErr = NULL;
		VARIANT_BOOL varStatus;
		VARIANT varFileName;
		VariantInit(&varFileName);
		VARIANT val;
		VariantInit(&val);
		CHK_HR(CreateAndInitDOM(&pXMLDom));

		// XML file name to load
		CHK_HR(VariantFromString(L"ThorMCM301Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));

		

		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM301Settings/SlotLayout/@slot");
			IXMLDOMNode* pNode = NULL;
			BSTR bstrQuery;
			
			for (int i = 0; i < TOTAL_CARD_SLOTS - 4; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);
				
				if (pNode)
				{
					
					pNode->get_nodeValue(&val);
					
					if (wstring(val.bstrVal).compare(L"X") == 0 || wstring(val.bstrVal).compare(L"x") == 0)
					{
						mcm301Params->x_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->xConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Y") == 0 || wstring(val.bstrVal).compare(L"y") == 0)
					{
						mcm301Params->y_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->yConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Z") == 0 || wstring(val.bstrVal).compare(L"z") == 0)
					{
						mcm301Params->z_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->zConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"R") == 0)
					{
						mcm301Params->r_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->rConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Condenser") == 0)
					{
						mcm301Params->condenser_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->condenserConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"ZElevator") == 0)
					{
						mcm301Params->ze_slot_id = CARD_ID_START_ADDRESS + i + 4;
						mcm301Params->zeConfigured = TRUE;
						mcm301Params->cardConfigured[i + 4] = TRUE;
					}
					/*else // add else if for other statements before uncommenting
					{
						wstring messageWstring = L"Please configure axis for slot" + to_wstring(i + 1) + L" for ThorMCM301Settings.xml";
						MessageBox(NULL, messageWstring.c_str(), L"No slot layout set", MB_OK);

					}*/
				}
				SysFreeString(bstrQuery);
			}

			//Check if the SlotID is valid and was configured, enabled the corresponding bit on _setSlots
			//:TODO: This should be set when the boards are checked for what type of board they are, instead of doing it the other way around
			setSlots |= (IsSlotIdValid(mcm301Params->x_slot_id)) ? 1 << (mcm301Params->x_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm301Params->y_slot_id)) ? 1 << (mcm301Params->y_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm301Params->z_slot_id)) ? 1 << (mcm301Params->z_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm301Params->r_slot_id)) ? 1 << (mcm301Params->r_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm301Params->ze_slot_id)) ? 1 << (mcm301Params->ze_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm301Params->condenser_slot_id)) ? 1 << (mcm301Params->condenser_slot_id - CARD_ID_START_ADDRESS) : setSlots;

			numberOfSetSlots = CountSetBits(setSlots);

			////////////////////////////////////////////////////////////
			// --------------------------- max --------------------------- 
			bq = L"/MCM301Settings/XRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@max";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- min --------------------------- 

			bq = L"/MCM301Settings/XRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@min";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- threshold --------------------------- 

			bq = L"/MCM301Settings/XRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rThreshold = _wtof(val.bstrVal) / 1e3;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- invert --------------------------- 

			bq = L"/MCM301Settings/XRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// --------------------------- Move by Threshold --------------------------- 
			bq = L"/MCM301Settings/XRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			// --------------------------- pid --------------------------- 
			/* Might be used later
			bq = L"/MCM301Settings/XRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// --------------------------- PidKickout ---------------------------
			bq = L"/MCM301Settings/XRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->xPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/YRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->yPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/ZRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->zPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/RRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->rPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/CondenserRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm301Params->condenserPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);
			*/
			///////////////////////////////////////////////////////////////////

			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

			//*TODO* _serialNumber is not currently used. We can use it if there are multiple
			// MCM301 connected and the user can pick one, but this would need to be discussed.
			/*
			bq = L"/MCM301Settings/DeviceInfo/@serialNumber";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				if (SysStringLen(val.bstrVal) != 0)
					_serialNumber = converterX.to_bytes(wstring(val.bstrVal));
			}
			SysFreeString(bstrQuery);
			*/

			bq = L"/MCM301Settings/DeviceInfo/@baudRate";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				if (t > 0)
					baudRate = t;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM301Settings/DeviceInfo/@portId";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				if (SysStringLen(val.bstrVal) != 0)
					portId = converterX.to_bytes(wstring(val.bstrVal));;
			}
			SysFreeString(bstrQuery);

			/*bq = L"/MCM301Settings/DeviceInfo/@scopeType";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				_scopeType = (t == 1) ? INVERTED : UPRIGHT;
			}
			SysFreeString(bstrQuery);*/
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: ReadSettingsFile error, failed to load xml");
			LogMessage(errMsg, ERROR_EVENT);
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

long MCM301XML::SaveSlotNameToSettingsFile(Mcm301Params* mcm301Params, long& settingsFileChanged)
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
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: SaveSlotNameToSettingsFile error, could not initialize COM");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		else
		{
			initializedCOM = true;
		}
	}
	if (SUCCEEDED(hr) || initializedCOM) {
		hr = S_OK;
		IXMLDOMDocument* pXMLDom = NULL;
		IXMLDOMParseError* pXMLErr = NULL;

		BSTR bstrXML = NULL;
		BSTR bstrErr = NULL;
		VARIANT_BOOL varStatus;
		VARIANT varFileName;
		VariantInit(&varFileName);
		VARIANT varStage;
		VariantInit(&varStage);
		VARIANT val;
		VariantInit(&val);
		CHK_HR(CreateAndInitDOM(&pXMLDom));

		// XML file name to load
		CHK_HR(VariantFromString(L"ThorMCM301Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));
		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM301Settings/SlotLayout/@slot");
			IXMLDOMNode* pNode = NULL;
			BSTR bstrQuery;
			//For now, use the TOTAL_CARD_SLOTS - 4 and slotName[i + 4] so there are only 3 slots listed in xml
			for (int i = 0; i < TOTAL_CARD_SLOTS - 4; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);

				if (pNode)
				{
					pNode->get_nodeValue(&val);
					string slotName(mcm301Params->slotName[i + 4], 16);

					//If the slot configuration is empty and the slot name read from the device is not empty
					if (wstring(val.bstrVal).compare(L"") == 0 && slotName.compare("") != 0)
					{
						if (slotName.find('X') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"X", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.find('Y') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"Y", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.find('Z') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"Z", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.find('R') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"R", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.find('C') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"Condenser", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.find('E') != std::string::npos)
						{
							CHK_HR(VariantFromString(L"ZElevator", varStage));
							pNode->put_nodeValue(varStage);
						}

						settingsFileChanged = TRUE;
					}
				}
				SysFreeString(bstrQuery);
			}
			if (TRUE == settingsFileChanged)
			{
				CHK_HR(pXMLDom->save(varFileName));
			}
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: SaveSlotNameToSettingsFile error, failed to load xml");
			LogMessage(errMsg, ERROR_EVENT);
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

long MCM301XML::SaveSerialNoToSettingsFile(Mcm301Params* mcm301Params) 
{
	HRESULT hr = CoInitialize(NULL);
	bool initializedCOM = false;
	long settingsFileChanged;

	// Only record that we initialized COM if that is what we really did. An error code of RPC_E_CHANGED_MODE means that the call
	// to CoInitialize failed because COM had already been initialized on another mode - which isn't a fatal condition and so in this
	// case we don't want to call CoUninitialize
	if (FAILED(hr))
	{
		if (hr != RPC_E_CHANGED_MODE)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: SaveSlotNameToSettingsFile error, could not initialize COM");
			LogMessage(errMsg, ERROR_EVENT);
			return FALSE;
		}
		else
		{
			initializedCOM = true;
		}
	}
	if (SUCCEEDED(hr) || initializedCOM) {
		hr = S_OK;
		IXMLDOMDocument* pXMLDom = NULL;
		IXMLDOMParseError* pXMLErr = NULL;

		BSTR bstrXML = NULL;
		BSTR bstrErr = NULL;
		VARIANT_BOOL varStatus;
		VARIANT varFileName;
		VariantInit(&varFileName);
		VARIANT varStage;
		VariantInit(&varStage);
		VARIANT val;
		VariantInit(&val);
		CHK_HR(CreateAndInitDOM(&pXMLDom));

		// XML file name to load
		CHK_HR(VariantFromString(L"ThorMCM301Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));
		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM301Settings/SlotSerialNo/@slot");
			IXMLDOMNode* pNode = NULL;
			BSTR bstrQuery;
			//Assume only 3 slots: x, y and z at slot 1, 2, & 3
			for (int i = 0; i < TOTAL_CARD_SLOTS - 4; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);

				if (pNode)
				{
					pNode->get_nodeValue(&val);
					wstring slotSerialNo = to_wstring(mcm301Params->deviceSerialNo[i + 4]);
					/*If a device was not found, then later nofiy user the slot wasn't selected(VerifySlotCards)*/
					long devFound = !mcm301Params->noDeviceFound[i + 4];
					//If the serial number configuration is empty, update and notify user
					if (wstring(val.bstrVal).compare(L"") == 0 && devFound)
					{
						CHK_HR(VariantFromString(slotSerialNo.c_str() , varStage));
						string messageString;
						wstring messageWstring;
						switch (i + 4) {
							case 4:
								messageWstring = L"MCM301: Serial number not found for slot 1. Save " + slotSerialNo + L" as slot 1 stage?";
								break;
							case 5:
								messageWstring = L"MCM301: Serial number not found for slot 2. Save " + slotSerialNo + L" as slot 2 stage?";
								break;
							case 6:
								messageWstring = L"MCM301: Serial number not found for slot 3. Save " + slotSerialNo + L" as slot 3 stage?";
								break;
						}

						const int result = MessageBox(NULL, messageWstring.c_str() , L"ThorMCM301: New serial number from settings file", MB_YESNO);
						if (result == IDYES) {
							CHK_HR(VariantFromString(slotSerialNo.c_str(), varStage));
							pNode->put_nodeValue(varStage);
							CHK_HR(pXMLDom->save(varFileName));
							settingsFileChanged = TRUE;
						}
						else
						{
							return FALSE;
						}
						/*CHK_HR(VariantFromString(slotSerialNo.c_str(), varStage));
						pNode->put_nodeValue(varStage);
						CHK_HR(pXMLDom->save(varFileName));*/
					}
					else if (wstring(val.bstrVal).compare(slotSerialNo) != 0 && devFound)
					{

						wstring messageWstring = L"Serial number mismatch; check cable configuration or check ThorMCM301Settings.xml serial number for slot " + to_wstring(i + 1);
						MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301: Serial number mismatch from settings file", MB_OK);
						return FALSE;

					}
				}
				SysFreeString(bstrQuery);
			}
			if (settingsFileChanged)
			{
				CHK_HR(pXMLDom->save(varFileName));
			}
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM301: SaveSerialNumberToSettingsFile error, failed to load xml");
			LogMessage(errMsg, ERROR_EVENT);
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

//Compare the name of the card read from the device and check if it matches the type of slot that it is supposed to be
long MCM301XML::VerifySlotCards(Mcm301Params* mcm301Params, ScopeType& scopeType)
{
	long ret = TRUE;
	//The +/- 4 is used because the TOTAL_CARD_SLOTS are still 7 in the firmware but we want to display them as slots 1-3 in the xml
	/*
	for (int i = 0; i < TOTAL_CARD_SLOTS - 4; i++)
	{
		string slotName(mcm301Params->slotName[i + 4], 16);

		//If the card name doesn't match the card that was initially  configured for it, either the cards got moved or the card parameters are corrupted
		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->x_slot_id && (slotName.find('X') == std::string::npos))
		{
			mcm301Params->x_slot_id = 0;
			mcm301Params->xConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the X Axis doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The cards might have been moved or it is missing parameters configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: X Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->y_slot_id && (slotName.find('Y') == std::string::npos))
		{
			mcm301Params->y_slot_id = 0;
			mcm301Params->yConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the Y Axis doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Y Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->z_slot_id && (slotName.find('Z') == std::string::npos))
		{
			mcm301Params->z_slot_id = 0;
			mcm301Params->zConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the Z Axis doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Z Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->r_slot_id && (slotName.find('R') == std::string::npos))
		{
			mcm301Params->r_slot_id = 0;
			mcm301Params->rConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the R Axis doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: R Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->condenser_slot_id && (slotName.find('C') == std::string::npos))
		{
			mcm301Params->condenser_slot_id = 0;
			mcm301Params->condenserConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the Condenser doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Condenser Card", MB_OK);
			ret = FALSE;
		}

		if (i + 4 + CARD_ID_START_ADDRESS == mcm301Params->ze_slot_id && (slotName.find('E') == std::string::npos))
		{
			mcm301Params->ze_slot_id = 0;
			mcm301Params->zeConfigured = FALSE;
			mcm301Params->cardConfigured[i + 4] = FALSE;
			wstring messageWstring = L"The slot configured for the Z Elevator doesn't have the correct card. Please check ThorMCM301Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Z Elevator Card", MB_OK);
			ret = FALSE;
		}
	}
	*/
	//Check for plug-and-play errors and display any error to user (start from i = 4, as MCM301 only uses slot indices 4-6)
	for (int i = 4; i < TOTAL_CARD_SLOTS; i++)
	{
		//Raise general error for configured cards when a pnp status error is detected that is not a serial number mismatch or no device connected error
		if (mcm301Params->pnpErrorRaised[i] == 1 && mcm301Params->serialNumberMismatch[i] == 0 && mcm301Params->noDeviceFound[i] == 0 && mcm301Params->cardConfigured[i])
		{
			wstring messageWstring = L"Stepper Motor Plug-and-Play Status Errors for the device connected to slot " + to_wstring(i - 3) + L". Please check all connections to the MCM301 and stepper motors. \n\nIf error persists please contact Thorlabs customer support.\n";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Plug-and-Play Status Error", MB_OK);
			mcm301Params->cardConfigured[i] = FALSE;
			if (mcm301Params->x_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->xConfigured = FALSE;

			else if (mcm301Params->y_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->yConfigured = FALSE;

			else if (mcm301Params->z_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->zConfigured = FALSE;
		}
		//Raise error for configured cards when there is a serial number mismatch detected and there is not a no device connected error
		else if (mcm301Params->serialNumberMismatch[i] == 1 && mcm301Params->noDeviceFound[i] == 0 && mcm301Params->cardConfigured[i])
		{
			wstring messageWstring = L"Stepper Motor Serial Number Mismatch for the device connected to slot " + to_wstring(i - 3) + L". Please check if the stepper motor cables have been swapped. \n\nIf error persists please contact Thorlabs customer support.\n";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: Serial Number Mismatch", MB_OK);
			mcm301Params->cardConfigured[i] = FALSE;
			
			if (mcm301Params->x_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->xConfigured = FALSE;

			else if (mcm301Params->y_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->yConfigured = FALSE;

			else if (mcm301Params->z_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->zConfigured = FALSE;
		}
		//Raise error for configured cards when there is no device connected
		else if (mcm301Params->noDeviceFound[i] == 1 && mcm301Params->cardConfigured[i]) 
		{
			wstring messageWstring = L"Stepper Motor No Device Connected to slot " + to_wstring(i - 3) + L". Please check the connection from the stepper motor to the MCM301. \n\nIf error persists please contact Thorlabs customer support.\n";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM301 Error: No Device Connected", MB_OK);
			mcm301Params->cardConfigured[i] = FALSE;

			if (mcm301Params->x_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->xConfigured = FALSE;

			else if (mcm301Params->y_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->yConfigured = FALSE;

			else if (mcm301Params->z_slot_id - CARD_ID_START_ADDRESS == i)
				mcm301Params->zConfigured = FALSE;
		}
	}
	return ret;
}
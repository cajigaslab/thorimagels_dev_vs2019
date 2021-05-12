#include "MCM6000XML.h"

// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)

// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)

/// <summary>
/// Initializes a new instance of the <see cref="MCM6000XML"/> class.
/// </summary>
MCM6000XML::MCM6000XML()
{
	/*_xmlObj = NULL;
	_currentPathAndFile[0] = NULL;*/
}

/// <summary>
/// Finalizes an instance of the <see cref="MCM6000XML"/> class.
/// </summary>
MCM6000XML::~MCM6000XML()
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

void MCM6000XML::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

//Using Brian Kernighan’s Algorithm
unsigned long MCM6000XML::CountSetBits(int value)
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

bool MCM6000XML::IsSlotIdValid(UCHAR id)
{
	return (id < CARD_ID_START_ADDRESS + TOTAL_CARD_SLOTS && id >= CARD_ID_START_ADDRESS);
}


long MCM6000XML::ReadSettingsFile(Mcm6kParams* mcm6kParams, ScopeType& scopeType, string& portId, int& baudRate, string& ftdiPortId, int& ftdiBaudRate, long& ftdiModeEnabled, long& setSlots, unsigned long& numberOfSetSlots)
{
	HRESULT hr = CoInitialize(NULL);
	bool initializedCOM = false;
	scopeType = UPRIGHT;
	mcm6kParams->x_slot_id = 0;
	mcm6kParams->y_slot_id = 0;
	mcm6kParams->z_slot_id = 0;
	mcm6kParams->r_slot_id = 0;
	mcm6kParams->ze_slot_id = 0;
	mcm6kParams->lp_slot_id = 0;
	mcm6kParams->et_slot_id = 0;
	mcm6kParams->inverted_lp_slot_id = 0;
	mcm6kParams->condenser_slot_id = 0;
	mcm6kParams->shutter_slot_id = 0;
	mcm6kParams->piezo_slot_id = 0;
	mcm6kParams->ndd_slot_id = 0;
	mcm6kParams->xConfigured = FALSE;
	mcm6kParams->yConfigured = FALSE;
	mcm6kParams->zConfigured = FALSE;
	mcm6kParams->rConfigured = FALSE;
	mcm6kParams->lightPathConfigured = FALSE;
	mcm6kParams->epiTurretConfigured = FALSE;
	mcm6kParams->zeConfigured = FALSE;
	mcm6kParams->condenserConfigured = FALSE;
	mcm6kParams->piezoConfigured = FALSE;
	mcm6kParams->nddConfigured = FALSE;
	mcm6kParams->shutterConfigured = FALSE;
	mcm6kParams->piezoMode = -1;

	// Only record that we initialized COM if that is what we really did. An error code of RPC_E_CHANGED_MODE means that the call
	// to CoInitialize failed because COM had already been initialized on another mode - which isn't a fatal condition and so in this
	// case we don't want to call CoUninitialize
	if (FAILED(hr))
	{
		if (hr != RPC_E_CHANGED_MODE)
		{
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: ReadSettingsFile error, could not initialize COM");
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
		CHK_HR(VariantFromString(L"ThorMCM6000Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));
		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM6000Settings/SlotLayout/@slot");
			IXMLDOMNode* pNode = NULL;
			BSTR bstrQuery;

			for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);

				if (pNode)
				{
					pNode->get_nodeValue(&val);

					if (wstring(val.bstrVal).compare(L"X") == 0)
					{
						mcm6kParams->x_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->xConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Y") == 0)
					{
						mcm6kParams->y_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->yConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Z") == 0)
					{
						mcm6kParams->z_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->zConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"R") == 0)
					{
						mcm6kParams->r_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->rConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"LP") == 0)
					{
						mcm6kParams->lp_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->lightPathConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"EpiTurret") == 0)
					{
						mcm6kParams->et_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->epiTurretConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"InvertedLP") == 0)
					{
						mcm6kParams->inverted_lp_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->lightPathConfigured = TRUE;
						scopeType = INVERTED;
					}
					if (wstring(val.bstrVal).compare(L"Shutter") == 0)
					{
						mcm6kParams->shutter_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->shutterConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Condenser") == 0)
					{
						mcm6kParams->condenser_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->condenserConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"ZElevator") == 0)
					{
						mcm6kParams->ze_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->zeConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"Piezo") == 0)
					{
						mcm6kParams->piezo_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->piezoConfigured = TRUE;
					}
					if (wstring(val.bstrVal).compare(L"AFSwitch") == 0)
					{
						mcm6kParams->ndd_slot_id = CARD_ID_START_ADDRESS + i;
						mcm6kParams->nddConfigured = TRUE;
					}
				}
				SysFreeString(bstrQuery);
			}

			//Check if the SlotID is valid and was configured, enabled the corresponding bit on _setSlots
			//:TODO: This should be set when the boards are checked for what type of board they are, instead of doing it the other way around
			setSlots |= (IsSlotIdValid(mcm6kParams->x_slot_id)) ? 1 << (mcm6kParams->x_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->y_slot_id)) ? 1 << (mcm6kParams->y_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->z_slot_id)) ? 1 << (mcm6kParams->z_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->r_slot_id)) ? 1 << (mcm6kParams->r_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->ze_slot_id)) ? 1 << (mcm6kParams->ze_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->lp_slot_id)) ? 1 << (mcm6kParams->lp_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->et_slot_id)) ? 1 << (mcm6kParams->et_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->inverted_lp_slot_id)) ? 1 << (mcm6kParams->inverted_lp_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->condenser_slot_id)) ? 1 << (mcm6kParams->condenser_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->shutter_slot_id)) ? 1 << (mcm6kParams->shutter_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->piezo_slot_id)) ? 1 << (mcm6kParams->piezo_slot_id - CARD_ID_START_ADDRESS) : setSlots;
			setSlots |= (IsSlotIdValid(mcm6kParams->ndd_slot_id)) ? 1 << (mcm6kParams->ndd_slot_id - CARD_ID_START_ADDRESS) : setSlots;

			numberOfSetSlots = CountSetBits(setSlots);

			////////////////////////////////////////////////////////////
			// --------------------------- max --------------------------- 
			bq = L"/MCM6000Settings/XRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@max";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@maxMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserMax = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- min --------------------------- 

			bq = L"/MCM6000Settings/XRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@min";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@minMM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserMin = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- threshold --------------------------- 

			bq = L"/MCM6000Settings/XRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rThreshold = _wtof(val.bstrVal) / 1e3;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@threshold";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserThreshold = _wtof(val.bstrVal);
			}
			SysFreeString(bstrQuery);

			// --------------------------- invert --------------------------- 

			bq = L"/MCM6000Settings/XRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@invert";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserInvert = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// --------------------------- Move by Threshold --------------------------- 
			bq = L"/MCM6000Settings/XRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@moveByThresholduM";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserMoveByThreshold = _wtof(val.bstrVal) / static_cast<double>(UM_TO_MM);
			}
			SysFreeString(bstrQuery);

			// --------------------------- pid --------------------------- 
			/* Might be used later
			bq = L"/MCM6000Settings/XRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@pid";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserPidEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			// --------------------------- PidKickout ---------------------------
			bq = L"/MCM6000Settings/XRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->xPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/YRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->yPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/ZRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->zPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/RRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->rPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/CondenserRangeConfig/@pidKickout";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				mcm6kParams->condenserPidKickoutEnable = (_wtoi(val.bstrVal) == 1) ? true : false;
			}
			SysFreeString(bstrQuery);
			*/
			///////////////////////////////////////////////////////////////////

			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;

			//*TODO* _serialNumber is not currently used. We can use it if there are multiple
			// MCM6000 connected and the user can pick one, but this would need to be discussed.
			/*
			bq = L"/MCM6000Settings/DeviceInfo/@serialNumber";
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

			bq = L"/MCM6000Settings/DeviceInfo/@baudRate";
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

			bq = L"/MCM6000Settings/DeviceInfo/@portId";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				if (SysStringLen(val.bstrVal) != 0)
					portId = converterX.to_bytes(wstring(val.bstrVal));;
			}
			SysFreeString(bstrQuery);

			/*bq = L"/MCM6000Settings/DeviceInfo/@scopeType";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				_scopeType = (t == 1) ? INVERTED : UPRIGHT;
			}
			SysFreeString(bstrQuery);*/

			bq = L"/MCM6000Settings/FTDIsettings/@portID";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				if (SysStringLen(val.bstrVal) != 0)
					ftdiPortId = converterX.to_bytes(wstring(val.bstrVal));;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/FTDIsettings/@baudRate";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				if (t > 0)
					ftdiBaudRate = t;
			}
			SysFreeString(bstrQuery);

			bq = L"/MCM6000Settings/FTDIsettings/@FTDIMode";
			bstrQuery = SysAllocString(bq.c_str());
			pXMLDom->selectSingleNode(bstrQuery, &pNode);
			if (pNode)
			{
				pNode->get_nodeValue(&val);
				int t = _wtoi(val.bstrVal);
				ftdiModeEnabled = (t == 1) ? TRUE : FALSE;
			}
			SysFreeString(bstrQuery);
		}
		else
		{
			// Failed to load xml, get last parsing error
			CHK_HR(pXMLDom->get_parseError(&pXMLErr));
			CHK_HR(pXMLErr->get_reason(&bstrErr));
			wchar_t errMsg[MSG_SIZE];
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: ReadSettingsFile error, failed to load xml");
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

long MCM6000XML::SaveSlotNameToSettingsFile(Mcm6kParams* mcm6kParams, long& settingsFileChanged)
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
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: SaveSlotNameToSettingsFile error, could not initialize COM");
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
		CHK_HR(VariantFromString(L"ThorMCM6000Settings.xml", varFileName));
		CHK_HR(pXMLDom->load(varFileName, &varStatus));
		if (varStatus == VARIANT_TRUE)
		{
			CHK_HR(pXMLDom->get_xml(&bstrXML));
			IXMLDOMElement* pElem = NULL;
			pXMLDom->get_documentElement(&pElem);
			wstring bq(L"/MCM6000Settings/SlotLayout/@slot");
			IXMLDOMNode* pNode = NULL;
			BSTR bstrQuery;

			for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
			{
				wchar_t n;
				_itow_s(i + 1, &n, sizeof(wchar_t), 10);
				bstrQuery = SysAllocString((bq + n).c_str());

				pXMLDom->selectSingleNode(bstrQuery, &pNode);

				if (pNode)
				{
					pNode->get_nodeValue(&val);
					string slotName(mcm6kParams->slotName[i]);

					//If the slot configuration is empty and the slot name read from the device is not empty
					if (wstring(val.bstrVal).compare(L"") == 0 && slotName.compare("") != 0)
					{
						if (slotName.compare("X") == 0 ||
							slotName.compare("X Axis") == 0 ||
							slotName.compare("PLS X") == 0)
						{
							CHK_HR(VariantFromString(L"X", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("Y") == 0 ||
							slotName.compare("Y Axis") == 0 ||
							slotName.compare("PLS Y") == 0)
						{
							CHK_HR(VariantFromString(L"Y", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("Z") == 0 ||
							slotName.compare("Z Axis") == 0 ||
							slotName.compare("Inverted Z Axis") == 0 ||
							slotName.compare("PLS Z") == 0)
						{
							CHK_HR(VariantFromString(L"Z", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("R") == 0 ||
							slotName.compare("R Axis") == 0)
						{
							CHK_HR(VariantFromString(L"R", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("C") == 0 ||
							slotName.compare("PLS C") == 0 ||
							slotName.compare("Condenser") == 0)
						{
							CHK_HR(VariantFromString(L"Condenser", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("E") == 0 ||
							slotName.compare("E Axis") == 0 ||
							slotName.compare("Z Elevator") == 0)
						{
							CHK_HR(VariantFromString(L"ZElevator", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("Light Path") == 0)
						{
							if (CardTypes::ST_Invert_Stepper_BISS_type == mcm6kParams->cardType[i] || CardTypes::ST_Invert_Stepper_SSI_type == mcm6kParams->cardType[i])
							{
								CHK_HR(VariantFromString(L"InvertedLP", varStage));
							}
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("EPI") == 0 ||
							slotName.compare("Epi Turret") == 0)
						{
							CHK_HR(VariantFromString(L"EpiTurret", varStage));
							pNode->put_nodeValue(varStage);
						}

						if (slotName.compare("Inverted AF Swit") == 0 ||
							slotName.compare("AF Switch") == 0 ||
							slotName.compare("NDD") == 0)
						{
							CHK_HR(VariantFromString(L"AFSwitch", varStage));
							pNode->put_nodeValue(varStage);
						}

						settingsFileChanged = TRUE;
					}
					//For Shutter, the slot name comes back as empty, instead we need check the card type and write it on the settings file
					else if (wstring(val.bstrVal).compare(L"") == 0 && (CardTypes::Shutter_type == mcm6kParams->cardType[i] || CardTypes::Shutter_4_type == mcm6kParams->cardType[i]))
					{
						CHK_HR(VariantFromString(L"Shutter", varStage));
						pNode->put_nodeValue(varStage);
						settingsFileChanged = TRUE;
					}
					//For Piezo, the slot name comes back as empty, instead we need check the card type and write it on the settings file. Note: Piezo card type needs to be OR'ed with 0x8000 sometimes
					else if (wstring(val.bstrVal).compare(L"") == 0 && (CardTypes::Piezo_Type == mcm6kParams->cardType[i] || (CardTypes::Piezo_Type | STATIC_CARD_SLOT_MASK) == mcm6kParams->cardType[i]))
					{
						CHK_HR(VariantFromString(L"Piezo", varStage));
						pNode->put_nodeValue(varStage);
						settingsFileChanged = TRUE;
					}
					//For Bergamo Light Path, the slot name comes back as empty, instead we need check the card type (Slider IO) and write it on the settings file.
					else if (wstring(val.bstrVal).compare(L"") == 0 && CardTypes::Slider_IO_type == mcm6kParams->cardType[i])
					{
						CHK_HR(VariantFromString(L"LP", varStage));
						pNode->put_nodeValue(varStage);
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
			StringCbPrintfW(errMsg, MSG_SIZE, L"MCM6000: SaveSlotNameToSettingsFile error, failed to load xml");
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
long MCM6000XML::VerifySlotCards(Mcm6kParams* mcm6kParams, ScopeType& scopeType)
{
	long ret = TRUE;
	for (int i = 0; i < TOTAL_CARD_SLOTS; i++)
	{
		string slotName(mcm6kParams->slotName[i]);

		//If the card name doesn't match the card that was initially  configured for it, either the cards got moved or the card parameters are corrupted
		if (i + CARD_ID_START_ADDRESS == mcm6kParams->x_slot_id && (0 != slotName.compare("X") && 0 != slotName.compare("X Axis") && 0 != slotName.compare("PLS X")))
		{
			mcm6kParams->x_slot_id = 0;
			mcm6kParams->xConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the X Axis doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The cards might have been moved or it is missing parameters configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: X Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->y_slot_id && (0 != slotName.compare("Y") && 0 != slotName.compare("Y Axis") && 0 != slotName.compare("PLS Y")))
		{
			mcm6kParams->y_slot_id = 0;
			mcm6kParams->yConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Y Axis doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Y Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->z_slot_id && (0 != slotName.compare("Z") && 0 != slotName.compare("Z Axis") && 0 != slotName.compare("PLS Z") && 0 != slotName.compare("Inverted Z Axis")))
		{
			mcm6kParams->z_slot_id = 0;
			mcm6kParams->zConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Z Axis doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Z Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->r_slot_id && (0 != slotName.compare("R") && 0 != slotName.compare("R Axis")))
		{
			mcm6kParams->r_slot_id = 0;
			mcm6kParams->rConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the R Axis doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: R Axis Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->condenser_slot_id && (0 != slotName.compare("C") && 0 != slotName.compare("Condenser") && 0 != slotName.compare("PLS C")))
		{
			mcm6kParams->condenser_slot_id = 0;
			mcm6kParams->condenserConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Condenser doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Condenser Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->ze_slot_id && (0 != slotName.compare("E") && 0 != slotName.compare("E Axis") && 0 != slotName.compare("Z Elevator")))
		{
			mcm6kParams->ze_slot_id = 0;
			mcm6kParams->zeConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Z Elevator doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Z Elevator Card", MB_OK);
			ret = FALSE;
		}

		if (ScopeType::INVERTED == scopeType && (i + CARD_ID_START_ADDRESS == mcm6kParams->lp_slot_id && (0 != slotName.compare("Light Path"))))
		{
			mcm6kParams->lp_slot_id = 0;
			mcm6kParams->lightPathConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Ligh Path doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Light Path Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->et_slot_id && (0 != slotName.compare("EPI") && 0 != slotName.compare("Epi Turret")))
		{
			mcm6kParams->et_slot_id = 0;
			mcm6kParams->epiTurretConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Epi Turret doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Epi Turret Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->ndd_slot_id && (0 != slotName.compare("Inverted AF Swit") && 0 != slotName.compare("AF Switch") && 0 != slotName.compare("NDD")))
		{
			mcm6kParams->ndd_slot_id = 0;
			mcm6kParams->nddConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Auto Focus Switch doesn't have the correct card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Auto Focus Switch Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->shutter_slot_id && (CardTypes::Shutter_type != mcm6kParams->cardType[i] && CardTypes::Shutter_4_type != mcm6kParams->cardType[i]))
		{
			mcm6kParams->shutter_slot_id = 0;
			mcm6kParams->shutterConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Shutter doesn't have the correct type of card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Shutter Card", MB_OK);
			ret = FALSE;
		}

		if (i + CARD_ID_START_ADDRESS == mcm6kParams->piezo_slot_id && (CardTypes::Piezo_Type != mcm6kParams->cardType[i] && (CardTypes::Piezo_Type | STATIC_CARD_SLOT_MASK) != mcm6kParams->cardType[i]))
		{
			mcm6kParams->piezo_slot_id = 0;
			mcm6kParams->piezoConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Piezo doesn't have the correct type of card. Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Piezo Card", MB_OK);
			ret = FALSE;
		}

		if (ScopeType::UPRIGHT == scopeType && (i + CARD_ID_START_ADDRESS == mcm6kParams->lp_slot_id && CardTypes::Slider_IO_type != mcm6kParams->cardType[i]))
		{
			mcm6kParams->lp_slot_id = 0;
			mcm6kParams->lightPathConfigured = FALSE;
			wstring messageWstring = L"The slot configured for the Light Path doesn't have the correct type of card (Slider IO). Please check ThorMCM6000Settings.xml and make sure SlotLayout is configured correctly. The card might have been moved or it is missing a parameter configuration. \n\nIf error persists please contact techsupport@thorlabs.com.";
			MessageBox(NULL, messageWstring.c_str(), L"ThorMCM6000 Error: Light Path slider Card", MB_OK);
			ret = FALSE;
		}
	}
	return ret;
}
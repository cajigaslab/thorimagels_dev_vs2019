// ThorDetector.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorDetector.h"
#include "ThorDetectorXML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>

#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// Prevents a default instance of the <see cref="ThorDetector"/> class from being created.
/// </summary>
ThorDetector::ThorDetector()
{
	_sleepTimeAfterMoveComplete = 0;
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		_deviceDetected[i] = FALSE;
		_defaultGain[i] = 0;
		_detectorType[i] = 0;
		_gainMin[i] = 0;
		_gainMax[i] = 0;
		_offsetMin[i] = 0;
		_offsetMax[i] = 0;
		_bandwidthMin[i] = 0;
		_bandwidthMax[i] = 0;
		_atenuationMin[i] = 0;
		_atenuationMax[i] = 0;
		_gainSliderMin[i] = 0;
		_gainSliderMax[i] = 100;
		_offsetSliderMin[i] = 0;
		_offsetSliderMax[i] = 100;
	}
	_connectedPMTs = 0;
	_deviceDetected[DEVICE_NUM] = FALSE;
	_errMsg[0] = NULL;
	_baudRate = 460800;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorDetector"/> class.
/// </summary>
ThorDetector::~ThorDetector()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		_defaultGain[i] = 0;
	}
}

bool ThorDetector::_instanceFlag = false;

auto_ptr<ThorDetector> ThorDetector::_single(new ThorDetector());
const string ThorDetector::_deviceSignature[DEVICE_NUM] = { "Detector1", "Detector2", "Detector3", "Detector4", "Detector5", "Detector6" };
const long ThorDetector::_pmtSelect[DEVICE_NUM] = { PMT1, PMT2, PMT3, PMT4, PMT5, PMT6 };
const vector<wstring> ThorDetector::_listOfTypes = { L"PMT Old Bootloader", L"PMT1000", L"PMT2100", L"PMT2106", L"APD", L"Photodiode", L"HPD1000", L"SIPM100", L"PMT2110", L"PMT3100" };

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorDetector *.</returns>
ThorDetector* ThorDetector::getInstance()
{
	if (!_instanceFlag)
	{
		_single.reset(new ThorDetector());
		_instanceFlag = true;
		return _single.get();
	}
	else
	{
		return _single.get();
	}
}


/// <summary>
/// Finds the devices.
/// </summary>
/// <param name="deviceCount">The device count.</param>
/// <returns>long.</returns>
long ThorDetector::FindDevices(long& deviceCount)
{
	long ret = TRUE;
	long portID = 0;
	int index = 0;

	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		try
		{
			portID = 0;
			//Get portID, etc from hardware ThorDetectorSettings.xml
			auto_ptr<ThorDetectorXML> pSetup(new ThorDetectorXML());
			long type = 0;
			pSetup->GetDeviceConnectionInfo(_deviceSignature[i], portID, _baudRate, _settingsSerialNumber[i], type);
		}
		catch (...)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"Unable to locate ThorDetectorSettings.xml file");
			LogMessage(_errMsg, ERROR_EVENT);
		}
	}

	for (int i = 0; i < _snList.size(); i++)
	{
		_snList[i].clear();
	}

	_snList.clear();

	//Search the Registry for any connected devices associated with this PID and VID.
	//The list of serial number needs to be pushed back in the order of the list of detector types
	vector<string> empty;
	_snList.push_back(empty); //type 0 PMT_WITH_OLD_BOOTLOADER
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PMT1000_PID)); // type 1 BIALKALI
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PMT2100_PID)); // type 2 GAASP_COMPACT
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PMT2106_PID)); // type 3 GAASP_WITH_SHUTTER
	_snList.push_back(empty); // type 4 APD
	_snList.push_back(empty); // type 5 PHOTODIODE
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_HPD1000_PID)); // type 6 HPD
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_SIPM100_PID)); // type 7 SIPM
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PMT2110_PID)); // type 8 GAASP_FAST
	_snList.push_back(FindSerialNumbersInRegistry(THORLABS_VID, THORLABS_PMT3100_PID));	// type 9 GAASP_3P

	for (int i = 0; i < _snList.size(); i++)
	{
		if (0 < _snList[i].size())
		{
			_deviceDetected[DEVICE_NUM] = TRUE;
			deviceCount = 1;
			ret = TRUE;
			for (int h = 0; h < _snList[i].size(); h++)
			{
				if (index < DEVICE_NUM)
				{
					_deviceDetected[index++] = TRUE;
				}
				else
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: Find device found more than 6 PMTs connected");
					logDll->TLTraceEvent(ERROR_EVENT, 1, _errMsg);
				}
			}
		}
		else
		{
			wstring message = L"ThorDetector: Unable to find any serial device connected in the registry associated with this PID and VID " + _listOfTypes[i];
			StringCbPrintfW(_errMsg, MSG_SIZE, message.c_str());
			logDll->TLTraceEvent(INFORMATION_EVENT, 1, _errMsg);
		}
	}

	return ret;
}


/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorDetector::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	string devicePID;
	wstring port = L"";
	string serialNum = "";
	string type = "";
	vector<string>::iterator it;
	map<string, long> pmtAlreadyFound;
	map<string, long>::iterator mapIt;
	auto_ptr<ThorDetectorXML> pSetup(new ThorDetectorXML());

	//Check if any device detected before continuing
	if (FALSE == _deviceDetected[DEVICE_NUM])
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}

	//First pass: Iterate through the list if PMTs that have a serial number set in ThorDetectorSettings.xml
	//If there is a serial number alredy written on it, connect to it.
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		if (!_settingsSerialNumber[i].empty())
		{
			// Iterate through the list of serial numbers until the detector with the serial number in the settings file is found
			for (int h = 0; h < _snList.size(); h++)
			{
				switch (h)
				{
				case (long)DetectorTypes::PMT1000:
					devicePID = THORLABS_PMT1000_PID;
					break;
				case (long)DetectorTypes::PMT2100:
					devicePID = THORLABS_PMT2100_PID;
					break;
				case (long)DetectorTypes::PMT2106:
					devicePID = THORLABS_PMT2106_PID;
					break;
				case (long)DetectorTypes::HPD1000:
					devicePID = THORLABS_HPD1000_PID;
					break;
				case (long)DetectorTypes::SIPM100:
					devicePID = THORLABS_SIPM100_PID;
					break;
				case (long)DetectorTypes::PMT2110:
					devicePID = THORLABS_PMT2110_PID;
					break;
				case (long)DetectorTypes::PMT3100:
					devicePID = THORLABS_PMT3100_PID;
					break;
				}
				for (it = _snList[h].begin(); it != _snList[h].end(); ++it)
				{
					serialNum = it->data();
					if (_settingsSerialNumber[i] == serialNum)
					{
						port = FindCOMPortInRegistry(THORLABS_VID, devicePID, serialNum);
						if (ConnectToComPort(port, i))
						{
							pmtAlreadyFound.insert(pair<string, long>(serialNum, TRUE));
							_detectorType[i] = h;
							type = ConvertWStringToString(_listOfTypes[_detectorType[i]]);
							pSetup->SetDetectorType(_deviceSignature[i], type);
							ret = TRUE;
						}
					}
				}
			}
		}
	}

	//Second pass: Iterate through all the detectors that don't have a serial number set in ThorDetectorSettings.xml
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		if (_settingsSerialNumber[i].empty())
		{
			// Iterate through the list of serial numbers and connect to any detector that hasn't been connected to yet
			for (int h = 0; h < _snList.size(); h++)
			{
				switch (h)
				{
				case (long)DetectorTypes::PMT3100:
					devicePID = THORLABS_PMT3100_PID;
					break;
				case (long)DetectorTypes::PMT2110:
					devicePID = THORLABS_PMT2110_PID;
					break;
				case (long)DetectorTypes::HPD1000:
					devicePID = THORLABS_HPD1000_PID;
					break;
				case (long)DetectorTypes::SIPM100:
					devicePID = THORLABS_SIPM100_PID;
					break;
				case (long)DetectorTypes::PMT1000:
					devicePID = THORLABS_PMT1000_PID;
					break;
				case (long)DetectorTypes::PMT2100:
					devicePID = THORLABS_PMT2100_PID;
					break;
				case (long)DetectorTypes::PMT2106:
					devicePID = THORLABS_PMT2106_PID;
					break;
				}
				for (it = _snList[h].begin(); it != _snList[h].end(); ++it)
				{
					serialNum = it->data();
					mapIt = pmtAlreadyFound.find(serialNum);
					if (mapIt == pmtAlreadyFound.end())
					{
						port = FindCOMPortInRegistry(THORLABS_VID, devicePID, serialNum);
						if (ConnectToComPort(port, i))
						{
							if (pSetup->SetSerialNumber(_deviceSignature[i], serialNum))
							{
								pmtAlreadyFound.insert(pair<string, long>(serialNum, TRUE));
								_settingsSerialNumber[i] = serialNum;
								_detectorType[i] = h;
								type = ConvertWStringToString(_listOfTypes[_detectorType[i]]);
								pSetup->SetDetectorType(_deviceSignature[i], type);
								ret = TRUE;
							}
						}
					}
				}
			}
		}
	}

	RetrieveDevicesInfo();
	BuildParamTable();
	return ret;
}

long ThorDetector::ConnectToComPort(wstring portName, long pmtIndex)
{
	long portID = 0;
	long ret = FALSE;
	long count = 0;
	long numberOfRetries = 5;
	try
	{
		if (!portName.empty())
		{
			// port wstring returned should be more than 3 characters
			if (3 >= portName.size())
			{
				wstring messageWstring = L"ThorDetector: Returned port ID doesn't match required format to continue. Format should be: COM##, format returned is: " + portName;
				StringCbPrintfW(_errMsg, MSG_SIZE, messageWstring.c_str());
				LogMessage(_errMsg, ERROR_EVENT);
				return FALSE;
			}
			// Open the serial port after getting the number
			wstring portNumber = portName.substr(3, portName.size());
			portID = stoi(portNumber);
			if (16 < portID && 56 > portID)
			{
				wchar_t message[512];
				StringCbPrintfW(message, 512, L"ThorDetector: WARNING One of the detectors COM port is unavailable. Port: COM%d. Please change the COM port number of this detector to one that is not between 17 and 56 to avoid any conflicts in ThorImageLS. \nRecommended ports COM56 - COM62.", portID);
				MessageBox(NULL, message, L"ThorDetector COM port warning", MB_OK | MB_DEFAULT_DESKTOP_ONLY | MB_ICONWARNING);
			}
			while (FALSE == _serialPort[pmtIndex].Open(portID, _baudRate) || numberOfRetries <= count)
			{
				count++;
				Sleep(18);
			}
			if (numberOfRetries <= count)
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector SelectDevice could not open serial port: %d", portID);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			else
			{
				_deviceDetected[DEVICE_NUM] = _deviceDetected[pmtIndex] = TRUE;
				_connectedPMTs |= _pmtSelect[pmtIndex];
				ret = TRUE;
			}
		}
	}
	catch (...)
	{
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: Unable to connect to device");
		LogMessage(_errMsg, ERROR_EVENT);
	}
	return ret;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorDetector::LogMessage(wchar_t* logMsg, long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::TeardownDevice()
{
	SetParam(IDevice::PARAM_PMT1_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT2_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT3_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT4_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT5_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT6_ENABLE, FALSE);

	SetupPosition();
	StartPosition();

	for (int i = 0; i < DEVICE_NUM; i++)
	{
		if (_deviceDetected[i] == TRUE)
		{
			_serialPort[i].Close();
			_deviceDetected[i] = FALSE;
		}
	}
	_deviceDetected[DEVICE_NUM] = FALSE;
	_connectedPMTs = FALSE;

	DestroyParamTable();
	return TRUE;
}

/// <summary>
/// Gets the parameter information.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="paramType">Type of the parameter.</param>
/// <param name="paramAvailable">The parameter available.</param>
/// <param name="paramReadOnly">The parameter read only.</param>
/// <param name="paramMin">The parameter minimum.</param>
/// <param name="paramMax">The parameter maximum.</param>
/// <param name="paramDefault">The parameter default.</param>
/// <returns>long.</returns>
long ThorDetector::GetParamInfo(const long paramID, long& paramType, long& paramAvailable, long& paramReadOnly, double& paramMin, double& paramMax, double& paramDefault)
{
	if (NULL != _tableParams[paramID])
	{
		if (_tableParams[paramID]->GetParamID() == paramID)
		{
			paramType = _tableParams[paramID]->GetParamType();
			paramAvailable = _tableParams[paramID]->GetParamAvailable();
			paramReadOnly = _tableParams[paramID]->GetParamReadOnly();
			paramMin = _tableParams[paramID]->GetParamMin();
			paramMax = _tableParams[paramID]->GetParamMax();
			paramDefault = _tableParams[paramID]->GetParamDefault();
			return TRUE;
		}
	}

	paramAvailable = FALSE;
	return FALSE;
}

/// <summary>
/// Gets the Devices Parameters.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::GetDeviceParameters()
{
	double firmwareVersion = 0.0;
	return TRUE;
}


/// <summary>
/// Sets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorDetector::SetParam(const long paramID, const double param)
{
	double value = param;
	if (NULL != _tableParams[paramID])
	{
		if (_tableParams[paramID]->GetParamID() == paramID)
		{
			if (FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if ((_tableParams[paramID]->GetParamMin() <= param) && (_tableParams[paramID]->GetParamMax() >= param))
			{
				_tableParams[paramID]->UpdateParam(param);
				return TRUE;
			}
			else
			{
				wstring message = L"ThorDetector SetParam failed. paramID: %d param: " + _tableParams[paramID]->GetParameterString(), paramID;
				StringCbPrintfW(_errMsg, MSG_SIZE, message.c_str());
				LogMessage(_errMsg, INFORMATION_EVENT);
				return FALSE;
			}
		}
	}

	return FALSE;
}

/// <summary>
/// Gets the parameter.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="param">The parameter.</param>
/// <returns>long.</returns>
long ThorDetector::GetParam(const long paramID, double& param)
{
	static USHORT returnedCommand;
	if (NULL != _tableParams[paramID])
	{
		if (FALSE == _tableParams[paramID]->GetParamAvailable())
		{
			return FALSE;
		}
		switch (_tableParams[paramID]->GetParamID())
		{
		case PARAM_PMT1_SAFETY:
		case PARAM_PMT2_SAFETY:
		case PARAM_PMT3_SAFETY:
		case PARAM_PMT4_SAFETY:
		case PARAM_PMT5_SAFETY:
		case PARAM_PMT6_SAFETY:
		{
			double status = 0;
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), _tableParams[paramID]->GetDeviceIndex(), status, TRUE, returnedCommand);
			if (MGMSG_GET_PMTTRIP == returnedCommand)
			{
				param = (1 == status) ? FALSE : TRUE;
				return TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: GetParam PARAM_PMT_SAFETY, incorrect header. Expected MGMSG_GET_PMTTRIP %d returned %d.", MGMSG_GET_PMTTRIP, returnedCommand);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			return FALSE;
		}
		case PARAM_PMT1_GAIN_POS_CURRENT_VOLTS:
		case PARAM_PMT2_GAIN_POS_CURRENT_VOLTS:
		case PARAM_PMT3_GAIN_POS_CURRENT_VOLTS:
		case PARAM_PMT4_GAIN_POS_CURRENT_VOLTS:
		case PARAM_PMT5_GAIN_POS_CURRENT_VOLTS:
		case PARAM_PMT6_GAIN_POS_CURRENT_VOLTS:
		{
			double value = 0;
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), _tableParams[paramID]->GetDeviceIndex(), value, FALSE, returnedCommand);
			if (MGMSG_GET_PMT_ATTENUATION == returnedCommand)
			{
				param = value;
				return TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: GetParam PARAM_PMT_GAIN_POS_CURRENT_VOLTS, incorrect header. Expected MGMSG_GET_PMT_ATTENUATION %d returned %d.", MGMSG_GET_PMT_ATTENUATION, returnedCommand);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			return FALSE;
		}
		case PARAM_PMT1_OUTPUT_OFFSET_CURRENT:
		case PARAM_PMT2_OUTPUT_OFFSET_CURRENT:
		case PARAM_PMT3_OUTPUT_OFFSET_CURRENT:
		case PARAM_PMT4_OUTPUT_OFFSET_CURRENT:
		case PARAM_PMT5_OUTPUT_OFFSET_CURRENT:
		case PARAM_PMT6_OUTPUT_OFFSET_CURRENT:
		{
			double value = 0;
			long pmtIndex = _tableParams[paramID]->GetDeviceIndex();
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), _tableParams[paramID]->GetDeviceIndex(), value, FALSE, returnedCommand);
			if (MGMSG_GET_PMT_OFFSET == returnedCommand)
			{
				param = Round((value - _offsetMin[pmtIndex]) * _offsetSliderMax[pmtIndex] / ((double)_offsetMax[pmtIndex] - _offsetMin[pmtIndex]), 3);
				return TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: GetParam PARAM_PMT_OUTPUT_OFFSET_CURRENT, incorrect header. Expected MGMSG_GET_PMT_OFFSET %d returned %d.", MGMSG_GET_PMT_OFFSET, returnedCommand);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			return FALSE;
		}
		case PARAM_PMT1_BANDWIDTH_POS_CURRENT:
		case PARAM_PMT2_BANDWIDTH_POS_CURRENT:
		case PARAM_PMT3_BANDWIDTH_POS_CURRENT:
		case PARAM_PMT4_BANDWIDTH_POS_CURRENT:
		case PARAM_PMT5_BANDWIDTH_POS_CURRENT:
		case PARAM_PMT6_BANDWIDTH_POS_CURRENT:
		{
			double value = 0;
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), _tableParams[paramID]->GetDeviceIndex(), value, FALSE, returnedCommand);
			if (MGMSG_GET_PMT_FREQ == returnedCommand)
			{
				param = value;
				return TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: GetParam PARAM_PMT_BANDWIDTH_POS_CURRENT, incorrect header. Expected MGMSG_GET_PMT_FREQ %d returned %d.", MGMSG_GET_PMT_FREQ, returnedCommand);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			return FALSE;
		}
		case PARAM_PMT1_SATURATIONS:
		case PARAM_PMT2_SATURATIONS:
		case PARAM_PMT3_SATURATIONS:
		case PARAM_PMT4_SATURATIONS:
		case PARAM_PMT5_SATURATIONS:
		case PARAM_PMT6_SATURATIONS:
		{
			double value = 0;
			ExecuteCmd(_tableParams[paramID]->GetCmdBytes(), _tableParams[paramID]->GetDeviceIndex(), value, FALSE, returnedCommand);
			if (MGMSG_GET_PMT_SATURATION_COUNT == returnedCommand)
			{
				param = value;
				return TRUE;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector: GetParam PARAM_PMT_SATURATIONS, incorrect header. Expected MGMSG_GET_PMT_SATURATION_COUNT %d returned %d.", MGMSG_GET_PMT_SATURATION_COUNT, returnedCommand);
				LogMessage(_errMsg, ERROR_EVENT);
			}
			return FALSE;
		}
		case PARAM_PMT1_DETECTOR_TYPE:
		case PARAM_PMT2_DETECTOR_TYPE:
		case PARAM_PMT3_DETECTOR_TYPE:
		case PARAM_PMT4_DETECTOR_TYPE:
		case PARAM_PMT5_DETECTOR_TYPE:
		case PARAM_PMT6_DETECTOR_TYPE:
		{
			param = _detectorType[_tableParams[paramID]->GetDeviceIndex()];
			return TRUE;
		}
		case PARAM_CONNECTION_STATUS:
		{
			param = (_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
			return TRUE;
		}
		case PARAM_PMT_OFFSET_STEP_SIZE:
		{
			param = 2.0 * (static_cast<double>(_offsetSliderMax[_tableParams[paramID]->GetDeviceIndex()]) - static_cast<double>(_offsetSliderMin[_tableParams[paramID]->GetDeviceIndex()])) / (static_cast<double>(_offsetMax[_tableParams[paramID]->GetDeviceIndex()]) - static_cast<double>(_offsetMin[_tableParams[paramID]->GetDeviceIndex()]));
			return TRUE;
		}
		default:
			param = _tableParams[paramID]->GetParamVal();
			return TRUE;
		}
	}

	return FALSE;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDetector::SetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Gets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDetector::GetParamBuffer(const long paramID, char* pBuffer, long size)
{
	long ret = TRUE;

	return ret;
}

/// <summary>
/// Sets the parameter string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <returns>long.</returns>
long ThorDetector::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;

	return ret;
}

wstring utf8toUtf16(const string& str)
{
	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), static_cast<long>(str.size()), &buffer[0], static_cast<long>(buffer.size()));
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorDetector::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = TRUE;

	switch (paramID)
	{
	case PARAM_PMT1_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[0]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	case PARAM_PMT2_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[1]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	case PARAM_PMT3_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[2]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	case PARAM_PMT4_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[3]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	case PARAM_PMT5_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[4]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	case PARAM_PMT6_SERIALNUMBER:
	{
		wstring wsTemp = utf8toUtf16(_serialNumber[5]);
		wcscpy_s(str, size, wsTemp.c_str());
	}
	break;
	default:
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::SetupPosition()
{
	for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if ((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if ((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
		{
			(iter->second)->SetParamBool(TRUE);
		}
		else
		{
			(iter->second)->SetParamBool(FALSE);
		}
	}

	return TRUE;
}

/// <summary>
/// Starts the position.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::StartPosition()
{
	long ret = TRUE;

	//iterate through map and set the parameters
	for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if (((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if ((iter->second)->GetParamBool() == TRUE)
		{
			ret = ExecuteCmdParam(iter->second);
			(iter->second)->UpdateParam_C();

			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(_errMsg, MSG_SIZE, L"StartPosition succeeded at paramID: %d", (iter->second)->GetParamID());
			LogMessage(_errMsg, INFORMATION_EVENT);
		}
	}

	return ret;
}

/// <summary>
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorDetector::StatusPosition(long& status)
{
	long ret = TRUE;
	status = IDevice::STATUS_READY;
	return ret;
}

/// <summary>
/// DEPRECATED
/// </summary>
/// <param name="deviceType">Type of the device.</param>
/// <param name="pos">The position.</param>
/// <returns>long.</returns>
long ThorDetector::ReadPosition(DeviceType deviceType, double& pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorDetector::ExecuteCmdParam(ParamInfo* pParamInfo)
{
	long paramID = pParamInfo->GetParamID();
	static USHORT returnedCommand;
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	switch (paramID)
	{
	case PARAM_PMT1_ENABLE:
	case PARAM_PMT2_ENABLE:
	case PARAM_PMT3_ENABLE:
	case PARAM_PMT4_ENABLE:
	case PARAM_PMT5_ENABLE:
	case PARAM_PMT6_ENABLE:
	{
		int paramVal = static_cast<int>(Round(pParamInfo->GetParamVal(), 0));
		std::vector<unsigned char> paramValBytes = GetBytes(paramVal);
		cmd[3] = paramValBytes[0];

		double readBackVal = -1;
		long index = pParamInfo->GetDeviceIndex();

		ExecuteCmd(cmd, index, readBackVal, FALSE, returnedCommand);

		if (TRUE == paramVal)
		{
			Sleep(30);
			long relatedID = pParamInfo->GetRelatedID();
			ParamInfo* pParamInfo2 = _tableParams[static_cast<unsigned int>(relatedID)];
			ExecuteCmdParam(pParamInfo2);
		}
		else
		{
			//clear the safety trip after disabling the PMT
			Sleep(30);
			ExecuteCmd(_tableParams[PARAM_PMT_CLEAR_TRIP]->GetCmdBytes(), index, readBackVal, FALSE, returnedCommand);
		}
	}
	break;
	case PARAM_PMT1_GAIN_POS:
	case PARAM_PMT2_GAIN_POS:
	case PARAM_PMT3_GAIN_POS:
	case PARAM_PMT4_GAIN_POS:
	case PARAM_PMT5_GAIN_POS:
	case PARAM_PMT6_GAIN_POS:
	{
		long pmtIndex = pParamInfo->GetDeviceIndex();
		long paramVal = static_cast<long>(Round((pParamInfo->GetParamVal() / _gainSliderMax[pmtIndex] * ((double)_gainMax[pmtIndex] - _gainMin[pmtIndex])) + _gainMin[pmtIndex], 0));

		std::vector<unsigned char> paramValBytes = GetBytes(paramVal);
		cmd[6] = paramValBytes[0];
		cmd[7] = paramValBytes[1];
		cmd[8] = paramValBytes[2];
		cmd[9] = paramValBytes[3];
		double readBackVal = -1;
		long index = pParamInfo->GetDeviceIndex();
		ExecuteCmd(cmd, index, readBackVal, FALSE, returnedCommand);
		break;
	}
	break;
	case PARAM_PMT1_OUTPUT_OFFSET:
	case PARAM_PMT2_OUTPUT_OFFSET:
	case PARAM_PMT3_OUTPUT_OFFSET:
	case PARAM_PMT4_OUTPUT_OFFSET:
	case PARAM_PMT5_OUTPUT_OFFSET:
	case PARAM_PMT6_OUTPUT_OFFSET:
	{
		long pmtIndex = pParamInfo->GetDeviceIndex();
		long paramVal = static_cast<long>(Round((pParamInfo->GetParamVal() / _offsetSliderMax[pmtIndex] * ((double)_offsetMax[pmtIndex] - _offsetMin[pmtIndex])) + _offsetMin[pmtIndex], 0));

		std::vector<unsigned char> paramValBytes = GetBytes(paramVal);
		cmd[6] = paramValBytes[0];
		cmd[7] = paramValBytes[1];
		cmd[8] = paramValBytes[2];
		cmd[9] = paramValBytes[3];
		double readBackVal = -1;
		ExecuteCmd(cmd, pmtIndex, readBackVal, FALSE, returnedCommand);
		break;
	}
	break;
	case PARAM_PMT1_BANDWIDTH_POS:
	case PARAM_PMT2_BANDWIDTH_POS:
	case PARAM_PMT3_BANDWIDTH_POS:
	case PARAM_PMT4_BANDWIDTH_POS:
	case PARAM_PMT5_BANDWIDTH_POS:
	case PARAM_PMT6_BANDWIDTH_POS:
	{
		//int paramVal = static_cast<int>(Round((pParamInfo->GetParamVal() + 100) / 2.132, 0));

		std::vector<unsigned char> paramValBytes = GetBytes(pParamInfo->GetParamVal());

		cmd[6] = paramValBytes[0];
		cmd[7] = paramValBytes[1];
		cmd[8] = paramValBytes[2];
		cmd[9] = paramValBytes[3];
		double readBackVal = -1;
		long index = pParamInfo->GetDeviceIndex();
		ExecuteCmd(cmd, index, readBackVal, FALSE, returnedCommand);
		break;
	}
	break;
	default:
		double readBackVal = -1;
		break;
	}

	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="cmd">The command.</param>
/// <param name="readBackValue">The read back value.</param>
/// <returns>long.</returns>
long ThorDetector::ExecuteCmd(std::vector<unsigned char> cmd, int portIndex, double& readBackValue, long requestStatus, USHORT& returnedCommmand)
{
	Lock lock(_critSect);

	char buf[100];

	memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)

	string cmdStr(cmd.begin(), cmd.end());
	size_t len = cmdStr.copy(buf, cmdStr.length(), 0);
#pragma warning(pop)
	_serialPort[portIndex].SendData((const unsigned char*)(buf), static_cast<int>(len));

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	memset(buf, 0, sizeof(buf));

	_serialPort[portIndex].ReadData(buf, 100);
	if (requestStatus)
	{
		readBackValue = (unsigned char)buf[3];
	}
	else if ((buf[4] & 0x80) != 0) // there is extended data in the response
	{
		readBackValue = ((unsigned char)buf[6] | (unsigned char)buf[7] << 8 | (unsigned char)buf[8] << 16 | (unsigned char)buf[9] << 24);
	}

	returnedCommmand = buf[0] | (buf[1] << 8);

	return TRUE;
}

/// <summary>
/// Executes the position now.
/// </summary>
void ThorDetector::ExecutePositionNow()
{
	PreflightPosition();
	SetupPosition();
	StartPosition();
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::statusPosition failed");
			LogMessage(_errMsg, INFORMATION_EVENT);
			break;
		}
	} while (status != IDevice::STATUS_READY);

	PostflightPosition();
}

/// <summary>
/// Retrieves the serial and version numbers for each connected device.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::RetrieveDevicesInfo()
{
	Lock lock(_critSect);
	char tmp;
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		_serialNumber[i] = "";
		if (FALSE == _deviceDetected[i])
		{
			continue;
		}

		char buf[100];

		//Find Serial Number and Firmware version number
		// ------------Send command MGMSG_PMT_REQ_INFO------------------------

		unsigned char commandBytesArray[] = { 0x00, 0x45, 0x00, 0x00, 0x00, 0x00 };
		std::vector<unsigned char> commandBytes;
		commandBytes.assign(commandBytesArray, commandBytesArray + sizeof(commandBytesArray) / sizeof(commandBytesArray[0]));

		memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)

		string cmdStr(commandBytes.begin(), commandBytes.end());
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);
#pragma warning(pop)
		_serialPort[i].SendData((const unsigned char*)(buf), static_cast<int>(len));

		Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

		memset(buf, 0, sizeof(buf));

		_serialPort[i].ReadData(buf, 100);

		//MGMSG_PMT_GET_INFO:  //0x4500
		//		    // Initial 6 bytes header 
		//          // 8 bytes of manufacture
		//          out_buffer[6] = 't';
		//          out_buffer[7] = 'h';
		//          out_buffer[8] = 'o';
		//          out_buffer[9] = 'r';
		//          out_buffer[10] = 'l';
		//          out_buffer[11] = 'a';
		//          out_buffer[12] = 'b';
		//          out_buffer[13] = 's';
		//          // 7 bytes of model number
		//          out_buffer[14] = 'p';
		//          out_buffer[15] = 'm';
		//          out_buffer[16] = 't';
		//          out_buffer[17] = '3';
		//          out_buffer[18] = '1';
		//          out_buffer[19] = '0';
		//          out_buffer[20] = '0';
		//          // 8 bytes of PMT serial number
		//          for(int i=0;i<8;i++)
		//              out_buffer[21+i] = ee_wr.dev_ser[i];
		//          // 16 bytes of cpu serial number
		//          for(int i=0;i<4;i++){
		//              convert_32_bit_to_byte_array(cpu_serial_number[i], temp);
		//              for(int j=0;j<4;j++)
		//              out_buffer[29 + 4*i+j] = temp[j];
		//          }
		//          // 3 bytes of firmware version
		//          out_buffer[45] = FW_MINOR_VER;
		//          out_buffer[46] = FW_INTERIM_VER;
		//          out_buffer[47] = FW_MAJOR_VER;
		//          // 20 bytes for internal use
		//          // 2 bytes of hardware version
		//          out_buffer[84] = 0x01;
		//          out_buffer[85] = 0x00;

		if ((buf[0] | (buf[1] << 8)) != MGMSG_PMT_GET_INFO)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::Error RetrieveDevicesInfo, incorrect header. Expected MGMSG_PMT_GET_INFO %d returned %d", MGMSG_PMT_GET_INFO, (buf[0] | (buf[1] << 8)));
			LogMessage(_errMsg, ERROR_EVENT);
		}
		else
		{
			if ((buf[4] & 0x80) != 0) // check if there is extendend data
			{
				for (int j = 0; j < 8; ++j)
				{
					tmp = (unsigned char)buf[21 + j];
					_serialNumber[i] += tmp;
				}
				if (_serialNumber[i] != _settingsSerialNumber[i])
				{

					StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::Warning, the serial number for detector %d doesn't match the serial number in ThorDetectorSettings.xml.", i + 1);
					LogMessage(_errMsg, ERROR_EVENT);
				}

				_firmwareVersion[i] = to_string((unsigned char)buf[47]) + '.' + to_string((unsigned char)buf[46]) + '.' + to_string((unsigned char)buf[45]);
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::RetrieveDevicesInfo, MGMSG_PMT_GET_INFO returned with no extended data.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}

		//Find PMT Limits
		// ------------Send command MGMSG_REQ_PMT_PARAMS------------------------

		unsigned char commandLimitsBytesArray[] = { 0x02, 0x45, 0x00, 0x00, 0x00, 0x00 };
		std::vector<unsigned char> commandLimitsBytes;
		commandLimitsBytes.assign(commandLimitsBytesArray, commandLimitsBytesArray + sizeof(commandLimitsBytesArray) / sizeof(commandLimitsBytesArray[0]));

		memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)

		string limitsCmdStr(commandLimitsBytes.begin(), commandLimitsBytes.end());
		size_t limitsLen = limitsCmdStr.copy(buf, limitsCmdStr.length(), 0);
#pragma warning(pop)
		_serialPort[i].SendData((const unsigned char*)(buf), static_cast<int>(limitsLen));

		Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

		memset(buf, 0, sizeof(buf));

		_serialPort[i].ReadData(buf, 100);

		//		    Initial 6 bytes header 
		//          out_buffer[6] = GainLimitLow;
		//          out_buffer[7] = GainLimitLow;
		//          out_buffer[8] = GainLimitLow;
		//          out_buffer[9] = GainLimitLow;
		//          out_buffer[10] = GainLimitHigh;
		//          out_buffer[11] = GainLimitHigh;
		//          out_buffer[12] = GainLimitHigh;
		//          out_buffer[13] = GainLimitHigh;
		//          out_buffer[14] = OffsetLimitLow;
		//          out_buffer[15] = OffsetLimitLow;
		//          out_buffer[16] = OffsetLimitLow;
		//          out_buffer[17] = OffsetLimitLow;
		//          out_buffer[18] = OffsetLimitHigh;
		//          out_buffer[19] = OffsetLimitHigh;
		//          out_buffer[20] = OffsetLimitHigh;
		//          out_buffer[21] = OffsetLimitHigh;
		//          out_buffer[22] = type;

		if ((buf[0] | (buf[1] << 8)) != MGMSG_GET_PMT_PARAMS)
		{
			StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::Error RetrieveDevicesInfo, incorrect header. Expected MGMSG_GET_PMT_PARAMS %d returned %d", MGMSG_PMT_GET_INFO, (buf[0] | (buf[1] << 8)));
			LogMessage(_errMsg, ERROR_EVENT);
		}
		else
		{
			if ((buf[4] & 0x80) != 0) // check if there is extendend data
			{
				_gainMin[i] = ((unsigned char)buf[6] | (unsigned char)buf[7] << 8 | (unsigned char)buf[8] << 16 | (unsigned char)buf[9] << 24);
				_gainMax[i] = ((unsigned char)buf[10] | (unsigned char)buf[11] << 8 | (unsigned char)buf[12] << 16 | (unsigned char)buf[13] << 24);
				_offsetMin[i] = ((unsigned char)buf[14] | (unsigned char)buf[15] << 8 | (unsigned char)buf[16] << 16 | (unsigned char)buf[17] << 24);
				_offsetMax[i] = ((unsigned char)buf[18] | (unsigned char)buf[19] << 8 | (unsigned char)buf[20] << 16 | (unsigned char)buf[21] << 24);
				//Check if the detector type matches what is read from the device
				if (_detectorType[i] != buf[22])
				{
					StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::RetrieveDevicesInfo, ERROR MGMSG_GET_PMT_PARAMS returned a different detector type %d.", buf[22]);
					LogMessage(_errMsg, ERROR_EVENT);
				}
				_detectorType[i] = buf[22];
				_bandwidthMin[i] = BW_250kHz;
				_bandwidthMax[i] = BW_300MHz;
				//_atenuationMin[i] = 0;
				//_atenuationMax[i] = 0;
			}
			else
			{
				StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector::RetrieveDevicesInfo, MGMSG_GET_PMT_PARAMS returned with no extended data.");
				LogMessage(_errMsg, ERROR_EVENT);
			}
		}
	}
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorDetector::GetLastErrorMsg(wchar_t* msg, long size)
{
	return TRUE;
}

/// <summary>
/// Rounds up to the specified decimal place value.
/// </summary>
/// <param name="val">The value.</param>
/// <param name="decPlace">The decimal place.</param>
/// <returns>double.</returns>
double ThorDetector::rndup(double val, int decPlace)
{
	val *= pow(10.0, decPlace);
	int istack = (int)floor(val);
	double delta = val - istack;
	if (delta < 0.5)
	{
		val = floor(val);
	}
	if (delta > 0.4)
	{
		val = ceil(val);
	}
	val /= pow(10.0, decPlace);
	return val;
}


/// <summary>
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ThorDetector::GetBytes(int value)
{
	std::vector<unsigned char> bytes(sizeof(int));
	std::memcpy(&bytes[0], &value, sizeof(int));
	return bytes;
}

/// <summary>
/// Rounds to the specified number.
/// </summary>
/// <param name="number">The number.</param>
/// <param name="decimals">The decimals.</param>
/// <returns>double.</returns>
double ThorDetector::Round(double number, int decimals)
{
	double decP = std::pow(10, decimals);
	double ret;
	if (number < 0.0)
		ret = -std::floor(-number * decP + 0.5) / decP;
	else
		ret = std::floor(number * decP + 0.5) / decP;
	return ret;
}



/// <summary>
/// Builds the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::BuildParamTable()
{
	DestroyParamTable();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(
		PARAM_DEVICE_TYPE,						//ID
		L"PARAM_DEVICE_TYPE",					//Parameter description
		_connectedPMTs,							//VAL
		_connectedPMTs,							//PARAM C
		FALSE,									//PARAM B
		TYPE_LONG,								//TYPE
		TRUE,									//AVAILABLE
		TRUE,									//READ ONLY
		0,										//MIN
		PMT1 | PMT2 | PMT3 | PMT4 | PMT5 | PMT6,//MAX
		0,										//DEFAULT
		commandBytes,							//Command
		-1,										//Device Index	
		-1);									//Related parameter ID			
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE, tempParamInfo));

	//***Connected pmts Command***//
	tempParamInfo = new ParamInfo(
		PARAM_CONNECTED_PMTS,
		L"PARAM_CONNECTED_PMTS",
		_connectedPMTs,
		_connectedPMTs,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		PMT1 | PMT2 | PMT3 | PMT4 | PMT5 | PMT6,
		0,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTED_PMTS, tempParamInfo));
	//***end Connected pmts Command***//

	//build table entry To turn PMT ON
	unsigned char commandBytesEnable[] = { 0x05, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesEnable, commandBytesEnable + sizeof(commandBytesEnable) / sizeof(commandBytesEnable[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_ENABLE,
		L"PARAM_PMT1_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		0,
		PARAM_PMT1_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_ENABLE, tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_ENABLE,
		L"PARAM_PMT2_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		1,
		PARAM_PMT2_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_ENABLE, tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_ENABLE,
		L"PARAM_PMT3_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		2,
		PARAM_PMT3_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_ENABLE, tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_ENABLE,
		L"PARAM_PMT4_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		3,
		PARAM_PMT4_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_ENABLE, tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_ENABLE,
		L"PARAM_PMT5_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		4,
		PARAM_PMT5_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_ENABLE, tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_ENABLE,
		L"PARAM_PMT6_ENABLE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		0,
		1,
		0,
		commandBytes,
		5,
		PARAM_PMT6_GAIN_POS);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_ENABLE, tempParamInfo));

	//build table entry To set gain
	unsigned char commandBytesGain[] = { 0x10, 0x45, 0x04, 0x00, 0x85, 0x01, 0x02, 0x0D, 0x00, 0x00 };
	commandBytes.assign(commandBytesGain, commandBytesGain + sizeof(commandBytesGain) / sizeof(commandBytesGain[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_GAIN_POS,
		L"PARAM_PMT1_GAIN_POS",
		_defaultGain[0],
		_defaultGain[0],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[0],
		_gainSliderMax[0],
		_gainSliderMin[0],
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS, tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_GAIN_POS,
		L"PARAM_PMT2_GAIN_POS",
		_defaultGain[1],
		_defaultGain[1],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[1],
		_gainSliderMax[1],
		_gainSliderMin[1],
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS, tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_GAIN_POS,
		L"PARAM_PMT3_GAIN_POS",
		_defaultGain[2],
		_defaultGain[2],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[2],
		_gainSliderMax[2],
		_gainSliderMin[2],
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS, tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_GAIN_POS,
		L"PARAM_PMT4_GAIN_POS",
		_defaultGain[3],
		_defaultGain[3],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[3],
		_gainSliderMax[3],
		_gainSliderMin[3],
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS, tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_GAIN_POS,
		L"PARAM_PMT5_GAIN_POS",
		_defaultGain[4],
		_defaultGain[4],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[4],
		_gainSliderMax[4],
		_gainSliderMin[4],
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_GAIN_POS, tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_GAIN_POS,
		L"PARAM_PMT6_GAIN_POS",
		_defaultGain[5],
		_defaultGain[5],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_gainSliderMin[5],
		_gainSliderMax[5],
		_gainSliderMin[5],
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_GAIN_POS, tempParamInfo));

	//build table entry To request current PMT atenuation
	unsigned char commandBytesReqPMTAtenuation[] = { 0x1B, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesReqPMTAtenuation, commandBytesReqPMTAtenuation + sizeof(commandBytesReqPMTAtenuation) / sizeof(commandBytesReqPMTAtenuation[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT1_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[0],
		_atenuationMax[0],
		_atenuationMin[0],
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry To request current PMT atenuation
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT2_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[1],
		_atenuationMax[1],
		_atenuationMin[1],
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry To request current PMT atenuation
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT3_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[2],
		_atenuationMax[2],
		_atenuationMin[2],
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry To request current PMT atenuation
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT4_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[3],
		_atenuationMax[3],
		_atenuationMin[3],
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry To request current PMT atenuation
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT5_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[4],
		_atenuationMax[4],
		_atenuationMin[4],
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry To request current PMT atenuation
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_GAIN_POS_CURRENT_VOLTS,
		L"PARAM_PMT6_GAIN_POS_CURRENT_VOLTS",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_atenuationMin[5],
		_atenuationMax[5],
		_atenuationMin[5],
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_GAIN_POS_CURRENT_VOLTS, tempParamInfo));

	//build table entry for the device connection status
	unsigned char commandBytesCON_STA[] = { 0x00 }; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesCON_STA, commandBytesCON_STA + sizeof(commandBytesCON_STA) / sizeof(commandBytesCON_STA[0]));
	tempParamInfo = new ParamInfo(
		PARAM_CONNECTION_STATUS,	//ID
		L"PARAM_CONNECTION_STATUS",
		FALSE,					//VAL
		FALSE,					//PARAM C
		TRUE,					//PARAM B
		TYPE_LONG,				//TYPE
		TRUE,					//AVAILABLE
		TRUE,					//READ ONLY
		(double)ConnectionStatusType::CONNECTION_READY,		//MIN
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//MAX
		(double)ConnectionStatusType::CONNECTION_UNAVAILABLE,	//DEFAULT
		commandBytes,
		-1,
		-1);			//COMMAND
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_CONNECTION_STATUS, tempParamInfo));

	//build table entry To request PMT trip status
	unsigned char commandBytesReqPMTTrip[] = { 0x0C, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesReqPMTTrip, commandBytesReqPMTTrip + sizeof(commandBytesReqPMTTrip) / sizeof(commandBytesReqPMTTrip[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_SAFETY,
		L"PARAM_PMT1_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SAFETY, tempParamInfo));

	//build table entry To request PMT trip status
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_SAFETY,
		L"PARAM_PMT2_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SAFETY, tempParamInfo));

	//build table entry To request PMT trip status
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_SAFETY,
		L"PARAM_PMT3_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SAFETY, tempParamInfo));

	//build table entry To request PMT trip status
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_SAFETY,
		L"PARAM_PMT4_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SAFETY, tempParamInfo));

	//build table entry To request PMT trip status
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_SAFETY,
		L"PARAM_PMT5_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SAFETY, tempParamInfo));

	//build table entry To request PMT trip status
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_SAFETY,
		L"PARAM_PMT6_SAFETY",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SAFETY, tempParamInfo));

	//build table entry To set offset
	unsigned char commandBytesSetOffset[] = { 0x13, 0x45, 0x04, 0x00, 0x85, 0x01, 0x02, 0x0D, 0x00, 0x00 };
	commandBytes.assign(commandBytesSetOffset, commandBytesSetOffset + sizeof(commandBytesSetOffset) / sizeof(commandBytesSetOffset[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_OUTPUT_OFFSET,
		L"PARAM_PMT1_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[0],
		_offsetSliderMax[0],
		_offsetSliderMin[0],
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To set offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_OUTPUT_OFFSET,
		L"PARAM_PMT2_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[1],
		_offsetSliderMax[1],
		_offsetSliderMin[1],
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To set offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_OUTPUT_OFFSET,
		L"PARAM_PMT3_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[2],
		_offsetSliderMax[2],
		_offsetSliderMin[2],
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To set offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_OUTPUT_OFFSET,
		L"PARAM_PMT4_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[3],
		_offsetSliderMax[3],
		_offsetSliderMin[3],
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To set offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_OUTPUT_OFFSET,
		L"PARAM_PMT5_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[4],
		_offsetSliderMax[4],
		_offsetSliderMin[4],
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To set offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_OUTPUT_OFFSET,
		L"PARAM_PMT6_OUTPUT_OFFSET",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		FALSE,
		_offsetSliderMin[5],
		_offsetSliderMax[5],
		_offsetSliderMin[5],
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_OUTPUT_OFFSET, tempParamInfo));

	//build table entry To request current PMT offset
	unsigned char commandBytesReqPMTOffset[] = { 0x14, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesReqPMTOffset, commandBytesReqPMTOffset + sizeof(commandBytesReqPMTOffset) / sizeof(commandBytesReqPMTOffset[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT1_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[0],
		_offsetSliderMax[0],
		_offsetSliderMin[0],
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To request current PMT offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT2_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[1],
		_offsetSliderMax[1],
		_offsetSliderMin[1],
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To request current PMT offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT3_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[2],
		_offsetSliderMax[2],
		_offsetSliderMin[2],
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To request current PMT offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT4_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[3],
		_offsetSliderMax[3],
		_offsetSliderMin[3],
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To request current PMT offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT5_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[4],
		_offsetSliderMax[4],
		_offsetSliderMin[4],
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To request current PMT offset
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_OUTPUT_OFFSET_CURRENT,
		L"PARAM_PMT6_OUTPUT_OFFSET_CURRENT",
		0,
		0,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		_offsetSliderMin[5],
		_offsetSliderMax[5],
		_offsetSliderMin[5],
		commandBytes,
		5,
		-1);

	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_OUTPUT_OFFSET_CURRENT, tempParamInfo));

	//build table entry To set bandwith
	unsigned char commandBytesSetBandwith[] = { 0x08, 0x45, 0x04, 0x00, 0x85, 0x01, 0x02, 0x0D, 0x00, 0x00 };
	commandBytes.assign(commandBytesSetBandwith, commandBytesSetBandwith + sizeof(commandBytesSetBandwith) / sizeof(commandBytesSetBandwith[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_BANDWIDTH_POS,
		L"PARAM_PMT1_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[0],
		_bandwidthMax[0],
		_bandwidthMin[0],
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_BANDWIDTH_POS, tempParamInfo));

	//build table entry To set bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_BANDWIDTH_POS,
		L"PARAM_PMT2_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[1],
		_bandwidthMax[1],
		_bandwidthMin[1],
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_BANDWIDTH_POS, tempParamInfo));

	//build table entry To set bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_BANDWIDTH_POS,
		L"PARAM_PMT3_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[2],
		_bandwidthMax[2],
		_bandwidthMin[2],
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_BANDWIDTH_POS, tempParamInfo));

	//build table entry To set bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_BANDWIDTH_POS,
		L"PARAM_PMT4_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[3],
		_bandwidthMax[3],
		_bandwidthMin[3],
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_BANDWIDTH_POS, tempParamInfo));

	//build table entry To set bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_BANDWIDTH_POS,
		L"PARAM_PMT5_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[4],
		_bandwidthMax[4],
		_bandwidthMin[4],
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_BANDWIDTH_POS, tempParamInfo));

	//build table entry To set bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_BANDWIDTH_POS,
		L"PARAM_PMT6_BANDWIDTH_POS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		_bandwidthMin[5],
		_bandwidthMax[5],
		_bandwidthMin[5],
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_BANDWIDTH_POS, tempParamInfo));

	//build table entry To request current PMT bandwith
	unsigned char commandBytesReqPMTFreq[] = { 0x09, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesReqPMTFreq, commandBytesReqPMTFreq + sizeof(commandBytesReqPMTFreq) / sizeof(commandBytesReqPMTFreq[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT1_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry To request current PMT bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT2_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry To request current PMT bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT3_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry To request current PMT bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT4_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry To request current PMT bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT5_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry To request current PMT bandwith
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_BANDWIDTH_POS_CURRENT,
		L"PARAM_PMT6_BANDWIDTH_POS_CURRENT",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		BW_2_5MHz,
		BW_80MHz,
		BW_2_5MHz,
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_BANDWIDTH_POS_CURRENT, tempParamInfo));

	//build table entry to clear trip
	unsigned char commandBytesClearTrip[] = { 0x0B, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesClearTrip, commandBytesClearTrip + sizeof(commandBytesClearTrip) / sizeof(commandBytesClearTrip[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT_CLEAR_TRIP,
		L"PARAM_PMT_CLEAR_TRIP",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT_CLEAR_TRIP, tempParamInfo));

	//build table entry To request current PMT saturation count
	unsigned char commandBytesReqPMTSaturationCount[] = { 0x23, 0x45, 0x00, 0x00, 0x21, 0x01 };
	commandBytes.assign(commandBytesReqPMTSaturationCount, commandBytesReqPMTSaturationCount + sizeof(commandBytesReqPMTSaturationCount) / sizeof(commandBytesReqPMTSaturationCount[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_SATURATIONS,
		L"PARAM_PMT1_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SATURATIONS, tempParamInfo));

	//build table entry To request current PMT saturation count
	tempParamInfo = new ParamInfo(
		PARAM_PMT2_SATURATIONS,
		L"PARAM_PMT2_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SATURATIONS, tempParamInfo));

	//build table entry To request current PMT saturation count
	tempParamInfo = new ParamInfo(
		PARAM_PMT3_SATURATIONS,
		L"PARAM_PMT3_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SATURATIONS, tempParamInfo));

	//build table entry To request current PMT saturation count
	tempParamInfo = new ParamInfo(
		PARAM_PMT4_SATURATIONS,
		L"PARAM_PMT4_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SATURATIONS, tempParamInfo));

	//build table entry To request current PMT saturation count
	tempParamInfo = new ParamInfo(
		PARAM_PMT5_SATURATIONS,
		L"PARAM_PMT5_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SATURATIONS, tempParamInfo));

	//build table entry To request current PMT saturation count
	tempParamInfo = new ParamInfo(
		PARAM_PMT6_SATURATIONS,
		L"PARAM_PMT6_SATURATIONS",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		UINT_MAX,
		0,
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SATURATIONS, tempParamInfo));

	// build table entry to return the detector type
	std::vector<unsigned char> tmpCommandBytes;
	tmpCommandBytes.push_back(0x00);
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_DETECTOR_TYPE,
		L"PARAM_PMT1_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_DETECTOR_TYPE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT2_DETECTOR_TYPE,
		L"PARAM_PMT2_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_DETECTOR_TYPE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT3_DETECTOR_TYPE,
		L"PARAM_PMT3_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_DETECTOR_TYPE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT4_DETECTOR_TYPE,
		L"PARAM_PMT4_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_DETECTOR_TYPE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT5_DETECTOR_TYPE,
		L"PARAM_PMT5_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_DETECTOR_TYPE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT6_DETECTOR_TYPE,
		L"PARAM_PMT6_DETECTOR_TYPE",
		0,
		0,
		FALSE,
		TYPE_LONG,
		TRUE,
		TRUE,
		0,
		DetectorTypes::LAST_TYPE,
		0,
		tmpCommandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_DETECTOR_TYPE, tempParamInfo));

	//***Serial Number Commands***//
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_SERIALNUMBER,
		L"PARAM_PMT1_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_SERIALNUMBER, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT2_SERIALNUMBER,
		L"PARAM_PMT2_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_SERIALNUMBER, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT3_SERIALNUMBER,
		L"PARAM_PMT3_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_SERIALNUMBER, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT4_SERIALNUMBER,
		L"PARAM_PMT4_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_SERIALNUMBER, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT5_SERIALNUMBER,
		L"PARAM_PMT5_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_SERIALNUMBER, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT6_SERIALNUMBER,
		L"PARAM_PMT6_SERIALNUMBER",
		0,
		0,
		FALSE,
		TYPE_STRING,
		TRUE,
		TRUE,
		0,
		0,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_SERIALNUMBER, tempParamInfo));
	//***end Serial Number Commands***//

	//***Update Firmware Commands***//
	unsigned char commandBytesSetToBootloader[] = { 0xA6, 0x00, 0x04, 0x00, 0x85, 0x01, 0x00, 0x00, 0x00, 0x00 }; // Update Firmware command, sets it to bootloader mode
	commandBytes.assign(commandBytesSetToBootloader, commandBytesSetToBootloader + sizeof(commandBytesSetToBootloader) / sizeof(commandBytesSetToBootloader[0]));
	tempParamInfo = new ParamInfo(
		PARAM_PMT1_FIRMWAREUPDATE,
		L"PARAM_PMT1_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_FIRMWAREUPDATE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT2_FIRMWAREUPDATE,
		L"PARAM_PMT2_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_FIRMWAREUPDATE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT3_FIRMWAREUPDATE,
		L"PARAM_PMT3_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_FIRMWAREUPDATE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT4_FIRMWAREUPDATE,
		L"PARAM_PMT4_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_FIRMWAREUPDATE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT5_FIRMWAREUPDATE,
		L"PARAM_PMT5_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		4,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT5_FIRMWAREUPDATE, tempParamInfo));

	tempParamInfo = new ParamInfo(
		PARAM_PMT6_FIRMWAREUPDATE,
		L"PARAM_PMT6_FIRMWAREUPDATE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		FALSE,
		TRUE,
		FALSE,
		commandBytes,
		5,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT6_FIRMWAREUPDATE, tempParamInfo));
	//***end Update Firmware Commands***//

	tempParamInfo = new ParamInfo(
		PARAM_PMT_OFFSET_STEP_SIZE,
		L"PARAM_PMT_OFFSET_STEP_SIZE",
		FALSE,
		FALSE,
		FALSE,
		TYPE_DOUBLE,
		TRUE,
		TRUE,
		0,
		100,
		0,
		tmpCommandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT_OFFSET_STEP_SIZE, tempParamInfo));

	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorDetector::DestroyParamTable()
{
	try
	{
		for (std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete iter->second;
		}
		_tableParams.clear();
	}
	catch (...)
	{
#ifdef LOGGING_ENABLED
		StringCbPrintfW(_errMsg, MSG_SIZE, L"ThorDetector DestroyParamTable unable to destroy the table created on heap");
		logDll->TLTraceEvent(VERBOSE_EVENT, 1, _errMsg);
#endif
		return FALSE;
	}

	return TRUE;
}
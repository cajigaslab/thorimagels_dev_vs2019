// ThorHPD.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Serial.h"
#include "ThorHPD.h"
#include "ThorHPDXML.h"
#include "Strsafe.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>



#ifdef LOGGING_ENABLED
auto_ptr<LogDll> logDll(new LogDll(L".\\Modules_Native\\ThorLoggingUnmanaged.dll"));
#endif

/// <summary>
/// The message
/// </summary>
wchar_t message[256];

/// <summary>
/// Prevents a default instance of the <see cref="ThorHPD"/> class from being created.
/// </summary>
ThorHPD::ThorHPD()
{
	_numOfAxes = 3;
	_sleepTimeAfterMoveComplete = 0;
	_defaultGain[0] = 0;
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		_deviceDetected[i] = FALSE;
		_defaultGain[i] = 0;
		
	}
	_connectedPMTs = 0;
	_deviceDetected[DEVICE_NUM] = FALSE;
	_errMsg[0] = NULL;
}

/// <summary>
/// Finalizes an instance of the <see cref="ThorHPD"/> class.
/// </summary>
ThorHPD::~ThorHPD()
{
	_instanceFlag = false;
	_errMsg[0] = 0;
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		_defaultGain[i] = 0;
	}
}

bool ThorHPD::_instanceFlag = false;

auto_ptr<ThorHPD> ThorHPD::_single (new ThorHPD());
const string ThorHPD::_deviceSignature[DEVICE_NUM] = {"Detector1", "Detector2", "Detector3", "Detector4", "Detector5", "Detector6"};
const long ThorHPD::_pmtSelect[DEVICE_NUM] = {PMT1, PMT2, PMT3, PMT4, PMT5, PMT6};

/// <summary>
/// Gets the instance.
/// </summary>
/// <returns>ThorHPD *.</returns>
ThorHPD* ThorHPD::getInstance()
{
	if(!_instanceFlag)
	{
		_single.reset(new ThorHPD());
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
long ThorHPD::FindDevices(long &deviceCount)
{
	long ret = TRUE;

	long portID = 0;
	long baudRate = 0;
	string serialNum = "";
	for(int i=0; i<DEVICE_NUM; ++i)
	{
		try
		{
			try
			{
				portID = 0;
				//Get portID, etc from hardware ThorBCMSettings.xml
				auto_ptr<ThorHPDXML> pSetup(new ThorHPDXML());
				long type = 0;
				pSetup->GetDeviceConnectionInfo(_deviceSignature[i], portID,baudRate, _settingsSerialNumber[i], type);			
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorHPDSettings.xml file");
			}
			if(FALSE == _serialPort[i].Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorHPD SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorHPD SelectDevice could not open serial port or configuration file is not avaible.");
			}
			else
			{
				//_deviceDetected[DEVICE_NUM] works as a flag to indicate some device has been found.
				_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
				deviceCount = 1;
				ret = TRUE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to connect to device");
		}
	}
	try
	{
		portID = 0;
		//Get portID, etc from hardware ThorBCMSettings.xml
		auto_ptr<ThorHPDXML> pSetup(new ThorHPDXML());
		long type = 0;
		pSetup->GetDetectorDefaultGain(_defaultGain[0], _defaultGain[1], _defaultGain[2], _defaultGain[3]);
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorHPDSettings.xml file");
	}
	
	RetrieveDevicesInfo();

	return ret;
}


/// <summary>
/// Selects the device.
/// </summary>
/// <param name="device">The device.</param>
/// <returns>long.</returns>
long ThorHPD::SelectDevice(const long device)
{
	Lock lock(_critSect);
	long ret = FALSE;
	long portID=0;
	long baudRate=0;

	//Check if any device detected before continuing
	if(FALSE == _deviceDetected[DEVICE_NUM]) 
	{
		return FALSE;
	}

	//Reset detected devices
	for (int i = 0; i < DEVICE_NUM; i++)
	{
		_deviceDetected[i] = FALSE;
	}
		
	for(int i=0; i< DEVICE_NUM; i++)
	{
		try
		{
			try
			{				
				auto_ptr<ThorHPDXML> pSetup(new ThorHPDXML());
				//Get portID, etc from  ThorBCMSettings.xml
				long type;
				pSetup->GetDeviceConnectionInfo(_deviceSignature[i], portID,baudRate, _settingsSerialNumber[i], type);
			}
			catch(...)
			{
				StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorHPDSettings.xml file");
				return ret;
			}
			if(FALSE == _serialPort[i].Open(portID, baudRate))
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorHPD SelectDevice could not open serial port");
				LogMessage(message);
				StringCbPrintfW(_errMsg,MSG_SIZE,L"ThorHPD SelectDevice could not open serial port or configuration file is not avaible.");
			}
			else 
			{
				_deviceDetected[DEVICE_NUM] = _deviceDetected[i] = TRUE;
				_connectedPMTs |= _pmtSelect[i];

				SetDetectorType(i, 5);

				ret = TRUE;
			}
		}
		catch(...)
		{
			StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to connect to device");
		}
	}
	BuildParamTable();
	return ret;
}

long ThorHPD::SetDetectorType(long index, long type)
{
	unsigned char commandBytesA[] = { 0x04, 0x45, 0x01, 0x00, 0x80, 0x00, 0x05 }; //MGMSG_MOT_SET_POSCOUNTER Axis 0 (Stage 1)
	std::vector<unsigned char> commandBytes;
	commandBytes.assign(commandBytesA, commandBytesA + sizeof(commandBytesA)/sizeof(commandBytesA[0]));


	std::vector<unsigned char> cmd = commandBytes;	
	std::vector<unsigned char> paramValBytes = GetBytes(type);

	commandBytes[6] =  paramValBytes[0];
	double readBack = 0;
	ExecuteCmd(commandBytes, index, readBack);
	return TRUE;
}

/// <summary>
/// Logs the message.
/// </summary>
/// <param name="message">The message.</param>
void ThorHPD::LogMessage(wchar_t *message)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(VERBOSE_EVENT, 1, message);
#endif
}

/// <summary>
/// Teardowns the device.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::TeardownDevice()
{	
	SetParam(IDevice::PARAM_PMT1_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT2_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT3_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT4_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT5_ENABLE, FALSE);
	SetParam(IDevice::PARAM_PMT6_ENABLE, FALSE);

	SetupPosition();
	StartPosition();

	for(int i=0; i<DEVICE_NUM; i++)
	{
		if(_deviceDetected[i] == TRUE)
		{
			_serialPort[i].Close();
			_deviceDetected[i] = FALSE;
		}
	}
	_deviceDetected[DEVICE_NUM] = FALSE;
	_connectedPMTs = FALSE;
	try
	{
		//TODO: device if this values should live in xml file
		//currently these values are being replaced by the active xml
		////Get portID, etc from hardware ThorBCMSettings.xml
		//auto_ptr<ThorHPDXML> pSetup(new ThorHPDXML());
	
		//long gain1 = static_cast<long>(_tableParams[PARAM_PMT1_GAIN_POS]->GetParamVal());
		//long gain2 = static_cast<long>(_tableParams[PARAM_PMT2_GAIN_POS]->GetParamVal());
		//long gain3 = static_cast<long>(_tableParams[PARAM_PMT3_GAIN_POS]->GetParamVal());
		//long gain4 = static_cast<long>(_tableParams[PARAM_PMT4_GAIN_POS]->GetParamVal());

		//pSetup->SetDetectorDefaultGain(gain1, gain2, gain3, gain4);			
	}
	catch(...)
	{
		StringCbPrintfW(_errMsg,MSG_SIZE,L"Unable to locate ThorHPDSettings.xml file");
	}
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
long ThorHPD::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
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
long ThorHPD::GetDeviceParameters()
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
long ThorHPD::SetParam(const long paramID, const double param)
{
	double value = param;
	if (NULL != _tableParams[paramID])
	{
		if(_tableParams[paramID]->GetParamID() == paramID)
		{
			if(FALSE == (_tableParams[paramID]->GetParamAvailable()) || (TRUE == _tableParams[paramID]->GetParamReadOnly()))
			{
				return FALSE;
			}
			else if((_tableParams[paramID]->GetParamMin() <= param) && (_tableParams[paramID]->GetParamMax() >= param))
			{
				_tableParams[paramID]->UpdateParam(param);
				return TRUE;
			}
			else
			{
				StringCbPrintfW(message,MSG_SIZE,L"ThorHPD SetParam failed. paramID: %d", paramID);
				LogMessage(message);
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
long ThorHPD::GetParam(const long paramID, double &param)
{
	if (NULL != _tableParams[paramID])
	{
		if(FALSE == _tableParams[paramID]->GetParamAvailable())
		{
			return FALSE;
		}
		switch(_tableParams[paramID]->GetParamID())
		{
		//case PARAM_PMT1_SAFETY:
		//case PARAM_PMT2_SAFETY:
		//case PARAM_PMT3_SAFETY:
		//case PARAM_PMT4_SAFETY:
		//case PARAM_PMT5_SAFETY:
		//case PARAM_PMT6_SAFETY:
		//	{
		//		long status = 0;
		//		long ret = TRUE;//QueryPMTStatus(_tableParams[paramID]->GetDeviceIndex(), status);				
		//		param = status;				
		//		return ret;
		//	}
		//case PARAM_PMT1_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT2_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT3_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT4_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT5_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT6_GAIN_POS_CURRENT_VOLTS:
		//case PARAM_PMT1_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT2_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT3_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT4_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT5_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT6_OUTPUT_OFFSET_CURRENT:
		//case PARAM_PMT1_BANDWIDTH_POS_CURRENT:
		//case PARAM_PMT2_BANDWIDTH_POS_CURRENT:
		//case PARAM_PMT3_BANDWIDTH_POS_CURRENT:
		//case PARAM_PMT4_BANDWIDTH_POS_CURRENT:
		//case PARAM_PMT5_BANDWIDTH_POS_CURRENT:
		//case PARAM_PMT6_BANDWIDTH_POS_CURRENT:
		//	{
		//		long ret = TRUE;//QueryPMTSetting(_tableParams[paramID], param);
		//		return ret;
		//	}
		case PARAM_CONNECTION_STATUS:
			{
				param = (_deviceDetected[DEVICE_NUM]) ? (double)ConnectionStatusType::CONNECTION_READY : (double)ConnectionStatusType::CONNECTION_UNAVAILABLE;
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
long ThorHPD::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorHPD::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorHPD::SetParamString(const long paramID, wchar_t* str)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Gets the parameter of type string.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="str">The parameter string.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorHPD::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

/// <summary>
/// Preflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::PreflightPosition()
{
	return TRUE;
}

/// <summary>
/// Setups the position.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::SetupPosition()
{	
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if((FALSE == (iter->second)->GetParamAvailable()) || (TRUE == (iter->second)->GetParamReadOnly()))
		{
			continue;
		}
		else if((iter->second)->GetParamVal() != (iter->second)->GetParamCurrent())
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
long ThorHPD::StartPosition()
{
	long ret = TRUE;

	//iterate through map and set the parameters
	for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
	{
		if (NULL == iter->second)
		{
			continue;
		}
		else if(((iter->second)->GetParamAvailable() == FALSE) || ((iter->second)->GetParamReadOnly() == TRUE))
		{
			continue;
		}
		else if((iter->second)->GetParamBool() == TRUE)
		{	
			ret = ExecuteCmd(iter->second);
			(iter->second)->UpdateParam_C();

			(iter->second)->SetParamBool(FALSE);

			StringCbPrintfW(message,MSG_SIZE,L"StartPosition succeeded at paramID: %d",(iter->second)->GetParamID());
		}
	}

	return ret;
}

/// <summary>
/// Status of the positioning.
/// </summary>
/// <param name="status">The status.</param>
/// <returns>long.</returns>
long ThorHPD::StatusPosition(long &status)
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
long ThorHPD::ReadPosition(DeviceType deviceType,double &pos)
{
	return FALSE;
}

/// <summary>
/// Postflights the position.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::PostflightPosition()
{
	return TRUE;
}


/// <summary>
/// Executes the command.
/// </summary>
/// <param name="pParamInfo">The p parameter information.</param>
/// <returns>long.</returns>
long ThorHPD::ExecuteCmd(ParamInfo* pParamInfo)
{
	long paramID = pParamInfo->GetParamID();
	std::vector<unsigned char> cmd = pParamInfo->GetCmdBytes();
	switch (paramID)
	{
	case PARAM_PMT1_ENABLE:
	case PARAM_PMT2_ENABLE:
	case PARAM_PMT3_ENABLE:
	case PARAM_PMT4_ENABLE:
		{
			int paramVal = static_cast<int>(Round(pParamInfo->GetParamVal(),0));
			std::vector<unsigned char> paramValBytes = GetBytes(paramVal);
			cmd[3] =  paramValBytes[0];

			double readBackVal = -1;
			long index = pParamInfo->GetDeviceIndex();
			
			ExecuteCmd(cmd, index, readBackVal);
			
			if (TRUE == paramVal)
			{
				Sleep(30);
				long relatedID = pParamInfo->GetRelatedID();
				ParamInfo* pParamInfo2 = _tableParams[static_cast<unsigned int>(relatedID)];		
				ExecuteCmd(pParamInfo2);
			}
		}
		break;
	case PARAM_PMT1_GAIN_POS:
	case PARAM_PMT2_GAIN_POS:
	case PARAM_PMT3_GAIN_POS:
	case PARAM_PMT4_GAIN_POS:
		{	
			int paramVal = static_cast<int>(Round((pParamInfo->GetParamVal() + 100) / 2.132, 0));

			std::vector<unsigned char> paramValBytes = GetBytes(paramVal);
			cmd[6] =  paramValBytes[0];
			cmd[7] =  paramValBytes[1];
			cmd[8] =  paramValBytes[2];
			cmd[9] =  paramValBytes[3];
			double readBackVal = -1;
			long index = pParamInfo->GetDeviceIndex();
			ExecuteCmd(cmd, index, readBackVal);
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
long ThorHPD::ExecuteCmd(std::vector<unsigned char> cmd, int portIndex, double &readBackValue)
{
	Lock lock(_critSect);

	char buf[100];

	memset(buf, 0, sizeof(buf));
#pragma warning(push)
#pragma warning(disable:4996)

	string cmdStr(cmd.begin(),cmd.end());
	size_t len = cmdStr.copy(buf, cmdStr.length(), 0);
#pragma warning(pop)
	_serialPort[portIndex].SendData((const unsigned char*)(buf), static_cast<int>(len));

	Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

	memset(buf, 0, sizeof(buf));

	_serialPort[portIndex].ReadData(buf, 100);
	readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
	return TRUE;
}

/// <summary>
/// Executes the position now.
/// </summary>
void ThorHPD::ExecutePositionNow()
{
	PreflightPosition();
	SetupPosition();
	StartPosition();
	long status = IDevice::STATUS_BUSY;
	do
	{
		if (FALSE == StatusPosition(status))
		{
			StringCbPrintfW(message,MSG_SIZE,L"ThorHPD::statusPosition failed");
			LogMessage(message);
			break;
		}
	}while(status != IDevice::STATUS_READY);

	PostflightPosition();	
}

/// <summary>
/// Retrieves the serial and version numbers for each connected device.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::RetrieveDevicesInfo()
{
	Lock lock(_critSect);
	for (int i = 0; i < DEVICE_NUM; ++i)
	{
		if (FALSE == _deviceDetected[i])
		{
			continue;
		}

		char buf[100];

		unsigned char commandBytesArray[] = {0x00, 0x45, 0x00, 0x00, 0x00, 0x00};
		std::vector<unsigned char> commandBytes;
		commandBytes.assign(commandBytesArray, commandBytesArray + sizeof(commandBytesArray)/sizeof(commandBytesArray[0]));

		memset(buf, 0, sizeof(buf));
	#pragma warning(push)
	#pragma warning(disable:4996)

		string cmdStr(commandBytes.begin(),commandBytes.end());
		size_t len = cmdStr.copy(buf, cmdStr.length(), 0);
	#pragma warning(pop)
		_serialPort[i].SendData((const unsigned char*)(buf), static_cast<int>(len));

		Sleep(50); //Determined by testing... Shorter sleep time may cause bad communications

		memset(buf, 0, sizeof(buf));

		_serialPort[i].ReadData(buf, 100);

		//MGMSG_PMT_REQ_INFO:  //0x4500
  //          // 8 bytes of manufacture
  //          out_buffer[0] = 't';
  //          out_buffer[1] = 'h';
  //          out_buffer[2] = 'o';
  //          out_buffer[3] = 'r';
  //          out_buffer[4] = 'l';
  //          out_buffer[5] = 'a';
  //          out_buffer[6] = 'b';
  //          out_buffer[7] = 's';
  //          // 7 bytes of model number
  //          out_buffer[8] = 'p';
  //          out_buffer[9] = 'm';
  //          out_buffer[10] = 't';
  //          out_buffer[11] = '2';
  //          out_buffer[12] = '1';
  //          out_buffer[13] = '0';
  //          out_buffer[14] = '0';
  //          // 8 bytes of PMT serial number
  //          for(int i=0;i<8;i++)
  //              out_buffer[15+i] = ee_wr.dev_ser[i];
  //          // 16 bytes of cpu serial number
  //          for(int i=0;i<4;i++){
  //              convert_32_bit_to_byte_array(cpu_serial_number[i], temp);
  //              for(int j=0;j<4;j++)
  //              out_buffer[23 + 4*i+j] = temp[j];
  //          }
  //          // 3 bytes of firmware version
  //          out_buffer[39] = FW_MINOR_VER;
  //          out_buffer[40] = FW_INTERIM_VER;
  //          out_buffer[41] = FW_MAJOR_VER;
  //          // 20 bytes for internal use
  //          // 2 bytes of hardware version
  //          out_buffer[78] = 0x01;
  //          out_buffer[79] = 0x00;

		for (int j = 0; j < 16; ++j)
		{
			_serialNumber[i] += to_string((unsigned char)buf[23 + j]);
		}

		_firmwareVersion[i] = to_string((unsigned char)buf[41]) + '.' + to_string((unsigned char)buf[40]) + '.' + to_string((unsigned char)buf[39]);

		int readBackValue = ((unsigned char)buf[8] | (unsigned char)buf[9] << 8 | (unsigned char)buf[10] << 16 | (unsigned char)buf[11] << 24);
	}
	return TRUE;
}

/// <summary>
/// Gets the last error MSG.
/// </summary>
/// <param name="msg">The MSG.</param>
/// <param name="size">The size.</param>
/// <returns>long.</returns>
long ThorHPD::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/// <summary>
/// Rounds up to the specified decimal place value.
/// </summary>
/// <param name="val">The value.</param>
/// <param name="decPlace">The decimal place.</param>
/// <returns>double.</returns>
double ThorHPD::rndup(double val,int decPlace)
{
	val *= pow(10.0,decPlace);
	int istack = (int)floor(val);
	double delta = val-istack;
	if (delta < 0.5)
	{	val = floor(val);	}
	if (delta > 0.4) 
	{	val = ceil(val);	}
	val /= pow(10.0,decPlace);
	return val;
}


/// <summary>
/// Gets the bytes.
/// </summary>
/// <param name="value">The value.</param>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ThorHPD::GetBytes(int value)
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
double ThorHPD::Round(double number, int decimals)
{
	double decP = std::pow(10,decimals);
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
long ThorHPD::BuildParamTable()
{
	DestroyParamTable();
	//Execution in order of top to bottom of list:
	std::vector<unsigned char> commandBytes;
	commandBytes.push_back(0x00);
	ParamInfo* tempParamInfo = new ParamInfo(	
		PARAM_DEVICE_TYPE,						//ID
		"PARAM_DEVICE_TYPE",					//Parameter description
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
	_tableParams.insert(std::pair<long, ParamInfo*>(Params::PARAM_DEVICE_TYPE,tempParamInfo));

	//build table entry To turn PMT ON
	unsigned char commandBytesEnable[] = {0x05, 0x45, 0x00, 0x01, 0x00, 0x00};
	commandBytes.assign(commandBytesEnable, commandBytesEnable + sizeof(commandBytesEnable)/sizeof(commandBytesEnable[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_ENABLE,
		"PARAM_PMT1_ENABLE",
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_ENABLE,tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_ENABLE,
		"PARAM_PMT2_ENABLE",
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_ENABLE,tempParamInfo));

	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_ENABLE,
		"PARAM_PMT3_ENABLE",
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_ENABLE,tempParamInfo));
	
	//build table entry To turn PMT ON
	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_ENABLE,
		"PARAM_PMT4_ENABLE",
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
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_ENABLE,tempParamInfo));
	
	//build table entry To set gain
	unsigned char commandBytesGain[] = {0x10, 0x45, 0x04, 0x00, 0x80, 0x00, 0x02, 0x0D, 0x00, 0x00}; 
	commandBytes.assign(commandBytesGain, commandBytesGain + sizeof(commandBytesGain)/sizeof(commandBytesGain[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_PMT1_GAIN_POS,
		"PARAM_PMT1_GAIN_POS",
		_defaultGain[0],
		_defaultGain[0],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		commandBytes,
		0,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS,tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(	
		PARAM_PMT2_GAIN_POS,
		"PARAM_PMT2_GAIN_POS",
		_defaultGain[1],
		_defaultGain[1],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		commandBytes,
		1,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS,tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(	
		PARAM_PMT3_GAIN_POS,
		"PARAM_PMT3_GAIN_POS",
		_defaultGain[2],
		_defaultGain[2],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		commandBytes,
		2,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS,tempParamInfo));

	//build table entry To set gain
	tempParamInfo = new ParamInfo(	
		PARAM_PMT4_GAIN_POS,
		"PARAM_PMT4_GAIN_POS",
		_defaultGain[3],
		_defaultGain[3],
		FALSE,
		TYPE_LONG,
		TRUE,
		FALSE,
		GAIN_MIN,
		GAIN_MAX,
		GAIN_MIN,
		commandBytes,
		3,
		-1);
	_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS,tempParamInfo));

	////build table entry To turn set gain
	//unsigned char commandBytesGain[] = {0x10, 0x45, 0x04, 0x00, 0x80, 0x00, 0x02, 0x0D, 0x00, 0x00}; 
	//commandBytes.assign(commandBytesGain, commandBytesGain + sizeof(commandBytesGain)/sizeof(commandBytesGain[0]));
	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT1_GAIN_POS,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	GAIN_MIN,
	//	GAIN_MAX,
	//	GAIN_MIN,
	//	commandBytes,
	//	0);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_GAIN_POS,tempParamInfo));

	////build table entry To turn set gain
	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT2_GAIN_POS,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	GAIN_MIN,
	//	GAIN_MAX,
	//	GAIN_MIN,
	//	commandBytes,
	//	1);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_GAIN_POS,tempParamInfo));

	////build table entry To turn set gain
	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT3_GAIN_POS,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	GAIN_MIN,
	//	GAIN_MAX,
	//	GAIN_MIN,
	//	commandBytes,
	//	2);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_GAIN_POS,tempParamInfo));

	////build table entry To turn set gain
	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT4_GAIN_POS,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	GAIN_MIN,
	//	GAIN_MAX,
	//	GAIN_MIN,
	//	commandBytes,
	//	3);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_GAIN_POS,tempParamInfo));

	////build table entry To turn set vbr
	//unsigned char commandBytesVbr[] = {0x22, 0x45, 0x04, 0x00, 0x80, 0x00, 0xE3, 0x0C, 0x00, 0x00}; 
	//commandBytes.assign(commandBytesGain, commandBytesGain + sizeof(commandBytesGain)/sizeof(commandBytesGain[0]));
	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT1_VBR,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	VBR_MIN,
	//	VBR_MAX,
	//	VBR_MIN,
	//	commandBytes,
	//	0);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT1_VBR,tempParamInfo));

	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT2_VBR,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	VBR_MIN,
	//	VBR_MAX,
	//	VBR_MIN,
	//	commandBytes,
	//	0);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT2_VBR,tempParamInfo));

	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT3_VBR,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	VBR_MIN,
	//	VBR_MAX,
	//	VBR_MIN,
	//	commandBytes,
	//	0);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT3_VBR,tempParamInfo));

	//tempParamInfo = new ParamInfo(	
	//	PARAM_PMT4_VBR,
	//	0,
	//	0,
	//	FALSE,
	//	TYPE_LONG,
	//	TRUE,
	//	FALSE,
	//	VBR_MIN,
	//	VBR_MAX,
	//	VBR_MIN,
	//	commandBytes,
	//	0);
	//_tableParams.insert(std::pair<long, ParamInfo*>(PARAM_PMT4_VBR,tempParamInfo));

	//build table entry for the device connection status
	unsigned char commandBytesCON_STA[] = { 0x00}; // Hex for "/SR<cr>" -> Request SerialNumber
	commandBytes.assign(commandBytesCON_STA, commandBytesCON_STA + sizeof(commandBytesCON_STA)/sizeof(commandBytesCON_STA[0]));
	tempParamInfo = new ParamInfo(	
		PARAM_CONNECTION_STATUS,	//ID
		"PARAM_CONNECTION_STATUS",
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


	return TRUE;
}

/// <summary>
/// Destroys the parameter table.
/// </summary>
/// <returns>long.</returns>
long ThorHPD::DestroyParamTable()
{
	try
	{
		for(std::map<long, ParamInfo*>::iterator iter = _tableParams.begin(); iter != _tableParams.end(); ++iter)
		{
			delete iter->second;
		}
		_tableParams.clear();
	}
	catch(...)
	{
#ifdef LOGGING_ENABLED
		StringCbPrintfW(message,MSG_SIZE,L"ThorHPD DestroyParamTable unable to destroy the table created on heap");
		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
#endif
		return FALSE;
	}

	return TRUE;
}
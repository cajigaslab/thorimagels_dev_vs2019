#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThorChrolis_UnitTest_NS
{
	BEGIN_TEST_MODULE_ATTRIBUTE()
		TEST_MODULE_ATTRIBUTE(L"Project", L"Hardware-Devices-ThorChrolis")
		TEST_MODULE_ATTRIBUTE(L"Owner", L"CWestphal")
		TEST_MODULE_ATTRIBUTE(L"Entity", L"Thorlabs GmbH - Dachau")
		TEST_MODULE_ATTRIBUTE(L"Date", L"05/30/2018")
	END_TEST_MODULE_ATTRIBUTE()

	#include "..\ThorChrolis\ThorChrolis.h"

	TEST_MODULE_INITIALIZE(InitUnitTestModule)
	{
		
	}

	TEST_MODULE_CLEANUP(FinalizeUnitTestModule)
	{
		
	}

	TEST_CLASS(ThorChrolis_UnitTest)
	{
		static DeviceDll* _pDeviceDll;
		static std::wstring _dllName;
		static bool _deviceConnected;

	public:
		BEGIN_TEST_CLASS_ATTRIBUTE()
			TEST_CLASS_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_CLASS_ATTRIBUTE(L"Description", L"UnitTest for the ThorChrolis library")
			TEST_CLASS_ATTRIBUTE(L"Priority", L"Critical")
		END_TEST_CLASS_ATTRIBUTE()

		TEST_CLASS_INITIALIZE(InitUnitTest)
		{
			Logger::WriteMessage(L"Initialize Test Class");

			Assert::IsNull(_pDeviceDll, L"  Dll almost loaded!", LINE_INFO());
			_pDeviceDll = new DeviceDll(_dllName.c_str());
			Assert::IsNotNull(_pDeviceDll, L"  Unable to load dll!", LINE_INFO());

			Logger::WriteMessage(L"  Created '_pDeviceDll' instance");
		}

		TEST_CLASS_CLEANUP(FinalizeUnitTest)
		{
			Logger::WriteMessage(L"Finalize Test Class");

			if (_deviceConnected)
			{
				Logger::WriteMessage(L"  Invoke TeardownDevice");
				long ret = _pDeviceDll->TeardownDevice();

				Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

				Logger::WriteMessage(L"  Device is Closed successfull!");
				_deviceConnected = FALSE;
			}

			Assert::IsNotNull(_pDeviceDll, L"  Dll not loaded!", LINE_INFO());
			delete(_pDeviceDll);
			_pDeviceDll = NULL;

			Logger::WriteMessage(L"  Deleted '_pDeviceDll' instance");
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(LookUpDevices)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Search for connected devices and initialize the first found.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"1")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(LookUpDevices)
		{
			Logger::WriteMessage(L"Function - LookUpDevices");
			ConnectDevice();
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDSerialNumbers)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Read the LED-Module serial numbers.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"2")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDSerialNumbers)
		{
			Logger::WriteMessage(L"Function - GetLEDSerialNumbers");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParamString - 'PARAM_LED1_SN' to 'PARAM_LED2_SN'");
			for (long i = 0; i <= 1; ++i)
			{
				wchar_t serNo[1024] = L"";
				long size = 1024;
				ret = _pDeviceDll->GetParamString(IDevice::PARAM_LED1_SN + i, serNo, size);

				Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Serial Number: %s",
							i + 1 , serNo);
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDPeakWaveLength)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Read the LED-Module peak wave length.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"2")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDPeakWaveLength)
		{
			Logger::WriteMessage(L"Function - GetLEDPeakWaveLength");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParam - 'PARAM_LED1_WAVELENGTH' to 'PARAM_LED2_WAVELENGTH'");
			for (long i = 0; i <= 1; ++i)
			{
				double paramVal = 0;
				ret = _pDeviceDll->GetParam(IDevice::PARAM_LED1_WAVELENGTH + i, paramVal);

				Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Peak Wave Length: %5.2f nm",
							i + 1 , paramVal);
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDControlNames)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Read the LED-Module names.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"2")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDControlNames)
		{
			Logger::WriteMessage(L"Function - GetLEDControlNames");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParamString - 'PARAM_LED1_CONTROL_NAME' to 'PARAM_LED2_CONTROL_NAME'");
			for (long i = 0; i <= 1; ++i)
			{
				wchar_t ctrlName[1024] = L"";
				long size = 1024;
				ret = _pDeviceDll->GetParamString(IDevice::PARAM_LED1_CONTROL_NAME + i, ctrlName, size);

				Assert::IsTrue(TRUE == ret, NULL, LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Control Name: %s",
							i + 1 , ctrlName);
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDControlCustomNames)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Read the LED-Module custom names.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"3")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDControlCustomNames)
		{
			Logger::WriteMessage(L"Function - GetLEDControlCustomNames");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParamString - 'PARAM_LED1_CONTROL_CUSTOM_NAME' to 'PARAM_LED2_CONTROL_CUSTOM_NAME'");
			for (long i = 0; i <= 1; ++i)
			{
				wchar_t customCtrlName[1024] = L"";
				long size = 1024;
				ret = _pDeviceDll->GetParamString(IDevice::PARAM_LED1_CONTROL_CUSTOM_NAME + i, customCtrlName, size);

				Assert::IsTrue(TRUE == ret, NULL, LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Control Custom Name: %s",
							i + 1 , customCtrlName);
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDPowerState)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Request the LED-Module power states.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"4")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDPowerState)
		{
			Logger::WriteMessage(L"Function - GetLEDPowerState");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParam - 'PARAM_LED1_POWER_STATE' to 'PARAM_LED2_POWER_STATE'");
			for (long i = 0; i <= 1; ++i)
			{
				double paramVal = 0;
				ret = _pDeviceDll->GetParam(IDevice::PARAM_LED1_POWER_STATE + i, paramVal);

				Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Power State: %s ",
							i + 1 , TRUE <= paramVal ? L"On" : L"Off");
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(SetLEDPowerStates)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Set the LED-Module power states.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"5")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(SetLEDPowerStates)
		{
			Logger::WriteMessage(L"Function - GetLEDPowerStates");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Switch Power States to 'On'");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED2_POWER_STATE' ");
			double paramVal = 1;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED2_POWER_STATE, paramVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			swprintf_s(logmsg,
						L"     LED2 Power State: %s ",
						L"On");
			Logger::WriteMessage(logmsg);

			Logger::WriteMessage(L"  >> Wait 2 Seconds << ");
			Wait(2);

			Logger::WriteMessage(L"  Switch Power States to 'Off'");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED2_POWER_STATE' ");
			paramVal = 0;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED2_POWER_STATE, paramVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			swprintf_s(logmsg,
						L"     LED2 Power State: %s ",
						L"Off");
			Logger::WriteMessage(logmsg);
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(GetLEDsPower)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Request the LED-Modules power.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"6")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(GetLEDsPower)
		{
			Logger::WriteMessage(L"Function - GetLEDsPower");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Invoke GetParam - 'PARAM_LED1_POWER' to 'PARAM_LED2_POWER'");
			for (long i = 0; i <= 1; ++i)
			{
				double paramVal = 0;
				ret = _pDeviceDll->GetParam(IDevice::PARAM_LED1_POWER + i, paramVal);

				Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

				swprintf_s(logmsg,
							L"   LED%d Power : %4.0f‰ ",
							i + 1 , paramVal);
				Logger::WriteMessage(logmsg);
			}
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(SetLEDsPower)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Set the LED-Module power states.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"7")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(SetLEDsPower)
		{
			Logger::WriteMessage(L"Function - SetLEDsPower");
			ConnectDevice();

			wchar_t logmsg[1024];
			long ret = -1;

			Logger::WriteMessage(L"  Switch Power State to 'On'");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED1_POWER_STATE' ");
			double stateVal = TRUE;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED1_POWER_STATE, stateVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			swprintf_s(logmsg,
						L"     LED1 Power State: %s ",
						L"On");
			Logger::WriteMessage(logmsg);

			Logger::WriteMessage(L"  Set Power to 200‰");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED1_POWER' ");
			double paramVal = 200, paramValSet = 0;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED1_POWER, paramVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			ret = _pDeviceDll->GetParam(IDevice::PARAM_LED1_POWER, paramValSet);
			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			Assert::AreEqual(paramVal, paramValSet, 0.1, L"Unable to set the specified Power.", LINE_INFO());

			swprintf_s(logmsg,
						L"     LED1 Power: %4.0f‰ ",
						paramValSet);
			Logger::WriteMessage(logmsg);

			Logger::WriteMessage(L"  >> Wait 2 Seconds << ");
			Wait(2);

			Logger::WriteMessage(L"  Set Power to 100‰");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED1_POWER' ");
			paramVal = 100, paramValSet = 0;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED1_POWER, paramVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			ret = _pDeviceDll->GetParam(IDevice::PARAM_LED1_POWER, paramValSet);
			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			Assert::AreEqual(paramVal, paramValSet, 0.1, L"Unable to set the specified Power.", LINE_INFO());

			swprintf_s(logmsg,
						L"   LED1 Power: %4.0f‰ ",
						paramValSet);
			Logger::WriteMessage(logmsg);

			Logger::WriteMessage(L"  >> Wait 2 Seconds << ");
			Wait(2);

			Logger::WriteMessage(L"  Switch Power State to 'Off'");
			Logger::WriteMessage(L"   Invoke SetParam - 'PARAM_LED1_POWER_STATE' ");
			stateVal = FALSE;
			//_pDeviceDll->GetParamInfo(IDevice::PARAM_TURRET_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
			// if(TRUE == paramAvailable && (paramVal >= paramMin && paramVal <= paramMax))
			ret = _pDeviceDll->SetParam(IDevice::PARAM_LED1_POWER_STATE, stateVal);
			_pDeviceDll->PreflightPosition();
			_pDeviceDll->SetupPosition();
			_pDeviceDll->StartPosition();
			_pDeviceDll->PostflightPosition();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			swprintf_s(logmsg,
						L"     LED1 Power State: %s ",
						L"Off");
			Logger::WriteMessage(logmsg);
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(CloseDeviceConnection)
			TEST_METHOD_ATTRIBUTE(L"Owner", L"CWestphal")
			TEST_METHOD_ATTRIBUTE(L"Description", L"Shutdown the device and close the connection.")
			TEST_METHOD_ATTRIBUTE(L"Priority", L"100")
		END_TEST_METHOD_ATTRIBUTE()
		TEST_METHOD(CloseDeviceConnection)
		{
			Logger::WriteMessage(L"Function - CloseDeviceConnection");
			ConnectDevice();

			long ret = -1;

			Logger::WriteMessage(L"  Invoke TeardownDevice");
			ret = _pDeviceDll->TeardownDevice();

			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());

			Logger::WriteMessage(L"  Device is Closed successfull!");
			_deviceConnected = FALSE;
		}

		static void ConnectDevice()
		{
			if (TRUE == _deviceConnected)
			{
				return;
			}

			wchar_t logmsg[200];
			long ret = -1;
			long devCount = 0;

			Logger::WriteMessage(L"  Invoke FindDevices");
			ret = _pDeviceDll->FindDevices(devCount);

			swprintf_s(logmsg,
						L"  DeviceCount: %d",
						devCount);
			Assert::IsTrue(TRUE == ret, L"Invalid Function Call", LINE_INFO());
			Assert::IsFalse(0 == devCount, logmsg, LINE_INFO());

			swprintf_s(logmsg,
						L"  Found >%d< device and connected to the first one!",
						devCount);
			Logger::WriteMessage(logmsg);
			_deviceConnected = TRUE;
		}

		static void Wait(int seconds)
		{
			DWORD ms = seconds * 1000;
			Sleep(ms);

			// stupid busy waiting!!
			/*clock_t endwait;
			endwait = clock () + seconds * CLK_TCK ;
			while (clock() < endwait) {}*/
		}
	};
	DeviceDll* ThorChrolis_UnitTest::_pDeviceDll = NULL;
	std::wstring ThorChrolis_UnitTest::_dllName = L"ThorChrolis.dll";
	bool ThorChrolis_UnitTest::_deviceConnected = FALSE;

}
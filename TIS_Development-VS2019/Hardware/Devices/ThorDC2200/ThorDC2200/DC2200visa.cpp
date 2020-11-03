// dlltest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DC2200visa.h"

ViStatus ThorDC2200Visa::StatusCode = 0;

bool ThorDC2200Visa::FindAndOpenDevice(ViSession &viRM, ViSession &session, long &status)
{
	ViFindList detectedDeviceList;
	ViUInt32 nDetectedDevices;
	ViChar address[256];
	StatusCode = viOpenDefaultRM(&viRM);
	if(StatusCode != 0)
	{
		return false;
	}
	StatusCode = viFindRsrc(viRM,"USB0::?*INSTR", &detectedDeviceList, &nDetectedDevices, address);
	if(StatusCode != 0)
	{
		return false;
	}

	StatusCode = viOpen(viRM, address, 0, 10, &session);
	if(StatusCode != 0)
	{
		return false;
	}

	ViChar idnStr[256];
	GetIDNStr(session, idnStr);
	if(strncmp(idnStr, "Thorlabs,DC2200", 15) == 0)
	{
		status = (long)IDevice::ConnectionStatusType::CONNECTION_READY;
		return true;
	}
	else
	{
		for (unsigned long i= 1; i < nDetectedDevices; i++)
		{
			StatusCode = viFindNext(detectedDeviceList, address);
			StatusCode = viOpen(viRM, address, 0, 10, &session);
			if(StatusCode != 0)
			{
				return false;
			}

			GetIDNStr(session, idnStr);
			if(strncmp(idnStr, "Thorlabs,DC2200", 15) == 0)
			{
				status = (long)IDevice::ConnectionStatusType::CONNECTION_READY;
				return true;
			} 
		}
	}
	return false;
}
bool ThorDC2200Visa::CloseDevice(ViSession &viRM, ViSession &session)
{
	StatusCode = viClose(session);
	if(StatusCode != 0)
	{
		return false;
	}
	StatusCode = viClose(viRM);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetIDNStr(ViSession session, char* idnStr)
{
	StatusCode = viQueryf(session,"*IDN?","%t",idnStr);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetTTLMaxCurrent(ViSession session, double &current)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:TTL:CURRent:AMPLitude? MAX", "%le", &current);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetTTLCurrent(ViSession session, double &current)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:TTL:CURRent:AMPLitude?", "%le", &current);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::SetTTLCurrent(ViSession session, double value)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:TTL:CURRent:AMPLitude %le", "", value);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetBrightness(ViSession session, double &brightness)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:CBRightness:BRIGhtness:AMPLitude?", "%le", &brightness);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::SetBrightness(ViSession session, double value)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:CBRightness:BRIGhtness:AMPLitude %le", "", value);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetMode(ViSession session, char* mode)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:MODe?","%t", mode);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::SetMode(ViSession session, char* mode)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "SOURce:MODe %s", "", mode);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::SetOutputState(ViSession session, long state)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "OUTPut:STATe %d", "", state);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
bool ThorDC2200Visa::GetOutputState(ViSession session, long &state)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "OUTPut:STATe?", "%d", &state);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}

bool ThorDC2200Visa::SetTerminal(ViSession session, long terminal)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "OUTPut:TERMinal %d", "", terminal);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}

bool ThorDC2200Visa::GetTerminal(ViSession session, long &terminal)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	StatusCode = viQueryf(session, "OUTPut:TERMinal?","%d", &terminal);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}

bool ThorDC2200Visa::GetLed1Connection(ViSession session, long &stat)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	long code=0;
	
	//StatusCode = viQueryf(session,"OUTPut:TERMinal1:TEST","",code);
	StatusCode = viQueryf(session, "OUTPut:TERMinal1:TEST:STATus?", "%d", &stat);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}

bool ThorDC2200Visa::GetLed2Connection(ViSession session, long &stat)
{
	viLock(session, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	long code=0;
	//StatusCode = viQueryf(session,"OUTPut:TERMinal2:TEST","",code);
	StatusCode = viQueryf(session, "OUTPut:TERMinal2:TEST:STATus?", "%d", &stat);
	viUnlock(session);
	if(StatusCode != 0)
	{
		return false;
	}
	return true;
}
#include "stdafx.h"
#include "math.h"
#include "CaptureSetup.h"
#include "HardwareSetupXML.h"

long _turretPosition=0;

DllExportLiveImage GetTurretPosition(long &pos)
{	
	pos = _turretPosition;
	return TRUE;
}

DllExportLiveImage SetTurretPosition(long pos)
{	
	_turretPosition = pos;
	return TRUE;
}

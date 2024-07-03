// ThorMesoXYZRStage.cpp : Defines the exported functions for the DLL application.

#include "ThorMCM301.h"

#define BUFFER_LENGTH 255
#define  MAX_BUF_SIZE 256

bool isConnected = false;
int bReadProcAlive = true;
bool gIsStopped_X = true;
bool gIsStopped_Y = true;
double xPosition = 0;
double yPosition = 0;

// This is the constructor of a class that has been exported.
// see ThorMesoXYZRStage.h for the class definition

MCM301Stage::~MCM301Stage()
{
	_instanceFlag = false;
	//delete _device;
}

MCM301Stage::MCM301Stage()
{
	_xPosition_B = false;
	_yPosition_B = false;
	_zPosition_B = false;
	_rPosition_B = false;
	_condenserPosition_B = false;
	_xMoveByDistance_B = false;
	_yMoveByDistance_B = false;
	_zMoveByDistance_B = false;
	_rMoveByDistance_B = false;
	_condenserMoveByDistance_B = false;

	_xPosition = 0;
	_yPosition = 0;
	_zPosition = 0;
	_rPosition = 0;
	_condenserPosition = 0;
	_xMoveByDistance = 0;
	_yMoveByDistance = 0;
	_zMoveByDistance = 0;
	_rMoveByDistance = 0;
	_condenserMoveByDistance = 0;

	_xPosition_C = 0;
	_yPosition_C = 0;
	_zPosition_C = 0;
	_rPosition_C = 0;
	_zePosition_C = 0;
	_condenserPosition_C = 0;
}

bool MCM301Stage::_instanceFlag = false;

std::shared_ptr<MCM301Stage> MCM301Stage::_instance(new MCM301Stage());

std::shared_ptr<MCM301> MCM301Stage::_device(new MCM301());

MCM301Stage* MCM301Stage::getInstance()
{
	if (!_instanceFlag)
	{
		_instance.reset(new MCM301Stage());
		_instanceFlag = true;
		return _instance.get();
	}
	else
	{
		return _instance.get();
	}
}

long MCM301Stage::FindDevices(long& DeviceCount)
{
	_device->FindAllDevs(DeviceCount);
	return (DeviceCount > 0) ? TRUE : FALSE;
}

long MCM301Stage::SetParam(const long paramID, const double param)
{
	switch (paramID)
	{
	case PARAM_X_POS:
	{
		if (param >= _device->_mcm301Params->xMin && param <= _device->_mcm301Params->xMax)
		{
			_xPosition = param * ((_device->_mcm301Params->xInvert) ? -1 : 1);
			_xPosition_B = true;
		}
		break;
	}

	case PARAM_Y_POS:
	{
		if (param >= _device->_mcm301Params->yMin && param <= _device->_mcm301Params->yMax)
		{
			_yPosition = param * ((_device->_mcm301Params->yInvert) ? -1 : 1);
			_yPosition_B = true;
		}
		break;
	}

	case PARAM_Z_POS:
	{
		if (param >= _device->_mcm301Params->zMin && param <= _device->_mcm301Params->zMax)
		{
			_zPosition = param * ((_device->_mcm301Params->zInvert) ? -1 : 1);
			_zPosition_B = true;
		}
		break;
	}

	case PARAM_R_POS:
	{
		if (param >= _device->_mcm301Params->rMin && param <= _device->_mcm301Params->rMax)
		{
			_rPosition = param / 1e3 * ((_device->_mcm301Params->rInvert) ? -1 : 1);
			_rPosition_B = true;
		}
		break;
	}

	case PARAM_CONDENSER_POS:
	{
		if (param >= _device->_mcm301Params->condenserMin && param <= _device->_mcm301Params->condenserMax)
		{
			_condenserPosition = param * ((_device->_mcm301Params->condenserInvert) ? -1 : 1);
			_condenserPosition_B = true;
		}
		break;
	}

	case PARAM_X_JOG:
	{
		if (param == 0)
			_device->XJogCW();
		else
			_device->XJogCCW();
		break;
	}
	case PARAM_Y_JOG:
	{
		if (param == 0)
			_device->YJogCW();
		else
			_device->YJogCCW();
		break;
	}
	case PARAM_Z_JOG:
	{
		if (param == 0)
			_device->ZJogCW();
		else
			_device->ZJogCCW();
		break;
	}
	case PARAM_R_JOG:
	{
		if (param == 0)
			_device->RJogCW();
		else
			_device->RJogCCW();
		break;
	}
	case PARAM_CONDENSER_JOG:
	{
		if (param == 0)
			_device->CondenserJogCW();
		else
			_device->CondenserJogCCW();
		break;
	}
	case PARAM_X_ZERO:
	{
		clock_t timeOutStart = clock();
		while (_device->IsXmoving() && (static_cast<unsigned long>(abs(timeOutStart - clock()) / (CLOCKS_PER_SEC / 1000)) < 10000))
		{
			// do not set zero until the stage stops moving or 10 seconds have passed. Temporary fix
		}
		_device->ZeroX();
		break;
	}
	case PARAM_Y_ZERO:
	{
		clock_t timeOutStart = clock();
		while (_device->IsYmoving() && (static_cast<unsigned long>(abs(timeOutStart - clock()) / (CLOCKS_PER_SEC / 1000)) < 10000))
		{
			// do not set zero until the stage stops moving or 10 seconds have passed. Temporary fix
		}
		_device->ZeroY();
		break;
	}
	case PARAM_Z_ZERO:
	{
		clock_t timeOutStart = clock();
		while (_device->IsZmoving() && (static_cast<unsigned long>(abs(timeOutStart - clock()) / (CLOCKS_PER_SEC / 1000)) < 10000))
		{
			// do not set zero until the stage stops moving or 10 seconds have passed. Temporary fix
		}
		_device->ZeroZ();
		break;
	}
	case PARAM_R_ZERO:
	{
		clock_t timeOutStart = clock();
		while (_device->IsRmoving() && (static_cast<unsigned long>(abs(timeOutStart - clock()) / (CLOCKS_PER_SEC / 1000)) < 10000))
		{
			// do not set zero until the stage stop moving or 10 seconds have passed. Temporary fix
		}
		_device->ZeroR();
		break;
	}
	case PARAM_CONDENSER_ZERO:
	{
		clock_t timeOutStart = clock();
		while (_device->IsCondenserMoving() && (static_cast<unsigned long>(abs(timeOutStart - clock()) / (CLOCKS_PER_SEC / 1000)) < 10000))
		{
			// do not set zero until the stage stops moving or 10 seconds have passed. Temporary fix
		}
		_device->ZeroCondenser();
		break;
	}
	case PARAM_X_INVERT:
	{
		_device->_mcm301Params->xInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_Y_INVERT:
	{
		_device->_mcm301Params->yInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_Z_INVERT:
	{
		_device->_mcm301Params->zInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_R_INVERT:
	{
		_device->_mcm301Params->rInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_CONDENSER_INVERT:
	{
		_device->_mcm301Params->condenserInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_X_STOP:
	{
		_device->StopX();
		break;
	}
	case PARAM_Y_STOP:
	{
		_device->StopY();
		break;
	}
	case PARAM_Z_STOP:
	{
		_device->StopZ();
		break;
	}
	case PARAM_R_STOP:
	{
		_device->StopR();
		break;
	}
	case PARAM_CONDENSER_STOP:
	{
		_device->StopCondenser();
		break;
	}
	case PARAM_X_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm301Params->xMoveByThreshold) && param <= _device->_mcm301Params->xMoveByThreshold)
		{
			_xMoveByDistance = param * ((_device->_mcm301Params->xInvert) ? -1 : 1);
			_xMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_Y_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm301Params->yMoveByThreshold) && param <= _device->_mcm301Params->yMoveByThreshold)
		{
			_yMoveByDistance = param * ((_device->_mcm301Params->yInvert) ? -1 : 1);
			_yMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_Z_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm301Params->zMoveByThreshold) && param <= _device->_mcm301Params->zMoveByThreshold)
		{
			_zMoveByDistance = param * ((_device->_mcm301Params->zInvert) ? -1 : 1);
			_zMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_R_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm301Params->rMoveByThreshold) && param <= _device->_mcm301Params->rMoveByThreshold)
		{
			_rMoveByDistance = param / 1e3 * ((_device->_mcm301Params->rInvert) ? -1 : 1);
			_rMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_CONDENSER_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm301Params->condenserMoveByThreshold) && param <= _device->_mcm301Params->condenserMoveByThreshold)
		{
			_condenserMoveByDistance = param * ((_device->_mcm301Params->condenserInvert) ? -1 : 1);
			_condenserMoveByDistance_B = true;
		}
		break;
	}
	}
	return TRUE;
}

long MCM301Stage::SelectDevice(const long Device)
{
	if (_device->SelectAndConnect(Device))
	{
		return TRUE;
	}
	return FALSE;
}

long MCM301Stage::TeardownDevice() {
	_device->Close();
	return TRUE;
}

long MCM301Stage::GetParamInfo
(
	const long	paramID,
	long& paramType,
	long& paramAvailable,
	long& paramReadOnly,
	double& paramMin,
	double& paramMax,
	double& paramDefault
)
{

	long	ret = TRUE;

	switch (paramID)
	{

	case PARAM_CONNECTION_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = CONNECTION_READY;
		paramMax = CONNECTION_READY;
		paramDefault = CONNECTION_READY;
		break;
	}

	case PARAM_X_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->xMin;
		paramMax = _device->_mcm301Params->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->yMin;
		paramMax = _device->_mcm301Params->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->zMin;
		paramMax = _device->_mcm301Params->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->rMin;
		paramMax = _device->_mcm301Params->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->condenserMin;
		paramMax = _device->_mcm301Params->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->xMin;
		paramMax = _device->_mcm301Params->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->yMin;
		paramMax = _device->_mcm301Params->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->zMin;
		paramMax = _device->_mcm301Params->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->rMin;
		paramMax = _device->_mcm301Params->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->condenserMin;
		paramMax = _device->_mcm301Params->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->xMin;
		paramMax = _device->_mcm301Params->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->yMin;
		paramMax = _device->_mcm301Params->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->zMin;
		paramMax = _device->_mcm301Params->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->rMin;
		paramMax = _device->_mcm301Params->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->condenserMin;
		paramMax = _device->_mcm301Params->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->xMin;
		paramMax = _device->_mcm301Params->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->yMin;
		paramMax = _device->_mcm301Params->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->zMin;
		paramMax = _device->_mcm301Params->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->rMin;
		paramMax = _device->_mcm301Params->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm301Params->condenserMin;
		paramMax = _device->_mcm301Params->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_INVERT:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_Y_INVERT:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_Z_INVERT:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_R_INVERT:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_CONDENSER_INVERT:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_X_STOP:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;
	}

	case PARAM_Y_STOP:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;
	}

	case PARAM_Z_STOP:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;
	}

	case PARAM_R_STOP:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;
	}

	case PARAM_CONDENSER_STOP:
	{
		paramType = TYPE_BOOL;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
		break;
	}

	case PARAM_SCOPE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_DEVICE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMax = paramDefault = (STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R);
		paramMin = STAGE_X;
	}
	break;
	case PARAM_DEVICE_STATUS_MESSAGE:
	{
		paramType = TYPE_STRING;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
	}
	break;
	// MOVE_BY params are only used in standalone app for now
	case PARAM_X_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm301Params->xMoveByThreshold);
		paramMax = _device->_mcm301Params->xMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm301Params->yMoveByThreshold);
		paramMax = _device->_mcm301Params->yMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm301Params->zMoveByThreshold);
		paramMax = _device->_mcm301Params->zMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm301Params->rMoveByThreshold);
		paramMax = _device->_mcm301Params->rMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm301Params->condenserMoveByThreshold);
		paramMax = _device->_mcm301Params->condenserMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_ELEVATOR_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = _device->_mcm301Params->zeConfigured;
		paramReadOnly = FALSE;
		paramMin = -FLT_MAX;
		paramMax = FLT_MAX;
		paramDefault = 0;
	}
	break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long MCM301Stage::GetParam(const long paramID, double& param)
{

	long	ret = TRUE;

	switch (paramID)
	{
	case PARAM_X_POS:
		param = _xPosition * ((_device->_mcm301Params->xInvert) ? -1 : 1);
		break;
	case PARAM_Y_POS:
		param = _yPosition * ((_device->_mcm301Params->yInvert) ? -1 : 1);
		break;
	case PARAM_Z_POS:
		param = _zPosition * ((_device->_mcm301Params->zInvert) ? -1 : 1);
		break;
	case PARAM_R_POS:
		param = _rPosition * 1e3 * ((_device->_mcm301Params->rInvert) ? -1 : 1);
		break;
	case PARAM_CONDENSER_POS:
		param = _condenserPosition * ((_device->_mcm301Params->condenserInvert) ? -1 : 1);
		break;
	case PARAM_X_POS_CURRENT:
		_device->GetXPos(_xPosition_C);
		param = _xPosition_C * ((_device->_mcm301Params->xInvert) ? -1 : 1);
		break;
	case PARAM_Y_POS_CURRENT:
		_device->GetYPos(_yPosition_C);
		param = _yPosition_C * ((_device->_mcm301Params->yInvert) ? -1 : 1);
		break;
	case PARAM_Z_POS_CURRENT:
		_device->GetZPos(_zPosition_C);
		param = _zPosition_C * ((_device->_mcm301Params->zInvert) ? -1 : 1);
		break;
	case PARAM_R_POS_CURRENT:
		_device->GetRPos(_rPosition_C);
		param = _rPosition_C * 1e3 * ((_device->_mcm301Params->rInvert) ? -1 : 1);
		break;
	case PARAM_CONDENSER_POS_CURRENT:
		_device->GetCondenserPos(_condenserPosition_C);
		param = _condenserPosition_C * ((_device->_mcm301Params->condenserInvert) ? -1 : 1);
		break;
	case PARAM_DEVICE_TYPE:
	{
		long configuredTypes = 0;
		if (TRUE == _device->_mcm301Params->xConfigured && TRUE == _device->_mcm301Params->yConfigured)
		{
			configuredTypes |= (STAGE_X | STAGE_Y);
		}
		if (_device->_mcm301Params->zConfigured)
		{
			configuredTypes |= (STAGE_Z | STAGE_Z2);
		}
		if (_device->_mcm301Params->rConfigured)
		{
			configuredTypes |= STAGE_R;
		}
		param = static_cast<double>(configuredTypes);
		break;
	}
	case PARAM_CONNECTION_STATUS:
	{
		param = (!_device->IsConnected()) ? CONNECTION_UNAVAILABLE : CONNECTION_READY;
		break;
	}
	case PARAM_X_INVERT:
	{
		param = (_device->_mcm301Params->xInvert) ? 1 : 0;
		break;
	}
	case PARAM_Y_INVERT:
	{
		param = (_device->_mcm301Params->yInvert) ? 1 : 0;
		break;
	}
	case PARAM_Z_INVERT:
	{
		param = (_device->_mcm301Params->zInvert) ? 1 : 0;
		break;
	}
	case PARAM_R_INVERT:
	{
		param = (_device->_mcm301Params->rInvert) ? 1 : 0;
		break;
	}
	case PARAM_CONDENSER_INVERT:
	{
		param = (_device->_mcm301Params->condenserInvert) ? 1 : 0;
		break;
	}
	case PARAM_SCOPE_TYPE:
	{
		param = static_cast<int>(_device->_scopeType);
		break;
	}
	
	case PARAM_Z_ELEVATOR_POS_CURRENT:
	{
		_device->GetZElevatorPos(_zePosition_C);
		param = _zePosition_C;
		break;
	}
	
	default:
		ret = FALSE;
	}

	return ret;
}
long MCM301Stage::SetParamBuffer(const long paramID, char* buffer, long size) {
	return TRUE;
}
long MCM301Stage::GetParamBuffer(const long paramID, char* buffer, long size) {
	return TRUE;
}
long MCM301Stage::SetParamString(const long paramID, wchar_t* str) {
	return TRUE;
}
long MCM301Stage::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;
	switch (paramID)
	{
	case PARAM_DEVICE_SERIALNUMBER:
		ret = _device->SerialNumber(str, size);
		break;
	case PARAM_DEVICE_FIRMWAREVERSION:
		ret = _device->FirmwareVersion(str, size);
		break;
	case PARAM_DEVICE_STATUS_MESSAGE:
		ret = TRUE;
		break;
	}
	return ret;
}
long MCM301Stage::PreflightPosition() {
	return TRUE;
}
long MCM301Stage::SetupPosition() {
	return TRUE;
}
long MCM301Stage::StartPosition() {
	if (_device->IsConnected())
	{
		if (_xPosition_B)
		{
			double xMin = _device->_mcm301Params->xMin;
			double xMax = _device->_mcm301Params->xMax;
			if (_device->_mcm301Params->xInvert)
			{
				xMin = -1 * _device->_mcm301Params->xMax;
				xMax = -1 * _device->_mcm301Params->xMin;
			}

			if (_xPosition >= xMin && _xPosition <= xMax)
			{
				double currentXPos = 0;
				_device->GetXPos(currentXPos);

				if (currentXPos >= xMin && currentXPos <= xMax)
				{
					double diff = (_xPosition - currentXPos) * ((_device->_mcm301Params->xInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm301Params->xThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large X stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the X stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm301Params->xThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm301Params->xMoveByThreshold)
						{
							_device->MoveXBy(diff * 1e3);
						}
						else
						{
							_device->MoveXTo(_xPosition * 1e3);
						}
						_xPosition_B = false;
					}
					else
					{
						_device->GetXPos(currentXPos);
						_xPosition = currentXPos;
						_xPosition_B = false;
					}
				}
			}
		}

		if (_yPosition_B)
		{
			double yMin = _device->_mcm301Params->yMin;
			double yMax = _device->_mcm301Params->yMax;
			if (_device->_mcm301Params->yInvert)
			{
				yMin = -1 * _device->_mcm301Params->yMax;
				yMax = -1 * _device->_mcm301Params->yMin;
			}

			if (_yPosition >= yMin && _yPosition <= yMax)
			{
				double currentYPos = 0;
				_device->GetYPos(currentYPos);

				if (currentYPos >= yMin && currentYPos <= yMax)
				{
					double diff = (_yPosition - currentYPos) * ((_device->_mcm301Params->yInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm301Params->yThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Y stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Y stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm301Params->yThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm301Params->yMoveByThreshold)
						{
							_device->MoveYBy(diff * 1e3);
						}
						else
						{
							_device->MoveYTo(_yPosition * 1e3);
						}
						_yPosition_B = false;
					}
					else
					{
						_device->GetYPos(currentYPos);
						_yPosition = currentYPos;
						_yPosition_B = false;
					}
				}
			}
		}

		if (_zPosition_B)
		{
			double zMin = _device->_mcm301Params->zMin;
			double zMax = _device->_mcm301Params->zMax;
			if (_device->_mcm301Params->zInvert)
			{
				zMin = -1 * _device->_mcm301Params->zMax;
				zMax = -1 * _device->_mcm301Params->zMin;
			}

			if (_zPosition >= zMin && _zPosition <= zMax)
			{
				double currentZPos = 0;
				_device->GetZPos(currentZPos);

				if (currentZPos >= zMin && currentZPos <= zMax)
				{
					double diff = (_zPosition - currentZPos) * ((_device->_mcm301Params->zInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm301Params->zThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Z stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Z stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm301Params->zThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm301Params->zMoveByThreshold)
						{
							_device->MoveZBy(diff * 1e3);
						}
						else
						{
							_device->MoveZTo(_zPosition * 1e3);
						}
						_zPosition_B = false;
					}
					else
					{
						_device->GetZPos(currentZPos);
						_zPosition = currentZPos;
						_zPosition_B = false;
					}
				}
			}
		}

		if (_rPosition_B)
		{
			double rMin = _device->_mcm301Params->rMin;
			double rMax = _device->_mcm301Params->rMax;
			if (_device->_mcm301Params->rInvert)
			{
				rMin = -1 * _device->_mcm301Params->rMax;
				rMax = -1 * _device->_mcm301Params->rMin;
			}

			if (_rPosition >= rMin && _rPosition <= rMax)
			{
				double currentRPos = 0;
				_device->GetRPos(currentRPos);

				if (currentRPos >= rMin && currentRPos <= rMax)
				{
					double diff = (_rPosition - currentRPos) * ((_device->_mcm301Params->rInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm301Params->rThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large R stage move (%d.%03d degrees). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the R stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm301Params->rThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm301Params->rMoveByThreshold)
						{
							_device->MoveRBy(diff * 1e3);
						}
						else
						{
							_device->MoveRTo(_rPosition * 1e3);
						}
						_rPosition_B = false;
					}
					else
					{
						_device->GetRPos(currentRPos);
						_rPosition = currentRPos;
						_rPosition_B = false;
					}
				}
			}
		}

		if (_condenserPosition_B)
		{
			double condenserMin = _device->_mcm301Params->condenserMin;
			double condenserMax = _device->_mcm301Params->condenserMax;
			if (_device->_mcm301Params->condenserInvert)
			{
				condenserMin = -1 * _device->_mcm301Params->condenserMax;
				condenserMax = -1 * _device->_mcm301Params->condenserMin;
			}

			if (_condenserPosition >= condenserMin && _condenserPosition <= condenserMax)
			{
				double currentCondenserPos = 0;
				_device->GetCondenserPos(currentCondenserPos);

				if (currentCondenserPos >= condenserMin && currentCondenserPos <= condenserMax)
				{
					double diff = (_condenserPosition - currentCondenserPos) * ((_device->_mcm301Params->condenserInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm301Params->condenserThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Condenser stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Condenser stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm301Params->condenserThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm301Params->condenserMoveByThreshold)
						{
							_device->MoveCondenserBy(diff * 1e3);
						}
						else
						{
							_device->MoveCondenserTo(_condenserPosition * 1e3);
						}
						_condenserPosition_B = false;
					}
					else
					{
						_device->GetCondenserPos(currentCondenserPos);
						_condenserPosition = currentCondenserPos;
						_condenserPosition_B = false;
					}
				}
			}
		}

		//--- This part is only used by standalone app---//
		if (_xMoveByDistance_B)
		{
			_device->MoveXBy(_xMoveByDistance * 1e3);
			_xMoveByDistance_B = false;
		}
		if (_yMoveByDistance_B)
		{
			_device->MoveYBy(_yMoveByDistance * 1e3);
			_yMoveByDistance_B = false;
		}
		if (_zMoveByDistance_B)
		{
			_device->MoveZBy(_zMoveByDistance * 1e3);
			_zMoveByDistance_B = false;
		}
		if (_rMoveByDistance_B)
		{
			_device->MoveRBy(_rMoveByDistance * 1e3);
			_rMoveByDistance_B = false;
		}
		if (_condenserMoveByDistance_B)
		{
			_device->MoveCondenserBy(_condenserMoveByDistance * 1e3);
			_condenserMoveByDistance_B = false;
		}
		//---------------------------------------------//
	}

	return TRUE;
}
long MCM301Stage::StatusPosition(long& status) {
	_device->StatusPosition(status);
	return TRUE;
}
long MCM301Stage::ReadPosition(DeviceType deviceType, double& pos) {
	return TRUE;
}
long MCM301Stage::PostflightPosition() {
	return TRUE;
}
long MCM301Stage::GetLastErrorMsg(wchar_t* msg, long size) {
	return TRUE;
}

long MCM301Stage::IsMCM301Connected()
{
	return _device->IsConnected() ? TRUE : FALSE;
}

long MCM301Stage::IsCondenserAvailable()
{
	return _device->_mcm301Params->condenserConfigured;
}
// ThorMesoXYZRStage.cpp : Defines the exported functions for the DLL application.

#include "ThorMCM6000.h"

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

MCM6000Stage::~MCM6000Stage()
{
	_instanceFlag = false;
	//delete _device;
}

MCM6000Stage::MCM6000Stage()
{
	_xPosition_B = false;
	_yPosition_B = false;
	_zPosition_B = false;
	_rPosition_B = false;
	_condenserPosition_B = false;
	_lpInvertedPos_B = false;
	_etInvertedPos_B = false;
	_ggLightpathPos_B = false;
	_grLightpathPos_B = false;
	_camLightpathPos_B = false;
	_nddPos_B = false;
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
	_lpInvertedPos = 0;
	_etInvertedPos = 0;
	_ggLightpathPos = 0;
	_grLightpathPos = 0;
	_camLightpathPos = 0;
	_nddPos = 0;
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
	_lpInvertedPos_C = 0;
	_etInvertedPos_C = 0;
	_condenserPosition_C = 0;
	_nddPos_C = 0;
}

bool MCM6000Stage::_instanceFlag = false;

std::shared_ptr<MCM6000Stage> MCM6000Stage::_instance(new MCM6000Stage());

std::shared_ptr<MCM6000> MCM6000Stage::_device(new MCM6000());

MCM6000Stage* MCM6000Stage::getInstance()
{
	if (!_instanceFlag)
	{
		_instance.reset(new MCM6000Stage());
		_instanceFlag = true;
		return _instance.get();
	}
	else
	{
		return _instance.get();
	}
}

long MCM6000Stage::FindDevices(long& DeviceCount)
{
	_device->FindAllDevs(DeviceCount);
	return (DeviceCount > 0) ? TRUE : FALSE;
}

long MCM6000Stage::SetParam(const long paramID, const double param)
{
	switch (paramID)
	{
	case PARAM_X_POS:
	{
		if (param >= _device->_mcm6kParams->xMin && param <= _device->_mcm6kParams->xMax)
		{
			_xPosition = param * ((_device->_mcm6kParams->xInvert) ? -1 : 1);
			_xPosition_B = true;
		}
		break;
	}

	case PARAM_Y_POS:
	{
		if (param >= _device->_mcm6kParams->yMin && param <= _device->_mcm6kParams->yMax)
		{
			_yPosition = param * ((_device->_mcm6kParams->yInvert) ? -1 : 1);
			_yPosition_B = true;
		}
		break;
	}

	case PARAM_Z_POS:
	{
		if (param >= _device->_mcm6kParams->zMin && param <= _device->_mcm6kParams->zMax)
		{
			_zPosition = param * ((_device->_mcm6kParams->zInvert) ? -1 : 1);
			_zPosition_B = true;
		}
		break;
	}

	case PARAM_R_POS:
	{
		if (param >= _device->_mcm6kParams->rMin && param <= _device->_mcm6kParams->rMax)
		{
			_rPosition = param / 1e3 * ((_device->_mcm6kParams->rInvert) ? -1 : 1);
			_rPosition_B = true;
		}
		break;
	}

	case PARAM_CONDENSER_POS:
	{
		if (param >= _device->_mcm6kParams->condenserMin && param <= _device->_mcm6kParams->condenserMax)
		{
			_condenserPosition = param * ((_device->_mcm6kParams->condenserInvert) ? -1 : 1);
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
		_device->_mcm6kParams->xInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_Y_INVERT:
	{
		_device->_mcm6kParams->yInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_Z_INVERT:
	{
		_device->_mcm6kParams->zInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_R_INVERT:
	{
		_device->_mcm6kParams->rInvert = (param > 0.5) ? true : false;
		break;
	}
	case PARAM_CONDENSER_INVERT:
	{
		_device->_mcm6kParams->condenserInvert = (param > 0.5) ? true : false;
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
	case PARAM_LIGHTPATH_GG:
	{
		_ggLightpathPos = (int)param;
		_ggLightpathPos_B = true;
		break;
	}
	case PARAM_LIGHTPATH_GR:
	{
		_grLightpathPos = (int)param;
		_grLightpathPos_B = true;
		break;
	}
	case PARAM_LIGHTPATH_CAMERA:
	{
		_camLightpathPos = (int)param;
		_camLightpathPos_B = true;
		break;
	}
	case PARAM_LIGHTPATH_INVERTED_POS:
	{
		_lpInvertedPos = (int)param;
		_lpInvertedPos_B = true;
		break;
	}
	case PARAM_EPI_TURRET_POS:
	{
		_etInvertedPos = (int)param;
		_etInvertedPos_B = true;
		break;
	}
	case PARAM_X_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm6kParams->xMoveByThreshold) && param <= _device->_mcm6kParams->xMoveByThreshold)
		{
			_xMoveByDistance = param * ((_device->_mcm6kParams->xInvert) ? -1 : 1);
			_xMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_Y_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm6kParams->yMoveByThreshold) && param <= _device->_mcm6kParams->yMoveByThreshold)
		{
			_yMoveByDistance = param * ((_device->_mcm6kParams->yInvert) ? -1 : 1);
			_yMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_Z_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm6kParams->zMoveByThreshold) && param <= _device->_mcm6kParams->zMoveByThreshold)
		{
			_zMoveByDistance = param * ((_device->_mcm6kParams->zInvert) ? -1 : 1);
			_zMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_R_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm6kParams->rMoveByThreshold) && param <= _device->_mcm6kParams->rMoveByThreshold)
		{
			_rMoveByDistance = param / 1e3 * ((_device->_mcm6kParams->rInvert) ? -1 : 1);
			_rMoveByDistance_B = true;
		}
		break;
	}

	case PARAM_CONDENSER_POS_MOVE_BY:
	{
		if (param >= (-1 * _device->_mcm6kParams->condenserMoveByThreshold) && param <= _device->_mcm6kParams->condenserMoveByThreshold)
		{
			_condenserMoveByDistance = param * ((_device->_mcm6kParams->condenserInvert) ? -1 : 1);
			_condenserMoveByDistance_B = true;
		}
		break;
	}
	case PARAM_LIGHTPATH_NDD:
	{
		_nddPos = (int)param;
		_nddPos_B = true;
		break;
	}
	}
	return TRUE;
}

long MCM6000Stage::SelectDevice(const long Device)
{
	if (_device->SelectAndConnect(Device))
	{
		return TRUE;
	}
	return FALSE;
}

long MCM6000Stage::TeardownDevice() {
	_device->Close();
	return TRUE;
}

long MCM6000Stage::GetParamInfo
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
		paramMin = _device->_mcm6kParams->xMin;
		paramMax = _device->_mcm6kParams->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->yMin;
		paramMax = _device->_mcm6kParams->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->zMin;
		paramMax = _device->_mcm6kParams->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->rMin;
		paramMax = _device->_mcm6kParams->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->condenserMin;
		paramMax = _device->_mcm6kParams->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->xMin;
		paramMax = _device->_mcm6kParams->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->yMin;
		paramMax = _device->_mcm6kParams->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->zMin;
		paramMax = _device->_mcm6kParams->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->rMin;
		paramMax = _device->_mcm6kParams->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->condenserMin;
		paramMax = _device->_mcm6kParams->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->xMin;
		paramMax = _device->_mcm6kParams->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->yMin;
		paramMax = _device->_mcm6kParams->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->zMin;
		paramMax = _device->_mcm6kParams->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->rMin;
		paramMax = _device->_mcm6kParams->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_STATUS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->condenserMin;
		paramMax = _device->_mcm6kParams->condenserMax;
		paramDefault = 0;
	}
	break;

	case PARAM_X_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->xMin;
		paramMax = _device->_mcm6kParams->xMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->yMin;
		paramMax = _device->_mcm6kParams->yMax;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->zMin;
		paramMax = _device->_mcm6kParams->zMax;
		paramDefault = 0;
	}
	break;

	case PARAM_R_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->rMin;
		paramMax = _device->_mcm6kParams->rMax;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_ZERO:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = _device->_mcm6kParams->condenserMin;
		paramMax = _device->_mcm6kParams->condenserMax;
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

	case PARAM_LIGHTPATH_GG:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_LIGHTPATH_GR:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_LIGHTPATH_CAMERA:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
		break;
	}

	case PARAM_LIGHTPATH_INVERTED_POS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 2;
		paramDefault = 0;
		break;
	}
	case PARAM_FW_DIC_POS:
	case PARAM_EPI_TURRET_POS:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = NUM_OF_TURRET_POS;
		paramDefault = 0;
		break;
	}

	case PARAM_DEVICE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMax = paramDefault = (STAGE_X | STAGE_Y | STAGE_Z | STAGE_Z2 | STAGE_R | LIGHT_PATH | FILTER_WHEEL_DIC);
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
		paramMin = (-1 * _device->_mcm6kParams->xMoveByThreshold);
		paramMax = _device->_mcm6kParams->xMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Y_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm6kParams->yMoveByThreshold);
		paramMax = _device->_mcm6kParams->yMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm6kParams->zMoveByThreshold);
		paramMax = _device->_mcm6kParams->zMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_R_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm6kParams->rMoveByThreshold);
		paramMax = _device->_mcm6kParams->rMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_CONDENSER_POS_MOVE_BY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = (-1 * _device->_mcm6kParams->condenserMoveByThreshold);
		paramMax = _device->_mcm6kParams->condenserMoveByThreshold;
		paramDefault = 0;
	}
	break;

	case PARAM_Z_ELEVATOR_POS_CURRENT:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = _device->_mcm6kParams->zeConfigured;
		paramReadOnly = FALSE;
		paramMin = -FLT_MAX;
		paramMax = FLT_MAX;
		paramDefault = 0;
	}
	break;
	case PARAM_LIGHTPATH_NDD_AVAILABLE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
	}
	break;
	case PARAM_LIGHTPATH_NDD:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = 0;
		paramMax = 1;
		paramDefault = 0;
	}
	break;
	case PARAM_SHUTTER_1_STATE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = SHUTTER_CLOSED;
		paramMax = SHUTTER_OPENED;
		paramDefault = SHUTTER_CLOSED;
	}
	break;
	case PARAM_SHUTTER_2_STATE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = SHUTTER_CLOSED;
		paramMax = SHUTTER_OPENED;
		paramDefault = SHUTTER_CLOSED;
	}
	break;
	case PARAM_SHUTTER_3_STATE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = SHUTTER_CLOSED;
		paramMax = SHUTTER_OPENED;
		paramDefault = SHUTTER_CLOSED;
	}
	break;
	case PARAM_SHUTTER_4_STATE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = SHUTTER_CLOSED;
		paramMax = SHUTTER_OPENED;
		paramDefault = SHUTTER_CLOSED;
	}
	break;
	case PARAM_SHUTTER_AVAILABLE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
	}
	break;
	case PARAM_SHUTTER_SAFETY_INTERLOCK_STATE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = SHUTTER_CLOSED;
		paramMax = SHUTTER_OPENED;
		paramDefault = SHUTTER_CLOSED;
	}
	break;
	case PARAM_EPI_TURRET_AVAILABLE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = FALSE;
		paramMax = TRUE;
		paramDefault = FALSE;
	}
	break;
	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}

	return ret;
}

long MCM6000Stage::GetParam(const long paramID, double& param)
{

	long	ret = TRUE;

	switch (paramID)
	{
	case PARAM_X_POS:
		param = _xPosition * ((_device->_mcm6kParams->xInvert) ? -1 : 1);
		break;
	case PARAM_Y_POS:
		param = _yPosition * ((_device->_mcm6kParams->yInvert) ? -1 : 1);
		break;
	case PARAM_Z_POS:
		param = _zPosition * ((_device->_mcm6kParams->zInvert) ? -1 : 1);
		break;
	case PARAM_R_POS:
		param = _rPosition * 1e3 * ((_device->_mcm6kParams->rInvert) ? -1 : 1);
		break;
	case PARAM_CONDENSER_POS:
		param = _condenserPosition * ((_device->_mcm6kParams->condenserInvert) ? -1 : 1);
		break;
	case PARAM_X_POS_CURRENT:
		_device->GetXPos(_xPosition_C);
		param = _xPosition_C * ((_device->_mcm6kParams->xInvert) ? -1 : 1);
		break;
	case PARAM_Y_POS_CURRENT:
		_device->GetYPos(_yPosition_C);
		param = _yPosition_C * ((_device->_mcm6kParams->yInvert) ? -1 : 1);
		break;
	case PARAM_Z_POS_CURRENT:
		_device->GetZPos(_zPosition_C);
		param = _zPosition_C * ((_device->_mcm6kParams->zInvert) ? -1 : 1);
		break;
	case PARAM_R_POS_CURRENT:
		_device->GetRPos(_rPosition_C);
		param = _rPosition_C * 1e3 * ((_device->_mcm6kParams->rInvert) ? -1 : 1);
		break;
	case PARAM_CONDENSER_POS_CURRENT:
		_device->GetCondenserPos(_condenserPosition_C);
		param = _condenserPosition_C * ((_device->_mcm6kParams->condenserInvert) ? -1 : 1);
		break;
	case PARAM_DEVICE_TYPE:
	{
		long configuredTypes = 0;
		if (TRUE == _device->_mcm6kParams->xConfigured && TRUE == _device->_mcm6kParams->yConfigured)
		{
			configuredTypes |= (STAGE_X | STAGE_Y);
		}
		if (_device->_mcm6kParams->zConfigured)
		{
			configuredTypes |= (STAGE_Z | STAGE_Z2);
		}
		if (_device->_mcm6kParams->rConfigured)
		{
			configuredTypes |= STAGE_R;
		}
		if (_device->_mcm6kParams->lightPathConfigured)
		{
			configuredTypes |= LIGHT_PATH;
		}
		if (_device->_mcm6kParams->epiTurretConfigured)
		{
			configuredTypes |= FILTER_WHEEL_DIC;
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
		param = (_device->_mcm6kParams->xInvert) ? 1 : 0;
		break;
	}
	case PARAM_Y_INVERT:
	{
		param = (_device->_mcm6kParams->yInvert) ? 1 : 0;
		break;
	}
	case PARAM_Z_INVERT:
	{
		param = (_device->_mcm6kParams->zInvert) ? 1 : 0;
		break;
	}
	case PARAM_R_INVERT:
	{
		param = (_device->_mcm6kParams->rInvert) ? 1 : 0;
		break;
	}
	case PARAM_CONDENSER_INVERT:
	{
		param = (_device->_mcm6kParams->condenserInvert) ? 1 : 0;
		break;
	}
	case PARAM_SCOPE_TYPE:
	{
		param = static_cast<int>(_device->_scopeType);
		break;
	}
	case PARAM_LIGHTPATH_GG:
	{
		param = _device->_mcm6kParams->lightpathGGPosition;
		break;
	}

	case PARAM_LIGHTPATH_GR:
	{
		param = _device->_mcm6kParams->lightpathGRPosition;
		break;
	}

	case PARAM_LIGHTPATH_CAMERA:
	{
		param = _device->_mcm6kParams->lightpathCameraPosition;
		break;
	}
	case PARAM_LIGHTPATH_INVERTED_POS:
	{
		_device->GetLpPos(_lpInvertedPos_C);
		if ((_lpInvertedPos_C <= 2 && _lpInvertedPos_C >= 0) && INVERTED == _device->_scopeType)
		{
			param = _lpInvertedPos_C;
		}
		else
		{
			param = -1;
		}
		break;
	}
	case PARAM_FW_DIC_POS:		//aka PARAM_EPI_TURRET_POS_CURRENT
	{
		_device->GetEtPos(_etInvertedPos_C);
		param = (double)_etInvertedPos_C + 1; // Offset by 1 for higher level.
		break;
	}
	case PARAM_Z_ELEVATOR_POS_CURRENT:
	{
		_device->GetZElevatorPos(_zePosition_C);
		param = _zePosition_C;
		break;
	}
	case PARAM_LIGHTPATH_NDD:
	{
		_device->GetNDDPos(_nddPos_C);
		param = _nddPos_C;
		break;
	}
	case PARAM_LIGHTPATH_NDD_AVAILABLE:
	{
		param = _device->_mcm6kParams->nddConfigured;
		break;
	}
	case PARAM_SHUTTER_1_STATE:
	{
		param = _device->_mcm6kParams->shuttersPositions[0];
		break;
	}
	case PARAM_SHUTTER_2_STATE:
	{
		param = _device->_mcm6kParams->shuttersPositions[1];
		break;
	}
	case PARAM_SHUTTER_3_STATE:
	{
		param = _device->_mcm6kParams->shuttersPositions[2];
		break;
	}
	case PARAM_SHUTTER_4_STATE:
	{
		param = _device->_mcm6kParams->shuttersPositions[3];
		break;
	}
	case PARAM_SHUTTER_AVAILABLE:
	{
		param = _device->_mcm6kParams->shutterConfigured;
		break;
	}
	case PARAM_SHUTTER_SAFETY_INTERLOCK_STATE:
	{
		param = _device->_mcm6kParams->safetyInterlockState;
		break;
	}
	case PARAM_EPI_TURRET_AVAILABLE:
	{
		param = _device->_mcm6kParams->epiTurretConfigured;
		break;
	}
	default:
		ret = FALSE;
	}

	return ret;
}
long MCM6000Stage::SetParamBuffer(const long paramID, char* buffer, long size) {
	return TRUE;
}
long MCM6000Stage::GetParamBuffer(const long paramID, char* buffer, long size) {
	return TRUE;
}
long MCM6000Stage::SetParamString(const long paramID, wchar_t* str) {
	return TRUE;
}
long MCM6000Stage::GetParamString(const long paramID, wchar_t* str, long size)
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
long MCM6000Stage::PreflightPosition() {
	return TRUE;
}
long MCM6000Stage::SetupPosition() {
	return TRUE;
}
long MCM6000Stage::StartPosition() {
	if (_device->IsConnected())
	{
		if (_xPosition_B)
		{
			double xMin = _device->_mcm6kParams->xMin;
			double xMax = _device->_mcm6kParams->xMax;
			if (_device->_mcm6kParams->xInvert)
			{
				xMin = -1 * _device->_mcm6kParams->xMax;
				xMax = -1 * _device->_mcm6kParams->xMin;
			}

			if (_xPosition >= xMin && _xPosition <= xMax)
			{
				double currentXPos = 0;
				_device->GetXPos(currentXPos);

				if (currentXPos >= xMin && currentXPos <= xMax)
				{
					double diff = (_xPosition - currentXPos) * ((_device->_mcm6kParams->xInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm6kParams->xThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large X stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the X stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm6kParams->xThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm6kParams->xMoveByThreshold)
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
			double yMin = _device->_mcm6kParams->yMin;
			double yMax = _device->_mcm6kParams->yMax;
			if (_device->_mcm6kParams->yInvert)
			{
				yMin = -1 * _device->_mcm6kParams->yMax;
				yMax = -1 * _device->_mcm6kParams->yMin;
			}

			if (_yPosition >= yMin && _yPosition <= yMax)
			{
				double currentYPos = 0;
				_device->GetYPos(currentYPos);

				if (currentYPos >= yMin && currentYPos <= yMax)
				{
					double diff = (_yPosition - currentYPos) * ((_device->_mcm6kParams->yInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm6kParams->yThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Y stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Y stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm6kParams->yThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm6kParams->yMoveByThreshold)
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
			double zMin = _device->_mcm6kParams->zMin;
			double zMax = _device->_mcm6kParams->zMax;
			if (_device->_mcm6kParams->zInvert)
			{
				zMin = -1 * _device->_mcm6kParams->zMax;
				zMax = -1 * _device->_mcm6kParams->zMin;
			}

			if (_zPosition >= zMin && _zPosition <= zMax)
			{
				double currentZPos = 0;
				_device->GetZPos(currentZPos);

				if (currentZPos >= zMin && currentZPos <= zMax)
				{
					double diff = (_zPosition - currentZPos) * ((_device->_mcm6kParams->zInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm6kParams->zThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Z stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Z stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm6kParams->zThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm6kParams->zMoveByThreshold)
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
			double rMin = _device->_mcm6kParams->rMin;
			double rMax = _device->_mcm6kParams->rMax;
			if (_device->_mcm6kParams->rInvert)
			{
				rMin = -1 * _device->_mcm6kParams->rMax;
				rMax = -1 * _device->_mcm6kParams->rMin;
			}

			if (_rPosition >= rMin && _rPosition <= rMax)
			{
				double currentRPos = 0;
				_device->GetRPos(currentRPos);

				if (currentRPos >= rMin && currentRPos <= rMax)
				{
					double diff = (_rPosition - currentRPos) * ((_device->_mcm6kParams->rInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm6kParams->rThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large R stage move (%d.%03d degrees). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the R stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm6kParams->rThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm6kParams->rMoveByThreshold)
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
			double condenserMin = _device->_mcm6kParams->condenserMin;
			double condenserMax = _device->_mcm6kParams->condenserMax;
			if (_device->_mcm6kParams->condenserInvert)
			{
				condenserMin = -1 * _device->_mcm6kParams->condenserMax;
				condenserMax = -1 * _device->_mcm6kParams->condenserMin;
			}

			if (_condenserPosition >= condenserMin && _condenserPosition <= condenserMax)
			{
				double currentCondenserPos = 0;
				_device->GetCondenserPos(currentCondenserPos);

				if (currentCondenserPos >= condenserMin && currentCondenserPos <= condenserMax)
				{
					double diff = (_condenserPosition - currentCondenserPos) * ((_device->_mcm6kParams->condenserInvert) ? -1 : 1);
					int msgRet = 0;
					if (abs(diff) > _device->_mcm6kParams->condenserThreshold)
					{
						wchar_t stageMessage[512];
						StringCbPrintfW(stageMessage, 512, L"Large Condenser stage move (%d.%03d um). Do you want to proceed?\n\nNote: If you are attempting a small stage movement and receiving this message there may be an internal error with the Condenser stage.\nPower cycling the Controller Stage may resolve the issue.", static_cast<long>(diff * 1e3), static_cast<long>(abs(static_cast<long>((diff * 1e3 - static_cast<long>(diff * 1e3)) * 1e3))));
						msgRet = MessageBox(NULL, stageMessage, L"", MB_YESNO);
					}

					if (abs(diff) <= _device->_mcm6kParams->condenserThreshold || msgRet == IDYES)
					{
						if (abs(diff) <= _device->_mcm6kParams->condenserMoveByThreshold)
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

		if (_lpInvertedPos_B)
		{
			_device->GetLpPos(_lpInvertedPos_C);
			//Do not move the Light Path if it is already in that position
			if (_lpInvertedPos_C != _lpInvertedPos)
			{
				_device->MoveLpTo(_lpInvertedPos);
			}
			_lpInvertedPos_B = false;
		}

		if (_etInvertedPos_B)
		{
			_device->GetEtPos(_etInvertedPos_C);
			//Do not move the Epi wheel if it is already in that position
			if (_etInvertedPos_C != _etInvertedPos)
			{
				_device->MoveEtTo(_etInvertedPos);
			}
			_etInvertedPos_B = false;
		}

		if (_ggLightpathPos_B)
		{
			_device->MoveGGLightpath(_ggLightpathPos);
			_ggLightpathPos_B = false;
		}

		if (_grLightpathPos_B)
		{
			_device->MoveGRLightpath(_grLightpathPos);
			_grLightpathPos_B = false;
		}

		if (_camLightpathPos_B)
		{
			_device->MoveCAMLightpath(_camLightpathPos);
			_camLightpathPos_B = false;
		}

		if (_nddPos_B)
		{
			_device->GetNDDPos(_nddPos_C);
			//Do not move the NDD mirror if it is already in that position
			if (_nddPos_C != _nddPos)
			{
				_device->MoveNDDTo(_nddPos);
			}
			_nddPos_B = false;
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
long MCM6000Stage::StatusPosition(long& status) {
	_device->StatusPosition(status);
	return TRUE;
}
long MCM6000Stage::ReadPosition(DeviceType deviceType, double& pos) {
	return TRUE;
}
long MCM6000Stage::PostflightPosition() {
	return TRUE;
}
long MCM6000Stage::GetLastErrorMsg(wchar_t* msg, long size) {
	return TRUE;
}

long MCM6000Stage::IsMCM6000Connected()
{
	return _device->IsConnected() ? TRUE : FALSE;
}

long MCM6000Stage::IsCondenserAvailable()
{
	return _device->_mcm6kParams->condenserConfigured;
}
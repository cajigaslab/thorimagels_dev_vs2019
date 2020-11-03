// ThorPowerSimulator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ThorPowerSimulator.h"


ThorPowerSimulator::ThorPowerSimulator()
{
	_rPos=0;
	_rPos_C=POWER_MIN-1;
	_rPos_B=FALSE;
	_zDevType = IDevice::POWER_REG;
	_rHome_B=TRUE;
	_zHome = POWER_HOME_MIN;
	_zVel = POWER_VELOCITY_MIN;
}

ThorPowerSimulator::~ThorPowerSimulator()
{
	_instanceFlag = false;
}

bool ThorPowerSimulator::_instanceFlag = false;

ThorPowerSimulator* ThorPowerSimulator::_single = NULL;

ThorPowerSimulator* ThorPowerSimulator::getInstance()
{
	if(!_instanceFlag)
	{
		_single = new ThorPowerSimulator();
		_instanceFlag = true;

		return _single;
	}
	else
	{
		return _single;
	}
}

void ThorPowerSimulator::SetHInstance(HINSTANCE hinst)
{
	_hinstance = hinst;
}


HINSTANCE ThorPowerSimulator::GetHInstance()
{
	return _hinstance;
}


long ThorPowerSimulator::FindDevices(long &deviceCount)
{
	long ret = FALSE;

	deviceCount = 0;


	deviceCount = 1;

	ret = TRUE;

	return ret;
}


long ThorPowerSimulator::SelectDevice(const long device)
{
	long ret = FALSE;

	switch(device)
	{
	case 0:
	{
		ret = TRUE;
	}
	break;
	default:
	{
	}
	}
	return ret;
}

long ThorPowerSimulator::TeardownDevice()
{

	return TRUE;
}


long ThorPowerSimulator::GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_CONNECTION_STATUS:
		{
			paramType = TYPE_LONG;
			paramAvailable = TRUE;
			paramReadOnly = TRUE;
			paramMin = CONNECTION_READY;
			paramMax = CONNECTION_READY;
			paramDefault = CONNECTION_READY;
		}
		break;
	case PARAM_POWER_POS:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = POWER_MIN;
		paramMax = POWER_MAX;
		paramDefault = R_DEFAULT;
	}
	break;

	case PARAM_POWER_HOME:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = POWER_HOME_MIN;
		paramMax = POWER_HOME_MAX;
		paramDefault = POWER_HOME_DEFAULT;
	}
	break;

	case PARAM_DEVICE_TYPE:
	{
		paramType = TYPE_LONG;
		paramAvailable = TRUE;
		paramReadOnly = TRUE;
		paramMin = paramMax = paramDefault = POWER_REG ;
	}
	break;

	case PARAM_POWER_VELOCITY:
	{
		paramType = TYPE_DOUBLE;
		paramAvailable = TRUE;
		paramReadOnly = FALSE;
		paramMin = POWER_VELOCITY_MIN;
		paramMax = POWER_VELOCITY_MAX;
		paramDefault = POWER_VELOCITY_DEFAULT;
	}
	break;

	default:
		paramAvailable = FALSE;
		ret = TRUE;
	}
	return ret;
}

long ThorPowerSimulator::SetParam(const long paramID, const double param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_DEVICE_TYPE:
		{
			DeviceType type = static_cast<DeviceType>(static_cast<int>(param));
			if((type >= IDevice::DEVICE_TYPE_FIRST) && (type <= IDevice::DEVICE_TYPE_LAST))
			{
				_zDevType = type;			
			}
			else
			{
				ret = FALSE;
			}
		}
		break;
	case PARAM_POWER_POS:
	{
		if((param >= POWER_MIN) && (param <= POWER_MAX))
		{
			_rPos = static_cast<double>(param);
		}
		else
		{
			ret = FALSE;
		}
		if(FALSE == ret)
		{
		}
	}
	break;

	case PARAM_POWER_HOME:
	{
		if((param >= POWER_HOME_MIN) && (param <= POWER_HOME_MAX))
		{
			_rHome_B = TRUE;
			_zHome = static_cast<int>(param);
		}
		else
		{
			ret = FALSE;
		}
		if(FALSE == ret)
		{
		}
	}
	break;

	case PARAM_POWER_VELOCITY:
	{
		if((param >= POWER_VELOCITY_MIN) && (param <= POWER_VELOCITY_MAX))
		{
			//SetControl(wndX.m_motor,R_CONTROL);

			//float minV,maxV,accel;
			////retrieve the current function values
			//if(wndX.m_motor.p->GetVelParams(CHAN1_ID,&minV,&accel,&maxV) == S_OK)
			//{
			//	if(wndX.m_motor.p->SetVelParams(CHAN1_ID,minV,accel,param) == S_OK)
			//	{
			//		wsprintf(message,L"ThorPowerSimulator Set R Velocity %d.%d",(int)param,(int)((param - static_cast<long>(param))*1000));
			//		logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			//	}
			//}

			_zVel = param;
		}
		else
		{
			ret = FALSE;
		}

		if(FALSE == ret)
		{
		}
	}
	break;

	default:
		ret = FALSE;
	}

	return ret;
}

long ThorPowerSimulator::GetParam(const long paramID, double &param)
{
	long ret = TRUE;

	switch(paramID)
	{
	case PARAM_POWER_POS:
	{
		param = static_cast<double>(_rPos);
	}
	break;
	case PARAM_POWER_HOME:
	{
		param = static_cast<int>(_zHome);
	}
	break;
	case PARAM_DEVICE_TYPE:
	{
		param = static_cast<double>(static_cast<int>(_zDevType));			
	}
	break;
	case PARAM_POWER_VELOCITY:
	{
		param = static_cast<double>(_zVel);				
	}
	break;
	case PARAM_CONNECTION_STATUS:
		{
			param  = static_cast<double>(CONNECTION_READY);
		}
		break;
	default:
		ret = FALSE;
	}

	return ret;
}

/// <summary>
/// Sets the parameter buffer.
/// </summary>
/// <param name="paramID">The parameter identifier.</param>
/// <param name="pBuffer">Pointer to the buffer.</param>
/// <param name="size">Buffer size.</param>
/// <returns>long.</returns>
long ThorPowerSimulator::SetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPowerSimulator::GetParamBuffer(const long paramID, char* pBuffer, long size)
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
long ThorPowerSimulator::SetParamString(const long paramID, wchar_t* str)
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
long ThorPowerSimulator::GetParamString(const long paramID, wchar_t* str, long size)
{
	long ret = FALSE;

	return ret;
}

long ThorPowerSimulator::PreflightPosition()
{
	return TRUE;
}

long ThorPowerSimulator::SetupPosition()
{
	if(_rPos != _rPos_C)
	{
		_rPos_B = TRUE;
	}

	return TRUE;
}


long ThorPowerSimulator::StartPosition()
{
	long ret = FALSE;

	if(_rHome_B)
	{
		/*SetControl(wndX.m_motor,R_CONTROL);

		if(wndX.m_motor.p->MoveHome(CHAN1_ID,1) == S_OK)
		{
			rPos_C = 0;
			ret = TRUE;
		}*/

		_rPos_C = 0;
		ret = TRUE;

		_rHome_B = FALSE;
	}

	long movedR = FALSE;

	if(_rPos_B)
	{
		/*SetControl(wndX.m_motor,R_CONTROL);

		if(wndX.m_motor.p->MoveAbsoluteRot(CHAN1_ID,static_cast<float>(rPos),0,ROT_MOVE_SHORT,1)== S_OK)
		{
			rPos_C = rPos;

			wsprintf(message,L"MoveAbsoluteR %d.%d",(int)rPos_C,(int)((rPos_C - static_cast<long>(rPos_C))*1000));
			logDll->TLTraceEvent(VERBOSE_EVENT,1,message);
			ret = TRUE;
			wndX.finishedMove = FALSE;
		}*/

		_rPos_C = _rPos;
		ret = TRUE;

		_rPos_B = FALSE;
		movedR = TRUE;
	}

	return ret;
}

long ThorPowerSimulator::StatusPosition(long &status)
{
	long ret = TRUE;

	status = IDevice::STATUS_READY;

	return ret;
}

long ThorPowerSimulator::ReadPosition(DeviceType deviceType,double &pos)
{
	long ret = FALSE;

	switch(deviceType)
	{
	case POWER_REG:

		/*SetControl(wndX.m_motor,R_CONTROL);
		if(wndX.m_motor.p->GetPosition(CHAN1_ID,&fPos) == S_OK)
		{
			pos = fPos;
		}*/

		pos = _rPos_C;
		break;
	}
	return ret;
}

long ThorPowerSimulator::PostflightPosition()
{
	return TRUE;
}

long ThorPowerSimulator::GetLastErrorMsg(wchar_t *msg, long size)
{
	return TRUE;
}

/*
LRESULT CALLBACK SWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		return 0;
	}
	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		SetWindowPos(hWnd,HWND_TOP,0,0,clientRect.right,clientRect.bottom,SWP_NOZORDER);

		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK XWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		return 0;
	}
	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		SetWindowPos(hWnd,HWND_TOP,0,0,clientRect.right,clientRect.bottom,SWP_NOZORDER);

		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);

	
}
*/
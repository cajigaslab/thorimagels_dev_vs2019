#include "StdAfx.h"
#include "ParamInfo.h"
#include <string>


ParamInfo::ParamInfo( long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,double ConvertFct,
					 double pMin,double pMax,double pDefault,string str)
{
	_paramID = pID;
	_paramVal = pVal;
	_param_C = p_C;
	
	_param_B = p_B;

	_type = typ;
	_available = avail;
	_readOnly = r;
	_needConvert = NdConvert;
	_convertFactor = ConvertFct;

	_paramMin = pMin;
	_paramMax = pMax;
	_paramDefault = pDefault;
	_strCommand = str;
	
}

ParamInfo::~ParamInfo(void)
{
}

double ParamInfo::GetParamVal()
{	
	return _paramVal;	
}

double ParamInfo::GetParamCurrent()
{
	return _param_C;
}

BOOL ParamInfo::GetParamBool()
{
	return _param_B;
}

long ParamInfo::GetParamType()
{
	return _type;
}
long ParamInfo::GetParamAvailable()
{
	return _available;
}

long ParamInfo::GetParamReadOnly()
{
	return _readOnly;
}

double ParamInfo::GetParamMin()
{
	return _paramMin;
}

double ParamInfo::GetParamMax()
{
	return _paramMax;
}

double ParamInfo::GetParamDefault()
{
	return _paramDefault;
}

string ParamInfo::GetCmdStr()
{
	return _strCommand;
}

long ParamInfo::GetParamID()
{
	return _paramID;
}

long ParamInfo::SetParamBool(BOOL b)
{
	_param_B = b;

	return TRUE;
}

long ParamInfo::UpdateParam_C()
{
	_param_C = _paramVal;

	return TRUE;
}

long ParamInfo::ResetVal()
{
	_paramVal = _paramMin;

	return TRUE;
}

long ParamInfo::ApplyConvertion()
{
	if (_needConvert==1)			//Position Send (before conversion unit: mm)
	{	
		_paramVal=_paramVal*_convertFactor/10;
		return TRUE;
	}
	else if (_needConvert==2)	//Velocity,Acceleration Send (before conversion length unit: steps)
	{	
		_paramVal=_paramVal*_convertFactor/10000000000;
		return TRUE;
	}		
	else 
	{	return FALSE;	}
}

///update the ParamVal using the specified val, change the 
///command string as well if bChangeStr is set
long ParamInfo::UpdateParam(double val)
{
	_paramVal = val;
	ApplyConvertion();
	string temp, strTemp2;

	if(0 < int(_strCommand.find('?')))	//no need to change command string
	{	return TRUE;	}
	//Prepare for dual commands if necessary:
	if(int(_strCommand.find("ML"))>0)
	{	
		int strStart = static_cast<int>(_strCommand.find_last_of("M")-2);
		int strEnd = static_cast<int>(_strCommand.find_last_of("\r")+1);
		strTemp2 = _strCommand.substr(strStart,strEnd);
	}
	//Process command string #1:
	if((2 < int(_strCommand.find("%d"))) || (2 < int(_strCommand.find_last_of("0123456789"))))
	{
		char buf[20];
		if(1 < int(_strCommand.find('M')))
		{	
			temp = _strCommand.substr(0, 4);	
		}
		else
		{	
			temp = _strCommand.substr(0, 3);	
		}

		if ((int(_strCommand.find('z'))>0) || (int(_strCommand.find('Z'))>0) || (int(_strCommand.find('T'))>0))	//no need to append value in command	
		{
			_strCommand = temp.append("R\r");
		}
		else
		{
			_strCommand = temp.append("%d");
			sprintf_s(buf, _strCommand.c_str(), static_cast<long>(_paramVal));
			_strCommand = string(buf);
			_strCommand = _strCommand.append("R\r");
		}			
	}
	//Append command string #2:
	if(!strTemp2.empty())
	{
		char buf[20];
		temp = strTemp2.substr(0, 3);		
		strTemp2 = temp.append("%d");
		double TempParamVal = (_paramVal == 0) ? 1 : 0;
		sprintf_s(buf, strTemp2.c_str(), static_cast<long>(TempParamVal));
		strTemp2 = string(buf);
		strTemp2 = strTemp2.append("R\r");
		_strCommand = _strCommand.append(strTemp2);
	}
	return TRUE;
}
#include "StdAfx.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>


ParamInfo::ParamInfo( long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,double ConvertFct,
					 double pMin,double pMax,double pDefault,std::vector<unsigned char> command)
{
	_paramID = pID;
	_paramVal = pVal;
	_param_Current = p_C;
	_param_set_enable = p_B;
	_type = typ;
	_available = avail;
	_readOnly = r;
	_needConvert = NdConvert;
	_convertFactor = ConvertFct;
	_paramMin = pMin;
	_paramMax = pMax;
	_paramDefault = pDefault;
	_commandBytes = command; 
}

ParamInfo::~ParamInfo(void)
{
}

double ParamInfo::GetConvertFactor()
{
	return _convertFactor;
}

double ParamInfo::GetParamVal()
{	
	return _paramVal;	
}

double ParamInfo::GetParamCurrent()
{
	return _param_Current;
}

BOOL ParamInfo::GetParamSetEnable()
{
	return _param_set_enable;
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

std::vector<unsigned char> ParamInfo::GetCmdBytes()
{
	return _commandBytes;
}

long ParamInfo::GetParamID()
{
	return _paramID;
}

long ParamInfo::UpdateParam_C(double val)
{
	_param_Current = val;
	if (TRUE == _needConvert)			//Position Send (before conversion unit: mm)
	{	
		_param_Current = _param_Current * 1000 * 1000 / _convertFactor;
	}

	return TRUE;
}

long ParamInfo::ResetVal()
{
	_paramVal = _paramMin;

	return TRUE;
}

long ParamInfo::SetParamSetEnable(BOOL b){
	_param_set_enable = b;
	return TRUE;
}

long ParamInfo::ApplyConvertion()
{
	if (TRUE == _needConvert)			//Position Send (before conversion unit: mm)
	{	
		_paramVal = _paramVal * 1000 * 1000 / _convertFactor;
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
	if (TRUE == _needConvert)			//Position Send (before conversion unit: mm)
	{	
		_paramVal = _paramVal * 1000 * 1000 / _convertFactor;
	}
	return TRUE;
}


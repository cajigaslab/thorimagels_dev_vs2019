#include "stdafx.h"
#include <Windows.h>
#include "ParamInfo.h"


ParamInfo::ParamInfo( long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,void * convertFunction,
					 double pMin,double pMax,double pDefault,long motorId,std::vector<unsigned char> command)
{
	_paramID = pID;
	_paramVal = pVal;
	_param_C = p_C;
	
	_param_B = p_B;

	_type = typ;
	_available = avail;
	_readOnly = r;
	_needConvert = NdConvert;
	_convertFunc = convertFunction;

	_paramMin = pMin;
	_paramMax = pMax;
	_paramDefault = pDefault;

	_paramZero = 0;

	_motorID = motorId;

	_commandBytes = command; 
}

ParamInfo::~ParamInfo(void)
{
}

long ParamInfo::GetNeedConvert()
{
	return _needConvert;
}

void * ParamInfo::GetConvertFunc()
{
	return _convertFunc;
}

long ParamInfo::GetMotorID()
{
	return _motorID;
}

double ParamInfo::GetParamVal()
{	
	return _paramVal;	
}

double ParamInfo::GetParamZero()
{	
	return _paramZero;	
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

std::vector<unsigned char> ParamInfo::GetCmdBytes()
{
	return _commandBytes;
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

long ParamInfo::UpdateParamZero(double val)
{
	_paramZero = val;

	return TRUE;
}

long ParamInfo::UpdateParam_C()
{
	_param_C = _paramVal;

	return TRUE;
}

///update the ParamVal using the specified val, change the 
///command string as well if bChangeStr is set
long ParamInfo::UpdateParam(double val)
{
	_paramVal = val;
	return TRUE;
}


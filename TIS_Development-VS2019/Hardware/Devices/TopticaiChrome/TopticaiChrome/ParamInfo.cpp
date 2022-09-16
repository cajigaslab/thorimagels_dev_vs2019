#include "stdafx.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>


ParamInfo::ParamInfo(long pID, wstring parameterString, double pVal, double p_C, 
	                 BOOL p_B, BOOL p_F, long typ, long avail, long r,double pMin, double pMax, 
					 double pDefault, wstring commandSet, wstring commandGet)
{
	_paramID = pID;
	_parameterString = parameterString;
	_paramVal = pVal;
	_param_C = p_C;
	_param_B = p_B;
	_param_First = p_F;
	_type = typ;
	_available = avail;
	_readOnly = r;
	_paramMin = pMin;
	_paramMax = pMax;
	_paramDefault = pDefault;
	_commandSet = commandSet;
	_commandGet = commandGet;
}

ParamInfo::~ParamInfo(void)
{
}

/// <summary>
/// Gets the parameter value.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetParamVal()
{
	return _paramVal;
}

/// <summary>
/// Gets the parameter current.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetParamCurrent()
{
	//if (_paramID==751||_paramID==755||_paramID==759||_paramID==763)
	//{	
	//	if (_param_C>0)
	//		return floor(_param_C*1000)/1000;
	//	else
	//		return ceil(_param_C*1000)/1000;
	//}
	//else
	//{
	return _param_C;
	//}
}

/// <summary>
/// Gets the parameter bool.
/// </summary>
/// <returns>BOOL.</returns>
BOOL ParamInfo::GetParamBool()
{
	return _param_B;
}

/// <summary>
///Gets the parameter first.
/// </summary>
/// <returns>BOOL.</returns>
BOOL ParamInfo::GetParamFirst() 
{
	return _param_First;
}

/// <summary>
/// Gets the type of the parameter.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::GetParamType()
{
	return _type;
}

/// <summary>
/// Gets the parameter available.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::GetParamAvailable()
{
	return _available;
}

/// <summary>
/// Gets the parameter read only.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::GetParamReadOnly()
{
	return _readOnly;
}

/// <summary>
/// Gets the parameter minimum.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetParamMin()
{
	return _paramMin;
}

/// <summary>
/// Gets the parameter maximum.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetParamMax()
{
	return _paramMax;
}

/// <summary>
/// Gets the parameter default.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetParamDefault()
{
	return _paramDefault;
}

/// <summary>
/// Gets the commandSet command.
/// </summary>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
wstring ParamInfo::GetCmdSet()
{
	return _commandSet;
}

/// <summary>
/// Gets the commandGet command.
/// </summary>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
wstring ParamInfo::GetCmdGet()
{
	return _commandGet;
}

/// <summary>
/// Gets the parameter identifier.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::GetParamID()
{
	return _paramID;
}

/// <summary>
/// Gets the string describing the parameter.
/// </summary>
/// <returns>long.</returns>
wstring ParamInfo::GetParameterString()
{
	return _parameterString;
}

/// <summary>
/// Updates the param_ c.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::SetParamBool(BOOL b)
{
	_param_B = b;

	return TRUE;
}

long ParamInfo::SetParamFirst() 
{
	_param_First = FALSE;

	return TRUE;
}

/// <summary>
/// Resets the value.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::UpdateParam_C()
{
	_param_C = _paramVal;

	return TRUE;
}

/// <summary>
/// Updates the parameter.
/// </summary>
/// <param name="val">The value.</param>
/// <returns>long.</returns>
/// update the ParamVal using the specified val.
long ParamInfo::UpdateParam(double val)
{
	_paramVal = val;
	return TRUE;
}
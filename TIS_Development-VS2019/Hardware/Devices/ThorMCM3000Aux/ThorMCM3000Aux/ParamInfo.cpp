#include "StdAfx.h"
#include "ParamInfo.h"
#include <string>
#include <math.h>


/// <summary>
/// Initializes a new instance of the <see cref="ParamInfo"/> class.
/// </summary>
/// <param name="pID">The p identifier.</param>
/// <param name="pVal">The p value.</param>
/// <param name="p_C">The p_ c.</param>
/// <param name="p_B">The p_ b.</param>
/// <param name="typ">The typ.</param>
/// <param name="avail">The avail.</param>
/// <param name="r">The r.</param>
/// <param name="NdConvert">The nd convert.</param>
/// <param name="ConvertFct">The convert FCT.</param>
/// <param name="pMin">The p minimum.</param>
/// <param name="pMax">The p maximum.</param>
/// <param name="pDefault">The p default.</param>
/// <param name="motorId">The motor identifier.</param>
/// <param name="command">The command.</param>
ParamInfo::ParamInfo( long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,double ConvertFct,
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
	_convertFactor = ConvertFct;

	_paramMin = pMin;
	_paramMax = pMax;
	_paramDefault = pDefault;

	_motorID = motorId;

	_commandBytes = command; 
}

/// <summary>
/// Finalizes an instance of the <see cref="ParamInfo"/> class.
/// </summary>
ParamInfo::~ParamInfo(void)
{
}

/// <summary>
/// Gets the convert factor.
/// </summary>
/// <returns>double.</returns>
double ParamInfo::GetConvertFactor()
{
	return _convertFactor;
}

/// <summary>
/// Gets the motor identifier.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::GetMotorID()
{
	return _motorID;
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
	return _param_C;
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
/// Gets the command bytes.
/// </summary>
/// <returns>std.vector&lt;unsigned char&gt;.</returns>
std::vector<unsigned char> ParamInfo::GetCmdBytes()
{
	return _commandBytes;
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
/// Sets the parameter bool.
/// </summary>
/// <param name="b">The b.</param>
/// <returns>long.</returns>
long ParamInfo::SetParamBool(BOOL b)
{
	_param_B = b;

	return TRUE;
}

/// <summary>
/// Updates the param_ c.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::UpdateParam_C()
{
	_param_C = _paramVal;

	return TRUE;
}

/// <summary>
/// Resets the value.
/// </summary>
/// <returns>long.</returns>
long ParamInfo::ResetVal()
{
	_paramVal = _paramMin;

	return TRUE;
}

/// <summary>
/// Updates the parameter.
/// </summary>
/// <param name="val">The value.</param>
/// <returns>long.</returns>
/// update the ParamVal using the specified val, change the
/// command string as well if bChangeStr is set
long ParamInfo::UpdateParam(double val)
{
	_paramVal = val;
	if (TRUE == _needConvert)			//Position Send (before conversion unit: mm)
	{	
		_paramVal = _paramVal / _convertFactor;
	}
	return TRUE;
}


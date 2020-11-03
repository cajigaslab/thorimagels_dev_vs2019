#pragma once
#include "stdafx.h"
class ParamInfo
{

private:
	long _paramID;
	double _paramVal;
	std::vector<unsigned char> _commandBytes;
	long _type;
	long _available;
	long _readOnly;
	long _needConvert;
	double _convertFactor;
	double _paramMin;
	double _paramMax;
	double _paramDefault;
	double _param_Current;
	BOOL _param_set_enable;

public:
	ParamInfo(long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,double ConvertFct,
					 double pMin,double pMax,double pDefault,std::vector<unsigned char> command);
	~ParamInfo(void);
	double GetConvertFactor();
	long GetMotorID();
	long GetParamID();
	double GetParamVal();
	double GetParamCurrent();
	BOOL GetParamSetEnable();
	long GetParamType();
	long GetParamAvailable();
	long GetParamReadOnly();	
	double GetParamMin();
	double GetParamMax();
	double GetParamDefault();
	std::vector<unsigned char> GetCmdBytes();

	long UpdateParam(double val);
	long SetParamSetEnable(BOOL b);
	long UpdateParam_C(double val);
	long ResetVal();
	long ApplyConvertion();
};

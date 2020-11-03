#pragma once
#include "stdafx.h"
class ParamInfo
{

private:
	long _paramID;
	long _motorID;
	double _paramVal;
	double _param_C;
	BOOL _param_B;
	std::vector<unsigned char> _commandBytes;
	long _type;
	long _available;
	long _readOnly;
	long _needConvert;
	double _convertFactor;
	double _paramMin;
	double _paramMax;
	double _paramDefault;


public:
	ParamInfo(long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long NdConvert,double ConvertFct,
					 double pMin,double pMax,double pDefault,long motorId,std::vector<unsigned char> command);
	~ParamInfo(void);
	double GetConvertFactor();
	long GetMotorID();
	long GetParamID();
	double GetParamVal();
	double GetParamCurrent();
	BOOL GetParamBool();
	long GetParamType();
	long GetParamAvailable();
	long GetParamReadOnly();	
	double GetParamMin();
	double GetParamMax();
	double GetParamDefault();
	std::vector<unsigned char> GetCmdBytes();
	long UpdateParam(double val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
	long ResetVal();
};

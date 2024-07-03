#pragma once
#include "stdafx.h"

class ParamInfo
{

private:
	long _paramID;
	double _paramVal;
	double _param_C;
	BOOL _param_B;
	std::vector<unsigned char> _commandBytes;
	long _type;
	long _available;
	long _readOnly;
	double _paramMin;
	double _paramMax;
	double _paramDefault;
	long _deviceIndex;
	wstring _parameterString;
	long _relatedID;
	//void* commandFunction;

public:
	ParamInfo(long pID, wstring parameterString, double pVal, double p_C, BOOL p_B,
		long typ, long avail, long readOnly,
		double pMin, double pMax, double pDefault, std::vector<unsigned char> command, long deviceIndex, long relatedID);
	~ParamInfo(void);

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
	long GetDeviceIndex();
	wstring GetParameterString();
	long GetRelatedID();
	long UpdateParam(double val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
	long ResetVal();
};

#pragma once

class ParamInfo
{

private:
	long _paramID;
	double _paramVal;
	double _param_C;
	BOOL _param_B;
	wstring _strCommand;
	long _type;
	long _available;
	long _readOnly;
	double _paramMin;
	double _paramMax;
	double _paramDefault;
	long _deviceIndex;
	//void* commandFunction;

public:
	ParamInfo(long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long readOnly,
					 double pMin,double pMax,double pDefault,wstring cmd, long deviceIndex);
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
	wstring GetCmdStr();
	long GetDeviceIndex();
	long UpdateParam(double val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
};

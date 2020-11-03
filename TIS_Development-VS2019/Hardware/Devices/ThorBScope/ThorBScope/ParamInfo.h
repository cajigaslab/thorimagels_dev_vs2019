#pragma once

class ParamInfo
{

private:
	long _paramID;
	double _paramVal;
	double _param_C;
	BOOL _param_B;
	string _strCommand;
	long _type;
	long _available;
	long _readOnly;
	long _needConvert;
	double _convertFactor;
	double _paramMin;
	double _paramMax;
	double _paramDefault;

	long ApplyConvertion();

public:
	ParamInfo(long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,long ndConvert,double convertFct,
					 double pMin,double pMax,double pDefault,string str);
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
	string GetCmdStr();
	long UpdateParam(double val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
	long ResetVal();	
};

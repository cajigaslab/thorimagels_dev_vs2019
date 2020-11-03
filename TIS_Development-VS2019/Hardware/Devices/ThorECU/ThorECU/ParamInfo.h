#pragma once

class ParamInfo
{

private:
	long paramID;

	double paramVal;
	double param_C;
	BOOL param_B;
	wstring strCommand;
	long type;
	long available;
	long readOnly;
	double paramMin;
	double paramMax;
	double paramDefault;


public:
	ParamInfo(long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long readOnly,
					 double pMin,double pMax,double pDefault,wstring str);
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


	long SetParamVal(double val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
};

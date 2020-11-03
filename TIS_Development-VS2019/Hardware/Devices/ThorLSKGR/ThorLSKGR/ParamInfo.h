#pragma once
class ParamInfo
{

private:
	long _paramID;
	long _paramVal;
	long _param_C;
	BOOL _param_B;	
	long _type;
	long _available;
	long _readOnly;
	long _paramMin;
	long _paramMax;
	long _paramDefault;
	std::vector<unsigned char> _commandBytes;

public:
	ParamInfo(long pID, long pVal,long p_C,BOOL p_B, long typ,long avail, long readOnly,
					 long pMin,long pMax,long pDefault ,std::vector<unsigned char> command);
	~ParamInfo(void);
	long GetParamID();
	long GetParamVal();
	long GetParamCurrent();
	BOOL GetParamBool();
	long GetParamType();
	long GetParamAvailable();
	long GetParamReadOnly();	
	double GetParamMin();
	double GetParamMax();
	double GetParamDefault();
	std::vector<unsigned char> GetCmdBytes();
	long SetParamVal(long val);
	long SetParamBool(BOOL b);
	long UpdateParam_C();
	long ResetVal();
};

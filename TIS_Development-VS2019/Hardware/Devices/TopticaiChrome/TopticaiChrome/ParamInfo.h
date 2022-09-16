class ParamInfo
{

private:
	long _paramID;
	double _paramVal;
	double _param_C;
	BOOL _param_B;
	BOOL _param_First;
	wstring _commandSet;
	wstring _commandGet;
	long _type;
	long _available;
	long _readOnly;
	double _paramMin;
	double _paramMax;
	double _paramDefault;
	wstring _parameterString;

public:
	ParamInfo(long pID, wstring parameterString, double pVal, double p_C, BOOL p_B, BOOL p_F,
		long typ, long avail, long readOnly,
		double pMin, double pMax, double pDefault, wstring _commandSet, wstring _commandGet);
	~ParamInfo(void);

	long GetParamID();

	double GetParamVal();
	double GetParamCurrent();
	BOOL GetParamBool();
	BOOL GetParamFirst();
	long GetParamType();
	long GetParamAvailable();
	long GetParamReadOnly();
	double GetParamMin();
	double GetParamMax();
	double GetParamDefault();
	wstring GetCmdGet();
	wstring GetCmdSet();
	wstring GetParameterString();
	long UpdateParam(double val);
	long SetParamBool(BOOL b);
	long SetParamFirst();
	long UpdateParam_C();
};

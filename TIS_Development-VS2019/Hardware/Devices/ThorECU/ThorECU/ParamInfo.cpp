#include "StdAfx.h"
#include "ParamInfo.h"
#include <string>


ParamInfo::ParamInfo( long pID, double pVal,double p_C,BOOL p_B,
					 long typ,long avail,long r,
					 double pMin,double pMax,double pDefault,wstring str)
{
	paramID = pID;
	paramVal = pVal;
	param_C = p_C;
	param_B = p_B;

	type = typ;
	available = avail;
	readOnly = r;
	paramMin = pMin;
	paramMax = pMax;
	paramDefault = pDefault;
	strCommand = str;
}

ParamInfo::~ParamInfo(void)
{
}

double ParamInfo::GetParamVal()
{
	return paramVal;
}
double ParamInfo::GetParamCurrent()
{
	return param_C;
}
BOOL ParamInfo::GetParamBool()
{
	return param_B;
}
long ParamInfo::GetParamType()
{
	return type;
}
long ParamInfo::GetParamAvailable()
{
	return available;
}
long ParamInfo::GetParamReadOnly()
{
	return readOnly;
}
double ParamInfo::GetParamMin()
{
	return paramMin;
}
double ParamInfo::GetParamMax()
{
	return paramMax;
}
double ParamInfo::GetParamDefault()
{
	return paramDefault;
}
wstring ParamInfo::GetCmdStr()
{
	return strCommand;
}
long ParamInfo::GetParamID()
{
	return paramID;
}

long ParamInfo::SetParamBool(BOOL b)
{
	param_B = b;

	return TRUE;
}
long ParamInfo::UpdateParam_C()
{
	param_C = paramVal;

	return TRUE;
}

///update the ParamVal using the specified val, change the 
///command string as well if bChangeStr is set
long ParamInfo::SetParamVal(double val)
{
	paramVal = val;

	if((GetParamMin() > paramVal) && (GetParamMax() < paramVal))
	{
		return FALSE;
	}

	if(param_C != paramVal)
	{
		SetParamBool(true);
	}
	
	TCHAR wsVal[20];
	swprintf(wsVal, 20, L"%d", static_cast<long>(val));
	wstring wsTemp = wstring(wsVal);

	int found = static_cast<int>(strCommand.find_last_of(L"="));

	if(strCommand.find_last_of(L"=") > 0)
	{
		strCommand.erase(found+1, strCommand.length());
		strCommand.append(wsTemp);
		//strCommand.replace(found+1, wsTemp.length(), wsTemp);
	}

	return TRUE;
}
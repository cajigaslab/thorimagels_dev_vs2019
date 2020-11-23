// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ExperimentXML.h"
#include "ExperimentManager.h"

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//The following are C interface wrappers for C# to use. 
//If you are using C++ the .lib class interface should be used for convenience.

DllExportExpManger GetActiveExperimentPathAndName(wchar_t * path, long size)
{
	wstring ws = ExperimentManager::getInstance()->GetActiveExperimentPathAndName();

	wcscpy_s(path,size,ws.c_str());

	return TRUE;
}

DllExportExpManger GetActiveExperimentPath(wchar_t * path, long size)
{
	wstring ws = ExperimentManager::getInstance()->GetActiveExperimentPath();

	wcscpy_s(path,size,ws.c_str());

	return TRUE;
}

DllExportExpManger SetActiveExperiment(wchar_t * path)
{
	return ExperimentManager::getInstance()->SetActiveExperiment(path);
}


DllExportExpManger GetName(wchar_t * name, long size)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);

	wcscpy_s(name,size,ws.c_str());

	return ret;
}

DllExportExpManger SetName(wchar_t * name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	wstring ws(name);

	string str = WStringToString(ws);

	ret = exp->SetName(str);

	return ret;
}

DllExportExpManger GetDate(wchar_t * date,long size)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetDate(str);

	wstring ws = StringToWString(str);

	wcscpy_s(date,size,ws.c_str());

	return ret;
}

DllExportExpManger SetDate(wchar_t * date)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	wstring ws(date);

	string str = WStringToString(ws);

	ret = exp->SetName(str);

	return ret;
}



/**TODO**
DllExportExpManger GetUser(string &name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetUser(string name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}
 
	
DllExportExpManger GetComputer(string &name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetComputer(string name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}


DllExportExpManger GetSoftware(double &version)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetSoftware(double version)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}


DllExportExpManger GetCamera(string &name,long &width,long &height,double &pixelSizeUM)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetCamera(string name, long width, long height, double pixelSizeUM)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}


DllExportExpManger GetMagnification(double &mag)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetMagnification(double mag)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}


DllExportExpManger AddWavelength(string name,double exposureTimeMS)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger RemoveWavelength(string name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

*/

DllExportExpManger GetNumberOfWavelengths()
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetNumberOfWavelengths");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}	
	ret = exp->GetNumberOfWavelengths();

	return ret;
}

DllExportExpManger GetWavelength(long index, wchar_t * name, long size, double &exposureTimeMS)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetWavelength");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetWavelength(index, str, exposureTimeMS);

	wstring ws = StringToWString(str);	
	wcscpy_s(name,size,ws.c_str());

	return ret;
}

DllExportExpManger SetWavelength(long index, wchar_t * name, double exposureTimeMS)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetWavelength");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	wstring ws(name);	
	string str = WStringToString(ws);

	ret = exp->SetWavelength(index, str, exposureTimeMS);	

	return ret;
}

DllExportExpManger GetZStage(wchar_t * name, long size, long &steps, double &stepSize, double &startPos,long &zStreamFrames,long &zStreamMode,  long &enable)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetZStage");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}
	string str;
	ret = exp->GetZStage(str, enable, steps, stepSize,startPos,zStreamFrames,zStreamMode);

	wstring ws = StringToWString(str);	
	wcscpy_s(name,size,ws.c_str());

	return ret;
}

DllExportExpManger SetZStage(wchar_t * name, long steps, double stepSize, double startPos,long zStreamFrames,long zStreamMode,long enable)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetZStage");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	wstring ws(name);	
	string str = WStringToString(ws);

	ret = exp->SetZStage(str, enable, steps, stepSize,startPos,zStreamFrames,zStreamMode);

	return ret;
}

DllExportExpManger GetTimelapse(long &timepoints,double &intervalSec, long &triggerMode)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetTimelapse");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->GetTimelapse(timepoints, intervalSec,triggerMode);

	return ret;
}

DllExportExpManger SetTimelapse(long timepoints, double intervalSec, long triggerMode)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetTimelapse");
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->SetTimelapse(timepoints, intervalSec, triggerMode);

	return ret;
}

DllExportExpManger GetSample(double &offsetXMM, double &offsetYMM, double &offsetZMM)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetSample");	

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}
	long tiltAdjustment;
	double fPt1X,fPt1Y,fPt1Z,fPt2X,fPt2Y,fPt2Z,fPt3X,fPt3Y,fPt3Z;
	ret = exp->GetSample(offsetXMM,offsetYMM,offsetZMM,tiltAdjustment,fPt1X,fPt1Y,fPt1Z,fPt2X,fPt2Y,fPt2Z,fPt3X,fPt3Y,fPt3Z);

	return ret;
}

DllExportExpManger SetSample(long type, double offsetXMM, double offsetYMM)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetSample");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	IExperiment::SampleType s = static_cast<IExperiment::SampleType>(type);

	ret = exp->SetSample(s,offsetXMM, offsetYMM, 0, 0, 0 ,0 ,0, 0 ,0, 0 ,0, 0);

	return ret;
}



DllExportExpManger GetWells(long &startRow, long &startColumn, long &rows, long &columns, double &wellOffsetXMM, double &wellOffsetYMM)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetWells");	

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->GetWells(startRow, startColumn, rows,columns,wellOffsetXMM,wellOffsetYMM);

	return ret;
}

DllExportExpManger SetWells(long startRow, long startColumn, long rows, long columns, double wellOffsetXMM, double wellOffsetYMM)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetWells");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->SetWells(startRow, startColumn,rows,columns,wellOffsetXMM,wellOffsetYMM);

	return ret;
}


DllExportExpManger GetSubImages(vector<IExperiment::SubImage>& subImages, long cameraType, long lsmType)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetSubImages");	

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->GetSubImages(subImages, cameraType, lsmType);

	return ret;
}

DllExportExpManger SetSubImages(long subRows,long subColumns, double subOffsetXMM, double subOffsetYMM, double transOffsetXMM, double transOffsetYMM)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  SetSubImages");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = exp->SetSubImages(subRows,subColumns,subOffsetXMM, subOffsetYMM,transOffsetXMM,transOffsetYMM);

	return ret;
}

DllExportExpManger 	GetLSM(long &areaMode, double &areaAngle, long &scanMode,long &interleave,long &pixelX,long &pixelY,long &channel, long &fieldSize, long &offsetX, long &offsetY,
						   long &averageMode, long &averageNum, long &clockSource, long &inputRange1, long &inputRange2, long &twoWayAlignment, long &extClockRate, double &dwellTime, 
						   long &flybackCycles, long &inputRange3, long &inputRange4, long &minimizeFlybackCycles, long &polarity1, long &polarity2, long &polarity3, long &polarity4,
						   long &verticalFlip, long &horizontalFlip, double &crsFrequencyHz, long& timeBasedLineScan, long& timeBasedLSTimeMS, long& threePhotonEnable, long& numberOfPlanes)
{
	logDll->TLTraceEvent(VERBOSE_EVENT,1,L"ExperimentManger  GetLSM");

	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	ret = 	exp->GetLSM(areaMode,areaAngle,scanMode,interleave,pixelX,pixelY,channel, fieldSize, offsetX, offsetY,averageMode, averageNum, clockSource, inputRange1, inputRange2, twoWayAlignment, extClockRate, dwellTime, flybackCycles, inputRange3, inputRange4, minimizeFlybackCycles,polarity1, polarity2, polarity3,polarity4, verticalFlip, horizontalFlip, crsFrequencyHz, timeBasedLineScan, timeBasedLSTimeMS, threePhotonEnable, numberOfPlanes);

	return ret;
}

/**TODO**

DllExportExpManger GetComments(string &name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}

DllExportExpManger SetComments(string name)
{
	IExperiment * exp = ExperimentManager::getInstance()->GetActiveExperiment();

	long ret;

	if(exp == NULL) 
	{
		return FALSE;
	}

	string str;
	ret = exp->GetName(str);

	wstring ws = StringToWString(str);
	
	wcscpy_s(name,length,ws.c_str())

	return ret
}
*/
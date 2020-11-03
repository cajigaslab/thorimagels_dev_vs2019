#pragma once

#include "..\..\thread.h"

class ExperimentXML;

#if defined(Experiment_MANAGER)
#define DllExportExperimentManger __declspec(dllexport)
#else
//    definitions used when using DLL
#define DllExportExperimentManger __declspec(dllimport)
#endif

class DllExportExperimentManger ExperimentManager
{
public:
	~ExperimentManager();
    static ExperimentManager* getInstance();
	IExperiment * CreateExperiment(wstring path);
	long DeleteExperiment(string path);
	IExperiment * GetExperiment(wstring path);
	long SetActiveExperiment(wstring path);
	IExperiment * GetActiveExperiment();
	wstring GetActiveExperimentPath();
	wstring GetActiveExperimentPathAndName();
	long  GetTotalSampleRowsAndColumns(IExperiment::SampleType type, long &totalRows,long &totalCols);


private:
    ExperimentManager();

#pragma warning(push)
#pragma warning(disable:4251)

	static bool instanceFlag;
    static auto_ptr<ExperimentManager> _single;
	static CritSect critSect;
	static void cleanup(void);
	static auto_ptr<ExperimentXML> activeExperiment;

#pragma warning(pop)
};

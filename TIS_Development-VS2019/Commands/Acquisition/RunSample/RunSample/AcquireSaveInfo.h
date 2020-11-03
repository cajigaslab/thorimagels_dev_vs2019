#pragma once

class AcquireSaveInfo
{
private:
	static double _expStartCount;
	list<double> listDeltaT_h;
	list<string> timestamps;

	AcquireSaveInfo();
	double GetExperimentStartCount();

public:
	static AcquireSaveInfo* getInstance();
	~AcquireSaveInfo();

	static bool _instanceFlag;
	static auto_ptr<AcquireSaveInfo> _single;

	void SetExperimentStartCount();

	long SaveTimingToFile(string filename);
	double AddTimingInfo();
	double RemoveTimingInfo();
	bool ClearTimingInfo();

	string AddTimestamp();
	string RemoveTimestamp();
	bool ClearTimestamps();

	void RemoveTimestampAt(long i);
	void RemoveTimingInfoAt(long i);
};

struct PhysicalSize
{
	double x,y,z;	// phiscal size, usually in micron, of a single voxle in three dimension
};
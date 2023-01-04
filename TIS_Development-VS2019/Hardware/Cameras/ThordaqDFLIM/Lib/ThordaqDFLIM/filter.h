#ifndef ___FILTER_H___
#define ___FILTER_H___
#include "thordaqcmd.h"

struct FilterSettings
{
	int filter1Settings;
	int filter2Settings;
};

class Filter
{
public:
	//Filter(void);
	//~Filter(void);
	virtual void initializeFilterTable() = 0;
	virtual FilterSettings findFilterSettings(double frequency, double adcSampleRate) = 0;
};

//class FIRFilter : public Filter
//{
//private:
//	double	gFirFilterTable[9][FITLER_MAX_TAP];
//public:
//	//FIRFilter(void);
//	//~FIRFilter(void);
//	void initializeFilterTable(); 
//	FilterSettings findFilterSettings(double frequency, double adcSampleRate);
//};

enum FilterType
{
	FIR,
};


class FilterFactory
{
public:
	FilterFactory(FilterType filterType);
	~FilterFactory();
	Filter* getFilterFactory();
private:
	Filter *pFilterFactory;
};


#endif // ___FILTER_H___
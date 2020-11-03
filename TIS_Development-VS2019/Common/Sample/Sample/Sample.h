#pragma once

#include "SampleSite.h"
#include "Acquire.h"

typedef void (_cdecl *sampleFPPrototype)(double &x, double &y);


class Sample
{
public:
	Sample();

	friend class SampleBuilder;

	void GoToOffset(double offsetX, double offsetY);
	void GoToSiteAndIndex(long site, long index);
	void GoToSiteAndOffset(long site, double offsetX, double offsetY);
	void GoToWellSite(long row, long col, long subRow, long subCol);
	void GoToWellSiteAndIndex(long row, long col, long index);
	void GoToWellSiteAndOffset(long row, long col, long subRow, long subColumn, double sampleOffsetX, double sampleOffsetY, double wellOffsetX, double wellOffsetY, double transOffsetX, double transOffsetY, double subOffsetX, double subOffsetY, sampleFPPrototype dm);
	void GoToAllWellSites(IDevice* , IAcquire*, IExperiment* pExp);

private:
	void Add(SampleSite site);
	void SetDimensions(long rows,long cols);
	void SetDimensionsMosaic(long startRow, long startCol, long totalRows, long totalCols, long rows,long cols,long subrows,long subcols);

	SampleSitesIterator _it;
	SampleSites _sampleSites;
	double _sampleOffsetX;
	double _sampleOffsetY;
	long _startRow;
	long _startCol;
	long _totalRows;
	long _totalCols;
	long _rows;
	long _cols;
	long _subrows;
	long _subcols;
	long _currentRow;
	long _currentCol;
};
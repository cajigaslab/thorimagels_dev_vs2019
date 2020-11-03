#pragma once

#include "Position.h"

class SampleSite;

typedef vector<SampleSite>::iterator SampleSitesIterator;
typedef vector<SampleSite> SampleSites;


class SampleSite
{
public:
	SampleSite();

	void Add(Position pos);
	Position Get(long index);
	int GetIndex();
	void SetIndex(int imageIndex);

private:
	long _rows;
	long _cols;
	PositionsIterator _it;
	Positions _positions;
	int index;
};
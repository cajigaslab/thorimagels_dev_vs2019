#include "stdafx.h"
#include "SampleSite.h"

SampleSite::SampleSite()
{
}

Position SampleSite::Get(long index)
{
	_it = _positions.begin();
	return _it[index];
}

void SampleSite::Add(Position pos)
{
	_positions.push_back(pos);
}

int SampleSite::GetIndex()
{
	return index;
}

void SampleSite::SetIndex(int imageIndex)
{
	index = imageIndex;
}
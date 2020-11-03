#pragma once

class Position;

typedef vector<Position>::iterator PositionsIterator;
typedef vector<Position> Positions;

class Position
{
public:
	Position();

	void Set(double x,double y);
	void Get(double &x,double &y);

private:
	double _x;
	double _y;
};
#include "stdafx.h"
#include "Position.h"


Position::Position()
{
}

void Position::Set(double x,double y)
{
	_x = x;
	_y = y;
}

void Position::Get(double &x, double &y)
{
	x = _x;
	y = _y;
}
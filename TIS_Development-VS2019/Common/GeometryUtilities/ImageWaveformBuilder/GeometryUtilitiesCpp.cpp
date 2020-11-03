#include "stdafx.h"
#include "GeometryUtilitiesCpp.h"

///Line Vector: Start->End
LineSegment::LineSegment(Cartesian2D start, Cartesian2D end)
{
	_startPoint = start;
	_endPoint = end;
	_length =  sqrt(pow((_endPoint.First() - _startPoint.First()), 2) + pow(((_endPoint.Second() - _startPoint.Second())), 2));
	_vector = _endPoint-_startPoint;
	_unitVector = _vector/_length;
}

double LineSegment::GetLineLength()
{
	return _length;
}

double LineSegment::GetMaxXYLength()
{
	return (_endPoint.First() - _startPoint.First()) > (_endPoint.Second() - _startPoint.Second()) ? (_endPoint.First() - _startPoint.First()) : (_endPoint.Second() - _startPoint.Second());
}

Cartesian2D LineSegment::GetUnitVector(double direction)
{
	return _unitVector*direction;
}

Cartesian2D LineSegment::GetEndPoint()
{
	return _endPoint;
}

Cartesian2D LineSegment::GetStartPoint()
{
	return _startPoint;
}


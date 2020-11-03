#pragma once
#include "Strsafe.h"
#include "..\..\PDLL\pdll.h"

#ifdef LOGGING_ENABLED
extern std::auto_ptr<LogDll> logDll;
#endif

extern wchar_t message[_MAX_PATH];
extern DWORD startTime;

static void LogMessage(wchar_t *logMsg,long eventLevel)
{
#ifdef LOGGING_ENABLED
	logDll->TLTraceEvent(eventLevel, 1, logMsg);
#endif
}

static void LogPerformance(const wchar_t* wstr)
{
#ifdef LOGGING_ENABLED
	long elapsedTime = static_cast<long>((GetTickCount() - startTime));
	std::wstring strLog = std::wstring(wstr) + L" %d";
	StringCbPrintfW(message,_MAX_PATH, strLog.c_str(), static_cast<long>(elapsedTime));
	LogMessage(message,VERBOSE_EVENT);								
	startTime = GetTickCount();
#endif
}

static long GetMutex(HANDLE mutex)
{
	long ret = TRUE;
	//wait for mutex if exist
	if(NULL != mutex)
	{
		ret = (WAIT_OBJECT_0 == WaitForSingleObject(mutex, Constants::EVENT_WAIT_TIME)) ? TRUE : FALSE;
	}
	return ret;
}

class Cartesian2D
{
private:
	double _val1;
	double _val2;
public:
	Cartesian2D(void){ _val1 = 0; _val2 = 0; };
	Cartesian2D(double val1,double val2){_val1 = val1; _val2 = val2; }
	double First(){return _val1;}
	double Second(){return _val2;}
	Cartesian2D operator+(const Cartesian2D& b)
	{		
		Cartesian2D a;
		a._val1 = this->_val1 + b._val1;
		a._val2 = this->_val2 + b._val2;
		return a;
	}
	Cartesian2D operator-(const Cartesian2D& b)
	{
		Cartesian2D a;
		a._val1 = this->_val1 - b._val1;
		a._val2 = this->_val2 - b._val2;
		return a;
	}
	Cartesian2D operator=(const Cartesian2D& b)
	{
		this->_val1 = b._val1;
		this->_val2 = b._val2;
		return *this;
	}
	Cartesian2D operator+(const double& b)
	{
		Cartesian2D a;
		a._val1 = this->_val1 + b;
		a._val2 = this->_val2 + b;
		return a;
	}
	Cartesian2D operator-(const double& b)
	{
		Cartesian2D a;
		a._val1 = this->_val1 - b;
		a._val2 = this->_val2 - b;
		return a;
	}
	Cartesian2D operator*(const double& b)
	{
		Cartesian2D a;
		a._val1 = this->_val1 * b;
		a._val2 = this->_val2 * b;
		return a;
	}
	Cartesian2D operator/(const double& b)
	{
		Cartesian2D a;
		a._val1 = this->_val1 / b;
		a._val2 = this->_val2 / b;
		return a;
	}

};

/// line: y=mx+b, with start point and end point, 
/// unit vector's direction is from start point to end point.
class LineSegment
{
private:
	Cartesian2D	_endPoint;
	double		_length;
	Cartesian2D	_startPoint;
	Cartesian2D	_unitVector;
	Cartesian2D	_vector;
	double		_voltPerPixel;

public:
	LineSegment(Cartesian2D startPoint, Cartesian2D endPoint);
	void SetVoltPerPixel(double voltPerPixel);
	double GetLineLength();
	double GetMaxXYLength();
	Cartesian2D GetUnitVector(double direction);
	Cartesian2D GetEndPoint();
	Cartesian2D GetStartPoint();
};

typedef std::vector<LineSegment> LineSegVec;	

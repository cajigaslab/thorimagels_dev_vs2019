#pragma once

class HighPerfTimer
{

private:
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	LARGE_INTEGER _frequency;
public:
	HighPerfTimer()
	{
		QueryPerformanceFrequency(&_frequency);
	}

	void Start()
	{
		QueryPerformanceCounter(&_start);
	}

	void Stop()
	{
		QueryPerformanceCounter(&_end);
	}

	double ElapsedMilliseconds()
	{
		return 1000.0*(_end.QuadPart - _start.QuadPart) / (double)(_frequency.QuadPart);
	}
};

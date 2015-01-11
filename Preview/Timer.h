//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Timer
{
	//////////////////////////////////////////////////////////////////////

	Timer()
	{
		Reset();
	}

	//////////////////////////////////////////////////////////////////////

	static uint64 Frequency()
	{
		uint64 frequency;
		QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
		return frequency;
	}

	//////////////////////////////////////////////////////////////////////

	static uint64 Ticks()
	{
		uint64 time;
		QueryPerformanceCounter((LARGE_INTEGER *)&time);
		return time;
	}

	//////////////////////////////////////////////////////////////////////

	void Reset()
	{
		mStartTime = Ticks();
		mOldTime = mStartTime;
	}

	//////////////////////////////////////////////////////////////////////

	double Elapsed()
	{
		uint64 time = Ticks();
		uint64 frequency = Frequency();
		return (double)(time - mStartTime) / (double)frequency;
	}

	//////////////////////////////////////////////////////////////////////

	double Delta()
	{
		uint64 time = Ticks();
		uint64 frequency = Frequency();
		double delta = (double)(time - mOldTime) / (double)frequency;
		mOldTime = time;
		return delta;
	}

	//////////////////////////////////////////////////////////////////////

private:

	uint64 mStartTime;
	uint64 mOldTime;
};

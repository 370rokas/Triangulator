#ifndef _FPERFORMANCE_H_
#define _FPERFORMANCE_H_

#include <windows.h>
#include <Mmsystem.h>
#include <conio.h>
#include <math.h>
#include "my_assert.h"

//#define FPERFORMANCERDTSC

class FPerformance {
	static double Freq;
	static bool isFirst;
	unsigned __int64 StartTicks;
	unsigned __int64 StopTicks;

public:
	inline void Start(void)
	{
		#ifdef FPERFORMANCERDTSC
			unsigned t1, t2;
			__asm {
				rdtsc
				mov t1,edx
				mov t2,eax
			}
			StartTicks = ((__int64)t1<<32)|(__int64)t2;
		#else
			QueryPerformanceCounter((LARGE_INTEGER*)&StartTicks);
		#endif
	}

	inline void Stop(void)
	{
		#ifdef FPERFORMANCERDTSC
			unsigned t1, t2;
			__asm {
				rdtsc
				mov t1,edx
				mov t2,eax
			}
			StopTicks = ((__int64)t1<<32)|(__int64)t2;
		#else
			QueryPerformanceCounter((LARGE_INTEGER*)&StopTicks);
		#endif
	}

	inline double GetDeltaTime(void)
	{
		my_assert(Freq > 1.0);
		return ((double)(StopTicks-StartTicks))/(Freq*1000.0);
	}

	double GetFreq(void)
	{
		#ifdef FPERFORMANCERDTSC
			unsigned tStart1, tStart2, tEnd1, tEnd2;
			timeBeginPeriod(1);
			unsigned time1 = timeGetTime();
			__asm {
				rdtsc
				mov tStart1,edx
				mov tStart2,eax
			}
			Sleep(1000);
			__asm {
				rdtsc
				mov tEnd1,edx
				mov tEnd2,eax
			}
			unsigned time2 = timeGetTime();
			timeEndPeriod(1);
			__int64 tDelta = (((__int64)tEnd1<<32)|(__int64)tEnd2) - (((__int64)tStart1<<32)|(__int64)tStart2);
			double fr = tDelta/(double)(time2-time1)/1000.0;
			return fr;
		#else
			unsigned __int64 _fr;
			QueryPerformanceFrequency((LARGE_INTEGER*)&_fr);
			return (double)_fr/1000.0/1000.0;
		#endif
	}

	FPerformance(void)
	{
		if(isFirst)
		{
			Freq = GetFreq();
			isFirst = false;
		}
		StartTicks = StopTicks = 0;
	}
	~FPerformance(void)
	{

	}
};

#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include <windows.h>

#include "cpu_timer.h"

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))


struct profile_anchor
{
    u64 TSCElapsed;
    u64 HitCount;
    char const *Label;
};

struct profiler
{
    profile_anchor Anchors[4096];
    
    u64 StartTSC;
    u64 EndTSC;
};
static profiler GlobalProfiler;

struct profile_block
{
    profile_block(char const *Label_, u32 AnchorIndex_)
    {
        AnchorIndex = AnchorIndex_;
        Label = Label_;
        StartTSC = ReadCPUTimer();
    }
    
    ~profile_block(void)
    {
        u64 Elapsed = ReadCPUTimer() - StartTSC;
        
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        Anchor->TSCElapsed += Elapsed;
        ++Anchor->HitCount;
                
        /* NOTE(casey): This write happens every time solely because there is no
           straightforward way in C++ to have the same ease-of-use. In a better programming
           language, it would be simple to have the anchor points gathered and labeled at compile
           time, and this repetative write would be eliminated. */
        Anchor->Label = Label;
    }

    char const *Label;
    u64 StartTSC;
    u32 AnchorIndex;
};

static void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
{
    u64 Elapsed = Anchor->TSCElapsed;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s[%llu]: %llu (%.2f%%)\n", Anchor->Label, Anchor->HitCount, Elapsed, Percent);
}

static void BeginProfile(void)
{
    GlobalProfiler.StartTSC = ReadCPUTimer();
}

static void EndAndPrintProfile()
{
    GlobalProfiler.EndTSC = ReadCPUTimer();
    u64 CPUFreq = EstimateCPUTimerFreq();
    
    u64 TotalCPUElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;
    
    if(CPUFreq)
    {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
    }
    
    for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GlobalProfiler.Anchors); ++AnchorIndex)
    {
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        if(Anchor->TSCElapsed)
        {
            PrintTimeElapsed(TotalCPUElapsed, Anchor);
        }
    }
}


static u64 GetOSTimerFreq(void)
{
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	return Freq.QuadPart;
}

static u64 ReadOSTimer(void)
{
	LARGE_INTEGER Value;
	QueryPerformanceCounter(&Value);
	return Value.QuadPart;
}

u64 ReadCPUTimer(void)
{
	// NOTE(casey): If you were on ARM, you would need to replace __rdtsc
	// with one of their performance counter read instructions, depending
	// on which ones are available on your platform.
	
	return __rdtsc();
}

u64 EstimateCPUTimerFreq(void)
{
	u64 MillisecondsToWait = 100;

	u64 OSFreq = GetOSTimerFreq();

	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
	while(OSElapsed < OSWaitTime)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}
	
	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;
	u64 CPUFreq = 0;
	if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
	
    return CPUFreq;
}
#ifndef CPU_TIMER_H
#define CPU_TIMER_H

#include <stdint.h>

typedef uint64_t u64;

u64 ReadCPUTimer(void);
u64 EstimateCPUTimerFreq(void);

#endif //CPU_TIMER_H
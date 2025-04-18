#ifndef _CPU_H_
#define _CPU_H_

#include "projectconfig.h"

typedef enum
{
  CPU_MULTIPLIER_1 = 0,
  CPU_MULTIPLIER_2,
  CPU_MULTIPLIER_3,
  CPU_MULTIPLIER_4
}
cpuMultiplier_t;

extern int last_clock;
extern int current_clock;
extern int timer32_cclk_30ms;
extern int timer16_cclk_inst;

//void cpuPllSetup (cpuMultiplier_t multiplier);
void cpuSetClock(int is_fast_clk); // is_fast_clk=0 (slow clock), =1 (fast_clock)
void cpuInit (void);
uint32_t cpuGetDeviceID (void);

#endif

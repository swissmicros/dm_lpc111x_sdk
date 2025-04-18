#ifndef __PMU_H__
#define __PMU_H__

#include "projectconfig.h"


void WAKEUP_IRQHandler( void );
void pmuInit( void );
void pmuSleep( void );
void pmuDeepSleep( uint32_t sleepCtrl, uint32_t wakeupMilliSeconds );
void pmuPowerDown( void );

// wakeup_type - second arg to sleep_ds
#define WT_ALL_KEYS  3
#define WT_KEYS      2
#define WT_ON        1
#define WT_NONE      0

// Sleep for val/10 seconds. The sleep could be interrupted
// by key press when wait_for_key == 1.
void sleep_ds(uint32_t val, uint8_t wakeup_type);

extern volatile int wakeup_int_cnt;
extern volatile int wdt_int_cnt;

#endif

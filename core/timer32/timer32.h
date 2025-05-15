#ifndef __TIMER32_H__ 
#define __TIMER32_H__

#include "projectconfig.h"

#define TIMER32_CCLK_1US        ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 1000000)
#define TIMER32_CCLK_10US       ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 100000)
#define TIMER32_CCLK_100US      ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 10000)
#define TIMER32_CCLK_1MS        ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 1000)
#define TIMER32_CCLK_10MS       ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 100)
#define TIMER32_CCLK_30MS       ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 33)  // rounded, but how accurate is it anyway?
#define TIMER32_CCLK_66MS       ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 16)  // rounded, but how accurate is it anyway?
#define TIMER32_CCLK_100MS      ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 10)
#define TIMER32_CCLK_250MS      ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 4)
#define TIMER32_CCLK_500MS      ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 2)
#define TIMER32_CCLK_1S         (CFG_CPU_CCLK/SCB_SYSAHBCLKDIV)
#define TIMER32_DEFAULTINTERVAL (TIMER32_CCLK_100US)

#define TIMER32_DELAY_100US     (1)            // 100uS delay = 1 tick
#define TIMER32_DELAY_1MS       (10)           // 1mS delay = 10 ticks
#define TIMER32_DELAY_1S        (10000)        // 1S delay = 10000 ticks

void TIMER32_0_IRQHandler(void);
void TIMER32_1_IRQHandler(void);

extern volatile uint32_t timer32_0_counter;
extern volatile uint32_t timer32_1_counter;

#define CFG_BEEP
void beep(int fr, int len);

void timer32Delay(uint8_t timerNum, uint32_t delay);
void timer32Enable(uint8_t timerNum);
void timer32Disable(uint8_t timerNum);
void timer32Reset(uint8_t timerNum);
void timer32Init(uint8_t timerNum, uint32_t timerInterval);

#endif

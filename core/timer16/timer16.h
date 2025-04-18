#ifndef __TIMER16_H__
#define __TIMER16_H__

#include "projectconfig.h"

#define TIMER16_DEFAULTINTERVAL   (0xFFFF)    // ~5.46mS @ 12MHz, ~1.82mS @ 36MHz, ~1.37mS @ 48MHz

#define TIMER16_CCLK_100US      ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 10000)
#define TIMER16_CCLK_1MS        ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV) / 1000)

void TIMER16_0_IRQHandler(void);
void TIMER16_1_IRQHandler(void);

extern volatile uint32_t timer16_0_counter;
extern volatile uint32_t timer16_1_counter;

void timer16DelayTicks(uint8_t timerNum, uint16_t delayInTicks);
void timer16DelayUS(uint8_t timerNum, uint16_t delayInUS);
void timer16Enable(uint8_t timerNum);
void timer16Disable(uint8_t timerNum);
void timer16Reset(uint8_t timerNum);
void timer16Init(uint8_t timerNum, uint16_t timerInterval);

#endif

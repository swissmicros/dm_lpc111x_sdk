#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "projectconfig.h"

void systickInit (uint32_t delayMs);
void systickDelay (uint32_t delayTicks) ;
uint32_t systickGetTicks(void);
uint32_t systickGetRollovers(void);
uint32_t systickGetSecondsActive(void);

#endif
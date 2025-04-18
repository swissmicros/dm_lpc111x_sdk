#ifndef _WDT_H_
#define _WDT_H_

#include "projectconfig.h"

void wdtInit (void);
void wdtFeed (void);
//volatile uint32_t wdt_counter;

#endif

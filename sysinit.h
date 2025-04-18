#ifndef __SYSINIT_H__
#define __SYSINIT_H__

#include "projectconfig.h"

#include "core/gpio/gpio.h"
#include "core/systick/systick.h"

#ifdef CFG_RTC
#include "drivers/rtc/pcf8563.h"
#endif

// Function prototypes
void systemInit();
void systemClkChgInit();

#endif

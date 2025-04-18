#include <stdio.h>

#include "projectconfig.h"

#include "core/cmd/cmd.h"
#include "commands.h"

#include "core/gpio/gpio.h"
#include "core/pmu/pmu.h"
#include "core/cpu/cpu.h"
#include "core/uart/uart.h"

#include "drivers/lcd/bitmap/st7565/st7565.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/vref/vref.h"

#include "rtc.h"
#include "main.h"


#ifdef CFG_INTERFACE


#define CR CFG_PRINTF_NEWLINE




// ==================
//      VREF
// ==================
#ifdef CFG_VREF
#ifndef CFG_32k_CODE
void cmd_battery_read(uint8_t argc, char **argv) {
  printf("BAT: %imV" CR, battery_voltage());
}
#endif
#endif



// ==================
//      RTC
// ==================
#ifdef CFG_RTC

void cmd_get_time(uint8_t argc, char **argv) {
  char s[30];
  rtc_get_date_string(s);
  printf("%s" CR, s);
}
int get3ints(char*s, uint8_t *a, uint8_t *b) {
  int v,w;

  if ( !getNumber(s, &v) ) return -1;

  w = v % 10000;
  *a = w % 100;
  *b = w / 100;
  return v / 10000;
}
void cmd_set_time(uint8_t argc, char **argv) {
  tm_t t;
  dt_t d;

  d.year = get3ints(argv[1], &d.day, &d.month);
  t.hour = get3ints(argv[2], &t.sec, &t.min);
  if (d.year < 1900 || t.hour < 0) {
    // Fail
    return;
  }
  rtc_set_datetime(&d, &t);
}
void cmd_update_time(uint8_t argc, char **argv) {
  int delta_min;
  getNumber(argv[1], &delta_min);
  rtc_update_time_sec(delta_min*60);
}
#endif




#endif
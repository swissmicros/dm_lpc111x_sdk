#ifndef __TIME_H__
#define __TIME_H__

#include <stdint.h>

// Julian date for 1.1.1900 -> 2415021
#define JUL_DAY_1_1_1900 2415021
#define DAY_SECONDS (24*3600)

typedef struct {
  uint16_t year;
  uint8_t  month;
  uint8_t  day;
} dt_t;

typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t csec; // 1/100s of second
} tm_t;

char* get_wday_shortcut(int day); // 0 = Monday

int julian_day(dt_t *d);
void julian_to_date(int julian_day, dt_t *d);

int tm_to_seconds(tm_t *t);
int tm_to_cseconds(tm_t *t);
void seconds_to_tm(int seconds, tm_t *t);
void cseconds_to_tm(int cseconds, tm_t *t);

int dt_equal(dt_t *a, dt_t *b);

#endif
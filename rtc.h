#ifndef __RTC_H__
#define __RTC_H__

#include "time.h"

#ifdef CFG_RTC

#define RTCINT_PORT    0
#define RTCINT_PIN       1
#define RTCINT_PIO     0_1
#define WAKEUP_RTCINT  WAKEUP1_IRQn

//#define RTC_ON_SEPARATE_PIN
#ifdef RTC_ON_SEPARATE_PIN
#define read_rtcint_pin() (!gpioGetValue(RTCINT_PORT,RTCINT_PIN))
#else
#define read_rtcint_pin() read_on_key_pin(0)
#endif


#define TMR_STAT_TF 4
#define TMR_STAT_AF 8

typedef struct {
  dt_t d;
  tm_t t;
} datetime_t;

typedef struct {
  int d;
  int t;
} idatetime_t;

typedef struct {
  // Status
  uint8_t rtc_status; // 4 - TMR_STAT_TF
                      // 8 - TMR_STAT_AF
 
  uint8_t rtc_second; // last INT detected on this second
  uint8_t ms_running;
  uint8_t update_in_second;

  // State
  uint8_t int_enabled;
  uint8_t int_configured;
  uint8_t alarm_enabled;
  uint8_t second_read;
  uint8_t full_second;

} tmr_stat_t;


void rtc_init();
void rtc_enable_int();
void rtc_disable_int();
void rtc_int();
void rtc_update_status();

int  rtc_get_cs();
int  rtc_get_ms();

void  display_clock();

// Needs some 30char string
void rtc_get_date_string(char* s);
int rtc_update_time_sec(int delta_sec);
// Sets datetime and resets fractional part of seconds
void rtc_set_datetime(dt_t *d, tm_t *t);
// Just write the time to RTC
int rtc_write_datetime(dt_t *d, tm_t *t, int wday);

extern volatile int rtc_last_ms_cnt;
extern volatile int rtc_ms_cnt;  

void rtc_ms_timer_start();
void rtc_ms_timer_stop();
void rtc_poll_int();

int  rtc_check_for_wakeup(int is_off);
void rtc_alarms_before_sleep(int is_off);


#endif
#endif

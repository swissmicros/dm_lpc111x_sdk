//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>

#include "projectconfig.h"
#include "sysinit.h"
#include "drivers/keyboard/keyboard.h"
#include "main.h"


#include <strings.h>

#ifdef CFG_RTC

#include "drivers/lcd/bitmap/st7565/st7565.h"
#include "drivers/lcd/smallfonts.h"

#include "rtc.h"

tmr_stat_t tmr_stat;
uint16_t base_year;

int rtc_read_sec();
void rtc_update_sec();

#define B2DEC(ix) bcd2dec(b[ix])
#define DEC2B(x)  dec2bcd(x)

int bcd2dec(int val) {
  return (val >> 4)*10 + (val & 0xf);
}

int dec2bcd(int val) {
  return ((val/10)<<4)|(val%10);
}


void rtc_init() {
  bzero(&tmr_stat, sizeof(tmr_stat));

  base_year = 2000;

  rtc_update_sec(); // Handle interrupts and read current second
  rtc_ms_cnt = 0;
  rtc_last_ms_cnt = 1000;

  rtc_enable_int();  // Force int configuration by enabling
  rtc_disable_int(); // ... and disabling it
}


void rtc_configure_int() {
/*
    IOCON_JTAG_TDI_PIO0_11 &= ~IOCON_JTAG_TDI_PIO0_11_FUNC_MASK;
    IOCON_JTAG_TDI_PIO0_11 |= IOCON_JTAG_TDI_PIO0_11_FUNC_GPIO;

    // Set GPIO0.11 to input
    gpioSetDir(0, 11, gpioDirection_Input);
    gpioSetPullup (&IOCON_PIO1_4, gpioPullupMode_PullUp);
    gpioSetPullup (&IOCON_JTAG_TDI_PIO0_11, gpioPullupMode_Inactive);

    // Setup an interrupt on GPIO0.11
    gpioSetInterrupt(0,                               // Port
                     11,                              // Pin
                     gpioInterruptSense_Edge,         // Edge/Level Sensitive
                     gpioInterruptEdge_Single,        // Single/Double Edge
                     gpioInterruptEvent_ActiveLow);   // Rising/Falling
    // Enable the interrupt
    gpioIntEnable(0, 11);
*/
    //IOCON_PIO1_4 &= ~IOCON_PIO1_4_FUNC_MASK;
    //IOCON_PIO1_4 |=  IOCON_PIO1_4_FUNC_GPIO;

    //gpioSetDir(1,  4, gpioDirection_Input);
    //gpioSetPullup (&IOCON_PIO1_4, gpioPullupMode_Inactive);
#if 0
    gpioSetInterrupt(1 /* Port */, 4 /* Pin */,
                     gpioInterruptSense_Edge,         // Edge/Level Sensitive
                     gpioInterruptEdge_Single,        // Single/Double Edge
                     gpioInterruptEvent_ActiveLow);   // Rising/Falling
    // Enable the interrupt
    gpioIntEnable(1, 4);
#endif

  pcf8563_write_val(PCF8563_TIMER_CONTROL, 2, 0x0182);
  tmr_stat.int_configured = 1;
}

void update_rtc_stat2() {
  if ( !tmr_stat.int_configured ) 
    rtc_configure_int();

  pcf8563_write_val(PCF8563_CTL_STAT2, 1, 0xc | (tmr_stat.alarm_enabled<<1) | tmr_stat.int_enabled);
}


// Enable 1sec interrupt
void rtc_enable_int() {
  if ( tmr_stat.int_enabled )
    return;
  if ( !tmr_stat.int_configured ) 
    rtc_configure_int();

  tmr_stat.int_enabled = 1;
  update_rtc_stat2(); 
}

// Disable 1sec interrupt
void rtc_disable_int() {
  if ( !tmr_stat.int_enabled )
    return;

  tmr_stat.int_enabled = 0;

  tmr_stat.full_second = 0;
  tmr_stat.update_in_second = 0;

  update_rtc_stat2();
}

void rtc_enable_alarm_int(datetime_t *dt) {
  // Set RTC alarm
  uint8_t* b = I2CDataToSend;
  //uint32_t* c = (uint32_t*)I2CDataToSend;

  b[0] = DEC2B(dt->t.min);
  b[1] = DEC2B(dt->t.hour);
  b[2] = DEC2B(dt->d.day);
  b[3] = 0x80; // Disable week alarm
  pcf8563_write_reg_retry(PCF8563_MINUTE_ALARM, 4);

  tmr_stat.alarm_enabled = 1;
  update_rtc_stat2();
}

void rtc_disable_alarm_int() {
  uint32_t* c = (uint32_t*)I2CDataToSend;

  if ( !tmr_stat.alarm_enabled )
    return;

  c[0] = 0x80808080; // Disable all alarm flags
  pcf8563_write_reg_retry(PCF8563_MINUTE_ALARM, 4);

  tmr_stat.alarm_enabled = 0;
  update_rtc_stat2();
}


int rtc_read_sec() {
  uint8_t* b = I2CDataRead;
  int ret = pcf8563_read_val(PCF8563_VL_SECONDS,1);
  tmr_stat.rtc_second = bcd2dec(b[0]&0x7f);
  tmr_stat.second_read = ret;
  return ret;
}



void rtc_poll_int() {
  if ( read_rtcint_pin() )
    rtc_int();
}

void rtc_update_sec() {
  tmr_stat.second_read = 0;
  rtc_poll_int();
  if (!tmr_stat.second_read)
    rtc_read_sec();
}

int rtc_get_ms() {
  int ms = 1000*rtc_ms_cnt/rtc_last_ms_cnt;
  return (ms>=1000)?999:ms;
}

int rtc_get_cs() {
  int cs = 100*rtc_ms_cnt/rtc_last_ms_cnt;
  return (cs>=100)?99:cs;
}

void rtc_ms_timer_start() {
  // rtc_update_sec(); - already in timer_wakeup
  // also new second detected and rtc_ms_cnt zeroed
  tmr_stat.ms_running = true;
}
void rtc_ms_timer_stop() {
  tmr_stat.ms_running = false;
  tmr_stat.full_second = 0;
  //tmr_stat.last_ms_sec = tmr_stat.rtc_second;
}

void rtc_timer_int() {
  // Timer interrupt handler
  //tmr_stat.sec_int++;

  // Read current second from RTC
  rtc_read_sec();

  // ms handling
  if (tmr_stat.ms_running) {
    if (tmr_stat.full_second < 2)
      tmr_stat.full_second++;
    else
      rtc_last_ms_cnt = rtc_ms_cnt;
  } else {
    tmr_stat.full_second = 0;
  }
  tmr_stat.update_in_second = 0;
  rtc_ms_cnt = 0;
}


void rtc_int() {
  rtc_update_status();

  if ( tmr_stat.rtc_status & TMR_STAT_TF )
    rtc_timer_int(); // Handle timer interrupt
  
  if ( tmr_stat.rtc_status & TMR_STAT_AF ) {
    // Alarm interrupt
  }

  // All handled
  tmr_stat.rtc_status = 0;
}


void rtc_update_status() {
  uint8_t ret = pcf8563_read_and_clear_status() & 0xc; // Store just flags
  if ( ret > 0 )
    tmr_stat.rtc_status |= ret;
}

int rtc_check_for_wakeup(int is_off) {
  rtc_poll_int();
  // TODO: default RTC functionality?
  return 0;
}


void rtc_alarms_before_sleep(int is_off) {
  // No alarm used by default
  rtc_disable_int();
}


void rtc_mask_time(dt_t *d, tm_t *t) {
  uint8_t* b = I2CDataRead;

/*         +----------------+ 7  6  5  4  3  2  1  0
  b[0]  02 | VL_seconds     | VL |---- SECONDS ----| (0 to 59)
  b[1]  03 | Minutes        | x  |---- MINUTES ----| (0 to 59)
  b[2]  04 | Hours          | x  x  |--- HOURS ----| (0 to 23)
  b[3]  05 | Days           | x  x  |---- DAYS ----| (1 to 31)
  b[4]  06 | Weekdays       | x  x  x  x  x  |- WD-| WEEK DAYS (0 to 6)
  b[5]  07 | Century_months | C  x  x  |- MONTHS --| (1 to 12)
  b[6]  08 | Years          | |--------- YEARS ----| (0 to 99)
           +----------------+ 7  6  5  4  3  2  1  0                        */

  /* Mask values */
  uint32_t *u = (uint32_t *)b;
  uint16_t *v = (uint16_t *)(b+4);
  u[0] &= 0x3f3f7f7f;
  v[0] &= 0x1f07;

  //b[0] &= 0x7f; // seconds
  //b[1] &= 0x7f; // minutes
  //b[2] &= 0x3f; // hours
  //b[3] &= 0x3f; // days
  //b[4] &= 7;    // wday
  //b[5] &= 0x1f; // month

  d->year  = B2DEC(6)+base_year;
  d->month = B2DEC(5);
  d->day   = B2DEC(3);
  t->hour  = B2DEC(2);
  t->min   = B2DEC(1);
  t->sec   = B2DEC(0);
}

void rtc_get_date_string(char* s) {
  uint8_t* b = I2CDataRead;
  dt_t d;
  tm_t t;
  
  if ( !pcf8563_read_time() ) {
    sprintf(s,"fail");
    return;
  }

  /* Mask values */
  rtc_mask_time(&d, &t);
  sprintf(s, "%4i-%02i-%02i %02i:%02i:%02i %s",
    d.year, d.month, d.day, t.hour, t.min, t.sec, get_wday_shortcut(b[4]));
}


int rtc_write_datetime(dt_t *d, tm_t *t, int wday) {
  uint8_t* b = I2CDataToSend;
  int y100 = d->year % 100;

  base_year = d->year - y100;

  // Set new date/time to RTC
/*         +----------------+ 7  6  5  4  3  2  1  0
  b[0]  02 | VL_seconds     | VL |---- SECONDS ----| (0 to 59)
  b[1]  03 | Minutes        | x  |---- MINUTES ----| (0 to 59)
  b[2]  04 | Hours          | x  x  |--- HOURS ----| (0 to 23)
  b[3]  05 | Days           | x  x  |---- DAYS ----| (1 to 31)
  b[4]  06 | Weekdays       | x  x  x  x  x  |- WD-| WEEK DAYS (0 to 6)
  b[5]  07 | Century_months | C  x  x  |- MONTHS --| (1 to 12)
  b[6]  08 | Years          | |--------- YEARS ----| (0 to 99)
           +----------------+ 7  6  5  4  3  2  1  0                        */

  b[0] = DEC2B(t->sec);
  b[1] = DEC2B(t->min);
  b[2] = DEC2B(t->hour);
  b[3] = DEC2B(d->day);
  b[4] = wday;
  b[5] = DEC2B(d->month);
  b[6] = DEC2B(y100);
  return pcf8563_write_time();
}


int rtc_set_stop_flag(int val) {
  return pcf8563_write_val(PCF8563_CTL_STAT1, 1, val? 0x20 : 0);
}


void rtc_set_datetime(dt_t *d, tm_t *t) {
  /* RTC starts next second after 0.507813 to 0.507935 sec after STOP flag is cleared.
     So, we have to set the time + 1 second to RTC, wait ~492ms (minus some delay compensation)
     and clear the STOP flag.

     Setting RTC time:
       1. Set STOP flag halting the time in RTC
       2. Write new date/time + 1 second (it looks like the RTC increments it itself)
       3. Wait 492ms - delays in set-time routines
       4. Clear STOP flag
       5. After 507ms we should get first second pulse
  */   
  //int ms_start = (int)systickTicks;
  rtc_set_stop_flag(1);
  rtc_write_datetime(d, t, julian_day(d) % 7);
  systickDelay(300); // Empirical value ... 
  rtc_set_stop_flag(0);
  //printf("SET TIME DURATION: %0ims\n", (int)systickTicks - ms_start);
}

int rtc_update_time_sec(int delta_sec) {
  int jday, sec;
  dt_t d;
  tm_t t;

  /* Read current second */
  if ( !rtc_read_sec() ) return 0;
  sec = tmr_stat.rtc_second;

  /* Wait for second start to get safe interval for time update */
  while ( sec == tmr_stat.rtc_second ) {
    systickDelay(10); // 10ms delay to not consume all power by continuous i2c communication
    if ( !rtc_read_sec() ) return 0;
  }

  if ( !pcf8563_read_time() )
    return 0;

  /* Mask values */
  rtc_mask_time(&d, &t);
  jday = julian_day(&d);

  /* Add seconds */
  sec = tm_to_seconds(&t) + delta_sec;

  // New seconds out of day range -> add/sub day
  if ( sec < 0 || sec >= DAY_SECONDS ) {
    //printf("RTC: day overflow %i/%02i/%02i jday=%i\n", d.year, d.month, d.day, jday);
    jday += (sec<0) ? -1 : 1;
    julian_to_date(jday, &d);
    //printf("     new day      %i/%02i/%02i jday=%i\n", d.year, d.month, d.day, jday);
    sec += (sec<0) ? DAY_SECONDS : -DAY_SECONDS;
  }
  seconds_to_tm(sec, &t);
  return rtc_write_datetime(&d, &t, jday % 7);
}





#endif

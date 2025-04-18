#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "projectconfig.h"
#include "sysinit.h"

#include "core/gpio/gpio.h"
#include "core/systick/systick.h"

#include "project/commands.h"

#include "core/cpu/cpu.h"
#include "core/pmu/pmu.h"

#include "core/timer32/timer32.h"
#include "core/timer16/timer16.h"

#include "core/cmd/cmd.h"
#include "core/uart/uart.h"


#include "drivers/lcd/bitmap/st7565/st7565.h"
#include "drivers/lcd/smallfonts.h"

#include "drivers/keyboard/keyboard.h"
#include "drivers/vref/vref.h"
#include "rtc.h"

#define FIRST_FONT font13x12s


void disp_two_lines(int i, char *s1, char *s2) {
  st7565ClearScreen();
  xGlcd_Write_Text(s1, i, 0, FIRST_FONT);
  xGlcd_Write_Text(s2, 1, 9, font3x6);
  st7565Refresh();
}


void tohex(char* s, uint32_t a, int nr_dig) {
  int i = nr_dig;
  char b;

  s[i--]=0;
  while(i>=0) {
    b=a & 0xf;
    s[i--] = b + ((b>9)?'A'-10:'0');
    a>>=4;
  }
}

#define CR CFG_PRINTF_NEWLINE

void serial_prompt(char *s) {
  printf("%s %s" CR, CFG_FIRMWARE_VERSION,s);
}



void scan_key(int *k1, int *k2) {
#ifdef CFG_RTC
  // Check whether wakeup line (used for ON key reading) is free
  if ( read_rtcint_pin() ) {
    // Common line with RTC -> handle interrupt and release it
    rtc_int();
  }
  *k1=*k2=0;
  read_key(k1,k2);
  if ( read_rtcint_pin() ) {
    // ON key line was driven with RTC during key scan -> handle 
    // the RTC, free the line and re-read keys - as the interrupt
    // should be generated in 1s intervals this scan shouldn't be
    // affected with other RTC interrupt
    rtc_int();
    *k1=*k2=0;
    read_key(k1,k2);
  }
#else
  read_key(k1,k2);
#endif
}


void scan_key_direct(int *k1, int *k2) {
  *k1=*k2=0;
  read_key(k1,k2);
}


void key_release() {
  int k1,k2;
  int i=0;
  while (i < 3) {
    scan_key(&k1,&k2);
    systickDelay(10);
    i++;
    if (k1) i=0;
  }
}


// Serial console loop
void cmdLoop() {
  int k1,k2;
  int cnt;

  uartInit(CFG_UART_BAUDRATE);

  st7565ClearScreen();
  xGlcd_Write_Text("SERIAL CONSOLE", 0, 3, FIRST_FONT);
  st7565Refresh();

  key_release();
  serial_prompt("ready");

  // We will return after 60s inactivity into calc mode
  for(cnt=0; cnt<6000; cnt++) {
    if (uartRxBufferDataPending()) {
      cmdPoll();
      cnt=0;
    } else {
      systickDelay(10);
      scan_key(&k1,&k2);
      if(k1) break; // key pressed -> break immediately
    }
  }
  key_release();
  printf(CR "Bye..." CR);
}

void get_firmware_str(char *str) {
  sprintf(str,"%s %s  %iMHZ",
          CFG_FIRMWARE_VERSION,
          CFG_FIRMWARE_VERSION_REVISION,
          current_clock/1000000
         );

  if ( is_vref() ) {
    int a = battery_voltage();
    int b = a/1000;
    sprintf(str+strlen(str), "  %i.%03iV", b, a-b*1000);
  }
}

#define CLOCK_MODE_TIMEOUT  5*60 // seconds

void display_clock() {
  char s[30],t[8];
  int tout = CLOCK_MODE_TIMEOUT;
  int k1,k2;
  key_release();
  rtc_enable_int();
  strcpy(t," ");
  do {
    rtc_get_date_string(s);
    s[19]=0;
    strcpy(t+1,s+20);
    disp_two_lines(4, s, t);
    sleep_ds(0, WT_ALL_KEYS); // Wait for int from RTC or key press
    k1=k2=0;
    scan_key(&k1, &k2);
  } while( !k1 && tout--);
  key_release();
}


void go_to_sleep() {
  //int k1,k2;
  key_release();

  rtc_disable_int();
  st7565TurnOff();

  sleep_ds(0, WT_ON);
  if(!read_rtcint_pin()) // No sleep workaround 
    sleep_ds(0, WT_ON);

  st7565Init();
  st7565ClearScreen();
  st7565Refresh();
}



int main(void)
{
  // Configure cpu and mandatory peripherals
  systemInit();
  init_keyboard_pins();

  /* Vref pins to idle state */
  gpioSetPullup(&IOCON_PIO1_11, gpioPullupMode_Inactive);
  gpioSetPullup(&IOCON_PIO3_2,  gpioPullupMode_Inactive);
  vref_idle();

#ifdef CFG_RTC
  pcf8563_init();
#else
  // Configure unused i2c pins - this way they have
  // lower consumption than with pull-up/down
  setpin(0, 4, 1, 0);
  setpin(0, 5, 1, 0);
#endif

#ifdef CFG_INTERFACE
  cmdInit();
#endif

#ifdef CFG_RTC
  rtc_init();
#endif

  // Main loop
  {
    char s[40],t[40];
    int k1,k2;
  
    for(;;) {
      k1=k2=0;
      scan_key(&k1,&k2);
      if ( k1 == 0x18 )  go_to_sleep();
      if ( k1 == 0x13 )  display_clock();
      if ( k1 == 0x73 )  cmdLoop();

      sprintf(s,"KEY: %02X - %02X",k1,k2);
      get_firmware_str(t);

      disp_two_lines(2,s,t);

      sleep_ds(6, WT_ALL_KEYS);  // wait 0.5s - not for key-press
    }
  }


  return 0;
}


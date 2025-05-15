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


void wait_ms(int ms) {
  systickDelay(ms);
}


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


/* ============= ISP ================*/

#define ENABLE_ISP
#ifdef ENABLE_ISP

#define IAP_ENTRY 0x1fff1ff1
#define IAP_CMD_REINVOKE_ISP 57

typedef void (*IAP)(unsigned int[], unsigned int[]);


void start_isp() {
  unsigned int cmd[1];
  unsigned int stat[1];
  IAP iap_entry = (IAP)IAP_ENTRY;

  disp_two_lines(1,"BOOTLOADER READY","RESET TO ABORT");
  systickDelay(100);

  __disable_irq();

#if 1 // Set clock to internal osc
  // Using just internal osc for slow clock
  SCB_MAINCLKSEL = SCB_CLKSEL_SOURCE_INTERNALOSC;

  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;  // Update clock source
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_DISABLE; // Toggle update register once
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;

  // Wait until the clock is updated
  while (!(SCB_MAINCLKUEN & SCB_MAINCLKUEN_UPDATE));

  // Set system AHB clock
  SCB_SYSAHBCLKDIV = 1; // Divide by 1 == SCB_SYSAHBCLKDIV_DIV1;

  // Enabled IOCON clock for I/O related peripherals
  SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_IOCON;

  // Disable system PLL for slow mode
  SCB_PDRUNCFG |= SCB_PDRUNCFG_SYSPLL_MASK;
#endif

  cmd[0] = IAP_CMD_REINVOKE_ISP;
  iap_entry(cmd,stat);
  // shouldn't return
}

#endif // ENABLE_ISP


#if 0 // Allows to program CRP
#define NO_ISP 0x4E697370
#define NO_CRP 0xFFFFFFFF

__attribute__ ((section(".crp"))) const uint32_t CRP_WORD = 
#ifdef DISABLE_DEFULAT_ISP_AFTER_RESET
 NO_ISP;
#else
 NO_CRP;
#endif
#endif
// ---



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
  char s[30],t[30];
  int tout = CLOCK_MODE_TIMEOUT;
  int k1,k2;
  key_release();
  rtc_enable_int();
  strcpy(t," ");
  do {
    rtc_get_date_string(s);
    s[19]=0;
    strcpy(t+1,s+20);
    sprintf(t+strlen(t), "       TIMEOUT:%i", tout);  // NO
    disp_two_lines(4, s, t);
    sleep_ds(0, WT_ALL_KEYS); // Wait for int from RTC or key press
    k1=k2=0;
    scan_key(&k1, &k2);
  } while( !k1 && tout--);
  rtc_disable_int();
  key_release();
}


void blip() {
  const int f = 1840; //1864;
  beep(f,18*2); wait_ms(27);
  beep(f,23*2); wait_ms(17);
  beep(f,23*2); wait_ms(13);
  beep(f,23*2); wait_ms(40);
  beep(f,23*2); wait_ms(30);
}


void go_to_sleep() {
  int k1=0,k2;

  rtc_disable_int();       // No 1-sec RTC int in off mode
  if ( read_rtcint_pin() ) // Handle pending RTC int
    rtc_int();

  while (k1 != 0x18) { // Accept wake-up only on ON-key
    key_release();

    st7565TurnOff();

    sleep_ds(0, WT_ON);
  
    k1=k2=0;
    scan_key(&k1,&k2);

    st7565Init();

#if 0
    st7565ClearScreen();
    xGlcd_Write_Text("WAKE-UP", 1, 0, FIRST_FONT);
    st7565Refresh();
    wait_ms(500);
#endif
  }

}



// RESET status value (SCB->SYSRSTSTAT)
uint32_t SCB_RESETSTAT_backup;


int main(void)
{
  // Force proper RESET (after start from bootloader) to avoid
  // configuration problems
  if ((SCB_RESETSTAT & 0x1f) == 0) SCB_AIRCR = SCB_AIRCR_SYSRESET;
  SCB_RESETSTAT_backup = SCB_RESETSTAT;
  SCB_RESETSTAT = 0x1F;  // Reset RESET status


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

  beep(1000,100); wait_ms(40); beep(1500,40); wait_ms(20); beep(1500,40);

  // Main loop
  {
    char s[40],t[40];
    int k1,k2;
    const int AUTO_OFF_TIMEOUT = 2*60*2; // 2min
    int auto_off = AUTO_OFF_TIMEOUT;
  
    for(;;) {
      k1=k2=0;
      scan_key(&k1,&k2);
      int k1rc = keycode2rc(k1);
      int k2rc = keycode2rc(k2);
      
      if (k1rc == 0x41) go_to_sleep();    // ON
      if (k1rc == 0x11) display_clock();  //  A
      if (k1rc == 0x12) blip();           //  B
      if (k1rc == 0x13) cmdLoop();        //  C
      if (k1rc == 0x42 && k2rc == 0x14) start_isp(); // [f]+D

      sprintf(s,"KEY: %02X - %02X",k1rc,k2rc);
      get_firmware_str(t);

      disp_two_lines(2,s,t);

#if 1
      sprintf(s, "OFF:%i:%X", auto_off, (int)SCB_RESETSTAT_backup);
      xGlcd_Write_Text(s, 90, 0, font3x6);
      st7565Refresh();
#endif

      // Auto-off handling
      if (k1) {
        auto_off = AUTO_OFF_TIMEOUT;
      } else {
        auto_off--;
        if (auto_off <= 0) {
          auto_off = AUTO_OFF_TIMEOUT;
          go_to_sleep();
        }
      }

      sleep_ds(6, WT_ALL_KEYS);  // wait 0.5s - or for key-press
    }
  }


  return 0;
}


#include "core/gpio/gpio.h"
#include "core/cpu/cpu.h"
#include "core/timer32/timer32.h"
#include "core/i2c/i2c.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/lcd/bitmap/st7565/st7565.h"
#include "pmu.h"
#include "rtc.h"

#define PMU_WDTCLOCKSPEED_HZ 7812

void pmuSetupHW();
void pmuRestoreHW();

volatile int pmu_not_slept = 0;

/**************************************************************************/
/*! 
    Setup the clock for the watchdog timer.  The default setting is 7.8kHz.
*/
/**************************************************************************/
void pmuWDTClockInit()
{
  /* Configure watchdog clock */
  /* Freq. = 0.5MHz, div = 64: WDT_OSC = 7.8125kHz  */
  /* Make sure this value is also reflected in PMU_WDTCLOCKSPEED_HZ */
  SCB_WDTOSCCTRL = SCB_WDTOSCCTRL_FREQSEL_0_5MHZ |
                   SCB_WDTOSCCTRL_DIVSEL_DIV64;

  /* Set divider */
  SCB_WDTCLKDIV = SCB_WDTCLKDIV_DIV1;

  /* Enable WDT clock */
  SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_WDTOSC);

  /* === Prepare WDT for running === */
#if 0

  // Set system AHB clock
  SCB_SYSAHBCLKDIV = SCB_SYSAHBCLKDIV_DIV1;

  /* Enable AHB clock to the WDT domain. */
  SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_WDT;

  // Set clock source (use WDT oscillator)
  SCB_WDTCLKSEL = SCB_WDTCLKSEL_SOURCE_WATCHDOGOSC;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_UPDATE;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_DISABLE;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_UPDATE;

  // Wait until updated
  while (!(SCB_WDTCLKUEN & SCB_WDTCLKUEN_UPDATE));

  /* Enable the watchdog timer (without system reset) */
  WDT_WDMOD = WDT_WDMOD_WDEN_ENABLED |
              WDT_WDMOD_WDRESET_DISABLED;

  /* Disable the WDT interrupt */
  NVIC_DisableIRQ(WDT_IRQn);
#endif
}


/**************************************************************************/
/*! 
    @brief Initialises the power management unit
*/
/**************************************************************************/
void pmuInit( void )
{
  SCB_PDRUNCFG &= ~(
                    SCB_PDRUNCFG_WDTOSC_MASK |
//                    SCB_PDRUNCFG_IRC_MASK    |
//                    SCB_PDRUNCFG_SYSOSC_MASK |
//                    SCB_PDRUNCFG_ADC_MASK    |
                    0);

  pmuWDTClockInit();

  return;
}



/**************************************************************************/
/*! 
    Wakeup interrupt handler
*/
/**************************************************************************/
volatile int wakeup_int_cnt = 0;


void wakeup_setup() {
  /*
  {
    uint32_t regVal;

    // This handler takes care of all the port pins if they
    // are configured as wakeup source.
    regVal = SCB_STARTSRP0;
    if (regVal != 0)
    SCB_STARTRSRP0CLR = regVal;
  }
  */
  // Keyboard lines
  NVIC_DisableIRQ(WAKEUP_KBD_H0);
  NVIC_DisableIRQ(WAKEUP_KBD_H1);
  NVIC_DisableIRQ(WAKEUP_KBD_H2);
  NVIC_DisableIRQ(WAKEUP_KBD_H3);
  NVIC_DisableIRQ(WAKEUP_KBD_ON);
#ifdef RTC_ON_SEPARATE_PIN
  NVIC_DisableIRQ(WAKEUP_RTCINT);
#endif

  // Time-out wakeup
  NVIC_DisableIRQ(WAKEUP9_IRQn);      // P0.9  (CT16B0_MAT1)
  //NVIC_DisableIRQ(WAKEUP1_IRQn);      // P0.1  (CT32B0_MAT2)

  // No wakeup sources
  SCB_STARTERP0 = 0;
  /* Clear all wakeup sources */
  SCB_STARTRSRP0CLR = SCB_STARTRSRP0CLR_MASK;

  //TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_DISABLED;
  TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_DISABLED;

  // Clear match bit on timer
  //TMR_TMR32B0EMR = 0;
  TMR_TMR16B0EMR = 0;

  /* Clear the interrupt flag */
  TMR_TMR16B0IR = TMR_TMR16B0IR_MR0;
  TMR_TMR32B0IR = TMR_TMR32B0IR_MR0;

  // Perform custom wakeup tasks
  pmuRestoreHW();
}

void WAKEUP_IRQHandler(void)
{
  // Wake-up counter
  wakeup_int_cnt++;

  // Reconfigure system clock/PLL
  cpuSetClock(last_clock);

  wakeup_setup();

  /* See tracker for bug report. */
  __asm volatile ("NOP");
  __asm volatile ("NOP");
}



/************************************************

**/

void pmuTmr0DeepSleep(uint32_t wakeup_dSec, uint8_t wakeup_type)
{
  __disable_irq();

  // Setup the board for deep sleep mode, shutting down certain
  // peripherals and remapping pins for lower power
  pmuSetupHW();

  SCB_PDAWAKECFG = SCB_PDRUNCFG;

  /*
  sleepCtrl &= ~(1 << 9);               // MAIN_REGUL_PD
  sleepCtrl |= (1 << 11) | (1 << 12);   // LP_REGUL_PD
  SCB_PDSLEEPCFG = sleepCtrl;
  */
  /* user.manual.lpc11xx.lpc11cxx.pdf states that this have to be
   initialized to one of four values depending on BOD/WDTosc use during
   deep-sleep. 
   See chapter "Deep-sleep mode configuration register"
         table "Allowed values for PDSLEEPCFG register"  */
  SCB_PDSLEEPCFG = (wakeup_dSec)?0x18BF:0x18FF;

  /* Sleep deeply */
  SCB_SCR |= SCB_SCR_SLEEPDEEP;

  // No wakeup sources
  SCB_STARTERP0 = 0;


  if (wakeup_type & WT_ALL_KEYS) {


#define _SCB_STARTAPRP0_APR(pio) SCB_STARTAPRP0_APRPIO ## pio
#define SCB_STARTAPRP0_APR(pio) _SCB_STARTAPRP0_APR(pio)
#define _SCB_STARTERP0_ER(pio)   SCB_STARTERP0_ERPIO ## pio
#define SCB_STARTERP0_ER(pio)   _SCB_STARTERP0_ER(pio)

    // Use FALLING EDGE for wakeup detection.
    /*
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_H0);
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_H1);
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_H2);
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_H3);
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_ON);
    */
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APR(KBD_PIO_H0)
                   &  ~SCB_STARTAPRP0_APR(KBD_PIO_H1)
                   &  ~SCB_STARTAPRP0_APR(KBD_PIO_H2)
                   &  ~SCB_STARTAPRP0_APR(KBD_PIO_H3)
#ifdef RTC_ON_SEPARATE_PIN    
                   &  ~SCB_STARTAPRP0_APR(RTCINT_PIO)
#endif
                   &  ~SCB_STARTAPRP0_APR(KBD_PIO_ON);

    /* Clear all wakeup sources */
    // if(!wakeup_dSec)
    //   SCB_STARTRSRP0CLR = SCB_STARTRSRP0CLR_MASK;

    /* Enable keyboard lines as wakeup source. */
    /*
    SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_H0);
    SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_H1);
    SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_H2);
    SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_H3);
    SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_ON);
    */
    if ( wakeup_type & WT_KEYS )
      SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_H0)
                    |  SCB_STARTERP0_ER(KBD_PIO_H1)
                    |  SCB_STARTERP0_ER(KBD_PIO_H2)
                    |  SCB_STARTERP0_ER(KBD_PIO_H3)
#ifdef RTC_ON_SEPARATE_PIN    
                    |  SCB_STARTERP0_ER(RTCINT_PIO)
#endif
                    |  SCB_STARTERP0_ER(KBD_PIO_ON);
    else
      SCB_STARTERP0 |= SCB_STARTERP0_ER(KBD_PIO_ON)
#ifdef RTC_ON_SEPARATE_PIN    
                    |  SCB_STARTERP0_ER(RTCINT_PIO)
#endif
                    ;

    // Keyboard lines
    if ( wakeup_type & WT_KEYS ) {
      NVIC_EnableIRQ(WAKEUP_KBD_H0);
      NVIC_EnableIRQ(WAKEUP_KBD_H1);
      NVIC_EnableIRQ(WAKEUP_KBD_H2);
      NVIC_EnableIRQ(WAKEUP_KBD_H3);
    }
#ifdef RTC_ON_SEPARATE_PIN    
    NVIC_EnableIRQ(WAKEUP_RTCINT);
#endif    
    NVIC_EnableIRQ(WAKEUP_KBD_ON);

  }


  /* Configure system to run from WDT and set TMR32B0 for wakeup */
  if (wakeup_dSec)
  {

    // Disable timers
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_DISABLED;
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_DISABLED;
    TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_DISABLED;
    TMR_TMR32B1TCR = TMR_TMR32B1TCR_COUNTERENABLE_DISABLED;

#define WDT_BY_16B0_MAT1_PIN_0_9
#ifdef WDT_BY_16B0_MAT1_PIN_0_9 // Original hardware

    // Disable internal pullup on 0.9
    gpioSetPullup(&IOCON_PIO0_9, gpioPullupMode_Inactive);
    //gpioSetPullup(&IOCON_PIO0_9, gpioPullupMode_PullDown);
    gpioSetDir(0,9,0);
    gpioSetValue(0,9,0);

    /* Enable the clock for CT16B0 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT16B0);

    /* Configure 0.9 as Timer0_16 MAT1 */
    IOCON_PIO0_9 &= ~IOCON_PIO0_9_FUNC_MASK;
    IOCON_PIO0_9 |=  IOCON_PIO0_9_FUNC_CT16B0_MAT1;

    /* Set appropriate timer delay */
    TMR_TMR16B0MR0 = (PMU_WDTCLOCKSPEED_HZ* wakeup_dSec)/10 ;
    TMR_TMR16B0TC = 0;


    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR16B0MCR = TMR_TMR16B0MCR_MR0_INT_ENABLED
                   | TMR_TMR16B0MCR_MR0_RESET_ENABLED
                   ;

    /* Configure external match register to set 0.9 high on match */
    TMR_TMR16B0EMR &= ~(0xFF<<4);                   // Clear EMR config bits
    TMR_TMR16B0EMR |= TMR_TMR16B0EMR_EMC1_HIGH;     // Set MR1 (0.9) high on match

    /* Enable wakeup interrupt (P0.1..11 and P1.0 can be used) */
    //NVIC_EnableIRQ(WAKEUP0_IRQn);    // P0.0  
    //NVIC_EnableIRQ(WAKEUP1_IRQn);    // P0.1  (CT32B0_MAT2)
    //NVIC_EnableIRQ(WAKEUP2_IRQn);    // P0.2  
    //NVIC_EnableIRQ(WAKEUP3_IRQn);    // P0.3  
    //NVIC_EnableIRQ(WAKEUP4_IRQn);    // P0.4  
    //NVIC_EnableIRQ(WAKEUP5_IRQn);    // P0.5  
    //NVIC_EnableIRQ(WAKEUP6_IRQn);    // P0.6
    //NVIC_EnableIRQ(WAKEUP7_IRQn);    // P0.7
    //NVIC_EnableIRQ(WAKEUP8_IRQn);    // P0.8
    NVIC_EnableIRQ(WAKEUP9_IRQn);      // P0.9  (CT16B0_MAT1)
    //NVIC_EnableIRQ(WAKEUP10_IRQn);   // P0.10  
    //NVIC_EnableIRQ(WAKEUP11_IRQn);   // P0.11 (CT32B0_MAT3)
    //NVIC_EnableIRQ(WAKEUP12_IRQn);   // P1.0

    // Use RISING EDGE for wakeup detection.
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO0_9;

    // Use FALLING EDGE for wakeup detection.
    //SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO0_1;

    /* Clear all wakeup sources */ 
    //SCB_STARTRSRP0CLR = SCB_STARTRSRP0CLR_MASK;

    /* Enable Port 0.9 as wakeup source. */
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO0_9;

    // Set system AHB clock
    SCB_SYSAHBCLKDIV = SCB_SYSAHBCLKDIV_DIV1;

    // Switch main clock to WDT output
    SCB_MAINCLKSEL = SCB_MAINCLKSEL_SOURCE_WDTOSC;
    SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;       // Update clock source
    SCB_MAINCLKUEN = SCB_MAINCLKUEN_DISABLE;      // Toggle update register once
    SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;

    // Wait until the clock is updated
    while (!(SCB_MAINCLKUEN & SCB_MAINCLKUEN_UPDATE));

    /* Start the timer */
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_ENABLED;
#endif


  }
 
  /* Clear all wakeup sources */
  SCB_STARTRSRP0CLR = SCB_STARTRSRP0CLR_MASK;
 
  // Send Wait For Interrupt command
  {
    volatile int w = wakeup_int_cnt;
    //dbg_send(0xeec);
      pmu_not_slept = 0;
    __enable_irq();
    __asm volatile ("WFI");
    __asm volatile ("NOP");
    if ( w == wakeup_int_cnt ) { // WFI not slept bug
      //dbg_send(0xeee);
      //__asm volatile ("WFI");
      // Reconfigure system clock/PLL
      pmu_not_slept = 1;
      cpuSetClock(last_clock);
      wakeup_setup();
      //dbg_send(0xe0e);
    }
    //dbg_send(0xe0f);
  }
  return;
}


void sleep_ds(uint32_t val, uint8_t wakeup_type)
{
  for(int i=100; i--; ) { // Safety limit
    pmuTmr0DeepSleep(val, wakeup_type);
    if (pmu_not_slept == 0) break;
  }
}



/**************************************************************************/
/*! 
    @brief Puts select peripherals in sleep mode.

    This function will put the device into sleep mode.  Most gpio pins
    can be used to wake the device up, but the pins must first be
    configured for this in pmuInit.

    @section Example
    @code 
    // Configure wakeup sources before going into sleep/deep-sleep.
    // By default, pin 0.1 is configured as wakeup source (falling edge)
    pmuInit();
  
    // Enter sleep mode
    -pmuSleep();
    @endcode
*/
/**************************************************************************/
void pmuSleep()
{
  uint32_t regVal;
  /*
  3.9.2.2 Programming Sleep mode
    The following steps must be performed to enter Sleep mode:
      1. The DPDEN bit in the PCON register must be set to zero (Table 50).
      2. The SLEEPDEEP bit in the ARM Cortex-M0 SCR register must be set to zero, see
         (Table 453).
      3. Use the ARM Cortex-M0 Wait-For-Interrupt (WFI) instruction.
  */

  // 1.
  regVal = PMU_PMUCTRL;
  regVal |= (0x1<<8);
  regVal &= ~(
              (PMU_PMUCTRL_DPDEN_SLEEP) |
              (PMU_PMUCTRL_DPDFLAG));
  PMU_PMUCTRL = regVal;

  // 2.
  SCB_SCR &= ~SCB_SCR_SLEEPDEEP;

  // ??
  SCB_PDAWAKECFG = SCB_PDRUNCFG;

  // 3.
  __asm volatile ("WFI");
  return;
}



/**************************************************************************/
/*! 
    @brief  Turns off select peripherals and puts the device in deep-sleep
            mode.

    The device can be configured to wakeup from deep-sleep mode after a
    specified delay by supplying a non-zero value to the wakeupSeconds
    parameter.  This will configure CT32B0 to toggle pin 0.1 (CT32B0_MAT2)
    after x seconds, waking the device up.  The timer will be configured
    to run off the WDT OSC while in deep-sleep mode, meaning that WDTOSC
    should not be powered off (using the sleepCtrl parameter) when a
    wakeup delay is specified.

    The sleepCtrl parameter is used to indicate which peripherals should
    be put in sleep mode (see the SCB_PDSLEEPCFG register for details).
    
    @param[in]  sleepCtrl  
                The bits to set in the SCB_PDSLEEPCFG register.  This
                controls which peripherals will be put in sleep mode.
    @param[in]  wakeupSeconds
                The number of seconds to wait until the device will
                wakeup.  If you do not wish to wakeup after a specific
                delay, enter a value of 0.

    @code 
    uint32_t pmuRegVal;
  
    // Configure wakeup sources before going into sleep/deep-sleep
    // By default, pin 0.1 is configured as wakeup source
    pmuInit();
  
    // Put peripherals into sleep mode
    pmuRegVal = SCB_PDSLEEPCFG_IRCOUT_PD |
                SCB_PDSLEEPCFG_IRC_PD |
                SCB_PDSLEEPCFG_FLASH_PD |
                SCB_PDSLEEPCFG_BOD_PD |
                SCB_PDSLEEPCFG_ADC_PD |
                SCB_PDSLEEPCFG_SYSPLL_PD |
                SCB_PDSLEEPCFG_SYSOSC_PD;
  
    // If the wakeup timer is not used, WDTOSC can also be stopped (saves ~2uA)
    // pmuRegVal |= SCB_PDSLEEPCFG_WDTOSC_PD;

    // Enter deep sleep mode (wakeup after 5 seconds)
    pmuDeepSleep(pmuRegVal, 5);
    @endcode
*/
/**************************************************************************/
#if 0
void pmuDeepSleep(uint32_t sleepCtrl, uint32_t wakeupMSec)
{
  // Setup the board for deep sleep mode, shutting down certain
  // peripherals and remapping pins for lower power
  pmuSetupHW();

  SCB_PDAWAKECFG = SCB_PDRUNCFG;
  sleepCtrl &= ~(1 << 9);               // MAIN_REGUL_PD
  sleepCtrl |= (1 << 11) | (1 << 12);   // LP_REGUL_PD
  SCB_PDSLEEPCFG = sleepCtrl;
  SCB_SCR |= SCB_SCR_SLEEPDEEP;

  /* Configure system to run from WDT and set TMR32B0 for wakeup          */
  if (wakeupMSec > 0)
  {
    // Make sure WDTOSC isn't disabled in PDSLEEPCFG
    SCB_PDSLEEPCFG &= ~(SCB_PDSLEEPCFG_WDTOSC_PD);

    // Disable 32-bit timer 0 if currently in use
    TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_DISABLED;

    // Disable internal pullup on 0.1
    gpioSetPullup(&IOCON_PIO0_1, gpioPullupMode_Inactive);

    /* Enable the clock for CT32B0 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT32B0);

    /* Configure 0.1 as Timer0_32 MAT2 */
    IOCON_PIO0_1 &= ~IOCON_PIO0_1_FUNC_MASK;
    IOCON_PIO0_1 |= IOCON_PIO0_1_FUNC_CT32B0_MAT2;

    /* Set appropriate timer delay */
    TMR_TMR32B0MR0 = ((PMU_WDTCLOCKSPEED_HZ/10) * wakeupMSec)/100;
  
    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR32B0MCR |= (TMR_TMR32B0MCR_MR0_INT_ENABLED | TMR_TMR32B0MCR_MR0_RESET_ENABLED);
  
    /* Configure external match register to set 0.1 high on match */
    TMR_TMR32B0EMR &= ~(0xFF<<4);                   // Clear EMR config bits
    TMR_TMR32B0EMR |= TMR_TMR32B0EMR_EMC2_HIGH;     // Set MR2 (0.1) high on match
                
    /* Enable wakeup interrupt (P0.1..11 and P1.0 can be used) */ 
    //NVIC_EnableIRQ(WAKEUP0_IRQn);    // P0.0  
    NVIC_EnableIRQ(WAKEUP1_IRQn);    // P0.1  (CT32B0_MAT2)  
    //NVIC_EnableIRQ(WAKEUP2_IRQn);    // P0.2  
    //NVIC_EnableIRQ(WAKEUP3_IRQn);    // P0.3  
    //NVIC_EnableIRQ(WAKEUP4_IRQn);    // P0.4  
    //NVIC_EnableIRQ(WAKEUP5_IRQn);    // P0.5  
    NVIC_EnableIRQ(WAKEUP6_IRQn);    // P0.6  
    NVIC_EnableIRQ(WAKEUP7_IRQn);    // P0.7  
    //NVIC_EnableIRQ(WAKEUP8_IRQn);    // P0.8  
    //NVIC_EnableIRQ(WAKEUP9_IRQn);    // P0.9  
    //NVIC_EnableIRQ(WAKEUP10_IRQn);   // P0.10  
    NVIC_EnableIRQ(WAKEUP11_IRQn);   // P0.11 (CT32B0_MAT3)  
    NVIC_EnableIRQ(WAKEUP12_IRQn);   // P1.0
                
    // Use RISING EDGE for wakeup detection. 
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO0_1;
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO0_6;
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO0_7;
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO0_11;
    SCB_STARTAPRP0 |= SCB_STARTAPRP0_APRPIO1_0;

    /*
    // Use FALLING EDGE for wakeup detection. 
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO0_1;
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO0_6;
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO0_7;
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO0_11;
    SCB_STARTAPRP0 &= ~SCB_STARTAPRP0_APRPIO1_0;
    */

    /* Clear all wakeup sources */ 
    SCB_STARTRSRP0CLR = SCB_STARTRSRP0CLR_MASK;

    /* Enable Port 0.1 as wakeup source. */
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO0_1;
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO0_6;
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO0_7;
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO0_11;
    SCB_STARTERP0 |= SCB_STARTERP0_ERPIO1_0;

    // Reconfigure clock to run from WDTOSC
    pmuWDTClockInit();
        
    /* Start the timer */
    TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_ENABLED;
  }

  // Send Wait For Interrupt command
  __asm volatile ("WFI");
  return;
}
#endif

/**************************************************************************/
/*! 
    @brief Puts the device in deep power-down mode.

    This function will configure the PMU control register and enter
    deep power-down mode.  Pre-determined values are stored in the four
    general-purpose registers (PMU_GPREG0..3), which can be used to persist
    any essential system settings while the device is in deep power-down
    mode, so long as 3.3V is still available.

    @warning    The only way to wake a device up from deep power-down mode
                is to set a low-level on P1.4.  If 3.3V power is lost, the
                values stored in the four general-purpose registers will
                also be lost.

    @section Example

    @code 
    #include "core/cpu/cpu.h"
    #include "core/pmu/pmu.h"

    int main(void)
    {
      cpuInit();
      pmuInit();

      // Enter power-down mode
      pmuPowerDown();

      while(1)
      {
        // Device was woken up by WAKEUP pin
      }
    }
    @endcode
*/
/**************************************************************************/
void pmuPowerDown( void )
{
  uint32_t regVal;

  // Make sure HW and external devices are in low power mode
  pmuSetupHW();

  if ( (PMU_PMUCTRL & ((0x1<<8) | (PMU_PMUCTRL_DPDFLAG))) != 0x0 )
  {
    /* Check sleep and deep power down bits. If sleep and/or
       deep power down mode are entered, clear the PCON bits. */
    regVal = PMU_PMUCTRL;
    regVal |= ((0x1<<8) | 
               (PMU_PMUCTRL_DPDEN_SLEEP) |
               (PMU_PMUCTRL_DPDFLAG));
    PMU_PMUCTRL = regVal;

    if ( (PMU_GPREG0 != 0x12345678)||(PMU_GPREG1 != 0x87654321)
       ||(PMU_GPREG2 != 0x56781234)||(PMU_GPREG3 != 0x43218765) )
    {
      while (1);
    }
  }
  else
  {
    /* If in neither sleep nor deep-sleep mode, enter deep power down mode. */
    PMU_GPREG0 = 0x12345678;
    PMU_GPREG1 = 0x87654321;
    PMU_GPREG2 = 0x56781234;
    PMU_GPREG3 = 0x43218765;
    SCB_SCR |= SCB_SCR_SLEEPDEEP;
    PMU_PMUCTRL = PMU_PMUCTRL_DPDEN_DEEPPOWERDOWN;
    __asm volatile ("WFI");
  }
  return;
}

/**************************************************************************/
/*! 
    @brief  Configures parts and system peripherals to use lower power
            before entering sleep mode
*/
/**************************************************************************/
void pmuSetupHW(void)
{
  i2cUninit();
  uninit_keyboard_pins();
#ifdef CFG_RTC
// Could be used to wakeup - leave disabling to higher level
//  rtc_disable_int ();
#endif
  // ... on LCD
  gpioSetPullup(&IOCON_LCD_A0,   gpioPullupMode_Inactive); // A0
  gpioSetPullup(&IOCON_LCD_RST,  gpioPullupMode_Inactive); // RST
  gpioSetPullup(&IOCON_LCD_CS,   gpioPullupMode_Inactive); // CS
  gpioSetPullup(&IOCON_LCD_SCLK, gpioPullupMode_Inactive); // SCLK
  gpioSetPullup(&IOCON_LCD_SDAT, gpioPullupMode_Inactive); // SDAT
}

/**************************************************************************/
/*! 
    @brief  Restores parts and system peripherals to an appropriate
            state after waking up from deep-sleep mode
*/
/**************************************************************************/
void pmuRestoreHW(void)
{
  // init_keyboard();
  i2cInit(I2CMASTER);
}

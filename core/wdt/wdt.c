
#include "wdt.h"

#define WDT_FEED_VALUE       (0x003FFFFF)

#if 0

volatile uint32_t wdt_counter;

/**************************************************************************/
/*! 
    IRQ Handler when the watchdog times out.  Any actions that you wish
    to take when a timeout occurs should be called from here.
*/
/**************************************************************************/
void WDT_IRQHandler(void)
{
  /* Clear the time-out interrupt flag */
  WDT_WDMOD &= ~WDT_WDMOD_WDTOF;
  wdt_counter++;
}

/**************************************************************************/
/*! 
    Setup the clock for the watchdog timer.  The default setting is 250kHz.
*/
/**************************************************************************/
static void wdtClockSetup (void)
{
  /* Watchdog Configuration */
  /* Freq. = 0.5MHz, div = 2: WDT_OSC = 250kHz  */
  SCB_WDTOSCCTRL = SCB_WDTOSCCTRL_FREQSEL_0_5MHZ | 
                   SCB_WDTOSCCTRL_DIVSEL_DIV2;
  
  /* Set clock source (use WDT oscillator) */
  SCB_WDTCLKSEL = SCB_WDTCLKSEL_SOURCE_WATCHDOGOSC;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_UPDATE;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_DISABLE;
  SCB_WDTCLKUEN = SCB_WDTCLKUEN_UPDATE;

  /* Wait until updated */
  while (!(SCB_WDTCLKUEN & SCB_WDTCLKUEN_UPDATE));

  /* Set divider */
  SCB_WDTCLKDIV = SCB_WDTCLKDIV_DIV1;

  /* Enable WDT clock */
  SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_WDTOSC);
}

/**************************************************************************/
/*! 
    Initialises the watchdog timer and sets up the interrupt.
*/
/**************************************************************************/
void wdtInit (void)
{
  /* Setup the WDT clock */
  wdtClockSetup();

  /* Enable AHB clock to the WDT domain. */
  SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_WDT;

  wdt_counter = 0;

  /* Enable the WDT interrupt */
  NVIC_EnableIRQ(WDT_IRQn);

  /* Set timeout value (must be at least 0x000000FF) */
  WDT_WDTC = WDT_FEED_VALUE;

  /* Enable the watchdog timer (without system reset) */
  WDT_WDMOD = WDT_WDMOD_WDEN_ENABLED |
              WDT_WDMOD_WDRESET_DISABLED;
}

#endif

/**************************************************************************/
/*! 
    Feeds the watchdog to keep it from timing out.  Interrupts will be
    disabled while feeding the watchdog.
*/
/**************************************************************************/
void wdtFeed (void)
{
  /* Pet the watchdog */
  __disable_irq();
  WDT_WDFEED = WDT_WDFEED_FEED1;
  WDT_WDFEED = WDT_WDFEED_FEED2;
  __enable_irq();
}



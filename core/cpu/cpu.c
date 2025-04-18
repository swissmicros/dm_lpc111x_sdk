
#include "cpu.h"
#include "core/gpio/gpio.h"
#include "core/uart/uart.h"
#include "core/systick/systick.h"
#include "sysinit.h"

/**************************************************************************/
/*! 
    @brief Configures the PLL and main system clock

    The speed at which the MCU operates is set here using the SCB_PLLCTRL
    register, and the SCB_PLLCLKSEL register can be used to select which
    oscillator to use to generate the system clocks (the internal 12MHz
    oscillator or an external crystal).
*/
/**************************************************************************/
void __cpuPllSetup (cpuMultiplier_t multiplier)
{
#if CFG_CPU_CCLK == 12000000
#define USE_INTOSC   //use internal osc at 12MHz
#endif

#ifndef USE_INTOSC
  /*
  uint32_t i;

  // Power up system oscillator
  SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_SYSOSC_MASK);

  // Setup the crystal input (bypass disabled, 1-20MHz crystal)
  SCB_SYSOSCCTRL = (SCB_SYSOSCCTRL_BYPASS_DISABLED | SCB_SYSOSCCTRL_FREQRANGE_1TO20MHZ);

  for (i = 0; i < 200; i++)
  {
    __asm volatile ("NOP");
  }
  */

  // Configure PLL
//SCB_PLLCLKSEL = SCB_CLKSEL_SOURCE_MAINOSC;    // Use the external crystal
  SCB_PLLCLKSEL = SCB_CLKSEL_SOURCE_INTERNALOSC;// Use the internal osc
  
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_UPDATE;         // Update clock source
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_DISABLE;        // Toggle update register once
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_UPDATE;         // Update clock source again
  
  // Wait until the clock is updated
  while (!(SCB_PLLCLKUEN & SCB_PLLCLKUEN_UPDATE));

  // Set clock speed
  switch (multiplier)
  {
    case CPU_MULTIPLIER_2:
      SCB_PLLCTRL = (SCB_PLLCTRL_MULT_2 | (1 << SCB_PLLCTRL_DIV_BIT));
      break;
    case CPU_MULTIPLIER_3:
      SCB_PLLCTRL = (SCB_PLLCTRL_MULT_3 | (1 << SCB_PLLCTRL_DIV_BIT));
      break;
    case CPU_MULTIPLIER_4:
      SCB_PLLCTRL = (SCB_PLLCTRL_MULT_4 | (1 << SCB_PLLCTRL_DIV_BIT));
      break;
    case CPU_MULTIPLIER_1:
    default:
      SCB_PLLCTRL = (SCB_PLLCTRL_MULT_1 | (1 << SCB_PLLCTRL_DIV_BIT));
      break;
  }

  // Enable system PLL
  SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_SYSPLL_MASK);

  // Wait for PLL to lock
  while (!(SCB_PLLSTAT & SCB_PLLSTAT_LOCK));
  
  // Setup main clock
  SCB_MAINCLKSEL = SCB_MAINCLKSEL_SOURCE_SYSPLLCLKOUT;
#else // USE_INTOSC
  SCB_MAINCLKSEL = SCB_CLKSEL_SOURCE_INTERNALOSC;
#endif
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;       // Update clock source
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_DISABLE;      // Toggle update register once
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;

  // Wait until the clock is updated
  while (!(SCB_MAINCLKUEN & SCB_MAINCLKUEN_UPDATE));

  // Set system AHB clock
  SCB_SYSAHBCLKDIV = SCB_SYSAHBCLKDIV_DIV1;

  // Enabled IOCON clock for I/O related peripherals
  SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_IOCON;
}


int last_clock = 77; // No clock configured at the beginning
int current_clock;
int timer32_cclk_30ms;
int timer16_cclk_inst;

/**************************************************************************/
/*! 
    @brief Configures the cpu's clock

    We are supporting two clock frequencies fast and slow. Indicated by the
    CFG_CPU_CCLK_SLOW and CFG_CPU_CCLK_FAST defines. The frequency could be
    changed using calculator command. CFG_CPU_CCLK_BASE indicates the frequency
    after startup 0 means CFG_CPU_CCLK_SLOW and 1 means CFG_CPU_CCLK_FAST.

*/
/**************************************************************************/
void cpuSetClock (int is_fast_clk)
{
  if (is_fast_clk) {
    // Using PLL for fast clock

    // Enable system PLL
    SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_SYSPLL_MASK);

    // Wait for PLL to lock
    while (!(SCB_PLLSTAT & SCB_PLLSTAT_LOCK));
  
    // Setup main clock
    SCB_MAINCLKSEL = SCB_MAINCLKSEL_SOURCE_SYSPLLCLKOUT;

  } else {

    // Using just internal osc for slow clock
    SCB_MAINCLKSEL = SCB_CLKSEL_SOURCE_INTERNALOSC;
  }

  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;  // Update clock source
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_DISABLE; // Toggle update register once
  SCB_MAINCLKUEN = SCB_MAINCLKUEN_UPDATE;

  // Wait until the clock is updated
  while (!(SCB_MAINCLKUEN & SCB_MAINCLKUEN_UPDATE));

  // Set system AHB clock
  SCB_SYSAHBCLKDIV = 1; // Divide by 1 == SCB_SYSAHBCLKDIV_DIV1;

  // Enabled IOCON clock for I/O related peripherals
  SCB_SYSAHBCLKCTRL |= SCB_SYSAHBCLKCTRL_IOCON;

  if (!is_fast_clk) {
    // Disable system PLL for slow mode
    SCB_PDRUNCFG |= SCB_PDRUNCFG_SYSPLL_MASK;
  }
  
  // Clock changed?
  if (is_fast_clk != last_clock) {

    // Store new clock value
    last_clock = is_fast_clk;
    current_clock = (last_clock)?CFG_CPU_CCLK_FAST:CFG_CPU_CCLK_SLOW;
    current_clock /= SCB_SYSAHBCLKDIV;

    // Recalculate clock dependent values
    timer32_cclk_30ms = (current_clock/1000)*30;
    timer16_cclk_inst =  current_clock/6400; // 6400 - instructions per second

    // Reconfigure clock dependent peripherals
    systemClkChgInit();
  }

}



/**************************************************************************/
/*! 
    @brief Initialises the CPU, setting up the PLL, etc.
*/
/**************************************************************************/
void cpuInit (void)
{
  gpioInit();

  // Set all GPIO pins to input by default
  GPIO_GPIO0DIR &= ~(GPIO_IO_ALL);
  GPIO_GPIO1DIR &= ~(GPIO_IO_ALL);
  GPIO_GPIO2DIR &= ~(GPIO_IO_ALL);
  GPIO_GPIO3DIR &= ~(GPIO_IO_ALL);

  // Configure PLL
  SCB_PLLCLKSEL = SCB_CLKSEL_SOURCE_INTERNALOSC;// Use the internal osc
  
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_UPDATE;         // Update clock source
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_DISABLE;        // Toggle update register once
  SCB_PLLCLKUEN = SCB_PLLCLKUEN_UPDATE;         // Update clock source again
  
  // Wait until the clock is updated
  while (!(SCB_PLLCLKUEN & SCB_PLLCLKUEN_UPDATE));

  // Set multiplier for fast clock CFG_CPU_CCLK_FAST
  SCB_PLLCTRL = (SCB_PLLCTRL_MULT_4 | (1 << SCB_PLLCTRL_DIV_BIT));

  // Configure PLL and main system clock
  cpuSetClock(CFG_CPU_CCLK_BASE);
}

/**************************************************************************/
/*! 
    @brief Get's the CPU Device ID
*/
/**************************************************************************/
uint32_t cpuGetDeviceID (void)
{
  return SCB_DEVICEID;
}

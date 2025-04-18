#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"

#include "core/pmu/pmu.h"
#include "core/gpio/gpio.h"

#include "drivers/keyboard/keyboard.h"

/**************************************************************************/
/*! 
    Puts the device into deep sleep
*/
/**************************************************************************/
void cmd_deepsleep(uint8_t argc, char **argv)
{
  // ToDo: Some peripherals may need to be manually disabled for the
  // lowest possible power consumption in deep sleep mode

  // Put peripherals into sleep mode
  uint32_t pmuRegVal;
  pmuRegVal = SCB_PDSLEEPCFG_IRCOUT_PD |
              SCB_PDSLEEPCFG_IRC_PD |
              SCB_PDSLEEPCFG_FLASH_PD |
              SCB_PDSLEEPCFG_BOD_PD |
              SCB_PDSLEEPCFG_ADC_PD |
              SCB_PDSLEEPCFG_SYSPLL_PD |
              SCB_PDSLEEPCFG_SYSOSC_PD;

  // If the wakeup timer is not used, WDTOSC can also be stopped (saves ~2uA)
  // pmuRegVal |= SCB_PDSLEEPCFG_WDTOSC_PD;

  // Enter deep sleep mode (wakeup after ~10 seconds)
  // Note that the exact delay is variable since the internal WDT oscillator
  // is used for lowest possible power consumption and because it requires
  // no external components, but it only has +-/40% accuracy
  printf("Entering Deep Sleep mode for 3 seconds%s", CFG_PRINTF_NEWLINE);
  pmuDeepSleep(pmuRegVal, 3);
  // On wakeup, the WAKEUP interrupt will be fired, which is handled
  // by WAKEUP_IRQHandler in 'core/pmu/pmu.c'.  This will set the CPU
  // back to an appropriate state, and execution will be returned to
  // the point that it left off before deep sleep mode was entered.
  printf("Woke up from deep sleep");
}


/**************************************************************************/
/*! 
    Puts the device into deep sleep
*/
/**************************************************************************/
void cmd_deepsleep2(uint8_t argc, char **argv)
{
  uint32_t pmuRegVal;
  pmuRegVal = SCB_PDSLEEPCFG_IRCOUT_PD |
              SCB_PDSLEEPCFG_IRC_PD |
              SCB_PDSLEEPCFG_FLASH_PD |
              SCB_PDSLEEPCFG_BOD_PD |
              SCB_PDSLEEPCFG_ADC_PD |
              SCB_PDSLEEPCFG_SYSPLL_PD |
              SCB_PDSLEEPCFG_WDTOSC_PD |
              SCB_PDSLEEPCFG_SYSOSC_PD;
                                                        
        gpioSetPullup(&IOCON_PIO0_6, gpioPullupMode_PullDown);
        gpioSetPullup(&IOCON_PIO0_7, gpioPullupMode_PullDown);
        gpioSetPullup(&IOCON_JTAG_TDI_PIO0_11, gpioPullupMode_PullDown);
        gpioSetPullup(&IOCON_JTAG_TMS_PIO1_0, gpioPullupMode_PullDown);
        

        // set all horizontal pins to output
        gpioSetDir(KEYBOARD_H0_PORT, KEYBOARD_H0_PIN, 1);
        gpioSetDir(KEYBOARD_H1_PORT, KEYBOARD_H1_PIN, 1);
        gpioSetDir(KEYBOARD_H2_PORT, KEYBOARD_H2_PIN, 1);
        gpioSetDir(KEYBOARD_H3_PORT, KEYBOARD_H3_PIN, 1);
        // set all horizontal pins high
        gpioSetValue(KEYBOARD_H0_PORT, KEYBOARD_H0_PIN, 1);
        gpioSetValue(KEYBOARD_H1_PORT, KEYBOARD_H1_PIN, 1);
        gpioSetValue(KEYBOARD_H2_PORT, KEYBOARD_H2_PIN, 1);
        gpioSetValue(KEYBOARD_H3_PORT, KEYBOARD_H3_PIN, 1);
        
        printf("Entering Deep Sleep mode until a key is pressed ...%s", CFG_PRINTF_NEWLINE);
  pmuDeepSleep(pmuRegVal, 30);
  // On wakeup, the WAKEUP interrupt will be fired, which is handled
  // by WAKEUP_IRQHandler in 'core/pmu/pmu.c'.  This will set the CPU
  // back to an appropriate state, and execution will be returned to
  // the point that it left off before deep sleep mode was entered.
  printf("\n\rWoke up from deep sleep after a key was pressed");
}

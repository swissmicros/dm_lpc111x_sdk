#include <stdio.h>
#include <stdlib.h>

#include "sysinit.h"

#include "core/cpu/cpu.h"
#include "core/pmu/pmu.h"

#ifdef CFG_PRINTF_UART
  #include "core/uart/uart.h"
#endif

#ifdef CFG_INTERFACE
  #include "core/cmd/cmd.h"
#endif

#ifdef CFG_ST7565
  #include "drivers/lcd/bitmap/st7565/st7565.h"
#endif

#include "core/timer32/timer32.h"
#include "core/wdt/wdt.h"

/**************************************************************************/
/*! 
    Configures the core system clock and sets up any mandatory
    peripherals like the systick timer, UART for printf, etc.

    This function should set the HW to the default state you wish to be
    in coming out of reset/startup, such as disabling or enabling LEDs,
    setting specific pin states, etc.
*/
/**************************************************************************/
void systemInit()
{
  cpuInit();
  gpioInit();
  pmuInit();

/* Unit usage:

   timer32_0: 
   timer32_1: pwm out (speaker)
   timer16_0: sleep wakeup
   timer16_1: 
   
   WDT: used for wakeup from sleep

*/  
  // pmuWDTClockInit();
  // timer32Init(0, TIMER32_CCLK_250MS);
  // timer32Enable(0);
  // timer32Init(1, TIMER32_CCLK_66MS);
  // timer32Enable(1);

  // Initialise the ST7565 132x16 pixel display
  #ifdef CFG_ST7565
    st7565Init();
  #endif

  // Start the command line interface (if requested)
  #ifdef CFG_INTERFACE
    //printf("%sType '?' for a list of available commands%s", CFG_PRINTF_NEWLINE, CFG_PRINTF_NEWLINE);
    cmdInit();
  #endif
  
  systemClkChgInit();
}

/**************************************************************************/
/*! 
    Configures devices after clock change.
*/
/**************************************************************************/
void systemClkChgInit()
{
  systickInit((current_clock / 1000) * CFG_SYSTICK_DELAY_IN_MS);

  #ifdef CFG_PRINTF_UART
    // Initialise UART with the default baud rate (set in projectconfig.h)
    uartInit(CFG_UART_BAUDRATE);
  #endif

  #ifdef CFG_RTC
    i2cInit(I2CMASTER);
    pcf8563_init();
  #endif
}





/**************************************************************************/
/*! 
    @brief Sends a single byte to a pre-determined peripheral (UART, etc.).

    @param[in]  byte
                Byte value to send
*/
/**************************************************************************/
void __putchar(const char c) 
{
  #ifdef CFG_PRINTF_UART
    // Send output to UART
    uartSendByte(c);
  #endif
}

/**************************************************************************/
/*! 
    @brief Sends a string to a pre-determined end point (UART, etc.).

    @param[in]  str
                Text to send

    @note This function is only called when using the GCC-compiler
          in Codelite or running the Makefile manually.  This function
          will not be called when using the C library in Crossworks for
          ARM.
*/
/**************************************************************************/
int puts(const char * str)
{
  while(*str) __putchar(*str++);
  return 0;
}


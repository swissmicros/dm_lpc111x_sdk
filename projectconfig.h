#ifndef _PROJECTCONFIG_H_
#define _PROJECTCONFIG_H_

#include "lpc111x.h"
#include "sysdefs.h"

/**************************************************************************

                PORT 0    PORT 1    PORT 2               PORT 3
                =======   ======    =================    ======
                3 11      2 8 9     4 5 6 7 8 9 10 11    1 2 3

    ST7565      . .       . . .     X X X X X X .  .     . . .

 **************************************************************************/




/*=========================================================================
    CORE CPU SETTINGS
    -----------------------------------------------------------------------

    CFG_CPU_CCLK    Value is for reference only.  'core/cpu/cpu.c' must
                    be modified to change the clock speed, but the value
                    should be indicated here since CFG_CPU_CCLK is used by
                    other peripherals to determine timing.

    Note:           At 36MHz 1 tick = ~27.777ns or 0.02777us
    -----------------------------------------------------------------------*/
//              #define CFG_CPU_CCLK              (12000000)
//              #define CFG_CPU_CCLK              (48000000)

//
// We are supporting two clock frequencies fast and slow. Indicated by the
// CFG_CPU_CCLK_SLOW and CFG_CPU_CCLK_FAST defines. The frequency could be
// changed using calculator command. CFG_CPU_CCLK_BASE indicates the frequency
// after startup 0 means CFG_CPU_CCLK_SLOW and 1 means CFG_CPU_CCLK_FAST.
//
//              #define CFG_CPU_CCLK_BASE         0
                #define CFG_CPU_CCLK_BASE         1
                #define CFG_CPU_CCLK_SLOW         (12000000)
                #define CFG_CPU_CCLK_FAST         (48000000)

/*=========================================================================*/

/*=========================================================================
    SYSTICK TIMER
    -----------------------------------------------------------------------

    CFG_SYSTICK_DELAY_IN_MS   The number of milliseconds between each tick
                              of the systick timer.
                                                          
    -----------------------------------------------------------------------*/
                #define CFG_SYSTICK_DELAY_IN_MS     (1)
/*=========================================================================*/


/*=========================================================================
    UART
    -----------------------------------------------------------------------

    CFG_UART_BAUDRATE         The default UART speed.  This value is used 
                              when initialising UART, and should be a 
                              standard value like 57600, 9600, etc.
    CFG_UART_BUFSIZE          The length in bytes of the UART RX FIFO. This
                              will determine the maximum number of received
                              characters to store in memory.

    -----------------------------------------------------------------------*/
                //#define CFG_UART_BAUDRATE           921600    // Works only with 48MHz clock
                //#define CFG_UART_BAUDRATE           576000    // Works only with 48MHz clock
                //#define CFG_UART_BAUDRATE           500000    // Works only with 48MHz clock
                //#define CFG_UART_BAUDRATE           230400
                //#define CFG_UART_BAUDRATE           115200
                //#define CFG_UART_BAUDRATE            57600
                #define CFG_UART_BAUDRATE            38400
                //#define CFG_UART_BAUDRATE            19200
                //#define CFG_UART_BAUDRATE             9600
                #define CFG_UART_BUFSIZE             (128)

/*=========================================================================*/



/*=========================================================================
    PRINTF REDIRECTION
    -----------------------------------------------------------------------

    CFG_PRINTF_UART           Will cause all printf statements to be 
                              redirected to UART
    CFG_PRINTF_NEWLINE        This should be either "\r\n" for Windows or
                              "\n" for *nix

    Note: If no printf redirection definitions are present, all printf
    output will be ignored, though this will also save ~350 bytes flash.

    NOTE: PRINTF Support =    ~350 bytes Flash (-Os)
    -----------------------------------------------------------------------*/
                #define CFG_PRINTF_UART
                #define CFG_PRINTF_NEWLINE          "\r\n"

/*=========================================================================*/


/*=========================================================================
    COMMAND LINE INTERFACE
    -----------------------------------------------------------------------

    CFG_INTERFACE             If this field is defined the UART or USBCDC
                              based command-line interface will be included
    CFG_INTERFACE_MAXMSGSIZE  The maximum number of bytes to accept for an
                              incoming command
    CFG_INTERFACE_PROMPT      The command prompt to display at the start
                              of every new data entry line

    NOTE:                     The command-line interface will use either
                              USB-CDC or UART depending on whether
                              CFG_PRINTF_UART or CFG_PRINTF_USBCDC are 
                              selected.
    -----------------------------------------------------------------------*/
                #define CFG_INTERFACE
                #define CFG_INTERFACE_MAXMSGSIZE    (256)
                #define CFG_INTERFACE_PROMPT        "DM_SDK> "

/*=========================================================================*/


/*=========================================================================
    Defines whether to add RTC code.
    Should be turned ON if the hw contains RTC chip.
    -----------------------------------------------------------------------*/
        #define CFG_RTC
/*=========================================================================*/


/*=========================================================================
    132x16 Graphic LCDs
    -----------------------------------------------------------------------

    CFG_ST7565                If defined, this will cause drivers for
                              the 132x16 pixel ST7565 LCD to be included
    DEPENDENCIES:             ST7565 requires the use of pins 2.4-9.
    -----------------------------------------------------------------------*/
          #define CFG_ST7565

/*=========================================================================*/

/*=========================================================================
    VREF diode available
    -----------------------------------------------------------------------

    CFG_VREF                  If defined, this will add code for reading of 
                              battery voltage.
    CFG_REF_VOLTAGE           Reference voltage in mV.
    -----------------------------------------------------------------------*/
          #define CFG_VREF
          #define CFG_REF_VOLTAGE     1200
          #define CFG_LOWBAT_VOLTAGE  2000

/*=========================================================================*/


/*=========================================================================
    FIRMWARE VERSION SETTINGS
    -----------------------------------------------------------------------*/
    #define CFG_FIRMWARE_VERSION_MAJOR            "DM_SDK"
    #define CFG_FIRMWARE_VERSION_MINOR            ""
    #define CFG_FIRMWARE_VERSION_REVISION         "V03"
    
#define CFG_FIRMWARE_VERSION CFG_FIRMWARE_VERSION_MAJOR CFG_FIRMWARE_VERSION_MINOR

/*=========================================================================*/

#endif

#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"       // Generic helper functions
#include "core/systick/systick.h"
#include "core/gpio/gpio.h"
#include "core/iap/iap.h"
#include "core/wdt/wdt.h"

#include "core/cpu/cpu.h"

/**************************************************************************/
/*! 
    'sysinfo' command handler
*/
/**************************************************************************/
void cmd_sysinfo(uint8_t argc, char **argv)
{
  printf("%-25s : %d.%d MHz %s", "System Clock", current_clock / 1000000, current_clock % 1000000, CFG_PRINTF_NEWLINE);
  //printf("%-25s : %s_%s %s", "Firmware", CFG_FIRMWARE_VERSION, CFG_FIRMWARE_VERSION_REVISION, CFG_PRINTF_NEWLINE);

  // 128-bit MCU Serial Number
  IAP_return_t iap_return;
  iap_return = iapReadSerialNumber();
  if(iap_return.ReturnCode == 0)
  {
   printf("%-25s : %08X %08X %08X %08X %s", "Serial Number",
          iap_return.Result[0],iap_return.Result[1],
          iap_return.Result[2],iap_return.Result[3], CFG_PRINTF_NEWLINE);
  }

  // CLI and buffer Settings
  #ifdef CFG_INTERFACE
    printf("%-25s : %d bytes %s", "Max CLI Command", CFG_INTERFACE_MAXMSGSIZE, CFG_PRINTF_NEWLINE);
  #endif

  // System Uptime (based on systick timer)
  printf("%-25s : %u s %s", "System Uptime", (unsigned int)systickGetSecondsActive(), CFG_PRINTF_NEWLINE);
  //printf("wdt_counter=%i\n\r",(int)wdt_counter);
}

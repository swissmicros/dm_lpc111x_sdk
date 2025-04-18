#ifndef __CMD_TBL_H__ 
#define __CMD_TBL_H__

#define CMD_COUNT (sizeof(cmd_tbl)/sizeof(cmd_t))

#include <stdio.h>

#ifdef CFG_INTERFACE_UART
  #include "core/uart/uart.h"
#endif

// Function prototypes for the command table
void cmd_help(uint8_t argc, char **argv);         // handled by core/cmd/cmd.c
void cmd_sysinfo(uint8_t argc, char **argv);

#ifdef CFG_ST7565
//void cmd_lcd_init(uint8_t argc, char **argv);
//void cmd_lcd_clear(uint8_t argc, char **argv);
//void cmd_lcd_write(uint8_t argc, char **argv);
#endif


#ifdef CFG_VREF
void cmd_battery_read(uint8_t argc, char **argv);
#endif

#ifdef CFG_RTC
void cmd_get_time(uint8_t argc, char **argv);
void cmd_set_time(uint8_t argc, char **argv);
void cmd_update_time(uint8_t argc, char **argv);
#endif


//void cmd_deepsleep(uint8_t argc, char **argv);
//void cmd_deepsleep2(uint8_t argc, char **argv);
//void cmd_powerdown(uint8_t argc, char **argv);


/**************************************************************************/
/*! 
    Command list for the command-line interpreter and the name of the
    corresponding method that handles the command.

    Note that a trailing ',' is required on the last entry, which will
    cause a NULL entry to be appended to the end of the table.
*/
/**************************************************************************/
cmd_t cmd_tbl[] = 
{
  // command name, min args, function name, command description, syntax
  // min_args==0xff -> args will be not parsed and passed in argv[1]
  { "?",   0, cmd_help,         "Help",                             "No params" },
#ifdef CFG_RTC
  { "t",   0, cmd_get_time,     "Get time",                         "No params" },
  { "ts",  2, cmd_set_time,     "Set time",                         "<YYYYMMDD> <HHMMSS>" },
  { "td",  1, cmd_update_time,  "Update time",                      "<offset_in_minutes>" },
#endif  
#ifdef CFG_VREF
  { "b",   0, cmd_battery_read, "Read battery voltage",             "No params" },
#endif
  { "V",   0, cmd_sysinfo,      "System Info",                      "No params" },
#ifdef CFG_ST7565
//{ "i",   0, cmd_lcd_init,     "Init LCD",                         "No params" },
//{ "c",   0, cmd_lcd_clear,    "Clear LCD",                        "No params" },
//{ "w",   1, cmd_lcd_write,    "Write to LCD",                     "<msg>" },
#endif
};

#endif

#include <stdio.h>

#include "projectconfig.h"
#include "core/cmd/cmd.h"
#include "commands.h"
#include "drivers/lcd/bitmap/st7565/st7565.h"
#include "core/gpio/gpio.h"
/*

int volatile pow;

void cmd_lcd_clear(uint8_t argc, char **argv)
{
        st7565ClearScreen();
        st7565Refresh();
        st7565DrawPixel(1,1);
  printf("lcd cleared");
}

void cmd_lcd_init(uint8_t argc, char **argv)
{
        st7565Init();
  printf("lcd initialized");
}

void cmd_lcd_write(uint8_t argc, char **argv)
{
        st7565DrawString(1,16, argv[0], Font_System5x8);
        st7565Refresh();
}
*/

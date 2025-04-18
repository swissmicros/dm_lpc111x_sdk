#ifndef __COMMANDS_H__ 
#define __COMMANDS_H__

#include "projectconfig.h"

// Method Prototypes
int getNumber (char *s, int *result);

char* skip_spaces(char* s);
char* skip_chars(char* s);
char* getHex(char* s, int* result);

int char2hex(char c);
int char2dec(char c);

#ifdef CFG_LCD_TO_SERIAL
extern bool debug_lcd_output;
#endif

#ifdef CFG_REG_TO_SERIAL
extern bool debug_reg_output;
#endif

#ifdef CFG_PRINTF_NEWLINE
#define CR CFG_PRINTF_NEWLINE
#endif

#endif

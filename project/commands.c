
#include "core/cmd/cmd.h"
#include "commands.h"

#define CR CFG_PRINTF_NEWLINE

#ifdef CFG_LCD_TO_SERIAL
bool debug_lcd_output;
#endif

#ifdef CFG_REG_TO_SERIAL
bool debug_reg_output;
#endif


/**************************************************************************/
/*!
    @brief  Attempts to convert the supplied decimal or hexadecimal
          string to the matching 32-bit value.  All hexadecimal values
          must be preceded by either '0x' or '0X' to be properly parsed.

    @param[in]  s
                Input string
    @param[out] result
                Signed 32-bit integer to hold the conversion results

    @section Example

    @code
    char *hex = "0xABCD";
    char *dec = "1234";

    // Convert supplied values to integers
    int32_t hexValue, decValue;
    getNumber (hex, &hexValue);
    getNumber (dec, &decValue);

    @endcode
*/
/**************************************************************************/
int getNumber (char *s, int *result)
{
  int32_t value;
  bool isHex = 0;
  bool sgn   = 0;
  uint8_t base = 10;

  if (!s)
    return 0;

  // Check if this is a hexadecimal value
  if (*s=='0') s++;
  if (*s=='x') { isHex=1; s++; base=16; }
  if (*s=='-') { sgn=1; s++; }

  // Try to convert value
  for (value = 0; *s; s++)
  {
    if (*s>='0' && *s <='9') {
      value = value*base+(*s-'0');
    } else {
      char c = *s | ' '; // to lower
      if (isHex && c>='a' && c<='f')
        value = (value<<4)|(c-'W');
      else {
        printf ("Bad number." CR);
        return 0;
      }
    }
  }

  *result = sgn?-value:value;

  return 1;
}


char* skip_spaces(char* s) {
  while(*s==' ') s++;
  return s;
}


char* skip_chars(char* s) {
  while(*s && *s!=' ') s++;
  return s;
}


int char2dec(char c) {
  return (c>='0' && c<='9') ? (c-'0') : -1;
}


int char2hex(char c) {
  if (c>='0' && c<='9') {
    return c-'0';
  }else{
    c |= ' '; // to lower
    if (c>='a' && c<='f')
      return c-'W';
  }
  return -1;
}


char* getHex(char* s, int* result)
{
  int value = 0;
  int e;

  s=skip_spaces(s);
  for(; *s; s++) {
    e = char2hex(*s);
    if(e<0) break;
    value = (value<<4)|e;
  }
  *result = value;
  return s;
}


#include <string.h>

#include "st7565.h"

#include "core/gpio/gpio.h"
#include "core/systick/systick.h"
#include "drivers/lcd/smallfonts.h"


#define LCD_X 132
#define LCD_Y 16


void sendByte(uint8_t byte);

//#define CMD(c)        do { gpioSetValue( ST7565_A0_PORT, ST7565_A0_PIN, 0 ); sendByte( c ); } while (0);
#define DATA(d)       do { gpioSetValue( ST7565_A0_PORT, ST7565_A0_PIN, 1 ); sendByte( d ); } while (0);
#define DELAY(mS)     do { systickDelay( mS / CFG_SYSTICK_DELAY_IN_MS ); } while(0);

void CMD(uint8_t c) {
  gpioSetValue( ST7565_A0_PORT, ST7565_A0_PIN, 0 );
  sendByte( c );
}



uint8_t buffer[LCD_X*LCD_Y/8];



uint8_t lcd_brightness = 0;
uint8_t lcd_oper_voltage = 5;
//uint8_t last_oper_voltage = 0;

// Operating voltage range 0-7, Brightness range 0-3f
void set_lcd_params() {
//  if (last_oper_voltage != lcd_oper_voltage) {
//    last_oper_voltage = lcd_oper_voltage;
    CMD(ST7565_CMD_SET_RESISTOR_RATIO | lcd_oper_voltage);  // Set LCD operating voltage
    DELAY(10);                                              // Wait 10mS ???
//  }
  st7565SetBrightness(lcd_brightness);                    // Set LCD brightness
}



/**************************************************************************/
/* Private Methods                                                        */
/**************************************************************************/

/*
inline char ReverseByteBits(char value){
  return
    ((value&1)?0x80:0)|((value&2)?0x40:0)|((value&4)?0x20:0)|((value&8)?0x10:0)|
    ((value&0x10)?8:0)|((value&0x20)?4:0)|((value&0x40)?2:0)|((value&0x80)?1:0);
}
*/

/**************************************************************************/
/*! 
    @brief Renders the buffer contents

    @param[in]  buffer
                Pointer to the buffer containing the raw pixel data
*/
/**************************************************************************/
void writeBuffer(uint8_t *buffer) 
{
  uint8_t c, p;
  int pagemap[] = { 1, 0};    // normal orientation
  //int pagemap[] = { 0, 1};    // upside-down orientation
  int pp = LCD_X;

  //__disable_irq();
  for(p = 0; p < 2; p++, pp+=LCD_X) 
  {
    int cc = pp-1;
    CMD(ST7565_CMD_SET_PAGE | pagemap[p]);
    CMD(ST7565_CMD_SET_COLUMN_LOWER);
    CMD(ST7565_CMD_SET_COLUMN_UPPER);
    CMD(ST7565_CMD_RMW);
    
    for(c = 0; c < LCD_X; c++,cc--) 
    {
      DATA(buffer[(LCD_X*p)+c]);  //normal orientation
      //DATA(ReverseByteBits(buffer[cc]));  //upside-down orientation
    }
  }
  //__enable_irq();
}

/**************************************************************************/
/*! 
    @brief Simulates an SPI write using GPIO

    @param[in]  byte
                The byte to send
*/
/**************************************************************************/
void sendByte(uint8_t byte)
{
  int8_t i;

  // Note: This code can be optimised to avoid the branches by setting
  // GPIO registers directly, but we'll leave it as is for the moment
  // for simplicity sake

  // Make sure clock pin starts high
  gpioSetValue(ST7565_SCLK_PORT, ST7565_SCLK_PIN, 1);

  // Write from MSB to LSB
  for (i=7; i>=0; i--) 
  {
    // Set clock pin low
    gpioSetValue(ST7565_SCLK_PORT, ST7565_SCLK_PIN, 0);
    // Set data pin high or low depending on the value of the current bit
    gpioSetValue(ST7565_SDAT_PORT, ST7565_SDAT_PIN, byte & (1 << i) ? 1 : 0);
    // Set clock pin high
    gpioSetValue(ST7565_SCLK_PORT, ST7565_SCLK_PIN, 1);
  }
}


/**************************************************************************/
/* Public Methods                                                         */
/**************************************************************************/

void st7565TurnOff(void) {
  CMD(ST7565_CMD_SET_STATIC_OFF);
  CMD(ST7565_CMD_DISPLAY_OFF);
  CMD(ST7565_CMD_SET_ALLPTS_ON);
  //CMD(ST7565_CMD_SET_POWER_CONTROL);
}

#ifdef GPIO_BY_MACRO
#define out_and_high_M(port, pin) \
  gpioSetDir(port, pin, 1);     \
  gpioSetValue(port, pin, 1)

#else
#define out_and_high_M out_and_high
#endif

void out_and_high(uint32_t port, uint32_t pin) {
  gpioSetDir(port, pin, 1);
  gpioSetValue(port, pin, 1);
}

/**************************************************************************/
/*! 
    @brief Initialises the ST7565 LCD display
*/
/**************************************************************************/
void st7565Init(void)
{
  // Note: This can be optimised to set all pins to output and high
  // in two commands by manipulating the registers directly (assuming
  // that the pins are located in the same GPIO bank).  The code is left
  // as is for clarity sake in case the pins are not all located in the
  // same bank.

  // Configure VDD pin and set high
//  gpioSetDir(ST7565_VDD_PORT, ST7565_VDD_PIN, 1);
//  gpioSetValue(ST7565_VDD_PORT, ST7565_VDD_PIN, 1);

  //DELAY(500);               // Wait 500mS

  // Set clock pin to output and high
  out_and_high(ST7565_SCLK_PORT, ST7565_SCLK_PIN);

  // Set data pin to output and high
  out_and_high(ST7565_SDAT_PORT, ST7565_SDAT_PIN);

  // Configure A0 pin to output and set high
  out_and_high(ST7565_A0_PORT, ST7565_A0_PIN);

  // Configure Reset pin and set high
  out_and_high(ST7565_RST_PORT, ST7565_RST_PIN);

  // Configure select pin and set high
  out_and_high(ST7565_CS_PORT, ST7565_CS_PIN);

  //DELAY(500);               // Wait 500mS

  // Reset
  gpioSetValue(ST7565_CS_PORT, ST7565_CS_PIN, 0);     // Set CS low
  gpioSetValue(ST7565_RST_PORT, ST7565_RST_PIN, 0);   // Set reset low
  //DELAY(500);               // Wait 500mS
  gpioSetValue(ST7565_RST_PORT, ST7565_RST_PIN, 1);   // Set reset high

  //clear screen
  st7565ClearScreen();
  st7565Refresh();

  // Configure Display
  CMD(ST7565_CMD_SET_BIAS_9);                         // LCD Bias Select
  CMD(ST7565_CMD_SET_ADC_NORMAL);                     // ADC Select
  CMD(ST7565_CMD_SET_COM_NORMAL);                     // SHL Select


  st7565SetBooster(ST7565_CMD_SET_BOOSTER_234);
  DELAY(10);                                          // Wait 10mS
  CMD(ST7565_CMD_SET_DISP_START_LINE);                // Initial Display Line
  CMD(ST7565_CMD_SET_POWER_CONTROL | 0x04);           // Turn on voltage converter (VC=1, VR=0, VF=0)
  DELAY(50);                                          // Wait 50mS
  CMD(ST7565_CMD_SET_POWER_CONTROL | 0x06);           // Turn on voltage regulator (VC=1, VR=1, VF=0)
  DELAY(50);                                          // Wait 50mS
  CMD(ST7565_CMD_SET_POWER_CONTROL | 0x07);           // Turn on voltage follower
  DELAY(10);                                          // Wait 10mS
  //CMD(ST7565_CMD_SET_RESISTOR_RATIO | 0x5);           // Set LCD operating voltage
  set_lcd_params();

  // Turn display on
  CMD(ST7565_CMD_DISPLAY_ON);
  CMD(ST7565_CMD_SET_ALLPTS_NORMAL);
}

/**************************************************************************/
/*! 
    @brief Turns the Display ON
*/
/**************************************************************************/
void st7565TurnDisplayON(void)
{
  // Turn display on
  CMD(ST7565_CMD_DISPLAY_ON);
  CMD(ST7565_CMD_SET_ALLPTS_NORMAL);
}


/**************************************************************************/
/*! 
    @brief Turns the Display OFF
*/
/**************************************************************************/
void st7565TurnDisplayOFF(void)
{
  // Turn display off
  CMD(ST7565_CMD_DISPLAY_OFF);
}

/**************************************************************************/
/*! 
    @brief Sets the display booster
*/
/**************************************************************************/
void st7565SetBooster(uint8_t val)
{
  CMD(ST7565_CMD_SET_BOOSTER_FIRST);
  CMD(val);
}

/**************************************************************************/
/*! 
    @brief Sets the display brightness
*/
/**************************************************************************/
void st7565SetBrightness(uint8_t val)
{
  CMD(ST7565_CMD_SET_VOLUME_FIRST);
  CMD(ST7565_CMD_SET_VOLUME_SECOND | (val & 0x3f));
}
/**************************************************************************/
/*! 
    @brief Clears the screen
*/
/**************************************************************************/
void st7565ClearScreen(void) 
{
  memset(&buffer, 0x00, LCD_X*LCD_Y/8);
}

/**************************************************************************/
/*! 
    @brief Renders the contents of the pixel buffer on the LCD
*/
/**************************************************************************/
void st7565Refresh(void)
{
  writeBuffer(buffer);
}

/**************************************************************************/
/*! 
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0..131)
    @param[in]  y
                The y position (0..15)
*/
/**************************************************************************/
void st7565DrawPixel(uint8_t x, uint8_t y)
{
  if ((x >= LCD_X) || (y >= LCD_Y))  return;

  // x is which column
  buffer[x+(y>>3)*LCD_X] |= 0x80>>(y&7);
}

void st7565ClearPixel(uint8_t x, uint8_t y)
{
  if ((x >= LCD_X) || (y >= LCD_Y))  return;

  // x is which column
  buffer[x+(y>>3)*LCD_X] &= ~(0x80>>(y&7));
}

void st7565DrawRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy) {
  int i,j;
  uint8_t m;
  dy+=y;
  for(; y<dy; y++) {
    j=x+(y>>3)*LCD_X; m=0x80>>(y&7); i=j+dx;
    for(;j<i; j++)
      buffer[j]|=m;
  }
}





/**************************************************************************/
/*                                                                        */
/*                          FONT FUNCTIONS                                */
/*                                                                        */
/**************************************************************************/


/*
uint8_t xGlcdSelFontNbRows;
//uint8_t xGlcdSelFontOffset;
uint8_t* xGlcdSelFont;
uint8_t xGlcdSelFontWidth;
uint8_t xGlcdSelFontHeight;

#define xGlcdSelFontOffset 32

//xGlcd_Set_Font(OCR_A_Std8x15, 8,15,32);
void xGlcd_Set_Font(const uint8_t* ptrFontTbl, uint8_t font_width, uint8_t font_height)
{
   xGlcdSelFont = (uint8_t*)ptrFontTbl;
   xGlcdSelFontWidth = font_width;
   xGlcdSelFontHeight = font_height;
   //xGlcdSelFontOffset = 32;
   xGlcdSelFontNbRows = xGlcdSelFontHeight / 8;
   if (xGlcdSelFontHeight  % 8) xGlcdSelFontNbRows++;
}


*/


// font_t { width, height, first_char, last_char, *font};

font_t font3x6_st    = { 3,  7, 32,  97, Font3x6};
font_t font7x10_st   = { 7, 10,  1, 128, MS_Sans_Serif7x10};
font_t font13x12_st  = {13, 12, 32,  90, MS_Reference_Sans_Serif13x12};
font_t font13x12s_st = {13, 12, 32,  90, MS_Reference_Sans_Serif13x12_slim};


//unsigned short xGlcd_Write_Char(uint8_t  ch, uint8_t x, uint8_t y, font_t * font)
int xGlcd_Write_Char(uint8_t  ch, int x, int y, font_t *font)
{
  int i, j, width, mask=0;
  int cLength=0;
  uint8_t const* chars = font->font;
  uint8_t const* chars_end;
  
  if(ch > font->last_char) return font->width;
  if (chars == Font3x6) {
    width = cLength = 3;
    chars += 3*(ch-font->first_char);
  } else {
    int char_ix = ch-font->first_char;
    for (i=0; i<char_ix; i++)
      chars += chars[0]+2;
    cLength = *chars++;
    width   = *chars++;
  }

  chars_end = chars + cLength;
  for (i = 0; i< width; i++) {
    int z=8;
    for(j=0; j<font->height; j++,z++) {
      if (z==8) { z=0; if(chars==chars_end) return width; mask=*chars++; }
      if (mask&1)
        st7565DrawPixel(x+i,y+j);
      mask>>=1;
    }
  }

  return width;
}

void xGlcd_Write_Text(char* text, int x, int y, font_t * font)
{
  //unsigned short i, l, posX;
  uint32_t i, l, posX;
  char* curChar;
  l = strlen(text);
  posX = x;
  curChar = text;
  for (i=0; i<l && posX<LCD_X; i++){
      posX += xGlcd_Write_Char(*curChar, posX, y, font) +  1; //add 1 blank column
      curChar++;
  }
}

/*
unsigned short xGlcd_Text_Width(char* text)
{
  unsigned short i, l, w;
  l = strlen(text);
  w = 0;
  for (i = 0; i<l; i++)
      w = w + xGlcd_Char_Width(text[i])+1; //add 1 blank column
  return w;
}

unsigned short xGlcd_Char_Width(uint8_t ch)
{
  const uint8_t* CurCharDt;
  uint8_t cOffset;
  cOffset = xGlcdSelFontWidth * xGlcdSelFontNbRows+1;
  cOffset = cOffset * (ch-xGlcdSelFontOffset);
  CurCharDt = xGlcdSelFont+cOffset;
  return *CurCharDt;
}
*/


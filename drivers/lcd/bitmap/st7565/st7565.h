#ifndef __ST7565_H__
#define __ST7565_H__

#include "projectconfig.h"

#include "drivers/lcd/smallfonts.h"

/*
// original Pin Definitions
#define ST7565_A0_PORT                    2     // Register Select Pin (A0)
#define ST7565_A0_PIN                     4
#define ST7565_RST_PORT                   2     // Reset
#define ST7565_RST_PIN                    5
#define ST7565_CS_PORT                    2     // Select
#define ST7565_CS_PIN                     6
#define ST7565_BL_PORT                    2     // Backlight
#define ST7565_BL_PIN                     7
#define ST7565_SCLK_PORT                  2     // Serial Clock
#define ST7565_SCLK_PIN                   8
#define ST7565_SDAT_PORT                  2     // Serial Data
#define ST7565_SDAT_PIN                   9
*/

/*
// Pin Definitions prototype spring 2011
#define ST7565_VDD_PORT                   1     // power supply for the display (VDD)
#define ST7565_VDD_PIN                    9

#define ST7565_A0_PORT                    2     // Register Select Pin (A0)
#define ST7565_A0_PIN                     8
#define ST7565_RST_PORT                   2     // Reset
#define ST7565_RST_PIN                    7
#define ST7565_CS_PORT                    0     // Select
#define ST7565_CS_PIN                     2
#define ST7565_SCLK_PORT                  2     // Serial Clock
#define ST7565_SCLK_PIN                   1
//#define ST7565_SDAT_PORT                3     // Serial Data ... this is weird: SDA is connected to pin 18 which should be GPIO3_4 not GPIO2_4 ...
#define ST7565_SDAT_PORT                  0     // Serial Data
#define ST7565_SDAT_PIN                   3
*/

// Pin Definitions HW rev3 december 2011
//#define ST7565_VDD_PORT                   1         // power supply for the display (VDD)
//#define ST7565_VDD_PIN                    9

#define ST7565_A0_PORT                    2           // Register Select Pin (A0)
#define ST7565_A0_PIN                     0
#define IOCON_LCD_A0                      IOCON_PIO2_0
#define ST7565_RST_PORT                   2           // Reset
#define ST7565_RST_PIN                    6
#define IOCON_LCD_RST                     IOCON_PIO2_6
#define ST7565_CS_PORT                    3           // Select
#define ST7565_CS_PIN                     3
#define IOCON_LCD_CS                      IOCON_PIO3_3
#define ST7565_SCLK_PORT                  1           // Serial Clock
#define ST7565_SCLK_PIN                   8
#define IOCON_LCD_SCLK                    IOCON_PIO1_8
#define ST7565_SDAT_PORT                  0           // Serial Data
#define ST7565_SDAT_PIN                   2
#define IOCON_LCD_SDAT                    IOCON_PIO0_2

// Commands
#define ST7565_CMD_DISPLAY_OFF            0xAE
#define ST7565_CMD_DISPLAY_ON             0xAF
#define ST7565_CMD_SET_DISP_START_LINE    0x40
#define ST7565_CMD_SET_PAGE               0xB0
#define ST7565_CMD_SET_COLUMN_UPPER       0x10
#define ST7565_CMD_SET_COLUMN_LOWER       0x00
#define ST7565_CMD_SET_ADC_NORMAL         0xA0
#define ST7565_CMD_SET_ADC_REVERSE        0xA1
#define ST7565_CMD_SET_DISP_NORMAL        0xA6
#define ST7565_CMD_SET_DISP_REVERSE       0xA7
#define ST7565_CMD_SET_ALLPTS_NORMAL      0xA4
#define ST7565_CMD_SET_ALLPTS_ON          0xA5
#define ST7565_CMD_SET_BIAS_9             0xA2 
#define ST7565_CMD_SET_BIAS_7             0xA3
#define ST7565_CMD_RMW                    0xE0
#define ST7565_CMD_RMW_CLEAR              0xEE
#define ST7565_CMD_INTERNAL_RESET         0xE2
#define ST7565_CMD_SET_COM_NORMAL         0xC0
#define ST7565_CMD_SET_COM_REVERSE        0xC8
#define ST7565_CMD_SET_POWER_CONTROL      0x28
#define ST7565_CMD_SET_RESISTOR_RATIO     0x20
#define ST7565_CMD_SET_VOLUME_FIRST       0x81
#define ST7565_CMD_SET_VOLUME_SECOND      0
#define ST7565_CMD_SET_STATIC_OFF         0xAC
#define ST7565_CMD_SET_STATIC_ON          0xAD
#define ST7565_CMD_SET_STATIC_REG         0x0
#define ST7565_CMD_SET_BOOSTER_FIRST      0xF8
#define ST7565_CMD_SET_BOOSTER_234        0
#define ST7565_CMD_SET_BOOSTER_5          1
#define ST7565_CMD_SET_BOOSTER_6          3
#define ST7565_CMD_NOP                    0xE3
#define ST7565_CMD_TEST                   0xF0

// Initialisation/Config Prototypes
void st7565VDD(bool state);

void st7565TurnOff(void);

void st7565Init( void );
void st7565Command( uint8_t c );
void st7565Data( uint8_t d );
void st7565SetBrightness( uint8_t val );
void st7565SetBooster( uint8_t val );
void sendByte(uint8_t byte);

void st7565TurnDisplayON(void);
void st7565TurnDisplayOFF(void);

// Drawing Prototypes
void st7565ClearScreen( void );
void st7565Refresh( void );
//void st7565SetPixel(uint8_t x, uint8_t y, bool draw);

void st7565DrawPixel( uint8_t x, uint8_t y );
void st7565ClearPixel( uint8_t x, uint8_t y );

void st7565DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
//uint8_t st7565GetPixel( uint8_t x, uint8_t y );
//void st7565DrawString( uint16_t x, uint16_t y, char* text, struct FONT_DEF font );
//void st7565ShiftFrameBuffer( uint8_t pixels );


typedef struct {
  uint8_t  width;
  uint8_t  height;
  uint8_t  first_char;
  uint8_t  last_char;
  uint8_t const* font;
} font_t;


extern font_t font3x6_st;
extern font_t font7x10_st;
extern font_t font13x12_st;
extern font_t font13x12s_st;

#define font3x6    (&font3x6_st)
#define font7x10   (&font7x10_st)
#define font13x12  (&font13x12_st)
#define font13x12s (&font13x12s_st)


void xGlcd_Write_Text(char* text, int x, int y, font_t * font);
//void xGlcd_Write_Text(char* text, unsigned short x, unsigned short y, font_t *font);
//void xGlcd_Set_Font(const uint8_t* ptrFontTbl, uint8_t font_width, uint8_t font_height);


// Regulation ration range 0-7, Brightness range 0-3f
void set_lcd_params();

#define LCD_BRIGHTNESS_MAX     0x3f
#define LCD_OPER_VOLTAGE_MAX      7

extern uint8_t lcd_brightness;
extern uint8_t lcd_oper_voltage;

#endif

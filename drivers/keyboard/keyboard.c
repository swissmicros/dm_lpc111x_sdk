#include "drivers/keyboard/keyboard.h"
#include "core/gpio/gpio.h"
#include "core/pmu/pmu.h"
#include "main.h"

#define MAX_SCAN_KEYS   2
#define ROW_COUNT       4
#define COL_COUNT      10


#define ROW(x)   key_matrix_h[x][0], key_matrix_h[x][1]
#define COL(x)   key_matrix_v[x][0], key_matrix_v[x][1]

#define OUT

const uint8_t key_matrix_h[ROW_COUNT][2]={
  { KEYBOARD_H0_PORT, KEYBOARD_H0_PIN },
  { KEYBOARD_H1_PORT, KEYBOARD_H1_PIN },
  { KEYBOARD_H2_PORT, KEYBOARD_H2_PIN },
  { KEYBOARD_H3_PORT, KEYBOARD_H3_PIN }
};

const uint8_t key_matrix_v[COL_COUNT][2]={
  { KEYBOARD_V0_PORT, KEYBOARD_V0_PIN },
  { KEYBOARD_V1_PORT, KEYBOARD_V1_PIN },
  { KEYBOARD_V2_PORT, KEYBOARD_V2_PIN },
  { KEYBOARD_V3_PORT, KEYBOARD_V3_PIN },
  { KEYBOARD_V4_PORT, KEYBOARD_V4_PIN },
  { KEYBOARD_V5_PORT, KEYBOARD_V5_PIN },
  { KEYBOARD_V6_PORT, KEYBOARD_V6_PIN },
  { KEYBOARD_V7_PORT, KEYBOARD_V7_PIN },
  { KEYBOARD_V8_PORT, KEYBOARD_V8_PIN },
  { KEYBOARD_V9_PORT, KEYBOARD_V9_PIN }
};

#ifdef COCONUT

const uint8_t key_code_normal[COL_COUNT * ROW_COUNT] = {
  0x18, 0xc6, 0x11, 0x10,
  0x12, 0xc5, 0x31, 0x30,
  0xc4, 0x32, 0x71, 0x70,
  0x72, 0xc2, 0x81, 0x80,
  0x82, 0xc3, 0xc1, 0xc0,
  0x13, 0x13, 0x83, 0x73,
  0x17, 0x16, 0x15, 0x14,
  0x37, 0x36, 0x35, 0x34,
  0x77, 0x76, 0x75, 0x74,
  0x87, 0x86, 0x85, 0x84
};

/*
const uint8_t key_code_alpha[COL_COUNT * ROW_COUNT] = {
  0x18, 0xc6, 0x32, 0x10,
  0x12, 0xc5, 0x72, 0x30,
  0xc4, 0x76, 0x82, 0x70,
  0x86, 0xc2, 0x13, 0x80,
  0x17, 0xc3, 0x73, 0xc0,
  0x37, 0x37, 0x83, 0x11,
  0x16, 0x15, 0x14, 0x31,
  0x36, 0x35, 0x34, 0x71,
  0x77, 0x75, 0x74, 0x81,
  0x87, 0x85, 0x84, 0xc1
};

#define KEY_TABLES_COUNT 2
const uint8_t *key_code[KEY_TABLES_COUNT] = { key_code_normal, key_code_alpha };

int current_key_table = 0;


void set_key_table(int table_nr) {
  if (table_nr < 0 || table_nr >= KEY_TABLES_COUNT)
    return;
  current_key_table = table_nr;
}

#define KEY_CODE key_code[current_key_table]
*/

#define KEY_CODE key_code_normal

#else

const uint8_t key_code[COL_COUNT * ROW_COUNT] = {
  0x18, 0x11, 0x10, 0x13,
  0x38, 0x31, 0x30, 0x33,
  0x78, 0x71, 0x70, 0x73,
  0xc8, 0xc1, 0xc0, 0xc3,
  0x88, 0x81, 0x80, 0x83,
  0x84, 0x84, 0x87, 0x82,
  0xc5, 0xc4, 0xc7, 0xc2,
  0x75, 0x74, 0x77, 0x72,
  0x35, 0x34, 0x37, 0x32,
  0x15, 0x14, 0x17, 0x12
};

#define KEY_CODE key_code

#endif



// Inverted function for tables above
uint8_t keycode2rc(uint8_t code) {
  int r=4,c=1,i;

  for(i=0; i< COL_COUNT * ROW_COUNT; i++) {
    if (code == KEY_CODE[i])
      return (r<<4)|c;
    r--; if(r==0) {r=4;c++;if(c>9) c=0;}
  }
  return 0;
}


// Row column code to key code
// returns 0 when code is out of bounds
uint8_t rc2keycode(uint8_t rc) {
  int r,c;
  r=(rc&0xf0)>>4; c=((rc&0xf)<<2)-4;
  if(c<0) c+=40;
  // check bounds
  if (r<1 || r>4 || c<0 || c>36)
    return 0;
  return KEY_CODE[4-r+c];
}

#define IOCON_BASE 0x40044000
#define IOCON_OFFSET(x) (uint8_t)( (uint32_t)(&x) - IOCON_BASE )

const uint8_t iocon_offsets[] = {
  IOCON_OFFSET(IOCON_KBD_V0),
  IOCON_OFFSET(IOCON_KBD_V1),
  IOCON_OFFSET(IOCON_KBD_V2),
  IOCON_OFFSET(IOCON_KBD_V3),
  IOCON_OFFSET(IOCON_KBD_V4),
  IOCON_OFFSET(IOCON_KBD_V5),
  IOCON_OFFSET(IOCON_KBD_V6),
  IOCON_OFFSET(IOCON_KBD_V7),
  IOCON_OFFSET(IOCON_KBD_V8),
  IOCON_OFFSET(IOCON_KBD_V9)
};

void set_kbd_pullups(gpioPullupMode_t mode) {
  int i=0;
  for(; i<10; i++)
    gpioSetPullup(pREG32(iocon_offsets[i]+IOCON_BASE),  mode);
  /*
  gpioSetPullup(&IOCON_KBD_V0,  mode);
  gpioSetPullup(&IOCON_KBD_V1,  mode);
  gpioSetPullup(&IOCON_KBD_V2,  mode);
  gpioSetPullup(&IOCON_KBD_V3,  mode);
  gpioSetPullup(&IOCON_KBD_V4,  mode);
  gpioSetPullup(&IOCON_KBD_V5,  mode);
  gpioSetPullup(&IOCON_KBD_V6,  mode);
  gpioSetPullup(&IOCON_KBD_V7,  mode);
  gpioSetPullup(&IOCON_KBD_V8,  mode);
  gpioSetPullup(&IOCON_KBD_V9,  mode);
  */
}

static int pins_initialized = 0;

void init_keyboard_pins(void) {
  int i;
  
  if (pins_initialized)
    return;

  // Enable pull-ups on keyboard for rows
  set_kbd_pullups(gpioPullupMode_PullUp);

  for (i=0;i<COL_COUNT;i++) {
    // set all column pins to in
    gpioSetDir(COL(i), 0);
    gpioSetValue(COL(i), 1);
  };

  pins_initialized = 1;
}


/***************************
   Prepare for sleep
***************************/
void uninit_keyboard_pins() {
  int i;

  // All column lines out and down
  for (i=0; i<COL_COUNT; i++) {
    gpioSetDir(COL(i), 1);
    gpioSetValue(COL(i), 0);
  }

  // Disable unnecessary pull-ups on keyboard
  set_kbd_pullups(gpioPullupMode_Inactive);

  pins_initialized = 0;
}


// As the ON line is shared with RTC we need quick
// scan option to check it.
// When read_on_key arg == 0 ... just read the pin state 
// to detect whether the line is free.
int read_on_key_pin(int read_on_key) {
  int k = 0;

  // Prepare pins for scan
  init_keyboard_pins();

  // Test ON key
  if ( read_on_key ) {
    gpioSetDir(COL(0),1);  gpioSetValueM(KEYBOARD_V0_PORT, KEYBOARD_V0_PIN,0);
  }
  if(!gpioGetValue(KEYBOARD_ON_PORT, KEYBOARD_ON_PIN) || !gpioGetValue(KEYBOARD_ON_PORT, KEYBOARD_ON_PIN))
    k = 1;
  if ( read_on_key ) {
    gpioSetDir(COL(0),0);  gpioSetValueM(KEYBOARD_V0_PORT, KEYBOARD_V0_PIN,1);
  }
  return k;
}


// Keyboard scanning routine.
//
// Returns scanned key in k1. The routine can scan [ON] key
// pressed with one other. Then k1 contains code for [ON]
// key and k2 code of the other key.
//
// Return value:
//     1 or 0 when key is pressed or not respectively.
//
int read_key(int *k1, int *k2) {
  int i,j,k;
  int keys[MAX_SCAN_KEYS];
  int press_count=0;

  // Reset scan buffer
  for(i=0; i<MAX_SCAN_KEYS; i++) keys[i]=0;

  // Prepare pins for scan and read ON key
  if ( read_on_key_pin(1) )
    keys[press_count++]=KEY_CODE[0];

  k=0;
  for (i=0; i<COL_COUNT; i++) {
    // Set col to output
    gpioSetDir(COL(i), 1);
    gpioSetValue(COL(i), 0);
    for (j=ROW_COUNT-1; (j >= 0) && (press_count < MAX_SCAN_KEYS); j--,k++)
      if (!gpioGetValue(ROW(j)) || !gpioGetValue(ROW(j)) || !gpioGetValue(ROW(j)))
        keys[press_count++]=KEY_CODE[k];
    // Back to input
    gpioSetDir(COL(i), 0);
    gpioSetValue(COL(i), 1);
  }

  if (press_count==0) return 0;

// done:
  // We have the key values in key1, key2
  *k1 = keys[0]; *k2 = keys[1];
  return 1;
}


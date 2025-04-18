#ifndef __KEYBOARD__
#define __KEYBOARD__

#include <stdint.h>

uint8_t rc2keycode(uint8_t rc);
uint8_t keycode2rc(uint8_t code);

void read_and_exec_key();
void init_keyboard_pins();
void uninit_keyboard_pins();
int read_key(int *k1, int *k2);
int read_on_key_pin(int read_on_key);

#ifdef COCONUT
void set_key_table(int table_nr);
#endif


#define KEYBOARD_H0_PORT                   0
#define KEYBOARD_H0_PIN                    7
#define WAKEUP_KBD_H0                      WAKEUP7_IRQn  // P0.7
#define KBD_PIO_H0                         0_7

#define KEYBOARD_H1_PORT                   0
#define KEYBOARD_H1_PIN                    8
#define WAKEUP_KBD_H1                      WAKEUP8_IRQn  // P0.8
#define KBD_PIO_H1                         0_8

#define KEYBOARD_H2_PORT                   0
#define KEYBOARD_H2_PIN                    10
#define WAKEUP_KBD_H2                      WAKEUP10_IRQn // P0.10
#define KBD_PIO_H2                         0_10

#define KEYBOARD_H3_PORT                   1
#define KEYBOARD_H3_PIN                    0
#define WAKEUP_KBD_H3                      WAKEUP12_IRQn // P1.0
#define KBD_PIO_H3                         1_0

#define KEYBOARD_ON_PORT                   0
#define KEYBOARD_ON_PIN                    11
#define WAKEUP_KBD_ON                      WAKEUP11_IRQn // P0.11 (CT32B0_MAT3)
#define KBD_PIO_ON                         0_11


#define KEYBOARD_V0_PORT                   2
#define KEYBOARD_V0_PIN                    7
#define IOCON_KBD_V0                       IOCON_PIO2_7
#define KEYBOARD_V1_PORT                   2
#define KEYBOARD_V1_PIN                    8
#define IOCON_KBD_V1                       IOCON_PIO2_8
#define KEYBOARD_V2_PORT                   2
#define KEYBOARD_V2_PIN                    1
#define IOCON_KBD_V2                       IOCON_PIO2_1
#define KEYBOARD_V3_PORT                   0
#define KEYBOARD_V3_PIN                    3
#define IOCON_KBD_V3                       IOCON_PIO0_3
#define KEYBOARD_V4_PORT                   1
#define KEYBOARD_V4_PIN                    9
#define IOCON_KBD_V4                       IOCON_PIO1_9
#define KEYBOARD_V5_PORT                   3
#define KEYBOARD_V5_PIN                    4
#define IOCON_KBD_V5                       IOCON_PIO3_4
#define KEYBOARD_V6_PORT                   2
#define KEYBOARD_V6_PIN                    4
#define IOCON_KBD_V6                       IOCON_PIO2_4
#define KEYBOARD_V7_PORT                   2
#define KEYBOARD_V7_PIN                    5
#define IOCON_KBD_V7                       IOCON_PIO2_5
#define KEYBOARD_V8_PORT                   3
#define KEYBOARD_V8_PIN                    5
#define IOCON_KBD_V8                       IOCON_PIO3_5
#define KEYBOARD_V9_PORT                   0
#define KEYBOARD_V9_PIN                    6
#define IOCON_KBD_V9                       IOCON_PIO0_6

#endif


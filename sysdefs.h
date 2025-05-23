#ifndef _SYSDEFS_H_
#define _SYSDEFS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Stay compatible with ugly "windows" style
#define BOOL bool
#define TRUE true
#define FALSE false

typedef volatile uint8_t  REG8;
typedef volatile uint16_t REG16;
typedef volatile uint32_t REG32;
typedef unsigned char byte_t;

#define pREG8  (REG8 *)
#define pREG16 (REG16 *)
#define pREG32 (REG32 *)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif


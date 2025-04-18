#ifndef _GPIO_H_
#define _GPIO_H_

#include "projectconfig.h"

/**************************************************************************/
/*! 
    Indicates whether the interrupt should be configured as edge or 
    level sensitive.
*/
/**************************************************************************/
typedef enum gpioInterruptSense_e
{
  gpioInterruptSense_Edge = 0,
  gpioInterruptSense_Level
} 
gpioInterruptSense_t;

/**************************************************************************/
/*! 
    Indicates whether one edge or both edges trigger an interrupt.
    Setting this to Double will cause both edges to trigger an interrupt.
*/
/**************************************************************************/
typedef enum gpioInterruptEdge_e
{
  gpioInterruptEdge_Single = 0,
  gpioInterruptEdge_Double
} 
gpioInterruptEdge_t;

/**************************************************************************/
/*! 
    Indicates whether the interrupt should be triggered in the rising
    or falling edge.  ActiveHigh means that a HIGH level on the pin will
    trigger an interrupt, ActiveLow means that a LOW level on the pin
    will trigger an interrupt.
*/
/**************************************************************************/
typedef enum gpioInterruptEvent_e
{
  gpioInterruptEvent_ActiveHigh = 0,
  gpioInterruptEvent_ActiveLow
} 
gpioInterruptEvent_t;

typedef enum gpioDirection_e
{
  gpioDirection_Input = 0,
  gpioDirection_Output
}
gpioDirection_t;

typedef enum gpioPullupMode_e
{
  gpioPullupMode_Inactive = IOCON_COMMON_MODE_INACTIVE,
  gpioPullupMode_PullDown = IOCON_COMMON_MODE_PULLDOWN,
  gpioPullupMode_PullUp =   IOCON_COMMON_MODE_PULLUP,
  gpioPullupMode_Repeater = IOCON_COMMON_MODE_REPEATER
}
gpioPullupMode_t;

void gpioInit (void);
void gpioSetDir (uint32_t portNum, uint32_t bitPos, gpioDirection_t dir);
uint32_t gpioGetValue (uint32_t portNum, uint32_t bitPos);
void gpioSetValue (uint32_t portNum, uint32_t bitPos, uint32_t bitVal);
void gpioSetInterrupt (uint32_t portNum, uint32_t bitPos, gpioInterruptSense_t sense, gpioInterruptEdge_t edge, gpioInterruptEvent_t event);
void gpioIntEnable (uint32_t portNum, uint32_t bitPos);
void gpioIntDisable (uint32_t portNum, uint32_t bitPos);
uint32_t  gpioIntStatus (uint32_t portNum, uint32_t bitPos);
void gpioIntClear (uint32_t portNum, uint32_t bitPos);
void gpioSetPullup (volatile uint32_t *ioconRegister, gpioPullupMode_t mode);

// dir=0 for input
#define setpin(port, pin, dir, val) gpioSetDir(port, pin, dir); gpioSetValue(port, pin, val)

#ifdef CFG_GPIO_MACRO

#define PORTNUM2PORT(portNum) \
 GPIO_GPIO ## portNum ## DATA

#define _gpioSetValue(port, bitPos, bitVal)  \
 (bitVal) ? (port |= (1 << bitPos)) : (port &= ~(1 << bitPos))

#define gpioSetValueM(portNum, bitPos, bitVal) \
 _gpioSetValue(PORTNUM2PORT(portNum), bitPos, bitVal)

#else

#define gpioSetValueM gpioSetValue


#endif

#endif

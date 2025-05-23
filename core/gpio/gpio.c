
#include "gpio.h"

static bool _gpioInitialised = false;



#ifdef CFG_RTC

#include "rtc.h"

/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 0 (currently checks pin 0.11)
*/
/**************************************************************************/
/*/
void PIOINT0_IRQHandler(void)
{
  uint32_t regVal;

  regVal = gpioIntStatus(0, 11);
  if (regVal)
  {
    gpioIntDisable(0, 11);
    rtc_int();
    gpioIntClear(0, 11);
    gpioIntEnable(0, 11);
  }
  return;
}
//*/

/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 1 (checks pin 1.4 for RTC INT)
*/
/**************************************************************************/
/*/
void PIOINT1_IRQHandler(void)
{
  uint32_t regVal;

  regVal = gpioIntStatus(1, 4);
  if ( regVal )
  {
    NVIC_DisableIRQ(EINT1_IRQn);
    tmr_stat.ints++;
    gpioIntDisable(1, 4);
    rtc_int();
    gpioIntClear(1, 4);
    gpioIntEnable(1, 4);
  }

  return;
}
//*/
#endif


#if 0
volatile uint32_t gpio0_counter = 0;
volatile uint32_t gpio1_counter = 0;
volatile uint32_t gpio2_counter = 0;
volatile uint32_t gpio3_counter = 0;
volatile uint32_t p0_1_counter  = 0;
volatile uint32_t p1_1_counter  = 0;
volatile uint32_t p2_1_counter  = 0;
volatile uint32_t p3_1_counter  = 0;


/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 0 (currently checks pin 0.1)
*/
/**************************************************************************/
void PIOINT0_IRQHandler(void)
{
  uint32_t regVal;

  gpio0_counter++;

  regVal = gpioIntStatus(0, 1);
  if (regVal)
  {
    p0_1_counter++;
    gpioIntClear(0, 1);
  }
  return;
}

/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 1 (currently checks pin 1.1)
*/
/**************************************************************************/
void PIOINT1_IRQHandler(void)
{
  uint32_t regVal;

  gpio1_counter++;

  regVal = gpioIntStatus(1, 1);
  if ( regVal )
  {
    p1_1_counter++;
    gpioIntClear(1, 1);
  }

  return;
}

/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 2 (currently checks pin 2.1)
*/
/**************************************************************************/
void PIOINT2_IRQHandler(void)
{
  uint32_t regVal;

  gpio2_counter++;
  regVal = gpioIntStatus(2, 1);
  if ( regVal )
  {
    p2_1_counter++;
    gpioIntClear(2, 1);
  }
  return;
}

/**************************************************************************/
/*! 
    @brief IRQ Handler for GPIO port 3 (currently checks pin 3.1)
*/
/**************************************************************************/
void PIOINT3_IRQHandler(void)
{
  uint32_t regVal;

  gpio3_counter++;
  regVal = gpioIntStatus(3, 1);
  if ( regVal )
  {
    p3_1_counter++;
    gpioIntClear(3, 1);
  }
 return;
}

#endif

/**************************************************************************/
/*! 
    @brief Initialises GPIO and enables the GPIO interrupt
           handler for all GPIO ports.
*/
/**************************************************************************/
void gpioInit (void)
{
  /* Enable AHB clock to the GPIO domain. */
  SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_GPIO);

  /* Set up NVIC when I/O pins are configured as external interrupts. */
  // NVIC_EnableIRQ(EINT0_IRQn);
  // NVIC_EnableIRQ(EINT1_IRQn);
  // NVIC_EnableIRQ(EINT2_IRQn);
  // NVIC_EnableIRQ(EINT3_IRQn);

  /* Set initialisation flag */
  _gpioInitialised = true;

  return;
}

/**************************************************************************/
/*! 
    @brief Sets the direction (input/output) for a specific port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..11)
    @param[in]  dir
                The pin direction (gpioDirection_Input or
                gpioDirection_Output)
*/
/**************************************************************************/
void gpioSetDir (uint32_t portNum, uint32_t bitPos, gpioDirection_t dir)
{
  if (!_gpioInitialised) gpioInit();

  // Get the appropriate register (handled this way to optimise code size)
  REG32 *gpiodir = &GPIO_GPIO0DIR;
  switch (portNum)
  {
    case 0:
      gpiodir = &GPIO_GPIO0DIR;
      break;
    case 1:
      gpiodir = &GPIO_GPIO1DIR;
      break;
    case 2:
      gpiodir = &GPIO_GPIO2DIR;
      break;
    case 3:
      gpiodir = &GPIO_GPIO3DIR;
      break;
  }

  // Toggle dir
  dir == gpioDirection_Output ? (*gpiodir |= (1 << bitPos)) : (*gpiodir &= ~(1 << bitPos));
}

/**************************************************************************/
/*! 
    @brief Gets the value for a specific port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)

    @return     The current value for the specified port pin (0..1)
*/
/**************************************************************************/
uint32_t gpioGetValue (uint32_t portNum, uint32_t bitPos)
{
  if (!_gpioInitialised) gpioInit();

  uint32_t value = 0;

  switch (portNum)
  {
    case 0:
      value = (GPIO_GPIO0DATA & (1 << bitPos)) ? 1 : 0;
      break;
    case 1:
      value = (GPIO_GPIO1DATA & (1 << bitPos)) ? 1 : 0;
      break;
    case 2:
      value = (GPIO_GPIO2DATA & (1 << bitPos)) ? 1 : 0;
      break;
    case 3:
      value = (GPIO_GPIO3DATA & (1 << bitPos)) ? 1 : 0;
      break;
    default:
      break;
  }

  return value;
}

/**************************************************************************/
/*! 
    @brief Sets the value for a specific port pin (only relevant when a
           pin is configured as output).

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)
    @param[in]  bitValue
                The value to set for the specified bit (0..1).  0 will set
                the pin low and 1 will set the pin high.
*/
/**************************************************************************/
void gpioSetValue (uint32_t portNum, uint32_t bitPos, uint32_t bitVal)
{
  if (!_gpioInitialised) gpioInit();


  // Get the appropriate register (handled this way to optimise code size)
  REG32 *gpiodata = &GPIO_GPIO0DATA;
  switch (portNum)
  {
    case 0:
      gpiodata = &GPIO_GPIO0DATA;
      break;
    case 1:
      gpiodata = &GPIO_GPIO1DATA;
      break;
    case 2:
      gpiodata = &GPIO_GPIO2DATA;
      break;
    case 3:
      gpiodata = &GPIO_GPIO3DATA;
      break;
  }

  // Toggle value
  bitVal == 1 ? (*gpiodata |= (1 << bitPos)) : (*gpiodata &= ~(1 << bitPos));
}

/**************************************************************************/
/*! 
    @brief Sets the interrupt sense, event, etc.

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)
    @param[in]  sense
                Whether the interrupt should be configured as edge or level
                sensitive.
    @param[in]  edge
                Whether one edge or both trigger an interrupt.
    @param[in]  event
                Whether the rising or the falling edge (high or low)
                should be used to trigger the interrupt.

    @section Example

    @code
    // Initialise gpio
    gpioInit();
    // Set GPIO1.8 to input
    gpioSetDir(1, 8, gpioDirection_Input);
    // Disable the internal pullup/down resistor on P1.8
    gpioSetPullup (&IOCON_PIO1_8, gpioPullupMode_Inactive);
    // Setup an interrupt on GPIO1.8
    gpioSetInterrupt(1,                               // Port
                     8,                               // Pin
                     gpioInterruptSense_Edge,         // Edge/Level Sensitive
                     gpioInterruptEdge_Single,        // Single/Double Edge
                     gpioInterruptEvent_ActiveHigh);  // Rising/Falling
    // Enable the interrupt
    gpioIntEnable(1, 8);
    @endcode
*/
/**************************************************************************/
void gpioSetInterrupt (uint32_t portNum, uint32_t bitPos, gpioInterruptSense_t sense, gpioInterruptEdge_t edge, gpioInterruptEvent_t event)
{
  if (!_gpioInitialised) gpioInit();

  // Get the appropriate register (handled this way to optimise code size)
  REG32 *gpiois  = &GPIO_GPIO0IS;
  REG32 *gpioibe = &GPIO_GPIO0IBE;
  REG32 *gpioiev = &GPIO_GPIO0IEV;
  switch (portNum)
  {
    case 0:
      gpiois  = &GPIO_GPIO0IS;
      gpioibe = &GPIO_GPIO0IBE;
      gpioiev = &GPIO_GPIO0IEV;
      break;
    case 1:
      gpiois  = &GPIO_GPIO1IS;
      gpioibe = &GPIO_GPIO1IBE;
      gpioiev = &GPIO_GPIO1IEV;
      break;
    case 2:
      gpiois  = &GPIO_GPIO2IS;
      gpioibe = &GPIO_GPIO2IBE;
      gpioiev = &GPIO_GPIO2IEV;
      break;
    case 3:
      gpiois  = &GPIO_GPIO3IS;
      gpioibe = &GPIO_GPIO3IBE;
      gpioiev = &GPIO_GPIO3IEV;
      break;
  }

  if (gpioInterruptSense_Edge)
  {
    *gpiois &= ~(0x1<<bitPos);
    /* single or double only applies when sense is 0(edge trigger). */
    if(gpioInterruptEdge_Single) *gpioibe &= ~(0x1<<bitPos); else *gpioibe |= (0x1<<bitPos);
  }
  else
  {
    *gpiois |= (0x1<<bitPos);
  }

  if(gpioInterruptEvent_ActiveHigh) *gpioiev &= ~(0x1<<bitPos); else *gpioiev |= (0x1<<bitPos);

  return;
}

/**************************************************************************/
/*! 
    @brief Enables the interrupt mask for a specific port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)
*/
/**************************************************************************/
void gpioIntEnable (uint32_t portNum, uint32_t bitPos)
{
  if (!_gpioInitialised) gpioInit();

  switch (portNum)
  {
    case 0:
      GPIO_GPIO0IE |= (0x1<<bitPos);
      break;
    case 1:
      GPIO_GPIO1IE |= (0x1<<bitPos);
      break;
    case 2:
      GPIO_GPIO2IE |= (0x1<<bitPos);
      break;
    case 3:
      GPIO_GPIO3IE |= (0x1<<bitPos);
      break;
    default:
      break;
  }
  return;
}

/**************************************************************************/
/*! 
    @brief Disables the interrupt mask for a specific port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)
*/
/**************************************************************************/
void gpioIntDisable (uint32_t portNum, uint32_t bitPos)
{
  if (!_gpioInitialised) gpioInit();

  switch (portNum)
  {
    case 0:
      GPIO_GPIO0IE &= ~(0x1<<bitPos);
      break;
    case 1:
      GPIO_GPIO1IE &= ~(0x1<<bitPos);
      break;
    case 2:
      GPIO_GPIO2IE &= ~(0x1<<bitPos);
      break;
    case 3:
      GPIO_GPIO3IE &= ~(0x1<<bitPos);
      break;
    default:
      break;
  }
  return;
}

/**************************************************************************/
/*! 
    @brief Gets the interrupt status for a specific port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)

    @return     The interrupt status for the specified port pin (0..1)
*/
/**************************************************************************/
uint32_t gpioIntStatus (uint32_t portNum, uint32_t bitPos)
{
  if (!_gpioInitialised) gpioInit();

  uint32_t regVal = 0;

  switch (portNum)
  {
    case 0:
      if (GPIO_GPIO0MIS & (0x1<<bitPos))
      {
        regVal = 1;
      }
      break;
    case 1:
      if (GPIO_GPIO1MIS & (0x1<<bitPos))
      {
        regVal = 1;
      }
      break;
    case 2:
      if (GPIO_GPIO2MIS & (0x1<<bitPos))
      {
        regVal = 1;
      }
      break;
    case 3:
      if (GPIO_GPIO3MIS & (0x1<<bitPos))
      {
        regVal = 1;
      }
      break;
    default:
      break;
  }
  return ( regVal );
}

/**************************************************************************/
/*! 
    @brief Clears the interrupt for a port pin

    @param[in]  portNum
                The port number (0..3)
    @param[in]  bitPos
                The bit position (0..31)
*/
/**************************************************************************/
void gpioIntClear (uint32_t portNum, uint32_t bitPos)
{
  if (!_gpioInitialised) gpioInit();

  switch (portNum)
  {
    case 0:
      GPIO_GPIO0IC |= (0x1<<bitPos); 
    break;
    case 1:
      GPIO_GPIO1IC |= (0x1<<bitPos);
    break;
    case 2:
      GPIO_GPIO2IC |= (0x1<<bitPos);
    break;
    case 3:
      GPIO_GPIO3IC |= (0x1<<bitPos);
    break;
    default:
      break;
  }
  return;
}

/**************************************************************************/
/*! 
    @brief Configures the internal pullup/down resistor for GPIO pins
           (only relevant for pins configured as inputs)

    @param[in]  ioconReg
                A pointer to the IOCON registry value corresponding to
                the pin you wish to change (for example: &IOCON_PIO2_0
                for GPIO pin 2.0).
    @param[in]  mode
                The 'mode' that the pin should be set to, which must be
                correspond to a value defined in gpioPullupMode_t

    @warning    By default, all GPIO pins have the internal pull-up
                resistor enabled.  This may cause unusual behaviour if
                care isn't taken to set the internal resistor to an
                appropriate state.

    @section Example

    @code
    // Initialise gpio
    gpioInit();
    // Set GPIO1.8 to input
    gpioSetDir(1, 8, gpioDirection_Input);
    // Disable the internal pullup/down resistor on P1.8
    gpioSetPullup(&IOCON_PIO1_8, gpioPullupMode_Inactive);
    @endcode
*/
/**************************************************************************/
void gpioSetPullup (volatile uint32_t *ioconReg, gpioPullupMode_t mode)
{
  if (!_gpioInitialised) gpioInit();

  // ToDo: Disable interrupts while we are doing this?

  *ioconReg &= ~(IOCON_COMMON_MODE_MASK);
  *ioconReg |= mode;

  // ToDo: Re-enable interrupts?
};

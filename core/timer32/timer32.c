
#include "timer32.h"
#include "core/gpio/gpio.h"

volatile uint32_t timer32_0_counter = 0;
volatile uint32_t timer32_1_counter = 0;



/**************************************************************************/
/*! 
    @brief      Causes a blocking delay for the specified number of
                timer ticks.  The duration of each 'tick' is determined by
                the 'timerInterval' property supplied to timer32Init.
            
    @param[in]  timerNum
                The 32-bit timer to user (0..1)
    @param[in]  delay
                The number of counter increments to wait
*/
/**************************************************************************/
void timer32Delay(uint8_t timerNum, uint32_t delay)
{
  uint32_t curTicks;

  if (timerNum == 0)
  {
    curTicks = timer32_0_counter;
    if (curTicks > 0xFFFFFFFF - delay)
    {
      // Rollover will occur during delay
      while (timer32_0_counter >= curTicks)
      {
        while (timer32_0_counter < (delay - (0xFFFFFFFF - curTicks)));
      }      
    }
    else
    {
      while ((timer32_0_counter - curTicks) < delay);
    }
  }

  else if (timerNum == 1)
  {
    curTicks = timer32_1_counter;
    if (curTicks > 0xFFFFFFFF - delay)
    {
      // Rollover will occur during delay
      while (timer32_1_counter >= curTicks)
      {
        while (timer32_1_counter < (delay - (0xFFFFFFFF - curTicks)));
      }      
    }
    else
    {
      while ((timer32_1_counter - curTicks) < delay);
    }
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Interrupt handler for 32-bit timer 0
*/
/**************************************************************************/
void TIMER32_0_IRQHandler(void)
{  
  /* Clear the interrupt flag */
  TMR_TMR32B0IR = TMR_TMR32B0IR_MR0;

  /* If you wish to perform some action after each timer 'tick' (such as 
     incrementing a counter variable) you can do so here */
  timer32_0_counter++;

  return;
}


/**************************************************************************/
/*! 
        @brief Example of beep handling
*/
/**************************************************************************/
#ifdef CFG_BEEP

#include "core/cpu/cpu.h"

volatile uint32_t spkr_state = 0;
volatile uint32_t cnt_to_end = 0;

void beep_stop() {
  gpioSetValue(1, 1, 1); gpioSetValue(1, 2, 1); gpioSetDir(1, 1, 0); gpioSetDir(1, 2, 0);
  timer32Disable(1);
}

void beep(int fr, int len) {
  // Beeping using timer interrupt
  {
    IOCON_JTAG_TDO_PIO1_1 &= ~IOCON_JTAG_TDO_PIO1_1_FUNC_MASK;
    IOCON_JTAG_TDO_PIO1_1 |=  IOCON_JTAG_TDO_PIO1_1_FUNC_GPIO;
    IOCON_JTAG_nTRST_PIO1_2 &= ~IOCON_JTAG_nTRST_PIO1_2_FUNC_MASK;
    IOCON_JTAG_nTRST_PIO1_2 |=  IOCON_JTAG_nTRST_PIO1_2_FUNC_GPIO;
    gpioSetDir(1, 1, 1); gpioSetDir(1, 2, 1);
  }
  timer32Init(1, current_clock/fr);
  timer32Reset(1);
  cnt_to_end=len;
  timer32Enable(1);
  while(cnt_to_end);

  beep_stop();
}


#endif

/**************************************************************************/
/*! 
        @brief Interrupt handler for 32-bit timer 1
*/
/**************************************************************************/
void TIMER32_1_IRQHandler(void)
{  
  /* Clear the interrupt flag */
  TMR_TMR32B1IR = TMR_TMR32B1IR_MR0;

  /* If you wish to perform some action after each timer 'tick' (such as 
     incrementing a counter variable) you can do so here */
  timer32_1_counter++;

#ifdef CFG_BEEP
  if (cnt_to_end > 0) {
    cnt_to_end--;

    if (cnt_to_end) {
      gpioSetValue(1, 1, spkr_state);
      spkr_state = 1-spkr_state;
      gpioSetValue(1, 2, spkr_state);
    } else {
      spkr_state = cnt_to_end;
      gpioSetValue(1, 1, spkr_state);
      gpioSetValue(1, 2, spkr_state);
      timer32Disable(1);
    }
  }
#endif

  return;
}

/**************************************************************************/
/*! 
    @brief Enables the specified timer

    @param[in]  timerNum
                The 32-bit timer to enable (0..1)
*/
/**************************************************************************/
void timer32Enable(uint8_t timerNum)
{
  if ( timerNum == 0 )
  {
    TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_ENABLED;
  }

  else if (timerNum == 1)
  {
    TMR_TMR32B1TCR = TMR_TMR32B1TCR_COUNTERENABLE_ENABLED;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Disables the specified timer

    @param[in]  timerNum
                The 32-bit timer to disable (0..1)
*/
/**************************************************************************/
void timer32Disable(uint8_t timerNum)
{
  if ( timerNum == 0 )
  {
    TMR_TMR32B0TCR = TMR_TMR32B0TCR_COUNTERENABLE_DISABLED;
  }

  else if (timerNum == 1)
  {
    TMR_TMR32B1TCR = TMR_TMR32B1TCR_COUNTERENABLE_DISABLED;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Resets the specified timer

    @param[in]  timerNum
                The 32-bit timer to reset (0..1)
*/
/**************************************************************************/
void timer32Reset(uint8_t timerNum)
{
  uint32_t regVal;

  if ( timerNum == 0 )
  {
    regVal = TMR_TMR32B0TCR;
    regVal |= TMR_TMR32B0TCR_COUNTERRESET_ENABLED;
    TMR_TMR32B0TCR = regVal;
  }

  else if (timerNum == 1)
  {
    regVal = TMR_TMR32B1TCR;
    regVal |= TMR_TMR32B1TCR_COUNTERRESET_ENABLED;
    TMR_TMR32B1TCR = regVal;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief  Initialises the specified 32-bit timer, and configures the
            timer to raise an interrupt and reset on match on MR0.
    
    @param[in]  timerNum
                The 32-bit timer to initiliase (0..1)
    @param[in]  timerInterval
                The number of clock 'ticks' between resets (0..0xFFFFFFFF)

    @note   Care needs to be taken when configuring the timers since the
            pins are all multiplexed with other peripherals.  This code is
            provided as a starting point, but it will need to be adjusted
            according to your own situation and pin/peripheral requirements
*/
/**************************************************************************/
void timer32Init(uint8_t timerNum, uint32_t timerInterval)
{
  // If timerInterval is invalid, use the default value
  //if (timerInterval < 1)
  //{
  //  timerInterval = TIMER32_DEFAULTINTERVAL;
  //}

  if ( timerNum == 0 )
  {
    /* Enable the clock for CT32B0 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT32B0);

    /* The physical pins associated with CT32B0 are not enabled by
       default in order to avoid conflicts with other peripherals.  If
       you wish to use any of the pin-dependant functionality, simply
       uncomment the appropriate lines below.                             */

    /* Configure PIO1.5 as Timer0_32 CAP0 */
    // IOCON_PIO1_5 &= ~IOCON_PIO1_5_FUNC_MASK;
    // IOCON_PIO1_5 |= IOCON_PIO1_5_FUNC_CT32B0_CAP0;

    /* Configure PIO1.6 as Timer0_32 MAT0 */
    // IOCON_PIO1_6 &= ~IOCON_PIO1_6_FUNC_MASK;
    // IOCON_PIO1_6 |= IOCON_PIO1_6_FUNC_CT32B0_MAT0;

    /* Configure PIO1.7 as Timer0_32 MAT1 */
    // IOCON_PIO1_7 &= ~IOCON_PIO1_7_FUNC_MASK;
    // IOCON_PIO1_7 |= IOCON_PIO1_7_FUNC_CT32B0_MAT1;

    /* Configure PIO0.1 as Timer0_32 MAT2 */
    // IOCON_PIO0_1 &= ~IOCON_PIO0_1_FUNC_MASK;
    // IOCON_PIO0_1 |= IOCON_PIO0_1_FUNC_CT32B0_MAT2;

    /* Configure PIO0.11 as Timer0_32 MAT3 */
    /* Note: This pin can not be used with JTAG/SWD */
    // IOCON_JTAG_TDI_PIO0_11 &= ~IOCON_JTAG_TDI_PIO0_11_FUNC_MASK;
    // IOCON_JTAG_TDI_PIO0_11 |= IOCON_JTAG_TDI_PIO0_11_FUNC_CT32B0_MAT3;

    timer32_0_counter = 0;
    TMR_TMR32B0MR0 = timerInterval;

    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR32B0MCR = (TMR_TMR32B0MCR_MR0_INT_ENABLED | TMR_TMR32B0MCR_MR0_RESET_ENABLED);

    /* Enable the TIMER0 interrupt */
    NVIC_EnableIRQ(TIMER_32_0_IRQn);
  }

  else if ( timerNum == 1 )
  {
    /* Enable the clock for CT32B1 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT32B1);

    /* The physical pins associated with CT32B0 are not enabled by
       default in order to avoid conflicts with other peripherals.        */

    /* Configure PIO1.0 as Timer1_32 CAP0 */
    /* Note: This pint can not be used with JTAG/SWD */
    // IOCON_JTAG_TMS_PIO1_0 &= ~IOCON_JTAG_TMS_PIO1_0_FUNC_MASK;
    // IOCON_JTAG_TMS_PIO1_0 |= IOCON_JTAG_TMS_PIO1_0_FUNC_CT32B1_CAP0;

    /* Configure PIO1.1 as Timer1_32 MAT0 */
    /* Note: This pint can not be used with JTAG/SWD */
    // IOCON_JTAG_TDO_PIO1_1 &= ~IOCON_JTAG_TDO_PIO1_1_FUNC_MASK;
    // IOCON_JTAG_TDO_PIO1_1 |= IOCON_JTAG_TDO_PIO1_1_FUNC_CT32B1_MAT0;

    /* Configure PIO1.2 as Timer1_32 MAT1 */
    /* Note: This pint can not be used with JTAG/SWD */
    // IOCON_JTAG_nTRST_PIO1_2 &= ~IOCON_JTAG_nTRST_PIO1_2_FUNC_MASK;
    // IOCON_JTAG_nTRST_PIO1_2 |= IOCON_JTAG_nTRST_PIO1_2_FUNC_CT32B1_MAT1;

    /* Configure PIO1.3 as Timer1_32 MAT2 */
    /* Note: This pint can not be used with JTAG/SWD */
    // IOCON_SWDIO_PIO1_3 &= ~IOCON_SWDIO_PIO1_3_FUNC_MASK;
    // IOCON_SWDIO_PIO1_3 |= IOCON_SWDIO_PIO1_3_FUNC_CT32B1_MAT2;

    /* Configure PIO1.4 as Timer1_32 MAT3 */
    // IOCON_PIO1_4 &= ~IOCON_PIO1_4_FUNC_MASK;
    // IOCON_PIO1_4 |= IOCON_PIO1_4_FUNC_CT32B1_MAT3;

    timer32_1_counter = 0;
    TMR_TMR32B1MR0 = timerInterval;

    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR32B1MCR = (TMR_TMR32B1MCR_MR0_INT_ENABLED | TMR_TMR32B1MCR_MR0_RESET_ENABLED);

    /* Enable the TIMER1 Interrupt */
    NVIC_EnableIRQ(TIMER_32_1_IRQn);
  }
  return;
}


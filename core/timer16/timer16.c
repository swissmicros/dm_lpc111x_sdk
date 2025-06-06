
#include "timer16.h"

volatile uint32_t timer16_0_counter = 0;
volatile uint32_t timer16_1_counter = 0;

#ifdef CFG_PWM
  volatile uint32_t pwmCounter = 0;
  extern volatile uint32_t pwmMaxPulses;    // See drivers/pwm/pwm.c
#endif

/**************************************************************************/
/*! 
    @brief  Causes a blocking delay for the specified number of
            clock ticks.  
            
    @note   The exact duration of this delay depends on the speed of the
            system clock, but it will invariably be short because of the
            16-bit limitation.  For example, on a system with a 12MHz
            clock, a 1mS delay would be equal to 12,000 ticks.  Thus, the
            maximum delay measured in mS with a 12MHz clock is ~5.46mS.
            
    @param[in]  timerNum
                The 16-bit timer to user (0..1)
    @param[in]  delayInTicks
                The number of clock ticks to delay (0..65534)

    @section Example

    @code
    #include "/core/cpu/cpu.h"
    #include "/core/timer16/timer16.h"

    int main(void)
    {
        cpuInit();

        // Initialise timer 0 ... delay is provided but not used here
        timer16Init(0, 0xFFFF);

        // Enable the timer
        timer16Enable(0);

        while(1)
        {
            // Cause blocking delay for 12000 ticks (1mS @ 12MHz)
            // Note: The delay must be 65534 or less (16-bit value)
            timer16DelayTicks(0, 12000);
        }
    }
    @endcode
*/
/**************************************************************************/
void timer16DelayTicks(uint8_t timerNum, uint16_t delayInTicks)
{
  // ToDo: Verify incoming value

  if (timerNum == 0)
  {
    /* Reset the timer */
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERRESET_ENABLED;

    /* Set the prescaler to zero */
    TMR_TMR16B0PR  = 0x00;

    TMR_TMR16B0MR0 = delayInTicks;

    /* Reset all interrupts */
    TMR_TMR16B0IR  = TMR_TMR16B0IR_MASK_ALL;

    /* Stop timer on match (MR0) */
    TMR_TMR16B0MCR = TMR_TMR16B0MCR_MR0_STOP_ENABLED;

    /* Start timer */
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_ENABLED;

    /* Wait until the delay time has elapsed */
    while (TMR_TMR16B0TCR & TMR_TMR16B0TCR_COUNTERENABLE_ENABLED);
  }

  else if (timerNum == 1)
  {
    /* Reset the timer */
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERRESET_ENABLED;

    /* Set the prescaler to zero */
    TMR_TMR16B1PR  = 0x00;

    TMR_TMR16B1MR0 = delayInTicks;

    /* Reset all interrupts */
    TMR_TMR16B1IR  = TMR_TMR16B1IR_MASK_ALL;

    /* Stop timer on match (MR0) */
    TMR_TMR16B1MCR = TMR_TMR16B1MCR_MR0_STOP_ENABLED;

    /* Start timer */
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_ENABLED;

    /* Wait until the delay time has elapsed */
    while (TMR_TMR16B1TCR & TMR_TMR16B1TCR_COUNTERENABLE_ENABLED);
  }

  return;
}

/**************************************************************************/
/*! 
    @brief      Causes a blocking delay for the specified number of
                microseconds
            
    @warning    The maximum delay in uS will depend on the clock speed,
                but running at 12MHz the maximum delay (MR = 0xFFFF)
                would be 5461uS (0xFFFF / 12 = 5461.25), or 5.46
                milliseconds.

    @param[in]  timerNum
                The 16-bit timer to user (0..1)
    @param[in]  delayInUs
                The number of microseconds to wait

    @section Example

    @code
    #include "/core/cpu/cpu.h"
    #include "/core/timer16/timer16.h"

    int main(void)
    {
        cpuInit();

        // Initialise timer 0 ... delay is provided but not used here
        timer16Init(0, 0xFFFF);

        // Enable the timer
        timer16Enable(0);

        while(1)
        {
            // Cause blocking delay for 500 microseconds (0.5mS)
            timer16DelayUS(0, 500);
        }
    }
    @endcode
*/
/**************************************************************************/
#if 0
void timer16DelayUS(uint8_t timerNum, uint16_t delayInUS)
{
  // ToDo: Check if the appropriate timer is enabled first?

  if (timerNum == 0)
  {
    /* Reset the timer */
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERRESET_ENABLED;

    /* Set the prescaler to zero */
    TMR_TMR16B0PR  = 0x00;

    TMR_TMR16B0MR0 = delayInUS * ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV)/1000000);

    /* Reset all interrupts */
    TMR_TMR16B0IR  = TMR_TMR16B0IR_MASK_ALL;

    /* Stop timer on match (MR0) */
    TMR_TMR16B0MCR = TMR_TMR16B0MCR_MR0_STOP_ENABLED;

    /* Start timer */
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_ENABLED;

    /* Wait until the delay time has elapsed */
    while (TMR_TMR16B0TCR & TMR_TMR16B0TCR_COUNTERENABLE_ENABLED);
  }

  else if (timerNum == 1)
  {
    /* Reset the timer */
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERRESET_ENABLED;

    /* Set the prescaler to zero */
    TMR_TMR16B1PR  = 0x00;

    TMR_TMR16B1MR0 = delayInUS * ((CFG_CPU_CCLK/SCB_SYSAHBCLKDIV)/1000000);

    /* Reset all interrupts */
    TMR_TMR16B1IR  = TMR_TMR16B1IR_MASK_ALL;

    /* Stop timer on match (MR0) */
    TMR_TMR16B1MCR = TMR_TMR16B1MCR_MR0_STOP_ENABLED;

    /* Start timer */
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_ENABLED;

    /* Wait until the delay time has elapsed */
    while (TMR_TMR16B1TCR & TMR_TMR16B1TCR_COUNTERENABLE_ENABLED);
  }

  return;
}
#endif

/**************************************************************************/
/*! 
    @brief Interrupt handler for 16-bit timer 0
*/
/**************************************************************************/
void TIMER16_0_IRQHandler(void)
{  
  /* Clear the interrupt flag */
  TMR_TMR16B0IR = TMR_TMR16B0IR_MR0;

  /* Increment timer counter by 1 (it will automatically roll back to 0) */
  timer16_0_counter++;

  return;
}

/**************************************************************************/
/*! 
    @brief Interrupt handler for 16-bit timer 1
*/
/**************************************************************************/
void TIMER16_1_IRQHandler(void)
{  
  /* Clear the interrupt flag */
  TMR_TMR16B1IR = TMR_TMR16B1IR_MR0;

  /* Increment timer counter by 1 (it will automatically roll back to 0) */
  timer16_1_counter++;

  #ifdef CFG_PWM
  /* Check if the PWM output should be disabled after pwmMaxPulses pulses */
  /* See "drivers/pwm/pwm.c" */
  if (TMR_TMR16B1IR & TMR_TMR16B1IR_MR3)
  {
    /* Clear the interrupt flag */
    TMR_TMR16B1IR = TMR_TMR16B1IR_MR3;

    if (pwmMaxPulses > 0)
    {
      pwmCounter++;
      if (pwmCounter == pwmMaxPulses)
      {
        /* Disable interrupt on MR3 */
        TMR_TMR16B1MCR  &= ~(TMR_TMR16B1MCR_MR3_INT_MASK);
        /* Disable Timer */
        TMR_TMR16B1TCR &= ~(TMR_TMR16B1TCR_COUNTERENABLE_MASK);
        /* Reset the counter variables */
        pwmCounter = 0;
        pwmMaxPulses = 0;
      }
    }
  }
  #endif

  return;
}

/**************************************************************************/
/*! 
    @brief Enables the specified timer

    @param[in]  timerNum
                The 16-bit timer to enable (0..1)
*/
/**************************************************************************/
void timer16Enable(uint8_t timerNum)
{
  if ( timerNum == 0 )
  {
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_ENABLED;
  }

  else if (timerNum == 1)
  {
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_ENABLED;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Disables the specified timer

    @param[in]  timerNum
                The 16-bit timer to disable (0..1)
*/
/**************************************************************************/
void timer16Disable(uint8_t timerNum)
{
  if ( timerNum == 0 )
  {
    TMR_TMR16B0TCR = TMR_TMR16B0TCR_COUNTERENABLE_DISABLED;
  }

  else if (timerNum == 1)
  {
    TMR_TMR16B1TCR = TMR_TMR16B1TCR_COUNTERENABLE_DISABLED;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Resets the specified timer

    @param[in]  timerNum
                The 16-bit timer to reset (0..1)
*/
/**************************************************************************/
void timer16Reset(uint8_t timerNum)
{
  uint32_t regVal;

  if ( timerNum == 0 )
  {
    regVal = TMR_TMR16B0TCR;
    regVal |= TMR_TMR16B0TCR_COUNTERRESET_ENABLED;
    TMR_TMR16B0TCR = regVal;
  }

  else if (timerNum == 1)
  {
    regVal = TMR_TMR16B1TCR;
    regVal |= TMR_TMR16B1TCR_COUNTERRESET_ENABLED;
    TMR_TMR16B1TCR = regVal;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief  Initialises the specified 16-bit timer, sets the timer
            interval, resets the timer, and configures the interrupt
            handler.
    
    Initialises a 16-bit timer with the supplied timer interval (the
    amount of time that passes between each timer 'tick').  Every time that
    this interval elapses, the timer's interrupt will be fired and the
    appropriate counter variable will be incremented by one (For example,
    with CT16B0, 'timer16_0_counter' would be incremented).

    @param[in]  timerNum
                The 16-bit timer to initiliase (0..1)
    @param[in]  timerInterval
                The number of clock 'ticks' between resets (0..65534)

    @warning    Care needs to be taken when configuring the timers since
                the pins are all multiplexed with other peripherals.  This
                code is provided as a starting point, but it will need to
                be adjusted according to your own situation and
                pin/peripheral requirements
*/
/**************************************************************************/
void timer16Init(uint8_t timerNum, uint16_t timerInterval)
{
  // If timerInterval is invalid, use the default value
  if (timerInterval < 1)
  {
    timerInterval = TIMER16_DEFAULTINTERVAL;
  }

  if ( timerNum == 0 )
  {
    /* Enable the clock for CT16B0 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT16B0);

    /* The physical pins associated with CT16B0 are not enabled by
       default in order to avoid conflicts with other peripherals.  
       Pin 0.10 (CT16B0_MAT2), for example, can not be used while
       debugging with a hardware debugger.  If you require one or
       more of these pins, simply uncomment the code below                */

    /* Configure PIO0.2 as Timer0_16 CAP0 */
    // IOCON_PIO0_2 &= ~IOCON_PIO0_2_FUNC_MASK;
    // IOCON_PIO0_2 |= IOCON_PIO0_2_FUNC_CT16B0_CAP0;

    /* Configure PIO0.8 as Timer0_16 MAT0 */
    // IOCON_PIO0_8 &= ~IOCON_PIO0_8_FUNC_MASK;
    // IOCON_PIO0_8 |= IOCON_PIO0_8_FUNC_CT16B0_MAT0;

    /* Configure PIO0.9 as Timer0_16 MAT1 */
    // IOCON_PIO0_9 &= ~IOCON_PIO0_9_FUNC_MASK;
    // IOCON_PIO0_9 |= IOCON_PIO0_9_FUNC_CT16B0_MAT1;

    /* Configure PIO0.10 as Timer0_16 MAT3 */
    // IOCON_JTAG_TCK_PIO0_10 &= ~IOCON_JTAG_TCK_PIO0_10_FUNC_MASK;
    // IOCON_JTAG_TCK_PIO0_10 |= IOCON_JTAG_TCK_PIO0_10_FUNC_CT16B0_MAT2;

    timer16_0_counter = 0;
    TMR_TMR16B0MR0 = timerInterval;

    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR16B0MCR = (TMR_TMR16B0MCR_MR0_INT_ENABLED | TMR_TMR16B0MCR_MR0_RESET_ENABLED);

    /* Enable the TIMER0 interrupt */
    NVIC_EnableIRQ(TIMER_16_0_IRQn);
  }

  else if ( timerNum == 1 )
  {
    /* Enable the clock for CT16B1 */
    SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_CT16B1);

    /* The physical pins associated with CT16B0 are not enabled by
       default in order to avoid conflicts with other peripherals.  
       Pin 0.10 (CT16B0_MAT2), for example, can not be used while
       debugging with a hardware debugger.  If you require one or
       more of these pins, simply uncomment the code below                */

    /* Configure PIO1.8 as Timer1_16 CAP0 */
    // IOCON_PIO1_8 &= ~IOCON_PIO1_8_FUNC_MASK;
    // IOCON_PIO1_8 |= IOCON_PIO1_8_FUNC_CT16B1_CAP0;

    /* Configure PIO1.9 as Timer1_16 MAT0 */
    // IOCON_PIO1_9 &= ~IOCON_PIO1_9_FUNC_MASK;
    // IOCON_PIO1_9 |= IOCON_PIO1_9_FUNC_CT16B1_MAT0;

    /* Configure PIO1.10 as Timer1_16 MAT1 */
    // IOCON_PIO1_10 &= ~IOCON_PIO1_10_FUNC_MASK;
    // IOCON_PIO1_10 |= IOCON_PIO1_10_FUNC_CT16B1_MAT1;

    timer16_1_counter = 0;
    TMR_TMR16B1MR0 = timerInterval;

    /* Configure match control register to raise an interrupt and reset on MR0 */
    TMR_TMR16B1MCR = (TMR_TMR16B1MCR_MR0_INT_ENABLED | TMR_TMR16B1MCR_MR0_RESET_ENABLED);

    /* Enable the TIMER1 Interrupt */
    NVIC_EnableIRQ(TIMER_16_1_IRQn);
  }
  return;
}

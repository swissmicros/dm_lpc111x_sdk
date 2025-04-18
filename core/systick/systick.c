
#include "systick.h"

volatile uint32_t systickTicks = 0;
volatile uint32_t systickRollovers = 0;

#ifdef CFG_RTC
volatile int rtc_last_ms_cnt = 0;
volatile int rtc_ms_cnt = 0;  
#endif

#include "core/gpio/gpio.h"

/**************************************************************************/
/*! 
    @brief Systick interrupt handler
*/
/**************************************************************************/
void SysTick_Handler (void)
{
  // Increment rollover counter
  if (systickTicks == 0xFFFFFFFF) systickRollovers++;
  systickTicks++;
#ifdef CFG_RTC
  rtc_ms_cnt++;
#endif  
}

/**************************************************************************/
/*! 
    @brief      Initialises the systick timer

    @param[in]  delayMs
                The number of milliseconds between each tick of the systick
                timer.
                  
    @note       The shortest possible delay is 1 millisecond, which will 
                allow fine grained delays, but will cause more load on the
                system than a 10mS delay.  The resolution of the systick
                timer needs to be balanced with the amount of processing
                time you can spare.  The delay should really only be set
                to 1 mS if you genuinely have a need for 1mS delays,
                otherwise a higher value like 5 or 10 mS is probably
                more appropriate.
*/
/**************************************************************************/
void systickInit (uint32_t ticks)
{
  // Check if 'ticks' is greater than maximum value
  if (ticks > SYSTICK_STRELOAD_MASK)
  {
    return;
  }

  // Reset counter
  systickTicks = 0;
                     
  // Set reload register
  SYSTICK_STRELOAD  = (ticks & SYSTICK_STRELOAD_MASK) - 1;

  // Load the systick counter value
  SYSTICK_STCURR = 0;

  // Enable systick IRQ and timer
  SYSTICK_STCTRL = SYSTICK_STCTRL_CLKSOURCE |
                   SYSTICK_STCTRL_TICKINT |
                   SYSTICK_STCTRL_ENABLE;
}

/**************************************************************************/
/*! 
    @brief      Causes a blocking delay for 'delayTicks' ticks on the
                systick timer.  For example: systickDelay(100) would cause
                a blocking delay for 100 ticks of the systick timer.

    @param[in]  delayTicks
                The number of systick ticks to cause a blocking delay for

    @Note       This function takes into account the fact that the tick
                counter may eventually roll over to 0 once it reaches
                0xFFFFFFFF.
*/
/**************************************************************************/
void systickDelay (uint32_t delayTicks) 
{
  uint32_t curTicks;
  curTicks = systickTicks;

  // Make sure delay is at least 1 tick in case of division, etc.
  if (delayTicks == 0) delayTicks = 1;

  if (curTicks > 0xFFFFFFFF - delayTicks)
  {
    // Rollover will occur during delay
    while (systickTicks >= curTicks)
    {
      while (systickTicks < (delayTicks - (0xFFFFFFFF - curTicks)));
    }      
  }
  else
  {
    while ((systickTicks - curTicks) < delayTicks);
  }
}

/**************************************************************************/
/*! 
    @brief      Returns the current value of the systick timer counter. 
                This value is incremented by one every time an interrupt
                fires for the systick timer.
*/
/**************************************************************************/
uint32_t systickGetTicks(void)
{
  return systickTicks;
}

/**************************************************************************/
/*! 
    @brief      Returns the current value of the systick timer rollover 
                counter. This value is incremented by one every time the
                tick counter rolls over from 0xFFFFFFFF to 0.
*/
/**************************************************************************/
uint32_t systickGetRollovers(void)
{
  return systickRollovers;
}

/**************************************************************************/
/*! 
    @brief      Returns the approximate number of seconds that the
                systick timer has been running.
*/
/**************************************************************************/
uint32_t systickGetSecondsActive(void)
{
  uint32_t currentTick = systickTicks;
  uint32_t rollovers = systickRollovers;
  uint32_t secsActive = currentTick / (1000 / CFG_SYSTICK_DELAY_IN_MS);
  secsActive += rollovers * (0xFFFFFFFF / (1000 / CFG_SYSTICK_DELAY_IN_MS));

  return secsActive;
}


#include "projectconfig.h"

#include "core/cpu/cpu.h"
#include "core/gpio/gpio.h"
#include "core/systick/systick.h"



int present = -1;


void vref_idle() {
  setpin(1, 11, 1, 0);
  setpin(3, 2, 1, 0);
}



int get_ref_value(int pin_value) {
  REG32 regVal;

  /* Disable Power down bit to the ADC block. */
  SCB_PDRUNCFG &= ~(SCB_PDRUNCFG_ADC);

  /* Enable AHB clock to the ADC. */
  SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_ADC);

  /* AD7 pin as input */
  setpin(1, 11, 0, 0);

  /* Set AD7 to analog input */
  IOCON_PIO1_11 &=  ~(IOCON_PIO1_11_ADMODE_MASK |
                      IOCON_PIO1_11_FUNC_MASK |
                      IOCON_PIO1_11_MODE_MASK);
  IOCON_PIO1_11 |=   (IOCON_PIO1_11_FUNC_AD7 &
                      IOCON_PIO1_11_ADMODE_ANALOG);

#define ADC_FREQ (4000 * 1000) // 4MHz

  ADC_AD0CR = (ADC_AD0CR_SEL_AD7               |  /* SEL=1,select channel 0 on ADC0 */
              ((current_clock/ADC_FREQ) << 8 ) |  /* CLKDIV =  as big to get ADC_FREQ from current_clock */
              ADC_AD0CR_BURST_SWMODE           |  /* BURST = 0, no BURST, software controlled */
              ADC_AD0CR_CLKS_10BITS            |  /* CLKS = 0, 11 clocks/10 bits */
              ADC_AD0CR_START_NOSTART          |  /* START = 0 A/D conversion stops */
              ADC_AD0CR_EDGE_RISING);             /* EDGE = 0 (CAP/MAT signal falling, trigger A/D conversion) */

  /* Pin to vref ON */
  gpioSetValue(3, 2, pin_value);

  systickDelay(20);

  /* deselect all channels */
  ADC_AD0CR &= ~ADC_AD0CR_SEL_MASK;

  /* switch channel and start converting */
  ADC_AD0CR |= (ADC_AD0CR_START_STARTNOW | ADC_AD0CR_SEL_AD7);

  /* wait for conversion end */
  for(;;) {
    regVal = *(pREG32(ADC_AD0DR7));
    if (regVal & ADC_DR_DONE) break;
  }

  /* stop ADC */
  ADC_AD0CR &= ~ADC_AD0CR_START_MASK;

  /* Pin to vref OFF */
  vref_idle();

  return (regVal >> 6) & 0x3FF;
}



int is_vref() {
  if (present >= 0)
    return present;

  // AD7 pin have to follow values on the 3.2
  present=0;

  if ( get_ref_value(0) > 10 ) goto done;
  {
    int val = get_ref_value(1);
    if ( val < 0x100 || val > 0x280 ) goto done;
  }
  present=1;
done:
  vref_idle();

  return present;
}



int battery_voltage() {
  int val;

  // When the vref hw isn't present return 0
  if (!is_vref())
    return 0;

  val = get_ref_value(1);

  return val?(CFG_REF_VOLTAGE*0x3ff)/val:0;
}


int is_lowbat = 0;

int battery_voltage_low() {
  if (!is_vref())
    return 0;
  is_lowbat = battery_voltage() <= CFG_LOWBAT_VOLTAGE;
  return is_lowbat;
}

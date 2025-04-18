#ifndef __VREF__
#define __VREF__

// Returns 0/1 whether the vref hw is present
int is_vref();

// Read battery voltage in mV.
// The value depends on the CFG_VREF_VOLTAGE which have to be
// set acording to the actual voltage reference hw.
int battery_voltage();

// Returns 0/1 whether the battery is OK or low respectively
int battery_voltage_low();

// Static value with last result of battery_woltage_low
extern int is_lowbat;

// Set vref related pins to idle state
int vref_idle();

// Read the vref adc value
int get_ref_value(int pin_value);

#endif

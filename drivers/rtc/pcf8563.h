/*

 PCF8563 - RTC chip
 
 */

#ifndef __PCF8563_H__
#define __PCF8563_H__


#include "projectconfig.h"
#include "core/i2c/i2c.h"

#define PCF8563_ADDR     0xA2
#define PCF8563_RD_ADDR (PCF8563_ADDR | 1)

#define PCF8563_CTL_STAT1     0
#define PCF8563_CTL_STAT2     1
#define PCF8563_VL_SECONDS    2
#define PCF8563_C_MONTH       7
#define PCF8563_MINUTE_ALARM  9
#define PCF8563_TIMER_CONTROL 0xe

// Masks
#define PCF8563_VL_SECONDS_VL 0x80
#define PCF8563_SECONDS       0x7f
#define PCF8563_CTL_STAT2_AF  8
#define PCF8563_CTL_STAT2_TF  4
#define PCF8563_CTL_STAT2_AIE 2
#define PCF8563_CTL_STAT2_TIE 1

// Just for debug purposes
extern int i2c_retry;

// Set by pcf8563_init()
extern int pcf8563_present;

// Detects presence of the pcf8563
void pcf8563_init();

#define DEFAULT_I2C_RETRY_COUNT 3

// Access read data in I2CDataRead byte array
uint32_t pcf8563_read_reg(uint8_t reg, int count);
uint32_t pcf8563_read_reg_retry(uint8_t reg, int count);

// Write data to send into I2CDataToSend byte array
uint32_t pcf8563_write_reg(uint8_t reg, int count);
uint32_t pcf8563_write_reg_retry(uint8_t reg, int count);

// High level write
int pcf8563_write_val(uint8_t reg, int count, uint32_t val);
int pcf8563_write_val2(uint8_t reg, int count, uint32_t val, uint32_t val2);
// High level read
int pcf8563_read_val(uint8_t reg, int count);

// Read and clear interrupt flags from control status 2
int pcf8563_read_and_clear_status();

// Access read data in I2CDataRead byte array
// In order: sec, min, hours, day, wday, year-2000
int pcf8563_read_time();
int pcf8563_write_time();

// Check for ALARM
int pcf8563_is_alarm();
// Clear ALARM flag
int pcf8563_clear_alarm();

// Enable interrupt from ALARM flag
int pcf8563_enable_alarm();

#endif

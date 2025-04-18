
#include "projectconfig.h"

#include "pcf8563.h"

#define R I2CDataRead
#define W I2CDataToSend

#define READ_REG(reg) pcf8563_read_reg_retry(reg,1)
#define WRITE_REG(reg, val) W[0]=(val), pcf8563_write_reg_retry(reg,1)

int pcf8563_present = 0;


uint32_t pcf8563_read_reg(uint8_t reg, int count) {
  I2CWriteLength = 2;
  I2CReadLength = count;
  I2CMasterBuffer[0] = PCF8563_ADDR;             // I2C device address
  I2CMasterBuffer[1] = reg;                      // Command register
  // Append address w/read bit
  I2CMasterBuffer[2] = PCF8563_RD_ADDR;

  return  i2cEngine();
}

uint32_t pcf8563_write_reg(uint8_t reg, int count) {
  I2CWriteLength = 2+count;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = PCF8563_ADDR;             // I2C device address
  I2CMasterBuffer[1] = reg;                      // Command register

  return  i2cEngine();
}



uint32_t pcf8563_read_reg_retry(uint8_t reg, int count) {
  int i2c_retry;
  uint32_t ret;
  for( i2c_retry = DEFAULT_I2C_RETRY_COUNT; i2c_retry; i2c_retry--) {
    ret = pcf8563_read_reg(reg, count);
    if ( ret == I2CSTATE_ACK ) break;
  }
  return ret;
}


uint32_t pcf8563_write_reg_retry(uint8_t reg, int count) {
  int i2c_retry;
  uint32_t ret;
  for( i2c_retry = DEFAULT_I2C_RETRY_COUNT; i2c_retry; i2c_retry--) {
    ret = pcf8563_write_reg(reg, count);
    if ( ret == I2CSTATE_ACK ) break;
  }
  return ret;
}

int pcf8563_write_val(uint8_t reg, int count, uint32_t val) {
  if(!pcf8563_present) return 0;

  uint32_t* b = (uint32_t*)I2CDataToSend;
  b[0] = val;

  return pcf8563_write_reg_retry(reg, count) == I2CSTATE_ACK;
}

int pcf8563_write_val2(uint8_t reg, int count, uint32_t val, uint32_t val2) {
  if(!pcf8563_present) return 0;

  uint32_t* b = (uint32_t*)I2CDataToSend;
  b[0] = val;
  b[1] = val2;

  return pcf8563_write_reg_retry(reg, count) == I2CSTATE_ACK;
}

int pcf8563_read_val(uint8_t reg, int count) {
  if ( pcf8563_present && pcf8563_read_reg_retry(reg, count) == I2CSTATE_ACK )
    return 1;
  return 0;
}


// ----------------


void pcf8563_init() {
  // Detect presence of the PCF8563 and clear possible pending interrupts
  pcf8563_read_and_clear_status();
  
  if ( READ_REG(PCF8563_VL_SECONDS) == I2CSTATE_ACK ) {
    
    // Init clock to 1.1.2014 8:00 if lowbat flag detected
    if ( R[0] & PCF8563_VL_SECONDS_VL ) {
      // Low voltage detected - clock integrity corrupted
      pcf8563_write_val2(PCF8563_VL_SECONDS, 7, 0x01080000, 0x140102);
    }

    // Disable all alarms
    pcf8563_write_val(PCF8563_MINUTE_ALARM, 4, 0x80808080);
  }
}


int pcf8563_read_time() {
  if(!pcf8563_present) return 0;

  return pcf8563_read_reg_retry(PCF8563_VL_SECONDS, 7) == I2CSTATE_ACK;
}

int pcf8563_write_time() {
  if(!pcf8563_present) return 0;

  return pcf8563_write_reg_retry(PCF8563_VL_SECONDS, 7) == I2CSTATE_ACK;
}


int pcf8563_read_and_clear_status() {
  int ret = -1;
  if ( READ_REG(PCF8563_CTL_STAT2) == I2CSTATE_ACK ) {
    pcf8563_present = 1;
    ret = R[0];
    if ( R[0] & 0xf ) {
      WRITE_REG(PCF8563_CTL_STAT2, R[0] & 3); // Preserve IE flags
    }
  }
  return ret;
}


// --- Old functions for simple clock code ---
#if 0

int pcf8563_is_alarm() {
  return READ_REG(PCF8563_CTL_STAT2) == I2CSTATE_ACK ? R[0] & PCF8563_CTL_STAT2_AF : -1;
}

int pcf8563_clear_alarm() {
  int e;
  uint32_t* b = (uint32_t*)I2CDataToSend;

  if(!pcf8563_present) return 0;
  
  e = WRITE_REG(PCF8563_CTL_STAT2, 0);
  if ( e != I2CSTATE_ACK )
    return e;

  b[0] = 0x80808080; // Clear alarm
  return pcf8563_write_reg_retry(PCF8563_MINUTE_ALARM, 4);
}


int pcf8563_enable_alarm() {
  return WRITE_REG(PCF8563_CTL_STAT2, PCF8563_CTL_STAT2_AIE);
}
#endif
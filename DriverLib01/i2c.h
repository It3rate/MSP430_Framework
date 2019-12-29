#ifndef I2C_H_
#define I2C_H_

#include "driverlib.h"

#define I2C_IS_MASTER

#define I2C_BUFFER_LENGTH 32
uint8_t i2cDataIn[I2C_BUFFER_LENGTH];
uint8_t i2cDataOut[I2C_BUFFER_LENGTH];

uint16_t _slaveAddress;

#endif /* I2C_H_ */

#ifndef I2C_H_
#define I2C_H_

#include "driverlib.h"

#define I2C_IS_MASTER

typedef struct {
    uint16_t slaveAddress;
    uint8_t *data;
    uint8_t counter;
    uint8_t totalLength;
} i2c_info;

static i2c_info i2c = {};

#define I2C_BUFFER_LENGTH 32
uint8_t i2cDataIn[I2C_BUFFER_LENGTH];

//uint16_t _slaveAddress;

#endif /* I2C_H_ */

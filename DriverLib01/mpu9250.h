#ifndef MPU9250_H_
#define MPU9250_H_

#include <msp430.h>
#include "i2c.h"

static i2cInstance i2c_gyro = {
    .slaveAddress = 0x68
};

#define I2C_BUFFER_LENGTH 32
uint8_t gyroData[I2C_BUFFER_LENGTH];

void mpu9250_init(void);
uint8_t* mpu9250_readSensors();

#endif /* MPU9250_H_ */

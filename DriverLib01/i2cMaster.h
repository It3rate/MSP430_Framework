#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_
#include "i2c.h"

#ifdef I2C_IS_MASTER
#include "driverlib.h"



void i2c_init(uint16_t slaveAddress);

void i2c_write(uint8_t data[32], uint8_t transmitLength);
void i2c_read(uint8_t data[32], uint8_t receiveLength);

#endif // I2C_IS_MASTER

#endif /* I2C_MASTER_H_ */

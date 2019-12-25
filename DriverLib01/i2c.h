#ifndef I2C_H_
#define I2C_H_

#include "driverlib.h"

bool i2c_isMaster;
bool i2c_isTransmitMode;

void i2c_init(uint16_t slaveAddress);
void i2c_updateState();

void i2c_transmitValues(uint8_t data[32], uint8_t transmitLength);
void i2c_receiveValues(uint8_t receiveLength);


#endif /* I2C_H_ */

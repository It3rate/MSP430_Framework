#ifndef I2C_H_
#define I2C_H_

#include "driverlib.h"

void initI2c(bool isMaster, uint16_t slaveAddress);

void transmitValues(uint8_t data[32], uint8_t transmitLength);
void receiveValues(uint8_t receiveLength);


#endif /* I2C_H_ */

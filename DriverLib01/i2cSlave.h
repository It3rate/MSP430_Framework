#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

#include "i2c.h"

#ifndef I2C_IS_MASTER

void i2c_initSlave(uint16_t slaveAddress);

#endif // NOT I2C_IS_MASTER

#endif /* I2CSLAVE_H_ */

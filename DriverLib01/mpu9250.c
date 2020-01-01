#include "mpu9250.h"
#include "i2c.h"

uint8_t WHOAMI[1] = {0x75};
uint8_t SENSORS[1] = {0x3B};
uint8_t sendIndex = 0;

void mpu9250_init(void)
{
}

uint8_t* mpu9250_readSensors()
{
    i2c_write(i2c_gyro, &SENSORS[0], 1);
    __bis_SR_register(LPM0_bits + GIE);
    i2c_read(i2c_gyro, gyroData, 16);
    __bis_SR_register(LPM0_bits + GIE);
    return gyroData;
}

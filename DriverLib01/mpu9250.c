#include "mpu9250.h"
#include "i2c.h"

void initMpu9250(void)
{
    i2c_isMaster = true;
    i2c_init(0x68); // address 0x69 if pin AD0 is logic high
}

void start()
{
//    while (1)
//    {
//        __delay_cycles(1000);
//        evaluateI2c();
//        if(i2c_isTransmitMode)
//        {
//            i2c_transmitValues(transmitData, dataLength);
//        }
//        else
//        {
//            i2c_receiveValues(dataLength);
//        }
//        __bis_SR_register(LPM0_bits + GIE);
//
//        if(i2c_isMaster && !i2c_isTransmitMode)
//        {
//            //printf("data: %s\n", (i2cData+1));
//        }
//    }
}

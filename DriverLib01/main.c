#include <msp430.h>
#include <stdio.h>
#include "driverlib.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "i2c.h"
#include "i2cMaster.h"
#include "i2cSlave.h"

void initGPIO(void);
void initTimers(void);

#define DATA0LENGTH 9
#define DATA1LENGTH 8
//uint8_t transmitData0[DATA0LENGTH] = {0x52, 0x6f, 0x62, 0x69, 0x6e, 0x41, 0x42, 0x43, 0x5A};
//uint8_t transmitData1[DATA1LENGTH] = {0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x59};
uint8_t WHOAMI[1] = {0x75};
uint8_t SENSORS[1] = {0x3B};
uint8_t *transmitData;
uint8_t dataLength = 0;
uint8_t sendIndex = 0;

bool i2c_isTransmitMode = true;

int main(void)
{
    WDT_A_hold(WDT_A_BASE);

    initGPIO();
    initClocks(1);
    printClockSpeeds();
    enableGPIOInterrupts();

#ifdef I2C_IS_MASTER
    initTimers();
    //i2c_initMaster(0x48);
    //i2c_initMaster(0x34);
    i2c_initMaster(0x68);

    while (1)
    {
        switch(sendIndex)
        {
        case 0:
            //i2c_masterTransmitByte(transmitData0[0]);
            i2c_masterTransmitMultibyte(&SENSORS[0], 1);
            //printf("out: %s\n", i2cDataOut);
            break;
        case 1:
            i2c_masterReceiveMultibyte(14);
            //printf("in: %s\n", i2cDataIn);
            break;
//        case 2:
//            //i2c_masterTransmitByte(transmitData1[0]);
//            i2c_masterTransmitMultibyte(&transmitData1[2], 1);
//            break;
//        case 3:
//            i2c_masterTransmitMultibyte(transmitData1, DATA1LENGTH);
//            break;
        }
        __bis_SR_register(LPM0_bits + GIE);

        sendIndex++;
        if(sendIndex > 1)
        {
            sendIndex = 0;

            printf("accel x%04X y%04X z%04X  temp: %04X  gyro: x%04X y%04X z%04X  \n",
               (i2cDataIn[0]<<8)+i2cDataIn[1], (i2cDataIn[2]<<8)+i2cDataIn[3],  (i2cDataIn[4]<<8)+i2cDataIn[5],
               (i2cDataIn[6]<<8)+i2cDataIn[7],
               (i2cDataIn[8]<<8)+i2cDataIn[9], (i2cDataIn[10]<<8)+i2cDataIn[11],  (i2cDataIn[12]<<8)+i2cDataIn[13]);
        }
        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
        __bis_SR_register(LPM0_bits + GIE);
    }
#else
    i2c_initSlave(0x48);
    while(1)
    {
        __bis_SR_register(LPM0_bits + GIE);
    }
#endif


}

void initGPIO(void)
{
    addGPIOOutput(GPIO_PORT_P1, GPIO_PIN0, false);
    addGPIOOutput(GPIO_PORT_P4, GPIO_PIN7, true);
    addGPIOInterrupt(GPIO_PORT_P1, GPIO_PIN1, true);
}

void initTimers(void)
{
//    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE, 0.0f);
    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_UP_MODE, 0.1f);
    Timer_A_stop(TIMER_A0_BASE);
    addCompare(1, 0.6f);
    addCompare(2, 0.9f);
}

#pragma vector=TIMER0_A1_VECTOR
interrupt void timerTA0_1(void)
{
    if(currentMode == TIMER_A_UP_MODE)
    {
        switch(__even_in_range(TA0IV, 14))
        {
        case 0: break; // none
        case 2: // CCR1 IFG
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN7);
            Timer_A_stop(TIMER_A0_BASE);
            __bic_SR_register_on_exit(LPM0_bits);
        break;
        case 4:  // CCR2 IFG
        break;
        case 6: break; // CCR3 IFG
        case 8: break; // CCR4 IFG
        case 10: break; // CCR5 IFG
        case 12: break; // CCR6 IFG
        case 14: break; // CCR7 IFG
        default: _never_executed();
        }
    }
}

#pragma vector=TIMER0_A0_VECTOR
interrupt void timerTA0_0(void)
{
    __no_operation();
}

#pragma vector=PORT1_VECTOR
interrupt void gpioP1(void)
{
    P1IFG &= ~BIT1;
    GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN7);
    __bic_SR_register_on_exit(LPM0_bits);
}





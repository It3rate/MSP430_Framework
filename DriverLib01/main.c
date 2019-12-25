#include <msp430.h>
#include "driverlib.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "i2c.h"

const unsigned long delays[4] = { 500000, 1000000, 1500000, 3000000 };
const int delayLength = 4;
uint16_t delayIndex = 0;

void initGPIO(void);
void initTimers(void);
void evaluateI2c(void);

#define DATA0LENGTH 9
#define DATA1LENGTH 8
uint8_t transmitData0[DATA0LENGTH] = {READ_FLAG, 0x52, 0x6f, 0x62, 0x69, 0x6e, 0x41, 0x42, 0x43};
uint8_t transmitData1[DATA1LENGTH] = {WRITE_FLAG, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x70};
uint8_t *transmitData;
uint8_t dataLength = 0;

int main(void)
{
    WDT_A_hold(WDT_A_BASE);

    initGPIO();
    initClocks(1);
    printClockSpeeds();

    //initTimers();
    //enableGPIOInterrupts();

    i2c_isMaster = true;
    i2c_init(0x48);

    i2cData[0] = WRITE_FLAG; // start with a write
    while (1)
    {
        __delay_cycles(1000);
        evaluateI2c();
        if(i2c_isTransmitMode)
        {
            i2c_transmitValues(transmitData, dataLength);
        }
        else
        {
            i2c_receiveValues(dataLength);
        }
        __bis_SR_register(LPM0_bits + GIE);
    }
}

void evaluateI2c(void)
{
    if(i2cData[0] == READ_FLAG){ // asked to read a bit, so get it back
        dataLength = DATA1LENGTH;
        if(i2c_isMaster) {
            i2c_isTransmitMode = false;
        }
        else {
            transmitData = transmitData1;
            i2c_isTransmitMode = true;
        }
    }
    else { // default master writes
        if(i2c_isMaster) {
            transmitData = transmitData0;
            dataLength = DATA0LENGTH;
            i2c_isTransmitMode = true;
        }
        else {
            dataLength = I2C_BUFFER_LENGTH;
            i2c_isTransmitMode = false;
        }
    }
}

void initGPIO(void)
{
    addGPIOOutput(GPIO_PORT_P1, GPIO_PIN0, false);
    addGPIOOutput(GPIO_PORT_P4, GPIO_PIN7, true);
    addGPIOInterrupt(GPIO_PORT_P1, GPIO_PIN1, true);
}

void initTimers(void)
{
    //setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE, 0.0f);
    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_UP_MODE, 0.5f);
    addCompare(1, 0.6f);
    addCompare(2, 0.9f);
}





#pragma vector=TIMER0_A1_VECTOR
interrupt void timerTA0_1(void)
{
    switch(__even_in_range(TA0IV, 14))
    {
    case 0: break; // none
    case 2: // CCR1 IFG
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
    } break;
    case 4:  // CCR2 IFG
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    } break;
    case 6: break; // CCR3 IFG
    case 8: break; // CCR4 IFG
    case 10: break; // CCR5 IFG
    case 12: break; // CCR6 IFG
    case 14: break; // CCR7 IFG
    default: _never_executed();
    }
}

#pragma vector=TIMER0_A0_VECTOR
interrupt void timerTA0_0(void)
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
    Timer_A_clearTimerInterrupt(TIMER_A0_BASE);
}

#pragma vector=PORT1_VECTOR
interrupt void gpioP1(void)
{
    delayIndex++;
    if (delayIndex >= delayLength)
    {
        delayIndex = 0;
    }
}





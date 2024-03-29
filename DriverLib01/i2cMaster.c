#include "i2c.h"

#ifdef I2C_IS_MASTER
#include <stdio.h>
#include "i2cMaster.h"

static i2cInstance currentTarget;

void i2c_init()
{
    // F5529 3.0 data and 3.1 clk for USCI_B
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);

    USCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = UCS_getSMCLK();
    param.dataRate = USCI_B_I2C_SET_DATA_RATE_400KBPS;
    USCI_B_I2C_initMaster(USCI_B0_BASE, &param);
}

// ***** RECEIVE *****
#define RECEIVE_INTERRUPTS (USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_START_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT)
void i2c_read(i2cInstance target, uint8_t data[32], uint8_t receiveLength)
{
    currentTarget = target;
    currentTarget.data = data;
    currentTarget.counter = receiveLength;
    currentTarget.totalLength = receiveLength;

    USCI_B_I2C_disable(USCI_B0_BASE);
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, currentTarget.slaveAddress);
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);

    USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE);
}

// ***** TRANSMIT *****
#define TRANSMIT_INTERRUPTS (USCI_B_I2C_TRANSMIT_INTERRUPT + USCI_B_I2C_START_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT)

void i2c_write(i2cInstance target, uint8_t data[32], uint8_t transmitLength)
{
    currentTarget = target;
    currentTarget.data = data;
    currentTarget.counter = transmitLength;
    currentTarget.totalLength = transmitLength;

    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    USCI_B_I2C_disable(USCI_B0_BASE);
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, currentTarget.slaveAddress);
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, TRANSMIT_INTERRUPTS);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, TRANSMIT_INTERRUPTS);

    //printf("%c", *ptrI2cData);
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, *currentTarget.data++);
}

#pragma vector=USCI_B0_VECTOR
__interrupt void usci_b0_isr(void)
{
    //__disable_interrupt();
    uint8_t iv = (uint8_t) UCB0IV;
    switch (__even_in_range(iv, 12))
    {
    case USCI_NONE: break;
    case USCI_I2C_UCALIFG: break; // arbitration lost flag (for multiple masters)
    case USCI_I2C_UCSTTIFG: break; // start
    case USCI_I2C_UCSTPIFG: break;// stop
    case USCI_I2C_UCNACKIFG: // Master NACK
        USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
        break;

    case USCI_I2C_UCTXIFG: // ****** Master Transmit ******
        currentTarget.counter--;
        if (currentTarget.counter)
        {
            USCI_B_I2C_masterSendMultiByteNext(USCI_B0_BASE, *currentTarget.data++);
        }
        else // end transmission
        {
            USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
            USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            __bic_SR_register_on_exit(LPM0_bits);
        }
        break;

    case USCI_I2C_UCRXIFG:  // ****** Master Receive ******
        currentTarget.counter--;
        if(currentTarget.totalLength == 1)
        {
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            *currentTarget.data = USCI_B_I2C_masterReceiveSingle(USCI_B0_BASE);
            __bic_SR_register_on_exit(LPM0_bits + GIE);
        }
        else if(currentTarget.counter)
        {
            if (currentTarget.counter == 1)
            {
                *currentTarget.data++ = USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE);
            }
            else
            {
                *currentTarget.data++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
            }
        }
        else
        {
            USCI_B_I2C_clearInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            *currentTarget.data = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
            __bic_SR_register_on_exit(LPM0_bits);
        }

//        *i2c.data++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
//        if (i2c.counter == 1 || i2c.totalLength == 1) // send stop if the second last element
//        {
//            //GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
//            USCI_B_I2C_masterReceiveMultiByteStop(USCI_B0_BASE);
//        }
//
//        if (i2c.counter == 0 || i2c.totalLength == 1) // finish read
//        {
//            USCI_B_I2C_clearInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);
//            __bic_SR_register_on_exit(LPM0_bits);
//        }

        break;
    }
    //__enable_interrupt();
}

#endif // I2C_IS_MASTER


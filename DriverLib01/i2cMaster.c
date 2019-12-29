#include "i2c.h"

#ifdef I2C_IS_MASTER
#include <stdio.h>
#include "i2cMaster.h"


uint8_t * ptrI2cData;
uint8_t i2cDataLenIn = 0;
uint8_t i2cDataLenOut = 0;
uint8_t i2cCounter = 0;

void prepareToReceiveData(uint8_t transmitLength);
void prepareToTransmitData(uint8_t data[32], uint8_t transmitLength);

void i2c_initMaster(uint16_t slaveAddress)
{
    _slaveAddress = slaveAddress;

    // F5529 3.0 data and 3.1 clk for USCI_B
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);

    USCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = UCS_getSMCLK();
    param.dataRate = USCI_B_I2C_SET_DATA_RATE_100KBPS;
    USCI_B_I2C_initMaster(USCI_B0_BASE, &param);
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, _slaveAddress);
}
void i2c_masterSetTransmitMode()
{
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
}
void i2c_masterSetReadMode()
{
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
}

// ***** RECEIVE *****
void i2c_masterReceiveByte()
{
    USCI_B_I2C_masterReceiveSingleStart(USCI_B0_BASE);
    i2cDataIn[0] = USCI_B_I2C_masterReceiveSingle (USCI_B0_BASE);
    i2cDataIn[1] = 0;
}
#define RECEIVE_INTERRUPTS (USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_START_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT)
void i2c_masterReceiveMultibyte(uint8_t receiveLength)
{
    prepareToReceiveData(receiveLength);

    USCI_B_I2C_disable(USCI_B0_BASE);
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);

    USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE);
}

// ***** TRANSMIT *****
#define TRANSMIT_INTERRUPTS (USCI_B_I2C_TRANSMIT_INTERRUPT + USCI_B_I2C_START_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT)

void i2c_masterTransmitByte(uint8_t data)
{
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE,TRANSMIT_INTERRUPTS);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE,TRANSMIT_INTERRUPTS);
    USCI_B_I2C_masterSendSingleByte(USCI_B0_BASE, *ptrI2cData);
    HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) &= ~USCI_B_I2C_TRANSMIT_MODE;
//    __delay_cycles(100);
//    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
//    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_STOP_INTERRUPT);
}

void i2c_masterTransmitMultibyte(uint8_t data[32], uint8_t transmitLength)
{
    prepareToTransmitData(data, transmitLength);

    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    USCI_B_I2C_disable(USCI_B0_BASE);
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, TRANSMIT_INTERRUPTS);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, TRANSMIT_INTERRUPTS);

    //printf("%c", *ptrI2cData);
    USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, *ptrI2cData++);
}


void prepareToReceiveData(uint8_t transmitLength)
{
    i2cDataIn[transmitLength] = 0; // nullterm string
    ptrI2cData = i2cDataIn;
    i2cCounter = transmitLength;
    i2cDataLenOut = transmitLength;
}
void prepareToTransmitData(uint8_t data[32], uint8_t transmitLength)
{
    uint8_t i = 0;
    while(i < transmitLength)
    {
        i2cDataOut[i] = data[i];
        i++;
    }
    i2cDataOut[transmitLength] = 0; // nullterm string
    ptrI2cData = i2cDataOut;
    i2cCounter = transmitLength;
    i2cDataLenOut = transmitLength;
}

uint32_t blinkCounter = 0;
static inline void updateBlink()
{
    blinkCounter++;
    if (blinkCounter > 0x0002)
    {
        blinkCounter = 0;
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
}


#pragma vector=USCI_B0_VECTOR
__interrupt void usci_b0_isr(void)
{
    updateBlink();
    uint8_t iv = (uint8_t) UCB0IV;
    switch (__even_in_range(iv, 12))
    {
    case USCI_NONE: break;
    case USCI_I2C_UCALIFG: break; // arbitration lost flag (for multiple masters)
    case USCI_I2C_UCSTTIFG: break; // start
    case USCI_I2C_UCTXIFG: // ****** Master Transmit ******
        if (i2cCounter)
        {
            i2cCounter--;
            if (i2cCounter)
            {
                //printf("%c", *ptrI2cData);
                USCI_B_I2C_masterSendMultiByteNext(USCI_B0_BASE, *ptrI2cData++);
            }
            else // end transmission
            {
                //printf("\n");
                USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
                USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
                __bic_SR_register_on_exit(LPM0_bits);
            }
        }
        break;
    case USCI_I2C_UCRXIFG:  // ****** Master Receive ******
            *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);

        if (i2cCounter)
        {
            i2cCounter--;
            if (i2cCounter == 1 || i2cDataLenOut == 1) // send stop if the second last element
            {
                USCI_B_I2C_masterReceiveMultiByteStop(USCI_B0_BASE);
            }

            if (i2cCounter == 0 || i2cDataLenOut == 1) // finish read
            {
                USCI_B_I2C_clearInterrupt(USCI_B0_BASE, RECEIVE_INTERRUPTS);
                __bic_SR_register_on_exit(LPM0_bits);
            }
        }
        break;

    case USCI_I2C_UCNACKIFG: // Master NACK
        //__bic_SR_register_on_exit(LPM0_bits);
        break;

    case USCI_I2C_UCSTPIFG: // Master Stop
        __bic_SR_register_on_exit(LPM0_bits);
        break;
    }
}

#endif // I2C_IS_MASTER


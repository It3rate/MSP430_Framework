#include "i2c.h"

#ifndef I2C_IS_MASTER
#include <stdio.h>
#include "i2cSlave.h"

uint8_t * ptrI2cData;

#define I2C_SLAVE_INTERRUPTS (USCI_B_I2C_TRANSMIT_INTERRUPT + USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_START_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT)

void i2c_initSlave(uint16_t slaveAddress)
{
    _slaveAddress = slaveAddress;
    // F5529 3.0 data and 3.1 clk for USCI_B
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);
    USCI_B_I2C_initSlave(USCI_B0_BASE, _slaveAddress);

    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, I2C_SLAVE_INTERRUPTS);
    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, I2C_SLAVE_INTERRUPTS);
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
    case USCI_I2C_UCALIFG: break;
    case USCI_I2C_UCSTTIFG:
    {
        ptrI2cData = i2cDataIn;
        printf("\n");
        __no_operation();
    }
        break;

    case USCI_I2C_UCTXIFG: // ****** Slave Transmit ******
        // eventually put the address requested with the last RX
        USCI_B_I2C_slavePutData(USCI_B0_BASE, *ptrI2cData++);
        break;

    case USCI_I2C_UCRXIFG: // ****** Slave Receive ******
        *ptrI2cData = USCI_B_I2C_slaveGetData(USCI_B0_BASE);
        printf("%c", *ptrI2cData);
        ptrI2cData++;
        break;
    case USCI_I2C_UCNACKIFG: // Slave NACK
        __no_operation();
        break;
    case USCI_I2C_UCSTPIFG: // Slave Stop
        *ptrI2cData = USCI_B_I2C_slaveGetData(USCI_B0_BASE);
        printf("%c\n", *ptrI2cData);
        ptrI2cData = i2cDataIn;
        __no_operation();
        break;
    }
}

#endif // NOT I2C_IS_MASTER

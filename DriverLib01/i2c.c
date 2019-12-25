#include "i2c.h"

uint16_t _slaveAddress;
bool _isMaster;
bool _isTransmitMode;

uint8_t i2cData[32];
uint8_t * ptrI2cData;
uint8_t i2cDataLen = 0;
uint8_t i2cCounter = 0;

void setReceiveMode();
void setTransmitMode();

void initI2c(bool isMaster, uint16_t slaveAddress)
{
    _isMaster = isMaster;
    _slaveAddress = slaveAddress;

    // F5529 3.0 data and 3.1 clk for USCI_B
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);

    if(isMaster)
    {
        USCI_B_I2C_initMasterParam param = {0};
        param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
        param.i2cClk = UCS_getSMCLK();
        param.dataRate = USCI_B_I2C_SET_DATA_RATE_100KBPS;
        USCI_B_I2C_initMaster(USCI_B0_BASE, &param);
        USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, _slaveAddress);
        //setTransmitMode();
    }
    else
    {
        USCI_B_I2C_initSlave(USCI_B0_BASE, _slaveAddress);
        //setReceiveMode();
    }
    //USCI_B_I2C_enable(USCI_B0_BASE);
    //while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
}

void setReceiveMode()
{
    USCI_B_I2C_disable(USCI_B0_BASE);
    _isTransmitMode = false;
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT);

    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT + USCI_B_I2C_NAK_INTERRUPT);
    if(_isMaster)
    {
        //while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    }
}

void receiveValues(uint8_t receiveLength)
{
    ptrI2cData = i2cData;
    i2cDataLen = receiveLength;
    i2cCounter = receiveLength;
    setReceiveMode();
    if(_isMaster)
    {
        USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE);
    }
}

void setTransmitMode()
{
    if(_isMaster)
    {
        while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    }
    USCI_B_I2C_disable(USCI_B0_BASE);
    _isTransmitMode = true;
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT);

    USCI_B_I2C_enable(USCI_B0_BASE);
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT);


}

void transmitValues(uint8_t data[32], uint8_t transmitLength)
{
    if(_isMaster)
    {
        while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    }
    uint8_t i = 0;
    while(i < transmitLength)
    {
        i2cData[i] = data[i];
        i++;
    }
    ptrI2cData = i2cData;
    i2cCounter = transmitLength;
    i2cDataLen = transmitLength;
    setTransmitMode();
    if(_isMaster)
    {
        USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, i2cData[0]);
    }
//    else
//    {
//        USCI_B_I2C_slavePutData(USCI_B0_BASE, i2cData[0]);
//    }
}

uint32_t transferCounter = 0;

#pragma vector=USCI_B0_VECTOR
__interrupt void usci_b0_isr(void)
{
    transferCounter++;
    if (transferCounter > 0x0F00)
    {
        transferCounter = 0;
        GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }

    uint8_t iv = (uint8_t) UCB0IV;
    if (_isMaster)
    {
        switch (__even_in_range(iv, 12))
        {
        case USCI_NONE: break;
        case USCI_I2C_UCALIFG: break;
        case USCI_I2C_UCSTTIFG: break;
        case USCI_I2C_UCNACKIFG: break; // NACK
        case USCI_I2C_UCTXIFG: // Master Transmit
            if (i2cCounter)
            {
                i2cCounter--;
                if (i2cCounter)
                {
                    USCI_B_I2C_masterSendMultiByteNext(USCI_B0_BASE, i2cData[i2cDataLen - i2cCounter]);
                }
                else // end transmission
                {
                    USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
                    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
                    __bic_SR_register_on_exit(LPM0_bits);
                }
            }
            break;
        case USCI_I2C_UCRXIFG:  // Master Receive
            if (i2cCounter)
            {
                i2cCounter--;
                if (i2cCounter > 1)
                {
                    *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
                }
                else if (i2cCounter == 1) // ask for last data element with a stop
                {
                    *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE);
                }
                else // read last byte and end receive
                {
                    *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
                    __bic_SR_register_on_exit(LPM0_bits);
                }
            }
            break;

        case USCI_I2C_UCSTPIFG: // Master Stop
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    }
    else // Slave
    {
        switch (__even_in_range(iv, 12))
        {
        case USCI_NONE: break;
        case USCI_I2C_UCALIFG: break;
        case USCI_I2C_UCSTTIFG: break;
        case USCI_I2C_UCTXIFG: // Slave Transmit
            if (i2cCounter)
            {
                USCI_B_I2C_slavePutData(USCI_B0_BASE, *ptrI2cData++);
                i2cCounter--;
            }
            break;
        case USCI_I2C_UCRXIFG: // Slave Receive
            if (i2cCounter)
            {
                *ptrI2cData++ = USCI_B_I2C_slaveGetData(USCI_B0_BASE);
                i2cCounter--;
            }
            break;
        case USCI_I2C_UCNACKIFG: // Slave NACK
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        case USCI_I2C_UCSTPIFG: // Slave Stop
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        }
    }

}



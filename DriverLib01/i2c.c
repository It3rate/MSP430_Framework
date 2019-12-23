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
    // 3.0 data and 3.1 clk for USCI_B
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);
    if(isMaster)
    {
        USCI_B_I2C_initMasterParam param = {0};
        param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
        param.i2cClk = UCS_getSMCLK();
        param.dataRate = USCI_B_I2C_SET_DATA_RATE_400KBPS;
        USCI_B_I2C_initMaster(USCI_B0_BASE, &param);
        USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, _slaveAddress);
        setTransmitMode();
    }
    else
    {
        USCI_B_I2C_initSlave(USCI_B0_BASE, _slaveAddress);
        setReceiveMode();
    }
    USCI_B_I2C_enable(USCI_B0_BASE); // may need to enable before setting mode?
}

void setReceiveMode()
{
    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    ptrI2cData = i2cData;
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_RECEIVE_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT);
    _isTransmitMode = false;
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, USCI_B_I2C_RECEIVE_INTERRUPT + USCI_B_I2C_STOP_INTERRUPT);
}

void setTransmitMode()
{
    while (USCI_B_I2C_isBusBusy(USCI_B0_BASE));
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);
    USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
    _isTransmitMode = true;
    USCI_B_I2C_enableInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
}

void transmitValues(uint8_t data[32], uint8_t transmitLength)
{
    uint8_t i = 0;
    while(i < transmitLength)
    {
        i2cData[i] = data[i];
        i++;
    }
    i2cCounter = transmitLength;
    i2cDataLen = transmitLength;
    setTransmitMode();
    if(_isMaster)
    {
        USCI_B_I2C_masterSendMultiByteStart(USCI_B0_BASE, i2cData[0]);
    }
    else
    {
        USCI_B_I2C_slavePutData(USCI_B0_BASE, i2cData[0]);
    }
    __bis_SR_register(LPM0_bits + GIE);
}
void receiveValues(uint8_t receiveLength)
{
    setReceiveMode();
    i2cCounter = receiveLength;
    if(_isMaster)
    {
        USCI_B_I2C_masterReceiveMultiByteStart(USCI_B0_BASE);
    }
    else
    {
    }
}


#pragma vector=USCI_B0_VECTOR
__interrupt void usci_b0_isr (void)
{
    switch (__even_in_range(UCB0IV,12)){
        case USCI_I2C_UCTXIFG:
        {
            if(_isMaster)
            {
                i2cCounter--;
                if(_isTransmitMode)
                {
                    if (i2cCounter > 0)
                    {
                        USCI_B_I2C_masterSendMultiByteNext(USCI_B0_BASE, i2cData[i2cDataLen - i2cCounter]);
                    }
                    else
                    {
                        USCI_B_I2C_masterSendMultiByteStop(USCI_B0_BASE);
                        USCI_B_I2C_clearInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
                        __bic_SR_register_on_exit(LPM0_bits);
                    }
                }
                else
                {
                    if (i2cCounter > 1)
                    {
                        *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
                    }
                    else if (i2cCounter > 0)
                    {
                        *ptrI2cData++ = USCI_B_I2C_masterReceiveMultiByteFinish(USCI_B0_BASE);
                    }
                    else {
                        *ptrI2cData = USCI_B_I2C_masterReceiveMultiByteNext(USCI_B0_BASE);
                        __bic_SR_register_on_exit(LPM0_bits);
                    }
                }
            }
            else
            {
            }
        }
        break;

        case USCI_I2C_UCRXIFG:
        {
            if(_isMaster)
            {
            }
            else
            {
                uint8_t data = USCI_B_I2C_slaveGetData(USCI_B0_BASE);
                *ptrI2cData++ = data;
            }
        }
        break;

        case USCI_I2C_UCSTPIFG:
        {
            __bic_SR_register_on_exit(LPM0_bits);
        }
        break;
    }
}



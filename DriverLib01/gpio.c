#include "driverlib.h"

uint8_t _interrputPortList[16];
uint16_t _interrputPinList[16];
unsigned int _interrputIndex = 0;

void addGPIOOutput(uint8_t port, uint16_t pin, bool startOn)
{
    GPIO_setAsOutputPin(port, pin);
    if(startOn)
    {
        GPIO_setOutputHighOnPin(port, pin);
    }
    else
    {
        GPIO_setOutputLowOnPin(port, pin);
    }
}

void addGPIOInput(uint8_t port, uint16_t pin)
{
    GPIO_setAsInputPin(port, pin);
}

void addGPIOInterrupt(uint8_t port, uint16_t pin, bool isLowToHigh)
{
    GPIO_setAsInputPinWithPullUpResistor(port, pin);
    uint8_t transition = isLowToHigh ? GPIO_LOW_TO_HIGH_TRANSITION : GPIO_HIGH_TO_LOW_TRANSITION;
    GPIO_selectInterruptEdge(port, pin, transition);
    GPIO_clearInterrupt(port, pin);

    _interrputPortList[_interrputIndex] = port;
    _interrputPinList[_interrputIndex] = pin;
    _interrputIndex++;
}

void enableGPIOInterrupts(void)
{
    unsigned int i = 0;
    for(i = 0; i < _interrputIndex; i++)
    {
        GPIO_enableInterrupt(_interrputPortList[i], _interrputPinList[i]);
    }
}

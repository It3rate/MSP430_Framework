#include <msp430.h>
#include <stdio.h>
#include "driverlib.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "i2c.h"
#include "i2cMaster.h"
#include "i2cSlave.h"
#include "ssd1306.h"
#include "mpu9250.h"

void initGPIO(void);
void initTimers(void);
uint16_t volatile timerCounter = 0;
uint8_t ccrRed = 1;
uint8_t ccrGreen = 2;
uint8_t ccrBlue = 3;

static enum States {s_init, s_read, s_print, s_calc, s_delay, s_idle};
enum States curState = s_init;
enum States nextState = s_read;
uint8_t *data;

int main(void)
{
    WDT_A_hold(WDT_A_BASE);

    initGPIO();
    initClocks(2);
    printClockSpeeds();
    enableGPIOInterrupts();
    initTimers();

    i2c_init();
    ssd1306_init();
    mpu9250_init();

    ssd1306_clearDisplay();

    uint16_t ax;
    uint16_t ay;
    uint16_t az;

    while (1)
    {
        switch (curState)
        {
        case s_init:
        {
            Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
            curState = s_read;
        }
            break;
        case s_read:
        {
            data = mpu9250_readSensors();
            curState = s_print;
        }
            break;
        case s_print:
        {
            Timer_A_stop(TIMER_A0_BASE);
            printGyroData(data);
            Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
            curState = s_calc;
        }
            break;
        case s_calc:
        {
            ax = (data[0]<<8)+data[1];
            ay = (data[2]<<8)+data[3];
            az = (data[4]<<8)+data[5];
            setCompareValue(ccrRed, ax / 65535.0f);
            setCompareValue(ccrGreen, ay / 65535.0f);
            setCompareValue(ccrBlue,  az / 65535.0f);
            curState = s_delay;
        }
            break;
        case s_delay:
        {
            timerCounter = 200;
            nextState = s_read;
            curState = s_idle;
        }
            break;
        case s_idle:
        {
            __bis_SR_register(LPM0_bits + GIE);
            curState = nextState;
        }
            break;
        default:
            __never_executed();
        }
    }

__no_operation();
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
            GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN3);
        break;
        case 4: // CCR2 IFG
            GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
        break;
        case 6: // CCR3 IFG
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
            break;
        case 8:  break; // CCR4 IFG
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
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
    if(timerCounter > 0 && curState == s_idle)
    {
        timerCounter--;
        if(timerCounter == 0)
        {
            GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN2);
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN7);
            //Timer_A_stop(TIMER_A0_BASE);
            __bic_SR_register_on_exit(LPM0_bits);
        }
    }
}

#pragma vector=PORT1_VECTOR
interrupt void gpioP1(void)
{
    P1IFG &= ~BIT1;
    GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN7);
    __bic_SR_register_on_exit(LPM0_bits);
}


void initGPIO(void)
{
    addGPIOOutput(GPIO_PORT_P1, GPIO_PIN0, false);
    addGPIOOutput(GPIO_PORT_P4, GPIO_PIN7, true);
    addGPIOInterrupt(GPIO_PORT_P1, GPIO_PIN1, true);

    addGPIOOutput(GPIO_PORT_P4, GPIO_PIN3, false);
    addGPIOOutput(GPIO_PORT_P4, GPIO_PIN0, false);
    addGPIOOutput(GPIO_PORT_P3, GPIO_PIN7, false);
    addGPIOOutput(GPIO_PORT_P8, GPIO_PIN2, false);
}

void initTimers(void)
{
//    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE, 0.0f);
    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_UP_MODE, 0.0005f); // 0.02f == ~50Hz
    Timer_A_stop(TIMER_A0_BASE);
    //addCompare(0, 1.0f);
    addCompare(1, 0.6f);
    addCompare(2, 0.2f);
    addCompare(3, 0.9f);
    //addCompare(4, 0.99f);
}





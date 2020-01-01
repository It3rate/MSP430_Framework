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

int main(void)
{
    WDT_A_hold(WDT_A_BASE);

    initGPIO();
    initClocks(1);
    printClockSpeeds();
    enableGPIOInterrupts();
    initTimers();

    i2c_init();
    ssd1306_init();
    mpu9250_init();

    ssd1306_clearDisplay();

//    ssd1306_printText(0,3, "012345678901234567890");


#ifdef I2C_IS_MASTER
    while (1)
    {
            uint8_t *data = mpu9250_readSensors();
            printGyroData(data);

//            printf("accel x%04X y%04X z%04X  temp: %04X  gyro: x%04X y%04X z%04X  \n",
//               (data[0]<<8)+data[1], (data[2]<<8)+data[3],  (data[4]<<8)+data[5],
//               (data[6]<<8)+data[7],
//               (data[8]<<8)+data[9], (data[10]<<8)+data[11],  (data[12]<<8)+data[13]);

        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
        __bis_SR_register(LPM0_bits + GIE);
    }
#endif

__no_operation();
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
    setTargetTimerAndMode(TIMER_A0_BASE, TIMER_A_UP_MODE, 0.01f);
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





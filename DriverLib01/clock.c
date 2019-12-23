#include "clock.h"
#include <stdio.h>
#include "driverlib.h"

#define XT1_FREQ 32768
#define XT2_FREQ 4000000
#define REFO_FREQ 32768

//#define MCLK_FREQ 20000000
//#define MCLK_FREQ_KHZ (MCLK_FREQ/1000)
//#define FLL_RATIO MCLK_FREQ/(XT2_FREQ/4)

void initClocks(uint8_t mclkMHz)
{
    switch(mclkMHz)
    {
    case 1:
        setMClock_1MHz();
        break;
    case 8:
        setMClock_8MHz();
        break;
    case 12:
        setMClock_12MHz();
        break;
    case 20:
        setMClock_20MHz();
        break;
    default:
        __never_executed();
        break;
    }
    //UCS_initClockSignal(UCS_SMCLK, UCS_DCOCLK_SELECT, UCS_CLOCK_DIVIDER_32);
    UCS_initClockSignal(UCS_ACLK, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_1);
    mClockSpeed = UCS_getMCLK();
    smClockSpeed = UCS_getSMCLK();
    aClockSpeed = UCS_getACLK();
}

void setMClock_20MHz(void)
{
    turnOnCrystals();
    PMM_setVCore(PMM_CORE_LEVEL_2);
    UCS_initClockSignal(UCS_FLLREF, UCS_XT2CLK_SELECT, UCS_CLOCK_DIVIDER_4);
    UCS_initFLLSettle(20000, 20000000/(XT2_FREQ/4));
    dividerForOneSecond = TIMER_A_CLOCKSOURCE_DIVIDER_20;
}
void setMClock_12MHz(void)
{
    PMM_setVCore(PMM_CORE_LEVEL_1);
    turnOnCrystals();
    UCS_initClockSignal(UCS_FLLREF, UCS_XT2CLK_SELECT, UCS_CLOCK_DIVIDER_8);
    UCS_initFLLSettle(12000, 12000000/(XT2_FREQ/8));
    dividerForOneSecond = TIMER_A_CLOCKSOURCE_DIVIDER_12;
}
void setMClock_8MHz(void)
{
    PMM_setVCore(PMM_CORE_LEVEL_0);
    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initFLLSettle(8000, 8000000/REFO_FREQ);
    dividerForOneSecond = TIMER_A_CLOCKSOURCE_DIVIDER_8;
}

void setMClock_1MHz(void)
{
    PMM_setVCore(PMM_CORE_LEVEL_0);
    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);
    UCS_initFLLSettle(1000, 1000000/REFO_FREQ);
    dividerForOneSecond = TIMER_A_CLOCKSOURCE_DIVIDER_1;
}

void turnOnCrystals(void)
{
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN4+GPIO_PIN2
    );
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P5,
        GPIO_PIN5+GPIO_PIN3
    );
    UCS_setExternalClockSource(XT1_FREQ, XT2_FREQ);
    bool success = UCS_turnOnXT2WithTimeout(UCS_XT2_DRIVE_4MHZ_8MHZ, UINT16_MAX);
    if(!success){
        printf("XT2 fail.\n");
    }
    success = UCS_turnOnLFXT1WithTimeout(UCS_XT1_DRIVE_0, UCS_XCAP_3, UINT16_MAX);
    if(!success){
       printf("XT1 fail.\n");
    }
}


void printClockSpeeds(void){
    printf("MCLK:%luHz  SMCLK:%luHz  ACLK:%luHz\n",
           (unsigned long)(mClockSpeed),(unsigned long)(smClockSpeed),(unsigned long)aClockSpeed);
}

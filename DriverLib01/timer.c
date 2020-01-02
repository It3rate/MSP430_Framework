#include "driverlib.h"
#include "timer.h"
#include "clock.h"

uint16_t currentTimer;
uint16_t currentTimePeriod;

void createTimerParam();

void initContinuousTimer(Timer_A_initContinuousModeParam *param)
{
    switch(currentTimer)
    {
    case TIMER_B0_BASE:
        Timer_B_initContinuousMode(currentTimer, param);
        Timer_B_startCounter(currentTimer, currentMode);
        break;
    default:
        Timer_A_initContinuousMode(currentTimer, param);
        Timer_A_startCounter(currentTimer, currentMode);
        break;
    }
}
void initUpTimer(Timer_A_initUpDownModeParam *param)
{
    switch(currentTimer)
    {
    case TIMER_B0_BASE:
        Timer_B_initUpMode(currentTimer, param);
        Timer_B_startCounter(currentTimer, currentMode);
        break;
    default:
        Timer_A_initUpMode(currentTimer, param);
        Timer_A_startCounter(currentTimer, currentMode);
        break;
    }
}

void setTargetTimerAndMode(uint16_t timer, uint16_t timerMode, float seconds)
{
    currentTimer = timer;
    currentMode = timerMode;

    Timer_A_clearTimerInterrupt(timer);

    switch(timerMode)
    {
        case TIMER_A_CONTINUOUS_MODE:
        {
            Timer_A_initContinuousModeParam param = {0};
            param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
            param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
            param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
            param.timerClear = TIMER_A_DO_CLEAR;
            param.startTimer = false;
            initContinuousTimer(&param);
        }
            break;
        case TIMER_A_UP_MODE:
        {
            Timer_A_initUpDownModeParam param = {0};
            param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
            param.clockSourceDivider = dividerForOneSecond;
            param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
            param.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
            param.timerClear = TIMER_A_DO_CLEAR;
            currentTimePeriod = seconds * smClockSpeed;
            param.timerPeriod = currentTimePeriod;
            param.startTimer = false;
            initUpTimer(&param);
        }
            break;
        case TIMER_A_UPDOWN_MODE:
        {

        }
            break;
    }
}

void setCompareValue(char registerIndex, float ratio)
{
    uint16_t reg = 0;
    switch(registerIndex)
    {
    case 0:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_0;
        break;
    case 1:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_1;
        break;
    case 2:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_2;
        break;
    case 3:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_3;
        break;
    case 4:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_4;
        break;
    case 5:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_5;
        break;
    case 6:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_6;
        break;
    }
    Timer_A_setCompareValue(currentTimer, reg, ratio * currentTimePeriod);
}

void addCompare(char registerIndex, float ratio)
{
    uint16_t reg = 0;
    switch(registerIndex)
    {
    case 0:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_0;
        break;
    case 1:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_1;
        break;
    case 2:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_2;
        break;
    case 3:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_3;
        break;
    case 4:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_4;
        break;
    case 5:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_5;
        break;
    case 6:
        reg = TIMER_A_CAPTURECOMPARE_REGISTER_6;
        break;
    }
    Timer_A_initCompareModeParam param = {0};
    param.compareRegister = reg;
    param.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    param.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    param.compareValue = ratio * currentTimePeriod;

    Timer_A_initCompareMode(currentTimer, &param);
    Timer_A_clearCaptureCompareInterrupt(currentTimer, reg);
}






#ifndef TIMER_H_
#define TIMER_H_

#include <msp430.h>

void setTargetTimerAndMode(uint16_t timer, uint16_t timerMode, float seconds);

void setTimerFrequency(uint16_t timer, unsigned int frequency);
void addCompare(char registerIndex, float ratio);

void addCapture(unsigned int frequency);



#endif /* TIMER_H_ */

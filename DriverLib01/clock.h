#ifndef CLOCK_H_
#define CLOCK_H_

#include "driverlib.h"

uint32_t mClockSpeed;
uint32_t smClockSpeed;
uint32_t aClockSpeed;

uint16_t dividerForOneSecond;

void initClocks(void);

void setMClock_20MHz(void);
void setMClock_12MHz(void);
void setMClock_8MHz(void);
void setMClock_1MHz(void);

void turnOnCrystals(void);
void printClockSpeeds(void);


#endif /* CLOCK_H_ */

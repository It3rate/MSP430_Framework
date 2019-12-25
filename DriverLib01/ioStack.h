
#ifndef IOSTACK_H_
#define IOSTACK_H_

#include "driverlib.h"

#define IOS_WRITEVALUE         0x01
#define IOS_WRITEARRAY         0x02
#define IOS_READVALUE          0x03
#define IOS_READARRAY          0x04

bool ios_nextState();

void ios_addWriteValueCommand(uint8_t value);
void ios_addWriteArrayCommand(uint16_t address, uint8_t length);
void ios_addReadValueCommand(uint8_t value);
void ios_addReadArrayCommand(uint16_t address, uint8_t length);


#endif /* IOSTACK_H_ */


#ifndef GPIO_H_
#define GPIO_H_

void addGPIOOutput(uint8_t port, uint16_t pin, bool startOn);
void addGPIOInterrupt(uint8_t port, uint16_t pin, bool isLowToHigh);
void enableGPIOInterrupts(void);

#endif /* GPIO_H_ */

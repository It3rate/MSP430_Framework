
#ifndef GPIO_H_
#define GPIO_H_

void addGPIOOutput(uint8_t port, uint16_t pin, bool startOn);
void addGPIOInput(uint8_t port, uint16_t pin);
void addGPIOInterrupt(uint8_t port, uint16_t pin, bool isLowToHigh);
void enableGPIOInterrupts(void);

#endif /* GPIO_H_ */

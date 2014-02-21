#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"

void usartInit();
void usartSendString(const char *s);
void usartSend(uint16_t Data);

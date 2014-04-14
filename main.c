#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "enc28j60.h"
#include "usart_jose.h"
#include "ipstack.h"
#include <CoOS.h>

/*---------------------------- Symbol Define -------------------------------*/
#define STACK_SIZE_TASKA 128              /*!< Define "taskA" task size */
#define STACK_SIZE_TASKB 128              /*!< Define "taskA" task size */

/*---------------------------- Variable Define -------------------------------*/
OS_STK     taskA_stk[STACK_SIZE_TASKA];	  /*!< Define "taskA" task stack */
OS_STK     taskB_stk[STACK_SIZE_TASKB];	  /*!< Define "taskB" task stack */

//RTOS tasks
static void taskA(void);
static void taskB(void);

void LED_init() {
	GPIO_InitTypeDef gpio;

	//Enable GPIOB peripheral clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_StructInit(&gpio);

	// B10, Pin 1 J2 header
	gpio.GPIO_Pin = GPIO_Pin_10;
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOB, &gpio);

	// B11, Pin 2 J2 header
	gpio.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &gpio);
}


void taskA(void) {
	while (1) {
		IPstackIdle();
	}
}

void taskB(void) {
	while (1) {
		usartSend(0xC1);
		CoTickDelay(16);
	}

}

int main(void) {
	//Hardware initialization
	LED_init();
	usartInit();
	IPstackInit();
	usartSend(0xC0);

	CoInitOS ();
	CoCreateTask ((FUNCPtr)taskA,0,1,&taskA_stk[STACK_SIZE_TASKA-1],STACK_SIZE_TASKA);
	CoCreateTask ((FUNCPtr)taskB,0,0,&taskB_stk[STACK_SIZE_TASKB-1],STACK_SIZE_TASKB);
	CoStartOS ();
	while (1);

//	while(1){
//		IPstackIdle();
//	}

}

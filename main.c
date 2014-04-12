#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0_spi_jose.h"
#include "enc28j60_jose.h"
#include "usart_jose.h"
#include "udpstack.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

//Global variables
static int led_state = 0;

//FreeRTOS tasks
static void task1(void *testBuffer);
static void task2(void *testBuffer);

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

//Task 1: Toggle LEDs
void task1(void *parameters) {
	static uint8_t packet[MAXPACKETLEN];
	uint8_t rlen = 0;
	static int i;
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	while (1) {

		GetPacket(0,packet);

//		usartSend(0xC0);
//		if (rlen != 0) {
//			usartSend(0xC1);
//			for (i=0; i <13; i++){
//							usartSend(rxBuffer[i]);
//					}
//		vTaskDelayUntil(&xNextWakeTime, xDelay);
	}
//		if (rlen != 0){
//		usartSend(0xC1);
//		for (i=0; i <rlen; i++){
//				usartSend(rxBuffer[i]);
//		}
////		usartSend(rlen);
////		usartSend(ETH_ReadETHControlRegister(EPKTCNT));
//		}
//	}
}

//Task 2: send a packet periodically
void task2(void *testBuffer) {
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
	const TickType_t xDelay = 1500 / portTICK_PERIOD_MS; //500 ms
	uint8_t target[] = {192,168,0,100};
	while (1) {
//		ETH_send_packet(testBuffer, 514);
//		usartSend(0xC0); //Packet sent
//		usartSend(0xC1);
		vTaskDelayUntil(&xNextWakeTime, xDelay);
	}

}

int main(void) {
	//local variables
//	static int i;
//	uint8_t rlen = 0;
	uint8_t macAdress[6] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
	uint8_t deviceIP[4] = { 192, 168, 0, 111 };
	uint8_t routerIP[4] = { 192, 168, 0, 1 };
//	static uint8_t testBuffer[514] = { 	0xE8, 0x03, 0x9A, 0xAE, 0x00, 0x9C, //Destination address
//										0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02, //Source address
//										0x01, 0xF4, //Type length
//										0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; //Data

	//Hardware initialization
	LED_init();
	ETH_SPI_init();
	usartInit();
	IPstackInit();
	usartSend(0xC1);
//	SendArpPacket(deviceIP);
//	SendArpPacket( routerIP );

	//Fill buffer with some data
//	for (i = 0; i < 500; i++) {
//		testBuffer[14 + i] = i * 2;
//	}

//	Create tasks
//	xTaskCreate(task1, 						/* The function that implements the task. */
//				"Task 1", 					/* The text name assigned to the task - for debug only as it is not used by the kernel. */
//				configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task. */
//				NULL,						/* The parameter passed to the task - just to check the functionality. */
//				1, 							/* The priority assigned to the task. */
//				NULL);
//	/* The task handle is not required, so NULL is passed. */
//	xTaskCreate(task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
//
//	/* Start the tasks and timer running. */
//	vTaskStartScheduler();
//
//	/* If all is well, the scheduler will now be running, and the following
//	 line will never be reached.  If the following line does execute, then
//	 there was insufficient FreeRTOS heap memory available for the idle and/or
//	 timer tasks	to be created.  See the memory management section on the
//	 FreeRTOS web site for more details. */
//	for (;;)
//		;
	static uint8_t packet[MAXPACKETLEN];
	while (1) GetPacket(0,packet);
//
//		rlen = ETH_ReceivePacket(testBuffer, 514);
////		usartSend(0xC0);
//
//		if (rlen != 0){
//			usartSend(0xC1); //Packet received
//			usartSend(rlen);
////			for (i=0; i <rlen; i++){
////					usartSend(testBuffer[i]);
////			}
//			testBuffer[0]=0xE8;
//			testBuffer[1]=0x03;
//			testBuffer[2]=0x9A;
//			testBuffer[3]=0xAE;
//			testBuffer[4]=0x00;
//			testBuffer[5]=0x9C;
//
//			testBuffer[6]=0x00;
//			testBuffer[7]=0xAA;
//			testBuffer[8]=0xBB;
//			testBuffer[9]=0xCC;
//			testBuffer[10]=0xDE;
//			testBuffer[11]=0x02;
//			ETH_send_packet(testBuffer, rlen);
//		}
//
////		for (i=0; i<1000000; ++i);
////		GPIO_WriteBit(GPIOB, GPIO_Pin_10, led_state ? Bit_SET : Bit_RESET);
////		led_state = !led_state;
////		GPIO_WriteBit(GPIOB, GPIO_Pin_11, led_state ? Bit_SET : Bit_RESET);
//		}
//	return 0;
}

void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created.  It is also called by various parts of the
	 demo application.  If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	 task.  It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()).  If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 function is called if a stack overflow is detected. */taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void) {
	/* This function will be called by each tick interrupt if
	 configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	 added here, but the tick hook is called from an interrupt context, so
	 code must not attempt to block, and only the interrupt safe FreeRTOS API
	 functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/


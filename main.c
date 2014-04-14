#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "enc28j60.h"
#include "usart_jose.h"
#include "ipstack.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

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
	while (1) {
		IPstackIdle();
	}
}

//Task 2: send a packet periodically
void task2(void *testBuffer) {
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS; //500 ms
//	uint8_t target[] = {192,168,0,100};
	while (1) {
//		SendPing(target);
//		ETH_send_packet(testBuffer, 514);
		usartSend(0xC1);
		vTaskDelayUntil(&xNextWakeTime, xDelay);
	}

}

int main(void) {
	//Hardware initialization
	LED_init();
	usartInit();
	IPstackInit();
	usartSend(0xC0);

//	while(1){
//		IPstackIdle();
//	}
//	Create tasks
	xTaskCreate(task1, 						/* The function that implements the task. */
				"Task 1", 					/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 	/* The size of the stack to allocate to the task. */
				NULL,						/* The parameter passed to the task - just to check the functionality. */
				2, 							/* The priority assigned to the task. */
				NULL);
	/* The task handle is not required, so NULL is passed. */
//	xTaskCreate(task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	 line will never be reached.  If the following line does execute, then
	 there was insufficient FreeRTOS heap memory available for the idle and/or
	 timer tasks	to be created.  See the memory management section on the
	 FreeRTOS web site for more details. */
	for (;;)
		;
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


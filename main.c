#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0_spi_jose.h"
#include "enc28j60_jose.h"
#include "usart_jose.h"
#include "uip.h"
#include "uip_arp.h"
#include "hello-world.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

#define BUF ((struct uip_eth_hdr * )&uip_buf[0])

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
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
	const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
	while (1) {

		vTaskDelayUntil(&xNextWakeTime, xDelay);
	}
}

//Task 2: send a packet periodically
void task2(void *testBuffer) {
	TickType_t xNextWakeTime;
	xNextWakeTime = xTaskGetTickCount();
	const TickType_t xDelay = 1500 / portTICK_PERIOD_MS; //500 ms
	while (1) {

		vTaskDelayUntil(&xNextWakeTime, xDelay);
	}

}

int main(void) {
	//local variables
	struct uip_eth_addr macAdress = { { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 } };

	//Hardware initialization
	LED_init();
	ETH_SPI_init();
	usartInit();
	ETH_MAC_init(macAdress.addr);

	uip_init();
	uip_arp_init();
	hello_world_init();
	uip_setethaddr(macAdress);

	uip_ipaddr_t ipaddr;
	uip_ipaddr(ipaddr, 192, 168, 0, 200);
	uip_sethostaddr(ipaddr);

	uip_ipaddr(ipaddr, 192, 168, 0, 1);
	uip_setdraddr(ipaddr);

	uip_ipaddr(ipaddr, 255, 255, 255, 0);
	uip_setnetmask(ipaddr);

	usartSend(0xC1);

	while (1) {
		uip_len = ETH_ReceivePacket((uint8_t *) uip_buf, UIP_BUFSIZE);
		if (uip_len > 0) {
			if (BUF->type == htons(UIP_ETHTYPE_IP)) {

				uip_arp_ipin();
				uip_input();
				if (uip_len > 0) {
					/* If the above function invocation resulted in data that
					 should be sent out on the network, the global variable
					 uip_len is set to a value > 0. */
					uip_arp_out();
					ETH_send_packet((uint8_t *) uip_buf, uip_len);
					usartSend(0xC2);
				}
			} else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {

				uip_arp_arpin();
				if (uip_len > 0) {
					ETH_send_packet((uint8_t *) uip_buf, uip_len);
					usartSend(0xC3);
				}
			}
		}
	}
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


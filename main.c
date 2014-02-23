
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0_spi_jose.h"
#include "enc28j60.h"
#include "usart_jose.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

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


int main(void)
 {

	static int i;
	static int led_state=0;
	uint8_t macAdress[6] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
	LED_init();
	ETH_SPI_init();
	usartInit();
	ETH_MAC_init(macAdress);
	uint8_t testBuffer[514] = {0xE8, 0x03, 0x9A, 0xAE, 0x00, 0x9C,	//Destination address
								   0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02,		//Source address
								   0x01, 0xF4,					//Type length
								   0, 1, 2, 3, 4, 5, 6, 7, 8, 9};	//Data

	for (i=0; i <500; i++){
		testBuffer[14+i] = i*2;
	}

	uint8_t rlen = 0;

	ETH_send_packet(testBuffer, 514);

    while(1){

		rlen = ETH_ReceivePacket(testBuffer, 514);
//		usartSend(0xC0);

		if (rlen != 0){
			usartSend(0xC1); //Packet received
			usartSend(rlen);
//			for (i=0; i <rlen; i++){
//					usartSend(testBuffer[i]);
//			}
			testBuffer[0]=0xE8;
			testBuffer[1]=0x03;
			testBuffer[2]=0x9A;
			testBuffer[3]=0xAE;
			testBuffer[4]=0x00;
			testBuffer[5]=0x9C;

			testBuffer[6]=0x00;
			testBuffer[7]=0xAA;
			testBuffer[8]=0xBB;
			testBuffer[9]=0xCC;
			testBuffer[10]=0xDE;
			testBuffer[11]=0x02;
			ETH_send_packet(testBuffer, rlen);
		}

//		for (i=0; i<1000000; ++i);
//		GPIO_WriteBit(GPIOB, GPIO_Pin_10, led_state ? Bit_SET : Bit_RESET);
//		led_state = !led_state;
//		GPIO_WriteBit(GPIOB, GPIO_Pin_11, led_state ? Bit_SET : Bit_RESET);
		}
}

void vApplicationMallocFailedHook( void )
{
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
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
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

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}
/*-----------------------------------------------------------*/



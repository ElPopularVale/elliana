#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "enc28j60.h"
#include "usart_jose.h"
#include <CoOS.h>
#include "uip.h"
#include "uip_arp.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

/*---------------------------- Symbol Define -------------------------------*/
#define STACK_SIZE_TASKA 128              /*!< Define "taskA" task size */
#define STACK_SIZE_TASKB 128              /*!< Define "taskA" task size */

/*---------------------------- Variable Define -------------------------------*/
OS_STK taskA_stk[STACK_SIZE_TASKA]; /*!< Define "taskA" task stack */
OS_STK taskB_stk[STACK_SIZE_TASKB]; /*!< Define "taskB" task stack */

//RTOS tasks
static void taskA(void);
static void taskB(void);
void IPstackIdle();

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

void IPstackIdle() {
	uip_len = enc28j60_recv_packet((uint8_t *) uip_buf, UIP_BUFSIZE);

	if (uip_len > 0) {
		if (BUF->type == htons(UIP_ETHTYPE_IP)) {
			uip_arp_ipin();
			uip_input();
			if (uip_len > 0) {
				uip_arp_out();
				enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
			}
		} else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {
			uip_arp_arpin();
			if (uip_len > 0) {
				enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
			}
		}
	}
}

int main(void) {
	struct uip_eth_addr mac = { { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 } };

	//Hardware initialization
	LED_init();
	usartInit();
	enc28j60_init(mac.addr);
	uip_init();
	uip_arp_init();
	hello_world_init();
	uip_setethaddr(mac);
	uip_ipaddr_t ipaddr;
	uip_ipaddr(ipaddr, 192, 168, 0, 200);
	uip_sethostaddr(ipaddr);
	uip_ipaddr(ipaddr, 192, 168, 0, 1);
	uip_setdraddr(ipaddr);
	uip_ipaddr(ipaddr, 255, 255, 255, 0);
	uip_setnetmask(ipaddr);
	usartSend(0xC0);

	CoInitOS();
	CoCreateTask((FUNCPtr)taskA, 0, 1, &taskA_stk[STACK_SIZE_TASKA-1],
			STACK_SIZE_TASKA);
	CoCreateTask((FUNCPtr)taskB, 0, 0, &taskB_stk[STACK_SIZE_TASKB-1],
			STACK_SIZE_TASKB);
	CoStartOS();
	while (1)
		;
}

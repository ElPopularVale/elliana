#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "usart_jose.h"
#include "uip.h"
#include "uip_arp.h"
#include "hello-world.h"
#include "enc28j60.h"

#define BUF ((struct uip_eth_hdr * )&uip_buf[0])

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

int main(void) {
	//local variables
	struct uip_eth_addr macAdress = { { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 } };

	//Hardware initialization
	usartInit();
	LED_init();

	enc28j60_init(macAdress.addr);
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
		uip_len = enc28j60_recv_packet((uint8_t *) uip_buf, UIP_BUFSIZE);
		if (uip_len > 0) {
			if (BUF->type == htons(UIP_ETHTYPE_IP)) {

				uip_arp_ipin();
				uip_input();
				if (uip_len > 0) {
					/* If the above function invocation resulted in data that
					 should be sent out on the network, the global variable
					 uip_len is set to a value > 0. */
					uip_arp_out();
					enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
					usartSend(0xC2);
				}
			} else if (BUF->type == htons(UIP_ETHTYPE_ARP)) {

				uip_arp_arpin();
				if (uip_len > 0) {
					enc28j60_send_packet((uint8_t *) uip_buf, uip_len);
					usartSend(0xC3);
				}
			}
		}
	}
}

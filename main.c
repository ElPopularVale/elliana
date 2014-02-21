
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0_spi_jose.h"
#include "enc28j60.h"
#include "usart_jose.h"

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

	usartSend(HIGH_BYTE(ENC_BUFFER_SIZE));
	usartSend(LOW_BYTE(ENC_BUFFER_SIZE));


	ETH_send_packet(testBuffer, 514);

    while(1){

		rlen = ETH_ReceivePacket(testBuffer, 514);
//		usartSend(0xC0);


//		usartSend(ETH_ReadETHControlRegister(EPKTCNT));
		if (rlen != 0){
			usartSend(0xC1); //Packet received
//			HIGH_BYTE(nextPacketPointer);
//			for (i=0; i <6; i++){
//					usartSend(testBuffer[i]);
//			}
		}
		for (i=0; i<1000000; ++i);
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, led_state ? Bit_SET : Bit_RESET);
		led_state = !led_state;
		GPIO_WriteBit(GPIOB, GPIO_Pin_11, led_state ? Bit_SET : Bit_RESET);
		}
}


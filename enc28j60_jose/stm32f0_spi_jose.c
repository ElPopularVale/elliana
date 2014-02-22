#include "stm32f0_spi_jose.h"
#include "enc28j60.h"
#include "usart_jose.h"

//Last bank selected
uint8_t last_bank = 0;

void ETH_SPI_init(){
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO clocks
	RCC_AHBPeriphClockCmd(
			SPIx_ETH_SCK_GPIO_CLK | SPIx_ETH_MISO_GPIO_CLK | SPIx_ETH_MOSI_GPIO_CLK
					| SPIx_ETH_NSS_GPIO_CLK, ENABLE);

	//Enable the SPI clock
	SPIx_ETH_CLK_INIT(SPIx_ETH_CLK, ENABLE);

	/* SPI GPIO Configuration --------------------------------------------------*/

	// Configure I/O for Chip select
	GPIO_InitStructure.GPIO_Pin = SPIx_ETH_NSS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPIx_ETH_NSS_GPIO_PORT, &GPIO_InitStructure);

	// SPI SCK pin configuration
	GPIO_InitStructure.GPIO_Pin = SPIx_ETH_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPIx_ETH_SCK_GPIO_PORT, &GPIO_InitStructure);

	// SPI  MOSI pin configuration
	GPIO_InitStructure.GPIO_Pin = SPIx_ETH_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPIx_ETH_MOSI_GPIO_PORT, &GPIO_InitStructure);

	// SPI  MISO pin configuration
	GPIO_InitStructure.GPIO_Pin = SPIx_ETH_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPIx_ETH_MISO_GPIO_PORT, &GPIO_InitStructure);

	// Connect SPI pins to AF0
	GPIO_PinAFConfig(SPIx_ETH_SCK_GPIO_PORT, SPIx_ETH_SCK_SOURCE, SPIx_ETH_SCK_AF);
	GPIO_PinAFConfig(SPIx_ETH_MOSI_GPIO_PORT, SPIx_ETH_MOSI_SOURCE,
			SPIx_ETH_MOSI_AF);
	GPIO_PinAFConfig(SPIx_ETH_MISO_GPIO_PORT, SPIx_ETH_MISO_SOURCE,
			SPIx_ETH_MOSI_AF);

	// SPI configuration
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8 ;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPIx_ETH, &SPI_InitStructure);
	SPI_RxFIFOThresholdConfig(SPIx_ETH, SPI_RxFIFOThreshold_QF);

	SPI_CalculateCRC(SPIx_ETH, DISABLE);
	SPI_Cmd(SPIx_ETH, ENABLE);
}

//SPI ETH read/write one byte.
uint8_t spi(uint8_t data)
{
	//Wait until the transmit buffer is empty
	while (SPI_I2S_GetFlagStatus(SPIx_ETH, SPI_I2S_FLAG_TXE) == RESET);
	//Dummy byte to keep the clock running and read the coming byte
	SPI_SendData8(SPIx_ETH, data);
	//Wait until data is received
	while (SPI_I2S_GetFlagStatus(SPIx_ETH, SPI_I2S_FLAG_RXNE) == RESET);
	//Save the value at the current location of the buffer pointer, then increase buffer pointer
	return SPI_ReceiveData8(SPIx_ETH);
}

uint8_t ETH_ReadETHControlRegister(uint8_t reg){
	uint8_t data = 0;

	//If the register is not one of the common ones for the banks and its bank is
	//different than the last bank selected then switch bank
	if (ETH_REG(reg) < EIE && ETH_BANK(reg) != last_bank)
		ETH_switch_bank(ETH_BANK(reg));
	ETHSELECT();
		//Send opcode (000, no need to put it) and register address
		spi(ETH_REG(reg));
		//Dummy byte to keep the clock running and read the coming byte
		data = spi(0);
	ETHDESELECT();
	return data;

}

uint8_t ETH_ReadMACandMIIControlRegister(uint8_t reg){
	uint8_t data = 0;
	//If the register is not one of the common ones for the banks and its bank is
	//different than the last bank selected then switch bank
	if (ETH_REG(reg) < EIE && ETH_BANK(reg) != last_bank)
		ETH_switch_bank(ETH_BANK(reg));
	ETHSELECT();
			//Send opcode (000, no need to put it) and register address
			spi(ETH_REG(reg));
			//Dummy byte TWICE to keep the clock running and read the coming byte
			data = spi(0);
			data = spi(0);
	ETHDESELECT();
	return data;

}

void ETH_WriteControlRegister(uint8_t reg, uint8_t data){
	//If the register is not one of the common ones for the banks and its bank is
	//different than the last bank selected then switch bank
	if (ETH_REG(reg) < EIE && ETH_BANK(reg) != last_bank)
		ETH_switch_bank(ETH_BANK(reg));
	ETHSELECT();
		//Send opcode (010) and register address
		spi((0b010 << 5) | ETH_REG(reg));
		//Send data
		spi(data);
	ETHDESELECT();
}

void ETH_WritePHYRegister(uint8_t reg, uint8_t valh, uint8_t vall)
{
  /* Wait for the MII interface to be ready */
  while ((ETH_ReadMACandMIIControlRegister(MISTAT) & BUSY));
  /* Write the data */
  ETH_WriteControlRegister(MIREGADR, reg);
  ETH_WriteControlRegister(MIWRL, vall);
  ETH_WriteControlRegister(MIWRH, valh);
}


void ETH_switch_bank(uint8_t bank)
{
	ETHSELECT();
		//Send Clear opcode (101) for ECON bits 1 and 2,(BSEL0 and BSEL0)
		spi((0b101 << 5) | ETH_REG(ECON1));
		spi(0x03);
	ETHDESELECT();
	ETHSELECT();
		//Send Set opcode (100) for the new bank
		spi((0b100 << 5) | ETH_REG(ECON1));
		spi(bank);
	ETHDESELECT();
	last_bank = bank;
}

void ETH_ReadBufferMemory(uint8_t *buffer, uint16_t length){
	uint16_t i;
	ETHSELECT();
		//Send opcode (001) and 5-bit constant 0x1A
		spi((0b001 << 5) | 0x1A);
		for(i = 0; i<length; i++){
			//Save the value at the current location of the buffer pointer, then increase buffer pointer
			*buffer++ = spi(0);
		}
	ETHDESELECT();
}

uint8_t ETH_ReadOneWordFromBufferMemory(){
	uint8_t data = 0;
	ETHSELECT();
		//Send opcode (001) and 5-bit constant 0x1A
		spi((0b001 << 5) | 0x1A);
		data = spi(0);
	ETHDESELECT();
	return data;

}

void ETH_WriteBufferMemory(uint8_t *buffer, uint16_t length){
	uint16_t i;
	ETHSELECT();
		//Send opcode (011) and 5-bit constant 0x1A
		spi((0b011 << 5) | 0x1A);
		for(i = 0; i<length; i++){
			//Send one byte of the buffer and increment pointer to buffer
			spi(*buffer++);
		}
	ETHDESELECT();
}

void ETH_SystemCommandSoftReset(){
	ETHSELECT();
		//Send opcode (111) and 5-bit constant 0x1F
		spi(0xFF);
	ETHDESELECT();
}

void ETH_bit_set(uint8_t reg, uint8_t mask)
{
	//If the register is not one of the common ones for the banks and its bank is
	//different than the last bank selected then switch bank
	if (ETH_REG(reg) < EIE && ETH_BANK(reg) != last_bank)
		ETH_switch_bank(ETH_BANK(reg));
	ETHSELECT();
		//Send opcode (100) and register address
		spi((0b100 << 5) | ETH_REG(reg));
		//Send bit mask to set
		spi(mask);
	ETHDESELECT();
}

void ETH_bit_clear(uint8_t reg, uint8_t mask)
{
	//If the register is not one of the common ones for the banks and its bank is
	//different than the last bank selected then switch bank
	if (ETH_REG(reg) < EIE && ETH_BANK(reg) != last_bank)
		ETH_switch_bank(ETH_BANK(reg));
	ETHSELECT();
		//Send opcode (101) and register address
		spi((0b101 << 5) | ETH_REG(reg));
		//Send bit mask to set
		spi(mask);
	ETHDESELECT();
}

void ETH_MAC_init(const uint8_t macAdress[6]){
	  /* Reset the NIC */
	  ETH_SystemCommandSoftReset();
	  /* Initialize NIC */
	  ETH_WriteControlRegister(ECON1, 0xC0); /* Hold it in Reset */
	  ETH_WriteControlRegister(ECON2, 0x80); /* Set the AUTOINC bit */
      /* Configure interrupts */
	  ETH_WriteControlRegister(EIR, 0x00); /* Reset interrupt flags */
	  ETH_WriteControlRegister(EIE, 0xC0);  /* Global INT Interrupt and Receive Packet Pending Interrupt enabled */
	  ETH_WriteControlRegister(EWOLIE, 0x00);  /* Disable wake-on-lan interrupts */
	  ETH_WriteControlRegister(EWOLIR, 0x00);  /* Clear wake-on-lan interrupt flags */
	  /* Allocate TX & RX buffers */
	  ETH_WriteControlRegister(ETXSTL, ENC_TXBUF_STL);
	  ETH_WriteControlRegister(ETXSTH, ENC_TXBUF_STH);
	  ETH_WriteControlRegister(ETXNDL, ENC_TXBUF_NDL);
	  ETH_WriteControlRegister(ETXNDH, ENC_TXBUF_NDH);  /* TX Buffer : 1536 (0x600) bytes */
	  ETH_WriteControlRegister(ERXSTL, ENC_RXBUF_STL);
	  ETH_WriteControlRegister(ERXSTH, ENC_RXBUF_STH);
	  ETH_WriteControlRegister(ERXNDL, ENC_RXBUF_NDL);
	  ETH_WriteControlRegister(ERXNDH, ENC_RXBUF_NDH);  /* RX Buffer : 6655 (0x19FF) bytes */
	  ETH_WriteControlRegister(ERDPTL, ENC_RXBUF_STL);
	  ETH_WriteControlRegister(ERDPTH, ENC_RXBUF_STH);
	  ETH_WriteControlRegister(ERXRDPTL, ENC_RXBUF_STL);
	  ETH_WriteControlRegister(ERXRDPTH, ENC_RXBUF_STH); /* RX Pointers (same) */
	  /* Set Receive Filters */
	  ETH_WriteControlRegister(ERXFCON, 0xA1); /* Unicast, CRC and Broadcast filters enabled */
	  /* Miscellaneous */
	  ETH_WriteControlRegister(EFLOCON, 0x00); /* Disable flow control sending */
	  ETH_WriteControlRegister(ECOCON, 0x00); /* Disable Clock Out pin */
	  ETH_WriteControlRegister(EBSTCON, 0x00); /* Disable built-in self test controller */
	  /* Wait for the MAC & PHY parts to be operational */
	  while (!(ETH_ReadETHControlRegister(ESTAT) & CLKRDY));
	  /* Initialize MII interface */
	  ETH_WriteControlRegister(MISTAT, 0x00);  /* Clear MII status register */
	  ETH_WriteControlRegister(MICMD, 0x00);   /* Clear MII command register */
	  ETH_WriteControlRegister(MICON, 0x00);  /* Release MII interface from reset */
	  /* Initialize MAC subsystem */
	  ETH_WriteControlRegister(MACON1, 0x0D);  /* Packet reception, Pause Control Frame Reception and Transmission enabled */
	  ETH_WriteControlRegister(MACON2, 0x00);  /* Release the MAC subsystem from reset */
	  ETH_WriteControlRegister(MACON3, 0x32);  /* Pad to 60 bytes and add CRC, Transmit CRC and Frame Length Checking enabled */
	  ETH_WriteControlRegister(MACON4, 0x00);  /* No preamble check */
	  ETH_WriteControlRegister(MAPHSUP, 0x10); /* Release MAC-PHY support from Reset */
	  ETH_WriteControlRegister(MAMXFLL, 0xEE);
	  ETH_WriteControlRegister(MAMXFLH, 0x05);  /* Maximum frame length = 1518 bytes */
	  ETH_WriteControlRegister(MABBIPG, 0x12); /* IEEE BTB Inter Packet Gap for half duplex*/
	  ETH_WriteControlRegister(MAIPGL, 0x12);
	  ETH_WriteControlRegister(MAIPGH, 0x0C);  /* IEEE Non BTB Inter Packet Gap for half duplex */
	  /* Leave MACLCON1 and MACLCON2 to their default values */
	  /* Set MAC Address */
	  ETH_WriteControlRegister(MAADR0, macAdress[5]);
	  ETH_WriteControlRegister(MAADR1, macAdress[4]);
	  ETH_WriteControlRegister(MAADR2, macAdress[3]);
	  ETH_WriteControlRegister(MAADR3, macAdress[2]);
	  ETH_WriteControlRegister(MAADR4, macAdress[1]);
	  ETH_WriteControlRegister(MAADR5, macAdress[0]);
	  /* Initialize PHY subsystem */
	  ETH_WritePHYRegister(PHCON1, 0x00, 0x00);  /* Half duplex, PHY in normal operation */
	  ETH_WritePHYRegister(PHCON2, 0x01, 0x00);	// Prevent automatic loopback of the data which is transmitted because of half duplex
	  ETH_WritePHYRegister(PHLCON, 0x02, 0x3A); // LEDA: Display link status, LEDB: Display tx and rx activity, Stretch LED pulses to 73 ms
	  ETH_WritePHYRegister(PHIE, 0x00, 0x00);  /* PHY interrupts disabled */
	  /* Initialization done, release the NIC from reset */
	  ETH_WriteControlRegister(ECON1, 0x14); //DMA Checksum and Receive packets enabled, bank 0 selected
	  last_bank = 0;
	  //Write the packet control byte to the beginning of the TX buffer so it doesn't have to be written every time a packet is sent
	  uint8_t buffer[1] = {0};
	  ETH_WriteBufferMemory(buffer, 1);
	  /* We're done */
}

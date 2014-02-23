#include "enc28j60_jose.h"
#include "stm32f0_spi_jose.h"

static unsigned int nextPacketPointer = ENC_RXBUF_ST;

void ETH_send_packet(uint8_t *buffer, uint16_t length) {

	//Wait for last transmission to end
	while ((ETH_ReadETHControlRegister(ECON1) & TXRTS)) {
		/* Rev. B4 Silicon Errata point 12 */
		if ((ETH_ReadETHControlRegister(EIR) & ETXERIF)) {
			ETH_bit_clear(ECON1, TXRTS);
		}
	}

	//Set the write pointer to start of transmit buffer area + 1 since the per packet control byte is already there from initialization
	ETH_WriteControlRegister(EWRPTL, LOW_BYTE((ENC_TXBUF_ST + 1)));
	ETH_WriteControlRegister(EWRPTH, HIGH_BYTE((ENC_TXBUF_ST + 1)));

	//Set the TXND pointer to correspond to the packet size given
	ETH_WriteControlRegister(ETXNDL, LOW_BYTE((ENC_TXBUF_ST + 1 + length)));
	ETH_WriteControlRegister(ETXNDH, HIGH_BYTE((ENC_TXBUF_ST + 1 + length)));

	//Copy the packet into the transmit buffer
	ETH_WriteBufferMemory(buffer, length);

	/* Start transmission */
	ETH_bit_set(ECON1, TXRTS);
}

uint16_t ETH_ReceivePacket(uint8_t* packet, uint16_t maxlen) {

	uint16_t rxstat;
	uint16_t len;

	//If the receive buffer is empty, get out
	if (ETH_ReadETHControlRegister(EPKTCNT) == 0) {
		return (0);
	}

	ETH_WriteControlRegister(ERDPTL, LOW_BYTE(nextPacketPointer));
	ETH_WriteControlRegister(ERDPTH, HIGH_BYTE(nextPacketPointer));

	//Read the next packet pointer (first two bytes of the buffer)
	nextPacketPointer = ETH_ReadOneWordFromBufferMemory();
	nextPacketPointer |= ETH_ReadOneWordFromBufferMemory();

	//Read length of received packet
	len = ETH_ReadOneWordFromBufferMemory();
	len |= ETH_ReadOneWordFromBufferMemory();

	//Remove CRC
	len -= 4;

	//Trim
	if (len > maxlen - 1) {
		len = maxlen - 1;
	}

	//Extract status bits
	rxstat = ETH_ReadOneWordFromBufferMemory();
	rxstat |= ETH_ReadOneWordFromBufferMemory();

	//If the received packet CRC is not OK then don't copy anything, else, copy to buffer
	if ((rxstat & 0x80) == 0) {
		len = 0;
	} else {
		ETH_ReadBufferMemory(packet, len);
	}

	// Errata workaround #13. Make sure ERXRDPT is odd
	uint16_t rs, re;
	rs = ETH_ReadETHControlRegister(ERXSTH);
	rs <<= 8;
	rs |= ETH_ReadETHControlRegister(ERXSTL);
	re = ETH_ReadETHControlRegister(ERXNDH);
	re <<= 8;
	re |= ETH_ReadETHControlRegister(ERXNDL);
	if (nextPacketPointer - 1 < rs || nextPacketPointer - 1 > re) {
		ETH_WriteControlRegister(ERXRDPTL, LOW_BYTE(re));
		ETH_WriteControlRegister(ERXRDPTH, HIGH_BYTE(re));
	} else {
		ETH_WriteControlRegister(ERXRDPTL, LOW_BYTE((nextPacketPointer-1)));
		ETH_WriteControlRegister(ERXRDPTH, HIGH_BYTE(nextPacketPointer-1));
	}

	//Decrement the packet counter indicating that we are done with this packet
	ETH_bit_set(ECON2, EPKTDEC);

	return len;
}

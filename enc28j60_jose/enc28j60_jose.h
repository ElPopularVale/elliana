#include "stdint.h" //Why is asking for this?

/* ENC28J60 Memory locations */
#define HIGH_BYTE(X)  (uint8_t)((X) >> 8)
#define LOW_BYTE(X)   (uint8_t)(X & 0xFF)

#define ENC_BUFFER_SIZE  0x2000
#define MAX_PACKET_SIZE 0x0600

#define ENC_RXBUF_ST  0x0000	//0x0000
#define ENC_TXBUF_ST  (ENC_RXBUF_ST + ENC_BUFFER_SIZE - MAX_PACKET_SIZE)	//0x1A00

#define ENC_RXBUF_STL LOW_BYTE(ENC_RXBUF_ST)
#define ENC_RXBUF_STH HIGH_BYTE(ENC_RXBUF_ST)
#define ENC_RXBUF_ND  (ENC_TXBUF_ST - 1)	//0x19FF
#define ENC_RXBUF_NDL LOW_BYTE(ENC_RXBUF_ND)
#define ENC_RXBUF_NDH HIGH_BYTE(ENC_RXBUF_ND)

#define ENC_TXBUF_STL LOW_BYTE(ENC_TXBUF_ST)
#define ENC_TXBUF_STH HIGH_BYTE(ENC_TXBUF_ST)
#define ENC_TXBUF_ND  (ENC_RXBUF_ST + ENC_BUFFER_SIZE)	//0x2000
#define ENC_TXBUF_NDL LOW_BYTE(ENC_TXBUF_ND)
#define ENC_TXBUF_NDH HIGH_BYTE(ENC_TXBUF_ND)

#define ENC_TX_BUF1   ENC_TXBUF_ST
#define ENC_TX_BUF2   ENC_TX_BUF1 + 300
#define ENC_TX_BUF3   ENC_TX_BUF2 + 300
#define ENC_TX_BUF4   ENC_TX_BUF3 + 300
#define ENC_TX_BUF5   ENC_TX_BUF4 + 300
#define ENC_TX_BUF6   ENC_TX_BUF5 + 300
#define ENC_TX_BUF7   ENC_TX_BUF6 + 300
#define ENC_TX_BUF8   ENC_TX_BUF7 + 300
#define ENC_TX_BUFNB  8

/* ENC28J60 SPI Commands */
#define ETH_READREG(REG)    (REG)
#define ETH_WRITEREG(REG)   ((1<<6)|(REG))
#define ETH_READMEM()       (BITS(0,0,1,1,1,0,1,0))
#define ETH_WRITEMEM()      (BITS(0,1,1,1,1,0,1,0))
#define ETH_BITSET(REG)     (0x80|(REG))
#define ETH_BITCLR(REG)     (0xA0|(REG))
#define ETH_RESET()         (0xFF)

/* Bank number hard coded in the register address */
#define ETH_BANK(REG)       (((REG & 0xE0) >> 5) & 0x3)
/* MAC & MII flag hard coded in the register address */
#define ETH_WAIT(REG)       ((REG >> 7) & 1)
/* The 'real' register address */
#define ETH_REG(REG)        (REG & 0x1F)

/* ENC28J60 Control Registers */
/* Three MSB must be bank number */
/* All banks */
#define EIE       0x1B
#define EIR       0x1C
#define ESTAT     0x1D
#define ECON2     0x1E
#define ECON1     0x1F
/* Bank 0 */
#define ERDPTL    0x00
#define ERDPTH    0x01
#define EWRPTL    0x02
#define EWRPTH    0x03
#define ETXSTL    0x04
#define ETXSTH    0x05
#define ETXNDL    0x06
#define ETXNDH    0x07
#define ERXSTL    0x08
#define ERXSTH    0x09
#define ERXNDL    0x0A
#define ERXNDH    0x0B
#define ERXRDPTL  0x0C
#define ERXRDPTH  0x0D
#define ERXWRPTL  0x0E
#define ERXWRPTH  0x0F
#define EDMASTL   0x10
#define EDMASTH   0x11
#define EDMANDL   0x12
#define EDMANDH   0x13
#define EDMADSTL  0x14
#define EDMADSTH  0x15
#define EDMACSL   0x16
#define EDMACSH   0x17
/* Bank 1 */
#define EHT0      0x20
#define EHT1      0x21
#define EHT2      0x22
#define EHT3      0x23
#define EHT4      0x24
#define EHT5      0x25
#define EHT6      0x26
#define EHT7      0x27
#define EPMM0     0x28
#define EPMM1     0x29
#define EPMM2     0x2A
#define EPMM3     0x2B
#define EPMM4     0x2C
#define EPMM5     0x2D
#define EPMM6     0x2E
#define EPMM7     0x2F
#define EPMCSL    0x30
#define EPMCSH    0x31
#define EPMOL     0x34
#define EPMOH     0x35
#define EWOLIE    0x36
#define EWOLIR    0x37
#define ERXFCON   0x38
#define EPKTCNT   0x39
/* Bank 2 */
#define MACON1    0xC0
#define MACON2    0xC1
#define MACON3    0xC2
#define MACON4    0xC3
#define MABBIPG   0xC4
#define MAIPGL    0xC6
#define MAIPGH    0xC7
#define MACLCON1  0xC8
#define MACLCON2  0xC9
#define MAMXFLL   0xCA
#define MAMXFLH   0xCB
#define MAPHSUP   0xCD
#define MICON     0xD1
#define MICMD     0xD2
#define MIREGADR  0xD4
#define MIWRL     0xD6
#define MIWRH     0xD7
#define MIRDL     0xD8
#define MIRDH     0xD9
/* Bank 3 */
#define MAADR1    0xE0
#define MAADR0    0xE1
#define MAADR3    0xE2
#define MAADR2    0xE3
#define MAADR5    0xE4
#define MAADR4    0xE5
#define EBSTSD    0x66
#define EBSTCON   0x67
#define EBSTCSL   0x68
#define EBSTCSH   0x69
#define MISTAT    0xEA
#define EREVID    0x72
#define ECOCON    0x75
#define EFLOCON   0x77
#define EPAUSL    0x78
#define EPAUSH    0x79
/* PHY registers */
#define PHCON1    0x00
#define PHSTAT1   0x01
#define PHID1     0x02
#define PHID2     0x03
#define PHCON2    0x10
#define PHSTAT2   0x11
#define PHIE      0x12
#define PHIR      0x13
#define PHLCON    0x14

/* Interrupts masks */
#define EINTIE  0x80
#define EPKTIF  0x40
#define EDMAIF  0x20
#define ELINKIF 0x10
#define ETXIF   0x08
#define EWOLIF  0x04
#define ETXERIF 0x02
#define ERXERIF 0x01

/* Interesting bits */
#define EPKTDEC 0x40
#define DMAST   0x20
#define CSUMEN  0x10
#define TXRTS   0x08
#define MIIRD   0x01
#define BUSY    0x01
#define CLKRDY  0x01
#define RXEN    0x04

void ETH_send_packet(uint8_t *buffer, uint16_t length);
uint16_t ETH_ReceivePacket(uint8_t* packet, uint16_t maxlen);


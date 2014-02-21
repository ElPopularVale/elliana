#include "stm32f0xx_spi.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

/* SPIx Communication boards Interface */
#define GPIO_AF_SPI2					 GPIO_AF_0
#define SPIx_ETH                         SPI2
#define SPIx_ETH_CLK                     RCC_APB1Periph_SPI2
#define SPIx_ETH_CLK_INIT                RCC_APB1PeriphClockCmd
#define SPIx_ETH_IRQn                    SPI2_IRQn
#define SPIx_ETH_IRQHANDLER              SPI2_IRQHandler

#define SPIx_ETH_SCK_PIN                 GPIO_Pin_13
#define SPIx_ETH_SCK_GPIO_PORT           GPIOB
#define SPIx_ETH_SCK_GPIO_CLK            RCC_AHBPeriph_GPIOB
#define SPIx_ETH_SCK_SOURCE              GPIO_PinSource13
#define SPIx_ETH_SCK_AF                  GPIO_AF_SPI2

#define SPIx_ETH_MISO_PIN                GPIO_Pin_14
#define SPIx_ETH_MISO_GPIO_PORT          GPIOB
#define SPIx_ETH_MISO_GPIO_CLK           RCC_AHBPeriph_GPIOB
#define SPIx_ETH_MISO_SOURCE             GPIO_PinSource14
#define SPIx_ETH_MISO_AF                 GPIO_AF_SPI2

#define SPIx_ETH_MOSI_PIN                GPIO_Pin_15
#define SPIx_ETH_MOSI_GPIO_PORT          GPIOB
#define SPIx_ETH_MOSI_GPIO_CLK           RCC_AHBPeriph_GPIOB
#define SPIx_ETH_MOSI_SOURCE             GPIO_PinSource15
#define SPIx_ETH_MOSI_AF                 GPIO_AF_SPI2

#define SPIx_ETH_NSS_PIN                 GPIO_Pin_12
#define SPIx_ETH_NSS_GPIO_PORT           GPIOB
#define SPIx_ETH_NSS_GPIO_CLK            RCC_AHBPeriph_GPIOB
#define SPIx_ETH_NSS_SOURCE              GPIO_PinSource12
#define SPIx_ETH_NSS_AF                  GPIO_AF_SPI2

//Pin select
#define ETHSELECT()        GPIOB->BRR = (1<<12) //pin low, ETH CS = L
#define ETHDESELECT()      GPIOB->BSRR = (1<<12) //pin high, ETH CS = H

void ETH_SPI_init();
uint8_t spi (uint8_t val);
uint8_t ETH_ReadETHControlRegister(uint8_t address);
uint8_t ETH_ReadMACandMIIControlRegister(uint8_t reg);
void ETH_WriteControlRegister(uint8_t address, uint8_t data);
void ETH_WritePHYRegister(uint8_t reg, uint8_t valh, uint8_t vall);
void ETH_switch_bank(uint8_t bank);
void ETH_ReadBufferMemory(uint8_t *buffer, uint16_t length);
uint8_t ETH_ReadOneWordFromBufferMemory();
void ETH_WriteBufferMemory(uint8_t *buffer, uint16_t length);
void ETH_SystemCommandSoftReset();
void ETH_bit_set(uint8_t reg, uint8_t mask);
void ETH_bit_clear(uint8_t reg, uint8_t mask);
void ETH_MAC_init(const uint8_t macAdress[6]);

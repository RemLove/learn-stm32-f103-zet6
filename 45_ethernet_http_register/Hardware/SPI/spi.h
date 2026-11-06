#ifndef __SPI_H__
#define __SPI_H__

#include"stm32f10x.h"

// CS->PB12  SCK->PB13  MOSI->PB15  MISO->PB14
#define CS_HIGH() (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW() (GPIOB->ODR &= ~GPIO_ODR_ODR12)

void SPI_Init(void);
void SPI_Start(void);
void SPI_Stop(void);
uint8_t SPI_SwapByte(uint8_t byte);
uint8_t SPI_ReadByte(void);      // 发 0xFF 读回一字节（W5500 读回调用）
void SPI_WriteByte(uint8_t byte); // 写一字节（W5500 写回调用）



#endif

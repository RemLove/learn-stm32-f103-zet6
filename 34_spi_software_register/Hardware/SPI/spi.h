#ifndef __SPI_H__
#define __SPI_H__

#include"stm32f10x.h"

// CS->PB12  SCK->PB13  MOSI->PB15  MISO->PB14  
#define CS_HIGH() (GPIOB->ODR |= GPIO_ODR_ODR12)
#define CS_LOW() (GPIOB->ODR &= ~GPIO_ODR_ODR12)

#define SCK_HIGH() (GPIOB->ODR |= GPIO_ODR_ODR13)
#define SCK_LOW() (GPIOB->ODR &= ~GPIO_ODR_ODR13)

#define MOSI_HIGH() (GPIOB->ODR |= GPIO_ODR_ODR15)
#define MOSI_LOW() (GPIOB->ODR &= ~GPIO_ODR_ODR15)

#define MISO_READ() (GPIOB->IDR & GPIO_IDR_IDR14)

void SPI_Init(void);
void SPI_Start(void);
void SPI_Stop(void);
uint8_t SPI_SwapByte(uint8_t byte);



#endif

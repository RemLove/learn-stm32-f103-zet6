#ifndef __I2C_H
#define __I2C_H

#include"stm32f10x.h"
#include"delay.h"

#define ACK 0
#define NACK 1
#define SCL_HIGH (GPIOB->ODR|=GPIO_ODR_ODR6)
#define SCL_LOW (GPIOB->ODR&=~GPIO_ODR_ODR6)
#define SDA_HIGH (GPIOB->ODR|=GPIO_ODR_ODR7)
#define SDA_LOW (GPIOB->ODR&=~GPIO_ODR_ODR7)

#define READ_SDA  ((GPIOB->IDR) & (GPIO_IDR_IDR7))
#define I2C_DELAY Delay_us(10)

#define OK 0
#define FAIL 1
void I2C_Init(void);

uint8_t I2C_Start(void);

void I2C_Stop(void);

void I2C_ACK(void);

void I2C_NACK(void);

uint8_t I2C_SendByte(uint8_t byte);

uint8_t I2C_SendAddr(uint8_t addr);

uint8_t I2C_ReadByte(void);

#endif

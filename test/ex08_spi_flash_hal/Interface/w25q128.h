#ifndef __W25Q128_H__
#define __W25Q128_H__

#include "stm32f1xx_hal.h"

/* 底层:SPI 全双工收发一个字节(发送 tx_data,返回收到的数据) */
uint8_t SPI_W25Q128_Write(uint8_t tx_data);

/* 读取 JEDEC 制造商 ID(W25Q128 应为 0xEF4018) */
uint32_t W25Q128_ReadID(void);

//写使能
void W25Q128_WriteEnable(void);

//等待空闲
void W25Q128_WaitBusy(void);

//扇区擦除
void W25Q128_SectorErase(uint32_t addr);

//读数据
void W25Q128_Read(uint32_t addr, uint8_t *buf, uint32_t len);


#endif // __W25Q128_H__

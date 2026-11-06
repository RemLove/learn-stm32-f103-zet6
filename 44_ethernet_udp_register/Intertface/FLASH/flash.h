#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"
#include "spi.h"
void FLASH_Init(void);

void FLASH_ReadID(uint8_t* mid ,uint16_t *Did);
//写使能
void FLASH_WriteEnable(void);
//写禁止
void FLASH_WriteDisable(void);
//等待不为忙
void FLASH_WaitNotBusy(void);
//段擦除    块 段 页
void FLASH_SectorErase(uint32_t addr);
//页写入    
void FLASH_PageWrite(uint32_t addr,uint8_t *buf,uint16_t len);
//页读取
void FLASH_PageRead(uint32_t addr,uint8_t *buf,uint16_t len);

#endif

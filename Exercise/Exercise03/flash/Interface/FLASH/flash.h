#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

/* 指令宏 */
#define FLASH_READID_CMD 0x9F          /* 你原来的，留着 */
#define FLASH_READSR1_CMD 0x05
#define FLASH_READ_CMD 0x03
#define FLASH_WRITE_ENABLE_CMD 0x06
#define FLASH_PAGE_PROGRAM_CMD 0x02
#define FLASH_SECTOR_ERASE_CMD 0x20

/* SR1 的位。写成名字不写 0x01/0x02，读代码时一眼看懂 */
#define SR1_BUSY 0x01
#define SR1_WEL  0x02

void Flash_Init(void);
//读id
uint32_t Flash_ReadId(void);
//读状态寄存器
uint8_t Flash_ReadSR1(void);
//写使能
void Flash_WriteEnable(void);
//等忙
void Flash_WaitBusy(uint32_t timeout_ms);
//扇擦除 4kb
void Flash_SectorErase(uint32_t addr);
//页编程 256字节
void Flash_PageProgram(uint32_t addr, const uint8_t *buf, uint16_t len);
//读数据 没有256字节的限制
void Flash_ReadData(uint32_t addr, uint8_t *buf, uint16_t len);

#endif

#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

#define FLASH_READID_CMD 0x9F

void Flash_Init(void);

uint32_t Flash_ReadId(void);


#endif

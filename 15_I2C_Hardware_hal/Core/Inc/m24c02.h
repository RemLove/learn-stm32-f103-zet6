#ifndef __M24C02_H
#define __M24C02_H

#include "i2c.h"
#include <stdint.h>

#define M24C02_W_ADDR   0xA0
#define M24C02_R_ADDR   0xA1
#define M24C02_PAGE_SIZE 16
#define M24C02_TIMEOUT   1000

void M24C02_WriteByte(uint8_t innerAddr, uint8_t byte);
uint8_t M24C02_ReadByte(uint8_t innerAddr);

void M24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t size);
void M24C02_ReadBytes(uint8_t innerAddr, uint8_t *buffer, uint8_t size);

#endif 

#include "m24c02.h"

void M24C02_WriteByte(uint8_t innerAddr, uint8_t byte)
{
    HAL_I2C_Mem_Write(&hi2c1, M24C02_W_ADDR, innerAddr,
                      I2C_MEMADD_SIZE_8BIT, &byte, 1, M24C02_TIMEOUT);
    while (HAL_I2C_IsDeviceReady(&hi2c1, M24C02_W_ADDR, 1, M24C02_TIMEOUT) != HAL_OK);
}

uint8_t M24C02_ReadByte(uint8_t innerAddr)
{
    uint8_t byte;
    HAL_I2C_Mem_Read(&hi2c1, M24C02_W_ADDR, innerAddr,
                     I2C_MEMADD_SIZE_8BIT, &byte, 1, M24C02_TIMEOUT);
    return byte;
}

void M24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t size)
{
    uint8_t remaining = size;
    uint8_t addr = innerAddr;
    uint8_t *p = bytes;

    while (remaining > 0) {
        uint8_t space = M24C02_PAGE_SIZE - (addr % M24C02_PAGE_SIZE);
        uint8_t chunk = (remaining < space) ? remaining : space;

        HAL_I2C_Mem_Write(&hi2c1, M24C02_W_ADDR, addr,
                          I2C_MEMADD_SIZE_8BIT, p, chunk, M24C02_TIMEOUT);
        while (HAL_I2C_IsDeviceReady(&hi2c1, M24C02_W_ADDR, 1, M24C02_TIMEOUT) != HAL_OK);

        p += chunk;
        addr += chunk;
        remaining -= chunk;
    }
}

void M24C02_ReadBytes(uint8_t innerAddr, uint8_t *buffer, uint8_t size)
{
    HAL_I2C_Mem_Read(&hi2c1, M24C02_W_ADDR, innerAddr,
                     I2C_MEMADD_SIZE_8BIT, buffer, size, M24C02_TIMEOUT);
}

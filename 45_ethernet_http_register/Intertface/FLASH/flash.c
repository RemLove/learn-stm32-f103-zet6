#include "flash.h"

void FLASH_Init(void)
{
    SPI_Init();
}
void FLASH_ReadID(uint8_t* mid ,uint16_t *Did)
{
    SPI_Start();
    //发送指令9fh
    SPI_SwapByte(0x9f);
    //读取ID
    *mid = SPI_SwapByte(0xff);


    *Did = 0;
    *Did |= SPI_SwapByte(0xff)<<8;
    *Did |= SPI_SwapByte(0xff)&0xff;
    SPI_Stop();
}

void FLASH_WriteEnable(void)
{
    SPI_Start();
    SPI_SwapByte(0x06);
    SPI_Stop();
}

void FLASH_WriteDisable(void)
{
    SPI_Start();
    SPI_SwapByte(0x04);
    SPI_Stop();
}

void FLASH_WaitNotBusy(void)
{
    SPI_Start();
    SPI_SwapByte(0x05);
    while(SPI_SwapByte(0xff)&0x01);
    SPI_Stop();
}

void FLASH_SectorErase(uint32_t addr)
{
    FLASH_WriteEnable();
    SPI_Start();
    SPI_SwapByte(0x20);
    SPI_SwapByte((addr>>16)&0xff);
    SPI_SwapByte((addr>>8)&0xff);
    SPI_SwapByte(addr&0xff);
    SPI_Stop();
    FLASH_WaitNotBusy();
}

void FLASH_PageWrite(uint32_t addr, uint8_t *buf, uint16_t len)
{
    FLASH_WriteEnable();
    SPI_Start();

    SPI_SwapByte(0x02);
    SPI_SwapByte((addr>>16)&0xff);
    SPI_SwapByte((addr>>8)&0xff);
    SPI_SwapByte(addr&0xff);

    for(uint16_t i=0;i<len;i++)
    {
        SPI_SwapByte(buf[i]);
    }
    SPI_Stop();
    FLASH_WaitNotBusy();
}

void FLASH_PageRead(uint32_t addr, uint8_t *buf, uint16_t len)
{
    SPI_Start();
    SPI_SwapByte(0x03);
    SPI_SwapByte((addr>>16)&0xff);
    SPI_SwapByte((addr>>8)&0xff);
    SPI_SwapByte(addr&0xff);
    for(uint16_t i=0;i<len;i++)
    {
        buf[i] = SPI_SwapByte(0xff);
    }
    SPI_Stop();
}

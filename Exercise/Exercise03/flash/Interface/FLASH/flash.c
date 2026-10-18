#include "flash.h"
#include "spi.h"

#define FLASH_CS_LOW()   HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)
//上电先拉高 然后延时10ms/官方说最低5ms
void Flash_Init(void)
{  
    FLASH_CS_HIGH();
    HAL_Delay(10);
}

static uint8_t Flash_SendCmd(uint8_t tx);

//上电稳定后 拉低CS 发送命令 再发数据 会依次收到 厂商ID,容量高字节 低字节 最后拉高CS
uint32_t Flash_ReadId(void)
{
    uint32_t id;
    FLASH_CS_LOW();
    Flash_SendCmd(FLASH_READID_CMD);
    id=(uint32_t)Flash_SendCmd(0xFF)<<16;
    id|=(uint32_t)Flash_SendCmd(0xFF)<<8;
    id|=(uint32_t)Flash_SendCmd(0xFF)<<0;
    FLASH_CS_HIGH();
    return id;
}

static uint8_t Flash_SendCmd(uint8_t tx)
{
    uint8_t rx=0xFF;
    if(HAL_SPI_TransmitReceive(&hspi2,&tx,&rx,1,100)!=HAL_OK)
    {
        return 0xFF;
    }
    return rx;
}

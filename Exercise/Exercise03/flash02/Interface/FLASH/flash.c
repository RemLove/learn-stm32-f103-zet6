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

uint8_t Flash_ReadSR1(void)
{
    FLASH_CS_LOW();
    Flash_SendCmd(FLASH_READSR1_CMD);
    uint8_t ret=Flash_SendCmd(0xFF);
    FLASH_CS_HIGH();
    return ret;
}

void Flash_WriteEnable(void)
{
    FLASH_CS_LOW();
    Flash_SendCmd(FLASH_WRITE_ENABLE_CMD);
    FLASH_CS_HIGH();
}

void Flash_WaitBusy(uint32_t timeout_ms)
{
    uint32_t start =HAL_GetTick();
    while(Flash_ReadSR1()&SR1_BUSY)
    {
        if(HAL_GetTick()-start>timeout_ms)
        {
            break;
        }
    }
}
//扇区擦除 4kb 4096个字节
void Flash_SectorErase(uint32_t addr)
{
    Flash_WriteEnable();
    FLASH_CS_LOW();
    Flash_SendCmd(FLASH_SECTOR_ERASE_CMD);
    Flash_SendCmd((uint8_t)(addr >> 16));
    Flash_SendCmd((uint8_t)(addr >> 8));
    Flash_SendCmd((uint8_t)(addr));

    FLASH_CS_HIGH();

    Flash_WaitBusy(500);   /* 扇区擦除最长约 400ms，给 500ms 余量 */
}
//批量发  页编程要连续推 256 字节，一字节一字节调 Flash_SendCmd会被 HAL 的函数开销拖死
static void Flash_TxBulk(const uint8_t *buf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi2, (uint8_t *)buf, len, 100);
}

void Flash_PageProgram(uint32_t addr, const uint8_t * buf, uint16_t len)
{
    Flash_WriteEnable();
    FLASH_CS_LOW();
    Flash_SendCmd(FLASH_PAGE_PROGRAM_CMD);
    Flash_SendCmd((uint8_t)(addr >> 16));
    Flash_SendCmd((uint8_t)(addr >> 8));
    Flash_SendCmd((uint8_t)(addr));

    Flash_TxBulk(buf, len);

    FLASH_CS_HIGH();

    Flash_WaitBusy(50);    /* 一页编程最长约 3ms，给 50ms 足够 */
}
static void Flash_RxBulk(uint8_t *buf, uint16_t len)
{
    HAL_SPI_Receive(&hspi2, buf, len, 100);
}
/* 批量收（只收）。
   F1 HAL 的暗坑：全双工主机模式下 HAL_SPI_Receive 内部实际是
   TransmitReceive(hspi, pData, pData, ...)，把你的接收缓冲当发送数据推出去。
   对 Flash 读没影响（它不看 MOSI），但你得知道有这回事。 */
void Flash_ReadData(uint32_t addr, uint8_t *buf, uint16_t len)
{
    FLASH_CS_LOW();

    Flash_SendCmd(FLASH_READ_CMD);
    Flash_SendCmd((uint8_t)(addr >> 16));
    Flash_SendCmd((uint8_t)(addr >> 8));
    Flash_SendCmd((uint8_t)(addr));

    Flash_RxBulk(buf, len);

    FLASH_CS_HIGH();
}

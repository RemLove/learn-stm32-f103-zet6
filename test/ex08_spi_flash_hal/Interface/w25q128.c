#include "w25q128.h"
#include "spi.h"

/* 底层:SPI 全双工收发一个字节 */
uint8_t SPI_W25Q128_Write(uint8_t tx_data)
{
  uint8_t rx_data = 0;

  if (HAL_SPI_TransmitReceive(&hspi2, &tx_data, &rx_data, 1, 1000) != HAL_OK)
  {
    return 0;
  }
  return rx_data;
}

/* 读取 JEDEC 制造商 ID */
uint32_t W25Q128_ReadID(void)
{
  uint32_t id = 0;

  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); /* 片选拉低 */
  SPI_W25Q128_Write(0x9F);                                 /* 发送读 ID 命令 */
  id  = (uint32_t)SPI_W25Q128_Write(0xFF) << 16;
  id |= (uint32_t)SPI_W25Q128_Write(0xFF) << 8;
  id |= (uint32_t)SPI_W25Q128_Write(0xFF);
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   /* 片选拉高 */

  return id;
}

//写使能
void W25Q128_WriteEnable(void)
{
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); /* 片选拉低 */
  SPI_W25Q128_Write(0x06);                                 /* 发送写使能命令 */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   /* 片选拉高 */
}

//等待空闲
void W25Q128_WaitBusy(void)
{
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); /* 片选拉低 */
  SPI_W25Q128_Write(0x05);                                 /* 发送读状态寄存器命令 */
  while (SPI_W25Q128_Write(0xFF) & 0x01) /* 等待 BUSY 位清空 */
  { 
  }
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   /* 片选拉高 */
}

//扇区擦除
void W25Q128_SectorErase(uint32_t addr)
{
  W25Q128_WriteEnable(); /* 写使能 */

  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); /* 片选拉低 */
  SPI_W25Q128_Write(0x20);                                 /* 发送扇区擦除命令 */
  SPI_W25Q128_Write((addr & 0xFF0000) >> 16);              /* 发送地址高字节 */
  SPI_W25Q128_Write((addr & 0x00FF00) >> 8);               /* 发送地址中字节 */
  SPI_W25Q128_Write(addr & 0x0000FF);                      /* 发送地址低字节 */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   /* 片选拉高 */

  W25Q128_WaitBusy(); /* 等待空闲 */
}

//读数据
void W25Q128_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); /* 片选拉低 */
  SPI_W25Q128_Write(0x03);                                 /* 发送读数据命令 */
  SPI_W25Q128_Write((addr & 0xFF0000) >> 16);              /* 发送地址高字节 */
  SPI_W25Q128_Write((addr & 0x00FF00) >> 8);               /* 发送地址中字节 */
  SPI_W25Q128_Write(addr & 0x0000FF);                      /* 发送地址低字节 */

  for (uint32_t i = 0; i < len; i++)
  {
    buf[i] = SPI_W25Q128_Write(0xFF); /* 接收数据 */
  }

  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);   /* 片选拉高 */
}


#ifndef __WIFI_H
#define __WIFI_H

#include "esp32.h"

//两种工作模式 STA AP
typedef enum
{
    STA=1,
    AP=2
}WIFI_MODE;
//wifi初始化传入工作模式
void WIFI_Init(WIFI_MODE mode);

void WIFI_TCP_ServertStart(void);
//发送数据一般是发送命令 命令包含ip 数据 长度
void WIFI_TCP_SendData(uint8_t id,uint8_t* data,uint16_t size);
//接收数据就是从缓冲区里面读数据 
void WIFI_TCP_ReadData(uint8_t* buff,uint16_t* size,uint8_t id,uint16_t ip,uint8_t port);

#endif

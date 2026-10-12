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
//接收数据就是从环形缓冲里攒一帧完整的数据出来
//buff:数据落地缓冲  cap:buff的容量(字节)  size:出参,本帧数据长度
void WIFI_TCP_ReadData(uint8_t* buff,uint16_t cap,uint16_t* size,
                       uint8_t* id,char* ip,uint16_t* port);

#endif

#ifndef CAN_H
#define CAN_H

#include "stm32f10x.h"
typedef struct 
{
    uint16_t id;
    uint8_t data[8];
    uint16_t len;
}RxMsg;

void CAN_Init(void);
void CAN_Send_Msg(uint32_t id,uint8_t *data,uint8_t len);
void CAN_Recive_Msg(RxMsg rxMsg[],uint8_t* MsgCount);
void CAN_FilterConfig(void);
#endif

#ifndef __TIME5_H
#define __TIME5_H

#include"stm32f10x.h"
#include"led.h"

void Time5_Init(void);
void Time5_Start(void);
void Time5_Stop(void);
void Time5_SetDuty(uint16_t duty);
#endif


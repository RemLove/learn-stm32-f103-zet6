#ifndef __USART_H
#define __USART_H

#include"stm32f10x.h"

//初始化
void USART_Init(void);
//发送一个字符
void USART_SendChar(uint8_t ch);
//接收一个字符
uint8_t USART_ReciveChar(void);


#endif

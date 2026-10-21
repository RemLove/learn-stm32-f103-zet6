#ifndef __KEY_H
#define __KEY_H

#include "usart.h"
#include "gpio.h"
//按键发生事件
typedef enum
{
    KEY_NONE=0,
    KEY_UP,
    KEY_DOWN,
    KEY_SHORT,
    KEY_LONG
}Key_Event_t;

extern volatile  Key_Event_t key_evt;

void Key_Scan(void);

#endif 

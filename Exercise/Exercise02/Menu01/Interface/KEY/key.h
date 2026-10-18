#ifndef __KEY_H
#define __KEY_H 

#include "stdint.h"
#include "gpio.h"
#include "usart.h"
typedef enum 
{
    KEY_NONE=0,
    KEY_UP,
    KEY_DOWN,
    KEY_SHORT,
    KEY_LONG
}Key_evt_t;

extern volatile Key_evt_t key_evt;

//三个按键 key_up(高电平有效)  pe4->key0    pe3->key1
void Key_Scan(void);


#endif

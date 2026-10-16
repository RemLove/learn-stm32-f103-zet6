#ifndef __KEY_H
#define __KEY_H 

#include "stdint.h"
#include "gpio.h"

extern  volatile uint8_t keyUp_flag;//判断keyUp是否按下的标志
extern  volatile uint8_t key0_flag;//判断key0是否按下的标志
extern  volatile uint8_t key1_flag;//判断key1是否按下的标志



//三个按键 key_up(高电平有效)  pe4->key0    pe3->key1
void Key_Scan(void);


#endif

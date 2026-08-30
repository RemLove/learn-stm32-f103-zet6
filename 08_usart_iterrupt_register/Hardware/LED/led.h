#ifndef __LED_H
#define __LED_H

#include"stm32f10x.h"
//宏定义
#define LED0 0//PB5
#define LED1 1//PE5
//初始化
void LED_Init(void);
//点亮
void LED_On(uint16_t led);
//熄灭
void LED_Off(uint16_t led);
//点亮一组LED
void LED_OnAll(uint16_t leds[],uint16_t size);
//熄灭一组LED
void LED_OffAll(uint16_t leds[],uint16_t size);
//反转LED(PE5)
void LED_PE5_Toggle(uint16_t led);

#endif

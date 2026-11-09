#ifndef __LED_H
#define __LED_H

#include "main.h"

typedef struct LedClass
{
    GPIO_TypeDef*port;
    uint16_t pin;
    GPIO_PinState OnBit;
    GPIO_PinState OffBit;
}Led_t;

extern Led_t led0;
extern Led_t led1;

void Led_On(Led_t* led);

void Led_Off(Led_t* led);

#endif

#ifndef __LED_H
#define __LED_H

#include "main.h"

#include <stdint.h>

typedef struct LedClass
{
    GPIO_TypeDef*port;
    uint16_t pin;
    GPIO_PinState OnBit;
    GPIO_PinState OffBit;

    void(*On)(struct LedClass*led);
    void(*Off)(struct LedClass*led);
    void(*Toggle)(struct LedClass*led);
}Led_t;

extern Led_t led0;
extern Led_t led1;

void Led_On(Led_t* led);

void Led_Off(Led_t* led);

void Led_Toggle(Led_t* led);

#endif

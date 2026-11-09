#include "led.h"
#include "gpio.h"

Led_t led0={LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET,GPIO_PIN_SET};
Led_t led1={LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET,GPIO_PIN_SET};

void Led_On(Led_t* led)
{
    HAL_GPIO_WritePin(led->port,led->pin,led->OnBit);
}

void Led_Off(Led_t* led)
{
    HAL_GPIO_WritePin(led->port,led->pin,led->OffBit);
}

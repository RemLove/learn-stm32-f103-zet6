#include "led.h"
#include "main.h"

static void Led_OnDefault(Led_t*led);
static void Led_OffDefault(Led_t*led);
static void Led_ToggleDefault(Led_t*led);
static void Led_ToggleThree(Led_t*led);

Led_t led0={LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET,GPIO_PIN_SET,
            Led_OnDefault,Led_OffDefault,Led_ToggleDefault};
Led_t led1={LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET,GPIO_PIN_SET,
            Led_OnDefault,Led_OffDefault,Led_ToggleThree};

void Led_On(Led_t* led)
{
    led->On(led);
}

void Led_Off(Led_t* led)
{
    led->Off(led);
}

void Led_Toggle(Led_t* led)
{
    led->Toggle(led);
}

static void Led_OnDefault(Led_t*led)
{
    HAL_GPIO_WritePin(led->port,led->pin,led->OnBit);
}

static void Led_OffDefault(Led_t*led)
{
    HAL_GPIO_WritePin(led->port,led->pin,led->OffBit);
}

static void Led_ToggleDefault(Led_t*led)
{
    HAL_GPIO_TogglePin(led->port,led->pin);
}

static void Led_ToggleThree(Led_t*led)
{
     uint8_t i;
    for (i = 0; i < 3; i++)
    {
        HAL_GPIO_TogglePin(led->port, led->pin);
        HAL_Delay(80);
        HAL_GPIO_TogglePin(led->port, led->pin);
        HAL_Delay(120);
    }
}

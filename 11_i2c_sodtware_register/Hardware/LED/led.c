#include"led.h"
//初始化
void LED_Init(void)
{
//1.开启GPIOB和GPIOE的时钟
RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
RCC->APB2ENR|=RCC_APB2ENR_IOPEEN;
//2.配置工作模式
GPIOB->CRL&=~GPIO_CRL_CNF5;
GPIOB->CRL|=GPIO_CRL_MODE5;

GPIOE->CRL&=~GPIO_CRL_CNF5;
GPIOE->CRL|=GPIO_CRL_MODE5;
//3.配置低电平点亮
GPIOB->ODR|=GPIO_ODR_ODR5;//现在是高电平 灭
GPIOE->ODR|=GPIO_ODR_ODR5;
}
// 点亮指定LED
void LED_On(uint16_t led)
{
    switch(led)
    {
        case LED0: GPIOB->ODR &= ~GPIO_ODR_ODR5; break; // PB5低电平点亮
        case LED1: GPIOE->ODR &= ~GPIO_ODR_ODR5; break; // PE5低电平点亮
        default: break;
    }
}

// 熄灭指定LED
void LED_Off(uint16_t led)
{
    switch(led)
    {
        case LED0: GPIOB->ODR |= GPIO_ODR_ODR5; break;  // PB5高电平熄灭
        case LED1: GPIOE->ODR |= GPIO_ODR_ODR5; break;  // PE5高电平熄灭
        default: break;
    }
}
//点亮一组LED
void LED_OnAll(uint16_t leds[],uint16_t size)
{
    for(uint16_t i=0;i<size;i++)
    {
        LED_On(leds[i]);
    }
}
//熄灭一组LED
void LED_OffAll(uint16_t leds[],uint16_t size)
{
    for(uint16_t i=0;i<size;i++)
    {
        LED_Off(leds[i]);
    }
}

void LED_PE5_Toggle(uint16_t led)
{
    // 反转 PE5 输出电平
    GPIOE->ODR ^= GPIO_ODR_ODR5;
}

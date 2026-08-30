#include "key.h"

void KEY_Init(void)
{
    //开启时钟 GPIO AFIO
    RCC->APB2ENR|=RCC_APB2ENR_IOPEEN;
    RCC->APB2ENR|=RCC_APB2ENR_AFIOEN;
    //2.配置GPIO的工作模式 输入上拉CNF10   输入模式MODE00   按键是PE4
    GPIOE->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);// 先清零4位配置，消除复位默认值
    GPIOE->CRL|=GPIO_CRL_CNF4_1;//10
    GPIOE->CRL&=~GPIO_CRL_MODE4;//11
    //需要拉高电平
    GPIOE->ODR|=GPIO_ODR_ODR4;
    //3.配置AFIO   PE4
    AFIO->EXTICR[1]|=AFIO_EXTICR2_EXTI4_PE;
    //4.配置EXTI 遇到下降沿产生中断 打开中断屏蔽（相当于开关）
    EXTI->FTSR|=EXTI_FTSR_TR4;
    EXTI->IMR|=EXTI_IMR_MR4;
    //5.配置NVIC
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(EXTI4_IRQn,3);
    NVIC_EnableIRQ(EXTI4_IRQn);
}
void EXTI4_IRQHandler(void)
{
    //先清除中断挂起标志位
    EXTI->PR|=EXTI_PR_PR4;
    //防抖
    Delay_ms(10);
    //如果是低电平，说明按下
    if((GPIOE->IDR&GPIO_IDR_IDR4)==0)
    {
        LED_PE5_Toggle(LED1);
    }

}

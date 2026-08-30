#include "time5.h"

void Time5_Init(void)
{
    //1.开启时钟
    RCC->APB2ENR|=RCC_APB2ENR_IOPAEN; //开启GPIOA时钟
    RCC->APB1ENR|=RCC_APB1ENR_TIM5EN; 
    //2.配置工作模式 PA1 复用推挽输出
    GPIOA->CRL|=GPIO_CRL_MODE1_1; //输出模式，最大输出速度50MHz
    GPIOA->CRL|=GPIO_CRL_CNF1_1; 
    GPIOA->CRL&=~GPIO_CRL_CNF1_0; //复用推挽输出
    //3.预分频器
    TIM5->PSC=7199; //7200分频，1ms计数一次
    //4.自动重装载寄存器
    TIM5->ARR=99; //1s计数一次
    //5.计数器上升沿触发
    TIM5->CR1&=~TIM_CR1_DIR; //向上计数   0
    //6.设置通道2为输出模式
    TIM5->CCMR1&=~TIM_CCMR1_CC2S;
    //6.设置通道2为PWM模式1    7->110
    TIM5->CCMR1|=TIM_CCMR1_OC2M_2;
    TIM5->CCMR1|=TIM_CCMR1_OC2M_1;
    TIM5->CCMR1&=~TIM_CCMR1_OC2M_0;
    //8.设置通道2的值
    TIM5->CCR2=50; //占空比50%
    //9.使能通道2输出
    TIM5->CCER|=TIM_CCER_CC2E;
}

void Time5_Start(void)
{
    TIM5->CR1|=TIM_CR1_CEN; //使能定时器
}

void Time5_Stop(void)
{
    TIM5->CR1&=~TIM_CR1_CEN; //失能定时器
}

void Time5_SetDuty(uint16_t duty)
{
    TIM5->CCR2=duty; //设置占空比
}

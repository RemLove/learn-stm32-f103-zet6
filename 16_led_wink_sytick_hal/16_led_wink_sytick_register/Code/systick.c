#include"systick.h"

void Systick_Init(void)
{
    //1.配置重装载值
    SysTick->LOAD|=72000;
    //2.配置时钟源
    SysTick->CTRL|=SysTick_CTRL_CLKSOURCE;
    //3.配置打开中断
    SysTick->CTRL|=SysTick_CTRL_TICKINT;
    //4.使能SysTick
    SysTick->CTRL|=SysTick_CTRL_ENABLE;
}
uint16_t cout=0;
void SysTick_Handler(void)
{
    cout++;
    if(cout==1000)//一秒
    {
        LED_PE5_Toggle(LED1);
        cout=0;
    }
}

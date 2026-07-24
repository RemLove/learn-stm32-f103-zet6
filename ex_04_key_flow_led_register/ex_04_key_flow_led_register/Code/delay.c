#include"delay.h"
void Delay_us(uint16_t us)
{
	SysTick->LOAD=72*us;
	SysTick->CTRL=0x05;
	while((SysTick_CTRL_COUNTFLAG&SysTick->CTRL)==0)
	{

	}
	//关闭定时器
	SysTick->CTRL&=~SysTick_CTRL_ENABLE;
}
void Delay_ms(uint16_t ms)
{
	while (ms--)
	{
		Delay_us(1000);
	}
	
}
void Delay_s(uint16_t s)
{
	while (s--)
	{
		Delay_ms(1000);
	}
	
}

#include"stm32f10x.h"
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
int main(void)//PB5,PE5
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
	while(1)
	{
		//PB5点亮 延迟 熄灭
		GPIOB->ODR&=~GPIO_ODR_ODR5;
		Delay_ms(500);
		GPIOB->ODR|=GPIO_ODR_ODR5;
		//PE5点亮 延迟 熄灭
		GPIOE->ODR&=~GPIO_ODR_ODR5;
		Delay_ms(500);
		GPIOE->ODR|=GPIO_ODR_ODR5;
	}
}


#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
int main(void)//PB5,PE5
{
	LED_Init();
	uint16_t leds[]={LED0,LED1};
	while(1)
	{
		for(uint16_t i=0;i<2;i++)
		{
			LED_On(leds[i]);
			Delay_ms(500);
			LED_Off(leds[i]);
			Delay_ms(500);
		}
	}
}


#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
uint16_t ison=0;
int main(void)//PB5,PE5
{
	LED_Init();
	KEY_Init();
	while(1)
	{
		if(ison)
		{
        	LED_On(LED0);
        	Delay_ms(500);
    		LED_Off(LED0);
        	LED_On(LED1);
        	Delay_ms(500);
        	LED_Off(LED1);
		}
		else
		{
			LED_Off(LED0);
			LED_Off(LED1);
		}
	}
	
}


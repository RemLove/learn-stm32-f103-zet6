#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"time5.h"
int main(void)//PB5,PE5
{
	LED_Init();
	Time5_Init();
	Time5_Start();
	uint16_t duty=0;
	uint8_t flag=0;
	while(1)
	{
		if(flag==0)
		{
			duty++;
			if(duty>=99)
			{
				flag=1;
			}
		}
		else
		{
			duty--;
			if(duty<=1)
			{
				flag=0;
			}
		}
		Time5_SetDuty(duty);
		Delay_ms(10);
	}
}

